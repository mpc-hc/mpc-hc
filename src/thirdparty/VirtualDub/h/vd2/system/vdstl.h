//	VirtualDub - Video processing and capture application
//	System library component
//	Copyright (C) 1998-2007 Avery Lee, All Rights Reserved.
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

#ifndef VD2_SYSTEM_VDSTL_H
#define VD2_SYSTEM_VDSTL_H

#ifdef _MSC_VER
	#pragma once
#endif

#include <limits.h>
#include <stdexcept>
#include <memory>
#include <string.h>
#include <vd2/system/vdtypes.h>
#include <vd2/system/memory.h>

///////////////////////////////////////////////////////////////////////////
//
//	glue
//
///////////////////////////////////////////////////////////////////////////

template<class Iterator, class T>
struct vdreverse_iterator {
#if defined(VD_COMPILER_MSVC) && (VD_COMPILER_MSVC < 1310 || (defined(VD_COMPILER_MSVC_VC8_PSDK) || defined(VD_COMPILER_MSVC_VC8_DDK)))
	typedef std::reverse_iterator<Iterator, T> type;
#else
	typedef std::reverse_iterator<Iterator> type;
#endif
};

///////////////////////////////////////////////////////////////////////////

struct vdfalse_type { };
struct vdtrue_type  { };

struct vdfalse_result { typedef vdfalse_type result; };
struct vdtrue_result { typedef vdtrue_type result; };

template<class T> void vdmove(T& dst, T& src);
template<class T> struct vdmove_capable : public vdfalse_result {};

template<class T>
T *vdmove_forward_impl(T *src1, T *src2, T *dst, vdfalse_type) {
	T *p = src1;
	while(p != src2) {
		*dst = *p;
		++dst;
		++p;
	}

	return dst;
}

template<class T>
T *vdmove_backward_impl(T *src1, T *src2, T *dst, vdfalse_type) {
	T *p = src2;
	while(p != src1) {
		--dst;
		--p;
		*dst = *p;
	}

	return dst;
}

template<class T>
T *vdmove_forward_impl(T *src1, T *src2, T *dst, vdtrue_type) {
	T *p = src1;
	while(p != src2) {
		vdmove(*dst, *p);
		++dst;
		++p;
	}

	return dst;
}

template<class T>
T *vdmove_backward_impl(T *src1, T *src2, T *dst, vdtrue_type) {
	T *p = src2;
	while(p != src1) {
		--dst;
		--p;
		vdmove(*dst, *p);
	}

	return dst;
}

template<class T>
T *vdmove_forward(T *src1, T *src2, T *dst) {
	return vdmove_forward_impl(src1, src2, dst, vdmove_capable<T>::result());
}

template<class T>
T *vdmove_backward(T *src1, T *src2, T *dst) {
	return vdmove_backward_impl(src1, src2, dst, vdmove_capable<T>::result());
}

#define VDMOVE_CAPABLE(type) \
	template<> struct vdmove_capable<type> : public vdtrue_result {}; \
	template<> void vdmove<type>(type& dst, type& src)

///////////////////////////////////////////////////////////////////////////

template<class T, size_t N> char (&VDCountOfHelper(const T(&)[N]))[N];

#define vdcountof(array) (sizeof(VDCountOfHelper(array)))

///////////////////////////////////////////////////////////////////////////

class vdallocator_base {
protected:
	void VDNORETURN throw_oom(size_t n, size_t elsize);
};

template<class T>
class vdallocator : public vdallocator_base {
public:
	typedef	size_t		size_type;
	typedef	ptrdiff_t	difference_type;
	typedef	T*			pointer;
	typedef	const T*	const_pointer;
	typedef	T&			reference;
	typedef	const T&	const_reference;
	typedef	T			value_type;

	template<class U> struct rebind { typedef vdallocator<U> other; };

	pointer			address(reference x) const			{ return &x; }
	const_pointer	address(const_reference x) const	{ return &x; }

	pointer allocate(size_type n, void *p_close = 0) {
		if (n > ((~(size_type)0) >> 1) / sizeof(T))
			throw_oom(n, sizeof(T));
		
		pointer p = (pointer)malloc(n*sizeof(T));

		if (!p)
			throw_oom(n, sizeof(T));

		return p;
	}

	void deallocate(pointer p, size_type n) {
		free(p);
	}

	size_type		max_size() const throw()			{ return ((~(size_type)0) >> 1) / sizeof(T); }

	void			construct(pointer p, const T& val)	{ new((void *)p) T(val); }
	void			destroy(pointer p)					{ ((T*)p)->~T(); }

#if defined(_MSC_VER) && _MSC_VER < 1300
	char *			_Charalloc(size_type n)				{ return rebind<char>::other::allocate(n); }
#endif
};

///////////////////////////////////////////////////////////////////////////

template<class T, unsigned kDeadZone = 16>
class vddebug_alloc {
public:
	typedef	size_t		size_type;
	typedef	ptrdiff_t	difference_type;
	typedef	T*			pointer;
	typedef	const T*	const_pointer;
	typedef	T&			reference;
	typedef	const T&	const_reference;
	typedef	T			value_type;

	template<class U> struct rebind { typedef vddebug_alloc<U, kDeadZone> other; };

	pointer			address(reference x) const			{ return &x; }
	const_pointer	address(const_reference x) const	{ return &x; }

	pointer allocate(size_type n, void *p_close = 0) {
		pointer p = (pointer)VDAlignedMalloc(n*sizeof(T) + 2*kDeadZone, 16);

		if (!p)
			return p;

		memset((char *)p, 0xa9, kDeadZone);
		memset((char *)p + kDeadZone + n*sizeof(T), 0xa9, kDeadZone);

		return (pointer)((char *)p + kDeadZone);
	}

	void deallocate(pointer p, size_type n) {
		char *p1 = (char *)p - kDeadZone;
		char *p2 = (char *)p + n*sizeof(T);

		for(uint32 i=0; i<kDeadZone; ++i) {
			VDASSERT(p1[i] == (char)0xa9);
			VDASSERT(p2[i] == (char)0xa9);
		}

		VDAlignedFree(p1);
	}

	size_type		max_size() const throw()			{ return INT_MAX - 2*kDeadZone; }

	void			construct(pointer p, const T& val)	{ new((void *)p) T(val); }
	void			destroy(pointer p)					{ ((T*)p)->~T(); }

#if defined(_MSC_VER) && _MSC_VER < 1300
	char *			_Charalloc(size_type n)				{ return rebind<char>::other::allocate(n); }
#endif
};

///////////////////////////////////////////////////////////////////////////

template<class T, unsigned kAlignment = 16>
class vdaligned_alloc {
public:
	typedef	size_t		size_type;
	typedef	ptrdiff_t	difference_type;
	typedef	T*			pointer;
	typedef	const T*	const_pointer;
	typedef	T&			reference;
	typedef	const T&	const_reference;
	typedef	T			value_type;

	vdaligned_alloc() {}

	template<class U, unsigned kAlignment2>
	vdaligned_alloc(const vdaligned_alloc<U, kAlignment2>&) {}

	template<class U> struct rebind { typedef vdaligned_alloc<U, kAlignment> other; };

	pointer			address(reference x) const			{ return &x; }
	const_pointer	address(const_reference x) const	{ return &x; }

