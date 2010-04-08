/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2010 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include <mmreg.h>
#include "mplayerc.h"
#include "../../filters/filters.h"
#include <moreuuids.h>
#include "FGManager.h"
#include "ConvertPropsDlg.h"
#include "ConvertResDlg.h"
#include "ConvertChapDlg.h"
#include "ConvertDlg.h"


// TODO: subtitle source filter for vobsub

// CConvertDlg dialog

CConvertDlg::CConvertDlg(CWnd* pParent /*=NULL*/)
    : CResizableDialog(CConvertDlg::IDD, pParent)
    , m_fn(_T(""))
{
}

CConvertDlg::~CConvertDlg()
{
}

void CConvertDlg::AddFile(CString fn)
{
    CString protocol;

    int i = fn.Find(_T("://"));
    if(i > 0)
    {
        CString url = fn.Mid(i);
        CPath path(fn.Left(i));
        path.StripPath();
        protocol = (LPCTSTR)path;
        fn = (LPCTSTR)path + url;
    }

    CComPtr<IBaseFilter> pBF;
    if(FAILED(m_pGB->AddSourceFilter(CStringW(fn), CStringW(fn), &pBF)))
        return;

    int nConnected = 0;
    BeginEnumPins(pBF, pEP, pPin)
    if(S_OK == m_pGB->ConnectFilter(pPin, m_pMux)) nConnected++;
    EndEnumPins
    if(!nConnected)
    {
        MessageBeep(-1);
        DeleteFilter(pBF);
        return;
    }

    if(m_tree.GetCount() == 0)
    {
        if(CComQIPtr<IDSMPropertyBag> pPB = m_pMux)
            pPB->DelAllProperties();

        CString ext(_T(".dsm"));

        if(!protocol.IsEmpty())
        {
            m_fn = protocol + ext;
        }
        else
        {
            CPath p(fn);
            if(ext.CompareNoCase(p.GetExtension()) == 0)
                ext = _T(" (remuxed)") + ext;
            p.RemoveExtension();
            m_fn = (LPCTSTR)p + ext;
        }

        UpdateData(FALSE);
    }

    CTreeItemFile* t = DNew CTreeItemFile(fn, pBF, m_tree, NULL);

    AddFilter(*t, pBF);

    m_tree.Expand(*t, TVE_EXPAND);
    m_tree.EnsureVisible(*t);
}

bool CConvertDlg::ConvertFile(LPCTSTR fn, IPin* pPin)
{
    OAFilterState fs;
    if(!m_pMC || FAILED(m_pMC->GetState(0, &fs)) || fs != State_Stopped)
        return false;

    m_pGB->NukeDownstream(m_pMux);

    CComPtr<IBaseFilter> pFW;
    pFW.CoCreateInstance(CLSID_FileWriter);
    CComQIPtr<IFileSinkFilter2> pFSF = pFW;

    if(pPin)
    {
        CComQIPtr<IBaseMuxerRelatedPin> pRP = pPin;
        if(!pRP) return false;

        pPin = pRP->GetRelatedPin();
    }
    else
    {
        pPin = GetFirstPin(m_pMux, PINDIR_OUTPUT);
    }

    if(!pPin || !pFSF
       || FAILED(m_pGB->AddFilter(pFW, NULL))
       || FAILED(pFSF->SetFileName(CStringW(fn), NULL))
       || FAILED(pFSF->SetMode(AM_FILE_OVERWRITE))
       || FAILED(m_pGB->ConnectDirect(pPin, GetFirstPin(pFW), NULL)))
    {
        m_pGB->RemoveFilter(pFW);
        return false;
    }

    if(m_pMS)
    {
        LONGLONG pos = 0;
        m_pMS->SetPositions(&pos, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning);
    }

    if(CComQIPtr<IDSMPropertyBag> pPB = m_pMux)
    {
        pPB->SetProperty(L"APPL", L"Media Player Classic");
    }

    if(CComQIPtr<IDSMResourceBag> pRB = m_pMux)
    {
        pRB->ResRemoveAll(0);
        POSITION pos = m_pTIs.GetHeadPosition();
        while(pos)
        {
            if(CTreeItemResource* t2 = dynamic_cast<CTreeItemResource*>((CTreeItem*)m_pTIs.GetNext(pos)))
                pRB->ResAppend(
                    t2->m_res.name, t2->m_res.desc, t2->m_res.mime,
                    t2->m_res.data.GetData(), t2->m_res.data.GetCount(),
                    NULL);
        }
    }

    if(CComQIPtr<IDSMChapterBag> pCB = m_pMux)
    {
        pCB->ChapRemoveAll();
        POSITION pos = m_pTIs.GetHeadPosition();
        while(pos)
        {
            if(CTreeItemChapter* t2 = dynamic_cast<CTreeItemChapter*>((CTreeItem*)m_pTIs.GetNext(pos)))
                pCB->ChapAppend(t2->m_chap.rt, t2->m_chap.name);
        }
    }

    if(FAILED(m_pMC->Run()))
        return false;

    m_tree.EnableWindow(FALSE);

    return true;
}

