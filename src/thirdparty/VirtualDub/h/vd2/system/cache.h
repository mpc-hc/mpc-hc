//	VirtualDub - Video processing and capture application
//	System library component
//	Copyright (C) 1998-2005 Avery Lee, All Rights Reserved.
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

#ifndef f_VD2_SYSTEM_CACHE_H
#define f_VD2_SYSTEM_CACHE_H

#include <vd2/system/thread.h>
#include <vd2/system/vdstl.h>

///////////////////////////////////////////////////////////////////////////

struct vdfixedhashmap_node {
	vdfixedhashmap_node *mpHashPrev;
	vdfixedhashmap_node *mpHashNext;
};

template<class K>
struct vdhash {
	size_t operator()(const K key) const {
		return (size_t)key;
	}
};

template<class K, class V, class Hash = vdhash<K>, int N = 256>
class vdfixedhashmap_iterator {
public:
	typedef	vdfixedhashmap_node	node;

	bool operator==(vdfixedhashmap_iterator& x) const { return mpNode == x.mpNode; }
	bool operator!=(vdfixedhashmap_iterator& x) const { return mpNode != x.mpNode; }

	V& operator*() const { return *static_cast<V *>((node *)mpNode); }
	V *operator->() const { return static_cast<V *>((node *)mpNode); }

	vdfixedhashmap_iterator& operator++() {
		do {
			mpNode = ((node *)mpNode)->mpHashNext;
			if (mpNode != mpTableNode)
				break;

			++mpTableNode;
			mpNode = mpTableNode->mpHashNext;
		} while(mpNode);

		return *this;
	}

	vdfixedhashmap_iterator operator++(int) {
		vdfixedhashmap_iterator it(*this);
		++*this;
		return it;
	}

public:
	vdfixedhashmap_node *mpNode;
	vdfixedhashmap_node *mpTableNode;
};

template<class K, class V, class Hash = vdhash<K>, int N = 256>
class vdfixedhashmap {
public:
	typedef	K					key_type;
	typedef	V					value_type;
	typedef	Hash				hash_type;
	typedef vdfixedhashmap_node		node;
	typedef	vdfixedhashmap_iterator<K, V>	iterator;

	vdfixedhashmap() {
		for(int i=0; i<N; ++i)
			m.mpTable[i].mpHashPrev = m.mpTable[i].mpHashNext = &m.mpTable[i];
	}

	iterator begin() {
		int i;
		for(i=0; i<N && !m.mpTable[i]; ++i)
			;
		iterator it = { m.mpTable[i].mpFirst, &m.mpTable[i] };
		return it;
	}

	iterator end() {
		iterator it = { NULL, NULL };
		return it;
	}

	V *operator[](const K& key) {
		const size_t htidx = m(key) % N;

		node *r = &m.mpTable[htidx];
		for(node *p = r->mpHashNext; p != r; p = p->mpHashNext) {
			if (static_cast<V *>(p)->mHashKey == key)
				return static_cast<V *>(p);
		}

		return NULL;
	}

	iterator find(const K& key) {
		const size_t htidx = m(key) % N;

		node *r = &m.mpTable[htidx];
		for(node *p = r->mpHashNext; p != r; p = p->mpHashNext) {
			if (static_cast<V *>(p)->mHashKey == key) {
				iterator it = { p, &m.mpTable[htidx] };
				return it;
			}
		}

		return end();
	}

	iterator insert(V *p) {
		const size_t htidx = m(p->mHashKey) % N;

		node *r = &m.mpTable[htidx];
		node *n = r->mpHashNext;
		r->mpHashNext = p;
		p->mpHashPrev = &m.mpTable[htidx];
		p->mpHashNext = n;
		n->mpHashPrev = p;

		iterator it = { p, &m.mpTable[htidx] };
		return it;
	}

	void erase(V *x) {
		node *p = x->mpHashPrev;
		node *n = x->mpHashNext;

		p->mpHashNext = n;
		n->mpHashPrev = p;
	}