	pointer			allocate(size_type n, void *p = 0)	{ return (pointer)VDAlignedMalloc(n*sizeof(T), kAlignment); }
	void			deallocate(pointer p, size_type n)	{ VDAlignedFree(p); }
	size_type		max_size() const throw()			{ return INT_MAX; }

	void			construct(pointer p, const T& val)	{ new((void *)p) T(val); }
	void			destroy(pointer p)					{ ((T*)p)->~T(); }

#if defined(_MSC_VER) && _MSC_VER < 1300
	char *			_Charalloc(size_type n)				{ return rebind<char>::other::allocate(n); }
#endif
};

///////////////////////////////////////////////////////////////////////////
//
//	vdblock
//
//	vdblock<T> is similar to vector<T>, except:
//
//	1) May only be used with POD types.
//	2) No construction or destruction of elements is performed.
//	3) Capacity is always equal to size, and reallocation is performed
//	   whenever the size changes.
//	4) Contents are undefined after a reallocation.
//	5) No insertion or deletion operations are provided.
//
///////////////////////////////////////////////////////////////////////////

template<class T, class A = vdallocator<T> >
class vdblock : protected A {
public:
	typedef	T									value_type;
	typedef	typename A::pointer					pointer;
	typedef	typename A::const_pointer			const_pointer;
	typedef	typename A::reference				reference;
	typedef	typename A::const_reference			const_reference;
	typedef	size_t								size_type;
	typedef	ptrdiff_t							difference_type;
	typedef	pointer								iterator;
	typedef	const_pointer						const_iterator;
	typedef typename vdreverse_iterator<iterator, T>::type			reverse_iterator;
	typedef typename vdreverse_iterator<const_iterator, const T>::type	const_reverse_iterator;

	vdblock(const A& alloc = A()) : A(alloc), mpBlock(NULL), mSize(0) {}
	vdblock(size_type s, const A& alloc = A()) : A(alloc), mpBlock(A::allocate(s, 0)), mSize(s) {}
	~vdblock() {
		if (mpBlock)
			A::deallocate(mpBlock, mSize);
	}

	reference				operator[](size_type n)			{ return mpBlock[n]; }
	const_reference			operator[](size_type n) const	{ return mpBlock[n]; }
	reference				at(size_type n)					{ return n < mSize ? mpBlock[n] : throw std::length_error("n"); }
	const_reference			at(size_type n) const			{ return n < mSize ? mpBlock[n] : throw std::length_error("n"); }
	reference				front()							{ return *mpBlock; }
	const_reference			front() const					{ return *mpBlock; }
	reference				back()							{ return mpBlock[mSize-1]; }
	const_reference			back() const					{ return mpBlock[mSize-1]; }

	const_pointer			data() const	{ return mpBlock; }
	pointer					data()			{ return mpBlock; }

	const_iterator			begin() const	{ return mpBlock; }
	iterator				begin()			{ return mpBlock; }
	const_iterator			end() const		{ return mpBlock + mSize; }
	iterator				end()			{ return mpBlock + mSize; }

	const_reverse_iterator	rbegin() const	{ return const_reverse_iterator(end()); }
	reverse_iterator		rbegin()		{ return reverse_iterator(end()); }
	const_reverse_iterator	rend() const	{ return const_reverse_iterator(begin()); }
	reverse_iterator		rend()			{ return reverse_iterator(begin()); }

	bool					empty() const		{ return !mSize; }
	size_type				size() const		{ return mSize; }
	size_type				capacity() const	{ return mSize; }

	void clear() {
		if (mpBlock)
			A::deallocate(mpBlock, mSize);
		mpBlock = NULL;
		mSize = 0;
	}

	void resize(size_type s) {
		if (s != mSize) {
			if (mpBlock) {
				A::deallocate(mpBlock, mSize);
				mpBlock = NULL;
				mSize = 0;
			}
			if (s)
				mpBlock = A::allocate(s, 0);
			mSize = s;
		}
	}

	void resize(size_type s, const T& value) {
		if (s != mSize) {
			if (mpBlock) {
				A::deallocate(mpBlock, mSize);
				mpBlock = NULL;
				mSize = 0;
			}
			if (s) {
				mpBlock = A::allocate(s, 0);
				std::fill(mpBlock, mpBlock+s, value);
			}
			mSize = s;
		}
	}

	void swap(vdblock& x) {
		std::swap(mpBlock, x.mpBlock);
		std::swap(mSize, x.mSize);
	}

protected:
	typename A::pointer		mpBlock;
	typename A::size_type	mSize;

	union PODType {
		T x;
	};
};

///////////////////////////////////////////////////////////////////////////
//
//	vdstructex
//
//	vdstructex describes an extensible format structure, such as
//	BITMAPINFOHEADER or WAVEFORMATEX, without the pain-in-the-butt
//	casting normally associated with one.
//
///////////////////////////////////////////////////////////////////////////

template<class T>
class vdstructex {
public:
	typedef size_t			size_type;
	typedef T				value_type;

	vdstructex() : mpMemory(NULL), mSize(0) {}

	explicit vdstructex(size_t len) : mpMemory(NULL), mSize(0) {
		resize(len);
	}

	vdstructex(const T *pStruct, size_t len) : mSize(len), mpMemory((T*)malloc(len)) {
		memcpy(mpMemory, pStruct, len);
	}

	vdstructex(const vdstructex<T>& src) : mSize(src.mSize), mpMemory((T*)malloc(src.mSize)) {
		memcpy(mpMemory, src.mpMemory, mSize);
	}

	~vdstructex() {
		free(mpMemory);
	}

	bool		empty() const		{ return !mpMemory; }
	size_type	size() const		{ return mSize; }
	T*			data() const		{ return mpMemory; }

	T&	operator *() const	{ return *(T *)mpMemory; }
	T*	operator->() const	{ return (T *)mpMemory; }

	bool operator==(const vdstructex& x) const {
		return mSize == x.mSize && (!mSize || !memcmp(mpMemory, x.mpMemory, mSize));
	}

	bool operator!=(const vdstructex& x) const {
		return mSize != x.mSize || (mSize && memcmp(mpMemory, x.mpMemory, mSize));
	}

	vdstructex<T>& operator=(const vdstructex<T>& src) {
		assign(src.mpMemory, src.mSize);
		return *this;
	}

	void assign(const T *pStruct, size_type len) {
		if (mSize != len)
			resize(len);

		memcpy(mpMemory, pStruct, len);
	}

	void clear() {
		free(mpMemory);
		mpMemory = NULL;
		mSize = 0;
	}

	void resize(size_type len) {
		if (mSize != len)
			mpMemory = (T *)realloc(mpMemory, mSize = len);
	}

protected:
	size_type	mSize;
	T *mpMemory;
};

///////////////////////////////////////////////////////////////////////////
//
//	vdlist
//
//	vdlist<T> is similar to list<T*>, except:
//
//	1) The node structure must be embedded as a superclass of T.
//     Thus, the client is in full control of allocation.
//	2) Node pointers may be converted back into iterators in O(1).
//
///////////////////////////////////////////////////////////////////////////

struct vdlist_node {
	vdlist_node *mListNodeNext, *mListNodePrev;
};

template<class T, class T_Nonconst>
class vdlist_iterator {
public:
	typedef ptrdiff_t difference_type;
	typedef T value_type;
	typedef T *pointer_type;
	typedef T& reference_type;
	typedef std::bidirectional_iterator_tag iterator_category;