void CConvertDlg::AddFilter(HTREEITEM hTIParent, IBaseFilter* pBFParent)
{
    BeginEnumPins(pBFParent, pEP, pPin)
    {
        CComPtr<IPin> pPinTo;
        CComPtr<IBaseFilter> pBF;
        if(S_OK != m_pGB->IsPinDirection(pPin, PINDIR_OUTPUT)
           || FAILED(pPin->ConnectedTo(&pPinTo)) || !pPinTo
           || !(pBF = GetFilterFromPin(pPinTo)))
            continue;

        CTreeItem* t = NULL;

        if(pBF == m_pMux)
        {
            t = DNew CTreeItemPin(pPin, m_tree, hTIParent);
        }
        else
        {
            t = DNew CTreeItemFilter(pBF, m_tree, hTIParent);
            AddFilter(*t, pBF);
        }
    }
    EndEnumPins

    if(CComQIPtr<IDSMPropertyBag> pPB = pBFParent)
    {
        ULONG props;
        if(FAILED(pPB->CountProperties(&props)))
            props = 0;

        for(ULONG i = 0; i < props; i++)
        {
            PROPBAG2 PropBag;
            memset(&PropBag, 0, sizeof(PropBag));
            ULONG cPropertiesReturned = 0;
            if(FAILED(pPB->GetPropertyInfo(i, 1, &PropBag, &cPropertiesReturned)))
                continue;

            HRESULT hr;
            CComVariant var;
            if(SUCCEEDED(pPB->Read(1, &PropBag, NULL, &var, &hr)) && SUCCEEDED(hr))
            {
                CComQIPtr<IDSMPropertyBag> pPBMux = m_pMux;
                CComBSTR value;
                if(pPBMux && FAILED(pPBMux->GetProperty(PropBag.pstrName, &value)))
                    pPBMux->SetProperty(PropBag.pstrName, var.bstrVal);
            }

            CoTaskMemFree(PropBag.pstrName);
        }
    }

    CTreeItem* t2 = DNew CTreeItemResourceFolder(m_tree, hTIParent);
    if(CComQIPtr<IDSMResourceBag> pRB = pBFParent)
    {
        for(DWORD i = 0, cnt = pRB->ResGetCount(); i < cnt; i++)
        {
            CComBSTR name, mime, desc;
            BYTE* pData = NULL;
            DWORD len = 0;
            if(FAILED(pRB->ResGet(i, &name, &desc, &mime, &pData, &len, NULL)))
                continue;

            if(len > 0)
            {
                m_pTIs.AddTail(DNew CTreeItemResource(CDSMResource(name, desc, mime, pData, len), m_tree, *t2));
            }

            CoTaskMemFree(pData);
        }
    }
    m_tree.Expand(*t2, TVE_EXPAND);

    CTreeItem* t3 = DNew CTreeItemChapterFolder(m_tree, hTIParent);
    if(CComQIPtr<IDSMChapterBag> pCB = pBFParent)
    {
        for(DWORD i = 0, cnt = pCB->ChapGetCount(); i < cnt; i++)
        {
            REFERENCE_TIME rt;
            CComBSTR name;
            if(FAILED(pCB->ChapGet(i, &rt, &name)))
                continue;

            m_pTIs.AddTail(DNew CTreeItemChapter(CDSMChapter(rt, name), m_tree, *t3));
        }
    }
    m_tree.Expand(*t3, TVE_EXPAND);

    m_tree.Expand(hTIParent, TVE_EXPAND);
}

void CConvertDlg::DeleteFilter(IBaseFilter* pBF)
{
    BeginEnumPins(pBF, pEP, pPin)
    {
        CComPtr<IPin> pPinTo;
        CComPtr<IBaseFilter> pBF;
        if(S_OK != m_pGB->IsPinDirection(pPin, PINDIR_OUTPUT)
           || FAILED(pPin->ConnectedTo(&pPinTo)) || !pPinTo
           || !(pBF = GetFilterFromPin(pPinTo)))
            continue;

        if(pBF != m_pMux) DeleteFilter(pBF);
    }
    EndEnumPins

    m_pGB->RemoveFilter(pBF);
}

void CConvertDlg::DeleteItem(HTREEITEM hTI)
{
    if(!hTI) return;

    DeleteChildren(hTI);

    CTreeItem* t = (CTreeItem*)m_tree.GetItemData(hTI);
    if(POSITION pos = m_pTIs.Find(t)) m_pTIs.RemoveAt(pos);
    delete t;
    m_tree.DeleteItem(hTI);
}

void CConvertDlg::DeleteChildren(HTREEITEM hTI)
{
    if(!hTI) return;

    if(m_tree.ItemHasChildren(hTI))
    {
        HTREEITEM hChildItem = m_tree.GetChildItem(hTI);

        while(hChildItem != NULL)
        {
            HTREEITEM hNextItem = m_tree.GetNextItem(hChildItem, TVGN_NEXT);
            DeleteItem(hChildItem);
            hChildItem = hNextItem;
        }
    }
}

HTREEITEM CConvertDlg::HitTest(CPoint& sp, CPoint& cp)
{
    sp = CPoint((LPARAM)GetMessagePos());
    cp = sp;
    m_tree.ScreenToClient(&cp);
    UINT flags = 0;
    HTREEITEM hTI = m_tree.HitTest(cp, &flags);
    return hTI && (flags & TVHT_ONITEM) ? hTI : NULL;
}

void CConvertDlg::ShowPopup(CPoint p)
{
    CMenu m;
    m.CreatePopupMenu();

    int i = 1;
    m.AppendMenu(MF_STRING, i++, ResStr(IDS_CONVERT_ADDFILE));
    m.AppendMenu(MF_SEPARATOR);
    m.AppendMenu(MF_STRING, i++, ResStr(IDS_CONVERT_PROPERTIES));

    switch((int)m.TrackPopupMenu(TPM_LEFTBUTTON | TPM_RETURNCMD, p.x, p.y, this))
    {
    case 1:
    {
        CFileDialog fd(TRUE, NULL, m_fn,
                       OFN_EXPLORER | OFN_ENABLESIZING | OFN_HIDEREADONLY | OFN_NOVALIDATE,
                       ResStr(IDS_CONVERTDLG_0), this, 0);
        if(fd.DoModal() == IDOK) AddFile(fd.GetPathName());
    }
    break;
    case 2:
        EditProperties(CComQIPtr<IDSMPropertyBag>(m_pMux));
        break;
    }
}

void CConvertDlg::ShowFilePopup(HTREEITEM hTI, CPoint p)
{
    CTreeItemFile* t = dynamic_cast<CTreeItemFile*>((CTreeItem*)m_tree.GetItemData(hTI));
    ASSERT(t);

    CMenu m;
    m.CreatePopupMenu();

    int i = 1;
    m.AppendMenu(MF_STRING, i++, ResStr(IDS_CONVERT_REMOVE));

    switch((int)m.TrackPopupMenu(TPM_LEFTBUTTON | TPM_RETURNCMD, p.x, p.y, this))
    {
    case 1:
        DeleteFilter(t->m_pBF);
        DeleteItem(hTI);
        break;
    }
}

