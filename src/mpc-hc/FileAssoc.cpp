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
#include "FileAssoc.h"


// TODO: change this along with the root key for settings and the mutex name to
//       avoid possible risks of conflict with the old MPC (non HC version).
#ifdef _WIN64
#define PROGID _T("mplayerc64")
#else
#define PROGID _T("mplayerc")
#endif // _WIN64

LPCTSTR CFileAssoc::strRegisteredAppName = _T("Media Player Classic");
LPCTSTR CFileAssoc::strOldAssocKey       = _T("PreviousRegistration");
LPCTSTR CFileAssoc::strRegisteredAppKey  = _T("Software\\Clients\\Media\\Media Player Classic\\Capabilities");

const CString CFileAssoc::strOpenCommand    = CFileAssoc::GetOpenCommand();
const CString CFileAssoc::strEnqueueCommand = CFileAssoc::GetEnqueueCommand();

CComPtr<IApplicationAssociationRegistration> CFileAssoc::m_pAAR;

static CString GetProgramDir()
{
    CString RtnVal;
    TCHAR    FileName[_MAX_PATH];
    ::GetModuleFileName(AfxGetInstanceHandle(), FileName, _MAX_PATH);
    RtnVal = FileName;
    RtnVal = RtnVal.Left(RtnVal.ReverseFind('\\'));
    return RtnVal;
}

static int FileExists(const TCHAR* fileName)
{
    DWORD fileAttr;
    fileAttr = ::GetFileAttributes(fileName);
    if (0xFFFFFFFF == fileAttr) {
        return false;
    }
    return true;
}

typedef int (*GetIconIndexFunc)(CString);

static int GetIconIndex(CString ext)
{
    int iconindex = -1;
    GetIconIndexFunc _getIconIndexFunc;
    HINSTANCE mpciconlib = LoadLibrary(_T("mpciconlib.dll"));
    if (mpciconlib) {
        _getIconIndexFunc = (GetIconIndexFunc) GetProcAddress(mpciconlib, "get_icon_index");
        if (_getIconIndexFunc) {
            iconindex = _getIconIndexFunc(ext);
        }
        FreeLibrary(mpciconlib);
    }

    return iconindex;
}

CString CFileAssoc::GetOpenCommand()
{
    CString path;
    TCHAR buff[_MAX_PATH];

    if (::GetModuleFileName(AfxGetInstanceHandle(), buff, _MAX_PATH) == 0) {
        return _T("");
    }

    path = buff;
    return _T("\"") + path + _T("\" \"%1\"");
}

CString CFileAssoc::GetEnqueueCommand()
{
    CString path;
    TCHAR buff[_MAX_PATH];

    if (::GetModuleFileName(AfxGetInstanceHandle(), buff, _MAX_PATH) == 0) {
        return _T("");
    }

    path = buff;
    return _T("\"") + path + _T("\" /add \"%1\"");
}

IApplicationAssociationRegistration* CFileAssoc::CreateRegistrationManager()
{
    IApplicationAssociationRegistration* pAAR = NULL;

    // Default manager (requires at least Vista)
    HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistration,
                                  NULL,
                                  CLSCTX_INPROC,
                                  __uuidof(IApplicationAssociationRegistration),
                                  (LPVOID*)&pAAR);
    UNREFERENCED_PARAMETER(hr);

    return pAAR;
}

