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

#ifndef f_VD2_SYSTEM_WIN32_MINIWINDOWS_H
#define f_VD2_SYSTEM_WIN32_MINIWINDOWS_H

#define VDZCALLBACK __stdcall

#ifndef _WIN64
	#ifdef VD_COMPILER_MSVC
		typedef __w64 int		VDZINT_PTR;
		typedef __w64 unsigned	VDZUINT_PTR;
		typedef __w64 long		VDZLONG_PTR;
	#else
		typedef int			VDZINT_PTR;
		typedef unsigned	VDZUINT_PTR;
		typedef long		VDZLONG_PTR;
	#endif
#else
	typedef __int64				VDZINT_PTR;
	typedef unsigned __int64	VDZUINT_PTR;
	typedef __int64				VDZLONG_PTR;
#endif

typedef struct HWND__	*VDZHWND;
typedef struct HDC__	*VDZHDC;
typedef struct HKEY__	*VDZHKEY;
typedef unsigned		VDZUINT;
typedef unsigned short	VDZWORD;
typedef unsigned long	VDZDWORD;
typedef VDZUINT_PTR		VDZWPARAM;
typedef VDZLONG_PTR		VDZLPARAM;
typedef VDZLONG_PTR		VDZLRESULT;
typedef struct HDROP__	*VDZHDROP;
typedef struct HACCEL__	*VDZHACCEL;

typedef VDZWORD			VDZATOM;

#endif
