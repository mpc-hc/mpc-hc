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

#ifndef f_VD2_SYSTEM_INT128_H
#define f_VD2_SYSTEM_INT128_H

#include <vd2/system/vdtypes.h>

struct vdint128;
struct vduint128;

#ifdef _M_AMD64
	extern "C" __int64 _mul128(__int64 x, __int64 y, __int64 *hiresult);
	extern "C" unsigned __int64 _umul128(unsigned __int64 x, unsigned __int64 y, unsigned __int64 *hiresult);
	extern "C" unsigned __int64 __shiftleft128(unsigned __int64 low, unsigned __int64 high, unsigned char shift);
	extern "C" unsigned __int64 __shiftright128(unsigned __int64 low, unsigned __int64 high, unsigned char shift);

	#pragma intrinsic(_mul128)
	#pragma intrinsic(_umul128)
	#pragma intrinsic(__shiftleft128)
	#pragma intrinsic(__shiftright128)

	extern "C" {
		void vdasm_uint128_add(uint64 dst[2], const uint64 x[2], const uint64 y[2]);
		void vdasm_uint128_sub(uint64 dst[2], const uint64 x[2], const uint64 y[2]);
		void vdasm_uint128_mul(uint64 dst[2], const uint64 x[2], const uint64 y[2]);
	}
#else
	extern "C" {
		void __cdecl vdasm_uint128_add(uint64 dst[2], const uint64 x[2], const uint64 y[2]);
		void __cdecl vdasm_uint128_sub(uint64 dst[2], const uint64 x[2], const uint64 y[2]);
	}
#endif

struct vdint128 {
public:
	union {
		sint32 d[4];
		sint64 q[2];
	};

	vdint128() {}

	vdint128(sint64 x) {
		q[0] = x;
		q[1] = x>>63;
	}

	vdint128(uint64 x) {
		q[0] = (sint64)x;
		q[1] = 0;
	}

	vdint128(int x) {
		q[0] = x;
		q[1] = (sint64)x >> 63;
	}

	vdint128(unsigned int x) {
		q[0] = x;
		q[1] = 0;
	}

	vdint128(unsigned long x) {
		q[0] = x;
		q[1] = 0;
	}

	vdint128(sint64 hi, uint64 lo) {
		q[0] = lo;
		q[1] = hi;
	}

	explicit inline vdint128(const vduint128& x);

	sint64 getHi() const { return q[1]; }
	uint64 getLo() const { return q[0]; }

	operator double() const;
	operator sint64() const {
		return (sint64)q[0];
	}
	operator uint64() const {
		return (uint64)q[0];
	}

	bool operator==(const vdint128& x) const {
		return q[1] == x.q[1] && q[0] == x.q[0];
	}

	bool operator!=(const vdint128& x) const {
		return q[1] != x.q[1] || q[0] != x.q[0];
	}

	bool operator<(const vdint128& x) const {
		return q[1] < x.q[1] || (q[1] == x.q[1] && (uint64)q[0] < (uint64)x.q[0]);
	}

	bool operator<=(const vdint128& x) const {
		return q[1] < x.q[1] || (q[1] == x.q[1] && (uint64)q[0] <= (uint64)x.q[0]);
	}

	bool operator>(const vdint128& x) const {
		return q[1] > x.q[1] || (q[1] == x.q[1] && (uint64)q[0] > (uint64)x.q[0]);
	}

	bool operator>=(const vdint128& x) const {
		return q[1] > x.q[1] || (q[1] == x.q[1] && (uint64)q[0] >= (uint64)x.q[0]);
	}

	const vdint128 operator+(const vdint128& x) const {
		vdint128 t;
		vdasm_uint128_add((uint64 *)t.q, (const uint64 *)q, (const uint64 *)x.q);
		return t;
	}

	const vdint128 operator-(const vdint128& x) const {
		vdint128 t;
		vdasm_uint128_sub((uint64 *)t.q, (const uint64 *)q, (const uint64 *)x.q);
		return t;
	}

	const vdint128& operator+=(const vdint128& x) {
		vdasm_uint128_add((uint64 *)q, (const uint64 *)q, (const uint64 *)x.q);
		return *this;
	}

	const vdint128& operator-=(const vdint128& x) {
		vdasm_uint128_sub((uint64 *)q, (const uint64 *)q, (const uint64 *)x.q);
		return *this;
	}

	const vdint128 operator*(const vdint128& x) const;

	const vdint128 operator/(int x) const;

	const vdint128 operator-() const {
		vdint128 t(0);
		vdasm_uint128_sub((uint64 *)t.q, (const uint64 *)t.q, (const uint64 *)q);
		return t;
	}

	const vdint128 abs() const {
		return q[1] < 0 ? -*this : *this;
	}

#ifdef _M_AMD64
	void setSquare(sint64 v) {
		const vdint128 v128(v);
		operator=(v128*v128);
	}

