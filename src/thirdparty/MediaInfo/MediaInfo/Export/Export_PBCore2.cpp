/*  Copyright (c) 2009-2013 MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Contributor: Dave Rice, dave@dericed.com
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
#if defined(MEDIAINFO_PBCORE_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Export/Export_PBCore2.h"
#include "MediaInfo/File__Analyse_Automatic.h"
#include <ctime>
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
extern MediaInfo_Config Config;
//---------------------------------------------------------------------------

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
Ztring PBCore2_MediaType(MediaInfo_Internal &MI)
{
    if (MI.Count_Get(Stream_Video))
        return __T("Moving Image");
    else if (MI.Count_Get(Stream_Audio))
        return __T("Sound");
    else if (MI.Count_Get(Stream_Image))
        return __T("Static Image");
    else if (MI.Count_Get(Stream_Text))
        return __T("Text");
    else
        return Ztring();
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
Export_PBCore2::Export_PBCore2 ()
{
}

//---------------------------------------------------------------------------
Export_PBCore2::~Export_PBCore2 ()
{
}

//***************************************************************************
// Input
//***************************************************************************

//---------------------------------------------------------------------------
void PBCore2_Transform(Ztring &ToReturn, MediaInfo_Internal &MI, stream_t StreamKind, size_t StreamPos)
{
    //Menu: only if TimeCode
    if (StreamKind==Stream_Menu && MI.Get(Stream_Menu, StreamPos, Menu_Format)!=__T("TimeCode"))
        return;

    //essenceTrackType
    Ztring essenceTrackType;
    switch (StreamKind)
    {
        case Stream_Video:
            essenceTrackType=__T("Video");
            break;
        case Stream_Audio:
            essenceTrackType=__T("Audio");
            break;
        case Stream_Image:
            essenceTrackType=__T("Image");
            break;
        case Stream_Text:
            {
            Ztring Format=MI.Get(Stream_Text, StreamPos, Text_Format);
            if (Format==__T("EIA-608") || Format==__T("EIA-708"))
                essenceTrackType=__T("CC");
            else
                essenceTrackType=__T("Text");
            }
            break;
        case Stream_Menu:
            if (MI.Get(Stream_Menu, StreamPos, Menu_Format)==__T("TimeCode"))
            {
                essenceTrackType=__T("TimeCode");
                break;
            }
            else
                return; //Not supported
        default:            return; //Not supported
    }

    ToReturn+=__T("\t<instantiationEssenceTrack>\n");

    ToReturn+=__T("\t\t<essenceTrackType>");
    ToReturn+=essenceTrackType;
    ToReturn+=__T("</essenceTrackType>\n");

    //essenceTrackIdentifier
    if (!MI.Get(StreamKind, StreamPos, __T("ID")).empty())
    {
        ToReturn+=__T("\t\t<essenceTrackIdentifier source=\"ID (Mediainfo)\">");
        ToReturn+=MI.Get(StreamKind, StreamPos, __T("ID"));
        ToReturn+=__T("</essenceTrackIdentifier>\n");
    }
    if (!MI.Get(Stream_General, 0, General_UniqueID).empty())
    {
        ToReturn+=__T("\t\t<essenceTrackIdentifier source=\"UniqueID (Mediainfo)\">");
        ToReturn+=MI.Get(StreamKind, StreamPos, __T("UniqueID"));
        ToReturn+=__T("</essenceTrackIdentifier>\n");
    }
    if (!MI.Get(StreamKind, StreamPos, __T("StreamKindID")).empty())
    {
        ToReturn+=__T("\t\t<essenceTrackIdentifier source=\"StreamKindID (Mediainfo)\">");
        ToReturn+=MI.Get(StreamKind, StreamPos, __T("StreamKindID"));
        ToReturn+=__T("</essenceTrackIdentifier>\n");
    }
    if (!MI.Get(StreamKind, StreamPos, __T("StreamOrder")).empty())
    {
        ToReturn+=__T("\t\t<essenceTrackIdentifier source=\"StreamOrder (Mediainfo)\">");
        ToReturn+=MI.Get(StreamKind, StreamPos, __T("StreamOrder"));
        ToReturn+=__T("</essenceTrackIdentifier>\n");
    }

    //essenceTrackStandard
    if (StreamKind==Stream_Video && !MI.Get(Stream_Video, StreamPos, Video_Standard).empty())
    {
        ToReturn+=__T("\t\t<essenceTrackStandard>");
        ToReturn+=MI.Get(Stream_Video, StreamPos, Video_Standard);
        ToReturn+=__T("</essenceTrackStandard>\n");
    }

    //essenceTrackEncoding
    if (!MI.Get(StreamKind, StreamPos, __T("Format")).empty())
    {
        ToReturn+=__T("\t\t<essenceTrackEncoding");
        if (!MI.Get(StreamKind, StreamPos, __T("CodecID")).empty())
        {
            ToReturn+=__T(" source=\"codecid\"");
            ToReturn+=__T(" ref=\"");
            ToReturn+=MI.Get(StreamKind, StreamPos, __T("CodecID"));
            ToReturn+=__T("\"");
        }
        if (!MI.Get(StreamKind, StreamPos, __T("Format_Version")).empty())
        {
            ToReturn+=__T(" version=\"");
            ToReturn+=MI.Get(StreamKind, StreamPos, __T("Format_Version"));
            ToReturn+=__T("\"");
        }
        if (!MI.Get(StreamKind, StreamPos, __T("Format_Profile")).empty())
        {
            ToReturn+=__T(" annotation=\"profile:");
            ToReturn+=MI.Get(StreamKind, StreamPos, __T("Format_Profile"));
            ToReturn+=__T("\"");
        }
        ToReturn+=__T(">");
        ToReturn+=MI.Get(StreamKind, StreamPos, __T("Format"));
        ToReturn+=__T("</essenceTrackEncoding>\n");
    }

    //essenceTrackDataRate
    if (!MI.Get(StreamKind, StreamPos, __T("BitRate")).empty())
    {
        ToReturn+=__T("\t\t<essenceTrackDataRate");
        ToReturn+=__T(" unitsOfMeasure=\"bits/second\"");
        if (!MI.Get(StreamKind, StreamPos, __T("BitRate_Mode")).empty())
        {
            ToReturn+=__T(" annotation=\"");
            ToReturn+=MI.Get(StreamKind, StreamPos, __T("BitRate_Mode"));
            ToReturn+=__T("\"");
        }
        ToReturn+=__T(">");
        ToReturn+=MI.Get(StreamKind, StreamPos, __T("BitRate"));
        ToReturn+=__T("</essenceTrackDataRate>\n");
    }

    //essenceTrackFrameRate
    if (StreamKind==Stream_Video && !MI.Get(Stream_Video, StreamPos, Video_FrameRate).empty())
    {
        ToReturn+=__T("\t\t<essenceTrackFrameRate");
        if (!MI.Get(Stream_Video, StreamPos, Video_FrameRate_Mode).empty())
        {
            ToReturn+=__T(" annotation=\"");
            ToReturn+=MI.Get(Stream_Video, StreamPos, Video_FrameRate_Mode);
            ToReturn+=__T("\"");
        }
        ToReturn+=__T(">");
        ToReturn+=MI.Get(Stream_Video, StreamPos, Video_FrameRate);
        ToReturn+=__T("</essenceTrackFrameRate>\n");
    }

    //essenceTrackSamplingRate
    if (StreamKind==Stream_Audio && !MI.Get(Stream_Audio, StreamPos, Audio_SamplingRate).empty())
    {
        ToReturn+=__T("\t\t<essenceTrackSamplingRate");
        ToReturn+=__T(" unitsOfMeasure=\"Hz\"");
        ToReturn+=__T(">");
        ToReturn+=MI.Get(Stream_Audio, StreamPos, Audio_SamplingRate);
        ToReturn+=__T("</essenceTrackSamplingRate>\n");
    }

    //essenceTrackBitDepth
    if (!MI.Get(StreamKind, StreamPos, __T("BitDepth")).empty())
    {
        ToReturn+=__T("\t\t<essenceTrackBitDepth>");
        ToReturn+=MI.Get(StreamKind, StreamPos, __T("BitDepth"));
        ToReturn+=__T("</essenceTrackBitDepth>\n");
    }

    //essenceTrackFrameSize
    if (StreamKind==Stream_Video && !MI.Get(Stream_Video, StreamPos, Video_Width).empty())
    {
        ToReturn+=__T("\t\t<essenceTrackFrameSize>");
        ToReturn+=MI.Get(Stream_Video, StreamPos, Video_Width);
        ToReturn+=__T('x');
        ToReturn+=MI.Get(Stream_Video, StreamPos, Video_Height);
        ToReturn+=__T("</essenceTrackFrameSize>\n");
    }

    //essenceTrackAspectRatio
    if (StreamKind==Stream_Video && !MI.Get(Stream_Video, StreamPos, Video_DisplayAspectRatio).empty())
    {
        ToReturn+=__T("\t\t<essenceTrackAspectRatio>");
        ToReturn+=MI.Get(Stream_Video, StreamPos, Video_DisplayAspectRatio);
        ToReturn+=__T("</essenceTrackAspectRatio>\n");
    }

    //essenceTrackDuration
    if (!MI.Get(StreamKind, StreamPos, __T("Duration_String3")).empty())
    {
        ToReturn+=__T("\t\t<essenceTrackDuration>");
        ToReturn+=MI.Get(StreamKind, StreamPos, __T("Duration_String3"));
        ToReturn+=__T("</essenceTrackDuration>\n");
    }

    //essenceTrackLanguage
    if (!MI.Get(StreamKind, StreamPos, __T("Language")).empty())
    {
        ToReturn+=__T("\t\t<essenceTrackLanguage>");
        ToReturn+=MediaInfoLib::Config.Iso639_2_Get(MI.Get(StreamKind, StreamPos, __T("Language")));
        ToReturn+=__T("</essenceTrackLanguage>\n");
    }

    //essenceTrackAnnotation - all fields (except *_String* and a blacklist)
    for (size_t Pos=0; Pos<MI.Count_Get(StreamKind, StreamPos); Pos++)
        if (
            MI.Get(StreamKind, StreamPos, Pos, Info_Name).find(__T("String"))==std::string::npos &&
            !MI.Get(StreamKind, StreamPos, Pos).empty() &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Bits-(Pixel*Frame)") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("BitDepth") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("BitRate") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("BitRate_Mode") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("ChannelPositions") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Codec") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Codec_Profile") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Codec_Settings") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Codec_Settings_CABAC") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Codec_Settings_RefFrames") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Codec/CC") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Codec/Family") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Codec/Info") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Codec/Url") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("CodecID") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("CodecID/Info") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("CodecID/Url") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Codec_Settings_Floor") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Colorimetry") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Count") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("DisplayAspectRatio") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Duration") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Encoded_Date") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Encoded_Library") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Format") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Format/Info") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Format/Url") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Format_Commercial") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Format_Profile") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Format_Version") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("FrameRate") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("FrameRate_Mode") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Height") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("ID") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("InternetMediaType") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Language") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Resolution") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("SamplingRate") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Standard") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("StreamCount") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("StreamKind") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("StreamKindID") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("StreamKindPos") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("StreamOrder") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("StreamSize_Proportion") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Tagged_Date") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("UniqueID") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Video_DisplayAspectRatio") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Video_FrameRate") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Video_FrameRate_Mode") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Video_Height") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Video_Standard") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Video_Width") &&
            MI.Get(StreamKind, StreamPos, Pos, Info_Name)!=__T("Width")
            )
            {
                ToReturn+=__T("\t\t<essenceTrackAnnotation");
                ToReturn+=__T(" annotationType=\"");
                ToReturn+=MI.Get(StreamKind, StreamPos, Pos, Info_Name);
                ToReturn+=__T("\">");
                ToReturn+=MI.Get(StreamKind, StreamPos, Pos);
                ToReturn+=__T("</essenceTrackAnnotation>\n");
            }
    ToReturn+=__T("\t</instantiationEssenceTrack>\n");
}

//---------------------------------------------------------------------------
Ztring Export_PBCore2::Transform(MediaInfo_Internal &MI)
{
    //Current date/time is ISO format
    time_t Time=time(NULL);
    Ztring TimeS; TimeS.Date_From_Seconds_1970((int32u)Time);
    TimeS.FindAndReplace(__T("UTC "), __T(""));
    TimeS.FindAndReplace(__T(" "), __T("T"));
    TimeS+=__T('Z');

    Ztring ToReturn;
    ToReturn+=__T("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    ToReturn+=__T("<pbcoreInstantiationDocument xsi:schemaLocation=\"http://www.pbcore.org/PBCore/PBCoreNamespace.html http://pbcore.org/xsd/pbcore-2.0.xsd\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://www.pbcore.org/PBCore/PBCoreNamespace.html\">\n");
    ToReturn+=__T("<!-- Generated at ")+TimeS+__T(" by ")+MediaInfoLib::Config.Info_Version_Get()+__T(" -->\n");

    //instantiationIdentifier
    ToReturn+=__T("\t<instantiationIdentifier source=\"File Name\">");
    ToReturn+=MI.Get(Stream_General, 0, General_FileName);
    if (!MI.Get(Stream_General, 0, General_FileExtension).empty())
    {
        ToReturn+=__T(".");
        ToReturn+=MI.Get(Stream_General, 0, General_FileExtension);
    }
    ToReturn+=__T("</instantiationIdentifier>\n");

    // need to figure out how to get to non-internally-declared-values
    //if (!MI.Get(Stream_General, 0, General_Media/UUID).empty())
    //{
    //    ToReturn+=__T("\t<instantiationIdentifier source=\"Media UUID\">")+MI.Get(Stream_General, 0, General_Media/UUID)+__T("</instantiationIdentifier>\n");
    //}

    //instantiationDates
    //dateIssued
    if (!MI.Get(Stream_General, 0, General_Recorded_Date).empty())
    {
        Ztring dateIssued=MI.Get(Stream_General, 0, General_Recorded_Date);
        dateIssued.FindAndReplace(__T("UTC"), __T(""));
        dateIssued.FindAndReplace(__T(" "), __T("T"));
        dateIssued+=__T('Z');
        ToReturn+=__T("\t<instantiationDate dateType=\"issued\">");
        ToReturn+=dateIssued+__T("</instantiationDate>\n");
    }

    //dateFileModified
    if (!MI.Get(Stream_General, 0, General_File_Modified_Date).empty())
    {
        Ztring dateModified=MI.Get(Stream_General, 0, General_File_Modified_Date);
        dateModified.FindAndReplace(__T("UTC "), __T(""));
        dateModified.FindAndReplace(__T(" "), __T("T"));
        dateModified+=__T('Z');
        ToReturn+=__T("\t<instantiationDate dateType=\"file modification\">");
        ToReturn+=dateModified+__T("</instantiationDate>\n");
    }

    //dateEncoder
    if (!MI.Get(Stream_General, 0, General_Encoded_Date).empty())
    {
        Ztring dateEncoded=MI.Get(Stream_General, 0, General_Encoded_Date);
        dateEncoded.FindAndReplace(__T("UTC "), __T(""));
        dateEncoded.FindAndReplace(__T(" "), __T("T"));
        dateEncoded+=__T('Z');
        ToReturn+=__T("\t<instantiationDate dateType=\"encoded\">");
        ToReturn+=dateEncoded+__T("</instantiationDate>\n");
    }

    //dateTagged
    if (!MI.Get(Stream_General, 0, General_Tagged_Date).empty())
    {
        Ztring dateTagged=MI.Get(Stream_General, 0, General_Tagged_Date);
        dateTagged.FindAndReplace(__T("UTC "), __T(""));
        dateTagged.FindAndReplace(__T(" "), __T("T"));
        dateTagged+=__T('Z');
        ToReturn+=__T("\t<instantiationDate dateType=\"tagged\">");
        ToReturn+=dateTagged+__T("</instantiationDate>\n");
    }

    //formatDigital
    if (!MI.Get(Stream_General, 0, General_InternetMediaType).empty())
    {
        ToReturn+=__T("\t<instantiationDigital>");
        ToReturn+=MI.Get(Stream_General, 0, General_InternetMediaType);
        ToReturn+=__T("</instantiationDigital>\n");
    }
    else
    {
        //TODO: how to implement formats without Media Type?
        ToReturn+=__T("\t<instantiationDigital>");
        if (MI.Count_Get(Stream_Video))
            ToReturn+=__T("video/x-");
        else if (MI.Count_Get(Stream_Image))
            ToReturn+=__T("image/x-");
        else if (MI.Count_Get(Stream_Audio))
            ToReturn+=__T("audio/x-");
        else
            ToReturn+=__T("application/x-");
        ToReturn+=Ztring(MI.Get(Stream_General, 0, __T("Format"))).MakeLowerCase();
        ToReturn+=__T("</instantiationDigital>\n");
    }

    //formatLocation
    ToReturn+=__T("\t<instantiationLocation>");
    ToReturn+=MI.Get(Stream_General, 0, General_CompleteName);
    ToReturn+=__T("</instantiationLocation>\n");

    //formatMediaType
    if (!PBCore2_MediaType(MI).empty())
    {
        ToReturn+=__T("\t<instantiationMediaType>");
        ToReturn+=PBCore2_MediaType(MI);
        ToReturn+=__T("</instantiationMediaType>\n");
    }

    //formatFileSize
    if (!MI.Get(Stream_General, 0, General_FileSize).empty())
    {
        ToReturn+=__T("\t<instantiationFileSize");
        ToReturn+=__T(" unitsOfMeasure=\"bytes\"");
        ToReturn+=__T(">");
        ToReturn+=MI.Get(Stream_General, 0, General_FileSize);
        ToReturn+=__T("</instantiationFileSize>\n");
    }

    //formatTimeStart
    if (!MI.Get(Stream_Video, 0, Video_Delay_Original_String3).empty())
    {
        ToReturn+=__T("\t<instantiationTimeStart>");
        ToReturn+=MI.Get(Stream_Video, 0, Video_Delay_Original_String3);
        ToReturn+=__T("</instantiationTimeStart>\n");
    }
    else if (!MI.Get(Stream_Video, 0, Video_Delay_String3).empty())
    {
        ToReturn+=__T("\t<instantiationTimeStart>");
        ToReturn+=MI.Get(Stream_Video, 0, Video_Delay_String3);
        ToReturn+=__T("</instantiationTimeStart>\n");
    }

    //formatDuration
    if (!MI.Get(Stream_General, 0, General_Duration_String3).empty())
    {
        ToReturn+=__T("\t<instantiationDuration>");
        ToReturn+=MI.Get(Stream_General, 0, General_Duration_String3);
        ToReturn+=__T("</instantiationDuration>\n");
    }

    //formatDataRate
    if (!MI.Get(Stream_General, 0, General_OverallBitRate).empty())
    {
        ToReturn+=__T("\t<instantiationDataRate");
        ToReturn+=__T(" unitsOfMeasure=\"bits/second\"");
        if (!MI.Get(Stream_General, 0, General_OverallBitRate_Mode).empty())
        {
            ToReturn+=__T(" annotation=\"");
            ToReturn+=MI.Get(Stream_General, 0, General_OverallBitRate_Mode);
            ToReturn+=__T("\"");
        }
        ToReturn+=__T(">");
        ToReturn+=MI.Get(Stream_General, 0, General_OverallBitRate);
        ToReturn+=__T("</instantiationDataRate>\n");
    }

    //formatTracks
    ToReturn+=__T("\t<instantiationTracks>")+Ztring::ToZtring(MI.Count_Get(Stream_Video)+MI.Count_Get(Stream_Audio)+MI.Count_Get(Stream_Image)+MI.Count_Get(Stream_Text))+__T("</instantiationTracks>\n");

    //Streams
    for (size_t StreamKind=Stream_General+1; StreamKind<Stream_Max; StreamKind++)
        for (size_t StreamPos=0; StreamPos<MI.Count_Get((stream_t)StreamKind); StreamPos++)
            PBCore2_Transform(ToReturn, MI, (stream_t)StreamKind, StreamPos);

    //instantiationAnnotations
    for (size_t Pos=0; Pos<MI.Count_Get(Stream_General, 0); Pos++)
        if (
            MI.Get(Stream_General, 0, Pos, Info_Name).find(__T("String"))==std::string::npos &&
            !MI.Get(Stream_General, 0, Pos).empty() &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("Count") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("StreamCount") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("StreamKind") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("StreamKindID") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("UniqueID") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("Format/Url") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("Format_Commercial") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("Codec") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("Codec/Url") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("CodecID") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("CodecID/Url") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("Encoded_Date") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("Tagged_Date") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("Codec/Url") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("Duration") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("StreamSize_Proportion") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("VideoCount") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("AudioCount") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("TextCount") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("MenuCount") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("Video_Format_List") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("Video_Format_WithHint_List") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("Video_Codec_List") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("Video_Language_List") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("Audio_Format_List") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("Audio_Format_WithHint_List") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("Audio_Codec_List") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("Audio_Language_List") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("Text_Format_List") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("Text_Format_WithHint_List") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("Text_Codec_List") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("Text_Language_List") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("CompleteName") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("FolderName") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("FileName") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("FileExtension") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("InternetMediaType") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("Format/Extensions") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("Codec/Extensions") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("FileSize") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("OverallBitRate_Mode") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("OverallBitRate") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("StreamSize") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("File_Modified_Date") &&
            MI.Get(Stream_General, 0, Pos, Info_Name)!=__T("File_Modified_Date_Local")
            )
            {
                ToReturn+=__T("\t<instantiationAnnotation");
                ToReturn+=__T(" annotationType=\"");
                ToReturn+=MI.Get(Stream_General, 0, Pos, Info_Name);
                ToReturn+=__T("\">");
                ToReturn+=MI.Get(Stream_General, 0, Pos);
                ToReturn+=__T("</instantiationAnnotation>\n");
            }

    ToReturn+=__T("</pbcoreInstantiationDocument>\n");

    //Carriage return
    ToReturn.FindAndReplace(__T("\n"), EOL, 0, Ztring_Recursive);

    return ToReturn;
}

//***************************************************************************
//
//***************************************************************************

} //NameSpace

#endif