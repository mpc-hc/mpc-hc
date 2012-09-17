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

#ifndef f_VD2_SYSTEM_VDSTL_HASHMAP_H
#define f_VD2_SYSTEM_VDSTL_HASHMAP_H

#include <vd2/system/vdstl_hash.h>
#include <vd2/system/vdstl_hashtable.h>
#include <functional>

template<class K, class V, class Hash = vdhash<K>, class Pred = std::equal_to<K>, class A = std::allocator<vdhashtable_node<std::pair<K, V> > > >
class vdhashmap : public vdhashtable<std::pair<K, V> > {
protected:
	using vdhashtable<std::pair<K, V> >::mpBucketStart;
	using vdhashtable<std::pair<K, V> >::mpBucketEnd;
	using vdhashtable<std::pair<K, V> >::mBucketCount;
	using vdhashtable<std::pair<K, V> >::mElementCount;
	using vdhashtable<std::pair<K, V> >::sEmptyBucket;

public:
	typedef typename vdhashtable<std::pair<K, V> >::node_type node_type;
	typedef typename vdhashtable<std::pair<K, V> >::value_type value_type;
	typedef typename vdhashtable<std::pair<K, V> >::size_type size_type;
	typedef typename vdhashtable<std::pair<K, V> >::difference_type difference_type;
	typedef typename vdhashtable<std::pair<K, V> >::pointer pointer;
	typedef typename vdhashtable<std::pair<K, V> >::const_pointer const_pointer;
	typedef typename vdhashtable<std::pair<K, V> >::reference reference;
	typedef typename vdhashtable<std::pair<K, V> >::const_reference const_reference;
	typedef typename vdhashtable<std::pair<K, V> >::iterator iterator;
	typedef typename vdhashtable<std::pair<K, V> >::const_iterator const_iterator;
	typedef typename vdhashtable<std::pair<K, V> >::local_iterator local_iterator;
	typedef typename vdhashtable<std::pair<K, V> >::const_local_iterator const_local_iterator;
	typedef K key_type;
	typedef V mapped_type;
	typedef Hash hasher;
	typedef Pred key_equal;
	typedef A allocator_type;
	typedef std::pair<iterator, bool> insert_return_type;

	vdhashmap();
	vdhashmap(const vdhashmap&);
	~vdhashmap();

	vdhashmap& operator=(const vdhashmap&);

	mapped_type&		operator[](const K& key);

	allocator_type get_allocator() const;

	// iterators
	using vdhashtable<value_type>::begin;
	using vdhashtable<value_type>::end;
//	iterator			begin();			Inherited.
//	const_iterator		begin() const;		Inherited.
//	iterator			end();				Inherited.
//	const_iterator		end() const;		Inherited.

	// modifiers
	insert_return_type	insert(const key_type& key);
	insert_return_type	insert(const std::pair<K, V>& obj);
//	iterator			insert(iterator hint, const value_type& obj);			// TODO
//	const_iterator		insert(const_iterator hint, const value_type& obj);	// TODO

	template<class U>
	insert_return_type	insert_as(const U& k);	// extension

	iterator			erase(iterator position);
	const_iterator		erase(const_iterator position);
	size_type			erase(const key_type& k);
//	iterator			erase(iterator first, iterator last);				// TODO
//	const_iterator		erase(const_iterator first, const_iterator last);	// TODO
	void				clear();

	// observers
	hasher				hash_function() const;
	key_equal			key_eq() const;

	// lookup
	iterator			find(const key_type& k);
	const_iterator		find(const key_type& k) const;
	size_type			count(const key_type& k) const;
	std::pair<iterator, iterator>	equal_range(const key_type& k);
	std::pair<const_iterator, const_iterator>	equal_range(const key_type& k) const;

	// lookup (extensions)
	template<class U>
	iterator			find_as(const U& k);

	template<class U>
	const_iterator		find_as(const U& k) const;

	// bucket interface
//	size_type			bucket_count() const;				Inherited.
//	size_type			max_bucket_count() const;			Inherited.
//	size_type			bucket_size(size_type n) const;		Inherited.
	size_type			bucket(const key_type& k) const;
	local_iterator		begin(size_type n);
	const_local_iterator	begin(size_type n) const;
	local_iterator		end(size_type n);
	const_local_iterator	end(size_type n) const;

