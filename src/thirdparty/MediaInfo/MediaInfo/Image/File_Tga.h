// File_Tga - Info for TGA files
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
// Information about TGA files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_TgaH
#define MediaInfo_File_TgaH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Tga
//***************************************************************************

class File_Tga : public File__Analyze
{
public :
    //Constructor/Destructor
    File_Tga();

private :
    //Streams management
    void Streams_Fill();

    //Buffer - File header
    bool FileHeader_Begin();

    //Buffer - Global
    void Read_Buffer_Continue ();

    //Elements
    void Tga_File_Header();
    void Image_Color_Map_Data();
    void Tga_File_Footer();

    //Temp - File Header
    int16u First_Entry_Index;
    int16u Color_map_Length;
    int16u Image_Width_;
    int16u Image_Height_;
    int8u  ID_Length;
    int8u  Color_Map_Type;
    int8u  Image_Type;
    int8u  Color_map_Entry_Size;
    int8u  Pixel_Depth;
    int8u  Image_Descriptor;

    //Temp - File Footer
    Ztring Image_ID;
    int8u Version;
};

} //NameSpace

#endif