	void erase(iterator it) {
		erase(it.mpNode);
	}

protected:
	struct Data : public Hash {
		vdfixedhashmap_node	mpTable[N];
	} m;
};

///////////////////////////////////////////////////////////////////////////

class VDCachedObject;

class IVDCacheAllocator {
public:
	virtual VDCachedObject *OnCacheAllocate() = 0;
};

///////////////////////////////////////////////////////////////////////////

enum VDCacheState {
	kVDCacheStateFree,
	kVDCacheStatePending,
	kVDCacheStateReady,
	kVDCacheStateActive,
	kVDCacheStateComplete,
	kVDCacheStateIdle,
	kVDCacheStateAborting,
	kVDCacheStateCount
};

struct VDCachedObjectNodes : public vdlist_node, public vdfixedhashmap_node {
	sint64	mHashKey;
};

class VDCache {
public:
	VDCache(IVDCacheAllocator *pAllocator);
	~VDCache();

	void Shutdown();

	int GetStateCount(int state);

	void DumpListStatus(int state);

	VDCachedObject *Create(sint64 key, bool& is_new);

	VDCachedObject *Allocate(sint64 key);
	void Schedule(VDCachedObject *);			// Moves a Pending or Active object to Ready.
	VDCachedObject *GetNextReady();				// Selects a Ready object and moves it to Active.
	void MarkCompleted(VDCachedObject *);		// Marks an object as completed.

public:
	void NotifyFree(VDCachedObject *pObject);

protected:
	void Evict(uint32 level);

protected:
	VDCriticalSection	mLock;

	IVDCacheAllocator	*mpAllocator;
	uint32		mObjectCount;
	uint32		mObjectLimit;

	typedef vdlist<VDCachedObjectNodes> ObjectList;
	ObjectList	mLists[kVDCacheStateCount];

	vdfixedhashmap<sint64, VDCachedObjectNodes>	mHash;
};

///////////////////////////////////////////////////////////////////////////

class VDCachedObject : private VDCachedObjectNodes {
	friend class VDCache;
public:
	VDCachedObject();
	virtual ~VDCachedObject() {}

	int AddRef();
	int Release();

	void WeakAddRef();
	void WeakRelease();

protected:
	virtual void OnCacheEvict() {}
	virtual void OnCacheAbortPending() {}
	virtual void DumpStatus() {}

protected:
	int GetRefCount() const { return mRefCount; }
	void SetCache(VDCache *pCache);

	VDCacheState GetState() const { return mState; }
	void SetState(VDCacheState state) { mState = state; }

	sint64 GetCacheKey() const { return mHashKey; }

	virtual bool IsValid() const { return true; }

protected:
	VDCache			*mpCache;
	VDAtomicInt		mRefCount;
	VDCacheState	mState;
};

///////////////////////////////////////////////////////////////////////////

class VDPooledObject;

class IVDPoolAllocator {
public:
	virtual VDPooledObject *OnPoolAllocate() = 0;
};

///////////////////////////////////////////////////////////////////////////

enum VDPoolState {
	kVDPoolStateFree,
	kVDPoolStateActive,
	kVDPoolStateCount
};

struct VDPooledObjectNodes : public vdlist_node {};

class VDPool {
public:
	VDPool(IVDPoolAllocator *pAllocator);
	~VDPool();

	void Shutdown();

	VDPooledObject *Allocate();

public:
	void NotifyFree(VDPooledObject *pObject);

protected:
	VDCriticalSection	mLock;

	IVDPoolAllocator	*mpAllocator;
	uint32		mObjectCount;
	uint32		mObjectLimit;

	typedef vdlist<VDPooledObjectNodes> ObjectList;
	ObjectList	mLists[kVDPoolStateCount];
};

class VDPooledObject : private VDPooledObjectNodes {
	friend class VDPool;
public:
	VDPooledObject();
	virtual ~VDPooledObject() {}

	int AddRef();
	int Release();

protected:
	int GetRefCount() const { return mRefCount; }
	void SetPool(VDPool *pPool);

protected:
	VDPool			*mpPool;
	VDAtomicInt		mRefCount;
};

#endif
