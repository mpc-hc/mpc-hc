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
#include "mplayerc.h"
#include "PPageFileInfoClip.h"
#include <atlbase.h>
#include <qnetwork.h>
#include "../DSUtil/DSUtil.h"
#include "../DSUtil/WinAPIUtils.h"


// CPPageFileInfoClip dialog

IMPLEMENT_DYNAMIC(CPPageFileInfoClip, CPropertyPage)
CPPageFileInfoClip::CPPageFileInfoClip(CString fn, IFilterGraph* pFG)
    : CPropertyPage(CPPageFileInfoClip::IDD, CPPageFileInfoClip::IDD)
    , m_fn(fn)
    , m_pFG(pFG)
    , m_clip(ResStr(IDS_AG_NONE))
    , m_author(ResStr(IDS_AG_NONE))
    , m_copyright(ResStr(IDS_AG_NONE))
    , m_rating(ResStr(IDS_AG_NONE))
    , m_location_str(ResStr(IDS_AG_NONE))
    , m_hIcon(NULL)
{
}

CPPageFileInfoClip::~CPPageFileInfoClip()
{
    if (m_hIcon) {
        DestroyIcon(m_hIcon);
    }
}

BOOL CPPageFileInfoClip::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_LBUTTONDBLCLK && pMsg->hwnd == m_location.m_hWnd && !m_location_str.IsEmpty()) {
        CString path = m_location_str;
        if (path[path.GetLength() - 1] != '\\') {
            path += _T("\\");
        }
        path += m_fn;

        if (ExploreToFile(path)) {
            return TRUE;
        }
    }

    m_tooltip.RelayEvent(pMsg);

    return __super::PreTranslateMessage(pMsg);
}


void CPPageFileInfoClip::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_DEFAULTICON, m_icon);
    DDX_Text(pDX, IDC_EDIT1, m_fn);
    DDX_Text(pDX, IDC_EDIT4, m_clip);
    DDX_Text(pDX, IDC_EDIT3, m_author);
    DDX_Text(pDX, IDC_EDIT2, m_copyright);
    DDX_Text(pDX, IDC_EDIT5, m_rating);
    DDX_Control(pDX, IDC_EDIT6, m_location);
    DDX_Control(pDX, IDC_EDIT7, m_desc);
}

#define SETPAGEFOCUS WM_APP+252 // arbitrary number, can be changed if necessary
BEGIN_MESSAGE_MAP(CPPageFileInfoClip, CPropertyPage)
    ON_MESSAGE(SETPAGEFOCUS, OnSetPageFocus)
END_MESSAGE_MAP()


// CPPageFileInfoClip message handlers

BOOL CPPageFileInfoClip::OnInitDialog()
{
    __super::OnInitDialog();

    if (m_fn == _T("")) {
        BeginEnumFilters(m_pFG, pEF, pBF) {
            CComQIPtr<IFileSourceFilter> pFSF = pBF;
            if (pFSF) {
                LPOLESTR pFN = NULL;
                AM_MEDIA_TYPE mt;
                if (SUCCEEDED(pFSF->GetCurFile(&pFN, &mt)) && pFN && *pFN) {
                    m_fn = CStringW(pFN);
                    CoTaskMemFree(pFN);
                }
                break;
            }
        }
        EndEnumFilters
    }

    m_hIcon = LoadIcon(m_fn, false);
    if (m_hIcon) {
        m_icon.SetIcon(m_hIcon);
    }

    m_fn.TrimRight('/');
    int i = max(m_fn.ReverseFind('\\'), m_fn.ReverseFind('/'));
    if (i >= 0 && i < m_fn.GetLength() - 1) {
        m_location_str = m_fn.Left(i);
        m_fn = m_fn.Mid(i + 1);

        if (m_location_str.GetLength() == 2 && m_location_str[1] == ':') {
            m_location_str += '\\';
        }
    }
    m_location.SetWindowText(m_location_str);

    bool fEmpty = true;
    BeginEnumFilters(m_pFG, pEF, pBF) {
        if (CComQIPtr<IAMMediaContent, &IID_IAMMediaContent> pAMMC = pBF) {
            CComBSTR bstr;
            if (SUCCEEDED(pAMMC->get_Title(&bstr)) && wcslen(bstr.m_str) > 0) {
                m_clip = bstr.m_str;
                fEmpty = false;
            }
            if (SUCCEEDED(pAMMC->get_AuthorName(&bstr)) && wcslen(bstr.m_str) > 0) {
                m_author = bstr.m_str;
                fEmpty = false;
            }
            if (SUCCEEDED(pAMMC->get_Copyright(&bstr)) && wcslen(bstr.m_str) > 0) {
                m_copyright = bstr.m_str;
                fEmpty = false;
            }
            if (SUCCEEDED(pAMMC->get_Rating(&bstr)) && wcslen(bstr.m_str) > 0) {
                m_rating = bstr.m_str;
                fEmpty = false;
            }
            if (SUCCEEDED(pAMMC->get_Description(&bstr)) && wcslen(bstr.m_str) > 0) {
                m_desc.SetWindowText(CString(bstr.m_str));
                fEmpty = false;
            }
            if (!fEmpty) {
                break;
            }
        }
    }
    EndEnumFilters;

    m_tooltip.Create(this, TTS_NOPREFIX | TTS_ALWAYSTIP);

    m_tooltip.SetDelayTime(TTDT_INITIAL, 0);
    m_tooltip.SetDelayTime(TTDT_AUTOPOP, 2500);
    m_tooltip.SetDelayTime(TTDT_RESHOW, 0);

    m_tooltip.AddTool(&m_location, IDS_TOOLTIP_EXPLORE_TO_FILE);

    UpdateData(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPageFileInfoClip::OnSetActive()
{
    BOOL ret = __super::OnSetActive();

    PostMessage(SETPAGEFOCUS, 0, 0L);

    return ret;
}

LRESULT CPPageFileInfoClip::OnSetPageFocus(WPARAM wParam, LPARAM lParam)
{
    CPropertySheet* psheet = (CPropertySheet*) GetParent();
    psheet->GetTabControl()->SetFocus();

    return 0;
}