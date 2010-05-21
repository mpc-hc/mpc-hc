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
#include <ctype.h>
#include <string.h>

#include <vd2/system/VDString.h>
#include <vd2/system/filesys.h>
#include <vd2/system/Error.h>
#include <vd2/system/vdstl.h>
#include <vd2/system/w32assist.h>

///////////////////////////////////////////////////////////////////////////

template<class T, class U>
static inline T splitimpL(const T& string, const U *s) {
	const U *p = string.c_str();
	return T(p, s - p);
}

template<class T, class U>
static inline T splitimpR(const T& string, const U *s) {
	const U *p = string.c_str();
	return T(s);
}

///////////////////////////////////////////////////////////////////////////

const char *VDFileSplitFirstDir(const char *s) {
	const char *start = s;

	while(*s++)
		if (s[-1] == ':' || s[-1] == '\\' || s[-1] == '/')
			return s;

	return start;
}

const wchar_t *VDFileSplitFirstDir(const wchar_t *s) {
	const wchar_t *start = s;

	while(*s++)
		if (s[-1] == L':' || s[-1] == L'\\' || s[-1] == L'/')
			return s;

	return start;
}

const char *VDFileSplitPath(const char *s) {
	const char *lastsep = s;

	while(*s++)
		if (s[-1] == ':' || s[-1] == '\\' || s[-1] == '/')
			lastsep = s;

	return lastsep;
}

const wchar_t *VDFileSplitPath(const wchar_t *s) {
	const wchar_t *lastsep = s;

	while(*s++)
		if (s[-1] == L':' || s[-1] == L'\\' || s[-1] == L'/')
			lastsep = s;

	return lastsep;
}

VDString  VDFileSplitPathLeft (const VDString&  s) { return splitimpL(s, VDFileSplitPath(s.c_str())); }
VDStringW VDFileSplitPathLeft (const VDStringW& s) { return splitimpL(s, VDFileSplitPath(s.c_str())); }
VDString  VDFileSplitPathRight(const VDString&  s) { return splitimpR(s, VDFileSplitPath(s.c_str())); }
VDStringW VDFileSplitPathRight(const VDStringW& s) { return splitimpR(s, VDFileSplitPath(s.c_str())); }

const char *VDFileSplitRoot(const char *s) {
	// Test for a UNC path.
	if (s[0] == '\\' && s[1] == '\\') {
		// For these, we scan for the fourth backslash.
		s += 2;
		for(int i=0; i<2; ++i) {
			while(*s && *s != '\\')
				++s;
			if (*s == '\\')
				++s;
		}
		return s;
	}

	const char *const t = s;

	while(*s && *s != ':' && *s != '/' && *s != '\\')
		++s;

	return *s ? *s == ':' && (s[1]=='/' || s[1]=='\\') ? s+2 : s+1 : t;
}

const wchar_t *VDFileSplitRoot(const wchar_t *s) {
	// Test for a UNC path.
	if (s[0] == '\\' && s[1] == '\\') {
		// For these, we scan for the fourth backslash.
		s += 2;
		for(int i=0; i<2; ++i) {
			while(*s && *s != '\\')
				++s;
			if (*s == '\\')
				++s;
		}
		return s;
	}

	const wchar_t *const t = s;

	while(*s && *s != L':' && *s != L'/' && *s != L'\\')
		++s;

	return *s ? *s == L':' && (s[1]==L'/' || s[1]==L'\\') ? s+2 : s+1 : t;
}

VDString  VDFileSplitRoot(const VDString&  s) { return splitimpL(s, VDFileSplitRoot(s.c_str())); }
VDStringW VDFileSplitRoot(const VDStringW& s) { return splitimpL(s, VDFileSplitRoot(s.c_str())); }

const char *VDFileSplitExt(const char *s) {
	const char *t = s;

	while(*t)
		++t;

	const char *const end = t;

	while(t>s) {
		--t;

		if (*t == '.')
			return t;

		if (*t == ':' || *t == '\\' || *t == '/')
			break;
	}

	return NULL;
}

