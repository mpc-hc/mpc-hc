// File_Jpeg - Info for NewFormat files
// Copyright (C) 2005-2010 MediaArea.net SARL, Info@MediaArea.net
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

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Links
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// http://www.fileformat.info/format/jpeg/
// http://park2.wakwak.com/~tsuruzoh/Computer/Digicams/exif-e.html
// http://www.w3.org/Graphics/JPEG/jfif3.pdf
// http://www.sentex.net/~mwandel/jhead/
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// Compilation conditions
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_JPEG_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Image/File_Jpeg.h"
#include "ZenLib/Utils.h"
#include <vector>
using namespace ZenLib;
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constants
//***************************************************************************

//---------------------------------------------------------------------------
namespace Elements
{
    const int16u TEM =0xFF01;
    const int16u SOC =0xFF4F; //JPEG 2000
    const int16u SIZ =0xFF51; //JPEG 2000
    const int16u COD =0xFF52; //JPEG 2000
    const int16u COC =0xFF53; //JPEG 2000
    const int16u QCD =0xFF5C; //JPEG 2000
    const int16u QCC =0xFF5D; //JPEG 2000
    const int16u RGN =0xFF5E; //JPEG 2000
    const int16u SOT =0xFF90; //JPEG 2000
    const int16u SOD =0xFF93; //JPEG 2000
    const int16u S0F0=0xFFC0;
    const int16u S0F1=0xFFC1;
    const int16u S0F2=0xFFC2;
    const int16u S0F3=0xFFC3;
    const int16u DHT =0xFFC4;
    const int16u S0F5=0xFFC5;
    const int16u S0F6=0xFFC6;
    const int16u S0F7=0xFFC7;
    const int16u JPG =0xFFC8;
    const int16u S0F9=0xFFC9;
    const int16u S0FA=0xFFCA;
    const int16u S0FB=0xFFCB;
    const int16u DAC =0xFFCC;
    const int16u S0FD=0xFFCD;
    const int16u S0FE=0xFFCE;
    const int16u S0FF=0xFFCF;
    const int16u RST0=0xFFD0;
    const int16u RST1=0xFFD1;
    const int16u RST2=0xFFD2;
    const int16u RST3=0xFFD3;
    const int16u RST4=0xFFD4;
    const int16u RST5=0xFFD5;
    const int16u RST6=0xFFD6;
    const int16u RST7=0xFFD7;
    const int16u SOI =0xFFD8;
    const int16u EOI =0xFFD9; //EOC in JPEG 2000
    const int16u SOS =0xFFDA;
    const int16u DQT =0xFFDB;
    const int16u DNL =0xFFDC;
    const int16u DRI =0xFFDD;
    const int16u DHP =0xFFDE;
    const int16u EXP =0xFFDF;
    const int16u APP0=0xFFE0;
    const int16u APP1=0xFFE1;
    const int16u APP2=0xFFE2;
    const int16u APP3=0xFFE3;
    const int16u APP4=0xFFE4;
    const int16u APP5=0xFFE5;
    const int16u APP6=0xFFE6;
    const int16u APP7=0xFFE7;
    const int16u APP8=0xFFE8;
    const int16u APP9=0xFFE9;
    const int16u APPA=0xFFEA;
    const int16u APPB=0xFFEB;
    const int16u APPC=0xFFEC;
    const int16u APPD=0xFFED;
    const int16u APPE=0xFFEE;
    const int16u APPF=0xFFEF;
    const int16u JPG0=0xFFF0;
    const int16u JPG1=0xFFF1;
    const int16u JPG2=0xFFF2;
    const int16u JPG3=0xFFF3;
    const int16u JPG4=0xFFF4;
    const int16u JPG5=0xFFF5;
    const int16u JPG6=0xFFF6;
    const int16u JPG7=0xFFF7;
    const int16u JPG8=0xFFF8;
    const int16u JPG9=0xFFF9;
    const int16u JPGA=0xFFFA;
    const int16u JPGB=0xFFFB;
    const int16u JPGC=0xFFFC;
    const int16u JPGD=0xFFFD;
    const int16u COM =0xFFFE;
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Jpeg::File_Jpeg()
{
    //In
    StreamKind=Stream_Image;

    //Temp
    Height_Multiplier=1;
}

//***************************************************************************
// Static stuff
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Jpeg::FileHeader_Begin()
{
    //Element_Size
    if (Buffer_Size<2)
        return false; //Must wait for more data

    if (CC2(Buffer)!=Elements::SOI
     && CC2(Buffer)!=Elements::SOC)
    {
        Reject("JPEG");
        return false;
    }

    //All should be OK...
    return true;
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Jpeg::Header_Parse()
{
    //Parsing
    int16u code, size;
    Get_B2 (code,                                               "Marker");
    switch (code)
    {
        case Elements::TEM :
        case Elements::RST0 :
        case Elements::RST1 :
        case Elements::RST2 :
        case Elements::RST3 :
        case Elements::RST4 :
        case Elements::RST5 :
        case Elements::RST6 :
        case Elements::RST7 :
        case Elements::SOC  :
        case Elements::SOD  :
        case Elements::SOI  :
        case Elements::EOI  :
                    size=0; break;
        default   : Get_B2 (size,                                  "Fl - Frame header length");
    }

    //Filling
    Header_Fill_Code(code, Ztring().From_CC2(code));
    Header_Fill_Size(2+size);
}

//---------------------------------------------------------------------------
void File_Jpeg::Data_Parse()
{
    #define CASE_INFO(_NAME, _DETAIL) \
        case Elements::_NAME : Element_Info(#_NAME); Element_Info(_DETAIL); _NAME(); break;

    //Parsing
    switch (Element_Code)
    {
        CASE_INFO(TEM ,                                         "TEM");
        CASE_INFO(SOC ,                                         "Start of codestream"); //JPEG 2000
        CASE_INFO(SIZ ,                                         "Image and tile size"); //JPEG 2000
        CASE_INFO(COD ,                                         "Coding style default"); //JPEG 2000
        CASE_INFO(COC ,                                         "Coding style component"); //JPEG 2000
        CASE_INFO(QCD ,                                         "Quantization default"); //JPEG 2000
        CASE_INFO(QCC ,                                         "Quantization component "); //JPEG 2000
        CASE_INFO(RGN ,                                         "Region-of-interest"); //JPEG 2000
        CASE_INFO(SOT ,                                         "Start of tile-part"); //JPEG 2000
        CASE_INFO(SOD ,                                         "Start of data"); //JPEG 2000
        CASE_INFO(S0F0,                                         "Baseline DCT (Huffman)");
        CASE_INFO(S0F1,                                         "Extended sequential DCT (Huffman)");
        CASE_INFO(S0F2,                                         "Progressive DCT (Huffman)");
        CASE_INFO(S0F3,                                         "Lossless (sequential) (Huffman)");
        CASE_INFO(DHT ,                                         "Define Huffman Tables");
        CASE_INFO(S0F5,                                         "Differential sequential DCT (Huffman)");
        CASE_INFO(S0F6,                                         "Differential progressive DCT (Huffman)");
        CASE_INFO(S0F7,                                         "Differential lossless (sequential) (Huffman)");
        CASE_INFO(JPG ,                                         "Reserved for JPEG extensions");
        CASE_INFO(S0F9,                                         "Extended sequential DCT (Arithmetic)");
        CASE_INFO(S0FA,                                         "Progressive DCT (Arithmetic)");
        CASE_INFO(S0FB,                                         "Lossless (sequential) (Arithmetic)");
        CASE_INFO(DAC ,                                         "Define Arithmetic Coding");
        CASE_INFO(S0FD,                                         "Differential sequential DCT (Arithmetic)");
        CASE_INFO(S0FE,                                         "Differential progressive DCT (Arithmetic)");
        CASE_INFO(S0FF,                                         "Differential lossless (sequential) (Arithmetic)");
        CASE_INFO(RST0,                                         "Restart Interval Termination 0");
        CASE_INFO(RST1,                                         "Restart Interval Termination 1");
        CASE_INFO(RST2,                                         "Restart Interval Termination 2");
        CASE_INFO(RST3,                                         "Restart Interval Termination 3");
        CASE_INFO(RST4,                                         "Restart Interval Termination 4");
        CASE_INFO(RST5,                                         "Restart Interval Termination 5");
        CASE_INFO(RST6,                                         "Restart Interval Termination 6");
        CASE_INFO(RST7,                                         "Restart Interval Termination 7");
        CASE_INFO(SOI ,                                         "Start Of Image");
        CASE_INFO(EOI ,                                         "End Of Image"); //Is EOC (End of codestream) in JPEG 2000
        CASE_INFO(SOS ,                                         "Start Of Scan");
        CASE_INFO(DQT ,                                         "Define Quantization Tables");
        CASE_INFO(DNL ,                                         "Define Number of Lines");
        CASE_INFO(DRI ,                                         "Define Restart Interval");
        CASE_INFO(DHP ,                                         "Define Hierarchical Progression");
        CASE_INFO(EXP ,                                         "Expand Reference Components");
        CASE_INFO(APP0,                                         "Application-specific marker 0");
        CASE_INFO(APP1,                                         "Application-specific marker 1");
        CASE_INFO(APP2,                                         "Application-specific marker 2");
        CASE_INFO(APP3,                                         "Application-specific marker 3");
        CASE_INFO(APP4,                                         "Application-specific marker 4");
        CASE_INFO(APP5,                                         "Application-specific marker 5");
        CASE_INFO(APP6,                                         "Application-specific marker 6");
        CASE_INFO(APP7,                                         "Application-specific marker 7");
        CASE_INFO(APP8,                                         "Application-specific marker 8");
        CASE_INFO(APP9,                                         "Application-specific marker 9");
        CASE_INFO(APPA,                                         "Application-specific marker 10");
        CASE_INFO(APPB,                                         "Application-specific marker 11");
        CASE_INFO(APPC,                                         "Application-specific marker 12");
        CASE_INFO(APPD,                                         "Application-specific marker 13");
        CASE_INFO(APPE,                                         "Application-specific marker 14");
        CASE_INFO(APPF,                                         "Application-specific marker 15");
        CASE_INFO(JPG0,                                         "JPG");
        CASE_INFO(JPG1,                                         "JPG");
        CASE_INFO(JPG2,                                         "JPG");
        CASE_INFO(JPG3,                                         "JPG");
        CASE_INFO(JPG4,                                         "JPG");
        CASE_INFO(JPG5,                                         "JPG");
        CASE_INFO(JPG6,                                         "JPG");
        CASE_INFO(JPG7,                                         "JPG");
        CASE_INFO(JPG8,                                         "JPG");
        CASE_INFO(JPG9,                                         "JPG");
        CASE_INFO(JPGA,                                         "JPG");
        CASE_INFO(JPGB,                                         "JPG");
        CASE_INFO(JPGC,                                         "JPG");
        CASE_INFO(JPGD,                                         "JPG");
        CASE_INFO(COM ,                                         "Comment");
        default : Element_Info("Reserved");
                  Skip_XX(Element_Size,                         "Data");
    }
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Jpeg::SIZ()
{
    //Parsing
    int32u Xsiz, Ysiz;
    int16u Count;
    Skip_B2(                                                    "Rsiz - Capability of the codestream");
    Get_B4 (Xsiz,                                               "Xsiz - Image size X");
    Get_B4 (Ysiz,                                               "Ysiz - Image size Y");
    Skip_B4(                                                    "XOsiz - Image offset X");
    Skip_B4(                                                    "YOsiz - Image offset Y");
    Skip_B4(                                                    "tileW - Size of tile W");
    Skip_B4(                                                    "tileH - Size of tile H");
    Skip_B4(                                                    "XTOsiz - Upper-left tile offset X");
    Skip_B4(                                                    "YTOsiz - Upper-left tile offset Y");
    Get_B2 (Count,                                              "Components and initialize related arrays");
    for (int16u Pos=0; Pos<Count; Pos++)
    {
        Element_Begin("Initialize related array");
        BS_Begin();
        Skip_SB(                                                "Signed");
        Info_S1(7, BitDepth,                                    "BitDepth"); Element_Info(BitDepth);
        BS_End();
        Skip_B1(                                                "compSubsX");
        Skip_B1(                                                "compSubsY");
        Element_End();
    }

    FILLING_BEGIN_PRECISE();
        Accept("JPEG 2000");

        if (Count_Get(StreamKind)==0)
            Stream_Prepare(StreamKind);
        Fill(StreamKind, 0, Fill_Parameter(StreamKind, Generic_Format), StreamKind==Stream_Image?"JPEG 2000":"M-JPEG 2000");
        Fill(StreamKind, 0, Fill_Parameter(StreamKind, Generic_Codec), StreamKind==Stream_Image?"JPEG 2000":"M-JPEG 2000");
        if (StreamKind==Stream_Image)
            Fill(Stream_Image, 0, Image_Codec_String, "JPEG 2000", Unlimited, true, true); //To Avoid automatic filling
        Fill(StreamKind, 0, StreamKind==Stream_Image?(size_t)Image_Width:(size_t)Video_Width, Xsiz);
        Fill(StreamKind, 0, StreamKind==Stream_Image?(size_t)Image_Height:(size_t)Video_Height, Ysiz);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Jpeg::COD()
{
    //Parsing
    int16u Levels;
    int8u Style, Style2, MultipleComponentTransform;
    bool PrecinctUsed;
    Get_B1 (Style,                                              "Scod - Style");
        Get_Flags (Style, 0, PrecinctUsed,                      "Precinct used");
        Skip_Flags(Style, 1,                                    "Use SOP (start of packet)");
        Skip_Flags(Style, 2,                                    "Use EPH (end of packet header)");
    Skip_B1(                                                    "Number of decomposition levels");
    Skip_B1(                                                    "Progression order");
    Get_B2 (Levels,                                             "Number of layers");
    Info_B1(DimX,                                               "Code-blocks dimensions X (2^(n+2))"); Param_Info(1<<(DimX+2), " pixels");
    Info_B1(DimY,                                               "Code-blocks dimensions Y (2^(n+2))"); Param_Info(1<<(DimY+2), " pixels");
    Get_B1 (Style2,                                             "Style of the code-block coding passes");
        Skip_Flags(Style, 0,                                    "Selective arithmetic coding bypass");
        Skip_Flags(Style, 1,                                    "MQ states for all contexts");
        Skip_Flags(Style, 2,                                    "Regular termination");
        Skip_Flags(Style, 3,                                    "Vertically stripe-causal context formation");
        Skip_Flags(Style, 4,                                    "Error resilience info is embedded on MQ termination");
        Skip_Flags(Style, 5,                                    "Segmentation marker is to be inserted at the end of each normalization coding pass");
    Skip_B1(                                                    "Transform");
    Get_B1(MultipleComponentTransform,                          "Multiple component transform");
    if (PrecinctUsed)
    {
        BS_Begin();
        Skip_S1(4,                                              "LL sub-band width");
        Skip_S1(4,                                              "LL sub-band height");
        BS_End();
        for (int16u Pos=0; Pos<Levels; Pos++)
        {
            Element_Begin("Decomposition level");
            BS_Begin();
            Skip_S1(4,                                          "decomposition level width");
            Skip_S1(4,                                          "decomposition level height");
            BS_End();
            Element_End();
        }
    }

    FILLING_BEGIN();
        switch (MultipleComponentTransform)
        {
            case 0x01 : Fill(Stream_Image, 0, Image_Format_Profile, "Reversible"); break;
            case 0x02 : Fill(Stream_Image, 0, Image_Format_Profile, "Irreversible"); break;
            default   : ;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Jpeg::QCD()
{
    //Parsing
    Skip_B1(                                                    "Sqcd - Style");
    Skip_XX(Element_Size-Element_Offset,                        "QCD data");
}

//---------------------------------------------------------------------------
void File_Jpeg::SOD()
{
    FILLING_BEGIN_PRECISE();
        Finish("JPEG 2000"); //No need of more
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Jpeg::SOF_()
{
    //Parsing
    vector<float> SamplingFactors;
    int8u SamplingFactors_Max=0;
    int16u Height, Width;
    int8u  Resolution, Count;
    Get_B1 (Resolution,                                         "P - Sample precision");
    Get_B2 (Height,                                             "Y - Number of lines");
    Get_B2 (Width,                                              "X - Number of samples per line");
    Get_B1 (Count,                                              "Nf - Number of image components in frame");
    for (int8u Pos=0; Pos<Count; Pos++)
    {
        int8u Hi, Vi;
        Element_Begin("Component");
        Info_B1(Ci,                                             "Ci - Component identifier"); Element_Info(Ci);
        BS_Begin();
        Get_S1 (4, Hi,                                          "Hi - Horizontal sampling factor"); Element_Info(Hi);
        Get_S1 (4, Vi,                                          "Vi - Vertical sampling factor"); Element_Info(Vi);
        BS_End();
        Skip_B1(                                                "Tqi - Quantization table destination selector");
        Element_End();

        //Filling list of HiVi
        SamplingFactors.push_back(Hi/Vi);
        if (((float)Hi)/Vi>SamplingFactors_Max)
            SamplingFactors_Max=((float)Hi)/Vi;
    }

    FILLING_BEGIN_PRECISE();
        Accept("JPEG");

        if (Count_Get(StreamKind)==0)
            Stream_Prepare(StreamKind);
        Fill(StreamKind, 0, StreamKind==Stream_Image?(size_t)Image_Format:(size_t)Video_Format, StreamKind==Stream_Image?"JPEG":"M-JPEG");
        Fill(StreamKind, 0, StreamKind==Stream_Image?(size_t)Image_Codec:(size_t)Video_Codec, StreamKind==Stream_Image?"JPEG":"M-JPEG");
        if (StreamKind==Stream_Image)
            Fill(Stream_Image, 0, Image_Codec_String, "JPEG", Unlimited, true, true); //To Avoid automatic filling
        if (StreamKind==Stream_Video)
            Fill(Stream_Video, 0, Video_InternetMediaType, "video/JPEG", Unlimited, true, true);
        Fill(Stream_Video, 0, Video_ColorSpace, "YUV");
        Fill(StreamKind, 0, Fill_Parameter(StreamKind, Generic_Resolution), Resolution);
        Fill(StreamKind, 0, StreamKind==Stream_Image?(size_t)Image_Height:(size_t)Video_Height, Height*Height_Multiplier);
        Fill(StreamKind, 0, StreamKind==Stream_Image?(size_t)Image_Width:(size_t)Video_Width, Width);

        //chroma
        if (SamplingFactors_Max)
            while (SamplingFactors_Max<4)
            {
                for (size_t Pos=0; Pos<SamplingFactors.size(); Pos++)
                    SamplingFactors[Pos]*=2;
                SamplingFactors_Max*=2;
            }
        while (SamplingFactors.size()<3)
            SamplingFactors.push_back(0);
        Fill(StreamKind, 0, "ChromaSubsampling", Ztring::ToZtring(SamplingFactors[0], 0)+_T(":")+Ztring::ToZtring(SamplingFactors[1], 0)+_T(":")+Ztring::ToZtring(SamplingFactors[2], 0));
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Jpeg::SOS()
{
    //Parsing
    int8u Count;
    Get_B1 (Count,                                              "Number of image components in scan");
    for (int8u Pos=0; Pos<Count; Pos++)
    {
        Skip_B1(                                                "Scan component selector");
        Skip_B1(                                                "Entropy coding table destination selector");
    }
    Skip_B1(                                                    "Start of spectral or predictor selection");
    Skip_B1(                                                    "End of spectral selection");
    Skip_B1(                                                    "Successive approximation bit position");

    FILLING_BEGIN_PRECISE();
        Finish("JPEG"); //No need of more
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Jpeg::APP0()
{
    //Parsing
    int32u Name;
    Get_C4(Name,                                                "Name");
    switch (Name)
    {
        case 0x41564931 : APP0_AVI1(); break; //"AVI1"
        case 0x4A464946 : APP0_JFIF(); break; //"JFIF"
        case 0x4A464646 : APP0_JFFF(); break; //"JFFF"
        default         : Skip_XX(Element_Size,                 "Data");
    }
}

//---------------------------------------------------------------------------
void File_Jpeg::APP0_AVI1()
{
    //Parsing
    int8u  FieldOrder=(int8u)-1;
    Element_Begin("AVI1");
        if (Element_Size==16-4)
        {
            Get_B1 (FieldOrder,                                     "Field Order");
            Skip_XX(7,                                              "Zeroes");
        }
        if (Element_Size==18-4)
        {
            Get_B1 (FieldOrder,                                     "Field Order");
            Skip_B1(                                                "Zero");
            Skip_B4(                                                "Size of 1st Field");
            Skip_B4(                                                "Size of 2nd Field");
        }
    Element_End();

    FILLING_BEGIN();
        if (!Status[IsAccepted])
        {
            Accept("JPEG");
            if (Count_Get(Stream_Video)==0)
                Stream_Prepare(Stream_Video);

            switch (FieldOrder)
            {
                case 0x00 : Fill(Stream_Video, 0, Video_Interlacement, "PPF"); Fill(Stream_Video, 0, Video_ScanType, "Progressive"); break;
                case 0x01 : Fill(Stream_Video, 0, Video_Interlacement, "TFF"); Fill(Stream_Video, 0, Video_ScanType, "Interlaced"); Fill(Stream_Video, 0, Video_ScanOrder, "TFF"); Height_Multiplier=2; break;
                case 0x02 : Fill(Stream_Video, 0, Video_Interlacement, "BFF"); Fill(Stream_Video, 0, Video_ScanType, "Interlaced"); Fill(Stream_Video, 0, Video_ScanOrder, "BFF"); Height_Multiplier=2; break;
                default   : ;
            }
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Jpeg::APP0_JFIF()
{
    //Parsing
    Skip_B1(                                                    "Zero");
    Element_Begin("JFIF");
        int16u Width, Height;
        int8u  Unit, ThumbailX, ThumbailY;
        Skip_B2(                                                "Version");
        Get_B1 (Unit,                                           "Unit"); //0=Pixels, 1=dpi, 2=dpcm
        Get_B2 (Width,                                          "Xdensity");
        Get_B2 (Height,                                         "Ydensity");
        Get_B1 (ThumbailX,                                      "Xthumbail");
        Get_B1 (ThumbailY,                                      "Ythumbail");
        Skip_XX(3*ThumbailX*ThumbailY,                          "RGB Thumbail");
    Element_End();
}

//---------------------------------------------------------------------------
void File_Jpeg::APP0_JFFF()
{
    Skip_B1(                                                    "Zero");
    Element_Begin("Extension");
        Skip_B1(                                                "extension_code"); //0x10 Thumbnail coded using JPEG, 0x11 Thumbnail stored using 1 byte/pixel, 0x13 Thumbnail stored using 3 bytes/pixel
        if (Element_Size>Element_Offset)
            Skip_XX(Element_Size-Element_Offset,                "extension_data");
    Element_End();
}

//---------------------------------------------------------------------------
void File_Jpeg::APP0_JFFF_JPEG()
{
    //Parsing
    Element_Begin("Thumbail JPEG");
        if (Element_Size>Element_Offset)
            Skip_XX(Element_Size-Element_Offset,                "Data");
    Element_End();
}

//---------------------------------------------------------------------------
void File_Jpeg::APP0_JFFF_1B()
{
    //Parsing
    Element_Begin("Thumbail 1 byte per pixel");
        int8u  ThumbailX, ThumbailY;
        Get_B1 (ThumbailX,                                      "Xthumbail");
        Get_B1 (ThumbailY,                                      "Ythumbail");
        Skip_XX(768,                                            "Palette");
        Skip_XX(ThumbailX*ThumbailY,                            "Thumbail");
    Element_End();
}

//---------------------------------------------------------------------------
void File_Jpeg::APP0_JFFF_3B()
{
    //Parsing
    Element_Begin("Thumbail 3 bytes per pixel");
        int8u  ThumbailX, ThumbailY;
        Get_B1 (ThumbailX,                                      "Xthumbail");
        Get_B1 (ThumbailY,                                      "Ythumbail");
        Skip_XX(3*ThumbailX*ThumbailY,                          "RGB Thumbail");
    Element_End();
}

//---------------------------------------------------------------------------
void File_Jpeg::APP1()
{
    //Parsing
    int64u Name;
    Get_C6(Name,                                                "Name");

    switch (Name)
    {
        case 0x457869660000LL : APP1_EXIF(); break; //"Exif\0\0"
        default               : Skip_XX(Element_Size,           "Data");
    }
}

//---------------------------------------------------------------------------
void File_Jpeg::APP1_EXIF()
{
    //Parsing
    Element_Begin("Exif");
        int32u Alignment;
        Get_C4(Alignment,                                       "Alignment");
        if (Alignment==0x49492A00)
            Skip_B4(                                            "First_IFD");
        if (Alignment==0x4D4D2A00)
            Skip_L4(                                            "First_IFD");
    Element_End();
}

} //NameSpace

#endif
