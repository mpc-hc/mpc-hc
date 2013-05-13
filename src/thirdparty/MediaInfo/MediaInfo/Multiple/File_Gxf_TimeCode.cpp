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
#if defined(MEDIAINFO_TIMECODE_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Gxf_TimeCode.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constants
//***************************************************************************

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_GXF_YES)
    extern double Gxf_FrameRate(int32u Content);
#else //defined(MEDIAINFO_GXF_YES)
    double Gxf_FrameRate(int32u Content) //TODO: remove any relationship with GXF
    {
        switch (Content)
        {
            case 1 : return 60.000;
            case 2 : return 59.940;
            case 3 : return 50.000;
            case 4 : return 30.000;
            case 5 : return 29.970;
            case 6 : return 25.000;
            case 7 : return 24.000;
            case 8 : return 23.976;
            default: return  0.000;
        }
    }
#endif //defined(MEDIAINFO_GXF_YES)

//---------------------------------------------------------------------------
const char* Atc_PayloadType (int8u PayloadType)
{
    switch (PayloadType)
    {
        case   0 : return "ATC_LTC";
        case   1 : return "ATC_VITC1";
        case   2 : return "ATC_VITC2";
        default  : return "";
    }
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Gxf_TimeCode::File_Gxf_TimeCode()
:File__Analyze()
{
    //Configuration
    ParserName=__T("Time code");
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Gxf;
        StreamIDs_Width[0]=0;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        Demux_Level=2; //Container
    #endif //MEDIAINFO_DEMUX

    //In
    FrameRate_Code=(int32u)-1;
    FieldsPerFrame_Code=(int32u)-1;
    IsAtc=false;

    //Out
    TimeCode_FirstFrame_ms=(int64u)-1;
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
    Fill(Stream_Video, 0, Video_Delay, TimeCode_FirstFrame_ms);
    if (TimeCode_FirstFrame.size()==11)
        Fill(Stream_Video, StreamPos_Last, Video_Delay_DropFrame, TimeCode_FirstFrame[8]==';'?"Yes":"No");
    Fill(Stream_Video, 0, Video_Delay_Source, "Container");
    Fill(Stream_Video, 0, Video_TimeCode_FirstFrame, TimeCode_FirstFrame.c_str());
    //Fill(Stream_Video, 0, Video_TimeCode_Source, "Time code track");

    Stream_Prepare(Stream_Audio);
    Fill(Stream_Audio, 0, Audio_Delay, TimeCode_FirstFrame_ms);
    if (TimeCode_FirstFrame.size()==11)
        Fill(Stream_Audio, StreamPos_Last, Audio_Delay_DropFrame, TimeCode_FirstFrame[8]==';'?"Yes":"No");
    Fill(Stream_Audio, 0, Audio_Delay_Source, "Container");
    Fill(Stream_Audio, 0, Video_TimeCode_FirstFrame, TimeCode_FirstFrame.c_str());
    //Fill(Stream_Audio, 0, Video_TimeCode_Source, "Time code track");
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Gxf_TimeCode::Read_Buffer_Continue()
{
    int8u Validity[504];

    if (!IsAtc)
    {
        if (Element_Size!=4096)
        {
            Skip_XX(Element_Size,                                   "Data");
            return;
        }

        //Reading bitmap first (validity of first byte is at the end)
        Element_Offset=504*8;
        Element_Begin1("Validity");
        BS_Begin_LE(); //is Little Endian
        for (size_t Pos=0; Pos<504; Pos++)
        {
            bool Validity_Bit;
            Get_TB (Validity_Bit,                                   "Bit");
            Validity[Pos]=Validity_Bit?1:0;
        }
        BS_End_LE();
        Skip_B1(                                                    "Pad");
        Element_End0();
    }

    //Parsing
    Element_Offset=0;
    for (size_t Pos=0; Pos<(IsAtc?(size_t)1:(size_t)504); Pos++)
    {
        if (IsAtc || Validity[Pos])
        {
            Element_Begin1("TimeCode");
            int8u Frames_Units, Frames_Tens, Seconds_Units, Seconds_Tens, Minutes_Units, Minutes_Tens, Hours_Units, Hours_Tens;
            bool  DropFrame;
            int8u DBB1=0, DBB2=0;
            if (IsAtc)
            {
                bool Temp;
                BS_Begin();

                Get_S1 (4, Frames_Units,                            "Frames (Units)");
                Get_SB (   Temp,                                    "DBB1_0"); if (Temp) DBB1|=(1<<0);
                Skip_S1(3,                                          "Zero");

                Skip_S1(4,                                          "BG1");
                Get_SB (   Temp,                                    "DBB1_1"); if (Temp) DBB1|=(1<<1);
                Skip_S1(3,                                          "Zero");

                Skip_SB(                                            "CF - Color fame");
                Get_SB (   DropFrame,                               "DP - Drop frame");
                Get_S1 (2, Frames_Tens,                             "Frames (Tens)");
                Get_SB (   Temp,                                    "DBB1_2"); if (Temp) DBB1|=(1<<2);
                Skip_S1(3,                                          "Zero");

                Skip_S1(4,                                          "BG2");
                Get_SB (   Temp,                                    "DBB1_3"); if (Temp) DBB1|=(1<<3);
                Skip_S1(3,                                          "Zero");

                Get_S1 (4, Seconds_Units,                           "Seconds (Units)");
                Get_SB (   Temp,                                    "DBB1_4"); if (Temp) DBB1|=(1<<4);
                Skip_S1(3,                                          "Zero");

                Skip_S1(4,                                          "BG3");
                Get_SB (   Temp,                                    "DBB1_5"); if (Temp) DBB1|=(1<<5);
                Skip_S1(3,                                          "Zero");

                Skip_SB(                                            "FM - Frame Mark");
                Get_S1 (3, Seconds_Tens,                            "Seconds (Tens)");
                Get_SB (   Temp,                                    "DBB1_6"); if (Temp) DBB1|=(1<<6);
                Skip_S1(3,                                          "Zero");

                Skip_S1(4,                                          "BG4");
                Get_SB (   Temp,                                    "DBB1_7"); if (Temp) DBB1|=(1<<7);
                Skip_S1(3,                                          "Zero");

                Get_S1 (4, Minutes_Units,                           "Minutes (Units)");
                Get_SB (   Temp,                                    "DBB2_0"); if (Temp) DBB2|=(1<<0);
                Skip_S1(3,                                          "Zero");

                Skip_S1(4,                                          "BG5");
                Get_SB (   Temp,                                    "DBB2_1"); if (Temp) DBB2|=(1<<1);
                Skip_S1(3,                                          "Zero");

                Skip_SB(                                            "BGF0");
                Get_S1 (3, Minutes_Tens,                            "Minutes (Tens)");
                Get_SB (   Temp,                                    "DBB2_2"); if (Temp) DBB2|=(1<<2);
                Skip_S1(3,                                          "Zero");

                Skip_S1(4,                                          "BG6");
                Get_SB (   Temp,                                    "DBB2_3"); if (Temp) DBB2|=(1<<3);
                Skip_S1(3,                                          "Zero");

                Get_S1 (4, Hours_Units,                             "Hours (Units)");
                Get_SB (   Temp,                                    "DBB2_4"); if (Temp) DBB2|=(1<<4);
                Skip_S1(3,                                          "Zero");

                Skip_S1(4,                                          "BG7");
                Get_SB (   Temp,                                    "DBB2_5"); if (Temp) DBB2|=(1<<5);
                Skip_S1(3,                                          "Zero");

                Skip_SB(                                            "BGF2");
                Skip_SB(                                            "BGF1");
                Get_S1 (2, Hours_Tens,                              "Hours (Tens)");
                Get_SB (   Temp,                                    "DBB2_6"); if (Temp) DBB2|=(1<<6);
                Skip_S1(3,                                          "Zero");

                Skip_S1(4,                                          "BG8");
                Get_SB (   Temp,                                    "DBB2_7"); if (Temp) DBB2|=(1<<7);
                Skip_S1(3,                                          "Zero");

                BS_End();
            }
            else
            {
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
            }

            int64u TimeCode_Ms=(int64u)(Hours_Tens     *10*60*60*1000
                                      + Hours_Units       *60*60*1000
                                      + Minutes_Tens      *10*60*1000
                                      + Minutes_Units        *60*1000
                                      + Seconds_Tens         *10*1000
                                      + Seconds_Units           *1000
                                      + (Gxf_FrameRate(FrameRate_Code)==0?0:((Frames_Tens*10+Frames_Units)*1000/float64_int32s(Gxf_FrameRate(FrameRate_Code)/(Gxf_FrameRate(FrameRate_Code)>30?2:1)))));

            if (TimeCode_FirstFrame.empty())
            {
                TimeCode_FirstFrame+=('0'+Hours_Tens);
                TimeCode_FirstFrame+=('0'+Hours_Units);
                TimeCode_FirstFrame+=':';
                TimeCode_FirstFrame+=('0'+Minutes_Tens);
                TimeCode_FirstFrame+=('0'+Minutes_Units);
                TimeCode_FirstFrame+=':';
                TimeCode_FirstFrame+=('0'+Seconds_Tens);
                TimeCode_FirstFrame+=('0'+Seconds_Units);
                TimeCode_FirstFrame+=DropFrame?';':':';
                TimeCode_FirstFrame+=('0'+Frames_Tens);
                TimeCode_FirstFrame+=('0'+Frames_Units);
            }

            #if MEDIAINFO_TRACE
                string TimeCode;
                TimeCode+=('0'+Hours_Tens);
                TimeCode+=('0'+Hours_Units);
                TimeCode+=':';
                TimeCode+=('0'+Minutes_Tens);
                TimeCode+=('0'+Minutes_Units);
                TimeCode+=':';
                TimeCode+=('0'+Seconds_Tens);
                TimeCode+=('0'+Seconds_Units);
                TimeCode+=DropFrame?';':':';
                TimeCode+=('0'+Frames_Tens);
                TimeCode+=('0'+Frames_Units);
                Element_Info1(TimeCode.c_str());
            #endif //MEDIAINFO_TRACE
            if (IsAtc)
            {
                Settings=Atc_PayloadType(DBB1);
                Element_Info1(__T("PayloadType=")+Ztring().From_UTF8(Settings.c_str()));
                Element_Info1(__T("VitcLineSelect=")+Ztring::ToZtring(DBB2&0x1F));
            }
            Element_End0();

            FILLING_BEGIN();
                if (TimeCode_FirstFrame_ms==(int64u)-1)
                    TimeCode_FirstFrame_ms=TimeCode_Ms;
            FILLING_END();
        }
        else
            Skip_XX(8,                                              "Junk");
    }

    //bitmap, already parsed
    Element_Offset+=64;

    FILLING_BEGIN();
    if (!Status[IsFilled] && TimeCode_FirstFrame_ms!=(int64u)-1)
    {
        Accept();
        Fill();

        if (MediaInfoLib::Config.ParseSpeed_Get()<1)
            Finish();
    }

    FILLING_END();
}

} //NameSpace

#endif //MEDIAINFO_TIMECODE_YES
