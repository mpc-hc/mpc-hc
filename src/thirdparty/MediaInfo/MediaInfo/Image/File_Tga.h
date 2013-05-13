/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

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
