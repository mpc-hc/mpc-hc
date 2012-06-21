// File_Celt - Info for CELT files
// Copyright (C) 2010-2011 Lionel Duchateau, kurtnoise@free.fr
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
#if defined(MEDIAINFO_CELT_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Celt.h"
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Celt::File_Celt()
:File__Analyze()
{
    //Internal
    Identification_Done=false;
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Celt::Header_Parse()
{
    //Filling
    Header_Fill_Code(0, "CELT");
    Header_Fill_Size(Element_Size);
}

//---------------------------------------------------------------------------
void File_Celt::Data_Parse()
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
void File_Celt::Identification()
{
    Element_Name("Identification");

    //Parsing
    Ztring celt_version;
    int32u Celt_version_id, sample_rate, nb_channels;
    Skip_Local(8,                                               "celt_codec_id");
    Get_Local(20, celt_version,                                 "celt_version");
    Get_L4 (Celt_version_id,                                    "celt_version_id");
    Skip_L4(                                                    "header_size");
    Get_L4 (sample_rate,                                        "rate");
    Get_L4 (nb_channels,                                        "nb_channels");
    Skip_L4(                                                    "frame_size");
    Skip_L4(                                                    "overlap");
    Skip_L4(                                                    "bytes_per_packet");
    Skip_L4(                                                    "extra_headers");

    //Filling
    FILLING_BEGIN()
        Accept("CELT");

        Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, Audio_Format, "CELT");
        Fill(Stream_Audio, 0, Audio_Codec, "CELT");

        if (!celt_version.empty())
        {
            //Fill(Stream_Audio, 0, Audio_Encoded_Library, celt_version); //Need more info about hte different possibilities, in the meanwhile trusting more the comment part
            Fill(Stream_Audio, 0, Audio_SamplingRate, sample_rate);
            Fill(Stream_Audio, 0, Audio_Channel_s_, nb_channels);
        }

    FILLING_END();

    //Filling
    Identification_Done=true;
}

//---------------------------------------------------------------------------
void File_Celt::Comment()
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
        if (value.find(_T("CELT "))!=std::string::npos)
        {
            Ztring Version=value.SubString(_T("CELT "), _T(" ("));
            Fill(Stream_Audio, 0, Audio_Encoded_Library, _T("CELT ")+Version);
            Fill(Stream_Audio, 0, Audio_Encoded_Library_Name, _T("CELT"));
            Fill(Stream_Audio, 0, Audio_Encoded_Library_Version, Version);
        }
        else if (!value.empty())
            Fill(Stream_Audio, 0, "Comment", value);
    }

    Finish("CELT");
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_CELT_YES
