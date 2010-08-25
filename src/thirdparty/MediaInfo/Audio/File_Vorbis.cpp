// File_Vorbis - Info for Vorbis files
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
//
// Note : the buffer must be given in ONE call
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
#if defined(MEDIAINFO_VORBIS_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Vorbis.h"
#include <cmath>
using namespace std;
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
int32u ilog(int32u Value)
{
    int32u ToReturn=0;
    while(Value)
    {
        ToReturn++;
        Value>>=1;
    }
    return(ToReturn);
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Vorbis::Header_Parse()
{
    //Filling
    Header_Fill_Code(0, "Vorbis");
    Header_Fill_Size(Buffer_Size);
}

//---------------------------------------------------------------------------
void File_Vorbis::Data_Parse()
{
    //Parsing
    if (Status[IsAccepted])
        Setup();
    else
        Identification();
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Vorbis::Identification()
{
    Element_Name("Identification");

    //Parsing
    int32u Version, SamplingRate, BitRate_Maximum, BitRate_Nominal, BitRate_Minimum;
    int8u Channels;
    Skip_B1   (                                                 "Signature");
    Skip_Local(6,                                               "Signature");
    Get_L4 (Version,                                            "Version");
    if (Version>0)
        return; //Not supported
    Get_L1 (Channels,                                           "Channels");
    Get_L4 (SamplingRate,                                       "SamplingRate");
    Get_L4 (BitRate_Maximum,                                    "BitRate_Maximum");
    Get_L4 (BitRate_Nominal,                                    "BitRate_Nominal");
    Get_L4 (BitRate_Minimum,                                    "BitRate_Minimum");
    BS_Begin();
    Skip_BS(4,                                                  "BlockSize_0"); //2^Value
    Skip_BS(4,                                                  "BlockSize_1"); //2^Value
    BS_End();
    Skip_L1(                                                    "Framing");

    //Filling
    FILLING_BEGIN()
        Accept("Vorbis");

        Stream_Prepare(Stream_Audio);
        Fill(Stream_Audio, StreamPos_Last, Audio_Format, "Vorbis");
        Fill(Stream_Audio, StreamPos_Last, Audio_Codec, "Vorbis");
        if (BitRate_Maximum!=0 && BitRate_Maximum<0x80000000) //This is a signed value, and negative values are not OK
            Fill(Stream_Audio, StreamPos_Last, Audio_BitRate_Maximum, BitRate_Maximum);
        if (BitRate_Nominal!=0 && BitRate_Nominal<0x80000000) //This is a signed value, and negative values are not OK
            Fill(Stream_Audio, StreamPos_Last, Audio_BitRate, BitRate_Nominal);
        if (BitRate_Minimum!=0 && BitRate_Minimum<0x80000000) //This is a signed value, and negative values are not OK
            Fill(Stream_Audio, StreamPos_Last, Audio_BitRate_Minimum, BitRate_Minimum);
        if (BitRate_Maximum==0 && BitRate_Nominal!=0 && BitRate_Minimum==0)
            Fill(Stream_Audio, StreamPos_Last, Audio_BitRate_Mode, "CBR");
        else if (BitRate_Maximum>BitRate_Nominal*1.1 && BitRate_Minimum<BitRate_Nominal*0.9)
            Fill(Stream_Audio, StreamPos_Last, Audio_BitRate_Mode, "VBR");
        Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, Channels);
        Fill(Stream_Audio, StreamPos_Last, Audio_SamplingRate, SamplingRate);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Vorbis::Setup()
{
    Element_Name("Setup");

    //Parsing
    Skip_Local(6,                                               "Signature");
    int32u codebook, codebook_dimensions, codebook_entries, ordered, codebook_lookup_type;
    int8u vorbis_codebook_count;
    Get_L1 (vorbis_codebook_count,                              "vorbis_codebook_count");
    BS_Begin_LE(); //Vorbis bitstream is Little Endian
    vorbis_codebook_count+=1;
    for (int Pos=0; Pos<vorbis_codebook_count; Pos++)
    {
        Element_Begin("codebook");
        Get_S3 (24, codebook,                                   "codebook");
        if (codebook!=0x564342)
            return;
        Get_BS (16, codebook_dimensions,                        "codebook_dimensions");
        Get_BS (24, codebook_entries,                           "codebook_entries");
        Get_BS (1, ordered,                                     "ordered");
        if (!ordered)
        {
            int32u sparse;
            Get_BS (1, sparse,                                  "sparse");
            for (int32u Pos2=0; Pos2<codebook_entries; Pos2++)
            {
                if (sparse)
                {
                    int32u flag;
                    Get_BS (1, flag,                            "flag");
                    if (flag)
                    {
                        Info_BS(5, length,                      "length");
                    }
                }
                else
                {
                    Info_BS(5, length,                          "length");
                }
            }
        }
        else
        {
            Skip_BS(5,                                          "length");
            int32u num;
            for(int32u i=0; i<codebook_entries; )
            {
                Get_BS (ilog(codebook_entries-i), num,          "num");
                for(int32u j=0; j<num && i<codebook_entries; j++, i++);
            }
        }
        Get_BS (4, codebook_lookup_type,                        "codebook_lookup_type");
        if (codebook_lookup_type>2)
            return; //Not decodable
        if (codebook_lookup_type>0)
        {
            int32u codebook_value_bits;
            Info_BS(32, codebook_minimum_value,                 "codebook_minimum_value");
            Info_BS(32, codebook_delta_value,                   "codebook_delta_value");
            Get_BS ( 4, codebook_value_bits,                    "codebook_value_bits");
            codebook_value_bits++;
            Info_BS( 1, codebook_sequence_p,                    "codebook_sequence_p");
            int32s vals;
            if (codebook_lookup_type==1)
            {
                vals=(int32u)floor(pow((float)codebook_entries,1.f/codebook_dimensions));
                while(1)
                {
                    int32u acc=1, acc1=1;
                    for(int32u i=0; i<codebook_dimensions; i++)
                    {
                        acc*=vals;
                        acc1*=vals+1;
                    }
                    if(acc<=codebook_entries && acc1>codebook_entries)
                        break;
                    else if(acc>codebook_entries)
                        vals--;
                    else
                        vals++;
                }
            }
            else //codebook_lookup_type==2
                vals=codebook_entries*codebook_dimensions;
            int32u codebook_multiplicands;
            for(int i=0; i<vals; i++)
                Get_BS (codebook_value_bits, codebook_multiplicands, "codebook_multiplicands");
        }
        Element_End();
    }

    //Time domain transforms
    int32u vorbis_time_count;
    Get_BS (6, vorbis_time_count,                               "vorbis_time_count");
    for (int32u Pos=0; Pos<vorbis_time_count+1; Pos++)
        Skip_BS(16,                                             "zero");

    //Floors
    int32u vorbis_floor_count;
    Get_BS (6, vorbis_floor_count,                              "vorbis_floor_count");
    for (int32u Pos=0; Pos<vorbis_floor_count; Pos++)
    {
        Info_BS(16, vorbis_floor_types,                         "vorbis_floor_types");

        FILLING_BEGIN();
            Fill(Stream_Audio, 0, Audio_Format_Settings_Floor, vorbis_floor_types);
            Fill(Stream_Audio, 0, Audio_Codec_Settings_Floor, vorbis_floor_types);
            if (vorbis_floor_types==0)
            {
                Fill(Stream_Audio, 0, Audio_Format_Settings, "Floor0");
                Fill(Stream_Audio, 0, Audio_Codec_Settings, "Floor0");
            }
        FILLING_END();
        //Must continue parsing...
    }
    BS_End_LE();

    Finish("Vorbis");
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_VORBIS_YES