	// hash policy
//	float				load_factor() const;				// TODO
//	float				max_load_factor() const;			// TODO
//	void				max_load_factor(float z);			// TODO
	void				rehash(size_type n);

protected:
	void				rehash_to_size(size_type n);
	void				reset();

	A mAllocator;
	typename A::template rebind<vdhashtable_base_node *>::other mBucketAllocator;
	Hash mHasher;
	Pred mPred;
};

template<class K, class V, class Hash, class Pred, class A>
vdhashmap<K, V, Hash, Pred, A>::vdhashmap() {
}

template<class K, class V, class Hash, class Pred, class A>
vdhashmap<K, V, Hash, Pred, A>::vdhashmap(const vdhashmap& src)
	: mHasher(src.mHasher)
	, mPred(src.mPred)
{
	rehash_to_size(src.mElementCount);

	try {
		for(vdhashtable_base_node **bucket = mpBucketStart; bucket != mpBucketEnd; ++bucket) {
			vdhashtable_base_node *p = *bucket;

			while(p) {
				vdhashtable_base_node *next = p->mpHashNext;
				node_type *node = static_cast<node_type *>(p);

				insert(node->mData);

				p = next;
			}

			*bucket = NULL;
		}
	} catch(...) {
		reset();
		throw;
	}
}

template<class K, class V, class Hash, class Pred, class A>
vdhashmap<K, V, Hash, Pred, A>::~vdhashmap() {
	reset();
}

template<class K, class V, class Hash, class Pred, class A>
vdhashmap<K, V, Hash, Pred, A>& vdhashmap<K, V, Hash, Pred, A>::operator=(const vdhashmap& src) {
	if (&src != this) {
		clear();

		mHasher = src.mHasher;
		mPred = src.mPred;

		for(vdhashtable_base_node **bucket = src.mpBucketStart; bucket != src.mpBucketEnd; ++bucket) {
			vdhashtable_base_node *p = *bucket;

			while(p) {
				vdhashtable_base_node *next = p->mpHashNext;
				node_type *node = static_cast<node_type *>(p);

				insert(node->mData);

				p = next;
			}

			*bucket = NULL;
		}
	}
	
	return *this;
}

template<class K, class V, class Hash, class Pred, class A>
typename vdhashmap<K, V, Hash, Pred, A>::mapped_type& vdhashmap<K, V, Hash, Pred, A>::operator[](const K& key) {
	return insert(key).first->second;
}

template<class K, class V, class Hash, class Pred, class A>
typename vdhashmap<K, V, Hash, Pred, A>::allocator_type vdhashmap<K, V, Hash, Pred, A>::get_allocator() const {
	return A();
}

// modifiers
template<class K, class V, class Hash, class Pred, class A>
typename vdhashmap<K, V, Hash, Pred, A>::insert_return_type vdhashmap<K, V, Hash, Pred, A>::insert(const key_type& key) {
	if (mElementCount >= mBucketCount)
		rehash_to_size(mElementCount + 1);

	size_type bucket = mHasher(key) % mBucketCount;

	for(node_type *p = static_cast<node_type *>(mpBucketStart[bucket]); p; p = static_cast<node_type *>(p->mpHashNext)) {
		if (mPred(p->mData.first, key))
			return std::pair<iterator, bool>(iterator(p, &mpBucketStart[bucket], mpBucketEnd), false);
	}

	node_type *node = mAllocator.allocate(1);
	try {
		new(node) node_type(static_cast<node_type *>(mpBucketStart[bucket]), value_type(key, V()));
	} catch(...) {
		mAllocator.deallocate(node, 1);
		throw;
	}

	mpBucketStart[bucket] = node;
	++mElementCount;

	return std::pair<iterator, bool>(iterator(node, &mpBucketStart[bucket], mpBucketEnd), true);
}

