//	VirtualDub - Video processing and capture application
//	System library component
//	Copyright (C) 1998-2010 Avery Lee, All Rights Reserved.
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

#include <stdafx.h>
#include <vd2/system/hash.h>
#include <vd2/system/VDString.h>
#include <vd2/system/vdstl_hash.h>

size_t vdhash<VDStringA>::operator()(const VDStringA& s) const {
	return VDHashString32(s.data(), s.length());
}

size_t vdhash<VDStringA>::operator()(const char *s) const {
	return VDHashString32(s, strlen(s));
}

size_t vdhash<VDStringW>::operator()(const VDStringW& s) const {
	return VDHashString32(s.data(), s.length());
}

size_t vdhash<VDStringW>::operator()(const wchar_t *s) const {
	return VDHashString32(s, wcslen(s));
}

size_t vdstringhashi::operator()(const VDStringA& s) const {
	return VDHashString32I(s.data(), s.length());
}

size_t vdstringhashi::operator()(const char *s) const {
	return VDHashString32I(s);
}

size_t vdstringhashi::operator()(const VDStringW& s) const {
	return VDHashString32I(s.data(), s.length());
}

size_t vdstringhashi::operator()(const wchar_t *s) const {
	return VDHashString32I(s);
}

bool vdstringpred::operator()(const VDStringA& s, const VDStringA& t) const {
	return s == t;
}

bool vdstringpred::operator()(const VDStringA& s, const VDStringSpanA& t) const {
	return s == t;
}

bool vdstringpred::operator()(const VDStringA& s, const char *t) const {
	return s == t;
}

bool vdstringpred::operator()(const VDStringW& s, const VDStringW& t) const {
	return s == t;
}

bool vdstringpred::operator()(const VDStringW& s, const VDStringSpanW& t) const {
	return s == t;
}

bool vdstringpred::operator()(const VDStringW& s, const wchar_t *t) const {
	return s == t;
}

bool vdstringpredi::operator()(const VDStringA& s, const VDStringA& t) const {
	return s.comparei(t) == 0;
}

bool vdstringpredi::operator()(const VDStringA& s, const VDStringSpanA& t) const {
	return s.comparei(t) == 0;
}

bool vdstringpredi::operator()(const VDStringA& s, const char *t) const {
	return s.comparei(t) == 0;
}

bool vdstringpredi::operator()(const VDStringW& s, const VDStringW& t) const {
	return s.comparei(t) == 0;
}

bool vdstringpredi::operator()(const VDStringW& s, const VDStringSpanW& t) const {
	return s.comparei(t) == 0;
}

bool vdstringpredi::operator()(const VDStringW& s, const wchar_t *t) const {
	return s.comparei(t) == 0;
}
