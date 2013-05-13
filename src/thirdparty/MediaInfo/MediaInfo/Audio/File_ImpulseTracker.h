/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Impulse Tracker files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_ImpulseTrackerH
#define MediaInfo_File_ImpulseTrackerH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_ImpulseTracker
//***************************************************************************

class File_ImpulseTracker : public File__Analyze
{
protected :
    //Buffer - File header
    bool FileHeader_Begin();

    //Buffer - Global
    void Read_Buffer_Continue ();
};

} //NameSpace

#endif
