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
#if defined(MEDIAINFO_VORBISCOM_YES) || defined(MEDIAINFO_OGG_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Tag/File_VorbisCom.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
extern const char* Id3v2_PictureType(int8u Type); //In Tag/File_Id3v2.cpp
extern std::string ExtensibleWave_ChannelMask (int32u ChannelMask); //In Multiple/File_Riff_Elements.cpp
extern std::string ExtensibleWave_ChannelMask2 (int32u ChannelMask); //In Multiple/File_Riff_Elements.cpp

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_VorbisCom::File_VorbisCom()
:File__Analyze()
{
    //In
    StreamKind_Specific=Stream_General;
    StreamKind_Multiple=Stream_General;
    StreamKind_Common  =Stream_General;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_VorbisCom::Streams_Fill()
{
    if (!Performers.empty())
    {
        Artists.Separator_Set(0, __T(" / "));
        Fill(StreamKind_Common,   0, "Performer", Performers.Read());
    }

    if (!Artists.empty() && Artists!=Performers)
    {
        Artists.Separator_Set(0, __T(" / "));
        Fill(StreamKind_Common,   0, Performers.empty()?"Performer":"Composer", Artists.Read());
    }

    if (!Accompaniments.empty() && Accompaniments!=Artists && Accompaniments!=Performers)
    {
        Artists.Separator_Set(0, __T(" / "));
        Fill(StreamKind_Common,   0, "Accompaniment", Accompaniments.Read());
    }

    if (!AlbumArtists.empty())
    {
        AlbumArtists.Separator_Set(0, __T(" / "));
        Fill(StreamKind_Common,   0, (Performers==Artists || Performers.empty())?"Album/Performer":"Album/Composer", AlbumArtists.Read());
    }
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
void File_VorbisCom::FileHeader_Parse()
{
    //Parsing
    Ztring vendor_string;
    int32u vendor_length;
    Get_L4 (vendor_length,                                      "vendor_length");
    Get_Local(vendor_length, vendor_string,                     "vendor_string");
    Get_L4 (user_comment_list_length,                           "user_comment_list_length");

    FILLING_BEGIN();
        Accept("VorbisCom");

        if (Count_Get(Stream_General)==0)
            Stream_Prepare(Stream_General);
        if (StreamKind_Specific!=Stream_General)
            Stream_Prepare(StreamKind_Specific);
        if (StreamKind_Multiple!=Stream_General && StreamKind_Multiple!=StreamKind_Specific)
            Stream_Prepare(StreamKind_Multiple);

        //vendor_string
        if (StreamKind_Specific!=Stream_Audio && vendor_string.find(__T("Xiph.Org libVorbis"))==0)
            vendor_string.clear(); //string was set "by default"
        Ztring Library_Name, Library_Version, Library_Date;
        Ztring vendor_string_Without=vendor_string; vendor_string_Without.FindAndReplace(__T(";"), __T(""), 0, Ztring_Recursive);
        Library_Version=MediaInfoLib::Config.Library_Get(InfoLibrary_Format_VorbisCom, vendor_string_Without, InfoLibrary_Version);
        Library_Date=MediaInfoLib::Config.Library_Get(InfoLibrary_Format_VorbisCom, vendor_string_Without, InfoLibrary_Date);
        if (Library_Version.empty())
        {
            if (vendor_string.find(__T(" I "))!=std::string::npos)
            {
                Library_Name=vendor_string.SubString(__T(""), __T(" I "));
                Library_Date=vendor_string.SubString(__T(" I "), __T(""));
                if (Library_Date.size()>9)
                {
                    Library_Version=Library_Date.substr(9, std::string::npos);
                    if (Library_Version.find(__T("("))==std::string::npos)
                    {
                        Library_Version.FindAndReplace(__T(" "), __T("."), 0, Ztring_Recursive);
                        Library_Date.resize(8);
                    }
                }
            }
            else if (vendor_string.size()>9 && Ztring(vendor_string.substr(vendor_string.size()-8, std::string::npos)).To_int32u()>20000000)
            {
                Library_Name=vendor_string.substr(0, vendor_string.size()-9);
                Library_Date=vendor_string.substr(vendor_string.size()-8, std::string::npos);
                if (!Library_Name.empty())
                {
                    std::string::size_type Pos=Library_Name.rfind(__T(' '));
                    if (Pos<Library_Name.size()-2 && Library_Name[Pos+1]>=__T('0') && Library_Name[Pos+1]<=__T('9'))
                    {
                        Library_Version=Library_Name.substr(Pos+1, std::string::npos);
                        Library_Name.resize(Pos);
                    }
                }
            }
            else if (vendor_string.find(__T("aoTuV "))!=std::string::npos)
            {
                Library_Name=__T("aoTuV");
                Library_Version=vendor_string.SubString(__T("aoTuV "), __T("["));
                Library_Date=vendor_string.SubString(__T("["), __T("]"));
            }
            else if (vendor_string.find(__T("Lancer "))!=std::string::npos)
            {
                Library_Name=__T("Lancer");
                Library_Date=vendor_string.SubString(__T("["), __T("]"));
            }
            if (Library_Version.empty())
                Library_Version=Library_Date;
            if (Library_Date.size()==8)
            {
                Library_Date.insert(6, 1, __T('-'));
                Library_Date.insert(4, 1, __T('-'));
                Library_Date.insert(0, __T("UTC "));
            }
        }
        if (vendor_string.find(__T("libFLAC"))!=std::string::npos) Library_Name="libFLAC";
        if (vendor_string.find(__T("libVorbis I"))!=std::string::npos) Library_Name="libVorbis";
        if (vendor_string.find(__T("libTheora I"))!=std::string::npos) Library_Name="libTheora";
        if (vendor_string.find(__T("AO; aoTuV"))==0) Library_Name="aoTuV";
        if (vendor_string.find(__T("BS; Lancer"))==0) Library_Name="Lancer";

        Fill(StreamKind_Specific, 0, "Encoded_Library", vendor_string);
        Fill(StreamKind_Specific, 0, "Encoded_Library/Name", Library_Name);
        Fill(StreamKind_Specific, 0, "Encoded_Library/Version", Library_Version);
        Fill(StreamKind_Specific, 0, "Encoded_Library/Date", Library_Date);
    FILLING_END();
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_VorbisCom::Header_Parse()
{
    //Parsing
    int32u user_comment_length;
    Get_L4 (user_comment_length,                                "length");

    //Filling
    Header_Fill_Size(Element_Offset+user_comment_length);
}

//---------------------------------------------------------------------------
void File_VorbisCom::Data_Parse()
{
    user_comment_list_length--;

    //Parsing
    Ztring comment;
    Get_UTF8(Element_Size, comment,                             "comment");
    if (Element_Size && comment.empty())
    {
        Element_Offset=0; //Retry
        Get_Local(Element_Size, comment,                             "comment");
    }
    Element_Name(comment);

    FILLING_BEGIN_PRECISE();
        Ztring Key=comment.SubString(__T(""), __T("="));
        Key.MakeUpperCase();
        Ztring Value=comment.SubString(__T("="), __T(""));

             if (Key==__T("ADDED_TIMESTAMP"))        Fill(StreamKind_Common,   0, "Added_Date", Ztring().Date_From_Milliseconds_1601(Value.To_int64u()/1000));
        else if (Key==__T("ALBUM ARTIST"))           AlbumArtists.push_back(Value);
        else if (Key==__T("ALBUM"))                  Fill(StreamKind_Common,   0, "Album", Value);
        else if (Key==__T("ALBUM_COMMENT"))          Fill(StreamKind_Common,   0, "Comment", Value);
        else if (Key==__T("ALBUMARTIST"))            AlbumArtists.push_back(Value);
        else if (Key==__T("ARTIST"))                 Artists.push_back(Value);
        else if (Key==__T("AUTHOR"))                 Fill(StreamKind_Common,   0, "WrittenBy", Value);
        else if (Key==__T("BUYCDURL"))               {}
        else if (Key==__T("CLASS"))                  Fill(StreamKind_Common,   0, "ContentType", Value);
        else if (Key==__T("COMPOSER"))               Fill(StreamKind_Common,   0, "Composer", Value);
        else if (Key==__T("COMMENT"))                Fill(StreamKind_Common,   0, "Comment", Value);
        else if (Key==__T("COMMENTS"))               Fill(StreamKind_Common,   0, "Comment", Value);
        else if (Key==__T("CONDUCTOR"))              Fill(StreamKind_Common,   0, "Conductor", Value);
        else if (Key==__T("CONTACT"))                Fill(StreamKind_Common,   0, "Publisher", Value);
        else if (Key==__T("COPYRIGHT"))              Fill(StreamKind_Common,   0, "Copyright", Value);
        else if (Key==__T("DATE"))                   Fill(StreamKind_Common,   0, "Recorded_Date", Value, true);
        else if (Key==__T("DESCRIPTION"))            Fill(StreamKind_Common,   0, "Description", Value);
        else if (Key==__T("DISC"))                   Fill(StreamKind_Common,   0, "Part", Value, true);
        else if (Key==__T("DISCID"))                 {}
        else if (Key==__T("DISCNUMBER"))             Fill(StreamKind_Common,   0, "Part", Value, true);
        else if (Key==__T("DISCTOTAL"))              Fill(StreamKind_Common,   0, "Part/Position_Total", Value);
        else if (Key==__T("ENCODEDBY"))              Fill(StreamKind_Common,   0, "EncodedBy", Value);
        else if (Key==__T("ENCODED-BY"))             Fill(StreamKind_Common,   0, "EncodedBy", Value);
        else if (Key==__T("ENCODER"))                Fill(StreamKind_Common,   0, "Encoded_Application", Value);
        else if (Key==__T("ENCODED_USING"))          Fill(StreamKind_Common,   0, "Encoded_Application", Value);
        else if (Key==__T("ENCODER_URL"))            Fill(StreamKind_Common,   0, "Encoded_Application/Url", Value);
        else if (Key==__T("ENSEMBLE"))               Accompaniments.push_back(Value);
        else if (Key==__T("GENRE"))                  Fill(StreamKind_Common,   0, "Genre", Value);
        else if (Key==__T("FIRST_PLAYED_TIMESTAMP")) Fill(StreamKind_Common,   0, "Played_First_Date", Ztring().Date_From_Milliseconds_1601(Value.To_int64u()/10000));
        else if (Key==__T("ISRC"))                   Fill(StreamKind_Multiple, 0, "ISRC", Value);
        else if (Key==__T("LABEL"))                  Fill(StreamKind_Common,   0, "Label", Value);
        else if (Key==__T("LANGUAGE"))               {if (Value.find(__T("Director"))==0) Fill(StreamKind_Specific, 0, "Language_More", Value); else if (!Value.SubString(__T("["), __T("]")).empty()) Fill(StreamKind_Specific, 0, "Language", Value.SubString(__T("["), __T("]"))); else Fill(StreamKind_Specific, 0, "Language", Value);}
        else if (Key==__T("LAST_PLAYED_TIMESTAMP"))  Fill(StreamKind_Multiple, 0, "Played_Last_Date", Ztring().Date_From_Milliseconds_1601(Value.To_int64u()/10000));
        else if (Key==__T("LICENCE"))                Fill(StreamKind_Common,   0, "TermsOfUse", Value);
        else if (Key==__T("LICENSE"))                Fill(StreamKind_Common,   0, "TermsOfUse", Value);
        else if (Key==__T("LYRICS"))                 Fill(StreamKind_Common,   0, "Lyrics", Value);
        else if (Key==__T("LWING_GAIN"))             Fill(StreamKind_Multiple, 0, "ReplayGain_Gain", Value.To_float64(), 2);
        else if (Key==__T("LOCATION"))               Fill(StreamKind_Common,   0, "Recorded/Location", Value);
        else if (Key==__T("MUSICBRAINZ_ALBUMID"))    {}
        else if (Key==__T("MUSICBRAINZ_ALBUMARTISTID")) {}
        else if (Key==__T("MUSICBRAINZ_ARTISTID"))   {}
        else if (Key==__T("MUSICBRAINZ_TRACKID"))    {}
        else if (Key==__T("MUSICBRAINZ_SORTNAME"))   Fill(StreamKind_Common,   0, "Performer/Sort", Value);
        else if (Key==__T("MUSICBRAINZ_DISCID"))     {}
        else if (Key==__T("ORGANIZATION"))           Fill(StreamKind_Common,   0, "Producer", Value);
        else if (Key==__T("PERFORMER"))              Performers.push_back(Value);
        else if (Key==__T("PLAY_COUNT"))             Fill(StreamKind_Multiple, 0, "Played_Count", Value.To_int64u());
        else if (Key==__T("RATING"))                 Fill(StreamKind_Multiple, 0, "Rating", Value);
        else if (Key==__T("REPLAYGAIN_ALBUM_GAIN"))  Fill(StreamKind_Common,   0, "Album_ReplayGain_Gain", Value.To_float64(), 2);
        else if (Key==__T("REPLAYGAIN_ALBUM_PEAK"))  Fill(StreamKind_Common,   0, "Album_ReplayGain_Peak", Value.To_float64(), 6);
        else if (Key==__T("REPLAYGAIN_REFERENCE_LOUDNESS")) {}
        else if (Key==__T("REPLAYGAIN_TRACK_GAIN"))  Fill(StreamKind_Specific, 0, "ReplayGain_Gain",       Value.To_float64(), 2);
        else if (Key==__T("REPLAYGAIN_TRACK_PEAK"))  Fill(StreamKind_Specific, 0, "ReplayGain_Peak",       Value.To_float64(), 6);
        else if (Key==__T("TITLE"))                  Fill(StreamKind_Common,   0, "Title", Value);
        else if (Key==__T("TOTALTRACKS"))            Fill(StreamKind_Common,   0, "Track/Position_Total", Value);
        else if (Key==__T("TOTALDISCS"))             Fill(StreamKind_Common,   0, "Part/Position_Total", Value);
        else if (Key==__T("TRACK_COMMENT"))          Fill(StreamKind_Multiple, 0, "Comment", Value);
        else if (Key==__T("TRACKNUMBER"))            Fill(StreamKind_Multiple, 0, "Track/Position", Value);
        else if (Key==__T("TRACKTOTAL"))             Fill(StreamKind_Multiple, 0, "Track/Position_Total", Value);
        else if (Key==__T("VERSION"))                Fill(StreamKind_Common,   0, "Track/More", Value);
        else if (Key==__T("BPM"))                    Fill(StreamKind_Common,   0, "BPM", Value);
        else if (Key==__T("WAVEFORMATEXTENSIBLE_CHANNEL_MASK"))
        {
            //This is an hexadecimal value
            if (Value.size()>2 && Value[0]==__T('0') && Value[1]==__T('x'))
            {
                int16u ValueI=0;
                for (size_t Pos=2; Pos<Value.size(); Pos++)
                {
                    ValueI*=16;
                    if (Value[Pos]>=__T('0') && Value[Pos]<=__T('9'))
                        ValueI+=Value[Pos]-__T('0');
                    else if (Value[Pos]>=__T('A') && Value[Pos]<=__T('F'))
                        ValueI+=10+Value[Pos]-__T('A');
                    else if (Value[Pos]>=__T('a') && Value[Pos]<=__T('f'))
                        ValueI+=10+Value[Pos]-__T('a');
                    else
                        break;
                }
                Fill(Stream_Audio, 0, Audio_ChannelPositions, ExtensibleWave_ChannelMask(ValueI));
                Fill(Stream_Audio, 0, Audio_ChannelPositions_String2, ExtensibleWave_ChannelMask2(ValueI));
            }
        }
        else if (Key==__T("YEAR"))                   {if (Value!=Retrieve(StreamKind_Common,   0, "Recorded_Date")) Fill(StreamKind_Common,   0, "Recorded_Date", Value);}
        else if (Key.find(__T("COVERART"))==0)
        {
                 if (Key==__T("COVERARTCOUNT"))
                ;
            else if (Key.find(__T("COVERARTMIME"))==0)
                Fill(Stream_General, 0, General_Cover_Mime, Value);
            else if (Key.find(__T("COVERARTFILELINK"))==0)
                Fill(Stream_General, 0, General_Cover_Data, __T("file://")+Value);
            else if (Key.find(__T("COVERARTTYPE"))==0)
                Fill(Stream_General, 0, General_Cover_Type, Id3v2_PictureType(Value.To_int8u()));
        }
        else if (Key.find(__T("CHAPTER"))==0)
        {
            if (Count_Get(Stream_Menu)==0)
            {
                Stream_Prepare(Stream_Menu);
                Fill(Stream_Menu, StreamPos_Last, Menu_Chapters_Pos_Begin, Count_Get(Stream_Menu, StreamPos_Last), 10, true);
            }
            if (Key.find(__T("NAME"))==Error)
            {
                Chapter_Pos=Key.SubString(__T("CHAPTER"), __T(""));
                Chapter_Time=Value;
            }
            else
            {
                Value.FindAndReplace(__T("\n"), __T(""), Count_Get(Stream_Text)-1); //Some chapters names have extra characters, not needed
                Value.FindAndReplace(__T("\r"), __T(""), Count_Get(Stream_Text)-1); //Some chapters names have extra characters, not needed
                Value.FindAndReplace(__T(" "),  __T(""), Count_Get(Stream_Text)-1); //Some chapters names have extra characters, not needed
                Fill(Stream_Menu, 0, Chapter_Time.To_UTF8().c_str(), Value);
            }
            Fill(Stream_Menu, StreamPos_Last, Menu_Chapters_Pos_End, Count_Get(Stream_Menu, StreamPos_Last), 10, true);
        }
        else                                Fill(Stream_General, 0, comment.SubString(__T(""), __T("=")).To_Local().c_str(), Value);
    FILLING_END();

    if (user_comment_list_length==0)
        Finish("VorbisCom");
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_VORBISCOM_YES

