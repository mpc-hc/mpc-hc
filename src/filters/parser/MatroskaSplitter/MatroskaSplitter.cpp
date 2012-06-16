/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
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
#include <MMReg.h>
#include "MatroskaSplitter.h"
#include "../../../DSUtil/AudioParser.h"

#ifdef STANDALONE_FILTER
#include <InitGuid.h>
#endif
#include "moreuuids.h"
#include <vector>

using namespace MatroskaReader;

#ifdef STANDALONE_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] = {
    {&MEDIATYPE_Stream, &MEDIASUBTYPE_Matroska},
    {&MEDIATYPE_Stream, &MEDIASUBTYPE_NULL}
};

const AMOVIESETUP_PIN sudpPins[] = {
    {L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, _countof(sudPinTypesIn), sudPinTypesIn},
    {L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, 0, NULL}
};

const AMOVIESETUP_FILTER sudFilter[] = {
    {&__uuidof(CMatroskaSplitterFilter), MatroskaSplitterName, MERIT_NORMAL, _countof(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory},
    {&__uuidof(CMatroskaSourceFilter), MatroskaSourceName, MERIT_NORMAL, 0, NULL, CLSID_LegacyAmFilterCategory},
};

CFactoryTemplate g_Templates[] = {
    {sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CMatroskaSplitterFilter>, NULL, &sudFilter[0]},
    {sudFilter[1].strName, sudFilter[1].clsID, CreateInstance<CMatroskaSourceFilter>, NULL, &sudFilter[1]},
};

int g_cTemplates = _countof(g_Templates);

STDAPI DllRegisterServer()
{
    RegisterSourceFilter(
        __uuidof(CMatroskaSourceFilter),
        MEDIASUBTYPE_Matroska,
        _T("0,4,,1A45DFA3"),
        _T(".mkv"), _T(".mka"), _T(".mks"), NULL);

    return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
    UnRegisterSourceFilter(MEDIASUBTYPE_Matroska);

    return AMovieDllRegisterServer2(FALSE);
}

#include "../../FilterApp.h"

CFilterApp theApp;

#endif

//
// CMatroskaSplitterFilter
//

CMatroskaSplitterFilter::CMatroskaSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr)
    : CBaseSplitterFilter(NAME("CMatroskaSplitterFilter"), pUnk, phr, __uuidof(this))
{
}

CMatroskaSplitterFilter::~CMatroskaSplitterFilter()
{
}

STDMETHODIMP CMatroskaSplitterFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    return
        QI(ITrackInfo)
        __super::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP CMatroskaSplitterFilter::QueryFilterInfo(FILTER_INFO* pInfo)
{
    CheckPointer(pInfo, E_POINTER);
    ValidateReadWritePtr(pInfo, sizeof(FILTER_INFO));

    if (m_pName && m_pName[0] == L'M' && m_pName[1] == L'P' && m_pName[2] == L'C') {
        (void)StringCchCopyW(pInfo->achName, NUMELMS(pInfo->achName), m_pName);
    } else {
        wcscpy_s(pInfo->achName, MatroskaSourceName);
    }
    pInfo->pGraph = m_pGraph;
    if (m_pGraph) {
        m_pGraph->AddRef();
    }

    return S_OK;
}