bool CFileAssoc::RegisterApp()
{
    if (!m_pAAR) {
        m_pAAR = CFileAssoc::CreateRegistrationManager();
    }

    if (m_pAAR) {
        CString AppIcon = _T("");
        TCHAR buff[_MAX_PATH];

        if (::GetModuleFileName(AfxGetInstanceHandle(), buff, _MAX_PATH)) {
            AppIcon = buff;
            AppIcon = "\"" + AppIcon + "\"";
            AppIcon += _T(",0");
        }

        // Register MPC for the windows "Default application" manager
        CRegKey key;

        if (ERROR_SUCCESS == key.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\RegisteredApplications"))) {
            key.SetStringValue(_T("Media Player Classic"), strRegisteredAppKey);

            if (ERROR_SUCCESS != key.Create(HKEY_LOCAL_MACHINE, strRegisteredAppKey)) {
                return false;
            }

            // ==>>  TODO icon !!!
            key.SetStringValue(_T("ApplicationDescription"), ResStr(IDS_APP_DESCRIPTION), REG_EXPAND_SZ);
            key.SetStringValue(_T("ApplicationIcon"), AppIcon, REG_EXPAND_SZ);
            key.SetStringValue(_T("ApplicationName"), ResStr(IDR_MAINFRAME), REG_EXPAND_SZ);
        }
    }
    return true;
}

bool CFileAssoc::Register(CString ext, CString strLabel, bool bRegister, bool bRegisterContextMenuEntries, bool bAssociatedWithIcon)
{
    CRegKey key;
    bool    bSetValue;
    CString strProgID = PROGID + ext;

    if (!bRegister) {
        if (bRegister != IsRegistered(ext)) {
            SetFileAssociation(ext, strProgID, bRegister);
        }
        key.Attach(HKEY_CLASSES_ROOT);
        key.RecurseDeleteKey(strProgID);
        return true;
    }

    bSetValue = bRegister || (ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\open\\command"), KEY_READ));

    // Create ProgID for this file type
    if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID)) {
        return false;
    }
    if (ERROR_SUCCESS != key.SetStringValue(NULL, strLabel)) {
        return false;
    }

    // Add to playlist option
    if (bRegisterContextMenuEntries) {
        if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\enqueue"))) {
            return false;
        }
        if (ERROR_SUCCESS != key.SetStringValue(NULL, ResStr(IDS_ADD_TO_PLAYLIST))) {
            return false;
        }

        if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\enqueue\\command"))) {
            return false;
        }
        if (bSetValue && (ERROR_SUCCESS != key.SetStringValue(NULL, strEnqueueCommand))) {
            return false;
        }
    } else {
        key.Close();
        key.Attach(HKEY_CLASSES_ROOT);
        key.RecurseDeleteKey(strProgID + _T("\\shell\\enqueue"));
    }

    // Play option
    if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\open"))) {
        return false;
    }
    if (bRegisterContextMenuEntries) {
        if (ERROR_SUCCESS != key.SetStringValue(NULL, ResStr(IDS_OPEN_WITH_MPC))) {
            return false;
        }
    } else {
        if (ERROR_SUCCESS != key.SetStringValue(NULL, _T(""))) {
            return false;
        }
    }

    if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\open\\command"))) {
        return false;
    }
    if (bSetValue && (ERROR_SUCCESS != key.SetStringValue(NULL, strOpenCommand))) {
        return false;
    }

    if (ERROR_SUCCESS != key.Create(HKEY_LOCAL_MACHINE, CString(strRegisteredAppKey) + _T("\\FileAssociations"))) {
        return false;
    }
    if (ERROR_SUCCESS != key.SetStringValue(ext, strProgID)) {
        return false;
    }

    if (bAssociatedWithIcon) {
        CString AppIcon = _T("");
        TCHAR buff[_MAX_PATH];

        CString mpciconlib = GetProgramDir() + _T("\\mpciconlib.dll");

        if (FileExists(mpciconlib)) {
            int icon_index = GetIconIndex(ext);
            CString m_typeicon = mpciconlib;

            /* icon_index value -1 means no icon was found in the iconlib for the file extension */
            if ((icon_index >= 0) && ExtractIcon(AfxGetApp()->m_hInstance, (LPCWSTR)m_typeicon, icon_index)) {
                m_typeicon = "\"" + mpciconlib + "\"";
                AppIcon.Format(_T("%s,%d"), m_typeicon, icon_index);
            }
        }

        /* no icon was found for the file extension, so use MPC's icon */
        if ((AppIcon.IsEmpty()) && (::GetModuleFileName(AfxGetInstanceHandle(), buff, _MAX_PATH))) {
            AppIcon = buff;
            AppIcon = "\"" + AppIcon + "\"";
            AppIcon += _T(",0");
        }

        if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\DefaultIcon"))) {
            return false;
        }
        if (bSetValue && (ERROR_SUCCESS != key.SetStringValue(NULL, AppIcon))) {
            return false;
        }
    } else {
        key.Attach(HKEY_CLASSES_ROOT);
        key.RecurseDeleteKey(strProgID + _T("\\DefaultIcon"));
    }

    if (bRegister != IsRegistered(ext)) {
        SetFileAssociation(ext, strProgID, bRegister);
    }

    return true;
}

