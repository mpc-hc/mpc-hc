		segment	.rdata, align=16

correct		dq			0000800000008000h
round		dq			0000200000002000h
round1		dq			0000020000000200h
round2		dq			0002000000020000h

		segment	.text

		%include	"a_triblt.inc"

		extern		_kVDCubicInterpTableFX14_075_MMX

;--------------------------------------------------------------------------
	global	_vdasm_triblt_span_bilinear_mmx
_vdasm_triblt_span_bilinear_mmx:
		push		ebp
		push		edi
		push		esi
		push		ebx
		mov			edi,[esp+4+16]
		mov			edx,[edi+texinfo.dst]
		mov			ebp,[edi+texinfo.w]
		shl			ebp,2
		mov			ebx,[edi+texinfo.mips+mipmap.bits]
		add			edx,ebp
		mov			esi,[edi+texinfo.mips+mipmap.pitch]
		neg			ebp
		movd		mm6,[edi+texinfo.mips+mipmap.uvmul]
		pxor		mm7,mm7
		mov			edi,[edi+texinfo.src]
.xloop:
		movq		mm4,[edi]
		movq		mm0,mm4
		psrld		mm0,16
		movq		mm5,mm4
		packssdw	mm0,mm0
		pmaddwd		mm0,mm6
		add			edi,8
		punpcklwd	mm4,mm4
		punpckldq	mm4,mm4
		movd		ecx,mm0
		add			ecx,ebx
		psrlw		mm4,1
		movd		mm0,dword [ecx]
		movd		mm1,dword [ecx+4]
		punpcklbw	mm0,mm7
		movd		mm2,dword [ecx+esi]
		punpcklbw	mm1,mm7
		movd		mm3,dword [ecx+esi+4]
		punpcklbw	mm2,mm7
		punpcklbw	mm3,mm7
		psubw		mm1,mm0
		psubw		mm3,mm2
		paddw		mm1,mm1
		paddw		mm3,mm3
		pmulhw		mm1,mm4
		pmulhw		mm3,mm4
		punpckhwd	mm5,mm5
		punpckldq	mm5,mm5
		paddw		mm0,mm1
		psrlw		mm5,1
		paddw		mm2,mm3
		psubw		mm2,mm0
		paddw		mm2,mm2
		pmulhw		mm2,mm5
		paddw		mm0,mm2
		packuswb	mm0,mm0
		movd		dword [edx+ebp],mm0
		add			ebp,4
		jnc			.xloop
		pop			ebx
		pop			esi
		pop			edi
		pop			ebp
		emms
		ret
		
;--------------------------------------------------------------------------
	global	_vdasm_triblt_span_trilinear_mmx
_vdasm_triblt_span_trilinear_mmx:
		push		ebp
		push		edi
		push		esi
		push		ebx
		mov			esi,[esp+4+16]
		mov			edx,[esi+texinfo.dst]
		mov			ebp,[esi+texinfo.w]
		shl			ebp,2
		add			edx,ebp
		neg			ebp
		mov			edi,[esi+texinfo.src]
		pxor		mm7,mm7
