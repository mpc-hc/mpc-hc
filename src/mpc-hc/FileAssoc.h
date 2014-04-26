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

#pragma once

#include <afxwin.h>
#include "MediaFormats.h"

#include <mutex>
#include <thread>

class CFileAssoc
{
private:
    typedef int (*GetIconIndexFunc)(LPCTSTR);
    typedef UINT(*GetIconLibVersionFunc)();

    static CString GetOpenCommand();
    static CString GetEnqueueCommand();

    IApplicationAssociationRegistration* CreateRegistrationManager();

    bool SetFileAssociation(CString strExt, CString strProgID, bool bRegister);

    std::mutex m_checkIconsAssocMutex;
    std::thread m_checkIconsAssocThread;
    void RunCheckIconsAssocThread();

    static CString m_iconLibPath;
    static HMODULE m_hIconLib;
    static GetIconIndexFunc GetIconIndex;
    static GetIconLibVersionFunc GetIconLibVersion;

    static LPCTSTR strRegisteredAppName;
    static LPCTSTR strOldAssocKey;
    static LPCTSTR strRegisteredAppKey;
    static LPCTSTR strRegAppFileAssocKey;

    static bool m_bNoRecentDocs;

    static const CString strOpenCommand;
    static const CString strEnqueueCommand;

    static CComPtr<IApplicationAssociationRegistration> m_pAAR;

public:
    enum reg_state_t {
        NOT_REGISTERED,
        SOME_REGISTERED,
        ALL_REGISTERED
    };
    enum autoplay_t {
        AP_VIDEO,
        AP_MUSIC,
        AP_AUDIOCD,
        AP_DVDMOVIE
    };

    CFileAssoc() = default;
    CFileAssoc(const CFileAssoc&) = delete;
    CFileAssoc& operator=(const CFileAssoc&) = delete;
    ~CFileAssoc();

    bool LoadIconLib();
    bool FreeIconLib();
    bool SaveIconLibVersion();

    void SetNoRecentDocs(bool bNoRecentDocs, bool bUpdateAssocs = false);

    bool RegisterApp();

    bool Register(CString ext, CString strLabel, bool bRegister, bool bRegisterContextMenuEntries, bool bAssociatedWithIcon);
    bool IsRegistered(CString ext);
    bool AreRegisteredFileContextMenuEntries(CString strExt);

    bool Register(const CMediaFormatCategory& mfc, bool bRegister, bool bRegisterContextMenuEntries, bool bAssociatedWithIcon);
    reg_state_t IsRegistered(const CMediaFormatCategory& mfc);
    reg_state_t AreRegisteredFileContextMenuEntries(const CMediaFormatCategory& mfc);

    bool RegisterFolderContextMenuEntries(bool bRegister);
    bool AreRegisteredFolderContextMenuEntries();

    bool RegisterAutoPlay(autoplay_t ap, bool bRegister);
    bool IsAutoPlayRegistered(autoplay_t ap);

    bool GetAssociatedExtensions(const CMediaFormats& mf, CAtlList<CString>& exts);
    bool GetAssociatedExtensionsFromRegistry(CAtlList<CString>& exts);

    bool ReAssocIcons(const CAtlList<CString>& exts);

    void CheckIconsAssoc();

    bool ShowWindowsAssocDialog();
};
