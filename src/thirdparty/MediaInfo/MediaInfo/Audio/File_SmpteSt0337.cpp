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
#include "MediaInfo/Audio/File_SmpteSt0337.h"
#if defined(MEDIAINFO_AAC_YES)
    #include "MediaInfo/Audio/File_Aac.h"
#endif
#if defined(MEDIAINFO_AC3_YES)
    #include "MediaInfo/Audio/File_Ac3.h"
#endif
#if defined(MEDIAINFO_DOLBYE_YES)
    #include "MediaInfo/Audio/File_DolbyE.h"
#endif
#if defined(MEDIAINFO_MPEGA_YES)
    #include "MediaInfo/Audio/File_Mpega.h"
#endif
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Events.h"
#endif // MEDIAINFO_EVENTS
#if MEDIAINFO_SEEK
    #include "MediaInfo/MediaInfo_Internal.h"
#endif // MEDIAINFO_SEEK
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const char* Smpte_St0337_data_type[32]= // SMPTE ST 338
{
    "",
    "AC-3",
    "Time stamp",
    "Pause",
    "MPEG Audio",
    "MPEG Audio",
    "MPEG Audio",
    "AAC",
    "MPEG Audio",
    "MPEG Audio",
    "AAC",
    "AAC",
    "",
    "",
    "",
    "",
    "E-AC-3",
    "",
    "",
    "AAC",
    "",
    "E-AC-3",
    "",
    "",
    "",
    "",
    "Utility",
    "KLV",
    "Dolby E",
    "Captioning",
    "User defined",
    "",
};

//---------------------------------------------------------------------------
stream_t Smpte_St0337_data_type_StreamKind[32]= // SMPTE 338M
{
    Stream_Max,
    Stream_Audio,
    Stream_Max,
    Stream_Max,
    Stream_Audio,
    Stream_Audio,
    Stream_Audio,
    Stream_Max,
    Stream_Audio,
    Stream_Audio,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Max,
    Stream_Menu,
    Stream_Audio,
    Stream_Text,
    Stream_Max,
    Stream_Max,
};

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_SmpteSt0337::File_SmpteSt0337()
:File__Analyze()
{
    // Configuration
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Aes3;
    #endif // MEDIAINFO_EVENTS
    MustSynchronize=true;
    Buffer_TotalBytes_FirstSynched_Max=32*1024;
    PTS_DTS_Needed=true;

    // In
    Container_Bits=0;
    Endianness=0x00;
    Aligned=false;

    // Temp
    FrameRate=0;
    Stream_Bits=0;
    data_type=(int8u)-1;
    GuardBand_Before=0;
    GuardBand_After=0;
    NullPadding_Size=0;

    // Parser
    Parser=NULL;

    #if MEDIAINFO_SEEK
        Duration_Detected=false;
    #endif // MEDIAINFO_SEEK
}

