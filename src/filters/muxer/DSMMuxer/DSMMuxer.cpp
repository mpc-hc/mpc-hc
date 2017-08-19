/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013, 2015-2017 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include <algorithm>
#include <MMReg.h>
#include "DSMMuxer.h"
#include "../../../DSUtil/DSUtil.h"

#ifdef STANDALONE_FILTER
#include <InitGuid.h>
#endif
#include <qnetwork.h>
#include "moreuuids.h"

#ifdef STANDALONE_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] = {
    {&MEDIATYPE_Stream, &MEDIASUBTYPE_DirectShowMedia}
};

const AMOVIESETUP_PIN sudpPins[] = {
    {const_cast<LPWSTR>(L"Input"), FALSE, FALSE, FALSE, TRUE, &CLSID_NULL, nullptr, 0, nullptr},
    {const_cast<LPWSTR>(L"Output"), FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, nullptr, _countof(sudPinTypesOut), sudPinTypesOut}
};

const AMOVIESETUP_FILTER sudFilter[] = {
    {&__uuidof(CDSMMuxerFilter), DSMMuxerName, MERIT_DO_NOT_USE, _countof(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory}
};

CFactoryTemplate g_Templates[] = {
    {sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CDSMMuxerFilter>, nullptr, &sudFilter[0]}
};

int g_cTemplates = _countof(g_Templates);

STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2(FALSE);
}

#include "../../FilterApp.h"

CFilterApp theApp;

#endif

template<typename T> static T myabs(T n)
{
    return n >= 0 ? n : -n;
}

static int GetByteLength(UINT64 data, int min = 0)
{
    int i = 7;
    while (i >= min && ((BYTE*)&data)[i] == 0) {
        i--;
    }
    return ++i;
}

//
// CDSMMuxerFilter
//

CDSMMuxerFilter::CDSMMuxerFilter(LPUNKNOWN pUnk, HRESULT* phr, bool fAutoChap, bool fAutoRes)
    : CBaseMuxerFilter(pUnk, phr, __uuidof(this))
    , m_fAutoChap(fAutoChap)
    , m_fAutoRes(fAutoRes)
    , m_rtPrevSyncPoint(_I64_MIN)
{
    if (phr) {
        *phr = S_OK;
    }
}

CDSMMuxerFilter::~CDSMMuxerFilter()
{
}

STDMETHODIMP CDSMMuxerFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    *ppv = nullptr;

    return
        __super::NonDelegatingQueryInterface(riid, ppv);
}

void CDSMMuxerFilter::MuxPacketHeader(IBitStream* pBS, dsmp_t type, UINT64 len)
{
    ASSERT(type < 32);

    int i = GetByteLength(len, 1);

    pBS->BitWrite(DSMSW, DSMSW_SIZE << 3);
    pBS->BitWrite(type, 5);
    pBS->BitWrite(i - 1, 3);
    pBS->BitWrite(len, i << 3);
}

void CDSMMuxerFilter::MuxFileInfo(IBitStream* pBS)
{
    int len = 1;
    CSimpleMap<CStringA, CStringA> si;

    for (int i = 0; i < GetSize(); i++) {
        CStringA key = CStringA(CString(GetKeyAt(i))), value = UTF16To8(GetValueAt(i));
        if (key.GetLength() != 4) {
            continue;
        }
        si.Add(key, value);
        len += 4 + value.GetLength() + 1;
    }

    MuxPacketHeader(pBS, DSMP_FILEINFO, len);
    pBS->BitWrite(DSMF_VERSION, 8);
    for (int i = 0; i < si.GetSize(); i++) {
        CStringA key = si.GetKeyAt(i), value = si.GetValueAt(i);
        pBS->ByteWrite((LPCSTR)key, 4);
        pBS->ByteWrite((LPCSTR)value, value.GetLength() + 1);
    }

}