template<class K, class V, class Hash, class Pred, class A>
typename vdhashmap<K, V, Hash, Pred, A>::insert_return_type vdhashmap<K, V, Hash, Pred, A>::insert(const std::pair<K, V>& obj) {
	if (mElementCount >= mBucketCount)
		rehash_to_size(mElementCount + 1);

	size_type bucket = mHasher(obj.first) % mBucketCount;

	for(node_type *p = static_cast<node_type *>(mpBucketStart[bucket]); p; p = static_cast<node_type *>(p->mpHashNext)) {
		if (mPred(p->mData.first, obj.first))
			return std::pair<iterator, bool>(iterator(p, &mpBucketStart[bucket], mpBucketEnd), false);
	}

	node_type *node = mAllocator.allocate(1);
	try {
		new(node) node_type(static_cast<node_type *>(mpBucketStart[bucket]), obj);
	} catch(...) {
		mAllocator.deallocate(node, 1);
		throw;
	}

	mpBucketStart[bucket] = node;
	++mElementCount;

	return std::pair<iterator, bool>(iterator(node, &mpBucketStart[bucket], mpBucketEnd), true);
}

template<class K, class V, class Hash, class Pred, class A>
template<class U>
typename vdhashmap<K, V, Hash, Pred, A>::insert_return_type vdhashmap<K, V, Hash, Pred, A>::insert_as(const U& key) {
	if (mElementCount >= mBucketCount)
		rehash_to_size(mElementCount + 1);

	size_type bucket = mHasher(key) % mBucketCount;

	for(node_type *p = static_cast<node_type *>(mpBucketStart[bucket]); p; p = static_cast<node_type *>(p->mpHashNext)) {
		if (mPred(p->mData.first, key))
			return std::pair<iterator, bool>(iterator(p, &mpBucketStart[bucket], mpBucketEnd), false);
	}

	node_type *node = mAllocator.allocate(1);
	try {
		new(node) node_type(static_cast<node_type *>(mpBucketStart[bucket]));

		try {
			node->mData.first = key;
		} catch(...) {
			node->~node_type();
		}

	} catch(...) {
		mAllocator.deallocate(node, 1);
		throw;
	}

	mpBucketStart[bucket] = node;
	++mElementCount;

	return std::pair<iterator, bool>(iterator(node, &mpBucketStart[bucket], mpBucketEnd), true);
}

template<class K, class V, class Hash, class Pred, class A>
typename vdhashmap<K, V, Hash, Pred, A>::iterator vdhashmap<K, V, Hash, Pred, A>::erase(iterator position) {
	size_type bucket = mHasher(position->first) % mBucketCount;
	vdhashtable_base_node *prev = NULL;
	vdhashtable_base_node *p = mpBucketStart[bucket];

	while(&static_cast<node_type *>(p)->mData != &*position) {
		prev = p;
		p = p->mpHashNext;
	}

	vdhashtable_base_node *next = p->mpHashNext;
	if (prev)
		prev->mpHashNext = next;
	else
		mpBucketStart[bucket] = next;

	node_type *node = static_cast<node_type *>(p);
	node->~node_type();
	mAllocator.deallocate(node, 1);
	--mElementCount;

	return iterator(next, &mpBucketStart[bucket], mpBucketEnd);
}

template<class K, class V, class Hash, class Pred, class A>
typename vdhashmap<K, V, Hash, Pred, A>::const_iterator vdhashmap<K, V, Hash, Pred, A>::erase(const_iterator position) {
	size_type bucket = mHasher(position->first) % mBucketCount;
	vdhashtable_base_node *prev = NULL;
	vdhashtable_base_node *p = mpBucketStart[bucket];

	while(&static_cast<const node_type *>(p)->mData != &*position) {
		prev = p;
		p = p->mpHashNext;
	}

	vdhashtable_base_node *next = p->mpHashNext;
	if (prev)
		prev->mpHashNext = next;
	else
		mpBucketStart[bucket] = next;

	node_type *node = static_cast<node_type *>(p);
	node->~node_type();
	mAllocator.deallocate(node, 1);
	--mElementCount;

	return const_iterator(next, &mpBucketStart[bucket], mpBucketEnd);
}

template<class K, class V, class Hash, class Pred, class A>
typename vdhashmap<K, V, Hash, Pred, A>::size_type vdhashmap<K, V, Hash, Pred, A>::erase(const key_type& k) {
	if (!mBucketCount)
		return 0;

	size_type bucket = mHasher(k) % mBucketCount;
	vdhashtable_base_node *prev = NULL;
	vdhashtable_base_node *p = mpBucketStart[bucket];

	while(p) {
		node_type *node = static_cast<node_type *>(p);

		if (mPred(node->mData.first, k)) {
			vdhashtable_base_node *next = p->mpHashNext;

			if (prev)
				prev->mpHashNext = next;
			else
				mpBucketStart[bucket] = next;

			node->~node_type();
			mAllocator.deallocate(node, 1);
			--mElementCount;
			return 1;
		}

		prev = p;
		p = p->mpHashNext;
	}

	return 0;
}