	vdlist_iterator() {}
	vdlist_iterator(T *p) : mp(p) {}
	vdlist_iterator(const vdlist_iterator<T_Nonconst, T_Nonconst>& src) : mp(src.mp) {}

	T* operator *() const {
		return static_cast<T*>(mp);
	}

	bool operator==(const vdlist_iterator<T, T_Nonconst>& x) const {
		return mp == x.mp;
	}

	bool operator!=(const vdlist_iterator<T, T_Nonconst>& x) const {
		return mp != x.mp;
	}

	vdlist_iterator& operator++() {
		mp = mp->mListNodeNext;
		return *this;
	}

	vdlist_iterator& operator--() {
		mp = mp->mListNodePrev;
		return *this;
	}

	vdlist_iterator operator++(int) {
		vdlist_iterator tmp(*this);
		mp = mp->mListNodeNext;
		return tmp;
	}

	vdlist_iterator& operator--(int) {
		vdlist_iterator tmp(*this);
		mp = mp->mListNodePrev;
		return tmp;
	}

	vdlist_node *mp;
};

class vdlist_base {
public:
	typedef	vdlist_node						node;
	typedef	size_t							size_type;
	typedef	ptrdiff_t						difference_type;

	bool empty() const {
		return mAnchor.mListNodeNext == &mAnchor;
	}

	size_type size() const {
		node *p = { mAnchor.mListNodeNext };
		size_type s = 0;

		if (p != &mAnchor)
			do {
				++s;
				p = p->mListNodeNext;
			} while(p != &mAnchor);

		return s;
	}

	void clear() {
		mAnchor.mListNodePrev	= &mAnchor;
		mAnchor.mListNodeNext	= &mAnchor;
	}

	void pop_front() {
		mAnchor.mListNodeNext = mAnchor.mListNodeNext->mListNodeNext;
		mAnchor.mListNodeNext->mListNodePrev = &mAnchor;
	}

	void pop_back() {
		mAnchor.mListNodePrev = mAnchor.mListNodePrev->mListNodePrev;
		mAnchor.mListNodePrev->mListNodeNext = &mAnchor;
	}

	static void unlink(vdlist_node& node) {
		vdlist_node& n1 = *node.mListNodePrev;
		vdlist_node& n2 = *node.mListNodeNext;

		n1.mListNodeNext = &n2;
		n2.mListNodePrev = &n1;
	}

protected:
	node	mAnchor;
};

template<class T>
class vdlist : public vdlist_base {
public:
	typedef	T*								value_type;
	typedef	T**								pointer;
	typedef	const T**						const_pointer;
	typedef	T*&								reference;
	typedef	const T*&						const_reference;
	typedef	vdlist_iterator<T, T>						iterator;
	typedef vdlist_iterator<const T, T>					const_iterator;
	typedef typename vdreverse_iterator<iterator, T>::type			reverse_iterator;
	typedef typename vdreverse_iterator<const_iterator, const T>::type	const_reverse_iterator;

	vdlist() {
		mAnchor.mListNodePrev	= &mAnchor;
		mAnchor.mListNodeNext	= &mAnchor;
	}

	iterator begin() {
		iterator it;
		it.mp = mAnchor.mListNodeNext;
		return it;
	}

	const_iterator begin() const {
		const_iterator it;
		it.mp = mAnchor.mListNodeNext;
		return it;
	}

	iterator end() {
		iterator it;
		it.mp = &mAnchor;
		return it;
	}

	const_iterator end() const {
		const_iterator it;
		it.mp = &mAnchor;
		return it;
	}

	reverse_iterator rbegin() {
		return reverse_iterator(begin());
	}

	const_reverse_iterator rbegin() const {
		return const_reverse_iterator(begin());
	}

	reverse_iterator rend() {
		return reverse_iterator(end);
	}

	const_reverse_iterator rend() const {
		return const_reverse_iterator(end());
	}

	const value_type front() const {
		return static_cast<T *>(mAnchor.mListNodeNext);
	}

	const value_type back() const {
		return static_cast<T *>(mAnchor.mListNodePrev);
	}

	iterator find(T *p) {
		iterator it;
		it.mp = mAnchor.mListNodeNext;

		if (it.mp != &mAnchor)
			do {
				if (it.mp == static_cast<node *>(p))
					break;

				it.mp = it.mp->mListNodeNext;
			} while(it.mp != &mAnchor);

		return it;
	}

	const_iterator find(T *p) const {
		const_iterator it;
		it.mp = mAnchor.mListNodeNext;

		if (it.mp != &mAnchor)
			do {
				if (it.mp == static_cast<node *>(p))
					break;

				it.mp = it.mp->mListNodeNext;
			} while(it.mp != &mAnchor);

		return it;
	}

	iterator fast_find(T *p) {
		iterator it(p);
		return it;
	}

	const_iterator fast_find(T *p) const {
		iterator it(p);
	}

	void push_front(T *p) {
		node& n = *p;
		n.mListNodePrev = &mAnchor;
		n.mListNodeNext = mAnchor.mListNodeNext;
		n.mListNodeNext->mListNodePrev = &n;
		mAnchor.mListNodeNext = &n;
	}

	void push_back(T *p) {
		node& n = *p;
		n.mListNodeNext = &mAnchor;
		n.mListNodePrev = mAnchor.mListNodePrev;
		n.mListNodePrev->mListNodeNext = &n;
		mAnchor.mListNodePrev = &n;
	}

	iterator erase(T *p) {
		return erase(fast_find(p));
	}

	iterator erase(iterator it) {
		node& n = *it.mp;

		n.mListNodePrev->mListNodeNext = n.mListNodeNext;
		n.mListNodeNext->mListNodePrev = n.mListNodePrev;

		it.mp = n.mListNodeNext;
		return it;
	}

	iterator erase(iterator i1, iterator i2) {
		node& np = *i1.mp->mListNodePrev;
		node& nn = *i2.mp;

		np.mListNodeNext = &nn;
		nn.mListNodePrev = &np;

		return i2;
	}

	void insert(iterator dst, T *src) {
		node& ns = *src;
		node& nd = *dst.mp;

		ns.mListNodeNext = &nd;
		ns.mListNodePrev = nd.mListNodePrev;
		nd.mListNodePrev->mListNodeNext = &ns;
		nd.mListNodePrev = &ns;
	}

	void insert(iterator dst, iterator i1, iterator i2) {
		if (i1 != i2) {
			node& np = *dst.mp->mListNodePrev;
			node& nn = *dst.mp;
			node& n1 = *i1.mp;
			node& n2 = *i2.mp->mListNodePrev;

			np.mListNodeNext = &n1;
			n1.mListNodePrev = &np;
			n2.mListNodeNext = &nn;
			nn.mListNodePrev = &n2;
		}
	}

	void splice(iterator dst, vdlist<T>& srclist) {
		insert(dst, srclist.begin(), srclist.end());
		srclist.clear();
	}

	void splice(iterator dst, vdlist<T>& srclist, iterator src) {
		T *v = *src;
		srclist.erase(src);
		insert(dst, v);
	}

	void splice(iterator dst, vdlist<T>& srclist, iterator i1, iterator i2) {
		if (dst.mp != i1.mp && dst.mp != i2.mp) {
			srclist.erase(i1, i2);
			insert(dst, i1, i2);
		}
	}
};

///////////////////////////////////////////////////////////////////////////////

#if defined(_DEBUG) && defined(_MSC_VER)
	#define VD_ACCELERATE_TEMPLATES
#endif

