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

#ifndef f_VD2_SYSTEM_REGISTRY_H
#define f_VD2_SYSTEM_REGISTRY_H

#include <vd2/system/VDString.h>

class VDRegistryKey {
private:
	void *pHandle;

public:
	VDRegistryKey(const char *pszKey, bool global = false, bool write = true);
	~VDRegistryKey();

	void *getRawHandle() const { return pHandle; }

	bool isReady() const { return pHandle != 0; }

	bool setBool(const char *pszName, bool) const;
	bool setInt(const char *pszName, int) const;
	bool setString(const char *pszName, const char *pszString) const;
	bool setString(const char *pszName, const wchar_t *pszString) const;
	bool setBinary(const char *pszName, const char *data, int len) const;

	bool getBool(const char *pszName, bool def=false) const;
	int getInt(const char *pszName, int def=0) const;
	int getEnumInt(const char *pszName, int maxVal, int def=0) const;
	bool getString(const char *pszName, VDStringA& s) const;
	bool getString(const char *pszName, VDStringW& s) const;

	int getBinaryLength(const char *pszName) const;
	bool getBinary(const char *pszName, char *buf, int maxlen) const;

	bool removeValue(const char *);
};

class VDRegistryValueIterator {
public:
	VDRegistryValueIterator(const VDRegistryKey& key);

	const char *Next();

protected:
	void *mpHandle;
	uint32 mIndex;
	char mName[256];
};

class VDRegistryAppKey : public VDRegistryKey {
private:
	static VDString s_appbase;

public:
	VDRegistryAppKey();
	VDRegistryAppKey(const char *pszKey, bool write = true);

	static void setDefaultKey(const char *pszAppName);
};

#endif
