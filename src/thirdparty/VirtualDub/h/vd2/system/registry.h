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

class IVDRegistryProvider {
public:
	enum Type {
		kTypeUnknown,
		kTypeInt,
		kTypeString,
		kTypeBinary
	};

	virtual void *GetMachineKey() = 0;
	virtual void *GetUserKey() = 0;
	virtual void *CreateKey(void *key, const char *path, bool write) = 0;
	virtual void CloseKey(void *key) = 0;

	virtual bool SetBool(void *key, const char *name, bool) = 0;
	virtual bool SetInt(void *key, const char *name, int) = 0;
	virtual bool SetString(void *key, const char *name, const char *pszString) = 0;
	virtual bool SetString(void *key, const char *name, const wchar_t *pszString) = 0;
	virtual bool SetBinary(void *key, const char *name, const char *data, int len) = 0;

	virtual Type GetType(void *key, const char *name) = 0;
	virtual bool GetBool(void *key, const char *name, bool& val) = 0;
	virtual bool GetInt(void *key, const char *name, int& val) = 0;
	virtual bool GetString(void *key, const char *name, VDStringA& s) = 0;
	virtual bool GetString(void *key, const char *name, VDStringW& s) = 0;

	virtual int GetBinaryLength(void *key, const char *name) = 0;
	virtual bool GetBinary(void *key, const char *name, char *buf, int maxlen) = 0;

	virtual bool RemoveValue(void *key, const char *name) = 0;
	virtual bool RemoveKey(void *key, const char *name) = 0;

	virtual void *EnumKeysBegin(void *key) = 0;
	virtual const char *EnumKeysNext(void *enumerator) = 0;
	virtual void EnumKeysClose(void *enumerator) = 0;

	virtual void *EnumValuesBegin(void *key) = 0;
	virtual const char *EnumValuesNext(void *enumerator) = 0;
	virtual void EnumValuesClose(void *enumerator) = 0;
};

IVDRegistryProvider *VDGetRegistryProvider();
void VDSetRegistryProvider(IVDRegistryProvider *provider);

///////////////////////////////////////////////////////////////////////////

class VDRegistryKey {
public:
	enum Type {
		kTypeUnknown,
		kTypeInt,
		kTypeString,
		kTypeBinary
	};

	VDRegistryKey(const char *pszKey, bool global = false, bool write = true);
	VDRegistryKey(VDRegistryKey& baseKey, const char *name, bool write = true);
	~VDRegistryKey();

	void *getRawHandle() const { return mKey; }

	bool isReady() const { return mKey != 0; }

	bool setBool(const char *name, bool) const;
	bool setInt(const char *name, int) const;
	bool setString(const char *name, const char *pszString) const;
	bool setString(const char *name, const wchar_t *pszString) const;
	bool setBinary(const char *name, const char *data, int len) const;

	Type getValueType(const char *name) const;

	bool getBool(const char *name, bool def=false) const;
	int getInt(const char *name, int def=0) const;
	int getEnumInt(const char *name, int maxVal, int def=0) const;
	bool getString(const char *name, VDStringA& s) const;
	bool getString(const char *name, VDStringW& s) const;

	int getBinaryLength(const char *name) const;
	bool getBinary(const char *name, char *buf, int maxlen) const;

	bool removeValue(const char *);
    bool removeKey(const char *);

private:
	void *mKey;
};

class VDRegistryValueIterator {
	VDRegistryValueIterator(const VDRegistryValueIterator&);
	VDRegistryValueIterator& operator=(const VDRegistryValueIterator&);
public:
	VDRegistryValueIterator(const VDRegistryKey& key);
	~VDRegistryValueIterator();

	const char *Next();

protected:
	void *mEnumerator;
};

class VDRegistryKeyIterator {
	VDRegistryKeyIterator(const VDRegistryKeyIterator& key);
	VDRegistryKeyIterator& operator=(const VDRegistryKeyIterator& key);
public:
	VDRegistryKeyIterator(const VDRegistryKey& key);
	~VDRegistryKeyIterator();

	const char *Next();

protected:
	void *mEnumerator;
};

class VDRegistryAppKey : public VDRegistryKey {
private:
	static VDString s_appbase;

public:
	VDRegistryAppKey();
	VDRegistryAppKey(const char *pszKey, bool write = true, bool global = false);

	static void setDefaultKey(const char *pszAppName);
};

#endif
