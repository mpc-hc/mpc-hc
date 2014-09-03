/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about VC-3 video streams
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_Vc3H
#define MediaInfo_Vc3H
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include "MediaInfo/Multiple/File_Mpeg4.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Avs
//***************************************************************************

class File_Vc3 : public File__Analyze
{
public :
    //In
    int64u  Frame_Count_Valid;
    float64 FrameRate;

    //constructor/Destructor
    File_Vc3();

private :
    //Streams management
    void Streams_Fill();

    //Buffer - Synchro
    bool Synchronize();
    bool Synched_Test();

    //Buffer - Demux
    #if MEDIAINFO_DEMUX
    bool Demux_UnpacketizeContainer_Test();
    #endif //MEDIAINFO_DEMUX

    //Buffer - Per element
    bool Header_Begin ();
    void Header_Parse ();
    void Data_Parse ();

    //Elements
    void HeaderPrefix();
    void CodingControlA();
    void ImageGeometry();
    void CompressionID();
    void CodingControlB();
    void TimeCode();
    void UserData();
    void MacroblockScanIndices();

    //Temp
    int64u  Data_ToParse;
    int32u  CID;
    bool    CRCF;
    int16u  ALPF;
    int16u  SPL;
    int8u   SBD;
    int8u   FFC_FirstFrame;
    bool    SST;
};

} //NameSpace

#endif
