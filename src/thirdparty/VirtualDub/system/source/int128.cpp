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

#include "stdafx.h"
#include <math.h>

#include <vd2/system/int128.h>

#if defined(VD_CPU_X86) && defined(VD_COMPILER_MSVC)
	void __declspec(naked) __cdecl vdasm_uint128_add(uint64 dst[2], const uint64 x[2], const uint64 y[2]) {
		__asm {
			push	ebx

			mov		ebx, [esp+16]
			mov		ecx, [esp+12]
			mov		edx, [esp+8]

			mov		eax, [ecx+0]
			add		eax, [ebx+0]
			mov		[edx+0],eax
			mov		eax, [ecx+4]
			adc		eax, [ebx+4]
			mov		[edx+4],eax
			mov		eax, [ecx+8]
			adc		eax, [ebx+8]
			mov		[edx+8],eax
			mov		eax, [ecx+12]
			adc		eax, [ebx+12]
			mov		[edx+12],eax

			pop		ebx
			ret
		}
	}

	void __declspec(naked) __cdecl vdasm_uint128_sub(uint64 dst[2], const uint64 x[2], const uint64 y[2]) {
		__asm {
			push	ebx

			mov		ebx, [esp+16]
			mov		ecx, [esp+12]
			mov		edx, [esp+8]

			mov		eax, [ecx+0]
			sub		eax, [ebx+0]
			mov		[edx+0],eax
			mov		eax, [ecx+4]
			sbb		eax, [ebx+4]
			mov		[edx+4],eax
			mov		eax, [ecx+8]
			sbb		eax, [ebx+8]
			mov		[edx+8],eax
			mov		eax, [ecx+12]
			sbb		eax, [ebx+12]
			mov		[edx+12],eax

			pop		ebx
			ret
		}
	}

	void __declspec(naked) vdint128::setSquare(sint64 v) {
		__asm {
			push	edi
			push	esi
			push	ebx
			mov		eax, [esp+20]
			cdq
			mov		esi, eax
			mov		eax, [esp+16]
			xor		eax, edx
			xor		esi, edx
			sub		eax, edx
			sbb		esi, edx
			mov		ebx, eax
			mul		eax
			mov		[ecx], eax
			mov		edi, edx
			mov		eax, ebx
			mul		esi
			mov		ebx, 0
			add		eax, eax
			adc		edx, edx
			add		eax, edi
			adc		edx, 0
			mov		edi, edx
			adc		ebx, 0
			mov		[ecx+4], eax
			mov		eax, esi
			mul		esi
			add		eax, edi
			adc		edx, ebx
			mov		[ecx+8], eax
			mov		[ecx+12], edx
			pop		ebx
			pop		esi
			pop		edi
			ret		8
		}
	}

	const vdint128 __declspec(naked) vdint128::operator<<(int v) const {
		__asm {
			push	ebp
			push	ebx
			push	esi
			push	edi

			mov		esi,ecx
			mov		edx,[esp+20]

			mov		ecx,[esp+24]
			cmp		ecx,128
			jae		zeroit

			mov		eax,[esi+12]
			mov		ebx,[esi+8]
			mov		edi,[esi+4]
			mov		ebp,[esi]

	dwordloop:
			cmp		ecx,32
			jb		bits

			mov		eax,ebx
			mov		ebx,edi
			mov		edi,ebp
			xor		ebp,ebp
			sub		ecx,32
			jmp		short dwordloop

	bits:
			shld	eax,ebx,cl
			shld	ebx,edi,cl
			mov		[edx+12],eax
			mov		[edx+8],ebx
			shld	edi,ebp,cl

			shl		ebp,cl
			mov		[edx+4],edi
			mov		[edx],ebp

			pop		edi
			pop		esi
			pop		ebx
			pop		ebp
			mov		eax,[esp+4]
			ret		8

	zeroit:
			xor		eax,eax
			mov		[edx+0],eax
			mov		[edx+4],eax
			mov		[edx+8],eax
			mov		[edx+12],eax

			pop		edi
			pop		esi
			pop		ebx
			pop		ebp
			mov		eax,[esp+4]
			ret		8
		}
	}

	const vdint128 __declspec(naked) vdint128::operator>>(int v) const {
		__asm {
			push	ebp
			push	ebx
			push	esi
			push	edi

			mov		esi,ecx
			mov		edx,[esp+20]

			mov		eax,[esi+12]
			mov		ecx,[esp+24]
			cmp		ecx,127
			jae		clearit

			mov		ebx,[esi+8]
			mov		edi,[esi+4]
			mov		ebp,[esi]

	dwordloop:
			cmp		ecx,32
			jb		bits

			mov		ebp,edi
			mov		edi,ebx
			mov		ebx,eax
			sar		eax,31
			sub		ecx,32
			jmp		short dwordloop

	bits:
			shrd	ebp,edi,cl
			shrd	edi,ebx,cl
			mov		[edx],ebp
			mov		[edx+4],edi
			shrd	ebx,eax,cl

			sar		eax,cl
			mov		[edx+8],ebx
			mov		[edx+12],eax

			pop		edi
			pop		esi
			pop		ebx
			pop		ebp
			mov		eax,[esp+4]
			ret		8

	clearit:
			sar		eax, 31
			mov		[edx+0],eax
			mov		[edx+4],eax
			mov		[edx+8],eax
			mov		[edx+12],eax

			pop		edi
			pop		esi
			pop		ebx
			pop		ebp
			mov		eax,[esp+4]
			ret		8
		}
	}

	const vduint128 __declspec(naked) vduint128::operator<<(int v) const {
		__asm {
			push	ebp
			push	ebx
			push	esi
			push	edi

			mov		esi,ecx
			mov		edx,[esp+20]

			mov		ecx,[esp+24]
			cmp		ecx,128
			jae		zeroit

			mov		eax,[esi+12]
			mov		ebx,[esi+8]
			mov		edi,[esi+4]
			mov		ebp,[esi]

	dwordloop:
			cmp		ecx,32
			jb		bits

			mov		eax,ebx
			mov		ebx,edi
			mov		edi,ebp
			xor		ebp,ebp
			sub		ecx,32
			jmp		short dwordloop

	bits:
			shld	eax,ebx,cl
			shld	ebx,edi,cl
			mov		[edx+12],eax
			mov		[edx+8],ebx
			shld	edi,ebp,cl

			shl		ebp,cl
			mov		[edx+4],edi
			mov		[edx],ebp

			pop		edi
			pop		esi
			pop		ebx
			pop		ebp
			mov		eax,[esp+4]
			ret		8

	zeroit:
			xor		eax,eax
			mov		[edx+0],eax
			mov		[edx+4],eax
			mov		[edx+8],eax
			mov		[edx+12],eax

			pop		edi
			pop		esi
			pop		ebx
			pop		ebp
			mov		eax,[esp+4]
			ret		8
		}
	}

	const vduint128 __declspec(naked) vduint128::operator>>(int v) const {
		__asm {
			push	ebp
			push	ebx
			push	esi
			push	edi

			mov		esi,ecx
			mov		edx,[esp+20]

			mov		eax,[esi+12]
			mov		ecx,[esp+24]
			cmp		ecx,127
			jae		clearit

			mov		ebx,[esi+8]
			mov		edi,[esi+4]
			mov		ebp,[esi]

	dwordloop:
			cmp		ecx,32
			jb		bits

			mov		ebp,edi
			mov		edi,ebx
			mov		ebx,eax
			xor		eax,eax
			sub		ecx,32
			jmp		short dwordloop

	bits:
			shrd	ebp,edi,cl
			shrd	edi,ebx,cl
			mov		[edx],ebp
			mov		[edx+4],edi
			shrd	ebx,eax,cl

			shr		eax,cl
			mov		[edx+8],ebx
			mov		[edx+12],eax

			pop		edi
			pop		esi
			pop		ebx
			pop		ebp
			mov		eax,[esp+4]
			ret		8

	clearit:
			sar		eax, 31
			mov		[edx+0],eax
			mov		[edx+4],eax
			mov		[edx+8],eax
			mov		[edx+12],eax

			pop		edi
			pop		esi
			pop		ebx
			pop		ebp
			mov		eax,[esp+4]
			ret		8
		}
	}

