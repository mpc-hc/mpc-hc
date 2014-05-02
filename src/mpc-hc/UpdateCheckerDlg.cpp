/*
 * (C) 2012-2014 see Authors.txt
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
#include "UpdateCheckerDlg.h"
#include "mpc-hc_config.h"
#include <afxdialogex.h>

#include <Softpub.h>
#include <wincrypt.h>
#include <wintrust.h>

// Link with the Wintrust.lib file.
#pragma comment (lib, "wintrust")

BOOL VerifyEmbeddedSignature(LPCWSTR pwszSourceFile)
{
    BOOL bRet = FALSE;

    // Initialize the WINTRUST_FILE_INFO structure.
    WINTRUST_FILE_INFO pFile;
    memset(&pFile, 0, sizeof(pFile));
    pFile.cbStruct = sizeof(WINTRUST_FILE_INFO);
    pFile.pcwszFilePath = pwszSourceFile;
    pFile.hFile = NULL;
    pFile.pgKnownSubject = NULL;

    GUID WVTPolicyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;

    // Initialize the WinVerifyTrust input data structure.
    // Default all fields to 0.
    WINTRUST_DATA pWinTrustData;
    memset(&pWinTrustData, 0, sizeof(pWinTrustData));

    pWinTrustData.cbStruct = sizeof(pWinTrustData);
    pWinTrustData.pPolicyCallbackData = NULL;
    pWinTrustData.pSIPClientData = NULL;
    pWinTrustData.dwUIChoice = WTD_UI_NONE;
    pWinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
    pWinTrustData.dwUnionChoice = WTD_CHOICE_FILE;
    pWinTrustData.pFile = &pFile;
    pWinTrustData.dwStateAction = WTD_STATEACTION_VERIFY;
    pWinTrustData.hWVTStateData = NULL;
    pWinTrustData.pwszURLReference = NULL;
    pWinTrustData.dwUIContext = 0;
    pWinTrustData.dwProvFlags = WTD_SAFER_FLAG;


    // WinVerifyTrust verifies signatures as specified by the GUID and Wintrust_Data.
    LONG lStatus = WinVerifyTrust(NULL, &WVTPolicyGUID, &pWinTrustData);
    bRet = (lStatus == ERROR_SUCCESS) ? TRUE : FALSE;

    // Any hWVTStateData must be released by a call with close.
    pWinTrustData.dwStateAction = WTD_STATEACTION_CLOSE;
    lStatus = WinVerifyTrust(NULL, &WVTPolicyGUID, &pWinTrustData);

    return bRet;
}

IMPLEMENT_DYNAMIC(UpdateCheckerDlg, CDialog)

UpdateCheckerDlg::UpdateCheckerDlg(Update_Status updateStatus, UpdateChecker *updateChecker, CWnd* pParent /*=nullptr*/)
    : CDialog(UpdateCheckerDlg::IDD, pParent)
    , m_updateChecker(updateChecker)
    , m_updateStatus(updateStatus)
    , m_pDownloadThread(nullptr)
{
    switch (updateStatus) {
        case UPDATER_UPDATE_AVAILABLE:
        case UPDATER_UPDATE_AVAILABLE_IGNORED:
            m_text.Format(IDS_NEW_UPDATE_AVAILABLE,
                          m_updateChecker->GetLatestVersion().ToString(),
                          UpdateChecker::MPC_HC_VERSION.ToString());
            break;
        case UPDATER_LATEST_STABLE:
            m_text.LoadString(IDS_USING_LATEST_STABLE);
            break;
        case UPDATER_NEWER_VERSION:
            m_text.Format(IDS_USING_NEWER_VERSION,
                          UpdateChecker::MPC_HC_VERSION.ToString(),
                          m_updateChecker->GetLatestVersion().ToString());
            break;
        case UPDATER_ERROR:
            m_text.LoadString(IDS_UPDATE_ERROR);
            break;
        default:
            ASSERT(0); // should never happen
    }
}

