// File_ChannelGrouping - Regrouping PCM streams
// Copyright (C) 2011-2012 MediaArea.net SARL, Info@MediaArea.net
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
    size_t  ByteDepth;
    struct common
    {
        struct channel
        {
            int8u*          Buffer;
            size_t          Buffer_Offset;
            size_t          Buffer_Size;
            size_t          Buffer_Size_Max;

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
        size_t              Channel_Master;
        File__Analyze*      Parser;
        bool                IsPcm;
        bool                IsAes3;

        common()
        {
            Channel_Current=0;
            Channel_Master=(size_t)-1;
            Parser=NULL;
            IsPcm=false;
            IsAes3=false;
        }

        ~common()
        {
            delete Parser; //Parser=NULL;
        }
    };
    int64u  StreamID;
    common* Common;
    size_t  Channel_Pos;
    size_t  Channel_Total;
    int64u  SampleRate;
    int8u   Endianness;
    bool    IsAes3;

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

    //Temp
    size_t  Buffer_Offset_AlreadyInCommon;
    int64u  IsPcm_Frame_Count;
};

} //NameSpace

#endif

