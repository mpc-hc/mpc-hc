/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about Ogg files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_OggH
#define MediaInfo_File_OggH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Ogg
//***************************************************************************

class File_Ogg : public File__Analyze
{
public :
    //In
    bool   SizedBlocks;
    bool   XiphLacing;

    //Constructor/Destructor
    File_Ogg();

private :
    //Streams management
    void Streams_Fill();
    void Streams_Finish();

    //Buffer - File header
    bool FileHeader_Begin();

    //Buffer - Synchro
    bool Synchronize();
    bool Synched_Test();

    //Buffer - Per element
    void Header_Parse();
    void Header_Parse_AdaptationField();
    void Data_Parse();

    //Temp - Global
    int32u StreamsToDo;
    bool   Parsing_End;

    //Temp - Stream
    struct stream
    {
        File__Analyze* Parser;
        stream_t StreamKind;
        size_t StreamPos;
        bool   SearchingPayload;
        bool   SearchingTimeCode;
        int64u absolute_granule_position;
        int64u absolute_granule_position_Resolution;

        stream()
        {
            Parser=NULL;
            StreamKind=Stream_Max;
            StreamPos=(size_t)-1;
            SearchingPayload=true;
            SearchingTimeCode=true;
            absolute_granule_position=0;
            absolute_granule_position_Resolution=0;
        }
        ~stream()
        {
            delete Parser; //Parser=NULL
        }
    };

    int8u packet_type;
    bool continued;
    bool eos;
    bool continued_NextFrame;
    std::map<int64u, stream> Stream;
    std::vector<size_t>      Chunk_Sizes;
    bool                     Chunk_Sizes_Finished;
};

} //NameSpace

#endif
