/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2013 see Authors.txt
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
#include "AboutDlg.h"
#include <Tlhelp32.h>
#include "MainFrm.h"
#include "DSUtil.h"
#include "FileVersionInfo.h"
#include "Struct.h"
#include <winternl.h>
#include <psapi.h>
#include "Ifo.h"
#include "Monitors.h"
#include "WinAPIUtils.h"
#include "FileAssoc.h"
#include "UpdateChecker.h"
#include "winddk/ntddcdvd.h"
#include "detours/detours.h"
#include <afxsock.h>
#include <atlsync.h>
#include <atlutil.h>
#include <atlrx.h>
#include <share.h>
#include "mpc-hc_config.h"


const LanguageResource CMPlayerCApp::languageResources[] = {
    {ID_LANGUAGE_ARMENIAN,              1067,   _T("Armenian"),                 _T("Lang\\mpcresources.hy.dll")},
    {ID_LANGUAGE_BASQUE,                1069,   _T("Basque"),                   _T("Lang\\mpcresources.eu.dll")},
    {ID_LANGUAGE_BELARUSIAN,            1059,   _T("Belarusian"),               _T("Lang\\mpcresources.by.dll")},
    {ID_LANGUAGE_CATALAN,               1027,   _T("Catalan"),                  _T("Lang\\mpcresources.ca.dll")},
    {ID_LANGUAGE_CHINESE_SIMPLIFIED,    2052,   _T("Chinese (Simplified)"),     _T("Lang\\mpcresources.sc.dll")},
    {ID_LANGUAGE_CHINESE_TRADITIONAL,   3076,   _T("Chinese (Traditional)"),    _T("Lang\\mpcresources.tc.dll")},
    {ID_LANGUAGE_CZECH,                 1029,   _T("Czech"),                    _T("Lang\\mpcresources.cz.dll")},
    {ID_LANGUAGE_DUTCH,                 1043,   _T("Dutch"),                    _T("Lang\\mpcresources.nl.dll")},
    {ID_LANGUAGE_ENGLISH,               0,      _T("English"),                  nullptr},
    {ID_LANGUAGE_FRENCH,                1036,   _T("French"),                   _T("Lang\\mpcresources.fr.dll")},
    {ID_LANGUAGE_GERMAN,                1031,   _T("German"),                   _T("Lang\\mpcresources.de.dll")},
    {ID_LANGUAGE_GREEK,                 1032,   _T("Greek"),                    _T("Lang\\mpcresources.el.dll")},
    {ID_LANGUAGE_HEBREW,                1037,   _T("Hebrew"),                   _T("Lang\\mpcresources.he.dll")},
    {ID_LANGUAGE_HUNGARIAN,             1038,   _T("Hungarian"),                _T("Lang\\mpcresources.hu.dll")},
    {ID_LANGUAGE_ITALIAN,               1040,   _T("Italian"),                  _T("Lang\\mpcresources.it.dll")},
    {ID_LANGUAGE_JAPANESE,              1041,   _T("Japanese"),                 _T("Lang\\mpcresources.ja.dll")},
    {ID_LANGUAGE_KOREAN,                1042,   _T("Korean"),                   _T("Lang\\mpcresources.kr.dll")},
    {ID_LANGUAGE_POLISH,                1045,   _T("Polish"),                   _T("Lang\\mpcresources.pl.dll")},
    {ID_LANGUAGE_PORTUGUESE_BR,         1046,   _T("Portuguese (Brazil)"),      _T("Lang\\mpcresources.br.dll")},
    {ID_LANGUAGE_ROMANIAN,              1048,   _T("Romanian"),                 _T("Lang\\mpcresources.ro.dll")},
    {ID_LANGUAGE_RUSSIAN,               1049,   _T("Russian"),                  _T("Lang\\mpcresources.ru.dll")},
    {ID_LANGUAGE_SLOVAK,                1051,   _T("Slovak"),                   _T("Lang\\mpcresources.sk.dll")},
    {ID_LANGUAGE_SWEDISH,               1053,   _T("Swedish"),                  _T("Lang\\mpcresources.sv.dll")},
    {ID_LANGUAGE_SPANISH,               1034,   _T("Spanish"),                  _T("Lang\\mpcresources.es.dll")},
    {ID_LANGUAGE_TURKISH,               1055,   _T("Turkish"),                  _T("Lang\\mpcresources.tr.dll")},
    {ID_LANGUAGE_UKRAINIAN,             1058,   _T("Ukrainian"),                _T("Lang\\mpcresources.ua.dll")}
};

const size_t CMPlayerCApp::languageResourcesCount = _countof(CMPlayerCApp::languageResources);

HICON LoadIcon(CString fn, bool fSmall)
{
    if (fn.IsEmpty()) {
        return nullptr;
    }

    CString ext = fn.Left(fn.Find(_T("://")) + 1).TrimRight(':');
    if (ext.IsEmpty() || !ext.CompareNoCase(_T("file"))) {
        ext = _T(".") + fn.Mid(fn.ReverseFind('.') + 1);
    }

    CSize size(fSmall ? 16 : 32, fSmall ? 16 : 32);

    if (!ext.CompareNoCase(_T(".ifo"))) {
        if (HICON hIcon = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_DVD), IMAGE_ICON, size.cx, size.cy, 0)) {
            return hIcon;
        }
    }

    if (!ext.CompareNoCase(_T(".cda"))) {
        if (HICON hIcon = (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_AUDIOCD), IMAGE_ICON, size.cx, size.cy, 0)) {
            return hIcon;
        }
    }

    do {
        CRegKey key;
        TCHAR buff[256];
        ULONG len;

        if (ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, ext + _T("\\DefaultIcon"), KEY_READ)) {
            if (ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, ext, KEY_READ)) {
                break;
            }

            len = _countof(buff);
            memset(buff, 0, sizeof(buff));
            if (ERROR_SUCCESS != key.QueryStringValue(nullptr, buff, &len) || (ext = buff).Trim().IsEmpty()) {
                break;
            }

            if (ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, ext + _T("\\DefaultIcon"), KEY_READ)) {
                break;
            }
        }

        CString icon;

        len = _countof(buff);
        memset(buff, 0, sizeof(buff));
        if (ERROR_SUCCESS != key.QueryStringValue(nullptr, buff, &len) || (icon = buff).Trim().IsEmpty()) {
            break;
        }

        int i = icon.ReverseFind(',');
        if (i < 0) {
            break;
        }

        int id = 0;
        if (_stscanf_s(icon.Mid(i + 1), _T("%d"), &id) != 1) {
            break;
        }

        icon = icon.Left(i);

        HICON hIcon = nullptr;
        UINT cnt = fSmall
                   ? ExtractIconEx(icon, id, nullptr, &hIcon, 1)
                   : ExtractIconEx(icon, id, &hIcon, nullptr, 1);
        UNREFERENCED_PARAMETER(cnt);
        if (hIcon) {
            return hIcon;
        }
    } while (0);

    return (HICON)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_UNKNOWN), IMAGE_ICON, size.cx, size.cy, 0);
}

bool LoadType(CString fn, CString& type)
{
    bool found = false;

    if (!fn.IsEmpty()) {
        CString ext = fn.Left(fn.Find(_T("://")) + 1).TrimRight(':');
        if (ext.IsEmpty() || !ext.CompareNoCase(_T("file"))) {
            ext = _T(".") + fn.Mid(fn.ReverseFind('.') + 1);
        }

        // Try MPC-HC's internal formats list
        const CMediaFormatCategory* mfc = AfxGetAppSettings().m_Formats.FindMediaByExt(ext);

        if (mfc != nullptr) {
            found = true;
            type = mfc->GetDescription();
        } else { // Fallback to registry
            CRegKey key;
            TCHAR buff[256];
            ULONG len;

            CString tmp = _T("");
            CString mplayerc_ext = _T("mplayerc") + ext;

            if (ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, mplayerc_ext)) {
                tmp = mplayerc_ext;
            }

            if (!tmp.IsEmpty() || ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, ext)) {
                found = true;

                if (tmp.IsEmpty()) {
                    tmp = ext;
                }

                while (ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, tmp)) {
                    len = _countof(buff);
                    memset(buff, 0, sizeof(buff));

                    if (ERROR_SUCCESS != key.QueryStringValue(nullptr, buff, &len)) {
                        break;
                    }

                    CString str(buff);
                    str.Trim();

                    if (str.IsEmpty() || str == tmp) {
                        break;
                    }

                    tmp = str;
                }

                type = tmp;
            }
        }
    }

    return found;
}

bool LoadResource(UINT resid, CStringA& str, LPCTSTR restype)
{
    str.Empty();
    HRSRC hrsrc = FindResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(resid), restype);
    if (!hrsrc) {
        return false;
    }
    HGLOBAL hGlobal = LoadResource(AfxGetApp()->m_hInstance, hrsrc);
    if (!hGlobal) {
        return false;
    }
    DWORD size = SizeofResource(AfxGetApp()->m_hInstance, hrsrc);
    if (!size) {
        return false;
    }
    memcpy(str.GetBufferSetLength(size), LockResource(hGlobal), size);
    return true;
}

WORD AssignedToCmd(UINT keyOrMouseValue, bool bIsFullScreen, bool bCheckMouse)
{
    WORD assignTo = 0;
    const CAppSettings& s = AfxGetAppSettings();

    POSITION pos = s.wmcmds.GetHeadPosition();
    while (pos && !assignTo) {
        const wmcmd& wc = s.wmcmds.GetNext(pos);

        if (bCheckMouse) {
            if (bIsFullScreen) {
                if (wc.mouseFS == keyOrMouseValue) {
                    assignTo = wc.cmd;
                }
            } else if (wc.mouse == keyOrMouseValue) {
                assignTo = wc.cmd;
            }
        } else if (wc.key == keyOrMouseValue) {
            assignTo = wc.cmd;
        }
    }

    return assignTo;
}

/////////////////////////////////////////////////////////////////////////////
// CMPlayerCApp

BEGIN_MESSAGE_MAP(CMPlayerCApp, CWinApp)
    //{{AFX_MSG_MAP(CMPlayerCApp)
    ON_COMMAND(ID_HELP_ABOUT, OnAppAbout)
    ON_COMMAND(ID_FILE_EXIT, OnFileExit)
    //}}AFX_MSG_MAP
    ON_COMMAND(ID_HELP_SHOWCOMMANDLINESWITCHES, OnHelpShowcommandlineswitches)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMPlayerCApp construction

CMPlayerCApp::CMPlayerCApp()
    : m_hNTDLL(nullptr)
    , m_fClosingState(false)
    , m_fProfileInitialized(false)
{
    TCHAR strApp[MAX_PATH];

    GetModuleFileNameEx(GetCurrentProcess(), AfxGetMyApp()->m_hInstance, strApp, MAX_PATH);
    m_strVersion = CFileVersionInfo::GetFileVersionStr(strApp);

    memset(&m_ColorControl, 0, sizeof(m_ColorControl));
    ResetColorControlRange();

    memset(&m_VMR9ColorControl, 0, sizeof(m_VMR9ColorControl));
    m_VMR9ColorControl[0].dwSize     = sizeof(VMR9ProcAmpControlRange);
    m_VMR9ColorControl[0].dwProperty = ProcAmpControl9_Brightness;
    m_VMR9ColorControl[1].dwSize     = sizeof(VMR9ProcAmpControlRange);
    m_VMR9ColorControl[1].dwProperty = ProcAmpControl9_Contrast;
    m_VMR9ColorControl[2].dwSize     = sizeof(VMR9ProcAmpControlRange);
    m_VMR9ColorControl[2].dwProperty = ProcAmpControl9_Hue;
    m_VMR9ColorControl[3].dwSize     = sizeof(VMR9ProcAmpControlRange);
    m_VMR9ColorControl[3].dwProperty = ProcAmpControl9_Saturation;

    memset(&m_EVRColorControl, 0, sizeof(m_EVRColorControl));

    GetRemoteControlCode = GetRemoteControlCodeMicrosoft;
}