#elif !defined(VD_CPU_AMD64)

	// These aren't really assembly routines, but we define them so we aren't asm dependent.

	void vdasm_uint128_add(uint64 dst[2], const uint64 x[2], const uint64 y[2]) {
		dst[0] = x[0] + y[0];
		dst[1] = x[1] + y[1] + (dst[0] < x[0]);
	}

	void vdasm_uint128_sub(uint64 dst[2], const uint64 x[2], const uint64 y[2]) {
		dst[0] = x[0] - y[0];
		dst[1] = x[1] - y[1] - (dst[0] > x[0]);
	}

	void vdint128::setSquare(sint64 v) {
		vdint128 r;

		uint32 u0 = (uint32)v;
		uint32 u1 = (uint32)(v >> 32);
		uint64 m0 = u0*u0;
		uint64 m1 = u0*u1;		// added twice
		uint64 m2 = u1*u1;
		uint32 s0  = (uint32)m0;
		uint32 s1a = (uint32)(m0 >> 32);
		uint32 s1b = (uint32)m1;
		uint32 s2a = (uint32)(m1 >> 32);

		q[1] = m2 + s2a;

		d[0] = s0;

		d[1] = s1a + s1b;
		if (d[1] < s1b)
			++q[1];

		d[1] += s1b;
		if (d[1] < s1b)
			++q[1];
	}

	const vdint128 vdint128::operator<<(int v) const {
		vdint128 r;

		r.q[0] = q[0];
		r.q[1] = q[1];

		if (v >= 64) {
			if (v >= 128) {
				r.q[0] = 0;
				r.q[1] = 0;
				return r;
			}

			r.q[1] = r.q[0];
			r.q[0] = 0;

			v -= 64;
		}

		if (v) {
			r.q[1] = (r.q[1] << v) + ((uint64)r.q[0] >> (64 - v));
			r.q[0] <<= v;
		}

		return r;
	}

	const vdint128 vdint128::operator>>(int v) const {
		vdint128 r;

		r.q[0] = q[0];
		r.q[1] = q[1];

		if (v >= 64) {
			sint64 sign = q[1] >> 63;

			if (v >= 128) {
				r.q[0] = sign;
				r.q[1] = sign;
				return r;
			}

			r.q[0] = r.q[1];
			r.q[1] = sign;

			v -= 64;
		}

		if (v) {
			r.q[0] = ((uint64)r.q[0] >> v) + (r.q[1] << (64 - v));
			r.q[1] >>= v;
		}

		return r;
	}

	const vduint128 vduint128::operator<<(int v) const {
		vduint128 r;

		r.q[0] = q[0];
		r.q[1] = q[1];

		if (v >= 64) {
			if (v >= 128) {
				r.q[0] = 0;
				r.q[1] = 0;
				return r;
			}

			r.q[1] = r.q[0];
			r.q[0] = 0;

			v -= 64;
		}

		if (v) {
			r.q[1] = (r.q[1] << v) + (r.q[0] >> (64 - v));
			r.q[0] <<= v;
		}

		return r;
	}

	const vduint128 vduint128::operator>>(int v) const {
		vduint128 r;

		r.q[0] = q[0];
		r.q[1] = q[1];

		if (v >= 64) {
			if (v >= 128) {
				r.q[0] = 0;
				r.q[1] = 0;
				return r;
			}

			r.q[0] = r.q[1];
			r.q[1] = 0;

			v -= 64;
		}

		if (v) {
			r.q[0] = (r.q[0] >> v) + (r.q[1] << (64 - v));
			r.q[1] >>= v;
		}

		return r;
	}
