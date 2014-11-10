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
#if defined(MEDIAINFO_PCM_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Pcm.h"
#if MEDIAINFO_DEMUX
    #include "MediaInfo/MediaInfo_Config_MediaInfo.h"
#endif //MEDIAINFO_DEMUX
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const char* Smpte_St0302_ChannelsPositions(int8u Channels)
{
    switch (Channels)
    {
        case  2 : return "Front: L R";                                  //2 channels
        case  4 : return "Front: L C R, LFE";                           //4 channels
        case  6 : return "Front: L C R, Side: L R, LFE";                //6 channels
        case  8 : return "Front: L C R, Side: L R, Back: L R, LFE";     //8 channels
        default : return "";
    }
}

//---------------------------------------------------------------------------
const char* Smpte_St0302_ChannelsPositions2(int8u Channels)
{
    switch (Channels)
    {
        case  2 : return "2/0/0.0";                                     //2 channels
        case  4 : return "3/0/0.1";                                     //4 channels
        case  6 : return "3/2/0.1";                                     //6 channels
        case  8 : return "3/2/2.1";                                     //8 channels
        default : return "";
    }
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Pcm::File_Pcm()
{
    //Configuration
    ParserName=__T("PCM");
    #if MEDIAINFO_TRACE
        Trace_Layers_Update(8); //Stream
    #endif //MEDIAINFO_TRACE
    IsRawStream=true;
    PTS_DTS_Needed=true;

    //In
    Frame_Count_Valid=4;
    BitDepth=0;
    BitDepth_Significant=0;
    Channels=0;
    SamplingRate=0;
    Endianness='\0';
    Sign='\0';
    #if MEDIAINFO_DEMUX
    Frame_Count_Valid_Demux=0;
    #endif //MEDIAINFO_DEMUX
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Pcm::Streams_Fill()
{
    if (Count_Get(Stream_Audio)==0)
    {
        Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, Audio_Format, "PCM");
        Fill(Stream_Audio, 0, Audio_Codec, "PCM");
    }

    //Filling
    Ztring Firm, ITU;
         if (Codec==__T("EVOB"))             {Firm=__T("");      Endianness='B';            Sign='S';}                        //PCM Signed 16 bits Big Endian, Interleavement is for 2 samples*2 channels L0-1/L0-0/R0-1/R0-0/L1-1/L1-0/R1-1/R1-0/L0-2/R0-2/L1-2/R1-2, http://wiki.multimedia.cx/index.php?title=PCM
    else if (Codec==__T("VOB"))              {Firm=__T("");      Endianness='B';            Sign='S';}                        //PCM Signed 16 bits Big Endian, Interleavement is for 2 samples*2 channels L0-1/L0-0/R0-1/R0-0/L1-1/L1-0/R1-1/R1-0/L0-2/R0-2/L1-2/R1-2, http://wiki.multimedia.cx/index.php?title=PCM
    else if (Codec==__T("M2TS"))             {Firm=__T("");      Endianness='B';            Sign='S';}                        //PCM Signed         Big Endian
    else if (Codec==__T("A_PCM/INT/BIG"))    {Firm=__T("");      Endianness='B';}
    else if (Codec==__T("A_PCM/INT/LITTLE")) {Firm=__T("");      Endianness='L';}
    else if (Codec==__T("A_PCM/INT/FLOAT"))  {Firm=__T("");      Endianness='B';            Sign='F';}
    else if (Codec==__T("fl32"))             {  if (!Endianness) Endianness='B'; if (!Sign) Sign='F'; BitDepth=32;}
    else if (Codec==__T("fl64"))             {  if (!Endianness) Endianness='B'; if (!Sign) Sign='F'; BitDepth=64;}
    else if (Codec==__T("in24"))             {  if (!Endianness) Endianness='B'; if (!Sign) Sign='U'; BitDepth=24;}
    else if (Codec==__T("in32"))             {  if (!Endianness) Endianness='B'; if (!Sign) Sign='U'; BitDepth=32;}
    else if (Codec==__T("raw "))             {  if (!Endianness) Endianness='L';            Sign='U';}
    else if (Codec==__T("twos"))             {                   Endianness='B';            Sign='S';}
    else if (Codec==__T("sowt"))             {                   Endianness='L';            Sign='S';}
    else if (Codec==__T("lpcm"))             {  if (!Endianness) Endianness='B'; if (!Sign) Sign='S';}
    else if (Codec==__T("SWF ADPCM"))        {Firm=__T("SWF");}
    else if (Codec==__T("1"))                {   if (BitDepth)
                                                {
                                                    if (BitDepth>8)
                                                    {            Endianness='L';            Sign='S';}
                                                    else
                                                    {                                       Sign='U';}
                                                }
                                            }
    else if (Codec==__T("2"))                {Firm=__T("Microsoft");}
    else if (Codec==__T("3"))                {                   Endianness='F';}
    else if (Codec==__T("10"))               {Firm=__T("OKI");}
    else if (Codec==__T("11"))               {Firm=__T("Intel");}
    else if (Codec==__T("12"))               {Firm=__T("Mediaspace");}
    else if (Codec==__T("13"))               {Firm=__T("Sierra");}
    else if (Codec==__T("14"))               {Firm=__T("Antex");}
    else if (Codec==__T("17"))               {Firm=__T("Dialogic");}
    else if (Codec==__T("18"))               {Firm=__T("Mediavision");}
    else if (Codec==__T("20"))               {Firm=__T("Yamaha");}
    else if (Codec==__T("33"))               {Firm=__T("Antex");}
    else if (Codec==__T("36"))               {Firm=__T("DSP Solution");}
    else if (Codec==__T("38"))               {Firm=__T("Natural MicroSystems");}
    else if (Codec==__T("39"))               {Firm=__T("Crystal Semiconductor");}
    else if (Codec==__T("3B"))               {Firm=__T("Rockwell");}
    else if (Codec==__T("40"))               {Firm=__T("Antex Electronics");}
    else if (Codec==__T("42"))               {Firm=__T("IBM");}
    else if (Codec==__T("45"))               {Firm=__T("Microsoft"); ITU=__T("G.726");}
    else if (Codec==__T("64"))               {Firm=__T("Apicom"); ITU=__T("G.726");}
    else if (Codec==__T("65"))               {Firm=__T("Apicom"); ITU=__T("G.722");}
    else if (Codec==__T("85"))               {Firm=__T("DataFusion Systems"); ITU=__T("G.726");}
    else if (Codec==__T("8B"))               {Firm=__T("Infocom"); ITU=__T("G.721");}
    else if (Codec==__T("97"))               {Firm=__T("ZyXEL");}
    else if (Codec==__T("100"))              {Firm=__T("Rhetorex");}
    else if (Codec==__T("125"))              {Firm=__T("Sanyo");}
    else if (Codec==__T("140"))              {Firm=__T("Dictaphone"); ITU=__T("G.726");}
    else if (Codec==__T("170"))              {Firm=__T("Unisys");}
    else if (Codec==__T("175"))              {Firm=__T("SyCom"); ITU=__T("G.726");}
    else if (Codec==__T("178"))              {Firm=__T("Knownledge");}
    else if (Codec==__T("200"))              {Firm=__T("Creative");}
    else if (Codec==__T("210"))              {Firm=__T("Uher");}
    else if (Codec==__T("285"))              {Firm=__T("Norcom Voice Systems");}
    else if (Codec==__T("1001"))             {Firm=__T("Olivetti");}
    else if (Codec==__T("1C03"))             {Firm=__T("Lucent"); ITU=__T("G.723");}
    else if (Codec==__T("1C0C"))             {Firm=__T("Lucent"); ITU=__T("G.723");}
    else if (Codec==__T("4243"))             {ITU=__T("G.726");}
    else if (Codec==__T("A105"))             {ITU=__T("G.726");}
    else if (Codec==__T("A107"))             {ITU=__T("G.726");}

    //Format
    Fill(Stream_Audio, 0, Audio_Codec_String, "PCM");
    Fill(Stream_Audio, 0, Audio_Codec_Family, "PCM");
    Fill(Stream_Audio, 0, Audio_BitRate_Mode, "CBR");

    //SamplingRate
    if (SamplingRate)
        Fill(Stream_Audio, 0, Audio_SamplingRate, SamplingRate);

    //Firm
    Fill(Stream_Audio, 0, Audio_Format_Settings, Firm);
    Fill(Stream_Audio, 0, Audio_Format_Settings_Firm, Firm);
    Fill(Stream_Audio, 0, Audio_Codec_Settings, Firm);
    Fill(Stream_Audio, 0, Audio_Codec_Settings_Firm, Firm);

    //Endianess
    const char* Value;
    switch (Endianness)
    {
        case 'B': Value="Big"; break;
        case 'L': Value="Little"; break;
        default : Value="";
    }
    Fill(Stream_Audio, 0, Audio_Format_Settings, Value);
    Fill(Stream_Audio, 0, Audio_Format_Settings_Endianness, Value);
    Fill(Stream_Audio, 0, Audio_Codec_Settings, Value);
    Fill(Stream_Audio, 0, Audio_Codec_Settings_Endianness, Value);

    //Sign
    switch (Sign)
    {
        case 'S': Value="Signed"; break;
        case 'U': Value="Unsigned"; break;
        default : Value="";
    }
    Fill(Stream_Audio, 0, Audio_Format_Settings, Value);
    Fill(Stream_Audio, 0, Audio_Format_Settings_Sign, Value);
    Fill(Stream_Audio, 0, Audio_Codec_Settings, Value);
    Fill(Stream_Audio, 0, Audio_Codec_Settings_Sign, Value);

    //ITU
    Fill(Stream_Audio, 0, Audio_Format_Settings, ITU);
    Fill(Stream_Audio, 0, Audio_Format_Settings_ITU, ITU);
    Fill(Stream_Audio, 0, Audio_Codec_Settings, ITU);
    Fill(Stream_Audio, 0, Audio_Codec_Settings_ITU, ITU);

    //BitDepth
    if (BitDepth_Significant)
    {
        Fill(Stream_Audio, 0, Audio_BitDepth, BitDepth_Significant);
        Fill(Stream_Audio, 0, Audio_BitDepth_Stored, BitDepth);
    }
    else if (BitDepth)
        Fill(Stream_Audio, 0, Audio_BitDepth, BitDepth);

    //Channels
    if (Channels)
        Fill(Stream_Audio, 0, Audio_Channel_s_, Channels);

    //Bit rate
    if (SamplingRate && BitDepth && Channels)
        Fill(Stream_Audio, 0, Audio_BitRate, SamplingRate*BitDepth*Channels);

    //ChannelsPositions
    if (Codec==__T("SMPTE ST 337"))
    {
        Fill(Stream_Audio, 0, Audio_ChannelPositions, Smpte_St0302_ChannelsPositions(Channels));
        Fill(Stream_Audio, 0, Audio_ChannelPositions_String2, Smpte_St0302_ChannelsPositions2(Channels));
    }
}

//---------------------------------------------------------------------------
void File_Pcm::Streams_Finish()
{
    //No frames in PCM!
    Frame_Count=(int64u)-1;
    Frame_Count_NotParsedIncluded=(int64u)-1;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Pcm::Read_Buffer_Continue()
{
    //Testing if we get enough data
    if (SamplingRate && BitDepth && Channels)
    {
        int64u BitRate=SamplingRate*BitDepth*Channels;
        int64u ByteRate=BitRate/8;
        if (Buffer_Size>=ByteRate/4) // 1/4 of second is enough for detection
            Frame_Count_Valid=2;
    }

    #if MEDIAINFO_DEMUX
        if (Demux_UnpacketizeContainer && !Status[IsAccepted])
        {
            Frame_Count_Valid_Demux++;
            if (Frame_Count_Valid_Demux<Frame_Count_Valid)
                Element_WaitForMoreData();
        }
    #endif //MEDIAINFO_DEMUX
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Pcm::FileHeader_Begin()
{
    if (!Frame_Count_Valid)
    {
        Accept();
        Finish();
    }

    return true;
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Pcm::Header_Parse()
{
    //Filling
    Header_Fill_Code(0, "Block");
    if (BitDepth && Channels)
    {
        int64u Size=(Element_Size/(BitDepth*Channels/8))*(BitDepth*Channels/8); //A complete sample
        if (Element_Size && Size==0)
        {
            Element_WaitForMoreData();
            return;
        }
        Header_Fill_Size(Size);
    }
    else
        Header_Fill_Size(Element_Size); // Unknown sample size
}

//---------------------------------------------------------------------------
void File_Pcm::Data_Parse()
{
    #if MEDIAINFO_DEMUX
        FrameInfo.PTS=FrameInfo.DTS;
        if (Frame_Count_Valid_Demux)
        {
            if (FrameInfo.DUR!=(int64u)-1)
                FrameInfo.DUR*=Frame_Count_Valid_Demux;
            if (Frame_Count_NotParsedIncluded!=(int64u)-1 && Frame_Count_NotParsedIncluded>=Frame_Count_Valid_Demux)
                Frame_Count_NotParsedIncluded-=Frame_Count_Valid_Demux-1;
        }
        Demux_random_access=true;
        Element_Code=(int64u)-1;

        if (BitDepth==20 && Endianness=='L' && Config->Demux_PCM_20bitTo16bit_Get())
        {
            size_t Info_Offset=(size_t)Element_Size;
            const int8u* Info=Buffer+Buffer_Offset;
            size_t Info2_Size=Info_Offset*4/5;
            int8u* Info2=new int8u[Info2_Size];
            size_t Info2_Pos=0;
            size_t Info_Pos=0;

            //Removing bits 3-0 (Little endian)
            // Dest  : 20LE / L1L0 L3L2 R0L4 R2R1 R4R3
            // Source:        L2L1 L4L3 R2R1 R4R2
            while (Info_Pos+5<=Info_Offset)
            {
                Info2[Info2_Pos  ] =(Info[Info_Pos+1]<<4  ) | (Info[Info_Pos+0]>>4  );
                Info2[Info2_Pos+1] =(Info[Info_Pos+2]<<4  ) | (Info[Info_Pos+1]>>4  );
                Info2[Info2_Pos+2] = Info[Info_Pos+3]                                ;
                Info2[Info2_Pos+3] = Info[Info_Pos+4]                                ;

                Info2_Pos+=4;
                Info_Pos+=5;
            }

            Demux(Info2, Info2_Pos, ContentType_MainStream);

            delete[] Info2;
        }
        else if (BitDepth==20 && Endianness=='L' && Config->Demux_PCM_20bitTo24bit_Get())
        {
            size_t Info_Offset=(size_t)Element_Size;
            const int8u* Info=Buffer+Buffer_Offset;
            size_t Info2_Size=Info_Offset*6/5;
            int8u* Info2=new int8u[Info2_Size];
            size_t Info2_Pos=0;
            size_t Info_Pos=0;

            //Padding bits 3-0 (Little endian)
            // Dest  : 20LE / L1L0 L3L2 R0L4 R2R1 R4R3
            // Source:        L0XX L2L1 L4L3 R0XX R2R1 R4R2
            while (Info_Pos+5<=Info_Offset)
            {
                Info2[Info2_Pos  ] = Info[Info_Pos+0]<<4                             ;
                Info2[Info2_Pos+1] =(Info[Info_Pos+1]<<4  ) | (Info[Info_Pos+0]>>4  );
                Info2[Info2_Pos+2] =(Info[Info_Pos+2]<<4  ) | (Info[Info_Pos+1]>>4  );
                Info2[Info2_Pos+3] = Info[Info_Pos+2]&0xF0                           ;
                Info2[Info2_Pos+4] = Info[Info_Pos+3]                                ;
                Info2[Info2_Pos+5] = Info[Info_Pos+4]                                ;

                Info2_Pos+=6;
                Info_Pos+=5;
            }

            Demux(Info2, Info2_Pos, ContentType_MainStream);

            delete[] Info2;
        }
        else
        {
            Demux(Buffer+Buffer_Offset, (size_t)Element_Size, ContentType_MainStream);
        }
    #endif //MEDIAINFO_DEMUX

    //Parsing
    Skip_XX(Element_Size,                                       "Data"); //It is impossible to detect... Default is no detection, only filling

    #if MEDIAINFO_DEMUX
    if (Frame_Count_Valid_Demux)
    {
        Frame_Count+=Frame_Count_Valid_Demux-1;
        if (Frame_Count_NotParsedIncluded!=(int64u)-1)
            Frame_Count_NotParsedIncluded+=Frame_Count_Valid_Demux-1;
        FrameInfo.DUR/=Frame_Count_Valid_Demux;
        if (FrameInfo.DTS!=(int64u)-1)
            FrameInfo.DTS+=FrameInfo.DUR*Frame_Count;
        Frame_Count_Valid_Demux=0;
    }
    #endif //MEDIAINFO_DEMUX
    Frame_Count++;
    if (Frame_Count_NotParsedIncluded!=(int64u)-1)
        Frame_Count_NotParsedIncluded++;
    if (FrameInfo.DTS!=(int64u)-1 && FrameInfo.DUR!=(int64u)-1)
    {
        FrameInfo.DTS+=FrameInfo.DUR;
        FrameInfo.PTS=FrameInfo.DTS;
    }
    if ((!Status[IsAccepted] && Frame_Count>=Frame_Count_Valid) || File_Offset+Buffer_Size>=File_Size)
    {
        Accept();
        Fill();
    }
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_PCM_YES
