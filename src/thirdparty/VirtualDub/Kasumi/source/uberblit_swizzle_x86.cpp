//	VirtualDub - Video processing and capture application
//	Graphics support library
//	Copyright (C) 1998-2009 Avery Lee
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include <stdafx.h>
#include "uberblit_swizzle_x86.h"

#ifdef VD_COMPILER_MSVC
	#pragma warning(disable: 4799)	// warning C4799: function 'vdasm_extract_8in16_even_MMX' has no EMMS instruction
#endif

void __declspec(naked) __fastcall vdasm_extract_8in16_even_MMX(void *dst, const void *src, uint32 count) {
	__asm {
		mov			eax, [esp+4]
		pcmpeqb		mm2, mm2
		psrlw		mm2, 8
		sub			eax, 8
		jc			xtra
xloop:
		movq		mm0, [edx]
		movq		mm1, [edx+8]
		pand		mm0, mm2
		pand		mm1, mm2
		packuswb	mm0, mm1
		add			edx, 16
		movq		[ecx], mm0
		add			ecx, 8
		sub			eax, 8
		jns			xloop
xtra:
		add			eax, 8
		jz			fin
		push		ebx
xtraloop:
		mov			bl, [edx]
		add			edx, 2
		mov			[ecx], bl
		add			ecx, 1
		sub			eax, 1
		jnz			xtraloop

		pop			ebx
fin:
		ret			4
	}
}

void __declspec(naked) __fastcall vdasm_extract_8in16_odd_MMX(void *dst, const void *src, uint32 count) {
	__asm {
		mov			eax, [esp+4]
		sub			eax, 8
		jc			xtra
xloop:
		movq		mm0, [edx]
		movq		mm1, [edx+8]
		psrlw		mm0, 8
		psrlw		mm1, 8
		add			edx, 16
		packuswb	mm0, mm1
		movq		[ecx], mm0
		add			ecx, 8
		sub			eax, 8
		jns			xloop
xtra:
		add			eax, 8
		jz			fin
		push		ebx
xtraloop:
		mov			bl, [edx+1]
		add			edx, 2
		mov			[ecx], bl
		add			ecx, 1
		sub			eax, 1
		jnz			xtraloop

		pop			ebx
fin:
		ret			4
	}
}

void __declspec(naked) __fastcall vdasm_extract_8in32_MMX(void *dst, const void *src, uint32 count, int byteshift) {
	__asm {
		movd		mm4, [esp+8]
		pcmpeqb		mm5, mm5
		pslld		mm4, 3
		mov			eax, [esp+4]
		psrld		mm5, 24
		sub			eax, 8
		jc			xtra
xloop:
		movq		mm0, [edx]
		movq		mm1, [edx+8]
		psrld		mm0, mm4
		movq		mm2, [edx+16]
		psrld		mm1, mm4
		pand		mm0, mm5
		movq		mm3, [edx+24]
		psrld		mm2, mm4
		pand		mm1, mm5
		packssdw	mm0, mm1
		psrld		mm3, mm4
		pand		mm2, mm5
		pand		mm3, mm5
		add			edx, 32
		packssdw	mm2, mm3
		packuswb	mm0, mm2
		movq		[ecx], mm0
		add			ecx, 8
		sub			eax, 8
		jns			xloop
xtra:
		add			eax, 8
		jz			fin
		add			edx, dword ptr [esp+8]
		push		ebx
xtraloop:
		mov			bl, [edx]
		add			edx, 4
		mov			[ecx], bl
		add			ecx, 1
		sub			eax, 1
		jnz			xtraloop

		pop			ebx
fin:
		ret			8
	}
}

void __declspec(naked) __fastcall vdasm_swap_8in16_MMX(void *dst, const void *src, uint32 count) {
	__asm {
		mov			eax, [esp+4]
		sub			eax, 8
		js			xtra
xloop:
		movq		mm0, [edx]
		add			edx, 8
		movq		mm1, mm0
		psllw		mm0, 8
		psrlw		mm1, 8
		paddb		mm0, mm1
		movq		[ecx], mm0
		add			ecx, 8
		sub			eax, 8
		jns			xloop
xtra:
		add			eax, 6
		js			nopairs
		push		ebx
pairloop:
		mov			bl, [edx]
		mov			bh, [edx+1]
		add			edx, 2
		mov			[ecx], bh
		mov			[ecx+1], bl
		add			ecx, 2
		sub			eax, 2
		jns			pairloop
		pop			ebx
nopairs:
		add			eax, 2
		jz			noodd
		mov			al, [edx]
		mov			[ecx], al
noodd:
		ret			4
	}
}

