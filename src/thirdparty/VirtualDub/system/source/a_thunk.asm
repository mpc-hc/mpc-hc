		segment	.text
		
		align		16
		global		_VDMethodToFunctionThunk32		
_VDMethodToFunctionThunk32:
		pop			eax					;get return address in thunk
		
		;re-copy arguments
		movzx		ecx, byte [eax+1]
		mov			edx, ecx
argsloop:
		push		dword [esp+edx]
		sub			ecx, 4
		jnz			argsloop

		push		eax					;replace thunk return address
		
		mov			ecx, [eax+7]		;load 'this' pointer
		jmp			dword [eax+3]	;tail-call function

		align		16
		global		_VDMethodToFunctionThunk32_4
_VDMethodToFunctionThunk32_4:
		pop			eax					;get return address in thunk
		push		dword [esp+4]		;replicate 1st argument
		push		eax					;replace thunk return address
		mov			ecx, [eax+7]		;load 'this' pointer
		jmp			dword [eax+3]		;tail-call function

		align		16
		global		_VDMethodToFunctionThunk32_8
_VDMethodToFunctionThunk32_8:
		pop			eax					;get return address in thunk
		push		dword [esp+8]		;replicate 2nd argument
		push		dword [esp+8]		;replicate 1st argument
		push		eax					;replace thunk return address
		mov			ecx, [eax+7]		;load 'this' pointer
		jmp			dword [eax+3]		;tail-call function

		align		16
		global		_VDMethodToFunctionThunk32_12
_VDMethodToFunctionThunk32_12:
		pop			eax					;get return address in thunk
		push		dword [esp+12]		;replicate 3rd argument
		push		dword [esp+12]		;replicate 2nd argument
		push		dword [esp+12]		;replicate 1st argument
		push		eax					;replace thunk return address
		mov			ecx, [eax+7]		;load 'this' pointer
		jmp			dword [eax+3]		;tail-call function

		align		16
		global		_VDMethodToFunctionThunk32_16
_VDMethodToFunctionThunk32_16:
		pop			eax					;get return address in thunk
		push		dword [esp+16]		;replicate 4th argument
		push		dword [esp+16]		;replicate 3rd argument
		push		dword [esp+16]		;replicate 2nd argument
		push		dword [esp+16]		;replicate 1st argument
		push		eax					;replace thunk return address
		mov			ecx, [eax+7]		;load 'this' pointer
		jmp			dword [eax+3]		;tail-call function

		end
