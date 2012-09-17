//	VirtualDub - Video processing and capture application
//	System library component
//	Copyright (C) 1998-2006 Avery Lee, All Rights Reserved.
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
#include <math.h>

#include <vd2/system/fraction.h>
#include <vd2/system/vdtypes.h>
#include <vd2/system/math.h>

VDFraction::VDFraction(double d) {
	int xp;
	double mant = frexp(d, &xp);

	if (xp >= 33) {
		hi = 0xFFFFFFFF;
		lo = 1;
	} else if (xp < -31) {
		hi = 0;
		lo = 1;
	} else if (xp >= 0) {
		*this = reduce((uint64)(0.5 + ldexp(mant, 62)), 1ll<<(62-xp));
	} else {
		// This is not quite accurate for very tiny numbers.
		VDFraction t(1.0 / d);
		lo = t.hi;
		hi = t.lo;
	}
}

VDFraction VDFraction::reduce(uint64 hi, uint64 lo) {

	// Check for undefined.

	if (!lo)
		return VDFraction(0,0);

	// Check for zero.

	if (!hi) {
		return VDFraction(0,1);
	}

	// Check for infinity.

	if (!((uint64)lo>>32) && (uint64)hi > ((uint64)lo<<32)-lo)
		return VDFraction(0xFFFFFFFFUL, 1);

	// Algorithm from Wikipedia, Continued Fractions:
	uint64 n0 = 0;
	uint64 d0 = 1;
	uint32 n1 = 1;
	uint32 d1 = 0;
	uint64 fp = 0;

	uint32 n_best;
	uint32 d_best;

	for(;;) {
		uint64 a = hi/lo;			// next continued fraction term
		uint64 f = hi%lo;			// remainder

		uint64 n2 = n0 + n1*a;		// next convergent numerator
		uint64 d2 = d0 + d1*a;		// next convergent denominator

		uint32 n_overflow = (uint32)(n2 >> 32);
		uint32 d_overflow = (uint32)(d2 >> 32);

		if (n_overflow | d_overflow) {
			uint64 a2 = a;

			// reduce last component until numerator and denominator are within range
			if (n_overflow)
				a2 = (0xFFFFFFFF - n0) / n1;

			if (d_overflow) {
				uint64 a3 = (0xFFFFFFFF - d0) / d1;
				if (a2 > a3)
					a2 = a3;
			}

			// check if new term is better
			// 1/2a_k admissibility test
			if (a2*2 < a || (a2*2 == a && d0*fp <= f*d1))
				return VDFraction((uint32)n_best, (uint32)d_best);

			return VDFraction((uint32)(n0 + n1*a2), (uint32)(d0 + d1*a2));
		}

		n_best = (uint32)n2;
		d_best = (uint32)d2;

		// if fraction is exact, we're done.
		if (!f)
			return VDFraction((uint32)n_best, (uint32)d_best);

		n0 = n1;
		n1 = (uint32)n2;
		d0 = d1;
		d1 = (uint32)d2;
		fp = f;

		hi = lo;
		lo = f;
	}
}

// a (cond) b
// a-b (cond) 0
// aH*bL - aL*bh (cond) 0
// aH*bL (cond) aL*bH

bool VDFraction::operator==(VDFraction b) const {
	return (uint64)hi * b.lo == (uint64)lo * b.hi;
}

bool VDFraction::operator!=(VDFraction b) const {
	return (uint64)hi * b.lo != (uint64)lo * b.hi;
}

bool VDFraction::operator< (VDFraction b) const {
	return (uint64)hi * b.lo < (uint64)lo * b.hi;
}

bool VDFraction::operator<=(VDFraction b) const {
	return (uint64)hi * b.lo <= (uint64)lo * b.hi;
}

bool VDFraction::operator> (VDFraction b) const {
	return (uint64)hi * b.lo > (uint64)lo * b.hi;
}

bool VDFraction::operator>=(VDFraction b) const {
	return (uint64)hi * b.lo >= (uint64)lo * b.hi;
}

VDFraction VDFraction::operator*(VDFraction b) const {
	return reduce((uint64)hi * b.hi, (uint64)lo * b.lo);
}

VDFraction VDFraction::operator/(VDFraction b) const {
	return reduce((uint64)hi * b.lo, (uint64)lo * b.hi);
}

