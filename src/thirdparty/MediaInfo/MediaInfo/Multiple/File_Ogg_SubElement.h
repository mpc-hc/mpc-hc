/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about MPEG Transport Stream files, Program Map Section
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_Ogg_Setup_VorbisH
#define MediaInfo_Ogg_Setup_VorbisH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Ogg_SubElement
//***************************************************************************

class File_Ogg_SubElement : public File__Analyze
{
public :
    //In
    stream_t StreamKind;
    bool     MultipleStreams;
    bool     InAnotherContainer;
    int64u   absolute_granule_position_Resolution;

protected :
    //Streams management
    void Streams_Fill();
    void Streams_Finish();

    //Buffer - File header
    void FileHeader_Parse ();

public :
    File_Ogg_SubElement();
    ~File_Ogg_SubElement();

private :
    //Buffer
    bool Header_Begin();
    void Header_Parse();
    void Data_Parse();

    //Elements
    void Identification();
    void Identification_CELT();
    void Identification_CMML();
    void Identification_BBCD();
    void Identification_FLAC();
    void Identification_JNG();
    void Identification_kate();
    void Identification_KW_DIRAC();
    void Identification_OggMIDI();
    void Identification_MNG();
    void Identification_OpusHead();
    void Identification_PCM();
    void Identification_PNG();
    void Identification_Speex();
    void Identification_theora();
    void Identification_vorbis();
    void Identification_YUV4MPEG();
    void Identification_video();
    void Identification_audio();
    void Identification_text();
    void Identification_fLaC();
    void Identification_fishead();
    void Identification_fisbone();
    void Comment();
    void Default();

    //Temp
    File__Analyze*  Parser;
    size_t          OldSize;
    bool            Identified;
    bool            WithType;
};

} //NameSpace

#endif
