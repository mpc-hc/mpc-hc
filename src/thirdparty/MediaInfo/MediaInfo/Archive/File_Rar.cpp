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
#if defined(MEDIAINFO_RAR_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Archive/File_Rar.h"
#include "ZenLib/Utils.h"
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************


//---------------------------------------------------------------------------
const char* Rar_host_os[6]=
{
    "MS DOS",
    "OS/2",
    "Win32",
    "Unix",
    "Mac OS",
    "BeOS"
};

//---------------------------------------------------------------------------
const char* Rar_packing_method[6]=
{
    "storing",
    "fastest compression",
    "fast compression",
    "normal compression",
    "good compression",
    "best compression"
};

//---------------------------------------------------------------------------
const char* Rar_HEADER_TYPE(int8u HEADER_TYPE)
{
    switch(HEADER_TYPE)
    {
        case 0x72 : return "marker block";
        case 0x73 : return "archive header";
        case 0x74 : return "file header";
        case 0x75 : return "old style comment header";
        case 0x76 : return "old style authenticity information";
        case 0x77 : return "old style subblock";
        case 0x78 : return "old style recovery record";
        case 0x79 : return "old style authenticity informatio";
        case 0x7A : return "subblock";
        case 0x7B : return "end of file";
        default   : return "";
    }
};

