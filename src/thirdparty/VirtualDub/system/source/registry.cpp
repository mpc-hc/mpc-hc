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
#include <windows.h>

#include <vd2/system/VDString.h>
#include <vd2/system/registry.h>
#include <vd2/system/w32assist.h>

///////////////////////////////////////////////////////////////////////////

class VDRegistryProviderW32 : public IVDRegistryProvider {
public:
	void *GetUserKey();
	void *GetMachineKey();
	void *CreateKey(void *key, const char *path, bool write);
	void CloseKey(void *key);

	bool SetBool(void *key, const char *pszName, bool);
	bool SetInt(void *key, const char *pszName, int);
	bool SetString(void *key, const char *pszName, const char *pszString);
	bool SetString(void *key, const char *pszName, const wchar_t *pszString);
	bool SetBinary(void *key, const char *pszName, const char *data, int len);

	Type GetType(void *key, const char *name);
	bool GetBool(void *key, const char *pszName, bool& val);
	bool GetInt(void *key, const char *pszName, int& val);
	bool GetString(void *key, const char *pszName, VDStringA& s);
	bool GetString(void *key, const char *pszName, VDStringW& s);

	int GetBinaryLength(void *key, const char *pszName);
	bool GetBinary(void *key, const char *pszName, char *buf, int maxlen);

	bool RemoveValue(void *key, const char *name);
    bool RemoveKey(void *key, const char *name);

	void *EnumKeysBegin(void *key);
	const char *EnumKeysNext(void *enumerator);
	void EnumKeysClose(void *enumerator);

	void *EnumValuesBegin(void *key);
	const char *EnumValuesNext(void *enumerator);
	void EnumValuesClose(void *enumerator);

protected:
	struct KeyEnumerator {
		void *mKey;
		uint32 mIndex;
		char mName[256];
	};

	struct ValueEnumerator {
		void *mKey;
		uint32 mIndex;
		char mName[256];
	};
};

void *VDRegistryProviderW32::GetUserKey() {
	return HKEY_CURRENT_USER;
}

void *VDRegistryProviderW32::GetMachineKey() {
	return HKEY_LOCAL_MACHINE;
}

void *VDRegistryProviderW32::CreateKey(void *key, const char *path, bool write) {
	HKEY newKey;

	if (write) {
		if (RegCreateKeyEx((HKEY)key, path, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &newKey, NULL))
			return NULL;
	} else {
		if (RegOpenKeyEx((HKEY)key, path, 0, KEY_READ, &newKey))
			return NULL;
	}

	return newKey;
}

void VDRegistryProviderW32::CloseKey(void *key) {
	RegCloseKey((HKEY)key);
}

bool VDRegistryProviderW32::SetBool(void *key, const char *pszName, bool val) {
	DWORD dw = val;
	
	return !RegSetValueEx((HKEY)key, pszName, 0, REG_DWORD, (const BYTE *)&dw, sizeof dw);
}

bool VDRegistryProviderW32::SetInt(void *key, const char *pszName, int val) {
	DWORD dw = val;

	return !RegSetValueEx((HKEY)key, pszName, 0, REG_DWORD, (const BYTE *)&dw, sizeof dw);
}

bool VDRegistryProviderW32::SetString(void *key, const char *pszName, const char *pszString) {
	return !RegSetValueEx((HKEY)key, pszName, 0, REG_SZ, (const BYTE *)pszString, strlen(pszString));
}

bool VDRegistryProviderW32::SetString(void *key, const char *pszName, const wchar_t *pszString) {
	if (!VDIsWindowsNT()) {
		VDStringA s(VDTextWToA(pszString));

		if (RegSetValueEx((HKEY)key, pszName, 0, REG_SZ, (const BYTE *)s.data(), s.size()))
			return false;
	} else {
		if (RegSetValueExW((HKEY)key, VDTextAToW(pszName).c_str(), 0, REG_SZ, (const BYTE *)pszString, sizeof(wchar_t) * wcslen(pszString)))
			return false;
	}

	return true;
}

