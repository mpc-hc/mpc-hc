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

#ifndef f_VD2_SYSTEM_TEXT_H
#define f_VD2_SYSTEM_TEXT_H

#include <ctype.h>
#include <stdarg.h>

class VDStringA;
class VDStringW;

// The max_dst value needs to include space for the NULL as well.  The number
// of characters produced is returned, minus the null terminator.

int VDTextWToA(char *dst, int max_dst, const wchar_t *src, int max_src = -1);
int VDTextAToW(wchar_t *dst, int max_dst, const char *src, int max_src = -1);

VDStringA VDTextWToA(const wchar_t *src, int length = -1);
VDStringA VDTextWToA(const VDStringW& sw);
VDStringW VDTextAToW(const char *src, int length = -1);
VDStringW VDTextAToW(const VDStringA& sw);

VDStringA VDTextWToU8(const VDStringW& s);
VDStringA VDTextWToU8(const wchar_t *s, int length);
VDStringW VDTextU8ToW(const VDStringA& s);
VDStringW VDTextU8ToW(const char *s, int length);

// The terminating NULL character is not included in these.

int VDTextWToALength(const wchar_t *s, int length=-1);
int VDTextAToWLength(const char *s, int length=-1);

VDStringW VDaswprintf(const wchar_t *format, int args, const void *const *argv);
VDStringW VDvswprintf(const wchar_t *format, int args, va_list val);
VDStringW VDswprintf(const wchar_t *format, int args, ...);

#endif
