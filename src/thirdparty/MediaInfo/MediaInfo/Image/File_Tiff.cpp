/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// TIFF Format
//
// From
// http://partners.adobe.com/public/developer/en/tiff/TIFF6.pdf
// http://www.fileformat.info/format/tiff/
// http://en.wikipedia.org/wiki/Tagged_Image_File_Format
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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
#if defined(MEDIAINFO_TIFF_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Image/File_Tiff.h"
#include "ZenLib/Utils.h"
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Info
//***************************************************************************

//---------------------------------------------------------------------------
namespace Tiff_Tag
{
    const int16u ImageWidth                 = 256;
    const int16u ImageLength                = 257;
    const int16u BitsPerSample              = 258;
    const int16u Compression                = 259;
    const int16u PhotometricInterpretation  = 262;
    const int16u ImageDescription           = 270;
    const int16u SamplesPerPixel            = 277;
    const int16u ExtraSamples               = 338;
}

//---------------------------------------------------------------------------
const char* Tiff_Tag_Name(int32u Tag)
{
    switch (Tag)
    {
        case Tiff_Tag::ImageWidth                   : return "ImageWidth";
        case Tiff_Tag::ImageLength                  : return "ImageLength";
        case Tiff_Tag::BitsPerSample                : return "BitsPerSample";
        case Tiff_Tag::Compression                  : return "Compression";
        case Tiff_Tag::PhotometricInterpretation    : return "PhotometricInterpretation";
        case Tiff_Tag::ImageDescription             : return "ImageDescription";
        case Tiff_Tag::SamplesPerPixel              : return "SamplesPerPixel";
        case Tiff_Tag::ExtraSamples                 : return "ExtraSamples";
        default                                     : return "Unknown";
    }
}

//---------------------------------------------------------------------------
namespace Tiff_Type
{
    const int16u Byte       = 1;
    const int16u Short      = 3;
    const int16u Long       = 4;
}

//---------------------------------------------------------------------------
const char* Tiff_Type_Name(int32u Type)
{
    switch (Type)
    {
        case Tiff_Type::Byte                        : return "Byte";
        case Tiff_Type::Short                       : return "Short";
        case Tiff_Type::Long                        : return "Long";
        default                                     : return ""; //Unknown
    }
}

//---------------------------------------------------------------------------
const int8u Tiff_Type_Size(int32u Type)
{
    switch (Type)
    {
        case Tiff_Type::Byte                        : return 1;
        case Tiff_Type::Short                       : return 2;
        case Tiff_Type::Long                        : return 4;
        default                                     : return 0; //Unknown
    }
}

//---------------------------------------------------------------------------
const char* Tiff_Compression(int32u Compression)
{
    switch (Compression)
    {
        case     1 : return "Raw";
        case     2 : return "CCITT Group 3";
        case     3 : return "CCITT T.4";
        case     5 : return "LZW";
        case     6 : return "JPEG";
        case 32773 : return "PackBits";
        default    : return ""; //Unknown
    }
}

//---------------------------------------------------------------------------
const char* Tiff_Compression_Mode(int32u Compression)
{
    switch (Compression)
    {
        case     1 :
        case     2 :
        case     3 :
        case     5 :
        case 32773 : return "Lossless";
        default    : return ""; //Unknown or depends of the compresser (e.g. JPEG can be lossless or lossy)
    }
}

//---------------------------------------------------------------------------
const char* Tiff_PhotometricInterpretation(int32u PhotometricInterpretation)
{
    switch (PhotometricInterpretation)
    {
        case     0 :
        case     1 : return "B/W or Grey scale";
        case     2 : return "RGB";
        case     3 : return "Palette";
        case     4 : return "Transparency mask";
        case     5 : return "CMYK";
        case     6 : return "YCbCr";
        case     8 : return "CIELAB";
        default    : return ""; //Unknown
    }
}

//---------------------------------------------------------------------------
const char* Tiff_PhotometricInterpretation_ColorSpace (int32u PhotometricInterpretation)
{
    switch (PhotometricInterpretation)
    {
        case     0 :
        case     1 : return "Y";
        case     2 : return "RGB";
        case     3 : return "RGB"; //Palette
        case     4 : return "A"; //Transparency mask;
        case     5 : return "CMYK";
        case     6 : return "YUV"; //YCbCr
        case     8 : return "CIELAB"; //What is it?
        default    : return ""; //Unknown
    }
}