void CDSMMuxerFilter::MuxStreamInfo(IBitStream* pBS, CBaseMuxerInputPin* pPin)
{
    int len = 1;
    CSimpleMap<CStringA, CStringA> si;

    for (int i = 0; i < pPin->GetSize(); i++) {
        CStringA key = CStringA(CString(pPin->GetKeyAt(i))), value = UTF16To8(pPin->GetValueAt(i));
        if (key.GetLength() != 4) {
            continue;
        }
        si.Add(key, value);
        len += 4 + value.GetLength() + 1;
    }

    if (len > 1) {
        MuxPacketHeader(pBS, DSMP_STREAMINFO, len);
        pBS->BitWrite(pPin->GetID(), 8);
        for (int i = 0; i < si.GetSize(); i++) {
            CStringA key = si.GetKeyAt(i), value = si.GetValueAt(i);
            pBS->ByteWrite((LPCSTR)key, 4);
            pBS->ByteWrite((LPCSTR)value, value.GetLength() + 1);
        }
    }
}

void CDSMMuxerFilter::MuxInit()
{
    m_sps.RemoveAll();
    m_isps.RemoveAll();
    m_rtPrevSyncPoint = _I64_MIN;
}

void CDSMMuxerFilter::MuxHeader(IBitStream* pBS)
{
    CString muxer;
    muxer.Format(_T("DSM Muxer (%S)"), __TIMESTAMP__);

    SetProperty(L"MUXR", CStringW(muxer));
    SetProperty(L"DATE", CStringW(CTime::GetCurrentTime().FormatGmt(_T("%Y-%m-%d %H:%M:%S"))));

    MuxFileInfo(pBS);

    POSITION pos = m_pPins.GetHeadPosition();
    while (pos) {
        CBaseMuxerInputPin* pPin = m_pPins.GetNext(pos);
        const CMediaType& mt = pPin->CurrentMediaType();

        ASSERT((mt.lSampleSize >> 30) == 0); // you don't need >1GB samples, do you?

        MuxPacketHeader(pBS, DSMP_MEDIATYPE, 5 + sizeof(GUID) * 3 + mt.FormatLength());
        pBS->BitWrite(pPin->GetID(), 8);
        pBS->ByteWrite(&mt.majortype, sizeof(mt.majortype));
        pBS->ByteWrite(&mt.subtype, sizeof(mt.subtype));
        pBS->BitWrite(mt.bFixedSizeSamples, 1);
        pBS->BitWrite(mt.bTemporalCompression, 1);
        pBS->BitWrite(mt.lSampleSize, 30);
        pBS->ByteWrite(&mt.formattype, sizeof(mt.formattype));
        pBS->ByteWrite(mt.Format(), mt.FormatLength());

        MuxStreamInfo(pBS, pPin);
    }

    // resources & chapters

    CInterfaceList<IDSMResourceBag> pRBs;
    pRBs.AddTail(this);

    CComQIPtr<IDSMChapterBag> pCB = (IUnknown*)(INonDelegatingUnknown*)this;

    pos = m_pPins.GetHeadPosition();
    while (pos) {
        for (CComPtr<IPin> pPin = m_pPins.GetNext(pos)->GetConnected(); pPin; pPin = GetUpStreamPin(GetFilterFromPin(pPin))) {
            if (m_fAutoRes) {
                CComQIPtr<IDSMResourceBag> pPB = GetFilterFromPin(pPin);
                if (pPB && !pRBs.Find(pPB)) {
                    pRBs.AddTail(pPB);
                }
            }

            if (m_fAutoChap) {
                if (!pCB || pCB->ChapGetCount() == 0) {
                    pCB = GetFilterFromPin(pPin);
                }
            }
        }
    }

    // resources

    pos = pRBs.GetHeadPosition();
    while (pos) {
        IDSMResourceBag* pRB = pRBs.GetNext(pos);

        for (DWORD i = 0, j = pRB->ResGetCount(); i < j; i++) {
            CComBSTR name, desc, mime;
            BYTE* pData = nullptr;
            DWORD len = 0;
            if (SUCCEEDED(pRB->ResGet(i, &name, &desc, &mime, &pData, &len, nullptr))) {
                CStringA utf8_name = UTF16To8(name);
                CStringA utf8_desc = UTF16To8(desc);
                CStringA utf8_mime = UTF16To8(mime);

                MuxPacketHeader(pBS, DSMP_RESOURCE,
                                1 +
                                utf8_name.GetLength() + 1 +
                                utf8_desc.GetLength() + 1 +
                                utf8_mime.GetLength() + 1 +
                                len);

                pBS->BitWrite(0, 2);
                pBS->BitWrite(0, 6); // reserved
                pBS->ByteWrite(utf8_name, utf8_name.GetLength() + 1);
                pBS->ByteWrite(utf8_desc, utf8_desc.GetLength() + 1);
                pBS->ByteWrite(utf8_mime, utf8_mime.GetLength() + 1);
                pBS->ByteWrite(pData, len);

                CoTaskMemFree(pData);
            }
        }
    }

    // chapters

    if (pCB) {
        CAtlList<CDSMChapter> chapters;
        REFERENCE_TIME rtPrev = 0;
        int len = 0;

        pCB->ChapSort();

        for (DWORD i = 0; i < pCB->ChapGetCount(); i++) {
            CDSMChapter c;
            CComBSTR name;
            if (SUCCEEDED(pCB->ChapGet(i, &c.rt, &name))) {
                REFERENCE_TIME rtDiff = c.rt - rtPrev;
                rtPrev = c.rt;
                c.rt = rtDiff;
                c.name = name;
                len += 1 + GetByteLength(myabs(c.rt)) + UTF16To8(c.name).GetLength() + 1;
                chapters.AddTail(c);
            }
        }

        if (chapters.GetCount()) {
            MuxPacketHeader(pBS, DSMP_CHAPTERS, len);

            pos = chapters.GetHeadPosition();
            while (pos) {
                CDSMChapter& c = chapters.GetNext(pos);
                CStringA name = UTF16To8(c.name);
                int irt = GetByteLength(myabs(c.rt));
                pBS->BitWrite(c.rt < 0, 1);
                pBS->BitWrite(irt, 3);
                pBS->BitWrite(0, 4);
                pBS->BitWrite(myabs(c.rt), irt << 3);
                pBS->ByteWrite((LPCSTR)name, name.GetLength() + 1);
            }
        }
    }
}

