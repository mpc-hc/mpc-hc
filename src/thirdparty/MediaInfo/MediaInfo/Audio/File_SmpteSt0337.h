/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Non-PCM Audio and Data in an AES3
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_SmpteSt0337H
#define MediaInfo_File_SmpteSt0337H
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_SmpteSt0337
//***************************************************************************

class File_SmpteSt0337 : public File__Analyze
{
public :
    // In
    int8u   Container_Bits;
    int8u   Endianness;
    bool    Aligned;

    // Constructor/Destructor
    File_SmpteSt0337();
    ~File_SmpteSt0337();

private :
    // Streams management
    void Streams_Accept();
    void Streams_Fill();

    // Buffer - Global
    #if MEDIAINFO_SEEK
    void Read_Buffer_Unsynched();
    size_t Read_Buffer_Seek (size_t Method, int64u Value, int64u ID);
    #endif // MEDIAINFO_SEEK

    // Buffer - Synchro
    bool Synchronize();
    bool Synched_Test();
    void Synched_Init();

    // Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    // Elements
    void Raw();
    void Frame();
    void Frame_WithPadding();
    void Frame_FromMpegPs();

    // Temp
    float64 FrameRate;
    int8u   Stream_Bits;
    int8u   data_type;
    std::map<int64u, int64u> FrameSizes;
    int64u  GuardBand_Before;
    int64u  GuardBand_After;
    size_t  NullPadding_Size;

    // Parser
    File__Analyze* Parser;
    void Parser_Parse(const int8u* Parser_Buffer, size_t Parser_Buffer_Size);

    #if MEDIAINFO_SEEK
        bool                        Duration_Detected;
    #endif // MEDIAINFO_SEEK
};

} // NameSpace

#endif

