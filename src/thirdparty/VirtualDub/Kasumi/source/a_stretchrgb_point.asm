		segment	.text

		struc	scaleinfo
.dst		resd	1
.src		resd	1
.xaccum		resd	1
.xfracinc	resd	1
.xintinc	resd	1
.count		resd	1
		endstruc

		global	_vdasm_resize_point32
_vdasm_resize_point32:
		push	ebp
		push	edi
		push	esi
		push	ebx

		mov		eax, [esp+4+16]

		mov		ebx, [eax+scaleinfo.xaccum]
		mov		ecx, [eax+scaleinfo.xfracinc]
		mov		edx, [eax+scaleinfo.src]
		mov		esi, [eax+scaleinfo.xintinc]
		mov		edi, [eax+scaleinfo.dst]
		mov		ebp, [eax+scaleinfo.count]
.xloop:
		mov		eax,[edx*4]
		add		ebx,ecx
		adc		edx,esi
		mov		[edi+ebp],eax
		add		ebp,4
		jne		.xloop

		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret

		global	_vdasm_resize_point32_MMX
_vdasm_resize_point32_MMX:
		push	ebp
		push	edi
		push	esi
		push	ebx

		mov		eax, [esp+4+16]

		push	0
		push	dword [fs:0]
		mov		dword [fs:0], esp

		mov		ebx, [eax+scaleinfo.xaccum]
		mov		esp, [eax+scaleinfo.xfracinc]
		mov		edx, [eax+scaleinfo.src]
		mov		esi, [eax+scaleinfo.xintinc]
		mov		edi, [eax+scaleinfo.dst]
		mov		ebp, [eax+scaleinfo.count]

		mov		eax, ebx
		mov		ecx, edx
		add		ebx, esp
		adc		edx, esi
		add		esp, esp
		adc		esi, esi

		add		ebp, 4
		jz		.odd
.dualloop:
		movd		mm0, dword [ecx*4]
		punpckldq	mm0,[edx*4]
		add		eax,esp
		adc		ecx,esi
		add		ebx,esp
		adc		edx,esi
		movq	[edi+ebp-4],mm0

		add		ebp,8
		jnc		.dualloop
		jnz		.noodd
.odd:
		mov		eax, [ecx*4]
		mov		[edi-4], eax
.noodd:
		mov		esp, dword [fs:0]
		pop		eax
		pop		eax

		pop		ebx
		pop		esi
		pop		edi
		pop		ebp
		ret

		end
