// File_Rar - Info for RAR files
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
//
// Information about RAR files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_RarH
#define MediaInfo_File_RarH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Rar
//***************************************************************************

class File_Rar : public File__Analyze
{
public :
    File_Rar();

protected :
    int state;

    //Buffer - File header
    bool FileHeader_Begin();

    //Buffer - Per element
    bool Header_Begin();
    void Header_Parse();
    void Header_Parse_Flags();
    void Header_Parse_Flags_73();
    void Header_Parse_Flags_74();
    void Header_Parse_Flags_XX();
    void Header_Parse_Content();
    void Header_Parse_Content_73();
    void Header_Parse_Content_74();
    void Header_Parse_Content_XX();
    void Data_Parse();

    //Temp
    int8u  HEAD_TYPE;
    int32u PACK_SIZE;
    int32u HIGH_PACK_SIZE;
    int16u HEAD_FLAGS;
    bool   high_fields;
    bool   usual_or_utf8;
    bool   salt;
    bool   exttime;
    bool   add_size;
};

} //NameSpace

#endif
