/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

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

