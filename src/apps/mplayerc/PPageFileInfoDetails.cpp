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
#include "mplayerc.h"
#include "PPageFileInfoDetails.h"
#include <atlbase.h>
#include "../../DSUtil/DSUtil.h"
#include <d3d9.h>
#include "Vmr9.h"
#include <moreuuids.h>


// CPPageFileInfoDetails dialog

IMPLEMENT_DYNAMIC(CPPageFileInfoDetails, CPropertyPage)
CPPageFileInfoDetails::CPPageFileInfoDetails(CString fn, IFilterGraph* pFG, ISubPicAllocatorPresenter* pCAP)
    : CPropertyPage(CPPageFileInfoDetails::IDD, CPPageFileInfoDetails::IDD)
    , m_fn(fn)
    , m_pFG(pFG)
    , m_pCAP(pCAP)
    , m_hIcon(NULL)
    , m_type(ResStr(IDS_AG_NOT_KNOWN))
    , m_size(ResStr(IDS_AG_NOT_KNOWN))
    , m_time(ResStr(IDS_AG_NOT_KNOWN))
    , m_res(ResStr(IDS_AG_NOT_KNOWN))
    , m_created(ResStr(IDS_AG_NOT_KNOWN))
{
}

CPPageFileInfoDetails::~CPPageFileInfoDetails()
{
    if(m_hIcon) DestroyIcon(m_hIcon);
}

void CPPageFileInfoDetails::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_DEFAULTICON, m_icon);
    DDX_Text(pDX, IDC_EDIT1, m_fn);
    DDX_Text(pDX, IDC_EDIT4, m_type);
    DDX_Text(pDX, IDC_EDIT3, m_size);
    DDX_Text(pDX, IDC_EDIT2, m_time);
    DDX_Text(pDX, IDC_EDIT5, m_res);
    DDX_Text(pDX, IDC_EDIT6, m_created);
    DDX_Control(pDX, IDC_EDIT7, m_encoding);
}

BEGIN_MESSAGE_MAP(CPPageFileInfoDetails, CPropertyPage)
END_MESSAGE_MAP()

inline int LNKO(int a, int b)
{
    if(a == 0 || b == 0)
        return(1);

    while(a != b)
    {
        if(a < b) b -= a;
        else if(a > b) a -= b;
    }

    return(a);
}

// CPPageFileInfoDetails message handlers

static bool GetProperty(IFilterGraph* pFG, LPCOLESTR propName, VARIANT* vt)
{
    BeginEnumFilters(pFG, pEF, pBF)
    {
        if(CComQIPtr<IPropertyBag> pPB = pBF)
            if(SUCCEEDED(pPB->Read(propName, vt, NULL)))
                return true;
    }
    EndEnumFilters

    return false;
}

static CString FormatDateTime(FILETIME tm)
{
    SYSTEMTIME t;
    FileTimeToSystemTime(&tm, &t);
    TCHAR buff[256];
    GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &t, NULL, buff, 256);
    CString	ret(buff);
    ret += _T(" ");
    GetTimeFormat(LOCALE_USER_DEFAULT, 0, &t, NULL, buff, 256);
    ret += buff;
    return ret;
}

