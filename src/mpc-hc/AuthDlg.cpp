/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014 see Authors.txt
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
#include "AuthDlg.h"
#include "SysVersion.h"
#include "version.h"

// We need to dynamically link to the functions provided by CredUI.lib in order
// to be able to use the features available to the OS.
#include <WinCred.h>
#include "WinApiFunc.h"

HRESULT PromptForCredentials(HWND hWnd, const CString& strCaptionText, const CString& strMessageText, CString& strDomain, CString& strUsername, CString& strPassword, BOOL* bSave)
{
    CREDUI_INFO info = { sizeof(info) };
    info.hwndParent = hWnd;
    info.pszCaptionText = strCaptionText.Left(CREDUI_MAX_CAPTION_LENGTH);
    info.pszMessageText = strMessageText.Left(CREDUI_MAX_MESSAGE_LENGTH);

    DWORD dwUsername = CREDUI_MAX_USERNAME_LENGTH + 1;
    DWORD dwPassword = CREDUI_MAX_PASSWORD_LENGTH + 1;
    DWORD dwDomain = CREDUI_MAX_GENERIC_TARGET_LENGTH + 1;

    if (SysVersion::IsVistaOrLater()) {
        // Define CredUI.dll functions for Windows Vista+
        const WinapiFunc<BOOL(DWORD, LPWSTR, LPWSTR, PBYTE, DWORD*)>
        fnCredPackAuthenticationBufferW = { "CREDUI.DLL", "CredPackAuthenticationBufferW" };

        const WinapiFunc<DWORD(PCREDUI_INFOW, DWORD, ULONG*, LPCVOID, ULONG, LPVOID*, ULONG*, BOOL*, DWORD)>
        fnCredUIPromptForWindowsCredentialsW = { "CREDUI.DLL", "CredUIPromptForWindowsCredentialsW" };

        const WinapiFunc<BOOL(DWORD, PVOID, DWORD, LPWSTR, DWORD*, LPWSTR, DWORD*, LPWSTR, DWORD*)>
        fnCredUnPackAuthenticationBufferW = { "CREDUI.DLL", "CredUnPackAuthenticationBufferW" };

        if (fnCredPackAuthenticationBufferW && fnCredUIPromptForWindowsCredentialsW && fnCredUnPackAuthenticationBufferW) {
            PVOID pvInAuthBlob = nullptr;
            ULONG cbInAuthBlob = 0;
            PVOID pvAuthBlob = nullptr;
            ULONG cbAuthBlob = 0;
            ULONG ulAuthPackage = 0;

            // Call CredPackAuthenticationBufferW once to determine the size, in bytes, of the authentication buffer.
            if (strUsername.GetLength()) {
                BOOL bResult = fnCredPackAuthenticationBufferW(0, (LPTSTR)(LPCTSTR)strUsername, (LPTSTR)(LPCTSTR)strPassword, nullptr, &cbInAuthBlob);
                if (!bResult && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                    if ((pvInAuthBlob = CoTaskMemAlloc(cbInAuthBlob)) != nullptr) {
                        VERIFY(fnCredPackAuthenticationBufferW(0, (LPTSTR)(LPCTSTR)strUsername, (LPTSTR)(LPCTSTR)strPassword, (PBYTE)pvInAuthBlob, &cbInAuthBlob));
                    }
                }
            }
            const DWORD dwFlags = CREDUIWIN_GENERIC | CREDUIWIN_ENUMERATE_CURRENT_USER | (bSave ? CREDUIWIN_CHECKBOX : 0);
            DWORD dwResult = fnCredUIPromptForWindowsCredentialsW(&info, 0, &ulAuthPackage, pvInAuthBlob, cbInAuthBlob, &pvAuthBlob, &cbAuthBlob, bSave, dwFlags);
            if (dwResult == ERROR_SUCCESS) {
                VERIFY(fnCredUnPackAuthenticationBufferW(0, pvAuthBlob, cbAuthBlob, strUsername.GetBufferSetLength(dwUsername), &dwUsername, strDomain.GetBufferSetLength(dwDomain), &dwDomain, strPassword.GetBufferSetLength(dwPassword), &dwPassword));
                strUsername.ReleaseBuffer();
                strPassword.ReleaseBuffer();
                strDomain.ReleaseBuffer();
            }

            // Delete the input authentication byte array.
            if (pvInAuthBlob) {
                SecureZeroMemory(pvInAuthBlob, cbInAuthBlob);
                CoTaskMemFree(pvInAuthBlob);
                pvInAuthBlob = nullptr;
            }
            // Delete the output authentication byte array.
            if (pvAuthBlob) {
                SecureZeroMemory(pvAuthBlob, cbAuthBlob);
                CoTaskMemFree(pvAuthBlob);
                pvAuthBlob = nullptr;
            }
            return dwResult; // ERROR_SUCCESS / ERROR_CANCELLED
        }
    } else if (SysVersion::IsXPOrLater()) {
        // Define CredUI.dll functions for Windows XP
        const WinapiFunc<DWORD(PCREDUI_INFOW, PCWSTR, PCtxtHandle, DWORD, PCWSTR, ULONG, PCWSTR, ULONG, PBOOL, DWORD)>
        fnCredUIPromptForCredentialsW = { "CREDUI.DLL", "CredUIPromptForCredentialsW" };

        const WinapiFunc<DWORD(PCWSTR, PWSTR, ULONG, PWSTR, ULONG)>
        fnCredUIParseUserNameW = { "CREDUI.DLL", "CredUIParseUserNameW" };

        if (fnCredUIPromptForCredentialsW && fnCredUIParseUserNameW) {
            const DWORD dwAuthError = 0;
            const DWORD dwFlags = CREDUI_FLAGS_ALWAYS_SHOW_UI | CREDUI_FLAGS_GENERIC_CREDENTIALS/* | CREDUI_FLAGS_EXPECT_CONFIRMATION*/ | CREDUI_FLAGS_COMPLETE_USERNAME | CREDUI_FLAGS_DO_NOT_PERSIST | (bSave ? CREDUI_FLAGS_SHOW_SAVE_CHECK_BOX : 0);
            CString strUserDomain(strUsername);
            if (!strDomain.GetLength()) {
                strDomain = _T("mpc-hc/") MPC_VERSION_STR;
            }

            DWORD dwResult = fnCredUIPromptForCredentialsW(&info, strDomain.Left(dwDomain), nullptr, dwAuthError,
                             strUserDomain.GetBufferSetLength(dwUsername), dwUsername, strPassword.GetBufferSetLength(dwPassword), dwPassword, bSave, dwFlags);
            strUserDomain.ReleaseBuffer();
            strPassword.ReleaseBuffer();

            fnCredUIParseUserNameW(strUserDomain, strUsername.GetBufferSetLength(dwUsername), dwUsername, strDomain.GetBufferSetLength(dwDomain), dwDomain);
            strUsername.ReleaseBuffer();
            strDomain.ReleaseBuffer();
            //dwResult = CredUIConfirmCredentials(szDomain.Left(cchDomain), TRUE);

            if (strDomain == _T("mpc-hc/") MPC_VERSION_STR) {
                strDomain.Empty();
            }

            return dwResult; // ERROR_SUCCESS / ERROR_CANCELLED
        }
    }
    return ERROR_CALL_NOT_IMPLEMENTED;
}
