/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about DPX files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_DpxH
#define MediaInfo_File_DpxH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Dpx
//***************************************************************************

class File_Dpx : public File__Analyze
{
public :
    //Constructor/Destructor
    File_Dpx();

private :
    //Streams management
    void Streams_Accept();

    //Buffer - Demux
    #if MEDIAINFO_DEMUX
    bool Demux_UnpacketizeContainer_Test() {return Demux_UnpacketizeContainer_Test_OneFramePerFile();}
    #endif //MEDIAINFO_DEMUX

    //Buffer - File header
    bool FileHeader_Begin();

    //Buffer - Global
    void Read_Buffer_Unsynched() {Read_Buffer_Unsynched_OneFramePerFile();}
    #if MEDIAINFO_SEEK
    size_t Read_Buffer_Seek (size_t Method, int64u Value, int64u ID) {return Read_Buffer_Seek_OneFramePerFile(Method, Value, ID);}
    #endif //MEDIAINFO_SEEK

    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    //Elements
    void GenericSectionHeader_v1();
    void GenericSectionHeader_v2();
    void GenericSectionHeader_v1_ImageElement();
    void GenericSectionHeader_v2_ImageElement();
    void IndustrySpecificHeader_v1();
    void IndustrySpecificHeader_v2();
    void UserDefinedHeader_v1();
    void UserDefinedHeader_v2();
    void Padding();
    void ImageData();

    //Temp
    std::vector<int32u> Sizes;
    size_t              Sizes_Pos;
    int8u               Version;
    bool                LittleEndian;

    //Helpers
    void Get_X2 (int16u &Info, const char* Name);
    void Get_X4 (int32u &Info, const char* Name);
};

} //NameSpace

#endif
