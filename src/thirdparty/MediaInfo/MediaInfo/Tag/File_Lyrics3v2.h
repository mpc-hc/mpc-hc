// File_Lyrics3v2 - Info for Lyrics3v2 tagged files
// Copyright (C) 2007-2011 MediaArea.net SARL, Info@MediaArea.net
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
// Information about Lyrics3v2 tagged files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_Lyrics3v2H
#define MediaInfo_File_Lyrics3v2H
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Lyrics3v2
//***************************************************************************

class File_Lyrics3v2 : public File__Analyze
{
public :
    //In
    int64u TotalSize;

    //Constructor/Destructor
    File_Lyrics3v2();

private :
    //Buffer - File header
    void FileHeader_Parse();

    //Buffer - Per element
    void Header_Parse ();
    void Data_Parse ();

    //Elements
    void Header();
    void Footer();
    void AUT() {Skip_Local(Element_Size, "Value");}
    void CRC() {Skip_Local(Element_Size, "Value");}
    void EAL();
    void EAR();
    void ETT();
    void IMG() {Skip_Local(Element_Size, "Value");}
    void IND();
    void INF();
    void LYR();
};

} //NameSpace

#endif