void __declspec(naked) __fastcall vdasm_interleave_BGRG_MMX(void *dst, const void *srcR, const void *srcG, const void *srcB, uint32 count) {
	__asm {
		push		edi
		push		esi
		push		ebx
		mov			esi, [esp+12+12]
		mov			edi, [esp+8+12]
		mov			ebx, [esp+4+12]
		sub			esi, 4
		jc			xtra
		; ecx = dst
		; edx = srcR
		; ebx = srcG
		; edi = srcB
xloop:
		movd		mm0, [edi]
		movd		mm1, [edx]
		punpcklbw	mm0, mm1
		movq		mm1, [ebx]
		movq		mm2, mm0
		punpcklbw	mm0, mm1
		add			edx, 4
		punpckhbw	mm2, mm1
		add			edi, 4
		movq		[ecx], mm0
		add			ebx, 8
		movq		[ecx+8], mm2
		add			ecx, 16
		sub			esi, 4
		jns			xloop
xtra:
		add			esi, 4
		jz			fin
xtraloop:
		mov			al, [edi]
		mov			[ecx], al
		mov			al, [ebx]
		mov			[ecx+1], al
		mov			al, [edx]
		mov			[ecx+2], al
		mov			al, [ebx+1]
		mov			[ecx+3], al
		add			ebx, 2
		add			edx, 1
		add			edi, 1
		add			ecx, 4
		sub			esi, 1
		jnz			xtraloop
fin:
		pop			ebx
		pop			esi
		pop			edi
		ret			12
	}
}

void __declspec(naked) __fastcall vdasm_interleave_GBGR_MMX(void *dst, const void *srcR, const void *srcG, const void *srcB, uint32 count) {
	__asm {
		push		edi
		push		esi
		push		ebx
		mov			esi, [esp+12+12]
		mov			edi, [esp+8+12]
		mov			ebx, [esp+4+12]
		sub			esi, 4
		jc			xtra
		; ecx = dst
		; edx = srcR
		; ebx = srcG
		; edi = srcB
xloop:
		movd		mm0, [edi]
		movd		mm1, [edx]
		punpcklbw	mm0, mm1
		movq		mm2, [ebx]
		movq		mm1, mm2
		punpcklbw	mm2, mm0
		add			edx, 4
		punpckhbw	mm1, mm0
		add			edi, 4
		movq		[ecx], mm2
		add			ebx, 8
		movq		[ecx+8], mm1
		add			ecx, 16
		sub			esi, 4
		jns			xloop
xtra:
		add			esi, 4
		jz			fin
xtraloop:
		mov			al, [ebx]
		mov			[ecx], al
		mov			al, [edi]
		mov			[ecx+1], al
		mov			al, [ebx+1]
		mov			[ecx+2], al
		mov			al, [edx]
		mov			[ecx+3], al
		add			ebx, 2
		add			edx, 1
		add			edi, 1
		add			ecx, 4
		sub			esi, 1
		jnz			xtraloop
fin:
		pop			ebx
		pop			esi
		pop			edi
		ret			12
	}
}

void __declspec(naked) __fastcall vdasm_interleave_BR_MMX(void *dst, const void *srcB, const void *srcR, uint32 count) {
	__asm {
		push		edi
		push		esi
		push		ebx
		mov			esi, [esp+8+12]
		mov			ebx, [esp+4+12]
		sub			esi, 8
		jc			xtra
		; ecx = dst
		; edx = srcB
		; ebx = srcG
xloop:
		movq		mm0, [edx]
		movq		mm1, [ebx]
		movq		mm2, mm0
		punpcklbw	mm0, mm1
		punpckhbw	mm2, mm1
		add			edx, 8
		movq		[ecx], mm0
		add			ebx, 8
		movq		[ecx+8], mm2
		add			ecx, 16
		sub			esi, 8
		jns			xloop
xtra:
		add			esi, 8
		jz			fin
xtraloop:
		mov			al, [edx]
		mov			[ecx], al
		mov			al, [ebx]
		mov			[ecx+1], al
		add			ebx, 1
		add			edx, 1
		add			ecx, 2
		sub			esi, 1
		jnz			xtraloop
fin:
		pop			ebx
		pop			esi
		pop			edi
		ret			8
	}
}