UpdateCheckerDlg::~UpdateCheckerDlg()
{
    if (m_pDownloadThread)
        delete m_pDownloadThread;
}

void UpdateCheckerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_UPDATE_DLG_TEXT, m_text);
    DDX_Control(pDX, IDC_UPDATE_ICON, m_icon);
    DDX_Control(pDX, IDC_UPDATE_DL_BUTTON, m_dlButton);
    DDX_Control(pDX, IDC_UPDATE_LATER_BUTTON, m_laterButton);
    DDX_Control(pDX, IDC_UPDATE_IGNORE_BUTTON, m_ignoreButton);
    DDX_Control(pDX, IDC_UPDATE_CANCEL, m_cancelButton);
    DDX_Control(pDX, IDC_PROGRESS1, m_progress);
}


BEGIN_MESSAGE_MAP(UpdateCheckerDlg, CDialog)
    ON_BN_CLICKED(IDC_UPDATE_DL_BUTTON, OnOpenDownloadPage)
    ON_BN_CLICKED(IDC_UPDATE_LATER_BUTTON, OnUpdateLater)
    ON_BN_CLICKED(IDC_UPDATE_IGNORE_BUTTON, OnIgnoreUpdate)
    ON_BN_CLICKED(IDC_UPDATE_CANCEL, OnCancelUpdate)
END_MESSAGE_MAP()

BOOL UpdateCheckerDlg::OnInitDialog()
{
    BOOL ret = __super::OnInitDialog();

    switch (m_updateStatus) {
        case UPDATER_UPDATE_AVAILABLE:
        case UPDATER_UPDATE_AVAILABLE_IGNORED:
            m_icon.SetIcon(LoadIcon(nullptr, IDI_QUESTION));
            m_progress.ShowWindow(SW_SHOW);

            break;
        case UPDATER_LATEST_STABLE:
        case UPDATER_NEWER_VERSION:
        case UPDATER_ERROR: {
            m_icon.SetIcon(LoadIcon(nullptr, (m_updateStatus == UPDATER_ERROR) ? IDI_WARNING : IDI_INFORMATION));
            m_dlButton.ShowWindow(SW_HIDE);
            m_laterButton.ShowWindow(SW_HIDE);
            m_ignoreButton.SetWindowText(ResStr(IDS_UPDATE_CLOSE));

            CRect buttonRect;
            m_laterButton.GetWindowRect(&buttonRect);
            ScreenToClient(&buttonRect);
            m_ignoreButton.MoveWindow(&buttonRect);

            // Change the default button
            SetDefID(IDC_UPDATE_IGNORE_BUTTON);
            ret = FALSE; // Focus has been set explicitly
        }
        break;
        default:
            ASSERT(0); // should never happen
    }

    return ret;
}

void UpdateCheckerDlg::OnOpenDownloadPage()
{
    //ShellExecute(nullptr, _T("open"), DOWNLOAD_URL, nullptr, nullptr, SW_SHOWNORMAL);
    m_text.Format(_T("Connecting to sourceforge.net, please wait..."));
    SetDlgItemText(IDC_UPDATE_DLG_TEXT, m_text);
    m_progress.ShowWindow(TRUE);
    m_cancelButton.ShowWindow(TRUE);
    m_dlButton.ShowWindow(FALSE);
    m_ignoreButton.ShowWindow(FALSE);
    m_laterButton.ShowWindow(FALSE);
    if (!m_pDownloadThread)
        m_pDownloadThread = new CDownloadThread(m_updateChecker, this);
    m_pDownloadThread->CreateThread();
}

void UpdateCheckerDlg::OnCancelUpdate()
{
    if (m_pDownloadThread) {
        m_pDownloadThread->AbortThread();
    }
}

void UpdateCheckerDlg::OnUpdateLater()
{
    EndDialog(IDC_UPDATE_LATER_BUTTON);
}

