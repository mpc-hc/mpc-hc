/********************************************************************
*
* Copyright (C) 1999-2000 Sven Wiegand
* Copyright (C) 2000-2001 ToolsCenter
*
* This file is free software; you can redistribute it and/or
* modify, but leave the headers intact and do not remove any
* copyrights from the source.
*
* If you have further questions, suggestions or bug fixes, visit
* our homepage
*
*    http://www.ToolsCenter.org
*
********************************************************************/

#include "stdafx.h"
#include "FileVersionInfo.h"

//-------------------------------------------------------------------
// CFileVersionInfo
//-------------------------------------------------------------------

CFileVersionInfo::CFileVersionInfo()
{
    Reset();
}


CFileVersionInfo::~CFileVersionInfo()
{}


BOOL CFileVersionInfo::GetTranslationId(LPVOID lpData, UINT unBlockSize, WORD wLangId, DWORD &dwId, BOOL bPrimaryEnough/*= FALSE*/)
{
    LPWORD lpwData;
    for (lpwData = (LPWORD)lpData; (LPBYTE)lpwData < ((LPBYTE)lpData)+unBlockSize; lpwData+=2)
    {
        if (*lpwData == wLangId)
        {
            dwId = *((DWORD*)lpwData);
            return TRUE;
        }
    }

    if (!bPrimaryEnough)
        return FALSE;

    for (lpwData = (LPWORD)lpData; (LPBYTE)lpwData < ((LPBYTE)lpData)+unBlockSize; lpwData+=2)
    {
        if (((*lpwData)&0x00FF) == (wLangId&0x00FF))
        {
            dwId = *((DWORD*)lpwData);
            return TRUE;
        }
    }

    return FALSE;
}


BOOL CFileVersionInfo::Create(HMODULE hModule /*= NULL*/)
{
    CString	strPath;

    GetModuleFileName(hModule, strPath.GetBuffer(_MAX_PATH), _MAX_PATH);
    strPath.ReleaseBuffer();
    return Create(strPath);
}


BOOL CFileVersionInfo::Create(LPCTSTR lpszFileName)
{
    Reset();

    DWORD	dwHandle;
    DWORD	dwFileVersionInfoSize = GetFileVersionInfoSize((LPTSTR)lpszFileName, &dwHandle);
    if (!dwFileVersionInfoSize)
        return FALSE;

    LPVOID	lpData = (LPVOID)DNew BYTE[dwFileVersionInfoSize];
    if (!lpData)
        return FALSE;

    try
    {
        if (!GetFileVersionInfo((LPTSTR)lpszFileName, dwHandle, dwFileVersionInfoSize, lpData))
            throw FALSE;

        // catch default information
        LPVOID	lpInfo;
        UINT		unInfoLen;
        if (VerQueryValue(lpData, _T("\\"), &lpInfo, &unInfoLen))
        {
            ASSERT(unInfoLen == sizeof(m_FileInfo));
            if (unInfoLen == sizeof(m_FileInfo))
                memcpy(&m_FileInfo, lpInfo, unInfoLen);
        }

        // find best matching language and codepage
        VerQueryValue(lpData, _T("\\VarFileInfo\\Translation"), &lpInfo, &unInfoLen);

        DWORD	dwLangCode = 0;
        if (!GetTranslationId(lpInfo, unInfoLen, GetUserDefaultLangID(), dwLangCode, FALSE))
        {
            if (!GetTranslationId(lpInfo, unInfoLen, GetUserDefaultLangID(), dwLangCode, TRUE))
            {
                if (!GetTranslationId(lpInfo, unInfoLen, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), dwLangCode, TRUE))
                {
                    if (!GetTranslationId(lpInfo, unInfoLen, MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL), dwLangCode, TRUE))
                        // use the first one we can get
                        dwLangCode = *((DWORD*)lpInfo);
                }
            }
        }


        CString	strSubBlock;
        strSubBlock.Format(_T("\\StringFileInfo\\%04X%04X\\"), dwLangCode&0x0000FFFF, (dwLangCode&0xFFFF0000)>>16);

        // catch string table
        if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock+_T("CompanyName")), &lpInfo, &unInfoLen))
            m_strCompanyName = CString((LPCTSTR)lpInfo);
        if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock+_T("FileDescription")), &lpInfo, &unInfoLen))
            m_strFileDescription = CString((LPCTSTR)lpInfo);
        if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock+_T("FileVersion")), &lpInfo, &unInfoLen))
            m_strFileVersion = CString((LPCTSTR)lpInfo);
        if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock+_T("InternalName")), &lpInfo, &unInfoLen))
            m_strInternalName = CString((LPCTSTR)lpInfo);
        if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock+_T("LegalCopyright")), &lpInfo, &unInfoLen))
            m_strLegalCopyright = CString((LPCTSTR)lpInfo);
        if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock+_T("OriginalFileName")), &lpInfo, &unInfoLen))
            m_strOriginalFileName = CString((LPCTSTR)lpInfo);
        if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock+_T("ProductName")), &lpInfo, &unInfoLen))
            m_strProductName = CString((LPCTSTR)lpInfo);
        if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock+_T("ProductVersion")), &lpInfo, &unInfoLen))
            m_strProductVersion = CString((LPCTSTR)lpInfo);
        if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock+_T("Comments")), &lpInfo, &unInfoLen))
            m_strComments = CString((LPCTSTR)lpInfo);
        if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock+_T("LegalTrademarks")), &lpInfo, &unInfoLen))
            m_strLegalTrademarks = CString((LPCTSTR)lpInfo);
        if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock+_T("PrivateBuild")), &lpInfo, &unInfoLen))
            m_strPrivateBuild = CString((LPCTSTR)lpInfo);
        if (VerQueryValue(lpData, (LPTSTR)(LPCTSTR)(strSubBlock+_T("SpecialBuild")), &lpInfo, &unInfoLen))
            m_strSpecialBuild = CString((LPCTSTR)lpInfo);

        delete[] lpData;
    }
    catch (BOOL)
    {
        delete[] lpData;
        return FALSE;
    }

    return TRUE;
}


