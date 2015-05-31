/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Information about DDS (DirectDraw Surface) files
//
// From http://msdn.microsoft.com/en-us/library/windows/desktop/bb943982%28v=vs.85%29.aspx
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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
#if defined(MEDIAINFO_DDS_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Image/File_Dds.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Dds::File_Dds()
{
    //Config
    #if MEDIAINFO_TRACE
        Trace_Layers_Update(8); //Stream
    #endif //MEDIAINFO_TRACE
    IsRawStream=true;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Dds::Streams_Accept()
{
    Fill(Stream_General, 0, General_Format, "DDS");

    if (!IsSub)
    {
        TestContinuousFileNames();

        Stream_Prepare((Config->File_Names.size()>1 || Config->File_IsReferenced_Get())?Stream_Video:Stream_Image);
        Fill(StreamKind_Last, StreamPos_Last, "StreamSize", File_Size);
        if (StreamKind_Last==Stream_Video)
            Fill(Stream_Video, StreamPos_Last, Video_FrameCount, Config->File_Names.size());
        if (pfFlags&0x4) //DDPF_FOURCC
            CodecID_Fill(Ztring().From_CC4(FourCC), StreamKind_Last, StreamPos_Last, InfoCodecID_Format_Riff, Stream_Video);
        if (Flags&0x2) //DDSD_HEIGHT
            Fill(StreamKind_Last, 0, "Height", Height);
        if (Flags&0x4) //DDSD_WIDTH
            Fill(StreamKind_Last, 0, "Width", Width);
        if (Flags&0x800000) //DDSD_DEPTH
            Fill(StreamKind_Last, 0, "BitDepth", Depth);
    }
    else
        Stream_Prepare(StreamKind_Last);
}

//***************************************************************************
// Header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Dds::FileHeader_Begin()
{
    // Minimum buffer size
    if (Buffer_Size<8)
        return false; // Must wait for more data

    // Testing
    if (Buffer[0]!=0x44 // "DDS "
     || Buffer[1]!=0x44
     || Buffer[2]!=0x53
     || Buffer[3]!=0x20
     || LittleEndian2int32u(Buffer+4)<124)
    {
        Reject();
        return false;
    }

    //All should be OK...
    return true;
}

//---------------------------------------------------------------------------
void File_Dds::FileHeader_Parse()
{
    //Parsing
    int32u Size;
    Skip_C4(                                                    "Magic");
    Get_L4 (Size,                                               "Size");
    Get_L4 (Flags,                                              "Flags");
    Get_L4 (Height,                                             "Height");
    Get_L4 (Width,                                              "Width");
    Skip_L4(                                                    "PitchOrLinearSize");
    Skip_L4(                                                    "Depth");
    Skip_L4(                                                    "MipMapCount");
    Skip_XX(4*11,                                               "Reserved1");
    Element_Begin1("Pixel format");
        int32u pf_Size;
        Get_L4 (pf_Size,                                        "Size");
        if (pf_Size>=32)
        {
            Get_L4 (pfFlags,                                    "Flags");
            Get_C4 (FourCC,                                     "FourCC");
            Skip_L4(                                            "RGBBitCount");
            Skip_L4(                                            "RBitMask");
            Skip_L4(                                            "GBitMask");
            Skip_L4(                                            "BBitMask");
            Skip_L4(                                            "ABitMask");
            if (pf_Size>32)
                Skip_XX(Size-32,                                "(Data)");
        }
        else if (pf_Size>4)
            Skip_XX(pf_Size-4,                                  "");
    Element_End0();
    Skip_L4(                                                    "Caps");
    Skip_L4(                                                    "Caps2");
    Skip_L4(                                                    "Caps3");
    Skip_L4(                                                    "Caps4");
    Skip_L4(                                                    "Reserved2");
    if (Size>124)
        Skip_XX(Size-124,                                       "(Data)");
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Dds::Read_Buffer_Unsynched()
{
    Read_Buffer_Unsynched_OneFramePerFile();
}

//---------------------------------------------------------------------------
void File_Dds::Read_Buffer_Continue()
{
    Skip_XX(File_Size-(File_Offset+Buffer_Offset),              "Data");

    FILLING_BEGIN();
        Frame_Count++;
        if (Frame_Count_NotParsedIncluded!=(int64u)-1)
            Frame_Count_NotParsedIncluded++;
        if (!Status[IsAccepted])
        {
            Accept();
            Fill();
            if (Config->ParseSpeed<1.0)
                Finish();
        }
    FILLING_END();
}

} //NameSpace

#endif
