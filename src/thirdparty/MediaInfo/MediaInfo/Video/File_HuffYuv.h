/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about HUFFYUV files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_HuffYUVH
#define MediaInfo_HuffYUVH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_HuffYuv
//***************************************************************************

class File_HuffYuv : public File__Analyze
{
public :
    //In
    bool    IsOutOfBandData;
    int16u  BitCount;
    int32u  Height;

    //Constructor/Destructor
    File_HuffYuv();

private :
    //Streams management
    void Streams_Accept();

    //Buffer - Global
    void Read_Buffer_Continue();

    //Elements
    void FrameHeader();
};

} //NameSpace

#endif
