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
#if defined(MEDIAINFO_DUMMY_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File_Dummy.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Format
//***************************************************************************

//---------------------------------------------------------------------------
void File_Dummy::FileHeader_Parse()
{
    File_Name=__T("D:\\Example\\"); File_Name+=KindOfDummy;
         if (KindOfDummy==__T("Album"))
    {
        Fill_Dummy_General();
        Fill_Dummy_Audio();
    }
    else if (KindOfDummy==__T("Comic"))
    {
        Fill_Dummy_General();
        Fill_Dummy_Video();
    }
    else if (KindOfDummy==__T("Movie"))
    {
        Fill_Dummy_General();
        Fill_Dummy_Video();
        Fill_Dummy_Video();
        Fill_Dummy_Audio();
        Fill_Dummy_Audio();
        Fill_Dummy_Audio();
        Fill_Dummy_Audio();
        Fill_Dummy_Text();
        Fill_Dummy_Text();
        Fill_Dummy_Text();
        Fill_Dummy_Text();
        Fill_Dummy_Chapters();
        Fill_Dummy_Chapters();
    }
    else
    {
        File_Name=__T("D:\\WhatIsIt.mkv");
        Fill(Stream_General, 0, General_Domain, "Starwars saga");
        Fill(Stream_General, 0, General_Movie, "Starwars 4");
        Fill(Stream_General, 0, General_Movie_More, "A new hope");
        Fill(Stream_General, 0, General_Director, "Georges Lucas");
        Fill(Stream_General, 0, General_Released_Date, "1977");
        Fill(Stream_General, 0, General_FileSize, "734000000");
        Fill(Stream_General, 0, General_Format, "Matroska");
        Fill(Stream_General, 0, General_Format_Url, "http://MediaArea.net/MediaInfo");
        Stream_Prepare(Stream_Video);
        Fill(Stream_Video, 0, Video_Codec, "XVID");
        Fill(Stream_Video, 0, Video_Codec_Url, "http://MediaArea.net/MediaInfo");
        Fill(Stream_Video, 0, Video_BitRate, "800000");
        Fill(Stream_Video, 0, Video_Width, "720");
        Fill(Stream_Video, 0, Video_Height, "320");
        Fill(Stream_Video, 0, Video_FrameRate, "24.976");
        Fill(Stream_Video, 0, Video_BitDepth, "8");
        Fill(Stream_Video, 0, Video_Language, "en");
        Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, Audio_Format, "AC-3");
        Fill(Stream_Audio, 0, Audio_Codec, "AC3");
        Fill(Stream_Audio, 0, Audio_Codec_Url, "http://MediaArea.net/MediaInfo");
        Fill(Stream_Audio, 0, Audio_BitRate, "384000");
        Fill(Stream_Audio, 0, Audio_BitRate_Mode, "CBR");
        Fill(Stream_Audio, 0, Audio_Channel_s_, "6");
        Fill(Stream_Audio, 0, Audio_SamplingRate, "48000");
        Fill(Stream_Audio, 0, Audio_Language, "en");
        Stream_Prepare(Stream_Text);
        Fill(Stream_Text, 0, Text_Codec, "SSA");
        Fill(Stream_Text, 0, Text_Codec_Url, "http://MediaArea.net/MediaInfo");
        Fill(Stream_Text, 0, Text_Language, "en");
        Fill(Stream_Text, 0, Text_Language_More, "Forced");
        Stream_Prepare(Stream_Other);
        Fill(Stream_Other, 0, Chapters_Total, "16");
        Fill(Stream_Other, 0, Chapters_Language, "en");
    }

    Accept();
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
void File_Dummy::Fill_Dummy_General()
{
    Fill(Stream_General, 0, General_Format, "Format");
    Fill(Stream_General, 0, General_Format_Url, "http://MediaArea.net/MediaInfo");
    Fill(Stream_General, 0, General_Format_Extensions, "fmt fmt fmt");
    Fill(Stream_General, 0, General_FileSize, "1000000");
    Fill(Stream_General, 0, General_Duration, "10000");
    Fill(Stream_General, 0, General_Domain, "Domain");
    Fill(Stream_General, 0, General_Collection, "Collection");
    Fill(Stream_General, 0, General_Season, "Season");
    Fill(Stream_General, 0, General_Season_Position_Total, "Season/Position_Total");
         if (KindOfDummy==__T("Album"))
    {
        Fill(Stream_General, 0, General_Album, "Album name");
        Fill(Stream_General, 0, General_Album_More, "More information about the album");
        Fill(Stream_General, 0, General_Part, "Part");
        Fill(Stream_General, 0, General_Part_Position_Total, "Part/Position_Total");
        Fill(Stream_General, 0, General_Part_Position, "Part/Position");
        Fill(Stream_General, 0, General_Track, "Track");
        Fill(Stream_General, 0, General_Track_Position, "Track/Position");
        Fill(Stream_General, 0, General_Track_More, "More information about the track");
    }
    else if (KindOfDummy==__T("Comic"))
    {
        Fill(Stream_General, 0, General_Comic, "Comic name");
        Fill(Stream_General, 0, General_Comic_More, "More information about the comic");
    }
    else //if (KindOfDummy==__T("Movie"))
    {
        Fill(Stream_General, 0, General_Movie, "Movie name");
        Fill(Stream_General, 0, General_Movie_More, "More information about the movie");
    }
    Fill(Stream_General, 0, General_Performer, "Performer");
    Fill(Stream_General, 0, General_Performer_Sort, "Performer/Sort");
    Fill(Stream_General, 0, General_Performer_Url, "Performer/Url");
    Fill(Stream_General, 0, General_Original_Performer, "Original/Performer");
    Fill(Stream_General, 0, General_Accompaniment, "Accompaniment");
    Fill(Stream_General, 0, General_Composer, "Composer");
    Fill(Stream_General, 0, General_Composer_Nationality, "Composer/Nationality");
    Fill(Stream_General, 0, General_Arranger, "Arranger");
    Fill(Stream_General, 0, General_Lyricist, "Lyricist");
    Fill(Stream_General, 0, General_Original_Lyricist, "Original/Lyricist");
    Fill(Stream_General, 0, General_Conductor, "Conductor");
    Fill(Stream_General, 0, General_Actor, "Actor");
    Fill(Stream_General, 0, General_Actor_Character, "Actor_Character");
    Fill(Stream_General, 0, General_WrittenBy, "WrittenBy");
    Fill(Stream_General, 0, General_ScreenplayBy, "ScreenplayBy");
    Fill(Stream_General, 0, General_Director, "Director");
    Fill(Stream_General, 0, General_AssistantDirector, "AssistantDirector");
    Fill(Stream_General, 0, General_DirectorOfPhotography, "DirectorOfPhotography");
    Fill(Stream_General, 0, General_ArtDirector, "ArtDirector");
    Fill(Stream_General, 0, General_EditedBy, "EditedBy");
    Fill(Stream_General, 0, General_Producer, "Producer");
    Fill(Stream_General, 0, General_CoProducer, "CoProducer");
    Fill(Stream_General, 0, General_ExecutiveProducer, "ExecutiveProducer");
    Fill(Stream_General, 0, General_ProductionDesigner, "ProductionDesigner");
    Fill(Stream_General, 0, General_CostumeDesigner, "CostumeDesigner");
    Fill(Stream_General, 0, General_Choregrapher, "Choregrapher");
    Fill(Stream_General, 0, General_SoundEngineer, "SoundEngineer");
    Fill(Stream_General, 0, General_MasteredBy, "MasteredBy");
    Fill(Stream_General, 0, General_RemixedBy, "RemixedBy");
    Fill(Stream_General, 0, General_ProductionStudio, "ProductionStudio");
    Fill(Stream_General, 0, General_Label, "Label");
    Fill(Stream_General, 0, General_Publisher, "Publisher");
    Fill(Stream_General, 0, General_Publisher_URL, "Publisher/URL");
    Fill(Stream_General, 0, General_DistributedBy, "DistributedBy");
    Fill(Stream_General, 0, General_EncodedBy, "EncodedBy");
    Fill(Stream_General, 0, General_ThanksTo, "ThanksTo");
    Fill(Stream_General, 0, General_ServiceName, "ServiceNeme");
    Fill(Stream_General, 0, General_ServiceProvider, "ServiceProvider");
    Fill(Stream_General, 0, General_Service_Url, "Service/URL");
    Fill(Stream_General, 0, General_ContentType, "ContentType");
    Fill(Stream_General, 0, General_Subject, "Subject");
    Fill(Stream_General, 0, General_Synopsis, "Synopsis");
    Fill(Stream_General, 0, General_Summary, "Summary");
    Fill(Stream_General, 0, General_Description, "Description");
    Fill(Stream_General, 0, General_Keywords, "Keywords");
    Fill(Stream_General, 0, General_Period, "Period");
    Fill(Stream_General, 0, General_LawRating, "LawRating");
    Fill(Stream_General, 0, General_Written_Date, "Written_Date");
    Fill(Stream_General, 0, General_Recorded_Date, "Recorded_Date");
    Fill(Stream_General, 0, General_Released_Date, "Released_Date");
    Fill(Stream_General, 0, General_Mastered_Date, "Mastered_Date");
    Fill(Stream_General, 0, General_Encoded_Date, "Encoded_Date");
    Fill(Stream_General, 0, General_Tagged_Date, "Tagged_Date");
    Fill(Stream_General, 0, General_Original_Released_Date, "Original/Released_Date");
    Fill(Stream_General, 0, General_Written_Location, "Written_Location");
    Fill(Stream_General, 0, General_Recorded_Location, "Recorded_Location");
    Fill(Stream_General, 0, General_Archival_Location, "Archival_Location");
    Fill(Stream_General, 0, General_Genre, "Genre");
    Fill(Stream_General, 0, General_Mood, "Mood");
    Fill(Stream_General, 0, General_Comment, "Comment");
    Fill(Stream_General, 0, General_Rating , "Rating ");
    Fill(Stream_General, 0, General_Encoded_Application, "Encoded_Application");
    Fill(Stream_General, 0, General_Encoded_Library, "Encoded_Library");
    Fill(Stream_General, 0, General_Encoded_Library_Settings, "Encoded_Library_Settings");
    Fill(Stream_General, 0, General_Copyright, "Copyright");
    Fill(Stream_General, 0, General_Producer_Copyright, "Producer_Copyright");
    Fill(Stream_General, 0, General_TermsOfUse, "TermsOfUse");
    Fill(Stream_General, 0, General_Copyright_Url, "Copyright/Url");
    Fill(Stream_General, 0, General_ISRC, "ISRC");
    Fill(Stream_General, 0, General_ISBN, "ISBN");
    Fill(Stream_General, 0, General_BarCode, "BarCode");
    Fill(Stream_General, 0, General_LCCN, "LCCN");
    Fill(Stream_General, 0, General_CatalogNumber, "CatalogNumber");
    Fill(Stream_General, 0, General_LabelCode, "LabelCode");
    Fill(Stream_General, 0, General_Cover, "Y");
    Fill(Stream_General, 0, General_Cover_Data, "Cover_Datas");
    Fill(Stream_General, 0, General_Summary, "Summary");
    Fill(Stream_General, 0, General_BPM, "100");
}

