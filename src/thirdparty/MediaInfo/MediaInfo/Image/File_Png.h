/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about PNG files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_PngH
#define MediaInfo_File_PngH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Png
//***************************************************************************

class File_Png : public File__Analyze
{
public :
    //Constructor/Destructor
    File_Png();

private :
    //Streams management
    void Streams_Accept();

    //Buffer - File header
    bool FileHeader_Begin();

    //Buffer - Demux
    #if MEDIAINFO_DEMUX
    bool Demux_UnpacketizeContainer_Test() {return Demux_UnpacketizeContainer_Test_OneFramePerFile();}
    #endif //MEDIAINFO_DEMUX

    //Buffer - Global
    void Read_Buffer_Unsynched();
    #if MEDIAINFO_SEEK
    size_t Read_Buffer_Seek (size_t Method, int64u Value, int64u ID) {return Read_Buffer_Seek_OneFramePerFile(Method, Value, ID);}
    #endif //MEDIAINFO_SEEK

    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    //Elements
    void Signature();
    void IDAT() {Skip_XX(Element_Size, "Data");}
    void IEND();
    void IHDR();
    void PLTE() {Skip_XX(Element_Size, "Data");}

    //Temp
    bool    Signature_Parsed;
};

} //NameSpace

#endif