bool VDRegistryProviderW32::SetBinary(void *key, const char *pszName, const char *data, int len) {
	return !RegSetValueEx((HKEY)key, pszName, 0, REG_BINARY, (const BYTE *)data, len);
}

IVDRegistryProvider::Type VDRegistryProviderW32::GetType(void *key, const char *name) {
	DWORD type;

	if (RegQueryValueEx((HKEY)key, name, 0, &type, NULL, NULL))
		return kTypeUnknown;

	switch(type) {
		case REG_SZ:
			return kTypeString;

		case REG_BINARY:
			return kTypeBinary;

		case REG_DWORD:
			return kTypeInt;

		default:
			return kTypeUnknown;
	}
}

bool VDRegistryProviderW32::GetBool(void *key, const char *pszName, bool& val) {
	DWORD type;
	DWORD v;
	DWORD len = sizeof(DWORD);

	if (RegQueryValueEx((HKEY)key, pszName, 0, &type, (BYTE *)&v, &len) || type != REG_DWORD)
		return false;

	val = (v != 0);
	return true;
}

bool VDRegistryProviderW32::GetInt(void *key, const char *pszName, int& val) {
	DWORD type;
	DWORD v;
	DWORD len = sizeof(DWORD);

	if (RegQueryValueEx((HKEY)key, pszName, 0, &type, (BYTE *)&v, &len) || type != REG_DWORD)
		return false;

	val = v;
	return true;
}

bool VDRegistryProviderW32::GetString(void *key, const char *pszName, VDStringA& str) {
	DWORD type;
	DWORD s = sizeof(DWORD);

	if (RegQueryValueEx((HKEY)key, pszName, 0, &type, NULL, &s) || type != REG_SZ)
		return false;

	str.resize(s);
	if (RegQueryValueEx((HKEY)key, pszName, 0, NULL, (BYTE *)str.data(), &s))
		return false;

	if (!s)
		str.clear();
	else
		str.resize(strlen(str.c_str()));		// Trim off pesky terminating NULLs.

	return true;
}

bool VDRegistryProviderW32::GetString(void *key, const char *pszName, VDStringW& str) {
	if (!VDIsWindowsNT()) {
		VDStringA v;
		if (!GetString(key, pszName, v))
			return false;
		str = VDTextAToW(v);
		return true;
	}

	const VDStringW wsName(VDTextAToW(pszName));
	DWORD type;
	DWORD s = sizeof(DWORD);

	if (RegQueryValueExW((HKEY)key, wsName.c_str(), 0, &type, NULL, &s) || type != REG_SZ)
		return false;

	if (s <= 0)
		str.clear();
	else {
		str.resize((s + sizeof(wchar_t) - 1) / sizeof(wchar_t));

		if (RegQueryValueExW((HKEY)key, wsName.c_str(), 0, NULL, (BYTE *)&str[0], &s))
			return false;

		str.resize(wcslen(str.c_str()));		// Trim off pesky terminating NULLs.
	}

	return true;
}

int VDRegistryProviderW32::GetBinaryLength(void *key, const char *pszName) {
	DWORD type;
	DWORD s = sizeof(DWORD);

	if (RegQueryValueEx((HKEY)key, pszName, 0, &type, NULL, &s) || type != REG_BINARY)
		return -1;

	return s;
}

bool VDRegistryProviderW32::GetBinary(void *key, const char *pszName, char *buf, int maxlen) {
	DWORD type;
	DWORD s = maxlen;

	if (RegQueryValueEx((HKEY)key, pszName, 0, &type, (BYTE *)buf, &s) || maxlen < (int)s || type != REG_BINARY)
		return false;

	return true;
}

bool VDRegistryProviderW32::RemoveValue(void *key, const char *name) {
	return 0 != RegDeleteValue((HKEY)key, name);
}

