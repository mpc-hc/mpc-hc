// File_Lagarith - Info for ProRes files
// Copyright (C) 2012-2012 MediaArea.net SARL, Info@MediaArea.net
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
#if defined(MEDIAINFO_PRORES_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Video/File_ProRes.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const char* Mpegv_colour_primaries(int8u colour_primaries);
const char* Mpegv_transfer_characteristics(int8u transfer_characteristics);
const char* Mpegv_matrix_coefficients(int8u matrix_coefficients);

//---------------------------------------------------------------------------
const char* ProRes_chrominance_factor(int8u chrominance_factor)
{
    switch (chrominance_factor)
    {
        case 0x02 : return "4:2:2";
        case 0x03 : return "4:4:4";
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* ProRes_frame_type_ScanType(int8u frame_type)
{
    switch (frame_type)
    {
        case 0x00 : return "Progressive";
        case 0x01 :
        case 0x02 : return "Interlaced";
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* ProRes_frame_type_ScanOrder(int8u frame_type)
{
    switch (frame_type)
    {
        case 0x01 : return "TFF";
        case 0x02 : return "BFF";
        default   : return "";
    }
}

//---------------------------------------------------------------------------
Ztring ProRes_creatorID(int32u creatorID)
{
    switch (creatorID)
    {
        case 0x61706C30 : return __T("Apple"); //apl0
        case 0x61727269 : return __T("Arnold & Richter Cine Technik"); //arri
        case 0x616A6130 : return __T("AJA Kona Hardware"); //aja0
        default         : return Ztring().From_CC4(creatorID);
    }
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_ProRes::File_ProRes()
:File__Analyze()
{
    //Configuration
    ParserName=__T("ProRes");
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_ProRes::Streams_Fill()
{
    Stream_Prepare(Stream_Video);
    Fill(Stream_Video, 0, Video_Format, "ProRes");
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_ProRes::Read_Buffer_Continue()
{
    //Parsing
    int32u  Name, creatorID;
    int16u  hdrSize, version, frameWidth, frameHeight;
    int8u   chrominance_factor, frame_type, primaries, transf_func, colorMatrix;
    bool    IsOk=true, luma, chroma;
    Element_Begin1("Header");
        Skip_B4(                                                "Size");
        Get_C4 (Name,                                           "Name");
    Element_End();
    Element_Begin1("Frame header");
        Get_B2 (hdrSize,                                        "hdrSize");
        Get_B2 (version,                                        "version");
        Get_C4 (creatorID,                                      "creatorID");
        Get_B2 (frameWidth,                                     "frameWidth");
        Get_B2 (frameHeight,                                    "frameHeight");
        BS_Begin();
        Get_S1 (2, chrominance_factor,                          "chrominance factor"); Param_Info1(ProRes_chrominance_factor(chrominance_factor));
        Skip_S1(2,                                              "reserved");
        Get_S1 (2, frame_type,                                  "frame type"); Param_Info1(ProRes_frame_type_ScanType(frame_type)); Param_Info1(ProRes_frame_type_ScanOrder(frame_type));
        Skip_S1(2,                                              "reserved");
        BS_End();
        Skip_B1(                                                "reserved");
        Get_B1 (primaries,                                      "primaries"); Param_Info1(Mpegv_colour_primaries(primaries));
        Get_B1 (transf_func,                                    "transf_func"); Param_Info1(Mpegv_transfer_characteristics(transf_func));
        Get_B1 (colorMatrix,                                    "colorMatrix"); Param_Info1(Mpegv_matrix_coefficients(colorMatrix));
        BS_Begin();
        Skip_S1(4,                                              "src_pix_fmt");
        Skip_S1(4,                                              "alpha_info");
        BS_End();
        Skip_B1(                                                "reserved");
        BS_Begin();
        Skip_S1(6,                                              "reserved");
        Get_SB (luma,                                           "custom luma quant matrix present");
        Get_SB (chroma,                                         "custom chroma quant matrix present");
        BS_End();
        if (luma)
            Skip_XX(64,                                         "QMatLuma");
        if (chroma)
            Skip_XX(64,                                         "QMatChroma");
    Element_End();
    if (Element_Offset!=8+hdrSize)
        IsOk=false;
    Skip_XX(Element_Size-Element_Offset,                        "Picture layout");

    FILLING_BEGIN();
        if (IsOk && Name==0x69637066 && !Status[IsAccepted]) //icpf
        {
            Accept();
            Fill();

            Fill(Stream_Video, 0, Video_Format_Version, __T("Version ")+Ztring::ToZtring(version));
            Fill(Stream_Video, 0, Video_Width, frameWidth);
            Fill(Stream_Video, 0, Video_Height, frameHeight);
            Fill(Stream_Video, 0, Video_Encoded_Library, ProRes_creatorID(creatorID));
            Fill(Stream_Video, 0, Video_ChromaSubsampling, ProRes_chrominance_factor(chrominance_factor));
            Fill(Stream_Video, 0, Video_ScanType, ProRes_frame_type_ScanType(frame_type));
            Fill(Stream_Video, 0, Video_ScanOrder, ProRes_frame_type_ScanOrder(frame_type));
            Fill(Stream_Video, 0, "colour_primaries", Mpegv_colour_primaries(primaries));
            Fill(Stream_Video, 0, "transfer_characteristics", Mpegv_transfer_characteristics(transf_func));
            Fill(Stream_Video, 0, "matrix_coefficients", Mpegv_matrix_coefficients(colorMatrix));

            Finish();
        }
    FILLING_END();
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_PRORES_YES