template<class K, class V, class Hash, class Pred, class A>
void vdhashmap<K, V, Hash, Pred, A>::clear() {
	for(vdhashtable_base_node **bucket = mpBucketStart; bucket != mpBucketEnd; ++bucket) {
		vdhashtable_base_node *p = *bucket;

		while(p) {
			vdhashtable_base_node *next = p->mpHashNext;
			node_type *node = static_cast<node_type *>(p);

			node->~node_type();

			mAllocator.deallocate(node, 1);

			p = next;
		}

		*bucket = NULL;
	}

	mElementCount = 0;
}

// observers
template<class K, class V, class Hash, class Pred, class A>
typename vdhashmap<K, V, Hash, Pred, A>::hasher vdhashmap<K, V, Hash, Pred, A>::hash_function() const {
	return mHasher;
}

template<class K, class V, class Hash, class Pred, class A>
typename vdhashmap<K, V, Hash, Pred, A>::key_equal vdhashmap<K, V, Hash, Pred, A>::key_eq() const {
	return mPred;
}

// lookup
template<class K, class V, class Hash, class Pred, class A>
typename vdhashmap<K, V, Hash, Pred, A>::iterator vdhashmap<K, V, Hash, Pred, A>::find(const key_type& k) {
	if (!mBucketCount)
		return iterator();

	size_type bucket = mHasher(k) % mBucketCount;

	for(vdhashtable_base_node *p = mpBucketStart[bucket]; p; p = p->mpHashNext) {
		node_type *node = static_cast<node_type *>(p);

		if (mPred(node->mData.first, k))
			return iterator(node, &mpBucketStart[bucket], mpBucketEnd);
	}

	return iterator();
}

template<class K, class V, class Hash, class Pred, class A>
typename vdhashmap<K, V, Hash, Pred, A>::const_iterator vdhashmap<K, V, Hash, Pred, A>::find(const key_type& k) const {
	if (!mBucketCount)
		return iterator();

	size_type bucket = mHasher(k) % mBucketCount;

	for(const vdhashtable_base_node *p = mpBucketStart[bucket]; p; p = p->mpHashNext) {
		const node_type *node = static_cast<const node_type *>(p);

		if (mPred(node->mData.first, k))
			return const_iterator(const_cast<vdhashtable_base_node *>(static_cast<const vdhashtable_base_node *>(node)), &mpBucketStart[bucket], mpBucketEnd);
	}

	return iterator();
}

template<class K, class V, class Hash, class Pred, class A>
typename vdhashmap<K, V, Hash, Pred, A>::size_type vdhashmap<K, V, Hash, Pred, A>::count(const key_type& k) const {
	return find(k) != end() ? 1 : 0;
}

template<class K, class V, class Hash, class Pred, class A>
std::pair<typename vdhashmap<K, V, Hash, Pred, A>::iterator, typename vdhashmap<K, V, Hash, Pred, A>::iterator> vdhashmap<K, V, Hash, Pred, A>::equal_range(const key_type& k) {
	iterator it = find(k);
	iterator itEnd;

	if (it == itEnd)
		return std::make_pair(itEnd, itEnd);

	itEnd = it;
	++itEnd;

	return std::make_pair<it, itEnd>();
}

template<class K, class V, class Hash, class Pred, class A>
std::pair<typename vdhashmap<K, V, Hash, Pred, A>::const_iterator, typename vdhashmap<K, V, Hash, Pred, A>::const_iterator> vdhashmap<K, V, Hash, Pred, A>::equal_range(const key_type& k) const {
	const_iterator it = find(k);
	const_iterator itEnd;

	if (it == itEnd)
		return std::make_pair(itEnd, itEnd);

	itEnd = it;
	++itEnd;

	return std::make_pair(it, itEnd);
}

template<class K, class V, class Hash, class Pred, class A>
template<class U>
typename vdhashmap<K, V, Hash, Pred, A>::iterator vdhashmap<K, V, Hash, Pred, A>::find_as(const U& k) {
	if (!mBucketCount)
		return iterator();

	size_type bucket = mHasher(k) % mBucketCount;

	for(vdhashtable_base_node *p = mpBucketStart[bucket]; p; p = p->mpHashNext) {
		node_type *node = static_cast<node_type *>(p);

		if (mPred(node->mData.first, k))
			return iterator(node, &mpBucketStart[bucket], mpBucketEnd);
	}

	return iterator();
}