HRESULT CMatroskaSplitterFilter::CreateOutputs(IAsyncReader* pAsyncReader)
{
    CheckPointer(pAsyncReader, E_POINTER);

    HRESULT hr = E_FAIL;

    m_pFile.Free();
    m_pTrackEntryMap.RemoveAll();
    m_pOrderedTrackArray.RemoveAll();

    CAtlArray<CMatroskaSplitterOutputPin*> pinOut;
    CAtlArray<TrackEntry*> pinOutTE;

    m_pFile.Attach(DNew CMatroskaFile(pAsyncReader, hr));
    if (!m_pFile) {
        return E_OUTOFMEMORY;
    }
    if (FAILED(hr)) {
        m_pFile.Free();
        return hr;
    }

    m_rtNewStart = m_rtCurrent = 0;
    m_rtNewStop = m_rtStop = m_rtDuration = 0;

    int iVideo = 1, iAudio = 1, iSubtitle = 1;
    bool bHasVideo = 0;

    POSITION pos = m_pFile->m_segment.Tracks.GetHeadPosition();
    while (pos) {
        Track* pT = m_pFile->m_segment.Tracks.GetNext(pos);

        POSITION pos2 = pT->TrackEntries.GetHeadPosition();
        while (pos2) {
            TrackEntry* pTE = pT->TrackEntries.GetNext(pos2);

            bool isSub = false;

            if (!pTE->Expand(pTE->CodecPrivate, ContentEncoding::TracksPrivateData)) {
                continue;
            }

            CStringA CodecID = pTE->CodecID.ToString();

            CStringW Name;
            Name.Format(L"Output %I64d", (UINT64)pTE->TrackNumber);

            CMediaType mt;
            CAtlArray<CMediaType> mts;

            mt.SetSampleSize(1);

            if (pTE->TrackType == TrackEntry::TypeVideo) {
                Name.Format(L"Video %d", iVideo++);

                mt.majortype = MEDIATYPE_Video;

                if (CodecID == "V_MS/VFW/FOURCC") {
                    mt.formattype = FORMAT_VideoInfo;
                    VIDEOINFOHEADER* pvih = (VIDEOINFOHEADER*)mt.AllocFormatBuffer(sizeof(VIDEOINFOHEADER) + pTE->CodecPrivate.GetCount() - sizeof(BITMAPINFOHEADER));
                    memset(mt.Format(), 0, mt.FormatLength());
                    memcpy(&pvih->bmiHeader, pTE->CodecPrivate.GetData(), pTE->CodecPrivate.GetCount());
                    mt.subtype = FOURCCMap(pvih->bmiHeader.biCompression);
                    switch (pvih->bmiHeader.biCompression) {
                        case BI_RGB:
                        case BI_BITFIELDS:
                            mt.subtype =
                                pvih->bmiHeader.biBitCount == 1 ? MEDIASUBTYPE_RGB1 :
                                pvih->bmiHeader.biBitCount == 4 ? MEDIASUBTYPE_RGB4 :
                                pvih->bmiHeader.biBitCount == 8 ? MEDIASUBTYPE_RGB8 :
                                pvih->bmiHeader.biBitCount == 16 ? MEDIASUBTYPE_RGB565 :
                                pvih->bmiHeader.biBitCount == 24 ? MEDIASUBTYPE_RGB24 :
                                pvih->bmiHeader.biBitCount == 32 ? MEDIASUBTYPE_ARGB32 :
                                MEDIASUBTYPE_NULL;
                            break;
                            //                  case BI_RLE8: mt.subtype = MEDIASUBTYPE_RGB8; break;
                            //                  case BI_RLE4: mt.subtype = MEDIASUBTYPE_RGB4; break;
                    }
                    if (!bHasVideo) {
                        mts.Add(mt);
                        if (mt.subtype == MEDIASUBTYPE_WVC1) {
                            mt.subtype = MEDIASUBTYPE_WVC1_CYBERLINK;
                            mts.Add(mt);
                        }
                    }
                    bHasVideo = true;
                } else if (CodecID == "V_UNCOMPRESSED") {
                } else if (CodecID.Find("V_MPEG4/ISO/AVC") == 0 && pTE->CodecPrivate.GetCount() >= 6) {
                    BYTE sps = pTE->CodecPrivate[5] & 0x1f;

                    std::vector<BYTE> avcC;
                    for (size_t i = 0, j = pTE->CodecPrivate.GetCount(); i < j; i++) {
                        avcC.push_back(pTE->CodecPrivate[i]);
                    }

                    std::vector<BYTE> sh;

                    unsigned jj = 6;

                    while (sps--) {
                        if (jj + 2 > avcC.size()) {
                            goto avcfail;
                        }
                        unsigned spslen = ((unsigned)avcC[jj] << 8) | avcC[jj + 1];
                        if (jj + 2 + spslen > avcC.size()) {
                            goto avcfail;
                        }
                        unsigned cur = sh.size();
                        sh.resize(cur + spslen + 2, 0);
                        std::copy(avcC.begin() + jj, avcC.begin() + jj + 2 + spslen, sh.begin() + cur);
                        jj += 2 + spslen;
                    }

                    if (jj + 1 > avcC.size()) {
                        continue;
                    }

                    unsigned pps = avcC[jj++];

                    while (pps--) {
                        if (jj + 2 > avcC.size()) {
                            goto avcfail;
                        }
                        unsigned ppslen = ((unsigned)avcC[jj] << 8) | avcC[jj + 1];
                        if (jj + 2 + ppslen > avcC.size()) {
                            goto avcfail;
                        }
                        unsigned cur = sh.size();
                        sh.resize(cur + ppslen + 2, 0);
                        std::copy(avcC.begin() + jj, avcC.begin() + jj + 2 + ppslen, sh.begin() + cur);
                        jj += 2 + ppslen;
                    }

                    goto avcsuccess;
avcfail:
                    continue;
avcsuccess:

                    CAtlArray<BYTE> data;
                    data.SetCount(sh.size());
                    std::copy(sh.begin(), sh.end(), data.GetData());

                    mt.subtype = FOURCCMap('1CVA');
                    mt.formattype = FORMAT_MPEG2Video;
                    MPEG2VIDEOINFO* pm2vi = (MPEG2VIDEOINFO*)mt.AllocFormatBuffer(FIELD_OFFSET(MPEG2VIDEOINFO, dwSequenceHeader) + data.GetCount());
                    memset(mt.Format(), 0, mt.FormatLength());
                    pm2vi->hdr.bmiHeader.biSize = sizeof(pm2vi->hdr.bmiHeader);
                    pm2vi->hdr.bmiHeader.biWidth = (LONG)pTE->v.PixelWidth;
                    pm2vi->hdr.bmiHeader.biHeight = (LONG)pTE->v.PixelHeight;
                    pm2vi->hdr.bmiHeader.biCompression = '1CVA';
                    pm2vi->hdr.bmiHeader.biPlanes = 1;
                    pm2vi->hdr.bmiHeader.biBitCount = 24;
                    pm2vi->dwProfile = pTE->CodecPrivate[1];
                    pm2vi->dwLevel = pTE->CodecPrivate[3];
                    pm2vi->dwFlags = (pTE->CodecPrivate[4] & 3) + 1;
                    BYTE* pSequenceHeader = (BYTE*)pm2vi->dwSequenceHeader;
                    memcpy(pSequenceHeader, data.GetData(), data.GetCount());
                    pm2vi->cbSequenceHeader = (DWORD)data.GetCount();
                    if (!bHasVideo) {
                        mts.Add(mt);
                    }
                    bHasVideo = true;
                } else if (CodecID.Find("V_MPEG4/") == 0) {
                    mt.subtype = FOURCCMap('V4PM');
                    mt.formattype = FORMAT_MPEG2Video;
                    MPEG2VIDEOINFO* pm2vi = (MPEG2VIDEOINFO*)mt.AllocFormatBuffer(FIELD_OFFSET(MPEG2VIDEOINFO, dwSequenceHeader) + pTE->CodecPrivate.GetCount());
                    memset(mt.Format(), 0, mt.FormatLength());
                    pm2vi->hdr.bmiHeader.biSize = sizeof(pm2vi->hdr.bmiHeader);
                    pm2vi->hdr.bmiHeader.biWidth = (LONG)pTE->v.PixelWidth;
                    pm2vi->hdr.bmiHeader.biHeight = (LONG)pTE->v.PixelHeight;
                    pm2vi->hdr.bmiHeader.biCompression = 'V4PM';
                    pm2vi->hdr.bmiHeader.biPlanes = 1;
                    pm2vi->hdr.bmiHeader.biBitCount = 24;
                    BYTE* pSequenceHeader = (BYTE*)pm2vi->dwSequenceHeader;
                    memcpy(pSequenceHeader, pTE->CodecPrivate.GetData(), pTE->CodecPrivate.GetCount());
                    pm2vi->cbSequenceHeader = (DWORD)pTE->CodecPrivate.GetCount();
                    if (!bHasVideo) {
                        mts.Add(mt);
                    }
                    bHasVideo = true;
                } else if (CodecID.Find("V_REAL/RV") == 0) {
                    mt.subtype = FOURCCMap('00VR' + ((CodecID[9] - 0x30) << 16));
                    mt.formattype = FORMAT_VideoInfo;
                    VIDEOINFOHEADER* pvih = (VIDEOINFOHEADER*)mt.AllocFormatBuffer(sizeof(VIDEOINFOHEADER) + pTE->CodecPrivate.GetCount());
                    memset(mt.Format(), 0, mt.FormatLength());
                    memcpy(mt.Format() + sizeof(VIDEOINFOHEADER), pTE->CodecPrivate.GetData(), pTE->CodecPrivate.GetCount());
                    pvih->bmiHeader.biSize = sizeof(pvih->bmiHeader);
                    pvih->bmiHeader.biWidth = (LONG)pTE->v.PixelWidth;
                    pvih->bmiHeader.biHeight = (LONG)pTE->v.PixelHeight;
                    pvih->bmiHeader.biCompression = mt.subtype.Data1;
                    if (!bHasVideo) {
                        mts.Add(mt);
                    }
                    bHasVideo = true;
                } else if (CodecID == "V_DIRAC") {
                    mt.subtype = MEDIASUBTYPE_DiracVideo;
                    mt.formattype = FORMAT_DiracVideoInfo;
                    DIRACINFOHEADER* dvih = (DIRACINFOHEADER*)mt.AllocFormatBuffer(FIELD_OFFSET(DIRACINFOHEADER, dwSequenceHeader) + pTE->CodecPrivate.GetCount());
                    memset(mt.Format(), 0, mt.FormatLength());
                    dvih->hdr.bmiHeader.biSize = sizeof(dvih->hdr.bmiHeader);
                    dvih->hdr.bmiHeader.biWidth = (LONG)pTE->v.PixelWidth;
                    dvih->hdr.bmiHeader.biHeight = (LONG)pTE->v.PixelHeight;
                    dvih->hdr.dwPictAspectRatioX = dvih->hdr.bmiHeader.biWidth;
                    dvih->hdr.dwPictAspectRatioY = dvih->hdr.bmiHeader.biHeight;

                    BYTE* pSequenceHeader = (BYTE*)dvih->dwSequenceHeader;
                    memcpy(pSequenceHeader, pTE->CodecPrivate.GetData(), pTE->CodecPrivate.GetCount());
                    dvih->cbSequenceHeader = (DWORD)pTE->CodecPrivate.GetCount();

                    if (!bHasVideo) {
                        mts.Add(mt);
                    }
                    bHasVideo = true;
                } else if (CodecID == "V_MPEG2") {
                    BYTE* seqhdr = pTE->CodecPrivate.GetData();
                    DWORD len = (DWORD)pTE->CodecPrivate.GetCount();
                    int w = (int)pTE->v.PixelWidth;
                    int h = (int)pTE->v.PixelHeight;

                    if (MakeMPEG2MediaType(mt, seqhdr, len, w, h)) {
                        if (!bHasVideo) {
                            mts.Add(mt);
                        }
                        bHasVideo = true;
                    }
                } else if (CodecID == "V_THEORA") {
                    BYTE* thdr = pTE->CodecPrivate.GetData() + 3;

                    mt.majortype        = MEDIATYPE_Video;
                    mt.subtype          = FOURCCMap('OEHT');
                    mt.formattype       = FORMAT_MPEG2_VIDEO;
                    MPEG2VIDEOINFO* vih = (MPEG2VIDEOINFO*)mt.AllocFormatBuffer(sizeof(MPEG2VIDEOINFO) + pTE->CodecPrivate.GetCount());
                    memset(mt.Format(), 0, mt.FormatLength());
                    vih->hdr.bmiHeader.biSize        = sizeof(vih->hdr.bmiHeader);
                    vih->hdr.bmiHeader.biWidth       = *(WORD*)&thdr[10] >> 4;
                    vih->hdr.bmiHeader.biHeight      = *(WORD*)&thdr[12] >> 4;
                    vih->hdr.bmiHeader.biCompression = 'OEHT';
                    vih->hdr.bmiHeader.biPlanes      = 1;
                    vih->hdr.bmiHeader.biBitCount    = 24;
                    int nFpsNum = (thdr[22] << 24) | (thdr[23] << 16) | (thdr[24] << 8) | thdr[25];
                    int nFpsDenum   = (thdr[26] << 24) | (thdr[27] << 16) | (thdr[28] << 8) | thdr[29];
                    if (nFpsNum) {
                        vih->hdr.AvgTimePerFrame = (REFERENCE_TIME)(10000000.0 * nFpsDenum / nFpsNum);
                    }
                    vih->hdr.dwPictAspectRatioX = (thdr[14] << 16) | (thdr[15] << 8) | thdr[16];
                    vih->hdr.dwPictAspectRatioY = (thdr[17] << 16) | (thdr[18] << 8) | thdr[19];
                    mt.bFixedSizeSamples = 0;

                    vih->cbSequenceHeader = (DWORD)pTE->CodecPrivate.GetCount();
                    memcpy(&vih->dwSequenceHeader, pTE->CodecPrivate.GetData(), vih->cbSequenceHeader);

                    if (!bHasVideo) {
                        mts.Add(mt);
                    }
                    bHasVideo = true;
                } else if (CodecID.Find("V_VP8") == 0) {
                    mt.subtype = FOURCCMap('08PV');
                    mt.formattype = FORMAT_VideoInfo;
                    VIDEOINFOHEADER* pvih = (VIDEOINFOHEADER*)mt.AllocFormatBuffer(sizeof(VIDEOINFOHEADER) + pTE->CodecPrivate.GetCount());
                    memset(mt.Format(), 0, mt.FormatLength());
                    memcpy(mt.Format() + sizeof(VIDEOINFOHEADER), pTE->CodecPrivate.GetData(), pTE->CodecPrivate.GetCount());
                    pvih->bmiHeader.biSize = sizeof(pvih->bmiHeader);
                    pvih->bmiHeader.biWidth = (LONG)pTE->v.PixelWidth;
                    pvih->bmiHeader.biHeight = (LONG)pTE->v.PixelHeight;
                    pvih->bmiHeader.biCompression = mt.subtype.Data1;
                    if (!bHasVideo) {
                        mts.Add(mt);
                    }
                    bHasVideo = true;
                } else if (CodecID == "V_QUICKTIME" && pTE->CodecPrivate.GetCount() >= 8) {
                    DWORD* type;
                    if (m_pFile->m_ebml.DocTypeReadVersion == 1) {
                        type = (DWORD*)(pTE->CodecPrivate.GetData());
                    } else {
                        type = (DWORD*)(pTE->CodecPrivate.GetData() + 4);
                    }
                    if (*type == MAKEFOURCC('S', 'V', 'Q', '3') || *type == MAKEFOURCC('S', 'V', 'Q', '1') || *type == MAKEFOURCC('c', 'v', 'i', 'd')) {
                        mt.subtype = FOURCCMap(*type);
                        mt.formattype = FORMAT_VideoInfo;
                        VIDEOINFOHEADER* pvih = (VIDEOINFOHEADER*)mt.AllocFormatBuffer(sizeof(VIDEOINFOHEADER) + pTE->CodecPrivate.GetCount());
                        memset(mt.Format(), 0, mt.FormatLength());
                        memcpy(mt.Format() + sizeof(VIDEOINFOHEADER), pTE->CodecPrivate.GetData(), pTE->CodecPrivate.GetCount());
                        pvih->bmiHeader.biSize = sizeof(pvih->bmiHeader);
                        pvih->bmiHeader.biWidth = (LONG)pTE->v.PixelWidth;
                        pvih->bmiHeader.biHeight = (LONG)pTE->v.PixelHeight;
                        pvih->bmiHeader.biCompression = mt.subtype.Data1;
                        if (!bHasVideo) {
                            mts.Add(mt);
                        }
                        bHasVideo = true;
                    }
                } else if (CodecID == "V_DSHOW/MPEG1VIDEO"  || CodecID == "V_MPEG1") {
                    mt.majortype    = MEDIATYPE_Video;
                    mt.subtype      = MEDIASUBTYPE_MPEG1Payload;
                    mt.formattype   = FORMAT_MPEGVideo;

                    MPEG1VIDEOINFO* pm1vi = (MPEG1VIDEOINFO*)mt.AllocFormatBuffer(sizeof(MPEG1VIDEOINFO) + pTE->CodecPrivate.GetCount());
                    memset(mt.Format(), 0, mt.FormatLength());
                    memcpy(mt.Format() + sizeof(MPEG1VIDEOINFO), pTE->CodecPrivate.GetData(), pTE->CodecPrivate.GetCount());

                    pm1vi->hdr.bmiHeader.biSize         = sizeof(pm1vi->hdr.bmiHeader);
                    pm1vi->hdr.bmiHeader.biWidth        = (LONG)pTE->v.PixelWidth;
                    pm1vi->hdr.bmiHeader.biHeight       = (LONG)pTE->v.PixelHeight;
                    pm1vi->hdr.bmiHeader.biBitCount     = 12;
                    pm1vi->hdr.bmiHeader.biSizeImage    = DIBSIZE(pm1vi->hdr.bmiHeader);

                    mt.SetSampleSize(pm1vi->hdr.bmiHeader.biWidth * pm1vi->hdr.bmiHeader.biHeight * 4);
                    if (!bHasVideo) {
                        mts.Add(mt);
                    }
                    bHasVideo = true;
                }
                REFERENCE_TIME AvgTimePerFrame = 0;

                if (pTE->v.FramePerSec > 0) {
                    AvgTimePerFrame = (REFERENCE_TIME)(10000000i64 / pTE->v.FramePerSec);
                } else if (pTE->DefaultDuration > 0 && pTE->DefaultDuration != 1000000) {
                    AvgTimePerFrame = (REFERENCE_TIME)pTE->DefaultDuration / 100;
                } else { // pTE->DefaultDuration == 1000000 or < 0
                    CMatroskaNode Root(m_pFile);
                    m_pSegment = Root.Child(0x18538067);
                    m_pCluster = m_pSegment->Child(0x1F43B675);

                    MatroskaReader::QWORD lastCueClusterPosition = (MatroskaReader::QWORD) - 1;
                    UINT64 timecode1 = -1;
                    UINT64 timecode2 = -1;
                    unsigned int framecount = 0;
                    bool readmore = true;

                    POSITION pos1 = m_pFile->m_segment.Cues.GetHeadPosition();
                    while (readmore && pos1) {
                        Cue* pCue = m_pFile->m_segment.Cues.GetNext(pos1);
                        POSITION pos2 = pCue->CuePoints.GetHeadPosition();
                        while (readmore && pos2) {
                            CuePoint* pCuePoint = pCue->CuePoints.GetNext(pos2);
                            POSITION pos3 = pCuePoint->CueTrackPositions.GetHeadPosition();
                            while (readmore && pos3) {
                                CueTrackPosition* pCueTrackPositions = pCuePoint->CueTrackPositions.GetNext(pos3);
                                if (pCueTrackPositions->CueTrack != pTE->TrackNumber) {
                                    continue;
                                }

                                if (lastCueClusterPosition == pCueTrackPositions->CueClusterPosition) {
                                    continue;
                                }
                                lastCueClusterPosition = pCueTrackPositions->CueClusterPosition;

                                m_pCluster->SeekTo(m_pSegment->m_start + pCueTrackPositions->CueClusterPosition);
                                m_pCluster->Parse();

                                Cluster c;
                                c.ParseTimeCode(m_pCluster);

                                if (CAutoPtr<CMatroskaNode> pBlock = m_pCluster->GetFirstBlock()) {
                                    do {
                                        CBlockGroupNode bgn;

                                        if (pBlock->m_id == 0xA0) {
                                            bgn.Parse(pBlock, true);
                                        } else if (pBlock->m_id == 0xA3) {
                                            CAutoPtr<BlockGroup> bg(DNew BlockGroup());
                                            bg->Block.Parse(pBlock, true);
                                            if (!(bg->Block.Lacing & 0x80)) {
                                                bg->ReferenceBlock.Set(0);    // not a kf
                                            }
                                            bgn.AddTail(bg);
                                        }

                                        POSITION pos4 = bgn.GetHeadPosition();
                                        while (pos4) {
                                            BlockGroup* bg = bgn.GetNext(pos4);
                                            if (bg->Block.TrackNumber != pTE->TrackNumber) {
                                                continue;
                                            }
                                            UINT64 tc = c.TimeCode + bg->Block.TimeCode;
                                            if (tc == timecode2) {
                                                continue;
                                            }

                                            if (timecode1 == -1) {
                                                timecode1 = tc;
                                            } else {
                                                timecode2 = tc;
                                                ++framecount;
                                            }

                                            if (framecount >= 24) {
                                                // good for 23.976, 24, 25, 30, 50, 60 fps.
                                                // for 29.97 and 59,94 can give a small inaccuracy
                                                readmore = false;
                                                break;
                                            }
                                        }
                                    } while (readmore && pBlock->NextBlock());
                                }
                            }
                        }
                    }
                    if (framecount) {
                        AvgTimePerFrame = m_pFile->m_segment.SegmentInfo.TimeCodeScale * (timecode2 - timecode1) / (100 * framecount);
                    }

                    m_pCluster.Free();
                }
                //if (AvgTimePerFrame < 0) AvgTimePerFrame = 0;

                for (size_t i = 0; i < mts.GetCount(); i++) {
                    if (mts[i].formattype == FORMAT_VideoInfo
                            || mts[i].formattype == FORMAT_VideoInfo2
                            || mts[i].formattype == FORMAT_MPEG2Video
                            || mts[i].formattype == FORMAT_MPEGVideo) {
                        if (pTE->v.PixelWidth && pTE->v.PixelHeight) {
                            RECT rect = {(LONG)pTE->v.VideoPixelCropLeft,
                                         (LONG)pTE->v.VideoPixelCropTop,
                                         (LONG)(pTE->v.PixelWidth - pTE->v.VideoPixelCropRight),
                                         (LONG)(pTE->v.PixelHeight - pTE->v.VideoPixelCropBottom)
                                        };
                            VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)mts[i].Format();
                            vih->rcSource = vih->rcTarget = rect;
                        }

                        if (AvgTimePerFrame) {
                            if (mts[i].subtype == MEDIASUBTYPE_MPEG1Payload) {
                                AvgTimePerFrame *= 2; // Need more testing, but work on all sample that i have :)
                            }
                            ((VIDEOINFOHEADER*)mts[i].Format())->AvgTimePerFrame = AvgTimePerFrame;
                        }
                    }
                }

                if (pTE->v.DisplayWidth && pTE->v.DisplayHeight) {
                    for (size_t i = 0; i < mts.GetCount(); i++) {
                        if (mts[i].formattype == FORMAT_VideoInfo) {
                            mt = mts[i];
                            DWORD vih1 = FIELD_OFFSET(VIDEOINFOHEADER, bmiHeader);
                            DWORD vih2 = FIELD_OFFSET(VIDEOINFOHEADER2, bmiHeader);
                            DWORD bmi = mts[i].FormatLength() - FIELD_OFFSET(VIDEOINFOHEADER, bmiHeader);
                            mt.formattype = FORMAT_VideoInfo2;
                            mt.AllocFormatBuffer(vih2 + bmi);
                            memcpy(mt.Format(), mts[i].Format(), vih1);
                            memset(mt.Format() + vih1, 0, vih2 - vih1);
                            memcpy(mt.Format() + vih2, mts[i].Format() + vih1, bmi);

                            CSize aspect((int)pTE->v.DisplayWidth, (int)pTE->v.DisplayHeight);
                            int lnko = LNKO(aspect.cx, aspect.cy);
                            if (lnko > 1) {
                                aspect.cx /= lnko, aspect.cy /= lnko;
                            }
                            ((VIDEOINFOHEADER2*)mt.Format())->dwPictAspectRatioX = aspect.cx;
                            ((VIDEOINFOHEADER2*)mt.Format())->dwPictAspectRatioY = aspect.cy;
                            mts.InsertAt(i++, mt);
                        } else if (mts[i].formattype == FORMAT_MPEG2Video) {
                            CSize aspect((int)pTE->v.DisplayWidth, (int)pTE->v.DisplayHeight);
                            int lnko = LNKO(aspect.cx, aspect.cy);
                            if (lnko > 1) {
                                aspect.cx /= lnko, aspect.cy /= lnko;
                            }
                            ((MPEG2VIDEOINFO*)mts[i].Format())->hdr.dwPictAspectRatioX = aspect.cx;
                            ((MPEG2VIDEOINFO*)mts[i].Format())->hdr.dwPictAspectRatioY = aspect.cy;
                        }
                    }
                }
            } else if (pTE->TrackType == TrackEntry::TypeAudio) {
                Name.Format(L"Audio %d", iAudio++);

                mt.majortype = MEDIATYPE_Audio;
                mt.formattype = FORMAT_WaveFormatEx;
                WAVEFORMATEX* wfe = (WAVEFORMATEX*)mt.AllocFormatBuffer(sizeof(WAVEFORMATEX));
                memset(wfe, 0, mt.FormatLength());
                wfe->nChannels = (WORD)pTE->a.Channels;
                wfe->nSamplesPerSec = (DWORD)pTE->a.SamplingFrequency;
                wfe->wBitsPerSample = (WORD)pTE->a.BitDepth;
                wfe->nBlockAlign = (WORD)((wfe->nChannels * wfe->wBitsPerSample) / 8);
                wfe->nAvgBytesPerSec = wfe->nSamplesPerSec * wfe->nBlockAlign;
                mt.SetSampleSize(256000);

                if (CodecID == "A_MPEG/L3") {
                    mt.subtype = FOURCCMap(wfe->wFormatTag = WAVE_FORMAT_MPEGLAYER3);
                    mts.Add(mt);
                } else if (CodecID == "A_MPEG/L2" ||
                           CodecID == "A_MPEG/L1") {
                    mt.subtype = FOURCCMap(wfe->wFormatTag = WAVE_FORMAT_MPEG);
                    mts.Add(mt);
                } else if (CodecID == "A_AC3" ||
                           CodecID == "A_EAC3") {
                    mt.subtype = FOURCCMap(wfe->wFormatTag = WAVE_FORMAT_DOLBY_AC3);
                    mts.Add(mt);
                } else if (CodecID == "A_TRUEHD" ||
                           CodecID == "A_MLP") {
                    wfe->wFormatTag = WAVE_FORMAT_DOLBY_AC3;
                    mt.subtype = MEDIASUBTYPE_DOLBY_TRUEHD;
                    mts.Add(mt);
                } else if (CodecID == "A_DTS") {
                    mt.subtype = FOURCCMap(wfe->wFormatTag = WAVE_FORMAT_DVD_DTS);
                    mts.Add(mt);
                } else if (CodecID == "A_TTA1") {
                    mt.subtype = FOURCCMap(wfe->wFormatTag = WAVE_FORMAT_TTA1);
                    wfe->cbSize = 30;
                    wfe = (WAVEFORMATEX*)mt.ReallocFormatBuffer(sizeof(WAVEFORMATEX) + 30);
                    BYTE* p = (BYTE*)(wfe + 1);
                    memcpy(p, (const unsigned char*)"TTA1\x01\x00", 6);
                    memcpy(p + 6,  &wfe->nChannels, 2);
                    memcpy(p + 8,  &wfe->wBitsPerSample, 2);
                    memcpy(p + 10, &wfe->nSamplesPerSec, 4);
                    memset(p + 14, 0, 30 - 14);
                    mts.Add(mt);
                } else if (CodecID == "A_AAC") {
                    mt.subtype = FOURCCMap(wfe->wFormatTag = WAVE_FORMAT_AAC);
                    wfe->cbSize = (WORD)pTE->CodecPrivate.GetCount();
                    wfe = (WAVEFORMATEX*)mt.ReallocFormatBuffer(sizeof(WAVEFORMATEX) + pTE->CodecPrivate.GetCount());
                    memcpy(wfe + 1, pTE->CodecPrivate.GetData(), pTE->CodecPrivate.GetCount());
                    mts.Add(mt);
                } else if (CodecID == "A_WAVPACK4") {
                    mt.subtype = FOURCCMap(wfe->wFormatTag = WAVE_FORMAT_WAVPACK4);
                    wfe->cbSize = (WORD)pTE->CodecPrivate.GetCount();
                    wfe = (WAVEFORMATEX*)mt.ReallocFormatBuffer(sizeof(WAVEFORMATEX) + pTE->CodecPrivate.GetCount());
                    memcpy(wfe + 1, pTE->CodecPrivate.GetData(), pTE->CodecPrivate.GetCount());
                    mts.Add(mt);
                } else if (CodecID == "A_FLAC") {
                    wfe->wFormatTag = WAVE_FORMAT_FLAC;
                    wfe->cbSize = (WORD)pTE->CodecPrivate.GetCount();
                    wfe = (WAVEFORMATEX*)mt.ReallocFormatBuffer(sizeof(WAVEFORMATEX) + pTE->CodecPrivate.GetCount());
                    memcpy(wfe + 1, pTE->CodecPrivate.GetData(), pTE->CodecPrivate.GetCount());

                    mt.subtype = MEDIASUBTYPE_FLAC_FRAMED;
                    mts.Add(mt);
                    mt.subtype = MEDIASUBTYPE_FLAC;
                    mts.Add(mt);
                } else if (CodecID == "A_PCM/INT/LIT") {
                    mt.subtype = MEDIASUBTYPE_PCM;
                    mt.SetSampleSize(wfe->nBlockAlign);
                    if (pTE->a.Channels <= 2 && pTE->a.BitDepth <= 16) {
                        wfe->wFormatTag = WAVE_FORMAT_PCM;
                    } else {
                        WAVEFORMATEXTENSIBLE* wfex = (WAVEFORMATEXTENSIBLE*)mt.ReallocFormatBuffer(sizeof(WAVEFORMATEXTENSIBLE));
                        if (pTE->a.BitDepth & 7) {
                            wfex->Format.wBitsPerSample = (WORD)(pTE->a.BitDepth + 7) & 0xFFF8;
                            wfex->Format.nBlockAlign = wfex->Format.nChannels * wfex->Format.wBitsPerSample / 8;
                            wfex->Format.nAvgBytesPerSec = wfex->Format.nSamplesPerSec * wfex->Format.nBlockAlign;
                            mt.SetSampleSize(wfex->Format.nBlockAlign);
                        }
                        wfex->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
                        wfex->Format.cbSize = 22;
                        wfex->Samples.wValidBitsPerSample = (WORD)pTE->a.BitDepth;
                        wfex->dwChannelMask = GetDefChannelMask(wfex->Format.nChannels);
                        wfex->SubFormat = MEDIASUBTYPE_PCM;
                    }
                    mts.Add(mt);
                } else if (CodecID == "A_PCM/FLOAT/IEEE") {
                    mt.subtype = MEDIASUBTYPE_IEEE_FLOAT;
                    mt.SetSampleSize(wfe->nBlockAlign);
                    if (pTE->a.Channels <= 2) {
                        wfe->wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
                    } else {
                        WAVEFORMATEXTENSIBLE* wfex = (WAVEFORMATEXTENSIBLE*)mt.ReallocFormatBuffer(sizeof(WAVEFORMATEXTENSIBLE));
                        wfex->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
                        wfex->Format.cbSize = 22;
                        wfex->Samples.wValidBitsPerSample = (WORD)pTE->a.BitDepth;
                        wfex->dwChannelMask = GetDefChannelMask(wfex->Format.nChannels);
                        wfex->SubFormat = MEDIASUBTYPE_IEEE_FLOAT;
                    }
                    mts.Add(mt);
                } else if (CodecID == "A_MS/ACM") {
                    wfe = (WAVEFORMATEX*)mt.AllocFormatBuffer(pTE->CodecPrivate.GetCount());
                    memcpy(wfe, (WAVEFORMATEX*)pTE->CodecPrivate.GetData(), pTE->CodecPrivate.GetCount());
                    if (wfe->wFormatTag == WAVE_FORMAT_EXTENSIBLE && wfe->cbSize == 22) {
                        mt.subtype = ((WAVEFORMATEXTENSIBLE*)wfe)->SubFormat;
                    } else { mt.subtype = FOURCCMap(wfe->wFormatTag); }
                    mts.Add(mt);
                } else if (CodecID == "A_VORBIS") {
                    BYTE* p = pTE->CodecPrivate.GetData();
                    CAtlArray<int> sizes;
                    int totalsize = 0;
                    for (BYTE n = *p++; n > 0; n--) {
                        int size = 0;
                        do {
                            size += *p;
                        } while (*p++ == 0xff);
                        sizes.Add(size);
                        totalsize += size;
                    }
                    sizes.Add(pTE->CodecPrivate.GetCount() - (p - pTE->CodecPrivate.GetData()) - totalsize);
                    totalsize += sizes[sizes.GetCount() - 1];

                    if (sizes.GetCount() == 3) {
                        mt.subtype = MEDIASUBTYPE_Vorbis2;
                        mt.formattype = FORMAT_VorbisFormat2;
                        VORBISFORMAT2* pvf2 = (VORBISFORMAT2*)mt.AllocFormatBuffer(sizeof(VORBISFORMAT2) + totalsize);
                        memset(pvf2, 0, mt.FormatLength());
                        pvf2->Channels = (WORD)pTE->a.Channels;
                        pvf2->SamplesPerSec = (DWORD)pTE->a.SamplingFrequency;
                        pvf2->BitsPerSample = (DWORD)pTE->a.BitDepth;
                        BYTE* p2 = mt.Format() + sizeof(VORBISFORMAT2);
                        for (size_t i = 0; i < sizes.GetCount(); p += sizes[i], p2 += sizes[i], i++) {
                            memcpy(p2, p, pvf2->HeaderSize[i] = sizes[i]);
                        }

                        mts.Add(mt);
                    }

                    mt.subtype = MEDIASUBTYPE_Vorbis;
                    mt.formattype = FORMAT_VorbisFormat;
                    VORBISFORMAT* vf = (VORBISFORMAT*)mt.AllocFormatBuffer(sizeof(VORBISFORMAT));
                    memset(vf, 0, mt.FormatLength());
                    vf->nChannels = (WORD)pTE->a.Channels;
                    vf->nSamplesPerSec = (DWORD)pTE->a.SamplingFrequency;
                    vf->nMinBitsPerSec = vf->nMaxBitsPerSec = vf->nAvgBitsPerSec = (DWORD) - 1;
                    mts.Add(mt);
                } else if (CodecID.Find("A_AAC/") == 0) {
                    mt.subtype = FOURCCMap(wfe->wFormatTag = WAVE_FORMAT_AAC);
                    wfe = (WAVEFORMATEX*)mt.ReallocFormatBuffer(sizeof(WAVEFORMATEX) + 5);
                    wfe->cbSize = 2;

                    int profile;

                    if (CodecID.Find("/MAIN") > 0) {
                        profile = 0;
                    } else if (CodecID.Find("/SBR") > 0) {
                        profile = -1;
                    } else if (CodecID.Find("/LC") > 0) {
                        profile = 1;
                    } else if (CodecID.Find("/SSR") > 0) {
                        profile = 2;
                    } else if (CodecID.Find("/LTP") > 0) {
                        profile = 3;
                    } else {
                        continue;
                    }

                    WORD cbSize = MakeAACInitData((BYTE*)(wfe + 1), profile, wfe->nSamplesPerSec, (int)pTE->a.Channels);

                    mts.Add(mt);

                    if (profile < 0) {
                        wfe->cbSize = cbSize;
                        wfe->nSamplesPerSec *= 2;
                        wfe->nAvgBytesPerSec *= 2;

                        mts.InsertAt(0, mt);
                    }
                } else if (CodecID.Find("A_REAL/") == 0 && CodecID.GetLength() >= 11) {
                    mt.subtype = FOURCCMap((DWORD)CodecID[7] | ((DWORD)CodecID[8] << 8) | ((DWORD)CodecID[9] << 16) | ((DWORD)CodecID[10] << 24));
                    mt.bTemporalCompression = TRUE;
                    wfe->cbSize = (WORD)pTE->CodecPrivate.GetCount();
                    wfe = (WAVEFORMATEX*)mt.ReallocFormatBuffer(sizeof(WAVEFORMATEX) + pTE->CodecPrivate.GetCount());
                    memcpy(wfe + 1, pTE->CodecPrivate.GetData(), pTE->CodecPrivate.GetCount());
                    wfe->cbSize = 0; // IMPORTANT: this is screwed, but cbSize has to be 0 and the extra data from codec priv must be after WAVEFORMATEX
                    mts.Add(mt);
                } else if (CodecID == "A_QUICKTIME" && pTE->CodecPrivate.GetCount() >= 8) {
                    DWORD* type = (DWORD*)(pTE->CodecPrivate.GetData() + 4);
                    if (*type == MAKEFOURCC('Q', 'D', 'M', '2') || *type == MAKEFOURCC('Q', 'D', 'M', 'C')) {
                        mt.subtype = FOURCCMap(*type);
                        wfe->cbSize = (WORD)pTE->CodecPrivate.GetCount();
                        wfe = (WAVEFORMATEX*)mt.ReallocFormatBuffer(sizeof(WAVEFORMATEX) + pTE->CodecPrivate.GetCount());
                        memcpy(wfe + 1, pTE->CodecPrivate.GetData(), pTE->CodecPrivate.GetCount());
                        mts.Add(mt);
                    }
                }
            } else if (pTE->TrackType == TrackEntry::TypeSubtitle) {
                if (iSubtitle == 1) {
                    InstallFonts();
                }

                Name.Format(L"Subtitle %d", iSubtitle++);

                mt.SetSampleSize(1);

                if (CodecID == "S_TEXT/ASCII") {
                    mt.majortype = MEDIATYPE_Text;
                    mt.subtype = MEDIASUBTYPE_NULL;
                    mt.formattype = FORMAT_None;
                    mts.Add(mt);
                    isSub = true;
                } else {
                    mt.majortype = MEDIATYPE_Subtitle;
                    mt.formattype = FORMAT_SubtitleInfo;
                    SUBTITLEINFO* psi = (SUBTITLEINFO*)mt.AllocFormatBuffer(sizeof(SUBTITLEINFO) + pTE->CodecPrivate.GetCount());
                    memset(psi, 0, mt.FormatLength());
                    strncpy_s(psi->IsoLang, pTE->Language, _countof(psi->IsoLang) - 1);
                    CString subtitle_Name = pTE->Name;
                    if (pTE->FlagForced) { // "Forced" overrides "Default"
                        subtitle_Name += L" [Forced]";
                    } else if (pTE->FlagDefault) {
                        subtitle_Name += L" [Default]";
                    }
                    subtitle_Name = subtitle_Name.Trim();

                    wcsncpy_s(psi->TrackName, subtitle_Name, _countof(psi->TrackName) - 1);
                    memcpy(mt.pbFormat + (psi->dwOffset = sizeof(SUBTITLEINFO)), pTE->CodecPrivate.GetData(), pTE->CodecPrivate.GetCount());

                    mt.subtype =
                        CodecID == "S_TEXT/UTF8" ? MEDIASUBTYPE_UTF8 :
                        CodecID == "S_TEXT/SSA" || CodecID == "S_SSA" ? MEDIASUBTYPE_SSA :
                        CodecID == "S_TEXT/ASS" || CodecID == "S_ASS" ? MEDIASUBTYPE_ASS :
                        CodecID == "S_TEXT/USF" || CodecID == "S_USF" ? MEDIASUBTYPE_USF :
                        CodecID == "S_HDMV/PGS" ? MEDIASUBTYPE_HDMVSUB :
                        //CodecID == "S_DVBSUB" ? MEDIASUBTYPE_DVB_SUBTITLES : // does not work
                        CodecID == "S_VOBSUB" ? MEDIASUBTYPE_VOBSUB :
                        MEDIASUBTYPE_NULL;

                    if (mt.subtype != MEDIASUBTYPE_NULL) {
                        mts.Add(mt);
                        isSub = true;
                    }
                }
            }

            if (mts.IsEmpty()) {
                TRACE(_T("CMatroskaSourceFilter: Unsupported TrackType %s (%I64d)\n"), CString(CodecID), (UINT64)pTE->TrackType);
                continue;
            }

            Name = CStringW(pTE->Language.IsEmpty() ? L"English" : CStringW(ISO6392ToLanguage(pTE->Language)))
                   + (pTE->Name.IsEmpty() ? L"" : L", " + pTE->Name)
                   + (L" (" + Name + L")");

            if (pTE->FlagForced) { // "Forced" overrides "Default"
                Name = Name + L" [Forced]";
            } else if (pTE->FlagDefault) {
                Name = Name + L" [Default]";
            }

            HRESULT hr;

            CAutoPtr<CBaseSplitterOutputPin> pPinOut(DNew CMatroskaSplitterOutputPin(pTE->MinCache, pTE->DefaultDuration / 100, mts, Name, this, this, &hr));
            if (!pTE->Name.IsEmpty()) {
                pPinOut->SetProperty(L"NAME", pTE->Name);
            }
            if (pTE->Language.GetLength() == 3) {
                pPinOut->SetProperty(L"LANG", CStringW(CString(pTE->Language)));
            }

            if (!isSub) {
                pinOut.InsertAt((iVideo + iAudio - 3), DNew CMatroskaSplitterOutputPin(pTE->MinCache, pTE->DefaultDuration / 100, mts, Name, this, this, &hr), 1);
                pinOutTE.InsertAt((iVideo + iAudio - 3), pTE, 1);
            } else {
                pinOut.Add(DNew CMatroskaSplitterOutputPin(pTE->MinCache, pTE->DefaultDuration / 100, mts, Name, this, this, &hr));
                pinOutTE.Add(pTE);
            }

        }
    }

    for (size_t i = 0; i < pinOut.GetCount(); i++) {

        CAutoPtr<CBaseSplitterOutputPin> pPinOut;
        pPinOut.Attach(pinOut[i]);
        TrackEntry* pTE = pinOutTE[i];

        if (pTE != NULL) {
            AddOutputPin((DWORD)pTE->TrackNumber, pPinOut);
            m_pTrackEntryMap[(DWORD)pTE->TrackNumber] = pTE;
            m_pOrderedTrackArray.Add(pTE);
        }
    }


    Info& info = m_pFile->m_segment.SegmentInfo;

    if (m_pFile->IsRandomAccess()) {
        m_rtDuration = (REFERENCE_TIME)(info.Duration * info.TimeCodeScale / 100);
    }

    m_rtNewStop = m_rtStop = m_rtDuration;

    /*#ifdef _DEBUG
        for (int i = 1, j = GetChapterCount(CHAPTER_ROOT_ID); i <= j; i++)
        {
            UINT id = GetChapterId(CHAPTER_ROOT_ID, i);
            struct ChapterElement ce;
            BOOL b = GetChapterInfo(id, &ce);
            BSTR bstr = GetChapterStringInfo(id, "eng", "");
            if (bstr) ::SysFreeString(bstr);
        }
    #endif*/

    SetProperty(L"TITL", info.Title);
    // TODO

    // resources

    {
        POSITION pos = m_pFile->m_segment.Attachments.GetHeadPosition();
        while (pos) {
            Attachment* pA = m_pFile->m_segment.Attachments.GetNext(pos);

            POSITION pos = pA->AttachedFiles.GetHeadPosition();
            while (pos) {
                AttachedFile* pF = pA->AttachedFiles.GetNext(pos);

                CAtlArray<BYTE> pData;
                pData.SetCount((size_t)pF->FileDataLen);
                m_pFile->Seek(pF->FileDataPos);
                if (SUCCEEDED(m_pFile->ByteRead(pData.GetData(), pData.GetCount()))) {
                    ResAppend(pF->FileName, pF->FileDescription, CStringW(pF->FileMimeType), pData.GetData(), (DWORD)pData.GetCount());
                }
            }
        }
    }

    // chapters

    if (ChapterAtom* caroot = m_pFile->m_segment.FindChapterAtom(0)) {
        CStringA str;
        str.ReleaseBufferSetLength(GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, str.GetBuffer(3), 3));
        CStringA ChapLanguage = CStringA(ISO6391To6392(str));
        if (ChapLanguage.GetLength() < 3) {
            ChapLanguage = "eng";
        }

        SetupChapters(ChapLanguage, caroot);
    }

    return m_pOutputs.GetCount() > 0 ? S_OK : E_FAIL;
}

