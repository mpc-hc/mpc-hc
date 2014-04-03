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
#if defined(MEDIAINFO_ELF_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Archive/File_Elf.h"
#include "ZenLib/Utils.h"
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const char* Elf_osabi(int8u osabi)
{
    switch (osabi)
    {
        case   0 : return "UNIX System V ABI";
        case   1 : return "HP-UX";
        case   2 : return "NetBSD";
        case   3 : return "Linux";
        case   6 : return "Sun Solaris";
        case   7 : return "IBM AIX";
        case   8 : return "SGI Irix";
        case   9 : return "FreeBSD";
        case  10 : return "Compaq TRU64 UNIX";
        case  11 : return "Novell Modesto";
        case  12 : return "OpenBSD";
        case  97 : return "ARM";
        case 255 : return "Standalone";
        default  : return "";
    }
}

//---------------------------------------------------------------------------
const char* Elf_type(int16u type)
{
    switch (type)
    {
        case   1 : return "Relocatable";
        case   2 : return "Executable";
        case   3 : return "Shared object";
        case   4 : return "Core";
        default  : return "";
    }
}

//---------------------------------------------------------------------------
const char* Elf_machine(int16u machine)
{
    switch (machine)
    {
        case   1 : return "AT&T WE 32100";
        case   2 : return "SUN SPARC";
        case   3 : return "Intel i386";
        case   4 : return "Motorola m68k";
        case   5 : return "Motorola m88k";
        case   7 : return "Intel i860";
        case   8 : return "MIPS R3000";
        case   9 : return "IBM System/370";
        case  10 : return "MIPS R3000";

        case  15 : return "HPPA";
        case  17 : return "Fujitsu VPP500";
        case  18 : return "Sun v8plus";
        case  19 : return "Intel i960";
        case  20 : return "PowerPC";
        case  21 : return "PowerPC 64-bit";
        case  22 : return "IBM S390";

        case  36 : return "NEC V800";
        case  37 : return "Fujitsu FR20";
        case  38 : return "TRW RH-32";
        case  39 : return "Motorola RCE";
        case  40 : return "ARM";
        case  41 : return "DEC Alpha";
        case  42 : return "Hitachi SH";
        case  43 : return "SPARC v9 64-bit";
        case  44 : return "Siemens Tricore";
        case  45 : return "Argonaut RISC Core";
        case  46 : return "Hitachi H8/300";
        case  47 : return "Hitachi H8/300H";
        case  48 : return "Hitachi H8S";
        case  49 : return "Hitachi H8/500";
        case  50 : return "Intel IA64";
        case  51 : return "Stanford MIPS-X";
        case  52 : return "Motorola Coldfire";
        case  53 : return "Motorola M68HC12";
        case  54 : return "Fujitsu MMA";
        case  55 : return "Siemens PCP";
        case  56 : return "Sony nCPU";
        case  57 : return "Denso NDR1";
        case  58 : return "Motorola Start*Core";
        case  59 : return "Toyota ME16";
        case  60 : return "STMicroelectronic ST100";
        case  61 : return "Advanced Logic Corp. Tinyj";
        case  62 : return "AMD x86-64";
        case  63 : return "Sony DSP";

        case  66 : return "Siemens FX66";
        case  67 : return "STMicroelectronics ST9+";
        case  68 : return "STmicroelectronics ST7";
        case  69 : return "Motorola MC68HC16";
        case  70 : return "Motorola MC68HC11";
        case  71 : return "Motorola MC68HC08";
        case  72 : return "Motorola MC68HC05";
        case  73 : return "Silicon Graphics SVx";
        case  74 : return "STMicroelectronics ST19";
        case  75 : return "DEC VAX";
        case  76 : return "Axis Communications 32-bit";
        case  77 : return "Infineon Technologies 32-bit";
        case  78 : return "Element 14 64-bit";
        case  79 : return "LSI Logic 16-bit";
        case  80 : return "Donald Knuth's educational 64-bit";
        case  81 : return "Harvard University machine-independent";
        case  82 : return "SiTera Prism";
        case  83 : return "Atmel AVR 8-bit";
        case  84 : return "Fujitsu FR30";
        case  85 : return "Mitsubishi D10V";
        case  86 : return "Mitsubishi D30V";
        case  87 : return "NEC v850";
        case  88 : return "Mitsubishi M32R";
        case  89 : return "Matsushita MN10300";
        case  90 : return "Matsushita MN10200";
        case  91 : return "picoJava";
        case  92 : return "OpenRISC 32-bit";
        case  93 : return "ARC Cores Tangent-A5";
        case  94 : return "Tensilica Xtensa";
        default  : return "";
    }
}

//***************************************************************************
// Static stuff
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Elf::FileHeader_Begin()
{
    //Element_Size
    if (Buffer_Size<4)
        return false; //Must wait for more data

    if (Buffer[0]!=0x7F //".ELF"
     || Buffer[1]!=0x45
     || Buffer[2]!=0x4C
     || Buffer[3]!=0x46)
    {
        Reject("ELF");
        return false;
    }

    //All should be OK...
    return true;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Elf::Read_Buffer_Continue()
{
    //Parsing
    int32u version4=(int32u)-1;
    int16u type=(int16u)-1, machine=(int16u)-1;
    int8u  classs, data, version1, osabi, abiversion;
    Skip_C4(                                                    "magic");
    Get_L1 (classs,                                             "class");
    Get_L1 (data,                                               "data");
    Get_L1 (version1,                                           "version");
    Get_L1 (osabi,                                              "osabi"); Param_Info1(Elf_osabi(osabi));
    Get_L1 (abiversion,                                         "abiversion");
    Skip_XX(7,                                                  "reserved");
    if (data==1) //LE
    {
        Get_L2 (type,                                           "type"); Param_Info1(Elf_type(type));
        Get_L2 (machine,                                        "machine"); Param_Info1(Elf_machine(machine));
        Get_L4 (version4,                                       "version");
    }
    if (data==2) //BE
    {
        Get_B2 (type,                                           "type"); Param_Info1(Elf_type(type));
        Get_B2 (machine,                                        "machine"); Param_Info1(Elf_machine(machine));
        Get_B4 (version4,                                       "version");
    }
    Skip_XX(Element_Size-Element_Offset,                        "Data");

    FILLING_BEGIN();
        if (version4!=(int32u)-1 && version1!=version4)
        {
            Reject("ELF");
            return;
        }

        Accept("ELF");

        Fill(Stream_General, 0, General_Format, "ELF");
        if (type!=(int16u)-1)
            Fill(Stream_General, 0, General_Format_Profile, Elf_type(type));
        if (machine!=(int16u)-1)
            Fill(Stream_General, 0, General_Format_Profile, Elf_machine(machine));

        //No need of more
        Finish("ELF");
    FILLING_END();
}

} //NameSpace

#endif //MEDIAINFO_ELF_YES
