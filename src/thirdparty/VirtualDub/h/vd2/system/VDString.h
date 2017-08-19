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

#ifndef f_VD2_SYSTEM_VDSTRING_H
#define f_VD2_SYSTEM_VDSTRING_H

#ifdef _MSC_VER
	#pragma once
#endif

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <functional>

#include <vd2/system/vdtypes.h>
#include <vd2/system/text.h>
#include <vd2/system/vdstl.h>

///////////////////////////////////////////////////////////////////////////

class VDStringSpanA {
public:
	typedef char					value_type;
	typedef uint32					size_type;
	typedef ptrdiff_t				difference_type;
	typedef value_type&				reference;
	typedef const value_type&		const_reference;
	typedef value_type *			pointer;
	typedef const value_type *		const_pointer;
	typedef pointer					iterator;
	typedef const_pointer			const_iterator;

	static const size_type npos = (size_type)-1;

	VDStringSpanA() 
		: mpBegin(const_cast<value_type *>(sNull))
		, mpEnd(const_cast<value_type *>(sNull))
	{
	}

	explicit VDStringSpanA(const value_type *s)
		: mpBegin(const_cast<value_type *>(s))
		, mpEnd(const_cast<value_type *>(s) + strlen(s))
	{
	}

	VDStringSpanA(const value_type *s, const value_type *t)
		: mpBegin(const_cast<value_type *>(s))
		, mpEnd(const_cast<value_type *>(t))
	{
	}

	// 21.3.2 iterators
	const_iterator		begin() const		{ return mpBegin; }
	const_iterator		end() const			{ return mpEnd; }

	// 21.3.3 capacity
	size_type			size() const		{ return (size_type)(mpEnd - mpBegin); }
	size_type			length() const		{ return (size_type)(mpEnd - mpBegin); }
	bool				empty() const		{ return mpBegin == mpEnd; }

	// 21.3.4 element access
	const_reference		operator[](size_type pos) const	{ VDASSERT(pos < (size_type)(mpEnd - mpBegin)); return mpBegin[pos]; }
	const_reference		at(size_type pos) const			{ VDASSERT(pos < (size_type)(mpEnd - mpBegin)); return mpBegin[pos]; }

	const_reference		front() const		{ VDASSERT(mpBegin != mpEnd); return *mpBegin; }
	const_reference		back() const		{ VDASSERT(mpBegin != mpEnd); return mpEnd[-1]; }

	// 21.3.6 string operations
	const_pointer		data() const		{ return mpBegin; }

	size_type copy(value_type *dst, size_type n, size_type pos = 0) const {
		size_type len = (size_type)(mpEnd - mpBegin);
		VDASSERT(pos <= len);

		len -= pos;
		if (n > len)
			n = len;

		memcpy(dst, mpBegin + pos, n*sizeof(value_type));
		return n;
	}

	size_type find(value_type c, size_type pos = 0) const {
		VDASSERT(pos <= (size_type)(mpEnd - mpBegin));
		const void *p = memchr(mpBegin + pos, c, mpEnd - (mpBegin + pos));

		return p ? (size_type)((const value_type *)p - mpBegin) : npos;
	}

	size_type find_last_of(value_type c) const {
		const value_type *s = mpEnd;

		while(s != mpBegin) {
			--s;

			if (*s == c)
				return (size_type)(s - mpBegin);
		}

		return npos;
	}

	int compare(const VDStringSpanA& s) const {
		size_type l1 = (size_type)(mpEnd - mpBegin);
		size_type l2 = (size_type)(s.mpEnd - s.mpBegin);
		size_type lm = l1 < l2 ? l1 : l2;

		int r = memcmp(mpBegin, s.mpBegin, lm);

		if (!r && l1 != l2)
			r = (int)mpBegin[lm] - (int)s.mpBegin[lm];

		return r;
	}

	int comparei(const char *s) const {
		return comparei(VDStringSpanA(s));
	}

	int comparei(const VDStringSpanA& s) const {
		size_type l1 = (size_type)(mpEnd - mpBegin);
		size_type l2 = (size_type)(s.mpEnd - s.mpBegin);
		size_type lm = l1 < l2 ? l1 : l2;

		const char *p = mpBegin;
		const char *q = s.mpBegin;

		while(lm--) {
			const unsigned char c = tolower((unsigned char)*p++);
			const unsigned char d = tolower((unsigned char)*q++);

			if (c != d)
				return (int)c - (int)d;
		}

		return (int)l1 - (int)l2;
	}

