// File_Id3 - Info for ID3 tagged files
// Copyright (C) 2005-2011 MediaArea.net SARL, Info@MediaArea.net
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
#if defined(MEDIAINFO_ID3_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Tag/File_Id3.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Id3::Read_Buffer_Continue()
{
    //Buffer size
    if (Buffer_Size<128)
        return;

    int32u Magic;
    Peek_B4(Magic);
    Ztring TitleAddition;
    Ztring ArtistAddition;
    Ztring AlbumAddition;
    Ztring GenreAddition;
    if (Magic==0x5441472B)
    {
        if (Buffer_Size<227+128)
            return;

        Skip_C4   (                                                 "ID");
        Get_Local (60, TitleAddition,                               "Title");
        Get_Local (60, ArtistAddition,                              "Artist");
        Get_Local (60, AlbumAddition,                               "Album");
        Skip_B1   (                                                 "Speed");
        Get_Local (30, GenreAddition,                               "Genre");
        Skip_Local(6,                                               "Start time"); //mmm:ss
        Skip_Local(6,                                               "End time"); //mmm:ss

        TitleAddition.TrimRight();
        ArtistAddition.TrimRight();
        AlbumAddition.TrimRight();
        GenreAddition.TrimRight();
    }

    //Parsing
    Ztring Title, Artist, Album, Year, Comment;
    int8u Track=0, Genre;
    Skip_C3   (                                                 "ID");
    Get_Local (30, Title,                                       "Title");
    Get_Local (30, Artist,                                      "Artist");
    Get_Local (30, Album,                                       "Album");
    Get_Local ( 4, Year,                                        "Year");
    Get_Local (30, Comment,                                     "Comment");
    if (Comment.size()<29) //Id3v1.1 specifications : Track number addition
    {
        Element_Offset-=2;
        int8u Zero;
        Peek_B1(Zero);
        if (Zero==0)
        {
            Skip_B1(                                            "Zero");
            Get_B1 (Track,                                      "Track");
        }
        else
            Element_Offset+=2;
    }
    Get_B1 (Genre,                                              "Genre");

    FILLING_BEGIN();
        if (TitleAddition.empty())
            Title.TrimRight();
        if (ArtistAddition.empty())
            Artist.TrimRight();
        if (AlbumAddition.empty())
            Album.TrimRight();
        Year.TrimRight();
        Comment.TrimRight();

        Accept("Id3");

        Stream_Prepare(Stream_General);
        Fill(Stream_General, 0, General_Album, Album+AlbumAddition);
        Fill(Stream_General, 0, General_Track, Title+TitleAddition);
        Fill(Stream_General, 0, General_Performer, Artist+ArtistAddition);
        Fill(Stream_General, 0, General_Comment, Comment);
        Fill(Stream_General, 0, General_Recorded_Date, Year);
        if (GenreAddition.empty())
            Fill(Stream_General, 0, General_Genre, GenreAddition);
        if (Genre && Genre!=(int8u)-1)
            Fill(Stream_General, 0, General_Genre, Genre);
        if (Track)
            Fill(Stream_General, 0, General_Track_Position, Track);

        Finish("Id3");
    FILLING_END();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_ID3_YES

