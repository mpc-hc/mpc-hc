// File_VorbisCom - Info for VorbisComments tagged files
// Copyright (C) 2007-2010 MediaArea.net SARL, Info@MediaArea.net
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
        if (StreamKind_Specific!=Stream_Audio && vendor_string.find(_T("Xiph.Org libVorbis"))==0)
            vendor_string.clear(); //string was set "by default"
        Ztring Library_Name, Library_Version, Library_Date;
        Ztring vendor_string_Without=vendor_string; vendor_string_Without.FindAndReplace(_T(";"), _T(""), 0, Ztring_Recursive);
        Library_Version=MediaInfoLib::Config.Library_Get(InfoLibrary_Format_VorbisCom, vendor_string_Without, InfoLibrary_Version);
        Library_Date=MediaInfoLib::Config.Library_Get(InfoLibrary_Format_VorbisCom, vendor_string_Without, InfoLibrary_Date);
        if (Library_Version.empty())
        {
            if (vendor_string.find(_T(" I "))!=std::string::npos)
            {
                Library_Name=vendor_string.SubString(_T(""), _T(" I "));
                Library_Date=vendor_string.SubString(_T(" I "), _T(""));
                if (Library_Date.size()>9)
                {
                    Library_Version=Library_Date.substr(9, std::string::npos);
                    if (Library_Version.find(_T("("))==std::string::npos)
                    {
                        Library_Version.FindAndReplace(_T(" "), _T("."), 0, Ztring_Recursive);
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
                    std::string::size_type Pos=Library_Name.rfind(_T(' '));
                    if (Pos<Library_Name.size()-2 && Library_Name[Pos+1]>=_T('0') && Library_Name[Pos+1]<=_T('9'))
                    {
                        Library_Version=Library_Name.substr(Pos+1, std::string::npos);
                        Library_Name.resize(Pos);
                    }
                }
            }
            else if (vendor_string.find(_T("aoTuV "))!=std::string::npos)
            {
                Library_Name=_T("aoTuV");
                Library_Version=vendor_string.SubString(_T("aoTuV "), _T("["));
                Library_Date=vendor_string.SubString(_T("["), _T("]"));
            }
            else if (vendor_string.find(_T("Lancer "))!=std::string::npos)
            {
                Library_Name=_T("Lancer");
                Library_Date=vendor_string.SubString(_T("["), _T("]"));
            }
            if (Library_Version.empty())
                Library_Version=Library_Date;
            if (Library_Date.size()==8)
            {
                Library_Date.insert(Library_Date.begin()+6, _T('-'));
                Library_Date.insert(Library_Date.begin()+4, _T('-'));
                Library_Date.insert(0, _T("UTC "));
            }
        }
        if (vendor_string.find(_T("libFLAC"))!=std::string::npos) Library_Name="libFLAC";
        if (vendor_string.find(_T("libVorbis I"))!=std::string::npos) Library_Name="libVorbis";
        if (vendor_string.find(_T("libTheora I"))!=std::string::npos) Library_Name="libTheora";
        if (vendor_string.find(_T("AO; aoTuV"))==0) Library_Name="aoTuV";
        if (vendor_string.find(_T("BS; Lancer"))==0) Library_Name="Lancer";

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
        Ztring Key=comment.SubString(_T(""), _T("="));
        Key.MakeUpperCase();
        Ztring Value=comment.SubString(_T("="), _T(""));

             if (Key==_T("ADDED_TIMESTAMP"))        Fill(StreamKind_Common,   0, "Added_Date", Ztring().Date_From_Milliseconds_1601(Value.To_int64u()/1000));
        else if (Key==_T("ALBUM ARTIST"))           {if (Value!=Retrieve(StreamKind_Common,   0, "Performer")) Fill(StreamKind_Common,   0, "Performer", Value);}
        else if (Key==_T("ALBUM"))                  Fill(StreamKind_Common,   0, "Album", Value);
        else if (Key==_T("ALBUM_COMMENT"))          Fill(StreamKind_Common,   0, "Comment", Value);
        else if (Key==_T("ALBUMARTIST"))            {if (Value!=Retrieve(StreamKind_Common,   0, "Performer")) Fill(StreamKind_Common,   0, "Performer", Value);}
        else if (Key==_T("ARTIST"))                 {if (Value!=Retrieve(StreamKind_Common,   0, "Performer")) Fill(StreamKind_Common,   0, "Performer", Value);}
        else if (Key==_T("AUTHOR"))                 Fill(StreamKind_Common,   0, "WrittenBy", Value);
        else if (Key==_T("BUYCDURL"))               {}
        else if (Key==_T("CLASS"))                  Fill(StreamKind_Common,   0, "ContentType", Value);
        else if (Key==_T("COMPOSER"))               Fill(StreamKind_Common,   0, "Composer", Value);
        else if (Key==_T("COMMENT"))                Fill(StreamKind_Common,   0, "Comment", Value);
        else if (Key==_T("COMMENTS"))               Fill(StreamKind_Common,   0, "Comment", Value);
        else if (Key==_T("CONDUCTOR"))              Fill(StreamKind_Common,   0, "Conductor", Value);
        else if (Key==_T("CONTACT"))                Fill(StreamKind_Common,   0, "Publisher", Value);
        else if (Key==_T("COPYRIGHT"))              Fill(StreamKind_Common,   0, "Copyright", Value);
        else if (Key==_T("DATE"))                   Fill(StreamKind_Common,   0, "Recorded_Date", Value, true);
        else if (Key==_T("DESCRIPTION"))            Fill(StreamKind_Common,   0, "Description", Value);
        else if (Key==_T("DISC"))                   Fill(StreamKind_Common,   0, "Part", Value, true);
        else if (Key==_T("DISCNUMBER"))             Fill(StreamKind_Common,   0, "Part", Value, true);
        else if (Key==_T("ENCODEDBY"))              Fill(StreamKind_Common,   0, "EncodedBy", Value);
        else if (Key==_T("ENCODER"))                Fill(StreamKind_Common,   0, "Encoded_Application", Value);
        else if (Key==_T("ENCODED_USING"))          Fill(StreamKind_Common,   0, "Encoded_Application", Value);
        else if (Key==_T("ENCODER_URL"))            Fill(StreamKind_Common,   0, "Encoded_Application/Url", Value);
        else if (Key==_T("ENSEMBLE"))               {if (Value!=Retrieve(StreamKind_Common,   0, "Performer")) Fill(StreamKind_Common,   0, "Accompaniment", Value);}
        else if (Key==_T("GENRE"))                  Fill(StreamKind_Common,   0, "Genre", Value);
        else if (Key==_T("FIRST_PLAYED_TIMESTAMP")) Fill(StreamKind_Common,   0, "Played_First_Date", Ztring().Date_From_Milliseconds_1601(Value.To_int64u()/10000));
        else if (Key==_T("ISRC"))                   Fill(StreamKind_Multiple, 0, "ISRC", Value);
        else if (Key==_T("LABEL"))                  Fill(StreamKind_Common,   0, "Label", Value);
        else if (Key==_T("LANGUAGE"))               {if (Value.find(_T("Director"))==0) Fill(StreamKind_Specific, 0, "Language_More", Value); else if (!Value.SubString(_T("["), _T("]")).empty()) Fill(StreamKind_Specific, 0, "Language", Value.SubString(_T("["), _T("]"))); else Fill(StreamKind_Specific, 0, "Language", Value);}
        else if (Key==_T("LAST_PLAYED_TIMESTAMP"))  Fill(StreamKind_Multiple, 0, "Played_Last_Date", Ztring().Date_From_Milliseconds_1601(Value.To_int64u()/10000));
        else if (Key==_T("LICENCE"))                Fill(StreamKind_Common,   0, "TermsOfUse", Value);
        else if (Key==_T("LYRICS"))                 Fill(StreamKind_Common,   0, "Lyrics", Value);
        else if (Key==_T("LWING_GAIN"))             Fill(StreamKind_Multiple, 0, "ReplayGain_Gain", Value.To_float64(), 2);
        else if (Key==_T("LOCATION"))               Fill(StreamKind_Common,   0, "Recorded/Location", Value);
        else if (Key==_T("ORGANIZATION"))           Fill(StreamKind_Common,   0, "Producer", Value);
        else if (Key==_T("PERFORMER"))              Fill(StreamKind_Common,   0, "Performer", Value);
        else if (Key==_T("PLAY_COUNT"))             Fill(StreamKind_Multiple, 0, "Played_Count", Value.To_int64u());
        else if (Key==_T("RATING"))                 Fill(StreamKind_Multiple, 0, "Rating", Value);
        else if (Key==_T("REPLAYGAIN_ALBUM_GAIN"))  Fill(StreamKind_Common,   0, "Album_ReplayGain_Gain", Value.To_float64(), 2);
        else if (Key==_T("REPLAYGAIN_ALBUM_PEAK"))  Fill(StreamKind_Common,   0, "Album_ReplayGain_Peak", Value.To_float64(), 6);
        else if (Key==_T("REPLAYGAIN_TRACK_GAIN"))  Fill(StreamKind_Specific, 0, "ReplayGain_Gain",       Value.To_float64(), 2);
        else if (Key==_T("REPLAYGAIN_TRACK_PEAK"))  Fill(StreamKind_Specific, 0, "ReplayGain_Peak",       Value.To_float64(), 6);
        else if (Key==_T("TITLE"))                  Fill(StreamKind_Common,   0, "Title", Value);
        else if (Key==_T("TOTALTRACKS"))            Fill(StreamKind_Common,   0, "Track/Position_Total", Value);
        else if (Key==_T("TRACK_COMMENT"))          Fill(StreamKind_Multiple, 0, "Comment", Value);
        else if (Key==_T("TRACKNUMBER"))            Fill(StreamKind_Multiple, 0, "Track/Position", Value);
        else if (Key==_T("VERSION"))                Fill(StreamKind_Common,   0, "Track/More", Value);
        else if (Key==_T("BPM"))                    Fill(StreamKind_Common,   0, "BPM", Value);
        else if (Key==_T("WAVEFORMATEXTENSIBLE_CHANNEL_MASK"))
        {
            //This is an hexadecimal value
            if (Value.size()>2 && Value[0]==_T('0') && Value[1]==_T('x'))
            {
                int16u ValueI=0;
                for (size_t Pos=2; Pos<Value.size(); Pos++)
                {
                    ValueI*=16;
                    if (Value[Pos]>=_T('0') && Value[Pos]<=_T('9'))
                        ValueI+=Value[Pos]-_T('0');
                    else if (Value[Pos]>=_T('A') && Value[Pos]<=_T('F'))
                        ValueI+=10+Value[Pos]-_T('A');
                    else if (Value[Pos]>=_T('a') && Value[Pos]<=_T('f'))
                        ValueI+=10+Value[Pos]-_T('a');
                    else
                        break;
                }
                Fill(Stream_Audio, 0, Audio_ChannelPositions, ExtensibleWave_ChannelMask(ValueI));
                Fill(Stream_Audio, 0, Audio_ChannelPositions_String2, ExtensibleWave_ChannelMask2(ValueI));
            }
        }
        else if (Key==_T("YEAR"))                   {if (Value!=Retrieve(StreamKind_Common,   0, "Recorded_Date")) Fill(StreamKind_Common,   0, "Recorded_Date", Value);}
        else if (Key.find(_T("COVERART"))==0)
        {
                 if (Key==_T("COVERARTCOUNT"))
                ;
            else if (Key.find(_T("COVERARTMIME"))==0)
                Fill(Stream_General, 0, General_Cover_Mime, Value);
            else if (Key.find(_T("COVERARTFILELINK"))==0)
                Fill(Stream_General, 0, General_Cover_Data, _T("file://")+Value);
            else if (Key.find(_T("COVERARTTYPE"))==0)
                Fill(Stream_General, 0, General_Cover_Type, Id3v2_PictureType(Value.To_int8u()));
        }
        else if (Key.find(_T("CHAPTER"))==0)
        {
            if (Count_Get(Stream_Menu)==0)
            {
                Stream_Prepare(Stream_Menu);
                Fill(Stream_Menu, StreamPos_Last, Menu_Chapters_Pos_Begin, Count_Get(Stream_Menu, StreamPos_Last), 10, true);
            }
            if (Key.find(_T("NAME"))==Error)
            {
                Chapter_Pos=Key.SubString(_T("CHAPTER"), _T(""));
                Chapter_Time=Value;
            }
            else
            {
                Value.FindAndReplace(_T("\n"), _T(""), Count_Get(Stream_Text)-1); //Some chapters names have extra characters, not needed
                Value.FindAndReplace(_T("\r"), _T(""), Count_Get(Stream_Text)-1); //Some chapters names have extra characters, not needed
                Value.FindAndReplace(_T(" "),  _T(""), Count_Get(Stream_Text)-1); //Some chapters names have extra characters, not needed
                Fill(Stream_Menu, 0, Chapter_Time.To_UTF8().c_str(), Value);
            }
            Fill(Stream_Menu, StreamPos_Last, Menu_Chapters_Pos_End, Count_Get(Stream_Menu, StreamPos_Last), 10, true);
        }
        else                                Fill(Stream_General, 0, comment.SubString(_T(""), _T("=")).To_Local().c_str(), Value);
    FILLING_END();

    if (user_comment_list_length==0)
        Finish("VorbisCom");
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_VORBISCOM_YES