	const VDStringSpanA trim(const value_type *s) const {
		bool flags[256]={false};

		while(value_type c = *s++)
			flags[(unsigned char)c] = true;

		const value_type *p = mpBegin;
		const value_type *q = mpEnd;

		while(p != q && flags[*p])
			++p;

		while(p != q && flags[q[-1]])
			--q;

		return VDStringSpanA(p, q);
	}

	const VDStringSpanA subspan(size_type pos = 0, size_type n = npos) const {
		
		size_type len = (size_type)(mpEnd - mpBegin);
		VDASSERT(pos <= len);

		len -= pos;
		if (n > len)
			n = len;

		value_type *p = mpBegin + pos;
		return VDStringSpanA(p, p+n);
	}

protected:
	friend bool operator==(const VDStringSpanA& x, const VDStringSpanA& y);
	friend bool operator==(const VDStringSpanA& x, const char *y);

	value_type *mpBegin;
	value_type *mpEnd;

	static const value_type sNull[1];
};

inline bool operator==(const VDStringSpanA& x, const VDStringSpanA& y) { VDStringSpanA::size_type len = (VDStringSpanA::size_type)(x.mpEnd - x.mpBegin); return len == (VDStringSpanA::size_type)(y.mpEnd - y.mpBegin) && !memcmp(x.mpBegin, y.mpBegin, len*sizeof(char)); }
inline bool operator==(const VDStringSpanA& x, const char *y) { size_t len = strlen(y); return len == (size_t)(x.mpEnd - x.mpBegin) && !memcmp(x.mpBegin, y, len*sizeof(char)); }
inline bool operator==(const char *x, const VDStringSpanA& y) { return y == x; }

inline bool operator!=(const VDStringSpanA& x, const VDStringSpanA& y) { return !(x == y); }
inline bool operator!=(const VDStringSpanA& x, const char *y) { return !(x == y); }
inline bool operator!=(const char *x, const VDStringSpanA& y) { return !(y == x); }

inline bool operator<(const VDStringSpanA& x, const VDStringSpanA& y) {
	return x.compare(y) < 0;
}

inline bool operator>(const VDStringSpanA& x, const VDStringSpanA& y) {
	return x.compare(y) > 0;
}

inline bool operator<=(const VDStringSpanA& x, const VDStringSpanA& y) {
	return x.compare(y) <= 0;
}

inline bool operator>=(const VDStringSpanA& x, const VDStringSpanA& y) {
	return x.compare(y) >= 0;
}

class VDStringRefA : public VDStringSpanA {
public:
	typedef VDStringRefA this_type;

	VDStringRefA()  {
	}

	explicit VDStringRefA(const value_type *s)
		: VDStringSpanA(s)
	{
	}

	explicit VDStringRefA(const VDStringSpanA& s)
		: VDStringSpanA(s)
	{
	}

	VDStringRefA(const value_type *s, const value_type *t)
		: VDStringSpanA(s, t)
	{
	}

	this_type& operator=(const value_type *s) {
		assign(s);
		return *this;
	}

	this_type& operator=(const VDStringSpanA& str) {
		assign(str);
		return *this;
	}

	void assign(const value_type *s) {
		static_cast<VDStringSpanA&>(*this) = VDStringSpanA(s);
	}

	void assign(const value_type *s, const value_type *t) {
		static_cast<VDStringSpanA&>(*this) = VDStringSpanA(s, t);
	}

	void assign(const VDStringSpanA& s) {
		static_cast<VDStringSpanA&>(*this) = s;
	}

	void clear() {
		mpBegin = mpEnd = NULL;
	}

	bool split(value_type c, VDStringRefA& token) {
		size_type pos = find(c);

		if (pos == npos)
			return false;

		token = subspan(0, pos);
		mpBegin += pos+1;
		return true;
	}
};

class VDStringA : public VDStringSpanA {
public:
	typedef VDStringA				this_type;

	// 21.3.1 construct/copy/destroy

	VDStringA()
		: mpEOS(const_cast<value_type *>(sNull))
	{
	}

	VDStringA(const VDStringSpanA& x)
		: mpEOS(const_cast<value_type *>(sNull))
	{
		assign(x.begin(), x.end());
	}

	VDStringA(const this_type& x)
		: mpEOS(const_cast<value_type *>(sNull))
	{
		assign(x);
	}

	explicit VDStringA(const value_type *s)
		: mpEOS(const_cast<value_type *>(sNull))
	{
		assign(s);
	}

	explicit VDStringA(size_type n)
		: mpEOS(const_cast<value_type *>(sNull))
	{
		resize(n);
	}

	VDStringA(const value_type *s, size_type n)
		: mpEOS(const_cast<value_type *>(sNull))
	{
		assign(s, n);
	}

