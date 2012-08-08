// File__Analyze - Base for analyze files
// Copyright (C) 2009-2012 Jerome Martinez, Zen@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
#include "MediaInfo/Export/Export_PBCore.h"
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
Ztring PBCore_MediaType(MediaInfo_Internal &MI)
{
         if (MI.Count_Get(Stream_Video))
        return __T("Video");
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
Export_PBCore::Export_PBCore ()
{
}

//---------------------------------------------------------------------------
Export_PBCore::~Export_PBCore ()
{
}

//***************************************************************************
// Input
//***************************************************************************

//---------------------------------------------------------------------------
void PBCore_Transform__Common_Begin(Ztring &ToReturn, MediaInfo_Internal &MI, stream_t StreamKind, size_t StreamPos)
{
    //essenceTrackIdentifier
    if (!MI.Get(StreamKind, StreamPos, __T("ID")).empty())
    {
        ToReturn+=__T("\t\t\t<essenceTrackIdentifier>")+MI.Get(StreamKind, StreamPos, __T("ID"))+__T("</essenceTrackIdentifier>\n");
        ToReturn+=__T("\t\t\t<essenceTrackIdentifierSource>ID (Mediainfo)</essenceTrackIdentifierSource>\n");
    }
    else if (!MI.Get(StreamKind, StreamPos, __T("UniqueID")).empty())
    {
        ToReturn+=__T("\t\t\t<essenceTrackIdentifier>")+MI.Get(StreamKind, StreamPos, __T("UniqueID"))+__T("</essenceTrackIdentifier>\n");
        ToReturn+=__T("\t\t\t<essenceTrackIdentifierSource>UniqueID (Mediainfo)</essenceTrackIdentifierSource>\n");
    }
    else if (!MI.Get(StreamKind, StreamPos, __T("StreamKindID")).empty())
    {
        ToReturn+=__T("\t\t\t<essenceTrackIdentifier>")+MI.Get(StreamKind, StreamPos, __T("StreamKindID"))+__T("</essenceTrackIdentifier>\n");
        ToReturn+=__T("\t\t\t<essenceTrackIdentifierSource>StreamKindID (Mediainfo)</essenceTrackIdentifierSource>\n");
    }

    //essenceTrackStandard
    if (StreamKind==Stream_Video && !MI.Get(Stream_Video, StreamPos, Video_Standard).empty()) //Video only, but must be placed here
        ToReturn+=__T("\t\t\t<essenceTrackStandard>")+MI.Get(Stream_Video, StreamPos, Video_Standard)+__T("</essenceTrackStandard>\n");

    //essenceTrackLanguage
    if (!MI.Get(StreamKind, StreamPos, __T("Language")).empty())
        ToReturn+=__T("\t\t\t<essenceTrackLanguage>")+MediaInfoLib::Config.Iso639_2_Get(MI.Get(StreamKind, StreamPos, __T("Language")))+__T("</essenceTrackLanguage>\n");

    //essenceTrackEncoding
    if (!MI.Get(StreamKind, StreamPos, __T("Format")).empty())
    {
        ToReturn+=__T("\t\t\t<essenceTrackEncoding>");
        ToReturn+=MI.Get(StreamKind, StreamPos, __T("Format"));
        if (!MI.Get(StreamKind, StreamPos, __T("Format_Profile")).empty()) ToReturn+=__T(' ')+MI.Get(StreamKind, StreamPos, __T("Format_Profile"));
        if (!MI.Get(StreamKind, StreamPos, __T("CodecID")).empty()) ToReturn+=__T(" (")+MI.Get(StreamKind, StreamPos, __T("CodecID"))+__T(')');
        ToReturn+=__T("</essenceTrackEncoding>\n");
    }

    //essenceTrackDataRate
    if (!MI.Get(StreamKind, StreamPos, __T("BitRate")).empty())
    {
        ToReturn+=__T("\t\t\t<essenceTrackDataRate>");
        ToReturn+=MI.Get(StreamKind, StreamPos, __T("BitRate"));
        if (!MI.Get(StreamKind, StreamPos, __T("BitRate_Mode")).empty())
            ToReturn+=__T(' ')+MI.Get(StreamKind, StreamPos, __T("BitRate_Mode"));
        ToReturn+=__T("</essenceTrackDataRate>\n");
    }

    //essenceTrackDuration
    if (!MI.Get(StreamKind, StreamPos, __T("Duration")).empty())
        ToReturn+=__T("\t\t\t<essenceTrackDuration>")+MI.Get(StreamKind, StreamPos, __T("Duration"))+__T("</essenceTrackDuration>\n");

    //essenceTrackBitDepth
    if (!MI.Get(StreamKind, StreamPos, __T("Resolution")).empty())
        ToReturn+=__T("\t\t\t<essenceTrackBitDepth version=\"PBCoreXSD_Ver_1.2_D1\">")+MI.Get(StreamKind, StreamPos, __T("Resolution"))+__T("</essenceTrackBitDepth>\n");
}

//---------------------------------------------------------------------------
void PBCore_Transform__Common_End(Ztring &ToReturn, MediaInfo_Internal &MI, stream_t StreamKind, size_t StreamPos)
{
    //essenceTrackAnnotation - all fields (except *_String*) separated by |
    Ztring Temp;
    for (size_t Pos=0; Pos<MI.Count_Get(StreamKind, StreamPos); Pos++)
        if (MI.Get(StreamKind, StreamPos, Pos, Info_Name).find(__T("String"))==std::string::npos && !MI.Get(StreamKind, StreamPos, Pos).empty())
            Temp+=MI.Get(StreamKind, StreamPos, Pos, Info_Name)+__T(": ")+MI.Get(StreamKind, StreamPos, Pos)+__T('|');
    if (!Temp.empty())
    {
        Temp.resize(Temp.size()-1);
        ToReturn+=__T("\t\t\t<essenceTrackAnnotation>"); ToReturn+=Temp; ToReturn+=__T("</essenceTrackAnnotation>\n");
    }
}

//---------------------------------------------------------------------------
void PBCore_Transform_Video(Ztring &ToReturn, MediaInfo_Internal &MI, size_t StreamPos)
{
    ToReturn+=__T("\t\t<pbcoreEssenceTrack>\n");

    //essenceTrackType
    ToReturn+=__T("\t\t\t<essenceTrackType>Video</essenceTrackType>\n");

    //Common
    PBCore_Transform__Common_Begin(ToReturn, MI, Stream_Video, StreamPos);

    //essenceTrackFrameSize
    if (!MI.Get(Stream_Video, StreamPos, Video_Width).empty())
        ToReturn+=__T("\t\t\t<essenceTrackFrameSize>")+MI.Get(Stream_Video, StreamPos, Video_Width)+__T('x')+MI.Get(Stream_Video, StreamPos, Video_Height)+__T("</essenceTrackFrameSize>\n");

    //essenceTrackAspectRatio
    if (!MI.Get(Stream_Video, StreamPos, Video_DisplayAspectRatio).empty())
        ToReturn+=__T("\t\t\t<essenceTrackAspectRatio>")+MI.Get(Stream_Video, StreamPos, Video_DisplayAspectRatio)+__T("</essenceTrackAspectRatio>\n");

    //essenceTrackFrameRate
    if (!MI.Get(Stream_Video, StreamPos, Video_FrameRate).empty())
    {
        ToReturn+=__T("\t\t\t<essenceTrackFrameRate>");
        ToReturn+=MI.Get(Stream_Video, StreamPos, Video_FrameRate);
        if (!MI.Get(Stream_Video, StreamPos, Video_FrameRate_Mode).empty())
            ToReturn+=__T(' ')+MI.Get(Stream_Video, StreamPos, Video_FrameRate_Mode);
        ToReturn+=__T("</essenceTrackFrameRate>\n");
    }

    //Comon
    PBCore_Transform__Common_End(ToReturn, MI, Stream_Video, StreamPos);

    ToReturn+=__T("\t\t</pbcoreEssenceTrack>\n");
}

//---------------------------------------------------------------------------
void PBCore_Transform_Audio(Ztring &ToReturn, MediaInfo_Internal &MI, size_t StreamPos)
{
    ToReturn+=__T("\t\t<pbcoreEssenceTrack>\n");

    //essenceTrackType
    ToReturn+=__T("\t\t\t<essenceTrackType>Audio</essenceTrackType>\n");

    //Common
    PBCore_Transform__Common_Begin(ToReturn, MI, Stream_Audio, StreamPos);

    if (!MI.Get(Stream_Audio, StreamPos, Audio_SamplingRate).empty())
        ToReturn+=__T("\t\t\t<essenceTrackSamplingRate>")+MI.Get(Stream_Audio, StreamPos, Audio_SamplingRate)+__T("</essenceTrackSamplingRate>\n");

    //Comon
    PBCore_Transform__Common_End(ToReturn, MI, Stream_Audio, StreamPos);

    ToReturn+=__T("\t\t</pbcoreEssenceTrack>\n");
}

//---------------------------------------------------------------------------
void PBCore_Transform_Text(Ztring &ToReturn, MediaInfo_Internal &MI, size_t StreamPos)
{
    //Init
    Ztring Format=MI.Get(Stream_Text, StreamPos, Text_Format);

    ToReturn+=__T("\t\t<pbcoreEssenceTrack>\n");

    //essenceTrackType
    ToReturn+=__T("\t\t\t<essenceTrackType>");
    if (Format==__T("EIA-608") || Format==__T("EIA-708"))
        ToReturn+=__T("caption");
    else
        ToReturn+=__T("text");
    ToReturn+=__T("</essenceTrackType>\n");

    //Common
    PBCore_Transform__Common_Begin(ToReturn, MI, Stream_Text, StreamPos);

    //Common
    PBCore_Transform__Common_End(ToReturn, MI, Stream_Text, StreamPos);

    ToReturn+=__T("\t\t</pbcoreEssenceTrack>\n");
}

//---------------------------------------------------------------------------
void PBCore_Transform_Menu(Ztring &ToReturn, MediaInfo_Internal &MI, size_t StreamPos)
{
    //Only if TimeCode
    if (MI.Get(Stream_Menu, StreamPos, Menu_Format)!=__T("TimeCode"))
        return;

    ToReturn+=__T("\t\t<pbcoreEssenceTrack>\n");

    //essenceTrackType
    ToReturn+=__T("\t\t\t<essenceTrackType>timecode</essenceTrackType>\n");

    //Common
    PBCore_Transform__Common_Begin(ToReturn, MI, Stream_Menu, StreamPos);

    //Common
    PBCore_Transform__Common_End(ToReturn, MI, Stream_Menu, StreamPos);

    ToReturn+=__T("\t\t</pbcoreEssenceTrack>\n");
}

//---------------------------------------------------------------------------
Ztring Export_PBCore::Transform(MediaInfo_Internal &MI)
{
    //Current date/time is ISO format
    time_t Time=time(NULL);
    Ztring TimeS; TimeS.Date_From_Seconds_1970((int32u)Time);
    TimeS.FindAndReplace(__T("UTC "), __T(""));
    TimeS.FindAndReplace(__T(" "), __T("T"));
    TimeS+=__T('Z');

    Ztring ToReturn;
    ToReturn+=__T("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    ToReturn+=__T("<PBCoreDescriptionDocument xsi:schemaLocation=\"http://www.pbcore.org/PBCore/PBCoreNamespace.html http://www.pbcore.org/PBCore/PBCoreXSD_Ver_1-2-1.xsd\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"http://www.pbcore.org/PBCore/PBCoreNamespace.html\">\n");
    ToReturn+=__T("\t<!-- Generated at ")+TimeS+__T(" by ")+MediaInfoLib::Config.Info_Version_Get()+__T(" -->\n");
    ToReturn+=__T("\t<!-- Warning: MediaInfo outputs only pbcoreInstantiation, other mandatory PBCore data is junk -->\n");
    ToReturn+=__T("\t<pbcoreIdentifier>\n");
    ToReturn+=__T("\t\t<identifier>***</identifier>\n");
    ToReturn+=__T("\t\t<identifierSource>***</identifierSource>\n");
    ToReturn+=__T("\t</pbcoreIdentifier>\n");
    ToReturn+=__T("\t<pbcoreTitle>\n");
    ToReturn+=__T("\t\t<title>***</title>\n");
    ToReturn+=__T("\t</pbcoreTitle>\n");
    ToReturn+=__T("\t<pbcoreDescription>\n");
    ToReturn+=__T("\t\t<description>***</description>\n");
    ToReturn+=__T("\t\t<descriptionType>***</descriptionType>\n");
    ToReturn+=__T("\t</pbcoreDescription>\n");
    ToReturn+=__T("\t<pbcoreInstantiation>\n");


    //pbcoreFormatID
    ToReturn+=__T("\t\t<pbcoreFormatID>\n");
        //formatIdentifier
        ToReturn+=__T("\t\t\t<formatIdentifier>")+MI.Get(Stream_General, 0, General_FileName)+__T("</formatIdentifier>\n");
        //formatIdentifierSource
        ToReturn+=__T("\t\t\t<formatIdentifierSource version=\"PBCoreXSD_Ver_1.2_D1\">File Name</formatIdentifierSource>\n");
    ToReturn+=__T("\t\t</pbcoreFormatID>\n");

    //formatDigital
    if (!MI.Get(Stream_General, 0, General_InternetMediaType).empty())
    {
        ToReturn+=__T("\t\t<formatDigital>");
        ToReturn+=MI.Get(Stream_General, 0, General_InternetMediaType);
        ToReturn+=__T("</formatDigital>\n");
    }
    else
    {
        //TODO: how to implement formats without Media Type?
        ToReturn+=__T("\t\t<formatDigital>");
        if (MI.Count_Get(Stream_Video))
            ToReturn+=__T("video/x-");
        else if (MI.Count_Get(Stream_Image))
            ToReturn+=__T("image/x-");
        else if (MI.Count_Get(Stream_Audio))
            ToReturn+=__T("audio/x-");
        else
            ToReturn+=__T("application/x-");
        ToReturn+=Ztring(MI.Get(Stream_General, 0, __T("Format"))).MakeLowerCase();
        ToReturn+=__T("</formatDigital>\n");
    }

    //formatLocation
    ToReturn+=__T("\t\t<formatLocation>")+MI.Get(Stream_General, 0, General_CompleteName)+__T("</formatLocation>\n");

    //dateCreated
    if (!MI.Get(Stream_General, 0, General_Encoded_Date).empty())
    {
        Ztring dateCreated=MI.Get(Stream_General, 0, General_Recorded_Date);
        dateCreated.FindAndReplace(__T("UTC"), __T("-"));
        dateCreated.FindAndReplace(__T(" "), __T("T"));
        dateCreated+=__T('Z');
        ToReturn+=__T("\t\t<dateCreated>")+dateCreated+__T("</dateCreated>\n");
    }

    //dateIssued
    if (!MI.Get(Stream_General, 0, General_Recorded_Date).empty())
    {
        Ztring dateIssued=MI.Get(Stream_General, 0, General_Recorded_Date);
        dateIssued.FindAndReplace(__T("UTC"), __T("-"));
        dateIssued.FindAndReplace(__T(" "), __T("T"));
        dateIssued+=__T('Z');
        ToReturn+=__T("\t\t<dateIssued>")+dateIssued+__T("</dateIssued>\n");
    }

    //formatMediaType
    if (!PBCore_MediaType(MI).empty())
        ToReturn+=__T("\t\t<formatMediaType version=\"PBCoreXSD_Ver_1.2_D1\">")+PBCore_MediaType(MI)+__T("</formatMediaType>\n");
    else
        ToReturn+=__T("\t\t<formatMediaType version=\"PBCoreXSD_Ver_1.2_D1\">application/octet-stream</formatMediaType>\n");

    //formatGenerations
    ToReturn+=__T("\t\t<formatGenerations version=\"PBCoreXSD_Ver_1.2_D1\" />\n");

    //formatFileSize
    if (!MI.Get(Stream_General, 0, General_FileSize).empty())
        ToReturn+=__T("\t\t<formatFileSize>")+MI.Get(Stream_General, 0, General_FileSize)+__T("</formatFileSize>\n");

    //formatTimeStart
    if (!MI.Get(Stream_Video, 0, Video_Delay_Original_String3).empty())
        ToReturn+=__T("\t\t<formatTimeStart>")+MI.Get(Stream_Video, 0, Video_Delay_Original_String3)+__T("</formatTimeStart>\n");
    else if (!MI.Get(Stream_Video, 0, Video_Delay_String3).empty())
        ToReturn+=__T("\t\t<formatTimeStart>")+MI.Get(Stream_Video, 0, Video_Delay_String3)+__T("</formatTimeStart>\n");

    //formatDuration
    if (!MI.Get(Stream_General, 0, General_Duration_String3).empty())
        ToReturn+=__T("\t\t<formatDuration>")+MI.Get(Stream_General, 0, General_Duration_String3)+__T("</formatDuration>\n");

    //formatDataRate
    if (!MI.Get(Stream_General, 0, General_OverallBitRate).empty())
    {
        ToReturn+=__T("\t\t<formatDataRate>");
        ToReturn+=MI.Get(Stream_General, 0, General_OverallBitRate);
        if (!MI.Get(Stream_General, 0, General_OverallBitRate_Mode).empty())
            ToReturn+=__T(' ')+MI.Get(Stream_General, 0, General_OverallBitRate_Mode);
        ToReturn+=__T("</formatDataRate>\n");
    }

    //formatTracks
    ToReturn+=__T("\t\t<formatTracks>")+Ztring::ToZtring(MI.Count_Get(Stream_Video)+MI.Count_Get(Stream_Audio)+MI.Count_Get(Stream_Image)+MI.Count_Get(Stream_Text))+__T("</formatTracks>\n");

    //Video streams
    for (size_t StreamPos=0; StreamPos<MI.Count_Get(Stream_Video); StreamPos++)
        PBCore_Transform_Video(ToReturn, MI, StreamPos);

    //Audio streams
    for (size_t StreamPos=0; StreamPos<MI.Count_Get(Stream_Audio); StreamPos++)
        PBCore_Transform_Audio(ToReturn, MI, StreamPos);

    //Text streams
    for (size_t StreamPos=0; StreamPos<MI.Count_Get(Stream_Text); StreamPos++)
        PBCore_Transform_Text(ToReturn, MI, StreamPos);

    //Menu streams
    for (size_t StreamPos=0; StreamPos<MI.Count_Get(Stream_Menu); StreamPos++)
        PBCore_Transform_Menu(ToReturn, MI, StreamPos);

    ToReturn+=__T("\t</pbcoreInstantiation>\n");
    ToReturn+=__T("</PBCoreDescriptionDocument>\n");

    //Carriage return
    ToReturn.FindAndReplace(__T("\n"), EOL, 0, Ztring_Recursive);

    return ToReturn;
}

//***************************************************************************
//
//***************************************************************************

} //NameSpace
