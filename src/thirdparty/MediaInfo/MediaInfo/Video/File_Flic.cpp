/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// From : http://www.compuphase.com/flic.htm
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
#if defined(MEDIAINFO_FLIC_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Video/File_Flic.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Format
//***************************************************************************

//---------------------------------------------------------------------------
void File_Flic::FileHeader_Parse()
{
    //Parsing
    int32u DelayBetweenFrames;
    int16u Type, Frames, Width, Height, BitsPerPixel, AspectX=0, AspectY=0;
    Skip_L4(                                                    "Size of FLIC including this header");
    Get_L2 (Type,                                               "File type");
    Get_L2 (Frames,                                             "Number of frames in first segment");
    Get_L2 (Width,                                              "Width");
    Get_L2 (Height,                                             "Height");
    Get_L2 (BitsPerPixel,                                       "Bits per pixel");
    Skip_L2(                                                    "Flags");
    Get_L4 (DelayBetweenFrames,                                 "Delay between frames");
    if (Type!=0xAF11)
    {
        Skip_L2(                                                "Reserved");
        Skip_L4(                                                "Date of Creation)");
        Skip_L4(                                                "Serial number or compiler id");
        Skip_L4(                                                "Date of FLIC update");
        Skip_L4(                                                "Serial number");
        Get_L2 (AspectX,                                        "Width of square rectangle");
        Get_L2 (AspectY,                                        "Height of square rectangle");
    }
    else
        Skip_XX(22,                                             "Reserved");
    Skip_L2(                                                    "EGI: flags for specific EGI extensions");
    Skip_L2(                                                    "EGI: key-image frequency");
    Skip_L2(                                                    "EGI: total number of frames (segments)");
    Skip_L4(                                                    "EGI: maximum chunk size (uncompressed)");
    Skip_L2(                                                    "EGI: max. number of regions in a CHK_REGION chunk");
    Skip_L2(                                                    "EGI: number of transparent levels");
    if (Type!=0xAF11)
    {
        Skip_XX(24,                                             "Reserved");
        Skip_L4(                                                "Offset to frame 1");
        Skip_L4(                                                "Offset to frame 2");
        Skip_XX(40,                                             "Reserved");
    }
    else
        Skip_XX(72,                                             "Reserved");

    //Filling
    FILLING_BEGIN();
        switch (Type)
        {
            case 0xAF11 :
            case 0xAF12 :
            case 0xAF30 :
            case 0xAF31 :
            case 0xAF44 :
                            break;
            default     :
                            Reject("FLIC");
                            return;
        }

        //Filling
        Accept("FLIC");

        Fill(Stream_General, 0, General_Format, "FLIC");

        Stream_Prepare(Stream_Video);
        if (Type==0xAF11)
        {
            Fill(Stream_Video, 0, Video_Format, "FLI");
            Fill(Stream_Video, 0, Video_Codec, "FLI");
            if (DelayBetweenFrames)
            {
                Fill(Stream_Video, StreamPos_Last, Video_FrameRate, 1000.0/(DelayBetweenFrames*70)); //multiple of 1/70 per frame
                Fill(Stream_Video, 0, Video_Duration, Frames*DelayBetweenFrames*70);
            }
        }
        else
        {
            Fill(Stream_Video, 0, Video_Format, "FLC");
            Fill(Stream_Video, 0, Video_Codec, "FLC");
            if (DelayBetweenFrames)
            {
                Fill(Stream_Video, StreamPos_Last, Video_FrameRate, 1000.0/DelayBetweenFrames); //ms per frame
                Fill(Stream_Video, 0, Video_Duration, Frames*DelayBetweenFrames);
            }
            if (AspectY>0)
                Fill(Stream_Video, StreamPos_Last, Video_DisplayAspectRatio, AspectX/AspectY, 3, true);
        }
        Fill(Stream_Video, 0, Video_FrameCount, Frames);
        Fill(Stream_Video, StreamPos_Last, Video_Width, Width);
        Fill(Stream_Video, StreamPos_Last, Video_Height, Height);
        Fill(Stream_Video, 0, Video_BitDepth, (BitsPerPixel%3)?BitsPerPixel:(BitsPerPixel/3), 10, true); //If not a multiple of 3, the total resolution is filled
        //No more need data
        Finish("FLIC");
    FILLING_END();
}

} //NameSpace

#endif //MEDIAINFO_FLIC_*