	VDStringA(const value_type *s, const value_type *t)
		: mpEOS(const_cast<value_type *>(sNull))
	{
		assign(s, t);
	}

	~VDStringA() {
		if (mpBegin != sNull)
			delete[] mpBegin;
	}

	this_type& operator=(const value_type *s) {
		assign(s);
		return *this;
	}

	this_type& operator=(const this_type& str) {
		assign(str);
		return *this;
	}

	this_type& operator=(const VDStringSpanA& str) {
		assign(str);
		return *this;
	}

	// 21.3.2 iterators
	using VDStringSpanA::begin;
	using VDStringSpanA::end;

	iterator			begin()				{ return mpBegin; }
	iterator			end()				{ return mpEnd; }

	// 21.3.3 capacity (COMPLETE)
	void resize(size_type n) {
		size_type current = (size_type)(mpEnd - mpBegin);

		if (n < current) {
			mpEnd = mpBegin + n;
			mpEnd[0] = 0;
		} else if (n > current)
			resize_slow(n, current);
	}

	void resize(size_type n, value_type v) {
		size_type current = (size_type)(mpEnd - mpBegin);

		if (n < current) {
			mpEnd = mpBegin + n;
			mpEnd[0] = 0;
		} else if (n > current)
			resize_slow(n, current, v);
	}

	size_type			capacity() const	{ return (size_type)(mpEOS - mpBegin); }

	void reserve(size_type n) {
		size_type current = (size_type)(mpEOS - mpBegin);

		if (n > current)
			reserve_slow(n, current);
	}

	void clear() {
		if (mpEnd != mpBegin) {
			mpEnd = mpBegin;
			mpEnd[0] = 0;
		}
	}

	// 21.3.4 element access
	using VDStringSpanA::operator[];
	using VDStringSpanA::at;
	using VDStringSpanA::front;
	using VDStringSpanA::back;

	reference			operator[](size_type pos)		{ VDASSERT(pos < (size_type)(mpEnd - mpBegin)); return mpBegin[pos]; }
	reference			at(size_type pos)				{ VDASSERT(pos < (size_type)(mpEnd - mpBegin)); return mpBegin[pos]; }
	reference			front()				{ VDASSERT(mpBegin != mpEnd); return *mpBegin; }
	reference			back()				{ VDASSERT(mpBegin != mpEnd); return mpEnd[-1]; }

	// 21.3.5 modifiers
	this_type& operator+=(const this_type& str) {
		return append(str.mpBegin, str.mpEnd);
	}

	this_type& operator+=(const value_type *s) {
		return append(s, s+strlen(s));
	}

	this_type& operator+=(value_type c) {
		if (mpEnd == mpEOS)
			push_back_extend();

		*mpEnd++ = c;
		*mpEnd = 0;
		return *this;
	}

	this_type& append(const this_type& str) {
		return append(str.mpBegin, str.mpEnd);
	}

	this_type& append(const this_type& str, size_type pos, size_type n) {
		size_type len = (size_type)(str.mpEnd - str.mpBegin);
		VDASSERT(pos <= len);

		len -= pos;
		if (n > len)
			n = len;

		return append(str.mpBegin + pos, str.mpBegin + pos + n);
	}

	this_type& append(const value_type *s, size_type n) {
		return append(s, s+n);
	}

	this_type& append(const value_type *s) {
		return append(s, s+strlen(s));
	}

	this_type& append(const value_type *s, const value_type *t) {
		if (s != t) {
			size_type current_size = (size_type)(mpEnd - mpBegin);
			size_type current_capacity = (size_type)(mpEOS - mpBegin);
			size_type n = (size_type)(t - s);

			if (current_capacity - current_size < n)
				reserve_amortized_slow(n, current_size, current_capacity);

			memcpy(mpBegin + current_size, s, n*sizeof(value_type));
			mpEnd += n;
			*mpEnd = 0;
		}
		return *this;
	}

	void push_back(const value_type c) {
		if (mpEnd == mpEOS)
			push_back_extend();

		*mpEnd++ = c;
		*mpEnd = 0;
	}

	void pop_back() {
		--mpEnd;
		*mpEnd = 0;
	}

	this_type& assign(const VDStringSpanA& str) {
		return assign(str.begin(), str.end());
	}

	this_type& assign(const this_type& str) {
		return assign(str.mpBegin, str.mpEnd);
	}

	this_type& assign(const this_type& str, size_type pos, size_type n) {
		size_type len = (size_type)(str.mpEnd - str.mpBegin);
		VDASSERT(pos <= len);

		len -= pos;
		if (n > len)
			n = len;

		return assign(str.mpBegin + pos, str.mpBegin + pos + n);
	}

