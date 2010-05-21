#include "stdafx.h"
#include <vd2/system/halffloat.h>

uint16 VDConvertFloatToHalf(const void *f) {
	uint32 v = *(const uint32 *)f;

	uint32 sign = (v >> 16) & 0x8000;
	sint32 exmant = v & 0x7fffffff;

	if (exmant > 0x7f800000) {
		// convert NaNs directly
		exmant = (exmant & 0x00400000) + 0x47a00000;
	} else if (exmant > 0x47800000) {
		// clamp large numbers to infinity
		exmant = 0x47800000;
	} else if (exmant < 0x33800000) {
		// clamp very tiny numbers to zero
		exmant = 0x38000000;
	} else if (exmant < 0x38800000) {
		// normalized finite converting to denormal
		uint32 ex = exmant & 0x7f800000;
		uint32 mant = (exmant & 0x007fffff) | 0x800000;
		uint32 sticky = 0;

		while(ex < 0x38800000) {
			ex += 0x00800000;
			sticky |= mant;
			mant >>= 1;
		}

		// round to nearest even
		sticky |= mant >> 13;

		// round up with sticky bits
		mant += (sticky & 1);

		// round up with round bit
		mant += 0x0fff;

		exmant = ex + mant - 0x800000;
	} else {
		// round normal numbers using round to nearest even
		exmant |= (exmant & 0x00002000) >> 13;
		exmant += 0x00000fff;
	}

	// shift and rebias exponent
	exmant -= 0x38000000;
	exmant >>= 13;

	return (uint16)(sign + exmant);
}

void VDConvertHalfToFloat(uint16 h, void *dst) {
	uint32 sign = ((uint32)h << 16) & 0x80000000;
	uint32 exmant = (uint32)h & 0x7fff;
	uint32 v = 0;

	if (exmant >= 0x7c00) {
		// infinity or NaN
		v = (exmant << 13) + 0x70000000;
	} else if (exmant >= 0x0400) {
		// normalized finite
		v = (exmant << 13) + 0x38000000;
	} else if (exmant) {
		// denormal
		uint32 ex32 = 0x38000000;
		uint32 mant32 = (exmant & 0x3ff) << 13;

		while(!(mant32 & 0x800000)) {
			mant32 <<= 1;
			ex32 -= 0x800000;
		}

		v = ex32 + mant32;
	}

	*(uint32 *)dst = v + sign;
}
