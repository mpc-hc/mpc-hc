/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about GXF files
// SMPTE 360M - General Exchange Format
// SMPTE RDD 14-2007 - General Exchange Format-2
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_Gxf_TimeCodeH
#define MediaInfo_File_Gxf_TimeCodeH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#if defined(MEDIAINFO_ANCILLARY_YES)
    #include <MediaInfo/Multiple/File_Ancillary.h>
#endif //defined(MEDIAINFO_ANCILLARY_YES)
#include "MediaInfo/TimeCode.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Gxf_TimeCode
//***************************************************************************

class File_Gxf_TimeCode : public File__Analyze
{
public :
    //In
    int32u FrameRate_Code;
    int32u FieldsPerFrame_Code;
    bool   IsAtc; // SMPTE ST 12-2

    //Out
    int64u TimeCode_FirstFrame_ms;
    string TimeCode_FirstFrame;
    string Settings;

    //Constructor/Destructor
    File_Gxf_TimeCode();
    ~File_Gxf_TimeCode();
private :
    //Streams management
    void Streams_Fill();

    //Buffer - Global
    void Read_Buffer_Continue ();
};

} //NameSpace

#endif
