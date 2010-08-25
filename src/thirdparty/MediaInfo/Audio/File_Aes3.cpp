// File_Aes3 - Info for AES3 packetized streams
// Copyright (C) 2008-2010 MediaArea.net SARL, Info@MediaArea.net
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
#if defined(MEDIAINFO_AES3_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Aes3.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const char* Aes3_ChannelsPositions(int8u number_channels)
{
    switch (number_channels)
    {
        case  0 : return "Front: L R";                                  //2 channels
        case  1 : return "Front: L C R, LFE";                           //4 channels
        case  2 : return "Front: L C R, Side: L R, LFE";                //6 channels
        case  3 : return "Front: L C R, Side: L R, Back: L R, LFE";     //8 channels
        default : return "";
    }
}

//---------------------------------------------------------------------------
const char* Aes3_ChannelsPositions2(int8u number_channels)
{
    switch (number_channels)
    {
        case  0 : return "2/0/0.0";                                     //2 channels
        case  1 : return "3/0/0.1";                                     //4 channels
        case  2 : return "3/2/0.1";                                     //6 channels
        case  3 : return "3/2/2.1";                                     //8 channels
        default : return "";
    }
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Aes3::File_Aes3()
:File__Analyze()
{
    //Configure
    PTS_DTS_Needed=true;

    //Temp
    Block_Count=0;
    Block_Last_PTS=(int32u)-1;
    Block_Last_Size=(int64u)-1;
}

//***************************************************************************
// Format
//***************************************************************************

//---------------------------------------------------------------------------
void File_Aes3::Header_Parse()
{
    //Parsing
    int16u Size;
    Get_B2 (Size,                                               "Size");
    BS_Begin();
    Get_S1 (2, number_channels,                                 "number_channels"); Param_Info(2+2*number_channels, " channels");
    Info_S1(8, channel_id,                                      "channel_id");
    Get_S1 (2, bits_per_samples,                                "bits_per_samples"); Param_Info(16+4*bits_per_samples, " bits");
    Info_S1(4, alignments,                                      "alignments");
    BS_End();

    //Filling
    Header_Fill_Size(Element_Offset+Size);
    Header_Fill_Code(0, "Block");
}

//---------------------------------------------------------------------------
static inline int8u Reverse8(int n)
{
    // Input: bit order is 76543210
    //Output: bit order is 45670123
    n=((n>>1)&0x55) | ((n<<1) & 0xaa);
    n=((n>>2)&0x33) | ((n<<2) & 0xcc);
    n=((n>>4)&0x0f) | ((n<<4) & 0xf0);
    return (int8u)n;
}

//---------------------------------------------------------------------------
void File_Aes3::Data_Parse()
{
    Block_Count++;

    //Parsing
    switch (bits_per_samples)
    {
        /*
        case 0  : //16 bits
        {
            while (Element_Offset<Element_Size)
            {
                Element_Begin("2Ch group");
                int8u Info[4];
                size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

                //Channel 1 (16 bits, as "s16l" codec)
                Info[0]= Reverse8(Buffer[Buffer_Pos+0]);
                Info[1]= Reverse8(Buffer[Buffer_Pos+1]);

                //Channel 2 (16 bits, as "s16l" codec)
                Info[2]=(Reverse8(Buffer[Buffer_Pos+2])>>4) | ((Reverse8(Buffer[Buffer_Pos+3])<<4)&0xF0);
                Info[3]=(Reverse8(Buffer[Buffer_Pos+3])>>4) | ((Reverse8(Buffer[Buffer_Pos+4])<<4)&0xF0);

                //Details
                if (Config_Details>0)
                {
                    Param("0", Info[0]);
                    Param("1", Info[1]);
                    Param("2", Info[2]);
                    Param("3", Info[3]);
                }

                Element_Offset+=5;
                Element_End();
            }
        }
        break;
        case 1  : //20 bits
        {
            while (Element_Offset<Element_Size)
            {
                Element_Begin("2Ch group");
                int8u Info[6];
                size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

                //Channel 1 (24 bits, as "s24l" codec, 4 highest bits are set to 0)
                Info[0]=                                       ((Reverse8(Buffer[Buffer_Pos+0])<<4)&0xF0);
                Info[1]=(Reverse8(Buffer[Buffer_Pos+0])>>4 ) | ((Reverse8(Buffer[Buffer_Pos+1])<<4)&0xF0);
                Info[2]=(Reverse8(Buffer[Buffer_Pos+1])>>4 ) | ((Reverse8(Buffer[Buffer_Pos+2])<<4)&0xF0);

                //Channel 2 (24 bits, as "s24l" codec, 4 highest bits are set to 0)
                Info[3]=                                      ((Reverse8(Buffer[Buffer_Pos+3])<<4)&0xF0);
                Info[4]=(Reverse8(Buffer[Buffer_Pos+3])>>4) | ((Reverse8(Buffer[Buffer_Pos+4])<<4)&0xF0);
                Info[5]=(Reverse8(Buffer[Buffer_Pos+4])>>4) | ((Reverse8(Buffer[Buffer_Pos+5])<<4)&0xF0);


                //Details
                if (Config_Details>0)
                {
                    Param("0", Info[0]);
                    Param("1", Info[1]);
                    Param("2", Info[2]);
                    Param("3", Info[3]);
                    Param("4", Info[4]);
                    Param("5", Info[5]);
                }

                Element_Offset+=6;
                Element_End();
            }
        }
        break;
        case 2  : //24 bits
        {
            while (Element_Offset<Element_Size)
            {
                Element_Begin("2Ch group");
                int8u Info[6];
                size_t Buffer_Pos=Buffer_Offset+(size_t)Element_Offset;

                //Channel 1 (24 bits, as "s24l" codec)
                Info[0] = Reverse8(Buffer[Buffer_Pos+0] );
                Info[1] = Reverse8(Buffer[Buffer_Pos+1] );
                Info[2] = Reverse8(Buffer[Buffer_Pos+2] );

                //Channel 2 (24 bits, as "s24l" codec)
                Info[3] = (Reverse8(Buffer[Buffer_Pos+3])>>4) | ((Reverse8(Buffer[Buffer_Pos+4])<<4)&0xF0 );
                Info[4] = (Reverse8(Buffer[Buffer_Pos+4])>>4) | ((Reverse8(Buffer[Buffer_Pos+5])<<4)&0xF0 );
                Info[5] = (Reverse8(Buffer[Buffer_Pos+5])>>4) | ((Reverse8(Buffer[Buffer_Pos+6])<<4)&0xF0 );

                //Details
                if (Config_Details>0)
                {
                    Param("0", Info[0]);
                    Param("1", Info[1]);
                    Param("2", Info[2]);
                    Param("3", Info[3]);
                    Param("4", Info[4]);
                    Param("5", Info[5]);
                }

                Element_Offset+=7;
                Element_End();
            }
        }
        break;
        */
        default :
            Skip_XX(Element_Size,                           "Data");
    }

    FILLING_BEGIN();
    FILLING_END();


    if (PTS==(int64u)-1)
        Block_Count=2; //We don't have PTS, don't need more
    else if (Block_Count==1)
    {
        Block_Last_PTS=PTS;
        Block_Last_Size=Element_Size;
    }

    if (Block_Count>=2)
    {
        //Filling
        Accept("AES3");

        Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, 0, Audio_Format, "PCM");
        Fill(Stream_Audio, 0, Audio_Format_Profile, "AES3");
        Fill(Stream_Audio, 0, Audio_Codec, "AES3");
        Fill(Stream_Audio, 0, Audio_Channel_s_, 2+2*number_channels);
        Fill(Stream_Audio, 0, Audio_ChannelPositions, Aes3_ChannelsPositions(number_channels));
        Fill(Stream_Audio, 0, Audio_ChannelPositions_String2, Aes3_ChannelsPositions2(number_channels));
        Fill(Stream_Audio, 0, Audio_Resolution, 16+4*bits_per_samples);
        if (PTS!=(int32u)-1 && Block_Last_PTS!=(int32u)-1 && PTS!=Block_Last_PTS)
        {
            //Rounding
            int64u BitRate=Block_Last_Size*8*1000*1000000/(PTS-Block_Last_PTS);
            int64u SamplingRate=BitRate*(4+bits_per_samples)/(5+bits_per_samples)/(2+2*number_channels)/(16+4*bits_per_samples);
            if (SamplingRate>  7840 && SamplingRate<  8160) SamplingRate=  8000;
            if (SamplingRate> 15680 && SamplingRate< 16320) SamplingRate= 16000;
            if (SamplingRate> 31360 && SamplingRate< 32640) SamplingRate= 32000;
            if (SamplingRate> 62720 && SamplingRate< 65280) SamplingRate= 64000;
            if (SamplingRate> 10804 && SamplingRate< 11246) SamplingRate= 11025;
            if (SamplingRate> 21609 && SamplingRate< 22491) SamplingRate= 22050;
            if (SamplingRate> 43218 && SamplingRate< 44982) SamplingRate= 44100;
            if (SamplingRate> 86436 && SamplingRate< 89964) SamplingRate= 88200;
            if (SamplingRate> 11760 && SamplingRate< 12240) SamplingRate= 12000;
            if (SamplingRate> 23520 && SamplingRate< 24480) SamplingRate= 24000;
            if (SamplingRate> 47040 && SamplingRate< 48960) SamplingRate= 48000;
            if (SamplingRate> 94080 && SamplingRate< 97920) SamplingRate= 96000;
            if (SamplingRate>188160 && SamplingRate<195840) SamplingRate=192000;
            BitRate=SamplingRate/(4+bits_per_samples)*(5+bits_per_samples)*(2+2*number_channels)*(16+4*bits_per_samples);

            Fill(Stream_Audio, 0, Audio_SamplingRate, SamplingRate);
            Fill(Stream_Audio, 0, Audio_BitRate, BitRate);
        }

        Finish("AES3");
    }
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_AES3_YES