//---------------------------------------------------------------------------
void File_Dummy::Fill_Dummy_Video()
{
    Stream_Prepare(Stream_Video);
    Fill(Stream_Video, 0, Video_ID, "ID");
    Fill(Stream_Video, 0, Video_UniqueID, "UniqueID");
    Fill(Stream_Video, 0, Video_Title, "Title");
    Fill(Stream_Video, 0, Video_Codec, "Codec");
    Fill(Stream_Video, 0, Video_Codec_Info, "Codec/Info");
    Fill(Stream_Video, 0, Video_Codec_Url, "http://--Codec/Url--");
    Fill(Stream_Video, 0, Video_BitRate, "10000");
    Fill(Stream_Video, 0, Video_BitRate_Mode, "BitRate_Mode");
    Fill(Stream_Video, 0, Video_Encoded_Library, "Encoded_Library");
    Fill(Stream_Video, 0, Video_Encoded_Library_Settings, "Encoded_Library_Settings");
         if (KindOfDummy==__T("Album"))
    {
        Fill(Stream_Video, 0, Video_Width, "2000");
        Fill(Stream_Video, 0, Video_Height, "3000");
    }
    else //if (KindOfDummy==__T("Movie"))
    {
        Fill(Stream_Video, 0, Video_DisplayAspectRatio, "2");
        Fill(Stream_Video, 0, Video_FrameRate, "24.976");
        Fill(Stream_Video, 0, Video_FrameCount, "FrameCount");
        Fill(Stream_Video, 0, Video_BitDepth, "8");
        Fill(Stream_Video, 0, Video_Bits__Pixel_Frame_, "Bits/(Pixel*Frame)");
        Fill(Stream_Video, 0, Video_Delay, "100");
        Fill(Stream_Video, 0, Video_Duration, "990000");
    }
    Fill(Stream_Video, 0, Video_Language, "eng");
    Fill(Stream_Video, 0, Video_Language_More, "Language_More");
}

