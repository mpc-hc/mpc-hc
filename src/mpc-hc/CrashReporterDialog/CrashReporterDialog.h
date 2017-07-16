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

#include <Windows.h>
#include <tchar.h>
#include <string>
#include <vector>


class CCrashReporterDialog
{
private:
    HWND m_hWnd = nullptr;
    std::wstring m_exePath;
    HMODULE m_hModuleExe = nullptr;
    HMODULE m_hModuleRes = nullptr;
    bool m_bSendCrashReport = true;
    std::vector<TCHAR> m_email;
    std::vector<TCHAR> m_description;

    static INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void OnInitDialog();
    void OnOK();
    void OnCheckBoxClicked();

public:
    CCrashReporterDialog(LPCTSTR exePath, LPCTSTR langDll = nullptr);
    ~CCrashReporterDialog();

    INT_PTR DoModal();

    bool GetSendCrashReport() const { return m_bSendCrashReport; };
    LPCTSTR GetEmail() const { return m_email.data(); };
    LPCTSTR GetDescription() const { return m_description.data(); };
};