void CConvertDlg::ShowPinPopup(HTREEITEM hTI, CPoint p)
{
    CTreeItemPin* t = dynamic_cast<CTreeItemPin*>((CTreeItem*)m_tree.GetItemData(hTI));
    ASSERT(t);

    if(!t->m_pPin) return;

    CComPtr<IPin> pPinTo;
    t->m_pPin->ConnectedTo(&pPinTo);

    CMediaType mt;
    if(pPinTo) t->m_pPin->ConnectionMediaType(&mt);

    CAtlArray<CMediaType> mts;
    BeginEnumMediaTypes(t->m_pPin, pEMT, pmt)
    mts.Add(*pmt);
    EndEnumMediaTypes(pmt)

    CMenu m;
    m.CreatePopupMenu();

    int i = 1, mtbase = 1000, mti = mtbase;

    m.AppendMenu(MF_STRING, i++, !pPinTo ? ResStr(IDS_CONVERT_ENABLESTREAM) : ResStr(IDS_CONVERT_DISABLESTREAM));
    m.AppendMenu(MF_STRING | (!pPinTo ? MF_GRAYED : 0), i++, ResStr(IDS_CONVERT_DEMUXSTREAM));

    if(mts.GetCount() > 1)
    {
        m.AppendMenu(MF_SEPARATOR);
        for(int i = 0; i < mts.GetCount(); i++)
            m.AppendMenu(MF_STRING | (mts[i] == mt ? MF_CHECKED : 0), mti++, CMediaTypeEx(mts[i]).ToString());
    }

    m.AppendMenu(MF_SEPARATOR);
    m.AppendMenu(MF_STRING | (!pPinTo ? MF_GRAYED : 0), i++, ResStr(IDS_CONVERT_PINPROPERTIES));

    switch(i = (int)m.TrackPopupMenu(TPM_LEFTBUTTON | TPM_RETURNCMD, p.x, p.y, this))
    {
    case 1:
        if(pPinTo)
        {
            m_pGB->Disconnect(pPinTo);
            m_pGB->Disconnect(t->m_pPin);
        }
        else if(pPinTo = GetFirstDisconnectedPin(m_pMux, PINDIR_INPUT)) m_pGB->ConnectDirect(t->m_pPin, pPinTo, NULL);
        t->Update();
        break;
    case 2:
    {
        UpdateData();

        CString ext = _T("raw");

        if(mt.subtype == MEDIASUBTYPE_AAC) ext = _T("aac");
        else if(mt.subtype == MEDIASUBTYPE_MP3) ext = _T("mp3");
        else if(mt.subtype == FOURCCMap(WAVE_FORMAT_MPEG)) ext = _T("m1a");
        else if(mt.subtype == MEDIASUBTYPE_MPEG2_AUDIO) ext = _T("m2a");
        else if(mt.subtype == MEDIASUBTYPE_WAVE_DOLBY_AC3 || mt.subtype == MEDIASUBTYPE_DOLBY_AC3) ext = _T("ac3");
        else if(mt.subtype == MEDIASUBTYPE_WAVE_DTS || mt.subtype == MEDIASUBTYPE_DTS) ext = _T("dts");
        else if((mt.subtype == FOURCCMap('1CVA') || mt.subtype == FOURCCMap('1cva')) && mt.formattype == FORMAT_MPEG2_VIDEO) ext = _T("h264");
        else if(mt.majortype == MEDIATYPE_Video && (mt.subtype == MEDIASUBTYPE_XVID || mt.subtype == MEDIASUBTYPE_xvid || mt.subtype == MEDIASUBTYPE_XVIX || mt.subtype == MEDIASUBTYPE_xvix)) ext = _T("xvid");
        else if(mt.majortype == MEDIATYPE_Video && (mt.subtype == MEDIASUBTYPE_DIVX || mt.subtype == MEDIASUBTYPE_divx || mt.subtype == MEDIASUBTYPE_DX50 || mt.subtype == MEDIASUBTYPE_dx50)) ext = _T("divx");
        else if(mt.majortype == MEDIATYPE_Video && (mt.subtype == MEDIASUBTYPE_MP4V || mt.subtype == MEDIASUBTYPE_mp4v || mt.subtype == MEDIASUBTYPE_M4S2 || mt.subtype == MEDIASUBTYPE_m4s2 || mt.subtype == MEDIASUBTYPE_MP4S || mt.subtype == MEDIASUBTYPE_mp4s)) ext = _T("mp4v");
        else if(mt.majortype == MEDIATYPE_Video && (mt.subtype == MEDIASUBTYPE_WMV1 || mt.subtype == MEDIASUBTYPE_wmv1)) ext = _T("WMV1");
        else if(mt.majortype == MEDIATYPE_Video && (mt.subtype == MEDIASUBTYPE_WMV2 || mt.subtype == MEDIASUBTYPE_wmv2)) ext = _T("WMV2");
        else if(mt.majortype == MEDIATYPE_Video && (mt.subtype == MEDIASUBTYPE_WMV3 || mt.subtype == MEDIASUBTYPE_wmv3)) ext = _T("WMV3");
        else if(mt.majortype == MEDIATYPE_Video && (mt.subtype == MEDIASUBTYPE_DIV3 || mt.subtype == MEDIASUBTYPE_div3 || mt.subtype == MEDIASUBTYPE_DIV4 || mt.subtype == MEDIASUBTYPE_div4 || mt.subtype == MEDIASUBTYPE_DIV5 || mt.subtype == MEDIASUBTYPE_div5)) ext = _T("div3");
        else if(mt.majortype == MEDIATYPE_Video && (mt.subtype == MEDIASUBTYPE_MP43 || mt.subtype == MEDIASUBTYPE_mp43 || mt.subtype == MEDIASUBTYPE_MPG3 || mt.subtype == MEDIASUBTYPE_mpg3)) ext = _T("mp43");
        else if(mt.majortype == MEDIATYPE_Video && (mt.subtype == MEDIASUBTYPE_MP42 || mt.subtype == MEDIASUBTYPE_mp42 || mt.subtype == MEDIASUBTYPE_DIV2 || mt.subtype == MEDIASUBTYPE_div2)) ext = _T("mp42");
        else if(mt.majortype == MEDIATYPE_Video && (mt.subtype == MEDIASUBTYPE_MP41 || mt.subtype == MEDIASUBTYPE_mp41 || mt.subtype == MEDIASUBTYPE_DIV1 || mt.subtype == MEDIASUBTYPE_div1)) ext = _T("mp41");
        else if(mt.majortype == MEDIATYPE_Video && mt.subtype == MEDIASUBTYPE_VP50) ext = _T("vp5");
        else if(mt.majortype == MEDIATYPE_Video && (mt.subtype == MEDIASUBTYPE_VP60 || mt.subtype == MEDIASUBTYPE_VP61 || mt.subtype == MEDIASUBTYPE_VP62 || mt.subtype == MEDIASUBTYPE_VP6A || mt.subtype == MEDIASUBTYPE_VP6F)) ext = _T("vp6");
        else if(mt.majortype == MEDIATYPE_Video && mt.subtype == MEDIASUBTYPE_FLV1) ext = _T("flv");
        else if(mt.majortype == MEDIATYPE_Video && mt.subtype == MEDIASUBTYPE_FLV4) ext = _T("flv");
        else if(mt.majortype == MEDIATYPE_Video && mt.subtype == MEDIASUBTYPE_SVQ3) ext = _T("svq3");
        else if(mt.majortype == MEDIATYPE_Video && mt.subtype == MEDIASUBTYPE_SVQ1) ext = _T("svq1");
        else if(mt.majortype == MEDIATYPE_Video && (mt.subtype == MEDIASUBTYPE_H263 || mt.subtype == MEDIASUBTYPE_h263)) ext = _T("h263");
        else if(mt.subtype == FOURCCMap('GEPJ') || mt.subtype == FOURCCMap('gepj')) ext = _T("jpg");
        else if(mt.majortype == MEDIATYPE_Video && mt.subtype == MEDIASUBTYPE_MPEG2_VIDEO) ext = _T("m2v");
        else if(mt.majortype == MEDIATYPE_Video && mt.subtype == MEDIASUBTYPE_MPEG1Payload) ext = _T("m1v");
        else if(mt.subtype == MEDIASUBTYPE_UTF8 || mt.majortype == MEDIATYPE_Text) ext = _T("srt");
        else if(mt.subtype == MEDIASUBTYPE_SSA) ext = _T("ssa");
        else if(mt.subtype == MEDIASUBTYPE_ASS || mt.subtype == MEDIASUBTYPE_ASS2) ext = _T("ass");
        else if(mt.subtype == MEDIASUBTYPE_SSF) ext = _T("ssf");
        else if(mt.subtype == MEDIASUBTYPE_VOBSUB) ext = _T("sub");
        else if(mt.subtype == MEDIASUBTYPE_PCM || mt.subtype == MEDIASUBTYPE_DVD_LPCM_AUDIO || mt.subtype == FOURCCMap(WAVE_FORMAT_EXTENSIBLE) || mt.subtype == FOURCCMap(WAVE_FORMAT_IEEE_FLOAT)) ext = _T("wav");
        // TODO: else if...

        CPath path(m_fn);
        path.RenameExtension('.' + ext);

        CFileDialog fd(FALSE, NULL, (LPCTSTR)path,
                       OFN_EXPLORER | OFN_ENABLESIZING | OFN_HIDEREADONLY,
                       ResStr(IDS_CONVERTDLG_0), this, 0);
        if(fd.DoModal() == IDOK)
        {
            if(!ConvertFile(fd.GetPathName(), pPinTo))
            {
                AfxMessageBox(ResStr(IDS_CONVERTDLG_2));
            }
        }
    }
    break;
    case 3:
        EditProperties(CComQIPtr<IDSMPropertyBag>(pPinTo));
        break;
    default:
        i -= mtbase;
        if(i >= 0 && i < mts.GetCount())
        {
            if(pPinTo)
            {
                m_pGB->Disconnect(pPinTo);
                m_pGB->Disconnect(t->m_pPin);
            }
            else
            {
                pPinTo = GetFirstDisconnectedPin(m_pMux, PINDIR_INPUT);
            }
            HRESULT hr = m_pGB->ConnectDirect(t->m_pPin, pPinTo, &mts[i]);
            if(FAILED(hr))
            {
                AfxMessageBox(ResStr(IDS_CONVERTDLG_3));
                if(mt.majortype != GUID_NULL)
                    hr = m_pGB->ConnectDirect(t->m_pPin, pPinTo, &mt);
            }
            t->Update();
        }
        break;
    }
}

