// File_Dummy - Fill with Name of tags
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
// Fill with Name of tags
// Used to give an example to the GUI of what MediaInfo can do
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_DummyH
#define MediaInfo_File_DummyH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Dummy
//***************************************************************************

class File_Dummy : public File__Analyze
{
public :
    Ztring KindOfDummy;

protected :
    //Buffer - File header
    void FileHeader_Parse ();

private :
    void Fill_Dummy_General();
    void Fill_Dummy_Video();
    void Fill_Dummy_Audio();
    void Fill_Dummy_Text();
    void Fill_Dummy_Chapters();
};

} //NameSpace

#endif