#ifndef VDTINLINE
	#ifdef VD_ACCELERATE_TEMPLATES
		#ifndef VDTEXTERN
			#define VDTEXTERN extern
		#endif

		#define VDTINLINE
	#else
		#define VDTINLINE inline
	#endif
#endif

///////////////////////////////////////////////////////////////////////////////

template<class T>
class vdspan {
public:
	typedef	T					value_type;
	typedef	T*					pointer;
	typedef	const T*			const_pointer;
	typedef	T&					reference;
	typedef	const T&			const_reference;
	typedef	size_t				size_type;
	typedef	ptrdiff_t			difference_type;
	typedef	pointer				iterator;
	typedef const_pointer		const_iterator;
	typedef typename vdreverse_iterator<iterator, T>::type			reverse_iterator;
	typedef typename vdreverse_iterator<const_iterator, const T>::type	const_reverse_iterator;

	VDTINLINE vdspan();

	template<size_t N>
	VDTINLINE vdspan(T (&arr)[N]);

	VDTINLINE vdspan(T *p1, T *p2);
	VDTINLINE vdspan(T *p1, size_type len);

public:
	VDTINLINE bool					empty() const;
	VDTINLINE size_type				size() const;

	VDTINLINE pointer				data();
	VDTINLINE const_pointer			data() const;

	VDTINLINE iterator				begin();
	VDTINLINE const_iterator			begin() const;
	VDTINLINE iterator				end();
	VDTINLINE const_iterator			end() const;

	VDTINLINE reverse_iterator		rbegin();
	VDTINLINE const_reverse_iterator	rbegin() const;
	VDTINLINE reverse_iterator		rend();
	VDTINLINE const_reverse_iterator	rend() const;

	VDTINLINE reference				front();
	VDTINLINE const_reference		front() const;
	VDTINLINE reference				back();
	VDTINLINE const_reference		back() const;

	VDTINLINE reference				operator[](size_type n);
	VDTINLINE const_reference		operator[](size_type n) const;

protected:
	T *mpBegin;
	T *mpEnd;
};

#ifdef VD_ACCELERATE_TEMPLATES
	#pragma warning(push)
	#pragma warning(disable: 4231)		//  warning C4231: nonstandard extension used : 'extern' before template explicit instantiation
	VDTEXTERN template vdspan<char>;
	VDTEXTERN template vdspan<uint8>;
	VDTEXTERN template vdspan<uint16>;
	VDTEXTERN template vdspan<uint32>;
	VDTEXTERN template vdspan<uint64>;
	VDTEXTERN template vdspan<sint8>;
	VDTEXTERN template vdspan<sint16>;
	VDTEXTERN template vdspan<sint32>;
	VDTEXTERN template vdspan<sint64>;
	VDTEXTERN template vdspan<float>;
	VDTEXTERN template vdspan<double>;
	VDTEXTERN template vdspan<wchar_t>;
	#pragma warning(pop)
#endif

template<class T> VDTINLINE vdspan<T>::vdspan() : mpBegin(NULL), mpEnd(NULL) {}
template<class T> template<size_t N> VDTINLINE vdspan<T>::vdspan(T (&arr)[N]) : mpBegin(&arr[0]), mpEnd(&arr[N]) {}
template<class T> VDTINLINE vdspan<T>::vdspan(T *p1, T *p2) : mpBegin(p1), mpEnd(p2) {}
template<class T> VDTINLINE vdspan<T>::vdspan(T *p, size_type len) : mpBegin(p), mpEnd(p+len) {}
template<class T> VDTINLINE bool					vdspan<T>::empty() const { return mpBegin == mpEnd; }
template<class T> VDTINLINE typename vdspan<T>::size_type			vdspan<T>::size() const { return size_type(mpEnd - mpBegin); }
template<class T> VDTINLINE typename vdspan<T>::pointer				vdspan<T>::data() { return mpBegin; }
template<class T> VDTINLINE typename vdspan<T>::const_pointer		vdspan<T>::data() const { return mpBegin; }
template<class T> VDTINLINE typename vdspan<T>::iterator				vdspan<T>::begin() { return mpBegin; }
template<class T> VDTINLINE typename vdspan<T>::const_iterator		vdspan<T>::begin() const { return mpBegin; }
template<class T> VDTINLINE typename vdspan<T>::iterator				vdspan<T>::end() { return mpEnd; }
template<class T> VDTINLINE typename vdspan<T>::const_iterator		vdspan<T>::end() const { return mpEnd; }
template<class T> VDTINLINE typename vdspan<T>::reverse_iterator		vdspan<T>::rbegin() { return reverse_iterator(mpEnd); }
template<class T> VDTINLINE typename vdspan<T>::const_reverse_iterator vdspan<T>::rbegin() const { return const_reverse_iterator(mpEnd); }
template<class T> VDTINLINE typename vdspan<T>::reverse_iterator		vdspan<T>::rend() { return reverse_iterator(mpBegin); }
template<class T> VDTINLINE typename vdspan<T>::const_reverse_iterator vdspan<T>::rend() const { return const_reverse_iterator(mpBegin); }
template<class T> VDTINLINE typename vdspan<T>::reference			vdspan<T>::front() { return *mpBegin; }
template<class T> VDTINLINE typename vdspan<T>::const_reference		vdspan<T>::front() const { return *mpBegin; }
template<class T> VDTINLINE typename vdspan<T>::reference			vdspan<T>::back() { VDASSERT(mpBegin != mpEnd); return mpEnd[-1]; }
template<class T> VDTINLINE typename vdspan<T>::const_reference		vdspan<T>::back() const { VDASSERT(mpBegin != mpEnd); return mpEnd[-1]; }
template<class T> VDTINLINE typename vdspan<T>::reference			vdspan<T>::operator[](size_type n) { VDASSERT(n < size_type(mpEnd - mpBegin)); return mpBegin[n]; }
template<class T> VDTINLINE typename vdspan<T>::const_reference		vdspan<T>::operator[](size_type n) const { VDASSERT(n < size_type(mpEnd - mpBegin)); return mpBegin[n]; }

///////////////////////////////////////////////////////////////////////////////

template<class T>
bool operator==(const vdspan<T>& x, const vdspan<T>& y) {
	uint32 len = x.size();
	if (len != y.size())
		return false;

	const T *px = x.data();
	const T *py = y.data();

	for(uint32 i=0; i<len; ++i) {
		if (px[i] != py[i])
			return false;
	}

	return true;
}

template<class T>
inline bool operator!=(const vdspan<T>& x, const vdspan<T>& y) { return !(x == y); }

///////////////////////////////////////////////////////////////////////////////

template<class T, class S, class A = vdallocator<T> >
class vdfastvector_base : public vdspan<T> {
protected:
	using vdspan<T>::mpBegin;
	using vdspan<T>::mpEnd;

public:
	typedef typename vdspan<T>::value_type value_type;
	typedef typename vdspan<T>::pointer pointer;
	typedef typename vdspan<T>::const_pointer const_pointer;
	typedef typename vdspan<T>::reference reference;
	typedef typename vdspan<T>::const_reference const_reference;
	typedef typename vdspan<T>::size_type size_type;
	typedef typename vdspan<T>::difference_type difference_type;
	typedef typename vdspan<T>::iterator iterator;
	typedef typename vdspan<T>::const_iterator const_iterator;
	typedef typename vdspan<T>::reverse_iterator reverse_iterator;
	typedef typename vdspan<T>::const_reverse_iterator const_reverse_iterator;

