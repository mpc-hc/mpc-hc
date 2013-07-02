//	VirtualDub - Video processing and capture application
//	System library component
//	Copyright (C) 1998-2010 Avery Lee, All Rights Reserved.
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
#include <vd2/system/vdalloc.h>
#include <vd2/system/registrymemory.h>
#include <vd2/system/hash.h>

class VDRegistryProviderMemory::Value : public vdhashtable_base_node {
public:
	enum Type {
		kTypeInt,
		kTypeString,
		kTypeBinary
	};

	Value();

	void SetInt(sint32 v);
	void SetString(const wchar_t *str);
	void SetBinary(const void *p, size_t len);

	Type GetType() const { return mType; }

	bool GetInt(sint32& v) const;
	bool GetString(const wchar_t *&s) const;
	bool GetBinary(const void *&p, size_t& len) const;

protected:
	Type mType;

	// we're being lazy for now
	sint32 mInt;
	VDStringW mString;
	vdfastvector<char> mRawData;
};

VDRegistryProviderMemory::Value::Value()
	: mType(kTypeInt)
	, mInt(0)
{
}

void VDRegistryProviderMemory::Value::SetInt(sint32 v) {
	if (mType != kTypeInt) {
		mString.swap(VDStringW());
		mRawData.swap(vdfastvector<char>());
		mType = kTypeInt;
	}

	mInt = v;
}

void VDRegistryProviderMemory::Value::SetString(const wchar_t *str) {
	if (mType != kTypeString) {
		mRawData.swap(vdfastvector<char>());
		mType = kTypeString;
	}

	mString = str;
}

void VDRegistryProviderMemory::Value::SetBinary(const void *p, size_t len) {
	if (mType != kTypeBinary) {
		mString.swap(VDStringW());
		mType = kTypeBinary;
	}

	mRawData.assign((char *)p, (char *)p + len);
}

bool VDRegistryProviderMemory::Value::GetInt(sint32& v) const {
	if (mType != kTypeInt)
		return false;

	v = mInt;
	return true;
}

bool VDRegistryProviderMemory::Value::GetString(const wchar_t *&s) const {
	if (mType != kTypeString)
		return false;

	s = mString.c_str();
	return true;
}

bool VDRegistryProviderMemory::Value::GetBinary(const void *&p, size_t& len) const {
	if (mType != kTypeBinary)
		return false;

	p = mRawData.data();
	len = mRawData.size();
	return true;
}

class VDRegistryProviderMemory::Key {
public:
	Key();
	~Key();

	void AddRef();
	void Release();

	bool Add(const VDStringA& name, VDRegistryProviderMemory::Value *value);
	void Remove(VDRegistryProviderMemory::Key *key);
	bool RemoveKey(const char *name);
	bool RemoveValue(const char *name);

	const char *GetKeyName(size_t index) const {
		if (index >= mKeyList.size())
			return NULL;

		return mKeyList[index]->first.c_str();
	}

	const char *GetValueName(size_t index) const {
		if (index >= mValueList.size())
			return NULL;

		return mValueList[index]->first.c_str();
	}

	Value *OpenValue(const char *name, bool create);
	Key *OpenKey(const VDStringSpanA& name, bool create);

	int mRefCount;
	bool mbCondemned;

	Key *mpParent;

	struct KeyHash {
		size_t operator()(const VDStringA& s) const;
		size_t operator()(const char *s) const;
	};

	struct KeyPred {
		bool operator()(const VDStringA& s, const VDStringA& t) const;
		bool operator()(const VDStringA& s, const VDStringSpanA& t) const;
		bool operator()(const VDStringA& s, const char *t) const;
	};

	typedef vdhashmap<VDStringA, Key, KeyHash, KeyPred> KeyMap;
	KeyMap mKeyMap;

	typedef vdfastvector<const KeyMap::value_type *> KeyList;
	KeyList mKeyList;

	typedef vdhashmap<VDStringA, Value, KeyHash, KeyPred> ValueMap;
	ValueMap mValueMap;

	typedef vdfastvector<const ValueMap::value_type *> ValueList;
	ValueList mValueList;
};