CMPlayerCApp::~CMPlayerCApp()
{
    if (m_hNTDLL) {
        FreeLibrary(m_hNTDLL);
    }
}

void CMPlayerCApp::ShowCmdlnSwitches() const
{
    CString s;

    if (m_s.nCLSwitches & CLSW_UNRECOGNIZEDSWITCH) {
        CAtlList<CString> sl;
        for (int i = 0; i < __argc; i++) {
            sl.AddTail(__targv[i]);
        }
        s += ResStr(IDS_UNKNOWN_SWITCH) + Implode(sl, ' ') + _T("\n\n");
    }

    s += ResStr(IDS_USAGE);

    AfxMessageBox(s, MB_ICONINFORMATION | MB_OK);
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMPlayerCApp object

CMPlayerCApp theApp;

HWND g_hWnd = nullptr;

bool CMPlayerCApp::StoreSettingsToIni()
{
    CString ini = GetIniPath();
    free((void*)m_pszRegistryKey);
    m_pszRegistryKey = nullptr;
    free((void*)m_pszProfileName);
    m_pszProfileName = _tcsdup(ini);

    return true;
}

bool CMPlayerCApp::StoreSettingsToRegistry()
{
    free((void*)m_pszRegistryKey);
    m_pszRegistryKey = nullptr;

    SetRegistryKey(_T("Gabest"));

    return true;
}

CString CMPlayerCApp::GetIniPath() const
{
    CString path = GetProgramPath(true);
    path = path.Left(path.ReverseFind('.') + 1) + _T("ini");
    return path;
}

bool CMPlayerCApp::IsIniValid() const
{
    return FileExists(GetIniPath());
}

bool CMPlayerCApp::GetAppSavePath(CString& path)
{
    path.Empty();

    if (IsIniValid()) { // If settings ini file found, store stuff in the same folder as the exe file
        path = GetProgramPath();
    } else {
        HRESULT hr = SHGetFolderPath(nullptr, CSIDL_APPDATA, nullptr, 0, path.GetBuffer(MAX_PATH));
        path.ReleaseBuffer();
        if (FAILED(hr)) {
            return false;
        }
        CPath p;
        p.Combine(path, _T("Media Player Classic"));
        path = (LPCTSTR)p;
    }

    return true;
}

bool CMPlayerCApp::ChangeSettingsLocation(bool useIni)
{
    bool success;

    // Load favorites so that they can be correctly saved to the new location
    CAtlList<CString> filesFav, DVDsFav, devicesFav;
    m_s.GetFav(FAV_FILE, filesFav);
    m_s.GetFav(FAV_DVD, DVDsFav);
    m_s.GetFav(FAV_DEVICE, devicesFav);

    if (useIni) {
        success = StoreSettingsToIni();
        // No need to delete old mpc-hc.ini,
        // as it will be overwritten during CAppSettings::SaveSettings()
    } else {
        success = StoreSettingsToRegistry();
        _tremove(GetIniPath());
    }

    // Save favorites to the new location
    m_s.SetFav(FAV_FILE, filesFav);
    m_s.SetFav(FAV_DVD, DVDsFav);
    m_s.SetFav(FAV_DEVICE, devicesFav);

    // Save external filters to the new location
    m_s.SaveExternalFilters();

    // Ensure the shaders are properly saved
    m_s.fShaderEditorWasOpened = true;

    // Write settings immediately
    m_s.SaveSettings();

    return success;
}

bool CMPlayerCApp::ExportSettings(CString savePath, CString subKey)
{
    bool success = false;
    m_s.SaveSettings();

    if (IsIniValid()) {
        success = !!CopyFile(GetIniPath(), savePath, FALSE);
    } else {
        CString regKey;
        if (subKey.IsEmpty()) {
            regKey.Format(_T("Software\\%s\\%s"), m_pszRegistryKey, m_pszProfileName);
        } else {
            regKey.Format(_T("Software\\%s\\%s\\%s"), m_pszRegistryKey, m_pszProfileName, subKey);
        }

        FILE* fStream;
        errno_t error = _tfopen_s(&fStream, savePath, _T("wt,ccs=UNICODE"));
        CStdioFile file(fStream);
        file.WriteString(_T("Windows Registry Editor Version 5.00\n\n"));

        success = !error && ExportRegistryKey(file, HKEY_CURRENT_USER, regKey);

        file.Close();
        if (!success && !error) {
            DeleteFile(savePath);
        }
    }

    return success;
}

void CMPlayerCApp::InitProfile()
{
    // Calls to CMPlayerCApp::InitProfile() are not serialized,
    // so we serialize its internals
    CSingleLock(&m_ProfileCriticalSection, TRUE);

    if (m_fProfileInitialized) {
        return;
    }
    m_fProfileInitialized = true;

    if (!m_pszRegistryKey) {
        ASSERT(m_pszProfileName);
        if (!FileExists(m_pszProfileName)) {
            return;
        }

        FILE* fp;
        int fpStatus;
        do { // Open mpc-hc.ini in UNICODE mode, retry if it is already being used by another process
            fp = _tfsopen(m_pszProfileName, _T("r, ccs=UNICODE"), _SH_SECURE);
            if (fp || (GetLastError() != ERROR_SHARING_VIOLATION)) {
                break;
            }
            Sleep(100);
        } while (true);
        if (!fp) {
            ASSERT(FALSE);
            return;
        }
        if (_ftell_nolock(fp) == 0L) {
            // No BOM was consumed, assume mpc-hc.ini is ANSI encoded
            fpStatus = fclose(fp);
            ASSERT(fpStatus == 0);
            do { // Reopen mpc-hc.ini in ANSI mode, retry if it is already being used by another process
                fp = _tfsopen(m_pszProfileName, _T("r"), _SH_SECURE);
                if (fp || (GetLastError() != ERROR_SHARING_VIOLATION)) {
                    break;
                }
                Sleep(100);
            } while (true);
            if (!fp) {
                ASSERT(FALSE);
                return;
            }
        }

        CStdioFile file(fp);
        CString line, section, var, val;
        while (file.ReadString(line)) {
            // Parse mpc-hc.ini file, this parser:
            //  - doesn't trim whitespaces
            //  - doesn't remove quotation marks
            //  - omits keys with empty names
            //  - omits unnamed sections
            int pos = 0;
            if (line[0] == _T('[')) {
                pos = line.Find(_T(']'));
                if (pos == -1) {
                    continue;
                }
                section = line.Mid(1, pos - 1);
            } else if (line[0] != _T(';')) {
                pos = line.Find(_T('='));
                if (pos == -1) {
                    continue;
                }
                var = line.Mid(0, pos);
                val = line.Mid(pos + 1);
                if (!section.IsEmpty() && !var.IsEmpty()) {
                    m_ProfileMap[section][var] = val;
                }
            }
        }
        fpStatus = fclose(fp);
        ASSERT(fpStatus == 0);
    }
}

void CMPlayerCApp::FlushProfile()
{
    ASSERT(m_fProfileInitialized);

    if (!m_pszRegistryKey) {
        ASSERT(m_pszProfileName);

        FILE* fp;
        int fpStatus;
        do { // Open mpc-hc.ini, retry if it is already being used by another process
            fp = _tfsopen(m_pszProfileName, _T("w, ccs=UTF-8"), _SH_SECURE);
            if (fp || (GetLastError() != ERROR_SHARING_VIOLATION)) {
                break;
            }
            Sleep(100);
        } while (true);
        if (!fp) {
            ASSERT(FALSE);
            return;
        }
        CStdioFile file(fp);
        CString line;
        m_ProfileCriticalSection.Lock();
        try {
            file.WriteString(_T("; Media Player Classic - Home Cinema\n"));
            for (auto it1 = m_ProfileMap.begin(); it1 != m_ProfileMap.end(); ++it1) {
                line.Format(_T("[%s]\n"), it1->first);
                file.WriteString(line);
                for (auto it2 = it1->second.begin(); it2 != it1->second.end(); ++it2) {
                    line.Format(_T("%s=%s\n"), it2->first, it2->second);
                    file.WriteString(line);
                }
            }
        } catch (CFileException& e) {
            // Fail silently if disk is full
            UNREFERENCED_PARAMETER(e);
            ASSERT(FALSE);
        }
        m_ProfileCriticalSection.Unlock();
        fpStatus = fclose(fp);
        ASSERT(fpStatus == 0);
    }
}

BOOL CMPlayerCApp::GetProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE* ppData, UINT* pBytes)
{
    if (!m_fProfileInitialized) {
        InitProfile();
        ASSERT(m_fProfileInitialized);
    }

    if (m_pszRegistryKey) {
        return CWinApp::GetProfileBinary(lpszSection, lpszEntry, ppData, pBytes);
    } else {
        if (!lpszSection || !lpszEntry || !ppData || !pBytes) {
            ASSERT(FALSE);
            return FALSE;
        }
        CString sectionStr(lpszSection);
        CString keyStr(lpszEntry);
        if (sectionStr.IsEmpty() || keyStr.IsEmpty()) {
            ASSERT(FALSE);
            return FALSE;
        }
        CString valueStr;

        m_ProfileCriticalSection.Lock();
        auto it1 = m_ProfileMap.find(sectionStr);
        if (it1 != m_ProfileMap.end()) {
            auto it2 = it1->second.find(keyStr);
            if (it2 != it1->second.end()) {
                valueStr = it2->second;
            }
        }
        m_ProfileCriticalSection.Unlock();
        if (valueStr.IsEmpty()) {
            return FALSE;
        }
        int length = valueStr.GetLength();
        // Encoding: each 4-bit sequence is coded in one character, from 'A' for 0x0 to 'P' for 0xf
        if (length % 2) {
            ASSERT(FALSE);
            return FALSE;
        }
        for (int i = 0; i < length; i++) {
            if (valueStr[i] < 'A' || valueStr[i] > 'P') {
                ASSERT(FALSE);
                return FALSE;
            }
        }
        *pBytes = length / 2;
        *ppData = new(std::nothrow) BYTE[*pBytes];
        if (!(*ppData)) {
            ASSERT(FALSE);
            return FALSE;
        }
        for (UINT i = 0; i < *pBytes; i++) {
            (*ppData)[i] = (valueStr[i * 2] - 'A') | ((valueStr[i * 2 + 1] - 'A') << 4);
        }
        return TRUE;
    }
}

UINT CMPlayerCApp::GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault)
{
    if (!m_fProfileInitialized) {
        InitProfile();
        ASSERT(m_fProfileInitialized);
    }

    int res = nDefault;
    if (m_pszRegistryKey) {
        res = CWinApp::GetProfileInt(lpszSection, lpszEntry, nDefault);
    } else {
        if (!lpszSection || !lpszEntry) {
            ASSERT(FALSE);
            return res;
        }
        CString sectionStr(lpszSection);
        CString keyStr(lpszEntry);
        if (sectionStr.IsEmpty() || keyStr.IsEmpty()) {
            ASSERT(FALSE);
            return res;
        }

        m_ProfileCriticalSection.Lock();
        auto it1 = m_ProfileMap.find(sectionStr);
        if (it1 != m_ProfileMap.end()) {
            auto it2 = it1->second.find(keyStr);
            if (it2 != it1->second.end()) {
                res = _ttoi(it2->second);
            }
        }
        m_ProfileCriticalSection.Unlock();
    }
    return res;
}

CString CMPlayerCApp::GetProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault)
{
    if (!m_fProfileInitialized) {
        InitProfile();
        ASSERT(m_fProfileInitialized);
    }

    CString res;
    if (m_pszRegistryKey) {
        res = CWinApp::GetProfileString(lpszSection, lpszEntry, lpszDefault);
    } else {
        if (!lpszSection || !lpszEntry) {
            ASSERT(FALSE);
            return res;
        }
        CString sectionStr(lpszSection);
        CString keyStr(lpszEntry);
        if (sectionStr.IsEmpty() || keyStr.IsEmpty()) {
            ASSERT(FALSE);
            return res;
        }
        if (lpszDefault) {
            res = lpszDefault;
        }

        m_ProfileCriticalSection.Lock();
        auto it1 = m_ProfileMap.find(sectionStr);
        if (it1 != m_ProfileMap.end()) {
            auto it2 = it1->second.find(keyStr);
            if (it2 != it1->second.end()) {
                res = it2->second;
            }
        }
        m_ProfileCriticalSection.Unlock();
    }
    return res;
}