void CMatroskaSplitterFilter::SetupChapters(LPCSTR lng, ChapterAtom* parent, int level)
{
    CStringW tabs('+', level);

    if (!tabs.IsEmpty()) {
        tabs += ' ';
    }

    POSITION pos = parent->ChapterAtoms.GetHeadPosition();
    while (pos) {
        // ChapUID zero not allow by Matroska specs
        UINT64 ChapUID  = parent->ChapterAtoms.GetNext(pos)->ChapterUID;
        ChapterAtom* ca = (ChapUID == 0) ? NULL : m_pFile->m_segment.FindChapterAtom(ChapUID);

        if (ca) {
            CStringW name, first;

            POSITION pos = ca->ChapterDisplays.GetHeadPosition();
            while (pos) {
                ChapterDisplay* cd = ca->ChapterDisplays.GetNext(pos);
                if (first.IsEmpty()) {
                    first = cd->ChapString;
                }
                if (cd->ChapLanguage == lng) {
                    name = cd->ChapString;
                }
            }

            name = tabs + (!name.IsEmpty() ? name : first);

            ChapAppend(ca->ChapterTimeStart / 100 - m_pFile->m_rtOffset, name);

            if (!ca->ChapterAtoms.IsEmpty() && level < 5) {
                // level < 5 - hard limit for the number of levels
                SetupChapters(lng, ca, level + 1);
            }
        }
    }

}