void CDSMMuxerFilter::MuxPacket(IBitStream* pBS, const MuxerPacket* pPacket)
{
    if (pPacket->IsEOS()) {
        return;
    }

    if (pPacket->pPin->CurrentMediaType().majortype == MEDIATYPE_Text) {
        CStringA str((char*)pPacket->pData.GetData(), (int)pPacket->pData.GetCount());
        str.Replace("\xff", " ");
        str.Replace("&nbsp;", " ");
        str.Replace("&nbsp", " ");
        str.Trim();
        if (str.IsEmpty()) {
            return;
        }
    }

    ASSERT(!pPacket->IsSyncPoint() || pPacket->IsTimeValid());

    REFERENCE_TIME rtTimeStamp = _I64_MIN, rtDuration = 0;
    int iTimeStamp = 0, iDuration = 0;

    if (pPacket->IsTimeValid()) {
        rtTimeStamp = pPacket->rtStart;
        rtDuration = std::max(pPacket->rtStop - pPacket->rtStart, 0ll);

        iTimeStamp = GetByteLength(myabs(rtTimeStamp));
        ASSERT(iTimeStamp <= 7);

        iDuration = GetByteLength(rtDuration);
        ASSERT(iDuration <= 7);

        IndexSyncPoint(pPacket, pBS->GetPos());
    }

    UINT64 len = 2 + iTimeStamp + iDuration + pPacket->pData.GetCount(); // id + flags + data

    MuxPacketHeader(pBS, DSMP_SAMPLE, len);
    pBS->BitWrite(pPacket->pPin->GetID(), 8);
    pBS->BitWrite(pPacket->IsSyncPoint(), 1);
    pBS->BitWrite(rtTimeStamp < 0, 1);
    pBS->BitWrite(iTimeStamp, 3);
    pBS->BitWrite(iDuration, 3);
    pBS->BitWrite(myabs(rtTimeStamp), iTimeStamp << 3);
    pBS->BitWrite(rtDuration, iDuration << 3);
    pBS->ByteWrite(pPacket->pData.GetData(), (int)pPacket->pData.GetCount());
}

