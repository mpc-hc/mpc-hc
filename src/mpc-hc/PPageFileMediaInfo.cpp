/*
 * (C) 2009-2013 see Authors.txt
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

// PPageFileMediaInfo.cpp : implementation file


#include "stdafx.h"
#include "mplayerc.h"
#include "PPageFileMediaInfo.h"
#include "WinAPIUtils.h"

#if USE_STATIC_MEDIAINFO
#include "MediaInfo/MediaInfo.h"
using namespace MediaInfoLib;
#define MediaInfo_int64u ZenLib::int64u
#else
#include "MediaInfoDLL.h"
using namespace MediaInfoDLL;
#endif

#define MEDIAINFO_BUFFER_SIZE 1024 * 256

// CPPageFileMediaInfo dialog

IMPLEMENT_DYNAMIC(CPPageFileMediaInfo, CPropertyPage)
CPPageFileMediaInfo::CPPageFileMediaInfo(CString path, IFilterGraph* pFG)
    : CPropertyPage(CPPageFileMediaInfo::IDD, CPPageFileMediaInfo::IDD)
    , m_fn(path)
    , m_path(path)
    , m_pFG(pFG)
    , m_pCFont(nullptr)
{
}

CPPageFileMediaInfo::~CPPageFileMediaInfo()
{
    delete m_pCFont;
    m_pCFont = nullptr;
}

void CPPageFileMediaInfo::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_MIEDIT, m_mediainfo);
}


BEGIN_MESSAGE_MAP(CPPageFileMediaInfo, CPropertyPage)
    ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

// CPPageFileMediaInfo message handlers
static WNDPROC OldControlProc;

static LRESULT CALLBACK ControlProc(HWND control, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_KEYDOWN) {
        if ((LOWORD(wParam) == 'A' || LOWORD(wParam) == 'a')
                && (GetKeyState(VK_CONTROL) < 0)) {
            CEdit* pEdit = (CEdit*)CWnd::FromHandle(control);
            pEdit->SetSel(0, pEdit->GetWindowTextLength(), TRUE);
            return 0;
        }
    }

    return CallWindowProc(OldControlProc, control, message, wParam, lParam); // call edit control's own windowproc
}

BOOL CPPageFileMediaInfo::OnInitDialog()
{
    __super::OnInitDialog();

    if (!m_pCFont) {
        m_pCFont = DEBUG_NEW CFont;
    }
    if (!m_pCFont) {
        return TRUE;
    }

    CComQIPtr<IAsyncReader> pAR;
    BeginEnumFilters(m_pFG, pEF, pBF) {
        if (CComQIPtr<IFileSourceFilter> pFSF = pBF) {
            LPOLESTR pFN;
            if (SUCCEEDED(pFSF->GetCurFile(&pFN, nullptr))) {
                m_fn = pFN;
                CoTaskMemFree(pFN);
            }
            BeginEnumPins(pBF, pEP, pPin) {
                if (pAR = pPin) {
                    break;
                }
            }
            EndEnumPins;
            break;
        }
    }
    EndEnumFilters;

    if (m_path.IsEmpty()) {
        m_path = m_fn;
    }

#if USE_STATIC_MEDIAINFO
    MediaInfoLib::String f_name = m_path;
    MediaInfoLib::MediaInfo MI;
#else
    MediaInfoDLL::String f_name = m_path;
    MediaInfo MI;
#endif

    MI.Option(_T("ParseSpeed"), _T("0"));
    MI.Option(_T("Complete"));
    MI.Option(_T("Language"), _T("  Config_Text_ColumnSize;30"));

    LONGLONG llSize, llAvailable;
    if (pAR && SUCCEEDED(pAR->Length(&llSize, &llAvailable))) {
        size_t ret = MI.Open_Buffer_Init((MediaInfo_int64u)llSize);

        size_t szLength = (size_t)min(llSize, MEDIAINFO_BUFFER_SIZE);
        BYTE* pBuffer = DEBUG_NEW BYTE[szLength];
        LONGLONG llPosition = 0;
        while ((ret & 0x1) && !(ret & 0x2)) { // While accepted and not filled
            if (pAR->SyncRead(llPosition, (LONG)szLength, pBuffer) != S_OK) {
                break;
            }
            ret = MI.Open_Buffer_Continue(pBuffer, szLength);

            // Seek to a different position if needed
            MediaInfo_int64u uiNeeded = MI.Open_Buffer_Continue_GoTo_Get();
            if (uiNeeded != (MediaInfo_int64u) - 1 && (ULONGLONG)llPosition < uiNeeded) {
                llPosition = (LONGLONG)uiNeeded;
            } else {
                llPosition += szLength;
            }
        }
        MI.Open_Buffer_Finalize();
        delete pBuffer;

        MI_Text = MI.Inform().c_str();
    } else {
        MI.Open(f_name);
        MI_Text = MI.Inform().c_str();
        MI.Close();

        if (!MI_Text.Find(_T("Unable to load"))) {
            MI_Text = _T("");
        }
    }

    LOGFONT lf;
    ZeroMemory(&lf, sizeof(lf));
    lf.lfPitchAndFamily = DEFAULT_PITCH | FF_MODERN;
    // The empty string will fallback to the first font that matches the other specified attributes.
    LPCTSTR fonts[] = { _T("Lucida Console"), _T("Courier New"), _T("") };
    // Use a negative value to match the character height instead of the cell height.
    int fonts_size[] = { -10, -11, -11 };
    UINT i = 0;
    BOOL success;
    do {
        _tcscpy_s(lf.lfFaceName, fonts[i]);
        lf.lfHeight = fonts_size[i];
        success = IsFontInstalled(fonts[i]) && m_pCFont->CreateFontIndirect(&lf);
        i++;
    } while (!success && i < _countof(fonts));
    m_mediainfo.SetFont(m_pCFont);
    m_mediainfo.SetWindowText(MI_Text);

    // subclass the edit control
    OldControlProc = (WNDPROC)SetWindowLongPtr(m_mediainfo.m_hWnd, GWLP_WNDPROC, (LONG_PTR)ControlProc);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CPPageFileMediaInfo::OnShowWindow(BOOL bShow, UINT nStatus)
{
    __super::OnShowWindow(bShow, nStatus);
    if (bShow) {
        GetParent()->GetDlgItem(IDC_BUTTON_MI)->ShowWindow(SW_SHOW);
    } else {
        GetParent()->GetDlgItem(IDC_BUTTON_MI)->ShowWindow(SW_HIDE);
    }
}

#if !USE_STATIC_MEDIAINFO
bool CPPageFileMediaInfo::HasMediaInfo()
{
    MediaInfo MI;
    return MI.IsReady();
}
#endif