void CConvertDlg::ShowResourceFolderPopup(HTREEITEM hTI, CPoint p)
{
    CTreeItemResourceFolder* t = dynamic_cast<CTreeItemResourceFolder*>((CTreeItem*)m_tree.GetItemData(hTI));
    ASSERT(t);

    CMenu m;
    m.CreatePopupMenu();

    int i = 1;
    m.AppendMenu(MF_STRING, i++, ResStr(IDS_CONVERT_ADDRESOURCE));
    if(m_tree.ItemHasChildren(*t))
    {
        m.AppendMenu(MF_SEPARATOR);
        m.AppendMenu(MF_STRING, i++, ResStr(IDS_CONVERT_REMOVEALL));
    }

    switch((int)m.TrackPopupMenu(TPM_LEFTBUTTON | TPM_RETURNCMD, p.x, p.y, this))
    {
    case 1:
    {
        CFileDialog fd(TRUE, NULL, NULL,
                       OFN_EXPLORER | OFN_ENABLESIZING | OFN_HIDEREADONLY,
                       ResStr(IDS_CONVERTDLG_4), this, 0);
        if(fd.DoModal() == IDOK)
        {
            CString fn = fd.GetPathName();
            if(FILE* f = _tfopen(fn, _T("rb")))
            {
                CDSMResource res;

                CPath path(fn);
                path.StripPath();
                res.name = (LPCTSTR)path;

                CRegKey key;
                TCHAR mime[256];
                ULONG len = countof(mime);
                if(ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, path.GetExtension().MakeLower(), KEY_READ)
                   && ERROR_SUCCESS == key.QueryStringValue(_T("Content Type"), mime, &len))
                    res.mime = mime;

                CTreeItemResource* t = DNew CTreeItemResource(res, m_tree, hTI);
                m_pTIs.AddTail(t);

                if(EditResource(t))
                {
                    fseek(f, 0, 2);
                    long size = ftell(f);
                    fseek(f, 0, 0);
                    t->m_res.data.SetCount(size);
                    for(BYTE* ptr = t->m_res.data.GetData(), * end = ptr + size;
                        size > 0 && end - ptr >= size && fread(ptr, min(size, 1024), 1, f) > 0;
                        ptr += 1024, size -= 1024);
                    fclose(f);
                }
                else
                {
                    DeleteItem(*t);
                }
            }
            else
            {
                AfxMessageBox(ResStr(IDS_CONVERTDLG_5));
            }
        }
    }
    break;
    case 2:
        DeleteChildren(hTI);
        break;
    }
}

void CConvertDlg::ShowResourcePopup(HTREEITEM hTI, CPoint p)
{
    CTreeItemResource* t = dynamic_cast<CTreeItemResource*>((CTreeItem*)m_tree.GetItemData(hTI));
    ASSERT(t);

    CMenu m;
    m.CreatePopupMenu();

    int i = 1;
    m.AppendMenu(MF_STRING, i++, ResStr(IDS_CONVERT_REMOVE));
    m.AppendMenu(MF_STRING, i++, ResStr(IDS_CONVERT_SAVEAS));
    if(AfxGetAppSettings().fEnableWebServer) m.AppendMenu(MF_STRING, 1000, ResStr(IDS_CONVERT_LAUNCHINBROWSER));
    m.AppendMenu(MF_SEPARATOR);
    m.AppendMenu(MF_STRING, i++, ResStr(IDS_CONVERT_RESOURCEPROPERTIES));

    switch((int)m.TrackPopupMenu(TPM_LEFTBUTTON | TPM_RETURNCMD, p.x, p.y, this))
    {
    case 1:
        DeleteItem(*t);
        break;
    case 2:
    {
        CFileDialog fd(FALSE, NULL, CString(t->m_res.name),
                       OFN_EXPLORER | OFN_ENABLESIZING | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                       ResStr(IDS_CONVERTDLG_4), this, 0);
        if(fd.DoModal() == IDOK)
        {
            if(FILE* f = _tfopen(fd.GetPathName(), _T("wb")))
            {
                fwrite(t->m_res.data.GetData(), 1, t->m_res.data.GetCount(), f);
                fclose(f);
            }
        }
    }
    break;
    case 3:
        EditResource(t);
        break;
    case 1000:
    {
        CString url;
        url.Format(_T("http://localhost:%d/convres.html?id=%x"), AfxGetAppSettings().nWebServerPort, (DWORD)&t->m_res);
        ShellExecute(NULL, _T("open"), url, NULL, NULL, SW_SHOWDEFAULT);
    }
    break;
    }
}