bool CFileAssoc::SetFileAssociation(CString strExt, CString strProgID, bool bRegister)
{
    CString extoldreg, extOldIcon;
    CRegKey key;
    HRESULT hr = S_OK;
    TCHAR   buff[256];
    ULONG   len = _countof(buff);
    memset(buff, 0, sizeof(buff));

    if (!m_pAAR) {
        m_pAAR = CFileAssoc::CreateRegistrationManager();
    }

    if (m_pAAR) {
        // The Vista way
        CString strNewApp;
        if (bRegister) {
            // Create non existing file type
            if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strExt)) {
                return false;
            }

            WCHAR*      pszCurrentAssociation;
            // Save current application associated
            if (SUCCEEDED(m_pAAR->QueryCurrentDefault(strExt, AT_FILEEXTENSION, AL_EFFECTIVE, &pszCurrentAssociation))) {
                if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID)) {
                    return false;
                }

                key.SetStringValue(strOldAssocKey, pszCurrentAssociation);

                // Get current icon for file type
                /*
                if (ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, CString(pszCurrentAssociation) + _T("\\DefaultIcon")))
                {
                    len = sizeof(buff);
                    memset(buff, 0, len);
                    if (ERROR_SUCCESS == key.QueryStringValue(NULL, buff, &len) && !CString(buff).Trim().IsEmpty())
                    {
                        if (ERROR_SUCCESS == key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\DefaultIcon")))
                            key.SetStringValue (NULL, buff);
                    }
                }
                */
                CoTaskMemFree(pszCurrentAssociation);
            }
            strNewApp = strRegisteredAppName;
        } else {
            if (ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, strProgID)) {
                return false;
            }

            if (ERROR_SUCCESS == key.QueryStringValue(strOldAssocKey, buff, &len)) {
                strNewApp = buff;
            }

            // TODO : retrieve registered app name from previous association (or find Bill function for that...)
        }

        hr = m_pAAR->SetAppAsDefault(strNewApp, strExt, AT_FILEEXTENSION);
    } else {
        // The 2000/XP way
        if (bRegister) {
            // Set new association
            if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strExt)) {
                return false;
            }

            len = _countof(buff);
            memset(buff, 0, sizeof(buff));
            if (ERROR_SUCCESS == key.QueryStringValue(NULL, buff, &len) && !CString(buff).Trim().IsEmpty()) {
                extoldreg = buff;
            }
            if (ERROR_SUCCESS != key.SetStringValue(NULL, strProgID)) {
                return false;
            }

            // Get current icon for file type
            /*
            if (!extoldreg.IsEmpty())
            {
                if (ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, extoldreg + _T("\\DefaultIcon")))
                {
                    len = sizeof(buff);
                    memset(buff, 0, len);
                    if (ERROR_SUCCESS == key.QueryStringValue(NULL, buff, &len) && !CString(buff).Trim().IsEmpty())
                        extOldIcon = buff;
                }
            }
            */

            // Save old association
            if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID)) {
                return false;
            }
            key.SetStringValue(strOldAssocKey, extoldreg);

            /*
            if (!extOldIcon.IsEmpty() && (ERROR_SUCCESS == key.Create(HKEY_CLASSES_ROOT, strProgID + _T("\\DefaultIcon"))))
                key.SetStringValue (NULL, extOldIcon);
            */
        } else {
            // Get previous association
            len = _countof(buff);
            memset(buff, 0, sizeof(buff));
            if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strProgID)) {
                return false;
            }
            if (ERROR_SUCCESS == key.QueryStringValue(strOldAssocKey, buff, &len) && !CString(buff).Trim().IsEmpty()) {
                extoldreg = buff;
            }

            // Set previous association
            if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, strExt)) {
                return false;
            }
            key.SetStringValue(NULL, extoldreg);
        }
    }

    return SUCCEEDED(hr);
}

