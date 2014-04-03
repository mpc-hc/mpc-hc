/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
// Pre-compilation
#include "MediaInfo/PreComp.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Setup.h"
//---------------------------------------------------------------------------

#if !MEDIAINFO_TRACE
//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include "MediaInfo/MediaInfo_Config.h"
#include "ZenLib/BitStream_LE.h"
#include <cmath>
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
extern MediaInfo_Config Config;
//---------------------------------------------------------------------------

//Integrity test
#define INTEGRITY(TOVALIDATE) \
    if (!(TOVALIDATE)) \
    { \
        Trusted_IsNot(); \
        return; \
    } \

#define INTEGRITY_BOOL(TOVALIDATE) \
    if (!(TOVALIDATE)) \
    { \
        Trusted_IsNot(); \
        Info=false; \
        return; \
    } \

#define INTEGRITY_INT(TOVALIDATE) \
    if (!(TOVALIDATE)) \
    { \
        Trusted_IsNot(); \
        Info=0; \
        return; \
    } \

#define INTEGRITY_SIZE_ATLEAST(_BYTES) \
    if (Element_Offset+_BYTES>Element_Size) \
    { \
        Trusted_IsNot(); \
        return; \
    } \

#define INTEGRITY_SIZE_ATLEAST_STRING(_BYTES) \
    if (Element_Offset+_BYTES>Element_Size) \
    { \
        Trusted_IsNot(); \
        Info.clear(); \
        return; \
    } \

#define INTEGRITY_SIZE_ATLEAST_INT(_BYTES) \
    if (Element_Offset+_BYTES>Element_Size) \
    { \
        Trusted_IsNot(); \
        Info=0; \
        return; \
    } \

#define INTEGRITY_SIZE_ATLEAST_BUFFER() \
    if (BS->Remain()==0) \
    { \
        Trusted_IsNot(); \
        Info=0; \
        return; \
    } \

//***************************************************************************
// Init
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::BS_Begin()
{
    size_t BS_Size;
    if (Element_Offset>=Element_Size)
        BS_Size=0;
    else if (Buffer_Offset+Element_Size<=Buffer_Size)
        BS_Size=(size_t)(Element_Size-Element_Offset);
    else if (Buffer_Offset+Element_Offset<=Buffer_Size)
        BS_Size=Buffer_Size-(size_t)(Buffer_Offset+Element_Offset);
    else
        BS_Size=0;

    BS->Attach(Buffer+Buffer_Offset+(size_t)Element_Offset, BS_Size);
}

//---------------------------------------------------------------------------
void File__Analyze::BS_Begin_LE()
{
    size_t BS_Size;
    if (Buffer_Offset+Element_Size<=Buffer_Size)
        BS_Size=(size_t)(Element_Size-Element_Offset);
    else if (Buffer_Offset+Element_Offset<=Buffer_Size)
        BS_Size=Buffer_Size-(size_t)(Buffer_Offset+Element_Offset);
    else
        BS_Size=0;

    BT->Attach(Buffer+Buffer_Offset+(size_t)Element_Offset, BS_Size);
}

//---------------------------------------------------------------------------
void File__Analyze::BS_End()
{
    BS->Byte_Align();
    Element_Offset+=BS->Offset_Get();
    BS->Attach(NULL, 0);
}

//---------------------------------------------------------------------------
void File__Analyze::BS_End_LE()
{
    BT->Byte_Align();
    Element_Offset+=BT->Offset_Get();
    BT->Attach(NULL, 0);
}

