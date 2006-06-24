#pragma once

#include <atlcoll.h>

class CFontInstaller
{
	HANDLE (WINAPI *pAddFontMemResourceEx)(PVOID,DWORD,PVOID,DWORD*);
	BOOL (WINAPI *pRemoveFontMemResourceEx)(HANDLE);
	int (WINAPI *pAddFontResourceEx)(LPCTSTR,DWORD,PVOID);
	BOOL (WINAPI *pRemoveFontResourceEx)(LPCTSTR,DWORD,PVOID);
	BOOL (WINAPI *pMoveFileEx)(LPCTSTR, LPCTSTR,DWORD);

	CAtlList<HANDLE> m_fonts;
	CAtlList<CString> m_files;
	bool InstallFontMemory(const void* pData, UINT len);
	bool InstallFontFile(const void* pData, UINT len);

public:
	CFontInstaller();
	virtual ~CFontInstaller();

	bool InstallFont(const CAtlArray<BYTE>& data);
	bool InstallFont(const void* pData, UINT len);	
	void UninstallFonts();
};
