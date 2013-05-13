/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about EXR files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_ExrH
#define MediaInfo_File_ExrH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Exr
//***************************************************************************

class File_Exr : public File__Analyze
{
public :
    //Constructor/Destructor
    File_Exr();

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
    bool Header_Begin();
    void Header_Parse();
    void Data_Parse();

    //Elements
    void Header();
    void ImageData();
    void comments();
    void compression();
    void dataWindow();
    void displayWindow();
    void pixelAspectRatio();

    //Temp
    std::string         name;
    std::string         type;
    size_t              name_End;
    size_t              type_End;
    int64u              ImageData_End;
};

} //NameSpace

#endif
