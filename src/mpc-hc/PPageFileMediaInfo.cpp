/*
 * (C) 2009-2017 see Authors.txt
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
#include "MainFrm.h"
#include "PPageFileMediaInfo.h"
#include "WinAPIUtils.h"

#include "MediaInfo/MediaInfoDLL.h"
using namespace MediaInfoDLL;

#define MEDIAINFO_BUFFER_SIZE 1024 * 256

// CPPageFileMediaInfo dialog

IMPLEMENT_DYNAMIC(CPPageFileMediaInfo, CMPCThemePropertyPage)
CPPageFileMediaInfo::CPPageFileMediaInfo(CString path, IFileSourceFilter* pFSF, IDvdInfo2* pDVDI, CMainFrame* pMainFrame)
    : CMPCThemePropertyPage(CPPageFileMediaInfo::IDD, CPPageFileMediaInfo::IDD)
    , m_fn(path)
    , m_path(path)
    , m_bSyncAnalysis(false)
{
    CComQIPtr<IAsyncReader> pAR;
    if (pFSF) {
        CComHeapPtr<OLECHAR> pFN;
        if (SUCCEEDED(pFSF->GetCurFile(&pFN, nullptr))) {
            m_fn = pFN;
        }

        if (CComQIPtr<IBaseFilter> pBF = pFSF) {
            BeginEnumPins(pBF, pEP, pPin) {
                if (pAR = pPin) {
                    break;
                }
            }
            EndEnumPins;
        }
    } else if (pDVDI) {
        ULONG len = 0;
        if (SUCCEEDED(pDVDI->GetDVDDirectory(m_path.GetBufferSetLength(MAX_PATH), MAX_PATH, &len)) && len) {
            m_path.ReleaseBuffer();
            m_fn = m_path += _T("\\VIDEO_TS.IFO");
        }
    }

    if (m_path.IsEmpty()) {
        m_path = m_fn;
    }

    if (m_path.GetLength() > 1 && m_path[1] == _T(':')
            && GetDriveType(m_path.Left(2) + _T('\\')) == DRIVE_CDROM) {
        // If we are playing from an optical drive, we do the analysis synchronously
        // when the user chooses to display the MediaInfo tab. We keep a reference
        // on the async reader filter but it will not cause any issue even if the
        // filter graph is destroyed before the analysis.
        m_bSyncAnalysis = true;
    }

    m_futureMIText = std::async(m_bSyncAnalysis ? std::launch::deferred : std::launch::async, [ = ]() {
        MediaInfoDLL::String filename = m_path;
        MediaInfo MI;
        // If we do a synchronous analysis on an optical drive, we pause the video during
        // the analysis to avoid concurrent accesses to the drive. Note that due to the
        // synchronous nature of the analysis, we are sure that the graph state will not
        // change during the analysis.
        bool bUnpause = false;
        if (m_bSyncAnalysis && pMainFrame->GetMediaState() == State_Running) {
            pMainFrame->SendMessage(WM_COMMAND, ID_PLAY_PAUSE);
            bUnpause = true;
        }

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

        if (bUnpause) {
            pMainFrame->SendMessage(WM_COMMAND, ID_PLAY_PLAY);
        }

        CString info = MI.Inform().c_str();

        if (info.IsEmpty() || !info.Find(_T("Unable to load"))) {
            info.LoadString(IDS_MEDIAINFO_NO_INFO_AVAILABLE);
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

BEGIN_MESSAGE_MAP(CPPageFileMediaInfo, CMPCThemePropertyPage)
    ON_WM_SHOWWINDOW()
    ON_WM_DESTROY()
    ON_MESSAGE_VOID(WM_MEDIAINFO_READY, OnMediaInfoReady)
END_MESSAGE_MAP()

// CPPageFileMediaInfo message handlers

BOOL CPPageFileMediaInfo::OnInitDialog()
{
    __super::OnInitDialog();

    LOGFONT lf;
    ZeroMemory(&lf, sizeof(lf));
    lf.lfPitchAndFamily = DEFAULT_PITCH | FF_MODERN;
    // The empty string will fall back to the first font that matches the other specified attributes.
    LPCTSTR fonts[] = { _T("Consolas"), _T("Lucida Console"), _T("Courier New"), _T("") };
    // Use a negative value to match the character height instead of the cell height.
    const int fonts_size[] = { 12, 12, 13, 13 };
    size_t i;
    if (AfxGetAppSettings().bMPCThemeLoaded) {
        i = 0; //added Consolas to the beginning for CMPCTheme
    } else {
        i = 1; //otherwise honor the old order (overly respectful of Lucida for old windows :) )
    }

    bool bSuccess;
    DpiHelper dpi;
    do {
        _tcscpy_s(lf.lfFaceName, fonts[i]);
        lf.lfHeight = -dpi.ScaleY(fonts_size[i]);
        bSuccess = IsFontInstalled(fonts[i]) && m_font.CreateFontIndirect(&lf);
        i++;
    } while (!bSuccess && i < _countof(fonts));
    m_mediainfo.SetFont(&m_font);

    GetParent()->GetDlgItem(IDC_BUTTON_MI)->EnableWindow(FALSE); // Initially disable the "Save As" button

    if (m_bSyncAnalysis) { // Wait until the analysis is finished
        OnMediaInfoReady();
    } else { // Spawn a thread that will asynchronously update the window
        m_mediainfo.SetWindowText(ResStr(IDS_MEDIAINFO_ANALYSIS_IN_PROGRESS));
        m_threadSetText = std::thread([this]() {
            m_futureMIText.wait(); // Wait for the info to be ready
            PostMessage(WM_MEDIAINFO_READY); // then notify the window that MediaInfo analysis finished
        });
    }

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

void CPPageFileMediaInfo::OnMediaInfoReady()
{
    if (m_futureMIText.get() != ResStr(IDS_MEDIAINFO_NO_INFO_AVAILABLE)) {
        GetParent()->GetDlgItem(IDC_BUTTON_MI)->EnableWindow(TRUE);
    }
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

bool CPPageFileMediaInfo::HasMediaInfo()
{
    MediaInfo MI;
    return MI.IsReady();
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
