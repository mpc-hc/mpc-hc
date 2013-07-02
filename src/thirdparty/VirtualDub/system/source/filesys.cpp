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
#include <vd2/system/strutil.h>

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

	const char *t = s;

	for(;;) {
		const char c = *t;

		if (c == ':') {
			if (t[1] == '/' || t[1] == '\\')
				return t+2;

			return t+1;
		}

		// check if it was a valid drive identifier
		if (!isalpha((unsigned char)c) && (s == t || !isdigit((unsigned char)c)))
			return s;

		++t;
	}
}

const wchar_t *VDFileSplitRoot(const wchar_t *s) {
	// Test for a UNC path.
	if (s[0] == L'\\' && s[1] == L'\\') {
		// For these, we scan for the fourth backslash.
		s += 2;
		for(int i=0; i<2; ++i) {
			while(*s && *s != L'\\')
				++s;
			if (*s == L'\\')
				++s;
		}
		return s;
	}

	const wchar_t *t = s;

	for(;;) {
		const wchar_t c = *t;

		if (c == L':') {
			if (t[1] == L'/' || t[1] == L'\\')
				return t+2;

			return t+1;
		}

		// check if it was a valid drive identifier
		if (!iswalpha(c) && (s == t || !iswdigit(c)))
			return s;

		++t;
	}
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

	return end;
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

VDParsedPath::VDParsedPath()
	: mbIsRelative(true)
{
}

VDParsedPath::VDParsedPath(const wchar_t *path)
	: mbIsRelative(true)
{
	// Check if the string contains a root, such as a drive or UNC specifier.
	const wchar_t *rootSplit = VDFileSplitRoot(path);
	if (rootSplit != path) {
		mRoot.assign(path, rootSplit);

		for(VDStringW::iterator it(mRoot.begin()), itEnd(mRoot.end()); it != itEnd; ++it) {
			if (*it == L'/')
				*it = L'\\';
		}

		mbIsRelative = (mRoot.back() == L':');

		// If the path is UNC, strip a trailing backslash.
		if (mRoot.size() >= 3 && mRoot[0] == L'\\' && mRoot[1] == L'\\' && mRoot.back() == L'\\')
			mRoot.resize(mRoot.size() - 1);

		path = rootSplit;
	}

	// Parse out additional components.
	for(;;) {
		// Skip any separators.
		wchar_t c = *path++;

		while(c == L'\\' || c == L'/')
			c = *path++;

		// If we've hit a null, we're done.
		if (!c)
			break;

		// Have we hit a semicolon? If so, we've found an NTFS stream identifier and we're done.
		if (c == L';') {
			mStream = path;
			break;
		}

		// Skip until we hit a separator or a null.
		const wchar_t *compStart = path - 1;

		while(c && c != L'\\' && c != L'/' && c != L';') {
			c = *path++;
		}

		--path;

		const wchar_t *compEnd = path;

		// Check if we've got a component that starts with .
		const size_t compLen = compEnd - compStart;
		if (*compStart == L'.') {
			// Is it . (current)?
			if (compLen == 1) {
				// Yes -- just ditch it.
				continue;
			}

			// Is it .. (parent)?
			if (compLen == 2 && compStart[1] == L'.') {
				// Yes -- see if we have a previous component we can remove. If we don't have a component,
				// then what we do depends on whether we have an absolute path or not.
				if (!mComponents.empty() && (!mbIsRelative || mComponents.back() != L"..")) {
					mComponents.pop_back();
				} else if (mbIsRelative) {
					// We've got a relative path. This means that we need to preserve this ..; we push
					// it into the root section to prevent it from being backed up over by another ...
					mComponents.push_back() = L"..";
				}

				// Otherwise, we have an absolute path, and we can just drop this.
				continue;
			}
		}

		// Copy the component.
		mComponents.push_back().assign(compStart, compEnd);
	}
}

VDStringW VDParsedPath::ToString() const {
	VDStringW s(mRoot);

	bool first = true;
	for(Components::const_iterator it(mComponents.begin()), itEnd(mComponents.end()); it != itEnd; ++it) {
		if (!first)
			s += L'\\';
		else
			first = false;

		s.append(*it);
	}

	// If the string is still empty, throw in a .
	if (s.empty())
		s = L".";

	if (!mStream.empty()) {
		s += L';';
		s.append(mStream);
	}

	return s;
}

/////////////////////////////////////////////////////////////////////////////

VDStringW VDFileGetCanonicalPath(const wchar_t *path) {
	return VDParsedPath(path).ToString();
}

VDStringW VDFileGetRelativePath(const wchar_t *basePath, const wchar_t *pathToConvert, bool allowAscent) {
	VDParsedPath base(basePath);
	VDParsedPath path(pathToConvert);

	// Fail if either path is relative.
	if (base.IsRelative() || path.IsRelative())
		return VDStringW();

	// Fail if the roots don't match.
	if (vdwcsicmp(base.GetRoot(), path.GetRoot()))
		return VDStringW();

	// Figure out how many components are in common.
	size_t n1 = base.GetComponentCount();
	size_t n2 = path.GetComponentCount();
	size_t nc = 0;

	while(nc < n1 && nc < n2 && !vdwcsicmp(base.GetComponent(nc), path.GetComponent(nc)))
		++nc;

	// Check how many extra components are in the base; these need to become .. identifiers.
	VDParsedPath relPath;

	if (n1 > nc) {
		if (!allowAscent)
			return VDStringW();

		while(n1 > nc) {
			relPath.AddComponent(L"..");
			--n1;
		}
	}

	// Append extra components from path.
	while(nc < n2) {
		relPath.AddComponent(path.GetComponent(nc++));
	}

	// Copy stream.
	relPath.SetStream(path.GetStream());

	return relPath.ToString();
}

bool VDFileIsRelativePath(const wchar_t *path) {
	VDParsedPath ppath(path);

	return ppath.IsRelative();
}

VDStringW VDFileResolvePath(const wchar_t *basePath, const wchar_t *pathToResolve) {
	if (VDFileIsRelativePath(pathToResolve))
		return VDFileGetCanonicalPath(VDMakePath(basePath, pathToResolve).c_str());

	return VDStringW(pathToResolve);
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

void VDRemoveDirectory(const wchar_t *path) {
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
		succeeded = RemoveDirectoryW(path);
	} else {
		succeeded = RemoveDirectoryA(VDTextWToA(path).c_str());
	}

	if (!succeeded)
		throw MyWin32Error("Cannot remove directory: %%s", GetLastError());
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

void VDMoveFile(const wchar_t *srcPath, const wchar_t *dstPath) {
	bool success;
	if (VDIsWindowsNT()) {
		success = MoveFileW(srcPath, dstPath) != 0;
	} else {
		success = MoveFileA(VDTextWToA(srcPath).c_str(), VDTextWToA(dstPath).c_str()) != 0;
	}

	if (!success)
		throw MyWin32Error("Cannot rename \"%ls\" to \"%ls\": %%s", GetLastError(), srcPath, dstPath);
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

VDStringW VDGetLongPathA(const wchar_t *s) {
	const VDStringA& pathA = VDTextWToA(s);
	CHAR buf[MAX_PATH];

	DWORD len = GetLongPathNameA(pathA.c_str(), buf, MAX_PATH);
	VDStringW longPath;

	if (!len)
		longPath = s;
	else if (len <= MAX_PATH)
		longPath = VDTextAToW(buf);
	else if (len > MAX_PATH) {
		vdfastvector<CHAR> extbuf(len, 0);

		DWORD len2 = GetLongPathNameA(pathA.c_str(), extbuf.data(), len);

		if (len2 && len2 <= len)
			longPath = VDTextAToW(extbuf.data());
		else
			longPath = s;
	}

	return longPath;
}

VDStringW VDGetLongPathW(const wchar_t *s) {
	WCHAR buf[MAX_PATH];

	DWORD len = GetLongPathNameW(s, buf, MAX_PATH);
	VDStringW longPath;

	if (!len)
		longPath = s;
	else if (len <= MAX_PATH)
		longPath = buf;
	else if (len > MAX_PATH) {
		vdfastvector<WCHAR> extbuf(len, 0);

		DWORD len2 = GetLongPathNameW(s, extbuf.data(), len);

		if (len2 && len2 <= len)
			longPath = extbuf.data();
		else
			longPath = s;
	}

	return longPath;
}

#if VD_CPU_X86
	VDStringW VDGetLongPathAutodetect(const wchar_t *s) {
		VDGetLongPath = VDIsWindowsNT() ? VDGetLongPathW : VDGetLongPathA;

		return VDGetLongPath(s);
	}

	extern VDStringW (*VDGetLongPath)(const wchar_t *path) = VDGetLongPathAutodetect;
#else
	extern VDStringW (*VDGetLongPath)(const wchar_t *path) = VDGetLongPathW;
#endif

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

bool VDFileIsPathEqual(const wchar_t *path1, const wchar_t *path2) {
	// check for UNC paths
	if (path1[0] == '\\' && path1[1] == '\\' && path1[2] != '\\' &&
		path2[0] == '\\' && path2[1] == '\\' && path2[2] != '\\')
	{
		path1 += 2;
		path2 += 2;
	}

	for(;;) {
		wchar_t c = *path1++;
		wchar_t d = *path2++;

		if (c == '\\' || c == '/') {
			c = '/';

			while(*path1 == '\\' || *path1 == '/')
				++path1;

			if (!*path1)
				c = 0;
		} else
			c = towupper(c);

		if (d == '\\' || d == '/') {
			d = '/';

			while(*path2 == '\\' || *path2 == '/')
				++path2;

			if (!*path2)
				d = 0;
		} else
			d = towupper(d);

		if (c != d)
			return false;

		if (!c)
			return true;
	}
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

VDStringW VDGetProgramFilePath() {
	union {
		wchar_t w[MAX_PATH];
		char a[MAX_PATH];
	} buf;

	VDStringW wstr;

	if (VDIsWindowsNT()) {
		if (!GetModuleFileNameW(NULL, buf.w, MAX_PATH))
			throw MyWin32Error("Unable to get program path: %%s", GetLastError());

		wstr = buf.w;
	} else {
		if (GetModuleFileNameA(NULL, buf.a, MAX_PATH))
			throw MyWin32Error("Unable to get program path: %%s", GetLastError());

		wstr = VDTextAToW(buf.a, -1);
	}

	return wstr;
}

VDStringW VDGetSystemPath9x() {
	char path[MAX_PATH];

	if (!GetSystemDirectoryA(path, MAX_PATH))
		throw MyWin32Error("Cannot locate system directory: %%s", GetLastError());

	return VDTextAToW(path);
}

VDStringW VDGetSystemPathNT() {
	wchar_t path[MAX_PATH];

	if (!GetSystemDirectoryW(path, MAX_PATH))
		throw MyWin32Error("Cannot locate system directory: %%s", GetLastError());

	return VDStringW(path);
}

VDStringW VDGetSystemPath() {
	if (VDIsWindowsNT())
		return VDGetSystemPathNT();

	return VDGetSystemPath9x();
}

void VDGetRootPaths(vdvector<VDStringW>& paths) {
	union {
		WCHAR w[512];
		CHAR a[1024];
	} buf;

	if (VDIsWindowsNT()) {
		vdfastvector<WCHAR> heapbufw;
		WCHAR *pw = buf.w;
		DWORD wlen = vdcountof(buf.w);

		for(;;) {
			*pw = 0;

			DWORD r = GetLogicalDriveStringsW(wlen, pw);

			if (!r)
				return;

			if (r <= wlen)
				break;

			heapbufw.resize(r);
			wlen = r;
			pw = heapbufw.data();
		}

		while(*pw) {
			paths.push_back() = pw;
			pw += wcslen(pw) + 1;
		}
	} else {
		vdfastvector<CHAR> heapbufa;
		CHAR *pa = buf.a;
		DWORD alen = vdcountof(buf.a);

		for(;;) {
			*pa = 0;

			DWORD r = GetLogicalDriveStringsA(alen, pa);

			if (!r)
				return;

			if (r <= alen)
				break;

			heapbufa.resize(r);
			alen = r;
			pa = heapbufa.data();
		}

		while(*pa) {
			paths.push_back() = VDTextAToW(pa);
			pa += strlen(pa) + 1;
		}
	}
}

VDStringW VDGetRootVolumeLabel(const wchar_t *rootPath) {
	union {
		char a[MAX_PATH * 2];
		wchar_t w[MAX_PATH];
	} buf;

	DWORD maxComponentLength;
	DWORD fsFlags;
	VDStringW name;

	if (VDIsWindowsNT()) {
		if (GetVolumeInformationW(rootPath, buf.w, vdcountof(buf.w), NULL, &maxComponentLength, &fsFlags, NULL, 0))
			name = buf.w;
	} else {
		if (GetVolumeInformationA(VDTextWToA(rootPath).c_str(), buf.a, vdcountof(buf.a), NULL, &maxComponentLength, &fsFlags, NULL, 0))
			name = VDTextAToW(buf.a);
	}

	return name;
}

uint32 VDFileGetAttributesFromNativeW32(uint32 nativeAttrs) {
	if (nativeAttrs == INVALID_FILE_ATTRIBUTES)
		return kVDFileAttr_Invalid;

	uint32 attrs = 0;

	if (nativeAttrs & FILE_ATTRIBUTE_READONLY)
		attrs |= kVDFileAttr_ReadOnly;

	if (nativeAttrs & FILE_ATTRIBUTE_SYSTEM)
		attrs |= kVDFileAttr_System;

	if (nativeAttrs & FILE_ATTRIBUTE_HIDDEN)
		attrs |= kVDFileAttr_Hidden;

	if (nativeAttrs & FILE_ATTRIBUTE_ARCHIVE)
		attrs |= kVDFileAttr_Archive;

	if (nativeAttrs & FILE_ATTRIBUTE_DIRECTORY)
		attrs |= kVDFileAttr_Directory;

	return attrs;
}

uint32 VDFileGetNativeAttributesFromAttrsW32(uint32 attrs) {
	uint32 nativeAttrs = 0;

	if (attrs == kVDFileAttr_Invalid)
		return INVALID_FILE_ATTRIBUTES;

	if (attrs & kVDFileAttr_ReadOnly)
		nativeAttrs |= FILE_ATTRIBUTE_READONLY;

	if (attrs & kVDFileAttr_System)
		nativeAttrs |= FILE_ATTRIBUTE_SYSTEM;

	if (attrs & kVDFileAttr_Hidden)
		nativeAttrs |= FILE_ATTRIBUTE_HIDDEN;

	if (attrs & kVDFileAttr_Archive)
		nativeAttrs |= FILE_ATTRIBUTE_ARCHIVE;

	if (attrs & kVDFileAttr_Directory)
		nativeAttrs |= FILE_ATTRIBUTE_DIRECTORY;

	return nativeAttrs;
}

uint32 VDFileGetAttributes(const wchar_t *path) {
	uint32 nativeAttrs = 0;
	if (VDIsWindowsNT())
		nativeAttrs = ::GetFileAttributesW(path);
	else
		nativeAttrs = ::GetFileAttributesA(VDTextWToA(path).c_str());

	return VDFileGetAttributesFromNativeW32(nativeAttrs);
}

void VDFileSetAttributes(const wchar_t *path, uint32 attrsToChange, uint32 newAttrs) {
	const uint32 nativeAttrMask = VDFileGetNativeAttributesFromAttrsW32(attrsToChange);
	const uint32 nativeAttrVals = VDFileGetNativeAttributesFromAttrsW32(newAttrs);

	if (VDIsWindowsNT()) {
		DWORD nativeAttrs = ::GetFileAttributesW(path);

		if (nativeAttrs != INVALID_FILE_ATTRIBUTES) {
			nativeAttrs ^= (nativeAttrs ^ nativeAttrVals) & nativeAttrMask;

			if (::SetFileAttributesW(path, nativeAttrs))
				return;
		}
	} else {
		VDStringA pathA(VDTextWToA(path));

		DWORD nativeAttrs = ::GetFileAttributesA(pathA.c_str());

		if (nativeAttrs != INVALID_FILE_ATTRIBUTES) {
			nativeAttrs ^= (nativeAttrs ^ nativeAttrVals) & nativeAttrMask;

			if (::SetFileAttributesA(pathA.c_str(), nativeAttrs))
				return;
		}
	}

	throw MyWin32Error("Cannot change attributes on \"%ls\": %%s.", GetLastError(), path);
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

	uint32 attribs;

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
		mLastWriteDate.mTicks = wfd.a.ftLastWriteTime.dwLowDateTime + ((uint64)wfd.a.ftLastWriteTime.dwHighDateTime << 32);

		attribs = wfd.a.dwFileAttributes;
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
		mLastWriteDate.mTicks = wfd.w.ftLastWriteTime.dwLowDateTime + ((uint64)wfd.w.ftLastWriteTime.dwHighDateTime << 32);

		attribs = wfd.w.dwFileAttributes;
	}

	mAttributes = VDFileGetAttributesFromNativeW32(attribs);
	return true;
}

bool VDDirectoryIterator::IsDotDirectory() const {
	if (!mbDirectory)
		return false;

	const wchar_t *s = mFilename.c_str();

	if (s[0] != L'.')
		return false;

	return !s[1] || (s[1] == L'.' && !s[2]);
}
