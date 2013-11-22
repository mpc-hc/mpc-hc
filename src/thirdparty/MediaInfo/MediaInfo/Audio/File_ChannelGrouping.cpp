/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
// Pre-compilation
#include "MediaInfo/PreComp.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Setup.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_SMPTEST0337_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_ChannelGrouping.h"
#if defined(MEDIAINFO_SMPTEST0337_YES)
    #include "MediaInfo/Audio/File_SmpteSt0337.h"
#endif
#if defined(MEDIAINFO_PCM_YES)
    #include "MediaInfo/Audio/File_Pcm.h"
#endif
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Events.h"
#endif //MEDIAINFO_EVENTS
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

File_ChannelGrouping::File_ChannelGrouping()
{
    //Configuration
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_ChannelGrouping;
        StreamIDs_Width[0]=0;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        Demux_Level=2; //Container
    #endif //MEDIAINFO_DEMUX
    #if MEDIAINFO_TRACE
        Trace_Layers_Update(0); //Container1
    #endif //MEDIAINFO_TRACE
    IsRawStream=true;

    //In
    BitDepth=0;
    SamplingRate=0;
    Endianness=0;
    Aligned=false;
    CanBePcm=false;
    Common=NULL;
    Channel_Pos=0;
    Channel_Total=1;
}

