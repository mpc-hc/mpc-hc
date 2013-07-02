//	VirtualDub - Video processing and capture application
//	System library component
//	Copyright (C) 1998-2004 Avery Lee, All Rights Reserved.
//
//	Beginning with 1.6.0, the VirtualDub system library is licensed
//	differently than the remainder of VirtualDub.  This particular file is
//	thus licensed as follows (the "zlib" license):
//
//	This software is provided 'as-is', without any express or implied
//	warranty.  In no event will the authors be held liable for any
//	damages arising from the use of this software.
//
//	Permission is granted to anyone to use this software for any purpose,
//	including commercial applications, and to alter it and redistribute it
//	freely, subject to the following restrictions:
//
//	1.	The origin of this software must not be misrepresented; you must
//		not claim that you wrote the original software. If you use this
//		software in a product, an acknowledgment in the product
//		documentation would be appreciated but is not required.
//	2.	Altered source versions must be plainly marked as such, and must
//		not be misrepresented as being the original software.
//	3.	This notice may not be removed or altered from any source
//		distribution.

#include "stdafx.h"
#include <vd2/system/w32assist.h>
#include <vd2/system/seh.h>
#include <vd2/system/text.h>
#include <vd2/system/vdstdc.h>
#include <vd2/system/vdstl.h>

bool VDIsForegroundTaskW32() {
	HWND hwndFore = GetForegroundWindow();

	if (!hwndFore)
		return false;

	DWORD dwProcessId = 0;
	GetWindowThreadProcessId(hwndFore, &dwProcessId);

	return dwProcessId == GetCurrentProcessId();
}

LPVOID VDConvertThreadToFiberW32(LPVOID parm) {
	typedef LPVOID (WINAPI *tpConvertThreadToFiber)(LPVOID p);
	static tpConvertThreadToFiber ctof = (tpConvertThreadToFiber)GetProcAddress(GetModuleHandle("kernel32"), "ConvertThreadToFiber");

	if (!ctof)
		return NULL;

	return ctof(parm);
}

void VDSwitchToFiberW32(LPVOID fiber) {
	typedef void (WINAPI *tpSwitchToFiber)(LPVOID p);
	static tpSwitchToFiber stof = (tpSwitchToFiber)GetProcAddress(GetModuleHandle("kernel32"), "SwitchToFiber");

	if (stof)
		stof(fiber);
}

int VDGetSizeOfBitmapHeaderW32(const BITMAPINFOHEADER *pHdr) {
	int palents = 0;

	if ((pHdr->biCompression == BI_RGB || pHdr->biCompression == BI_RLE4 || pHdr->biCompression == BI_RLE8) && pHdr->biBitCount <= 8) {
		palents = pHdr->biClrUsed;
		if (!palents)
			palents = 1 << pHdr->biBitCount;
	}
	int size = pHdr->biSize + palents * sizeof(RGBQUAD);

	if (pHdr->biSize < sizeof(BITMAPV4HEADER) && pHdr->biCompression == BI_BITFIELDS)
		size += sizeof(DWORD) * 3;

	return size;
}

void VDSetWindowTextW32(HWND hwnd, const wchar_t *s) {
	if (VDIsWindowsNT()) {
		SetWindowTextW(hwnd, s);
	} else {
		SetWindowTextA(hwnd, VDTextWToA(s).c_str());
	}
}

void VDSetWindowTextFW32(HWND hwnd, const wchar_t *format, ...) {
	va_list val;

	va_start(val, format);
	{
		wchar_t buf[512];
		int r = vdvswprintf(buf, 512, format, val);

		if ((unsigned)r < 512) {
			VDSetWindowTextW32(hwnd, buf);
			va_end(val);
			return;
		}
	}

	VDStringW s;
	s.append_vsprintf(format, val);
	VDSetWindowTextW32(hwnd, s.c_str());

	va_end(val);
}

VDStringA VDGetWindowTextAW32(HWND hwnd) {
	char buf[512];

	int len = GetWindowTextLengthA(hwnd);

	if (len > 511) {
		vdblock<char> tmp(len + 1);
		len = GetWindowTextA(hwnd, tmp.data(), tmp.size());

		const char *s = tmp.data();
		VDStringA text(s, s+len);
		return text;
	} else if (len > 0) {
		len = GetWindowTextA(hwnd, buf, 512);

		return VDStringA(buf, buf + len);
	}

	return VDStringA();
}

