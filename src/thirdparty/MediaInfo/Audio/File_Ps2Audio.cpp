// File_Ps2Audio - Info for PS2 Audio in MPEG files
// Copyright (C) 2007-2010 MediaArea.net SARL, Info@MediaArea.net
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
// Compilation conditions
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_PS2A_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Ps2Audio.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Ps2Audio::Read_Buffer_Continue()
{
    //Parsing
    while (Element_Offset<Element_Size)
    {
        int32u ID;
        Peek_B4(ID);
        switch (ID)
        {
            case 0x53536264 :   SSbd(); break;
            case 0x53536864 :   SShd(); break;
            default         :   Reject("PS2 Audio");
        }
    }
}

//---------------------------------------------------------------------------
void File_Ps2Audio::SSbd()
{
    if (Count_Get(Stream_Audio)!=1)
    {
        Trusted_IsNot("Element should not be here");
        return;
    }

    Element_Begin("SSbd (Body)");
        int32u Size;
        Skip_C4(                                                "ID");
        Get_L4 (Size,                                           "Size");
        Skip_XX(Element_Size-Element_Offset,                    "Data (Partial)");
    Element_End();

    FILLING_BEGIN();
        Fill(Stream_Audio, 0, Audio_StreamSize, Size);
        if (BitRate)
            Fill(Stream_Audio, 0, Audio_Duration, ((int64u)Size)*1000*8/BitRate);

        Finish("PS2 Audio");
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Ps2Audio::SShd()
{
    Element_Begin("SShd (Header)", 0x18);
        //Parsing
        int32u Size, Format, SamplingRate, Channels;
        Skip_C4(                                                "ID");
        Get_L4 (Size,                                           "Size");
        if (Size!=0x18)
        {
            Trusted_IsNot("Bad size");
            return;
        }
        Get_L4 (Format,                                         "Format");
        Get_L4 (SamplingRate,                                   "Sampling rate");
        Get_L4 (Channels,                                       "Channels");
        Skip_L4(                                                "Bytes per channel");
        Skip_L4(                                                "Reserved");
        Skip_L4(                                                "Reserved");
    Element_End();

    FILLING_BEGIN();
        Accept("PS2 Audio");

        BitRate=SamplingRate*Channels*16; //Always 16 bits

        Stream_Prepare(Stream_Audio);
        Ztring FormatS;
        switch(Format)
        {
            case 0x00000001 : FormatS=_T("PCM"); break;
            case 0x00000010 : FormatS=_T("ADPCM"); break;
            default         : ;
        }
        Fill(Stream_Audio, 0, Audio_Format, FormatS);
        Fill(Stream_Audio, 0, Audio_Codec,  FormatS);
        Fill(Stream_Audio, 0, Audio_MuxingMode, "PS2");
        Fill(Stream_Audio, 0, Audio_SamplingRate, SamplingRate);
        Fill(Stream_Audio, 0, Audio_Channel_s_, Channels);
        Fill(Stream_Audio, 0, Audio_BitRate, BitRate);
    FILLING_END();
}

} //NameSpace

#endif //MEDIAINFO_PS2A_YES