//---------------------------------------------------------------------------
Ztring Rar_version_number(int8u byte)
{
    //Version number is encoded as 10 * Major version + minor version.
    return Ztring::ToZtring(byte/10)+Ztring(".")+Ztring::ToZtring(byte%10);
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Rar::File_Rar()
:File__Analyze()
{
    //Configuration
    DataMustAlwaysBeComplete=false;
}

//***************************************************************************
// Static stuff
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Rar::FileHeader_Begin()
{
    //Element_Size
    if (Buffer_Size<7)
        return false; //Must wait for more data

    if (Buffer[0]!=0x52 //"Rar!"
     || Buffer[1]!=0x61
     || Buffer[2]!=0x72
     || Buffer[3]!=0x21
     || Buffer[4]!=0x1A
     || Buffer[5]!=0x07
     || Buffer[6]!=0x00)
    {
        Reject("RAR");
        return false;
    }

    state=0;

    //All should be OK...
    return true;
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Rar::Header_Begin()
{
    if (Element_Offset+7>Element_Size)
        return false; //Not enough data for header size
    int16u HEAD_SIZE=LittleEndian2int16u(Buffer+Buffer_Offset+(size_t)Element_Offset+5);

    if (Element_Offset+HEAD_SIZE>Element_Size)
        return false; //Not enough data for header

    return true;
}

//---------------------------------------------------------------------------
void File_Rar::Header_Parse()
{
    //Config
    HIGH_PACK_SIZE=0;
    PACK_SIZE=0;

    //Parsing
    int16u HEAD_SIZE;
    Skip_L2(                                                    "HEAD_CRC"); //CRC of total block or block part
    Get_L1 (HEAD_TYPE,                                          "HEAD_TYPE"); //Block type
    Get_L2 (HEAD_FLAGS,                                         "HEAD_FLAGS");
    Header_Parse_Flags();
    Get_L2 (HEAD_SIZE,                                          "HEAD_SIZE");
    Header_Parse_Content();
    Skip_XX(HEAD_SIZE-(size_t)Element_Offset,                   "REST OF HEADER");

    //Filling
    Header_Fill_Size(HEAD_SIZE+HIGH_PACK_SIZE*0x100000000LL+PACK_SIZE);
    Header_Fill_Code(HEAD_TYPE, Rar_HEADER_TYPE(HEAD_TYPE));
}

//---------------------------------------------------------------------------
void File_Rar::Header_Parse_Flags()
{
    switch (HEAD_TYPE)
    {
        case 0x73 : Header_Parse_Flags_73(); break;
        case 0x74 : Header_Parse_Flags_74(); break;
        default   : Header_Parse_Flags_XX();
    }
}

//---------------------------------------------------------------------------
// archive header
void File_Rar::Header_Parse_Flags_73()
{
    Skip_Flags(HEAD_FLAGS, 1,                                   "Volume attribute (archive volume)");
    Skip_Flags(HEAD_FLAGS, 2,                                   "Archive comment present");
    Skip_Flags(HEAD_FLAGS, 3,                                   "Archive lock attribute");
    Skip_Flags(HEAD_FLAGS, 4,                                   "Solid attribute (solid archive)");
    Skip_Flags(HEAD_FLAGS, 5,                                   "New volume naming scheme"); // (\'volname.partN.rar\')
    Skip_Flags(HEAD_FLAGS, 6,                                   "Authenticity information present");
    Skip_Flags(HEAD_FLAGS, 7,                                   "Recovery record present");
    Skip_Flags(HEAD_FLAGS, 8,                                   "Block headers are encrypted");
    Skip_Flags(HEAD_FLAGS, 9,                                   "First volume (set only by RAR 3.0 and later)");
    Get_Flags (HEAD_FLAGS, 15, add_size,                        "ADD_SIZE present");
}

//---------------------------------------------------------------------------
// file header
void File_Rar::Header_Parse_Flags_74()
{
    Skip_Flags(HEAD_FLAGS,0,                                    "file continued from previous volume");
    Skip_Flags(HEAD_FLAGS,1,                                    "file continued in next volume");
    Skip_Flags(HEAD_FLAGS,2,                                    "file encrypted with password");
    Skip_Flags(HEAD_FLAGS,3,                                    "file comment present");
    Skip_Flags(HEAD_FLAGS,4,                                    "information from previous files is used"); // (solid flag)
               // bits 7 6 5 (for RAR 2.0 and later)
               //      0 0 0    - dictionary size   64 KB
               //      0 0 1    - dictionary size  128 KB
               //      0 1 0    - dictionary size  256 KB
               //      0 1 1    - dictionary size  512 KB
               //      1 0 0    - dictionary size 1024 KB
               //      1 0 1    - dictionary size 2048 KB
               //      1 1 0    - dictionary size 4096 KB
               //      1 1 1    - file is directory
    Get_Flags (HEAD_FLAGS,  8, high_fields,                     "HIGH_PACK_SIZE and HIGH_UNP_SIZE fields");
    Get_Flags (HEAD_FLAGS,  9, usual_or_utf8,                   "FILE_NAME contains usual and encoded unicode");
    Get_Flags (HEAD_FLAGS, 10, salt,                            "SALT present");
    Skip_Flags(HEAD_FLAGS, 11,                                  "Version flag.");
    Get_Flags (HEAD_FLAGS, 12, exttime,                         "Extended time field present");
    Get_Flags (HEAD_FLAGS, 15, add_size,                        "ADD_SIZE present");
}

//---------------------------------------------------------------------------
// Generic
void File_Rar::Header_Parse_Flags_XX()
{
    Get_Flags (HEAD_FLAGS, 15, add_size,                        "ADD_SIZE present");
}

//---------------------------------------------------------------------------
void File_Rar::Header_Parse_Content()
{
    switch (HEAD_TYPE)
    {
        case 0x73 : Header_Parse_Content_73(); break;
        case 0x74 : Header_Parse_Content_74(); break;
        default   : Header_Parse_Content_XX(); break;
    }
}

//---------------------------------------------------------------------------
// archive header
void File_Rar::Header_Parse_Content_73()
{
    Skip_L2(                                                    "RESERVED_1");
    Skip_L4(                                                    "RESERVED_2");
}

//---------------------------------------------------------------------------
// file header
void File_Rar::Header_Parse_Content_74()
{
    int16u name_size;
    int8u HOST_OS, METHOD, UNP_VER;
    Get_L4 (PACK_SIZE,                                          "PACK_SIZE"); //Compressed file size
    Skip_L4(                                                    "UNP_SIZE"); //Uncompressed file size
    Get_L1 (HOST_OS,                                            "HOST_OS"); Param_Info1((HOST_OS<6?Rar_host_os[HOST_OS]:"Unknown"));
    Skip_L4(                                                    "FILE_CRC");
    Skip_L4(                                                    "FTIME"); //Date and time in standard MS DOS format
    Get_L1 (UNP_VER,                                            "UNP_VER"); Param_Info1(Rar_version_number(UNP_VER)); //RAR version needed to extract file
    Get_L1 (METHOD,                                             "METHOD"); Param_Info1(((METHOD>=0x30)&&(METHOD<0x36)?Rar_packing_method[METHOD-0x30]:"Unknown"));
    Get_L2 (name_size,                                          "NAME_SIZE"); //File name size
    Skip_L4(                                                    "ATTR"); //File attributes
    if(high_fields)
    {
        Get_L4 (HIGH_PACK_SIZE,                                 "HIGH_PACK_SIZE"); //High 4 bytes of 64 bit value of compressed file size.
        Skip_L4(                                                "HIGH_UNP_SIZE"); //High 4 bytes of 64 bit value of uncompressed file size.
    }
    else
        HIGH_PACK_SIZE=0;
    if (usual_or_utf8)
    {
        //Must test the content before reading, looking fore zero byte
        if (Element_Offset+name_size>Element_Size)
        {
            Skip_XX(Element_Size-Element_Offset,                "Error");
            return;
        }
        int64u ZeroPos=0;
        while (ZeroPos<name_size)
        {
            if (Buffer[Buffer_Offset+(size_t)(Element_Offset+ZeroPos)]==0)
                break; //Found
            ZeroPos++;
        }

        if (ZeroPos==name_size)
            Skip_UTF8(name_size,                                "FILE_NAME");
        else
        {
            Skip_Local(ZeroPos,                                 "FILE_NAME"); //Up to ZeroPos
            Skip_L1(                                            "Zero");
            Skip_UTF16L(name_size-(ZeroPos+1),                  "FILE_NAME"); //Spec is not precise, "Unicode" without encoding format (character size, endianess), because RAR is from Windows, we hope this is the format from Windows (UTF-16 Little Endian)
        }
    }
    else
        Skip_Local(name_size,                                   "FILE_NAME");

    if (salt)
        Skip_L8(                                                "SALT");
    //if(exttime)
        //Skip_XX("EXT_TIME"); //Which size?
}

//---------------------------------------------------------------------------
// Generic
void File_Rar::Header_Parse_Content_XX()
{
    if (add_size)
        Get_L4 (PACK_SIZE,                                      "ADD_SIZE"); //Additional data size
}

//---------------------------------------------------------------------------
void File_Rar::Data_Parse()
{
    switch (Element_Code)
    {
        case 0x72 : Accept("RAR"); Fill(Stream_General, 0, General_Format, "RAR"); break;
        default   : ;
    }

    Skip_XX(Element_Size,                                       "Data");
}

} //NameSpace

#endif //MEDIAINFO_RAR_YES