	const vdint128 operator<<(int count) const {
		vdint128 t;

		if (count >= 64) {
			t.q[0] = 0;
			t.q[1] = q[0] << (count-64);
		} else {
			t.q[0] = q[0] << count;
			t.q[1] = __shiftleft128(q[0], q[1], count);
		}

		return t;
	}

	const vdint128 operator>>(int count) const {
		vdint128 t;

		if (count >= 64) {
			t.q[0] = q[1] >> (count-64);
			t.q[1] = q[1] >> 63;
		} else {
			t.q[0] = __shiftright128(q[0], q[1], count);
			t.q[1] = q[1] >> count;
		}

		return t;
	}
#else
	void setSquare(sint64 v);

	const vdint128 operator<<(int v) const;
	const vdint128 operator>>(int v) const;
#endif
};

struct vduint128 {
public:
	union {
		uint32 d[4];
		uint64 q[2];
	};

	vduint128() {}

	vduint128(sint64 x) {
		q[0] = (sint64)x;
		q[1] = 0;
	}

	vduint128(uint64 x) {
		q[0] = x;
		q[1] = 0;
	}

	vduint128(int x) {
		q[0] = (uint64)x;
		q[1] = 0;
	}

	vduint128(unsigned x) {
		q[0] = x;
		q[1] = 0;
	}

	vduint128(uint64 hi, uint64 lo) {
		q[0] = lo;
		q[1] = hi;
	}

	explicit inline vduint128(const vdint128& x);

	uint64 getHi() const { return q[1]; }
	uint64 getLo() const { return q[0]; }

	operator sint64() const {
		return (sint64)q[0];
	}

	operator uint64() const {
		return (uint64)q[0];
	}

	bool operator==(const vduint128& x) const {
		return q[1] == x.q[1] && q[0] == x.q[0];
	}

	bool operator!=(const vduint128& x) const {
		return q[1] != x.q[1] || q[0] != x.q[0];
	}

	bool operator<(const vduint128& x) const {
		return q[1] < x.q[1] || (q[1] == x.q[1] && q[0] < x.q[0]);
	}

	bool operator<=(const vduint128& x) const {
		return q[1] < x.q[1] || (q[1] == x.q[1] && q[0] <= x.q[0]);
	}

	bool operator>(const vduint128& x) const {
		return q[1] > x.q[1] || (q[1] == x.q[1] && q[0] > x.q[0]);
	}

	bool operator>=(const vduint128& x) const {
		return q[1] > x.q[1] || (q[1] == x.q[1] && q[0] >= x.q[0]);
	}

	const vduint128 operator+(const vduint128& x) const {
		vduint128 t;
		vdasm_uint128_add(t.q, q, x.q);
		return t;
	}

	const vduint128 operator-(const vduint128& x) const {
		vduint128 t;
		vdasm_uint128_sub(t.q, q, x.q);
		return t;
	}

	const vduint128& operator+=(const vduint128& x) {
		vdasm_uint128_add(q, q, x.q);
		return *this;
	}

	const vduint128& operator-=(const vduint128& x) {
		vdasm_uint128_sub(q, q, x.q);
		return *this;
	}

	const vduint128 operator*(const vduint128& x) const;

	const vduint128 operator-() const {
		vduint128 t(0U);
		vdasm_uint128_sub((uint64 *)t.q, (const uint64 *)t.q, (const uint64 *)q);
		return t;
	}

	vduint128& operator<<=(int count) {
		return operator=(operator<<(count));
	}

	vduint128& operator>>=(int count) {
		return operator=(operator>>(count));
	}

#ifdef _M_AMD64
	const vduint128 operator<<(int count) const {
		vduint128 t;

		if (count >= 64) {
			t.q[0] = 0;
			t.q[1] = q[0] << (count-64);
		} else {
			t.q[0] = q[0] << count;
			t.q[1] = __shiftleft128(q[0], q[1], count);
		}

		return t;
	}

	const vduint128 operator>>(int count) const {
		vduint128 t;

		if (count >= 64) {
			t.q[0] = q[1] >> (count-64);
			t.q[1] = 0;
		} else {
			t.q[0] = __shiftright128(q[0], q[1], count);
			t.q[1] = q[1] >> count;
		}

		return t;
	}
#else
	const vduint128 operator<<(int v) const;
	const vduint128 operator>>(int v) const;
#endif
};

inline vdint128::vdint128(const vduint128& x) {
	q[0] = x.q[0];
	q[1] = x.q[1];
}

inline vduint128::vduint128(const vdint128& x) {
	q[0] = x.q[0];
	q[1] = x.q[1];
}

#ifdef _M_AMD64
	inline vduint128 VDUMul64x64To128(uint64 x, uint64 y) {
		vduint128 result;
		result.q[0] = _umul128(x, y, &result.q[1]);
		return result;
	}
	uint64 VDUDiv128x64To64(const vduint128& dividend, uint64 divisor, uint64& remainder);
#else
	vduint128 VDUMul64x64To128(uint64 x, uint64 y);
	uint64 VDUDiv128x64To64(const vduint128& dividend, uint64 divisor, uint64& remainder);
#endif

#endif