//---------------------------------------------------------------------------
File_SmpteSt0337::~File_SmpteSt0337()
{
    delete Parser; // Parser=NULL;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_SmpteSt0337::Streams_Accept()
{
    Fill(Stream_General, 0, General_Format, "AES3");
    Fill(Stream_General, 0, General_OverallBitRate_Mode, "CBR");
}

//---------------------------------------------------------------------------
void File_SmpteSt0337::Streams_Fill()
{
    if (Parser && Parser->Status[IsAccepted])
    {
        Fill(Parser);
        Merge(*Parser);

        int64u OverallBitRate=Parser->Retrieve(Stream_General, 0, General_OverallBitRate).To_int64u();
        if (OverallBitRate)
        {
            OverallBitRate*=Element_Size; OverallBitRate/=Element_Size-Stream_Bits*4/8;
            Fill(Stream_General, 0, General_OverallBitRate, Ztring::ToZtring(OverallBitRate)+__T(" / ")+Parser->Retrieve(Stream_General, 0, General_OverallBitRate));
        }
        if (Parser->Count_Get(Stream_Audio))
            FrameRate=Retrieve(Stream_Audio, 0, Audio_FrameRate).To_float64();
    }
    else if (data_type!=(int8u)-1)
    {
        if (Retrieve(Stream_Audio, 0, Audio_Format).empty() && Smpte_St0337_data_type_StreamKind[data_type]!=Stream_Max)
        {
            Stream_Prepare(Smpte_St0337_data_type_StreamKind[data_type]);
            Fill(StreamKind_Last, 0, Fill_Parameter(StreamKind_Last, Generic_Format), Smpte_St0337_data_type[data_type]);
            Fill(StreamKind_Last, 0, Fill_Parameter(StreamKind_Last, Generic_Codec), Smpte_St0337_data_type[data_type]);
        }
    }

    // Guard band
    if (GuardBand_Before) // With guard band, there is big chances that AES3 bit rate is respected
    {
        Fill(Stream_General, 0, General_OverallBitRate, Container_Bits*2*48000);
        if (!IsSub && File_Size!=(int64u)-1)
            Fill(Stream_General, 0, General_Duration, ((float64)File_Size)*8/(Container_Bits*2*48000)*1000);
    }

    if (FrameRate && FrameSizes.size()==1)
    {
        Fill(Stream_General, 0, General_OverallBitRate, FrameSizes.begin()->first*Container_Bits*FrameRate, 0);
    }

    for (size_t Pos=0; Pos<Count_Get(StreamKind_Last); Pos++)
    {
        if (Endianness=='L' && Retrieve(StreamKind_Last, Pos, "Format_Settings_Endianness")==__T("Little"))
            Endianness='B';
        switch (Endianness)
        {
            case 'B' :
                        Fill(StreamKind_Last, Pos, "Format_Settings", "Big");
                        Fill(StreamKind_Last, Pos, "Format_Settings_Endianness", "Big", Unlimited, true, true);
                        break;
            case 'L' :
                        Fill(StreamKind_Last, Pos, "Format_Settings", "Little");
                        Fill(StreamKind_Last, Pos, "Format_Settings_Endianness", "Little", Unlimited, true, true);
                        break;
            default  : ;
        }
        Fill(StreamKind_Last, Pos, "Format_Settings_Mode", Container_Bits);
        if (Retrieve(StreamKind_Last, Pos, Fill_Parameter(StreamKind_Last, Generic_BitDepth)).empty())
            Fill(StreamKind_Last, Pos, Fill_Parameter(StreamKind_Last, Generic_BitDepth), Stream_Bits);

        Fill(StreamKind_Last, Pos, "MuxingMode", "AES3");
        if (Retrieve(StreamKind_Last, Pos, Fill_Parameter(StreamKind_Last, Generic_BitRate_Mode))!=__T("CBR"))
            Fill(StreamKind_Last, Pos, Fill_Parameter(StreamKind_Last, Generic_BitRate_Mode), "CBR");
        if (File_Size!=(int64u)-1 && FrameSizes.size()==1)
            Fill(StreamKind_Last, Pos, Fill_Parameter(StreamKind_Last, Generic_FrameCount), File_Size/FrameSizes.begin()->first);
        if (Retrieve(StreamKind_Last, Pos, Fill_Parameter(StreamKind_Last, Generic_Duration)).empty())
            Fill(StreamKind_Last, Pos, Fill_Parameter(StreamKind_Last, Generic_Duration), Retrieve(Stream_General, 0, General_Duration));
    }
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

#if MEDIAINFO_SEEK
//---------------------------------------------------------------------------
void File_SmpteSt0337::Read_Buffer_Unsynched()
{
    if (Frame_Count_NotParsedIncluded!=(int64u)-1 && FrameRate)
    {
        Frame_Count_NotParsedIncluded=float64_int64s(File_GoTo/FrameRate);
        FrameInfo.DTS=Frame_Count_NotParsedIncluded*1000000000/48000;
    }
}
#endif // MEDIAINFO_SEEK

//---------------------------------------------------------------------------
#if MEDIAINFO_SEEK
size_t File_SmpteSt0337::Read_Buffer_Seek (size_t Method, int64u Value, int64u ID)
{
    // Init
    if (!Duration_Detected)
    {
        MediaInfo_Internal MI;
        MI.Option(__T("File_KeepInfo"), __T("1"));
        Ztring ParseSpeed_Save=MI.Option(__T("ParseSpeed_Get"), __T(""));
        Ztring Demux_Save=MI.Option(__T("Demux_Get"), __T(""));
        MI.Option(__T("ParseSpeed"), __T("0"));
        MI.Option(__T("Demux"), Ztring());
        size_t MiOpenResult=MI.Open(File_Name);
        MI.Option(__T("ParseSpeed"), ParseSpeed_Save); // This is a global value, need to reset it. TODO: local value
        MI.Option(__T("Demux"), Demux_Save); // This is a global value, need to reset it. TODO: local value
        if (!MiOpenResult)
            return 0;

        FrameRate=MI.Get(Stream_Audio, 0, __T("FrameRate")).To_float64();

        Duration_Detected=true;
    }

    // Parsing
    switch (Method)
    {
        case 0  :
                    if (FrameRate)
                    {
                        float64 FrameSize=3072000/FrameRate;
                        int64u  FrameCount=float64_int64s(Value/FrameSize);
                        Value=float64_int64s(FrameCount*FrameSize);
                    }
                    GoTo(Value);
                    Open_Buffer_Unsynch();
                    return 1;
        case 1  :
                    return Read_Buffer_Seek(0, File_Size*Value/10000, ID);
        case 2  :   // Timestamp
                    {
                    if (FrameRate)
                        return (size_t)-1; // Not supported

                    {
                        float64 FrameSize=3072000/FrameRate;
                        Unsynch_Frame_Count=float64_int64s(((float64)Value)/1000000000*FrameRate);
                        GoTo(float64_int64s(Unsynch_Frame_Count*FrameSize));
                        Open_Buffer_Unsynch();
                        return 1;
                    }
                    }
        case 3  :   // FrameNumber
                    {
                    if (FrameRate)
                        return (size_t)-1; // Not supported

                    {
                        float64 FrameSize=3072000/FrameRate;
                        Unsynch_Frame_Count=Value;
                        GoTo(float64_int64s(Unsynch_Frame_Count*FrameSize));
                        Open_Buffer_Unsynch();
                        return 1;
                    }
                    }
        default :   return (size_t)-1; // Not supported
    }
}
#endif // MEDIAINFO_SEEK

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_SmpteSt0337::Synchronize()
{
    // Guard band
    size_t Buffer_Offset_Base=Buffer_Offset;

    // Synchronizing
    while (Buffer_Offset+16<=Buffer_Size)
    {
        if (!Status[IsAccepted] && !IsSub && File_Offset_FirstSynched==(int64u)-1 && Buffer_TotalBytes+Buffer_Offset>=Buffer_TotalBytes_FirstSynched_Max)
        {
            Reject();
            return false;
        }

        if ((Container_Bits==0 || Container_Bits==16) && (!Aligned || ((Buffer_TotalBytes+Buffer_Offset)%4)==0))
        {
            if (Buffer[Buffer_Offset  ]==0xF8
             && Buffer[Buffer_Offset+1]==0x72
             && Buffer[Buffer_Offset+2]==0x4E
             && Buffer[Buffer_Offset+3]==0x1F) // 16-bit, BE
            {
                Container_Bits=16;
                Stream_Bits=16;
                Endianness='B'; // BE
                break; // while()
            }
            if (Buffer[Buffer_Offset  ]==0x72
             && Buffer[Buffer_Offset+1]==0xF8
             && Buffer[Buffer_Offset+2]==0x1F
             && Buffer[Buffer_Offset+3]==0x4E) // 16-bit, LE
            {
                Container_Bits=16;
                Stream_Bits=16;
                Endianness='L'; // LE
                break; // while()
            }
        }
        if ((Container_Bits==0 || Container_Bits==20) && (!Aligned || ((Buffer_TotalBytes+Buffer_Offset)%5)==0))
        {
            if (Buffer[Buffer_Offset  ]==0x6F
             && Buffer[Buffer_Offset+1]==0x87
             && Buffer[Buffer_Offset+2]==0x25
             && Buffer[Buffer_Offset+3]==0x4E
             && Buffer[Buffer_Offset+4]==0x1F) // 20-bit, BE
            {
                Container_Bits=20;
                Stream_Bits=20;
                Endianness='B'; // BE
                break; // while()
            }
        }
        if ((Container_Bits==0 || Container_Bits==20) && (!Aligned || ((Buffer_TotalBytes+Buffer_Offset)%5)==0))
        {
            if (Buffer[Buffer_Offset  ]==0x72
             && Buffer[Buffer_Offset+1]==0xF8
             && Buffer[Buffer_Offset+2]==0xF6
             && Buffer[Buffer_Offset+3]==0xE1
             && Buffer[Buffer_Offset+4]==0x54) // 20-bit, LE
            {
                Container_Bits=20;
                Stream_Bits=20;
                Endianness='L'; // BE
                break; // while()
            }
        }
        if ((Container_Bits==0 || Container_Bits==24) && (!Aligned || ((Buffer_TotalBytes+Buffer_Offset)%6)==0))
        {
            if (Buffer[Buffer_Offset  ]==0x96
             && Buffer[Buffer_Offset+1]==0xF8
             && Buffer[Buffer_Offset+2]==0x72
             && Buffer[Buffer_Offset+3]==0xA5
             && Buffer[Buffer_Offset+4]==0x4E
             && Buffer[Buffer_Offset+5]==0x1F) // 24-bit, BE
            {
                Container_Bits=24;
                Stream_Bits=24;
                Endianness='B'; // BE
                break; // while()
            }
            if (Buffer[Buffer_Offset  ]==0x72
             && Buffer[Buffer_Offset+1]==0xF8
             && Buffer[Buffer_Offset+2]==0x96
             && Buffer[Buffer_Offset+3]==0x1F
             && Buffer[Buffer_Offset+4]==0x4E
             && Buffer[Buffer_Offset+5]==0xA5) // 24-bit, LE
            {
                Container_Bits=24;
                Stream_Bits=24;
                Endianness='L'; // LE
                break; // while()
            }
            if (Buffer[Buffer_Offset  ]==0x00
             && Buffer[Buffer_Offset+1]==0xF8
             && Buffer[Buffer_Offset+2]==0x72
             && Buffer[Buffer_Offset+3]==0x00
             && Buffer[Buffer_Offset+4]==0x4E
             && Buffer[Buffer_Offset+5]==0x1F) // 16-bit in 24-bit, BE
            {
                Container_Bits=24;
                Stream_Bits=16;
                Endianness='B'; // BE
                NullPadding_Size=1;
                break; // while()
            }
            if (Buffer[Buffer_Offset  ]==0x00
             && Buffer[Buffer_Offset+1]==0x72
             && Buffer[Buffer_Offset+2]==0xF8
             && Buffer[Buffer_Offset+3]==0x00
             && Buffer[Buffer_Offset+4]==0x1F
             && Buffer[Buffer_Offset+5]==0x4E) // 16-bit in 24-bit, LE
            {
                Container_Bits=24;
                Stream_Bits=16;
                Endianness='L'; // LE
                NullPadding_Size=1;
                break; // while()
            }
            if (Buffer[Buffer_Offset  ]==0x6F
             && Buffer[Buffer_Offset+1]==0x87
             && Buffer[Buffer_Offset+2]==0x20
             && Buffer[Buffer_Offset+3]==0x54
             && Buffer[Buffer_Offset+4]==0xE1
             && Buffer[Buffer_Offset+5]==0xF0) // 20-bit in 24-bit, BE
            {
                Container_Bits=24;
                Stream_Bits=20;
                Endianness='B'; // BE
                break; // while()
            }
            if (Buffer[Buffer_Offset  ]==0x20
             && Buffer[Buffer_Offset+1]==0x87
             && Buffer[Buffer_Offset+2]==0x6F
             && Buffer[Buffer_Offset+3]==0xF0
             && Buffer[Buffer_Offset+4]==0xE1
             && Buffer[Buffer_Offset+5]==0x54) // 20-bit in 24-bit, LE
            {
                Container_Bits=24;
                Stream_Bits=20;
                Endianness='L'; // LE
                break; // while()
            }
        }
        if ((Container_Bits==0 || Container_Bits==32) && (!Aligned || ((Buffer_TotalBytes+Buffer_Offset)%8)==0))
        {
            if (Buffer[Buffer_Offset  ]==0x00
             && Buffer[Buffer_Offset+1]==0x00
             && Buffer[Buffer_Offset+2]==0xF8
             && Buffer[Buffer_Offset+3]==0x72
             && Buffer[Buffer_Offset+4]==0x00
             && Buffer[Buffer_Offset+5]==0x00
             && Buffer[Buffer_Offset+6]==0x4E
             && Buffer[Buffer_Offset+7]==0x1F) // 16-bit in 32-bit, BE
            {
                Container_Bits=32;
                Stream_Bits=16;
                Endianness='B'; // BE
                NullPadding_Size=2;
                break; // while()
            }
            if (Buffer[Buffer_Offset  ]==0x00
             && Buffer[Buffer_Offset+1]==0x00
             && Buffer[Buffer_Offset+2]==0x72
             && Buffer[Buffer_Offset+3]==0xF8
             && Buffer[Buffer_Offset+4]==0x00
             && Buffer[Buffer_Offset+5]==0x00
             && Buffer[Buffer_Offset+6]==0x1F
             && Buffer[Buffer_Offset+7]==0x4E) // 16-bit in 32-bit, LE
            {
                Container_Bits=32;
                Stream_Bits=16;
                Endianness='L'; // LE
                NullPadding_Size=2;
                break; // while()
            }
            if (Buffer[Buffer_Offset  ]==0x00
             && Buffer[Buffer_Offset+1]==0x6F
             && Buffer[Buffer_Offset+2]==0x87
             && Buffer[Buffer_Offset+3]==0x20
             && Buffer[Buffer_Offset+4]==0x00
             && Buffer[Buffer_Offset+5]==0x54
             && Buffer[Buffer_Offset+6]==0xE1
             && Buffer[Buffer_Offset+7]==0xF0) // 20-bit in 32-bit, BE
            {
                Container_Bits=32;
                Stream_Bits=20;
                Endianness='B'; // BE
                NullPadding_Size=1;
                break; // while()
            }
            if (Buffer[Buffer_Offset  ]==0x00
             && Buffer[Buffer_Offset+1]==0x20
             && Buffer[Buffer_Offset+2]==0x87
             && Buffer[Buffer_Offset+3]==0x6F
             && Buffer[Buffer_Offset+4]==0x00
             && Buffer[Buffer_Offset+5]==0xF0
             && Buffer[Buffer_Offset+6]==0xE1
             && Buffer[Buffer_Offset+7]==0x54) // 20-bit in 32-bit, LE
            {
                Container_Bits=32;
                Stream_Bits=20;
                Endianness='L'; // LE
                NullPadding_Size=1;
                break; // while()
            }
            if (Buffer[Buffer_Offset  ]==0x00
             && Buffer[Buffer_Offset+1]==0x96
             && Buffer[Buffer_Offset+2]==0xF8
             && Buffer[Buffer_Offset+3]==0x72
             && Buffer[Buffer_Offset+4]==0x00
             && Buffer[Buffer_Offset+5]==0xA5
             && Buffer[Buffer_Offset+6]==0x4E
             && Buffer[Buffer_Offset+7]==0x1F) // 24-bit in 32-bit, BE
            {
                Container_Bits=32;
                Stream_Bits=24;
                Endianness='B'; // BE
                NullPadding_Size=1;
                break; // while()
            }
            if (Buffer[Buffer_Offset  ]==0x00
             && Buffer[Buffer_Offset+1]==0x72
             && Buffer[Buffer_Offset+2]==0xF8
             && Buffer[Buffer_Offset+3]==0x96
             && Buffer[Buffer_Offset+4]==0x00
             && Buffer[Buffer_Offset+5]==0x1F
             && Buffer[Buffer_Offset+6]==0x4E
             && Buffer[Buffer_Offset+7]==0xA5) // 24-bit in 32-bit, LE
            {
                Container_Bits=32;
                Stream_Bits=24;
                Endianness='L'; // LE
                NullPadding_Size=1;
                break; // while()
            }
        }

        if (Container_Bits>=4 && Aligned)
            Buffer_Offset+=Container_Bits/4;
        else
            Buffer_Offset++;
    }

    // Guard band
    GuardBand_Before+=Buffer_Offset-Buffer_Offset_Base;

    // Parsing last bytes if needed
    if (Buffer_Offset+16>Buffer_Size)
    {
        return false;
    }

    if (!Status[IsAccepted])
        Accept("AES3");

    // Synched
    return true;
}

//---------------------------------------------------------------------------
bool File_SmpteSt0337::Synched_Test()
{
    // Skip NULL padding
    size_t Buffer_Offset_Temp=Buffer_Offset;
    if (Aligned)
    {
        if (Container_Bits==16)
        {
            while ((Buffer_TotalBytes+Buffer_Offset_Temp)%4) // Padding in part of the AES3 block
            {
                if (Buffer_Offset_Temp+1>Buffer_Size)
                {
                    Element_WaitForMoreData();
                    return false;
                }
                if (Buffer[Buffer_Offset_Temp])
                {
                    Trusted_IsNot("Bad sync");
                    return true;
                }
                Buffer_Offset_Temp++;
            }
            while(Buffer_Offset_Temp+4<=Buffer_Size && CC4(Buffer+Buffer_Offset_Temp)==0x00000000)
                Buffer_Offset_Temp+=4;
            if (Buffer_Offset_Temp+4>Buffer_Size)
            {
                Element_WaitForMoreData();
                return false;
            }
        }
        if (Container_Bits==20)
        {
            while ((Buffer_TotalBytes+Buffer_Offset_Temp)%5) // Padding in part of the AES3 block
            {
                if (Buffer_Offset_Temp+1>Buffer_Size)
                {
                    Element_WaitForMoreData();
                    return false;
                }
                if (Buffer[Buffer_Offset_Temp])
                {
                    Trusted_IsNot("Bad sync");
                    return true;
                }
                Buffer_Offset_Temp++;
            }
            while(Buffer_Offset_Temp+5<=Buffer_Size && CC5(Buffer+Buffer_Offset_Temp)==0x0000000000LL)
                Buffer_Offset_Temp+=5;
            if (Buffer_Offset_Temp+5>Buffer_Size)
            {
                Element_WaitForMoreData();
                return false;
            }
        }
        if (Container_Bits==24)
        {
            while ((Buffer_TotalBytes+Buffer_Offset_Temp)%6) // Padding in part of the AES3 block
            {
                if (Buffer_Offset_Temp+1>Buffer_Size)
                {
                    Element_WaitForMoreData();
                    return false;
                }
                if (Buffer[Buffer_Offset_Temp])
                {
                    Trusted_IsNot("Bad sync");
                    return true;
                }
                Buffer_Offset_Temp++;
            }
            while(Buffer_Offset_Temp+6<=Buffer_Size && CC6(Buffer+Buffer_Offset_Temp)==0x000000000000LL)
                Buffer_Offset_Temp+=6;
            if (Buffer_Offset_Temp+6>Buffer_Size)
            {
                Element_WaitForMoreData();
                return false;
            }
        }
        else if (Container_Bits==32)
        {
            while ((Buffer_TotalBytes+Buffer_Offset_Temp)%8) // Padding in part of the AES3 block
            {
                if (Buffer_Offset_Temp+1>Buffer_Size)
                {
                    Element_WaitForMoreData();
                    return false;
                }
                if (Buffer[Buffer_Offset_Temp])
                {
                    Trusted_IsNot("Bad sync");
                    return true;
                }
                Buffer_Offset_Temp++;
            }
            while(Buffer_Offset_Temp+8<=Buffer_Size && CC8(Buffer+Buffer_Offset_Temp)==0x0000000000000000LL)
                Buffer_Offset_Temp+=8;
            if (Buffer_Offset_Temp+8>Buffer_Size)
            {
                Element_WaitForMoreData();
                return false;
            }
        }
    }
    else
    {
        while(Buffer_Offset_Temp+NullPadding_Size<Buffer_Size && !Buffer[Buffer_Offset_Temp+NullPadding_Size])
            Buffer_Offset_Temp++;
        if (Buffer_Offset_Temp+NullPadding_Size>=Buffer_Size)
        {
            Element_WaitForMoreData();
            return false;
        }
    }

    #if MEDIAINFO_TRACE
        if (Buffer_Offset_Temp-Buffer_Offset)
        {
            Element_Size=Buffer_Offset_Temp-Buffer_Offset;
            Skip_XX(Buffer_Offset_Temp-Buffer_Offset,           "Guard band");

            GuardBand_Before+=Buffer_Offset_Temp-Buffer_Offset;
        }
    #endif // MEDIAINFO_TRACE
    Buffer_Offset=Buffer_Offset_Temp;

    // Must have enough buffer for having header
    if (Buffer_Offset+16>Buffer_Size)
        return false;

    // Quick test of synchro
    switch (Endianness)
    {
        case 'B' :
                    switch (Container_Bits)
                    {
                        case 16 :   if (CC4(Buffer+Buffer_Offset)!=0xF8724E1F) {Synched=false; return true;} break;
                        case 20 :   if (CC5(Buffer+Buffer_Offset)!=0x6F87254E1FLL) {Synched=false; return true;} break;
                        case 24 :
                                    switch (Stream_Bits)
                                    {
                                        case 16 : if (CC6(Buffer+Buffer_Offset)!=0xF872004E1F00LL) {Synched=false; return true;} break;
                                        case 20 : if (CC6(Buffer+Buffer_Offset)!=0x6F872054E1F0LL) {Synched=false; return true;} break;
                                        case 24 : if (CC6(Buffer+Buffer_Offset)!=0x96F872A54E1FLL) {Synched=false; return true;} break;
                                        default : ;
                                    }
                                    break;
                        case 32 :
                                    switch (Stream_Bits)
                                    {
                                        case 16 : if (CC8(Buffer+Buffer_Offset)!=0x0000F87200004E1FLL) {Synched=false; return true;} break;
                                        case 20 : if (CC8(Buffer+Buffer_Offset)!=0x006F87200054E1F0LL) {Synched=false; return true;} break;
                                        case 24 : if (CC8(Buffer+Buffer_Offset)!=0x0096F87200A5F41FLL) {Synched=false; return true;} break;
                                        default : ;
                                    }
                                    break;
                        default : ;
                    }
                    break;
        case 'L'  :
                    switch (Container_Bits)
                    {
                        case 16 :   if (CC4(Buffer+Buffer_Offset)!=0x72F81F4E) {Synched=false; return true;} break;
                        case 20 :   if (CC5(Buffer+Buffer_Offset)!=0x72F8F6E154LL) {Synched=false; return true;} break;
                        case 24 :
                                    switch (Stream_Bits)
                                    {
                                        case 16 : if (CC6(Buffer+Buffer_Offset)!=0x0072F8001F4ELL) {Synched=false; return true;} break;
                                        case 20 : if (CC6(Buffer+Buffer_Offset)!=0x20876FF0E154LL) {Synched=false; return true;} break;
                                        case 24 : if (CC6(Buffer+Buffer_Offset)!=0x72F8961F4EA5LL) {Synched=false; return true;} break;
                                        default : ;
                                    }
                                    break;
                        case 32 :
                                    switch (Stream_Bits)
                                    {
                                        case 16 : if (CC8(Buffer+Buffer_Offset)!=0x000072F800001F4ELL) {Synched=false; return true;} break;
                                        case 20 : if (CC8(Buffer+Buffer_Offset)!=0x0020876F00F0E154LL) {Synched=false; return true;} break;
                                        case 24 : if (CC8(Buffer+Buffer_Offset)!=0x0072F896001F4EA5LL) {Synched=false; return true;} break;
                                        default : ;
                                    }
                                    break;
                        default : ;
                    }
                    break;
        default    : ; // Should never happen
    }

    // We continue
    return true;
}

//---------------------------------------------------------------------------
void File_SmpteSt0337::Synched_Init()
{
    if (Frame_Count_NotParsedIncluded==(int64u)-1)
        Frame_Count_NotParsedIncluded=0;
    if (FrameInfo.DTS==(int64u)-1)
        FrameInfo.DTS=0;
}

//***************************************************************************
// Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_SmpteSt0337::Header_Parse()
{
    // Parsing
    int32u Size=0;
    switch (Endianness)
    {
        case 'B' :
                    switch (Container_Bits)
                    {
                        case 16 :   Size=BigEndian2int16u(Buffer+Buffer_Offset+6)         ; break;
                        case 20 :   Size=BigEndian2int24u(Buffer+Buffer_Offset+7)&0x0FFFFF; break;
                        case 24 :
                                    switch (Stream_Bits)
                                    {
                                        case 16 : Size=BigEndian2int16u(Buffer+Buffer_Offset+9)   ; break;
                                        case 20 : Size=BigEndian2int24u(Buffer+Buffer_Offset+9)>>4; break;
                                        case 24 : Size=BigEndian2int24u(Buffer+Buffer_Offset+9)   ; break;
                                        default : ;
                                    }
                                    break;
                        case 32 :
                                    switch (Stream_Bits)
                                    {
                                        case 16 : Size=BigEndian2int16u(Buffer+Buffer_Offset+0xE)   ; break;
                                        case 20 : Size=BigEndian2int24u(Buffer+Buffer_Offset+0xD)>>4; break;
                                        case 24 : Size=BigEndian2int24u(Buffer+Buffer_Offset+0xD)   ; break;
                                        default : ;
                                    }
                                    break;
                        default : ;
                    }
                    break;
        case 'L'  :
                    switch (Container_Bits)
                    {
                        case 16 :   Size=LittleEndian2int16u(Buffer+Buffer_Offset+6)   ; break;
                        case 20 :   Size=LittleEndian2int24u(Buffer+Buffer_Offset+7)>>4; break;
                        case 24 :
                                    switch (Stream_Bits)
                                    {
                                        case 16 : Size=LittleEndian2int16u(Buffer+Buffer_Offset+0xA)   ; break;
                                        case 20 : Size=LittleEndian2int24u(Buffer+Buffer_Offset+0x9)>>4; break;
                                        case 24 : Size=LittleEndian2int24u(Buffer+Buffer_Offset+0x9)   ; break;
                                        default : ;
                                    }
                                    break;
                        case 32 :
                                    switch (Stream_Bits)
                                    {
                                        case 16 : Size=LittleEndian2int16u(Buffer+Buffer_Offset+0xE)   ; break;
                                        case 20 : Size=LittleEndian2int24u(Buffer+Buffer_Offset+0xD)>>4; break;
                                        case 24 : Size=LittleEndian2int24u(Buffer+Buffer_Offset+0xD)   ; break;
                                        default : ;
                                    }
                                    break;
                        default : ;
                    }
                    break;
        default   : ; // Should never happen
    }

    // Adaptation
    if (Container_Bits!=Stream_Bits)
    {
        Size*=Container_Bits; Size/=Stream_Bits;
        GuardBand_Before*=Container_Bits; GuardBand_Before/=Stream_Bits;
    }

    // Coherency test
    if (!IsSub && !Status[IsAccepted])
    {
        size_t Offset=Buffer_Offset+(size_t)(Container_Bits*4/8+Size/8);
        while (Offset<Buffer_Size && Buffer[Offset]==0x00)
            Offset++;
        if (Offset+Container_Bits/4>Buffer_Size)
        {
            Element_WaitForMoreData();
            return;
        }
        Offset/=Container_Bits/4;
        Offset*=Container_Bits/4;
        bool IsOK=true;
        for (int8u Pos=0; Pos<Container_Bits/4; Pos++)
            if (Buffer[Buffer_Offset+Pos]!=Buffer[Offset+Pos])
            {
                IsOK=false;
                break;
            }
        if (!IsOK)
        {
            Trusted_IsNot("Bad sync");
            Buffer_Offset++;
            return;
        }
    }

    // Filling
    Header_Fill_Size(Container_Bits*4/8+Size/8);
    Header_Fill_Code(0, "AES3");

    //Guard band
    if (IsSub && FrameInfo.DTS!=(int64u)-1)
        GuardBand_After+=Element_Size-(Container_Bits*4/8+Size/8);
}

//---------------------------------------------------------------------------
void File_SmpteSt0337::Data_Parse()
{
    #if MEDIAINFO_DEMUX
        FrameInfo.PTS=FrameInfo.DTS;
        Demux_random_access=true;
        Element_Code=(int64u)-1;
    #endif //MEDIAINFO_DEMUX

    // Adapting
    const int8u* Save_Buffer=NULL;
    size_t Save_Buffer_Offset=0;
    size_t Save_Buffer_Size=0;
    int64u Save_Element_Size=0;

    if (Endianness=='L'|| Container_Bits!=Stream_Bits)
    {
        int8u* Info=new int8u[(size_t)Element_Size];
        int8u* Info_Temp=Info;

        if (Endianness=='L' && Container_Bits==16 && Stream_Bits==16)
        {
            // Source: 16LE / L1L0 L3L2 R1R0 R3R2
            // Dest  : 16BE / L3L2 L1L0 R3R2 R1R0
            while (Element_Offset+4<=Element_Size)
            {
                size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

                *(Info_Temp++)= Buffer[Buffer_Pos+1]                                    ;
                *(Info_Temp++)= Buffer[Buffer_Pos  ]                                    ;
                *(Info_Temp++)= Buffer[Buffer_Pos+3]                                    ;
                *(Info_Temp++)= Buffer[Buffer_Pos+2]                                    ;

                Element_Offset+=4;
            }
            if (Element_Offset+2<=Element_Size) // Only in half of the AES3 stream
            {
                size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

                *(Info_Temp++)= Buffer[Buffer_Pos+1]                                    ;
                *(Info_Temp++)= Buffer[Buffer_Pos  ]                                    ;

                Element_Offset+=2;
            }
        }

        if (Endianness=='L' && Container_Bits==20 && Stream_Bits==20)
        {
            // Source: 20LE / L1L0 L3L2 R0L4 R2R1 R4R3
            // Dest  : 20BE / L4L3 L2L1 L0R4 R3R2 R1R0
            while (Element_Offset+5<=Element_Size)
            {
                size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

                *(Info_Temp++)=(Buffer[Buffer_Pos+2]<<4  ) | (Buffer[Buffer_Pos+1]>>4  );
                *(Info_Temp++)=(Buffer[Buffer_Pos+1]<<4  ) | (Buffer[Buffer_Pos+0]>>4  );
                *(Info_Temp++)=(Buffer[Buffer_Pos+0]<<4  ) | (Buffer[Buffer_Pos+4]>>4  );
                *(Info_Temp++)=(Buffer[Buffer_Pos+4]<<4  ) | (Buffer[Buffer_Pos+3]>>4  );
                *(Info_Temp++)=(Buffer[Buffer_Pos+3]<<4  ) | (Buffer[Buffer_Pos+2]>>4  );

                Element_Offset+=5;
            }
        }

        if (Endianness=='L' && Container_Bits==24 && Stream_Bits==16)
        {
            // Source:        XXXX L1L0 L3L2 XXXX R1R0 R3R2
            // Dest  : 16BE / L3L2 L1L0 R3R2 R1R0
            while (Element_Offset+6<=Element_Size)
            {
                size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

                *(Info_Temp++)= Buffer[Buffer_Pos+2]                                    ;
                *(Info_Temp++)= Buffer[Buffer_Pos+1]                                    ;
                *(Info_Temp++)= Buffer[Buffer_Pos+5]                                    ;
                *(Info_Temp++)= Buffer[Buffer_Pos+4]                                    ;

                Element_Offset+=6;
            }
        }

        if (Endianness=='L' && Container_Bits==24 && Stream_Bits==20)
        {
            // Source:        L0XX L2L1 L4L3 R0XX R2R1 R4R3
            // Dest  : 20BE / L4L3 L2L1 L0R4 R3R2 R1R0
            while (Element_Offset+6<=Element_Size)
            {
                size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

                *(Info_Temp++)= Buffer[Buffer_Pos+2]                                    ;
                *(Info_Temp++)= Buffer[Buffer_Pos+1]                                    ;
                *(Info_Temp++)=(Buffer[Buffer_Pos  ]&0xF0) | (Buffer[Buffer_Pos+5]>>4  );
                *(Info_Temp++)=(Buffer[Buffer_Pos+5]<<4  ) | (Buffer[Buffer_Pos+4]>>4  );
                *(Info_Temp++)=(Buffer[Buffer_Pos+4]<<4  ) | (Buffer[Buffer_Pos+3]>>4  );

                Element_Offset+=6;
            }
        }

        if (Endianness=='L' && Container_Bits==24 && Stream_Bits==24)
        {
            // Source: 24LE / L1L0 L3L2 L5L3 R1R0 R3R2 R5R4
            // Dest  : 24BE / L5L3 L3L2 L1L0 R5R4 R3R2 R1R0
            while (Element_Offset+6<=Element_Size)
            {
                size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

                *(Info_Temp++)= Buffer[Buffer_Pos+2]                                    ;
                *(Info_Temp++)= Buffer[Buffer_Pos+1]                                    ;
                *(Info_Temp++)= Buffer[Buffer_Pos  ]                                    ;
                *(Info_Temp++)= Buffer[Buffer_Pos+5]                                    ;
                *(Info_Temp++)= Buffer[Buffer_Pos+4]                                    ;
                *(Info_Temp++)= Buffer[Buffer_Pos+3]                                    ;

                Element_Offset+=6;
            }
        }

        if (Endianness=='L' && Container_Bits==32 && Stream_Bits==16)
        {
            // Source:        XXXX XXXX L1L0 L3L2 XXXX XXXX R1R0 R3R2
            // Dest  : 16BE / L3L2 L1L0 R3R2 R1R0
            while (Element_Offset+8<=Element_Size)
            {
                size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

                *(Info_Temp++)= Buffer[Buffer_Pos+3]                                    ;
                *(Info_Temp++)= Buffer[Buffer_Pos+2]                                    ;
                *(Info_Temp++)= Buffer[Buffer_Pos+7]                                    ;
                *(Info_Temp++)= Buffer[Buffer_Pos+6]                                    ;

                Element_Offset+=8;
            }
        }
        if (Endianness=='L' && Container_Bits==32 && Stream_Bits==20)
        {
            // Source:        XXXX L0XX L2L1 L4L3 XXXX R0XX R2R1 R4R3
            // Dest  : 20BE / L4L3 L2L1 L0R4 R3R2 R1R0
            while (Element_Offset+8<=Element_Size)
            {
                size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

                *(Info_Temp++)= Buffer[Buffer_Pos+3]                                    ;
                *(Info_Temp++)= Buffer[Buffer_Pos+2]                                    ;
                *(Info_Temp++)=(Buffer[Buffer_Pos+1]&0xF0) | (Buffer[Buffer_Pos+7]>>4  );
                *(Info_Temp++)=(Buffer[Buffer_Pos+7]<<4  ) | (Buffer[Buffer_Pos+6]>>4  );
                *(Info_Temp++)=(Buffer[Buffer_Pos+6]<<4  ) | (Buffer[Buffer_Pos+5]>>4  );

                Element_Offset+=8;
            }
        }

        if (Endianness=='L' && Container_Bits==32 && Stream_Bits==24)
        {
            // Source:        XXXX L1L0 L3L2 L5L3 XXXX R1R0 R3R2 R5R4
            // Dest  : 24BE / L5L3 L3L2 L1L0 R5R4 R3R2 R1R0
            while (Element_Offset+8<=Element_Size)
            {
                size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

                *(Info_Temp++)= Buffer[Buffer_Pos+3]                                    ;
                *(Info_Temp++)= Buffer[Buffer_Pos+2]                                    ;
                *(Info_Temp++)= Buffer[Buffer_Pos+1]                                    ;
                *(Info_Temp++)= Buffer[Buffer_Pos+7]                                    ;
                *(Info_Temp++)= Buffer[Buffer_Pos+6]                                    ;
                *(Info_Temp++)= Buffer[Buffer_Pos+5]                                    ;

                Element_Offset+=8;
            }
        }

        if (Endianness=='B' && Container_Bits==24 && Stream_Bits==20)
        {
            // Source:        L4L3 L2L1 L0XX R4R3 R2R1 R0XX
            // Dest  : 20BE / L4L3 L2L1 L0R4 R3R2 R1R0
            while (Element_Offset+6<=Element_Size)
            {
                size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

                *(Info_Temp++)= Buffer[Buffer_Pos  ]                                    ;
                *(Info_Temp++)= Buffer[Buffer_Pos+1]                                    ;
                *(Info_Temp++)=(Buffer[Buffer_Pos+2]&0xF0) | (Buffer[Buffer_Pos+3]>>4  );
                *(Info_Temp++)=(Buffer[Buffer_Pos+3]<<4  ) | (Buffer[Buffer_Pos+4]>>4  );
                *(Info_Temp++)=(Buffer[Buffer_Pos+4]<<4  ) | (Buffer[Buffer_Pos+5]>>4  );

                Element_Offset+=6;
            }
        }

        Save_Buffer=Buffer;
        Save_Buffer_Offset=Buffer_Offset;
        Save_Buffer_Size=Buffer_Size;
        Save_Element_Size=Element_Size;
        File_Offset+=Buffer_Offset;
        Buffer=Info;
        Buffer_Offset=0;
        Buffer_Size=Info_Temp-Info;
        Element_Offset=0;
        Element_Size=Buffer_Size;
    }

    // Parsing
    int32u  length_code;
    Element_Begin1("Header");
        BS_Begin();
        Skip_S3(Stream_Bits,                                    "Pa");
        Skip_S3(Stream_Bits,                                    "Pb");
        Element_Begin1("Pc");
            Skip_S1( 3,                                         "data_stream_number");
            Skip_S1( 5,                                         "data_type_dependent");
            Skip_SB(                                            "error_flag");
            Info_S1( 2, data_mode,                              "data_mode"); Param_Info2(16+4*data_mode, " bits");
            Get_S1 ( 5, data_type,                              "data_type"); Param_Info1(Smpte_St0337_data_type[data_type]);
            if (Stream_Bits>16)
                Skip_S1( 4,                                     "reserved");
            if (Stream_Bits>20)
                Skip_S1( 4,                                     "reserved");
        Element_End0();
        Get_S3 (Stream_Bits, length_code,                       "length_code"); Param_Info2(length_code/8, " bytes");
        BS_End();
    Element_End0();

    if (Parser==NULL)
    {
        switch(data_type)
        {
            // SMPTE ST338
            case  1 :   // AC-3
            case 16 :   // E-AC-3 (professional)
            case 21 :   // E-AC-3 (consumer)
                        Parser=new File_Ac3();
                        ((File_Ac3*)Parser)->Frame_Count_Valid=2;
                        #if MEDIAINFO_DEMUX
                            if (Config->Demux_Unpacketize_Get())
                            {
                                Demux_UnpacketizeContainer=false; //No demux from this parser
                                Demux_Level=4; //Intermediate
                                Parser->Demux_Level=2; //Container
                                Parser->Demux_UnpacketizeContainer=true;
                            }
                        #endif //MEDIAINFO_DEMUX
                        break;
            case  4 :   // MPEG-1 Layer 1
            case  5 :   // MPEG-1 Layer 2/3, MPEG-2 Layer 1/2/3 without extension
            case  6 :   // MPEG-2 Layer 1/2/3 with extension
            case  8 :   // MPEG-2 Layer 1 low frequency
            case  9 :   // MPEG-2 Layer 2/3 low frequency
                        Parser=new File_Mpega();
                        break;
            case  7 :   // MPEG-2 AAC in ADTS
            case 19 :   // MPEG-2 AAC in ADTS low frequency
                        Parser=new File_Aac();
                        ((File_Aac*)Parser)->Mode=File_Aac::Mode_ADTS;
                        break;
            case 10 :   // MPEG-4 AAC in ADTS or LATM
            case 11 :   // MPEG-4 AAC in ADTS or LATM
                        Parser=new File_Aac();
                        break;
            case 28 :   // Dolby E
                        Parser=new File_DolbyE();
                        break;
            default : ;
        }

        if (Parser)
        {
            Open_Buffer_Init(Parser);
        }
    }

    #if MEDIAINFO_DEMUX
        if (Save_Buffer)
        {
            std::swap(Buffer, Save_Buffer);
            std::swap(Buffer_Offset, Save_Buffer_Offset);
            std::swap(Buffer_Size, Save_Buffer_Size);
            std::swap(Element_Size, Save_Element_Size);
            File_Offset-=Buffer_Offset;
        }

        if (data_type==28) //If Dolby E, we must demux the SMPTE ST 337 header too (TODO: add an option for forcing SMPTE ST 337 header)
        {
            int64u Demux_Element_Offset=Element_Offset;
            Element_Offset=0;

            if (Container_Bits==20)
            {
                //We must pad to 24 bits
                int8u*          Info2=new int8u[(size_t)Element_Size*6/5];
                size_t          Info2_Offset=0;
                const int8u*    Demux_Buffer=Buffer+Buffer_Offset;
                size_t          Demux_Buffer_Size=(size_t)Element_Size;
                size_t          Demux_Buffer_Pos=0;

                // Source: 20LE L1L0 L3L2 R0L4 R2R1 R4R3
                // Dest  :      L0XX L2L1 L4L3 R0XX R2R1 R4R3
                while (Demux_Buffer_Pos+5<=Demux_Buffer_Size)
                {
                    Info2[Info2_Offset+0]= Demux_Buffer[Demux_Buffer_Pos+0]<<4                                             ;
                    Info2[Info2_Offset+1]=(Demux_Buffer[Demux_Buffer_Pos+1]<<4  ) | (Demux_Buffer[Demux_Buffer_Pos+0]>>4  );
                    Info2[Info2_Offset+2]=(Demux_Buffer[Demux_Buffer_Pos+2]<<4  ) | (Demux_Buffer[Demux_Buffer_Pos+1]>>4  );
                    Info2[Info2_Offset+3]= Demux_Buffer[Demux_Buffer_Pos+2]&0xF0                                           ;
                    Info2[Info2_Offset+4]= Demux_Buffer[Demux_Buffer_Pos+3]                                                ;
                    Info2[Info2_Offset+5]= Demux_Buffer[Demux_Buffer_Pos+4]                                                ;

                    Info2_Offset+=6;
                    Demux_Buffer_Pos+=5;
                }

                Demux(Info2, Info2_Offset, ContentType_MainStream);

                delete[] Info2;
            }
            else
                Demux(Buffer+Buffer_Offset, (size_t)Element_Size, ContentType_MainStream);

            Element_Offset=Demux_Element_Offset;
        }
        else
            Demux(Buffer+Buffer_Offset+Container_Bits/2, (size_t)(Element_Size-Container_Bits/2), ContentType_MainStream);

        if (Save_Buffer)
        {
            File_Offset+=Buffer_Offset;
            std::swap(Buffer, Save_Buffer);
            std::swap(Buffer_Offset, Save_Buffer_Offset);
            std::swap(Buffer_Size, Save_Buffer_Size);
            std::swap(Element_Size, Save_Element_Size);
        }
     #endif //MEDIAINFO_DEMUX

    if (Parser && !Parser->Status[IsFinished])
    {
        switch(data_type)
        {
            case 28 :
                        ((File_DolbyE*)Parser)->GuardBand_Before+=GuardBand_Before;
                        ((File_DolbyE*)Parser)->GuardBand_After+=GuardBand_After;
                        break;
            default : ;
        }

        Parser->FrameInfo=FrameInfo;
        Open_Buffer_Continue(Parser, Buffer+Buffer_Offset+(size_t)Element_Offset, (size_t)(Element_Size-Element_Offset));
        Element_Offset=Element_Size;
        #if MEDIAINFO_DEMUX
            FrameInfo.DUR=Parser->FrameInfo.DUR;
            if (FrameInfo.DUR!=(int64u)-1)
                FrameInfo.DTS+=FrameInfo.DUR;
            else
                FrameInfo.DTS=(int64u)-1;
            FrameInfo.PTS=FrameInfo.DTS;
        #endif // MEDIAINFO_DEMUX
    }
    else
    {
        Skip_XX(Element_Size-Element_Offset,                    "Data");
    }

    FILLING_BEGIN();
        FrameSizes[IsSub?Buffer_Size:((GuardBand_Before+Element_Size)*Container_Bits/Stream_Bits)]++;

        Frame_Count++;
        if (Frame_Count_NotParsedIncluded!=(int64u)-1)
            Frame_Count_NotParsedIncluded++;

        if (Parser==NULL || (Frame_Count>=2 && Parser->Status[IsFinished]))
            Finish("AES3");
    FILLING_END();

    if (Save_Buffer)
    {
        delete[] Buffer;
        Buffer=Save_Buffer;
        Buffer_Offset=Save_Buffer_Offset;
        Buffer_Size=Save_Buffer_Size;
        File_Offset-=Buffer_Offset;
    }

    // Guard band
    GuardBand_Before=0;
    GuardBand_After=0;
}

//***************************************************************************
// C++
//***************************************************************************

} // NameSpace

#endif // MEDIAINFO_SMPTEST0337_YES
