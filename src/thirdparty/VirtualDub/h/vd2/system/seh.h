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

#ifndef f_VD2_SYSTEM_SEH_H
#define f_VD2_SYSTEM_SEH_H

#include <vd2/system/vdtypes.h>

//////////////////////////////////////////////////////////////////////////////
// Structured Exception Handling (SEH) macros.
//
// These are used for memory access operations that may be possibly invalid
// and must be guarded. This is currently only supported on Win32/Win64
// platforms with the VC++ compiler, since GCC does not currently support SEH.
//
// For cases where a memcpy() is the guarded operation, the VDMemcpyGuarded()
// function should be used instead.
//
//////////////////////////////////////////////////////////////////////////////

#if defined(VD_COMPILER_MSVC) && defined(_WIN32)
	#include <excpt.h>

	#define vd_seh_guard_try		__try
	#define vd_seh_guard_except		__except(GetExceptionCode() == STATUS_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
#else
	#define vd_seh_guard_try		if (true)
	#define vd_seh_guard_except		else
#endif

#endif