VDStringW VDGetWindowTextW32(HWND hwnd) {
	union {
		wchar_t w[256];
		char a[512];
	} buf;

	if (VDIsWindowsNT()) {
		int len = GetWindowTextLengthW(hwnd);

		if (len > 255) {
			vdblock<wchar_t> tmp(len + 1);
			len = GetWindowTextW(hwnd, tmp.data(), tmp.size());

			VDStringW text(tmp.data(), len);
			return text;
		} else if (len > 0) {
			len = GetWindowTextW(hwnd, buf.w, 256);

			VDStringW text(buf.w, len);
			return text;
		}
	} else {
		int len = GetWindowTextLengthA(hwnd);

		if (len > 511) {
			vdblock<char> tmp(len + 1);
			len = GetWindowTextA(hwnd, tmp.data(), tmp.size());

			VDStringW text(VDTextAToW(tmp.data(), len));
			return text;
		} else if (len > 0) {
			len = GetWindowTextA(hwnd, buf.a, 512);

			VDStringW text(VDTextAToW(buf.a, len));
			return text;
		}
	}

	return VDStringW();
}

void VDAppendMenuW32(HMENU hmenu, UINT flags, UINT id, const wchar_t *text){
	if (VDIsWindowsNT()) {
		AppendMenuW(hmenu, flags, id, text);
	} else {
		AppendMenuA(hmenu, flags, id, VDTextWToA(text).c_str());
	}
}

bool VDAppendPopupMenuW32(HMENU hmenu, UINT flags, HMENU hmenuPopup, const wchar_t *text){
	flags |= MF_POPUP;

	if (VDIsWindowsNT())
		return 0 != AppendMenuW(hmenu, flags, (UINT_PTR)hmenuPopup, text);
	else
		return 0 != AppendMenuA(hmenu, flags, (UINT_PTR)hmenuPopup, VDTextWToA(text).c_str());
}

void VDAppendMenuSeparatorW32(HMENU hmenu) {
	int pos = GetMenuItemCount(hmenu);
	if (pos < 0)
		return;

	if (VDIsWindowsNT()) {
		MENUITEMINFOW mmiW;
		vdfastfixedvector<wchar_t, 256> bufW;

		mmiW.cbSize		= MENUITEMINFO_SIZE_VERSION_400W;
		mmiW.fMask		= MIIM_TYPE;
		mmiW.fType		= MFT_SEPARATOR;

		InsertMenuItemW(hmenu, pos, TRUE, &mmiW);
	} else {
		MENUITEMINFOA mmiA;

		mmiA.cbSize		= MENUITEMINFO_SIZE_VERSION_400A;
		mmiA.fMask		= MIIM_TYPE;
		mmiA.fType		= MFT_SEPARATOR;

		InsertMenuItemA(hmenu, pos, TRUE, &mmiA);
	}
}

void VDCheckMenuItemByPositionW32(HMENU hmenu, uint32 pos, bool checked) {
	CheckMenuItem(hmenu, pos, checked ? MF_BYPOSITION|MF_CHECKED : MF_BYPOSITION|MF_UNCHECKED);
}

void VDCheckMenuItemByCommandW32(HMENU hmenu, UINT cmd, bool checked) {
	CheckMenuItem(hmenu, cmd, checked ? MF_BYCOMMAND|MF_CHECKED : MF_BYCOMMAND|MF_UNCHECKED);
}

void VDCheckRadioMenuItemByPositionW32(HMENU hmenu, uint32 pos, bool checked) {
	MENUITEMINFOA mii;

	mii.cbSize = sizeof(MENUITEMINFOA);
	mii.fMask = MIIM_FTYPE | MIIM_STATE;
	if (GetMenuItemInfo(hmenu, pos, TRUE, &mii)) {
		mii.fType |= MFT_RADIOCHECK;
		mii.fState &= ~MFS_CHECKED;
		if (checked)
			mii.fState |= MFS_CHECKED;
		SetMenuItemInfo(hmenu, pos, TRUE, &mii);
	}
}

