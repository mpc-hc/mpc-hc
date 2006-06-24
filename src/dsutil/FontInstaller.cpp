#include "StdAfx.h"
#include ".\fontinstaller.h"

CFontInstaller::CFontInstaller()
{
	if(HMODULE hGdi = GetModuleHandle(_T("gdi32.dll")))
	{
		pAddFontMemResourceEx = (HANDLE (WINAPI *)(PVOID,DWORD,PVOID,DWORD*))GetProcAddress(hGdi, "AddFontMemResourceEx");
		pAddFontResourceEx = (int (WINAPI *)(LPCTSTR,DWORD,PVOID))GetProcAddress(hGdi, "AddFontResourceExA");
		pRemoveFontMemResourceEx = (BOOL (WINAPI *)(HANDLE))GetProcAddress(hGdi, "RemoveFontMemResourceEx");
		pRemoveFontResourceEx = (BOOL (WINAPI *)(LPCTSTR,DWORD,PVOID))GetProcAddress(hGdi, "RemoveFontResourceExA");
	}

	if(HMODULE hGdi = GetModuleHandle(_T("kernel32.dll")))
	{
		pMoveFileEx = (BOOL (WINAPI *)(LPCTSTR, LPCTSTR, DWORD))GetProcAddress(hGdi, "MoveFileExA");
	}
}

CFontInstaller::~CFontInstaller()
{
	UninstallFonts();
}

bool CFontInstaller::InstallFont(const CAtlArray<BYTE>& data)
{
	return InstallFont(data.GetData(), data.GetCount());
}

bool CFontInstaller::InstallFont(const void* pData, UINT len)
{
	return InstallFontFile(pData, len) || InstallFontMemory(pData, len);
}

void CFontInstaller::UninstallFonts()
{
	if(pRemoveFontMemResourceEx)
	{
		POSITION pos = m_fonts.GetHeadPosition();
		while(pos) pRemoveFontMemResourceEx(m_fonts.GetNext(pos));
		m_fonts.RemoveAll();
	}

	if(pRemoveFontResourceEx)
	{
		POSITION pos = m_files.GetHeadPosition();
		while(pos)
		{
			CString fn = m_files.GetNext(pos);
			pRemoveFontResourceEx(fn, FR_PRIVATE, 0);
			if(!DeleteFile(fn) && pMoveFileEx)
				pMoveFileEx(fn, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
		}
		
		m_files.RemoveAll();
	}
}

bool CFontInstaller::InstallFontMemory(const void* pData, UINT len)
{
	if(!pAddFontMemResourceEx)
		return false;

	DWORD nFonts = 0;
	HANDLE hFont = pAddFontMemResourceEx((PVOID)pData, len, NULL, &nFonts);
	if(hFont && nFonts > 0) m_fonts.AddTail(hFont);
	return hFont && nFonts > 0;
}

bool CFontInstaller::InstallFontFile(const void* pData, UINT len)
{
	if(!pAddFontResourceEx) 
		return false;

	CFile f;
	TCHAR path[MAX_PATH], fn[MAX_PATH];
	if(!GetTempPath(MAX_PATH, path) || !GetTempFileName(path, _T("g_font"), 0, fn))
		return false;

	if(f.Open(fn, CFile::modeWrite))
	{
		f.Write(pData, len);
		f.Close();

		if(pAddFontResourceEx(fn, FR_PRIVATE, 0) > 0)
		{
			m_files.AddTail(fn);
			return true;
		}
	}

	DeleteFile(fn);
	return false;
}