	this_type& assign(const value_type *s, size_type n) {
		return assign(s, s+n);
	}

	this_type& assign(const value_type *s) {
		return assign(s, s+strlen(s));
	}

	this_type& assign(size_type n, value_type c) {
		size_type current_capacity = (size_type)(mpEOS - mpBegin);

		if (current_capacity < n)
			reserve_slow(n, current_capacity);

		if (mpBegin != sNull) {
			mpEnd = mpBegin;
			while(n--)
				*mpEnd++ = c;
		}

		return *this;
	}

	this_type& assign(const value_type *s, const value_type *t) {
		size_type current_capacity = (size_type)(mpEOS - mpBegin);
		size_type n = (size_type)(t - s);

		if (current_capacity < n)
			reserve_slow(n, current_capacity);

		if (mpBegin != sNull) {
			memcpy(mpBegin, s, sizeof(value_type)*n);
			mpEnd = mpBegin + n;
			*mpEnd = 0;
		}

		return *this;
	}

	this_type& insert(iterator it, value_type c) {
		if (mpEnd == mpEOS) {
			size_type pos = (size_type)(it - mpBegin);
			push_back_extend();
			it = mpBegin + pos;
		}

		memmove(it + 1, it, (mpEnd - it + 1)*sizeof(value_type));
		*it = c;
		++mpEnd;
		return *this;
	}

	this_type& erase(size_type pos = 0, size_type n = npos) {
		size_type len = (size_type)(mpEnd - mpBegin);

		VDASSERT(pos <= len);
		len -= pos;
		if (n > len)
			n = len;

		if (n) {
			size_type pos2 = pos + n;
			memmove(mpBegin + pos, mpBegin + pos2, (len + 1 - n)*sizeof(value_type));
			mpEnd -= n;
		}

		return *this;
	}

	iterator erase(iterator x) {
		VDASSERT(x != mpEnd);

		memmove(x, x+1, (mpEnd - x)*sizeof(value_type));
		--mpEnd;
		return x;
	}

	iterator erase(iterator first, iterator last) {
		VDASSERT(last >= first);

		memmove(first, last, ((mpEnd - last) + 1)*sizeof(value_type));
		mpEnd -= (last - first);
		return first;
	}

	this_type& replace(size_type pos, size_type n1, const value_type *s, size_type n2) {
		size_type len = (size_type)(mpEnd - mpBegin);

		VDASSERT(pos <= len);
		size_type limit = len - pos;
		if (n1 > limit)
			n1 = limit;

		size_type len2 = len - n1 + n2;
		size_type current_capacity = (size_type)(mpEOS - mpBegin);

		if (current_capacity < len2)
			reserve_slow(len2, current_capacity);

		memmove(mpBegin + pos + n2, mpBegin + pos + n1, (limit - n1 + 1) * sizeof(value_type));
		memcpy(mpBegin + pos, s, n2*sizeof(value_type));
		mpEnd = mpBegin + len2;
		return *this;
	}

	void swap(this_type& x) {
		value_type *p;

		p = mpBegin;	mpBegin = x.mpBegin;	x.mpBegin = p;
		p = mpEnd;		mpEnd = x.mpEnd;		x.mpEnd = p;
		p = mpEOS;		mpEOS = x.mpEOS;		x.mpEOS = p;
	}

	// 21.3.6 string operations
	const_pointer		c_str() const		{ return mpBegin; }

	this_type& sprintf(const value_type *format, ...);
	this_type& append_sprintf(const value_type *format, ...);
	this_type& append_vsprintf(const value_type *format, va_list val);

	void move_from(VDStringA& src);

protected:
	void push_back_extend();
	void resize_slow(size_type n, size_type current_size);
	void resize_slow(size_type n, size_type current_size, value_type c);
	void reserve_slow(size_type n, size_type current_capacity);
	void reserve_amortized_slow(size_type n, size_type current_size, size_type current_capacity);

	char *mpEOS;
};

///////////////////////////////////////////////////////////////////////////

inline VDStringA operator+(const VDStringA& str, const VDStringA& s) {
	VDStringA result;
	result.reserve((VDStringA::size_type)(str.size() + s.size()));
	result.assign(str);
	result.append(s);
	return result;
}

inline VDStringA operator+(const VDStringA& str, const char *s) {
	VDStringA result;
	result.reserve((VDStringA::size_type)(str.size() + strlen(s)));
	result.assign(str);
	result.append(s);
	return result;
}

inline VDStringA operator+(const VDStringA& str, char c) {
	VDStringA result;
	result.reserve(str.size() + 1);
	result.assign(str);
	result += c;
	return result;
}

