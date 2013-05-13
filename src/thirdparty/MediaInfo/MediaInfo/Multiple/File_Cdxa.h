/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about CDXA files
// (like Video-CD...)
// CDXA are read by MS-Windows with CRC bytes
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_CdxaH
#define MediaInfo_File_CdxaH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

class MediaInfo_Internal;

//***************************************************************************
// Class File_Cdxa
//***************************************************************************

class File_Cdxa : public File__Analyze
{
public :
    //Constructor/Destructor
    File_Cdxa();
    ~File_Cdxa();

private :
    //Streams management
    void Streams_Finish ();

    //Buffer - File header
    bool FileHeader_Begin();
    void FileHeader_Parse();

    //Buffer - Synchro
    bool Synchronize();
    bool Synched_Test();

    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    //Temp
    MediaInfo_Internal* MI;
};

} //NameSpace

#endif
