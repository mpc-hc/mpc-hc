// File_Gxf_TimeCode - Info for GXF (SMPTE 360M) files
// Copyright (C) 2010-2010 MediaArea.net SARL, Info@MediaArea.net
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
// Compilation conditions
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_GXF_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Gxf_TimeCode.h"
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Events.h"
#endif //MEDIAINFO_EVENTS
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constants
//***************************************************************************

//---------------------------------------------------------------------------
extern double Gxf_FrameRate(int32u Content);

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Gxf_TimeCode::File_Gxf_TimeCode()
:File__Analyze()
{
    //Configuration
    ParserName=_T("GXF");
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Gxf;
        StreamIDs_Width[0]=2;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        Demux_Level=2; //Container
    #endif //MEDIAINFO_DEMUX

    //In
    FrameRate_Code=(int32u)-1;
    FieldsPerFrame_Code=(int32u)-1;

    //Out
    TimeCode_First=(int64u)-1;
}

//---------------------------------------------------------------------------
File_Gxf_TimeCode::~File_Gxf_TimeCode()
{
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Gxf_TimeCode::Streams_Fill()
{
    Stream_Prepare(Stream_Video);
    Fill(Stream_Video, 0, Video_Delay, TimeCode_First);
    Fill(Stream_Video, 0, Video_Delay_Source, "Container");

    Stream_Prepare(Stream_Audio);
    Fill(Stream_Audio, 0, Audio_Delay, TimeCode_First);
    Fill(Stream_Audio, 0, Audio_Delay_Source, "Container");
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Gxf_TimeCode::Read_Buffer_Continue()
{
    if (Element_Size!=4096)
    {
        Skip_XX(Element_Size,                                   "Data");
        return;
    }

    //Reading bitmap first (validity of first byte is at the end)
    Element_Offset=504*8;
    Element_Begin("Validity");
    int8u Validity[504];
    BS_Begin_LE(); //is Little Endian
    for (size_t Pos=0; Pos<504; Pos++)
    {
        bool Validity_Bit;
        Get_SB (Validity_Bit,                                   "Bit");
        Validity[Pos]=Validity_Bit?1:0;
    }
    BS_End_LE();
    Skip_B1(                                                    "Pad");
    Element_End();

    //Parsing
    Element_Offset=0;
    for (size_t Pos=0; Pos<504; Pos++)
    {
        if (Validity[Pos])
        {
            Element_Begin("TimeCode");
            int8u Frames_Units, Frames_Tens, Seconds_Units, Seconds_Tens, Minutes_Units, Minutes_Tens, Hours_Units, Hours_Tens;
            bool  DropFrame;
            BS_Begin();

            Skip_S1(4,                                          "BG1");
            Get_S1 (4, Frames_Units,                            "Frames (Units)");

            Skip_S1(4,                                          "BG2");
            Skip_SB(                                            "CF - Color fame");
            Get_SB (   DropFrame,                               "DP - Drop frame");
            Get_S1 (2, Frames_Tens,                             "Frames (Tens)");

            Skip_S1(4,                                          "BG3");
            Get_S1 (4, Seconds_Units,                           "Seconds (Units)");

            Skip_S1(4,                                          "BG4");
            Skip_SB(                                            "FM - Frame Mark");
            Get_S1 (3, Seconds_Tens,                            "Seconds (Tens)");

            Skip_S1(4,                                          "BG5");
            Get_S1 (4, Minutes_Units,                           "Minutes (Units)");

            Skip_S1(4,                                          "BG6");
            Skip_SB(                                            "BGF0");
            Get_S1 (3, Minutes_Tens,                            "Minutes (Tens)");

            Skip_S1(4,                                          "BG7");
            Get_S1 (4, Hours_Units,                             "Hours (Units)");

            Skip_S1(4,                                          "BG8");
            Skip_SB(                                            "BGF2");
            Skip_SB(                                            "BGF1");
            Get_S1 (2, Hours_Tens,                              "Hours (Tens)");

            BS_End();

            int64u TimeCode=(int64u)(Hours_Tens     *10*60*60*1000
                               + Hours_Units       *60*60*1000
                               + Minutes_Tens      *10*60*1000
                               + Minutes_Units        *60*1000
                               + Seconds_Tens         *10*1000
                               + Seconds_Units           *1000
                               + (Gxf_FrameRate(FrameRate_Code)==0?0:((Frames_Tens*10+Frames_Units)*1000/float64_int32s(Gxf_FrameRate(FrameRate_Code)/(Gxf_FrameRate(FrameRate_Code)>30?2:1)))));

            Element_Info(Ztring().Duration_From_Milliseconds(TimeCode));

            Element_End();

            FILLING_BEGIN();
                if (TimeCode_First==(int64u)-1)
                    TimeCode_First=TimeCode;
            FILLING_END();
        }
        else
            Skip_XX(8,                                              "Junk");
    }

    //bitmap, already parsed
    Element_Offset+=64;

    FILLING_BEGIN();
    if (!Status[IsFilled] && TimeCode_First!=(int64u)-1)
    {
        Accept();
        Fill();

        if (MediaInfoLib::Config.ParseSpeed_Get()<1)
            Finish();
    }

    FILLING_END();
}

} //NameSpace

#endif //MEDIAINFO_GXF_YES
