/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about h.263 files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_H263H
#define MediaInfo_H263H
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_H263
//***************************************************************************

class File_H263 : public File__Analyze
{
public :
    //In
    int64u Frame_Count_Valid;
    bool   FrameIsAlwaysComplete;

    //Constructor/Destructor
    File_H263();
    ~File_H263();

private :
    //Streams management
    void Streams_Accept();
    void Streams_Update();
    void Streams_Fill();
    void Streams_Finish();

    //Buffer - Synchro
    bool Synchronize();
    bool Synched_Test();
    void Synched_Init();

    //Buffer - Global
    void Read_Buffer_Unsynched();

    //Buffer - Per element
    void Header_Parse();
    bool Header_Parser_Fill_Size();
    void Data_Parse();

    //Elements

    //Temp
    int8u Temporal_Reference;
    int8u Source_Format;
    int8u PAR_W;
    int8u PAR_H;
    bool  Temporal_Reference_IsValid;
};

} //NameSpace

#endif
