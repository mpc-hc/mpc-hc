// File_Vc3 - Info for VC-3 streams
// Copyright (C) 2010-2011 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
    int64u Frame_Count_Valid;
    float  FrameRate;

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
    bool    SST;
};

} //NameSpace

#endif
