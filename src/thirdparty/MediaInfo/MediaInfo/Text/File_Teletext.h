/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Teletext streams
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_TeletextH
#define MediaInfo_File_TeletextH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Teletext
//***************************************************************************

class File_Teletext : public File__Analyze
{
public :
    File_Teletext();
    ~File_Teletext();

private :
    //Streams management
    void Streams_Fill();
    void Streams_Finish();

    //Buffer - Global
    void Read_Buffer_Continue();
};

} //NameSpace

#endif
