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

#ifndef f_VD2_SYSTEM_FILESYS_H
#define f_VD2_SYSTEM_FILESYS_H

#include <ctype.h>
#include <vector>

#include <vd2/system/vdtypes.h>
#include <vd2/system/VDString.h>

// VDFileSplitPath returns a pointer to the first character of the filename,
// or the beginning of the string if the path only contains one component.

const char *VDFileSplitFirstDir(const char *s);
const wchar_t *VDFileSplitFirstDir(const wchar_t *s);

static inline char *VDFileSplitFirstDir(char *s) {
	return const_cast<char *>(VDFileSplitFirstDir(const_cast<const char *>(s)));
}

static inline wchar_t *VDFileSplitFirstDir(wchar_t *s) {
	return const_cast<wchar_t *>(VDFileSplitFirstDir(const_cast<const wchar_t *>(s)));
}

const char *VDFileSplitPath(const char *);
const wchar_t *VDFileSplitPath(const wchar_t *);

static inline char *VDFileSplitPath(char *s) {
	return const_cast<char *>(VDFileSplitPath(const_cast<const char *>(s)));
}

static inline wchar_t *VDFileSplitPath(wchar_t *s) {
	return const_cast<wchar_t *>(VDFileSplitPath(const_cast<const wchar_t *>(s)));
}

VDString VDFileSplitPathLeft(const VDString&);
VDString VDFileSplitPathRight(const VDString&);
VDStringW VDFileSplitPathLeft(const VDStringW&);
VDStringW VDFileSplitPathRight(const VDStringW&);

// VDSplitRoot returns a pointer to the second component of the filename,
// or the beginning of the string if there is no second component.

const char *VDFileSplitRoot(const char *);
const wchar_t *VDFileSplitRoot(const wchar_t *);

static inline char *VDFileSplitRoot(char *s) {
	return const_cast<char *>(VDFileSplitRoot(const_cast<const char *>(s)));
}

static inline wchar_t *VDFileSplitRoot(wchar_t *s) {
	return const_cast<wchar_t *>(VDFileSplitRoot(const_cast<const wchar_t *>(s)));
}

VDString VDFileSplitRoot(const VDString&);
VDStringW VDFileSplitRoot(const VDStringW&);

// VDSplitExtension returns a pointer to the extension, including the period.
// The ending null terminator is returned if there is no extension.

const char *VDFileSplitExt(const char *);
const wchar_t *VDFileSplitExt(const wchar_t *);

static inline char *VDFileSplitExt(char *s) {
	return const_cast<char *>(VDFileSplitExt(const_cast<const char *>(s)));
}

static inline wchar_t *VDFileSplitExt(wchar_t *s) {
	return const_cast<wchar_t *>(VDFileSplitExt(const_cast<const wchar_t *>(s)));
}

VDString VDFileSplitExtLeft(const VDString&);
VDStringW VDFileSplitExtLeft(const VDStringW&);
VDString VDFileSplitExtRight(const VDString&);
VDStringW VDFileSplitExtRight(const VDStringW&);

/////////////////////////////////////////////////////////////////////////////

/// Perform a case-insensitive wildcard match against a filename; returns
/// true if the pattern matches, false otherwise. '?' matches any single
/// character, and '*' matches zero or more characters.
///
/// NOTE: This is not guaranteed or intended to perfectly match the
/// underlying OS wildcard mechanism. In particular, we don't try to
/// emulate MSDOS or Windows goofiness.
bool VDFileWildMatch(const char *pattern, const char *path);
bool VDFileWildMatch(const wchar_t *pattern, const wchar_t *path);

/////////////////////////////////////////////////////////////////////////////

sint64 VDGetDiskFreeSpace(const wchar_t *path);
void VDCreateDirectory(const wchar_t *path);

extern bool (*VDRemoveFile)(const wchar_t *path);

bool VDDoesPathExist(const wchar_t *fileName);

uint64 VDFileGetLastWriteTime(const wchar_t *path);
VDStringW VDFileGetRootPath(const wchar_t *partialPath);
VDStringW VDGetFullPath(const wchar_t *partialPath);

VDStringW VDMakePath(const wchar_t *base, const wchar_t *file);
void VDFileFixDirPath(VDStringW& path);
VDStringW VDGetLocalModulePath();
VDStringW VDGetProgramPath();

/////////////////////////////////////////////////////////////////////////////

class VDDirectoryIterator {
	VDDirectoryIterator(const VDDirectoryIterator&);
	VDDirectoryIterator& operator=(VDDirectoryIterator&);
public:
	VDDirectoryIterator(const wchar_t *path);
	~VDDirectoryIterator();

	bool Next();

	bool IsDirectory() const {
		return mbDirectory;
	}

	const wchar_t *GetName() const {
		return mFilename.c_str();
	}

	const VDStringW GetFullPath() const {
		return mBasePath + mFilename;
	}

	const sint64 GetSize() const {
		return mFileSize;
	}

protected:
	void *mpHandle;
	bool mbSearchComplete;

	VDStringW	mSearchPath;
	VDStringW	mBasePath;

	VDStringW	mFilename;
	sint64		mFileSize;
	bool		mbDirectory;
};

#endif
