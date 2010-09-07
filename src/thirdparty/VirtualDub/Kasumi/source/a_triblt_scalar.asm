		segment	.text

		%include	"a_triblt.inc"

	global	_vdasm_triblt_span_point
_vdasm_triblt_span_point:
		push	ebp
		push	edi
		push	esi
		push	ebx
		mov		eax,[esp+4+16]
		mov		ebp,[eax+texinfo.w]
		mov		ebx,[eax+texinfo.mips+mipmap.pitch]
		shl		ebp,2
		mov		edi,[eax+texinfo.src]
		mov		edx,[eax+texinfo.dst]
		mov		ecx,[eax+texinfo.mips+mipmap.bits]
		sar		ebx,2
		add		edx,ebp
		neg		ebp
.xloop:
		mov		eax,[edi+span.v]
		imul	eax,ebx
		add		eax,[edi+span.u]
		add		edi,8
		mov		eax,[ecx+eax*4]
		mov		[edx+ebp],eax
		add		ebp,4
		jnc		.xloop
		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret

		end