void CMatroskaSplitterFilter::InstallFonts()
{
    POSITION pos = m_pFile->m_segment.Attachments.GetHeadPosition();
    while (pos) {
        Attachment* pA = m_pFile->m_segment.Attachments.GetNext(pos);

        POSITION p2 = pA->AttachedFiles.GetHeadPosition();
        while (p2) {
            AttachedFile* pF = pA->AttachedFiles.GetNext(p2);

            if (pF->FileMimeType == "application/x-truetype-font" ||
                    pF->FileMimeType == "application/x-font-ttf" ||
                    pF->FileMimeType == "application/vnd.ms-opentype") {
                // assume this is a font resource

                if (BYTE* pData = DNew BYTE[(UINT)pF->FileDataLen]) {
                    m_pFile->Seek(pF->FileDataPos);

                    if (SUCCEEDED(m_pFile->ByteRead(pData, pF->FileDataLen))) {
                        //m_fontinst.InstallFont(pData, (UINT)pF->FileDataLen);
                        m_fontinst.InstallFontMemory(pData, (UINT)pF->FileDataLen);
                    }

                    delete [] pData;
                }
            }
        }
    }
}

void CMatroskaSplitterFilter::SendVorbisHeaderSample()
{
    HRESULT hr;

    POSITION pos = m_pTrackEntryMap.GetStartPosition();
    while (pos) {
        DWORD TrackNumber = 0;
        TrackEntry* pTE = NULL;
        m_pTrackEntryMap.GetNextAssoc(pos, TrackNumber, pTE);

        CBaseSplitterOutputPin* pPin = GetOutputPin(TrackNumber);

        if (!(pTE && pPin && pPin->IsConnected())) {
            continue;
        }

        if (pTE->CodecID.ToString() == "A_VORBIS" && pPin->CurrentMediaType().subtype == MEDIASUBTYPE_Vorbis
                && pTE->CodecPrivate.GetCount() > 0) {
            BYTE* ptr = pTE->CodecPrivate.GetData();

            CAtlList<int> sizes;
            long last = 0;
            for (BYTE n = *ptr++; n > 0; n--) {
                int size = 0;
                do {
                    size += *ptr;
                } while (*ptr++ == 0xff);
                sizes.AddTail(size);
                last += size;
            }
            sizes.AddTail(pTE->CodecPrivate.GetCount() - (ptr - pTE->CodecPrivate.GetData()) - last);

            hr = S_OK;

            POSITION pos = sizes.GetHeadPosition();
            while (pos && SUCCEEDED(hr)) {
                long len = sizes.GetNext(pos);

                CAutoPtr<Packet> p(DNew Packet());
                p->TrackNumber = (DWORD)pTE->TrackNumber;
                p->rtStart = 0;
                p->rtStop = 1;
                p->bSyncPoint = FALSE;

                p->SetData(ptr, len);
                ptr += len;

                hr = DeliverPacket(p);
            }

            if (FAILED(hr)) {
                TRACE(_T("ERROR: Vorbis initialization failed for stream %I64d\n"), TrackNumber);
            }
        }
    }
}