size_t VDRegistryProviderMemory::Key::KeyHash::operator()(const VDStringA& s) const {
	return VDHashString32I(s.data(), s.size());
}

size_t VDRegistryProviderMemory::Key::KeyHash::operator()(const char *s) const {
	return VDHashString32I(s);
}

bool VDRegistryProviderMemory::Key::KeyPred::operator()(const VDStringA& s, const VDStringA& t) const {
	return s.comparei(t) == 0;
}

bool VDRegistryProviderMemory::Key::KeyPred::operator()(const VDStringA& s, const VDStringSpanA& t) const {
	return s.comparei(t) == 0;
}

bool VDRegistryProviderMemory::Key::KeyPred::operator()(const VDStringA& s, const char *t) const {
	return s.comparei(t) == 0;
}

VDRegistryProviderMemory::Key::Key()
	: mRefCount(0)
	, mbCondemned(false)
	, mpParent(NULL)
{
}

VDRegistryProviderMemory::Key::~Key() {
	VDASSERT(!mRefCount);
}

void VDRegistryProviderMemory::Key::AddRef() {
	++mRefCount;
}

void VDRegistryProviderMemory::Key::Release() {
	if (!--mRefCount && mbCondemned)
		mpParent->Remove(this);
}

bool VDRegistryProviderMemory::Key::Add(const VDStringA& name, VDRegistryProviderMemory::Value *value) {
	ValueMap::insert_return_type r(mValueMap.insert(name));
	if (!r.second)
		return false;

	mValueList.push_back(&*r.first);
	return true;
}

void VDRegistryProviderMemory::Key::Remove(VDRegistryProviderMemory::Key *key) {
	VDASSERT(key->mRefCount == 0);

	for(KeyList::iterator it(mKeyList.begin()), itEnd(mKeyList.end()); it != itEnd; ++it) {
		const KeyMap::value_type *e = *it;

		if (&e->second == key) {
			mKeyMap.erase(e->first);
			mKeyList.erase(it);
			break;
		}
	}
}

bool VDRegistryProviderMemory::Key::RemoveKey(const char *name) {
	if (!name)
		name = "";

	// look up the subkey
	KeyMap::iterator it(mKeyMap.find_as(name));
	
	// fail if not found
	if (it != mKeyMap.end())
		return false;

	// can't delete key if it has subkeys
	Key& key = it->second;
	if (!key.mKeyList.empty())
		return false;
	
	// if the key is open, we have to condemn it and delete it later
	if (key.mRefCount) {
		key.mbCondemned = true;
		return true;
	}

	// delete the key
	mKeyMap.erase(it);

	KeyList::iterator it2(std::find(mKeyList.begin(), mKeyList.end(), &*it));
	VDASSERT(it2 != mKeyList.end());

	mKeyList.erase(it2);
	return true;
}

bool VDRegistryProviderMemory::Key::RemoveValue(const char *name) {
	if (!name)
		name = "";

	ValueMap::iterator it(mValueMap.find_as(name));

	if (it == mValueMap.end())
		return false;

	ValueList::iterator it2(std::find(mValueList.begin(), mValueList.end(), &*it));
	VDASSERT(it2 != mValueList.end());

	mValueList.erase(it2);
	mValueMap.erase(it);
	return true;
}

VDRegistryProviderMemory::Value *VDRegistryProviderMemory::Key::OpenValue(const char *name, bool create) {
	if (!name)
		name = "";

	ValueMap::iterator it(mValueMap.find_as(name));

	if (it != mValueMap.end())
		return &it->second;

	if (!create)
		return NULL;

	ValueMap::insert_return_type r(mValueMap.insert(VDStringA(name)));
	mValueList.push_back(&*r.first);

	return &r.first->second;
}

VDRegistryProviderMemory::Key *VDRegistryProviderMemory::Key::OpenKey(const VDStringSpanA& name, bool create) {
	KeyMap::iterator it(mKeyMap.find_as(name));

	if (it != mKeyMap.end())
		return &it->second;

	if (!create)
		return NULL;

	KeyMap::insert_return_type r(mKeyMap.insert(VDStringA(name)));
	mKeyList.push_back(&*r.first);

	Key *key = &r.first->second;

	key->mpParent = this;

	return key;
}

