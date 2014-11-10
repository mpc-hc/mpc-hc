/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Main part
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
#ifdef MEDIAINFO_WM_YES
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Wm.h"
#if defined(MEDIAINFO_MPEGPS_YES)
    #include "MediaInfo/Multiple/File_MpegPs.h"
#endif
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Events.h"
#endif //MEDIAINFO_EVENTS
#include "ZenLib/Utils.h"
using namespace ZenLib;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Format
//***************************************************************************

//---------------------------------------------------------------------------
File_Wm::File_Wm()
:File__Analyze()
{
    //Configuration
    ParserName=__T("Wm");
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Wm;
        StreamIDs_Width[0]=2;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        Demux_Level=2; //Container
    #endif //MEDIAINFO_DEMUX
    DataMustAlwaysBeComplete=false;

    //Stream
    Packet_Count=0;
    MaximumDataPacketSize=(int32u)-1;
    Header_ExtendedContentDescription_AspectRatioX=0;
    Header_ExtendedContentDescription_AspectRatioY=0;
    SizeOfMediaObject_BytesAlreadyParsed=0;
    FileProperties_Preroll=0;
    Codec_Description_Count=0;
    Stream_Number=0;
    Data_Parse_Padding=0;
    NumberPayloads=1;
    NumberPayloads_Pos=0;
    Data_Parse_Begin=true;
    IsDvrMs=false;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Wm::Streams_Finish()
{
    //Encryption management
    /*const Ztring& Encryption=Retrieve(Stream_General, 0, General_Encryption);
    if (!Encryption.empty())
    {
        for (size_t StreamKind=Stream_General+1; StreamKind<Stream_Max; StreamKind++)
            for (size_t Pos=0; Pos<(*Stream[StreamKind]).size(); Pos++)
                Fill ((stream_t)StreamKind, 0, "Encryption", Encryption);
    }

        Fill("BitRate", CurrentBitRate[StreamNumber]);
    */

    std::map<int16u, stream>::iterator Temp=Stream.begin();
    while (Temp!=Stream.end())
    {
        for (std::map<std::string, ZenLib::Ztring>::iterator Info_Temp=Temp->second.Info.begin(); Info_Temp!=Temp->second.Info.end(); ++Info_Temp)
            Fill(Temp->second.StreamKind, Temp->second.StreamPos, Info_Temp->first.c_str(), Info_Temp->second, true);

        //Codec Info
        for (size_t Pos=0; Pos<CodecInfos.size(); Pos++)
        {
            if ((CodecInfos[Pos].Type==1 && Temp->second.StreamKind==Stream_Video)
             || (CodecInfos[Pos].Type==2 && Temp->second.StreamKind==Stream_Audio))
            {
                Fill(Temp->second.StreamKind, Temp->second.StreamPos, "CodecID_Description", CodecInfos[Pos].Info, true);
                Fill(Temp->second.StreamKind, Temp->second.StreamPos, "Codec_Description", CodecInfos[Pos].Info, true);
            }
        }

        if (Temp->second.StreamKind==Stream_Video)
        {
            //Some tests about the frame rate
            int32u PresentationTime_Previous=(int32u)-1;
            size_t TimeDiffs_Sum=0;
            std::map<int32u, size_t> TimeDiffs;
            for (std::set<int32u>::iterator PresentationTime=Temp->second.PresentationTimes.begin(); PresentationTime!=Temp->second.PresentationTimes.end(); ++PresentationTime)
            {
               if (PresentationTime_Previous!=(int32u)-1)
                   TimeDiffs[*PresentationTime-PresentationTime_Previous]++;
               PresentationTime_Previous=*PresentationTime;
            }
            for (std::map<int32u, size_t>::iterator TimeDiff=TimeDiffs.begin(); TimeDiff!=TimeDiffs.end();)
            {
                if (TimeDiff->second<=2)
                    TimeDiffs.erase(TimeDiff++);
                else
                {
                    TimeDiffs_Sum+=TimeDiff->second;
                    ++TimeDiff;
                }
            }

            if (TimeDiffs.empty()
             || (TimeDiffs.size()==1 && TimeDiffs_Sum<16)
             || (TimeDiffs.size()==2 && TimeDiffs_Sum<32)
             || TimeDiffs.begin()->first==1)
            {
                if (Temp->second.AverageTimePerFrame>0)
                    Fill(Stream_Video, Temp->second.StreamPos, Video_FrameRate, ((float)10000000)/(Temp->second.AverageTimePerFrame*(Temp->second.Parser && Temp->second.Parser->Retrieve(Stream_Video, 0, Video_ScanType)==__T("Interlaced")?2:1)), 3, true);
            }
            else if (TimeDiffs.size()==1)
            {
                Fill(Stream_Video, Temp->second.StreamPos, Video_FrameRate, 1000/((float64)TimeDiffs.begin()->first), 3, true);
                if (Temp->second.AverageTimePerFrame>0)
                    Fill(Stream_Video, Temp->second.StreamPos, Video_FrameRate_Nominal, ((float)10000000)/(Temp->second.AverageTimePerFrame*(Temp->second.Parser && Temp->second.Parser->Retrieve(Stream_Video, 0, Video_ScanType)==__T("Interlaced")?2:1)), 3, true);
            }
            else if (TimeDiffs.size()==2)
            {
                std::map<int32u, size_t>::iterator PresentationTime_Delta_Most=TimeDiffs.begin();
                float64 PresentationTime_Deltas_1_Value=(float64)PresentationTime_Delta_Most->first;
                float64 PresentationTime_Deltas_1_Count=(float64)PresentationTime_Delta_Most->second;
                ++PresentationTime_Delta_Most;
                float64 PresentationTime_Deltas_2_Value=(float64)PresentationTime_Delta_Most->first;
                float64 PresentationTime_Deltas_2_Count=(float64)PresentationTime_Delta_Most->second;
                float64 FrameRate_Real=1000/(((PresentationTime_Deltas_1_Value*PresentationTime_Deltas_1_Count)+(PresentationTime_Deltas_2_Value*PresentationTime_Deltas_2_Count))/(PresentationTime_Deltas_1_Count+PresentationTime_Deltas_2_Count));
                Fill(Temp->second.StreamKind, Temp->second.StreamPos, Video_FrameRate, FrameRate_Real, 3, true);
                if (Temp->second.AverageTimePerFrame>0)
                    Fill(Stream_Video, Temp->second.StreamPos, Video_FrameRate_Nominal, ((float)10000000)/(Temp->second.AverageTimePerFrame*(Temp->second.Parser && Temp->second.Parser->Retrieve(Stream_Video, 0, Video_ScanType)==__T("Interlaced")?2:1)), 3, true);
            }
            else
            {
                Fill(Stream_Video, Temp->second.StreamPos, Video_FrameRate_Mode, "VFR");
                if (Temp->second.AverageTimePerFrame>0)
                    Fill(Stream_Video, Temp->second.StreamPos, Video_FrameRate_Nominal, ((float)10000000)/(Temp->second.AverageTimePerFrame*(Temp->second.Parser && Temp->second.Parser->Retrieve(Stream_Video, 0, Video_ScanType)==__T("Interlaced")?2:1)), 3, true);
            }
        }
        if (Temp->second.AverageBitRate>0)
            Fill(Temp->second.StreamKind, Temp->second.StreamPos, "BitRate", Temp->second.AverageBitRate, 10, true);
        if (Temp->second.LanguageID!=(int16u)-1 && Temp->second.LanguageID<(int16u)Languages.size())
            Fill(Temp->second.StreamKind, Temp->second.StreamPos, "Language", Languages[Temp->second.LanguageID]);
        else if (!Language_ForAll.empty())
            Fill(Temp->second.StreamKind, Temp->second.StreamPos, "Language", Language_ForAll);
        if (Temp->second.Parser)
        {
            if (Temp->second.StreamKind==Stream_Max)
                if (Temp->second.Parser->Count_Get(Stream_Audio))
                {
                    Stream_Prepare(Stream_Audio);
                    Temp->second.StreamKind=StreamKind_Last;
                    Temp->second.StreamPos=StreamPos_Last;
                }
            Ztring Format_Profile;
            if (Temp->second.StreamKind==Stream_Video)
                Format_Profile=Retrieve(Stream_Video, Temp->second.StreamPos, Video_Format_Profile);
            Finish(Temp->second.Parser);
            if (Temp->second.Parser->Get(Stream_Video, 0, Video_Format)==__T("MPEG Video"))
                {
                    //Width/Height are junk
                    Clear(Stream_Video, Temp->second.StreamPos, Video_Width);
                    Clear(Stream_Video, Temp->second.StreamPos, Video_Height);
                    Clear(Stream_Video, Temp->second.StreamPos, Video_PixelAspectRatio);
                    Clear(Stream_Video, Temp->second.StreamPos, Video_DisplayAspectRatio);
                }

            //Delay (in case of MPEG-PS)
            if (Temp->second.TimeCode_First!=(int64u)-1)
            {
                Fill(Temp->second.StreamKind, Temp->second.StreamPos, Fill_Parameter(Temp->second.StreamKind, Generic_Delay), Temp->second.TimeCode_First, 10);
                Fill(Temp->second.StreamKind, Temp->second.StreamPos, Fill_Parameter(Temp->second.StreamKind, Generic_Delay_Source), "Container");
            }


            Merge(*Temp->second.Parser, Temp->second.StreamKind, 0, Temp->second.StreamPos);
            if (!Format_Profile.empty() && Format_Profile.find(Retrieve(Stream_Video, Temp->second.StreamPos, Video_Format_Profile))==0)
                Fill(Stream_Video, Temp->second.StreamPos, Video_Format_Profile, Format_Profile, true);
        }

        ++Temp;
    }

    if (Count_Get(Stream_Video)==0 && Count_Get(Stream_Image)==0)
        Fill(Stream_General, 0, General_InternetMediaType, "audio/x-ms-wma", Unlimited, true, true);

    //Purge what is not needed anymore
    if (!File_Name.empty()) //Only if this is not a buffer, with buffer we can have more data
        Stream.clear();
}

//***************************************************************************
// Buffer
//***************************************************************************

//---------------------------------------------------------------------------
void File_Wm::Header_Parse()
{
    if (!MustUseAlternativeParser)
    {
        //Parsing
        int128u Name;
        int64u Size;
        Get_GUID(Name,                                              "Name");
        Get_L8 (Size,                                               "Size");

        //Filling
        Header_Fill_Code(Name.hi, Ztring().From_GUID(Name));
        Header_Fill_Size(Size);
    }
    else
    {
        Header_Fill_Code(0, "Packet");
        Header_Fill_Size(MaximumDataPacketSize);
    }
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_WM_YES