bool VDRegistryProviderW32::RemoveKey(void *key, const char *name) {
	return 0 != RegDeleteKey((HKEY)key, name);
}

void *VDRegistryProviderW32::EnumKeysBegin(void *key) {
	KeyEnumerator *ke = new KeyEnumerator;

	ke->mKey = key;
	ke->mIndex = 0;

	return ke;
}

const char *VDRegistryProviderW32::EnumKeysNext(void *enumerator) {
	KeyEnumerator *ke = (KeyEnumerator *)enumerator;

	DWORD len = sizeof(ke->mName)/sizeof(ke->mName[0]);
	FILETIME ft;
	LONG error = RegEnumKeyExA((HKEY)ke->mKey, ke->mIndex, ke->mName, &len, NULL, NULL, NULL, &ft);

	if (error)
		return NULL;

	++ke->mIndex;
	return ke->mName;
}

void VDRegistryProviderW32::EnumKeysClose(void *enumerator) {
	delete (KeyEnumerator *)enumerator;
}

void *VDRegistryProviderW32::EnumValuesBegin(void *key) {
	ValueEnumerator *ve = new ValueEnumerator;

	ve->mKey = key;
	ve->mIndex = 0;

	return ve;
}

const char *VDRegistryProviderW32::EnumValuesNext(void *enumerator) {
	ValueEnumerator *ve = (ValueEnumerator *)enumerator;

	DWORD len = sizeof(ve->mName)/sizeof(ve->mName[0]);
	LONG error = RegEnumValueA((HKEY)ve->mKey, ve->mIndex, ve->mName, &len, NULL, NULL, NULL, NULL);

	if (error)
		return NULL;

	++ve->mIndex;
	return ve->mName;
}

void VDRegistryProviderW32::EnumValuesClose(void *enumerator) {
	delete (ValueEnumerator *)enumerator;
}

///////////////////////////////////////////////////////////////////////////

VDRegistryProviderW32 g_VDRegistryProviderW32;
IVDRegistryProvider *g_pVDRegistryProvider = &g_VDRegistryProviderW32;

IVDRegistryProvider *VDGetRegistryProvider() {
	return g_pVDRegistryProvider;
}

void VDSetRegistryProvider(IVDRegistryProvider *provider) {
	g_pVDRegistryProvider = provider;
}

///////////////////////////////////////////////////////////////////////////

VDRegistryKey::VDRegistryKey(const char *keyName, bool global, bool write) {
	IVDRegistryProvider *provider = VDGetRegistryProvider();
	void *rootKey = global ? provider->GetMachineKey() : provider->GetUserKey();

	mKey = provider->CreateKey(rootKey, keyName, write);
}

VDRegistryKey::VDRegistryKey(VDRegistryKey& baseKey, const char *name, bool write) {
	IVDRegistryProvider *provider = VDGetRegistryProvider();
	void *rootKey = baseKey.getRawHandle();

	mKey = rootKey ? provider->CreateKey(rootKey, name, write) : NULL;
}

VDRegistryKey::~VDRegistryKey() {
	if (mKey)
		VDGetRegistryProvider()->CloseKey(mKey);
}

bool VDRegistryKey::setBool(const char *name, bool v) const {
	return mKey && VDGetRegistryProvider()->SetBool(mKey, name, v);
}

bool VDRegistryKey::setInt(const char *name, int i) const {
	return mKey && VDGetRegistryProvider()->SetInt(mKey, name, i);
}

bool VDRegistryKey::setString(const char *name, const char *s) const {
	return mKey && VDGetRegistryProvider()->SetString(mKey, name, s);
}

bool VDRegistryKey::setString(const char *name, const wchar_t *s) const {
	return mKey && VDGetRegistryProvider()->SetString(mKey, name, s);
}