//---------------------------------------------------------------------------
void File_Dummy::Fill_Dummy_Audio()
{
    Stream_Prepare(Stream_Audio);
    Fill(Stream_Audio, 0, Audio_ID, "ID");
    Fill(Stream_Audio, 0, Audio_UniqueID, "UniqueID");
    Fill(Stream_Audio, 0, Audio_Title, "Title");
    Fill(Stream_Audio, 0, Audio_Format, "Format");
    Fill(Stream_Audio, 0, Audio_Format_Info, "Format/Info");
    Fill(Stream_Audio, 0, Audio_Format_Url, "http://--Format/Url--");
    Fill(Stream_Audio, 0, Audio_Codec, "Codec");
    Fill(Stream_Audio, 0, Audio_Codec_Info, "Codec/Info");
    Fill(Stream_Audio, 0, Audio_Codec_Url, "http://--Codec/Url--");
    Fill(Stream_Audio, 0, Audio_BitRate, "1000");
    Fill(Stream_Audio, 0, Audio_BitRate_Mode, "BitRate_Mode");
    Fill(Stream_Audio, 0, Audio_Encoded_Library, "Encoded_Library");
    Fill(Stream_Audio, 0, Audio_Encoded_Library_Settings, "Encoded_Library_Settings");
    Fill(Stream_Audio, 0, Audio_Channel_s_, 2);
    Fill(Stream_Audio, 0, Audio_ChannelPositions, "ChannelPositions");
    Fill(Stream_Audio, 0, Audio_SamplingRate, "48000");
    Fill(Stream_Audio, 0, Audio_SamplingCount, "SamplingCount");
    Fill(Stream_Audio, 0, Audio_BitDepth, "BitDepth");
    Fill(Stream_Audio, 0, Audio_Delay, "10");
    Fill(Stream_Audio, 0, Audio_Duration, "100000");
    Fill(Stream_Audio, 0, Audio_Language, "fre");
    Fill(Stream_Audio, 0, Audio_Language_More, "Language_More");
}