// Start patch MPC-HC
/*
namespace std {
	template<>
	struct less<VDStringA> : binary_function<VDStringA, VDStringA, bool> {
		bool operator()(const VDStringA& x, const VDStringA& y) const {
			return x.compare(y) < 0;
		}
	};
}
*/
// End patch MPC-HC

///////////////////////////////////////////////////////////////////////////

class VDStringSpanW {
public:
	typedef wchar_t					value_type;
	typedef uint32					size_type;
	typedef ptrdiff_t				difference_type;
	typedef value_type&				reference;
	typedef const value_type&		const_reference;
	typedef value_type *			pointer;
	typedef const value_type *		const_pointer;
	typedef pointer					iterator;
	typedef const_pointer			const_iterator;

	static const size_type npos = (size_type)-1;

	VDStringSpanW() 
		: mpBegin(const_cast<value_type *>(sNull))
		, mpEnd(const_cast<value_type *>(sNull))
	{
	}

	explicit VDStringSpanW(const value_type *s)
		: mpBegin(const_cast<value_type *>(s))
		, mpEnd(const_cast<value_type *>(s) + wcslen(s))
	{
	}

	VDStringSpanW(const value_type *s, const value_type *t)
		: mpBegin(const_cast<value_type *>(s))
		, mpEnd(const_cast<value_type *>(t))
	{
	}

	// 21.3.2 iterators
	const_iterator		begin() const		{ return mpBegin; }
	const_iterator		end() const			{ return mpEnd; }

	// 21.3.3 capacity
	size_type			size() const		{ return (size_type)(mpEnd - mpBegin); }
	size_type			length() const		{ return (size_type)(mpEnd - mpBegin); }
	bool				empty() const		{ return mpBegin == mpEnd; }

	// 21.3.4 element access
	const_reference		operator[](size_type pos) const	{ VDASSERT(pos < (size_type)(mpEnd - mpBegin)); return mpBegin[pos]; }
	const_reference		at(size_type pos) const			{ VDASSERT(pos < (size_type)(mpEnd - mpBegin)); return mpBegin[pos]; }

	const_reference		front() const		{ VDASSERT(mpBegin != mpEnd); return *mpBegin; }
	const_reference		back() const		{ VDASSERT(mpBegin != mpEnd); return mpEnd[-1]; }

	// 21.3.6 string operations
	const_pointer		data() const		{ return mpBegin; }

	size_type copy(value_type *dst, size_type n, size_type pos = 0) const {
		size_type len = (size_type)(mpEnd - mpBegin);
		VDASSERT(pos <= len);

		len -= pos;
		if (n > len)
			n = len;

		memcpy(dst, mpBegin + pos, n*sizeof(value_type));
		return n;
	}

	size_type find(value_type c, size_type pos = 0) const {
		VDASSERT(pos <= (size_type)(mpEnd - mpBegin));
		const void *p = wmemchr(mpBegin + pos, c, mpEnd - (mpBegin + pos));

		return p ? (size_type)((const value_type *)p - mpBegin) : npos;
	}

	int compare(const VDStringSpanW& s) const {
		size_type l1 = (size_type)(mpEnd - mpBegin);
		size_type l2 = (size_type)(s.mpEnd - s.mpBegin);
		size_type lm = l1 < l2 ? l1 : l2;

		for(size_type i = 0; i < lm; ++i) {
			if (mpBegin[i] != s.mpBegin[i])
				return mpBegin[i] < s.mpBegin[i] ? -1 : +1;
		}

		if (l1 == l2)
			return 0;

		return l1 < l2 ? -1 : +1;
	}

	int comparei(const wchar_t *s) const {
		return comparei(VDStringSpanW(s));
	}

	int comparei(const VDStringSpanW& s) const {
		size_type l1 = (size_type)(mpEnd - mpBegin);
		size_type l2 = (size_type)(s.mpEnd - s.mpBegin);
		size_type lm = l1 < l2 ? l1 : l2;

		for(size_type i = 0; i < lm; ++i) {
			wint_t c = towlower(mpBegin[i]);
			wint_t d = towlower(s.mpBegin[i]);

			if (c != d)
				return c < d ? -1 : +1;
		}

		if (l1 == l2)
			return 0;

		return l1 < l2 ? -1 : +1;
	}

	// extensions
	const VDStringSpanW subspan(size_type pos, size_type n) const {
		size_type len = (size_type)(mpEnd - mpBegin);
		VDASSERT(pos <= len);

		len -= pos;
		if (n > len)
			n = len;

		value_type *p = mpBegin + pos;
		return VDStringSpanW(p, p+n);
	}

protected:
	friend class VDStringW;
	friend bool operator==(const VDStringSpanW& x, const VDStringSpanW& y);
	friend bool operator==(const VDStringSpanW& x, const wchar_t *y);

