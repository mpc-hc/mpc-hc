/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Source: http://svn.mplayerhq.hu/nut/docs/nut.txt?view=markup
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
#if defined(MEDIAINFO_NUT_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Nut.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Const
//***************************************************************************

//---------------------------------------------------------------------------
namespace Elements
{
    const int64u main       =0x4E4D7A561F5F04ADLL;
    const int64u stream     =0x4E5311405BF2F9DBLL;
    const int64u syncpoint  =0x4E4BE4ADEECA4569LL;
    const int64u index      =0x4E58DD672F23E64ELL;
    const int64u info       =0x4E49AB68B596BA78LL;
}

//***************************************************************************
// Buffer
//***************************************************************************

//---------------------------------------------------------------------------
void File_Nut::Header_Parse()
{
    /*
    //Parsing
    int8u  N;
    Peek_B1(N);
    if (N==0x4E) //'N'
    {
        //Header
        int64u startcode, forward_ptr;
        Get_B8(startcode,                                       "startcode");
        Get_VS(forward_ptr,                                     "forward_ptr");
        if (forward_ptr>4096)
            Skip_B4(                                            "header_checksum");

        Header_Fill_Code(startcode, Ztring().From_Number(startcode, 16)); //Quick filling for CC8 with text
        Header_Fill_Size(Element_Offset+forward_ptr); //4 for cheksum

    }
    else
    {
        //Frame
        Header_Fill_Code(0, "Frame");
        Header_Fill_Size(0);
        Finished();
    }
    */
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Nut::FileHeader_Parse()
{
    //Parsing
    Element_Begin1("Nut header");
    std::string file_id_string;
    Get_String(25, file_id_string,                               "file_id_string");
    Element_End0();

    FILLING_BEGIN();
        //Integrity
        if (file_id_string!="nut/multimedia container")
        {
            Reject("Nut");
            return;
        }

        //Filling
        Accept("Nut");

        Fill(Stream_General, 0, General_Format, "Nut");

        Finish("Nut");
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Nut::Data_Parse()
{
}

/*
//---------------------------------------------------------------------------
void File_Nut::Data_Parse()
{
    #define ELEMENT_CASE(_NAME) \
        case Elements::_NAME : _NAME(); break;

    //Parsing
    #ifndef __BORLANDC__
        switch (Element_Code)
    #else //__BORLANDC__
        switch (Element_Code&0xFFFFFFFF) //Borland does not like int64u for const?
    #endif //__BORLANDC__
    {
        ELEMENT_CASE(main);
        ELEMENT_CASE(stream);
        ELEMENT_CASE(syncpoint);
        ELEMENT_CASE(index);
        ELEMENT_CASE(info);
        default : Skip_XX(Element_Size-4,                       "Data");
    }

    Skip_B4(                                                    "cheksum");
}
*/

//***************************************************************************
// Elements
//***************************************************************************

/*
//---------------------------------------------------------------------------
void File_Nut::main()
{
    Element_Name("main");

    //Parsing
    int64u time_base_count;
    Skip_VS(                                                    "version");
    Skip_VS(                                                    "stream_count");
    Skip_VS(                                                    "max_distance");
    Get_VS (time_base_count,                                    "time_base_count");
    for(int64u i=0; i<time_base_count; i++)
    {
        Skip_VS(                                                "time_base_num");
        Skip_VS(                                                "time_base_denom");
        //time_base[i]= time_base_num/time_base_denom
    }
    int64u tmp_mul=1, tmp_stream=0;
    int64s tmp_pts=0;
    for(int16u i=0; i<256;)
    {
        int64u tmp_fields, tmp_size, tmp_res, count;
        Skip_VS(                                                "tmp_flag");
        Get_VS (tmp_fields,                                     "tmp_fields");
        if(tmp_fields>0)
            Skip_SL(                                            "tmp_pts");
        if(tmp_fields>1)
            Skip_VS(                                            "tmp_mul");
        if(tmp_fields>2)
            Skip_VS(                                            "tmp_stream");
        if(tmp_fields>3)
            Get_VS (tmp_size,                                   "tmp_size");
        else
            tmp_size=0;
        if(tmp_fields>4)
            Get_VS (tmp_res,                                    "tmp_res");
        else
            tmp_res=0;
        if(tmp_fields>5)
            Skip_VS(                                            "count");
        else
            count=tmp_mul-tmp_size;
        for(int64u j=6; j<tmp_fields; j++)
            Skip_VS(                                            "tmp_reserved[i]");

        for(int64u j=0; j<count && i<256; j++, i++)
        {
            if (i == 'N')
            {
                //flags[i]= FLAG_INVALID;
                j--;
                continue;
            }
            //flags[i]= tmp_flag;
            //stream_id[i]= tmp_stream;
            //data_size_mul[i]= tmp_mul;
            //data_size_lsb[i]= tmp_size + j;
            //pts_delta[i]= tmp_pts;
            //reserved_count[i]= tmp_res;
        }
    }
}

//---------------------------------------------------------------------------
void File_Nut::stream()
{
    Element_Name("stream");

    //Parsing
    int64u stream_class, fourcc_length, codec_specific_data_length;
    Skip_VS(                                                    "stream_id");
    Get_VS (stream_class,                                       "stream_class");
    Get_VS (fourcc_length,                                      "fourcc length");
    switch (fourcc_length)
    {
        case 2 : Skip_C2(                                       "fourcc"); break;
        case 4 : Skip_C4(                                       "fourcc"); break;
        default: Skip_XX(fourcc_length,                         "fourcc");
    }
    Skip_VS(                                                    "time_base_id");
    Skip_VS(                                                    "msb_pts_shift");
    Skip_VS(                                                    "max_pts_distance");
    Skip_VS(                                                    "decode_delay");
    Skip_VS(                                                    "stream_flags");
    Get_VS (codec_specific_data_length,                         "codec_specific_data length");
    Skip_XX(codec_specific_data_length,                         "codec_specific_data");
    switch (stream_class)
    {
        case 0 : //video
            {
                Skip_VS(                                        "width");
                Skip_VS(                                        "height");
                Skip_VS(                                        "sample_width");
                Skip_VS(                                        "sample_height");
                Skip_VS(                                        "colorspace_type");
            }
            break;
        case 1 : //audio
            {
                Skip_VS(                                         "samplerate_num");
                Skip_VS(                                         "samplerate_denom");
                Skip_VS(                                         "channel_count");
            }
            break;
        case 2 : //subtitles
            {
            }
            break;
        case 3 : //userdata
            {
            }
            break;
        default: ;
    }
}

//---------------------------------------------------------------------------
void File_Nut::syncpoint()
{
    Element_Name("syncpoint");
}

//---------------------------------------------------------------------------
void File_Nut::index()
{
    Element_Name("index");
}

//---------------------------------------------------------------------------
void File_Nut::info()
{
    Element_Name("info");
}

*/

}

#endif //MEDIAINFO_NUT_YES

