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
#if defined(MEDIAINFO_CAF_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Caf.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//***************************************************************************
// Constants
//***************************************************************************

//---------------------------------------------------------------------------
namespace Elements
{
    const int64u data=0x64617461;
    const int64u desc=0x64657363;
    const int64u free=0x66726565;
    const int64u info=0x696E666F;
    const int64u kuki=0x6B756B69;
    const int64u pakt=0x70616B74;
    const int64u uuid=0x75756964;
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Caf::File_Caf()
:File__Analyze()
{
    DataMustAlwaysBeComplete=false;
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Caf::FileHeader_Begin()
{
    //Synchro
    if (3>Buffer_Size)
        return false;
    if (Buffer[0]!=0x63 //"caff"
     || Buffer[1]!=0x61
     || Buffer[2]!=0x66
     || Buffer[3]!=0x66)
    {
        Reject();
        return false;
    }
    if (8>Buffer_Size)
        return false;

    return true;
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Caf::FileHeader_Parse()
{
    //Parsing
    int16u FileVersion;
    Skip_C4(                                                    "FileType");
    Get_B2 (FileVersion,                                        "FileVersion");
    Skip_B2(                                                    "FileFlags");

    FILLING_BEGIN();
        Accept();
        Fill(Stream_General, 0, General_Format, "CAF");
        Fill(Stream_General, 0, General_Format_Version, __T("Version ")+Ztring::ToZtring(FileVersion));
        Stream_Prepare(Stream_Audio);

        if (FileVersion!=1)
            Finish(); //Version 0 or 2+ are not supported
    FILLING_END();
}

//***************************************************************************
// Buffer
//***************************************************************************

//---------------------------------------------------------------------------
void File_Caf::Header_Parse()
{
    //Parsing
    int64u  ChunkSize;
    int32u  ChunkType;
    Get_B4 (ChunkType,                                          "ChunkType");
    Get_B8(ChunkSize,                                           "ChunkSize");

    //Filling
    Header_Fill_Code2(ChunkType, Ztring().From_CC4(ChunkType));
    Header_Fill_Size(12+ChunkSize);
}

//---------------------------------------------------------------------------
void File_Caf::Data_Parse()
{
    if (Element_Code!=Elements::data && !Element_IsComplete_Get())
    {
        Element_WaitForMoreData();
        return;
    }

    #define ELEMENT_CASE(_NAME, _DETAIL) \
        case Elements::_NAME : Element_Name(_DETAIL); _NAME(); break;

    //Parsing
    switch (Element_Code)
    {
        ELEMENT_CASE(data, "Audio Data");
        ELEMENT_CASE(desc, "Audio Description");
        ELEMENT_CASE(free, "Free");
        ELEMENT_CASE(info, "Information");
        ELEMENT_CASE(kuki, "Magic Cookie");
        ELEMENT_CASE(pakt, "Packet Table");
        ELEMENT_CASE(uuid, "User-Defined Chunk");
        default :
            Skip_XX(Element_Size,                               "Data");
    }
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Caf::data()
{
    //Parsing
    Skip_XX(Element_Size,                                       "Data");

    Fill(Stream_Audio, 0, Retrieve(Stream_Audio, 0, Audio_Source_Duration).empty()?Audio_StreamSize:Audio_Source_StreamSize, Element_Size);

    //TODO: put this code in the common section Streams_Finish_StreamOnly()
    int64u BitRate=Retrieve(Stream_Audio, 0, "BitRate").To_int64u();
    if (BitRate && Element_Size && Retrieve(Stream_Audio, 0, Audio_Source_Duration).empty() && Retrieve(Stream_Audio, 0, Audio_Duration).empty())
        Fill(Stream_Audio, 0, Audio_Duration, Element_Size*8*1000/BitRate);
}

//---------------------------------------------------------------------------
void File_Caf::desc()
{
    //Parsing
    float64 SampleRate;
    int32u  FormatID, FormatFlags, BytesPerPacket, FramesPerPacket, ChannelsPerFrame, BitsPerChannel;
    Get_BF8(SampleRate,                                         "SampleRate");
    Get_C4 (FormatID,                                           "FormatID");
    Get_B4 (FormatFlags,                                        "FormatFlags");
    Get_B4 (BytesPerPacket,                                     "BytesPerPacket");
    Get_B4 (FramesPerPacket,                                    "FramesPerPacket");
    Get_B4 (ChannelsPerFrame,                                   "ChannelsPerFrame");
    Get_B4 (BitsPerChannel,                                     "BitsPerChannel");

    FILLING_BEGIN();
        if (SampleRate)
            Fill(Stream_Audio, 0, Audio_SamplingRate, SampleRate);
        CodecID_Fill(Ztring().From_CC4(FormatID), Stream_Audio, 0, InfoCodecID_Format_Mpeg4);
        if (ChannelsPerFrame)
            Fill(Stream_Audio, 0, Audio_Channel_s_, ChannelsPerFrame);
        if (BitsPerChannel)
            Fill(Stream_Audio, 0, Audio_BitDepth, BitsPerChannel);
        if (BytesPerPacket && SampleRate && FramesPerPacket)
            Fill(Stream_Audio, 0, Audio_BitRate, SampleRate*BytesPerPacket*8/FramesPerPacket);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Caf::free()
{
    //Parsing
    Skip_XX(Element_Size,                                       "Junk");
}

//---------------------------------------------------------------------------
void File_Caf::info()
{
    if (Element_Size<4)
        return;

    //Parsing
    int32u NumEntries;
    Get_B4 (NumEntries,                                         "NumEntries");
    ZtringList List;
    std::map<Ztring, Ztring> ListList;
    const int8u* Buffer_Max = Buffer+(size_t)(Buffer_Offset+Element_Size);
    while (Element_Offset<Element_Size)
    {
        const int8u* Buffer_Begin = Buffer+(size_t)(Buffer_Offset+Element_Offset);
        const int8u* Buffer_Middle = Buffer_Begin;
        while (Buffer_Middle<Buffer_Max && *Buffer_Middle)
            ++Buffer_Middle;
        const int8u* Buffer_End = Buffer_Middle + 1;
        while (Buffer_End<Buffer_Max && *Buffer_End)
            ++Buffer_End;

        Ztring Key, Value;
        Get_UTF8(Buffer_Middle-Buffer_Begin, Key,               "Key");
        Skip_B1 (                                               "Zero");
        Get_UTF8(Buffer_End-(Buffer_Middle+1), Value,           "Value");
        if (Buffer_End!=Buffer_Max)
            Skip_B1 (                                           "Zero");

        ListList[Key]=Value;
    }

    if (ListList.size()!=NumEntries)
        return;

    for (std::map<Ztring, Ztring>::iterator Item=ListList.begin(); Item!=ListList.end(); ++Item)
        Fill(Stream_General, 0, Item->first.To_UTF8().c_str(), Item->second);
}

//---------------------------------------------------------------------------
void File_Caf::kuki()
{
    //Parsing
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Caf::pakt()
{
    //Parsing
    int64u  NumberPackets, NumberValidFrames;
    int32u  PrimingFrames, RemainderFrames;
    Get_B8 (NumberPackets,                                      "NumberPackets");
    Get_B8 (NumberValidFrames,                                  "NumberValidFrames");
    Get_B4 (PrimingFrames,                                      "PrimingFrames");
    Get_B4 (RemainderFrames,                                    "RemainderFrames");
    Skip_XX(Element_Size-Element_Offset,                        "Packet sizes");

    FILLING_BEGIN();
        float64 SampleRate=Retrieve(Stream_Audio, 0, Audio_SamplingRate).To_float64();
        Fill(Stream_Audio, 0, Audio_FrameCount, NumberPackets);
        Fill(Stream_Audio, 0, Audio_Duration, NumberValidFrames/SampleRate*1000, 0);
        if (PrimingFrames && RemainderFrames)
            Fill(Stream_Audio, 0, Audio_Source_Duration, (PrimingFrames+NumberValidFrames+RemainderFrames)/SampleRate*1000, 0);
        Fill(Stream_Audio, 0, Audio_Delay, PrimingFrames/SampleRate*1000, 0);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Caf::uuid()
{
    //Parsing
    Skip_UUID(                                                  "UUID");
    Skip_XX(Element_Size-Element_Offset,                        "Data");
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_CAF_YES