	value_type *mpBegin;
	value_type *mpEnd;

	static const value_type sNull[1];
};

inline bool operator==(const VDStringSpanW& x, const VDStringSpanW& y) { VDStringA::size_type len = (VDStringSpanW::size_type)(x.mpEnd - x.mpBegin); return len == (VDStringSpanW::size_type)(y.mpEnd - y.mpBegin) && !memcmp(x.mpBegin, y.mpBegin, len*sizeof(wchar_t)); }
inline bool operator==(const VDStringSpanW& x, const wchar_t *y) { size_t len = wcslen(y); return len == (size_t)(x.mpEnd - x.mpBegin) && !memcmp(x.mpBegin, y, len*sizeof(wchar_t)); }
inline bool operator==(const wchar_t *x, const VDStringSpanW& y) { return y == x; }

inline bool operator!=(const VDStringSpanW& x, const VDStringSpanW& y) { return !(x == y); }
inline bool operator!=(const VDStringSpanW& x, const wchar_t *y) { return !(x == y); }
inline bool operator!=(const wchar_t *x, const VDStringSpanW& y) { return !(y == x); }

inline bool operator<(const VDStringSpanW& x, const VDStringSpanW& y) {
	return x.compare(y) < 0;
}

inline bool operator>(const VDStringSpanW& x, const VDStringSpanW& y) {
	return x.compare(y) > 0;
}

inline bool operator<=(const VDStringSpanW& x, const VDStringSpanW& y) {
	return x.compare(y) <= 0;
}

inline bool operator>=(const VDStringSpanW& x, const VDStringSpanW& y) {
	return x.compare(y) >= 0;
}

class VDStringRefW : public VDStringSpanW {
public:
	typedef VDStringRefW this_type;

	VDStringRefW()  {
	}

	explicit VDStringRefW(const value_type *s)
		: VDStringSpanW(s)
	{
	}

	explicit VDStringRefW(const VDStringSpanW& s)
		: VDStringSpanW(s)
	{
	}

	VDStringRefW(const value_type *s, const value_type *t)
		: VDStringSpanW(s, t)
	{
	}

	this_type& operator=(const value_type *s) {
		assign(s);
		return *this;
	}

	this_type& operator=(const VDStringSpanW& str) {
		assign(str);
		return *this;
	}

	void assign(const value_type *s) {
		static_cast<VDStringSpanW&>(*this) = VDStringSpanW(s);
	}

	void assign(const value_type *s, const value_type *t) {
		static_cast<VDStringSpanW&>(*this) = VDStringSpanW(s, t);
	}

	void assign(const VDStringSpanW& s) {
		static_cast<VDStringSpanW&>(*this) = s;
	}

	void clear() {
		mpBegin = mpEnd = NULL;
	}

	bool split(value_type c, VDStringRefW& token) {
		size_type pos = find(c);

		if (pos == npos)
			return false;

		token = subspan(0, pos);
		mpBegin += pos+1;
		return true;
	}
};

class VDStringW : public VDStringSpanW {
public:
	typedef VDStringW				this_type;

	// 21.3.1 construct/copy/destroy

	VDStringW()
		: mpEOS(const_cast<value_type *>(sNull))
	{
	}

	VDStringW(const VDStringSpanW& x)
		: mpEOS(const_cast<value_type *>(sNull))
	{
		assign(x.begin(), x.end());
	}

	VDStringW(const this_type& x)
		: mpEOS(const_cast<value_type *>(sNull))
	{
		assign(x);
	}

	explicit VDStringW(const value_type *s)
		: mpEOS(const_cast<value_type *>(sNull))
	{
		assign(s);
	}

	explicit VDStringW(size_type n)
		: mpEOS(const_cast<value_type *>(sNull))
	{
		resize(n);
	}

	VDStringW(const value_type *s, size_type n)
		: mpEOS(const_cast<value_type *>(sNull))
	{
		assign(s, n);
	}

	VDStringW(const value_type *s, const value_type *t)
		: mpEOS(const_cast<value_type *>(sNull))
	{
		assign(s, t);
	}

	~VDStringW() {
		if (mpBegin != sNull)
			delete[] mpBegin;
	}

	this_type& operator=(const wchar_t *s) {
		assign(s);
		return *this;
	}

	this_type& operator=(const this_type& str) {
		assign(str);
		return *this;
	}

	// 21.3.2 iterators
	using VDStringSpanW::begin;
	using VDStringSpanW::end;
	iterator			begin()				{ return mpBegin; }
	iterator			end()				{ return mpEnd; }

