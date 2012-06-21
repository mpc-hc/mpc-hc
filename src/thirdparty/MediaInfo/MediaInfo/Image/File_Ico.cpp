// File_Ico - Info for Icon files
// Copyright (C) 2005-2011 MediaArea.net SARL, Info@MediaArea.net
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
#if defined(MEDIAINFO_ICO_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Image/File_Ico.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Ico::File_Ico()
{
    IcoDataSize=0;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Ico::Streams_Fill()
{
    Fill(Stream_General, 0, General_Format, Type==1?"ICO":"CUR");

    for (size_t Pos=0; Pos<Streams.size(); Pos++)
    {
        Stream_Prepare(Stream_Image);
        Fill(Stream_Image, StreamPos_Last, Image_Width, Streams[Pos].Width?Streams[Pos].Width:256);
        Fill(Stream_Image, StreamPos_Last, Image_Height, Streams[Pos].Height?Streams[Pos].Height:256);
        if (Type==1)
            Fill(Stream_Image, StreamPos_Last, Image_BitDepth, Streams[Pos].BitsPerPixel);
        Fill(Stream_Image, StreamPos_Last, Image_StreamSize, Streams[Pos].Size);
    }
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Ico::FileHeader_Begin()
{
    //Element_Size
    if (Buffer_Size<4)
        return false; //Must wait for more data

    if (CC2(Buffer) || (LittleEndian2int16u(Buffer+2)!=1 && LittleEndian2int16u(Buffer+2)!=2))
    {
        Reject("ICO");
        return false;
    }

    //All should be OK...
    return true;
}

//---------------------------------------------------------------------------
void File_Ico::FileHeader_Parse()
{
    //Parsing
    Skip_L2(                                                    "Reserved");
    Get_L2 (Type,                                               "Type");
    Get_L2 (Count,                                              "Count");
}

//***************************************************************************

// Buffer - Per element
//***************************************************************************
//---------------------------------------------------------------------------
void File_Ico::Header_Parse()
{
    Header_Fill_Size(16);
    Header_Fill_Code(0, "Directory");
}

//---------------------------------------------------------------------------
void File_Ico::Data_Parse()
{
    //Parsing
    int32u Size, Offset;
    int16u BitsPerPixel;
    int8u Width, Height;
    Get_L1 (Width,                                      "Width");
    Get_L1 (Height,                                     "Height");
    Skip_L1(                                            "Colour count");
    Skip_L1(                                            "Reserved");
    Skip_L2(                                            Type==1?"Colour planes":"X hotspot");
    Get_L2 (BitsPerPixel,                               Type==1?"Bits per pixel":"Y hotspot");
    Get_L4 (Size,                                       "Size of the bitmap data");
    Get_L4 (Offset,                                     "Offset of the bitmap data");

    FILLING_BEGIN_PRECISE();
        stream Stream;
        Stream.Width=Width;
        Stream.Height=Height;
        Stream.BitsPerPixel=BitsPerPixel;
        Stream.Size=Size;
        Stream.Offset=Offset;
        Streams.push_back(Stream);

        IcoDataSize+=Size;
        if (Offset>File_Size || File_Offset+Buffer_Offset+Element_Size+IcoDataSize>File_Size)
            Reject("ICO");
        Count--;
        if (Count==0)
        {
            if (File_Offset+Buffer_Offset+Element_Size+IcoDataSize!=File_Size)
                Reject("ICO");
            else
            {
                Accept("ICO");
                Finish("ICO");
            }
        }
    FILLING_END();
}

//***************************************************************************
//
//***************************************************************************

} //NameSpace

#endif
