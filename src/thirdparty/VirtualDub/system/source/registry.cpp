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

VDRegistryKey::VDRegistryKey(const char *keyName, bool global, bool write) {
	const HKEY rootKey = global ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;

	if (write) {
		if (RegCreateKeyEx(rootKey, keyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, (PHKEY)&pHandle, NULL))
			pHandle = NULL;
	} else {
		if (RegOpenKeyEx(rootKey, keyName, 0, KEY_READ, (PHKEY)&pHandle))
			pHandle = NULL;
	}
}

VDRegistryKey::~VDRegistryKey() {
	if (pHandle)
		RegCloseKey((HKEY)pHandle);
}

bool VDRegistryKey::setBool(const char *pszName, bool v) const {
	if (pHandle) {
		DWORD dw = v;

		if (RegSetValueEx((HKEY)pHandle, pszName, 0, REG_DWORD, (const BYTE *)&dw, sizeof dw))
			return true;
	}

	return false;
}

bool VDRegistryKey::setInt(const char *pszName, int i) const {
	if (pHandle) {
		DWORD dw = i;

		if (RegSetValueEx((HKEY)pHandle, pszName, 0, REG_DWORD, (const BYTE *)&dw, sizeof dw))
			return true;
	}

	return false;
}

bool VDRegistryKey::setString(const char *pszName, const char *pszString) const {
	if (pHandle) {
		if (RegSetValueEx((HKEY)pHandle, pszName, 0, REG_SZ, (const BYTE *)pszString, strlen(pszString)))
			return true;
	}

	return false;
}

bool VDRegistryKey::setString(const char *pszName, const wchar_t *pszString) const {
	if (pHandle) {
		if (GetVersion() & 0x80000000) {
			VDStringA s(VDTextWToA(pszString));

			if (RegSetValueEx((HKEY)pHandle, pszName, 0, REG_SZ, (const BYTE *)s.data(), s.size()))
				return true;
		} else {
			if (RegSetValueExW((HKEY)pHandle, VDTextAToW(pszName).c_str(), 0, REG_SZ, (const BYTE *)pszString, sizeof(wchar_t) * wcslen(pszString)))
				return true;
		}
	}

	return false;
}

bool VDRegistryKey::setBinary(const char *pszName, const char *data, int len) const {
	if (pHandle) {
		if (RegSetValueEx((HKEY)pHandle, pszName, 0, REG_BINARY, (const BYTE *)data, len))
			return true;
	}

	return false;
}

bool VDRegistryKey::getBool(const char *pszName, bool def) const {
	DWORD type, v, s=sizeof(DWORD);

	if (!pHandle || RegQueryValueEx((HKEY)pHandle, pszName, 0, &type, (BYTE *)&v, &s)
		|| type != REG_DWORD)
		return def;

	return v != 0;
}

int VDRegistryKey::getInt(const char *pszName, int def) const {
	DWORD type, v, s=sizeof(DWORD);

	if (!pHandle || RegQueryValueEx((HKEY)pHandle, pszName, 0, &type, (BYTE *)&v, &s)
		|| type != REG_DWORD)
		return def;

	return (int)v;
}

int VDRegistryKey::getEnumInt(const char *pszName, int maxVal, int def) const {
	int v = getInt(pszName, def);

	if (v<0 || v>=maxVal)
		v = def;

	return v;
}

bool VDRegistryKey::getString(const char *pszName, VDStringA& str) const {
	DWORD type, s = sizeof(DWORD);

	if (!pHandle || RegQueryValueEx((HKEY)pHandle, pszName, 0, &type, NULL, &s) || type != REG_SZ)
		return false;

	str.resize(s);
	if (RegQueryValueEx((HKEY)pHandle, pszName, 0, NULL, (BYTE *)str.data(), &s))
		return false;

	if (!s)
		str.clear();
	else
		str.resize(strlen(str.c_str()));		// Trim off pesky terminating NULLs.

	return true;
}

bool VDRegistryKey::getString(const char *pszName, VDStringW& str) const {
	if (!pHandle)
		return false;

	if (GetVersion() & 0x80000000) {
		VDStringA v;
		if (!getString(pszName, v))
			return false;
		str = VDTextAToW(v);
		return true;
	}

	const VDStringW wsName(VDTextAToW(pszName));
	DWORD type, s = sizeof(DWORD);

	if (!pHandle || RegQueryValueExW((HKEY)pHandle, wsName.c_str(), 0, &type, NULL, &s) || type != REG_SZ)
		return false;

	if (s <= 0)
		str.clear();
	else {
		str.resize((s + sizeof(wchar_t) - 1) / sizeof(wchar_t));

		if (RegQueryValueExW((HKEY)pHandle, wsName.c_str(), 0, NULL, (BYTE *)&str[0], &s))
			return false;

		str.resize(wcslen(str.c_str()));		// Trim off pesky terminating NULLs.
	}

	return true;
}

int VDRegistryKey::getBinaryLength(const char *pszName) const {
	DWORD type, s = sizeof(DWORD);

	if (!pHandle || RegQueryValueEx((HKEY)pHandle, pszName, 0, &type, NULL, &s)
		|| type != REG_BINARY)
		return -1;

	return s;
}

bool VDRegistryKey::getBinary(const char *pszName, char *buf, int maxlen) const {
	DWORD type, s = maxlen;

	if (!pHandle || RegQueryValueEx((HKEY)pHandle, pszName, 0, &type, (BYTE *)buf, &s) || maxlen < (int)s || type != REG_BINARY)
		return false;

	return true;
}

bool VDRegistryKey::removeValue(const char *name) {
	if (!pHandle || RegDeleteValue((HKEY)pHandle, name))
		return false;

	return true;
}

///////////////////////////////////////////////////////////////////////////////

VDRegistryValueIterator::VDRegistryValueIterator(const VDRegistryKey& key)
	: mpHandle(key.getRawHandle())
	, mIndex(0)
{
}

const char *VDRegistryValueIterator::Next() {
	DWORD len = sizeof(mName)/sizeof(mName[0]);
	LONG error = RegEnumValueA((HKEY)mpHandle, mIndex, mName, &len, NULL, NULL, NULL, NULL);

	if (error)
		return NULL;

	++mIndex;
	return mName;
}

///////////////////////////////////////////////////////////////////////////////

VDString VDRegistryAppKey::s_appbase;

VDRegistryAppKey::VDRegistryAppKey() : VDRegistryKey(s_appbase.c_str()) {
}

VDRegistryAppKey::VDRegistryAppKey(const char *pszKey, bool write)
	: VDRegistryKey((s_appbase + pszKey).c_str(), false, write)
{
}

void VDRegistryAppKey::setDefaultKey(const char *pszAppName) {
	s_appbase = pszAppName;
}