BOOL CMPlayerCApp::WriteProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPBYTE pData, UINT nBytes)
{
    if (!m_fProfileInitialized) {
        InitProfile();
        ASSERT(m_fProfileInitialized);
    }

    if (m_pszRegistryKey) {
        return CWinApp::WriteProfileBinary(lpszSection, lpszEntry, pData, nBytes);
    } else {
        if (!lpszSection || !lpszEntry || !pData || !nBytes) {
            ASSERT(FALSE);
            return FALSE;
        }
        CString sectionStr(lpszSection);
        CString keyStr(lpszEntry);
        if (sectionStr.IsEmpty() || keyStr.IsEmpty()) {
            ASSERT(FALSE);
            return FALSE;
        }
        CString valueStr;

        TCHAR* buffer = valueStr.GetBufferSetLength(nBytes * 2);
        // Encoding: each 4-bit sequence is coded in one character, from 'A' for 0x0 to 'P' for 0xf
        for (UINT i = 0; i < nBytes; i++) {
            buffer[i * 2] = 'A' + (pData[i] & 0xf);
            buffer[i * 2 + 1] = 'A' + (pData[i] >> 4 & 0xf);
        }
        valueStr.ReleaseBufferSetLength(nBytes * 2);
        m_ProfileCriticalSection.Lock();
        m_ProfileMap[sectionStr][keyStr] = valueStr;
        m_ProfileCriticalSection.Unlock();
        return TRUE;
    }
}

BOOL CMPlayerCApp::WriteProfileInt(LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue)
{
    if (!m_fProfileInitialized) {
        InitProfile();
        ASSERT(m_fProfileInitialized);
    }

    if (m_pszRegistryKey) {
        return CWinApp::WriteProfileInt(lpszSection, lpszEntry, nValue);
    } else {
        if (!lpszSection || !lpszEntry) {
            ASSERT(FALSE);
            return FALSE;
        }
        CString sectionStr(lpszSection);
        CString keyStr(lpszEntry);
        if (sectionStr.IsEmpty() || keyStr.IsEmpty()) {
            ASSERT(FALSE);
            return FALSE;
        }
        CString valueStr;

        valueStr.Format(_T("%d"), nValue);
        m_ProfileCriticalSection.Lock();
        m_ProfileMap[sectionStr][keyStr] = valueStr;
        m_ProfileCriticalSection.Unlock();
        return TRUE;
    }
}

BOOL CMPlayerCApp::WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
    if (!m_fProfileInitialized) {
        InitProfile();
        ASSERT(m_fProfileInitialized);
    }

    if (m_pszRegistryKey) {
        return CWinApp::WriteProfileString(lpszSection, lpszEntry, lpszValue);
    } else {
        if (!lpszSection) {
            ASSERT(FALSE);
            return FALSE;
        }
        CString sectionStr(lpszSection);
        if (sectionStr.IsEmpty()) {
            ASSERT(FALSE);
            return FALSE;
        }

        // Mimic CWinApp::WriteProfileString() behavior
        if (lpszEntry) {
            CString keyStr(lpszEntry);
            if (keyStr.IsEmpty()) {
                ASSERT(FALSE);
                return FALSE;
            }

            m_ProfileCriticalSection.Lock();
            if (lpszValue) {
                m_ProfileMap[sectionStr][keyStr] = lpszValue;
            } else { // Delete key
                auto it = m_ProfileMap.find(sectionStr);
                if (it != m_ProfileMap.end()) {
                    it->second.erase(keyStr);
                }
            }
            m_ProfileCriticalSection.Unlock();
        } else { // Delete section
            m_ProfileCriticalSection.Lock();
            m_ProfileMap.erase(sectionStr);
            m_ProfileCriticalSection.Unlock();
        }
        return TRUE;
    }
}

void CMPlayerCApp::PreProcessCommandLine()
{
    m_cmdln.RemoveAll();

    for (int i = 1; i < __argc; i++) {
        m_cmdln.AddTail(CString(__targv[i]).Trim(_T(" \"")));
    }
}

bool CMPlayerCApp::SendCommandLine(HWND hWnd)
{
    if (m_cmdln.IsEmpty()) {
        return false;
    }

    int bufflen = sizeof(DWORD);

    POSITION pos = m_cmdln.GetHeadPosition();
    while (pos) {
        bufflen += (m_cmdln.GetNext(pos).GetLength() + 1) * sizeof(TCHAR);
    }

    CAutoVectorPtr<BYTE> buff;
    if (!buff.Allocate(bufflen)) {
        return FALSE;
    }

    BYTE* p = buff;

    *(DWORD*)p = (DWORD)m_cmdln.GetCount();
    p += sizeof(DWORD);

    pos = m_cmdln.GetHeadPosition();
    while (pos) {
        const CString& s = m_cmdln.GetNext(pos);
        int len = (s.GetLength() + 1) * sizeof(TCHAR);
        memcpy(p, s, len);
        p += len;
    }

    COPYDATASTRUCT cds;
    cds.dwData = 0x6ABE51;
    cds.cbData = bufflen;
    cds.lpData = (void*)(BYTE*)buff;

    return !!SendMessage(hWnd, WM_COPYDATA, (WPARAM)nullptr, (LPARAM)&cds);
}

/////////////////////////////////////////////////////////////////////////////
// CMPlayerCApp initialization

BOOL (__stdcall* Real_IsDebuggerPresent)()
    = IsDebuggerPresent;

LONG(__stdcall* Real_ChangeDisplaySettingsExA)(LPCSTR a0,
        LPDEVMODEA a1,
        HWND a2,
        DWORD a3,
        LPVOID a4)
    = ChangeDisplaySettingsExA;

LONG(__stdcall* Real_ChangeDisplaySettingsExW)(LPCWSTR a0,
        LPDEVMODEW a1,
        HWND a2,
        DWORD a3,
        LPVOID a4)
    = ChangeDisplaySettingsExW;

HANDLE(__stdcall* Real_CreateFileA)(LPCSTR a0,
                                    DWORD a1,
                                    DWORD a2,
                                    LPSECURITY_ATTRIBUTES a3,
                                    DWORD a4,
                                    DWORD a5,
                                    HANDLE a6)
    = CreateFileA;

HANDLE(__stdcall* Real_CreateFileW)(LPCWSTR a0,
                                    DWORD a1,
                                    DWORD a2,
                                    LPSECURITY_ATTRIBUTES a3,
                                    DWORD a4,
                                    DWORD a5,
                                    HANDLE a6)
    = CreateFileW;

BOOL (__stdcall* Real_DeviceIoControl)(HANDLE a0,
                                       DWORD a1,
                                       LPVOID a2,
                                       DWORD a3,
                                       LPVOID a4,
                                       DWORD a5,
                                       LPDWORD a6,
                                       LPOVERLAPPED a7)
    = DeviceIoControl;

MMRESULT(__stdcall* Real_mixerSetControlDetails)(HMIXEROBJ hmxobj,
        LPMIXERCONTROLDETAILS pmxcd,
        DWORD fdwDetails)
    = mixerSetControlDetails;


typedef NTSTATUS(WINAPI* FUNC_NTQUERYINFORMATIONPROCESS)(HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);
static FUNC_NTQUERYINFORMATIONPROCESS Real_NtQueryInformationProcess = nullptr;
/*
NTSTATUS (* Real_NtQueryInformationProcess) (HANDLE             ProcessHandle,
                                             PROCESSINFOCLASS   ProcessInformationClass,
                                             PVOID              ProcessInformation,
                                             ULONG              ProcessInformationLength,
                                             PULONG             ReturnLength)
    = nullptr;
*/


BOOL WINAPI Mine_IsDebuggerPresent()
{
    TRACE(_T("Oops, somebody was trying to be naughty! (called IsDebuggerPresent)\n"));
    return FALSE;
}


NTSTATUS WINAPI Mine_NtQueryInformationProcess(HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength)
{
    NTSTATUS nRet;

    nRet = Real_NtQueryInformationProcess(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);

    if (ProcessInformationClass == ProcessBasicInformation) {
        PROCESS_BASIC_INFORMATION* pbi = (PROCESS_BASIC_INFORMATION*)ProcessInformation;
        PEB_NT* pPEB;
        PEB_NT PEB;

        pPEB = (PEB_NT*)pbi->PebBaseAddress;
        ReadProcessMemory(ProcessHandle, pPEB, &PEB, sizeof(PEB), nullptr);
        PEB.BeingDebugged = 0;
        WriteProcessMemory(ProcessHandle, pPEB, &PEB, sizeof(PEB), nullptr);
    } else if (ProcessInformationClass == 7) { // ProcessDebugPort
        BOOL* pDebugPort = (BOOL*)ProcessInformation;
        *pDebugPort = FALSE;
    }

    return nRet;
}

LONG WINAPI Mine_ChangeDisplaySettingsEx(LONG ret, DWORD dwFlags, LPVOID lParam)
{
    if (dwFlags & CDS_VIDEOPARAMETERS) {
        VIDEOPARAMETERS* vp = (VIDEOPARAMETERS*)lParam;

        if (vp->Guid == GUIDFromCString(_T("{02C62061-1097-11d1-920F-00A024DF156E}"))
                && (vp->dwFlags & VP_FLAGS_COPYPROTECT)) {
            if (vp->dwCommand == VP_COMMAND_GET) {
                if ((vp->dwTVStandard & VP_TV_STANDARD_WIN_VGA)
                        && vp->dwTVStandard != VP_TV_STANDARD_WIN_VGA) {
                    TRACE(_T("Ooops, tv-out enabled? macrovision checks suck..."));
                    vp->dwTVStandard = VP_TV_STANDARD_WIN_VGA;
                }
            } else if (vp->dwCommand == VP_COMMAND_SET) {
                TRACE(_T("Ooops, as I already told ya, no need for any macrovision bs here"));
                return 0;
            }
        }
    }

    return ret;
}

LONG WINAPI Mine_ChangeDisplaySettingsExA(LPCSTR lpszDeviceName, LPDEVMODEA lpDevMode, HWND hwnd, DWORD dwFlags, LPVOID lParam)
{
    return Mine_ChangeDisplaySettingsEx(
               Real_ChangeDisplaySettingsExA(lpszDeviceName, lpDevMode, hwnd, dwFlags, lParam),
               dwFlags,
               lParam);
}

LONG WINAPI Mine_ChangeDisplaySettingsExW(LPCWSTR lpszDeviceName, LPDEVMODEW lpDevMode, HWND hwnd, DWORD dwFlags, LPVOID lParam)
{
    return Mine_ChangeDisplaySettingsEx(
               Real_ChangeDisplaySettingsExW(lpszDeviceName, lpDevMode, hwnd, dwFlags, lParam),
               dwFlags,
               lParam);
}

HANDLE WINAPI Mine_CreateFileA(LPCSTR p1, DWORD p2, DWORD p3, LPSECURITY_ATTRIBUTES p4, DWORD p5, DWORD p6, HANDLE p7)
{
    //CStringA fn(p1);
    //fn.MakeLower();
    //int i = fn.Find(".part");
    //if (i > 0 && i == fn.GetLength() - 5)
    p3 |= FILE_SHARE_WRITE;

    return Real_CreateFileA(p1, p2, p3, p4, p5, p6, p7);
}