///////////////////////////////////////////////////////////////////////

VDRegistryProviderMemory::VDRegistryProviderMemory() {
	vdautoptr<Key> machineKey(new Key);
	vdautoptr<Key> userKey(new Key);

	mpMachineKey = machineKey.release();
	mpUserKey = userKey.release();
}

VDRegistryProviderMemory::~VDRegistryProviderMemory() {
	delete mpMachineKey;
	delete mpUserKey;
}

void *VDRegistryProviderMemory::GetMachineKey() {
	return mpMachineKey;
}

void *VDRegistryProviderMemory::GetUserKey() {
	return mpUserKey;
}

void *VDRegistryProviderMemory::CreateKey(void *key0, const char *path, bool write) {
	Key *key = (Key *)key0;

	vdsynchronized(mMutex) {
		// check for root specifier
		if (*path == '\\') {
			do {
				++path;
			} while(*path == '\\');

			while(key->mpParent)
				key = key->mpParent;
		}

		// parse out a component at a time
		for(;;) {
			const char *split = strchr(path, '\\');
			if (!split)
				split = path + strlen(path);

			if (path == split)
				break;

			VDStringSpanA component(path, split);

			// lookup component
			key = key->OpenKey(component, write);
			if (!key)
				return NULL;

			// skip path specifier
			if (!*split)
				break;

			path = split;
			do {
				++path;
			} while(*path == L'\\');
		}

		key->AddRef();
	}

	return key;
}

void VDRegistryProviderMemory::CloseKey(void *key0) {
	Key *key = (Key *)key0;

	vdsynchronized(mMutex) {
		key->Release();	
	}
}

bool VDRegistryProviderMemory::SetBool(void *key0, const char *name, bool v) {
	vdsynchronized(mMutex) {
		Key *key = (Key *)key0;
		Value *value = key->OpenValue(name, true);

		if (!value)
			return false;

		value->SetInt(v);
	}
	return true;
}

bool VDRegistryProviderMemory::SetInt(void *key0, const char *name, int v) {
	vdsynchronized(mMutex) {
		Key *key = (Key *)key0;
		Value *value = key->OpenValue(name, true);

		if (!value)
			return false;

		value->SetInt(v);
	}
	return true;
}

bool VDRegistryProviderMemory::SetString(void *key0, const char *name, const char *str) {
	vdsynchronized(mMutex) {
		Key *key = (Key *)key0;
		Value *value = key->OpenValue(name, true);

		if (!value)
			return false;

		value->SetString(VDTextAToW(str).c_str());
	}
	return true;
}

bool VDRegistryProviderMemory::SetString(void *key0, const char *name, const wchar_t *str) {
	vdsynchronized(mMutex) {
		Key *key = (Key *)key0;
		Value *value = key->OpenValue(name, true);

		if (!value)
			return false;

		value->SetString(str);
	}
	return true;
}

bool VDRegistryProviderMemory::SetBinary(void *key0, const char *name, const char *data, int len) {
	vdsynchronized(mMutex) {
		Key *key = (Key *)key0;
		Value *value = key->OpenValue(name, true);

		if (!value)
			return false;

		value->SetBinary(data, len);
	}
	return true;
}

IVDRegistryProvider::Type VDRegistryProviderMemory::GetType(void *key0, const char *name) {
	Type type = kTypeUnknown;

	vdsynchronized(mMutex) {
		Key *key = (Key *)key0;
		Value *value = key->OpenValue(name, true);

		if (value) {
			switch(value->GetType()) {
				case Value::kTypeInt:
					type = kTypeInt;
					break;

				case Value::kTypeString:
					type = kTypeString;
					break;

				case Value::kTypeBinary:
					type = kTypeBinary;
					break;
			}
		}
	}

	return type;
}

bool VDRegistryProviderMemory::GetBool(void *key0, const char *name, bool& val) {
	vdsynchronized(mMutex) {
		Key *key = (Key *)key0;
		Value *value = key->OpenValue(name, false);

		sint32 v32;
		if (!value || !value->GetInt(v32))
			return false;

		val = v32 != 0;
	}
	return true;
}

