/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about SMPTE ST0302
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_SmpteSt0302H
#define MediaInfo_File_SmpteSt0302H
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_SmpteSt0302
//***************************************************************************

class File_SmpteSt0302 : public File__Analyze
{
public :
    //Constructor/Destructor
    File_SmpteSt0302();
    ~File_SmpteSt0302();

private :
    //Streams management
    void Streams_Accept();
    void Streams_Fill();

    //Buffer - Global
    void Read_Buffer_Continue ();

    //Temp
    int16u  audio_packet_size;
    int8u   number_channels;
    int8u   bits_per_sample;

    //Parsers
    std::vector<File__Analyze*> Parsers;
    void            Parsers_Init();
    void            Parsers_Parse(const int8u* Parser_Buffer, size_t Parser_Buffer_Size);
};

} //NameSpace

#endif

