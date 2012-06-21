// File_Lagarith - Info for Lagarith  files
// Copyright (C) 2010-2011 MediaArea.net SARL, Info@MediaArea.net
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
#if defined(MEDIAINFO_LAGARITH_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Video/File_Lagarith.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Lagarith::File_Lagarith()
:File__Analyze()
{
    //Configuration
    ParserName=_T("Lagarith");
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Lagarith::Streams_Fill()
{
    Stream_Prepare(Stream_Video);
    Fill(Stream_Video, 0, Video_Format, "Lagarith");
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Lagarith::Read_Buffer_Continue()
{
    //Parsing
    int8u version;
    Get_L1 (version,                                            "version");
    Skip_XX(Element_Size-Element_Offset,                        "data");

    FILLING_BEGIN();
        Accept();
        Fill();
        switch (version)
        {
            case 0x02 : Fill(Stream_Video, 0, Video_ColorSpace, "RGB"); Fill(Stream_Video, 0, Video_BitDepth, 8); break;
            case 0x03 : Fill(Stream_Video, 0, Video_ColorSpace, "YUV"); Fill(Stream_Video, 0, Video_ChromaSubsampling, "4:2:2"); Fill(Stream_Video, 0, Video_BitDepth, 8); break;
            case 0x04 : Fill(Stream_Video, 0, Video_ColorSpace, "RGB"); Fill(Stream_Video, 0, Video_BitDepth, 8); break;
            case 0x05 : Fill(Stream_Video, 0, Video_ColorSpace, "Grey"); break;
            case 0x06 : Fill(Stream_Video, 0, Video_ColorSpace, "RGB"); break;
            case 0x07 : Fill(Stream_Video, 0, Video_ColorSpace, "RGB"); break;
            case 0x08 : Fill(Stream_Video, 0, Video_ColorSpace, "RGBA"); break;
            case 0x09 : Fill(Stream_Video, 0, Video_ColorSpace, "RGBA"); break;
            case 0x0A : Fill(Stream_Video, 0, Video_ColorSpace, "YUV"); Fill(Stream_Video, 0, Video_ChromaSubsampling, "4:2:0"); Fill(Stream_Video, 0, Video_BitDepth, 8); break;
            case 0x0B : Fill(Stream_Video, 0, Video_ColorSpace, "YUV"); Fill(Stream_Video, 0, Video_ChromaSubsampling, "4:2:0"); Fill(Stream_Video, 0, Video_BitDepth, 8); break;
            default   : ;
        }

    FILLING_END();
    Finish();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_LAGARITH_YES