File_ChannelGrouping::~File_ChannelGrouping()
{
    Common->Instances--;

    if (Common->Instances==0)
    {
        for (size_t Pos=0; Pos<Common->Channels.size(); Pos++)
            delete Common->Channels[Pos]; //Common->Channels[Pos]=NULL;
        delete Common; //Common=NULL;
    }
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_ChannelGrouping::Streams_Fill()
{
    Fill(Stream_General, 0, General_Format, "ChannelGrouping");

    if (Channel_Pos!=Common->Channels.size()-1)
        return;

    if (Common->Parsers.size()!=1 && CanBePcm) // Last parser is PCM, impossible to detect with another method if htere is only one block
    {
        for (size_t Pos=0; Pos<Common->Parsers.size()-1; Pos++)
            delete Common->Parsers[Pos];
        Common->Parsers.erase(Common->Parsers.begin(), Common->Parsers.begin()+Common->Parsers.size()-1);
        Common->Parsers[0]->Accept();
        Common->Parsers[0]->Fill();
    }

    if (Common->Parsers.size()!=1)
        return;

    Fill(Common->Parsers[0]);
    Merge(*Common->Parsers[0]);
}

//---------------------------------------------------------------------------
void File_ChannelGrouping::Streams_Finish()
{
    if (Channel_Pos!=Common->Channels.size()-1 || Common->Parsers.size()!=1)
        return;

    Finish(Common->Parsers[0]);
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_ChannelGrouping::Read_Buffer_Init()
{
    if (Common==NULL)
    {
        //Common
        Common=new common;
        Common->Channels.resize(Channel_Total);
        for (size_t Pos=0; Pos<Common->Channels.size(); Pos++)
            Common->Channels[Pos]=new common::channel;
        Element_Code=(int64u)-1;

        //SMPTE ST 337
        {
            File_SmpteSt0337* Parser=new File_SmpteSt0337;
            Parser->Container_Bits=BitDepth;
            Parser->Endianness=Endianness;
            Parser->Aligned=Aligned;
            Common->Parsers.push_back(Parser);
        }

        //PCM
        if (CanBePcm)
        {
            File_Pcm* Parser=new File_Pcm;
            Parser->BitDepth=BitDepth;
            Parser->Channels=Channel_Total;
            Parser->SamplingRate=SamplingRate;
            Parser->Endianness=Endianness;
            Common->Parsers.push_back(Parser);
        }

        //for all parsers
        for (size_t Pos=0; Pos<Common->Parsers.size(); Pos++)
        {
            #if MEDIAINFO_DEMUX
                if (Config->Demux_Unpacketize_Get())
                {
                    Common->Parsers[Pos]->Demux_UnpacketizeContainer=true;
                    Common->Parsers[Pos]->Demux_Level=2; //Container
                    Demux_Level=4; //Intermediate
                }
            #endif //MEDIAINFO_DEMUX
            Open_Buffer_Init(Common->Parsers[Pos]);
        }
    }
    Common->Instances++;
    Common->Instances_Max++;
}

//---------------------------------------------------------------------------
void File_ChannelGrouping::Read_Buffer_Continue()
{
    //Verifying that all instances are still present
    if (Common->Instances!=Common->Instances_Max)
    {
        Reject();
        return;
    }

    //Handling of multiple frames in one block
    if (Buffer_Size==0)
    {
        Offsets_Stream.clear();
        Offsets_Buffer.clear();
        for (size_t Pos=0; Pos<Common->Parsers.size(); Pos++)
            Open_Buffer_Continue(Common->Parsers[Pos], Common->MergedChannel.Buffer+Common->MergedChannel.Buffer_Offset, 0, false);
        return;
    }

    //Demux
    #if MEDIAINFO_DEMUX
        Demux(Common->MergedChannel.Buffer+Common->MergedChannel.Buffer_Offset, Common->MergedChannel.Buffer_Size-Common->MergedChannel.Buffer_Offset, ContentType_MainStream);
    #endif //MEDIAINFO_EVENTS

    //Copying to Channel buffer
    if (Common->Channels[Channel_Pos]->Buffer_Size+Buffer_Size>Common->Channels[Channel_Pos]->Buffer_Size_Max)
        Common->Channels[Channel_Pos]->resize(Common->Channels[Channel_Pos]->Buffer_Size+Buffer_Size);
    memcpy(Common->Channels[Channel_Pos]->Buffer+Common->Channels[Channel_Pos]->Buffer_Size, Buffer, Buffer_Size);
    Common->Channels[Channel_Pos]->Buffer_Size+=Buffer_Size;
    Common->Channels[Channel_Pos]->Offsets_Stream.insert(Common->Channels[Channel_Pos]->Offsets_Stream.begin(), Offsets_Stream.begin(), Offsets_Stream.end());
    Offsets_Stream.clear();
    Common->Channels[Channel_Pos]->Offsets_Buffer.insert(Common->Channels[Channel_Pos]->Offsets_Buffer.begin(), Offsets_Buffer.begin(), Offsets_Buffer.end());
    Offsets_Buffer.clear();
    Skip_XX(Buffer_Size,                                        "Channel grouping data");
    Common->Channel_Current++;
    if (Common->Channel_Current>=Channel_Total)
        Common->Channel_Current=0;

    //Copying to merged channel
    size_t Minimum=(size_t)-1;
    for (size_t Pos=0; Pos<Common->Channels.size(); Pos++)
        if (Minimum>Common->Channels[Pos]->Buffer_Size-Common->Channels[Pos]->Buffer_Offset)
            Minimum=Common->Channels[Pos]->Buffer_Size-Common->Channels[Pos]->Buffer_Offset;
    if (Minimum*8>=BitDepth)
    {
        for (size_t Pos=0; Pos<Common->Channels.size(); Pos++)
        {
            Common->MergedChannel.Offsets_Stream.insert(Common->MergedChannel.Offsets_Stream.end(), Common->Channels[Pos]->Offsets_Stream.begin(), Common->Channels[Pos]->Offsets_Stream.end());
            Common->Channels[Pos]->Offsets_Stream.clear();
            Common->MergedChannel.Offsets_Buffer.insert(Common->MergedChannel.Offsets_Buffer.end(), Common->Channels[Pos]->Offsets_Buffer.begin(), Common->Channels[Pos]->Offsets_Buffer.end());
            Common->Channels[Pos]->Offsets_Buffer.clear();
        }

        while (Minimum*8>=BitDepth)
        {
            switch (BitDepth)
            {
                case 16:
                    // Source: 16XE / L3L2 L1L0 + R3R2 R1R0
                    // Dest  : 16XE / L3L2 L1L0 R3R2 R1R0
                    for (size_t Pos=0; Pos<Common->Channels.size(); Pos++)
                    {
                        if (Common->MergedChannel.Buffer_Size+Minimum>Common->MergedChannel.Buffer_Size_Max)
                            Common->MergedChannel.resize(Common->MergedChannel.Buffer_Size+Minimum);
                        Common->MergedChannel.Buffer[Common->MergedChannel.Buffer_Size++]= Common->Channels[Pos]->Buffer[Common->Channels[Pos]->Buffer_Offset++];
                        Common->MergedChannel.Buffer[Common->MergedChannel.Buffer_Size++]= Common->Channels[Pos]->Buffer[Common->Channels[Pos]->Buffer_Offset++];
                    }
                    Minimum-=2;
                    break;
                case 20:
                    // Source: 20BE / L4L3 L2L1 L0L4 L3L2 L1L0 + R4R3 R2R1 R0R4 R3R2 R1R0
                    // Dest  : 20BE / L4L3 L2L1 L0R4 R3R2 R1R0 L4L3 L2L1 L0R4 R2R1 R1R0
                    if (Endianness=='B')
                        for (size_t Pos=0; Pos+1<Common->Channels.size(); Pos+=2)
                        {
                            if (Common->MergedChannel.Buffer_Size+Minimum*2>Common->MergedChannel.Buffer_Size_Max)
                                Common->MergedChannel.resize(Common->MergedChannel.Buffer_Size+Minimum*2);
                            int8u* Channel1=Common->Channels[Pos]->Buffer+Common->Channels[Pos]->Buffer_Offset;
                            int8u* Channel2=Common->Channels[Pos]->Buffer+Common->Channels[Pos]->Buffer_Offset;
                            Common->MergedChannel.Buffer[Common->MergedChannel.Buffer_Size++]= Channel1[0];
                            Common->MergedChannel.Buffer[Common->MergedChannel.Buffer_Size++]= Channel1[1];
                            Common->MergedChannel.Buffer[Common->MergedChannel.Buffer_Size++]=(Channel1[0]&0xF0) | (Channel2[0]>>4  );
                            Common->MergedChannel.Buffer[Common->MergedChannel.Buffer_Size++]=(Channel2[0]<<4  ) | (Channel2[1]>>4  );
                            Common->MergedChannel.Buffer[Common->MergedChannel.Buffer_Size++]=(Channel2[1]<<4  ) | (Channel2[2]>>4  );
                            Common->MergedChannel.Buffer[Common->MergedChannel.Buffer_Size++]=(Channel1[2]<<4  ) | (Channel1[3]>>4  );
                            Common->MergedChannel.Buffer[Common->MergedChannel.Buffer_Size++]=(Channel1[3]<<4  ) | (Channel1[4]>>4  );
                            Common->MergedChannel.Buffer[Common->MergedChannel.Buffer_Size++]=(Channel1[4]<<4  ) | (Channel2[2]&0x0F);
                            Common->MergedChannel.Buffer[Common->MergedChannel.Buffer_Size++]= Channel2[3];
                            Common->MergedChannel.Buffer[Common->MergedChannel.Buffer_Size++]= Channel2[4];
                            Common->Channels[Pos]->Buffer_Offset+=5;
                            Common->Channels[Pos+1]->Buffer_Offset+=5;
                        }
                    // Source: 20LE / L1L0 L3L2 L0L4 L2L1 L4L3 + R1R0 R3R2 R0R4 R2R1 R4R3
                    // Dest  : 20LE / L1L0 L3L2 R0L4 R2R1 R4R3 L1L0 L3L2 R0L4 R2R1 R4R3
                    else
                        for (size_t Pos=0; Pos+1<Common->Channels.size(); Pos+=2)
                        {
                            if (Common->MergedChannel.Buffer_Size+Minimum*2>Common->MergedChannel.Buffer_Size_Max)
                                Common->MergedChannel.resize(Common->MergedChannel.Buffer_Size+Minimum*2);
                            int8u* Channel1=Common->Channels[Pos]->Buffer+Common->Channels[Pos]->Buffer_Offset;
                            int8u* Channel2=Common->Channels[Pos]->Buffer+Common->Channels[Pos]->Buffer_Offset;
                            Common->MergedChannel.Buffer[Common->MergedChannel.Buffer_Size++]= Channel1[0];
                            Common->MergedChannel.Buffer[Common->MergedChannel.Buffer_Size++]= Channel1[1];
                            Common->MergedChannel.Buffer[Common->MergedChannel.Buffer_Size++]=(Channel2[0]<<4  ) | (Channel1[2]&0x0F);
                            Common->MergedChannel.Buffer[Common->MergedChannel.Buffer_Size++]=(Channel2[1]<<4  ) | (Channel2[0]>>4  );
                            Common->MergedChannel.Buffer[Common->MergedChannel.Buffer_Size++]=(Channel2[2]<<4  ) | (Channel2[1]>>4  );
                            Common->MergedChannel.Buffer[Common->MergedChannel.Buffer_Size++]=(Channel1[3]<<4  ) | (Channel1[2]>>4  );
                            Common->MergedChannel.Buffer[Common->MergedChannel.Buffer_Size++]=(Channel1[4]<<4  ) | (Channel1[3]>>4  );
                            Common->MergedChannel.Buffer[Common->MergedChannel.Buffer_Size++]=(Channel2[2]&0xF0) | (Channel1[4]>>4  );
                            Common->MergedChannel.Buffer[Common->MergedChannel.Buffer_Size++]= Channel2[3];
                            Common->MergedChannel.Buffer[Common->MergedChannel.Buffer_Size++]= Channel2[4];
                            Common->Channels[Pos]->Buffer_Offset+=5;
                            Common->Channels[Pos+1]->Buffer_Offset+=5;
                        }
                    Minimum-=5; //2.5 twice
                    break;
                case 24:
                    // Source: 24XE / L5L4 L3L2 L1L0 + R5R4 R3R2 R1R0
                    // Dest  : 24XE / L5L4 L3L2 L1L0 R5R4 R3R2 R1R0
                    for (size_t Pos=0; Pos<Common->Channels.size(); Pos++)
                    {
                        if (Common->MergedChannel.Buffer_Size+Minimum>Common->MergedChannel.Buffer_Size_Max)
                            Common->MergedChannel.resize(Common->MergedChannel.Buffer_Size+Minimum);
                        Common->MergedChannel.Buffer[Common->MergedChannel.Buffer_Size++]= Common->Channels[Pos]->Buffer[Common->Channels[Pos]->Buffer_Offset++];
                        Common->MergedChannel.Buffer[Common->MergedChannel.Buffer_Size++]= Common->Channels[Pos]->Buffer[Common->Channels[Pos]->Buffer_Offset++];
                        Common->MergedChannel.Buffer[Common->MergedChannel.Buffer_Size++]= Common->Channels[Pos]->Buffer[Common->Channels[Pos]->Buffer_Offset++];
                    }
                    Minimum-=3;
                    break;
                default: ;
                    // Not supported
                    Reject();
                    return;
            }
        }
    }

    if (Common->MergedChannel.Buffer_Size>Common->MergedChannel.Buffer_Offset)
    {
        for (size_t Pos=0; Pos<Common->Parsers.size(); Pos++)
        {
            if (FrameInfo_Next.DTS!=(int64u)-1)
                Common->Parsers[Pos]->FrameInfo=FrameInfo_Next; //AES3 parse has its own buffer management
            else if (FrameInfo.DTS!=(int64u)-1)
            {
                Common->Parsers[Pos]->FrameInfo=FrameInfo;
                FrameInfo=frame_info();
            }
            Common->Parsers[Pos]->Offsets_Stream.insert(Common->Parsers[Pos]->Offsets_Stream.end(), Common->MergedChannel.Offsets_Stream.begin(), Common->MergedChannel.Offsets_Stream.end());
            Common->Parsers[Pos]->Offsets_Buffer.insert(Common->Parsers[Pos]->Offsets_Buffer.end(), Common->MergedChannel.Offsets_Buffer.begin(), Common->MergedChannel.Offsets_Buffer.end());
            for (size_t Offsets_Pos_Temp=Common->Parsers[Pos]->Offsets_Buffer.size()-Common->MergedChannel.Offsets_Buffer.size(); Offsets_Pos_Temp<Common->Parsers[Pos]->Offsets_Buffer.size(); Offsets_Pos_Temp++)
                Common->Parsers[Pos]->Offsets_Buffer[Offsets_Pos_Temp]+=Common->Parsers[Pos]->Buffer_Size/Common->Channels.size();
            Open_Buffer_Continue(Common->Parsers[Pos], Common->MergedChannel.Buffer+Common->MergedChannel.Buffer_Offset, Common->MergedChannel.Buffer_Size-Common->MergedChannel.Buffer_Offset, false);

            //Multiple parsers
            if (Common->Parsers.size()>1)
            {
                if (!Common->Parsers[Pos]->Status[IsAccepted] && Common->Parsers[Pos]->Status[IsFinished])
                {
                    delete *(Common->Parsers.begin()+Pos);
                    Common->Parsers.erase(Common->Parsers.begin()+Pos);
                    Pos--;
                }
                else if (Common->Parsers.size()>1 && Common->Parsers[Pos]->Status[IsAccepted])
                {
                    File__Analyze* Parser=Common->Parsers[Pos];
                    for (size_t Pos2=0; Pos2<Common->Parsers.size(); Pos2++)
                    {
                        if (Pos2!=Pos)
                            delete *(Common->Parsers.begin()+Pos2);
                    }
                    Common->Parsers.clear();
                    Common->Parsers.push_back(Parser);
                }
            }
        }
        Common->MergedChannel.Buffer_Offset=Common->MergedChannel.Buffer_Size;
        Common->MergedChannel.Offsets_Stream.clear();
        Common->MergedChannel.Offsets_Buffer.clear();
    }
    if (!Status[IsAccepted] && Common->Parsers.size()==1 && Common->Parsers[0]->Status[IsAccepted])
        Accept();
    if (!Status[IsFilled] && Common->Parsers.size()==1 && Common->Parsers[0]->Status[IsFilled])
        Fill();
    if (!Status[IsFinished] && Common->Parsers.size()==1 && Common->Parsers[0]->Status[IsFinished])
        Finish();

    //Optimize buffer
    for (size_t Pos=0; Pos<Common->Channels.size(); Pos++)
        Common->Channels[Pos]->optimize();
    Common->MergedChannel.optimize();
}

//---------------------------------------------------------------------------
void File_ChannelGrouping::Read_Buffer_Unsynched()
{
    for (size_t Pos=0; Pos<Common->Parsers.size(); Pos++)
        if (Common->Parsers[Pos])
            Common->Parsers[Pos]->Open_Buffer_Unsynch();

    Common->MergedChannel.Buffer_Offset=0;
    Common->MergedChannel.Buffer_Size=0;
    for (size_t Pos=0; Pos<Common->Channels.size(); Pos++)
    {
        Common->Channels[Pos]->Buffer_Offset=0;
        Common->Channels[Pos]->Buffer_Size=0;
    }
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_SMPTEST0337_YES