void VDCheckRadioMenuItemByCommandW32(HMENU hmenu, UINT cmd, bool checked) {
	MENUITEMINFOA mii;

	mii.cbSize = sizeof(MENUITEMINFOA);
	mii.fMask = MIIM_FTYPE | MIIM_STATE;
	if (GetMenuItemInfo(hmenu, cmd, FALSE, &mii)) {
		mii.fType |= MFT_RADIOCHECK;
		mii.fState &= ~MFS_CHECKED;
		if (checked)
			mii.fState |= MFS_CHECKED;
		SetMenuItemInfo(hmenu, cmd, FALSE, &mii);
	}
}

void VDEnableMenuItemByCommandW32(HMENU hmenu, UINT cmd, bool checked) {
	EnableMenuItem(hmenu, cmd, checked ? MF_BYCOMMAND|MF_ENABLED : MF_BYCOMMAND|MF_GRAYED);
}

VDStringW VDGetMenuItemTextByCommandW32(HMENU hmenu, UINT cmd) {
	VDStringW s;

	if (VDIsWindowsNT()) {
		MENUITEMINFOW mmiW;
		vdfastfixedvector<wchar_t, 256> bufW;

		mmiW.cbSize		= MENUITEMINFO_SIZE_VERSION_400W;
		mmiW.fMask		= MIIM_TYPE;
		mmiW.fType		= MFT_STRING;
		mmiW.dwTypeData	= NULL;
		mmiW.cch		= 0;		// required to avoid crash on NT4

		if (GetMenuItemInfoW(hmenu, cmd, FALSE, &mmiW)) {
			bufW.resize(mmiW.cch + 1, 0);
			++mmiW.cch;
			mmiW.dwTypeData = bufW.data();

			if (GetMenuItemInfoW(hmenu, cmd, FALSE, &mmiW))
				s = bufW.data();
		}
	} else {
		MENUITEMINFOA mmiA;
		vdfastfixedvector<char, 256> bufA;

		mmiA.cbSize		= MENUITEMINFO_SIZE_VERSION_400A;
		mmiA.fMask		= MIIM_TYPE;
		mmiA.fType		= MFT_STRING;
		mmiA.dwTypeData	= NULL;

		if (GetMenuItemInfoA(hmenu, cmd, FALSE, &mmiA)) {
			bufA.resize(mmiA.cch + 1, 0);
			++mmiA.cch;
			mmiA.dwTypeData = bufA.data();

			if (GetMenuItemInfoA(hmenu, cmd, FALSE, &mmiA))
				s = VDTextAToW(bufA.data());
		}
	}

	return s;
}

void VDSetMenuItemTextByCommandW32(HMENU hmenu, UINT cmd, const wchar_t *text) {
	if (VDIsWindowsNT()) {
		MENUITEMINFOW mmiW;

		mmiW.cbSize		= MENUITEMINFO_SIZE_VERSION_400W;
		mmiW.fMask		= MIIM_TYPE;
		mmiW.fType		= MFT_STRING;
		mmiW.dwTypeData	= (LPWSTR)text;

		SetMenuItemInfoW(hmenu, cmd, FALSE, &mmiW);
	} else {
		MENUITEMINFOA mmiA;
		VDStringA textA(VDTextWToA(text));

		mmiA.cbSize		= MENUITEMINFO_SIZE_VERSION_400A;
		mmiA.fMask		= MIIM_TYPE;
		mmiA.fType		= MFT_STRING;
		mmiA.dwTypeData	= (LPSTR)textA.c_str();

		SetMenuItemInfoA(hmenu, cmd, FALSE, &mmiA);
	}
}

LRESULT	VDDualCallWindowProcW32(WNDPROC wp, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	return (IsWindowUnicode(hwnd) ? CallWindowProcW : CallWindowProcA)(wp, hwnd, msg, wParam, lParam);
}

LRESULT VDDualDefWindowProcW32(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	return IsWindowUnicode(hwnd) ? DefWindowProcW(hwnd, msg, wParam, lParam) : DefWindowProcA(hwnd, msg, wParam, lParam);
}