BOOL CPPageFileInfoDetails::OnInitDialog()
{
    __super::OnInitDialog();

    CString ext = m_fn.Left(m_fn.Find(_T("://"))+1).TrimRight(':');
    if(ext.IsEmpty() || !ext.CompareNoCase(_T("file")))
        ext = _T(".") + m_fn.Mid(m_fn.ReverseFind('.')+1);

    if(m_hIcon = LoadIcon(m_fn, false))
        m_icon.SetIcon(m_hIcon);

    if(!LoadType(ext, m_type))
        m_type = ResStr(IDS_AG_NOT_KNOWN);

    CComVariant vt;
    if(::GetProperty(m_pFG, L"CurFile.TimeCreated", &vt))
    {
        if(V_VT(&vt) == VT_UI8)
        {
            ULARGE_INTEGER  uli;
            uli.QuadPart = V_UI8(&vt);

            FILETIME ft;
            ft.dwLowDateTime = uli.LowPart;
            ft.dwHighDateTime = uli.HighPart;

            m_created = FormatDateTime(ft);
        }
    }

    WIN32_FIND_DATA wfd;
    HANDLE hFind = FindFirstFile(m_fn, &wfd);
    if(hFind != INVALID_HANDLE_VALUE)
    {
        FindClose(hFind);

        __int64 size = (__int64(wfd.nFileSizeHigh)<<32)|wfd.nFileSizeLow;
        __int64 shortsize = size;
        CString measure = _T("B");
        if(shortsize > 10240) shortsize /= 1024, measure = _T("KB");
        if(shortsize > 10240) shortsize /= 1024, measure = _T("MB");
        if(shortsize > 10240) shortsize /= 1024, measure = _T("GB");
        m_size.Format(_T("%I64d%s (%I64d bytes)"), shortsize, measure, size);

        if(m_created.IsEmpty())
        {
            m_created = FormatDateTime(wfd.ftCreationTime);
        }
    }

    REFERENCE_TIME rtDur = 0;
    CComQIPtr<IMediaSeeking> pMS = m_pFG;
    if(pMS && SUCCEEDED(pMS->GetDuration(&rtDur)) && rtDur > 0)
    {
        m_time.Format(_T("%02d:%02d:%02d"),
                      int(rtDur/10000000/60/60),
                      int((rtDur/10000000/60)%60),
                      int((rtDur/10000000)%60));
    }

    CSize wh(0, 0), arxy(0, 0);
    long fps = 0;

    if(m_pCAP)
    {
        wh = m_pCAP->GetVideoSize(false);
        arxy = m_pCAP->GetVideoSize(true);
    }
    else
    {
        if(CComQIPtr<IBasicVideo> pBV = m_pFG)
        {
            if(SUCCEEDED(pBV->GetVideoSize(&wh.cx, &wh.cy)))
            {
                if(CComQIPtr<IBasicVideo2> pBV2 = m_pFG)
                    pBV2->GetPreferredAspectRatio(&arxy.cx, &arxy.cy);
            }
            else
            {
                wh.SetSize(0, 0);
            }
        }

        if(wh.cx == 0 && wh.cy == 0)
        {
            BeginEnumFilters(m_pFG, pEF, pBF)
            {
                if(CComQIPtr<IBasicVideo> pBV = pBF)
                {
                    pBV->GetVideoSize(&wh.cx, &wh.cy);
                    if(CComQIPtr<IBasicVideo2> pBV2 = pBF)
                        pBV2->GetPreferredAspectRatio(&arxy.cx, &arxy.cy);
                    break;
                }
                else if(CComQIPtr<IVMRWindowlessControl> pWC = pBF)
                {
                    pWC->GetNativeVideoSize(&wh.cx, &wh.cy, &arxy.cx, &arxy.cy);
                    break;
                }
                else if(CComQIPtr<IVMRWindowlessControl9> pWC = pBF)
                {
                    pWC->GetNativeVideoSize(&wh.cx, &wh.cy, &arxy.cx, &arxy.cy);
                    break;
                }
            }
            EndEnumFilters
        }
    }

    if(wh.cx > 0 && wh.cy > 0)
    {
        m_res.Format(_T("%d x %d"), wh.cx, wh.cy);

        int lnko = 0;
        do
        {
            lnko = LNKO(arxy.cx, arxy.cy);
            if(lnko > 1) arxy.cx /= lnko, arxy.cy /= lnko;
        }
        while(lnko > 1);

        if(arxy.cx > 0 && arxy.cy > 0 && arxy.cx*wh.cy != arxy.cy*wh.cx)
        {
            CString ar;
            ar.Format(_T(" (AR %d:%d)"), arxy.cx, arxy.cy);
            m_res += ar;
        }
    }

    m_fn.TrimRight('/');
    m_fn.Replace('\\', '/');
    m_fn = m_fn.Mid(m_fn.ReverseFind('/')+1);

    UpdateData(FALSE);

    InitEncoding();

    m_pFG = NULL;
    m_pCAP = NULL;

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CPPageFileInfoDetails::InitEncoding()
{
    CAtlList<CString> sl;

    BeginEnumFilters(m_pFG, pEF, pBF)
    {
        CComPtr<IBaseFilter> pUSBF = GetUpStreamFilter(pBF);

        if(GetCLSID(pBF) == CLSID_NetShowSource)
        {
            continue;
        }
        else if(GetCLSID(pUSBF) != CLSID_NetShowSource)
        {
            if(IPin* pPin = GetFirstPin(pBF, PINDIR_INPUT))
            {
                CMediaType mt;
                if(FAILED(pPin->ConnectionMediaType(&mt)) || mt.majortype != MEDIATYPE_Stream)
                    continue;
            }

            if(IPin* pPin = GetFirstPin(pBF, PINDIR_OUTPUT))
            {
                CMediaType mt;
                if(SUCCEEDED(pPin->ConnectionMediaType(&mt)) && mt.majortype == MEDIATYPE_Stream)
                    continue;
            }
        }

        BeginEnumPins(pBF, pEP, pPin)
        {
            CMediaTypeEx mt;
            PIN_DIRECTION dir;
            if(FAILED(pPin->QueryDirection(&dir)) || dir != PINDIR_OUTPUT
               || FAILED(pPin->ConnectionMediaType(&mt)))
                continue;

            CString str = mt.ToString();

            if(!str.IsEmpty())
            {
                sl.AddTail(mt.ToString() + CString(L" [" + GetPinName(pPin) + L"]"));
            }
        }
        EndEnumPins
    }
    EndEnumFilters

    CString text = Implode(sl, '\n');
    text.Replace(_T("\n"), _T("\r\n"));
    m_encoding.SetWindowText(text);
}
