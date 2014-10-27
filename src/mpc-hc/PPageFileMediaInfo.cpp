/*
 * (C) 2009-2014 see Authors.txt
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
CPPageFileMediaInfo::CPPageFileMediaInfo(CString path, IFileSourceFilter* pFSF)
    : CPropertyPage(CPPageFileMediaInfo::IDD, CPPageFileMediaInfo::IDD)
    , m_fn(path)
    , m_path(path)
{
    CComQIPtr<IAsyncReader> pAR;
    if (pFSF) {
        LPOLESTR pFN;
        if (SUCCEEDED(pFSF->GetCurFile(&pFN, nullptr))) {
            m_fn = pFN;
            CoTaskMemFree(pFN);
        }

        if (CComQIPtr<IBaseFilter> pBF = pFSF) {
            BeginEnumPins(pBF, pEP, pPin) {
                if (pAR = pPin) {
                    break;
                }
            }
            EndEnumPins;
        }
    }

    if (m_path.IsEmpty()) {
        m_path = m_fn;
    }

    m_futureMIText = std::async(std::launch::async, [ = ]() {
#if USE_STATIC_MEDIAINFO
        MediaInfoLib::String filename = m_path;
        MediaInfoLib::MediaInfo MI;
#else
        MediaInfoDLL::String filename = m_path;
        MediaInfo MI;
#endif

        MI.Option(_T("ParseSpeed"), _T("0.5"));
        MI.Option(_T("Complete"));
        MI.Option(_T("Language"), _T("  Config_Text_ColumnSize;30"));

        LONGLONG llSize, llAvailable;
        if (pAR && SUCCEEDED(pAR->Length(&llSize, &llAvailable))) {
            size_t ret = MI.Open_Buffer_Init((MediaInfo_int64u)llSize);

            std::vector<BYTE> buffer(MEDIAINFO_BUFFER_SIZE);
            LONGLONG llPosition = 0;
            while ((ret & 0x1) && !(ret & 0x8) && llPosition < llAvailable) { // While accepted and not finished
                size_t szLength = (size_t)std::min(llAvailable - llPosition, (LONGLONG)buffer.size());
                if (pAR->SyncRead(llPosition, (LONG)szLength, buffer.data()) != S_OK) {
                    break;
                }

                ret = MI.Open_Buffer_Continue(buffer.data(), szLength);

                // Seek to a different position if needed
                MediaInfo_int64u uiNeeded = MI.Open_Buffer_Continue_GoTo_Get();
                if (uiNeeded != MediaInfo_int64u(-1)) {
                    llPosition = (LONGLONG)uiNeeded;
                    // Inform MediaInfo of the seek
                    MI.Open_Buffer_Init((MediaInfo_int64u)llSize, (MediaInfo_int64u)llPosition);
                } else {
                    llPosition += (LONGLONG)szLength;
                }

                if (FAILED(pAR->Length(&llSize, &llAvailable))) {
                    break;
                }
            }
            MI.Open_Buffer_Finalize();
        } else {
            MI.Open(filename);
        }

        CString info = MI.Inform().c_str();

        if (info.IsEmpty() || !info.Find(_T("Unable to load"))) {
            info = ResStr(IDS_MEDIAINFO_NO_INFO_AVAILABLE);
        }

        return info;
    }).share();
}

CPPageFileMediaInfo::~CPPageFileMediaInfo()
{
}

void CPPageFileMediaInfo::DoDataExchange(CDataExchange* pDX)
{
    __super::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_MIEDIT, m_mediainfo);
}

BOOL CPPageFileMediaInfo::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN && pMsg->hwnd == m_mediainfo) {
        if (OnKeyDownInEdit(pMsg)) {
            return TRUE;
        }
    }

    return __super::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CPPageFileMediaInfo, CPropertyPage)
    ON_WM_SHOWWINDOW()
    ON_WM_DESTROY()
    ON_MESSAGE_VOID(WM_REFRESH_TEXT, OnRefreshText)
END_MESSAGE_MAP()

// CPPageFileMediaInfo message handlers

BOOL CPPageFileMediaInfo::OnInitDialog()
{
    __super::OnInitDialog();

    LOGFONT lf;
    ZeroMemory(&lf, sizeof(lf));
    lf.lfPitchAndFamily = DEFAULT_PITCH | FF_MODERN;
    // The empty string will fall back to the first font that matches the other specified attributes.
    LPCTSTR fonts[] = { _T("Lucida Console"), _T("Courier New"), _T("") };
    // Use a negative value to match the character height instead of the cell height.
    int fonts_size[] = { -10, -11, -11 };
    size_t i = 0;
    bool bSuccess;
    do {
        _tcscpy_s(lf.lfFaceName, fonts[i]);
        lf.lfHeight = fonts_size[i];
        bSuccess = IsFontInstalled(fonts[i]) && m_font.CreateFontIndirect(&lf);
        i++;
    } while (!bSuccess && i < _countof(fonts));
    m_mediainfo.SetFont(&m_font);

    m_mediainfo.SetWindowText(ResStr(IDS_MEDIAINFO_ANALYSIS_IN_PROGRESS));
    m_threadSetText = std::thread([this]() {
        m_futureMIText.wait(); // Wait for the info to be ready
        PostMessage(WM_REFRESH_TEXT); // then notify the window to set the text
    });

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CPPageFileMediaInfo::OnShowWindow(BOOL bShow, UINT nStatus)
{
    __super::OnShowWindow(bShow, nStatus);

    GetParent()->GetDlgItem(IDC_BUTTON_MI)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
}

void CPPageFileMediaInfo::OnDestroy()
{
    if (m_threadSetText.joinable()) {
        m_threadSetText.join();
    }
}

void CPPageFileMediaInfo::OnRefreshText()
{
    m_mediainfo.SetWindowText(m_futureMIText.get());
}

bool CPPageFileMediaInfo::OnKeyDownInEdit(MSG* pMsg)
{
    bool bHandled = false;

    if ((LOWORD(pMsg->wParam) == _T('A') || LOWORD(pMsg->wParam) == _T('a'))
            && (GetKeyState(VK_CONTROL) < 0)) {
        m_mediainfo.SetSel(0, -1, TRUE);
        bHandled = true;
    }

    return bHandled;
}

void CPPageFileMediaInfo::OnSaveAs()
{
    CString fn = m_fn;

    fn.TrimRight(_T('/'));
    int i = std::max(fn.ReverseFind(_T('\\')), fn.ReverseFind(_T('/')));
    if (i >= 0 && i < fn.GetLength() - 1) {
        fn = fn.Mid(i + 1);
    }
    fn.Append(_T(".MediaInfo.txt"));

    CFileDialog fileDlg(FALSE, _T("*.txt"), fn,
                        OFN_EXPLORER | OFN_ENABLESIZING | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR,
                        _T("Text Files (*.txt)|*.txt|All Files (*.*)|*.*||"), this, 0);

    if (fileDlg.DoModal() == IDOK) { // user has chosen a file
        CFile file;
        if (file.Open(fileDlg.GetPathName(), CFile::modeCreate | CFile::modeWrite)) {
            TCHAR bom = (TCHAR)0xFEFF;
            file.Write(&bom, sizeof(TCHAR));
            file.Write(LPCTSTR(m_futureMIText.get()), m_futureMIText.get().GetLength() * sizeof(TCHAR));
            file.Close();
        }
    }
}
