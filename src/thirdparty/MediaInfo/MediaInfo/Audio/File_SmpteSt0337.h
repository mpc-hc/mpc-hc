// File_SmpteSt0337 - Info about SMPTE ST 337 stream
// Copyright (C) 2008-2012 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
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
    int8u   Container_Bits_Original; // In the case of demux from 20-bit stream, data is provided in 24-bit form
    int8u   Endianness;

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

    // Parser
    File__Analyze* Parser;
    void Parser_Parse(const int8u* Parser_Buffer, size_t Parser_Buffer_Size);

    #if MEDIAINFO_SEEK
        bool                        Duration_Detected;
    #endif // MEDIAINFO_SEEK
};

} // NameSpace

#endif