bool CFileAssoc::IsRegistered(CString ext)
{
    HRESULT hr;
    BOOL    bIsDefault = FALSE;
    CString strProgID = PROGID + ext;

    if (!m_pAAR) {
        m_pAAR = CFileAssoc::CreateRegistrationManager();
    }

    if (m_pAAR) {
        // The Vista way
        hr = m_pAAR->QueryAppIsDefault(ext, AT_FILEEXTENSION, AL_EFFECTIVE, strRegisteredAppName, &bIsDefault);
    } else {
        // The 2000/XP way
        CRegKey key;
        TCHAR   buff[256];
        ULONG   len = _countof(buff);
        memset(buff, 0, sizeof(buff));

        if (ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT, ext)) {
            return false;
        }

        if (ERROR_SUCCESS != key.QueryStringValue(NULL, buff, &len) && !CString(buff).Trim().IsEmpty()) {
            return false;
        }

        bIsDefault = (buff == strProgID);
    }

    // Check if association is for this instance of MPC
    if (bIsDefault) {
        CRegKey key;
        TCHAR   buff[_MAX_PATH];
        ULONG   len = _countof(buff);

        bIsDefault = FALSE;
        if (ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\open\\command"), KEY_READ)) {
            if (ERROR_SUCCESS == key.QueryStringValue(NULL, buff, &len)) {
                bIsDefault = (strOpenCommand.CompareNoCase(CString(buff)) == 0);
            }
        }
    }

    return !!bIsDefault;
}

bool CFileAssoc::AreRegisteredFileContextMenuEntries(CString strExt)
{
    CRegKey key;
    TCHAR   buff[_MAX_PATH];
    ULONG   len = _countof(buff);
    CString strProgID = PROGID + strExt;
    bool    registered = false;

    if (ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, strProgID + _T("\\shell\\open"), KEY_READ)) {
        CString strCommand = ResStr(IDS_OPEN_WITH_MPC);
        if (ERROR_SUCCESS == key.QueryStringValue(NULL, buff, &len)) {
            registered = (strCommand.CompareNoCase(CString(buff)) == 0);
        }
    }

    return registered;
}

bool CFileAssoc::Register(CMediaFormatCategory& mfc, bool bRegister, bool bRegisterContextMenuEntries, bool bAssociatedWithIcon)
{
    CAtlList<CString> exts;
    ExplodeMin(mfc.GetExtsWithPeriod(), exts, ' ');

    CString strLabel = mfc.GetDescription();
    bool res = true;

    POSITION pos = exts.GetHeadPosition();
    while (pos) {
        res &= Register(exts.GetNext(pos), strLabel, bRegister, bRegisterContextMenuEntries, bAssociatedWithIcon);
    }

    return res;
}

CFileAssoc::reg_state_t CFileAssoc::IsRegistered(CMediaFormatCategory& mfc)
{
    CAtlList<CString> exts;
    ExplodeMin(mfc.GetExtsWithPeriod(), exts, ' ');

    size_t cnt = 0;

    POSITION pos = exts.GetHeadPosition();
    while (pos) {
        if (CFileAssoc::IsRegistered(exts.GetNext(pos))) {
            cnt++;
        }
    }

    reg_state_t res;
    if (cnt == 0) {
        res = NOT_REGISTERED;
    } else if (cnt == exts.GetCount()) {
        res = ALL_REGISTERED;
    } else {
        res = SOME_REGISTERED;
    }

    return res;
}