#endif

const vdint128 vdint128::operator*(const vdint128& x) const {
	vdint128 X = x.abs();
	vdint128 Y = abs();

	vduint128 bd(VDUMul64x64To128(X.q[0], Y.q[0]));

	bd.q[1] += X.q[0]*Y.q[1] + X.q[1]*Y.q[0];

	return (q[1]^x.q[1])<0 ? -vdint128(bd) : vdint128(bd);
}

const vdint128 vdint128::operator/(int x) const {
	vdint128 r;
	sint64 accum;

	r.d[3] = d[3] / x;
	
	accum = ((sint64)(d[3] % x) << 32) + d[2];
	r.d[2] = (sint32)(accum / x);

	accum = ((accum % x) << 32) + d[1];
	r.d[1] = (sint32)(accum / x);

	accum = ((accum % x) << 32) + d[0];
	r.d[0] = (sint32)(accum / x);

	return r;
}

vdint128::operator double() const {
	return (double)(unsigned long)q[0]
		+ ldexp((double)(unsigned long)((unsigned __int64)q[0]>>32), 32)
		+ ldexp((double)q[1], 64);
}

/////////////////////////////////////////////////////////////////////////////

const vduint128 vduint128::operator*(const vduint128& x) const {
	vduint128 result(VDUMul64x64To128(q[0], x.q[0]));

	result.q[1] += q[0]*x.q[1] + q[1]*x.q[0];

	return result;
}