bool CMatroskaSplitterFilter::DemuxInit()
{
    SetThreadName((DWORD) - 1, "CMatroskaSplitterFilter");

    CMatroskaNode Root(m_pFile);
    if (!m_pFile
            || !(m_pSegment = Root.Child(0x18538067))
            || !(m_pCluster = m_pSegment->Child(0x1F43B675))) {
        return false;
    }

    // reindex if needed

    if (m_pFile->IsRandomAccess() && m_pFile->m_segment.Cues.GetCount() == 0) {
        m_nOpenProgress = 0;
        m_pFile->m_segment.SegmentInfo.Duration.Set(0);

        UINT64 TrackNumber = m_pFile->m_segment.GetMasterTrack();

        CAutoPtr<Cue> pCue(DNew Cue());

        do {
            Cluster c;
            c.ParseTimeCode(m_pCluster);

            m_pFile->m_segment.SegmentInfo.Duration.Set((float)c.TimeCode - m_pFile->m_rtOffset / 10000);

            CAutoPtr<CuePoint> pCuePoint(DNew CuePoint());
            CAutoPtr<CueTrackPosition> pCueTrackPosition(DNew CueTrackPosition());
            pCuePoint->CueTime.Set(c.TimeCode);
            pCueTrackPosition->CueTrack.Set(TrackNumber);
            pCueTrackPosition->CueClusterPosition.Set(m_pCluster->m_filepos - m_pSegment->m_start);
            pCuePoint->CueTrackPositions.AddTail(pCueTrackPosition);
            pCue->CuePoints.AddTail(pCuePoint);

            m_nOpenProgress = m_pFile->GetPos() * 100 / m_pFile->GetLength();

            DWORD cmd;
            if (CheckRequest(&cmd)) {
                if (cmd == CMD_EXIT) {
                    m_fAbort = true;
                } else {
                    Reply(S_OK);
                }
            }
        } while (!m_fAbort && m_pCluster->Next(true));

        m_nOpenProgress = 100;

        if (!m_fAbort) {
            m_pFile->m_segment.Cues.AddTail(pCue);
        }

        m_fAbort = false;
    }

    m_pCluster.Free();
    m_pBlock.Free();

    return true;
}