WORD CFileVersionInfo::GetFileVersion(int nIndex) const
{
	switch(nIndex)
	{
	case 0:
		return (WORD)(m_FileInfo.dwFileVersionLS & 0x0000FFFF);
	case 1:
		return (WORD)((m_FileInfo.dwFileVersionLS & 0xFFFF0000) >> 16);
	case 2:
		return (WORD)(m_FileInfo.dwFileVersionMS & 0x0000FFFF);
	case 3:
        return (WORD)((m_FileInfo.dwFileVersionMS & 0xFFFF0000) >> 16);
	default:
        return 0;
	}
}

CString CFileVersionInfo::GetFileVersionEx() const
{
    CString		strTemp;

    strTemp.Format (L"%d.%d.%d.%d",
                    (m_FileInfo.dwFileVersionMS & 0xFFFF0000) >> 16,
                    (m_FileInfo.dwFileVersionMS & 0x0000FFFF),
                    (m_FileInfo.dwFileVersionLS & 0xFFFF0000) >> 16,
                    m_FileInfo.dwFileVersionLS & 0x0000FFFF);
    return strTemp;
}


WORD CFileVersionInfo::GetProductVersion(int nIndex) const
{
	switch(nIndex)
	{
	case 0:
		return (WORD)(m_FileInfo.dwProductVersionLS & 0x0000FFFF);
	case 1:
		return (WORD)((m_FileInfo.dwProductVersionLS & 0xFFFF0000) >> 16);
	case 2:
		return (WORD)(m_FileInfo.dwProductVersionMS & 0x0000FFFF);
	case 3:
		return (WORD)((m_FileInfo.dwProductVersionMS & 0xFFFF0000) >> 16);
	default:
		return 0;
	}
}


DWORD CFileVersionInfo::GetFileFlagsMask() const
{
    return m_FileInfo.dwFileFlagsMask;
}


DWORD CFileVersionInfo::GetFileFlags() const
{
    return m_FileInfo.dwFileFlags;
}


DWORD CFileVersionInfo::GetFileOs() const
{
    return m_FileInfo.dwFileOS;
}


DWORD CFileVersionInfo::GetFileType() const
{
    return m_FileInfo.dwFileType;
}


DWORD CFileVersionInfo::GetFileSubtype() const
{
    return m_FileInfo.dwFileSubtype;
}


CTime CFileVersionInfo::GetFileDate() const
{
    FILETIME	ft;
    ft.dwLowDateTime = m_FileInfo.dwFileDateLS;
    ft.dwHighDateTime = m_FileInfo.dwFileDateMS;
    return CTime(ft);
}


CString CFileVersionInfo::GetCompanyName() const
{
    return m_strCompanyName;
}


CString CFileVersionInfo::GetFileDescription() const
{
    return m_strFileDescription;
}


CString CFileVersionInfo::GetFileVersion() const
{
    return m_strFileVersion;
}


CString CFileVersionInfo::GetInternalName() const
{
    return m_strInternalName;
}


CString CFileVersionInfo::GetLegalCopyright() const
{
    return m_strLegalCopyright;
}


CString CFileVersionInfo::GetOriginalFileName() const
{
    return m_strOriginalFileName;
}


CString CFileVersionInfo::GetProductName() const
{
    return m_strProductName;
}


CString CFileVersionInfo::GetProductVersion() const
{
    return m_strProductVersion;
}


CString CFileVersionInfo::GetComments() const
{
    return m_strComments;
}


CString CFileVersionInfo::GetLegalTrademarks() const
{
    return m_strLegalTrademarks;
}


CString CFileVersionInfo::GetPrivateBuild() const
{
    return m_strPrivateBuild;
}


CString CFileVersionInfo::GetSpecialBuild() const
{
    return m_strSpecialBuild;
}


void CFileVersionInfo::Reset()
{
    ZeroMemory(&m_FileInfo, sizeof(m_FileInfo));
    m_strCompanyName.Empty();
    m_strFileDescription.Empty();
    m_strFileVersion.Empty();
    m_strInternalName.Empty();
    m_strLegalCopyright.Empty();
    m_strOriginalFileName.Empty();
    m_strProductName.Empty();
    m_strProductVersion.Empty();
    m_strComments.Empty();
    m_strLegalTrademarks.Empty();
    m_strPrivateBuild.Empty();
    m_strSpecialBuild.Empty();
}


