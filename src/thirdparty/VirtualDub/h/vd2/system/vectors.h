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

#ifndef f_VD2_SYSTEM_VECTORS_H
#define f_VD2_SYSTEM_VECTORS_H

#ifdef _MSC_VER
	#pragma once
#endif

#include <vd2/system/vdtypes.h>
#include <math.h>
#include <limits>

///////////////////////////////////////////////////////////////////////////

bool VDSolveLinearEquation(double *src, int n, ptrdiff_t stride_elements, double *b, double tolerance = 1e-5);

///////////////////////////////////////////////////////////////////////////

#include <vd2/system/vectors_float.h>
#include <vd2/system/vectors_int.h>

///////////////////////////////////////////////////////////////////////////

class vdfloat2x2 {
public:
	enum zero_type { zero };
	enum identity_type { identity };

	typedef float			value_type;
	typedef vdfloat2		vector_type;
	typedef vdfloat2c		vector_ctor_type;
	typedef vdfloat2x2		self_type;

	vdfloat2x2() {}
	vdfloat2x2(zero_type) { m[0] = m[1] = vector_ctor_type(0, 0); }
	vdfloat2x2(identity_type) {
		m[0] = vector_ctor_type(1, 0);
		m[1] = vector_ctor_type(0, 1);
	}

	vector_type& operator[](int k) { return m[k]; }
	const vector_type& operator[](int k) const { return m[k]; }

	self_type operator*(const self_type& v) const {
		self_type result;

#define DO(i,j) result.m[i].v[j] = m[i].v[0]*v.m[0].v[j] + m[i].v[1]*v.m[1].v[j]
		DO(0,0);
		DO(0,1);
		DO(1,0);
		DO(1,1);
#undef DO

		return result;
	}

	vector_type operator*(const vector_type& r) const {
		return vector_ctor_type(
				m[0].v[0]*r.v[0] + m[0].v[1]*r.v[1],
				m[1].v[0]*r.v[0] + m[1].v[1]*r.v[1]);
	}

	self_type transpose() const {
		self_type res;

		res.m[0].v[0] = m[0].v[0];
		res.m[0].v[1] = m[1].v[0];
		res.m[1].v[0] = m[0].v[1];
		res.m[1].v[1] = m[1].v[1];

		return res;
	}

	self_type adjunct() const {
		self_type res;
		
		res.m[0].set(m[1].v[1], -m[0].v[1]);
		res.m[1].set(-m[1].v[0], -m[0].v[0]);

		return res;
	}

	value_type det() const {
		return m[0].v[0]*m[1].v[1] - m[1].v[0]*m[0].v[1];
	}

	self_type operator~() const {
		return adjunct() / det();
	}

	self_type& operator*=(const value_type factor) {
		m[0] *= factor;
		m[1] *= factor;

		return *this;
	}

	self_type& operator/=(const value_type factor) {
		return operator*=(value_type(1)/factor);
	}

	self_type operator*(const value_type factor) const {
		return self_type(*this) *= factor;
	}

	self_type operator/(const value_type factor) const {
		return self_type(*this) /= factor;
	}

	vector_type m[2];
};

class vdfloat3x3 {
public:
	enum zero_type { zero };
	enum identity_type { identity };
	enum rotation_x_type { rotation_x };
	enum rotation_y_type { rotation_y };
	enum rotation_z_type { rotation_z };

	typedef float			value_type;
	typedef vdfloat3		vector_type;
	typedef vdfloat3c		vector_ctor_type;
	typedef vdfloat3x3		self_type;

	vdfloat3x3() {}
	vdfloat3x3(zero_type) { m[0] = m[1] = m[2] = vector_ctor_type(0, 0, 0); }
	vdfloat3x3(identity_type) {
		m[0].set(1, 0, 0);
		m[1].set(0, 1, 0);
		m[2].set(0, 0, 1);
	}
	vdfloat3x3(rotation_x_type, value_type angle) {
		const value_type s(sin(angle));
		const value_type c(cos(angle));

		m[0].set( 1, 0, 0);
		m[1].set( 0, c,-s);
		m[2].set( 0, s, c);
	}

	vdfloat3x3(rotation_y_type, value_type angle) {
		const value_type s(sin(angle));
		const value_type c(cos(angle));

		m[0].set( c, 0, s);
		m[1].set( 0, 1, 0);
		m[2].set(-s, 0, c);
	}
	vdfloat3x3(rotation_z_type, value_type angle) {
		const value_type s(sin(angle));
		const value_type c(cos(angle));

		m[0].set( c,-s, 0);
		m[1].set( s, c, 0);
		m[2].set( 0, 0, 1);
	}