const wchar_t *VDFileSplitExt(const wchar_t *s) {
	const wchar_t *t = s;

	while(*t)
		++t;

	const wchar_t *const end = t;

	while(t>s) {
		--t;

		if (*t == L'.')
			return t;

		if (*t == L':' || *t == L'\\' || *t == L'/')
			break;
	}

	return end;
}

VDString  VDFileSplitExtLeft (const VDString&  s) { return splitimpL(s, VDFileSplitExt(s.c_str())); }
VDStringW VDFileSplitExtLeft (const VDStringW& s) { return splitimpL(s, VDFileSplitExt(s.c_str())); }
VDString  VDFileSplitExtRight(const VDString&  s) { return splitimpR(s, VDFileSplitExt(s.c_str())); }
VDStringW VDFileSplitExtRight(const VDStringW& s) { return splitimpR(s, VDFileSplitExt(s.c_str())); }

/////////////////////////////////////////////////////////////////////////////

bool VDFileWildMatch(const char *pattern, const char *path) {
	// What we do here is split the string into segments that are bracketed
	// by sequences of asterisks. The trick is that the first match for a
	// segment as the best possible match, so we can continue. So we just
	// take each segment at a time and walk it forward until we find the
	// first match or we fail.
	//
	// Time complexity is O(NM), where N=length of string and M=length of
	// the pattern. In practice, it's rather fast.

	bool star = false;
	int i = 0;
	for(;;) {
		char c = (char)tolower((unsigned char)pattern[i]);
		if (c == '*') {
			star = true;
			pattern += i+1;
			if (!*pattern)
				return true;
			path += i;
			i = 0;
			continue;
		}

		char d = (char)tolower((unsigned char)path[i]);
		++i;

		if (c == '?') {		// ? matches any character but null.
			if (!d)
				return false;
		} else if (c != d) {	// Literal character must match itself.
			// If we're at the end of the string or there is no
			// previous asterisk (anchored search), there's no other
			// match to find.
			if (!star || !d || !i)
				return false;

			// Restart segment search at next position in path.
			++path;
			i = 0;
			continue;
		}

		if (!c)
			return true;
	}
}

bool VDFileWildMatch(const wchar_t *pattern, const wchar_t *path) {
	// What we do here is split the string into segments that are bracketed
	// by sequences of asterisks. The trick is that the first match for a
	// segment as the best possible match, so we can continue. So we just
	// take each segment at a time and walk it forward until we find the
	// first match or we fail.
	//
	// Time complexity is O(NM), where N=length of string and M=length of
	// the pattern. In practice, it's rather fast.

	bool star = false;
	int i = 0;
	for(;;) {
		wchar_t c = towlower(pattern[i]);
		if (c == L'*') {
			star = true;
			pattern += i+1;
			if (!*pattern)
				return true;
			path += i;
			i = 0;
			continue;
		}

		wchar_t d = towlower(path[i]);
		++i;

		if (c == L'?') {		// ? matches any character but null.
			if (!d)
				return false;
		} else if (c != d) {	// Literal character must match itself.
			// If we're at the end of the string or there is no
			// previous asterisk (anchored search), there's no other
			// match to find.
			if (!star || !d || !i)
				return false;

			// Restart segment search at next position in path.
			++path;
			i = 0;
			continue;
		}

		if (!c)
			return true;
	}
}

/////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <vd2/system/w32assist.h>

