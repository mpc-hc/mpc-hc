/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// From http://bellard.org/bpg/bpg_spec.txt
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
#if defined(MEDIAINFO_BPG_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Image/File_Bpg.h"
#include <cmath>
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
const char* Bpg_ColorSpace(int8u ColorSpace)
{
    switch (ColorSpace)
    {
        case 0:
        case 3:
        case 4: return "YUV";
        case 1: return "RGB";
        case 2: return "YCgCo";
        default: return "";
    }
};

//---------------------------------------------------------------------------
const char* Bpg_colour_primaries(int8u ColorSpace)
{
    switch (ColorSpace)
    {
        case 0: return "BT.601";
        case 3: return "BT.701";
        case 4: return "BT.2020";
        default: return "";
    }
};

//---------------------------------------------------------------------------
const char* Bpg_Pixel_format(int8u PixelFormat)
{
    switch (PixelFormat)
    {
        case 0 : return "Grayscale";
        case 1 :
        case 4 : return "4:2:0";
        case 2 :
        case 5 : return "4:2:2";
        case 3 : return "4:4:4";
        default: return "";
    }

};

//***************************************************************************
// Static stuff
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Bpg::FileHeader_Begin()
{
    //Element_Size
    if (Buffer_Size<4)
        return false; //Must wait for more data

    if (CC4(Buffer) != 0x425047FB) //"BPG"
    {
        Reject("BPG");
        return false;
    }

    //All should be OK...
    return true;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Bpg::Read_Buffer_Continue()
{
    //Parsing
    Ztring Version;
    int64u Width, Height;
    int8u  pixelformat, BitsDepth, ColorSpace;
    bool   Alpha1_flag, Alpha2_flag, ReservedZeros, limited_range_flag, ExtensionPresentFlag;
    Element_Begin1("File header");
    Skip_C4(                                                    "Magic"); // File Magic
    BS_Begin();
        Get_S1 (3, pixelformat,                                 "pixel_format"); Param_Info1(Bpg_Pixel_format(pixelformat));
        Get_SB (Alpha1_flag,                                    "Alpha1 Present Flag");
        Get_S1 (4, BitsDepth,                                   "bit_depth_minus_8");

        Get_S1(4, ColorSpace,                                   "color_space"); Param_Info1(Bpg_ColorSpace(ColorSpace)); Param_Info1(Bpg_colour_primaries(ColorSpace));
        Get_SB (ExtensionPresentFlag,                           "Extension Present Flag");
        Get_SB (Alpha2_flag,                                    "Alpha2 Present Flag");
        Get_SB (limited_range_flag,                             "limited_range_flag");
        Get_SB (ReservedZeros,                                  "Reserved");
    BS_End();

    Get_VS(Width,                                               "Picture Width");
    Get_VS(Height,                                              "Picture Height");


    Element_End0();

    FILLING_BEGIN();
        Accept("BPG");

        Stream_Prepare(Stream_Image);
        Fill(Stream_Image, 0, Image_Width, Width);
        Fill(Stream_Image, 0, Image_Height, Height);
        Fill(Stream_Image, 0, Image_Format, __T("BPG"));
        Fill(Stream_Image, 0, Image_ChromaSubsampling, Bpg_Pixel_format(pixelformat));
        Fill(Stream_Image, 0, Image_ColorSpace, Bpg_ColorSpace(ColorSpace));
        Fill(Stream_Image, 0, Image_colour_primaries, Bpg_colour_primaries(ColorSpace));
        Fill(Stream_Image, 0, Image_BitDepth, BitsDepth + 8);
        Fill(Stream_Image, 0, Image_Codec, __T("BPG"));
    FILLING_END();

    Finish("BPG");
}

} //NameSpace

#endif
