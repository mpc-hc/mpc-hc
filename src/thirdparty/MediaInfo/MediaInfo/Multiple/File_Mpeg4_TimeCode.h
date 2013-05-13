/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_Mpeg4_TimeCodeH
#define MediaInfo_File_Mpeg4_TimeCodeH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Mpeg4_TimeCode
//***************************************************************************

class File_Mpeg4_TimeCode : public File__Analyze
{
public :
    //In
    int8u   NumberOfFrames;
    bool    DropFrame;
    bool    NegativeTimes;

    //Out
    int64s  Pos;

    //Constructor/Destructor
    File_Mpeg4_TimeCode();

protected :
    //Streams management
    void Streams_Fill();

    //Buffer - Global
    void Read_Buffer_Continue();
};

} //NameSpace

#endif