sint64 VDGetDiskFreeSpace(const wchar_t *path) {
	typedef BOOL (WINAPI *tpGetDiskFreeSpaceExA)(LPCSTR lpDirectoryName, PULARGE_INTEGER lpFreeBytesAvailable, PULARGE_INTEGER lpTotalNumberOfBytes, PULARGE_INTEGER lpTotalNumberOfFreeBytes);
	typedef BOOL (WINAPI *tpGetDiskFreeSpaceExW)(LPCWSTR lpDirectoryName, PULARGE_INTEGER lpFreeBytesAvailable, PULARGE_INTEGER lpTotalNumberOfBytes, PULARGE_INTEGER lpTotalNumberOfFreeBytes);

	static bool sbChecked = false;
	static tpGetDiskFreeSpaceExA spGetDiskFreeSpaceExA;
	static tpGetDiskFreeSpaceExW spGetDiskFreeSpaceExW;

	if (!sbChecked) {
		HMODULE hmodKernel = GetModuleHandle("kernel32.dll");
		spGetDiskFreeSpaceExA = (tpGetDiskFreeSpaceExA)GetProcAddress(hmodKernel, "GetDiskFreeSpaceExA");
		spGetDiskFreeSpaceExW = (tpGetDiskFreeSpaceExW)GetProcAddress(hmodKernel, "GetDiskFreeSpaceExW");

		sbChecked = true;
	}

	if (spGetDiskFreeSpaceExA) {
		BOOL success;
		uint64 freeClient, totalBytes, totalFreeBytes;
		VDStringW directoryName(path);

		if (!directoryName.empty()) {
			wchar_t c = directoryName[directoryName.length()-1];

			if (c != L'/' && c != L'\\')
				directoryName += L'\\';
		}

		if ((LONG)GetVersion() < 0)
			success = spGetDiskFreeSpaceExA(VDTextWToA(directoryName).c_str(), (PULARGE_INTEGER)&freeClient, (PULARGE_INTEGER)&totalBytes, (PULARGE_INTEGER)&totalFreeBytes);
		else
			success = spGetDiskFreeSpaceExW(directoryName.c_str(), (PULARGE_INTEGER)&freeClient, (PULARGE_INTEGER)&totalBytes, (PULARGE_INTEGER)&totalFreeBytes);

		return success ? (sint64)freeClient : -1;
	} else {
		DWORD sectorsPerCluster, bytesPerSector, freeClusters, totalClusters;
		BOOL success;

		VDStringW rootPath(VDFileGetRootPath(path));

		if ((LONG)GetVersion() < 0)
			success = GetDiskFreeSpaceA(rootPath.empty() ? NULL : VDTextWToA(rootPath).c_str(), &sectorsPerCluster, &bytesPerSector, &freeClusters, &totalClusters);
		else
			success = GetDiskFreeSpaceW(rootPath.empty() ? NULL : rootPath.c_str(), &sectorsPerCluster, &bytesPerSector, &freeClusters, &totalClusters);

		return success ? (sint64)((uint64)sectorsPerCluster * bytesPerSector * freeClusters) : -1;
	}
}

bool VDDoesPathExist(const wchar_t *fileName) {
	bool bExists;

	if (!(GetVersion() & 0x80000000)) {
		bExists = ((DWORD)-1 != GetFileAttributesW(fileName));
	} else {
		bExists = ((DWORD)-1 != GetFileAttributesA(VDTextWToA(fileName).c_str()));
	}

	return bExists;
}

void VDCreateDirectory(const wchar_t *path) {
	// can't create dir with trailing slash
	VDStringW::size_type l(wcslen(path));

	if (l) {
		const wchar_t c = path[l-1];

		if (c == L'/' || c == L'\\') {
			VDCreateDirectory(VDStringW(path, l-1).c_str());
			return;
		}
	}

	BOOL succeeded;

	if (!(GetVersion() & 0x80000000)) {
		succeeded = CreateDirectoryW(path, NULL);
	} else {
		succeeded = CreateDirectoryA(VDTextWToA(path).c_str(), NULL);
	}

	if (!succeeded)
		throw MyWin32Error("Cannot create directory: %%s", GetLastError());
}

///////////////////////////////////////////////////////////////////////////

bool VDDeletePathAutodetect(const wchar_t *path);
bool (*VDRemoveFile)(const wchar_t *path) = VDDeletePathAutodetect;