	// 21.3.3 capacity (COMPLETE)
	void resize(size_type n) {
		size_type current = (size_type)(mpEnd - mpBegin);

		if (n < current) {
			mpEnd = mpBegin + n;
			mpEnd[0] = 0;
		} else if (n > current)
			resize_slow(n, current);
	}

	void resize(size_type n, value_type v) {
		size_type current = (size_type)(mpEnd - mpBegin);

		if (n < current) {
			mpEnd = mpBegin + n;
			mpEnd[0] = 0;
		} else if (n > current)
			resize_slow(n, current);
		wmemset(mpBegin, v, n);
	}

	size_type			capacity() const	{ return (size_type)(mpEOS - mpBegin); }

	void reserve(size_type n) {
		size_type current = (size_type)(mpEOS - mpBegin);

		if (n > current)
			reserve_slow(n, current);
	}

	void clear() {
		if (mpEnd != mpBegin) {
			mpEnd = mpBegin;
			mpEnd[0] = 0;
		}
	}

	// 21.3.4 element access
	using VDStringSpanW::operator[];
	using VDStringSpanW::at;
	using VDStringSpanW::front;
	using VDStringSpanW::back;
	reference			operator[](size_type pos)		{ VDASSERT(pos < (size_type)(mpEnd - mpBegin)); return mpBegin[pos]; }
	reference			at(size_type pos)				{ VDASSERT(pos < (size_type)(mpEnd - mpBegin)); return mpBegin[pos]; }
	reference			front()				{ VDASSERT(mpBegin != mpEnd); return *mpBegin; }
	reference			back()				{ VDASSERT(mpBegin != mpEnd); return mpEnd[-1]; }

	// 21.3.5 modifiers
	this_type& operator+=(const this_type& str) {
		return append(str.mpBegin, str.mpEnd);
	}

	this_type& operator+=(const value_type *s) {
		return append(s, s+wcslen(s));
	}

	this_type& operator+=(value_type c) {
		if (mpEnd == mpEOS)
			push_back_extend();

		*mpEnd++ = c;
		*mpEnd = 0;
		return *this;
	}

	this_type& append(const this_type& str) {
		return append(str.mpBegin, str.mpEnd);
	}

	this_type& append(const this_type& str, size_type pos, size_type n) {
		size_type len = (size_type)(str.mpEnd - str.mpBegin);
		VDASSERT(pos <= len);

		len -= pos;
		if (n > len)
			n = len;

		return append(str.mpBegin + pos, str.mpBegin + pos + n);
	}

	this_type& append(const value_type *s, size_type n) {
		return append(s, s+n);
	}

	this_type& append(const value_type *s) {
		return append(s, s+wcslen(s));
	}

	this_type& append(const value_type *s, const value_type *t) {
		if (s != t) {
			size_type current_size = (size_type)(mpEnd - mpBegin);
			size_type current_capacity = (size_type)(mpEOS - mpBegin);
			size_type n = (size_type)(t - s);

			if (current_capacity - current_size < n)
				reserve_amortized_slow(n, current_size, current_capacity);

			memcpy(mpBegin + current_size, s, n*sizeof(value_type));
			mpEnd += n;
			*mpEnd = 0;
		}
		return *this;
	}

	void push_back(const value_type c) {
		if (mpEnd == mpEOS)
			push_back_extend();

		*mpEnd++ = c;
		*mpEnd = 0;
	}

	void pop_back() {
		--mpEnd;
		*mpEnd = 0;
	}

	this_type& assign(const VDStringSpanW& str) {
		return assign(str.mpBegin, str.mpEnd);
	}

	this_type& assign(const VDStringSpanW& str, size_type pos, size_type n) {
		size_type len = (size_type)(str.mpEnd - str.mpBegin);
		VDASSERT(pos <= len);

		len -= pos;
		if (n > len)
			n = len;

		return assign(str.mpBegin + pos, str.mpBegin + pos + n);
	}

	this_type& assign(const value_type *s, size_type n) {
		return assign(s, s+n);
	}

	this_type& assign(const value_type *s) {
		return assign(s, s+wcslen(s));
	}

	this_type& assign(size_type n, value_type c) {
		size_type current_capacity = (size_type)(mpEOS - mpBegin);

		if (current_capacity < n)
			reserve_slow(n, current_capacity);

		if (mpBegin != sNull) {
			mpEnd = mpBegin;
			while(n--)
				*mpEnd++ = c;
		}

		return *this;
	}

