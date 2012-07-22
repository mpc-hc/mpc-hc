/****************************************************************************
 * 
 *  Copyright (C) 1995-1999 RealNetworks, Inc. All rights reserved.
 *  
 *  http://www.real.com/devzone
 *
 *  This file is a replacement for the basetsd.h file from VC++ 6.0.
 *  This will automatically be included by pntypes.h when using VC++
 *  6.0 or greater. This file correctly defines the basic sized types
 *  so they don't conflict with the G2 SDK.
 *
 */

#if !(defined(_MSC_VER) && (_MSC_VER > 1100))
#error pnbastsd.h is only intended for use with Microsoft VC++ 6.0 or higher
#endif

#ifndef _BASETSD_H_
#define _BASETSD_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The following types were defined in pntypes.h and so are intentionally
 * not included in this file. They are left here for documentation purposes.
 */
#if 0
typedef int LONG32;
typedef int INT32;
typedef unsigned int ULONG32;
typedef unsigned int UINT32;
#endif

/*
 * The following types are 32 bits wide and unsigned.
 */
typedef unsigned int *PULONG32;
typedef unsigned int DWORD32, *PDWORD32;
typedef unsigned int *PUINT32;

/*
 * INT_PTR is the same size as a pointer.  Its size changes with pointer 
 * size (32 bit or 64 bit).  HALF_PTR is half the size of a pointer.
 */
#ifdef _WIN64

typedef __int64 INT_PTR, *PINT_PTR;
typedef unsigned __int64 UINT_PTR, *PUINT_PTR;

#define MAXINT_PTR (0x7fffffffffffffffI64)
#define MININT_PTR (0x8000000000000000I64)
#define MAXUINT_PTR (0xffffffffffffffffUI64)

typedef unsigned int UHALF_PTR, *PUHALF_PTR;
typedef int HALF_PTR, *PHALF_PTR;

#define MAXUHALF_PTR (0xffffffffUL)
#define MAXHALF_PTR (0x7fffffffL)
#define MINHALF_PTR (0x80000000L)

#pragma warning(disable:4311)   /* type cast truncation */

#if !defined(__midl)
__inline
unsigned long
HandleToUlong(
    void *h
    )
{
    return((unsigned long) h );
}

__inline
unsigned long
PtrToUlong(
    void  *p
    )
{
    return((unsigned long) p );
}

__inline
unsigned short
PtrToUshort(
    void  *p
    )
{
    return((unsigned short) p );
}

__inline
long
PtrToLong(
    void  *p
    )
{
    return((long) p );
}

__inline
short
PtrToShort(
    void  *p
    )
{
    return((short) p );
}
#endif
#pragma warning(3:4311)   /* type cast truncation */

#else


typedef long INT_PTR, *PINT_PTR;
typedef unsigned long UINT_PTR, *PUINT_PTR;

#define MAXINT_PTR (0x7fffffffL)
#define MININT_PTR (0x80000000L)
#define MAXUINT_PTR (0xffffffffUL)

typedef unsigned short UHALF_PTR, *PUHALF_PTR;
typedef short HALF_PTR, *PHALF_PTR;

#define MAXUHALF_PTR 0xffff
#define MAXHALF_PTR 0x7fff
#define MINHALF_PTR 0x8000

#define HandleToUlong( h ) ((ULONG) (h) )
#define PtrToUlong( p ) ((ULONG) (p) )
#define PtrToLong( p ) ((LONG) (p) )
#define PtrToUshort( p ) ((unsigned short) (p) )
#define PtrToShort( p ) ((short) (p) )

#endif

/*
 * Basic SIZE_T support.
 */
typedef UINT_PTR SIZE_T, *PSIZE_T;
typedef INT_PTR SSIZE_T, *PSSIZE_T;

/*
 * These types are 64 bits wide and signed.
 */
typedef __int64 LONG64, *PLONG64;
typedef __int64 INT64, *PINT64;

/*
 * These types are 64 bits wide and unsigned.
 */
typedef unsigned __int64 ULONG64, *PULONG64;
typedef unsigned __int64 DWORD64, *PDWORD64;
typedef unsigned __int64 UINT64, *PUINT64;

#ifdef __cplusplus
}
#endif

#endif /* _BASETSD_H_ */
