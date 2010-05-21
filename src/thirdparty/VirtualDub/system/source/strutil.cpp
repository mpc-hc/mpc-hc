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
#include <string.h>
#include <ctype.h>

#include <vd2/system/strutil.h>

char *strncpyz(char *strDest, const char *strSource, size_t count) {
	char *s;

	s = strncpy(strDest, strSource, count);
	strDest[count-1] = 0;

	return s;
}

wchar_t *wcsncpyz(wchar_t *strDest, const wchar_t *strSource, size_t count) {
	wchar_t *s;

	s = wcsncpy(strDest, strSource, count);
	strDest[count-1] = 0;

	return s;
}

const char *strskipspace(const char *s) {
	while(isspace((unsigned char)*s++))
		;

	return s-1;
}

size_t vdstrlcpy(char *dst, const char *src, size_t size) {
	size_t len = strlen(src);

	if (size) {
		if (size > len)
			size = len;

		memcpy(dst, src, size);
		dst[size] = 0;
	}
	return len;
}

size_t vdwcslcpy(wchar_t *dst, const wchar_t *src, size_t size) {
	size_t len = wcslen(src);

	if (size) {
		if (size > len)
			size = len;

		memcpy(dst, src, size * sizeof(wchar_t));
		dst[size] = 0;
	}
	return len;
}

size_t vdstrlcat(char *dst, const char *src, size_t size) {
	size_t dlen = strlen(dst);
	size_t slen = strlen(src);

	if (dlen < size) {
		size_t maxappend = size - dlen - 1;
		if (maxappend > slen)
			maxappend = slen;

		if (maxappend) {
			memcpy(dst + dlen, src, maxappend);
			dst[dlen+maxappend] = 0;
		}
	}

	return dlen+slen;
}
