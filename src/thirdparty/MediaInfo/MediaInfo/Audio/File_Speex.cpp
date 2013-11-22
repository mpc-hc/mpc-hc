/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Note : the buffer must be given in ONE call
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
#if defined(MEDIAINFO_SPEEX_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Speex.h"
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Speex::File_Speex()
:File__Analyze()
{
    //Internal
    Identification_Done=false;
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Speex::Header_Parse()
{
    //Filling
    Header_Fill_Code(0, "Speex");
    Header_Fill_Size(Element_Size);
}

//---------------------------------------------------------------------------
void File_Speex::Data_Parse()
{
    //Parsing
    if (Identification_Done)
        Comment();
    else
        Identification();
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Speex::Identification()
{
    Element_Name("Identification");

    //Parsing
    Ztring speex_version;
    int32u Speex_version_id, header_size, rate, nb_channels, bitrate, vbr;
    Skip_Local(8,                                               "speex_string");
    Get_Local(20, speex_version,                                "speex_version");
    Get_L4 (Speex_version_id,                                   "Speex_version_id");
    if (Speex_version_id==1)
    {
        Get_L4 (header_size,                                    "header_size");
        Get_L4 (rate,                                           "rate");
        Skip_L4(                                                "mode");
        Skip_L4(                                                "mode_bitstream_version");
        Get_L4 (nb_channels,                                    "nb_channels");
        Get_L4 (bitrate,                                        "bitrate");
        Skip_L4(                                                "frame_size");
        Get_L4 (vbr,                                            "vbr");
        Skip_L4(                                                "frames_per_packet");
        Skip_L4(                                                "extra_headers");
        Skip_L4(                                                "reserved1");
        Skip_L4(                                                "reserved2");
        if (header_size<Element_Size)
            Skip_XX(Element_Size-header_size,                   "Unknown");

        //Filling
        FILLING_BEGIN();
            Accept("Speex");

            Stream_Prepare(Stream_Audio);
            Fill(Stream_Audio, 0, Audio_Format, "Speex");
            Fill(Stream_Audio, 0, Audio_Codec, "Speex");
            if (Speex_version_id==1)
            {
                if (!speex_version.empty())
                    Fill(Stream_Audio, 0, Audio_Encoded_Library, speex_version);
                Fill(Stream_Audio, 0, Audio_SamplingRate, rate);
                Fill(Stream_Audio, 0, Audio_Channel_s_, nb_channels);
                if (bitrate!=(int32u)-1)
                    Fill(Stream_Audio, 0, Audio_BitRate, bitrate);
                Fill(Stream_Audio, 0, Audio_BitRate_Mode, vbr?"VBR":"CBR");
            }
        FILLING_END();
    }

    //Filling
    Identification_Done=true;
}

//---------------------------------------------------------------------------
void File_Speex::Comment()
{
    Element_Name("Comment?");

    while (Element_Offset<Element_Size)
    {
        Ztring value;
        int32u size;
        Get_L4(size,                                            "size");
        if (size)
            Get_Local(size, value,                              "value");

        //Filling
        if (!value.empty())
            Fill(Stream_Audio, 0, "Comment", value);
    }

    Finish("Speex");
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_SPEEX_YES