BOOL CreateFakeVideoTS(LPCWSTR strIFOPath, LPWSTR strFakeFile, size_t nFakeFileSize)
{
    BOOL  bRet = FALSE;
    WCHAR szTempPath[MAX_PATH];
    WCHAR strFileName[MAX_PATH];
    WCHAR strExt[_MAX_EXT];
    CIfo  Ifo;

    if (!GetTempPathW(MAX_PATH, szTempPath)) {
        return FALSE;
    }

    _wsplitpath_s(strIFOPath, nullptr, 0, nullptr, 0, strFileName, _countof(strFileName), strExt, _countof(strExt));
    _snwprintf_s(strFakeFile, nFakeFileSize, _TRUNCATE, L"%sMPC%s%s", szTempPath, strFileName, strExt);

    if (Ifo.OpenFile(strIFOPath) &&
            Ifo.RemoveUOPs()  &&
            Ifo.SaveFile(strFakeFile)) {
        bRet = TRUE;
    }

    return bRet;
}

HANDLE WINAPI Mine_CreateFileW(LPCWSTR p1, DWORD p2, DWORD p3, LPSECURITY_ATTRIBUTES p4, DWORD p5, DWORD p6, HANDLE p7)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    size_t nLen  = wcslen(p1);

    p3 |= FILE_SHARE_WRITE;

    if (nLen >= 4 && _wcsicmp(p1 + nLen - 4, L".ifo") == 0) {
        WCHAR strFakeFile[MAX_PATH];
        if (CreateFakeVideoTS(p1, strFakeFile, _countof(strFakeFile))) {
            hFile = Real_CreateFileW(strFakeFile, p2, p3, p4, p5, p6, p7);
        }
    }

    if (hFile == INVALID_HANDLE_VALUE) {
        hFile = Real_CreateFileW(p1, p2, p3, p4, p5, p6, p7);
    }

    return hFile;
}

MMRESULT WINAPI Mine_mixerSetControlDetails(HMIXEROBJ hmxobj, LPMIXERCONTROLDETAILS pmxcd, DWORD fdwDetails)
{
    if (fdwDetails == (MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE)) {
        return MMSYSERR_NOERROR;    // don't touch the mixer, kthx
    }
    return Real_mixerSetControlDetails(hmxobj, pmxcd, fdwDetails);
}

BOOL WINAPI Mine_DeviceIoControl(HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned, LPOVERLAPPED lpOverlapped)
{
    BOOL ret = Real_DeviceIoControl(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped);

    if (IOCTL_DVD_GET_REGION == dwIoControlCode && lpOutBuffer
            && lpBytesReturned && *lpBytesReturned == sizeof(DVD_REGION)) {
        DVD_REGION* pDVDRegion = (DVD_REGION*)lpOutBuffer;
        pDVDRegion->SystemRegion = ~pDVDRegion->RegionData;
    }

    return ret;
}

BOOL SetHeapOptions()
{
    HMODULE hLib = LoadLibrary(L"kernel32.dll");
    if (hLib == nullptr) {
        return FALSE;
    }

    typedef BOOL (WINAPI * HSI)
    (HANDLE, HEAP_INFORMATION_CLASS , PVOID, SIZE_T);
    HSI pHsi = (HSI)GetProcAddress(hLib, "HeapSetInformation");
    if (!pHsi) {
        FreeLibrary(hLib);
        return FALSE;
    }

#ifndef HeapEnableTerminationOnCorruption
#   define HeapEnableTerminationOnCorruption (HEAP_INFORMATION_CLASS)1
#endif

    BOOL fRet = (pHsi)(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0)
                ? TRUE
                : FALSE;
    if (hLib) {
        FreeLibrary(hLib);
    }

    return fRet;
}

BOOL CMPlayerCApp::InitInstance()
{
    // Remove the working directory from the search path to work around the DLL preloading vulnerability
    SetDllDirectory(_T(""));

    long lError;

    if (SetHeapOptions()) {
        TRACE(_T("Terminate on corruption enabled\n"));
    } else {
        CString heap_err;
        heap_err.Format(_T("Terminate on corruption error = %d\n"), GetLastError());
        TRACE(heap_err);
    }

    DetourRestoreAfterWith();
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    DetourAttach(&(PVOID&)Real_IsDebuggerPresent, (PVOID)Mine_IsDebuggerPresent);
    DetourAttach(&(PVOID&)Real_ChangeDisplaySettingsExA, (PVOID)Mine_ChangeDisplaySettingsExA);
    DetourAttach(&(PVOID&)Real_ChangeDisplaySettingsExW, (PVOID)Mine_ChangeDisplaySettingsExW);
    DetourAttach(&(PVOID&)Real_CreateFileA, (PVOID)Mine_CreateFileA);
    DetourAttach(&(PVOID&)Real_CreateFileW, (PVOID)Mine_CreateFileW);
    DetourAttach(&(PVOID&)Real_mixerSetControlDetails, (PVOID)Mine_mixerSetControlDetails);
    DetourAttach(&(PVOID&)Real_DeviceIoControl, (PVOID)Mine_DeviceIoControl);

    m_hNTDLL = LoadLibrary(_T("ntdll.dll"));
#ifndef _DEBUG  // Disable NtQueryInformationProcess in debug (prevent VS debugger to stop on crash address)
    if (m_hNTDLL) {
        Real_NtQueryInformationProcess = (FUNC_NTQUERYINFORMATIONPROCESS)GetProcAddress(m_hNTDLL, "NtQueryInformationProcess");

        if (Real_NtQueryInformationProcess) {
            DetourAttach(&(PVOID&)Real_NtQueryInformationProcess, (PVOID)Mine_NtQueryInformationProcess);
        }
    }
#endif

    CFilterMapper2::Init();

    lError = DetourTransactionCommit();
    ASSERT(lError == NOERROR);

    if (FAILED(OleInitialize(0))) {
        AfxMessageBox(_T("OleInitialize failed!"));
        return FALSE;
    }

    // Be careful if you move that code: IDR_MAINFRAME icon can only be loaded from the executable,
    // LoadIcon can't be used after the language DLL has been set as the main resource.
    HICON icon = LoadIcon(IDR_MAINFRAME);

    WNDCLASS wndcls;
    memset(&wndcls, 0, sizeof(WNDCLASS));
    wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wndcls.lpfnWndProc = ::DefWindowProc;
    wndcls.hInstance = AfxGetInstanceHandle();
    wndcls.hIcon = icon;
    wndcls.hCursor = LoadCursor(IDC_ARROW);
    wndcls.hbrBackground = 0;//(HBRUSH)(COLOR_WINDOW + 1); // no bkg brush, the view and the bars should always fill the whole client area
    wndcls.lpszMenuName = nullptr;
    wndcls.lpszClassName = MPC_WND_CLASS_NAME;

    if (!AfxRegisterClass(&wndcls)) {
        AfxMessageBox(_T("MainFrm class registration failed!"));
        return FALSE;
    }

    if (!AfxSocketInit(nullptr)) {
        AfxMessageBox(_T("AfxSocketInit failed!"));
        return FALSE;
    }

    PreProcessCommandLine();

    if (IsIniValid()) {
        StoreSettingsToIni();
    } else {
        StoreSettingsToRegistry();
    }

    m_s.ParseCommandLine(m_cmdln);

    if (m_s.nCLSwitches & (CLSW_HELP | CLSW_UNRECOGNIZEDSWITCH)) { // show comandline help window
        m_s.LoadSettings();
        ShowCmdlnSwitches();
        return FALSE;
    }

    if (m_s.nCLSwitches & CLSW_RESET) { // reset settings
        // We want the other instances to be closed before resetting the settings.
        HWND hWnd = FindWindow(MPC_WND_CLASS_NAME, nullptr);

        while (hWnd) {
            Sleep(500);

            hWnd = FindWindow(MPC_WND_CLASS_NAME, nullptr);

            if (hWnd && MessageBox(nullptr, ResStr(IDS_RESET_SETTINGS_MUTEX), ResStr(IDS_RESET_SETTINGS), MB_ICONEXCLAMATION | MB_RETRYCANCEL) == IDCANCEL) {
                return FALSE;
            }
        }

        // If the profile was already cached, it should be cleared here
        ASSERT(!m_fProfileInitialized);

        // Remove the settings
        if (IsIniValid()) {
            CFile::Remove(GetIniPath());
        } else {
            HKEY reg = GetAppRegistryKey();
            SHDeleteKey(reg, _T(""));
            RegCloseKey(reg);
        }

        // Remove the current playlist if it exists
        CString strSavePath;
        if (AfxGetMyApp()->GetAppSavePath(strSavePath)) {
            CPath playlistPath;
            playlistPath.Combine(strSavePath, _T("default.mpcpl"));

            if (FileExists(playlistPath)) {
                CFile::Remove(playlistPath);
            }
        }
    }

    if ((m_s.nCLSwitches & CLSW_CLOSE) && m_s.slFiles.IsEmpty()) { // "/close" switch and empty file list
        return FALSE;
    }

    if (m_s.nCLSwitches & (CLSW_REGEXTVID | CLSW_REGEXTAUD | CLSW_REGEXTPL)) { // register file types
        CFileAssoc::RegisterApp();

        CMediaFormats& mf = m_s.m_Formats;
        mf.UpdateData(false);

        bool bAudioOnly, bPlaylist;

        CFileAssoc::LoadIconLib();
        CFileAssoc::SaveIconLibVersion();

        for (size_t i = 0, cnt = mf.GetCount(); i < cnt; i++) {
            bPlaylist = !mf[i].GetLabel().CompareNoCase(_T("pls"));

            if (bPlaylist && !(m_s.nCLSwitches & CLSW_REGEXTPL)) {
                continue;
            }

            bAudioOnly = mf[i].IsAudioOnly();

            if (((m_s.nCLSwitches & CLSW_REGEXTVID) && !bAudioOnly) ||
                    ((m_s.nCLSwitches & CLSW_REGEXTAUD) && bAudioOnly) ||
                    ((m_s.nCLSwitches & CLSW_REGEXTPL) && bPlaylist)) {
                CFileAssoc::Register(mf[i], true, false, true);
            }
        }

        CFileAssoc::FreeIconLib();

        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);

        return FALSE;
    }

    if (m_s.nCLSwitches & CLSW_UNREGEXT) { // unregistered file types
        CMediaFormats& mf = m_s.m_Formats;
        mf.UpdateData(false);

        for (size_t i = 0, cnt = mf.GetCount(); i < cnt; i++) {
            CFileAssoc::Register(mf[i], false, false, false);
        }

        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);

        return FALSE;
    }

    if (m_s.nCLSwitches & CLSW_ICONSASSOC) {
        CMediaFormats& mf = m_s.m_Formats;
        mf.UpdateData(false);

        CAtlList<CString> registeredExts;
        CFileAssoc::GetAssociatedExtensionsFromRegistry(registeredExts);

        CFileAssoc::ReAssocIcons(registeredExts);

        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);

        return FALSE;
    }

    // Enable to open options with administrator privilege (for Vista UAC)
    if (m_s.nCLSwitches & CLSW_ADMINOPTION) {
        m_s.LoadSettings(); // read all settings. long time but not critical at this point

        switch (m_s.iAdminOption) {
            case CPPageFormats::IDD: {
                CPPageSheet options(ResStr(IDS_OPTIONS_CAPTION), nullptr, nullptr, m_s.iAdminOption);
                options.LockPage();
                options.DoModal();
            }
            break;

            default:
                ASSERT(FALSE);
        }
        return FALSE;
    }

    m_mutexOneInstance.Create(nullptr, TRUE, MPC_WND_CLASS_NAME);

    if (GetLastError() == ERROR_ALREADY_EXISTS &&
            (!(m_s.GetAllowMultiInst() || m_s.nCLSwitches & CLSW_NEW || m_cmdln.IsEmpty()) || m_s.nCLSwitches & CLSW_ADD)) {

        DWORD res = WaitForSingleObject(m_mutexOneInstance.m_h, 5000);
        if (res == WAIT_OBJECT_0 || res == WAIT_ABANDONED) {
            HWND hWnd = ::FindWindow(MPC_WND_CLASS_NAME, nullptr);
            if (hWnd) {
                SetForegroundWindow(hWnd);
                if (!(m_s.nCLSwitches & CLSW_MINIMIZED) && IsIconic(hWnd)) {
                    ShowWindow(hWnd, SW_RESTORE);
                }
                if (SendCommandLine(hWnd)) {
                    m_mutexOneInstance.Close();
                    return FALSE;
                }
            }
        }
    }

    m_s.LoadSettings(); // read settings

    AfxGetMyApp()->m_AudioRendererDisplayName_CL = _T("");

    if (!__super::InitInstance()) {
        AfxMessageBox(_T("InitInstance failed!"));
        return FALSE;
    }

    if (!IsIniValid()) {
        CRegKey key;
        CString exePath = GetProgramPath(true);
        if (ERROR_SUCCESS == key.Create(HKEY_CURRENT_USER, _T("Software\\Gabest\\Media Player Classic"))) {
            key.SetStringValue(_T("ExePath"), exePath);
        }
    }

    AfxEnableControlContainer();

    CMainFrame* pFrame = DEBUG_NEW CMainFrame;
    m_pMainWnd = pFrame;
    if (!pFrame->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, nullptr, nullptr)) {
        AfxMessageBox(_T("CMainFrame::LoadFrame failed!"));
        return FALSE;
    }
    pFrame->SetDefaultWindowRect((m_s.nCLSwitches & CLSW_MONITOR) ? m_s.iMonitor : 0);
    pFrame->RestoreControlBars();
    pFrame->SetDefaultFullscreenState();
    pFrame->SetIcon(icon, TRUE);
    pFrame->DragAcceptFiles();
    pFrame->ShowWindow((m_s.nCLSwitches & CLSW_MINIMIZED) ? SW_SHOWMINIMIZED : SW_SHOW);
    pFrame->UpdateWindow();
    pFrame->m_hAccelTable = m_s.hAccel;
    m_s.WinLircClient.SetHWND(m_pMainWnd->m_hWnd);
    if (m_s.fWinLirc) {
        m_s.WinLircClient.Connect(m_s.strWinLircAddr);
    }
    m_s.UIceClient.SetHWND(m_pMainWnd->m_hWnd);
    if (m_s.fUIce) {
        m_s.UIceClient.Connect(m_s.strUIceAddr);
    }

    SendCommandLine(m_pMainWnd->m_hWnd);
    RegisterHotkeys();

    pFrame->SetFocus();

    // set HIGH I/O Priority for better playback performance
    if (m_hNTDLL) {
        typedef NTSTATUS(WINAPI * FUNC_NTSETINFORMATIONPROCESS)(HANDLE, ULONG, PVOID, ULONG);
        FUNC_NTSETINFORMATIONPROCESS NtSetInformationProcess = (FUNC_NTSETINFORMATIONPROCESS)GetProcAddress(m_hNTDLL, "NtSetInformationProcess");

        if (NtSetInformationProcess && SetPrivilege(SE_INC_BASE_PRIORITY_NAME)) {
            ULONG IoPriority = 3;
            ULONG ProcessIoPriority = 0x21;
            NTSTATUS NtStatus = NtSetInformationProcess(GetCurrentProcess(), ProcessIoPriority, &IoPriority, sizeof(ULONG));
            TRACE(_T("Set I/O Priority - %d\n"), NtStatus);
#ifndef _DEBUG
            UNREFERENCED_PARAMETER(NtStatus);
#endif
        }
    }

    m_mutexOneInstance.Release();

    CWebServer::Init();

    if (UpdateChecker::IsAutoUpdateEnabled()) {
        UpdateChecker::CheckForUpdate(true);
    }

    if (m_s.fAssociatedWithIcons) {
        CFileAssoc::CheckIconsAssoc();
    }

    return TRUE;
}