EXECUTION_STATE VDSetThreadExecutionStateW32(EXECUTION_STATE esFlags) {
	EXECUTION_STATE es = 0;

	// SetThreadExecutionState(): requires Windows 98+/2000+.
	typedef EXECUTION_STATE (WINAPI *tSetThreadExecutionState)(EXECUTION_STATE);
	static tSetThreadExecutionState pFunc = (tSetThreadExecutionState)GetProcAddress(GetModuleHandle("kernel32"), "SetThreadExecutionState");

	if (pFunc)
		es = pFunc(esFlags);

	return es;
}

bool VDSetFilePointerW32(HANDLE h, sint64 pos, DWORD dwMoveMethod) {
	LONG posHi = (LONG)(pos >> 32);
	DWORD result = SetFilePointer(h, (LONG)pos, &posHi, dwMoveMethod);

	if (result != INVALID_SET_FILE_POINTER)
		return true;

	DWORD dwError = GetLastError();

	return (dwError == NO_ERROR);
}

bool VDGetFileSizeW32(HANDLE h, sint64& size) {
	DWORD dwSizeHigh;
	DWORD dwSizeLow = GetFileSize(h, &dwSizeHigh);

	if (dwSizeLow == (DWORD)-1 && GetLastError() != NO_ERROR)
		return false;

	size = dwSizeLow + ((sint64)dwSizeHigh << 32);
	return true;
}

#if !defined(_MSC_VER) || _MSC_VER < 1300
HMODULE VDGetLocalModuleHandleW32() {
	MEMORY_BASIC_INFORMATION meminfo;
	static HMODULE shmod = (VirtualQuery((HINSTANCE)&VDGetLocalModuleHandleW32, &meminfo, sizeof meminfo), (HMODULE)meminfo.AllocationBase);

	return shmod;
}
#endif

bool VDDrawTextW32(HDC hdc, const wchar_t *s, int nCount, LPRECT lpRect, UINT uFormat) {
	RECT r;
	if (VDIsWindowsNT()) {
		// If multiline and vcentered (not normally supported...)
		if (!((uFormat ^ DT_VCENTER) & (DT_VCENTER|DT_SINGLELINE))) {
			uFormat &= ~DT_VCENTER;

			r = *lpRect;
			if (!DrawTextW(hdc, s, nCount, &r, uFormat | DT_CALCRECT))
				return false;

			int dx = ((lpRect->right - lpRect->left) - (r.right - r.left)) >> 1;
			int dy = ((lpRect->bottom - lpRect->top) - (r.bottom - r.top)) >> 1;

			r.left += dx;
			r.right += dx;
			r.top += dy;
			r.bottom += dy;
			lpRect = &r;
		}

		return !!DrawTextW(hdc, s, nCount, lpRect, uFormat);
	} else {
		VDStringA strA(VDTextWToA(s, nCount));

		// If multiline and vcentered (not normally supported...)
		if (!((uFormat ^ DT_VCENTER) & (DT_VCENTER|DT_SINGLELINE))) {
			uFormat &= ~DT_VCENTER;

			r = *lpRect;
			if (!DrawTextA(hdc, strA.data(), strA.size(), &r, uFormat | DT_CALCRECT))
				return false;

			int dx = ((lpRect->right - lpRect->left) - (r.right - r.left)) >> 1;
			int dy = ((lpRect->bottom - lpRect->top) - (r.bottom - r.top)) >> 1;

			r.left += dx;
			r.right += dx;
			r.top += dy;
			r.bottom += dy;
			lpRect = &r;
		}

		return !!DrawTextA(hdc, strA.data(), strA.size(), lpRect, uFormat);
	}
}