void CMatroskaSplitterFilter::DemuxSeek(REFERENCE_TIME rt)
{
    m_pCluster = m_pSegment->Child(0x1F43B675);
    m_pBlock.Free();

    if (rt > 0) {
        rt += m_pFile->m_rtOffset;

        MatroskaReader::QWORD lastCueClusterPosition = (MatroskaReader::QWORD) - 1;

        Segment& s = m_pFile->m_segment;

        UINT64 TrackNumber = s.GetMasterTrack();

        POSITION pos1 = s.Cues.GetHeadPosition();
        while (pos1) {
            Cue* pCue = s.Cues.GetNext(pos1);

            POSITION pos2 = pCue->CuePoints.GetTailPosition();
            while (pos2) {
                CuePoint* pCuePoint = pCue->CuePoints.GetPrev(pos2);

                if (rt < s.GetRefTime(pCuePoint->CueTime)) {
                    continue;
                }

                POSITION pos3 = pCuePoint->CueTrackPositions.GetHeadPosition();
                while (pos3) {
                    CueTrackPosition* pCueTrackPositions = pCuePoint->CueTrackPositions.GetNext(pos3);

                    if (TrackNumber != pCueTrackPositions->CueTrack) {
                        continue;
                    }

                    if (lastCueClusterPosition == pCueTrackPositions->CueClusterPosition) {
                        continue;
                    }

                    lastCueClusterPosition = pCueTrackPositions->CueClusterPosition;

                    m_pCluster->SeekTo(m_pSegment->m_start + pCueTrackPositions->CueClusterPosition);
                    m_pCluster->Parse();

                    bool fFoundKeyFrame = false;
                    /*
                                        if (pCueTrackPositions->CueBlockNumber > 0)
                                        {
                                            // TODO: CueBlockNumber only tells the block num of the track and not for all mixed in the cluster
                                            m_nLastBlock = (int)pCueTrackPositions->CueBlockNumber;
                                            fFoundKeyFrame = true;
                                        }
                                        else
                    */
                    {
                        Cluster c;
                        c.ParseTimeCode(m_pCluster);

                        if (CAutoPtr<CMatroskaNode> pBlock = m_pCluster->GetFirstBlock()) {
                            bool fPassedCueTime = false;

                            do {
                                CBlockGroupNode bgn;

                                if (pBlock->m_id == 0xA0) {
                                    bgn.Parse(pBlock, true);
                                } else if (pBlock->m_id == 0xA3) {
                                    CAutoPtr<BlockGroup> bg(DNew BlockGroup());
                                    bg->Block.Parse(pBlock, true);
                                    if (!(bg->Block.Lacing & 0x80)) {
                                        bg->ReferenceBlock.Set(0);    // not a kf
                                    }
                                    bgn.AddTail(bg);
                                }

                                POSITION pos4 = bgn.GetHeadPosition();
                                while (!fPassedCueTime && pos4) {
                                    BlockGroup* bg = bgn.GetNext(pos4);

                                    if (bg->Block.TrackNumber == pCueTrackPositions->CueTrack && rt < s.GetRefTime(c.TimeCode + bg->Block.TimeCode)
                                            || rt + 5000000i64 < s.GetRefTime(c.TimeCode + bg->Block.TimeCode)) { // allow 500ms difference between tracks, just in case intreleaving wasn't that much precise
                                        fPassedCueTime = true;
                                    } else if (bg->Block.TrackNumber == pCueTrackPositions->CueTrack && !bg->ReferenceBlock.IsValid()) {
                                        fFoundKeyFrame = true;
                                        m_pBlock = pBlock->Copy();
                                    }
                                }
                            } while (!fPassedCueTime && pBlock->NextBlock());
                        }
                    }

                    if (fFoundKeyFrame) {
                        pos1 = pos2 = pos3 = NULL;
                    }
                }
            }
        }

        if (!m_pBlock) {
            m_pCluster = m_pSegment->Child(0x1F43B675);
        }
    }
}

