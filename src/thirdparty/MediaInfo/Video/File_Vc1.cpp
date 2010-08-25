// File_Vc1 - Info for VC-1 files
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
#if defined(MEDIAINFO_VC1_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Video/File_Vc1.h"
#include "ZenLib/BitStream.h"
#undef FILLING_BEGIN
#define FILLING_BEGIN() \
    while (Element_Offset<Element_Size && Buffer[Buffer_Offset+(size_t)Element_Offset]==0x00) \
        Element_Offset++; \
    if (Element_Offset!=Element_Size) \
        Trusted_IsNot("Size error"); \
    else if (Element_IsOK()) \
    {
#include <cmath>
using namespace std;
using namespace ZenLib;
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const char* Vc1_Profile[]=
{
    "SP",
    "MP",
    "CP",
    "AP",
};

//---------------------------------------------------------------------------
const char* Vc1_ColorimetryFormat[]=
{
    "",
    "4:2:0",
    "",
    "",
};

//---------------------------------------------------------------------------
const float32 Vc1_PixelAspectRatio[]=
{
    (float32)1, //Reserved
    (float32)1,
    (float32)12/(float32)11,
    (float32)10/(float32)11,
    (float32)16/(float32)11,
    (float32)40/(float32)33,
    (float32)24/(float32)11,
    (float32)20/(float32)11,
    (float32)32/(float32)11,
    (float32)80/(float32)33,
    (float32)18/(float32)11,
    (float32)15/(float32)11,
    (float32)64/(float32)33,
    (float32)160/(float32)99,
    (float32)1, //Reserved
    (float32)1, //Custom
};

//---------------------------------------------------------------------------
float32 Vc1_FrameRate_enr(int8u Code)
{
    switch (Code)
    {
        case 0x01 : return (float32)24000;
        case 0x02 : return (float32)25000;
        case 0x03 : return (float32)30000;
        case 0x04 : return (float32)50000;
        case 0x05 : return (float32)60000;
        case 0x06 : return (float32)48000;
        case 0x07 : return (float32)72000;
        default   : return (float32)0;
    }
}

//---------------------------------------------------------------------------
float32 Vc1_FrameRate_dr(int8u Code)
{
    switch (Code)
    {
        case 0x01 : return (float32)1000;
        case 0x02 : return (float32)1001;
        default   : return (float32)0;
    }
}

//---------------------------------------------------------------------------
const char* Vc1_Type[]=
{
    "I",
    "P",
    "B",
    "BI",
    "Skipped",
};

//---------------------------------------------------------------------------
const char* Vc1_PictureFormat[]=
{
    "Progressive frame",
    "Interlaced frame",
    "Two interlaced fields",
    "",
};

//---------------------------------------------------------------------------
const int8u Vc1_FieldTypeTable[][2]=
{
    {0, 0},
    {0, 1},
    {1, 0},
    {1, 1},
    {2, 2},
    {2, 3},
    {3, 2},
    {3, 3},
};

//---------------------------------------------------------------------------
int32u Vc1_ptype(int8u Size, int32u Value)
{
    switch (Size)
    {
        case 1 :
                    switch (Value)
                    {
                        case 0x0 : return 1;
                        default  : return (int32u)-1;
                    }
        case 2 :
                    switch (Value)
                    {
                        case 0x2 : return 2;
                        default  : return (int32u)-1;
                    }
        case 3 :
                    switch (Value)
                    {
                        case 0x6 : return 0;
                        default  : return (int32u)-1;
                    }
        case 4 :
                    switch (Value)
                    {
                        case 0xE : return 3;
                        case 0xF : return 4;
                        default  : return (int32u)-1;
                    }
        default: return (int32u)-1;
    }
};

//---------------------------------------------------------------------------
int32u Vc1_bfraction(int8u Size, int32u Value)
{
    switch (Size)
    {
        case 3 :
                    switch (Value)
                    {
                        case 0x00 : return 0x00;
                        case 0x01 : return 0x01;
                        case 0x02 : return 0x02;
                        case 0x03 : return 0x03;
                        case 0x04 : return 0x04;
                        case 0x05 : return 0x05;
                        case 0x06 : return 0x06;
                        default  : return (int32u)-1;
                    }
        case 7 :
                    switch (Value)
                    {
                        case 0x70 : return 0x70;
                        case 0x71 : return 0x71;
                        case 0x72 : return 0x72;
                        case 0x73 : return 0x73;
                        case 0x74 : return 0x74;
                        case 0x75 : return 0x75;
                        case 0x76 : return 0x76;
                        case 0x77 : return 0x77;
                        case 0x78 : return 0x78;
                        case 0x79 : return 0x79;
                        case 0x7A : return 0x7A;
                        case 0x7B : return 0x7B;
                        case 0x7C : return 0x7C;
                        case 0x7D : return 0x7D;
                        case 0x7E : return 0x7E;
                        case 0x7F : return 0x7F;
                        default  : return (int32u)-1;
                    }
        default: return (int32u)-1;
    }
};

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Vc1::File_Vc1()
:File__Analyze()
{
    //Config
    MustSynchronize=true;
    Buffer_TotalBytes_FirstSynched_Max=64*1024;

    //In
    Frame_Count_Valid=30;
    FrameIsAlwaysComplete=false;
    From_WMV3=false;
    Only_0D=false;

    //Temp
    EntryPoint_Parsed=false;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Vc1::Streams_Fill()
{
    //Calculating - PixelAspectRatio
    float32 PixelAspectRatio;
    if (AspectRatio!=0x0F)
        PixelAspectRatio=Vc1_PixelAspectRatio[AspectRatio];
    else if (AspectRatioY)
        PixelAspectRatio=((float)AspectRatioX)/((float)AspectRatioY);
    else
        PixelAspectRatio=1; //Unknown

    //Calculating - FrameRate
    float32 FrameRate=0;
    if (framerate_present)
    {
        if (framerate_form)
            FrameRate=((float32)(framerateexp+1))/(float32)32;
        else if (Vc1_FrameRate_dr(frameratecode_dr))
            FrameRate=Vc1_FrameRate_enr(frameratecode_enr)/Vc1_FrameRate_dr(frameratecode_dr);
    }

    //Filling
    Stream_Prepare(Stream_Video);
    Fill(Stream_Video, 0, Video_Format, "VC-1");
    Fill(Stream_Video, 0, Video_Codec, From_WMV3?"WMV3":"VC-1"); //For compatibility with the old reaction
    Fill(Stream_Video, 0, Video_Resolution, 8);

    Ztring Profile=Vc1_Profile[profile];
    if (profile==3)
        Profile+=_T("@L")+Ztring::ToZtring(level);
    Fill(Stream_Video, 0, Video_Format_Profile, Profile);
    Fill(Stream_Video, 0, Video_Codec_Profile, Profile);
    Fill(Stream_Video, 0, Video_Colorimetry, Vc1_ColorimetryFormat[colordiff_format]);
    if (coded_width && coded_height)
    {
        Fill(Stream_Video, StreamPos_Last, Video_Width, (coded_width+1)*2);
        Fill(Stream_Video, StreamPos_Last, Video_Height, (coded_height+1)*2);
    }
    if (PixelAspectRatio!=0)
        Fill(Stream_Video, 0, Video_PixelAspectRatio, PixelAspectRatio, 3, true);
    if (FrameRate!=0)
        Fill(Stream_Video, StreamPos_Last, Video_FrameRate, FrameRate);

    //Interlacement
    if (!interlace || (PictureFormat_Count[1]==0 && PictureFormat_Count[2]==0)) //No interlaced frame/field
    {
        Fill(Stream_Video, 0, Video_ScanType, "Progressive");
        Fill(Stream_Video, 0, Video_Interlacement, "PPF");
    }
    else if (PictureFormat_Count[0]>0) //Interlaced and non interlaced frames/fields
    {
        Fill(Stream_Video, 0, Video_ScanType, "Mixed");
        Fill(Stream_Video, 0, Video_Interlacement, "Mixed");
    }
    else
    {
        Fill(Stream_Video, 0, Video_ScanType, "Interlaced");
        Fill(Stream_Video, 0, Video_Interlacement, "Interlaced");
    }
    if (Frame_Count>0 && interlace)
        Fill(Stream_Video, 0, Video_ScanOrder, Interlaced_Bottom?"BFF":"TFF");
    std::string TempRef;
    for (std::map<int16u, temporalreference>::iterator Temp=TemporalReference.begin(); Temp!=TemporalReference.end(); Temp++)
    {
        TempRef+=Temp->second.top_field_first?"T":"B";
        TempRef+=Temp->second.repeat_first_field?"3":"2";
    }
    if (TempRef.find('3')!=std::string::npos) //A pulldown maybe is detected
    {
        if (TempRef.find("T2T3B2B3T2T3B2B3")!=std::string::npos
         || TempRef.find("B2B3T2T3B2B3T2T3")!=std::string::npos)
        {
            Fill(Stream_Video, 0, Video_ScanOrder, "2:3 Pulldown", Unlimited, true, true);
            Fill(Stream_Video, 0, Video_FrameRate, FrameRate*24/30, 3, true); //Real framerate
            Fill(Stream_Video, 0, Video_ScanType, "Progressive", Unlimited, true, true);
            Fill(Stream_Video, 0, Video_Interlacement, "PPF", Unlimited, true, true);
        }
        if (TempRef.find("T2T2T2T2T2T2T2T2T2T2T2T3B2B2B2B2B2B2B2B2B2B2B2B3")!=std::string::npos
         || TempRef.find("B2B2B2B2B2B2B2B2B2B2B2B3T2T2T2T2T2T2T2T2T2T2T2T3")!=std::string::npos)
        {
            Fill(Stream_Video, 0, Video_ScanOrder, "2:2:2:2:2:2:2:2:2:2:2:3 Pulldown", Unlimited, true, true);
            Fill(Stream_Video, 0, Video_FrameRate, FrameRate*24/25, 3, true); //Real framerate
            Fill(Stream_Video, 0, Video_ScanType, "Progressive", Unlimited, true, true);
            Fill(Stream_Video, 0, Video_Interlacement, "PPF", Unlimited, true, true);
        }
    }

    //Buffer
    for (size_t Pos=0; Pos<hrd_buffers.size(); Pos++)
        Fill(Stream_Video, 0, Video_BufferSize, hrd_buffers[Pos]);
}

//---------------------------------------------------------------------------
void File_Vc1::Streams_Finish()
{
    //Purge what is not needed anymore
    if (!File_Name.empty()) //Only if this is not a buffer, with buffer we can have more data
        Streams.clear();
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Vc1::FileHeader_Begin()
{
    if (!File__Analyze::FileHeader_Begin_0x000001())
        return false;

    if (!MustSynchronize)
    {
        Synched_Init();
        Buffer_TotalBytes_FirstSynched+=0;
        File_Offset_FirstSynched=File_Offset;
    }

    //All should be OK
    return true;
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Vc1::Synched_Test()
{
    //Must have enough buffer for having header
    if (Buffer_Offset+3>Buffer_Size)
        return false;

    //Quick test of synchro
    if (CC3(Buffer+Buffer_Offset)!=0x000001)
        Synched=false;

    //Quick search
    if (Synched && !Header_Parser_QuickSearch())
        return false;

    //We continue
    return true;
}

//---------------------------------------------------------------------------
void File_Vc1::Synched_Init()
{
    //Count
    Frame_Count=0;
    Interlaced_Top=0;
    Interlaced_Bottom=0;
    PictureFormat_Count.resize(4);

    //Temp
    coded_width=0;
    coded_height=0;
    framerateexp=0;
    frameratecode_enr=0;
    frameratecode_dr=0;
    profile=0;
    level=0;
    colordiff_format=0;
    AspectRatio=0;
    AspectRatioX=0;
    AspectRatioY=0;
    hrd_num_leaky_buckets=0;
    max_b_frames=7; //Default for advanced profile
    interlace=false;
    tfcntrflag=false;
    framerate_present=false;
    framerate_form=false;
    hrd_param_flag=false;
    finterpflag=false;
    rangered=false;
    psf=false;
    pulldown=false;
    panscan_flag=false;

    TemporalReference_Offset=0;

    //Default stream values
    Streams.resize(0x100);
    Streams[0x0F].Searching_Payload=true;
}

//***************************************************************************
// Buffer
//***************************************************************************

//---------------------------------------------------------------------------
void File_Vc1::Header_Parse()
{
    //Specific
    if (From_WMV3 || Only_0D)
    {
        Header_Fill_Size(Buffer_Size);
        Header_Fill_Code(From_WMV3?0x0F:0x0D, Ztring().From_CC1(From_WMV3?0x0F:0x0D));
        return;
    }

    //Parsing
    int8u start_code;
    Skip_B3(                                                    "synchro");
    Get_B1 (start_code,                                         "start_code");
    if (!Header_Parser_Fill_Size())
    {
        Element_WaitForMoreData();
        return;
    }

    //Filling
    Header_Fill_Code(start_code, Ztring().From_CC1(start_code));
}

//---------------------------------------------------------------------------
void File_Vc1::Data_Parse()
{
    //Parse
    switch (Element_Code)
    {
        case 0x0A: EndOfSequence(); break;
        case 0x0B: Slice(); break;
        case 0x0C: Field(); break;
        case 0x0D: FrameHeader(); break;
        case 0x0E: EntryPointHeader(); break;
        case 0x0F: SequenceHeader(); break;
        case 0x1B: UserDefinedSlice(); break;
        case 0x1C: UserDefinedField(); break;
        case 0x1D: UserDefinedFrameHeader(); break;
        case 0x1E: UserDefinedEntryPointHeader(); break;
        case 0x1F: UserDefinedSequenceHeader(); break;
        default:
                Trusted_IsNot("Unattended element!");
    }
}

//---------------------------------------------------------------------------
bool File_Vc1::Header_Parser_Fill_Size()
{
    //Look for next Sync word
    if (Buffer_Offset_Temp==0) //Buffer_Offset_Temp is not 0 if Header_Parse_Fill_Size() has already parsed first frames
        Buffer_Offset_Temp=Buffer_Offset+4;
    while (Buffer_Offset_Temp+4<=Buffer_Size
        && CC3(Buffer+Buffer_Offset_Temp)!=0x000001)
    {
        Buffer_Offset_Temp+=2;
        while(Buffer_Offset_Temp<Buffer_Size && Buffer[Buffer_Offset_Temp]!=0x00)
            Buffer_Offset_Temp+=2;
        if (Buffer_Offset_Temp<Buffer_Size && Buffer[Buffer_Offset_Temp-1]==0x00 || Buffer_Offset_Temp>=Buffer_Size)
            Buffer_Offset_Temp--;
    }

    //Must wait more data?
    if (Buffer_Offset_Temp+4>Buffer_Size)
    {
        if (FrameIsAlwaysComplete || File_Offset+Buffer_Size==File_Size)
            Buffer_Offset_Temp=Buffer_Size; //We are sure that the next bytes are a start
        else
            return false;
    }

    //OK, we continue
    Header_Fill_Size(Buffer_Offset_Temp-Buffer_Offset);
    Buffer_Offset_Temp=0;
    return true;
}

//---------------------------------------------------------------------------
bool File_Vc1::Header_Parser_QuickSearch()
{
    while (       Buffer_Offset+4<=Buffer_Size
      &&   Buffer[Buffer_Offset  ]==0x00
      &&   Buffer[Buffer_Offset+1]==0x00
      &&   Buffer[Buffer_Offset+2]==0x01)
    {
        //Getting start_code
        int8u start_code=CC1(Buffer+Buffer_Offset+3);

        //Searching start
        if (Streams[start_code].Searching_Payload)
            return true;

        //Synchronizing
        Buffer_Offset+=4;
        Synched=false;
        if (!Synchronize_0x000001())
            return false;
    }

    if (Buffer_Offset+3==Buffer_Size)
        return false; //Sync is OK, but start_code is not available
    Trusted_IsNot("VC-1, Synchronisation lost");
    return Synchronize();
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
// Packet "0A"
void File_Vc1::EndOfSequence()
{
    Element_Name("EndOfSequence");
}

//---------------------------------------------------------------------------
// Packet "0B"
void File_Vc1::Slice()
{
    Element_Name("Slice");
}

//---------------------------------------------------------------------------
// Packet "0C"
void File_Vc1::Field()
{
    Element_Name("Field");
}

//---------------------------------------------------------------------------
// Packet "0D"
void File_Vc1::FrameHeader()
{
    //Counting
    if (File_Offset+Buffer_Offset+Element_Size==File_Size)
        Frame_Count_Valid=Frame_Count; //Finish frames in case of there are less than Frame_Count_Valid frames
    Frame_Count++;
    Frame_Count_InThisBlock++;

    //Name
    Element_Name("FrameHeader");
    Element_Info(Ztring(_T("Frame ")+Ztring::ToZtring(Frame_Count)));

    //Parsing
    BS_Begin();
    int8u ptype;
    if (profile==3) //Advanced
    {
        int8u PictureFormat=0; //Default=Progressive frame
        if (interlace)
        {
            bool fcm_1;
            Get_SB (   fcm_1,                                   "fcm_1");
            if (fcm_1)
            {
                bool fcm_2;
                Get_SB (   fcm_2,                               "fcm_2");
                PictureFormat=fcm_2?2:1; //Interlaced Field : Interlaced Frame
            }
        }
        Param_Info(Vc1_PictureFormat[PictureFormat]);
        PictureFormat_Count[PictureFormat]++;

        if (PictureFormat==2) //Interlaced Field
        {
            int8u ptype_;
            Get_S1 ( 3, ptype_,                                 "ptype");
            if (ptype_<5)
            {
                Param_Info(Vc1_Type[Vc1_FieldTypeTable[ptype_][0]]); Element_Info(Vc1_Type[Vc1_FieldTypeTable[ptype_][0]]); //First field
                Param_Info(Vc1_Type[Vc1_FieldTypeTable[ptype_][1]]); Element_Info(Vc1_Type[Vc1_FieldTypeTable[ptype_][1]]); //Second field
                ptype=Vc1_FieldTypeTable[ptype_][0]; //Saving the ptype from the first field
            }
            else
            {
                Trusted_IsNot("ptype is out of range");
                ptype=0; //Error
            }
        }
        else
        {
            int32u ptype_;
            Get_VL (Vc1_ptype, ptype_,                          "ptype"); if (ptype_<5) {Param_Info(Vc1_Type[(size_t)ptype_]); Element_Info(Vc1_Type[(size_t)ptype_]);}
            ptype=(int8u)ptype_;
        }

        if (ptype!=4) //!=Skipping
        {
            if (tfcntrflag)
            {
                Skip_S1( 8,                                     "tfcntr - frame counter");
            }
        }

        if (interlace && !psf)
        {
            bool tff=true, rff=false;
            if (pulldown)
            {
                Get_SB (tff,                                    "tff - top field first");
                Get_SB (rff,                                    "rff - repeat first field");
                if (tff)
                    Interlaced_Top++;
                else
                    Interlaced_Bottom++;

                if (TemporalReference.size()<30)
                {
                    if (ptype!=2 && ptype!=3  //if not B and BI-frame
                     && !TemporalReference_Waiting.empty()) //We must have 2 I or P pictures to be sure not having B picture later
                    {
                        //We have 2 I or P pictures
                        for (size_t Pos=1; Pos<TemporalReference_Waiting.size(); Pos++) //All B frames (not the first frame, which is I or P)
                        {
                            TemporalReference_Offset++;
                            TemporalReference[TemporalReference_Offset]=TemporalReference_Waiting[Pos];
                        }
                        TemporalReference_Offset++;
                        TemporalReference[TemporalReference_Offset]=TemporalReference_Waiting[0];
                        TemporalReference_Waiting.clear();
                    }

                    //We must wait for having another I or P picture
                    temporalreference Temp;
                    Temp.top_field_first=tff;
                    Temp.repeat_first_field=rff;
                    TemporalReference_Waiting.push_back(Temp);
                }

            }
        }
        else
        {
            int8u rptfrm=0;
            if (pulldown)
            {
                Get_S1 ( 2, rptfrm,                             "rptfrm - repeate frame");
            }
        }

        /*
        if (panscan_flag)
        {
            //TODO
        }

        if (ptype!=4) //!=Skipping
        {
                bool rndctrl;
            Get_SB(    rndctrl,                                 "rndctrl - rounding control");
            if (rndctrl && (ptype==0 || ptype==3)) //I or BI type
                Trusted_IsNot("Should not be true!");

            if (interlace)
                Skip_SB(                                        "uvsamp - uv sampling mode");

            if (finterpflag && PictureFormat==0) //Progressive frame
                Skip_SB(                                        "interrpfrm");

            if (PictureFormat!=1) //!=Interlaced frame
            {
                if (ptype==2 //Type B
                 || (ptype==3 && PictureFormat==2)) //Type BI and Interlaced field
                    Skip_VL(Vc1_bfraction,                      "bfraction");
            }
        }
        */
    }
    BS_End();
    if (Element_Size-Element_Offset)
        Skip_XX(Element_Size-Element_Offset,                    "Data");

    FILLING_BEGIN();
        //NextCode
        NextCode_Test();
        NextCode_Clear();
        NextCode_Add(0x0D);
        NextCode_Add(0x0F);

        //Autorisation of other streams
        Streams[0x0D].Searching_Payload=true;
        Streams[0x0F].Searching_Payload=true;

        //Filling only if not already done
        if (!Status[IsFilled] && Frame_Count>=Frame_Count_Valid && MediaInfoLib::Config.ParseSpeed_Get()<1)
            Finish("VC-1");
    FILLING_END();
}

//---------------------------------------------------------------------------
// Packet "0E"
void File_Vc1::EntryPointHeader()
{
    Element_Name("EntryPointHeader");

    //Parsing
    bool extended_mv;
    BS_Begin();
    Skip_SB(                                                    "broken_link");
    Skip_SB(                                                    "closed_entry");
    Get_SB (    panscan_flag,                                   "panscan_flag");
    Skip_SB(                                                    "refdist_flag");
    Skip_SB(                                                    "loopfilter");
    Skip_SB(                                                    "fastuvmc");
    Get_SB (    extended_mv,                                    "extended_mv");
    Skip_S1( 2,                                                 "dquant");
    Skip_SB(                                                    "vstransform");
    Skip_SB(                                                    "overlap");
    Skip_S1( 2,                                                 "quantizer");
    if (hrd_param_flag)
        for (int8u Pos=0; Pos<hrd_num_leaky_buckets; Pos++)
        {
            Element_Begin("leaky_bucket");
            Skip_S1( 8,                                         "hrd_full");
            Element_End();
        }
    TEST_SB_SKIP(                                               "coded_size_flag");
        Info_S2(12, coded_width,                                "coded_width"); Param_Info((coded_width+1)*2, " pixels");
        Info_S2(12, coded_height,                               "coded_height"); Param_Info((coded_height+1)*2, " pixels");
    TEST_SB_END();
    if (extended_mv)
        Skip_SB(                                                "extended_dmv");
    TEST_SB_SKIP(                                               "range_mapy_flag");
        Skip_S1( 3,                                             "range_mapy");
    TEST_SB_END();
    TEST_SB_SKIP(                                               "range_mapuv_flag");
        Skip_S1( 3,                                             "range_mapuv");
    TEST_SB_END();
    Mark_1();
    BS_End();

    FILLING_BEGIN();
        //NextCode
        NextCode_Test();
        NextCode_Clear();
        NextCode_Add(0x0D);

        //Autorisation of other streams
        Streams[0x0D].Searching_Payload=true;

        EntryPoint_Parsed=true;
        if (!Status[IsAccepted])
            Accept("VC-1");
    FILLING_END();
}

//---------------------------------------------------------------------------
// Packet "0F"
void File_Vc1::SequenceHeader()
{
    Element_Name("SequenceHeader");

    //Parsing
    BS_Begin();
    Get_S1 ( 2, profile,                                        "profile"); Param_Info(Vc1_Profile[profile]);
    if (profile==0 || profile==1) //Simple or Main
    {
        Skip_S1( 2,                                             "res_sm");
        Skip_S1( 3,                                             "frmrtq_postproc");
        Skip_S1( 5,                                             "bitrtq_postproc");
        Skip_SB(                                                "loopfilter");
        Skip_SB(                                                "res_x8");
        Skip_SB(                                                "multires");
        Skip_SB(                                                "res_fasttx");
        Skip_SB(                                                "fastuvmc");
        Skip_SB(                                                "extended_mv");
        Skip_S1( 2,                                             "dquant");
        Skip_SB(                                                "vtransform");
        Skip_SB(                                                "res_transtab");
        Skip_SB(                                                "overlap");
        Skip_SB(                                                "syncmarker");
        Skip_SB(                                                "rangered");
        Skip_S1( 2,                                             "maxbframes");
        Skip_S1( 2,                                             "quantizer");
        Skip_SB(                                                "finterpflag");
        Skip_SB(                                                "res_rtm_flag");
    }
    else if (profile==3) //Advanced
    {
        Get_S1 ( 3, level,                                      "level");
        Get_S1 ( 2, colordiff_format,                           "colordiff_format"); Param_Info(Vc1_ColorimetryFormat[colordiff_format]);
        Skip_S1( 3,                                             "frmrtq_postproc");
        Skip_S1( 5,                                             "bitrtq_postproc");
        Skip_SB(                                                "postprocflag");
        Get_S2 (12, coded_width,                                "max_coded_width"); Param_Info((coded_width+1)*2, " pixels");
        Get_S2 (12, coded_height,                               "max_coded_height"); Param_Info((coded_height+1)*2, " pixels");
        Get_SB (    pulldown,                                   "pulldown");
        Get_SB (    interlace,                                  "interlace");
        Get_SB (    tfcntrflag,                                 "tfcntrflag - frame counter");
        Get_SB (    finterpflag,                                "finterpflag");
        Skip_SB(                                                "reserved");
        Get_SB (    psf,                                        "psf - progressive segmented frame");
        TEST_SB_SKIP(                                           "display_ext");
            Info_S2(14, display_x,                              "display_horiz_size"); Param_Info(display_x+1, " pixels");
            Info_S2(14, display_y,                              "display_vert_size"); Param_Info(display_y+1, " pixels");
            TEST_SB_SKIP(                                       "aspectratio_flag");
                Get_S1 ( 4, AspectRatio,                        "aspect_ratio"); Param_Info(Vc1_PixelAspectRatio[AspectRatio]);
                if (AspectRatio==0x0F)
                {
                    Get_S1 ( 8, AspectRatioX,                   "aspect_horiz_size");
                    Get_S1 ( 8, AspectRatioY,                   "aspect_vert_size");
                }
            TEST_SB_END();
            TEST_SB_GET(framerate_present,                      "framerate_flag");
                TESTELSE_SB_GET(framerate_form,                 "framerateind");
                    Get_S2 (16, framerateexp,                   "framerateexp"); Param_Info((float32)((framerateexp+1)/32.0), 3, " fps");
                TESTELSE_SB_ELSE(                               "framerateind");
                    Get_S1 ( 8, frameratecode_enr,              "frameratenr"); Param_Info(Vc1_FrameRate_enr(frameratecode_enr));
                    Get_S1 ( 4, frameratecode_dr,               "frameratedr"); Param_Info(Vc1_FrameRate_dr(frameratecode_dr));
                TESTELSE_SB_END();
            TEST_SB_END();
            TEST_SB_SKIP(                                       "color_format_flag");
                Skip_S1( 8,                                     "color_prim");
                Skip_S1( 8,                                     "transfer_char");
                Skip_S1( 8,                                     "matrix_coef");
            TEST_SB_END();
        TEST_SB_END();
        TEST_SB_GET (hrd_param_flag,                            "hrd_param_flag");
            int8u buffer_size_exponent;
            Get_S1 ( 5, hrd_num_leaky_buckets,                  "hrd_num_leaky_buckets");
            Skip_S1( 4,                                         "bitrate_exponent");
            Get_S1 ( 4, buffer_size_exponent,                   "buffer_size_exponent");
            hrd_buffers.clear();
            for (int8u Pos=0; Pos<hrd_num_leaky_buckets; Pos++)
            {
                Element_Begin("leaky_bucket");
                int16u hrd_buffer;
                Skip_S2(16,                                     "hrd_rate");
                Get_S2(16, hrd_buffer,                         "hrd_buffer");
                int32u hrd_buffer_value=(int32u)((hrd_buffer+1)*pow(2.0, 1+buffer_size_exponent)); Param_Info(hrd_buffer_value, " bytes");
                Element_End();

                //Filling
                hrd_buffers.push_back(hrd_buffer_value);
            }
        TEST_SB_END();
    }
    else //forbidden
    {
        Element_DoNotTrust("Forbidden value");
    }
    Mark_1();
    BS_End();

    FILLING_BEGIN();
        //NextCode
        NextCode_Clear();
        NextCode_Add(0x0D);
        NextCode_Add(0x0E);

        //Autorisation of other streams
        Streams[0x0D].Searching_Payload=true;
        Streams[0x0E].Searching_Payload=true;

        if (From_WMV3)
        {
            if (!Status[IsAccepted])
                Accept("VC-1");
            Finish("VC-1");
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
// Packet "1B"
void File_Vc1::UserDefinedSlice()
{
    Element_Name("UserDefinedSlice");
}

//---------------------------------------------------------------------------
// Packet "1C"
void File_Vc1::UserDefinedField()
{
    Element_Name("UserDefinedField");
}

//---------------------------------------------------------------------------
// Packet "1D"
void File_Vc1::UserDefinedFrameHeader()
{
    Element_Name("UserDefinedFrameHeader");
}

//---------------------------------------------------------------------------
// Packet "1E"
void File_Vc1::UserDefinedEntryPointHeader()
{
    Element_Name("UserDefinedEntryPointHeader");
}

//---------------------------------------------------------------------------
// Packet "1F"
void File_Vc1::UserDefinedSequenceHeader()
{
    Element_Name("UserDefinedSequenceHeader");
}

} //NameSpace

#endif //MEDIAINFO_VC1_YES

