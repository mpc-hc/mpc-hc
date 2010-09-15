// File__Analyze - Base for analyze files
// Copyright (C) 2007-2010 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
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
#define INTEGRITY(TOVALIDATE, ERRORTEXT, OFFSET) \
    if (!(TOVALIDATE)) \
    { \
        Trusted_IsNot(ERRORTEXT); \
        Element_Offset=Element_Size; \
        return; \
    } \

#define INTEGRITY_INT(TOVALIDATE, ERRORTEXT, OFFSET) \
    if (!(TOVALIDATE)) \
    { \
        Trusted_IsNot(ERRORTEXT); \
        Element_Offset=Element_Size; \
        Info=0; \
        return; \
    } \

#define INTEGRITY_SIZE_ATLEAST(_BYTES) \
    if (Element_Offset+_BYTES>Element_Size) \
    { \
        Trusted_IsNot("Size is wrong"); \
        Element_Offset=Element_Size; \
        return; \
    } \

#define INTEGRITY_SIZE_ATLEAST_STRING(_BYTES) \
    if (Element_Offset+_BYTES>Element_Size) \
    { \
        Trusted_IsNot("Size is wrong"); \
        Element_Offset=Element_Size; \
        Info.clear(); \
        return; \
    } \

#define INTEGRITY_SIZE_ATLEAST_INT(_BYTES) \
    if (Element_Offset+_BYTES>Element_Size) \
    { \
        Trusted_IsNot("Size is wrong"); \
        Element_Offset=Element_Size; \
        Info=0; \
        return; \
    } \

#define INTEGRITY_SIZE_ATLEAST_BUFFER() \
    if (BS->Remain()==0) \
    { \
        Trusted_IsNot("Size is wrong"); \
        Element_Offset=Element_Size; \
        Info=0; \
        return; \
    } \

//***************************************************************************
// Init
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::BS_Begin()
{
    if (Element_Offset>Element_Size)
        return; //There is a problem

    size_t BS_Size;
    if (Buffer_Offset+Element_Size<=Buffer_Size)
        BS_Size=(size_t)(Element_Size-Element_Offset);
    else
        BS_Size=Buffer_Size-(size_t)(Buffer_Offset+Element_Offset);
    BS->Attach(Buffer+Buffer_Offset+(size_t)Element_Offset, BS_Size);
}

//---------------------------------------------------------------------------
void File__Analyze::BS_Begin_LE()
{
    //Change the bitstream for Little Endian version
    delete BS; BS=(BitStream*)new BitStream_LE();

    BS_Begin();
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
    BS_End();

    //Change the bitstream for the normal one
    delete BS; BS=new BitStream;
}