const char* Tiff_ExtraSamples(int32u ExtraSamples)
{
    switch (ExtraSamples)
    {
        case     0 :
        case     1 : return "Associated Alpha";
        case     2 : return "Unassocieted Alpha";
        default    : return ""; //Unknown
    }
}

const char* Tiff_ExtraSamples_ColorSpace(int32u ExtraSamples)
{
    switch (ExtraSamples)
    {
        case     1 : return "A";
        default    : return ""; //Unknown
    }
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Tiff::File_Tiff()
{
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Tiff::FileHeader_Begin()
{
    //Element_Size
    /* Minimum header for a tiff file is 8 byte */
    if (Buffer_Size<8)
        return false; //Must wait for more data
    if (CC4(Buffer)==0x49492A00)
        LittleEndian = true;
    else if (CC4(Buffer)==0x4D4D002A)
        LittleEndian = false;
    else
    {
        Reject("TIFF");
        return false;
    }

    //All should be OK...
    Accept("TIFF");
    Fill(Stream_General, 0, General_Format, "TIFF");
    return true;
}

//---------------------------------------------------------------------------
void File_Tiff::FileHeader_Parse()
{
    //The only IFD that is known at forehand is the first one, it's offset is placed byte 4-7 in the file.
    int32u IFDOffset;
    Skip_B4(                                                    "Magic");
    Get_X4 (IFDOffset,                                          "IFDOffset");

    FILLING_BEGIN();
        //Initial IFD
        GoTo(IFDOffset, "TIFF");
    FILLING_END();
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Tiff::Header_Parse()
{
    //Handling remaining IFD data
    if (!IfdItems.empty())
    {
        if (File_Offset+Buffer_Offset!=IfdItems.begin()->first)
            IfdItems.clear(); //There was a problem during the seek, trashing remaining positions from last IFD
        else
        {
            Header_Fill_Code(IfdItems.begin()->second.Tag, Tiff_Tag_Name(IfdItems.begin()->second.Tag));
            Header_Fill_Size(Tiff_Type_Size(IfdItems.begin()->second.Type)*IfdItems.begin()->second.Count);
            return;
        }
    }

    /* A tiff images consist in principle of two types of blocks, IFD's and data blocks                       */
    /* Each datablock, which could be a image, tiles, transperancy filter is described by one IFD.            */
    /* These IFD's can be placed at any offset in the file and are linked in a chain fashion way.             */
    /* where one IFD points out where the next IFD is placed                                                  */
    /*                                                                                                        */
    /* A creator of a tiff file must describe the "main image" in the first IFD, this means that a            */
    /* reader, such this one, only need to read the first IFD in order to get the bitdepth, resolution etc.   */
    /* of the main image.                                                                                     */

    /* Read one IFD and print out the result */

    /* Scan the tiff file for the IFD's (Image File Directory)                */
    /* As long as the IFD offset to the next IFD in the file is not 0         */

    /* Get number of directories for this IFD */
    int16u NrOfDirectories;
    Get_X2 (NrOfDirectories,                                    "NrOfDirectories");

    //Filling
    Header_Fill_Code(0xFFFFFFFF, "IFD"); //OxFFFFFFFF can not be a Tag, so using it as a magic value
    Header_Fill_Size(2+12*((int64u)NrOfDirectories)+4); //2 for header + 12 per directory + 4 for next IFD offset
}

//---------------------------------------------------------------------------
void File_Tiff::Data_Parse()
{
    int32u IFDOffset=0;
    if (IfdItems.empty())
    {
        //Default values
        Infos.clear();
        Infos[Tiff_Tag::BitsPerSample]=__T("1");

        //Parsing new IFD
        while (Element_Offset+8+4<Element_Size)
            Read_Directory();
        Get_X4 (IFDOffset,                                          "IFDOffset");
    }
    else
    {
        //Handling remaining IFD data from a previous IFD
        GetValueOffsetu(IfdItems.begin()->second); //Parsing the IFD item
        IfdItems.erase(IfdItems.begin()->first); //Removing IFD item from the list of IFD items to parse
    }

    //Some items are not inside the directory, jumping to the offset
    if (!IfdItems.empty())
        GoTo(IfdItems.begin()->first, "TIFF");
    else
    {
        //This IFD is finished, filling data then going to next IFD
        Data_Parse_Fill();
        if (IFDOffset)
            GoTo(IFDOffset, "TIFF");
        else
        {
            Finish(); //No more IFDs
            GoToFromEnd(0);
        }
    }
}

//---------------------------------------------------------------------------
void File_Tiff::Data_Parse_Fill()
{
    Stream_Prepare(Stream_Image);

    infos::iterator Info;

    //Width
    Info=Infos.find(Tiff_Tag::ImageWidth);
    if (Info!=Infos.end())
        Fill(Stream_Image, StreamPos_Last, Image_Width, Info->second.Read());

    //Height
    Info=Infos.find(Tiff_Tag::ImageLength);
    if (Info!=Infos.end())
        Fill(Stream_Image, StreamPos_Last, Image_Height, Info->second.Read());

    //BitsPerSample
    Info=Infos.find(Tiff_Tag::BitsPerSample);
    if (Info!=Infos.end())
    {
        if (Info->second.size()>1)
        {
            bool IsOk=true;
            for (size_t Pos=1; Pos<Info->second.size(); ++Pos)
                if (Info->second[Pos]!=Info->second[0])
                    IsOk=false;
            if (IsOk)
                Info->second.resize(1); //They are all same, we display 1 piece of information
        }

        Fill(Stream_Image, StreamPos_Last, Image_BitDepth, Info->second.Read());
    }

    //Compression
    Info=Infos.find(Tiff_Tag::Compression);
    if (Info!=Infos.end())
    {
        int32u Value=Info->second.Read().To_int32u();
        Fill(Stream_Image, StreamPos_Last, Image_Format, Tiff_Compression(Value));
        Fill(Stream_Image, StreamPos_Last, Image_Codec, Tiff_Compression(Value));
        Fill(Stream_Image, StreamPos_Last, Image_Compression_Mode, Tiff_Compression_Mode(Value));
    }

    //PhotometricInterpretation
    Info=Infos.find(Tiff_Tag::PhotometricInterpretation);
    if (Info!=Infos.end())
    {
        int32u Value=Info->second.Read().To_int32u();
        Fill(Stream_Image, StreamPos_Last, Image_ColorSpace, Tiff_PhotometricInterpretation_ColorSpace(Value));
        //Note: should we differeniate between raw RGB and palette (also RGB actually...)
    }

    //ImageDescription
    Info=Infos.find(Tiff_Tag::ImageDescription);
    if (Info!=Infos.end())
        Fill(Stream_Image, StreamPos_Last, Image_Title, Info->second.Read());

    //ExtraSamples
    Info=Infos.find(Tiff_Tag::ExtraSamples);
    if (Info!=Infos.end())
    {
        Ztring ColorSpace=Retrieve(Stream_Image, StreamPos_Last, Image_ColorSpace);
        ColorSpace+=Ztring().From_Local(Tiff_ExtraSamples_ColorSpace(Info->second.Read().To_int32u()));
        Fill(Stream_Image, StreamPos_Last, Image_ColorSpace, ColorSpace, true);
    }
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Tiff::Read_Directory()
{
    /* Each directory consist of 4 fields */
    /* Get information for this directory */
    Element_Begin0();
    ifditem   IfdItem;
    Get_X2 (IfdItem.Tag,                                        "Tag"); Param_Info1(Tiff_Tag_Name(IfdItem.Tag));
    Get_X2 (IfdItem.Type,                                       "Type"); Param_Info1(Tiff_Type_Name(IfdItem.Type));
    Get_X4 (IfdItem.Count,                                      "Count");
    Element_Name(Tiff_Tag_Name(IfdItem.Tag));

    if (Tiff_Type_Size(IfdItem.Type)*IfdItem.Count<=4)
    {
        GetValueOffsetu(IfdItem);

        /* Padding up, skip dummy bytes */
        if (Tiff_Type_Size(IfdItem.Type)==0)
        {
            if (Element_Offset+4<Element_Size)
                Skip_XX(Element_Size-(Element_Offset+4),        "Unknown");
        }
        else if (Tiff_Type_Size(IfdItem.Type)*IfdItem.Count<4)
            Skip_XX(Tiff_Type_Size(IfdItem.Type)*IfdItem.Count, "Padding");
    }
    else
    {
        int32u IFDOffset;
        Get_X4 (IFDOffset,                                      "IFDOffset");
        IfdItems[IFDOffset]=IfdItem;
    }
    Element_End0();
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
void File_Tiff::Get_X2(int16u &Info, const char* Name)
{
    if (LittleEndian)
        Get_L2 (Info,                                           Name);
    else
        Get_B2 (Info,                                           Name);
}

//---------------------------------------------------------------------------
void File_Tiff::Get_X4(int32u &Info, const char* Name)
{
    if (LittleEndian)
        Get_L4 (Info,                                           Name);
    else
        Get_B4 (Info,                                           Name);
}

//---------------------------------------------------------------------------
void File_Tiff::GetValueOffsetu(ifditem &IfdItem)
{
    ZtringList &Info=Infos[IfdItem.Tag]; Info.clear(); Info.Separator_Set(0, __T(" / "));
    const char* Name=Tiff_Tag_Name(IfdItem.Tag);

    if (IfdItem.Count>=10)
    {
        //Too many data, we don't currently need it and we skip it
        Skip_XX(Element_Size-(Element_Offset+4),                Name);
        Info.clear();
        return;
    }

    switch (IfdItem.Type)
    {
        case 1:                /* 8-bit unsigned integer. */
                for (int16u Pos=0; Pos<IfdItem.Count; Pos++)
                {
                    int8u Ret8;
                    #if MEDIAINFO_TRACE
                        if (LittleEndian)
                            Get_L1 (Ret8,                       Name);
                        else
                            Get_B1 (Ret8,                       Name);
                        Element_Info1(Ztring::ToZtring(Ret8));
                    #else //MEDIAINFO_TRACE
                        if (Element_Offset+1>Element_Size)
                        {
                            Trusted_IsNot();
                            break;
                        }
                        if (LittleEndian)
                            Ret8=LittleEndian2int8u(Buffer+Buffer_Offset+(size_t)Element_Offset);
                        else
                            Ret8=BigEndian2int8u(Buffer+Buffer_Offset+(size_t)Element_Offset);
                        Element_Offset++;
                    #endif //MEDIAINFO_TRACE
                    Info.push_back(Ztring::ToZtring(Ret8));
                }
                break;

        case 3:                /* 16-bit (2-byte) unsigned integer. */
                for (int16u Pos=0; Pos<IfdItem.Count; Pos++)
                {
                    int16u Ret16;
                    #if MEDIAINFO_TRACE
                        if (LittleEndian)
                            Get_L2 (Ret16,                      Name);
                        else
                            Get_B2 (Ret16,                      Name);
                        switch (IfdItem.Tag)
                        {
                            case Tiff_Tag::Compression : Element_Info1(Tiff_Compression(Ret16)); break;
                            case Tiff_Tag::PhotometricInterpretation : Element_Info1(Tiff_PhotometricInterpretation(Ret16)); break;
                            default : Element_Info1(Ztring::ToZtring(Ret16));
                        }
                    #else //MEDIAINFO_TRACE
                        if (Element_Offset+2>Element_Size)
                        {
                            Trusted_IsNot();
                            break;
                        }
                        if (LittleEndian)
                            Ret16=LittleEndian2int16u(Buffer+Buffer_Offset+(size_t)Element_Offset);
                        else
                            Ret16=BigEndian2int16u(Buffer+Buffer_Offset+(size_t)Element_Offset);
                        Element_Offset+=2;
                    #endif //MEDIAINFO_TRACE
                    Info.push_back(Ztring::ToZtring(Ret16));
                }
                break;

        case 4:                /* 32-bit (4-byte) unsigned integer */
                for (int16u Pos=0; Pos<IfdItem.Count; Pos++)
                {
                    int32u Ret32;
                    #if MEDIAINFO_TRACE
                        if (LittleEndian)
                            Get_L4 (Ret32,                      Name);
                        else
                            Get_B4 (Ret32,                      Name);
                        Element_Info1(Ztring::ToZtring(Ret32));
                    #else //MEDIAINFO_TRACE
                        if (Element_Offset+4>Element_Size)
                        {
                            Trusted_IsNot();
                            break;
                        }
                        if (LittleEndian)
                            Ret32=LittleEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset);
                        else
                            Ret32=BigEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset);
                        Element_Offset+=4;
                    #endif //MEDIAINFO_TRACE
                    Info.push_back(Ztring::ToZtring(Ret32));
                }
                break;

        default:            //Unknown
                {
                    if (LittleEndian)
                        Skip_L4(                                Name);
                    else
                        Skip_B4(                                Name);
                    Info.clear(); //We actually do not know the type
                }
    }
}

} //NameSpace

#endif