	vector_type& operator[](int k) { return m[k]; }
	const vector_type& operator[](int k) const { return m[k]; }

	self_type operator*(const self_type& v) const {
		self_type result;

#define DO(i,j) result.m[i].v[j] = m[i].v[0]*v.m[0].v[j] + m[i].v[1]*v.m[1].v[j] + m[i].v[2]*v.m[2].v[j]
		DO(0,0);
		DO(0,1);
		DO(0,2);
		DO(1,0);
		DO(1,1);
		DO(1,2);
		DO(2,0);
		DO(2,1);
		DO(2,2);
#undef DO

		return result;
	}

	vector_type operator*(const vector_type& r) const {
		return vector_ctor_type(
				m[0].v[0]*r.v[0] + m[0].v[1]*r.v[1] + m[0].v[2]*r.v[2],
				m[1].v[0]*r.v[0] + m[1].v[1]*r.v[1] + m[1].v[2]*r.v[2],
				m[2].v[0]*r.v[0] + m[2].v[1]*r.v[1] + m[2].v[2]*r.v[2]);
	}

	self_type transpose() const {
		self_type res;

		res.m[0].v[0] = m[0].v[0];
		res.m[0].v[1] = m[1].v[0];
		res.m[0].v[2] = m[2].v[0];
		res.m[1].v[0] = m[0].v[1];
		res.m[1].v[1] = m[1].v[1];
		res.m[1].v[2] = m[2].v[1];
		res.m[2].v[0] = m[0].v[2];
		res.m[2].v[1] = m[1].v[2];
		res.m[2].v[2] = m[2].v[2];

		return res;
	}

	self_type adjunct() const {
		using namespace nsVDMath;

		self_type res;

		res.m[0] = cross(m[1], m[2]);
		res.m[1] = cross(m[2], m[0]);
		res.m[2] = cross(m[0], m[1]);

		return res.transpose();
	}

	value_type det() const {
		return	+ m[0].v[0] * m[1].v[1] * m[2].v[2]
				+ m[1].v[0] * m[2].v[1] * m[0].v[2]
				+ m[2].v[0] * m[0].v[1] * m[1].v[2]
				- m[0].v[0] * m[2].v[1] * m[1].v[2]
				- m[1].v[0] * m[0].v[1] * m[2].v[2]
				- m[2].v[0] * m[1].v[1] * m[0].v[2];
	}

	self_type operator~() const {
		return adjunct() / det();
	}

	self_type& operator*=(const value_type factor) {
		m[0] *= factor;
		m[1] *= factor;
		m[2] *= factor;

		return *this;
	}

	self_type& operator/=(const value_type factor) {
		return operator*=(value_type(1)/factor);
	}

	self_type operator*(const value_type factor) const {
		return self_type(*this) *= factor;
	}

	self_type operator/(const value_type factor) const {
		return self_type(*this) /= factor;
	}

	vector_type m[3];
};

inline vdfloat3 operator*(const vdfloat3& v, const vdfloat3x3& m) {
	return v.x * m.m[0] + v.y * m.m[1] + v.z * m.m[2];
}

class vdfloat4x4 {
public:
	enum zero_type { zero };
	enum identity_type { identity };
	enum rotation_x_type { rotation_x };
	enum rotation_y_type { rotation_y };
	enum rotation_z_type { rotation_z };

	typedef float			value_type;
	typedef vdfloat4		vector_type;
	typedef vdfloat4c		vector_ctor_type;

	vdfloat4x4() {}
	vdfloat4x4(const vdfloat3x3& v) {
		m[0].set(v.m[0].x, v.m[0].y, v.m[0].z, 0.0f);
		m[1].set(v.m[1].x, v.m[1].y, v.m[1].z, 0.0f);
		m[2].set(v.m[2].x, v.m[2].y, v.m[2].z, 0.0f);
		m[3].set(0, 0, 0, 1);
	}

	vdfloat4x4(zero_type) {
		m[0].setzero();
		m[1].setzero();
		m[2].setzero();
		m[3].setzero();
	}

