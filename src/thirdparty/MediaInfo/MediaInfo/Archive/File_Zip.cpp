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
#if defined(MEDIAINFO_ZIP_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Archive/File_Zip.h"
#include "ZenLib/Utils.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************


//---------------------------------------------------------------------------
const char* Zip_made_by[20]=
{
    "MS-DOS and OS/2 (FAT / VFAT / FAT32 file systems)",
    "Amiga",
    "OpenVMS",
    "UNIX",
    "VM/CMS",
    "Atari ST",
    "OS/2 H.P.F.S.",
    "Macintosh",
    "Z-System",
    "CP/M",
    "Windows NTFS",
    "MVS (OS/390 - Z/OS)",
    "VSE",
    "Acorn Risc",
    "VFAT",
    "alternate MVS",
    "BeOS",
    "Tandem",
    "OS/400",
    "OS/X (Darwin)"
};

//---------------------------------------------------------------------------
const char* Zip_compression_method[22]=
{
    "stored (no compression)",
    "Shrunk",
    "Reduced with compression factor 1",
    "Reduced with compression factor 2",
    "Reduced with compression factor 3",
    "Reduced with compression factor 4",
    "Imploded",
    "Tokenizing compression algorithm",
    "Deflated",
    "Enhanced Deflating using Deflate64(tm)",
    "PKWARE Data Compression Library Imploding (old IBM TERSE)",
    "Reserved by PKWARE",
    "compressed using BZIP2 algorithm",
    "Reserved by PKWARE",
    "LZMA (EFS)",
    "Reserved by PKWARE",
    "Reserved by PKWARE",
    "Reserved by PKWARE",
    "File is compressed using IBM TERSE (new)",
    "IBM LZ77 z Architecture (PFS)",
    "WavPack compressed data", // 97
    "PPMd version I, Rev 1" // 98
};

