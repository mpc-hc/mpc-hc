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

#ifndef f_VD2_SYSTEM_VDTYPES_H
#define f_VD2_SYSTEM_VDTYPES_H

#ifdef _MSC_VER
	#pragma once
#endif

#include <algorithm>
#include <stdio.h>
#include <stdarg.h>
#include <new>

#ifndef NULL
#define NULL 0
#endif

///////////////////////////////////////////////////////////////////////////
//
//	compiler detection
//
///////////////////////////////////////////////////////////////////////////

#ifndef VD_COMPILER_DETECTED
	#define VD_COMPILER_DETECTED

	#if defined(_MSC_VER)
		#define VD_COMPILER_MSVC	_MSC_VER

		#if _MSC_VER >= 1400
			#define VD_COMPILER_MSVC_VC8		1
			#define VD_COMPILER_MSVC_VC8_OR_LATER 1

			#if _MSC_FULL_VER == 140040310
				#define VD_COMPILER_MSVC_VC8_PSDK 1
			#elif _MSC_FULL_VER == 14002207
				#define VD_COMPILER_MSVC_VC8_DDK 1
			#endif

		#elif _MSC_VER >= 1310
			#define VD_COMPILER_MSVC_VC71	1
		#elif _MSC_VER >= 1300
			#define VD_COMPILER_MSVC_VC7		1
		#elif _MSC_VER >= 1200
			#define VD_COMPILER_MSVC_VC6		1
		#endif
	#elif defined(__GNUC__)
		#define VD_COMPILER_GCC
		#if defined(__MINGW32__) || defined(__MINGW64__)
			#define VD_COMPILER_GCC_MINGW
		#endif
	#endif
#endif

#ifndef VD_CPU_DETECTED
	#define VD_CPU_DETECTED

	#if defined(_M_AMD64)
		#define VD_CPU_AMD64	1
	#elif defined(_M_IX86) || defined(__i386__)
		#define VD_CPU_X86		1
	#elif defined(_M_ARM)
		#define VD_CPU_ARM
	#endif
#endif

///////////////////////////////////////////////////////////////////////////
//
//	types
//
///////////////////////////////////////////////////////////////////////////

#ifndef VD_STANDARD_TYPES_DECLARED
	#if defined(_MSC_VER)
		typedef signed __int64		sint64;
		typedef unsigned __int64	uint64;
	#elif defined(__GNUC__)
		typedef signed long long	sint64;
		typedef unsigned long long	uint64;
	#endif
	typedef signed int			sint32;
	typedef unsigned int		uint32;
	typedef signed short		sint16;
	typedef unsigned short		uint16;
	typedef signed char			sint8;
	typedef unsigned char		uint8;

	typedef sint64				int64;
	typedef sint32				int32;
	typedef sint16				int16;
	typedef sint8				int8;

	#ifdef _M_AMD64
		typedef sint64 sintptr;
		typedef uint64 uintptr;
	#else
		#if _MSC_VER >= 1310
			typedef __w64 sint32 sintptr;
			typedef __w64 uint32 uintptr;
		#else
			typedef sint32 sintptr;
			typedef uint32 uintptr;
		#endif
	#endif
#endif

#if defined(_MSC_VER)
	#define VD64(x) x##i64
#elif defined(__GNUC__)
	#define VD64(x) x##ll
#else
	#error Please add an entry for your compiler for 64-bit constant literals.
#endif

	
#define VDAPIENTRY			__cdecl

typedef int64 VDTime;
typedef int64 VDPosition;
typedef	struct __VDGUIHandle *VDGUIHandle;

// enforce wchar_t under Visual C++

#if defined(_MSC_VER) && !defined(_WCHAR_T_DEFINED)
	#include <ctype.h>
#endif

///////////////////////////////////////////////////////////////////////////
//
//	allocation
//
///////////////////////////////////////////////////////////////////////////