bool VDRegistryProviderMemory::GetInt(void *key0, const char *name, int& val) {
	vdsynchronized(mMutex) {
		Key *key = (Key *)key0;
		Value *value = key->OpenValue(name, false);

		sint32 v32;
		if (!value || !value->GetInt(v32))
			return false;

		val = v32;
	}
	return true;
}

bool VDRegistryProviderMemory::GetString(void *key0, const char *name, VDStringA& s) {
	vdsynchronized(mMutex) {
		Key *key = (Key *)key0;
		Value *value = key->OpenValue(name, false);

		const wchar_t *raws;
		if (!value || !value->GetString(raws))
			return false;

		s = VDTextWToA(raws);
	}
	return true;
}

bool VDRegistryProviderMemory::GetString(void *key0, const char *name, VDStringW& s) {
	vdsynchronized(mMutex) {
		Key *key = (Key *)key0;
		Value *value = key->OpenValue(name, false);

		const wchar_t *raws;
		if (!value || !value->GetString(raws))
			return false;

		s = raws;
	}
	return true;
}

int VDRegistryProviderMemory::GetBinaryLength(void *key0, const char *name) {
	size_t len;

	vdsynchronized(mMutex) {
		Key *key = (Key *)key0;
		Value *value = key->OpenValue(name, false);

		const void *p;
		if (!value || !value->GetBinary(p, len))
			return -1;
	}
	return len;
}

bool VDRegistryProviderMemory::GetBinary(void *key0, const char *name, char *buf, int maxlen) {
	vdsynchronized(mMutex) {
		Key *key = (Key *)key0;
		Value *value = key->OpenValue(name, false);

		const void *p;
		size_t len;
		if (!value || !value->GetBinary(p, len) || (int)len > maxlen)
			return false;

		memcpy(buf, p, len);
	}

	return true;
}

bool VDRegistryProviderMemory::RemoveValue(void *key0, const char *name) {
	bool success;

	vdsynchronized(mMutex) {
		Key *key = (Key *)key0;

		success = key->RemoveValue(name);
	}

	return true;
}

bool VDRegistryProviderMemory::RemoveKey(void *key0, const char *name) {
	bool success;

	vdsynchronized(mMutex) {
		Key *key = (Key *)key0;

		// if the key is a root key, silently ignore the request
		if (!key->mpParent)
			return true;

		success = key->RemoveKey(name);
	}

	return true;
}

struct VDRegistryProviderMemory::Enumerator {
	Enumerator(Key *key) : mKey(key), mIndex(0) {}

	Key *mKey;
	size_t mIndex;
	VDStringA mName;
};

void *VDRegistryProviderMemory::EnumKeysBegin(void *key) {
	return new Enumerator((Key *)key);
}

const char *VDRegistryProviderMemory::EnumKeysNext(void *enumerator) {
	Enumerator *en = (Enumerator *)enumerator;

	vdsynchronized(mMutex) {
		const char *s = en->mKey->GetKeyName(en->mIndex);
		if (!s)
			return NULL;

		++en->mIndex;
		en->mName = s;
	}

	return en->mName.c_str();
}

void VDRegistryProviderMemory::EnumKeysClose(void *enumerator) {
	Enumerator *en = (Enumerator *)enumerator;

	delete en;
}

void *VDRegistryProviderMemory::EnumValuesBegin(void *key) {
	return new Enumerator((Key *)key);
}

const char *VDRegistryProviderMemory::EnumValuesNext(void *enumerator) {
	Enumerator *en = (Enumerator *)enumerator;

	if (!en->mKey)
		return NULL;

	vdsynchronized(mMutex) {
		const char *s = en->mKey->GetValueName(en->mIndex);
		if (!s)
			return NULL;

		++en->mIndex;
		en->mName = s;
	}

	return en->mName.c_str();
}

void VDRegistryProviderMemory::EnumValuesClose(void *enumerator) {
	Enumerator *en = (Enumerator *)enumerator;

	delete en;
}