	~vdfastvector_base() {
		if (static_cast<const S&>(m).is_deallocatable_storage(mpBegin))
			m.deallocate(mpBegin, m.eos - mpBegin);
	}

	size_type capacity() const { return size_type(m.eos - mpBegin); }

public:
	T *alloc(size_type n) {
		size_type offset = (size_type)(mpEnd - mpBegin);
		resize(offset + n);
		return mpBegin + offset;
	}

	void assign(const T *p1, const T *p2) {
		resize(p2 - p1);
		memcpy(mpBegin, p1, (char *)p2 - (char *)p1);
	}

	void clear() {
		mpEnd = mpBegin;
	}

	iterator erase(iterator it) {
		VDASSERT(it - mpBegin < mpEnd - mpBegin);

		memmove(it, it+1, (char *)mpEnd - (char *)(it+1));

		--mpEnd;

		return it;
	}

	iterator erase(iterator it1, iterator it2) {
		VDASSERT(it1 - mpBegin <= mpEnd - mpBegin);
		VDASSERT(it2 - mpBegin <= mpEnd - mpBegin);
		VDASSERT(it1 <= it2);

		memmove(it1, it2, (char *)mpEnd - (char *)it2);

		mpEnd -= (it2 - it1);

		return it1;
	}

	iterator insert(iterator it, const T& value) {
		const T temp(value);		// copy in case value is inside container.

		if (mpEnd == m.eos) {
			difference_type delta = it - mpBegin;
			_reserve_always_add_one();
			it = mpBegin + delta;
		}

		memmove(it+1, it, sizeof(T) * (mpEnd - it));
		*it = temp;
		++mpEnd;
		VDASSERT(mpEnd <= m.eos);

		return it;
	}

	iterator insert(iterator it, size_type n, const T& value) {
		const T temp(value);		// copy in case value is inside container.

		ptrdiff_t bytesToInsert = n * sizeof(T);

		if ((char *)m.eos - (char *)mpEnd < bytesToInsert) {
			difference_type delta = it - mpBegin;
			_reserve_always_add(bytesToInsert);
			it = mpBegin + delta;
		}

		memmove((char *)it + bytesToInsert, it, (char *)mpEnd - (char *)it);
		for(size_t i=0; i<n; ++i)
			*it++ = temp;
		mpEnd += n;
		VDASSERT(mpEnd <= m.eos);
		return it;
	}

	iterator insert(iterator it, const T *p1, const T *p2) {
		ptrdiff_t elementsToCopy = p2 - p1;
		ptrdiff_t bytesToCopy = (char *)p2 - (char *)p1;

		if ((char *)m.eos - (char *)mpEnd < bytesToCopy) {
			difference_type delta = it - mpBegin;
			_reserve_always_add(bytesToCopy);
			it = mpBegin + delta;
		}

		memmove((char *)it + bytesToCopy, it, (char *)mpEnd - (char *)it);
		memcpy(it, p1, bytesToCopy);
		mpEnd += elementsToCopy;
		VDASSERT(mpEnd <= m.eos);
		return it;
	}

	reference push_back() {
		if (mpEnd == m.eos)
			_reserve_always_add_one();

		return *mpEnd++;
	}

	void push_back(const T& value) {
		const T temp(value);		// copy in case value is inside container.

		if (mpEnd == m.eos)
			_reserve_always_add_one();

		*mpEnd++ = temp;
	}

	void pop_back() {
		VDASSERT(mpBegin != mpEnd);
		--mpEnd;
	}

	void resize(size_type n) {
		if (n*sizeof(T) > size_type((char *)m.eos - (char *)mpBegin))
			_reserve_always_amortized(n);

		mpEnd = mpBegin + n;
	}

	void resize(size_type n, const T& value) {
		const T temp(value);

		if (n*sizeof(T) > size_type((char *)m.eos - (char *)mpBegin)) {
			_reserve_always_amortized(n);
		}

		const iterator newEnd(mpBegin + n);
		if (newEnd > mpEnd)
			std::fill(mpEnd, newEnd, temp);
		mpEnd = newEnd;
	}

	void reserve(size_type n) {
		if (n*sizeof(T) > size_type((char *)m.eos - (char *)mpBegin))
			_reserve_always(n);
	}

protected:
#ifdef _MSC_VER
	__declspec(noinline)
#endif
	void _reserve_always_add_one() {
		_reserve_always((m.eos - mpBegin) * 2 + 1);
	}

#ifdef _MSC_VER
	__declspec(noinline)
#endif
	void _reserve_always_add(size_type n) {
		_reserve_always((m.eos - mpBegin) * 2 + n);
	}

#ifdef _MSC_VER
	__declspec(noinline)
#endif
	void _reserve_always(size_type n) {
		size_type oldSize = mpEnd - mpBegin;
		T *oldStorage = mpBegin;
		T *newStorage = m.allocate(n, NULL);

		memcpy(newStorage, mpBegin, (char *)mpEnd - (char *)mpBegin);
		if (static_cast<const S&>(m).is_deallocatable_storage(oldStorage))
			m.deallocate(oldStorage, m.eos - mpBegin);
		mpBegin = newStorage;
		mpEnd = newStorage + oldSize;
		m.eos = newStorage + n;
	}

#ifdef _MSC_VER
	__declspec(noinline)
#endif
	void _reserve_always_amortized(size_type n) {
		size_type nextCapacity = (size_type)((m.eos - mpBegin)*2);

		if (nextCapacity < n)
			nextCapacity = n;

		_reserve_always(nextCapacity);
	}

	struct : A, S {
		T *eos;
	} m;

	union TrivialObjectConstraint {
		T m;
	};
};

///////////////////////////////////////////////////////////////////////////////

struct vdfastvector_storage {
	bool is_deallocatable_storage(void *p) const {
		return p != 0;
	}
};

template<class T, class A = vdallocator<T> >
class vdfastvector : public vdfastvector_base<T, vdfastvector_storage, A> {
protected:
	using vdfastvector_base<T, vdfastvector_storage, A>::m;
	using vdfastvector_base<T, vdfastvector_storage, A>::mpBegin;
	using vdfastvector_base<T, vdfastvector_storage, A>::mpEnd;

public:
	typedef typename vdfastvector_base<T, vdfastvector_storage, A>::value_type value_type;
	typedef typename vdfastvector_base<T, vdfastvector_storage, A>::pointer pointer;
	typedef typename vdfastvector_base<T, vdfastvector_storage, A>::const_pointer const_pointer;
	typedef typename vdfastvector_base<T, vdfastvector_storage, A>::reference reference;
	typedef typename vdfastvector_base<T, vdfastvector_storage, A>::const_reference const_reference;
	typedef typename vdfastvector_base<T, vdfastvector_storage, A>::size_type size_type;
	typedef typename vdfastvector_base<T, vdfastvector_storage, A>::difference_type difference_type;
	typedef typename vdfastvector_base<T, vdfastvector_storage, A>::iterator iterator;
	typedef typename vdfastvector_base<T, vdfastvector_storage, A>::const_iterator const_iterator;
	typedef typename vdfastvector_base<T, vdfastvector_storage, A>::reverse_iterator reverse_iterator;
	typedef typename vdfastvector_base<T, vdfastvector_storage, A>::const_reverse_iterator const_reverse_iterator;

	vdfastvector() {
		m.eos = NULL;
	}