UINT CMPlayerCApp::GetRemoteControlCodeMicrosoft(UINT nInputcode, HRAWINPUT hRawInput)
{
    UINT dwSize = 0;
    UINT nMceCmd = 0;

    // Support for MCE remote control
    GetRawInputData(hRawInput, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
    if (dwSize > 0) {
        BYTE* pRawBuffer = DEBUG_NEW BYTE[dwSize];
        if (GetRawInputData(hRawInput, RID_INPUT, pRawBuffer, &dwSize, sizeof(RAWINPUTHEADER)) != -1) {
            RAWINPUT* raw = (RAWINPUT*)pRawBuffer;
            if (raw->header.dwType == RIM_TYPEHID) {
                nMceCmd = 0x10000 + (raw->data.hid.bRawData[1] | raw->data.hid.bRawData[2] << 8);
            }
        }
        delete [] pRawBuffer;
    }

    return nMceCmd;
}

UINT CMPlayerCApp::GetRemoteControlCodeSRM7500(UINT nInputcode, HRAWINPUT hRawInput)
{
    UINT dwSize = 0;
    UINT nMceCmd = 0;

    GetRawInputData(hRawInput, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
    if (dwSize > 21) {
        BYTE* pRawBuffer = DEBUG_NEW BYTE[dwSize];
        if (GetRawInputData(hRawInput, RID_INPUT, pRawBuffer, &dwSize, sizeof(RAWINPUTHEADER)) != -1) {
            RAWINPUT* raw = (RAWINPUT*)pRawBuffer;

            // data.hid.bRawData[21] set to one when key is pressed
            if (raw->header.dwType == RIM_TYPEHID && raw->data.hid.bRawData[21] == 1) {
                // data.hid.bRawData[21] has keycode
                switch (raw->data.hid.bRawData[20]) {
                    case 0x0033:
                        nMceCmd = MCE_DETAILS;
                        break;
                    case 0x0022:
                        nMceCmd = MCE_GUIDE;
                        break;
                    case 0x0036:
                        nMceCmd = MCE_MYTV;
                        break;
                    case 0x0026:
                        nMceCmd = MCE_RECORDEDTV;
                        break;
                    case 0x0005:
                        nMceCmd = MCE_RED;
                        break;
                    case 0x0002:
                        nMceCmd = MCE_GREEN;
                        break;
                    case 0x0045:
                        nMceCmd = MCE_YELLOW;
                        break;
                    case 0x0046:
                        nMceCmd = MCE_BLUE;
                        break;
                    case 0x000A:
                        nMceCmd = MCE_MEDIA_PREVIOUSTRACK;
                        break;
                    case 0x004A:
                        nMceCmd = MCE_MEDIA_NEXTTRACK;
                        break;
                }
            }
        }
        delete [] pRawBuffer;
    }

    return nMceCmd;
}

void CMPlayerCApp::RegisterHotkeys()
{
    CAutoVectorPtr<RAWINPUTDEVICELIST> inputDeviceList;
    UINT nInputDeviceCount = 0, nErrCode;
    RID_DEVICE_INFO deviceInfo;
    RAWINPUTDEVICE MCEInputDevice[] = {
        // usUsagePage     usUsage         dwFlags     hwndTarget
        {  0xFFBC,         0x88,           0,          nullptr},
        {  0x000C,         0x01,           0,          nullptr},
        {  0x000C,         0x80,           0,          nullptr}
    };

    // Register MCE Remote Control raw input
    for (unsigned int i = 0; i < _countof(MCEInputDevice); i++) {
        MCEInputDevice[i].hwndTarget = m_pMainWnd->m_hWnd;
    }

    // Get the size of the device list
    nErrCode = GetRawInputDeviceList(nullptr, &nInputDeviceCount, sizeof(RAWINPUTDEVICELIST));
    inputDeviceList.Attach(new RAWINPUTDEVICELIST[nInputDeviceCount]);
    if (nErrCode == UINT(-1) || !nInputDeviceCount || !inputDeviceList) {
        ASSERT(nErrCode != UINT(-1));
        return;
    }

    nErrCode = GetRawInputDeviceList(inputDeviceList, &nInputDeviceCount, sizeof(RAWINPUTDEVICELIST));
    if (nErrCode == UINT(-1)) {
        ASSERT(FALSE);
        return;
    }

    for (UINT i = 0; i < nInputDeviceCount; i++) {
        UINT nTemp = deviceInfo.cbSize = sizeof(deviceInfo);

        if (GetRawInputDeviceInfo(inputDeviceList[i].hDevice, RIDI_DEVICEINFO, &deviceInfo, &nTemp) > 0) {
            if (deviceInfo.hid.dwVendorId == 0x00000471 &&         // Philips HID vendor id
                    deviceInfo.hid.dwProductId == 0x00000617) {    // IEEE802.15.4 RF Dongle (SRM 7500)
                MCEInputDevice[0].usUsagePage = deviceInfo.hid.usUsagePage;
                MCEInputDevice[0].usUsage = deviceInfo.hid.usUsage;
                GetRemoteControlCode = GetRemoteControlCodeSRM7500;
            }
        }
    }

    RegisterRawInputDevices(MCEInputDevice, _countof(MCEInputDevice), sizeof(RAWINPUTDEVICE));

    if (m_s.fGlobalMedia) {
        POSITION pos = m_s.wmcmds.GetHeadPosition();

        while (pos) {
            wmcmd& wc = m_s.wmcmds.GetNext(pos);
            if (wc.appcmd != 0) {
                RegisterHotKey(m_pMainWnd->m_hWnd, wc.appcmd, 0, GetVKFromAppCommand(wc.appcmd));
            }
        }
    }
}

void CMPlayerCApp::UnregisterHotkeys()
{
    if (m_s.fGlobalMedia) {
        POSITION pos = m_s.wmcmds.GetHeadPosition();

        while (pos) {
            wmcmd& wc = m_s.wmcmds.GetNext(pos);
            if (wc.appcmd != 0) {
                UnregisterHotKey(m_pMainWnd->m_hWnd, wc.appcmd);
            }
        }
    }
}

UINT CMPlayerCApp::GetVKFromAppCommand(UINT nAppCommand)
{
    switch (nAppCommand) {
        case APPCOMMAND_BROWSER_BACKWARD:
            return VK_BROWSER_BACK;
        case APPCOMMAND_BROWSER_FORWARD:
            return VK_BROWSER_FORWARD;
        case APPCOMMAND_BROWSER_REFRESH:
            return VK_BROWSER_REFRESH;
        case APPCOMMAND_BROWSER_STOP:
            return VK_BROWSER_STOP;
        case APPCOMMAND_BROWSER_SEARCH:
            return VK_BROWSER_SEARCH;
        case APPCOMMAND_BROWSER_FAVORITES:
            return VK_BROWSER_FAVORITES;
        case APPCOMMAND_BROWSER_HOME:
            return VK_BROWSER_HOME;
        case APPCOMMAND_VOLUME_MUTE:
            return VK_VOLUME_MUTE;
        case APPCOMMAND_VOLUME_DOWN:
            return VK_VOLUME_DOWN;
        case APPCOMMAND_VOLUME_UP:
            return VK_VOLUME_UP;
        case APPCOMMAND_MEDIA_NEXTTRACK:
            return VK_MEDIA_NEXT_TRACK;
        case APPCOMMAND_MEDIA_PREVIOUSTRACK:
            return VK_MEDIA_PREV_TRACK;
        case APPCOMMAND_MEDIA_STOP:
            return VK_MEDIA_STOP;
        case APPCOMMAND_MEDIA_PLAY_PAUSE:
            return VK_MEDIA_PLAY_PAUSE;
        case APPCOMMAND_LAUNCH_MAIL:
            return VK_LAUNCH_MAIL;
        case APPCOMMAND_LAUNCH_MEDIA_SELECT:
            return VK_LAUNCH_MEDIA_SELECT;
        case APPCOMMAND_LAUNCH_APP1:
            return VK_LAUNCH_APP1;
        case APPCOMMAND_LAUNCH_APP2:
            return VK_LAUNCH_APP2;
    }

    return 0;
}

int CMPlayerCApp::ExitInstance()
{
    m_s.SaveSettings();

    OleUninitialize();

    return CWinApp::ExitInstance();
}

/////////////////////////////////////////////////////////////////////////////
// CMPlayerCApp message handlers
// App command to run the dialog

void CMPlayerCApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}

void CMPlayerCApp::OnFileExit()
{
    m_fClosingState = true;
    OnAppExit();
}

// CRemoteCtrlClient

CRemoteCtrlClient::CRemoteCtrlClient()
    : m_pWnd(nullptr)
    , m_nStatus(DISCONNECTED)
{
}

void CRemoteCtrlClient::SetHWND(HWND hWnd)
{
    CAutoLock cAutoLock(&m_csLock);

    m_pWnd = CWnd::FromHandle(hWnd);
}

void CRemoteCtrlClient::Connect(CString addr)
{
    CAutoLock cAutoLock(&m_csLock);

    if (m_nStatus == CONNECTING && m_addr == addr) {
        TRACE(_T("CRemoteCtrlClient (Connect): already connecting to %s\n"), addr);
        return;
    }

    if (m_nStatus == CONNECTED && m_addr == addr) {
        TRACE(_T("CRemoteCtrlClient (Connect): already connected to %s\n"), addr);
        return;
    }

    m_nStatus = CONNECTING;

    TRACE(_T("CRemoteCtrlClient (Connect): connecting to %s\n"), addr);

    Close();

    Create();

    CString ip = addr.Left(addr.Find(':') + 1).TrimRight(':');
    int port = _tcstol(addr.Mid(addr.Find(':') + 1), nullptr, 10);

    __super::Connect(ip, port);

    m_addr = addr;
}

void CRemoteCtrlClient::DisConnect()
{
    CAutoLock cAutoLock(&m_csLock);

    ShutDown(2);
    Close();
}

void CRemoteCtrlClient::OnConnect(int nErrorCode)
{
    CAutoLock cAutoLock(&m_csLock);

    m_nStatus = (nErrorCode == 0 ? CONNECTED : DISCONNECTED);

    TRACE(_T("CRemoteCtrlClient (OnConnect): %d\n"), nErrorCode);
}

void CRemoteCtrlClient::OnClose(int nErrorCode)
{
    CAutoLock cAutoLock(&m_csLock);

    if (m_hSocket != INVALID_SOCKET && m_nStatus == CONNECTED) {
        TRACE(_T("CRemoteCtrlClient (OnClose): connection lost\n"));
    }

    m_nStatus = DISCONNECTED;

    TRACE(_T("CRemoteCtrlClient (OnClose): %d\n"), nErrorCode);
}

void CRemoteCtrlClient::OnReceive(int nErrorCode)
{
    if (nErrorCode != 0 || !m_pWnd) {
        return;
    }

    CStringA str;
    int ret = Receive(str.GetBuffer(256), 255, 0);
    if (ret <= 0) {
        return;
    }
    str.ReleaseBuffer(ret);

    TRACE(_T("CRemoteCtrlClient (OnReceive): %s\n"), CString(str));

    OnCommand(str);

    __super::OnReceive(nErrorCode);
}

void CRemoteCtrlClient::ExecuteCommand(CStringA cmd, int repcnt)
{
    cmd.Trim();
    if (cmd.IsEmpty()) {
        return;
    }
    cmd.Replace(' ', '_');

    const CAppSettings& s = AfxGetAppSettings();

    POSITION pos = s.wmcmds.GetHeadPosition();
    while (pos) {
        wmcmd wc = s.wmcmds.GetNext(pos);
        CStringA name = TToA(wc.GetName());
        name.Replace(' ', '_');
        if ((repcnt == 0 && wc.rmrepcnt == 0 || wc.rmrepcnt > 0 && (repcnt % wc.rmrepcnt) == 0)
                && (!name.CompareNoCase(cmd) || !wc.rmcmd.CompareNoCase(cmd) || wc.cmd == (WORD)strtol(cmd, nullptr, 10))) {
            CAutoLock cAutoLock(&m_csLock);
            TRACE(_T("CRemoteCtrlClient (calling command): %s\n"), wc.GetName());
            m_pWnd->SendMessage(WM_COMMAND, wc.cmd);
            break;
        }
    }
}

// CWinLircClient

CWinLircClient::CWinLircClient()
{
}

void CWinLircClient::OnCommand(CStringA str)
{
    TRACE(_T("CWinLircClient (OnCommand): %s\n"), CString(str));

    int i = 0, j = 0, repcnt = 0;
    for (CStringA token = str.Tokenize(" ", i);
            !token.IsEmpty();
            token = str.Tokenize(" ", i), j++) {
        if (j == 1) {
            repcnt = strtol(token, nullptr, 16);
        } else if (j == 2) {
            ExecuteCommand(token, repcnt);
        }
    }
}

// CUIceClient

CUIceClient::CUIceClient()
{
}

void CUIceClient::OnCommand(CStringA str)
{
    TRACE(_T("CUIceClient (OnCommand): %s\n"), CString(str));

    CStringA cmd;
    int i = 0, j = 0;
    for (CStringA token = str.Tokenize("|", i);
            !token.IsEmpty();
            token = str.Tokenize("|", i), j++) {
        if (j == 0) {
            cmd = token;
        } else if (j == 1) {
            ExecuteCommand(cmd, strtol(token, nullptr, 16));
        }
    }
}

void CMPlayerCApp::OnHelpShowcommandlineswitches()
{
    ShowCmdlnSwitches();
}

//
void GetCurDispMode(dispmode& dm, CString& DisplayName)
{
    HDC hDC;
    CString DisplayName1 = DisplayName;
    if ((DisplayName == _T("Current")) || DisplayName.IsEmpty()) {
        CMonitor monitor;
        CMonitors monitors;
        monitor = monitors.GetNearestMonitor(AfxGetApp()->m_pMainWnd);
        monitor.GetName(DisplayName1);
    }
    hDC = CreateDC(DisplayName1, nullptr, nullptr, nullptr);
    if (hDC) {
        dm.fValid = true;
        dm.size = CSize(GetDeviceCaps(hDC, HORZRES), GetDeviceCaps(hDC, VERTRES));
        dm.bpp = GetDeviceCaps(hDC, BITSPIXEL);
        dm.freq = GetDeviceCaps(hDC, VREFRESH);
        DeleteDC(hDC);
    }
}

bool GetDispMode(int i, dispmode& dm, CString& DisplayName)
{
    DEVMODE devmode;
    CString DisplayName1 = DisplayName;
    devmode.dmSize = sizeof(DEVMODE);
    if ((DisplayName == _T("Current")) || DisplayName.IsEmpty()) {
        CMonitor monitor;
        CMonitors monitors;
        monitor = monitors.GetNearestMonitor(AfxGetApp()->m_pMainWnd);
        monitor.GetName(DisplayName1);
    }
    if (!EnumDisplaySettings(DisplayName1, i, &devmode)) {
        return false;
    }
    dm.fValid = true;
    dm.size = CSize(devmode.dmPelsWidth, devmode.dmPelsHeight);
    dm.bpp = devmode.dmBitsPerPel;
    dm.freq = devmode.dmDisplayFrequency;
    dm.dmDisplayFlags = devmode.dmDisplayFlags;
    return true;
}

void SetDispMode(const dispmode& dm, CString& DisplayName)
{
    dispmode dm1;
    GetCurDispMode(dm1, DisplayName);
    if ((dm.size == dm1.size) && (dm.bpp == dm1.bpp) && (dm.freq == dm1.freq)) {
        return;
    }

    if (!dm.fValid) {
        return;
    }
    DEVMODE dmScreenSettings;
    memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
    dmScreenSettings.dmSize = sizeof(dmScreenSettings);
    dmScreenSettings.dmPelsWidth = dm.size.cx;
    dmScreenSettings.dmPelsHeight = dm.size.cy;
    dmScreenSettings.dmBitsPerPel = dm.bpp;
    dmScreenSettings.dmDisplayFrequency = dm.freq;
    dmScreenSettings.dmDisplayFlags = dm.dmDisplayFlags;
    dmScreenSettings.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY  | DM_DISPLAYFLAGS;
    CString DisplayName1 = DisplayName;
    if ((DisplayName == _T("Current")) || DisplayName.IsEmpty()) {
        CMonitor monitor;
        CMonitors monitors;
        monitor = monitors.GetNearestMonitor(AfxGetApp()->m_pMainWnd);
        monitor.GetName(DisplayName1);
    }
    if (AfxGetAppSettings().fRestoreResAfterExit) {
        ChangeDisplaySettingsEx(DisplayName1, &dmScreenSettings, nullptr, CDS_FULLSCREEN, nullptr);
    } else {
        ChangeDisplaySettingsEx(DisplayName1, &dmScreenSettings, nullptr, 0, nullptr);
    }
}

void SetAudioRenderer(int AudioDevNo)
{
    CStringArray m_AudioRendererDisplayNames;
    AfxGetMyApp()->m_AudioRendererDisplayName_CL = _T("");
    m_AudioRendererDisplayNames.Add(_T(""));
    int i = 2;

    BeginEnumSysDev(CLSID_AudioRendererCategory, pMoniker) {
        LPOLESTR olestr = nullptr;
        if (FAILED(pMoniker->GetDisplayName(0, 0, &olestr))) {
            continue;
        }
        CStringW str(olestr);
        CoTaskMemFree(olestr);
        m_AudioRendererDisplayNames.Add(CString(str));
        i++;
    }
    EndEnumSysDev;

    m_AudioRendererDisplayNames.Add(AUDRNDT_NULL_COMP);
    m_AudioRendererDisplayNames.Add(AUDRNDT_NULL_UNCOMP);
    m_AudioRendererDisplayNames.Add(AUDRNDT_MPC);
    i += 3;
    if (AudioDevNo >= 1 && AudioDevNo <= i) {
        AfxGetMyApp()->m_AudioRendererDisplayName_CL = m_AudioRendererDisplayNames[AudioDevNo - 1];
    }
}

void SetHandCursor(HWND m_hWnd, UINT nID)
{
    SetClassLongPtr(GetDlgItem(m_hWnd, nID), GCLP_HCURSOR, (LONG_PTR)AfxGetApp()->LoadStandardCursor(IDC_HAND));
}


typedef CAtlRegExp<CAtlRECharTraits> CAtlRegExpT;
typedef CAtlREMatchContext<CAtlRECharTraits> CAtlREMatchContextT;

bool FindRedir(CUrl& src, CString ct, CString& body, CAtlList<CString>& urls, CAutoPtrList<CAtlRegExpT>& res)
{
    POSITION pos = res.GetHeadPosition();
    while (pos) {
        CAtlRegExpT* re = res.GetNext(pos);

        CAtlREMatchContextT mc;
        const CAtlREMatchContextT::RECHAR* s = (LPCTSTR)body;
        const CAtlREMatchContextT::RECHAR* e = nullptr;
        for (; s && re->Match(s, &mc, &e); s = e) {
            const CAtlREMatchContextT::RECHAR* szStart = 0;
            const CAtlREMatchContextT::RECHAR* szEnd = 0;
            mc.GetMatch(0, &szStart, &szEnd);

            CString url;
            url.Format(_T("%.*s"), szEnd - szStart, szStart);
            url.Trim();

            if (url.CompareNoCase(_T("asf path")) == 0) {
                continue;
            }

            CUrl dst;
            dst.CrackUrl(CString(url));
            if (_tcsicmp(src.GetSchemeName(), dst.GetSchemeName())
                    || _tcsicmp(src.GetHostName(), dst.GetHostName())
                    || _tcsicmp(src.GetUrlPath(), dst.GetUrlPath())) {
                urls.AddTail(url);
            } else {
                // recursive
                urls.RemoveAll();
                break;
            }
        }
    }

    return !urls.IsEmpty();
}

bool FindRedir(CString& fn, CString ct, CAtlList<CString>& fns, CAutoPtrList<CAtlRegExpT>& res)
{
    CString body;

    CTextFile f(CTextFile::ANSI);
    if (f.Open(fn)) for (CString tmp; f.ReadString(tmp); body += tmp + '\n') {
            ;
        }

    CString dir = fn.Left(max(fn.ReverseFind('/'), fn.ReverseFind('\\')) + 1); // "ReverseFindOneOf"

    POSITION pos = res.GetHeadPosition();
    while (pos) {
        CAtlRegExpT* re = res.GetNext(pos);

        CAtlREMatchContextT mc;
        const CAtlREMatchContextT::RECHAR* s = (LPCTSTR)body;
        const CAtlREMatchContextT::RECHAR* e = nullptr;
        for (; s && re->Match(s, &mc, &e); s = e) {
            const CAtlREMatchContextT::RECHAR* szStart = 0;
            const CAtlREMatchContextT::RECHAR* szEnd = 0;
            mc.GetMatch(0, &szStart, &szEnd);

            CString fn2;
            fn2.Format(_T("%.*s"), szEnd - szStart, szStart);
            fn2.Trim();

            if (!fn2.CompareNoCase(_T("asf path"))) {
                continue;
            }
            if (fn2.Find(_T("EXTM3U")) == 0 || fn2.Find(_T("#EXTINF")) == 0) {
                continue;
            }

            if (fn2.Find(_T(":")) < 0 && fn2.Find(_T("\\\\")) != 0 && fn2.Find(_T("//")) != 0) {
                CPath p;
                p.Combine(dir, fn2);
                fn2 = (LPCTSTR)p;
            }

            if (!fn2.CompareNoCase(fn)) {
                continue;
            }

            fns.AddTail(fn2);
        }
    }

    return !fns.IsEmpty();
}

CStringA GetContentType(CString fn, CAtlList<CString>* redir)
{
    CUrl url;
    CString ct, body;

    if (fn.Find(_T("://")) >= 0) {
        url.CrackUrl(fn);

        if (_tcsicmp(url.GetSchemeName(), _T("pnm")) == 0) {
            return "audio/x-pn-realaudio";
        }

        if (_tcsicmp(url.GetSchemeName(), _T("mms")) == 0) {
            return "video/x-ms-asf";
        }

        if (_tcsicmp(url.GetSchemeName(), _T("http")) != 0) {
            return "";
        }

        DWORD ProxyEnable = 0;
        CString ProxyServer;
        DWORD ProxyPort = 0;
        ULONG len = 256 + 1;
        CRegKey key;

        if (ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"), KEY_READ)
                && ERROR_SUCCESS == key.QueryDWORDValue(_T("ProxyEnable"), ProxyEnable) && ProxyEnable
                && ERROR_SUCCESS == key.QueryStringValue(_T("ProxyServer"), ProxyServer.GetBufferSetLength(256), &len)) {
            ProxyServer.ReleaseBufferSetLength(len);

            CAtlList<CString> sl;
            ProxyServer = Explode(ProxyServer, sl, ';');
            if (sl.GetCount() > 1) {
                POSITION pos = sl.GetHeadPosition();
                while (pos) {
                    CAtlList<CString> sl2;
                    if (!Explode(sl.GetNext(pos), sl2, '=', 2).CompareNoCase(_T("http"))
                            && sl2.GetCount() == 2) {
                        ProxyServer = sl2.GetTail();
                        break;
                    }
                }
            }

            ProxyServer = Explode(ProxyServer, sl, ':');
            if (sl.GetCount() > 1) {
                ProxyPort = _tcstol(sl.GetTail(), nullptr, 10);
            }
        }

        CSocket s;
        s.Create();
        if (s.Connect(
                    ProxyEnable ? ProxyServer : url.GetHostName(),
                    ProxyEnable ? ProxyPort : url.GetPortNumber())) {
            CStringA host = CStringA(url.GetHostName());
            CStringA path = CStringA(url.GetUrlPath()) + CStringA(url.GetExtraInfo());

            if (ProxyEnable) {
                path = "http://" + host + path;
            }

            CStringA hdr;
            hdr.Format(
                "GET %s HTTP/1.0\r\n"
                "User-Agent: Media Player Classic\r\n"
                "Host: %s\r\n"
                "Accept: */*\r\n"
                "\r\n", path, host);

            // MessageBox(nullptr, CString(hdr), _T("Sending..."), MB_OK);

            if (s.Send((LPCSTR)hdr, hdr.GetLength()) < hdr.GetLength()) {
                return "";
            }

            hdr.Empty();
            for (;;) {
                CStringA str;
                str.ReleaseBuffer(s.Receive(str.GetBuffer(256), 256)); // SOCKET_ERROR == -1, also suitable for ReleaseBuffer
                if (str.IsEmpty()) {
                    break;
                }
                hdr += str;
                int hdrend = hdr.Find("\r\n\r\n");
                if (hdrend >= 0) {
                    body = hdr.Mid(hdrend + 4);
                    hdr = hdr.Left(hdrend);
                    break;
                }
            }

            // MessageBox(nullptr, CString(hdr), _T("Received..."), MB_OK);

            CAtlList<CStringA> sl;
            Explode(hdr, sl, '\n');
            POSITION pos = sl.GetHeadPosition();
            while (pos) {
                CStringA& hdrline = sl.GetNext(pos);
                CAtlList<CStringA> sl2;
                Explode(hdrline, sl2, ':', 2);
                CStringA field = sl2.RemoveHead().MakeLower();
                if (field == "location" && !sl2.IsEmpty()) {
                    return GetContentType(CString(sl2.GetHead()), redir);
                }
                if (field == "content-type" && !sl2.IsEmpty()) {
                    ct = sl2.GetHead();
                }
            }

            while (body.GetLength() < 256) {
                CStringA str;
                str.ReleaseBuffer(s.Receive(str.GetBuffer(256), 256)); // SOCKET_ERROR == -1, also suitable for ReleaseBuffer
                if (str.IsEmpty()) {
                    break;
                }
                body += str;
            }

            if (body.GetLength() >= 8) {
                CStringA str = TToA(body);
                if (!strncmp((LPCSTR)str, ".ra", 3)) {
                    return "audio/x-pn-realaudio";
                }
                if (!strncmp((LPCSTR)str, ".RMF", 4)) {
                    return "audio/x-pn-realaudio";
                }
                if (*(DWORD*)(LPCSTR)str == 0x75b22630) {
                    return "video/x-ms-wmv";
                }
                if (!strncmp((LPCSTR)str + 4, "moov", 4)) {
                    return "video/quicktime";
                }
            }

            if (redir && (ct == _T("audio/x-scpls") || ct == _T("audio/x-mpegurl"))) {
                while (body.GetLength() < 4 * 1024) { // should be enough for a playlist...
                    CStringA str;
                    str.ReleaseBuffer(s.Receive(str.GetBuffer(256), 256)); // SOCKET_ERROR == -1, also suitable for ReleaseBuffer
                    if (str.IsEmpty()) {
                        break;
                    }
                    body += str;
                }
            }
        }
    } else if (!fn.IsEmpty()) {
        CPath p(fn);
        CString ext = p.GetExtension().MakeLower();
        if (ext == _T(".asx")) {
            ct = _T("video/x-ms-asf");
        } else if (ext == _T(".pls")) {
            ct = _T("audio/x-scpls");
        } else if (ext == _T(".m3u") || ext == _T(".m3u8")) {
            ct = _T("audio/x-mpegurl");
        } else if (ext == _T(".qtl")) {
            ct = _T("application/x-quicktimeplayer");
        } else if (ext == _T(".mpcpl")) {
            ct = _T("application/x-mpc-playlist");
        } else if (ext == _T(".bdmv")) {
            ct = _T("application/x-bdmv-playlist");
        }

        FILE* f = nullptr;
        if (!_tfopen_s(&f, fn, _T("rb"))) {
            CStringA str;
            str.ReleaseBufferSetLength((int)fread(str.GetBuffer(10240), 1, 10240, f));
            body = AToT(str);
            fclose(f);
        }
    }

    if (body.GetLength() >= 4) { // here only those which cannot be opened through dshow
        CStringA str = TToA(body);
        if (!strncmp((LPCSTR)str, ".ra", 3)) {
            return "audio/x-pn-realaudio";
        }
        if (!strncmp((LPCSTR)str, "FWS", 3)) {
            return "application/x-shockwave-flash";
        }

    }

    if (redir && !ct.IsEmpty()) {
        CAutoPtrList<CAtlRegExpT> res;
        CAutoPtr<CAtlRegExpT> re;

        if (ct == _T("video/x-ms-asf")) {
            // ...://..."/>
            re.Attach(DEBUG_NEW CAtlRegExpT());
            if (re && REPARSE_ERROR_OK == re->Parse(_T("{[a-zA-Z]+://[^\n\">]*}"), FALSE)) {
                res.AddTail(re);
            }
            // Ref#n= ...://...\n
            re.Attach(DEBUG_NEW CAtlRegExpT());
            if (re && REPARSE_ERROR_OK == re->Parse(_T("Ref\\z\\b*=\\b*[\"]*{([a-zA-Z]+://[^\n\"]+}"), FALSE)) {
                res.AddTail(re);
            }
        } else if (ct == _T("audio/x-scpls")) {
            // File1=...\n
            re.Attach(DEBUG_NEW CAtlRegExp<>());
            if (re && REPARSE_ERROR_OK == re->Parse(_T("file\\z\\b*=\\b*[\"]*{[^\n\"]+}"), FALSE)) {
                res.AddTail(re);
            }
        } else if (ct == _T("audio/x-mpegurl")) {
            // #comment
            // ...
            re.Attach(DEBUG_NEW CAtlRegExp<>());
            if (re && REPARSE_ERROR_OK == re->Parse(_T("{[^#][^\n]+}"), FALSE)) {
                res.AddTail(re);
            }
        } else if (ct == _T("audio/x-pn-realaudio")) {
            // rtsp://...
            re.Attach(DEBUG_NEW CAtlRegExp<>());
            if (re && REPARSE_ERROR_OK == re->Parse(_T("{rtsp://[^\n]+}"), FALSE)) {
                res.AddTail(re);
            }
        }

        if (!body.IsEmpty()) {
            if (fn.Find(_T("://")) >= 0) {
                FindRedir(url, ct, body, *redir, res);
            } else {
                FindRedir(fn, ct, *redir, res);
            }
        }
    }

    return TToA(ct);
}

COLORPROPERTY_RANGE* CMPlayerCApp::GetColorControl(ControlType nFlag)
{
    switch (nFlag) {
        case ProcAmp_Brightness:
            return &m_ColorControl[0];
        case ProcAmp_Contrast:
            return &m_ColorControl[1];
        case ProcAmp_Hue:
            return &m_ColorControl[2];
        case ProcAmp_Saturation:
            return &m_ColorControl[3];
    }
    return nullptr;
}

void CMPlayerCApp::ResetColorControlRange()
{
    m_ColorControl[0].dwProperty   = ProcAmp_Brightness;
    m_ColorControl[0].MinValue     = -100;
    m_ColorControl[0].MaxValue     = 100;
    m_ColorControl[0].DefaultValue = 0;
    m_ColorControl[0].StepSize     = 1;
    m_ColorControl[1].dwProperty   = ProcAmp_Contrast;
    m_ColorControl[1].MinValue     = -100;
    m_ColorControl[1].MaxValue     = 100;
    m_ColorControl[1].DefaultValue = 0;
    m_ColorControl[1].StepSize     = 1;
    m_ColorControl[2].dwProperty   = ProcAmp_Hue;
    m_ColorControl[2].MinValue     = -180;
    m_ColorControl[2].MaxValue     = 180;
    m_ColorControl[2].DefaultValue = 0;
    m_ColorControl[2].StepSize     = 1;
    m_ColorControl[3].dwProperty   = ProcAmp_Saturation;
    m_ColorControl[3].MinValue     = -100;
    m_ColorControl[3].MaxValue     = 100;
    m_ColorControl[3].DefaultValue = 0;
    m_ColorControl[3].StepSize     = 1;
}

void CMPlayerCApp::UpdateColorControlRange(bool isEVR)
{
    if (isEVR) {
        // Brightness
        m_ColorControl[0].MinValue      = FixedToInt(m_EVRColorControl[0].MinValue);
        m_ColorControl[0].MaxValue      = FixedToInt(m_EVRColorControl[0].MaxValue);
        m_ColorControl[0].DefaultValue  = FixedToInt(m_EVRColorControl[0].DefaultValue);
        m_ColorControl[0].StepSize      = max(1, FixedToInt(m_EVRColorControl[0].StepSize));
        // Contrast
        m_ColorControl[1].MinValue      = FixedToInt(m_EVRColorControl[1].MinValue, 100) - 100;
        m_ColorControl[1].MaxValue      = FixedToInt(m_EVRColorControl[1].MaxValue, 100) - 100;
        m_ColorControl[1].DefaultValue  = FixedToInt(m_EVRColorControl[1].DefaultValue, 100) - 100;
        m_ColorControl[1].StepSize      = max(1, FixedToInt(m_EVRColorControl[1].StepSize, 100));
        // Hue
        m_ColorControl[2].MinValue      = FixedToInt(m_EVRColorControl[2].MinValue);
        m_ColorControl[2].MaxValue      = FixedToInt(m_EVRColorControl[2].MaxValue);
        m_ColorControl[2].DefaultValue  = FixedToInt(m_EVRColorControl[2].DefaultValue);
        m_ColorControl[2].StepSize      = max(1, FixedToInt(m_EVRColorControl[2].StepSize));
        // Saturation
        m_ColorControl[3].MinValue      = FixedToInt(m_EVRColorControl[3].MinValue, 100) - 100;
        m_ColorControl[3].MaxValue      = FixedToInt(m_EVRColorControl[3].MaxValue, 100) - 100;
        m_ColorControl[3].DefaultValue  = FixedToInt(m_EVRColorControl[3].DefaultValue, 100) - 100;
        m_ColorControl[3].StepSize      = max(1, FixedToInt(m_EVRColorControl[3].StepSize, 100));
    } else {
        // Brightness
        m_ColorControl[0].MinValue      = (int)floor(m_VMR9ColorControl[0].MinValue + 0.5);
        m_ColorControl[0].MaxValue      = (int)floor(m_VMR9ColorControl[0].MaxValue + 0.5);
        m_ColorControl[0].DefaultValue  = (int)floor(m_VMR9ColorControl[0].DefaultValue + 0.5);
        m_ColorControl[0].StepSize      = max(1, (int)(m_VMR9ColorControl[0].StepSize + 0.5));
        // Contrast
        /*if (m_VMR9ColorControl[1].MinValue == 0.0999908447265625) {
              m_VMR9ColorControl[1].MinValue = 0.11;    //fix nvidia bug
          }*/
        if (*(int*)&m_VMR9ColorControl[1].MinValue == 1036830720) {
            m_VMR9ColorControl[1].MinValue = 0.11f;    //fix nvidia bug
        }
        m_ColorControl[1].MinValue      = (int)floor(m_VMR9ColorControl[1].MinValue * 100 + 0.5) - 100;
        m_ColorControl[1].MaxValue      = (int)floor(m_VMR9ColorControl[1].MaxValue * 100 + 0.5) - 100;
        m_ColorControl[1].DefaultValue  = (int)floor(m_VMR9ColorControl[1].DefaultValue * 100 + 0.5) - 100;
        m_ColorControl[1].StepSize      = max(1, (int)(m_VMR9ColorControl[1].StepSize * 100 + 0.5));
        // Hue
        m_ColorControl[2].MinValue      = (int)floor(m_VMR9ColorControl[2].MinValue + 0.5);
        m_ColorControl[2].MaxValue      = (int)floor(m_VMR9ColorControl[2].MaxValue + 0.5);
        m_ColorControl[2].DefaultValue  = (int)floor(m_VMR9ColorControl[2].DefaultValue + 0.5);
        m_ColorControl[2].StepSize      = max(1, (int)(m_VMR9ColorControl[2].StepSize + 0.5));
        // Saturation
        m_ColorControl[3].MinValue      = (int)floor(m_VMR9ColorControl[3].MinValue * 100 + 0.5) - 100;
        m_ColorControl[3].MaxValue      = (int)floor(m_VMR9ColorControl[3].MaxValue * 100 + 0.5) - 100;
        m_ColorControl[3].DefaultValue  = (int)floor(m_VMR9ColorControl[3].DefaultValue * 100 + 0.5) - 100;
        m_ColorControl[3].StepSize      = max(1, (int)(m_VMR9ColorControl[3].StepSize * 100 + 0.5));
    }

    // Brightness
    if (m_ColorControl[0].MinValue < -100) {
        m_ColorControl[0].MinValue = -100;
    }
    if (m_ColorControl[0].MaxValue > 100) {
        m_ColorControl[0].MaxValue = 100;
    }
    // Contrast
    if (m_ColorControl[1].MinValue < -100) {
        m_ColorControl[1].MinValue = -100;
    }
    if (m_ColorControl[1].MaxValue > 100) {
        m_ColorControl[1].MaxValue = 100;
    }
    // Hue
    if (m_ColorControl[2].MinValue < -180) {
        m_ColorControl[2].MinValue = -180;
    }
    if (m_ColorControl[2].MaxValue > 180) {
        m_ColorControl[2].MaxValue = 180;
    }
    // Saturation
    if (m_ColorControl[3].MinValue < -100) {
        m_ColorControl[3].MinValue = -100;
    }
    if (m_ColorControl[3].MaxValue > 100) {
        m_ColorControl[3].MaxValue = 100;
    }
}

VMR9ProcAmpControlRange* CMPlayerCApp::GetVMR9ColorControl(ControlType nFlag)
{
    switch (nFlag) {
        case ProcAmp_Brightness:
            return &m_VMR9ColorControl[0];
        case ProcAmp_Contrast:
            return &m_VMR9ColorControl[1];
        case ProcAmp_Hue:
            return &m_VMR9ColorControl[2];
        case ProcAmp_Saturation:
            return &m_VMR9ColorControl[3];
    }
    return nullptr;
}

DXVA2_ValueRange* CMPlayerCApp::GetEVRColorControl(ControlType nFlag)
{
    switch (nFlag) {
        case ProcAmp_Brightness:
            return &m_EVRColorControl[0];
        case ProcAmp_Contrast:
            return &m_EVRColorControl[1];
        case ProcAmp_Hue:
            return &m_EVRColorControl[2];
        case ProcAmp_Saturation:
            return &m_EVRColorControl[3];
    }
    return nullptr;
}

const LanguageResource& CMPlayerCApp::GetLanguageResourceByResourceID(UINT resourceID)
{
    size_t defaultResource = 0;

    for (size_t i = 0; i < languageResourcesCount; i++) {
        if (resourceID == languageResources[i].resourceID) {
            return languageResources[i];
        } else if (ID_LANGUAGE_ENGLISH == languageResources[i].resourceID) {
            defaultResource = i;
        }
    }

    return languageResources[defaultResource];
}

const LanguageResource& CMPlayerCApp::GetLanguageResourceByLocaleID(LANGID localeID)
{
    size_t defaultResource = 0;

    for (size_t i = 0; i < languageResourcesCount; i++) {
        if (localeID == languageResources[i].localeID) {
            return languageResources[i];
        } else if (0 == languageResources[i].localeID) {
            defaultResource = i;
        }
    }

    return languageResources[defaultResource];
}

void CMPlayerCApp::SetDefaultLanguage()
{
    const LanguageResource& languageResource = GetLanguageResourceByLocaleID(GetUserDefaultUILanguage());

    // Try to set the language resource but don't fail if it can't be loaded
    // English will we used instead in case of error
    SetLanguage(languageResource, false);
}

LRESULT CALLBACK RTLWindowsLayoutCbtFilterHook(int code, WPARAM wParam, LPARAM lParam)
{
    if (code == HCBT_CREATEWND) {
        //LPCREATESTRUCT lpcs = ((LPCBT_CREATEWND)lParam)->lpcs;

        //if ((lpcs->style & WS_CHILD) == 0)
        //  lpcs->dwExStyle |= WS_EX_LAYOUTRTL; // doesn't seem to have any effect, but shouldn't hurt

        HWND hWnd = (HWND)wParam;
        if ((GetWindowLongPtr(hWnd, GWL_STYLE) & WS_CHILD) == 0) {
            SetWindowLongPtr(hWnd, GWL_EXSTYLE, GetWindowLongPtr(hWnd, GWL_EXSTYLE) | WS_EX_LAYOUTRTL);
        }
    }
    return CallNextHookEx(nullptr, code, wParam, lParam);
}

bool CMPlayerCApp::SetLanguage(const LanguageResource& languageResource, bool showErrorMsg /*= true*/)
{
    CAppSettings& s = AfxGetAppSettings();
    HMODULE hMod = nullptr;
    bool success = false;

    // Try to load the resource dll if any
    if (languageResource.dllPath) {
        hMod = LoadLibrary(languageResource.dllPath);
        if (hMod == nullptr) { // The dll failed to load for some reason
            if (showErrorMsg) {
                MessageBox(nullptr, _T("Error loading the chosen language.\n\nPlease reinstall MPC-HC."),
                           _T("Media Player Classic - Home Cinema"), MB_ICONWARNING | MB_OK);
            }
        } else { // Check if the version of the resource dll is correct
            CString strSatVersion = CFileVersionInfo::GetFileVersionStr(languageResource.dllPath);

            const CString& v = AfxGetMyApp()->m_strVersion;
            CString strNeededVersion = v.Left(v.ReverseFind(_T('.')) + 1) + _T('0');

            if (strSatVersion == strNeededVersion) {
                s.language = languageResource.localeID;
                success = true;
            }

            if (!success) { // The version wasn't correct
                if (showErrorMsg) {
                    // This message should stay in English!
                    int sel = MessageBox(nullptr, _T("Your language pack will not work with this version.\n\nDo you want to visit the download page to get a full package including the translations?"),
                                         _T("Media Player Classic - Home Cinema"), MB_ICONWARNING | MB_YESNO);
                    if (sel == IDYES) {
                        ShellExecute(nullptr, _T("open"), DOWNLOAD_URL, nullptr, nullptr, SW_SHOWDEFAULT);
                    }
                }
                // Free the loaded resource dll
                FreeLibrary(hMod);
                hMod = nullptr;
            }
        }
    }

    // In case no dll was loaded, load the English translation from the executable
    if (hMod == nullptr) {
        hMod = AfxGetApp()->m_hInstance;
        s.language = 0;
        // If a resource dll was supposed to be loaded we had an error
        success = (languageResource.dllPath == nullptr);
    }
    // In case a dll was loaded, check if some special action is needed
    else if (languageResource.resourceID == ID_LANGUAGE_HEBREW) {
        // Hebrew needs the RTL flag.
        SetProcessDefaultLayout(LAYOUT_RTL);
        SetWindowsHookEx(WH_CBT, RTLWindowsLayoutCbtFilterHook, nullptr, GetCurrentThreadId());
    }

    // Free the old resource if it was a dll
    if (AfxGetResourceHandle() != AfxGetApp()->m_hInstance) {
        FreeLibrary(AfxGetResourceHandle());
    }
    // Set the new resource
    AfxSetResourceHandle(hMod);

    return success;
}

void CMPlayerCApp::RunAsAdministrator(LPCTSTR strCommand, LPCTSTR strArgs, bool bWaitProcess)
{
    SHELLEXECUTEINFO execinfo;
    memset(&execinfo, 0, sizeof(execinfo));
    execinfo.lpFile = strCommand;
    execinfo.cbSize = sizeof(execinfo);
    execinfo.lpVerb = _T("runas");
    execinfo.fMask  = SEE_MASK_NOCLOSEPROCESS;
    execinfo.nShow  = SW_SHOWDEFAULT;
    execinfo.lpParameters = strArgs;

    ShellExecuteEx(&execinfo);

    if (bWaitProcess) {
        WaitForSingleObject(execinfo.hProcess, INFINITE);
    }
}

CRenderersData* GetRenderersData()
{
    return &AfxGetMyApp()->m_Renderers;
}

CRenderersSettings& GetRenderersSettings()
{
    return AfxGetAppSettings().m_RenderersSettings;
}
