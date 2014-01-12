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
#if defined(MEDIAINFO_ANCILLARY_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Ancillary.h"
#if defined(MEDIAINFO_CDP_YES)
    #include "MediaInfo/Text/File_Cdp.h"
#endif
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Events.h"
#endif //MEDIAINFO_EVENTS
#if defined(MEDIAINFO_TIMECODE_YES)
    #include "MediaInfo/Multiple/File_Gxf_TimeCode.h"
#endif
#if defined(MEDIAINFO_ARIBSTDB24B37_YES)
    #include "MediaInfo/Text/File_AribStdB24B37.h"
#endif
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#include <cstring>
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
namespace MediaInfoLib
{
//---------------------------------------------------------------------------

//***************************************************************************
// Infos
//***************************************************************************

const char* Ancillary_DataID(int8u DataID, int8u SecondaryDataID)
{
         if (DataID==0x00)
        return "Undefined format";
    else if (DataID<=0x03)
        return "Reserved";
    else if (DataID<=0x0F)
        return "Reserved for 8-bit applications";
    else if (DataID<=0x3F)
        return "Reserved";
    else if (DataID==0x41)
    {
        //SMPTE 2016-3-2007
        switch (SecondaryDataID)
        {
            case 0x05 : return "Bar Data";
            default   : return "Internationally registered";
        }
    }
    else if (DataID==0x45)
    {
        //SMPTE 2020-1-2008
        switch (SecondaryDataID)
        {
            case 0x01 : return "Audio Metadata - No association";
            case 0x02 : return "Audio Metadata - Channels 1/2";
            case 0x03 : return "Audio Metadata - Channels 3/4";
            case 0x04 : return "Audio Metadata - Channels 5/6";
            case 0x05 : return "Audio Metadata - Channels 7/8";
            case 0x06 : return "Audio Metadata - Channels 9/10";
            case 0x07 : return "Audio Metadata - Channels 11/12";
            case 0x08 : return "Audio Metadata - Channels 13/14";
            case 0x09 : return "Audio Metadata - Channels 15/16";
            default   : return "SMPTE 2020-1-2008?";
        }
    }
    else if (DataID<=0x4F)
        return "Internationally registered";
    else if (DataID<=0x5F)
        return "Reserved";
    else if (DataID==0x5F)
    {
        switch (SecondaryDataID&0xF0)
        {
            case 0xD0 : return "ARIB B37";
            default   : return "Reserved";
        }
    }
    else if (DataID==0x60)
        return "Ancillary time code"; //RP188
    else if (DataID==0x61)
    {
        switch (SecondaryDataID)
        {
            case 0x01 : return "EIA-708 (CDP)";
            case 0x02 : return "EIA-608";
            default   : return "S334-1-2007 Defined data services?";
        }
    }
    else if (DataID==0x62)
    {
        switch (SecondaryDataID)
        {
            case 0x01 : return "Program description";
            case 0x02 : return "Data broadcast";
            case 0x03 : return "VBI data";
            default   : return "S334-1-2007 Variable-format data services?";
        }
    }
    else if (DataID<=0x7F)
        return "Internationally registered";
    else if (DataID==0x80)
        return "Ancillary packet marked for deletion";
    else if (DataID<=0x83)
        return "Reserved";
    else if (DataID==0x84)
        return "Optional ancillary packet data end marker";
    else if (DataID<=0x87)
        return "Reserved";
    else if (DataID==0x88)
        return "Optional ancillary packet data start marker";
    else if (DataID<=0x9F)
        return "Reserved";
    else if (DataID<=0xBF)
        return "Internationally registered";
    else if (DataID<=0xCF)
        return "User application";
    else
        return "Internationally registered";
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Ancillary::File_Ancillary()
:File__Analyze()
{
    //Configuration
    ParserName=__T("Ancillary");
    PTS_DTS_Needed=true;

    //In
    WithTenBit=false;
    WithChecksum=false;
    HasBFrames=false;
    InDecodingOrder=false;
    LineNumber_IsSecondField=false;
    AspectRatio=0;
    FrameRate=0;
    LineNumber=(int32u)-1;
    #if defined(MEDIAINFO_CDP_YES)
        Cdp_Parser=NULL;
    #endif //defined(MEDIAINFO_CDP_YES)
    #if defined(MEDIAINFO_ARIBSTDB24B37_YES)
        AribStdB34B37_Parser=NULL;
    #endif //defined(MEDIAINFO_ARIBSTDB24B37_YES)
}

//---------------------------------------------------------------------------
File_Ancillary::~File_Ancillary()
{
    #if defined(MEDIAINFO_CDP_YES)
        delete Cdp_Parser; //Cdp_Parser=NULL;
        for (size_t Pos=0; Pos<Cdp_Data.size(); Pos++)
            delete Cdp_Data[Pos]; //Cdp_Data[Pos]=NULL;
    #endif //defined(MEDIAINFO_CDP_YES)
    #if defined(MEDIAINFO_AFDBARDATA_YES)
        for (size_t Pos=0; Pos<AfdBarData_Data.size(); Pos++)
            delete AfdBarData_Data[Pos]; //AfdBarData_Data[Pos]=NULL;
    #endif //defined(MEDIAINFO_AFDBARDATA_YES)
    #if defined(MEDIAINFO_ARIBSTDB24B37_YES)
        delete AribStdB34B37_Parser; //AribStdB34B37_Parser=NULL;
    #endif //defined(MEDIAINFO_ARIBSTDB24B37_YES)
}

//---------------------------------------------------------------------------
void File_Ancillary::Streams_Finish()
{
    #if defined(MEDIAINFO_CDP_YES)
        if (Cdp_Parser && !Cdp_Parser->Status[IsFinished] && Cdp_Parser->Status[IsAccepted])
        {
            Finish(Cdp_Parser);
            for (size_t StreamPos=0; StreamPos<Cdp_Parser->Count_Get(Stream_Text); StreamPos++)
            {
                Merge(*Cdp_Parser, Stream_Text, StreamPos, StreamPos);
                Ztring MuxingMode=Cdp_Parser->Retrieve(Stream_Text, StreamPos, "MuxingMode");
                Fill(Stream_Text, StreamPos, "MuxingMode", __T("Ancillary data / ")+MuxingMode, true);
                Ztring LawRating=Cdp_Parser->Retrieve(Stream_General, 0, General_LawRating);
                if (!LawRating.empty())
                    Fill(Stream_General, 0, General_LawRating, LawRating, true);
            }

            Ztring LawRating=Cdp_Parser->Retrieve(Stream_General, 0, General_LawRating);
            if (!LawRating.empty())
                Fill(Stream_General, 0, General_LawRating, LawRating, true);
        }
    #endif //defined(MEDIAINFO_CDP_YES)

    #if defined(MEDIAINFO_ARIBSTDB24B37_YES)
        if (AribStdB34B37_Parser && !AribStdB34B37_Parser->Status[IsFinished] && AribStdB34B37_Parser->Status[IsAccepted])
        {
            Finish(AribStdB34B37_Parser);
            for (size_t StreamPos=0; StreamPos<AribStdB34B37_Parser->Count_Get(Stream_Text); StreamPos++)
            {
                Merge(*AribStdB34B37_Parser, Stream_Text, StreamPos, StreamPos);
                Ztring MuxingMode=AribStdB34B37_Parser->Retrieve(Stream_Text, StreamPos, "MuxingMode");
                Fill(Stream_Text, StreamPos, "MuxingMode", __T("Ancillary data / ")+MuxingMode, true);
            }
        }
    #endif //defined(MEDIAINFO_ARIBSTDB24B37_YES)
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Ancillary::Synchronize()
{
    //Synchronizing
    while (Buffer_Offset+6<=Buffer_Size && ( Buffer[Buffer_Offset  ]!=0x00
                                         ||  Buffer[Buffer_Offset+ 1]!=0xFF
                                         ||  Buffer[Buffer_Offset+ 2]!=0xFF))
        Buffer_Offset++;

    if (!Status[IsAccepted])
    {
        Accept();

        Fill(Stream_General, 0, General_Format, "Ancillary");
    }

    //Synched is OK
    return true;
}

//---------------------------------------------------------------------------
bool File_Ancillary::Synched_Test()
{
    Synchronize(); //This is not always in synch directly, there may be some garbage

    //Must have enough buffer for having header
    if (Buffer_Offset+6>Buffer_Size)
        return false;

    //Quick test of synchro
    if (CC3(Buffer+Buffer_Offset)!=0x00FFFF)
        Synched=false;

    //We continue
    return true;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Ancillary::Read_Buffer_Continue()
{
    if (!Cdp_Data.empty() && AspectRatio && FrameRate)
    {
        ((File_Cdp*)Cdp_Parser)->AspectRatio=AspectRatio;
        for (size_t Pos=0; Pos<Cdp_Data.size(); Pos++)
        {
            if (Cdp_Parser->PTS_DTS_Needed)
                Cdp_Parser->FrameInfo.DTS=FrameInfo.DTS-(Cdp_Data.size()-Pos)*FrameInfo.DUR;
            Open_Buffer_Continue(Cdp_Parser, Cdp_Data[Pos]->Data, Cdp_Data[Pos]->Size);
            delete Cdp_Data[Pos]; //Cdp_Data[0]=NULL;
        }
        Cdp_Data.clear();
    }

    if (Element_Size==0)
    {
        #if defined(MEDIAINFO_AFDBARDATA_YES)
            //Keeping only one, TODO: parse it without video stream
            for (size_t Pos=1; Pos<AfdBarData_Data.size(); Pos++)
                delete AfdBarData_Data[Pos]; //AfdBarData_Data[0]=NULL;
            if (!AfdBarData_Data.empty())
                AfdBarData_Data.resize(1);
        #endif //defined(MEDIAINFO_AFDBARDATA_YES)

        return;
    }

    if (!Status[IsAccepted] && !MustSynchronize)
        Accept();
}

//---------------------------------------------------------------------------
void File_Ancillary::Read_Buffer_AfterParsing()
{
    Buffer_Offset=Buffer_Size; //This is per frame

    Frame_Count++;
    Frame_Count_InThisBlock++;
    if (Frame_Count_NotParsedIncluded!=(int64u)-1)
        Frame_Count_NotParsedIncluded++;
}

//---------------------------------------------------------------------------
void File_Ancillary::Read_Buffer_Unsynched()
{
    #if defined(MEDIAINFO_CDP_YES)
        for (size_t Pos=0; Pos<Cdp_Data.size(); Pos++)
            delete Cdp_Data[Pos]; //Cdp_Data[Pos]=NULL;
        Cdp_Data.clear();
        if (Cdp_Parser)
            Cdp_Parser->Open_Buffer_Unsynch();
    #endif //defined(MEDIAINFO_CDP_YES)
    #if defined(MEDIAINFO_AFDBARDATA_YES)
        for (size_t Pos=0; Pos<AfdBarData_Data.size(); Pos++)
            delete AfdBarData_Data[Pos]; //AfdBarData_Data[Pos]=NULL;
        AfdBarData_Data.clear();
    #endif //defined(MEDIAINFO_AFDBARDATA_YES)
    #if defined(MEDIAINFO_ARIBSTDB24B37_YES)
        if (AribStdB34B37_Parser)
            AribStdB34B37_Parser->Open_Buffer_Unsynch();
    #endif //defined(MEDIAINFO_ARIBSTDB24B37_YES)
    AspectRatio=0;
}
//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Ancillary::Header_Parse()
{
    //Parsing
    if (MustSynchronize)
    {
        if (WithTenBit)
        {
            Skip_L2(                                            "Ancillary data flag");
            Skip_L2(                                            "Ancillary data flag");
            Skip_L2(                                            "Ancillary data flag");
        }
        else
        {
            Skip_L1(                                            "Ancillary data flag");
            Skip_L1(                                            "Ancillary data flag");
            Skip_L1(                                            "Ancillary data flag");
        }
    }
    Get_L1 (DataID,                                             "Data ID");
    if (WithTenBit)
        Skip_L1(                                                "Parity+Unused"); //even:1, odd:2
    Get_L1 (SecondaryDataID,                                    "Secondary Data ID"); Param_Info1(Ancillary_DataID(DataID, SecondaryDataID));
    if (WithTenBit)
        Skip_L1(                                                "Parity+Unused"); //even:1, odd:2
    Get_L1 (DataCount,                                          "Data count");
    if (WithTenBit)
        Skip_L1(                                                "Parity+Unused"); //even:1, odd:2

    //Test (in some container formats, Cheksum is present sometimes)
    bool WithChecksum_Temp=WithChecksum;
    if (!MustSynchronize && !WithChecksum && (size_t)((3+DataCount+1)*(WithTenBit?2:1))==Buffer_Size)
        WithChecksum_Temp=true;

    //Filling
    Header_Fill_Code((((int16u)DataID)<<8)|SecondaryDataID, Ztring().From_CC1(DataID)+__T('-')+Ztring().From_CC1(SecondaryDataID));
    Header_Fill_Size(((MustSynchronize?3:0)+3+DataCount+(WithChecksum_Temp?1:0))*(WithTenBit?2:1));
}

//---------------------------------------------------------------------------
void File_Ancillary::Data_Parse()
{
    Element_Info1(Ancillary_DataID(DataID, SecondaryDataID));

    //Buffer
    int8u* Payload=new int8u[DataCount];
    Element_Begin1("Raw data");
    for(int8u Pos=0; Pos<DataCount; Pos++)
    {
        Get_L1 (Payload[Pos],                                   "Data");
        if (WithTenBit)
            Skip_L1(                                            "Parity+Unused"); //even:1, odd:2
    }

    //Parsing
    if (WithChecksum)
        Skip_L1(                                                "Checksum");
    if (WithTenBit)
        Skip_L1(                                                "Parity+Unused"); //even:1, odd:2
    Element_End0();

    FILLING_BEGIN();
        switch (DataID)
        {
            case 0x40 :
                        switch (SecondaryDataID)
                        {
                            case 0x01 : // (from SMPTE ST 305)
                                        // SDTI
                                        break;
                            default   : ;
                            ;
                        }
                        break;
            case 0x41 : // (from SMPTE 2016-3)
                        switch (SecondaryDataID)
                        {
                            case 0x05 : //Bar Data (from SMPTE 2016-3), saving data for future use
                                        #if defined(MEDIAINFO_AFDBARDATA_YES)
                                        {
                                            buffered_data* AfdBarData=new buffered_data;
                                            AfdBarData->Data=new int8u[(size_t)DataCount];
                                            std::memcpy(AfdBarData->Data, Payload, (size_t)DataCount);
                                            AfdBarData->Size=(size_t)DataCount;
                                            AfdBarData_Data.push_back(AfdBarData);
                                        }
                                        #endif //MEDIAINFO_AFDBARDATA_YES
                                        break;
                            default   : ;
                            ;
                        }
                        break;
            case 0x45 : // (from SMPTE 2020-1)
                        switch (SecondaryDataID)
                        {
                            case 0x01 : //No association
                            case 0x02 : //Channel pair 1/2
                            case 0x03 : //Channel pair 3/4
                            case 0x04 : //Channel pair 5/6
                            case 0x05 : //Channel pair 7/8
                            case 0x06 : //Channel pair 9/10
                            case 0x07 : //Channel pair 11/12
                            case 0x08 : //Channel pair 13/14
                            case 0x09 : //Channel pair 15/16
                                        break;
                            default   : ;
                            ;
                        }
                        break;
            case 0x5F : // (from ARIB STD-B37)
                        if ((SecondaryDataID&0xF0)==0xD0) //Digital Closed Caption
                        {
                            #if defined(MEDIAINFO_ARIBSTDB24B37_YES)
                            if (AribStdB34B37_Parser==NULL)
                            {
                                AribStdB34B37_Parser=new File_AribStdB24B37;
                                ((File_AribStdB24B37*)AribStdB34B37_Parser)->IsAncillaryData=true;
                                ((File_AribStdB24B37*)AribStdB34B37_Parser)->ParseCcis=true;
                                Open_Buffer_Init(AribStdB34B37_Parser);
                            }
                            if (!AribStdB34B37_Parser->Status[IsFinished])
                            {
                                if (AribStdB34B37_Parser->PTS_DTS_Needed)
                                    AribStdB34B37_Parser->FrameInfo=FrameInfo;
                                Open_Buffer_Continue(AribStdB34B37_Parser, Payload, (size_t)DataCount);
                            }
                            #endif //defined(MEDIAINFO_ARIBSTDB24B37_YES)
                        }
                        break;
            case 0x60 :
                        switch (SecondaryDataID)
                        {
                            case 0x60 : // (from SMPTE RP 188 / SMPTE ST 12-2)
                                        // Time code ATC
                                        #if defined(MEDIAINFO_TIMECODE_YES)
                                        {
                                        File_Gxf_TimeCode Parser;
                                        Parser.IsAtc=true;
                                        Open_Buffer_Init(&Parser);
                                        Open_Buffer_Continue(&Parser, Payload, (size_t)DataCount);

                                        bool Exists=false;
                                        if (LineNumber!=(int32u)-1)
                                            for (size_t Pos=0; Pos<Count_Get(Stream_Other); Pos++)
                                            {
                                                if (__T("Line")+Ztring::ToZtring(LineNumber)==Retrieve(Stream_Other, Pos, Other_ID)
                                                 && ((!LineNumber_IsSecondField && Retrieve(Stream_Other, Pos, "IsSecondField").empty()) || (LineNumber_IsSecondField && !Retrieve(Stream_Other, Pos, "IsSecondField").empty()))
                                                 && Parser.Settings==Retrieve(Stream_Other, Pos, Other_TimeCode_Settings).To_UTF8())
                                                    Exists=true;
                                            }
                                        else
                                            for (size_t Pos=0; Pos<Count_Get(Stream_Other); Pos++)
                                            {
                                                if (Parser.Settings==Retrieve(Stream_Other, Pos, Other_TimeCode_Settings).To_UTF8())
                                                    Exists=true;
                                            }
                                        if (!Exists)
                                        {
                                            Stream_Prepare(Stream_Other);
                                            Fill(Stream_Other, StreamPos_Last, Other_Type, "Time code");
                                            Fill(Stream_Other, StreamPos_Last, Other_Format, "SMPTE ATC");
                                            Fill(Stream_Other, StreamPos_Last, Other_TimeCode_FirstFrame, Parser.TimeCode_FirstFrame);
                                            Fill(Stream_Other, StreamPos_Last, Other_TimeCode_Settings, Parser.Settings);
                                            Fill(Stream_Other, StreamPos_Last, Other_MuxingMode, "Ancillary data / SMPTE RP 188");
                                            if (LineNumber!=(int32u)-1)
                                                Fill(Stream_Other, StreamPos_Last, Other_ID, __T("Line")+Ztring::ToZtring(LineNumber));
                                            if (LineNumber_IsSecondField)
                                                Fill(Stream_Other, StreamPos_Last, "IsSecondField", "Yes");
                                        }
                                        }
                                        #endif //defined(MEDIAINFO_TIMECODE_YES)
                                        break;
                            default   : ;
                            ;
                        }
                        break;
            case 0x61 : //Defined data services (from SMPTE 334-1)
                        switch (SecondaryDataID)
                        {
                            case 0x01 : //CDP (from SMPTE 334-1)
                                        #if defined(MEDIAINFO_CDP_YES)
                                        {
                                            if (Cdp_Parser==NULL)
                                            {
                                                Cdp_Parser=new File_Cdp;
                                                Open_Buffer_Init(Cdp_Parser);
                                            }
                                            Demux(Payload, (size_t)DataCount, ContentType_MainStream);
                                            if (InDecodingOrder || (!HasBFrames && AspectRatio && FrameRate))
                                            {
                                                if (!Cdp_Parser->Status[IsFinished])
                                                {
                                                    if (Cdp_Parser->PTS_DTS_Needed)
                                                        Cdp_Parser->FrameInfo.DTS=FrameInfo.DTS;
                                                    ((File_Cdp*)Cdp_Parser)->AspectRatio=AspectRatio;
                                                    Open_Buffer_Continue(Cdp_Parser, Payload, (size_t)DataCount);
                                                }
                                            }
                                            else
                                            {
                                                //Saving data for future use
                                                buffered_data* Cdp=new buffered_data;
                                                Cdp->Data=new int8u[(size_t)DataCount];
                                                std::memcpy(Cdp->Data, Payload, (size_t)DataCount);
                                                Cdp->Size=(size_t)DataCount;
                                                Cdp_Data.push_back(Cdp);
                                            }
                                        }
                                        #endif //MEDIAINFO_CDP_YES
                                        break;
                            case 0x02 : //CEA-608 (from SMPTE 334-1)
                                        #if defined(MEDIAINFO_EIA608_YES)
                                        if (DataCount==3) //This must be 3-byte data
                                        {
                                            //CEA-608 in video presentation order
                                        }
                                        #endif //MEDIAINFO_EIA608_YES
                                        break;
                            default   : ;
                            ;
                        }
                        break;
            case 0x62 : //Variable-format data services (from SMPTE 334-1)
                        switch (SecondaryDataID)
                        {
                            case 0x01 : //Program description (from SMPTE 334-1),
                                        break;
                            case 0x02 : //Data broadcast (from SMPTE 334-1)
                                        break;
                            case 0x03 : //VBI data (from SMPTE 334-1)
                                        break;
                            default   : ;
                            ;
                        }
                        break;
            case 0x64 :
                        switch (SecondaryDataID)
                        {
                            case 0x64 : // (from SMPTE RP 196)
                                        // LTC in HANC space
                                        {
                                        bool Exists=false;
                                        if (LineNumber!=(int32u)-1)
                                            for (size_t Pos=0; Pos<Count_Get(Stream_Other); Pos++)
                                            {
                                                if (__T("Line")+Ztring::ToZtring(LineNumber)==Retrieve(Stream_Other, Pos, Other_ID))
                                                    Exists=true;
                                            }
                                        else
                                            if (Count_Get(Stream_Other)!=0)
                                                Exists=true;
                                        if (!Exists)
                                        {
                                            Stream_Prepare(Stream_Other);
                                            Fill(Stream_Other, StreamPos_Last, Other_Type, "Time code");
                                            Fill(Stream_Other, StreamPos_Last, Other_Format, "LTC");
                                            Fill(Stream_Other, StreamPos_Last, Other_MuxingMode, "Ancillary data / SMPTE RP 196");
                                            if (LineNumber!=(int32u)-1)
                                                Fill(Stream_Other, StreamPos_Last, Other_ID, __T("Line")+Ztring::ToZtring(LineNumber));
                                        }
                                        }
                                        break;
                            case 0x7F : // (from SMPTE RP 196)
                                        // VITC in HANC space
                                        {
                                        bool Exists=false;
                                        if (LineNumber!=(int32u)-1)
                                            for (size_t Pos=0; Pos<Count_Get(Stream_Other); Pos++)
                                            {
                                                if (__T("Line")+Ztring::ToZtring(LineNumber)==Retrieve(Stream_Other, Pos, Other_ID))
                                                    Exists=true;
                                            }
                                        else
                                            if (Count_Get(Stream_Other)!=0)
                                                Exists=true;
                                        if (!Exists)
                                        {
                                            Stream_Prepare(Stream_Other);
                                            Fill(Stream_Other, StreamPos_Last, Other_Type, "Time code");
                                            Fill(Stream_Other, StreamPos_Last, Other_Format, "VITC");
                                            Fill(Stream_Other, StreamPos_Last, Other_MuxingMode, "Ancillary data / SMPTE RP 196");
                                            if (LineNumber!=(int32u)-1)
                                                Fill(Stream_Other, StreamPos_Last, Other_ID, __T("Line")+Ztring::ToZtring(LineNumber));
                                        }
                                        }
                                        break;
                            default   : ;
                            ;
                        }
                        break;
            default   : ;
        }
    FILLING_END();

    delete[] Payload; //Payload=NULL
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_ANCILLARY_YES