bool VDPatchModuleImportTableW32(HMODULE hmod, const char *srcModule, const char *name, void *pCompareValue, void *pNewValue, void *volatile *ppOldValue) {
	char *pBase = (char *)hmod;

	vd_seh_guard_try {
		// The PEheader offset is at hmod+0x3c.  Add the size of the optional header
		// to step to the section headers.

		const uint32 peoffset = ((const long *)pBase)[15];
		const uint32 signature = *(uint32 *)(pBase + peoffset);

		if (signature != IMAGE_NT_SIGNATURE)
			return false;

		const IMAGE_FILE_HEADER *pHeader = (const IMAGE_FILE_HEADER *)(pBase + peoffset + 4);

		// Verify the PE optional structure.

		if (pHeader->SizeOfOptionalHeader < 104)
			return false;

		// Find import header.

		const IMAGE_IMPORT_DESCRIPTOR *pImportDir;
		int nImports;

		switch(*(short *)((char *)pHeader + IMAGE_SIZEOF_FILE_HEADER)) {

#ifdef _M_AMD64
		case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
			{
				const IMAGE_OPTIONAL_HEADER64 *pOpt = (IMAGE_OPTIONAL_HEADER64 *)((const char *)pHeader + sizeof(IMAGE_FILE_HEADER));

				if (pOpt->NumberOfRvaAndSizes < 2)
					return false;

				pImportDir = (const IMAGE_IMPORT_DESCRIPTOR *)(pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
				nImports = pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size / sizeof(IMAGE_IMPORT_DESCRIPTOR);
			}
			break;
#else
		case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
			{
				const IMAGE_OPTIONAL_HEADER32 *pOpt = (IMAGE_OPTIONAL_HEADER32 *)((const char *)pHeader + sizeof(IMAGE_FILE_HEADER));

				if (pOpt->NumberOfRvaAndSizes < 2)
					return false;

				pImportDir = (const IMAGE_IMPORT_DESCRIPTOR *)(pBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
				nImports = pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size / sizeof(IMAGE_IMPORT_DESCRIPTOR);
			}
			break;
#endif

		default:		// reject PE32+
			return false;
		}

		// Hmmm... no imports?

		if ((const char *)pImportDir == pBase)
			return false;

		// Scan down the import entries.  We are looking for MSVFW32.

		int i;

		for(i=0; i<nImports; ++i) {
			if (!_stricmp(pBase + pImportDir[i].Name, srcModule))
				break;
		}

		if (i >= nImports)
			return false;

		// Found it.  Start scanning MSVFW32 imports until we find DrawDibDraw.

		const long *pImports = (const long *)(pBase + pImportDir[i].OriginalFirstThunk);
		void * volatile *pVector = (void * volatile *)(pBase + pImportDir[i].FirstThunk);

		while(*pImports) {
			if (*pImports >= 0) {
				const char *pName = pBase + *pImports + 2;

				if (!strcmp(pName, name)) {

					// Found it!  Reset the protection.

					DWORD dwOldProtect;

					if (VirtualProtect((void *)pVector, sizeof(void *), PAGE_EXECUTE_READWRITE, &dwOldProtect)) {
						if (ppOldValue) {
							for(;;) {
								void *old = *pVector;
								if (pCompareValue && pCompareValue != old)
									return false;

								*ppOldValue = old;
								if (old == VDAtomicCompareExchangePointer(pVector, pNewValue, old))
									break;
							}
						} else {
							*pVector = pNewValue;
						}

						VirtualProtect((void *)pVector, sizeof(void *), dwOldProtect, &dwOldProtect);

						return true;
					}

					break;
				}
			}

			++pImports;
			++pVector;
		}
	} vd_seh_guard_except {
	}

	return false;
}

bool VDPatchModuleExportTableW32(HMODULE hmod, const char *name, void *pCompareValue, void *pNewValue, void *volatile *ppOldValue) {
	char *pBase = (char *)hmod;

	vd_seh_guard_try {
		// The PEheader offset is at hmod+0x3c.  Add the size of the optional header
		// to step to the section headers.

		const uint32 peoffset = ((const long *)pBase)[15];
		const uint32 signature = *(uint32 *)(pBase + peoffset);

		if (signature != IMAGE_NT_SIGNATURE)
			return false;

		const IMAGE_FILE_HEADER *pHeader = (const IMAGE_FILE_HEADER *)(pBase + peoffset + 4);

		// Verify the PE optional structure.

		if (pHeader->SizeOfOptionalHeader < 104)
			return false;

		// Find export directory.

		const IMAGE_EXPORT_DIRECTORY *pExportDir;

		switch(*(short *)((char *)pHeader + IMAGE_SIZEOF_FILE_HEADER)) {

#ifdef _M_AMD64
		case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
			{
				const IMAGE_OPTIONAL_HEADER64 *pOpt = (IMAGE_OPTIONAL_HEADER64 *)((const char *)pHeader + sizeof(IMAGE_FILE_HEADER));

				if (pOpt->NumberOfRvaAndSizes < 1)
					return false;

				DWORD exportDirRVA = pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

				if (!exportDirRVA)
					return false;

				pExportDir = (const IMAGE_EXPORT_DIRECTORY *)(pBase + exportDirRVA);
			}
			break;
#else
		case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
			{
				const IMAGE_OPTIONAL_HEADER32 *pOpt = (IMAGE_OPTIONAL_HEADER32 *)((const char *)pHeader + sizeof(IMAGE_FILE_HEADER));

				if (pOpt->NumberOfRvaAndSizes < 1)
					return false;

				DWORD exportDirRVA = pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

				if (!exportDirRVA)
					return false;

				pExportDir = (const IMAGE_EXPORT_DIRECTORY *)(pBase + exportDirRVA);
			}
			break;
#endif

		default:		// reject PE32+
			return false;
		}

		// Scan for the export name.
		DWORD nameCount = pExportDir->AddressOfNames;
		const DWORD *nameRVAs = (const DWORD *)(pBase + pExportDir->AddressOfNames);
		const WORD *nameOrdinals = (const WORD *)(pBase + pExportDir->AddressOfNameOrdinals);
		DWORD *functionTable = (DWORD *)(pBase + pExportDir->AddressOfFunctions);

		for(DWORD i=0; i<nameCount; ++i) {
			DWORD nameRVA = nameRVAs[i];
			const char *pName = (const char *)(pBase + nameRVA);

			// compare names
			if (!strcmp(pName, name)) {

				// name matches -- look up the function entry
				WORD ordinal = nameOrdinals[i];
				DWORD *pRVA = &functionTable[ordinal];
				
				// Reset the protection.

				DWORD newRVA = (DWORD)pNewValue - (DWORD)pBase;

				DWORD dwOldProtect;
				if (VirtualProtect((void *)pRVA, sizeof(DWORD), PAGE_EXECUTE_READWRITE, &dwOldProtect)) {
					if (ppOldValue) {
						for(;;) {
							DWORD oldRVA = *pRVA;
							void *old = pBase + oldRVA;
							if (pCompareValue && pCompareValue != old)
								return false;

							*ppOldValue = pBase + oldRVA;
							if (oldRVA == VDAtomicInt::staticCompareExchange((volatile int *)pRVA, newRVA, oldRVA))
								break;
						}
					} else {
						*pRVA = newRVA;
					}

					VirtualProtect((void *)pRVA, sizeof(DWORD), dwOldProtect, &dwOldProtect);

					return true;
				}

				break;
			}
		}
	} vd_seh_guard_except {
	}

	return false;
}

HMODULE VDLoadSystemLibraryW32(const char *name) {
	if (VDIsWindowsNT()) {
		vdfastvector<wchar_t> pathW(MAX_PATH, 0);

		size_t len = GetSystemDirectoryW(pathW.data(), MAX_PATH);

		if (!len)
			return NULL;

		if (len > MAX_PATH) {
			pathW.resize(len + 1, 0);

			len = GetSystemDirectoryW(pathW.data(), len);
			if (!len || len >= pathW.size())
				return NULL;
		}

		pathW.resize(len);

		if (pathW.back() != '\\')
			pathW.push_back('\\');

		while(const char c = *name++)
			pathW.push_back(c);

		pathW.push_back(0);

		return LoadLibraryW(pathW.data());
	} else {
		vdfastvector<char> pathA(MAX_PATH, 0);
		size_t len = GetSystemDirectoryA(pathA.data(), MAX_PATH);

		if (!len)
			return NULL;

		if (len > MAX_PATH) {
			pathA.resize(len + 1, 0);

			len = GetSystemDirectoryA(pathA.data(), len);
			if (!len || len >= pathA.size())
				return NULL;
		}

		pathA.resize(len);

		if (pathA.back() != '\\')
			pathA.push_back('\\');

		pathA.insert(pathA.end(), name, name + strlen(name));
		pathA.push_back(0);

		return LoadLibraryA(pathA.data());
	}
}