VDFraction VDFraction::operator*(unsigned long b) const {
	return reduce((uint64)hi * b, lo);
}

VDFraction VDFraction::operator/(unsigned long b) const {
	return reduce(hi, (uint64)lo * b);
}

VDFraction& VDFraction::operator*=(VDFraction b) {
	return *this = reduce((uint64)hi * b.hi, (uint64)lo * b.lo);
}

VDFraction& VDFraction::operator/=(VDFraction b) {
	return *this = reduce((uint64)hi * b.lo, (uint64)lo * b.hi);
}

VDFraction& VDFraction::operator*=(unsigned long b) {
	return *this = reduce((uint64)hi * b, lo);
}

VDFraction& VDFraction::operator/=(unsigned long b) {
	return *this = reduce(hi, (uint64)lo * b);
}

///////////////////////////////////////////////////////////////////////////

sint64 VDFraction::scale64t(sint64 v) const {
	uint32 r;
	return v<0 ? -VDFractionScale64(-v, hi, lo, r) : VDFractionScale64(v, hi, lo, r);
}

sint64 VDFraction::scale64u(sint64 v) const {
	uint32 r;
	if (v<0) {
		v = -VDFractionScale64(-v, hi, lo, r);
		return v;
	} else {
		v = +VDFractionScale64(+v, hi, lo, r);
		return v + (r > 0);
	}
}

sint64 VDFraction::scale64r(sint64 v) const {
	uint32 r;
	if (v<0) {
		v = -VDFractionScale64(-v, hi, lo, r);
		return v - (r >= (lo>>1) + (lo&1));
	} else {
		v = +VDFractionScale64(+v, hi, lo, r);
		return v + (r >= (lo>>1) + (lo&1));
	}
}

sint64 VDFraction::scale64it(sint64 v) const {
	uint32 r;
	return v<0 ? -VDFractionScale64(-v, lo, hi, r) : +VDFractionScale64(+v, lo, hi, r);
}

sint64 VDFraction::scale64ir(sint64 v) const {
	uint32 r;
	if (v<0) {
		v = -VDFractionScale64(-v, lo, hi, r);
		return v - (r >= (hi>>1) + (hi&1));
	} else {
		v = +VDFractionScale64(+v, lo, hi, r);
		return v + (r >= (hi>>1) + (hi&1));
	}
}

sint64 VDFraction::scale64iu(sint64 v) const {
	uint32 r;
	if (v<0) {
		v = -VDFractionScale64(-v, lo, hi, r);
		return v;
	} else {
		v = +VDFractionScale64(+v, lo, hi, r);
		return v + (r > 0);
	}
}

///////////////////////////////////////////////////////////////////////////

double VDFraction::asDouble() const {
	return (double)hi / (double)lo;
}

double VDFraction::AsInverseDouble() const {
	return (double)lo / (double)hi;
}

unsigned long VDFraction::roundup32ul() const {
	return (hi + (lo-1)) / lo;
}

///////////////////////////////////////////////////////////////////////////

bool VDFraction::Parse(const char *s) {
	char c;

	// skip whitespace
	while((c = *s) && (c == ' ' || c == '\t'))
		++s;

	// accumulate integer digits
	uint64 x = 0;
	uint64 y = 1;

	while(c = *s) {
		uint32 offset = (uint32)c - '0';

		if (offset >= 10)
			break;

		x = (x * 10) + offset;

		// check for overflow
		if (x >> 32)
			return false;

		++s;
	}

	if (c == '.') {
		++s;

		while(c = *s) {
			uint32 offset = (uint32)c - '0';

			if (offset >= 10)
				break;

			if (x >= 100000000000000000 ||
				y >= 100000000000000000) {
				if (offset >= 5)
					++x;
				while((c = *s) && (unsigned)(c - '0') < 10)
					++s;
				break;
			}

			x = (x * 10) + offset;
			y *= 10;
			++s;
		}
	}

	while(c == ' ' || c == '\t')
		c = *++s;

	// check for trailing garbage
	if (c)
		return false;

	// check for overflow
	if (!(y >> 32) && ((uint64)(uint32)y << 32) <= x)
		return false;

	// reduce fraction and return success
	*this = reduce(x, y);
	return true;
}
