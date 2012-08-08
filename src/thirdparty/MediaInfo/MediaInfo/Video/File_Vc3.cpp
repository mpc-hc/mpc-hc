// File_Vc3 - Info for VC-3 streams
// Copyright (C) 2010-2012 MediaArea.net SARL, Info@MediaArea.net
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
#if defined(MEDIAINFO_VC3_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Video/File_Vc3.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const int32u Vc3_CompressedFrameSize(int32u CompressionID)
{
    switch (CompressionID)
    {
        case 1235 : return 917504;
        case 1237 : return 606208;
        case 1238 : return 917504;
        case 1241 : return 917504;
        case 1242 : return 606208;
        case 1243 : return 917504;
        case 1250 : return 458752;
        case 1251 : return 458752;
        case 1252 : return 303104;
        case 1253 : return 188416;
        default   : return 0;
    }
};

//---------------------------------------------------------------------------
const int8u Vc3_SBD(int32u SBD) //Sample Bit Depth
{
    switch (SBD)
    {
        case 1 : return  8;
        case 2 : return 10;
        default: return  0;
    }
};

//---------------------------------------------------------------------------
const char* Vc3_FFC[4]=
{
    "",
    "Progressive",
    "Interlaced",
    "Interlaced",
};

//---------------------------------------------------------------------------
const char* Vc3_SST[2]=
{
    "Progressive",
    "Interlaced",
};

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Vc3::File_Vc3()
:File__Analyze()
{
    //Configuration
    MustSynchronize=true;

    //In
    Frame_Count_Valid=2;
    FrameRate=0;

    //Temp
    Data_ToParse=0;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Vc3::Streams_Fill()
{
    //Filling
    Stream_Prepare(Stream_Video);
    Fill(Stream_Video, 0, Video_Format, "VC-3");
    Fill(Stream_Video, 0, Video_BitRate_Mode, "CBR");
    if (FrameRate)
        Fill(Stream_Video, 0, Video_BitRate, Vc3_CompressedFrameSize(CID)*8*FrameRate, 0);
    Fill(Stream_Video, 0, Video_Width, SPL);
    Fill(Stream_Video, 0, Video_Height, ALPF);
    Fill(Stream_Video, 0, Video_BitDepth, Vc3_SBD(SBD));
    Fill(Stream_Video, 0, Video_ColorSpace, "YUV");
    Fill(Stream_Video, 0, Video_ChromaSubsampling, "4:2:2");
    Fill(Stream_Video, 0, Video_ScanType, Vc3_SST[SST]);
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Vc3::Synchronize()
{
    //Synchronizing
    while (Buffer_Offset+5<=Buffer_Size && (Buffer[Buffer_Offset  ]!=0x00
                                         || Buffer[Buffer_Offset+1]!=0x00
                                         || Buffer[Buffer_Offset+2]!=0x02
                                         || Buffer[Buffer_Offset+3]!=0x80
                                         || Buffer[Buffer_Offset+4]!=0x01))
    {
        Buffer_Offset+=2;
        while (Buffer_Offset<Buffer_Size && Buffer[Buffer_Offset]!=0x00)
            Buffer_Offset+=2;
        if (Buffer_Offset>=Buffer_Size || Buffer[Buffer_Offset-1]==0x00)
            Buffer_Offset--;
    }

    //Parsing last bytes if needed
    if (Buffer_Offset+4==Buffer_Size && (Buffer[Buffer_Offset  ]!=0x00
                                      || Buffer[Buffer_Offset+1]!=0x00
                                      || Buffer[Buffer_Offset+2]!=0x02
                                      || Buffer[Buffer_Offset+3]!=0x80))
        Buffer_Offset++;
    if (Buffer_Offset+3==Buffer_Size && (Buffer[Buffer_Offset  ]!=0x00
                                      || Buffer[Buffer_Offset+1]!=0x00
                                      || Buffer[Buffer_Offset+2]!=0x02))
        Buffer_Offset++;
    if (Buffer_Offset+2==Buffer_Size && (Buffer[Buffer_Offset  ]!=0x00
                                      || Buffer[Buffer_Offset+1]!=0x00))
        Buffer_Offset++;
    if (Buffer_Offset+1==Buffer_Size &&  Buffer[Buffer_Offset  ]!=0x00)
        Buffer_Offset++;

    if (Buffer_Offset+5>Buffer_Size)
        return false;

    //Synched is OK
    Synched=true;
    return true;
}

//---------------------------------------------------------------------------
bool File_Vc3::Synched_Test()
{
    //Must have enough buffer for having header
    if (Buffer_Offset+5>Buffer_Size)
        return false;

    //Quick test of synchro
    if (Buffer[Buffer_Offset  ]!=0x00
     || Buffer[Buffer_Offset+1]!=0x00
     || Buffer[Buffer_Offset+2]!=0x02
     || Buffer[Buffer_Offset+3]!=0x80
     || Buffer[Buffer_Offset+4]!=0x01)
    {
        Synched=false;
        return true;
    }

    //We continue
    return true;
}

//***************************************************************************
// Buffer - Demux
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_DEMUX
bool File_Vc3::Demux_UnpacketizeContainer_Test()
{
    if (Buffer_Offset+0x2C>Buffer_Size)
        return false;

    int32u CompressionID=BigEndian2int32u(Buffer+Buffer_Offset+0x28);
    int32u Size=Vc3_CompressedFrameSize(CompressionID);
    Demux_Offset=Buffer_Offset+Size;

    if (Demux_Offset>Buffer_Size && File_Offset+Buffer_Size!=File_Size)
        return false; //No complete frame

    Demux_UnpacketizeContainer_Demux();

    return true;
}
#endif //MEDIAINFO_DEMUX

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Vc3::Header_Begin()
{
    if (Buffer_Offset+0x2C>Buffer_Size)
        return false;

    return true;
}

//---------------------------------------------------------------------------
void File_Vc3::Header_Parse()
{
    int32u CompressionID=BigEndian2int32u(Buffer+Buffer_Offset+0x28);

    Header_Fill_Code(0, "Frame");
    Header_Fill_Size(Vc3_CompressedFrameSize(CompressionID));
}

//---------------------------------------------------------------------------
void File_Vc3::Data_Parse()
{
    //Parsing
    Element_Info1(Frame_Count+1);
    HeaderPrefix();
    CodingControlA();
    Skip_XX(16,                                                 "Reserved");
    ImageGeometry();
    Skip_XX( 5,                                                 "Reserved");
    CompressionID();

    Skip_XX(640-Element_Offset,                                 "ToDo");
    Skip_XX(Element_Size-Element_Offset,                        "Data");

    FILLING_BEGIN();
        Data_ToParse-=Buffer_Size-(size_t)Buffer_Offset;
        Frame_Count++;
        if (!Status[IsFinished] && Frame_Count>=Frame_Count_Valid)
            Finish("VC-3");
    FILLING_END();
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Vc3::HeaderPrefix()
{
    //Parsing
    Element_Begin1("Header Prefix");
    int64u Data;
    Get_B5 (Data,                                               "Contents");
    Element_End0();

    FILLING_BEGIN();
        if (Data==0x0000028001LL)
            Accept("VC-3");
        else
            Reject("VC-3");
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Vc3::CodingControlA()
{
    //Parsing
    Element_Begin1("Coding Control A");
    BS_Begin();

    Mark_0();
    Mark_0();
    Mark_0();
    Mark_0();
    Mark_0();
    Mark_0();
    Info_S1(2, FFC,                                             "Field/Frame Count"); Param_Info1(Vc3_FFC[FFC]);

    Mark_1();
    Mark_0();
    Mark_0();
    Get_SB (   CRCF,                                             "CRC flag");
    Mark_0();
    Mark_0();
    Mark_0();
    Mark_0();

    Mark_1();
    Mark_0();
    Mark_1();
    Mark_0();
    Mark_0();
    Mark_0();
    Mark_0();
    Mark_0();

    BS_End();
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Vc3::ImageGeometry()
{
    //Parsing
    Element_Begin1("Image Geometry");
    Get_B2 (ALPF,                                               "Active lines-per-frame");
    Get_B2 (SPL,                                                "Samples-per-line");
    Skip_B1(                                                    "Zero");
    Skip_B2(                                                    "Number of active lines");
    Skip_B2(                                                    "Zero");

    BS_Begin();

    Get_S1 (3, SBD,                                             "Sample bit depth");
    Mark_1();
    Mark_1();
    Mark_0();
    Mark_0();
    Mark_0();

    Mark_1();
    Mark_0();
    Mark_0();
    Mark_0();
    Mark_1();
    Get_SB (   SST,                                             "Source scan type");
    Mark_0();
    Mark_0();

    BS_End();
    Element_End0();
}

//---------------------------------------------------------------------------
void File_Vc3::CompressionID()
{
    //Parsing
    Element_Begin1("Compression ID");
    int32u Data;
    Get_B4 (Data,                                               "Compression ID");
    Element_End0();

    FILLING_BEGIN();
        CID=Data;
        Data_ToParse=Vc3_CompressedFrameSize(Data);
        if (Data_ToParse==0)
            Reject("VC-3");
    FILLING_END();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_VC3_*