//***************************************************************************
// Static stuff
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Zip::FileHeader_Begin()
{
    //Element_Size
    if (Buffer_Size<4)
        return false; //Must wait for more data

    if (Buffer[0]!=0x50 //"PK.."
     || Buffer[1]!=0x4B
     || Buffer[2]!=0x03
     || Buffer[3]!=0x04)
    {
        Reject("ZIP");
        return false;
    }

    //This is OK, ZIP detected
    Accept();
    Fill(Stream_General, 0, General_Format, "ZIP");

    //Init
    signature=0x00000000;
    local_file_Step=0;
    end_of_central_directory_IsParsed=false;

    //Jumping to the end of the file minus  end_of_central_directory size (we hope there is no comment)
    GoTo(File_Size-22);

    return true;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Zip::Read_Buffer_Continue()
{
    for (;;)
    {
        //Probing
        if (signature==0x00000000) //If not already tested (else an element is being parsed)
        {
            if (Element_Offset+4>Element_Size) //signature size
                return; //Not enough data
            signature=LittleEndian2int32u(Buffer+(size_t)Element_Offset);
        }

        //Parsing
        switch (signature)
        {
            case 0x04034b50 :   if (!local_file())
                                    return; //Not enough data
                                break;
            case 0x02014b50 :   if (!central_directory())
                                    return; //Not enough data
                                break;
            case 0x05054b50 :   if (!digital_signature())
                                    return; //Not enough data
                                break;
            case 0x06054b50 :   if (!end_of_central_directory())
                                    return; //Not enough data
                                break;
            case 0x08064b50 :   if(!archive_extra_data_record())
                                    return;
                                break;
            case 0x06064b50 :   if(!Zip64_end_of_central_directory_record())
                                    return;
                                break;
            case 0x07064b50 :   if(!Zip64_end_of_central_directory_locator())
                                    return;
                                break;
            default:    Finish(); return; //Reject(); return; //Unknown value  //Décommenter quand toutes les signatures sont implémentées
        }

        //Cleanup
        signature=0x00000000; //Reset, must probe again the signature (next element)
    }
}

//***************************************************************************
// Files
//***************************************************************************

bool File_Zip::local_file()
{
    switch(local_file_Step)
    {
        case 0 :
                    if (!local_file_header())
                        return false;
                    local_file_Step=1; //local_file_header parsed
                    break;
        case 1 :
                    local_file_Step=2; //file_data is always parsed
                    if (!file_data())
                        return false;
                    break;
        case 2:
                    if (!data_descriptor())
                        return false;
                    local_file_Step=0; //data_descriptor is parsed, back to begin
                    break;
        default:    ; //Should never happen
    }

    return true;
}

bool File_Zip::archive_extra_data_record()
{
    if (Element_Offset+8>Element_Size) //archive_extra_data_record
        return false; //Not enough data

    //Retrieving complete archive_extra_data_record size
    int32u extra_field_length=LittleEndian2int32u(Buffer+(size_t)Element_Offset+4);

    //Parsing
    Element_Begin1("archive_extra_data_record");
        Skip_C4("Archive extra data signature");
        Skip_L4("extra field length");
        Skip_XX(extra_field_length,"extra_field_data");
    Element_End0();

    return true;
}

bool File_Zip::digital_signature()
{
    if (Element_Offset+6>Element_Size) //digital_signature
        return false; //Not enough data

    //Retrieving complete archive_extra_data_record size
    int16u size_of_data=LittleEndian2int16u(Buffer+(size_t)Element_Offset+4);

    //Parsing
    Element_Begin1("digital_signature");
        Skip_C4("Header signature");
        Skip_L2("size of data");
        Skip_XX(size_of_data,"signature data");
    Element_End0();

    return true;
}

bool File_Zip::local_file_header()
{
    if (Element_Offset+30>Element_Size) //local_file_header up to extra_field_length included
        return false; //Not enough data

    //Retrieving complete local_file_header size
    int16u file_name_length=LittleEndian2int16u(Buffer+(size_t)Element_Offset+26);
    int16u extra_field_length=LittleEndian2int16u(Buffer+(size_t)Element_Offset+28);
    if (Element_Offset+30+file_name_length+extra_field_length>Element_Size) //local_file_header all included
        return false; //Not enough data

    //Parsing
    Element_Begin1("local_file_header");
    int16u general_purpose_bit_flag,compression_method;
    bool efs;
    Skip_C4("Local file header signature");
    Skip_L2("Version needed to extract");
    Get_L2 (general_purpose_bit_flag,"general purpose bit flag");
    Skip_Flags(general_purpose_bit_flag, 0,                     "encrypted file");
    Skip_Flags(general_purpose_bit_flag, 1,                     "8K sliding dictionary");
    Skip_Flags(general_purpose_bit_flag, 2,                     "3 Shannon-Fano trees");
    Get_Flags (general_purpose_bit_flag, 3, data_descriptor_set,    "data descriptor");
    Skip_Flags(general_purpose_bit_flag, 4,                     "Reserved for use with method 8");
    Skip_Flags(general_purpose_bit_flag, 4,                     "file is compressed patched data");
    Skip_Flags(general_purpose_bit_flag, 4,                     "Strong encryption");
    //~ Skip_Flags(general_purpose_bit_flag, 4,                     "Currently unused");
    //~ Skip_Flags(general_purpose_bit_flag, 4,                     "Currently unused");
    //~ Skip_Flags(general_purpose_bit_flag, 4,                     "Currently unused");
    //~ Skip_Flags(general_purpose_bit_flag, 4,                     "Currently unused");
    Get_Flags (general_purpose_bit_flag, 11, efs,                "Language encoding flag (EFS)");
    //~ Skip_Flags(general_purpose_bit_flag, 4,                     "Reserved by PKWARE for enhanced compression");
    //~ Skip_Flags(general_purpose_bit_flag, 4,                     "");
    //~ Skip_Flags(general_purpose_bit_flag, 4,                     "Reserved by PKWARE");
    //~ Skip_Flags(general_purpose_bit_flag, 4,                     "Reserved by PKWARE");
    Get_L2 (compression_method,"compression method");
    Param_Info1C((compression_method<20), Zip_compression_method[compression_method]);
    Param_Info1C((compression_method==97||compression_method==98), Zip_compression_method[compression_method-97+20]);
    Skip_L2("last mod file time");
    Skip_L2("last mod file date");
    Skip_L4("crc-32");
    Get_L4(compressed_size,"compressed size");
    Skip_L4("uncompressed size");
    Get_L2(file_name_length,"file name lenth");
    Get_L2(extra_field_length,"extra field length");
    if(efs) {
        Skip_UTF8(file_name_length,"file name");
        Skip_UTF8(extra_field_length,"extra field");
    } else {
        Skip_Local(file_name_length,"file name");
        Skip_Local(extra_field_length,"extra field");
    }
    Element_End0();

    FILLING_BEGIN();
        Accept("Zip");
        Fill(Stream_General, 0, General_Format, "ZIP");
    FILLING_END();
    return true;
}

bool File_Zip::file_data()
{
    Element_Begin1("file_data");
        Skip_XX(compressed_size,"File_data");
    Element_End0();

    if (Element_Offset>Element_Size)
    {
        GoTo(File_Offset+Element_Offset);
        return false;
    }
    return true;

}

bool File_Zip::data_descriptor()
{
    if(data_descriptor_set)
    {
        if (Element_Offset+12>Element_Size)
            return false; //Not enough data

        Element_Begin1("data_descriptor");
            Skip_L4("crc-32");
            Skip_L4("compressed size");
            Skip_L4("uncompressed size");
        Element_End0();
    }
    return true;
}

bool File_Zip::central_directory()
{
    if (Element_Offset+46>Element_Size) //central_directory up to relative offset of local header included
        return false; //Not enough data

    //Retrieving complete local_file_header size
    int16u file_name_length=LittleEndian2int16u(Buffer+(size_t)Element_Offset+28);
    int16u extra_field_length=LittleEndian2int16u(Buffer+(size_t)Element_Offset+30);
    int16u file_comment_length=LittleEndian2int16u(Buffer+(size_t)Element_Offset+32);
    if (Element_Offset+46+file_name_length+extra_field_length+file_comment_length>Element_Size) //central_directory_structure all included
        return false; //Not enough data

    int16u general_purpose_bit_flag;
    bool efs;
    int16u version_made_by,compression_method;

    //Parsing
    Element_Begin1("Central directory");
    Skip_C4("central file header signature");
    Get_L2 (version_made_by,"version made by");Param_Info1((version_made_by>>8)>20?"unused":Zip_made_by[version_made_by>>8]);
    Skip_L2("version needed to extract");
    Get_L2 (general_purpose_bit_flag,"general purpose bit flag");
    Skip_Flags(general_purpose_bit_flag, 0,                     "encrypted file");
    Skip_Flags(general_purpose_bit_flag, 1,                     "8K sliding dictionary");
    Skip_Flags(general_purpose_bit_flag, 2,                     "3 Shannon-Fano trees");
    Skip_Flags(general_purpose_bit_flag, 3,                     "data descriptor");
    Skip_Flags(general_purpose_bit_flag, 4,                     "Reserved for use with method 8");
    Skip_Flags(general_purpose_bit_flag, 4,                     "file is compressed patched data");
    Skip_Flags(general_purpose_bit_flag, 4,                     "Strong encryption");
    //~ Skip_Flags(general_purpose_bit_flag, 4,                     "Currently unused");
    //~ Skip_Flags(general_purpose_bit_flag, 4,                     "Currently unused");
    //~ Skip_Flags(general_purpose_bit_flag, 4,                     "Currently unused");
    //~ Skip_Flags(general_purpose_bit_flag, 4,                     "Currently unused");
    Get_Flags (general_purpose_bit_flag, 11, efs,                "Language encoding flag (EFS)");
    //~ Skip_Flags(general_purpose_bit_flag, 4,                     "Reserved by PKWARE for enhanced compression");
    //~ Skip_Flags(general_purpose_bit_flag, 4,                     "");
    //~ Skip_Flags(general_purpose_bit_flag, 4,                     "Reserved by PKWARE");
    //~ Skip_Flags(general_purpose_bit_flag, 4,                     "Reserved by PKWARE");
    Get_L2 (compression_method,"compression method");
    Param_Info1C((compression_method<20), Zip_compression_method[compression_method]);
    Param_Info1C((compression_method==97||compression_method==98), Zip_compression_method[compression_method-97+20]);
    Skip_L2("last mod file time");
    Skip_L2("last mod file date");
    Skip_L4("crc-32");
    Skip_L4("compressed size");
    Skip_L4("uncompressed size");
    Skip_L2("file name length");
    Skip_L2("extra field length");
    Skip_L2("file comment length");
    Skip_L2("disk number start");
    Skip_L2("internal file attributes");
    Skip_L4("external file attributes");
    Skip_L4("relative offset of local header");
    if(efs) {
        Skip_UTF8(file_name_length,"file name");
        Skip_UTF8(extra_field_length,"extra field");
        Skip_UTF8(file_comment_length,"file comment");
    } else {
        Skip_Local(file_name_length,"file name");
        Skip_Local(extra_field_length,"extra field");
        Skip_Local(file_comment_length,"file comment");
    }
    Element_End0();

    return true;
}

bool File_Zip::end_of_central_directory()
{
    if (Element_Offset+22>Element_Size) //end_of_central_directory up to relative offset of .ZIP file comment length included
        return false; //Not enough data

    //Retrieving complete local_file_header size
    int16u zip_comment_length=LittleEndian2int16u(Buffer+(size_t)Element_Offset+20);
    if (Element_Offset+22+zip_comment_length>Element_Size) //end_of_central_directory all included
        return false; //Not enough data

    //Parsing
    int32u offset;
    Element_Begin1("End of central directory");
    Skip_C4(                                                    "end of central dir signature");
    Skip_L2(                                                    "number of this disk");
    Skip_L2(                                                    "number of the disk");// with the start of the central directory
    Skip_L2(                                                    "total number of entries on this disk");// in the central directory
    Skip_L2(                                                    "total number of entries");// in the central directory
    Skip_L4(                                                    "size of the central directory");
    Get_L4 (offset,                                             "offset of start of central directory");// with respect to the starting disk number
    Skip_L2(                                                    "zip file comment length");
    Skip_XX(zip_comment_length,                                 "zip file comment");
    Element_End0();

    //Going to first central directory (once)
    if (!end_of_central_directory_IsParsed)
    {
        end_of_central_directory_IsParsed=true;
        GoTo(offset);
    }
    return true;
}

bool File_Zip::Zip64_end_of_central_directory_record()
{
    if (Element_Offset+12>Element_Size) //Zip64_end_of_central_directory_record
        return false; //Not enough data

    //Retrieving complete Zip64_end_of_central_directory_record size
    int64u size_of_Zip64_end_of_central_directory_record=LittleEndian2int64u(Buffer+(size_t)Element_Offset+4);
    if (Element_Offset+12+size_of_Zip64_end_of_central_directory_record>Element_Size) //end_of_central_directory all included
        return false; //Not enough data

    //Parsing
    //~ int32u offset;
    int16u version_made_by;
    Element_Begin1("Zip64 End of central directory record");
    Skip_C4(                                                    "Zip64 end of central dir signature");
    Skip_L8(                                                    "size of zip64 end of central directory record");
    Get_L2 (version_made_by,                                    "version made by");
    Param_Info1((version_made_by>>8)>20?"unused":Zip_made_by[version_made_by>>8]);
    Skip_L2(                                                    "version needed to extract");
    Skip_L4(                                                    "number of this disk");
    Skip_L4(                                                    "number of the disk");// with the start of the central directory
    Skip_L8(                                                    "total number of entries on this disk");// in the central directory
    Skip_L8(                                                    "total number of entries");// in the central directory
    Skip_L8(                                                    "size of the central directory");
    Skip_L8(                                                    "offset of start of central directory"); //  with respect to the starting disk number
    Skip_XX(size_of_Zip64_end_of_central_directory_record-44,   "zip64 extensible data sector");
    Element_End0();

    return true;
}

bool File_Zip::Zip64_end_of_central_directory_locator()
{
    if (Element_Offset+20>Element_Size) //Zip64_end_of_central_directory_locator
        return false; //Not enough data

    //Parsing
    Element_Begin1("Zip64 end of central directory locator");
    Skip_C4("zip64 end of central dir locator signature");
    Skip_L4("number of the disk");// with the start of the zip64 end of central directory
    Skip_L8("relative offset of the zip64 end of central directory record");
    Skip_L4("total number of disks");
    Element_End0();

    return true;
}

} //NameSpace

#endif //MEDIAINFO_ZIP_YES