//***************************************************************************
// Big Endian
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::Get_B1_(int8u  &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(1);
    Info=BigEndian2int8u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=1;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_B2_(int16u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(2);
    Info=BigEndian2int16u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=2;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_B3_(int32u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(3);
    Info=BigEndian2int24u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=3;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_B4_(int32u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(4);
    Info=BigEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=4;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_B5_(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(5);
    Info=BigEndian2int40u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=5;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_B6_(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(6);
    Info=BigEndian2int48u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=6;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_B7_(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(7);
    Info=BigEndian2int56u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=7;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_B8_(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(8);
    Info=BigEndian2int64u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=8;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_B16_(int128u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(16);
    //Info=BigEndian2int128u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Info.hi=BigEndian2int64u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Info.lo=BigEndian2int64u(Buffer+Buffer_Offset+(size_t)Element_Offset+8);
    Element_Offset+=16;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_BF4_(float32 &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(4);
    Info=BigEndian2float32(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=4;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_BF8_(float64 &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(8);
    Info=BigEndian2float64(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=8;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_BF10_(float80 &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(10);
    Info=BigEndian2float80(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=10;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_BFP4_(int8u  Bits, float32 &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(4);
    BS_Begin();
    int32s Integer=(int32s)BS->Get4(Bits);
    int32u Fraction=BS->Get4(32-Bits);
    BS_End();
    if (Integer>=(1<<Bits)/2)
        Integer-=1<<Bits;
    Info=Integer+((float32)Fraction)/(1<<(32-Bits));
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_B1(int8u  &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(1);
    Info=BigEndian2int8u(Buffer+Buffer_Offset+(size_t)Element_Offset);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_B2(int16u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(2);
    Info=BigEndian2int16u(Buffer+Buffer_Offset+(size_t)Element_Offset);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_B3(int32u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(3);
    Info=BigEndian2int24u(Buffer+Buffer_Offset+(size_t)Element_Offset);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_B4(int32u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(4);
    Info=BigEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_B5(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(5);
    Info=BigEndian2int40u(Buffer+Buffer_Offset+(size_t)Element_Offset);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_B6(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(6);
    Info=BigEndian2int48u(Buffer+Buffer_Offset+(size_t)Element_Offset);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_B7(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(7);
    Info=BigEndian2int56u(Buffer+Buffer_Offset+(size_t)Element_Offset);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_B8(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(8);
    Info=BigEndian2int64u(Buffer+Buffer_Offset+(size_t)Element_Offset);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_B16(int128u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(16);
    Info=BigEndian2int128u(Buffer+Buffer_Offset+(size_t)Element_Offset);
}

//***************************************************************************
// Little Endian
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::Get_L1_(int8u  &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(1);
    Info=LittleEndian2int8u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=1;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_L2_(int16u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(2);
    Info=LittleEndian2int16u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=2;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_L3_(int32u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(3);
    Info=LittleEndian2int24u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=3;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_L4_(int32u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(4);
    Info=LittleEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=4;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_L5_(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(5);
    Info=LittleEndian2int40u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=5;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_L6_(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(6);
    Info=LittleEndian2int48u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=6;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_L7_(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(7);
    Info=LittleEndian2int56u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=7;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_L8_(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(8);
    Info=LittleEndian2int64u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=8;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_L16_(int128u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(16);
    //Info=LittleEndian2int128u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Info.lo=LittleEndian2int64u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Info.hi=LittleEndian2int64u(Buffer+Buffer_Offset+(size_t)Element_Offset+8);
    Element_Offset+=16;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_LF4_(float32 &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(4);
    Info=LittleEndian2float32(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=4;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_LF8_(float64 &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(8);
    Info=LittleEndian2float64(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=8;
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_L1(int8u  &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(1);
    Info=LittleEndian2int8u(Buffer+Buffer_Offset+(size_t)Element_Offset);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_L2(int16u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(2);
    Info=LittleEndian2int16u(Buffer+Buffer_Offset+(size_t)Element_Offset);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_L3(int32u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(3);
    Info=LittleEndian2int24u(Buffer+Buffer_Offset+(size_t)Element_Offset);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_L4(int32u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(4);
    Info=LittleEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_L5(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(5);
    Info=LittleEndian2int40u(Buffer+Buffer_Offset+(size_t)Element_Offset);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_L6(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(6);
    Info=LittleEndian2int48u(Buffer+Buffer_Offset+(size_t)Element_Offset);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_L7(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(7);
    Info=LittleEndian2int56u(Buffer+Buffer_Offset+(size_t)Element_Offset);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_L8(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(8);
    Info=LittleEndian2int64u(Buffer+Buffer_Offset+(size_t)Element_Offset);
}

//***************************************************************************
// Little and Big Endian together
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::Get_D1_(int8u  &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(2);
    Info=LittleEndian2int8u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=2;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_D2_(int16u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(4);
    Info=LittleEndian2int16u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=4;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_D3_(int32u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(6);
    Info=LittleEndian2int24u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=6;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_D4_(int32u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(8);
    Info=LittleEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=8;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_D5_(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(10);
    Info=LittleEndian2int40u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=10;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_D6_(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(12);
    Info=LittleEndian2int48u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=12;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_D7_(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(14);
    Info=LittleEndian2int56u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=14;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_D8_(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(16);
    Info=LittleEndian2int64u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=16;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_D16_(int128u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(32);
    //Info=LittleEndian2int128u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Info.lo=LittleEndian2int64u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Info.hi=LittleEndian2int64u(Buffer+Buffer_Offset+(size_t)Element_Offset+8);
    Element_Offset+=32;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_DF4_(float32 &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(8);
    Info=LittleEndian2float32(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=8;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_DF8_(float64 &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(16);
    Info=LittleEndian2float64(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=16;
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_D1(int8u  &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(2);
    Info=LittleEndian2int8u(Buffer+Buffer_Offset+(size_t)Element_Offset);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_D2(int16u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(4);
    Info=LittleEndian2int16u(Buffer+Buffer_Offset+(size_t)Element_Offset);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_D3(int32u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(6);
    Info=LittleEndian2int24u(Buffer+Buffer_Offset+(size_t)Element_Offset);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_D4(int32u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(8);
    Info=LittleEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_D5(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(10);
    Info=LittleEndian2int40u(Buffer+Buffer_Offset+(size_t)Element_Offset);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_D6(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(12);
    Info=LittleEndian2int48u(Buffer+Buffer_Offset+(size_t)Element_Offset);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_D7(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(14);
    Info=LittleEndian2int56u(Buffer+Buffer_Offset+(size_t)Element_Offset);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_D8(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(16);
    Info=LittleEndian2int64u(Buffer+Buffer_Offset+(size_t)Element_Offset);
}

//***************************************************************************
// GUID
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::Get_GUID(int128u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(16);
    Info.hi=LittleEndian2int64u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Info.lo=BigEndian2int64u   (Buffer+Buffer_Offset+(size_t)Element_Offset+8);
    Element_Offset+=16;
}

//***************************************************************************
// EBML
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::Get_EB(int64u &Info)
{
    //Element size
    INTEGRITY_SIZE_ATLEAST_INT(1);
    if (Buffer[Buffer_Offset+Element_Offset]==0xFF)
    {
        Info=File_Size-(File_Offset+Buffer_Offset+Element_Offset);
        Element_Offset++;
        return;
    }
    int8u  Size=0;
    int8u  Size_Mark=0;
    BS_Begin();
    while (Size_Mark==0 && BS->Remain() && Size<=8)
    {
        Size++;
        Peek_S1(Size, Size_Mark);
    }

    //Integrity
    if (!BS->Remain() || Size>8)
    {
        if (Size>8)
        {
            //Element[Element_Level].IsComplete=true; //If it is in a header
            Trusted_IsNot("EBML integer parsing error");
        }
        Info=0;
        return;
    }
    BS_End();
    if (File_Offset+Buffer_Offset+Element_Offset>=Element[Element_Level].Next)
    {
        //Element[Element_Level].IsComplete=true; //If it is in a header
        Trusted_IsNot("Not enough place to have an EBML");
        Info=0;
        return; //Not enough space
    }
    INTEGRITY_SIZE_ATLEAST_INT(Size);

    //Element Name
    switch (Size)
    {
        case 1 : {
                    int8u Element_Name;
                    Peek_B1 (Element_Name);
                    Info=Element_Name&0x7F; //Keep out first bit
                 }
                 break;
        case 2 : {
                    int16u Element_Name;
                    Peek_B2(Element_Name);
                    Info=Element_Name&0x3FFF; //Keep out first bits
                 }
                 break;
        case 3 : {
                    int32u Element_Name;
                    Peek_B3(Element_Name);
                    Info=Element_Name&0x1FFFFF; //Keep out first bits
                 }
                 break;
        case 4 : {
                    int32u Element_Name;
                    Peek_B4(Element_Name);
                    Info=Element_Name&0x0FFFFFFF; //Keep out first bits
                 }
                 break;
        case 5 : {
                    int64u Element_Name;
                    Peek_B5(Element_Name);
                    Info=Element_Name&0x07FFFFFFFFLL; //Keep out first bits
                 }
                 break;
        case 6 : {
                    int64u Element_Name;
                    Peek_B6(Element_Name);
                    Info=Element_Name&0x03FFFFFFFFFFLL; //Keep out first bits
                 }
                 break;
        case 7 : {
                    int64u Element_Name;
                    Peek_B7(Element_Name);
                    Info=Element_Name&0x01FFFFFFFFFFFFLL; //Keep out first bits
                 }
                 break;
        case 8 : {
                    int64u Element_Name;
                    Peek_B8(Element_Name);
                    Info=Element_Name&0x00FFFFFFFFFFFFFFLL; //Keep out first bits
                 }
                 break;
    }

    Element_Offset+=Size;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_ES(int64s &Info)
{
    //Element size
    INTEGRITY_SIZE_ATLEAST_INT(1);
    int8u  Size=0;
    int8u  Size_Mark=0;
    BS_Begin();
    while (Size_Mark==0 && BS->Remain() && Size<=8)
    {
        Size++;
        Peek_S1(Size, Size_Mark);
    }

    //Integrity
    if (!BS->Remain() || Size>8)
    {
        if (Size>8)
        {
            //Element[Element_Level].IsComplete=true; //If it is in a header
            Trusted_IsNot("EBML integer parsing error");
        }
        Info=0;
        return;
    }
    BS_End();
    if (File_Offset+Buffer_Offset+Element_Offset>=Element[Element_Level].Next)
    {
        //Element[Element_Level].IsComplete=true; //If it is in a header
        Trusted_IsNot("Not enough place to have an EBML");
        Info=0;
        return; //Not enough space
    }
    INTEGRITY_SIZE_ATLEAST_INT(Size);

    //Element Name
    switch (Size)
    {
        case 1 : {
                    int8u Element_Name;
                    Peek_B1 (Element_Name);
                    Info=(Element_Name&0x7F)-0x3F; //Keep out first bit and sign
                 }
                 break;
        case 2 : {
                    int16u Element_Name;
                    Peek_B2(Element_Name);
                    Info=(Element_Name&0x3FFF)-0x1FFF; //Keep out first bits and sign
                 }
                 break;
        case 3 : {
                    int32u Element_Name;
                    Peek_B3(Element_Name);
                    Info=(Element_Name&0x1FFFFF)-0x0FFFFF; //Keep out first bits and sign
                 }
                 break;
        case 4 : {
                    int32u Element_Name;
                    Peek_B4(Element_Name);
                    Info=(Element_Name&0x0FFFFFFF)-0x07FFFFFF; //Keep out first bits and sign
                 }
                 break;
        case 5 : {
                    int64u Element_Name;
                    Peek_B5(Element_Name);
                    Info=(Element_Name&0x07FFFFFFFFLL)-0x03FFFFFFFFLL; //Keep out first bits and sign
                 }
                 break;
        case 6 : {
                    int64u Element_Name;
                    Peek_B6(Element_Name);
                    Info=(Element_Name&0x03FFFFFFFFFFLL)-0x01FFFFFFFFFFLL; //Keep out first bits and sign
                 }
                 break;
        case 7 : {
                    int64u Element_Name;
                    Peek_B7(Element_Name);
                    Info=(Element_Name&0x01FFFFFFFFFFFFLL)-0x00FFFFFFFFFFFFLL; //Keep out first bits and sign
                 }
                 break;
        case 8 : {
                    int64u Element_Name;
                    Peek_B8(Element_Name);
                    Info=(Element_Name&0x00FFFFFFFFFFFFFFLL)-0x007FFFFFFFFFFFFFLL; //Keep out first bits and sign
                 }
                 break;
    }

    Element_Offset+=Size;
}

//***************************************************************************
// Variable Length Value
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::Get_VS(int64u &Info)
{
    //Element size
    Info=0;
    int8u  Size=0;
    bool more_data;
    BS_Begin();
    do
    {
        Size++;
        INTEGRITY_INT(8<=BS->Remain())
        more_data=BS->GetB();
        Info=128*Info+BS->Get1(7);
    }
    while (more_data && Size<=8 && BS->Remain());
    BS_End();

    //Integrity
    if (Size>8)
    {
        Trusted_IsNot("Variable Length Value parsing error");
        Info=0;
        return;
    }
    if (File_Offset+Buffer_Offset+Element_Offset>=Element[Element_Level].Next)
    {
        Trusted_IsNot("Not enough place to have a Variable Length Value");
        Info=0;
        return; //Not enough space
    }
}

//---------------------------------------------------------------------------
void File__Analyze::Skip_VS()
{
    //Element size
    int64u Info=0;
    int8u  Size=0;
    bool more_data;
    BS_Begin();
    do
    {
        Size++;
        INTEGRITY_INT(8<=BS->Remain())
        more_data=BS->GetB();
        Info=128*Info+BS->Get1(7);
    }
    while (more_data && Size<=8 && BS->Remain());
    BS_End();

    //Integrity
    if (Size>8)
    {
        Trusted_IsNot("Variable Size Value parsing error");
        Info=0;
        return;
    }
    if (File_Offset+Buffer_Offset+Element_Offset>=Element[Element_Level].Next)
    {
        Trusted_IsNot("Not enough place to have a Variable Size Value");
        Info=0;
        return; //Not enough space
    }
}

//***************************************************************************
// Exp-Golomb
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::Get_SE(int32s &Info)
{
    INTEGRITY_SIZE_ATLEAST_BUFFER();
    int8u LeadingZeroBits=0;
    while(BS->Remain()>0 && !BS->GetB())
        LeadingZeroBits++;
    INTEGRITY(LeadingZeroBits<=32)
    double InfoD=pow((float)2, (float)LeadingZeroBits)-1+BS->Get4(LeadingZeroBits);
    INTEGRITY(InfoD<int32u(-1))
    Info=(int32s)(pow((double)-1, InfoD+1)*(int32u)ceil(InfoD/2));
}

//---------------------------------------------------------------------------
void File__Analyze::Get_UE(int32u &Info)
{
    INTEGRITY_SIZE_ATLEAST_BUFFER();
    int8u LeadingZeroBits=0;
    while(BS->Remain()>0 && !BS->GetB())
        LeadingZeroBits++;
    INTEGRITY(LeadingZeroBits<=32)
    double InfoD=pow((float)2, (float)LeadingZeroBits);
    Info=(int32u)InfoD-1+BS->Get4(LeadingZeroBits);
}

//---------------------------------------------------------------------------
void File__Analyze::Skip_UE()
{
    INTEGRITY(BS->Remain())
    int8u LeadingZeroBits=0;
    while(BS->Remain()>0 && !BS->GetB())
        LeadingZeroBits++;
    BS->Skip(LeadingZeroBits);
}

//***************************************************************************
// Inverted Exp-Golomb
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::Get_SI(int32s &Info)
{
    INTEGRITY_SIZE_ATLEAST_BUFFER();

    Info=1;
    while(BS->Remain()>0 && BS->GetB()==0)
    {
        Info<<=1;
        if (BS->Remain()==0)
        {
            Trusted_IsNot("(Problem)");
            Info=0;
            return;
        }
        if(BS->GetB()==1)
            Info++;
    }
    Info--;

    if (Info!=0 && BS->Remain()>0 && BS->GetB()==1)
        Info=-Info;
}

//---------------------------------------------------------------------------
void File__Analyze::Skip_SI()
{
    int32s Info;
    Get_SI(Info);
}

//---------------------------------------------------------------------------
void File__Analyze::Get_UI(int32u &Info)
{
    INTEGRITY_SIZE_ATLEAST_BUFFER();
    Info=1;
    while(BS->Remain()>0 && BS->GetB()==0)
    {
        Info<<=1;
        if (BS->Remain()==0)
        {
            Trusted_IsNot("(Problem)");
            Info=0;
            return;
        }
        if(BS->GetB()==1)
            Info++;
    }
    Info--;
}

//---------------------------------------------------------------------------
void File__Analyze::Skip_UI()
{
    int32u Info;
    Get_UI(Info);
}

//***************************************************************************
// Variable Length Code
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::Get_VL_(const vlc Vlc[], size_t &Info)
{
    Info=0;
    int32u Value=0;

    for(;;)
    {
        switch (Vlc[Info].bit_increment)
        {
            case 255 :
                        Trusted_IsNot();
                        return;
            default  : ;
                        Value<<=Vlc[Info].bit_increment;
                        Value|=BS->Get1(Vlc[Info].bit_increment);
                        break;
            case   1 :
                        Value<<=1;
                        if (BS->GetB())
                            Value++;
            case   0 :  ;
        }

        if (Value==Vlc[Info].value)
            return;
        Info++;
    }
}

//---------------------------------------------------------------------------
void File__Analyze::Skip_VL_(const vlc Vlc[])
{
    size_t Info=0;
    int32u Value=0;

    for(;;)
    {
        switch (Vlc[Info].bit_increment)
        {
            case 255 :
                        Trusted_IsNot();
                        return;
            default  : ;
                        Value<<=Vlc[Info].bit_increment;
                        Value|=BS->Get1(Vlc[Info].bit_increment);
                        break;
            case   1 :
                        Value<<=1;
                        if (BS->GetB())
                            Value++;
            case   0 :  ;
        }

        if (Value==Vlc[Info].value)
            return;
        Info++;
    }
}

//---------------------------------------------------------------------------
void File__Analyze::Get_VL_Prepare(vlc_fast &Vlc)
{
    Vlc.Array=new int8u[((size_t)1)<<Vlc.Size];
    Vlc.BitsToSkip=new int8u[((size_t)1)<<Vlc.Size];
    memset(Vlc.Array, 0xFF, ((size_t)1)<<Vlc.Size);
    int8u  Increment=0;
    int8u  Pos=0;
    for(; ; Pos++)
    {
        if (Vlc.Vlc[Pos].bit_increment==255)
            break;
        Increment+=Vlc.Vlc[Pos].bit_increment;
        size_t Value=Vlc.Vlc[Pos].value<<(Vlc.Size-Increment);
        size_t ToFill_Size=1<<(Vlc.Size-Increment);
        for (size_t ToFill_Pos=0; ToFill_Pos<ToFill_Size; ToFill_Pos++)
        {
            Vlc.Array[Value+ToFill_Pos]=Pos;
            Vlc.BitsToSkip[Value+ToFill_Pos]=Increment;
        }
    }
    for (size_t Pos2=0; Pos2<(((size_t)1)<<Vlc.Size); Pos2++)
        if (Vlc.Array[Pos2]==(int8u)-1)
        {
            Vlc.Array[Pos2]=Pos;
            Vlc.BitsToSkip[Pos2]=(int8u)-1;
        }
}

//---------------------------------------------------------------------------
void File__Analyze::Get_VL_(const vlc_fast &Vlc, size_t &Info)
{
    if (BS->Remain()<Vlc.Size)
    {
        Get_VL(Vlc.Vlc, Info, Name);
        return;
    }

    int32u Value=BS->Peek4(Vlc.Size);
    Info=Vlc.Array[Value];

    if (Vlc.BitsToSkip[Value]==(int8u)-1)
    {
        Trusted_IsNot();
        return;
    }

    BS->Skip(Vlc.BitsToSkip[Value]);
}

//---------------------------------------------------------------------------
void File__Analyze::Skip_VL_(const vlc_fast &Vlc)
{
    if (BS->Remain()<Vlc.Size)
    {
        Skip_VL_(Vlc.Vlc);
        return;
    }

    int32u Value=BS->Peek4(Vlc.Size);

    if (Vlc.BitsToSkip[Value]==(int8u)-1)
    {
        Trusted_IsNot();
        return;
    }

    BS->Skip(Vlc.BitsToSkip[Value]);
}

//***************************************************************************
// Characters
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::Get_C1(int8u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(1);
    Info=CC1(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=1;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_C2(int16u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(2);
    Info=CC2(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=2;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_C3(int32u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(3);
    Info=CC3(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=3;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_C4(int32u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(4);
    Info=CC4(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=4;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_C5(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(5);
    Info=CC5(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=5;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_C6(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(6);
    Info=CC6(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=6;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_C7(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(7);
    Info=CC7(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=7;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_C8(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(8);
    Info=CC8(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=8;
}

//***************************************************************************
// Text
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::Get_Local(int64u Bytes, Ztring &Info)
{
    INTEGRITY_SIZE_ATLEAST_STRING(Bytes);
    Info.From_Local((const char*)(Buffer+Buffer_Offset+(size_t)Element_Offset), (size_t)Bytes);
    Element_Offset+=Bytes;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_ISO_8859_1(int64u Bytes, Ztring &Info)
{
    INTEGRITY_SIZE_ATLEAST_STRING(Bytes);
    Info.From_ISO_8859_1((const char*)(Buffer+Buffer_Offset+(size_t)Element_Offset), (size_t)Bytes);
    Element_Offset+=Bytes;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_ISO_8859_2(int64u Bytes, Ztring &Info)
{
    INTEGRITY_SIZE_ATLEAST_STRING(Bytes);
    Info.clear();
    size_t End=Buffer_Offset+(size_t)(Element_Offset+Bytes);
    for (size_t Pos=Buffer_Offset+(size_t)Element_Offset; Pos<End; ++Pos)
    {
        switch (Buffer[Pos])
        {
            case 0xAD : Info+=__T('\x00AD'); break;
            case 0xF0 : Info+=__T('\x2116'); break;
            case 0xFD : Info+=__T('\x00A7'); break;
            default   : Info+=(Buffer[Pos]<=0xA0?0x0000:0x0360)+Buffer[Pos];
        }
    }
    Element_Offset+=Bytes;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_ISO_8859_5(int64u Bytes, Ztring &Info)
{
    INTEGRITY_SIZE_ATLEAST_STRING(Bytes);
    Info.From_ISO_8859_2((const char*)(Buffer+Buffer_Offset+(size_t)Element_Offset), (size_t)Bytes);
    Element_Offset+=Bytes;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_String(int64u Bytes, std::string &Info)
{
    INTEGRITY_SIZE_ATLEAST_STRING(Bytes);
    Info.assign((const char*)(Buffer+Buffer_Offset+(size_t)Element_Offset), (size_t)Bytes);
    Element_Offset+=Bytes;
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_Local(int64u Bytes, Ztring &Info)
{
    INTEGRITY_SIZE_ATLEAST_STRING(Bytes);
    Info.From_Local((const char*)(Buffer+Buffer_Offset+(size_t)Element_Offset), (size_t)Bytes);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_String(int64u Bytes, std::string &Info)
{
    INTEGRITY_SIZE_ATLEAST_STRING(Bytes);
    Info.assign((const char*)(Buffer+Buffer_Offset+(size_t)Element_Offset), (size_t)Bytes);
}

//---------------------------------------------------------------------------
void File__Analyze::Get_UTF8(int64u Bytes, Ztring &Info)
{
    INTEGRITY_SIZE_ATLEAST_STRING(Bytes);
    Info.From_UTF8((const char*)(Buffer+Buffer_Offset+(size_t)Element_Offset), (size_t)Bytes);
    Element_Offset+=Bytes;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_UTF16(int64u Bytes, Ztring &Info)
{
    INTEGRITY_SIZE_ATLEAST_STRING(Bytes);
    Info.From_UTF16((const char*)(Buffer+Buffer_Offset+(size_t)Element_Offset), (size_t)Bytes);
    Element_Offset+=Bytes;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_UTF16B(int64u Bytes, Ztring &Info)
{
    INTEGRITY_SIZE_ATLEAST_STRING(Bytes);
    Info.From_UTF16BE((const char*)(Buffer+Buffer_Offset+(size_t)Element_Offset), (size_t)Bytes);
    Element_Offset+=Bytes;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_UTF16L(int64u Bytes, Ztring &Info)
{
    INTEGRITY_SIZE_ATLEAST_STRING(Bytes);
    Info.From_UTF16LE((const char*)(Buffer+Buffer_Offset+(size_t)Element_Offset), (size_t)Bytes);
    Element_Offset+=Bytes;
}

//***************************************************************************
// Text
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::Skip_PA()
{
    INTEGRITY_SIZE_ATLEAST(1);
    int8u Size=Buffer[Buffer_Offset+Element_Offset];
    int8u Pad=(Size%2)?0:1;
    INTEGRITY_SIZE_ATLEAST(1+Size+Pad);
    Element_Offset+=(size_t)(1+Size+Pad);
}

//***************************************************************************
// Unknown
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::Skip_XX(int64u Bytes)
{
    if (Element_Offset+Bytes!=Element_TotalSize_Get()) //Exception for seek to end of the element
    {
        INTEGRITY_SIZE_ATLEAST(Bytes);
    }
    Element_Offset+=Bytes;
}

//***************************************************************************
// Flags
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::Get_Flags_ (int64u Flags, size_t Order, bool &Info)
{
    if (Flags&((int64u)1<<Order))
        Info=true;
    else
        Info=false;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_FlagsM_ (int64u ValueToPut, int8u &Info)
{
    Info=(int8u)ValueToPut;
}

//***************************************************************************
// BitStream
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::Get_BS_(int8u Bits, int32u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain())
    Info=BS->Get4(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Get_SB_(             bool &Info)
{
    INTEGRITY_INT(1<=BS->Remain())
    Info=BS->GetB();
}

//---------------------------------------------------------------------------
void File__Analyze::Get_S1_(int8u  Bits, int8u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain())
    Info=BS->Get1(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Get_S2_(int8u  Bits, int16u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain())
    Info=BS->Get2(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Get_S3_(int8u  Bits, int32u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain())
    Info=BS->Get4(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Get_S4_(int8u  Bits, int32u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain())
    Info=BS->Get4(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Get_S5_(int8u  Bits, int64u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain())
    Info=BS->Get8(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Get_S6_(int8u  Bits, int64u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain())
    Info=BS->Get8(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Get_S7_(int8u  Bits, int64u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain())
    Info=BS->Get8(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Get_S8_(int8u  Bits, int64u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain())
    Info=BS->Get8(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_BS(int8u  Bits, int32u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain())
    Info=BS->Peek4(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_SB(              bool &Info)
{
    INTEGRITY_INT(1<=BS->Remain())
    Info=BS->PeekB();
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_S1(int8u  Bits, int8u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain())
    Info=BS->Peek1(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_S2(int8u  Bits, int16u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain())
    Info=BS->Peek2(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_S3(int8u  Bits, int32u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain())
    Info=BS->Peek4(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_S4(int8u  Bits, int32u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain())
    Info=BS->Peek4(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_S5(int8u  Bits, int64u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain())
    Info=BS->Peek8(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_S6(int8u  Bits, int64u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain())
    Info=BS->Peek8(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_S7(int8u  Bits, int64u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain())
    Info=BS->Peek8(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_S8(int8u  Bits, int64u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain())
    Info=BS->Peek8(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Mark_0()
{
    INTEGRITY(1<=BS->Remain())
    if (BS->GetB())
        Element_DoNotTrust("Mark bit is wrong");
}

//---------------------------------------------------------------------------
void File__Analyze::Mark_1()
{
    INTEGRITY(1<=BS->Remain())
    if (!BS->GetB())
        Element_DoNotTrust("Mark bit is wrong");
}

//***************************************************************************
// BitStream (Little Endian)
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::Get_BT_(int8u Bits, int32u &Info)
{
    INTEGRITY_INT(Bits<=BT->Remain())
    Info=BT->Get(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Get_TB_(             bool &Info)
{
    INTEGRITY_INT(1<=BT->Remain())
    Info=BT->GetB();
}

//---------------------------------------------------------------------------
void File__Analyze::Get_T1_(int8u  Bits, int8u &Info)
{
    INTEGRITY_INT(Bits<=BT->Remain())
    Info=BT->Get1(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Get_T2_(int8u  Bits, int16u &Info)
{
    INTEGRITY_INT(Bits<=BT->Remain())
    Info=BT->Get2(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Get_T4_(int8u  Bits, int32u &Info)
{
    INTEGRITY_INT(Bits<=BT->Remain())
    Info=BT->Get4(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Get_T8_(int8u  Bits, int64u &Info)
{
    INTEGRITY_INT(Bits<=BT->Remain())
    Info=BT->Get8(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_BT(int8u  Bits, int32u &Info)
{
    INTEGRITY_INT(Bits<=BT->Remain())
    Info=BT->Peek(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_TB(              bool &Info)
{
    INTEGRITY_INT(1<=BT->Remain())
    Info=BT->PeekB();
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_T1(int8u  Bits, int8u &Info)
{
    INTEGRITY_INT(Bits<=BT->Remain())
    Info=BT->Peek1(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_T2(int8u  Bits, int16u &Info)
{
    INTEGRITY_INT(Bits<=BT->Remain())
    Info=BT->Peek2(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_T4(int8u  Bits, int32u &Info)
{
    INTEGRITY_INT(Bits<=BT->Remain())
    Info=BT->Peek4(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_T8(int8u  Bits, int64u &Info)
{
    INTEGRITY_INT(Bits<=BT->Remain())
    Info=BT->Peek8(Bits);
}

} //NameSpace
#endif //MEDIAINFO_TRACE