#if defined(VD_COMPILER_MSVC) && (VD_COMPILER_MSVC < 1300 || (defined(VD_COMPILER_MSVC_VC8_PSDK) || defined(VD_COMPILER_MSVC_VC8_DDK)))
#define new_nothrow new
#else
#define new_nothrow new(std::nothrow)
#endif

///////////////////////////////////////////////////////////////////////////
//
//	STL fixes
//
///////////////////////////////////////////////////////////////////////////

#if defined(VD_COMPILER_MSVC_VC6) || defined(VD_COMPILER_MSVC_VC8_DDK) || defined(VD_COMPILER_MSVC_VC8_PSDK)
	// The VC6 STL was deliberately borked to avoid conflicting with
	// Windows min/max macros.  We work around this bogosity here.  Note
	// that NOMINMAX must be defined for these to compile properly.  Also,
	// there is a bug in the VC6 compiler that sometimes causes long
	// lvalues to "promote" to int, causing ambiguous override errors.
	// To avoid this, always explicitly declare which type you are using,
	// i.e. min<int>(x,0).  None of this is a problem with VC7 or later.
	namespace std {
		template<class T>
		inline const T& min(const T& x, const T& y) {
			return _cpp_min(x, y);
		}

		template<class T>
		inline const T& max(const T& x, const T& y) {
			return _cpp_max(x, y);
		}
	};
#endif

///////////////////////////////////////////////////////////////////////////
//
//	compiler fixes
//
///////////////////////////////////////////////////////////////////////////

#if defined(VD_COMPILER_MSVC) && (VD_COMPILER_MSVC < 1400 || (defined(VD_COMPILER_MSVC_VC8_PSDK) || defined(VD_COMPILER_MSVC_VC8_DDK)))
	inline int vswprintf(wchar_t *dst, size_t bufsize, const wchar_t *format, va_list val) {
		return _vsnwprintf(dst, bufsize, format, val);
	}

	inline int swprintf(wchar_t *dst, size_t bufsize, const wchar_t *format, ...) {
		va_list val;

		va_start(val, format);
		int r = vswprintf(dst, bufsize, format, val);
		va_end(val);

		return r;
	}

	#define _strdup strdup
	#define _stricmp stricmp
	#define _strnicmp strnicmp
	#define _wcsdup wcsdup
	#define _wcsicmp wcsicmp
	#define _wcsnicmp wcsnicmp
#endif

#if defined(VD_COMPILER_MSVC) && VD_COMPILER_MSVC < 1400
	#define vdfor if(0);else for
#else
	#define vdfor for
#endif

///////////////////////////////////////////////////////////////////////////
//
//	attribute support
//
///////////////////////////////////////////////////////////////////////////

#if defined(VD_COMPILER_MSVC)
	#define VDINTERFACE			__declspec(novtable)
	#define VDNORETURN			__declspec(noreturn)
	#define VDPUREFUNC

	#if VD_COMPILER_MSVC >= 1400
		#define VDRESTRICT		__restrict
	#else
		#define VDRESTRICT
	#endif

	#define VDNOINLINE			__declspec(noinline)
	#define VDFORCEINLINE		__forceinline
	#define VDALIGN(alignment)	__declspec(align(alignment))
#elif defined(VD_COMPILER_GCC)
	#define VDINTERFACE
	#define VDNORETURN			__attribute__((noreturn))
	#define VDPUREFUNC			__attribute__((pure))
	#define VDRESTRICT			__restrict
	#define VDNOINLINE			__attribute__((noinline))
	#define VDFORCEINLINE		inline __attribute__((always_inline))
	#define VDALIGN(alignment)	__attribute__((aligned(alignment)))
#else
	#define VDINTERFACE
	#define VDNORETURN
	#define VDPUREFUNC
	#define VDRESTRICT
	#define VDFORCEINLINE
	#define VDALIGN(alignment)
#endif

///////////////////////////////////////////////////////////////////////////
//
//	debug support
//
///////////////////////////////////////////////////////////////////////////

enum VDAssertResult {
	kVDAssertBreak,
	kVDAssertContinue,
	kVDAssertIgnore
};