namespace {
	typedef BOOL (APIENTRY *tpDeleteFileW)(LPCWSTR path);
	tpDeleteFileW spDeleteFileW;
}

bool VDDeleteFile9x(const wchar_t *path) {
	return !!DeleteFileA(VDTextWToA(path).c_str());
}

bool VDDeleteFileNT(const wchar_t *path) {
	return !!spDeleteFileW(path);
}

bool VDDeletePathAutodetect(const wchar_t *path) {
	if (VDIsWindowsNT()) {
		spDeleteFileW = (tpDeleteFileW)GetProcAddress(GetModuleHandle("kernel32"), "DeleteFileW");
		VDRemoveFile = VDDeleteFileNT;
	} else
		VDRemoveFile = VDDeleteFile9x;

	return VDRemoveFile(path);
}

///////////////////////////////////////////////////////////////////////////

namespace {
	typedef BOOL (WINAPI *tpGetVolumePathNameW)(LPCWSTR lpszPathName, LPWSTR lpszVolumePathName, DWORD cchBufferLength);
	typedef BOOL (WINAPI *tpGetFullPathNameW)(LPCWSTR lpFileName, DWORD nBufferLength, LPWSTR lpBuffer, LPWSTR *lpFilePart);
}

uint64 VDFileGetLastWriteTime(const wchar_t *path) {
	if (VDIsWindowsNT()) {
		WIN32_FIND_DATAW fdw;
		HANDLE h = FindFirstFileW(path, &fdw);
		if (h == INVALID_HANDLE_VALUE)
			return 0;

		FindClose(h);

		return ((uint64)fdw.ftLastWriteTime.dwHighDateTime << 32) + fdw.ftLastWriteTime.dwLowDateTime;
	} else {
		WIN32_FIND_DATAA fda;
		HANDLE h = FindFirstFileA(VDTextWToA(path).c_str(), &fda);
		if (h == INVALID_HANDLE_VALUE)
			return 0;

		FindClose(h);

		return ((uint64)fda.ftLastWriteTime.dwHighDateTime << 32) + fda.ftLastWriteTime.dwLowDateTime;
	}
}

VDStringW VDFileGetRootPath(const wchar_t *path) {
	static tpGetVolumePathNameW spGetVolumePathNameW = (tpGetVolumePathNameW)GetProcAddress(GetModuleHandle("kernel32.dll"), "GetVolumePathNameW");
	static tpGetFullPathNameW spGetFullPathNameW = (tpGetFullPathNameW)GetProcAddress(GetModuleHandle("kernel32.dll"), "GetFullPathNameW");

	VDStringW fullPath(VDGetFullPath(path));

	// Windows 2000/XP path
	if (spGetVolumePathNameW) {
		vdblock<wchar_t> buf(std::max<size_t>(fullPath.size() + 1, MAX_PATH));

		if (spGetVolumePathNameW(path, buf.data(), buf.size()))
			return VDStringW(buf.data());
	}

	// Windows 95/98/ME/NT4 path
	const wchar_t *s = fullPath.c_str();
	VDStringW root(s, VDFileSplitRoot(s) - s);
	VDFileFixDirPath(root);
	return root;
}

VDStringW VDGetFullPath(const wchar_t *partialPath) {
	static tpGetFullPathNameW spGetFullPathNameW = (tpGetFullPathNameW)GetProcAddress(GetModuleHandle("kernel32.dll"), "GetFullPathNameW");

	union {
		char		a[MAX_PATH];
		wchar_t		w[MAX_PATH];
	} tmpBuf;

	if (spGetFullPathNameW && !(GetVersion() & 0x80000000)) {
		LPWSTR p;

		tmpBuf.w[0] = 0;
		DWORD count = spGetFullPathNameW(partialPath, MAX_PATH, tmpBuf.w, &p);

		if (count < MAX_PATH)
			return VDStringW(tmpBuf.w);

		VDStringW tmp(count);

		DWORD newCount = spGetFullPathNameW(partialPath, count, (wchar_t *)tmp.data(), &p);
		if (newCount < count)
			return tmp;

		return VDStringW(partialPath);
	} else {
		LPSTR p;
		VDStringA pathA(VDTextWToA(partialPath));

		tmpBuf.a[0] = 0;
		DWORD count = GetFullPathNameA(pathA.c_str(), MAX_PATH, tmpBuf.a, &p);

		if (count < MAX_PATH)
			return VDStringW(VDTextAToW(tmpBuf.a));

		VDStringA tmpA(count);

		DWORD newCount = GetFullPathNameA(pathA.c_str(), count, (char *)tmpA.data(), &p);
		if (newCount < count)
			return VDTextAToW(tmpA);

		return VDStringW(partialPath);
	}
}