	vdfloat4x4(identity_type) {
		m[0].set(1, 0, 0, 0);
		m[1].set(0, 1, 0, 0);
		m[2].set(0, 0, 1, 0);
		m[3].set(0, 0, 0, 1);
	}
	vdfloat4x4(rotation_x_type, value_type angle) {
		const value_type s(sin(angle));
		const value_type c(cos(angle));

		m[0].set( 1, 0, 0, 0);
		m[1].set( 0, c,-s, 0);
		m[2].set( 0, s, c, 0);
		m[3].set( 0, 0, 0, 1);
	}
	vdfloat4x4(rotation_y_type, value_type angle) {
		const value_type s(sin(angle));
		const value_type c(cos(angle));

		m[0].set( c, 0, s, 0);
		m[1].set( 0, 1, 0, 0);
		m[2].set(-s, 0, c, 0);
		m[3].set( 0, 0, 0, 1);
	}
	vdfloat4x4(rotation_z_type, value_type angle) {
		const value_type s(sin(angle));
		const value_type c(cos(angle));

		m[0].set( c,-s, 0, 0);
		m[1].set( s, c, 0, 0);
		m[2].set( 0, 0, 1, 0);
		m[3].set( 0, 0, 0, 1);
	}

	const value_type *data() const { return &m[0][0]; }

	vector_type& operator[](int n) { return m[n]; }
	const vector_type& operator[](int n) const { return m[n]; }

	vdfloat4x4 operator*(const vdfloat4x4& v) const {
		vdfloat4x4 result;

#define DO(i,j) result.m[i].v[j] = m[i].v[0]*v.m[0].v[j] + m[i].v[1]*v.m[1].v[j] + m[i].v[2]*v.m[2].v[j] + m[i].v[3]*v.m[3].v[j]
		DO(0,0);
		DO(0,1);
		DO(0,2);
		DO(0,3);
		DO(1,0);
		DO(1,1);
		DO(1,2);
		DO(1,3);
		DO(2,0);
		DO(2,1);
		DO(2,2);
		DO(2,3);
		DO(3,0);
		DO(3,1);
		DO(3,2);
		DO(3,3);
#undef DO

		return result;
	}

	vdfloat4x4& operator*=(const vdfloat4x4& v) {
		return operator=(operator*(v));
	}

	vector_type operator*(const vdfloat3& r) const {
		return vector_ctor_type(
				m[0].v[0]*r.v[0] + m[0].v[1]*r.v[1] + m[0].v[2]*r.v[2] + m[0].v[3],
				m[1].v[0]*r.v[0] + m[1].v[1]*r.v[1] + m[1].v[2]*r.v[2] + m[1].v[3],
				m[2].v[0]*r.v[0] + m[2].v[1]*r.v[1] + m[2].v[2]*r.v[2] + m[2].v[3],
				m[3].v[0]*r.v[0] + m[3].v[1]*r.v[1] + m[3].v[2]*r.v[2] + m[3].v[3]);
	}

	vector_type operator*(const vector_type& r) const {
		return vector_ctor_type(
				m[0].v[0]*r.v[0] + m[0].v[1]*r.v[1] + m[0].v[2]*r.v[2] + m[0].v[3]*r.v[3],
				m[1].v[0]*r.v[0] + m[1].v[1]*r.v[1] + m[1].v[2]*r.v[2] + m[1].v[3]*r.v[3],
				m[2].v[0]*r.v[0] + m[2].v[1]*r.v[1] + m[2].v[2]*r.v[2] + m[2].v[3]*r.v[3],
				m[3].v[0]*r.v[0] + m[3].v[1]*r.v[1] + m[3].v[2]*r.v[2] + m[3].v[3]*r.v[3]);
	}

	vector_type m[4];
};

template<class T>
struct VDSize {
	typedef T value_type;

	int w, h;

	VDSize() {}
	VDSize(int _w, int _h) : w(_w), h(_h) {}

	bool operator==(const VDSize& s) const { return w==s.w && h==s.h; }
	bool operator!=(const VDSize& s) const { return w!=s.w || h!=s.h; }

	VDSize& operator+=(const VDSize& s) {
		w += s.w;
		h += s.h;
		return *this;
	}

	T area() const { return w*h; }

	void include(const VDSize& s) {
		if (w < s.w)
			w = s.w;
		if (h < s.h)
			h = s.h;
	}
};

template<class T>
class VDPoint {
public:
	VDPoint();
	VDPoint(T x_, T y_);

	T x;
	T y;
};

template<class T>
VDPoint<T>::VDPoint() {
}

template<class T>
VDPoint<T>::VDPoint(T x_, T y_)
	: x(x_), y(y_)
{
}

