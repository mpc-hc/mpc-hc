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

#pragma once

#include <afxwin.h>


class CFileAssoc
{
private:
    CFileAssoc();

    static CString GetOpenCommand();
    static CString GetEnqueueCommand();

    static IApplicationAssociationRegistration* CreateRegistrationManager();

    static bool SetFileAssociation(CString strExt, CString strProgID, bool bRegister);

    static LPCTSTR strRegisteredAppName;
    static LPCTSTR strOldAssocKey;
    static LPCTSTR strRegisteredAppKey;

    static const CString strOpenCommand;
    static const CString strEnqueueCommand;

    static CComPtr<IApplicationAssociationRegistration> m_pAAR;

public:
    enum reg_state_t { NOT_REGISTERED, SOME_REGISTERED, ALL_REGISTERED };
    enum autoplay_t { AP_VIDEO, AP_MUSIC, AP_AUDIOCD, AP_DVDMOVIE };

    static bool RegisterApp();

    static bool Register(CString ext, CString strLabel, bool bRegister, bool bRegisterContextMenuEntries, bool bAssociatedWithIcon);
    static bool IsRegistered(CString ext);
    static bool AreRegisteredFileContextMenuEntries(CString strExt);

    static bool Register(CMediaFormatCategory& mfc, bool bRegister, bool bRegisterContextMenuEntries, bool bAssociatedWithIcon);
    static reg_state_t IsRegistered(CMediaFormatCategory& mfc);
    static reg_state_t AreRegisteredFileContextMenuEntries(CMediaFormatCategory& mfc);

    static bool RegisterFolderContextMenuEntries(bool bRegister);
    static bool AreRegisteredFolderContextMenuEntries();

    static void RegisterAutoPlay(autoplay_t ap, bool bRegister);
    static bool IsAutoPlayRegistered(autoplay_t ap);
};