extern VDAssertResult VDAssert(const char *exp, const char *file, int line);
extern VDAssertResult VDAssertPtr(const char *exp, const char *file, int line);
extern void VDDebugPrint(const char *format, ...);

#if defined(_MSC_VER)
	#if _MSC_VER >= 1300
		#define VDBREAK		__debugbreak()
	#else
		#define VDBREAK		__asm { int 3 }
	#endif
#elif defined(__GNUC__)
	#define VDBREAK		__asm__ volatile ("int3" : : )
#else
	#define VDBREAK		*(volatile char *)0 = *(volatile char *)0
#endif


#ifdef _DEBUG

	namespace {
		template<int line>
		struct VDAssertHelper {
			VDAssertHelper(const char *exp, const char *file) {
				if (!sbAssertDisabled)
					switch(VDAssert(exp, file, line)) {
					case kVDAssertBreak:
						VDBREAK;
						break;
					case kVDAssertIgnore:
						sbAssertDisabled = true;
						break;
					}
			}

			static bool sbAssertDisabled;
		};

		template<int lineno>
		bool VDAssertHelper<lineno>::sbAssertDisabled;

		template<int lineno>
		struct VDAssertHelper2 { static bool sDisabled; };

		template<int lineno>
		bool VDAssertHelper2<lineno>::sDisabled;
	}

// MPC-HC custom code start
// We use the old code since the new templated code isn't compatible with MSVC "Edit and continue" feature 
// because __LINE__ can't be known at compile time.
#if !defined(VD_COMPILER_MSVC) || defined(__INTEL_COMPILER)
	#define VDASSERT(exp)		if (!VDAssertHelper2<__LINE__>::sDisabled) if (exp); else switch(VDAssert   (#exp, __FILE__, __LINE__)) { case kVDAssertBreak: VDBREAK; break; case kVDAssertIgnore: VDAssertHelper2<__LINE__>::sDisabled = true; } else ((void)0)
	#define VDASSERTPTR(exp) 	if (!VDAssertHelper2<__LINE__>::sDisabled) if (exp); else switch(VDAssertPtr(#exp, __FILE__, __LINE__)) { case kVDAssertBreak: VDBREAK; break; case kVDAssertIgnore: VDAssertHelper2<__LINE__>::sDisabled = true; } else ((void)0)
	#define VDVERIFY(exp)		if (exp); else if (!VDAssertHelper2<__LINE__>::sDisabled) switch(VDAssert   (#exp, __FILE__, __LINE__)) { case kVDAssertBreak: VDBREAK; break; case kVDAssertIgnore: VDAssertHelper2<__LINE__>::sDisabled = true; } else ((void)0)
	#define VDVERIFYPTR(exp) 	if (exp); else if (!VDAssertHelper2<__LINE__>::sDisabled) switch(VDAssertPtr(#exp, __FILE__, __LINE__)) { case kVDAssertBreak: VDBREAK; break; case kVDAssertIgnore: VDAssertHelper2<__LINE__>::sDisabled = true; } else ((void)0)
#else
	#define VDASSERT(exp)		do 	{ } while (0)
	#define VDASSERTPTR(exp) 	do 	{ } while (0)
	#define VDVERIFY(exp)		do 	{ } while (0)
	#define VDVERIFYPTR(exp) 	do 	{ } while (0)
#endif
// MPC-HC custom code end
	#define VDASSERTCT(exp)		(void)sizeof(int[(exp)?1:-1])

	#define VDINLINEASSERT(exp)			((exp)||(VDAssertHelper<__LINE__>(#exp, __FILE__),false))
	#define VDINLINEASSERTFALSE(exp)	((exp)&&(VDAssertHelper<__LINE__>("!("#exp")", __FILE__),true))

	#define NEVER_HERE			do { if (VDAssert( "[never here]", __FILE__, __LINE__ )) VDBREAK; __assume(false); } while(false)
	#define	VDNEVERHERE			do { if (VDAssert( "[never here]", __FILE__, __LINE__ )) VDBREAK; __assume(false); } while(false)

	#define VDDEBUG				VDDebugPrint