#if defined(VD_CPU_X86) && defined(VD_COMPILER_MSVC)
	vduint128 __declspec(naked) __cdecl VDUMul64x64To128(uint64 x, uint64 y) {
		__asm {
			mov		ecx,[esp+4]

			mov		eax,[esp+8]
			mul		dword ptr [esp+16]		;EDX:EAX = BD
			mov		[ecx+0],eax
			mov		[ecx+4],edx

			mov		eax,[esp+12]
			mul		dword ptr [esp+20]		;EDX:EAX = AC
			mov		[ecx+8],eax
			mov		[ecx+12],edx

			mov		eax,[esp+8]
			mul		dword ptr [esp+20]		;EDX:EAX = BC
			add		[ecx+4],eax
			adc		[ecx+8],edx
			adc		dword ptr [ecx+12], 0

			mov		eax,[esp+12]
			mul		dword ptr [esp+16]		;EDX:EAX = AD
			add		[ecx+4],eax
			adc		[ecx+8],edx
			adc		dword ptr [ecx+12], 0

			mov		eax, ecx
			ret
		}
	}
#elif !defined(VD_CPU_AMD64)
	vduint128 VDUMul64x64To128(uint64 x, uint64 y) {
		uint32 x0 = (uint32)x;
		uint32 x1 = (uint32)(x >> 32);
		uint32 y0 = (uint32)y;
		uint32 y1 = (uint32)(y >> 32);

		uint64 m0  = (uint64)x0*y0;
		uint64 m1a = (uint64)x1*y0;
		uint64 m1b = (uint64)x0*y1;
		uint64 m2  = (uint64)x1*y1;

		uint32 s0  = (uint32)m0;
		uint32 s1a = (uint32)(m0 >> 32);
		uint32 s1b = (uint32)m1a;
		uint32 s1c = (uint32)m1b;
		uint32 s2a = (uint32)(m1a >> 32);
		uint32 s2b = (uint32)(m1b >> 32);
		uint32 s2c = (uint32)m2;
		uint32 s3  = (uint32)(m2 >> 32);

		vduint128 r;
		r.d[0] = s0;
		r.d[1] = s1a + s1b;
		r.d[2] = r.d[1] < s1b;
		r.d[1] += s1c;
		r.d[2] += r.d[1] < s1c;
		r.d[2] += s2a;
		r.d[3] = r.d[2] < s2a;
		r.d[2] += s2b;
		r.d[3] += r.d[2] < s2b;
		r.d[2] += s2c;
		r.d[3] += r.d[2] < s2c;
		r.d[3] += s3;

		return r;
	}
#endif

uint64 VDUDiv128x64To64(const vduint128& dividend, uint64 divisor, uint64& remainder) {
	vduint128 temp(dividend);
	vduint128 divisor2(divisor);

	divisor2 <<= 63;

	uint64 result = 0;
	for(int i=0; i<64; ++i) {
		result += result;
		if (temp >= divisor2) {
			temp -= divisor2;
			++result;
		}
		temp += temp;
	}

	remainder = temp.q[1];

	return result;
}
