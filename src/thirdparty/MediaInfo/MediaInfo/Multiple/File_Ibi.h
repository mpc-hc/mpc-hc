// File_Mk - Info for Ibi Video/Audio files
// Copyright (C) 2011-2011 MediaArea.net SARL, Info@MediaArea.net
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
//
// Information about Ibi files
// 
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_IbiH
#define MediaInfo_File_IbiH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include <map>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Classe File_Ibi
//***************************************************************************

class File_Ibi : public File__Analyze
{
public :
    File_Ibi();
    ~File_Ibi();

    //In
    ibi* Ibi;

private :
    //Get information
    const Ztring &Get (stream_t StreamKind, size_t StreamNumber, const Ztring &Parameter, info_t KindOfInfo=Info_Text, info_t KindOfSearch=Info_Name);

    //Streams management
    void Streams_Accept();
    void Streams_Finish();

    //Buffer - Element
    void Header_Parse();
    void Data_Parse();

    //Elements
    void Zero();
    void CRC32();
    void Void();
    void Ebml();
    void Ebml_Version();
    void Ebml_ReadVersion();
    void Ebml_MaxIDLength();
    void Ebml_MaxSizeLength();
    void Ebml_DocType();
    void Ebml_DocTypeVersion();
    void Ebml_DocTypeReadVersion();
    void Stream();
    void Stream_Header();
    void Stream_ByteOffset();
    void Stream_FrameNumber();
    void Stream_Dts();
    void CompressedIndex();

    //Data
    int64u   UInteger_Get();
    int128u  UInteger16_Get();
    void     UInteger_Info();

    //Temp
    Ztring  Get_Temp;
    int64u  ID_Current;
    bool    Ibi_MustDelete;
};

} //NameSpace

#endif

