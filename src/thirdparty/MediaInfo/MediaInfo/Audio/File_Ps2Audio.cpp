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
            default         :   Element_Offset=Element_Size; Reject("PS2 Audio");
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

    Element_Begin1("SSbd (Body)");
        int32u Size;
        Skip_C4(                                                "ID");
        Get_L4 (Size,                                           "Size");
        Skip_XX(Element_Size-Element_Offset,                    "Data (Partial)");
    Element_End0();

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
    Element_Begin1("SShd (Header)");
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
    Element_End0();

    FILLING_BEGIN();
        Accept("PS2 Audio");

        BitRate=SamplingRate*Channels*16; //Always 16 bits

        Stream_Prepare(Stream_Audio);
        Ztring FormatS;
        switch(Format)
        {
            case 0x00000001 : FormatS=__T("PCM"); break;
            case 0x00000010 : FormatS=__T("ADPCM"); break;
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