bool CConvertDlg::EditProperties(IDSMPropertyBag* pPB)
{
    CConvertPropsDlg dlg(!!CComQIPtr<IPin>(pPB), this);

    ULONG props;
    if(FAILED(pPB->CountProperties(&props)))
        props = 0;

    for(ULONG i = 0; i < props; i++)
    {
        PROPBAG2 PropBag;
        memset(&PropBag, 0, sizeof(PropBag));
        ULONG cPropertiesReturned = 0;
        if(FAILED(pPB->GetPropertyInfo(i, 1, &PropBag, &cPropertiesReturned)))
            continue;

        HRESULT hr;
        CComVariant var;
        if(SUCCEEDED(pPB->Read(1, &PropBag, NULL, &var, &hr)) && SUCCEEDED(hr))
            dlg.m_props[CString(PropBag.pstrName)] = CString(var);

        CoTaskMemFree(PropBag.pstrName);
    }

    if(IDOK != dlg.DoModal())
        return false;

    pPB->DelAllProperties();

    POSITION pos = dlg.m_props.GetStartPosition();
    while(pos)
    {
        CString key, value;
        dlg.m_props.GetNextAssoc(pos, key, value);
        pPB->SetProperty(CStringW(key), CStringW(value));
    }

    return true;
}

bool CConvertDlg::EditResource(CTreeItemResource* t)
{
    CConvertResDlg dlg(this);

    dlg.m_name = t->m_res.name;
    dlg.m_mime = t->m_res.mime;
    dlg.m_desc = t->m_res.desc;

    if(IDOK != dlg.DoModal())
        return false;

    t->m_res.name = dlg.m_name;
    t->m_res.mime = dlg.m_mime;
    t->m_res.desc = dlg.m_desc;

    t->Update();

    return true;
}

void CConvertDlg::ShowChapterFolderPopup(HTREEITEM hTI, CPoint p)
{
    CTreeItemChapterFolder* t = dynamic_cast<CTreeItemChapterFolder*>((CTreeItem*)m_tree.GetItemData(hTI));
    ASSERT(t);

    CMenu m;
    m.CreatePopupMenu();

    int i = 1;
    m.AppendMenu(MF_STRING, i++, ResStr(IDS_CONVERT_ADDCHAPTER));
    if(m_tree.ItemHasChildren(*t))
    {
        m.AppendMenu(MF_SEPARATOR);
        m.AppendMenu(MF_STRING, i++, ResStr(IDS_CONVERT_REMOVEALL));
    }

    switch((int)m.TrackPopupMenu(TPM_LEFTBUTTON | TPM_RETURNCMD, p.x, p.y, this))
    {
    case 1:
    {
        CDSMChapter chap;
        CTreeItemChapter* t = DNew CTreeItemChapter(CDSMChapter(0, L""), m_tree, hTI);
        m_pTIs.AddTail(t);
        if(!EditChapter(t))
            DeleteItem(*t);
    }
    break;
    case 2:
        DeleteChildren(hTI);
        break;
    }
}

void CConvertDlg::ShowChapterPopup(HTREEITEM hTI, CPoint p)
{
    CTreeItemChapter* t = dynamic_cast<CTreeItemChapter*>((CTreeItem*)m_tree.GetItemData(hTI));
    ASSERT(t);

    CMenu m;
    m.CreatePopupMenu();

    int i = 1;
    m.AppendMenu(MF_STRING, i++, ResStr(IDS_CONVERT_REMOVE));
    m.AppendMenu(MF_SEPARATOR);
    m.AppendMenu(MF_STRING, i++, ResStr(IDS_CONVERT_CHAPTERPROPERTIES));

    switch((int)m.TrackPopupMenu(TPM_LEFTBUTTON | TPM_RETURNCMD, p.x, p.y, this))
    {
    case 1:
        DeleteItem(hTI);
        break;
    case 2:
        EditChapter(t);
        break;
    }
}

bool CConvertDlg::EditChapter(CTreeItemChapter* t)
{
    CConvertChapDlg dlg(this);

    int h = (int)(t->m_chap.rt / 10000000 / 60 / 60);
    int m = (int)(t->m_chap.rt / 10000000 / 60 % 60);
    int s = (int)(t->m_chap.rt / 10000000 % 60);
    int ms = (int)(t->m_chap.rt / 10000 % 1000);

    dlg.m_name = t->m_chap.name;
    dlg.m_time.Format(_T("%02d:%02d:%02d.%03d"), h, m, s, ms);

    if(IDOK != dlg.DoModal())
        return false;

    TCHAR c;
    if(_stscanf(dlg.m_time, _T("%d%c%d%c%d%c%d"), &h, &c, &m, &c, &s, &c, &ms) != 7)
        return false;

    t->m_chap.name = dlg.m_name;
    t->m_chap.rt = ((((__int64)h * 60 + m) * 60 + s) * 1000 + ms) * 10000;

    t->Update();

    return true;
}

void CConvertDlg::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TREE1, m_tree);
    DDX_Text(pDX, IDC_EDIT1, m_fn);
}

BOOL CConvertDlg::PreTranslateMessage(MSG* pMsg)
{
    if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
        return TRUE;

    return __super::PreTranslateMessage(pMsg);
}