bool VDRegistryKey::setBinary(const char *name, const char *data, int len) const {
	return mKey && VDGetRegistryProvider()->SetBinary(mKey, name, data, len);
}

VDRegistryKey::Type VDRegistryKey::getValueType(const char *name) const {
	Type type = kTypeUnknown;

	if (mKey) {
		switch(VDGetRegistryProvider()->GetType(mKey, name)) {
			case IVDRegistryProvider::kTypeInt:
				type = kTypeInt;
				break;

			case IVDRegistryProvider::kTypeString:
				type = kTypeString;
				break;

			case IVDRegistryProvider::kTypeBinary:
				type = kTypeBinary;
				break;
		}
	}

	return type;
}

bool VDRegistryKey::getBool(const char *name, bool def) const {
	bool v;
	return mKey && VDGetRegistryProvider()->GetBool(mKey, name, v) ? v : def;
}

int VDRegistryKey::getInt(const char *name, int def) const {
	int v;
	return mKey && VDGetRegistryProvider()->GetInt(mKey, name, v) ? v : def;
}

int VDRegistryKey::getEnumInt(const char *pszName, int maxVal, int def) const {
	int v = getInt(pszName, def);

	if (v<0 || v>=maxVal)
		v = def;

	return v;
}

bool VDRegistryKey::getString(const char *name, VDStringA& str) const {
	return mKey && VDGetRegistryProvider()->GetString(mKey, name, str);
}

bool VDRegistryKey::getString(const char *name, VDStringW& str) const {
	return mKey && VDGetRegistryProvider()->GetString(mKey, name, str);
}

int VDRegistryKey::getBinaryLength(const char *name) const {
	return mKey ? VDGetRegistryProvider()->GetBinaryLength(mKey, name) : -1;
}

bool VDRegistryKey::getBinary(const char *name, char *buf, int maxlen) const {
	return mKey && VDGetRegistryProvider()->GetBinary(mKey, name, buf, maxlen);
}

bool VDRegistryKey::removeValue(const char *name) {
	return mKey && VDGetRegistryProvider()->RemoveValue(mKey, name);
}

bool VDRegistryKey::removeKey(const char *name) {
	return mKey && VDGetRegistryProvider()->RemoveKey(mKey, name);
}

///////////////////////////////////////////////////////////////////////////////

VDRegistryValueIterator::VDRegistryValueIterator(const VDRegistryKey& key)
	: mEnumerator(VDGetRegistryProvider()->EnumValuesBegin(key.getRawHandle()))
{
}

VDRegistryValueIterator::~VDRegistryValueIterator() {
	VDGetRegistryProvider()->EnumValuesClose(mEnumerator);
}

const char *VDRegistryValueIterator::Next() {
	return mEnumerator ? VDGetRegistryProvider()->EnumValuesNext(mEnumerator) : NULL;
}

///////////////////////////////////////////////////////////////////////////////

VDRegistryKeyIterator::VDRegistryKeyIterator(const VDRegistryKey& key)
	: mEnumerator(VDGetRegistryProvider()->EnumKeysBegin(key.getRawHandle()))
{
}

VDRegistryKeyIterator::~VDRegistryKeyIterator() {
	VDGetRegistryProvider()->EnumKeysClose(mEnumerator);
}

const char *VDRegistryKeyIterator::Next() {
	return mEnumerator ? VDGetRegistryProvider()->EnumKeysNext(mEnumerator) : NULL;
}

///////////////////////////////////////////////////////////////////////////////

VDString VDRegistryAppKey::s_appbase;

VDRegistryAppKey::VDRegistryAppKey() : VDRegistryKey(s_appbase.c_str()) {
}

VDRegistryAppKey::VDRegistryAppKey(const char *pszKey, bool write, bool global)
	: VDRegistryKey((s_appbase + pszKey).c_str(), global, write)
{
}

void VDRegistryAppKey::setDefaultKey(const char *pszAppName) {
	s_appbase = pszAppName;
}