.xloop:
		movd		mm6,[edi+mipspan.u]
		punpckldq	mm6,[edi+mipspan.v]
		mov			eax,[edi+mipspan.lambda]
		shr			eax,4
		and			eax,byte -16
		movd		mm2,eax
		psrlq		mm2,4
		psrld		mm6,mm2
		paddd		mm6,[correct]

		;fetch mipmap 1
		mov			ebx,[esi+eax+mipmap.pitch]
		movd		mm1,[esi+eax+mipmap.uvmul]
		movq		mm4,mm6
		movq		mm0,mm6
		psrld		mm0,16
		packssdw	mm0,mm0
		pmaddwd		mm0,mm1
		movq		mm5,mm4
		punpcklwd	mm4,mm4
		punpckldq	mm4,mm4
		punpckhwd	mm5,mm5
		punpckldq	mm5,mm5
		movd		ecx,mm0
		add			ecx,[esi+eax+mipmap.bits]
		psrlw		mm4,1
		movd		mm0,dword [ecx]
		movd		mm1,dword [ecx+4]
		punpcklbw	mm0,mm7
		movd		mm2,dword [ecx+ebx]
		punpcklbw	mm1,mm7
		movd		mm3,dword [ecx+ebx+4]
		punpcklbw	mm2,mm7
		punpcklbw	mm3,mm7
		psubw		mm1,mm0
		psubw		mm3,mm2
		paddw		mm1,mm1
		paddw		mm3,mm3
		pmulhw		mm1,mm4
		pmulhw		mm3,mm4
		paddw		mm0,mm1
		psrlw		mm5,1
		paddw		mm2,mm3
		psubw		mm2,mm0
		paddw		mm2,mm2
		pmulhw		mm2,mm5
		paddw		mm0,mm2

		;fetch mipmap 2
		mov			ebx,[esi+eax+16+mipmap.pitch]
		movd		mm1,[esi+eax+16+mipmap.uvmul]
		paddd		mm6,[correct]
		psrld		mm6,1
		movq		mm4,mm6
		psrld		mm6,16
		packssdw	mm6,mm6
		pmaddwd		mm6,mm1
		movq		mm5,mm4
		punpcklwd	mm4,mm4
		punpckldq	mm4,mm4
		punpckhwd	mm5,mm5
		punpckldq	mm5,mm5
		movd		ecx,mm6
		add			ecx,[esi+eax+16+mipmap.bits]
		psrlw		mm4,1
		movd		mm6,dword [ecx]
		movd		mm1,dword [ecx+4]
		punpcklbw	mm6,mm7
		movd		mm2,dword [ecx+ebx]
		punpcklbw	mm1,mm7
		movd		mm3,dword [ecx+ebx+4]
		punpcklbw	mm2,mm7
		punpcklbw	mm3,mm7
		psubw		mm1,mm6
		psubw		mm3,mm2
		paddw		mm1,mm1
		paddw		mm3,mm3
		pmulhw		mm1,mm4
		pmulhw		mm3,mm4
		paddw		mm6,mm1
		psrlw		mm5,1
		paddw		mm2,mm3
		psubw		mm2,mm6
		paddw		mm2,mm2
		pmulhw		mm2,mm5
		paddw		mm6,mm2

		;blend mips
		movd		mm1,[edi+mipspan.lambda]
		punpcklwd	mm1,mm1
		punpckldq	mm1,mm1
		psllw		mm1,8
		psrlq		mm1,1
		psubw		mm6,mm0
		paddw		mm6,mm6
		pmulhw		mm6,mm1
		paddw		mm0,mm6
		packuswb	mm0,mm0

		movd		dword [edx+ebp],mm0
		add			edi, mipspan_size
		add			ebp,4
		jnc			.xloop
		pop			ebx
		pop			esi
		pop			edi
		pop			ebp
		emms
		ret

;--------------------------------------------------------------------------
%macro .SETUPADDR 1
		;compute mipmap index and UV
		movd		mm0, [edi + mipspan.u]
		punpckldq	mm0, [edi + mipspan.v]
		mov			ebx, [edi + mipspan.lambda]
		shr			ebx, 4
		and			ebx, byte -16
		
		add			ebx, mipmap_size*%1
		movd		mm2, ebx
		add			ebx, [esp + .af_mipbase]
		psrlq		mm2, 4
		psrad		mm0, mm2
		paddd		mm0, [correct]
		movq		mm1, mm0
		psrlq		mm1, 32

		;compute horizontal filters
		movd		ecx, mm0
		shr			ecx, 4
		and			ecx, 0ff0h
		add			ecx, _kVDCubicInterpTableFX14_075_MMX
		
		;compute vertical filter
		movd		edx, mm1
		and			edx, 0ff00h
		shr			edx, 4
		add			edx, _kVDCubicInterpTableFX14_075_MMX

		;compute texel address
		movd		mm1, [ebx + mipmap.uvmul]
		psrld		mm0, 16
		packssdw	mm0, mm0
		pmaddwd		mm0, mm1
		movd		eax, mm0
		add			eax, [ebx + mipmap.bits]
%endmacro
		
%macro .HCUBIC 4
		movd		%1, dword [eax]
		punpcklbw	%1, qword [eax+4]
		movd		%3, dword [eax+8]
		punpcklbw	%3, qword [eax+12]
		movq		%2, %1
		movq		%4, %3
		punpcklbw	%1, mm7
		pmaddwd		%1, [ecx]
		punpcklbw	%3, mm7
		pmaddwd		%3, [ecx+8]
		punpckhbw	%2, mm7
		pmaddwd		%2, [ecx]
		punpckhbw	%4, mm7
		pmaddwd		%4, [ecx+8]
		paddd		%1, %3
		paddd		%2, %4