	vdfastvector(size_type len) {
		mpBegin = m.allocate(len, NULL);
		mpEnd = mpBegin + len;
		m.eos = mpEnd;
	}

	vdfastvector(size_type len, const T& fill) {
		mpBegin = m.allocate(len, NULL);
		mpEnd = mpBegin + len;
		m.eos = mpEnd;

		std::fill(mpBegin, mpEnd, fill);
	}

	vdfastvector(const vdfastvector& x) {
		size_type n = x.mpEnd - x.mpBegin;
		mpBegin = m.allocate(n, NULL);
		mpEnd = mpBegin + n;
		m.eos = mpEnd;
		memcpy(mpBegin, x.mpBegin, sizeof(T) * n);
	}

	vdfastvector(const value_type *p, const value_type *q) {
		m.eos = NULL;

		assign(p, q);
	}

	vdfastvector& operator=(const vdfastvector& x) {
		if (this != &x)
			assign(x.mpBegin, x.mpEnd);

		return *this;
	}

	void swap(vdfastvector& x) {
		std::swap(mpBegin, x.mpBegin);
		std::swap(mpEnd, x.mpEnd);
		std::swap(m.eos, x.m.eos);
	}

	void swap(vdfastvector&& x) {
		std::swap(mpBegin, x.mpBegin);
		std::swap(mpEnd, x.mpEnd);
		std::swap(m.eos, x.m.eos);
	}
};

///////////////////////////////////////////////////////////////////////////////

template<class T, size_t N>
struct vdfastfixedvector_storage {
	T mArray[N];

	bool is_deallocatable_storage(void *p) const {
		return p != mArray;
	}
};

template<class T, size_t N, class A = vdallocator<T> >
class vdfastfixedvector : public vdfastvector_base<T, vdfastfixedvector_storage<T, N>, A> {
protected:
	using vdfastvector_base<T, vdfastfixedvector_storage<T, N>, A>::mpBegin;
	using vdfastvector_base<T, vdfastfixedvector_storage<T, N>, A>::mpEnd;
	using vdfastvector_base<T, vdfastfixedvector_storage<T, N>, A>::m;

public:
	typedef typename vdfastvector_base<T, vdfastfixedvector_storage<T, N>, A>::value_type value_type;
	typedef typename vdfastvector_base<T, vdfastfixedvector_storage<T, N>, A>::pointer pointer;
	typedef typename vdfastvector_base<T, vdfastfixedvector_storage<T, N>, A>::const_pointer const_pointer;
	typedef typename vdfastvector_base<T, vdfastfixedvector_storage<T, N>, A>::reference reference;
	typedef typename vdfastvector_base<T, vdfastfixedvector_storage<T, N>, A>::const_reference const_reference;
	typedef typename vdfastvector_base<T, vdfastfixedvector_storage<T, N>, A>::size_type size_type;
	typedef typename vdfastvector_base<T, vdfastfixedvector_storage<T, N>, A>::difference_type difference_type;
	typedef typename vdfastvector_base<T, vdfastfixedvector_storage<T, N>, A>::iterator iterator;
	typedef typename vdfastvector_base<T, vdfastfixedvector_storage<T, N>, A>::const_iterator const_iterator;
	typedef typename vdfastvector_base<T, vdfastfixedvector_storage<T, N>, A>::reverse_iterator reverse_iterator;
	typedef typename vdfastvector_base<T, vdfastfixedvector_storage<T, N>, A>::const_reverse_iterator const_reverse_iterator;

	vdfastfixedvector() {
		mpBegin = m.mArray;
		mpEnd = m.mArray;
		m.eos = m.mArray + N;
	}

	vdfastfixedvector(size_type len) {
		if (len <= N) {
			mpBegin = m.mArray;
			mpEnd = m.mArray + len;
			m.eos = m.mArray + N;
		} else {
			mpBegin = m.allocate(len, NULL);
			mpEnd = mpBegin + len;
			m.eos = mpEnd;
		}
	}

	vdfastfixedvector(size_type len, const T& fill) {
		mpBegin = m.allocate(len, NULL);
		mpEnd = mpBegin + len;
		m.eos = mpEnd;

		std::fill(mpBegin, mpEnd, fill);
	}

	vdfastfixedvector(const vdfastfixedvector& x) {
		size_type n = x.mpEnd - x.mpBegin;

		if (n <= N) {
			mpBegin = m.mArray;
			mpEnd = m.mArray + n;
			m.eos = m.mArray + N;
		} else {
			mpBegin = m.allocate(n, NULL);
			mpEnd = mpBegin + n;
			m.eos = mpEnd;
		}

		memcpy(mpBegin, x.mpBegin, sizeof(T) * n);
	}

	vdfastfixedvector(const value_type *p, const value_type *q) {
		mpBegin = m.mArray;
		mpEnd = m.mArray;
		m.eos = m.mArray + N;

		assign(p, q);
	}

	vdfastfixedvector& operator=(const vdfastfixedvector& x) {
		if (this != &x)
			assign(x.mpBegin, x.mpEnd);

		return *this;
	}

	void swap(vdfastfixedvector& x) {
		size_t this_bytes = (char *)mpEnd - (char *)mpBegin;
		size_t other_bytes = (char *)x.mpEnd - (char *)x.mpBegin;

		T *p;

		if (mpBegin == m.mArray) {
			if (x.mpBegin == x.m.mArray) {
				if (this_bytes < other_bytes) {
					VDSwapMemory(m.mArray, x.m.mArray, this_bytes);
					memcpy((char *)m.mArray + this_bytes, (char *)x.m.mArray + this_bytes, other_bytes - this_bytes);
				} else {
					VDSwapMemory(m.mArray, x.m.mArray, other_bytes);
					memcpy((char *)m.mArray + other_bytes, (char *)x.m.mArray + other_bytes, this_bytes - other_bytes);
				}

				mpEnd = (T *)((char *)mpBegin + other_bytes);
				x.mpEnd = (T *)((char *)x.mpBegin + this_bytes);
			} else {
				memcpy(x.m.mArray, mpBegin, this_bytes);

				mpBegin = x.mpBegin;
				mpEnd = x.mpEnd;
				m.eos = x.m.eos;

				x.mpBegin = x.m.mArray;
				x.mpEnd = (T *)((char *)x.m.mArray + this_bytes);
				x.m.eos = x.m.mArray + N;
			}
		} else {
			if (x.mpBegin == x.m.mArray) {
				memcpy(x.m.mArray, mpBegin, other_bytes);

				x.mpBegin = mpBegin;
				x.mpEnd = mpEnd;
				x.m.eos = m.eos;

				mpBegin = m.mArray;
				mpEnd = (T *)((char *)m.mArray + other_bytes);
				m.eos = m.mArray + N;
			} else {
				p = mpBegin;		mpBegin = x.mpBegin;		x.mpBegin = p;
				p = mpEnd;			mpEnd = x.mpEnd;			x.mpEnd = p;
				p = m.eos;			m.eos = x.m.eos;			x.m.eos = p;
			}
		}
	}
};

///////////////////////////////////////////////////////////////////////////////

template<class T>
struct vdfastdeque_block {
	enum {
		kBlockSize = 32,
		kBlockSizeBits = 5
	};

	T data[kBlockSize];
};

template<class T, class T_Base>
class vdfastdeque_iterator {
public:
	typedef T value_type;
	typedef T* pointer;
	typedef T& reference;
	typedef ptrdiff_t difference_type;
	typedef std::bidirectional_iterator_tag iterator_category;