void UpdateCheckerDlg::OnIgnoreUpdate()
{
    if (m_updateStatus == UPDATER_UPDATE_AVAILABLE)
        m_updateChecker->IgnoreLatestVersion();
    EndDialog((m_updateStatus == UPDATER_UPDATE_AVAILABLE) ? IDC_UPDATE_IGNORE_BUTTON : 0);
}

void UpdateCheckerDlg::DownloadStarted(double total)
{
    m_icon.SetIcon(LoadIcon(nullptr, IDI_APPLICATION));
    m_text.Format(_T("Downloading v%s...\n\nProgress: %.2f mb of %.2f mb"), m_updateChecker->GetLatestVersion().ToString(), (double)0.0, (double)(total / 1000));
    SetDlgItemText(IDC_UPDATE_DLG_TEXT, m_text);
    m_progress.SetRange32(0, (int)total);
    m_progress.SetPos(0);
    m_progress.ShowWindow(SW_SHOWNORMAL);
}

void UpdateCheckerDlg::DownloadProgress(double done, double total, double speed)
{
    m_text.Format(_T("Downloading v%s...\n\nProgress: %.2f mb of %.2f mb (%.2f kb/s)"), m_updateChecker->GetLatestVersion().ToString(), (double)(done / 1000), (double)(total / 1000), speed);
    SetDlgItemText(IDC_UPDATE_DLG_TEXT, m_text);
    m_progress.SetPos((int)done);
}

void UpdateCheckerDlg::DownloadFinished(double total, const CString &file)
{
    m_text.Format(_T("Finished downloading v%s.\n\nRunning installer..."), m_updateChecker->GetLatestVersion().ToString());
    SetDlgItemText(IDC_UPDATE_DLG_TEXT, m_text);
    m_progress.SetPos((int)total);
    m_cancelButton.EnableWindow(FALSE);

    SHELLEXECUTEINFO shExecInfo = { 0 };
    shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    shExecInfo.fMask = SEE_MASK_CLASSNAME;
    shExecInfo.lpClass = _T("exefile");
    shExecInfo.lpVerb = _T("open");
    shExecInfo.lpFile = file;
    shExecInfo.nShow = SW_SHOWNORMAL;
    //shExecInfo.lpParameters = _T("/VERYSILENT /SP- /NORESTART /SUPPRESSMSGBOXES /RESTARTAPPLICATIONS");
    ShellExecuteEx(&shExecInfo);

    Sleep(1000);
    PostMessage(WM_CLOSE, 0, 0);
    AfxGetMainWnd()->PostMessage(WM_CLOSE, 0, 0);
}

void UpdateCheckerDlg::DownloadAborted()
{
    m_text.Format(_T("Downloading v%s, aborted..."), m_updateChecker->GetLatestVersion().ToString());
    SetDlgItemText(IDC_UPDATE_DLG_TEXT, m_text);
    m_progress.SetPos(0);
    m_cancelButton.EnableWindow(FALSE);
    Sleep(1000);
    PostMessage(WM_CLOSE, 0, 0);
}

void UpdateCheckerDlg::DownloadFailed()
{
    m_text.Format(_T("Downloading v%s, failed..."), m_updateChecker->GetLatestVersion().ToString());
    SetDlgItemText(IDC_UPDATE_DLG_TEXT, m_text);
    m_progress.SetPos(0);
    m_progress.ShowWindow(SW_HIDE);
    m_cancelButton.ShowWindow(SW_HIDE);
    m_dlButton.ShowWindow(SW_SHOW);
    m_laterButton.ShowWindow(SW_SHOW);
    m_ignoreButton.ShowWindow(SW_SHOW);
}