	this_type& assign(const value_type *s, const value_type *t) {
		size_type current_capacity = (size_type)(mpEOS - mpBegin);
		size_type n = (size_type)(t - s);

		if (current_capacity < n)
			reserve_slow(n, current_capacity);

		if (mpBegin != sNull) {
			memcpy(mpBegin, s, sizeof(value_type)*n);
			mpEnd = mpBegin + n;
			*mpEnd = 0;
		}

		return *this;
	}

	this_type& insert(iterator it, value_type c) {
		if (mpEnd == mpEOS) {
			size_type pos = (size_type)(it - mpBegin);
			push_back_extend();
			it = mpBegin + pos;
		}

		memmove(it + 1, it, (mpEnd - it + 1)*sizeof(value_type));
		*it = c;
		++mpEnd;
		return *this;
	}

	this_type& erase(size_type pos = 0, size_type n = npos) {
		size_type len = (size_type)(mpEnd - mpBegin);

		VDASSERT(pos <= len);
		len -= pos;
		if (n > len)
			n = len;

		if (n) {
			size_type pos2 = pos + n;
			memmove(mpBegin + pos, mpBegin + pos2, (len + 1 - n)*sizeof(value_type));
			mpEnd -= n;
		}

		return *this;
	}

	iterator erase(iterator x) {
		VDASSERT(x != mpEnd);

		memmove(x, x+1, (mpEnd - x)*sizeof(value_type));
		--mpEnd;
		return x;
	}

	iterator erase(iterator first, iterator last) {
		VDASSERT(last >= first);

		memmove(first, last, ((mpEnd - last) + 1)*sizeof(value_type));
		mpEnd -= (last - first);
		return first;
	}

	this_type& replace(size_type pos, size_type n1, const value_type *s, size_type n2) {
		size_type len = (size_type)(mpEnd - mpBegin);

		VDASSERT(pos <= len);
		size_type limit = len - pos;
		if (n1 > limit)
			n1 = limit;

		size_type len2 = len - n1 + n2;
		size_type current_capacity = (size_type)(mpEOS - mpBegin);

		if (current_capacity < len2)
			reserve_slow(len2, current_capacity);

		memmove(mpBegin + pos + n2, mpBegin + pos + n1, (limit - n1 + 1) * sizeof(value_type));
		memcpy(mpBegin + pos, s, n2*sizeof(value_type));
		mpEnd = mpBegin + len2;
		return *this;
	}

	void swap(this_type& x) {
		std::swap(mpBegin, x.mpBegin);
		std::swap(mpEnd, x.mpEnd);
		std::swap(mpEOS, x.mpEOS);
	}

	void swap(this_type&& x) {
		std::swap(mpBegin, x.mpBegin);
		std::swap(mpEnd, x.mpEnd);
		std::swap(mpEOS, x.mpEOS);
	}

	// 21.3.6 string operations
	const_pointer		c_str() const		{ return mpBegin; }

	this_type& sprintf(const value_type *format, ...);
	this_type& append_sprintf(const value_type *format, ...);
	this_type& append_vsprintf(const value_type *format, va_list val);

	void move_from(VDStringW& src);

protected:
	void push_back_extend();
	void resize_slow(size_type n, size_type current_size);
	void reserve_slow(size_type n, size_type current_capacity);
	void reserve_amortized_slow(size_type n, size_type current_size, size_type current_capacity);

	value_type *mpEOS;
};

///////////////////////////////////////////////////////////////////////////

inline VDStringW operator+(const VDStringW& str, const VDStringW& s) {
	VDStringW result;
	result.reserve((VDStringA::size_type)(str.size() + s.size()));
	result.assign(str);
	result.append(s);
	return result;
}

inline VDStringW operator+(const VDStringW& str, const wchar_t *s) {
	VDStringW result;
	result.reserve((VDStringA::size_type)(str.size() + wcslen(s)));
	result.assign(str);
	result.append(s);
	return result;
}

inline VDStringW operator+(const VDStringW& str, wchar_t c) {
	VDStringW result;
	result.reserve(str.size() + 1);
	result.assign(str);
	result += c;
	return result;
}

///////////////////////////////////////////////////////////////////////////

typedef VDStringA				VDString;

template<> VDStringA *vdmove_forward(VDStringA *src1, VDStringA *src2, VDStringA *dst);
template<> VDStringW *vdmove_forward(VDStringW *src1, VDStringW *src2, VDStringW *dst);
template<> VDStringA *vdmove_backward(VDStringA *src1, VDStringA *src2, VDStringA *dst);
template<> VDStringW *vdmove_backward(VDStringW *src1, VDStringW *src2, VDStringW *dst);

VDMOVE_CAPABLE(VDStringA);
VDMOVE_CAPABLE(VDStringW);

#endif
