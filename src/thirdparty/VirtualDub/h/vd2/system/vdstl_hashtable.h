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

#ifndef f_VD2_SYSTEM_VDSTL_HASHTABLE_H
#define f_VD2_SYSTEM_VDSTL_HASHTABLE_H

///////////////////////////////////////////////////////////////////////////////
//	vdhashtable_base_node
//
struct vdhashtable_base_node {
	vdhashtable_base_node *mpHashNext;
};

///////////////////////////////////////////////////////////////////////////////
//	vdhashtable_base
//
class vdhashtable_base {
public:
	typedef	size_t		size_type;
	typedef	ptrdiff_t	difference_type;

	vdhashtable_base();

	// size and capacity
	bool		empty() const { return mElementCount == 0; }
	size_type	size() const { return mElementCount; }
	size_type	max_size() const { return (size_type)-1 >> 1; }

	// bucket interface
	size_type			bucket_count() const;
	size_type			max_bucket_count() const;
	size_type			bucket_size(size_type n) const;

protected:
	static size_type compute_bucket_count(size_type n);

	size_type	mBucketCount;
	size_type	mElementCount;
	vdhashtable_base_node **mpBucketStart;
	vdhashtable_base_node **mpBucketEnd;

	static vdhashtable_base_node *const sEmptyBucket;
};

///////////////////////////////////////////////////////////////////////////

template<class T>
struct vdhashtable_node : public vdhashtable_base_node {
	T mData;

	vdhashtable_node() {}
	vdhashtable_node(vdhashtable_node<T> *next) : mData() {
		mpHashNext = next;
	}

	vdhashtable_node(vdhashtable_node<T> *next, const T& val) : mData(val) {
		mpHashNext = next;
	}
};

///////////////////////////////////////////////////////////////////////////

template<class T>
class vdhashtable_local_iterator {
public:
	typedef std::forward_iterator_tag iterator_category;
	typedef T value_type;
	typedef T* pointer;
	typedef T& reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	vdhashtable_local_iterator(vdhashtable_base_node *node);

	vdhashtable_local_iterator& operator++();
	vdhashtable_local_iterator operator++(int);

	T& operator*() const;
	T* operator->() const;

private:
	template<class U> friend bool operator==(const vdhashtable_local_iterator<U>& x, const vdhashtable_local_iterator<U>& y);
	template<class U> friend bool operator==(const vdhashtable_local_iterator<const U>& x, const vdhashtable_local_iterator<U>& y);
	template<class U> friend bool operator==(const vdhashtable_local_iterator<U>& x, const vdhashtable_local_iterator<const U>& y);
	template<class U> friend bool operator!=(const vdhashtable_local_iterator<U>& x, const vdhashtable_local_iterator<U>& y);
	template<class U> friend bool operator!=(const vdhashtable_local_iterator<const U>& x, const vdhashtable_local_iterator<U>& y);
	template<class U> friend bool operator!=(const vdhashtable_local_iterator<U>& x, const vdhashtable_local_iterator<const U>& y);

	vdhashtable_base_node *mpNode;
};

template<class T>
vdhashtable_local_iterator<T>::vdhashtable_local_iterator(vdhashtable_base_node *node)
	: mpNode(node)
{
}

template<class T>
vdhashtable_local_iterator<T>& vdhashtable_local_iterator<T>::operator++() {
	mpNode = mpNode->mpHashNext;
	return *this;
}

template<class T>
vdhashtable_local_iterator<T> vdhashtable_local_iterator<T>::operator++(int) {
	vdhashtable_local_iterator prev(*this);
	mpNode = mpNode->mpHashNext;
	return prev;
}

template<class T>
T& vdhashtable_local_iterator<T>::operator*() const {
	return static_cast<vdhashtable_node<T> *>(mpNode)->mData;
}

template<class T>
T* vdhashtable_local_iterator<T>::operator->() const {
	return &static_cast<vdhashtable_node<T> *>(mpNode)->mData;
}

template<class T>
bool operator==(const vdhashtable_local_iterator<T>& x, const vdhashtable_local_iterator<T>& y) {
	return x.mpNode == y.mpNode;
}

template<class T>
bool operator==(const vdhashtable_local_iterator<const T>& x, const vdhashtable_local_iterator<T>& y) {
	return x.mpNode == y.mpNode;
}

template<class T>
bool operator==(const vdhashtable_local_iterator<T>& x, const vdhashtable_local_iterator<const T>& y) {
	return x.mpNode == y.mpNode;
}

template<class T>
bool operator!=(const vdhashtable_local_iterator<T>& x, const vdhashtable_local_iterator<T>& y) {
	return x.mpNode != y.mpNode;
}

template<class T>
bool operator!=(const vdhashtable_local_iterator<const T>& x, const vdhashtable_local_iterator<T>& y) {
	return x.mpNode != y.mpNode;
}

template<class T>
bool operator!=(const vdhashtable_local_iterator<T>& x, const vdhashtable_local_iterator<const T>& y) {
	return x.mpNode != y.mpNode;
}

///////////////////////////////////////////////////////////////////////////

template<class T> struct vdhashtable_iterator_nonconst {
	typedef T result;
};

template<class T> struct vdhashtable_iterator_nonconst<const T> {
	typedef T result;
};

template<class T>
class vdhashtable_iterator {
	typedef typename vdhashtable_iterator_nonconst<T>::result T_NonConst;
public:
	typedef std::forward_iterator_tag iterator_category;
	typedef T value_type;
	typedef T* pointer;
	typedef T& reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	vdhashtable_iterator();
	vdhashtable_iterator(vdhashtable_base_node *node, vdhashtable_base_node **bucket, vdhashtable_base_node **bucketEnd);
	vdhashtable_iterator(const vdhashtable_iterator<T_NonConst>& src);