void CDSMMuxerFilter::MuxFooter(IBitStream* pBS)
{
    // syncpoints

    int len = 0;
    CAtlList<IndexedSyncPoint> isps;
    REFERENCE_TIME rtPrev = 0, rt;
    UINT64 fpPrev = 0, fp;

    POSITION pos = m_isps.GetHeadPosition();
    while (pos) {
        IndexedSyncPoint& isp = m_isps.GetNext(pos);
        TRACE(_T("sp[%d]: %I64d %I64x\n"), isp.id, isp.rt, isp.fp);

        rt = isp.rt - rtPrev;
        rtPrev = isp.rt;
        fp = isp.fp - fpPrev;
        fpPrev = isp.fp;

        IndexedSyncPoint isp2;
        isp2.fp = fp;
        isp2.rt = rt;
        isps.AddTail(isp2);

        len += 1 + GetByteLength(myabs(rt)) + GetByteLength(fp); // flags + rt + fp
    }

    MuxPacketHeader(pBS, DSMP_SYNCPOINTS, len);

    pos = isps.GetHeadPosition();
    while (pos) {
        IndexedSyncPoint& isp = isps.GetNext(pos);

        int irt = GetByteLength(myabs(isp.rt));
        int ifp = GetByteLength(isp.fp);

        pBS->BitWrite(isp.rt < 0, 1);
        pBS->BitWrite(irt, 3);
        pBS->BitWrite(ifp, 3);
        pBS->BitWrite(0, 1); // reserved
        pBS->BitWrite(myabs(isp.rt), irt << 3);
        pBS->BitWrite(isp.fp, ifp << 3);
    }
}

void CDSMMuxerFilter::IndexSyncPoint(const MuxerPacket* p, __int64 fp)
{
    // Yes, this is as complicated as it looks.
    // Rule #1: don't write this packet if you can't do it reliably.
    // (think about overlapped subtitles, line1: 0->10, line2: 1->9)

    // FIXME: the very last syncpoints won't get moved to m_isps because there are no more syncpoints to trigger it!

    if (fp < 0 || !p || !p->IsTimeValid() || !p->IsSyncPoint()) {
        return;
    }

    ASSERT(p->rtStart >= m_rtPrevSyncPoint);
    m_rtPrevSyncPoint = p->rtStart;

    SyncPoint sp;
    sp.id = (BYTE)p->pPin->GetID();
    sp.rtStart = p->rtStart;
    sp.rtStop = p->pPin->IsSubtitleStream() ? p->rtStop : _I64_MAX;
    sp.fp = fp;

    {
        SyncPoint& head = !m_sps.IsEmpty() ? m_sps.GetHead() : sp;
        SyncPoint& tail = !m_sps.IsEmpty() ? m_sps.GetTail() : sp;
        REFERENCE_TIME rtfp = !m_isps.IsEmpty() ? m_isps.GetTail().rtfp : _I64_MIN;

        if (head.rtStart > rtfp + 1000000) { // 100ms limit, just in case every stream had only keyframes, then sycnpoints would be too frequent
            IndexedSyncPoint isp;
            isp.id = head.id;
            isp.rt = tail.rtStart;
            isp.rtfp = head.rtStart;
            isp.fp = head.fp;
            m_isps.AddTail(isp);
        }
    }

    POSITION pos = m_sps.GetHeadPosition();
    while (pos) {
        POSITION cur = pos;
        SyncPoint& sp2 = m_sps.GetNext(pos);
        if (sp2.id == sp.id && sp2.rtStop <= sp.rtStop || sp2.rtStop <= sp.rtStart) {
            m_sps.RemoveAt(cur);
        }
    }

    m_sps.AddTail(sp);
}