//***************************************************************************
// Big Endian
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::Get_B1(int8u  &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(1);
    Info=BigEndian2int8u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=1;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_B2(int16u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(2);
    Info=BigEndian2int16u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=2;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_B3(int32u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(3);
    Info=BigEndian2int24u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=3;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_B4(int32u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(4);
    Info=BigEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=4;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_B5(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(5);
    Info=BigEndian2int40u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=5;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_B6(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(6);
    Info=BigEndian2int48u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=6;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_B7(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(7);
    Info=BigEndian2int56u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=7;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_B8(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(8);
    Info=BigEndian2int64u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=8;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_B16(int128u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(16);
    //Info=BigEndian2int128u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Info.hi=BigEndian2int64u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Info.lo=BigEndian2int64u(Buffer+Buffer_Offset+(size_t)Element_Offset+8);
    Element_Offset+=16;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_BF4(float32 &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(4);
    Info=BigEndian2float32(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=4;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_BF8(float64 &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(8);
    Info=BigEndian2float64(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=8;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_BF10(float80 &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(10);
    Info=BigEndian2float80(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=10;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_BFP4(size_t Bits, float32 &Info)
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
void File__Analyze::Get_L1(int8u  &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(1);
    Info=LittleEndian2int8u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=1;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_L2(int16u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(2);
    Info=LittleEndian2int16u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=2;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_L3(int32u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(3);
    Info=LittleEndian2int24u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=3;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_L4(int32u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(4);
    Info=LittleEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=4;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_L5(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(5);
    Info=LittleEndian2int40u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=5;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_L6(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(6);
    Info=LittleEndian2int48u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=6;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_L7(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(7);
    Info=LittleEndian2int56u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=7;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_L8(int64u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(8);
    Info=LittleEndian2int64u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=8;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_L16(int128u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(16);
    //Info=LittleEndian2int128u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Info.lo=LittleEndian2int64u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Info.hi=LittleEndian2int64u(Buffer+Buffer_Offset+(size_t)Element_Offset+8);
    Element_Offset+=16;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_LF4(float32 &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(4);
    Info=LittleEndian2float32(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Element_Offset+=4;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_LF8(float64 &Info)
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
// UUID
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::Get_UUID(int128u &Info)
{
    INTEGRITY_SIZE_ATLEAST_INT(16);
    Info.hi=BigEndian2int64u(Buffer+Buffer_Offset+(size_t)Element_Offset);
    Info.lo=BigEndian2int64u(Buffer+Buffer_Offset+(size_t)Element_Offset+8);
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
    int32u Size=0;
    int32u Size_Mark=0;
    BS_Begin();
    while (Size_Mark==0 && BS->Remain() && Size<=8)
    {
        Size++;
        Peek_BS(Size, Size_Mark);
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
    int32u Size=0;
    int32u Size_Mark=0;
    BS_Begin();
    while (Size_Mark==0 && BS->Remain() && Size<=8)
    {
        Size++;
        Peek_BS(Size, Size_Mark);
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
        INTEGRITY_INT(8<=BS->Remain(), "Size is wrong", BS->Offset_Get())
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
        INTEGRITY_INT(8<=BS->Remain(), "Size is wrong", BS->Offset_Get())
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
    int LeadingZeroBits=0;
    while(BS->Remain()>0 && BS->Get(1)==0)
        LeadingZeroBits++;
    INTEGRITY(LeadingZeroBits<=32, "(Problem)", 0)
    double InfoD=pow((float)2, (float)LeadingZeroBits)-1+BS->Get(LeadingZeroBits);
    INTEGRITY(InfoD<int32u(-1), "(Problem)", 0)
    Info=(int32s)(pow((double)-1, InfoD+1)*(int32u)ceil(InfoD/2));
}

//---------------------------------------------------------------------------
void File__Analyze::Skip_SE()
{
    INTEGRITY(BS->Remain(), "Size is wrong", 0)
    int LeadingZeroBits=0;
    while(BS->Remain()>0 && BS->Get(1)==0)
        LeadingZeroBits++;
    BS->Skip(LeadingZeroBits);
}

//---------------------------------------------------------------------------
void File__Analyze::Get_UE(int32u &Info)
{
    INTEGRITY_SIZE_ATLEAST_BUFFER();
    int LeadingZeroBits=0;
    while(BS->Remain()>0 && BS->Get(1)==0)
        LeadingZeroBits++;
    INTEGRITY(LeadingZeroBits<=32, "(Problem)", 0)
    double InfoD=pow((float)2, (float)LeadingZeroBits);
    Info=(int32u)InfoD-1+BS->Get(LeadingZeroBits);
}

//---------------------------------------------------------------------------
void File__Analyze::Skip_UE()
{
    INTEGRITY(BS->Remain(), "Size is wrong", 0)
    int LeadingZeroBits=0;
    while(BS->Remain()>0 && BS->Get(1)==0)
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
void File__Analyze::Get_VL(int32u Call(int8u Size, int32u ToCall), int32u &Info)
{
    //Element size
    Info=0;
    int32u Code=0;
    int8u  Size=0;
    do
    {
        Size++;
        INTEGRITY_INT(1<BS->Remain(), "Size is wrong", BS->Offset_Get())
            Code=(Code<<1)|(BS->GetB()?1:0);
        Info=Call(Size, Code);
        if (Info!=(int32u)-1)
            break;
    }
    while (Size<=32);

    //Integrity
    if (Size>32)
    {
        Trusted_IsNot("Variable Length Code error");
        Info=0;
        return;
    }
}

//---------------------------------------------------------------------------
void File__Analyze::Skip_VL(int32u Call(int8u Size, int32u ToCall))
{
    int32u Info;
    Get_VL(Call, Info);
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
// Flags
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::Get_Flags (int64u Flags, size_t Order, bool &Info)
{
    if (Flags&((int64u)1<<Order))
        Info=true;
    else
        Info=false;
}

//---------------------------------------------------------------------------
void File__Analyze::Get_Flags (int64u ValueToPut, int8u &Info)
{
    Info=(int8u)ValueToPut;
}

//***************************************************************************
// BitStream
//***************************************************************************

//---------------------------------------------------------------------------
void File__Analyze::Get_BS(size_t Bits, int32u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain(), "Size is wrong", BS->Offset_Get())
    Info=BS->Get(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Get_SB(             bool &Info)
{
    INTEGRITY_INT(1<=BS->Remain(), "Size is wrong", BS->Offset_Get())
    Info=BS->GetB();
}

//---------------------------------------------------------------------------
void File__Analyze::Get_S1(size_t Bits, int8u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain(), "Size is wrong", BS->Offset_Get())
    Info=BS->Get1(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Get_S2(size_t Bits, int16u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain(), "Size is wrong", BS->Offset_Get())
    Info=BS->Get2(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Get_S3(size_t Bits, int32u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain(), "Size is wrong", BS->Offset_Get())
    Info=BS->Get4(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Get_S4(size_t Bits, int32u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain(), "Size is wrong", BS->Offset_Get())
    Info=BS->Get4(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Get_S5(size_t Bits, int64u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain(), "Size is wrong", BS->Offset_Get())
    Info=BS->Get8(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Get_S6(size_t Bits, int64u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain(), "Size is wrong", BS->Offset_Get())
    Info=BS->Get8(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Get_S7(size_t Bits, int64u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain(), "Size is wrong", BS->Offset_Get())
    Info=BS->Get8(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Get_S8(size_t Bits, int64u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain(), "Size is wrong", BS->Offset_Get())
    Info=BS->Get8(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_BS(size_t Bits, int32u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain(), "Size is wrong", BS->Offset_Get())
    Info=BS->Peek(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_SB(              bool &Info)
{
    INTEGRITY_INT(1<=BS->Remain(), "Size is wrong", BS->Offset_Get())
    Info=BS->PeekB();
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_S1(size_t Bits, int8u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain(), "Size is wrong", BS->Offset_Get())
    Info=BS->Peek1(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_S2(size_t Bits, int16u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain(), "Size is wrong", BS->Offset_Get())
    Info=BS->Peek2(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_S3(size_t Bits, int32u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain(), "Size is wrong", BS->Offset_Get())
    Info=BS->Peek4(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_S4(size_t Bits, int32u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain(), "Size is wrong", BS->Offset_Get())
    Info=BS->Peek4(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_S5(size_t Bits, int64u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain(), "Size is wrong", BS->Offset_Get())
    Info=BS->Peek8(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_S6(size_t Bits, int64u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain(), "Size is wrong", BS->Offset_Get())
    Info=BS->Peek8(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_S7(size_t Bits, int64u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain(), "Size is wrong", BS->Offset_Get())
    Info=BS->Peek8(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Peek_S8(size_t Bits, int64u &Info)
{
    INTEGRITY_INT(Bits<=BS->Remain(), "Size is wrong", BS->Offset_Get())
    Info=BS->Peek8(Bits);
}

//---------------------------------------------------------------------------
void File__Analyze::Mark_0_NoTrustError()
{
    INTEGRITY(1<=BS->Remain(), "Size is wrong", BS->Offset_Get())
    BS->GetB();
}

//---------------------------------------------------------------------------
void File__Analyze::Mark_0()
{
    INTEGRITY(1<=BS->Remain(), "Size is wrong", BS->Offset_Get())
    if (BS->GetB())
        Element_DoNotTrust("Mark bit is wrong");
}

//---------------------------------------------------------------------------
void File__Analyze::Mark_1()
{
    INTEGRITY(1<=BS->Remain(), "Size is wrong", BS->Offset_Get())
    if (!BS->GetB())
        Element_DoNotTrust("Mark bit is wrong");
}

//---------------------------------------------------------------------------
void File__Analyze::Mark_1_NoTrustError()
{
    INTEGRITY(1<=BS->Remain(), "Size is wrong", BS->Offset_Get())
    BS->GetB();
}

} //NameSpace
#endif //MEDIAINFO_TRACE