bool CMatroskaSplitterFilter::DemuxLoop()
{
    HRESULT hr = S_OK;

    SendVorbisHeaderSample(); // HACK: init vorbis decoder with the headers

    do {
        Cluster c;
        c.ParseTimeCode(m_pCluster);

        if (!m_pBlock) {
            m_pBlock = m_pCluster->GetFirstBlock();
        }
        if (!m_pBlock) {
            continue;
        }

        do {
            CBlockGroupNode bgn;

            if (m_pBlock->m_id == 0xA0) {
                bgn.Parse(m_pBlock, true);
            } else if (m_pBlock->m_id == 0xA3) {
                CAutoPtr<BlockGroup> bg(DNew BlockGroup());
                bg->Block.Parse(m_pBlock, true);
                if (!(bg->Block.Lacing & 0x80)) {
                    bg->ReferenceBlock.Set(0);    // not a kf
                }
                bgn.AddTail(bg);
            }

            while (bgn.GetCount() && SUCCEEDED(hr)) {
                CAutoPtr<MatroskaPacket> p(DNew MatroskaPacket());
                p->bg = bgn.RemoveHead();

                p->bSyncPoint = !p->bg->ReferenceBlock.IsValid();
                p->TrackNumber = (DWORD)p->bg->Block.TrackNumber;

                TrackEntry* pTE = NULL;

                if (!m_pTrackEntryMap.Lookup(p->TrackNumber, pTE) || !pTE) {
                    continue;
                }

                p->rtStart = m_pFile->m_segment.GetRefTime((REFERENCE_TIME)c.TimeCode + p->bg->Block.TimeCode);
                p->rtStop = p->rtStart + (p->bg->BlockDuration.IsValid() ? m_pFile->m_segment.GetRefTime(p->bg->BlockDuration) : 1);

                // Fix subtitle with duration = 0
                if (pTE->TrackType == TrackEntry::TypeSubtitle && !p->bg->BlockDuration.IsValid()) {
                    p->bg->BlockDuration.Set(1); // just setting it to be valid
                    p->rtStop = p->rtStart;
                }

                POSITION pos = p->bg->Block.BlockData.GetHeadPosition();
                while (pos) {
                    CBinary* pb = p->bg->Block.BlockData.GetNext(pos);
                    pTE->Expand(*pb, ContentEncoding::AllFrameContents);
                }

                // HACK
                p->rtStart -= m_pFile->m_rtOffset;
                p->rtStop -= m_pFile->m_rtOffset;

                hr = DeliverPacket(p);
            }
        } while (m_pBlock->NextBlock() && SUCCEEDED(hr) && !CheckRequest(NULL));

        m_pBlock.Free();
    } while (m_pFile->GetPos() < (__int64)(m_pFile->m_segment.pos + m_pFile->m_segment.len)
             && m_pCluster->Next(true) && SUCCEEDED(hr) && !CheckRequest(NULL));

    m_pCluster.Free();

    return true;
}

// IKeyFrameInfo

STDMETHODIMP CMatroskaSplitterFilter::GetKeyFrameCount(UINT& nKFs)
{
    if (!m_pFile) {
        return E_UNEXPECTED;
    }

    HRESULT hr = S_OK;

    nKFs = 0;

    POSITION pos = m_pFile->m_segment.Cues.GetHeadPosition();
    while (pos) {
        nKFs += (UINT)m_pFile->m_segment.Cues.GetNext(pos)->CuePoints.GetCount();
    }

    return hr;
}

STDMETHODIMP CMatroskaSplitterFilter::GetKeyFrames(const GUID* pFormat, REFERENCE_TIME* pKFs, UINT& nKFs)
{
    CheckPointer(pFormat, E_POINTER);
    CheckPointer(pKFs, E_POINTER);

    if (!m_pFile) {
        return E_UNEXPECTED;
    }
    if (*pFormat != TIME_FORMAT_MEDIA_TIME) {
        return E_INVALIDARG;
    }

    UINT nKFsTmp = 0;

    POSITION pos1 = m_pFile->m_segment.Cues.GetHeadPosition();
    while (pos1 && nKFsTmp < nKFs) {
        Cue* pCue = m_pFile->m_segment.Cues.GetNext(pos1);

        POSITION pos2 = pCue->CuePoints.GetHeadPosition();
        while (pos2 && nKFsTmp < nKFs) {
            pKFs[nKFsTmp++] = m_pFile->m_segment.GetRefTime(pCue->CuePoints.GetNext(pos2)->CueTime);
        }
    }

    nKFs = nKFsTmp;

    return S_OK;
}

//
// CMatroskaSourceFilter
//

CMatroskaSourceFilter::CMatroskaSourceFilter(LPUNKNOWN pUnk, HRESULT* phr)
    : CMatroskaSplitterFilter(pUnk, phr)
{
    m_clsid = __uuidof(this);
    m_pInput.Free();
}

//
// CMatroskaSplitterOutputPin
//

CMatroskaSplitterOutputPin::CMatroskaSplitterOutputPin(
    unsigned int nMinCache, REFERENCE_TIME rtDefaultDuration,
    CAtlArray<CMediaType>& mts, LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr)
    : CBaseSplitterOutputPin(mts, pName, pFilter, pLock, phr)
    , m_nMinCache(nMinCache), m_rtDefaultDuration(rtDefaultDuration)
{
    m_nMinCache = 1;//max(m_nMinCache, 1);
}

CMatroskaSplitterOutputPin::~CMatroskaSplitterOutputPin()
{
}

HRESULT CMatroskaSplitterOutputPin::DeliverEndFlush()
{
    {
        CAutoLock cAutoLock(&m_csQueue);
        m_packets.RemoveAll();
        m_rob.RemoveAll();
        m_tos.RemoveAll();
    }

    return __super::DeliverEndFlush();
}

HRESULT CMatroskaSplitterOutputPin::DeliverEndOfStream()
{
    CAutoLock cAutoLock(&m_csQueue);

    // send out the last remaining packets from the queue

    while (m_rob.GetCount()) {
        MatroskaPacket* mp = m_rob.RemoveHead();
        if (m_rob.GetCount() && !mp->bg->BlockDuration.IsValid()) {
            mp->rtStop = m_rob.GetHead()->rtStart;
        } else if (m_rob.GetCount() == 0 && m_rtDefaultDuration > 0) {
            mp->rtStop = mp->rtStart + m_rtDefaultDuration;
        }

        timeoverride to = {mp->rtStart, mp->rtStop};
        m_tos.AddTail(to);
    }

    while (m_packets.GetCount()) {
        HRESULT hr = DeliverBlock(m_packets.RemoveHead());
        if (hr != S_OK) {
            return hr;
        }
    }

    return __super::DeliverEndOfStream();
}