BOOL CConvertDlg::OnInitDialog()
{
    __super::OnInitDialog();

    SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), TRUE);
    SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), FALSE);

    AddAnchor(IDC_TREE1, TOP_LEFT, BOTTOM_RIGHT);
    AddAnchor(IDC_EDIT1, BOTTOM_LEFT, BOTTOM_RIGHT);
    AddAnchor(IDC_BUTTON1, BOTTOM_RIGHT);
    AddAnchor(IDC_HLINE, BOTTOM_LEFT, BOTTOM_RIGHT);
    AddAnchor(IDC_BUTTON2, BOTTOM_CENTER);
    AddAnchor(IDC_BUTTON3, BOTTOM_CENTER);
    AddAnchor(IDC_BUTTON4, BOTTOM_CENTER);

    CSize s(400, 200);
    SetMinTrackSize(s);

    m_streamtypesbm.LoadBitmap(IDB_STREAMTYPES);
    m_streamtypes.Create(16, 18, ILC_MASK | ILC_COLOR32, 0, 4);
    m_streamtypes.Add(&m_streamtypesbm, 0xffffff);
    m_tree.SetImageList(&m_streamtypes, TVSIL_NORMAL);

    GetWindowText(m_title);
    m_nIDEventStatus = SetTimer(1, 1000, NULL);

    HRESULT hr;
    m_pMux = DNew CDSMMuxerFilter(NULL, &hr, false, false);

    m_pGB = DNew CFGManagerMuxer(_T("CFGManagerMuxer"), NULL);
    m_pGB->AddToROT();

    if(FAILED(m_pGB->AddFilter(m_pMux, L"Mux"))
       || !(m_pMC = m_pGB) || !(m_pME = m_pGB) || !(m_pMS = m_pMux)
       || FAILED(m_pME->SetNotifyWindow((OAHWND)m_hWnd, WM_GRAPHNOTIFY, 0)))
    {
        MessageBeep(-1);
        SendMessage(WM_CLOSE);
        return TRUE;
    }

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CConvertDlg::OnOK()
{
}

BEGIN_MESSAGE_MAP(CConvertDlg, CResizableDialog)
    ON_MESSAGE(WM_GRAPHNOTIFY, OnGraphNotify)
    ON_WM_DROPFILES()
    ON_WM_CLOSE()
    ON_NOTIFY(NM_CLICK, IDC_TREE1, OnNMClickTree1)
    ON_NOTIFY(NM_RCLICK, IDC_TREE1, OnNMRclickTree1)
    ON_NOTIFY(NM_DBLCLK, IDC_TREE1, OnNMDblclkTree1)
    ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
    ON_UPDATE_COMMAND_UI(IDC_BUTTON1, OnUpdateButton1)
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_BUTTON2, OnBnClickedButton2)
    ON_UPDATE_COMMAND_UI(IDC_BUTTON2, OnUpdateButton2)
    ON_BN_CLICKED(IDC_BUTTON3, OnBnClickedButton3)
    ON_UPDATE_COMMAND_UI(IDC_BUTTON3, OnUpdateButton3)
    ON_BN_CLICKED(IDC_BUTTON4, OnBnClickedButton4)
    ON_UPDATE_COMMAND_UI(IDC_BUTTON4, OnUpdateButton4)
END_MESSAGE_MAP()

// CConvertDlg message handlers

LRESULT CConvertDlg::OnGraphNotify(WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;

    LONG evCode, evParam1, evParam2;
    while(m_pME && SUCCEEDED(m_pME->GetEvent(&evCode, (LONG_PTR*)&evParam1, (LONG_PTR*)&evParam2, 0)))
    {
        hr = m_pME->FreeEventParams(evCode, evParam1, evParam2);

        bool fStop = false;

        if(EC_COMPLETE == evCode)
        {
            fStop = true;
        }
        else if(EC_ERRORABORT == evCode)
        {
            fStop = true;

            CString errmsg;
            LPVOID lpMsgBuf;
            if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                             NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL))
            {
                errmsg = (LPCTSTR)lpMsgBuf;
                LocalFree(lpMsgBuf);
            }

            CString str;
            str.Format(ResStr(IDS_CONVERTDLG_7), evParam1);
            if(!errmsg.IsEmpty()) str += _T(" (") + errmsg + _T(")");
            AfxMessageBox(str, MB_OK);
        }

        if(fStop && m_pMC)
        {
            m_pMC->Stop();
            m_tree.EnableWindow(TRUE);
        }
    }

    return hr;
}

void CConvertDlg::OnDropFiles(HDROP hDropInfo)
{
    for(int i = 0, j = DragQueryFile(hDropInfo, 0xffffffff, 0, 0); i < j; i++)
    {
        CString fn;
        fn.ReleaseBufferSetLength(DragQueryFile(hDropInfo, i, fn.GetBuffer(MAX_PATH), MAX_PATH));

        AddFile(fn);
    }

    __super::OnDropFiles(hDropInfo);
}

void CConvertDlg::OnClose()
{
    HTREEITEM hTI = m_tree.GetRootItem();
    while(hTI)
    {
        HTREEITEM hTINext = m_tree.GetNextSiblingItem(hTI);
        DeleteItem(hTI);
        hTI = hTINext;
    }

    m_pGB->RemoveFromROT();
    m_pGB = NULL;

    __super::OnClose();
}

void CConvertDlg::OnNMClickTree1(NMHDR* pNMHDR, LRESULT* pResult)
{
    CPoint sp, cp;
    HTREEITEM hTI = HitTest(sp, cp);
    if(!hTI) return;
    m_tree.SelectItem(hTI);

    *pResult = 0;
}

void CConvertDlg::OnNMRclickTree1(NMHDR* pNMHDR, LRESULT* pResult)
{
    CPoint sp, cp;
    HTREEITEM hTI = HitTest(sp, cp);

    if(hTI)
    {
        m_tree.SelectItem(hTI);

        CTreeItem* t = (CTreeItem*)m_tree.GetItemData(hTI);

        if(dynamic_cast<CTreeItemPin*>(t))
            ShowPinPopup(hTI, sp);
        else if(dynamic_cast<CTreeItemFile*>(t))
            ShowFilePopup(hTI, sp);
        else if(dynamic_cast<CTreeItemResourceFolder*>(t))
            ShowResourceFolderPopup(hTI, sp);
        else if(dynamic_cast<CTreeItemResource*>(t))
            ShowResourcePopup(hTI, sp);
        else if(dynamic_cast<CTreeItemChapterFolder*>(t))
            ShowChapterFolderPopup(hTI, sp);
        else if(dynamic_cast<CTreeItemChapter*>(t))
            ShowChapterPopup(hTI, sp);
    }
    else
    {
        ShowPopup(sp);
    }

    *pResult = 0;
}

void CConvertDlg::OnNMDblclkTree1(NMHDR *pNMHDR, LRESULT *pResult)
{
    CPoint sp, cp;
    HTREEITEM hTI = HitTest(sp, cp);

    if(hTI)
    {
        CTreeItem* t = (CTreeItem*)m_tree.GetItemData(hTI);

        if(CTreeItemPin* t2 = dynamic_cast<CTreeItemPin*>(t))
        {
            CComPtr<IPin> pPinTo;
            t2->m_pPin->ConnectedTo(&pPinTo);

            if(CComQIPtr<IDSMPropertyBag> pPB = pPinTo)
                EditProperties(pPB);
        }
        else if(CTreeItemResource* t2 = dynamic_cast<CTreeItemResource*>(t))
        {
            EditResource(t2);
        }
        else if(CTreeItemChapter* t2 = dynamic_cast<CTreeItemChapter*>(t))
        {
            EditChapter(t2);
        }
    }

    *pResult = 0;
}