VDStringW VDMakePath(const wchar_t *base, const wchar_t *file) {
	if (!*base)
		return VDStringW(file);

	VDStringW result(base);

	const wchar_t c = result[result.size() - 1];

	if (c != L'/' && c != L'\\' && c != L':')
		result += L'\\';

	result.append(file);

	return result;
}

void VDFileFixDirPath(VDStringW& path) {
	if (!path.empty()) {
		wchar_t c = path[path.size()-1];

		if (c != L'/' && c != L'\\' && c != L':')
			path += L'\\';
	}
}

namespace {
	VDStringW VDGetModulePathW32(HINSTANCE hInst) {
		union {
			wchar_t w[MAX_PATH];
			char a[MAX_PATH];
		} buf;

		VDStringW wstr;

		if (VDIsWindowsNT()) {
			wcscpy(buf.w, L".");
			if (GetModuleFileNameW(hInst, buf.w, MAX_PATH))
				*VDFileSplitPath(buf.w) = 0;
			wstr = buf.w;
		} else {
			strcpy(buf.a, ".");
			if (GetModuleFileNameA(hInst, buf.a, MAX_PATH))
				*VDFileSplitPath(buf.a) = 0;
			wstr = VDTextAToW(buf.a, -1);
		}

		VDStringW wstr2(VDGetFullPath(wstr.c_str()));

		return wstr2;
	}
}

VDStringW VDGetLocalModulePath() {
	return VDGetModulePathW32(VDGetLocalModuleHandleW32());
}

VDStringW VDGetProgramPath() {
	return VDGetModulePathW32(NULL);
}

///////////////////////////////////////////////////////////////////////////

VDDirectoryIterator::VDDirectoryIterator(const wchar_t *path)
	: mSearchPath(path)
	, mpHandle(NULL)
	, mbSearchComplete(false)
{
	mBasePath = VDFileSplitPathLeft(mSearchPath);
	VDFileFixDirPath(mBasePath);
}

VDDirectoryIterator::~VDDirectoryIterator() {
	if (mpHandle)
		FindClose((HANDLE)mpHandle);
}

