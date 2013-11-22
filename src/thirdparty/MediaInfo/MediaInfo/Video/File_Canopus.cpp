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
#if defined(MEDIAINFO_CANOPUS_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Video/File_Canopus.h"
#include "MediaInfo/MediaInfo_Config_MediaInfo.h"
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Canopus::File_Canopus()
:File__Analyze()
{
    //Configuration
    ParserName=__T("Canopus");
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Canopus::Streams_Fill()
{
    Stream_Prepare(Stream_Video);
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Canopus::Read_Buffer_Continue()
{
    //Parsing
    int32u PAR_X=0, PAR_Y=0, FieldOrder=(int32u)-1;
    while (Element_Offset<Element_Size)
    {
        Element_Begin0();
        int32u FourCC;
        Get_C4 (FourCC,                                         "FourCC");
        switch (FourCC)
        {
            case 0x494E464F :   // "INFO"
                                {
                                Element_Name("Information");
                                int32u Info_Size;
                                Get_L4 (Info_Size,              "Size");
                                int64u Info_End=Element_Offset+Info_Size;
                                if (Info_Size<16 || Info_End>Element_Size)
                                {
                                    Skip_XX(Element_Size-Element_Offset, "Problem");
                                    Element_End0();
                                    return;
                                }
                                Skip_L4(                        "Unknown");
                                Skip_L4(                        "Unknown");
                                Get_L4 (PAR_X,                  "PAR_X");
                                Get_L4 (PAR_Y,                  "PAR_Y");
                                while (Element_Offset<Info_End)
                                {
                                    Element_Begin0();
                                    Get_C4 (FourCC,                                     "FourCC");
                                    switch (FourCC)
                                    {
                                        case 0x4649454C :   // "FIEL"
                                                            {
                                                            Element_Name("Field information?");
                                                            int32u FIEL_Size;
                                                            Get_L4 (FIEL_Size,          "Size");
                                                            int64u FIEL_End=Element_Offset+FIEL_Size;
                                                            if (FIEL_End>Info_End)
                                                            {
                                                                Skip_XX(Info_End-Element_Offset, "Problem");
                                                                break;
                                                            }
                                                            if (Element_Offset<FIEL_End)
                                                                Get_L4(FieldOrder,       "Field order");
                                                            while (Element_Offset<FIEL_End)
                                                                Skip_L4(                "Unknown");
                                                            }
                                                            break;
                                        case 0x52445254 :   // "RDRT"
                                                            {
                                                            Element_Name("RDRT?");
                                                            int32u RDRT_Size;
                                                            Get_L4 (RDRT_Size,          "Size");
                                                            int64u RDRT_End=Element_Offset+RDRT_Size;
                                                            if (RDRT_End>Info_End)
                                                            {
                                                                Skip_XX(Info_End-Element_Offset, "Problem");
                                                                break;
                                                            }
                                                            while (Element_Offset<RDRT_End)
                                                                Skip_L4(                "Unknown");
                                                            }
                                                            break;
                                        default:            Element_Name("Unknown");
                                                            Skip_XX(Info_End-Element_Offset, "Unknown");
                                    }
                                    Element_End0();
                                }
                                }
                                break;
            case 0x55564307 :   // "UVC" 7
                                Element_Name("Data?");
                                Skip_XX(Element_Size-Element_Offset, "Unknown");
                                break;
            default:            Element_Name("Unknown");
                                Skip_XX(Element_Size-Element_Offset, "Unknown");
        }
        Element_End0();
    }

    FILLING_BEGIN();
        if (!Status[IsAccepted])
        {
            Accept();
            Fill();

            //Info
            if (PAR_X && PAR_Y)
                Fill(Stream_Video, 0, Video_PixelAspectRatio, ((float32)PAR_X)/PAR_Y, 3);
            switch (FieldOrder)
            {
                case 0 :
                        Fill(Stream_Video, 0, Video_ScanType, "Interlaced");
                        Fill(Stream_Video, 0, Video_ScanOrder, "TFF");
                        break;
                case 1 :
                        Fill(Stream_Video, 0, Video_ScanType, "Interlaced");
                        Fill(Stream_Video, 0, Video_ScanOrder, "BFF");
                        break;
                case 2 :
                        Fill(Stream_Video, 0, Video_ScanType, "Progressive");
                        break;
                default : ;
            }

            if (Config->ParseSpeed<1.0)
                Finish();
        }
    FILLING_END();
}

//***************************************************************************
// C++
//***************************************************************************


} //NameSpace

#endif //MEDIAINFO_CANOPUS_YES