HRESULT CMatroskaSplitterOutputPin::DeliverPacket(CAutoPtr<Packet> p)
{
    MatroskaPacket* mp = dynamic_cast<MatroskaPacket*>(p.m_p);
    if (!mp) {
        return __super::DeliverPacket(p);
    }

    // don't try to understand what's happening here, it's magic

    CAutoLock cAutoLock(&m_csQueue);

    CAutoPtr<MatroskaPacket> p2;
    p.Detach();
    p2.Attach(mp);
    m_packets.AddTail(p2);

    POSITION pos = m_rob.GetTailPosition();
    _ASSERTE(m_nMinCache > 0);
    for (int i = m_nMinCache - 1; i > 0 && pos && mp->bg->ReferencePriority < m_rob.GetAt(pos)->bg->ReferencePriority; --i) {
        m_rob.GetPrev(pos);
    }

    if (!pos) {
        m_rob.AddHead(mp);
    } else {
        m_rob.InsertAfter(pos, mp);
    }

    mp = NULL;

    if (m_rob.GetCount() == m_nMinCache + 1) {
        ASSERT(m_nMinCache > 0);
        pos = m_rob.GetHeadPosition();
        MatroskaPacket* mp1 = m_rob.GetNext(pos);
        MatroskaPacket* mp2 = m_rob.GetNext(pos);
        if (!mp1->bg->BlockDuration.IsValid()) {
            mp1->bg->BlockDuration.Set(1); // just to set it valid

            if (mp1->rtStart >= mp2->rtStart) {
                /*              CString str;
                                str.Format(_T("mp1->rtStart (%I64d) >= mp2->rtStart (%I64d)!!!\n"), mp1->rtStart, mp2->rtStart);
                                AfxMessageBox(str);
                */
                // TRACE(_T("mp1->rtStart (%I64d) >= mp2->rtStart (%I64d)!!!\n"), mp1->rtStart, mp2->rtStart);
            } else {
                mp1->rtStop = mp2->rtStart;
            }
        }
    }

    while (m_packets.GetCount()) {
        mp = m_packets.GetHead();
        if (!mp->bg->BlockDuration.IsValid()) {
            break;
        }

        mp = m_rob.RemoveHead();
        timeoverride to = {mp->rtStart, mp->rtStop};
        m_tos.AddTail(to);

        HRESULT hr = DeliverBlock(m_packets.RemoveHead());
        if (hr != S_OK) {
            return hr;
        }
    }

    return S_OK;
}

HRESULT CMatroskaSplitterOutputPin::DeliverBlock(MatroskaPacket* p)
{
    HRESULT hr = S_FALSE;

    if (m_tos.GetCount()) {
        timeoverride to = m_tos.RemoveHead();

#if defined(_DEBUG) && 0
        TRACE(_T("(track=%d) %I64d, %I64d -> %I64d, %I64d (buffcnt=%d)\n"),
              p->TrackNumber, p->rtStart, p->rtStop, to.rtStart, to.rtStop,
              QueueCount());
#endif

        p->rtStart = to.rtStart;
        p->rtStop = to.rtStop;
    }

    REFERENCE_TIME
    rtStart = p->rtStart,
    rtDelta = (p->rtStop - p->rtStart) / p->bg->Block.BlockData.GetCount(),
    rtStop = p->rtStart + rtDelta;

    POSITION pos = p->bg->Block.BlockData.GetHeadPosition();
    while (pos) {
        CAutoPtr<Packet> tmp(DNew Packet());
        tmp->TrackNumber = p->TrackNumber;
        tmp->bDiscontinuity = p->bDiscontinuity;
        tmp->bSyncPoint = p->bSyncPoint;
        tmp->rtStart = rtStart;
        tmp->rtStop = rtStop;
        tmp->Copy(*p->bg->Block.BlockData.GetNext(pos));
        if (S_OK != (hr = DeliverPacket(tmp))) {
            break;
        }

        rtStart += rtDelta;
        rtStop += rtDelta;

        p->bSyncPoint = false;
        p->bDiscontinuity = false;
    }

    if (m_mt.subtype == FOURCCMap(WAVE_FORMAT_WAVPACK4)) {
        POSITION pos = p->bg->ba.bm.GetHeadPosition();
        while (pos) {
            const BlockMore* bm = p->bg->ba.bm.GetNext(pos);
            CAutoPtr<Packet> tmp(DNew Packet());
            tmp->TrackNumber = p->TrackNumber;
            tmp->bDiscontinuity = false;
            tmp->bSyncPoint = false;
            tmp->rtStart = p->rtStart;
            tmp->rtStop = p->rtStop;
            tmp->Copy(bm->BlockAdditional);
            if (S_OK != (hr = DeliverPacket(tmp))) {
                break;
            }
        }
    }

    return hr;
}

// ITrackInfo

TrackEntry* CMatroskaSplitterFilter::GetTrackEntryAt(UINT aTrackIdx)
{
    if (aTrackIdx >= m_pOrderedTrackArray.GetCount()) {
        return NULL;
    }
    return m_pOrderedTrackArray[aTrackIdx];
}

STDMETHODIMP_(UINT) CMatroskaSplitterFilter::GetTrackCount()
{
    return (UINT)m_pTrackEntryMap.GetCount();
}

STDMETHODIMP_(BOOL) CMatroskaSplitterFilter::GetTrackInfo(UINT aTrackIdx, struct TrackElement* pStructureToFill)
{
    TrackEntry* pTE = GetTrackEntryAt(aTrackIdx);
    if (pTE == NULL) {
        return FALSE;
    }

    pStructureToFill->FlagDefault = !!pTE->FlagDefault;
    pStructureToFill->FlagForced = !!pTE->FlagForced;
    pStructureToFill->FlagLacing = !!pTE->FlagLacing;
    strncpy_s(pStructureToFill->Language, pTE->Language, 3);
    if (pStructureToFill->Language[0] == '\0') {
        strncpy_s(pStructureToFill->Language, "eng", 3);
    }
    pStructureToFill->Language[3] = '\0';
    pStructureToFill->MaxCache = (UINT)pTE->MaxCache;
    pStructureToFill->MinCache = (UINT)pTE->MinCache;
    pStructureToFill->Type = (BYTE)pTE->TrackType;
    return TRUE;
}

STDMETHODIMP_(BOOL) CMatroskaSplitterFilter::GetTrackExtendedInfo(UINT aTrackIdx, void* pStructureToFill)
{
    TrackEntry* pTE = GetTrackEntryAt(aTrackIdx);
    if (pTE == NULL) {
        return FALSE;
    }

    if (pTE->TrackType == TrackEntry::TypeVideo) {
        TrackExtendedInfoVideo* pTEIV = (TrackExtendedInfoVideo*)pStructureToFill;
        pTEIV->AspectRatioType = (BYTE)pTE->v.AspectRatioType;
        pTEIV->DisplayUnit = (BYTE)pTE->v.DisplayUnit;
        pTEIV->DisplayWidth = (UINT)pTE->v.DisplayWidth;
        pTEIV->DisplayHeight = (UINT)pTE->v.DisplayHeight;
        pTEIV->Interlaced = !!pTE->v.FlagInterlaced;
        pTEIV->PixelWidth = (UINT)pTE->v.PixelWidth;
        pTEIV->PixelHeight = (UINT)pTE->v.PixelHeight;
    } else if (pTE->TrackType == TrackEntry::TypeAudio) {
        TrackExtendedInfoAudio* pTEIA = (TrackExtendedInfoAudio*)pStructureToFill;
        pTEIA->BitDepth = (UINT)pTE->a.BitDepth;
        pTEIA->Channels = (UINT)pTE->a.Channels;
        pTEIA->OutputSamplingFrequency = (FLOAT)pTE->a.OutputSamplingFrequency;
        pTEIA->SamplingFreq = (FLOAT)pTE->a.SamplingFrequency;
    } else {
        return FALSE;
    }

    return TRUE;
}

STDMETHODIMP_(BSTR) CMatroskaSplitterFilter::GetTrackName(UINT aTrackIdx)
{
    TrackEntry* pTE = GetTrackEntryAt(aTrackIdx);
    if (pTE == NULL) {
        return NULL;
    }
    return pTE->Name.AllocSysString();
}

STDMETHODIMP_(BSTR) CMatroskaSplitterFilter::GetTrackCodecID(UINT aTrackIdx)
{
    TrackEntry* pTE = GetTrackEntryAt(aTrackIdx);
    if (pTE == NULL) {
        return NULL;
    }
    return pTE->CodecID.ToString().AllocSysString();
}

STDMETHODIMP_(BSTR) CMatroskaSplitterFilter::GetTrackCodecName(UINT aTrackIdx)
{
    TrackEntry* pTE = GetTrackEntryAt(aTrackIdx);
    if (pTE == NULL) {
        return NULL;
    }
    return pTE->CodecName.AllocSysString();
}

STDMETHODIMP_(BSTR) CMatroskaSplitterFilter::GetTrackCodecInfoURL(UINT aTrackIdx)
{
    TrackEntry* pTE = GetTrackEntryAt(aTrackIdx);
    if (pTE == NULL) {
        return NULL;
    }
    return pTE->CodecInfoURL.AllocSysString();
}

STDMETHODIMP_(BSTR) CMatroskaSplitterFilter::GetTrackCodecDownloadURL(UINT aTrackIdx)
{
    TrackEntry* pTE = GetTrackEntryAt(aTrackIdx);
    if (pTE == NULL) {
        return NULL;
    }
    return pTE->CodecDownloadURL.AllocSysString();
}