void CDownloadThread::ThreadProc() {
#ifdef _WIN64
    CStringA strDownloadLink = "http://sourceforge.net/projects/mpc-hc/files/MPC HomeCinema - x64/MPC-HC_v%s_x64/MPC-HC.%s.x64.exe/download";
#else
    CStringA strDownloadLink = "http://sourceforge.net/projects/mpc-hc/files/MPC HomeCinema - Win32/MPC-HC_v%s_x86/MPC-HC.%s.x86.exe/download";
#endif
    CStringA strVersion = m_updateChecker->GetLatestVersion().ToString();

    CStringA strLink;
    strLink.Format(strDownloadLink, strVersion, strVersion);

    try {
        CInternetSession is;

        is.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 5000 /*default=60000*/);
        CAutoPtr<CHttpFile> pHttpFile((CHttpFile*)is.OpenURL(CString(UrlEncode(strLink)), 1, INTERNET_FLAG_NEED_FILE | INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_RELOAD | INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_DONT_CACHE, NULL, NULL));

        if (pHttpFile) {
            TCHAR path[MAX_PATH], file[MAX_PATH];
            GetTempPath(MAX_PATH, path);
            UINT unique(GetTempFileName(path, _T("mpc"), 0, file));
            CFile pFile;
            pFile.Open(file, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareDenyNone);

            DWORD total_length(0), length(0), download_length(0), index(0);
            while (pHttpFile->QueryInfo(HTTP_QUERY_CONTENT_LENGTH, length, &index)) {
                total_length += length;
            }

            total_length = std::max((size_t)total_length, (size_t)pHttpFile->GetLength());
            if (IsWindow(*m_pUpdateCheckerDlg))
                m_pUpdateCheckerDlg->DownloadStarted(total_length / 1024);

            std::vector<char> buff(1024);
            LARGE_INTEGER ticks_per_sec = { 0 };
            QueryPerformanceFrequency(&ticks_per_sec);
            LARGE_INTEGER iStartTime = { 0 };
            QueryPerformanceCounter(&iStartTime);
            LARGE_INTEGER iInterval = iStartTime;
            for (int len(0); (!IsThreadAborting()) && ((len = pHttpFile->Read(&buff[0], (UINT)buff.size())) > 0);) {
                //Write to file
                pFile.Write(&buff[0], len);
                download_length += len;

                LARGE_INTEGER iEndTime = { 0 };
                QueryPerformanceCounter(&iEndTime);

                if ((iEndTime.QuadPart - (ticks_per_sec.QuadPart / 4)) > iInterval.QuadPart) {
                    iInterval = iEndTime;
                    double dblDurationTime = ((double)(iEndTime.QuadPart - iStartTime.QuadPart) / (double)ticks_per_sec.QuadPart);
                    double kb_per_sec = (((double)download_length / 1024) / (dblDurationTime));

                    if (IsWindow(*m_pUpdateCheckerDlg))
                        m_pUpdateCheckerDlg->DownloadProgress(download_length / 1024, total_length / 1024, kb_per_sec);
                }
            }
            pHttpFile->Close(); // must close it because the destructor doesn't seem to do it and we will get an exception when "is" is destroying
            pFile.Close();

            if (VerifyEmbeddedSignature(file) == FALSE) {
                if (IsWindow(*m_pUpdateCheckerDlg))  {
                    m_pUpdateCheckerDlg->DownloadFailed();
                }
            } else if (IsWindow(*m_pUpdateCheckerDlg)) {
                if (IsThreadAborting()) {
                    m_pUpdateCheckerDlg->DownloadAborted();
                } else {
                    m_pUpdateCheckerDlg->DownloadFinished(total_length / 1024, file);
                }
            }
        }
    } catch (CInternetException* ie) {
        ie->Delete();
        if (IsWindow(*m_pUpdateCheckerDlg))  {
            m_pUpdateCheckerDlg->DownloadFailed();
        }
    } catch (CFileException* fe) {
        fe->Delete();
        if (IsWindow(*m_pUpdateCheckerDlg))  {
            m_pUpdateCheckerDlg->DownloadFailed();
        }
    }
}


BOOL UpdateCheckerDlg::DestroyWindow()
{
    if (m_pDownloadThread) {
        m_pDownloadThread->AbortThread();
    }

    return CDialog::DestroyWindow();
}
