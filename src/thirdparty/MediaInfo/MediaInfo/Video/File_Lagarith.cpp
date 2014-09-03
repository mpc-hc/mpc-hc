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
    ParserName=__T("Lagarith");
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
            case 0x02 :
            case 0x04 : Fill(Stream_Video, 0, Video_ColorSpace, "RGB"); Fill(Stream_Video, 0, Video_BitDepth, 8); break;
            case 0x03 : Fill(Stream_Video, 0, Video_ColorSpace, "YUV"); Fill(Stream_Video, 0, Video_ChromaSubsampling, "4:2:2"); Fill(Stream_Video, 0, Video_BitDepth, 8); break;
            case 0x05 : Fill(Stream_Video, 0, Video_ColorSpace, "Y"); break;
            case 0x06 :
            case 0x07 : Fill(Stream_Video, 0, Video_ColorSpace, "RGB"); break;
            case 0x08 :
            case 0x09 : Fill(Stream_Video, 0, Video_ColorSpace, "RGBA"); break;
            case 0x0A :
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