	vdfastdeque_iterator(const vdfastdeque_iterator<T_Base, T_Base>&);
	vdfastdeque_iterator(vdfastdeque_block<T_Base> **pMapEntry, uint32 index);

	T& operator *() const;
	T& operator ->() const;
	vdfastdeque_iterator& operator++();
	vdfastdeque_iterator operator++(int);
	vdfastdeque_iterator& operator--();
	vdfastdeque_iterator operator--(int);

public:
	vdfastdeque_block<T_Base> **mpMap;
	vdfastdeque_block<T_Base> *mpBlock;
	uint32 mIndex;
};

template<class T, class T_Base>
vdfastdeque_iterator<T, T_Base>::vdfastdeque_iterator(const vdfastdeque_iterator<T_Base, T_Base>& x)
	: mpMap(x.mpMap)
	, mpBlock(x.mpBlock)
	, mIndex(x.mIndex)
{
}

template<class T, class T_Base>
vdfastdeque_iterator<T, T_Base>::vdfastdeque_iterator(vdfastdeque_block<T_Base> **pMapEntry, uint32 index)
	: mpMap(pMapEntry)
	, mpBlock(mpMap ? *mpMap : NULL)
	, mIndex(index)
{
}

template<class T, class T_Base>
T& vdfastdeque_iterator<T, T_Base>::operator *() const {
	return mpBlock->data[mIndex];
}

template<class T, class T_Base>
T& vdfastdeque_iterator<T, T_Base>::operator ->() const {
	return mpBlock->data[mIndex];
}

template<class T, class T_Base>
vdfastdeque_iterator<T, T_Base>& vdfastdeque_iterator<T, T_Base>::operator++() {
	if (++mIndex >= vdfastdeque_block<T>::kBlockSize) {
		mIndex = 0;
		mpBlock = *++mpMap;
	}
	return *this;
}

template<class T, class T_Base>
vdfastdeque_iterator<T, T_Base> vdfastdeque_iterator<T, T_Base>::operator++(int) {
	vdfastdeque_iterator r(*this);
	operator++();
	return r;
}

template<class T, class T_Base>
vdfastdeque_iterator<T, T_Base>& vdfastdeque_iterator<T, T_Base>::operator--() {
	if (mIndex-- == 0) {
		mIndex = vdfastdeque_block<T>::kBlockSize - 1;
		mpBlock = *--mpMap;
	}
	return *this;
}

template<class T, class T_Base>
vdfastdeque_iterator<T, T_Base> vdfastdeque_iterator<T, T_Base>::operator--(int) {
	vdfastdeque_iterator r(*this);
	operator--();
	return r;
}

template<class T, class U, class T_Base>
bool operator==(const vdfastdeque_iterator<T, T_Base>& x,const vdfastdeque_iterator<U, T_Base>& y) {
	return x.mpBlock == y.mpBlock && x.mIndex == y.mIndex;
}

template<class T, class U, class T_Base>
bool operator!=(const vdfastdeque_iterator<T, T_Base>& x,const vdfastdeque_iterator<U, T_Base>& y) {
	return x.mpBlock != y.mpBlock || x.mIndex != y.mIndex;
}

///////////////////////////////////////////////////////////////////////////////

template<class T, class A = vdallocator<T> >
class vdfastdeque {
public:
	typedef	typename A::reference		reference;
	typedef	typename A::const_reference	const_reference;
	typedef	typename A::pointer			pointer;
	typedef	typename A::const_pointer	const_pointer;
	typedef	T					value_type;
	typedef A					allocator_type;
	typedef	size_t				size_type;
	typedef	ptrdiff_t			difference_type;
	typedef	vdfastdeque_iterator<T, T>			iterator;
	typedef vdfastdeque_iterator<const T, T>	const_iterator;
	typedef typename vdreverse_iterator<iterator, T>::type			reverse_iterator;
	typedef typename vdreverse_iterator<const_iterator, const T>::type	const_reverse_iterator;

	vdfastdeque();
	~vdfastdeque();

	bool				empty() const;
	size_type			size() const;

	reference			front();
	const_reference		front() const;
	reference			back();
	const_reference		back() const;

	iterator			begin();
	const_iterator		begin() const;
	iterator			end();
	const_iterator		end() const;

	reference			operator[](size_type n);
	const_reference		operator[](size_type n) const;

	void				clear();

	reference			push_back();
	void				push_back(const_reference x);

	void				pop_front();
	void				pop_back();

	void				swap(vdfastdeque& x);

protected:
	void				push_back_extend();
	void				validate();

	typedef vdfastdeque_block<T> Block;

	enum {
		kBlockSize = Block::kBlockSize,
		kBlockSizeBits = Block::kBlockSizeBits
	};

	struct M1 : public A::template rebind<Block *>::other {
		Block **mapStartAlloc;		// start of map
		Block **mapStartCommit;		// start of range of allocated blocks
		Block **mapStart;			// start of range of active blocks
		Block **mapEnd;				// end of range of active blocks
		Block **mapEndCommit;		// end of range of allocated blocks
		Block **mapEndAlloc;		// end of map
	} m;

	struct M2 : public A::template rebind<Block>::other {
		int startIndex;
		int endIndex;
	} mTails;

	union TrivialObjectConstraint {
		T obj;
	};
};

template<class T, class A>
vdfastdeque<T, A>::vdfastdeque() {
	m.mapStartAlloc		= NULL;
	m.mapStartCommit	= NULL;
	m.mapStart			= NULL;
	m.mapEnd			= NULL;
	m.mapEndCommit		= NULL;
	m.mapEndAlloc		= NULL;
	mTails.startIndex	= 0;
	mTails.endIndex		= kBlockSize - 1;
}

template<class T, class A>
vdfastdeque<T,A>::~vdfastdeque() {
	while(m.mapStartCommit != m.mapEndCommit) {
		mTails.deallocate(*m.mapStartCommit++, 1);
	}

	if (m.mapStartAlloc)
		m.deallocate(m.mapStartAlloc, m.mapEndAlloc - m.mapStartAlloc);
}

template<class T, class A>
bool vdfastdeque<T,A>::empty() const {
	return size() == 0;
}

template<class T, class A>
typename vdfastdeque<T,A>::size_type vdfastdeque<T,A>::size() const {
	if (m.mapEnd == m.mapStart)
		return 0;

	return kBlockSize * ((m.mapEnd - m.mapStart) - 1) + (mTails.endIndex + 1) - mTails.startIndex;
}

template<class T, class A>
typename vdfastdeque<T,A>::reference vdfastdeque<T,A>::front() {
	VDASSERT(m.mapStart != m.mapEnd);
	return (*m.mapStart)->data[mTails.startIndex];
}

template<class T, class A>
typename vdfastdeque<T,A>::const_reference vdfastdeque<T,A>::front() const {
	VDASSERT(m.mapStart != m.mapEnd);
	return (*m.mapStart)->data[mTails.startIndex];
}

template<class T, class A>
typename vdfastdeque<T,A>::reference vdfastdeque<T,A>::back() {
	VDASSERT(m.mapStart != m.mapEnd);
	return m.mapEnd[-1]->data[mTails.endIndex];
}

template<class T, class A>
typename vdfastdeque<T,A>::const_reference vdfastdeque<T,A>::back() const {
	VDASSERT(m.mapStart != m.mapEnd);
	return m.mapEnd[-1]->data[mTails.endIndex];
}

