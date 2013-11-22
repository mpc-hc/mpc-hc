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
int8u ilog(int32u Value)
{
    int8u ToReturn=0;
    while(Value)
    {
        ToReturn++;
        Value>>=1;
    }
    return ToReturn;
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
    FILLING_BEGIN();
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
        if (BitRate_Nominal && BitRate_Maximum==BitRate_Nominal && BitRate_Nominal==BitRate_Minimum)
            Fill(Stream_Audio, StreamPos_Last, Audio_BitRate_Mode, "CBR");
        else
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
        Element_Begin1("codebook");
        Get_T4 (24, codebook,                                   "codebook");
        if (codebook!=0x564342)
            return;
        Get_BT (16, codebook_dimensions,                        "codebook_dimensions");
        Get_BT (24, codebook_entries,                           "codebook_entries");
        Get_BT (1, ordered,                                     "ordered");
        if (!ordered)
        {
            int32u sparse;
            Get_BT (1, sparse,                                  "sparse");
            for (int32u Pos2=0; Pos2<codebook_entries; Pos2++)
            {
                if (sparse)
                {
                    int32u flag;
                    Get_BT (1, flag,                            "flag");
                    if (flag)
                    {
                        Info_BT(5, length,                      "length");
                    }
                }
                else
                {
                    Info_BT(5, length,                          "length");
                }
            }
        }
        else
        {
            Skip_BT(5,                                          "length");
            int32u num;
            for(int32u i=0; i<codebook_entries; )
            {
                Get_BT (ilog(codebook_entries-i), num,          "num");
                for(int32u j=0; j<num && i<codebook_entries; j++, i++);
            }
        }
        Get_BT (4, codebook_lookup_type,                        "codebook_lookup_type");
        if (codebook_lookup_type>2)
            return; //Not decodable
        if (codebook_lookup_type>0)
        {
            int8u codebook_value_bits;
            Info_BT(32, codebook_minimum_value,                 "codebook_minimum_value");
            Info_BT(32, codebook_delta_value,                   "codebook_delta_value");
            Get_T1 ( 4, codebook_value_bits,                    "codebook_value_bits");
            codebook_value_bits++;
            Info_BT( 1, codebook_sequence_p,                    "codebook_sequence_p");
            int32s vals;
            if (codebook_lookup_type==1)
            {
                vals=(int32u)floor(pow((float)codebook_entries,1.f/codebook_dimensions));
                for (;;)
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
                Get_BT (codebook_value_bits, codebook_multiplicands, "codebook_multiplicands");
        }
        Element_End0();
    }

    //Time domain transforms
    int32u vorbis_time_count;
    Get_BT (6, vorbis_time_count,                               "vorbis_time_count");
    for (int32u Pos=0; Pos<vorbis_time_count+1; Pos++)
        Skip_BT(16,                                             "zero");

    //Floors
    int32u vorbis_floor_count;
    Get_BT (6, vorbis_floor_count,                              "vorbis_floor_count");
    for (int32u Pos=0; Pos<vorbis_floor_count; Pos++)
    {
        int16u vorbis_floor_types;
        Get_T2(16, vorbis_floor_types,                          "vorbis_floor_types");

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
