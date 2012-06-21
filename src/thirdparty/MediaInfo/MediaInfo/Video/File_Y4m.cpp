// File_Y4m - Info for YUV4MPEG2 files
// Copyright (C) 2011-2011 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
//
// Source:
// http://wiki.multimedia.cx/index.php?title=YUV4MPEG2
//
//---------------------------------------------------------------------------

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
#if defined(MEDIAINFO_Y4M_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Video/File_Y4m.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
namespace MediaInfoLib
{
//---------------------------------------------------------------------------

//***************************************************************************
// Infos
//***************************************************************************

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Y4m::File_Y4m()
:File__Analyze()
{
    //Configuration
    ParserName=_T("YUV4MPEG2");
    IsRawStream=true;
    Frame_Count_NotParsedIncluded=0;

    //Temp
    HeaderEnd=0;
}

//---------------------------------------------------------------------------
File_Y4m::~File_Y4m()
{
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Y4m::Streams_Accept()
{
    Fill(Stream_General, 0, General_Format, "YUV4MPEG2");

    Stream_Prepare(Stream_Video);
    Fill(Stream_Video, 0, Video_Format, "YUV");
    Fill(Stream_Video, 0, Video_ColorSpace, "YUV");
}

//---------------------------------------------------------------------------
void File_Y4m::Streams_Fill()
{
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Y4m::FileHeader_Begin()
{
    if (Buffer_Size<10)
        return false;

    if (!(Buffer[0]==0x59
       && Buffer[1]==0x55
       && Buffer[2]==0x56
       && Buffer[3]==0x34
       && Buffer[4]==0x4D
       && Buffer[5]==0x50
       && Buffer[6]==0x45
       && Buffer[7]==0x47
       && Buffer[8]==0x32
       && Buffer[9]==0x20))
    {
        Reject();
        return false;
    }

    for (; HeaderEnd<Buffer_Size; HeaderEnd++)
    {    
        if (Buffer[HeaderEnd]==0x0A)
        {
            Accept();
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------------------------
void File_Y4m::FileHeader_Parse()
{
    //Parsing
    Ztring Header;
    Get_Local(HeaderEnd, Header,                                "Data");

    ZtringList List; List.Separator_Set(0, " ");
    List.Write(Header);
    int64u Width=0, Height=0, Multiplier=0, Divisor=1;
    float64 FrameRate=0;
    for (size_t Pos=1; Pos<List.size(); Pos++)
    {
        if (!List[Pos].empty())
        {
            switch (List[Pos][0])
            {
                case 'A' :  //Pixel aspect ratio
                            {
                            ZtringList Value; Value.Separator_Set(0, ":");
                            Value.Write(List[Pos].substr(1));
                            if (Value.size()==2)
                            {
                                float64 x=Value[0].To_float64();
                                float64 y=Value[1].To_float64();
                                if (x && y)
                                    Fill(Stream_Video, 0, Video_PixelAspectRatio, x/y, 3);
                            }
                            }
                            break;
                case 'C' :  //Color space
                            if (List[Pos]==_T("C420jpeg") || List[Pos]==_T("C420paldv") || List[Pos]==_T("C420"))
                            {
                                Fill(Stream_Video, 0, Video_ChromaSubsampling, "4:2:0");
                                Multiplier=3;
                                Divisor=2;
                            }
                            if (List[Pos]==_T("C422"))
                            {
                                Fill(Stream_Video, 0, Video_ChromaSubsampling, "4:2:2");
                                Multiplier=2;
                            }
                            if (List[Pos]==_T("C444"))
                            {
                                Fill(Stream_Video, 0, Video_ChromaSubsampling, "4:4:4");
                                Multiplier=3;
                            }
                            break;
                case 'F' :  //Frame rate
                            {
                            ZtringList Value; Value.Separator_Set(0, ":");
                            Value.Write(List[Pos].substr(1));
                            if (Value.size()==2)
                            {
                                float64 N=Value[0].To_float64();
                                float64 D=Value[1].To_float64();
                                if (N && D)
                                {
                                    FrameRate=N/D;
                                    Fill(Stream_Video, 0, Video_FrameRate, FrameRate, 3);
                                }
                            }
                            }
                            break;
                case 'H' :  //Height
                            {
                            Ztring Value=List[Pos].substr(1);
                            Height=Value.To_int64u();
                            Fill(Stream_Video, 0, Video_Height, Height); break;
                            }
                            break;
                case 'I' :  //Interlacing
                            if (List[Pos].size()==2)
                                switch (List[Pos][1])
                                {
                                    case 'p' : Fill(Stream_Video, 0, Video_ScanType, "Progressive"); break;
                                    case 't' : Fill(Stream_Video, 0, Video_ScanType, "Progressive"); Fill(Stream_Video, 0, Video_ScanOrder, "TFF"); break;
                                    case 'b' : Fill(Stream_Video, 0, Video_ScanType, "Progressive"); Fill(Stream_Video, 0, Video_ScanOrder, "BFF"); break;
                                    case 'm' : Fill(Stream_Video, 0, Video_ScanType, "Mixed"); break;
                                    default  : ;
                                }
                            break;
                case 'W' :  //Width
                            {
                            Ztring Value=List[Pos].substr(1);
                            Width=Value.To_int64u();
                            Fill(Stream_Video, 0, Video_Width, Width); break;
                            }
                            break;
                default  : ;
            }
        }
    }

    //Duration (we expect no meta per frame)
    if (Width && Height && Multiplier)
    {
        int64u Frame_ByteSize=6+Width*Height*Multiplier/Divisor; //5 for "FRAME\0"
        Fill(Stream_Video, 0, Video_FrameCount, File_Size/Frame_ByteSize);
        if (FrameRate)
            Fill(Stream_Video, 0, Video_BitRate, Width*Height*Multiplier/Divisor*8*FrameRate);
    }

    Finish();
}

} //NameSpace

#endif //MEDIAINFO_Y4M_YES
