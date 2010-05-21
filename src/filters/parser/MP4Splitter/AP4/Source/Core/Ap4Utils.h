/*****************************************************************
|
|    AP4 - Utilities
|
|    Copyright 2002-2008 Axiomatic Systems, LLC
|
|
|    This file is part of Bento4/AP4 (MP4 Atom Processing Library).
|
|    Unless you have obtained Bento4 under a difference license,
|    this version of Bento4 is Bento4|GPL.
|    Bento4|GPL is free software; you can redistribute it and/or modify
|    it under the terms of the GNU General Public License as published by
|    the Free Software Foundation; either version 2, or (at your option)
|    any later version.
|
|    Bento4|GPL is distributed in the hope that it will be useful,
|    but WITHOUT ANY WARRANTY; without even the implied warranty of
|    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|    GNU General Public License for more details.
|
|    You should have received a copy of the GNU General Public License
|    along with Bento4|GPL; see the file COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
 ****************************************************************/

#ifndef _AP4_UTILS_H_
#define _AP4_UTILS_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Config.h"
#include "Ap4Types.h"
#include "Ap4Results.h"
#include "Ap4Config.h"

/*----------------------------------------------------------------------
|   non-inline functions
+---------------------------------------------------------------------*/
double   AP4_BytesToDoubleBE(const unsigned char* bytes);
AP4_UI64 AP4_BytesToUInt64BE(const unsigned char* bytes);
void AP4_BytesFromDoubleBE(unsigned char* bytes, double value);
void AP4_BytesFromUInt64BE(unsigned char* bytes, AP4_UI64 value);

/*----------------------------------------------------------------------
|   AP4_BytesToUInt32BE
+---------------------------------------------------------------------*/
inline AP4_UI32
AP4_BytesToUInt32BE(const unsigned char* bytes)
{
    return 
        ( ((AP4_UI32)bytes[0])<<24 ) |
        ( ((AP4_UI32)bytes[1])<<16 ) |
        ( ((AP4_UI32)bytes[2])<<8  ) |
        ( ((AP4_UI32)bytes[3])     );    
}

/*----------------------------------------------------------------------
|   AP4_BytesToInt32BE
+---------------------------------------------------------------------*/
inline AP4_SI32
AP4_BytesToInt32BE(const unsigned char* bytes)
{
    return AP4_BytesToUInt32BE(bytes);
}

/*----------------------------------------------------------------------
|   AP4_BytesToUInt24BE
+---------------------------------------------------------------------*/
inline AP4_UI32
AP4_BytesToUInt24BE(const unsigned char* bytes)
{
    return 
        ( ((AP4_UI32)bytes[0])<<16 ) |
        ( ((AP4_UI32)bytes[1])<<8  ) |
        ( ((AP4_UI32)bytes[2])     );    
}

/*----------------------------------------------------------------------
|   AP4_BytesToInt16BE
+---------------------------------------------------------------------*/
inline AP4_UI16
AP4_BytesToUInt16BE(const unsigned char* bytes)
{
    return 
        ( ((AP4_UI16)bytes[0])<<8  ) |
        ( ((AP4_UI16)bytes[1])     );    
}

/*----------------------------------------------------------------------
|   AP4_BytesToInt16BE
+---------------------------------------------------------------------*/
inline AP4_SI16
AP4_BytesToInt16BE(const unsigned char* bytes)
{
    return (AP4_SI16)AP4_BytesToUInt16BE(bytes);
}

/*----------------------------------------------------------------------
|   AP4_BytesFromUInt32BE
+---------------------------------------------------------------------*/
inline void
AP4_BytesFromUInt32BE(unsigned char* bytes, AP4_UI32 value)
{
    bytes[0] = (unsigned char)(value >> 24);
    bytes[1] = (unsigned char)(value >> 16);
    bytes[2] = (unsigned char)(value >>  8);
    bytes[3] = (unsigned char)(value      );
}

/*----------------------------------------------------------------------
|   AP4_BytesFromUInt24BE
+---------------------------------------------------------------------*/
inline void
AP4_BytesFromUInt24BE(unsigned char* bytes, AP4_UI32 value)
{
    bytes[0] = (unsigned char)(value >> 16);
    bytes[1] = (unsigned char)(value >>  8);
    bytes[2] = (unsigned char)(value      );
}

/*----------------------------------------------------------------------
|   AP4_BytesFromUInt16BE
+---------------------------------------------------------------------*/
inline void
AP4_BytesFromUInt16BE(unsigned char* bytes, AP4_UI16 value)
{
    bytes[0] = (unsigned char)(value >> 8);
    bytes[1] = (unsigned char)(value     );
}

/*----------------------------------------------------------------------
|   time functions
+---------------------------------------------------------------------*/
AP4_UI32 AP4_DurationMsFromUnits(AP4_UI64 units,
                                 AP4_UI32 units_per_second);
AP4_UI64 AP4_ConvertTime(AP4_UI64 time_value,
                         AP4_UI32 from_time_scale,
                         AP4_UI32 to_time_scale);

/*----------------------------------------------------------------------
|   string utils
+---------------------------------------------------------------------*/
#if defined (AP4_CONFIG_HAVE_STDIO_H)
#include <stdio.h>
#endif

#if defined (AP4_CONFIG_HAVE_SNPRINTF)
#define AP4_FormatString AP4_snprintf
#else
int AP4_FormatString(char* str, AP4_Size size, const char* format, ...);
#endif
#if defined(AP4_CONFIG_HAVE_VSNPRINTF)
#define AP4_FormatStringVN(s,c,f,a) AP4_vsnprintf(s,c,f,a)
#else
extern int AP4_FormatStringVN(char *buffer, size_t count, const char *format, va_list argptr);
#endif

#if defined (AP4_CONFIG_HAVE_STRING_H)
#include <string.h>
#define AP4_StringLength(x) strlen(x)
#define AP4_CopyMemory(x,y,z) memcpy(x,y,z)
#define AP4_CompareMemory(x, y, z) memcmp(x, y, z)
#define AP4_SetMemory(x,y,z) memset(x,y,z)
#define AP4_CompareStrings(x,y) strcmp(x,y)
#endif

unsigned char AP4_HexNibble(char c);
char AP4_NibbleHex(unsigned int nibble);
void AP4_FormatFourChars(char* str, AP4_UI32 value);
void AP4_FormatFourCharsPrintable(char* str, AP4_UI32 value);
AP4_Result
AP4_ParseHex(const char* hex, unsigned char* bytes, unsigned int count);
AP4_Result
AP4_FormatHex(const AP4_UI08* data, unsigned int data_size, char* hex);
AP4_Result
AP4_SplitArgs(char* arg, char*& arg0, char*& arg1, char*& arg2);
AP4_Result
AP4_SplitArgs(char* arg, char*& arg0, char*& arg1);

/*----------------------------------------------------------------------
|   AP4_BitWriter
+---------------------------------------------------------------------*/
class AP4_BitWriter
{
public:
    AP4_BitWriter(AP4_Size size) : m_DataSize(size), m_BitCount(0) {
        if (size) {
            m_Data = new unsigned char[size];
            AP4_SetMemory(m_Data, 0, size);
        } else {
            m_Data = NULL;
        }
    }
    ~AP4_BitWriter() { delete m_Data; }
    
    void Write(AP4_UI32 bits, unsigned int bit_count);
    
    unsigned int GetBitCount()     { return m_BitCount; }
    const unsigned char* GetData() { return m_Data;     }
    
private:
    unsigned char* m_Data;
    unsigned int   m_DataSize;
    unsigned int   m_BitCount;
};


#endif // _AP4_UTILS_H_
