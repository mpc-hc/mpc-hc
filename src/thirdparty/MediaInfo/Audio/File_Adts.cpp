// File_Adts - Info for AAC (ADTS) files
// Copyright (C) 2002-2010 MediaArea.net SARL, Info@MediaArea.net
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
#if defined(MEDIAINFO_ADTS_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Audio/File_Adts.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************
//---------------------------------------------------------------------------
const char* ADTS_SyntacticElements[]=
{
	"ID_SCE",
	"ID_CPE",
	"ID_CCE",
	"ID_LFE",
	"ID_DSE",
	"ID_PCE",
	"ID_FIL",
	"ID_END"
};

//---------------------------------------------------------------------------
const int32u ADTS_SamplingRate[]=
{96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
 16000, 12000, 11025,  8000,  7350,     0,     0,     0,};

//---------------------------------------------------------------------------
const char* ADTS_ID[]=
{
    "MPEG-4",
    "MPEG-2",
};

//---------------------------------------------------------------------------
const char* ADTS_Format_Profile[]=
{
    "Main",
    "LC",
    "SSR",
    "LTP",
};

//---------------------------------------------------------------------------
const char* ADTS_Profile[]=
{
    "A_AAC/MPEG4/MAIN",
    "A_AAC/MPEG4/LC",
    "A_AAC/MPEG4/SSR",
    "A_AAC/MPEG4/LTP",
};

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Adts::File_Adts()
:File__Analyze(), File__Tags_Helper()
{
    //File__Tags_Helper
    Base=this;

    //Configuration
    MustSynchronize=true;
    Buffer_TotalBytes_FirstSynched_Max=64*1024;

    //In
    Frame_Count_Valid=MediaInfoLib::Config.ParseSpeed_Get()>=0.5?128:(MediaInfoLib::Config.ParseSpeed_Get()>=0.3?32:2);
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Adts::Streams_Fill()
{
    //Calculating
    int64u BitRate=0;
    if (!aac_frame_lengths.empty())
    {
        int64u aac_frame_length_Total=0;
        for (size_t Pos=0; Pos<aac_frame_lengths.size(); Pos++)
            aac_frame_length_Total+=aac_frame_lengths[Pos];

        BitRate=(ADTS_SamplingRate[sampling_frequency_index]/1024);
        BitRate*=aac_frame_length_Total*8;
        BitRate/=aac_frame_lengths.size();
    }

    if (!IsSub)
        Fill(Stream_General, 0, General_Format, "ADTS");

    File__Tags_Helper::Stream_Prepare(Stream_Audio);
    Fill(Stream_Audio, 0, Audio_Format, "AAC");
    Fill(Stream_Audio, 0, Audio_Format_Version, id?"Version 2":"Version 4");
    Fill(Stream_Audio, 0, Audio_Format_Profile, ADTS_Format_Profile[profile_ObjectType]);
    Fill(Stream_Audio, 0, Audio_Codec, ADTS_Profile[profile_ObjectType]);
    Fill(Stream_Audio, 0, Audio_SamplingRate, ADTS_SamplingRate[sampling_frequency_index]);
    Fill(Stream_Audio, 0, Audio_Channel_s_, channel_configuration);
    Fill(Stream_Audio, 0, Audio_MuxingMode, "ADTS");
    if (adts_buffer_fullness==0x7FF)
        Fill(Stream_Audio, 0, Audio_BitRate_Mode, "VBR");
    else
    {
        Fill(Stream_Audio, 0, Audio_BitRate_Mode, "CBR");
        if (BitRate)
            Fill(Stream_Audio, 0, Audio_BitRate, BitRate);
    }

    File__Tags_Helper::Streams_Fill();
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Adts::Synchronize()
{
    //Tags
    bool Tag_Found;
    if (!File__Tags_Helper::Synchronize(Tag_Found))
        return false;
    if (Tag_Found)
        return true;

    //Synchronizing
    while (Buffer_Offset+6<=Buffer_Size)
    {
         while (Buffer_Offset+6<=Buffer_Size
             && (CC2(Buffer+Buffer_Offset)&0xFFF6)!=0xFFF0)
            Buffer_Offset++;

        if (Buffer_Offset+6<=Buffer_Size)//Testing if size is coherant
        {
            //Testing next start, to be sure
            int16u aac_frame_length=(CC3(Buffer+Buffer_Offset+3)>>5)&0x1FFF;
            if (IsSub && Buffer_Offset+aac_frame_length==Buffer_Size)
                break;
            if (File_Offset+Buffer_Offset+aac_frame_length!=File_Size-File_EndTagSize)
            {
                if (Buffer_Offset+aac_frame_length+2>Buffer_Size)
                    return false; //Need more data

                //Testing
                if (aac_frame_length<=7 || (CC2(Buffer+Buffer_Offset+aac_frame_length)&0xFFF6)!=0xFFF0)
                    Buffer_Offset++;
                else
                {
                    //Testing next start, to be sure
                    int16u aac_frame_length2=(CC3(Buffer+Buffer_Offset+aac_frame_length+3)>>5)&0x1FFF;
                    if (File_Offset+Buffer_Offset+aac_frame_length+aac_frame_length2!=File_Size-File_EndTagSize)
                    {
                        if (Buffer_Offset+aac_frame_length+aac_frame_length2+2>Buffer_Size)
                            return false; //Need more data

                        //Testing
                        if (aac_frame_length<=7 || (CC2(Buffer+Buffer_Offset+aac_frame_length+aac_frame_length2)&0xFFF6)!=0xFFF0)
                            Buffer_Offset++;
                        else
                            break; //while()
                    }
                    else
                        break; //while()
                }
            }
            else
                break; //while()
        }
    }

    //Parsing last bytes if needed
    if (Buffer_Offset+6>Buffer_Size)
    {
        if (Buffer_Offset+5==Buffer_Size && (CC2(Buffer+Buffer_Offset)&0xFFF6)!=0xFFF0)
            Buffer_Offset++;
        if (Buffer_Offset+4==Buffer_Size && (CC2(Buffer+Buffer_Offset)&0xFFF6)!=0xFFF0)
            Buffer_Offset++;
        if (Buffer_Offset+3==Buffer_Size && (CC2(Buffer+Buffer_Offset)&0xFFF6)!=0xFFF0)
            Buffer_Offset++;
        if (Buffer_Offset+2==Buffer_Size && (CC2(Buffer+Buffer_Offset)&0xFFF6)!=0xFFF0)
            Buffer_Offset++;
        if (Buffer_Offset+1==Buffer_Size && CC1(Buffer+Buffer_Offset)!=0xFF)
            Buffer_Offset++;
        return false;
    }

    //Synched is OK
    return true;
}

//---------------------------------------------------------------------------
bool File_Adts::Synched_Test()
{
    //Tags
    if (!File__Tags_Helper::Synched_Test())
        return false;

    //Must have enough buffer for having header
    if (Buffer_Offset+2>Buffer_Size)
        return false;

    //Quick test of synchro
    if ((CC2(Buffer+Buffer_Offset)&0xFFF6)!=0xFFF0)
        Synched=false;

    //We continue
    return true;
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Adts::Header_Begin()
{
    //There is no real header in ADTS, retrieving only the frame length
    if (Buffer_Offset+8>Buffer_Size) //size of adts_fixed_header + adts_variable_header
        return false;

    return true;
}

//---------------------------------------------------------------------------
void File_Adts::Header_Parse()
{
    //There is no "header" in ADTS, retrieving only the frame length
    aac_frame_length=(BigEndian2int24u(Buffer+Buffer_Offset+3)>>5)&0x1FFF; //13 bits

    //Filling
    Header_Fill_Size(aac_frame_length);
    Header_Fill_Code(0, "adts_frame");
}

//---------------------------------------------------------------------------
void File_Adts::Data_Parse()
{
    //Counting
    if (File_Offset+Buffer_Offset+Element_Size==File_Size)
        Frame_Count_Valid=Frame_Count; //Finish frames in case of there are less than Frame_Count_Valid frames
    Frame_Count++;

    //Name
    Element_Info(Ztring::ToZtring(Frame_Count));

    //Parsing
    BS_Begin();
    adts_frame();
	BS_End();
	Skip_XX(Element_Size-Element_Offset,                        "Data not yet parsed");

    //Filling
    aac_frame_lengths.push_back(aac_frame_length);
    if (!Status[IsAccepted] && Frame_Count>=Frame_Count_Valid)
    {
        //Filling
        File__Tags_Helper::Accept("ADTS");

        Fill("ADTS");

        //No more need data
        File__Tags_Helper::Finish("ADTS");
    }
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Adts::adts_frame()
{
    //Parsing
    adts_fixed_header();
    adts_variable_header();
    if (num_raw_data_blocks==0)
    {
        if (!protection_absent)
    	{
        	Element_Begin("adts_error_check");
	    		Skip_S2(16,                                     "crc_check");
		    Element_End();
        }
		raw_data_block();
	}
    else
    {
		Element_Begin("adts_header_error_check");
    		if (!protection_absent)
	    		for (int i=1; i<=num_raw_data_blocks; i++)
    				Skip_S2(16,                                 "raw_data_block_position(i)");
			Skip_S2(16,                                         "crc_check");
		Element_End();

		for(int i=0; i<=num_raw_data_blocks; i++)
        {
			raw_data_block();
			if (!protection_absent)
    		{
            	Element_Begin("adts_raw_data_block_error_check");
				    Skip_BS(16,                                 "crc_check");
	    		Element_End();
            }
		}
	}
}

//---------------------------------------------------------------------------
void File_Adts::adts_fixed_header()
{
    //Parsing
    Element_Begin("adts_fixed_header");
    Skip_BS(12,                                                 "syncword");
    Get_SB (    id,                                             "id"); Param_Info(ADTS_ID[id]);
    Skip_BS( 2,                                                 "layer");
    Get_SB (    protection_absent,                              "protection_absent");
    Get_S1 ( 2, profile_ObjectType,                             "profile_ObjectType"); Param_Info(ADTS_Profile[profile_ObjectType]);
    Get_S1 ( 4, sampling_frequency_index,                       "sampling_frequency_index"); Param_Info(ADTS_SamplingRate[sampling_frequency_index], " Hz");
    Skip_SB(                                                    "private");
    Get_S1 ( 3, channel_configuration,                          "channel_configuration");
    Skip_SB(                                                    "original");
    Skip_SB(                                                    "home");
    Element_End();
}

//---------------------------------------------------------------------------
void File_Adts::adts_variable_header()
{
    //Parsing
    Element_Begin("adts_variable_header");
    Skip_SB(                                                    "copyright_id");
    Skip_SB(                                                    "copyright_id_start");
    Get_S2 (13, aac_frame_length,                               "aac_frame_length");
    Get_S2 (11, adts_buffer_fullness,                           "adts_buffer_fullness"); Param_Info(adts_buffer_fullness==0x7FF?"VBR":"CBR");
    Get_S1 ( 2, num_raw_data_blocks,                            "num_raw_data_blocks");
    Element_End();
}

//---------------------------------------------------------------------------
void File_Adts::raw_data_block()
{
	Element_Begin("raw_data_block");
	int8u id;
    do
    {
		bool NotImplemented=false; //Remove this when all is implemented
    	Element_Begin();
        Get_S1 (3, id,                                          "id_syn_ele");
        switch (id)
        {
			/*
            case 0x00://ID_SCE
			//~ single_channel_element();
			Element_Begin("single_channel_element");
			Skip_S1 ( 4,"element_instance_tag");
			//~ individual_channel_stream(0,0);
			Element_End();
            NotImplemented=true;
			break;
			case 0x01://ID_CPE: channel_pair_element();
            NotImplemented=true;
			break;
			case 0x02://ID_CCE: coupling_channel_element();
            NotImplemented=true;
			break;
			case 0x03://ID_LFE: lfe_channel_element();
            NotImplemented=true;
			break;
			case 0x04://ID_DSE: data_stream_element();
            NotImplemented=true;
			break;
			case 0x05 : //ID_PCE
                        program_config_element();
			            break;
			*/
            case 0x06 : //ID_FIL
                        fill_element();
                        break;
            case 0x07 : //ID_END
                        Element_Name("ID_END");
                        break;
            default   :
                    	Element_Name(Ztring::ToZtring(id));
                        NotImplemented=true;
		}
        Element_End();

        if (NotImplemented) //Remove this when all is implemented
            break; //Remove this when all is implemented
	}
	while(id!=0x07); //ID_END
	BS_End();BS_Begin(); //Byte synch
	Element_End();
}

//---------------------------------------------------------------------------
void File_Adts::channel_pair_element()
{
	//~ element_instance_tag;

		//~ common_window;
	//~ if (common_window) {
		//~ ics_info();
		//~ ms_mask_present;
		//~ if ( ms_mask_present == 1 ) {
			//~ for (g = 0; g < num_window_groups; g++) {
				//~ for (sfb = 0; sfb < max_sfb; sfb++) {
					//~ ms_used[g][sfb];
				//~ }
			//~ }
		//~ }
	//~ }
	//~ individual_channel_stream(common_window,0);
	//~ individual_channel_stream(common_window,0);

}

//---------------------------------------------------------------------------
void File_Adts::fill_element()
{
    Element_Name("ID_FIL - fill_element");

    //Parsing
    int16u cnt;
    int8u  count;
    Get_S1 (4, count,                                           "count");
    cnt=count;
    if (count==15)
    {
        int8u esc_count;
        Get_S1 (8, esc_count,                                   "esc_count");
        cnt+=esc_count;
    }
    Skip_BS(cnt,                                                "extension_payload (not implemented)");
}

} //NameSpace

#endif //MEDIAINFO_ADTS_YES

