// ZenLib::Utils - Very small utilities
// Copyright (C) 2002-2010 MediaArea.net SARL, Info@MediaArea.net
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Integer and float manipulation
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef ZenUtilsH
#define ZenUtilsH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "ZenLib/Conf.h"
#include "ZenLib/int128u.h"
//---------------------------------------------------------------------------

namespace ZenLib
{

//***************************************************************************
// Integer transformations
//***************************************************************************

//---------------------------------------------------------------------------
//Little Endians
int8s  LittleEndian2int8s  (const char* List);
int8u  LittleEndian2int8u  (const char* List);
int16s LittleEndian2int16s (const char* List);
int16u LittleEndian2int16u (const char* List);
int32s LittleEndian2int24s (const char* List);
int32u LittleEndian2int24u (const char* List);
int32s LittleEndian2int32s (const char* List);
int32u LittleEndian2int32u (const char* List);
#if (MAXTYPE_INT >= 64)
int64s LittleEndian2int40s (const char* List);
int64u LittleEndian2int40u (const char* List);
int64s LittleEndian2int48s (const char* List);
int64u LittleEndian2int48u (const char* List);
int64s LittleEndian2int56s (const char* List);
int64u LittleEndian2int56u (const char* List);
int64s LittleEndian2int64s (const char* List);
int64u LittleEndian2int64u (const char* List);
int128u LittleEndian2int128u (const char* List);
#endif
float32 LittleEndian2float32 (const char* List);
float64 LittleEndian2float64 (const char* List);
float80 LittleEndian2float80 (const char* List);

void   int8s2LittleEndian     (char* List, int8s  Value);
void   int8u2LittleEndian     (char* List, int8u  Value);
void   int16s2LittleEndian    (char* List, int16s Value);
void   int16u2LittleEndian    (char* List, int16u Value);
void   int24s2LittleEndian    (char* List, int32s Value);
void   int24u2LittleEndian    (char* List, int32u Value);
void   int32s2LittleEndian    (char* List, int32s Value);
void   int32u2LittleEndian    (char* List, int32u Value);
#if (MAXTYPE_INT >= 64)
void   int40s2LittleEndian    (char* List, int64s Value);
void   int40u2LittleEndian    (char* List, int64u Value);
void   int48s2LittleEndian    (char* List, int64s Value);
void   int48u2LittleEndian    (char* List, int64u Value);
void   int56s2LittleEndian    (char* List, int64s Value);
void   int56u2LittleEndian    (char* List, int64u Value);
void   int64s2LittleEndian    (char* List, int64s Value);
void   int64u2LittleEndian    (char* List, int64u Value);
void   int128u2LittleEndian   (char* List, int128u Value);
#endif
void   float322LittleEndian   (char* List, float32 Value);
void   float642LittleEndian   (char* List, float64 Value);
void   float802LittleEndian   (char* List, float80 Value);

#ifndef __BORLANDC__
inline int8s  LittleEndian2int8s  (const int8u* List) {return LittleEndian2int8s  ((const char*)List);}
inline int8u  LittleEndian2int8u  (const int8u* List) {return LittleEndian2int8u  ((const char*)List);}
inline int16s LittleEndian2int16s (const int8u* List) {return LittleEndian2int16s ((const char*)List);}
inline int16u LittleEndian2int16u (const int8u* List) {return LittleEndian2int16u ((const char*)List);}
inline int32s LittleEndian2int24s (const int8u* List) {return LittleEndian2int24s ((const char*)List);}
inline int32u LittleEndian2int24u (const int8u* List) {return LittleEndian2int24u ((const char*)List);}
inline int32s LittleEndian2int32s (const int8u* List) {return LittleEndian2int32s ((const char*)List);}
inline int32u LittleEndian2int32u (const int8u* List) {return LittleEndian2int32u ((const char*)List);}
#if (MAXTYPE_INT >= 64)
inline int64s LittleEndian2int40s (const int8u* List) {return LittleEndian2int40s ((const char*)List);}
inline int64u LittleEndian2int40u (const int8u* List) {return LittleEndian2int40u ((const char*)List);}
inline int64s LittleEndian2int48s (const int8u* List) {return LittleEndian2int48s ((const char*)List);}
inline int64u LittleEndian2int48u (const int8u* List) {return LittleEndian2int48u ((const char*)List);}
inline int64s LittleEndian2int56s (const int8u* List) {return LittleEndian2int56s ((const char*)List);}
inline int64u LittleEndian2int56u (const int8u* List) {return LittleEndian2int56u ((const char*)List);}
inline int64s LittleEndian2int64s (const int8u* List) {return LittleEndian2int64s ((const char*)List);}
inline int64u LittleEndian2int64u (const int8u* List) {return LittleEndian2int64u ((const char*)List);}
inline int128u LittleEndian2int128u (const int8u* List) {return LittleEndian2int64u ((const char*)List);}
#endif
inline float32 LittleEndian2float32 (const int8u* List) {return LittleEndian2float32 ((const char*)List);}
inline float64 LittleEndian2float64 (const int8u* List) {return LittleEndian2float64 ((const char*)List);}
inline float80 LittleEndian2float80 (const int8u* List) {return LittleEndian2float80 ((const char*)List);}

inline void   int8s2LittleEndian     (int8u* List, int8s  Value) {return int8s2LittleEndian    ((char*)List, Value);}
inline void   int8u2LittleEndian     (int8u* List, int8u  Value) {return int8u2LittleEndian    ((char*)List, Value);}
inline void   int16s2LittleEndian    (int8u* List, int16s Value) {return int16s2LittleEndian   ((char*)List, Value);}
inline void   int16u2LittleEndian    (int8u* List, int16u Value) {return int16u2LittleEndian   ((char*)List, Value);}
inline void   int24s2LittleEndian    (int8u* List, int32s Value) {return int24s2LittleEndian   ((char*)List, Value);}
inline void   int24u2LittleEndian    (int8u* List, int32u Value) {return int24u2LittleEndian   ((char*)List, Value);}
inline void   int32s2LittleEndian    (int8u* List, int32s Value) {return int32s2LittleEndian   ((char*)List, Value);}
inline void   int32u2LittleEndian    (int8u* List, int32u Value) {return int32u2LittleEndian   ((char*)List, Value);}
#if (MAXTYPE_INT >= 64)
inline void   int40s2LittleEndian    (int8u* List, int64s Value) {return int40s2LittleEndian   ((char*)List, Value);}
inline void   int40u2LittleEndian    (int8u* List, int64u Value) {return int40u2LittleEndian   ((char*)List, Value);}
inline void   int48s2LittleEndian    (int8u* List, int64s Value) {return int48s2LittleEndian   ((char*)List, Value);}
inline void   int48u2LittleEndian    (int8u* List, int64u Value) {return int48u2LittleEndian   ((char*)List, Value);}
inline void   int56s2LittleEndian    (int8u* List, int64s Value) {return int56s2LittleEndian   ((char*)List, Value);}
inline void   int56u2LittleEndian    (int8u* List, int64u Value) {return int56u2LittleEndian   ((char*)List, Value);}
inline void   int64s2LittleEndian    (int8u* List, int64s Value) {return int64s2LittleEndian   ((char*)List, Value);}
inline void   int64u2LittleEndian    (int8u* List, int64u Value) {return int64u2LittleEndian   ((char*)List, Value);}
inline void   int128u2LittleEndian   (int8u* List, int128u Value) {return int128u2LittleEndian ((char*)List, Value);}
#endif
inline void   float322LittleEndian   (int8u* List, float32 Value) {return float322LittleEndian ((char*)List, Value);}
inline void   float642LittleEndian   (int8u* List, float64 Value) {return float642LittleEndian ((char*)List, Value);}
inline void   float802LittleEndian   (int8u* List, float80 Value) {return float802LittleEndian ((char*)List, Value);}
#endif //__BORLANDC__

//---------------------------------------------------------------------------
//Big Endians
int8s  BigEndian2int8s     (const char* List);
int8u  BigEndian2int8u     (const char* List);
int16s BigEndian2int16s    (const char* List);
int16u BigEndian2int16u    (const char* List);
int32s BigEndian2int24s    (const char* List);
int32u BigEndian2int24u    (const char* List);
int32s BigEndian2int32s    (const char* List);
int32u BigEndian2int32u    (const char* List);
#if (MAXTYPE_INT >= 64)
int64s BigEndian2int40s    (const char* List);
int64u BigEndian2int40u    (const char* List);
int64s BigEndian2int48s    (const char* List);
int64u BigEndian2int48u    (const char* List);
int64s BigEndian2int56s    (const char* List);
int64u BigEndian2int56u    (const char* List);
int64s BigEndian2int64s    (const char* List);
int64u BigEndian2int64u    (const char* List);
int128u BigEndian2int128u  (const char* List);
#endif
float32 BigEndian2float32  (const char* List);
float64 BigEndian2float64  (const char* List);
float80 BigEndian2float80  (const char* List);

void   int8s2BigEndian     (char* List, int8s  Value);
void   int8u2BigEndian     (char* List, int8u  Value);
void   int16s2BigEndian    (char* List, int16s Value);
void   int16u2BigEndian    (char* List, int16u Value);
void   int24s2BigEndian    (char* List, int32s Value);
void   int24u2BigEndian    (char* List, int32u Value);
void   int32s2BigEndian    (char* List, int32s Value);
void   int32u2BigEndian    (char* List, int32u Value);
#if (MAXTYPE_INT >= 64)
void   int40s2BigEndian    (char* List, int64s Value);
void   int40u2BigEndian    (char* List, int64u Value);
void   int48s2BigEndian    (char* List, int64s Value);
void   int48u2BigEndian    (char* List, int64u Value);
void   int56s2BigEndian    (char* List, int64s Value);
void   int56u2BigEndian    (char* List, int64u Value);
void   int64s2BigEndian    (char* List, int64s Value);
void   int64u2BigEndian    (char* List, int64u Value);
void   int128u2BigEndian   (char* List, int128u Value);
#endif
void   float322BigEndian   (char* List, float32 Value);
void   float642BigEndian   (char* List, float64 Value);
void   float802BigEndian   (char* List, float80 Value);

#ifndef __BORLANDC__
inline int8s  BigEndian2int8s     (const int8u* List) {return BigEndian2int8s     ((const char*)List);}
inline int8u  BigEndian2int8u     (const int8u* List) {return BigEndian2int8u     ((const char*)List);}
inline int16s BigEndian2int16s    (const int8u* List) {return BigEndian2int16s    ((const char*)List);}
inline int16u BigEndian2int16u    (const int8u* List) {return BigEndian2int16u    ((const char*)List);}
inline int32s BigEndian2int32s    (const int8u* List) {return BigEndian2int32s    ((const char*)List);}
inline int32u BigEndian2int24u    (const int8u* List) {return BigEndian2int24u    ((const char*)List);}
inline int32s BigEndian2int24s    (const int8u* List) {return BigEndian2int24s    ((const char*)List);}
inline int32u BigEndian2int32u    (const int8u* List) {return BigEndian2int32u    ((const char*)List);}
#if (MAXTYPE_INT >= 64)
inline int64s BigEndian2int40s    (const int8u* List) {return BigEndian2int40s    ((const char*)List);}
inline int64u BigEndian2int40u    (const int8u* List) {return BigEndian2int40u    ((const char*)List);}
inline int64s BigEndian2int48s    (const int8u* List) {return BigEndian2int48s    ((const char*)List);}
inline int64u BigEndian2int48u    (const int8u* List) {return BigEndian2int48u    ((const char*)List);}
inline int64s BigEndian2int56s    (const int8u* List) {return BigEndian2int56s    ((const char*)List);}
inline int64u BigEndian2int56u    (const int8u* List) {return BigEndian2int56u    ((const char*)List);}
inline int64s BigEndian2int64s    (const int8u* List) {return BigEndian2int64s    ((const char*)List);}
inline int64u BigEndian2int64u    (const int8u* List) {return BigEndian2int64u    ((const char*)List);}
inline int128u BigEndian2int128u  (const int8u* List) {return BigEndian2int128u   ((const char*)List);}
#endif
inline float32 BigEndian2float32  (const int8u* List) {return BigEndian2float32   ((const char*)List);}
inline float64 BigEndian2float64  (const int8u* List) {return BigEndian2float64   ((const char*)List);}
inline float80 BigEndian2float80  (const int8u* List) {return BigEndian2float80   ((const char*)List);}

inline void   int8s2BigEndian     (int8u* List, int8s  Value) {return int8s2BigEndian    ((char*)List, Value);}
inline void   int8u2BigEndian     (int8u* List, int8u  Value) {return int8u2BigEndian    ((char*)List, Value);}
inline void   int16s2BigEndian    (int8u* List, int16s Value) {return int16s2BigEndian   ((char*)List, Value);}
inline void   int16u2BigEndian    (int8u* List, int16u Value) {return int16u2BigEndian   ((char*)List, Value);}
inline void   int24s2BigEndian    (int8u* List, int32s Value) {return int24s2BigEndian   ((char*)List, Value);}
inline void   int24u2BigEndian    (int8u* List, int32u Value) {return int24u2BigEndian   ((char*)List, Value);}
inline void   int32s2BigEndian    (int8u* List, int32s Value) {return int32s2BigEndian   ((char*)List, Value);}
inline void   int32u2BigEndian    (int8u* List, int32u Value) {return int32u2BigEndian   ((char*)List, Value);}
#if (MAXTYPE_INT >= 64)
inline void   int40s2BigEndian    (int8u* List, int64s Value) {return int40s2BigEndian   ((char*)List, Value);}
inline void   int40u2BigEndian    (int8u* List, int64u Value) {return int40u2BigEndian   ((char*)List, Value);}
inline void   int48s2BigEndian    (int8u* List, int64s Value) {return int48s2BigEndian   ((char*)List, Value);}
inline void   int48u2BigEndian    (int8u* List, int64u Value) {return int48u2BigEndian   ((char*)List, Value);}
inline void   int56s2BigEndian    (int8u* List, int64s Value) {return int56s2BigEndian   ((char*)List, Value);}
inline void   int56u2BigEndian    (int8u* List, int64u Value) {return int56u2BigEndian   ((char*)List, Value);}
inline void   int64s2BigEndian    (int8u* List, int64s Value) {return int64s2BigEndian   ((char*)List, Value);}
inline void   int64u2BigEndian    (int8u* List, int64u Value) {return int64u2BigEndian   ((char*)List, Value);}
inline void   int128u2BigEndian   (int8u* List, int128u Value) {return int128u2BigEndian ((char*)List, Value);}
#endif
inline void   float322BigEndian   (int8u* List, float32 Value) {return float322BigEndian ((char*)List, Value);}
inline void   float642BigEndian   (int8u* List, float64 Value) {return float642BigEndian ((char*)List, Value);}
inline void   float802BigEndian   (int8u* List, float80 Value) {return float802BigEndian ((char*)List, Value);}
#endif //__BORLANDC__

//---------------------------------------------------------------------------
// int32 - int64
int64s int32s_int64s (                int32s  High, int32u  Low);
int64u int32u_int64u (                int32u  High, int32u  Low);
void   int32s_int64s (int64s &BigInt, int32s  High, int32u  Low);
void   int32u_int64u (int64s &BigInt, int32u  High, int32u  Low);
void   int64s_int32s (int64s  BigInt, int32s &High, int32u &Low);
void   int64u_int32u (int64u  BigInt, int32u &High, int32u &Low);

//---------------------------------------------------------------------------
// Floats and ints
int32s float32_int32s  (float32 F, bool Rounded=true);
int64s float32_int64s  (float32 F, bool Rounded=true);
int32s float64_int32s  (float64 F, bool Rounded=true);
int64s float64_int64s  (float64 F, bool Rounded=true);

// These functions are used because MSVC6 isn't able to convert an unsigned int64 to a floating-point value, and I couldn't think of a cleaner way to handle it.
#if defined(_MSC_VER) && _MSC_VER<=1200
    inline float32 int64u_float32 (int64u v) {return static_cast<float32>(static_cast<int64s>(v>>1))*2.0f + static_cast<float32>(static_cast<int64s>(v & 1));}
    inline float64 int64u_float64 (int64u v) {return static_cast<float64>(static_cast<int64s>(v>>1))*2.0f + static_cast<float32>(static_cast<int64s>(v & 1));}
#else
    #if defined(_MSC_VER)
       #pragma warning( disable : 4244 )
    #endif
    inline float32 int64u_float32 (int64u v) {return v;}
    inline float64 int64u_float64 (int64u v) {return v;}
    #if defined(_MSC_VER)
        #pragma warning( default : 4244 )
    #endif
#endif // defined(_MSC_VER) && _MSC_VER<=1200

//---------------------------------------------------------------------------
// CC (often used in all containers to identify a stream
inline int64u CC8(const char*  C) {return BigEndian2int64u(C);}
inline int64u CC7(const char*  C) {return BigEndian2int56u(C);}
inline int64u CC6(const char*  C) {return BigEndian2int48u(C);}
inline int64u CC5(const char*  C) {return BigEndian2int40u(C);}
inline int32u CC4(const char*  C) {return BigEndian2int32u(C);}
inline int32u CC3(const char*  C) {return BigEndian2int24u(C);}
inline int16u CC2(const char*  C) {return BigEndian2int16u(C);}
inline int8u  CC1(const char*  C) {return BigEndian2int8u (C);}
#ifndef __BORLANDC__
inline int64u CC8(const int8u* C) {return BigEndian2int64u(C);}
inline int64u CC7(const int8u* C) {return BigEndian2int56u(C);}
inline int64u CC6(const int8u* C) {return BigEndian2int48u(C);}
inline int64u CC5(const int8u* C) {return BigEndian2int40u(C);}
inline int32u CC4(const int8u* C) {return BigEndian2int32u(C);}
inline int32u CC3(const int8u* C) {return BigEndian2int24u(C);}
inline int16u CC2(const int8u* C) {return BigEndian2int16u(C);}
inline int8u  CC1(const int8u* C) {return BigEndian2int8u (C);}
#endif // __BORLANDC__

//---------------------------------------------------------------------------
// turn a numeric literal into a hex constant
// (avoids problems with leading zeroes)
// 8-bit constants max value 0x11111111, always fits in unsigned long
#define HEX__(n) 0x##n##LU

// 8-bit conversion function
#define B8__(x) ((x&0x0000000FLU)?0x01:0) \
               +((x&0x000000F0LU)?0x02:0) \
               +((x&0x00000F00LU)?0x04:0) \
               +((x&0x0000F000LU)?0x08:0) \
               +((x&0x000F0000LU)?0x10:0) \
               +((x&0x00F00000LU)?0x20:0) \
               +((x&0x0F000000LU)?0x40:0) \
               +((x&0xF0000000LU)?0x80:0)

// for upto 8-bit binary constants
#define B8(d) ((int8u)B8__(HEX__(d)))

// for upto 16-bit binary constants, MSB first
#define B16(dmsb, dlsb) (((int16u)B8(dmsb)<<8) \
                       + ((int16u)B8(dlsb)<<0))

// for upto 32-bit binary constants, MSB first
#define B32(dmsb, db2, db3, dlsb) (((int32u)B8(dmsb)<<24) \
                                 + ((int32u)B8( db2)<<16) \
                                 + ((int32u)B8( db3)<< 8) \
                                 + ((int32u)B8(dlsb)<< 0))

} //namespace ZenLib
#endif
