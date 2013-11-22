/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about DPG (Nintendo DS) files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_DpgH
#define MediaInfo_File_DpgH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Dpg
//***************************************************************************

class File_Dpg : public File__Analyze
{
public :
    //Constructor/Destructor
    File_Dpg();
    ~File_Dpg();

private :
    //Buffer - File header
    bool FileHeader_Begin();
    void FileHeader_Parse();

    //Buffer
    void Read_Buffer_Unsynched();
    void Read_Buffer_Continue();

    //Elements
    void Audio();
    void Video();

    //Data
    File__Analyze* Parser;

    //Temp
    int32u Audio_Offset;
    int32u Audio_Size;
    int32u Video_Offset;
    int32u Video_Size;
};

} //NameSpace

#endif
