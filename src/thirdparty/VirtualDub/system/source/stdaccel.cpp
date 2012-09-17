//	VirtualDub - Video processing and capture application
//	System library component
//	Copyright (C) 1998-2007 Avery Lee, All Rights Reserved.
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

#define VDTEXTERN

#include <vd2/system/vdtypes.h>
#include <vd2/system/vdstl.h>

template class vdspan<char>;
template class vdspan<uint8>;
template class vdspan<uint16>;
template class vdspan<uint32>;
template class vdspan<uint64>;
template class vdspan<sint8>;
template class vdspan<sint16>;
template class vdspan<sint32>;
template class vdspan<sint64>;
template class vdspan<float>;
template class vdspan<double>;
template class vdspan<wchar_t>;
