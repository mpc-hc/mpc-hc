/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about MPEG files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_MpegH
#define MediaInfo_MpegH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File__MultipleParsing
//***************************************************************************

class File__MultipleParsing : public File__Analyze
{
public :
    //Out
    File__Analyze* Parser_Get();

    //Constructor
    File__MultipleParsing();
    ~File__MultipleParsing();

private :
    //Streams management
    void Streams_Finish();

    //Buffer - Global
    void Read_Buffer_Init();
    void Read_Buffer_Unsynched();
    void Read_Buffer_Continue();

    //Temp
    std::vector<File__Analyze*> Parser;
};

} //NameSpace

#endif