template<class T, class A>
typename vdfastdeque<T,A>::iterator vdfastdeque<T,A>::begin() {
	return iterator(m.mapStart, mTails.startIndex);
}

template<class T, class A>
typename vdfastdeque<T,A>::const_iterator vdfastdeque<T,A>::begin() const {
	return const_iterator(m.mapStart, mTails.startIndex);
}

template<class T, class A>
typename vdfastdeque<T,A>::iterator vdfastdeque<T,A>::end() {
	if (mTails.endIndex == kBlockSize - 1)
		return iterator(m.mapEnd, 0);
	else
		return iterator(m.mapEnd - 1, mTails.endIndex + 1);
}

template<class T, class A>
typename vdfastdeque<T,A>::const_iterator vdfastdeque<T,A>::end() const {
	if (mTails.endIndex == kBlockSize - 1)
		return const_iterator(m.mapEnd, 0);
	else
		return const_iterator(m.mapEnd - 1, mTails.endIndex + 1);
}

template<class T, class A>
typename vdfastdeque<T,A>::reference vdfastdeque<T,A>::operator[](size_type n) {
	n += mTails.startIndex;
	return m.mapStart[n >> kBlockSizeBits]->data[n & (kBlockSize - 1)];
}

template<class T, class A>
typename vdfastdeque<T,A>::const_reference vdfastdeque<T,A>::operator[](size_type n) const {
	n += mTails.startIndex;
	return m.mapStart[n >> kBlockSizeBits]->data[n & (kBlockSize - 1)];
}

template<class T, class A>
void vdfastdeque<T,A>::clear() {
	m.mapEnd			= m.mapStart;
	mTails.startIndex	= 0;
	mTails.endIndex		= kBlockSize - 1;
}

template<class T, class A>
typename vdfastdeque<T,A>::reference vdfastdeque<T,A>::push_back() {
	if (mTails.endIndex >= kBlockSize - 1) {
		push_back_extend();

		mTails.endIndex = -1;
	}

	++mTails.endIndex;

	VDASSERT(m.mapEnd[-1]);
	reference r = m.mapEnd[-1]->data[mTails.endIndex];
	return r;
}

template<class T, class A>
void vdfastdeque<T,A>::push_back(const_reference x) {
	const T x2(x);
	push_back() = x2;
}

template<class T, class A>
void vdfastdeque<T,A>::pop_front() {
	if (++mTails.startIndex >= kBlockSize) {
		VDASSERT(m.mapEnd != m.mapStart);
		mTails.startIndex = 0;
		++m.mapStart;
	}
}

template<class T, class A>
void vdfastdeque<T,A>::pop_back() {
	if (--mTails.endIndex < 0) {
		VDASSERT(m.mapEnd != m.mapStart);
		mTails.endIndex = kBlockSize - 1;
		--m.mapEnd;
	}
}

template<class T, class A>
void vdfastdeque<T,A>::swap(vdfastdeque& x) {
	std::swap(m.mapStartAlloc, x.m.mapStartAlloc);
	std::swap(m.mapStartCommit, x.m.mapStartCommit);
	std::swap(m.mapStart, x.m.mapStart);
	std::swap(m.mapEnd, x.m.mapEnd);
	std::swap(m.mapEndCommit, x.m.mapEndCommit);
	std::swap(m.mapEndAlloc, x.m.mapEndAlloc);
	std::swap(mTails.startIndex, x.mTails.startIndex);
	std::swap(mTails.endIndex, x.mTails.endIndex);
}

/////////////////////////////////

template<class T, class A>
void vdfastdeque<T,A>::push_back_extend() {
	validate();

	// check if we need to extend the map itself
	if (m.mapEnd == m.mapEndAlloc) {
		// can we just shift the map?
		size_type currentMapSize = m.mapEndAlloc - m.mapStartAlloc;
		size_type freeAtStart = m.mapStartCommit - m.mapStartAlloc;

		if (freeAtStart >= 2 && (freeAtStart + freeAtStart) >= currentMapSize) {
			size_type shiftDistance = freeAtStart >> 1;

			VDASSERT(!m.mapStartAlloc[0]);
			memmove(m.mapStartAlloc, m.mapStartAlloc + shiftDistance, sizeof(Block *) * (currentMapSize - shiftDistance));
			memset(m.mapStartAlloc + (currentMapSize - shiftDistance), 0, shiftDistance * sizeof(Block *));

			// relocate pointers
			m.mapEndCommit		-= shiftDistance;
			m.mapEnd			-= shiftDistance;
			m.mapStart			-= shiftDistance;
			m.mapStartCommit	-= shiftDistance;
			validate();
		} else {
			size_type newMapSize = currentMapSize*2+1;

			Block **newMap = m.allocate(newMapSize);

			memcpy(newMap, m.mapStartAlloc, currentMapSize * sizeof(Block *));
			memset(newMap + currentMapSize, 0, (newMapSize - currentMapSize) * sizeof(Block *));

			// relocate pointers
			m.mapEndAlloc		= newMap + newMapSize;
			m.mapEndCommit		= newMap + (m.mapEndCommit		- m.mapStartAlloc);
			m.mapEnd			= newMap + (m.mapEnd			- m.mapStartAlloc);
			m.mapStart			= newMap + (m.mapStart			- m.mapStartAlloc);
			m.mapStartCommit	= newMap + (m.mapStartCommit	- m.mapStartAlloc);

			m.deallocate(m.mapStartAlloc, currentMapSize);
			m.mapStartAlloc		= newMap;
			validate();
		}
	}

	VDASSERT(m.mapEnd != m.mapEndAlloc);

	// check if we already have a block we can use
	if (*m.mapEnd) {
		++m.mapEnd;
		validate();
		return;
	}

	// check if we can steal a block from the beginning
	if (m.mapStartCommit != m.mapStart) {
		VDASSERT(*m.mapStartCommit);
		if (m.mapStartCommit != m.mapEnd) {
			*m.mapEnd = *m.mapStartCommit;
			*m.mapStartCommit = NULL;
			++m.mapStartCommit;
		}
		++m.mapEnd;
		m.mapEndCommit = m.mapEnd;
		validate();
		return;
	}

	// allocate a new block
	*m.mapEnd = mTails.allocate(1);
	++m.mapEnd;
	m.mapEndCommit = m.mapEnd;
	validate();
}

template<class T, class A>
void vdfastdeque<T,A>::validate() {
	VDASSERT(m.mapStartAlloc <= m.mapStartCommit);
	VDASSERT(m.mapStartCommit <= m.mapStart);
	VDASSERT(m.mapStart <= m.mapEnd);
	VDASSERT(m.mapEnd <= m.mapEndCommit);
	VDASSERT(m.mapEndCommit <= m.mapEndAlloc);

	VDASSERT(m.mapStartAlloc == m.mapStartCommit || !*m.mapStartAlloc);
	VDASSERT(m.mapStartCommit == m.mapEndCommit || m.mapStartCommit[0]);
	VDASSERT(m.mapStart == m.mapEnd || (m.mapStart[0] && m.mapEnd[-1]));
	VDASSERT(m.mapEndCommit == m.mapEndAlloc || !m.mapEndCommit[0]);
}

#include <vd2/system/vdstl_vector.h>
#include <vd2/system/vdstl_hash.h>
#include <vd2/system/vdstl_hashmap.h>

#endif