CFileAssoc::reg_state_t CFileAssoc::AreRegisteredFileContextMenuEntries(CMediaFormatCategory& mfc)
{
    CAtlList<CString> exts;
    ExplodeMin(mfc.GetExtsWithPeriod(), exts, ' ');

    size_t cnt = 0;

    POSITION pos = exts.GetHeadPosition();
    while (pos) {
        if (CFileAssoc::AreRegisteredFileContextMenuEntries(exts.GetNext(pos))) {
            cnt++;
        }
    }

    reg_state_t res;
    if (cnt == 0) {
        res = NOT_REGISTERED;
    } else if (cnt == exts.GetCount()) {
        res = ALL_REGISTERED;
    } else {
        res = SOME_REGISTERED;
    }

    return res;
}

bool CFileAssoc::RegisterFolderContextMenuEntries(bool bRegister)
{
    CRegKey key;
    bool success;

    if (bRegister) {
        success = false;

        if (ERROR_SUCCESS == key.Create(HKEY_CLASSES_ROOT, _T("Directory\\shell\\") PROGID _T(".enqueue"))) {
            key.SetStringValue(NULL, ResStr(IDS_ADD_TO_PLAYLIST));

            if (ERROR_SUCCESS == key.Create(HKEY_CLASSES_ROOT, _T("Directory\\shell\\") PROGID _T(".enqueue\\command"))) {
                key.SetStringValue(NULL, strEnqueueCommand);
                success = true;
            }
        }

        if (success && ERROR_SUCCESS == key.Create(HKEY_CLASSES_ROOT, _T("Directory\\shell\\") PROGID _T(".play"))) {
            success = false;

            key.SetStringValue(NULL, ResStr(IDS_OPEN_WITH_MPC));

            if (ERROR_SUCCESS == key.Create(HKEY_CLASSES_ROOT, _T("Directory\\shell\\") PROGID _T(".play\\command"))) {
                key.SetStringValue(NULL, strOpenCommand);
                success = true;
            }
        }

    } else {
        key.Attach(HKEY_CLASSES_ROOT);
        success  = (ERROR_SUCCESS == key.RecurseDeleteKey(_T("Directory\\shell\\") PROGID _T(".enqueue")));
        success &= (ERROR_SUCCESS == key.RecurseDeleteKey(_T("Directory\\shell\\") PROGID _T(".play")));
    }

    return success;
}

bool CFileAssoc::AreRegisteredFolderContextMenuEntries()
{
    CRegKey key;
    TCHAR   buff[_MAX_PATH];
    ULONG   len = _countof(buff);
    bool    registered = false;

    if (ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, _T("Directory\\shell\\") PROGID _T(".play\\command"), KEY_READ)) {
        if (ERROR_SUCCESS == key.QueryStringValue(NULL, buff, &len)) {
            registered = (strOpenCommand.CompareNoCase(CString(buff)) == 0);
        }
    }

    return registered;
}

static struct {
    LPCSTR verb, cmd;
    UINT action;
} handlers[] = {
    {"VideoFiles", " %1", IDS_AUTOPLAY_PLAYVIDEO},
    {"MusicFiles", " %1", IDS_AUTOPLAY_PLAYMUSIC},
    {"CDAudio", " %1 /cd", IDS_AUTOPLAY_PLAYAUDIOCD},
    {"DVDMovie", " %1 /dvd", IDS_AUTOPLAY_PLAYDVDMOVIE},
};