///////////////////////////////////////////////////////////////////////////////

void VDPixmapGen_8In16_Even_MMX::Compute(void *dst, sint32 y) {
	const uint8 *srcp = (const uint8 *)mpSrc->GetRow(y, mSrcIndex);

	vdasm_extract_8in16_even_MMX(dst, srcp, mWidth);
}

void VDPixmapGen_8In16_Odd_MMX::Compute(void *dst, sint32 y) {
	const uint8 *srcp = (const uint8 *)mpSrc->GetRow(y, mSrcIndex);

	vdasm_extract_8in16_odd_MMX(dst, srcp, mWidth);
}

void VDPixmapGen_8In32_MMX::Compute(void *dst, sint32 y) {
	const uint8 *srcp = (const uint8 *)mpSrc->GetRow(y, mSrcIndex);

	vdasm_extract_8in32_MMX(dst, srcp, mWidth, mOffset);
}

void VDPixmapGen_Swap8In16_MMX::Compute(void *dst, sint32 y) {
	const uint8 *src = (const uint8 *)mpSrc->GetRow(y, mSrcIndex);

	vdasm_swap_8in16_MMX(dst, src, mRowLength);
}

void VDPixmapGen_B8x2_To_B8R8_MMX::Compute(void *dst0, sint32 y) {
	uint8 *VDRESTRICT dst = (uint8 *VDRESTRICT)dst0;
	const uint8 *VDRESTRICT srcCb = (const uint8 *VDRESTRICT)mpSrcCb->GetRow(y, mSrcIndexCb);
	const uint8 *VDRESTRICT srcCr = (const uint8 *VDRESTRICT)mpSrcCr->GetRow(y, mSrcIndexCr);

	vdasm_interleave_BR_MMX(dst, srcCb, srcCr, mWidth);
}

void VDPixmapGen_B8x3_To_G8B8_G8R8_MMX::Compute(void *VDRESTRICT dst0, sint32 y) {
	uint8 *VDRESTRICT dst = (uint8 *VDRESTRICT)dst0;
	const uint8 *VDRESTRICT srcY = (const uint8 *VDRESTRICT)mpSrcY->GetRow(y, mSrcIndexY);
	const uint8 *VDRESTRICT srcCb = (const uint8 *VDRESTRICT)mpSrcCb->GetRow(y, mSrcIndexCb);
	const uint8 *VDRESTRICT srcCr = (const uint8 *VDRESTRICT)mpSrcCr->GetRow(y, mSrcIndexCr);

	vdasm_interleave_GBGR_MMX(dst, srcCr, srcY, srcCb, mWidth >> 1);

	if (mWidth & 1) {
		int w2 = mWidth >> 1;
		srcY += mWidth;
		srcCb += w2;
		srcCr += w2;
		dst += mWidth * 2;

		dst[-2] = srcY[-1];
		dst[-1] = srcCb[0];
		dst[ 0] = 0;			// must be zero for QuickTime compatibility
		dst[ 1] = srcCr[0];
	}
}

void VDPixmapGen_B8x3_To_B8G8_R8G8_MMX::Compute(void *VDRESTRICT dst0, sint32 y) {
	uint8 *VDRESTRICT dst = (uint8 *VDRESTRICT)dst0;
	const uint8 *VDRESTRICT srcY = (const uint8 * VDRESTRICT)mpSrcY->GetRow(y, mSrcIndexY);
	const uint8 *VDRESTRICT srcCb = (const uint8 * VDRESTRICT)mpSrcCb->GetRow(y, mSrcIndexCb);
	const uint8 *VDRESTRICT srcCr = (const uint8 * VDRESTRICT)mpSrcCr->GetRow(y, mSrcIndexCr);

	vdasm_interleave_BGRG_MMX(dst, srcCr, srcY, srcCb, mWidth >> 1);

	if (mWidth & 1) {
		int w2 = mWidth >> 1;
		srcY += mWidth;
		srcCb += w2;
		srcCr += w2;
		dst += mWidth * 2;

		dst[-2] = srcCb[0];
		dst[-1] = srcY[-1];
		dst[ 0] = srcCr[0];
		dst[ 1] = 0;			// must be zero for QuickTime compatibility
	}
}