#else

	#if defined(_MSC_VER)
		#ifndef _M_AMD64
			#define VDASSERT(exp)		__assume(!!(exp))
			#define VDASSERTPTR(exp)	__assume(!!(exp))
		#else
			#define VDASSERT(exp)		__noop(exp)
			#define VDASSERTPTR(exp)	__noop(exp)
		#endif
	#elif defined(__GNUC__)
		#define VDASSERT(exp)		__builtin_expect(0 != (exp), 1)
		#define VDASSERTPTR(exp)	__builtin_expect(0 != (exp), 1)
	#endif

	#define VDVERIFY(exp)		(exp)
	#define VDVERIFYPTR(exp)	(exp)
	#define VDASSERTCT(exp)

	#define VDINLINEASSERT(exp)	(exp)
	#define VDINLINEASSERTFALSE(exp)	(exp)

	#if defined(VD_COMPILER_MSVC)
		#define NEVER_HERE			__assume(false)
		#define	VDNEVERHERE			__assume(false)
	#else
		#define NEVER_HERE			VDASSERT(false)
		#define	VDNEVERHERE			VDASSERT(false)
	#endif

	extern int VDDEBUG_Helper(const char *, ...);
	#define VDDEBUG				(void)sizeof VDDEBUG_Helper

#endif

#define VDDEBUG2			VDDebugPrint

// TODO macros
//
// These produce a diagnostic during compilation that indicate a TODO for
// later:
//
//		#pragma message(__TODO__ "Fix this.)
//		#vdpragma_TODO("Fix this.")

#define vdpragma_TODO2(x)	#x
#define vdpragma_TODO1(x)	vdpragma_TODO2(x)
#define vdpragma_TODO0		__FILE__ "(" vdpragma_TODO1(__LINE__) ") : TODO: "

#ifdef _MSC_VER
#define vdpragma_TODO(x)		message(vdpragma_TODO0 x)
#else
#define vdpragma_TODO(x)
#endif

// BS macros
//
// These tag code that is not meant to go into a final build.

#define vdpragma_BS2(x)	#x
#define vdpragma_BS1(x)	vdpragma_BS2(x)
#define vdpragma_BS0		__FILE__ "(" vdpragma_BS1(__LINE__) ") : BS: "

#ifdef _MSC_VER
#define vdpragma_BS(x)		message(vdpragma_BS0 x)
#else
#define vdpragma_BS(x)
#endif

///////////////////////////////////////////////////////////////////////////
//
// Object scope macros
//
// vdobjectscope() allows you to define a construct where an object is
// constructed and live only within the controlled statement.  This is
// used for vdsynchronized (thread.h) and protected scopes below.
// It relies on a strange quirk of C++ regarding initialized objects
// in the condition of a selection statement and also horribly abuses
// the switch statement, generating rather good code in release builds.
// The catch is that the controlled object must implement a conversion to
// bool returning false and must only be initialized with one argument (C
// syntax).
//
// Unfortunately, handy as this macro is, it is also damned good at
// breaking compilers.  For a start, declaring an object with a non-
// trivial destructor in a switch() kills both VC6 and VC7 with a C1001.
// The bug is fixed in VC8 (MSC 14.00).
//
// A somewhat safer alternative is the for() statement, along the lines
// of:
//
// switch(bool v=false) case 0: default: for(object_def; !v; v=true)
//
// This avoids the conversion operator but unfortunately usually generates
// an actual loop in the output.

#if defined(VD_COMPILER_MSVC) && (VD_COMPILER_MSVC < 1400 || defined(VD_COMPILER_MSVC_VC8_DDK))
#define vdobjectscope(object_def) if(object_def) VDNEVERHERE; else
#else
#define vdobjectscope(object_def) switch(object_def) case 0: default:
#endif

#endif