template<class T>
class VDRect {
public:
	typedef T value_type;

	VDRect();
	VDRect(T left_, T top_, T right_, T bottom_);

	bool empty() const;
	bool valid() const;

	void clear();
	void invalidate();
	void set(T l, T t, T r, T b);

	void add(T x, T y);
	void add(const VDRect& r);
	void translate(T x, T y);
	void scale(T x, T y);
	void transform(T scaleX, T scaleY, T offsetX, T offsety);
	void resize(T w, T h);

	bool operator==(const VDRect& r) const;
	bool operator!=(const VDRect& r) const;

	T width() const;
	T height() const;
	T area() const;
	VDSize<T> size() const;
	VDPoint<T> top_left() const;
	VDPoint<T> bottom_right() const;

	bool contains(const VDPoint<T>& pt) const;

public:
	T left, top, right, bottom;
};

template<class T>
VDRect<T>::VDRect() {
}

template<class T>
VDRect<T>::VDRect(T left_, T top_, T right_, T bottom_)
	: left(left_)
	, top(top_)
	, right(right_)
	, bottom(bottom_)
{
}

template<class T>
bool VDRect<T>::empty() const {
	return left >= right || top >= bottom;
}

template<class T>
bool VDRect<T>::valid() const {
	return left <= right;
}

template<class T>
void VDRect<T>::clear() {
	left = top = right = bottom = 0;
}

template<class T>
void VDRect<T>::invalidate() {
	left = top = (std::numeric_limits<T>::max)();
	right = bottom = std::numeric_limits<T>::is_signed ? -(std::numeric_limits<T>::max)() : T(0);
}

template<class T>
void VDRect<T>::set(T l, T t, T r, T b) {
	left = l;
	top = t;
	right = r;
	bottom = b;
}

template<class T>
void VDRect<T>::add(T x, T y) {
	if (left > x)
		left = x;
	if (top > y)
		top = y;
	if (right < x)
		right = x;
	if (bottom < y)
		bottom = y;
}

template<class T>
void VDRect<T>::add(const VDRect& src) {
	if (left > src.left)
		left = src.left;
	if (top > src.top)
		top = src.top;
	if (right < src.right)
		right = src.right;
	if (bottom < src.bottom)
		bottom = src.bottom;
}

template<class T>
void VDRect<T>::translate(T x, T y) {
	left += x;
	top += y;
	right += x;
	bottom += y;
}

template<class T>
void VDRect<T>::scale(T x, T y) {
	left *= x;
	top *= y;
	right *= x;
	bottom *= y;
}

template<class T>
void VDRect<T>::transform(T scaleX, T scaleY, T offsetX, T offsetY) {
	left	= left		* scaleX + offsetX;
	top		= top		* scaleY + offsetY;
	right	= right		* scaleX + offsetX;
	bottom	= bottom	* scaleY + offsetY;
}

template<class T>
void VDRect<T>::resize(T w, T h) {
	right = left + w;
	bottom = top + h;
}

template<class T>
bool VDRect<T>::operator==(const VDRect& r) const { return left==r.left && top==r.top && right==r.right && bottom==r.bottom; }

template<class T>
bool VDRect<T>::operator!=(const VDRect& r) const { return left!=r.left || top!=r.top || right!=r.right || bottom!=r.bottom; }

template<class T>
T VDRect<T>::width() const { return right-left; }

template<class T>
T VDRect<T>::height() const { return bottom-top; }

template<class T>
T VDRect<T>::area() const { return (right-left)*(bottom-top); }

template<class T>
VDPoint<T> VDRect<T>::top_left() const { return VDPoint<T>(left, top); }

template<class T>
VDPoint<T> VDRect<T>::bottom_right() const { return VDPoint<T>(right, bottom); }

template<class T>
VDSize<T> VDRect<T>::size() const { return VDSize<T>(right-left, bottom-top); }

template<class T>
bool VDRect<T>::contains(const VDPoint<T>& pt) const {
	return pt.x >= left
		&& pt.x < right
		&& pt.y >= top
		&& pt.y < bottom;
}

///////////////////////////////////////////////////////////////////////////////
typedef VDPoint<sint32>	vdpoint32;
typedef VDSize<sint32>	vdsize32;
typedef VDSize<float>	vdsize32f;
typedef	VDRect<sint32>	vdrect32;
typedef	VDRect<float>	vdrect32f;

template<> bool vdrect32::contains(const vdpoint32& pt) const;

#endif
