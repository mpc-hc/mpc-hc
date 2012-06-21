// File_Ico - Info for Icon files
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
// Information about Icon files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_IcoH
#define MediaInfo_File_IcoH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Ico
//***************************************************************************

class File_Ico : public File__Analyze
{
public:
    //Constructor/Destructor
    File_Ico();

private :
    //Streams management
    void Streams_Fill();

    //Buffer - File header
    bool FileHeader_Begin();
    void FileHeader_Parse ();

    //Buffer - Per element
    void Header_Parse ();
    void Data_Parse ();

    //Temp
    int64u IcoDataSize;
    int16u Type;
    int16u Count;
    struct stream
    {
        int32u  Size;
        int32u  Offset;
        int16u  BitsPerPixel;
        int8u   Width;
        int8u   Height;
    };
    std::vector<stream> Streams;
};

} //NameSpace

#endif