void CConvertDlg::OnBnClickedButton1()
{
    UpdateData();

    CFileDialog fd(FALSE, _T(".dsm"), m_fn,
                   OFN_EXPLORER | OFN_ENABLESIZING | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                   ResStr(IDS_CONVERTDLG_8), this, 0);

    if(fd.DoModal() == IDOK)
    {
        m_fn = fd.GetPathName();
        UpdateData(FALSE);
    }
}

void CConvertDlg::OnUpdateButton1(CCmdUI* pCmdUI)
{
    OAFilterState fs;
    pCmdUI->Enable(m_pMC && SUCCEEDED(m_pMC->GetState(0, &fs)) && fs == State_Stopped);
}

void CConvertDlg::OnTimer(UINT_PTR nIDEvent)
{
    if(nIDEvent == m_nIDEventStatus && m_pMS && m_pMC)
    {
        OAFilterState fs;
        if(SUCCEEDED(m_pMC->GetState(0, &fs)) && fs != State_Stopped)
        {
            GUID tf;
            m_pMS->GetTimeFormat(&tf);

            REFERENCE_TIME rtCur, rtDur;
            HRESULT hr = m_pMS->GetDuration(&rtDur);
            m_pMS->GetCurrentPosition(&rtCur);

            CString str;
            if(hr == S_OK && rtDur != 0) str.Format(_T("%.2f%%"), 1.0 *(rtCur * 100) / rtDur);
            else if(hr == S_OK && rtDur == 0) str = _T("Live");
            else if(tf == TIME_FORMAT_BYTE) str.Format(_T("%.2fKB"), 1.0 * rtCur / 1024);
            else if(tf == TIME_FORMAT_MEDIA_TIME) str.Format(_T("%02d:%02d:%02d"), int(rtCur / 3600000000) % 60, int(rtCur / 60000000) % 60, int(rtCur / 1000000) % 60);
            else str = ResStr(IDS_AG_PLEASE_WAIT);

            SetWindowText(ResStr(IDS_AG_CONVERTING) + str);
        }
        else
        {
            SetWindowText(m_title);
        }
    }

    __super::OnTimer(nIDEvent);
}

void CConvertDlg::OnBnClickedButton2()
{
    OAFilterState fs;
    if(FAILED(m_pMC->GetState(0, &fs)))
        return;

    if(fs != State_Stopped)
    {
        m_pMC->Run();
        return;
    }

    UpdateData();

    if(!ConvertFile(m_fn))
    {
        AfxMessageBox(ResStr(IDS_CONVERTDLG_2));
    }
}

void CConvertDlg::OnUpdateButton2(CCmdUI* pCmdUI)
{
    int nIn, nOut, nInC, nOutC;
    CountPins(m_pMux, nIn, nOut, nInC, nOutC);

    OAFilterState fs;
    pCmdUI->Enable(nInC > 0 && GetDlgItem(IDC_EDIT1)->GetWindowTextLength() > 0
                   && m_pMS && m_pMC && SUCCEEDED(m_pMC->GetState(0, &fs)) && fs != State_Running);
}

void CConvertDlg::OnBnClickedButton3()
{
    if(m_pMC) m_pMC->Pause();
}

void CConvertDlg::OnUpdateButton3(CCmdUI* pCmdUI)
{
    OAFilterState fs;
    pCmdUI->Enable(m_pMC && SUCCEEDED(m_pMC->GetState(0, &fs)) && fs == State_Running);
}

void CConvertDlg::OnBnClickedButton4()
{
    if(m_pMC) m_pMC->Stop();
    m_tree.EnableWindow(TRUE);
}

void CConvertDlg::OnUpdateButton4(CCmdUI* pCmdUI)
{
    OAFilterState fs;
    pCmdUI->Enable(m_pMC && SUCCEEDED(m_pMC->GetState(0, &fs)) && fs != State_Stopped);
}

//
// CFilterTreeCtrl
//

CFilterTreeCtrl::CFilterTreeCtrl()
{
}

void CFilterTreeCtrl::PreSubclassWindow()
{
    EnableToolTips(TRUE);

    __super::PreSubclassWindow();
}

INT_PTR CFilterTreeCtrl::OnToolHitTest(CPoint p, TOOLINFO* pTI) const
{
    UINT nFlags;
    HTREEITEM hTI = HitTest(p, &nFlags);
    if(nFlags & TVHT_ONITEM)
    {
        CRect r;
        GetItemRect(hTI, r, TRUE);
        pTI->hwnd = m_hWnd;
        pTI->uId = (UINT)hTI;
        pTI->lpszText = LPSTR_TEXTCALLBACK;
        pTI->rect = r;
        return pTI->uId;
    }

    return -1;
}

BEGIN_MESSAGE_MAP(CFilterTreeCtrl, CTreeCtrl)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
END_MESSAGE_MAP()

BOOL CFilterTreeCtrl::OnToolTipText(UINT id, NMHDR* pNMHDR, LRESULT* pResult)
{
    TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
    TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;

    UINT nID = pNMHDR->idFrom;

    if(nID == (UINT)m_hWnd
       && (pNMHDR->code == TTN_NEEDTEXTA && (pTTTA->uFlags & TTF_IDISHWND)
           || pNMHDR->code == TTN_NEEDTEXTW && (pTTTW->uFlags & TTF_IDISHWND)))
        return FALSE;

    ::SendMessage(pNMHDR->hwndFrom, TTM_SETMAXTIPWIDTH, 0, (LPARAM)(INT)1000);

    HTREEITEM hTI = (HTREEITEM)nID;

    CString str;
    static CStringA m_strTipTextA;
    static CStringW m_strTipTextW;

    CConvertDlg::CTreeItem* t = (CConvertDlg::CTreeItem*)GetItemData(hTI);
    if(!t || !t->ToolTip(str)) return FALSE;

    m_strTipTextA = str;
    m_strTipTextW = str;

    if(pNMHDR->code == TTN_NEEDTEXTA) pTTTA->lpszText = (LPSTR)(LPCSTR)m_strTipTextA;
    else pTTTW->lpszText = (LPWSTR)(LPCWSTR)m_strTipTextW;

    *pResult = 0;

    return TRUE;    // message was handled
}