void CFileAssoc::RegisterAutoPlay(autoplay_t ap, bool bRegister)
{
    TCHAR buff[_MAX_PATH];
    if (::GetModuleFileName(AfxGetInstanceHandle(), buff, _MAX_PATH) == 0) {
        return;
    }
    CString exe = buff;

    int i = (int)ap;
    if (i < 0 || i >= _countof(handlers)) {
        return;
    }

    CRegKey key;

    if (bRegister) {
        if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT, _T("MediaPlayerClassic.Autorun"))) {
            return;
        }
        key.Close();

        if (ERROR_SUCCESS != key.Create(HKEY_CLASSES_ROOT,
                                        CString(CStringA("MediaPlayerClassic.Autorun\\Shell\\Play") + handlers[i].verb + "\\Command"))) {
            return;
        }
        key.SetStringValue(NULL, _T("\"") + exe + _T("\"") + handlers[i].cmd);
        key.Close();

        if (ERROR_SUCCESS != key.Create(HKEY_LOCAL_MACHINE,
                                        CString(CStringA("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoplayHandlers\\Handlers\\MPCPlay") + handlers[i].verb + "OnArrival"))) {
            return;
        }
        key.SetStringValue(_T("Action"), ResStr(handlers[i].action));
        key.SetStringValue(_T("Provider"), _T("Media Player Classic"));
        key.SetStringValue(_T("InvokeProgID"), _T("MediaPlayerClassic.Autorun"));
        key.SetStringValue(_T("InvokeVerb"), CString(CStringA("Play") + handlers[i].verb));
        key.SetStringValue(_T("DefaultIcon"), exe + _T(",0"));
        key.Close();

        if (ERROR_SUCCESS != key.Create(HKEY_LOCAL_MACHINE,
                                        CString(CStringA("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoplayHandlers\\EventHandlers\\Play") + handlers[i].verb + "OnArrival"))) {
            return;
        }
        key.SetStringValue(CString(CStringA("MPCPlay") + handlers[i].verb + "OnArrival"), _T(""));
        key.Close();
    } else {
        if (ERROR_SUCCESS != key.Create(HKEY_LOCAL_MACHINE,
                                        CString(CStringA("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoplayHandlers\\EventHandlers\\Play") + handlers[i].verb + "OnArrival"))) {
            return;
        }
        key.DeleteValue(CString(CStringA("MPCPlay") + handlers[i].verb + "OnArrival"));
        key.Close();
    }
}

bool CFileAssoc::IsAutoPlayRegistered(autoplay_t ap)
{
    ULONG len;
    TCHAR buff[_MAX_PATH];
    if (::GetModuleFileName(AfxGetInstanceHandle(), buff, _MAX_PATH) == 0) {
        return false;
    }
    CString exe = buff;

    int i = (int)ap;
    if (i < 0 || i >= _countof(handlers)) {
        return false;
    }

    CRegKey key;

    if (ERROR_SUCCESS != key.Open(HKEY_LOCAL_MACHINE,
                                  CString(CStringA("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\AutoplayHandlers\\EventHandlers\\Play") + handlers[i].verb + "OnArrival"),
                                  KEY_READ)) {
        return false;
    }
    len = _countof(buff);
    if (ERROR_SUCCESS != key.QueryStringValue(
                CString(_T("MPCPlay")) + handlers[i].verb + _T("OnArrival"),
                buff, &len)) {
        return false;
    }
    key.Close();

    if (ERROR_SUCCESS != key.Open(HKEY_CLASSES_ROOT,
                                  CString(CStringA("MediaPlayerClassic.Autorun\\Shell\\Play") + handlers[i].verb + "\\Command"),
                                  KEY_READ)) {
        return false;
    }
    len = _countof(buff);
    if (ERROR_SUCCESS != key.QueryStringValue(NULL, buff, &len)) {
        return false;
    }
    if (_tcsnicmp(_T("\"") + exe, buff, exe.GetLength() + 1)) {
        return false;
    }
    key.Close();

    return true;
}
