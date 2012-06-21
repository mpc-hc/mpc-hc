// File__Base - Base for other files
// Copyright (C) 2002-2011 MediaArea.net SARL, Info@MediaArea.net
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
#include "MediaInfo/File__Base.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#include "ZenLib/File.h"
#include <cstring>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
extern MediaInfo_Config Config;
//---------------------------------------------------------------------------

//***************************************************************************
// Gestion de la classe
//***************************************************************************

//---------------------------------------------------------------------------
//Constructeurs
File__Base::File__Base ()
{
    //Init pointers
    #if MEDIAINFO_TRACE
        Details=NULL;
    #endif //MEDIAINFO_TRACE
    Stream=NULL;
    Stream_More=NULL;
    Stream_MustBeDeleted=false;

    //File
    File_Size=(int64u)-1;
    File_Offset=0;
    File_Offset_FirstSynched=(int64u)-1;
    File_GoTo=(int64u)-1;

    //Optimization init
    StreamKind_Last=Stream_Max;
    StreamPos_Last=Error;

    //Config
    Config=NULL;
}

//---------------------------------------------------------------------------
//Constructeurs
File__Base::~File__Base ()
{
    if (Stream_MustBeDeleted)
    {
        delete Stream; //Stream=NULL;
        delete Stream_More; //Stream_More=NULL;
    }
}

//---------------------------------------------------------------------------
//Base
#if MEDIAINFO_TRACE
void File__Base::Init (MediaInfo_Config_MediaInfo * Config_, Ztring* Details_, std::vector<std::vector<ZtringList> > * Stream_, std::vector<std::vector<ZtringListList> > * Stream_More_)
#else //MEDIAINFO_TRACE
void File__Base::Init (MediaInfo_Config_MediaInfo * Config_, std::vector<std::vector<ZtringList> > * Stream_, std::vector<std::vector<ZtringListList> > * Stream_More_)
#endif //MEDIAINFO_TRACE
{
    if (Config)
        return; //Already done
        
    if (Stream_)
    {
        Stream=Stream_;
        Stream_More=Stream_More_;
        Stream_MustBeDeleted=false;
    }
    else
    {
        Stream=new std::vector<std::vector<ZtringList> >;
        Stream->resize(Stream_Max);
        Stream_More=new std::vector<std::vector<ZtringListList> >;
        Stream_More->resize(Stream_Max);
        Stream_MustBeDeleted=true;
    }

    Config=Config_;
    #if MEDIAINFO_TRACE
        Details=Details_;
    #endif //MEDIAINFO_TRACE
}

//***************************************************************************
// Fonctions
//***************************************************************************

//---------------------------------------------------------------------------
size_t File__Base::Count_Get (stream_t StreamKind, size_t Pos) const
{
    //Integrity
    if (StreamKind>=Stream_Max)
        return 0;

    //Count of streams
    if (!Stream)
        return 0;
    if (Pos==Error)
        return (*Stream)[StreamKind].size();

    //Integrity
    if (Pos>=(*Stream)[StreamKind].size())
        return 0;

    //Count of piece of information in a stream
    return MediaInfoLib::Config.Info_Get(StreamKind).size()+(*Stream_More)[StreamKind][Pos].size();
}

//---------------------------------------------------------------------------
const Ztring &File__Base::Get (stream_t StreamKind, size_t StreamNumber, size_t Parameter, info_t KindOfInfo)
{
    //Check integrity
    if (StreamKind>=Stream_Max || StreamNumber>=(*Stream)[StreamKind].size() || Parameter>=MediaInfoLib::Config.Info_Get(StreamKind).size()+(*Stream_More)[StreamKind][StreamNumber].size() || KindOfInfo>=Info_Max)
        return MediaInfoLib::Config.EmptyString_Get(); //Parameter is unknown

    else if (Parameter<MediaInfoLib::Config.Info_Get(StreamKind).size())
    {
        //Optimization : KindOfInfo>Info_Text is in static lists
        if (KindOfInfo!=Info_Text)
            return MediaInfoLib::Config.Info_Get(StreamKind)[Parameter][KindOfInfo]; //look for static information only
        else if (Parameter<(*Stream)[StreamKind][StreamNumber].size())
            return (*Stream)[StreamKind][StreamNumber][Parameter];
        else
            return MediaInfoLib::Config.EmptyString_Get(); //This parameter is known, but not filled
    }
    else
    {
        if (((size_t)(Parameter-MediaInfoLib::Config.Info_Get(StreamKind).size()))<(*Stream_More)[StreamKind][StreamNumber].size() && ((size_t)KindOfInfo)<(*Stream_More)[StreamKind][StreamNumber][Parameter-MediaInfoLib::Config.Info_Get(StreamKind).size()].size())
            return (*Stream_More)[StreamKind][StreamNumber][Parameter-MediaInfoLib::Config.Info_Get(StreamKind).size()][KindOfInfo];
        else
            return MediaInfoLib::Config.EmptyString_Get(); //Not filled
    }
}

//---------------------------------------------------------------------------
const Ztring &File__Base::Get (stream_t StreamKind, size_t StreamPos, const Ztring &Parameter, info_t KindOfInfo, info_t KindOfSearch)
{
    size_t ParameterI=0;

    //Check integrity
    if (StreamKind>=Stream_Max || StreamPos>=(*Stream)[StreamKind].size() || KindOfInfo>=Info_Max)
        return MediaInfoLib::Config.EmptyString_Get();
    if ((ParameterI=MediaInfoLib::Config.Info_Get(StreamKind).Find(Parameter, KindOfSearch))==Error)
    {
        ParameterI=(*Stream_More)[StreamKind][StreamPos].Find(Parameter, KindOfSearch);
        if (ParameterI==Error)
            return MediaInfoLib::Config.EmptyString_Get(); //Parameter is unknown
        return (*Stream_More)[StreamKind][StreamPos][ParameterI](KindOfInfo);
    }

    return Get(StreamKind, StreamPos, ParameterI, KindOfInfo);
}

//---------------------------------------------------------------------------
int File__Base::Set (stream_t StreamKind, size_t StreamNumber, size_t Parameter, const Ztring &ToSet, const Ztring &OldValue)
{
    //Integrity
    if (Count_Get(StreamKind)<=StreamNumber)
        return 0;

    return Set(StreamKind, StreamNumber, Get(StreamKind, StreamNumber, Parameter, Info_Name), ToSet, OldValue);
}

//---------------------------------------------------------------------------
int File__Base::Set (stream_t StreamKind, size_t StreamNumber, const Ztring &Parameter, const Ztring &ToSet, const Ztring &OldValue)
{
    //Integrity
    if (Count_Get(StreamKind)<=StreamNumber)
        return 0;

    return Write(StreamKind, StreamNumber, Parameter, ToSet, OldValue);
}

//---------------------------------------------------------------------------
void File__Base::Language_Set()
{
}

//***************************************************************************
// Divers
//***************************************************************************

void File__Base::Clear()
{
    for (size_t StreamKind=0; StreamKind<Stream_Max; StreamKind++)
    {
        (*Stream)[StreamKind].clear();
        (*Stream_More)[StreamKind].clear();
    }
}

} //NameSpace


