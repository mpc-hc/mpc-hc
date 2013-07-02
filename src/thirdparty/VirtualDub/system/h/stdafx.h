// Force C locale to avoid this warning:
//
// mmreg.h : warning C4819: The file contains a character that cannot be represented in the current code page (932). Save the file in Unicode format to prevent data loss
#pragma setlocale("C")

/* MPC-HC comment out
// Detect the Windows SDK in use and select Windows 2000 baseline
// if the Vista SDK, else Windows 98 baseline.
#ifdef _MSC_VER
#include <ntverp.h>
#else
#define VER_PRODUCTBUILD 6001
#endif
#if VER_PRODUCTBUILD > 6000
#define _WIN32_WINNT 0x0500
#else
#define _WIN32_WINNT 0x0410
#endif
*/

// Start patch MPC-HC
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#endif
// End patch MPC-HC

#include <vd2/system/vdtypes.h>
#include <vd2/system/atomic.h>
#include <vd2/system/thread.h>
#include <vd2/system/error.h>
#include <vd2/system/filesys.h> // MPC-HC patch
#include <windows.h>
#include <process.h>
#include <vd2/system/win32/intrin.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <ctype.h>