	vdhashtable_iterator& operator++();
	vdhashtable_iterator operator++(int);

	T& operator*() const;
	T* operator->() const;

private:
	friend class vdhashtable_iterator<const T>;
	template<class U> friend bool operator==(const vdhashtable_iterator<U>& x, const vdhashtable_iterator<U>& y);
	template<class U> friend bool operator==(const vdhashtable_iterator<const U>& x, const vdhashtable_iterator<U>& y);
	template<class U> friend bool operator==(const vdhashtable_iterator<U>& x, const vdhashtable_iterator<const U>& y);
	template<class U> friend bool operator!=(const vdhashtable_iterator<U>& x, const vdhashtable_iterator<U>& y);
	template<class U> friend bool operator!=(const vdhashtable_iterator<const U>& x, const vdhashtable_iterator<U>& y);
	template<class U> friend bool operator!=(const vdhashtable_iterator<U>& x, const vdhashtable_iterator<const U>& y);

	vdhashtable_base_node *mpNode;
	vdhashtable_base_node **mpBucket;
	vdhashtable_base_node **mpBucketEnd;
};

template<class T>
vdhashtable_iterator<T>::vdhashtable_iterator()
	: mpNode(NULL)
	, mpBucket(NULL)
	, mpBucketEnd(NULL)
{
}

template<class T>
vdhashtable_iterator<T>::vdhashtable_iterator(vdhashtable_base_node *node, vdhashtable_base_node **bucket, vdhashtable_base_node **bucketEnd)
	: mpNode(node)
	, mpBucket(bucket)
	, mpBucketEnd(bucketEnd)
{
}

template<class T>
vdhashtable_iterator<T>::vdhashtable_iterator(const vdhashtable_iterator<T_NonConst>& src)
	: mpNode(src.mpNode)
	, mpBucket(src.mpBucket)
	, mpBucketEnd(src.mpBucketEnd)
{
}

template<class T>
vdhashtable_iterator<T>& vdhashtable_iterator<T>::operator++() {
	mpNode = mpNode->mpHashNext;

	while(!mpNode && ++mpBucket != mpBucketEnd)
		mpNode = static_cast<vdhashtable_node<T> *>(*mpBucket);

	return *this;
}

template<class T>
vdhashtable_iterator<T> vdhashtable_iterator<T>::operator++(int) {
	vdhashtable_iterator prev(*this);
	mpNode = mpNode->mpHashNext;
	return prev;
}

template<class T>
T& vdhashtable_iterator<T>::operator*() const {
	return static_cast<vdhashtable_node<T> *>(mpNode)->mData;
}

template<class T>
T* vdhashtable_iterator<T>::operator->() const {
	return &static_cast<vdhashtable_node<T> *>(mpNode)->mData;
}

template<class T>
bool operator==(const vdhashtable_iterator<T>& x, const vdhashtable_iterator<T>& y) {
	return x.mpNode == y.mpNode;
}

template<class T>
bool operator==(const vdhashtable_iterator<const T>& x, const vdhashtable_iterator<T>& y) {
	return x.mpNode == y.mpNode;
}

template<class T>
bool operator==(const vdhashtable_iterator<T>& x, const vdhashtable_iterator<const T>& y) {
	return x.mpNode == y.mpNode;
}

template<class T>
bool operator!=(const vdhashtable_iterator<T>& x, const vdhashtable_iterator<T>& y) {
	return x.mpNode != y.mpNode;
}

template<class T>
bool operator!=(const vdhashtable_iterator<const T>& x, const vdhashtable_iterator<T>& y) {
	return x.mpNode != y.mpNode;
}

template<class T>
bool operator!=(const vdhashtable_iterator<T>& x, const vdhashtable_iterator<const T>& y) {
	return x.mpNode != y.mpNode;
}

///////////////////////////////////////////////////////////////////////////

template<class T>
class vdhashtable : public vdhashtable_base {
public:
	typedef T value_type;
	typedef vdhashtable_node<value_type> node_type;
	typedef value_type *pointer;
	typedef const value_type *const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef vdhashtable_iterator<value_type> iterator;
	typedef vdhashtable_iterator<const value_type> const_iterator;
	typedef vdhashtable_local_iterator<value_type> local_iterator;
	typedef vdhashtable_local_iterator<const value_type> const_local_iterator;

	// iterators
	iterator			begin();
	const_iterator		begin() const;
	iterator			end();
	const_iterator		end() const;
};

// iterators
template<class T>
typename vdhashtable<T>::iterator vdhashtable<T>::begin() {
	vdhashtable_base_node **bucket = mpBucketStart;
	vdhashtable_base_node *p = NULL;

	while(bucket != mpBucketEnd) {
		p = *bucket;
		if (p)
			break;

		++bucket;
	}

	return iterator(static_cast<node_type *>(p), bucket, mpBucketEnd);
}

template<class T>
typename vdhashtable<T>::const_iterator vdhashtable<T>::begin() const {
	vdhashtable_base_node **bucket = mpBucketStart;
	vdhashtable_base_node *p = NULL;

	while(bucket != mpBucketEnd) {
		p = *bucket;
		if (p)
			break;

		++bucket;
	}

	return const_iterator(static_cast<node_type *>(p), bucket, mpBucketEnd);
}

template<class T>
typename vdhashtable<T>::iterator vdhashtable<T>::end() {
	return iterator(NULL, NULL, NULL);
}

template<class T>
typename vdhashtable<T>::const_iterator vdhashtable<T>::end() const {
	return const_iterator(NULL, NULL, NULL);
}

#endif	// f_VD2_SYSTEM_VDSTL_HASHTABLE_H
