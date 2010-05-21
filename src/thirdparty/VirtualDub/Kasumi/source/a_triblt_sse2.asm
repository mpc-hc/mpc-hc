		segment	.rdata, align=16

correct		dq			0000800000008000h, 0000800000008000h
round		dq			0000200000002000h, 0000200000002000h
round1		dq			0000020000000200h, 0000020000000200h
round2		dq			0002000000020000h, 0002000000020000h

		segment	.text

		%include	"a_triblt.inc"

		extern		_kVDCubicInterpTableFX14_075_MMX

;--------------------------------------------------------------------------
	global	_vdasm_triblt_span_bicubic_mip_linear_sse2
_vdasm_triblt_span_bicubic_mip_linear_sse2:

;parameters
%define .p_texinfo	20

;aligned frame
%define .af_vtemp0	0
%define .af_mipbase	16
%define	.af_prevesp	20
%define .afsize		24

		push		ebp
		lea			ebp, [esp-12]
		push		edi
		push		esi
		push		ebx
		
		sub			esp, .afsize
		and			esp, -16
		
		mov			[esp + .af_prevesp], ebp
		
		mov			ebx, [ebp + .p_texinfo]
		mov			ebp, [ebx + texinfo.dst]
		mov			esi, [ebx + texinfo.w]
		shl			esi, 2
		add			ebp,esi
		neg			esi

		mov			edi, [ebx + texinfo.src]
		mov			[esp + .af_mipbase], ebx
		pxor		xmm7, xmm7

.xloop:

		;registers:
		;	eax		base texel address
		;	ebx		first mip info
		;	ecx		horizontal filter
		;	edx		vertical filter
		;	esi		horizontal count
		;	edi		mipspan
		;	ebp		destination

%macro .SETUPADDR 1
		;compute mipmap index and UV
		movd		xmm0, [edi + mipspan.u]
		movd		xmm1, [edi + mipspan.v]
		punpckldq	xmm0, xmm1
		mov			ebx, [edi + mipspan.lambda]
		shr			ebx, 4
		and			ebx, byte -16
		
		add			ebx, mipmap_size*%1
		movd		xmm2, ebx
		add			ebx, [esp + .af_mipbase]
		psrlq		xmm2, 4
		psrad		xmm0, xmm2
		paddd		xmm0, [correct]
		pshufd		xmm1, xmm0, 01010101b

		;compute horizontal filters
		movd		ecx, xmm0
		shr			ecx, 4
		and			ecx, 0ff0h
		add			ecx, _kVDCubicInterpTableFX14_075_MMX
		
		;compute vertical filter
		movd		edx, xmm1
		and			edx, 0ff00h
		shr			edx, 4
		add			edx, _kVDCubicInterpTableFX14_075_MMX

		;compute texel address
		movd		xmm1, [ebx + mipmap.uvmul]
		psrld		xmm0, 16
		packssdw	xmm0, xmm0
		pmaddwd		xmm0, xmm1
		movd		eax, xmm0
		add			eax, [ebx + mipmap.bits]
%endmacro
		
%macro .HCUBIC 4
		movd		%1, dword [eax]
		movd		%3, dword [eax+4]
		movd		%2, dword [eax+8]
		movd		%4, dword [eax+12]		
		punpcklbw	%1, %3
		punpcklbw	%2, %4
		punpcklbw	%1, xmm7
		punpcklbw	%2, xmm7
		movdqa		%3, [ecx]
		pshufd		%4, %3, 11101110b
		pshufd		%3, %3, 01000100b
		pmaddwd		%1, %3
		pmaddwd		%2, %4
		paddd		%1, %2
%endmacro

%macro	.VCUBIC		1
		.HCUBIC		xmm0, xmm4, xmm5, xmm6
		add			eax, %1		
		.HCUBIC		xmm1, xmm4, xmm5, xmm6
		add			eax, %1
		.HCUBIC		xmm2, xmm4, xmm5, xmm6
		add			eax, %1		
		.HCUBIC		xmm3, xmm4, xmm5, xmm6
		
		movq		xmm4, [round1]
		
		paddd		xmm0, xmm4
		
		paddd		xmm1, xmm4
		psrad		xmm0, 10
		
		paddd		xmm2, xmm4
		psrad		xmm1, 10
		packssdw	xmm0, xmm0
		
		paddd		xmm3, xmm4
		psrad		xmm2, 10
		packssdw	xmm1, xmm1

		movdqa		xmm5, [edx]
		psrad		xmm3, 10		
		punpcklwd	xmm0, xmm1

		packssdw	xmm2, xmm2
		packssdw	xmm3, xmm3
		pshufd		xmm4, xmm5, 01000100b				

		pmaddwd		xmm0, xmm4
		punpcklwd	xmm2, xmm3

		pshufd		xmm5, xmm5, 11101110b
		
		pmaddwd		xmm2, xmm5
		paddd		xmm0, xmm2
		paddd		xmm0, [round2]
		psrad		xmm0, 18

		packssdw	xmm0, xmm0
%endmacro

		;fetch mipmap 1
		.SETUPADDR	0
		.VCUBIC		[ebx+mipmap.pitch]
		
		movq		[esp + .af_vtemp0], xmm0

		;fetch mipmap 2		
		.SETUPADDR	1
		.VCUBIC		[ebx+mipmap.pitch]
		
		;blend mips
		movq		xmm1, [esp + .af_vtemp0]
		
		psubw		xmm0, xmm1

		movd		xmm3, [edi+mipspan.lambda]
		pshuflw		xmm3, xmm3, 0
		psllw		xmm3, 8
		psrlq		xmm3, 1
		
		paddw		xmm0, xmm0
		pmulhw		xmm0, xmm3
		paddw		xmm0, xmm1
		packuswb	xmm0, xmm0

		movd		dword [ebp+esi], xmm0
		add			edi, mipspan_size
		add			esi,4
		jnc			.xloop

		mov			esp, [esp + .af_prevesp]
		pop			ebx
		pop			esi
		pop			edi
		pop			ebp
		ret

		end
