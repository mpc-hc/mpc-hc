/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Some containers use mono stream for AES3 (Stereo) grouping
// We need to group the 2-mono streams in one before sending
// data to AES parser
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_ChannelGroupingH
#define MediaInfo_File_ChannelGroupingH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include <cstring>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_ChannelGrouping
//***************************************************************************

class File_ChannelGrouping : public File__Analyze
{
public :
    //In
    int8u   BitDepth;
    int16u  SamplingRate;
    int8u   Endianness;
    bool    Aligned;
    bool    CanBePcm;

    struct common
    {
        struct channel
        {
            int8u*          Buffer;
            size_t          Buffer_Offset;
            size_t          Buffer_Size;
            size_t          Buffer_Size_Max;
            std::vector<int64u> Offsets_Stream;
            std::vector<int64u> Offsets_Buffer;

            channel()
            {
                Buffer=new int8u[32768];
                Buffer_Offset=0;
                Buffer_Size=0;
                Buffer_Size_Max=32768;
            }

            ~channel()
            {
                delete[] Buffer; //Buffer=NULL;
            }

            void resize(size_t NewSize)
            {
                if (NewSize<Buffer_Size_Max)
                    return;
                int8u* Old=Buffer;
                if (NewSize<2*Buffer_Size_Max)
                    Buffer_Size_Max*=2;
                else
                    Buffer_Size_Max=NewSize;
                Buffer=new int8u[Buffer_Size_Max];
                std::memcpy(Buffer, Old, Buffer_Size);
                delete[] Old; //Old=NULL;
            }

            void optimize()
            {
                if (Buffer_Offset<Buffer_Size_Max/2)
                    return;
                std::memcpy(Buffer, Buffer+Buffer_Offset, Buffer_Size-Buffer_Offset);
                Buffer_Size-=Buffer_Offset;
                Buffer_Offset=0;
            }

        };
        vector<channel*>    Channels;
        channel             MergedChannel;
        size_t              Channel_Current;
        std::vector<File__Analyze*> Parsers;
        size_t              Instances;
        size_t              Instances_Max;

        common()
        {
            Channel_Current=0;
            Instances=0;
            Instances_Max=0;
        }

        ~common()
        {
            for (size_t Pos=0; Pos<Parsers.size(); Pos++)
                delete Parsers[Pos];
        }
    };
    int64u  StreamID;
    common* Common;
    int8u   Channel_Pos;
    int8u   Channel_Total;

    //Constructor/Destructor
    File_ChannelGrouping();
    ~File_ChannelGrouping();

private :
    //Streams management
    void Streams_Fill();
    void Streams_Finish();

    //Buffer - Global
    void Read_Buffer_Init ();
    void Read_Buffer_Continue ();
    void Read_Buffer_Unsynched ();
};

} //NameSpace

#endif

