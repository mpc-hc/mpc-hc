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

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_MZ_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Archive/File_Mz.h"
#include "ZenLib/Utils.h"
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const char* Mz_Machine(int16u Machine)
{
    switch (Machine)
    {
        case 0x014D : return "Intel i860";
        case 0x014C : return "Intel i386";
        case 0x0162 : return "MIPS R3000";
        case 0x0166 : return "MIPS R4000";
        case 0x0183 : return "DEC Alpha";
        case 0x0200 : return "Intel IA64";
        case 0x8664 : return "AMD x86-64";
        default  : return "";
    }
}

//***************************************************************************
// Static stuff
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Mz::FileHeader_Begin()
{
    //Element_Size
    if (Buffer_Size<2)
        return false; //Must wait for more data

    if (Buffer[0]!=0x4D //"MZ"
     || Buffer[1]!=0x5A)
    {
        Reject("MZ");
        return false;
    }

    //All should be OK...
    return true;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mz::Read_Buffer_Continue()
{
    //Parsing
    int32u lfanew;
    Element_Begin1("MZ");
    Skip_C2(                                                    "magic");
    Skip_L2(                                                    "cblp");
    Skip_L2(                                                    "cp");
    Skip_L2(                                                    "crlc");
    Skip_L2(                                                    "cparhdr");
    Skip_L2(                                                    "minalloc");
    Skip_L2(                                                    "maxalloc");
    Skip_L2(                                                    "ss");
    Skip_L2(                                                    "sp");
    Skip_L2(                                                    "csum");
    Skip_L2(                                                    "ip");
    Skip_L2(                                                    "cs");
    Skip_L2(                                                    "lsarlc");
    Skip_L2(                                                    "ovno");
    Skip_L2(                                                    "res");
    Skip_L2(                                                    "res");
    Skip_L2(                                                    "res");
    Skip_L2(                                                    "res");
    Skip_L2(                                                    "oemid");
    Skip_L2(                                                    "oeminfo");
    Skip_L2(                                                    "res2");
    Skip_L2(                                                    "res2");
    Skip_L2(                                                    "res2");
    Skip_L2(                                                    "res2");
    Skip_L2(                                                    "res2");
    Skip_L2(                                                    "res2");
    Skip_L2(                                                    "res2");
    Skip_L2(                                                    "res2");
    Skip_L2(                                                    "res2");
    Skip_L2(                                                    "res2");
    Get_L4 (lfanew,                                             "lfanew");

    //Computing
    if (lfanew>Element_Offset)
    {
        Skip_XX(lfanew-Element_Offset,                          "MZ data");
        Element_End0();
    }
    if (Element_Offset>lfanew)
    {
        Element_End0();
        Element_Offset=lfanew; //Multi usage off the first bytes
    }

    //Parsing
    int32u Signature, TimeDateStamp=0;
    int16u Machine=0, Characteristics=0;
    Peek_B4(Signature);
    if (Signature==0x50450000) //"PE"
    {
        Element_Begin1("PE");
        Skip_C4(                                                "Header");
        Get_L2 (Machine,                                        "Machine"); Param_Info1(Mz_Machine(Machine));
        Skip_L2(                                                "NumberOfSections");
        Get_L4 (TimeDateStamp,                                  "TimeDateStamp"); Param_Info1(Ztring().Date_From_Seconds_1970(TimeDateStamp));
        Skip_L4(                                                "PointerToSymbolTable");
        Skip_L4(                                                "NumberOfSymbols");
        Skip_L2(                                                "SizeOfOptionalHeader");
        Get_L2 (Characteristics,                                "Characteristics");
        Element_End0();
    }

    FILLING_BEGIN();
        Accept("MZ");

        Fill(Stream_General, 0, General_Format, "MZ");
        if (Characteristics&0x2000)
            Fill(Stream_General, 0, General_Format_Profile, "DLL");
        else if (Characteristics&0x0002)
            Fill(Stream_General, 0, General_Format_Profile, "Executable");
        Fill(Stream_General, 0, General_Format_Profile, Mz_Machine(Machine));
        if (TimeDateStamp)
            Fill(Stream_General, 0, General_Encoded_Date, Ztring().Date_From_Seconds_1970(TimeDateStamp));

        //No more need data
        Finish("MZ");
    FILLING_END();
}

} //NameSpace

#endif //MEDIAINFO_MZ_YES