//---------------------------------------------------------------------------
void File_Dummy::Fill_Dummy_Text()
{
    Stream_Prepare(Stream_Text);
    Fill(Stream_Text, 0, Text_ID, "ID");
    Fill(Stream_Text, 0, Text_UniqueID, "UniqueID");
    Fill(Stream_Text, 0, Text_Title, "Title");
    Fill(Stream_Text, 0, Text_Codec, "Codec");
    Fill(Stream_Text, 0, Text_Codec_Url, "http://--Codec/Url--");
    Fill(Stream_Text, 0, Text_Delay, "100");
    Fill(Stream_Text, 0, Text_Duration, "100");
    Fill(Stream_Text, 0, Text_Language, "de");
    Fill(Stream_Text, 0, Text_Language_More, "Language_More");
    Fill(Stream_Text, 0, Text_Summary, "Summary");
}

//---------------------------------------------------------------------------
void File_Dummy::Fill_Dummy_Chapters()
{
    Stream_Prepare(Stream_Other);
    Fill(Stream_Other, 0, Chapters_ID, "ID");
    Fill(Stream_Other, 0, Chapters_UniqueID, "UniqueID");
    Fill(Stream_Other, 0, Chapters_Title, "Title");
    Fill(Stream_Other, 0, Chapters_Total, "Total");
    Fill(Stream_Other, 0, Chapters_Language, "de");
}

} //NameSpace

#endif
