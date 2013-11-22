/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Note : the buffer must be given in ONE call
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
#if defined(MEDIAINFO_THEORA_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Video/File_Theora.h"
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Format
//***************************************************************************

//---------------------------------------------------------------------------
void File_Theora::Header_Parse()
{
    //Filling
    Header_Fill_Code(0, "Theora");
    Header_Fill_Size(Element_Size);
}

//---------------------------------------------------------------------------
void File_Theora::Data_Parse()
{
    //Parsing
    if (Status[IsAccepted])
        Setup();
    else
        Identification();
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Theora::Identification()
{
    Element_Name("Identification");

    //Parsing
    int32u Version, PICW=0, PICH=0, FRN=0, FRD=0, PARN=0, PARD=0, NOMBR=0;
    Skip_B1   (                                                 "Signature");
    Skip_Local(6,                                               "Signature");
    Get_B3 (Version,                                            "Version");
    if ((Version&0x030200)==0x030200) //Version 3.2.x
    {
        Skip_B2(                                                "FMBW");
        Skip_B2(                                                "FMBH");
        Get_B3 (PICW,                                           "PICW");
        Get_B3 (PICH,                                           "PICH");
        Skip_B1(                                                "PICX");
        Skip_B1(                                                "PICY");
        Get_B4 (FRN,                                            "FRN");
        Get_B4 (FRD,                                            "FRD");
        Get_B3 (PARN,                                           "PARN");
        Get_B3 (PARD,                                           "PARD");
        Skip_B1(                                                "CS"); // //0=4:2:0, 2=4:2:2, 3=4:4:4
        Get_B3 (NOMBR,                                          "NOMBR"); //The nominal bitrate of the stream
        BS_Begin();
        Skip_BS( 6,                                             "QUAL"); //The quality hint.
        Skip_BS( 5,                                             "KFGSHIFT");
        Skip_BS( 2,                                             "PF"); //The Pixel Format
        Skip_BS( 3,                                             "Reserved");
        BS_End();
    }

    //Filling
    FILLING_BEGIN();
        Accept("Theora");

        Stream_Prepare(Stream_Video);
        Fill(Stream_Video, StreamPos_Last, Video_Format, "Theora");
        Fill(Stream_Video, StreamPos_Last, Video_Codec, "Theora");
        if ((Version&0x030200)!=0x030200) //Version 3.2.x
            return;
        if (FRN && FRD)
            Fill(Stream_Video, StreamPos_Last, Video_FrameRate, ((float)FRN)/FRD, 3);
        float PixelRatio=1;
        if (PARN && PARD)
            PixelRatio=((float)PARN)/(float)PARD;
        Fill(Stream_Video, StreamPos_Last, Video_Width, PICW);
        Fill(Stream_Video, StreamPos_Last, Video_Height, PICH);
        Fill(Stream_Video, StreamPos_Last, Video_DisplayAspectRatio, ((float)PICW)/((float)PICH)*PixelRatio, 3, true);
        if (NOMBR)
            Fill(Stream_Video, StreamPos_Last, Video_BitRate_Nominal, NOMBR);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Theora::Setup()
{
    Element_Name("Setup");

    //Parsing
    Skip_XX(Element_Size,                                       "Unknown");

    Finish("Theora");
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_THEORA_YES