//
// CConvertDlg::CTreeItem*
//

CConvertDlg::CTreeItem::CTreeItem(CTreeCtrl& tree, HTREEITEM hTIParent)
    : m_tree(tree)
{
    m_hTI = m_tree.InsertItem(_T(""), hTIParent);
    m_tree.SetItemData(m_hTI, (DWORD_PTR)this);
    Update();
}

CConvertDlg::CTreeItem::~CTreeItem()
{
}

void CConvertDlg::CTreeItem::SetLabel(LPCTSTR label)
{
    m_tree.SetItemText(m_hTI, label);
}

void CConvertDlg::CTreeItem::SetImage(int nImage, int nSelectedImage)
{
    m_tree.SetItemImage(m_hTI, nImage, nSelectedImage);
}

//

CConvertDlg::CTreeItemFilter::CTreeItemFilter(IBaseFilter* pBF, CTreeCtrl& tree, HTREEITEM hTIParent)
    : CTreeItem(tree, hTIParent)
    , m_pBF(pBF)
{
    Update();
}

void CConvertDlg::CTreeItemFilter::Update()
{
    SetLabel(CString(GetFilterName(m_pBF)));
}

//

CConvertDlg::CTreeItemFile::CTreeItemFile(CString fn, IBaseFilter* pBF, CTreeCtrl& tree, HTREEITEM hTIParent)
    : CTreeItemFilter(pBF, tree, hTIParent)
    , m_fn(fn)
{
    Update();
}

void CConvertDlg::CTreeItemFile::Update()
{
    CPath path = m_fn;
    path.StripPath();
    SetLabel(path);
}

bool CConvertDlg::CTreeItemFile::ToolTip(CString& str)
{
    str = m_fn;
    return true;
}

//

CConvertDlg::CTreeItemPin::CTreeItemPin(IPin* pPin, CTreeCtrl& tree, HTREEITEM hTIParent)
    : CTreeItem(tree, hTIParent)
    , m_pPin(pPin)
{
    Update();
}

void CConvertDlg::CTreeItemPin::Update()
{
    if(!m_pPin)
    {
        ASSERT(0);
        return;
    }

    CString label = GetPinName(m_pPin);
    if(!IsConnected()) label = _T("[D] ") + label;
    SetLabel(label);

    CMediaType mt;
    if(S_OK == m_pPin->ConnectionMediaType(&mt))
    {
        if(mt.majortype == MEDIATYPE_Video) SetImage(1, 1);
        else if(mt.majortype == MEDIATYPE_Audio) SetImage(2, 2);
        else if(mt.majortype == MEDIATYPE_Text || mt.majortype == MEDIATYPE_Subtitle) SetImage(3, 3);
    }
}

bool CConvertDlg::CTreeItemPin::ToolTip(CString& str)
{
    CMediaTypeEx mt;
    if(FAILED(m_pPin->ConnectionMediaType(&mt))) return false;
    str = mt.ToString(m_pPin);
    return true;
}

bool CConvertDlg::CTreeItemPin::IsConnected()
{
    CComPtr<IPin> pPinTo;
    return m_pPin && SUCCEEDED(m_pPin->ConnectedTo(&pPinTo)) && pPinTo;
}

//

CConvertDlg::CTreeItemResourceFolder::CTreeItemResourceFolder(CTreeCtrl& tree, HTREEITEM hTIParent)
    : CTreeItem(tree, hTIParent)
{
    Update();
}

void CConvertDlg::CTreeItemResourceFolder::Update()
{
    SetLabel(ResStr(IDS_AG_RESOURCES));
}

bool CConvertDlg::CTreeItemResourceFolder::ToolTip(CString& str)
{
    if(!m_tree.ItemHasChildren(m_hTI))
        return false;

    int files = 0;
    float size = 0;

    HTREEITEM hChildItem = m_tree.GetChildItem(m_hTI);

    while(hChildItem != NULL)
    {
        HTREEITEM hNextItem = m_tree.GetNextItem(hChildItem, TVGN_NEXT);
        if(CTreeItemResource* t = dynamic_cast<CTreeItemResource*>((CTreeItem*)m_tree.GetItemData(hChildItem)))
            size += t->m_res.data.GetCount(), files++;
        hChildItem = hNextItem;
    }

    size /= 1024;
    if(size < 1024) str.Format(ResStr(IDS_CONVERTDLG_13), files, size);
    else str.Format(ResStr(IDS_CONVERTDLG_14), files, size / 1024);

    return true;
}

//

CConvertDlg::CTreeItemResource::CTreeItemResource(const CDSMResource& res, CTreeCtrl& tree, HTREEITEM hTIParent)
    : CTreeItem(tree, hTIParent)
{
    m_res = res;
    Update();
}

CConvertDlg::CTreeItemResource::~CTreeItemResource()
{
}

void CConvertDlg::CTreeItemResource::Update()
{
    SetLabel(CString(m_res.name));

    CStringW mime = m_res.mime;
    mime.Trim();
    mime.MakeLower();
    if(mime == L"application/x-truetype-font") SetImage(4, 4);
    else if(mime.Find(L"text/") == 0) SetImage(5, 5);
    else SetImage(6, 6);
}

bool CConvertDlg::CTreeItemResource::ToolTip(CString& str)
{
    if(!m_res.mime.IsEmpty()) str = CString(m_res.mime) + _T("\r\n\r\n");
    if(!m_res.desc.IsEmpty()) str += CString(m_res.desc);
    str.Trim();
    return true;
}

//

CConvertDlg::CTreeItemChapterFolder::CTreeItemChapterFolder(CTreeCtrl& tree, HTREEITEM hTIParent)
    : CTreeItem(tree, hTIParent)
{
    Update();
}

void CConvertDlg::CTreeItemChapterFolder::Update()
{
    SetLabel(ResStr(IDS_AG_CHAPTERS));
}

//

CConvertDlg::CTreeItemChapter::CTreeItemChapter(const CDSMChapter& chap, CTreeCtrl& tree, HTREEITEM hTIParent)
    : CTreeItem(tree, hTIParent)
{
    m_chap = chap;
    Update();
}

void CConvertDlg::CTreeItemChapter::Update()
{
    REFERENCE_TIME rt = m_chap.rt;
    rt /= 10000;
    int ms = (int)(rt % 1000);
    rt /= 1000;
    int s = (int)(rt % 60);
    rt /= 60;
    int m = (int)(rt % 60);
    rt /= 60;
    int h = (int)(rt);

    CString label;
    label.Format(_T("%02d:%02d:%02d.%03d - %s"), h, m, s, ms, CString(m_chap.name));
    SetLabel(label);

    SetImage(7, 7);
}
