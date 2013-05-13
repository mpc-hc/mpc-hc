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
#if defined(MEDIAINFO_AIC_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Video/File_Aic.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aic::Streams_Fill()
{
    //Filling
    Stream_Prepare(Stream_Video);
    Fill(Stream_Video, 0, Video_Format, "AIC");
    Fill(Stream_Video, 0, Video_Width, Width);
    Fill(Stream_Video, 0, Video_Height, Height);
    Fill(Stream_Video, 0, Video_BitDepth, 8);
    //Fill(Stream_Video, 0, Video_ColorSpace, "YUV");
    //Fill(Stream_Video, 0, Video_ChromaSubsampling, "4:2:0");
    switch (FieldFrame)
    {
        case 0 : Fill(Stream_Video, 0, Video_ScanType, "Progressive"); break;
        case 3 : Fill(Stream_Video, 0, Video_ScanType, "Interlaced"); break;
        default: ;
    }
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aic::Header_Parse()
{
    //Parsing
    int32u Size;
    int16u Sync;
    Get_B2 (Sync,                                               "Sync");
    Get_B4 (Size,                                               "Size");
    if (Sync!=0x0116 || Size<24 || Size!=Buffer_Size)
    {
        Reject("AIC");
        return;
    }
    Get_B2 (Width,                                              "Width");
    Get_B2 (Height,                                             "Height");
    Skip_B2(                                                    "Width again?");
    Skip_B2(                                                    "Height again?");
    Skip_B2(                                                    "Unknown");
    BS_Begin();
    Get_S1 (4, FieldFrame,                                      "field/Frame info?");
    Skip_S1(4,                                                  "Unknown");
    Skip_S1(4,                                                  "Unknown");
    Skip_S1(4,                                                  "Unknown");
    BS_End();
    Skip_B3(                                                    "Unknown");
    Skip_B3(                                                    "Unknown");

    Header_Fill_Code(0, "Frame");
    Header_Fill_Size(Size);
}

//---------------------------------------------------------------------------
void File_Aic::Data_Parse()
{
    //Parsing
    Skip_XX(Element_Size,                                       "Data");

    FILLING_BEGIN();
        Frame_Count++;
        if (Frame_Count_NotParsedIncluded!=(int64u)-1)
            Frame_Count_NotParsedIncluded++;
        if (!Status[IsFilled])
        {
            Accept("AIC");
            Finish("AIC");
        }
    FILLING_END();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_AIC_*