%endmacro

%macro	.VCUBIC		1
		.HCUBIC		mm0, mm1, mm2, mm3
		add			eax, %1

		.HCUBIC		mm4, mm5, mm2, mm3
		add			eax, %1
		
		movq		mm2, [round1]
		
		paddd		mm0, mm2
		paddd		mm1, mm2
		paddd		mm4, mm2
		paddd		mm5, mm2

		psrad		mm0, 10
		psrad		mm1, 10
		psrad		mm4, 10
		psrad		mm5, 10
		
		packssdw	mm0, mm0
		packssdw	mm1, mm1
		packssdw	mm4, mm4
		packssdw	mm5, mm5
				
		punpcklwd	mm0, mm4
		punpcklwd	mm1, mm5
		
		movq		mm3, [edx]
		
		pmaddwd		mm0, mm3
		pmaddwd		mm1, mm3
		
		movq		[esp + .af_htemp0], mm0
		movq		[esp + .af_htemp1], mm1
		
		.HCUBIC		mm0, mm1, mm2, mm3
		add			eax, %1
		.HCUBIC		mm4, mm5, mm2, mm3

		movq		mm2, [round1]
		
		paddd		mm0, mm2
		paddd		mm1, mm2
		paddd		mm4, mm2
		paddd		mm5, mm2

		psrad		mm0, 10
		psrad		mm1, 10
		psrad		mm4, 10
		psrad		mm5, 10
		
		packssdw	mm0, mm0
		packssdw	mm1, mm1
		packssdw	mm4, mm4
		packssdw	mm5, mm5
				
		punpcklwd	mm0, mm4
		punpcklwd	mm1, mm5

		movq		mm2, [round2]		
		movq		mm3, [edx + 8]
		
		pmaddwd		mm0, mm3
		pmaddwd		mm1, mm3
		
		paddd		mm0, [esp + .af_htemp0]
		paddd		mm1, [esp + .af_htemp1]
		
		paddd		mm0, mm2
		paddd		mm1, mm2
		
		psrad		mm0, 18
		psrad		mm1, 18
		packssdw	mm0, mm1
%endmacro

	global	_vdasm_triblt_span_bicubic_mip_linear_mmx
_vdasm_triblt_span_bicubic_mip_linear_mmx:

;parameters
%define .p_texinfo	20

;aligned frame
%define .af_htemp0	0
%define .af_htemp1	8
%define .af_vtemp0	16
%define .af_mipbase	24
%define	.af_prevesp	28
%define .afsize		32

		push		ebp
		lea			ebp, [esp-12]
		push		edi
		push		esi
		push		ebx
		
		sub			esp, .afsize
		and			esp, -8
		
		mov			[esp + .af_prevesp], ebp
		
		mov			ebx, [ebp + .p_texinfo]
		mov			ebp, [ebx + texinfo.dst]
		mov			esi, [ebx + texinfo.w]
		shl			esi, 2
		add			ebp,esi
		neg			esi

		mov			edi, [ebx + texinfo.src]
		mov			[esp + .af_mipbase], ebx
		pxor		mm7, mm7

.xloop:

		;registers:
		;	eax		base texel address
		;	ebx		first mip info
		;	ecx		horizontal filter
		;	edx		vertical filter
		;	esi		horizontal count
		;	edi		mipspan
		;	ebp		destination

		;fetch mipmap 1
		.SETUPADDR	0
		.VCUBIC		[ebx+mipmap.pitch]
		
		movq		[esp + .af_vtemp0], mm0

		;fetch mipmap 2		
		.SETUPADDR	1
		.VCUBIC		[ebx+mipmap.pitch]
		
		;blend mips
		movq		mm1, [esp + .af_vtemp0]
		
		psubw		mm0, mm1

		movd		mm3,[edi+mipspan.lambda]
		punpcklwd	mm3,mm3
		punpckldq	mm3,mm3
		psllw		mm3,8
		psrlq		mm3,1
		
		paddw		mm0,mm0
		pmulhw		mm0,mm3
		paddw		mm0,mm1
		packuswb	mm0,mm0

		movd		dword [ebp+esi],mm0
		add			edi, mipspan_size
		add			esi,4
		jnc			.xloop

		mov			esp, [esp + .af_prevesp]
		pop			ebx
		pop			esi
		pop			edi
		pop			ebp
		emms
		ret

		end