template<class K, class V, class Hash, class Pred, class A>
template<class U>
typename vdhashmap<K, V, Hash, Pred, A>::const_iterator vdhashmap<K, V, Hash, Pred, A>::find_as(const U& k) const {
	if (!mBucketCount)
		return iterator();

	size_type bucket = mHasher(k) % mBucketCount;

	for(const vdhashtable_base_node *p = mpBucketStart[bucket]; p; p = p->mpHashNext) {
		const node_type *node = static_cast<const node_type *>(p);

		if (mPred(node->mData.first, k))
			return const_iterator(const_cast<vdhashtable_base_node *>(static_cast<const vdhashtable_base_node *>(node)), &mpBucketStart[bucket], mpBucketEnd);
	}

	return iterator();
}

// bucket interface
template<class K, class V, class Hash, class Pred, class A>
typename vdhashmap<K, V, Hash, Pred, A>::size_type vdhashmap<K, V, Hash, Pred, A>::bucket(const key_type& k) const {
	size_type bucket = 0;

	if (mBucketCount)
		bucket = mHasher(k) % mBucketCount;

	return bucket;
}

template<class K, class V, class Hash, class Pred, class A>
typename vdhashmap<K, V, Hash, Pred, A>::local_iterator vdhashmap<K, V, Hash, Pred, A>::begin(size_type n) {
	return local_iterator(mpBucketStart[n]);
}

template<class K, class V, class Hash, class Pred, class A>
typename vdhashmap<K, V, Hash, Pred, A>::const_local_iterator vdhashmap<K, V, Hash, Pred, A>::begin(size_type n) const {
	return const_local_iterator(mpBucketStart[n]);
}

template<class K, class V, class Hash, class Pred, class A>
typename vdhashmap<K, V, Hash, Pred, A>::local_iterator vdhashmap<K, V, Hash, Pred, A>::end(size_type n) {
	return local_iterator(NULL);
}

template<class K, class V, class Hash, class Pred, class A>
typename vdhashmap<K, V, Hash, Pred, A>::const_local_iterator vdhashmap<K, V, Hash, Pred, A>::end(size_type n) const {
	return const_local_iterator(NULL);
}

// hash policy
template<class K, class V, class Hash, class Pred, class A>
void vdhashmap<K, V, Hash, Pred, A>::rehash(size_type n) {
	if (!n)
		n = 1;

	if (mBucketCount == n)
		return;

	vdhashtable_base_node **newBuckets = mBucketAllocator.allocate(n + 1);

	for(size_type i=0; i<=n; ++i)
		newBuckets[i] = NULL;

	const size_type m = mBucketCount;
	for(size_type i=0; i<m; ++i) {
		for(vdhashtable_base_node *p = mpBucketStart[i]; p;) {
			vdhashtable_base_node *next = p->mpHashNext;
			const value_type& vt = static_cast<vdhashtable_node<value_type> *>(p)->mData;
			size_t bucket = mHasher(vt.first) % n;

			vdhashtable_base_node *head = newBuckets[bucket];

			p->mpHashNext = head;
			newBuckets[bucket] = p;

			p = next;
		}
	}

	if (mpBucketStart != &sEmptyBucket)
		mBucketAllocator.deallocate(mpBucketStart, (mpBucketEnd - mpBucketStart) + 1);

	mpBucketStart = newBuckets;
	mpBucketEnd = newBuckets + n;
	mBucketCount = n;
}

template<class K, class V, class Hash, class Pred, class A>
void vdhashmap<K, V, Hash, Pred, A>::rehash_to_size(size_type n) {
	size_type buckets = compute_bucket_count(n);
	rehash(buckets);
}

template<class K, class V, class Hash, class Pred, class A>
void vdhashmap<K, V, Hash, Pred, A>::reset() {
	clear();

	if (mpBucketStart != &sEmptyBucket)
		mBucketAllocator.deallocate(mpBucketStart, (mpBucketEnd - mpBucketStart) + 1);
}

#endif	// f_VD2_SYSTEM_VDSTL_HASHMAP_H