bool VDDirectoryIterator::Next() {
	if (mbSearchComplete)
		return false;

	union {
		WIN32_FIND_DATAA a;
		WIN32_FIND_DATAW w;
	} wfd;

	if (GetVersion() & 0x80000000) {
		if (mpHandle)
			mbSearchComplete = !FindNextFileA((HANDLE)mpHandle, &wfd.a);
		else {
			mpHandle = FindFirstFileA(VDTextWToA(mSearchPath).c_str(), &wfd.a);
			mbSearchComplete = (INVALID_HANDLE_VALUE == mpHandle);
		}
		if (mbSearchComplete)
			return false;

		mbDirectory = (wfd.a.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
		mFilename = VDTextAToW(wfd.a.cFileName);
		mFileSize = wfd.a.nFileSizeLow + ((sint64)wfd.w.nFileSizeHigh << 32);
	} else {
		if (mpHandle)
			mbSearchComplete = !FindNextFileW((HANDLE)mpHandle, &wfd.w);
		else {
			mpHandle = FindFirstFileW(mSearchPath.c_str(), &wfd.w);
			mbSearchComplete = (INVALID_HANDLE_VALUE == mpHandle);
		}
		if (mbSearchComplete)
			return false;

		mbDirectory = (wfd.w.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
		mFilename = wfd.w.cFileName;
		mFileSize = wfd.w.nFileSizeLow + ((sint64)wfd.w.nFileSizeHigh << 32);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG

struct VDSystemFilesysTestObject {
	VDSystemFilesysTestObject() {
#define TEST(fn, x, y1, y2) VDASSERT(!strcmp(fn(x), y2)); VDASSERT(!wcscmp(fn(L##x), L##y2)); VDASSERT(fn##Left(VDStringA(x))==y1); VDASSERT(fn##Right(VDStringA(x))==y2); VDASSERT(fn##Left(VDStringW(L##x))==L##y1); VDASSERT(fn##Right(VDStringW(L##x))==L##y2)
		TEST(VDFileSplitPath, "", "", "");
		TEST(VDFileSplitPath, "x", "", "x");
		TEST(VDFileSplitPath, "x\\y", "x\\", "y");
		TEST(VDFileSplitPath, "x\\y\\z", "x\\y\\", "z");
		TEST(VDFileSplitPath, "x\\", "x\\", "");
		TEST(VDFileSplitPath, "x\\y\\z\\", "x\\y\\z\\", "");
		TEST(VDFileSplitPath, "c:", "c:", "");
		TEST(VDFileSplitPath, "c:x", "c:", "x");
		TEST(VDFileSplitPath, "c:\\", "c:\\", "");
		TEST(VDFileSplitPath, "c:\\x", "c:\\", "x");
		TEST(VDFileSplitPath, "c:\\x\\", "c:\\x\\", "");
		TEST(VDFileSplitPath, "c:\\x\\", "c:\\x\\", "");
		TEST(VDFileSplitPath, "c:x\\y", "c:x\\", "y");
		TEST(VDFileSplitPath, "\\\\server\\share\\", "\\\\server\\share\\", "");
		TEST(VDFileSplitPath, "\\\\server\\share\\x", "\\\\server\\share\\", "x");
#undef TEST
#define TEST(fn, x, y1, y2) VDASSERT(!strcmp(fn(x), y2)); VDASSERT(!wcscmp(fn(L##x), L##y2)); VDASSERT(fn(VDStringA(x))==y1); VDASSERT(fn(VDStringW(L##x))==L##y1)
		TEST(VDFileSplitRoot, "", "", "");
		TEST(VDFileSplitRoot, "c:", "c:", "");
		TEST(VDFileSplitRoot, "c:x", "c:", "x");
		TEST(VDFileSplitRoot, "c:x\\", "c:", "x\\");
		TEST(VDFileSplitRoot, "c:x\\y", "c:", "x\\y");
		TEST(VDFileSplitRoot, "c:\\", "c:\\", "");
		TEST(VDFileSplitRoot, "c:\\x", "c:\\", "x");
		TEST(VDFileSplitRoot, "c:\\x\\", "c:\\", "x\\");
		TEST(VDFileSplitRoot, "\\", "\\", "");
		TEST(VDFileSplitRoot, "\\x", "\\", "x");
		TEST(VDFileSplitRoot, "\\x\\", "\\", "x\\");
		TEST(VDFileSplitRoot, "\\x\\y", "\\", "x\\y");
		TEST(VDFileSplitRoot, "\\\\server\\share", "\\\\server\\share", "");
		TEST(VDFileSplitRoot, "\\\\server\\share\\", "\\\\server\\share\\", "");
		TEST(VDFileSplitRoot, "\\\\server\\share\\x", "\\\\server\\share\\", "x");
		TEST(VDFileSplitRoot, "\\\\server\\share\\x\\", "\\\\server\\share\\", "x\\");
		TEST(VDFileSplitRoot, "\\\\server\\share\\x\\y", "\\\\server\\share\\", "x\\y");
#undef TEST
	}
} g_VDSystemFilesysTestObject;

#endif
