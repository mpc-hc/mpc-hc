/*
* (C) 2015 see Authors.txt
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

#include "CrashReporterDialog.h"

#include "DoctorDump/CrashRpt.h"
#include "../resource.h"
#include <Psapi.h>
#include <sstream>


using namespace crash_rpt::custom_data_collection;

Result CALLBACK CreateCrashDialog(const ExceptionInfo& exceptionInfo, IDataBag* dataBag)
{
    Result res = Result::CancelUpload;

    TCHAR exePath[MAX_PATH];
    if (!GetModuleFileNameEx(exceptionInfo.Process, nullptr, exePath, _countof(exePath))) {
#ifdef _WIN64
        _tcscpy_s(exePath, _T("..\\mpc-hc64.exe"));
#else
        _tcscpy_s(exePath, _T("..\\mpc-hc.exe"));
#endif
    }

    CCrashReporterDialog dlg(exePath, (LPCTSTR)exceptionInfo.UserData);

    if (dlg.DoModal() != IDCANCEL && dlg.GetSendCrashReport()) {
        res = Result::DoUpload;
        if (LPCTSTR email = dlg.GetEmail()) {
            dataBag->AddUserInfoToReport(L"email", email);
        }
        if (LPCTSTR description = dlg.GetDescription()) {
            dataBag->AddUserInfoToReport(L"description", description);
        }
    }

    return res;
}

CCrashReporterDialog::CCrashReporterDialog(LPCTSTR exePath, LPCTSTR langDll /*= nullptr*/)
    : m_exePath(exePath)
{
    m_hModuleExe = LoadLibrary(exePath);

    if (langDll) {
        std::wostringstream langDllPath;
        langDllPath << _T("..\\") << langDll;
        m_hModuleRes = LoadLibrary(langDllPath.str().c_str());
    }
}

CCrashReporterDialog::~CCrashReporterDialog()
{
    FreeLibrary(m_hModuleExe);
    FreeLibrary(m_hModuleRes);
}

INT_PTR CCrashReporterDialog::DoModal()
{
    return DialogBoxParam(m_hModuleRes ? m_hModuleRes : m_hModuleExe, MAKEINTRESOURCE(IDD_CRASH_REPORTER),
                          GetDesktopWindow(), DialogProc, (LPARAM)this);
}

INT_PTR CALLBACK CCrashReporterDialog::DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    INT_PTR res = FALSE;

    switch (uMsg) {
        case WM_INITDIALOG: {
            // Attach the window handle to the object
            CCrashReporterDialog* dlg = (CCrashReporterDialog*)lParam;
            dlg->m_hWnd = hwndDlg;
            SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
            dlg->OnInitDialog();
            res = TRUE;
        }
        break;
        case WM_COMMAND: {
            CCrashReporterDialog* dlg = (CCrashReporterDialog*)GetWindowLongPtr(hwndDlg, DWLP_USER);

            switch (LOWORD(wParam)) {
                case IDC_CHECK1:
                    if (HIWORD(wParam) == BN_CLICKED) {
                        dlg->OnCheckBoxClicked();
                    }
                    break;
                case IDC_BUTTON1: // Restart
                    ShellExecute(nullptr, _T("open"), dlg->m_exePath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
                // No break
                case IDC_BUTTON2: // Quit
                    dlg->OnOK();
                // No break
                case IDCANCEL: // Escape
                    EndDialog(hwndDlg, wParam);
                    res = TRUE;
                    break;
            }
        }
        break;
    }

    return res;
}

void CCrashReporterDialog::OnInitDialog()
{
    // Set the icon
    HICON hIcon = (HICON)LoadImage(m_hModuleExe, MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON,
                                   GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED);
    if (hIcon) { // Since LR_SHARED is used, there is no need to explicitly destroy the icon
        SendMessage(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    }

    // Center the dialog window
    RECT rcDlg;
    GetWindowRect(m_hWnd, &rcDlg);
    MONITORINFO monitorInfo;
    monitorInfo.cbSize = sizeof(monitorInfo);
    GetMonitorInfo(MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTONEAREST), &monitorInfo);
    SIZE offset;
    offset.cx = ((monitorInfo.rcWork.right - monitorInfo.rcWork.left) - (rcDlg.right - rcDlg.left)) / 2;
    offset.cy = ((monitorInfo.rcWork.bottom - monitorInfo.rcWork.top) - (rcDlg.bottom - rcDlg.top)) / 2;
    SetWindowPos(m_hWnd, nullptr,
                 monitorInfo.rcWork.left + offset.cx, monitorInfo.rcWork.top + offset.cy,
                 0, 0,
                 SWP_NOSIZE | SWP_NOZORDER);

    // Check "send a bug report" by default
    SendDlgItemMessage(m_hWnd, IDC_CHECK1, BM_SETCHECK, BST_CHECKED, 0);
}

void CCrashReporterDialog::OnOK()
{
    auto getText = [this](int nID, std::vector<TCHAR>& str) {
        HWND hwndField = GetDlgItem(m_hWnd, nID);
        if (hwndField) {
            int nLenght = GetWindowTextLength(hwndField);
            if (nLenght > 0) {
                nLenght++; // Account for the null char
                str.resize(nLenght);
                if (!GetWindowText(hwndField, str.data(), nLenght)) {
                    str[0] = _T('\0');
                }
            }
        }
        return str;
    };

    m_bSendCrashReport = (SendDlgItemMessage(m_hWnd, IDC_CHECK1, BM_GETCHECK, 0, 0) == BST_CHECKED);
    getText(IDC_EDIT1, m_email);
    getText(IDC_EDIT2, m_description);
}

void CCrashReporterDialog::OnCheckBoxClicked()
{
    auto enableDlgItem = [this](int nID, bool bEnable) {
        EnableWindow(GetDlgItem(m_hWnd, nID), bEnable);
    };

    bool bEnable = (SendDlgItemMessage(m_hWnd, IDC_CHECK1, BM_GETCHECK, 0, 0) == BST_CHECKED);
    enableDlgItem(IDC_STATIC1, bEnable);
    enableDlgItem(IDC_STATIC2, bEnable);
    enableDlgItem(IDC_EDIT1,   bEnable);
    enableDlgItem(IDC_STATIC3, bEnable);
    enableDlgItem(IDC_STATIC4, bEnable);
    enableDlgItem(IDC_EDIT2,   bEnable);
}
