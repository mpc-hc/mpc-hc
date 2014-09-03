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
#if MEDIAINFO_EVENTS
    #include "MediaInfo/MediaInfo_Config_MediaInfo.h"
    #include "MediaInfo/MediaInfo_Events.h"
    #include "MediaInfo/MediaInfo_Events_Internal.h"
#endif //MEDIAINFO_EVENTS
#if MEDIAINFO_DEMUX
    #include <cstring>
    #include "base64.h"
#endif //MEDIAINFO_DEMUX
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
    "Simple",
    "Main",
    "Complex",
    "Advanced",
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
const File__Analyze::vlc Vc1_ptype[]=
{
    //                                  macroblock_address_increment
    {   0   ,   1   ,   0   ,   0   ,   1   },
    {   2   ,   1   ,   0   ,   0   ,   2   },
    {   6   ,   1   ,   0   ,   0   ,   0   },
    {   14  ,   1   ,   0   ,   0   ,   3   },
    {   15  ,   0   ,   0   ,   0   ,   4   },
    VLC_END
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
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Vc1;
        StreamIDs_Width[0]=0;
    #endif //MEDIAINFO_EVENTS
    MustSynchronize=true;
    Buffer_TotalBytes_FirstSynched_Max=64*1024;
    PTS_DTS_Needed=true;
    IsRawStream=true;
    Frame_Count_NotParsedIncluded=0;

    //In
    Frame_Count_Valid=30;
    FrameIsAlwaysComplete=false;
    From_WMV3=false;
    Only_0D=false;

    //Temp
    EntryPoint_Parsed=false;
    FrameRate=0;
    RefFramesCount=0;

    #if MEDIAINFO_DEMUX
        InitData_Buffer=NULL;
        InitData_Buffer_Size=0;
    #endif //MEDIAINFO_DEMUX
}

//---------------------------------------------------------------------------
File_Vc1::~File_Vc1()
{
    #if MEDIAINFO_DEMUX
        delete[] InitData_Buffer;
    #endif //MEDIAINFO_DEMUX
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Vc1::Streams_Accept()
{
    //Filling
    Stream_Prepare(Stream_Video);
    Fill(Stream_Video, 0, Video_Format, "VC-1");
    Fill(Stream_Video, 0, Video_Codec, From_WMV3?"WMV3":"VC-1"); //For compatibility with the old reaction
    Fill(Stream_Video, 0, Video_BitDepth, 8);
}

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

    Ztring Profile;
    if (profile!=(int8u)-1)
        Profile=Vc1_Profile[profile];
    if (profile==3 && level!=(int8u)-1)
        Profile+=__T("@L")+Ztring::ToZtring(level);
    Fill(Stream_Video, 0, Video_Format_Profile, Profile);
    Fill(Stream_Video, 0, Video_Codec_Profile, Profile);
    Fill(Stream_Video, 0, Video_ColorSpace, "YUV");
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
    for (std::map<int16u, temporalreference>::iterator Temp=TemporalReference.begin(); Temp!=TemporalReference.end(); ++Temp)
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
    if (PTS_End>PTS_Begin)
        Fill(Stream_Video, 0, Video_Duration, float64_int64s(((float64)(PTS_End-PTS_Begin))/1000000));

    #if MEDIAINFO_IBI
        int64u Numerator=0, Denominator=0;
        if (framerate_present)
        {
            if (framerate_form)
            {
                Numerator=framerateexp+1;
                Denominator=32;
            }
            else if (Vc1_FrameRate_dr(frameratecode_dr))
            {
                Numerator=(int64u)Vc1_FrameRate_enr(frameratecode_enr);
                Denominator=(int64u)Vc1_FrameRate_dr(frameratecode_dr);
            }
        }
        if (Numerator)
            Ibi_Stream_Finish(Numerator, Denominator);
    #endif //MEDIAINFO_IBI
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
    if (Buffer_Offset+4>Buffer_Size)
        return false;

    //Quick test of synchro
    if (Buffer[Buffer_Offset  ]!=0x00
     || Buffer[Buffer_Offset+1]!=0x00
     || Buffer[Buffer_Offset+2]!=0x01)
    {
        Synched=false;
        return true;
    }

    //Quick search
    if (!Header_Parser_QuickSearch())
        return false;

    #if MEDIAINFO_IBI
        bool RandomAccess=Buffer[Buffer_Offset+3]==0x0F; //SequenceHeader
        if (RandomAccess)
            Ibi_Add();
    #endif //MEDIAINFO_IBI

        //We continue
    return true;
}

//---------------------------------------------------------------------------
void File_Vc1::Synched_Init()
{
    //Count
    Interlaced_Top=0;
    Interlaced_Bottom=0;
    PictureFormat_Count.resize(4);
    if (Frame_Count_NotParsedIncluded==(int64u)-1)
        Frame_Count_NotParsedIncluded=0; //No Frame_Count_NotParsedIncluded in the container

    //Temp
    coded_width=0;
    coded_height=0;
    framerateexp=0;
    frameratecode_enr=0;
    frameratecode_dr=0;
    profile=(int8u)-1;
    level=(int8u)-1;
    colordiff_format=1; //Default is 4:2:0
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
    #if MEDIAINFO_DEMUX
        Demux_IntermediateItemFound=true;
    #endif //MEDIAINFO_DEMUX

    TemporalReference_Offset=0;

    if (!IsSub)
        FrameInfo.DTS=0;

    //Default stream values
    Streams.resize(0x100);
    Streams[0x0F].Searching_Payload=true;
}

//***************************************************************************
// Buffer - Demux
//***************************************************************************

//---------------------------------------------------------------------------
#if MEDIAINFO_DEMUX
bool File_Vc1::Demux_UnpacketizeContainer_Test()
{
    if ((Demux_IntermediateItemFound && Buffer[Buffer_Offset+3]==0x0D) || Buffer[Buffer_Offset+3]==0x0F)
    {
        if (Demux_Offset==0)
        {
            Demux_Offset=Buffer_Offset;
            Demux_IntermediateItemFound=false;
        }
        while (Demux_Offset+4<=Buffer_Size)
        {
            //Synchronizing
            while(Demux_Offset+3<=Buffer_Size && (Buffer[Demux_Offset  ]!=0x00
                                                || Buffer[Demux_Offset+1]!=0x00
                                                || Buffer[Demux_Offset+2]!=0x01))
            {
                Demux_Offset+=2;
                while(Demux_Offset<Buffer_Size && Buffer[Buffer_Offset]!=0x00)
                    Demux_Offset+=2;
                if (Demux_Offset>=Buffer_Size || Buffer[Demux_Offset-1]==0x00)
                    Demux_Offset--;
            }

            if (Demux_Offset+4<=Buffer_Size)
            {
                if (Demux_IntermediateItemFound)
                {
                    bool MustBreak;
                    switch (Buffer[Demux_Offset+3])
                    {
                        case 0x0D :
                        case 0x0F :
                                    MustBreak=true; break;
                        default   :
                                    Demux_Offset+=3;
                                    MustBreak=false;
                    }
                    if (MustBreak)
                        break; //while() loop
                }
                else
                {
                    if (Buffer[Demux_Offset+3]==0x0D)
                        Demux_IntermediateItemFound=true;
                }
            }
            Demux_Offset++;
        }

        if (Demux_Offset+4>Buffer_Size && File_Offset+Buffer_Size!=File_Size)
            return false; //No complete frame

        if (!Status[IsAccepted])
        {
            Accept("VC-1");
            if (Config->Demux_EventWasSent)
                return false;
        }

        //Demux
        #if MEDIAINFO_DEMUX
            if (InitData_Buffer_Size && Buffer[Buffer_Offset+3]==0x0F) //First SequenceHeader
            {
                //Searching begin of frame (after SequenceHeader/EntryPointHeader)
                size_t Header_End=4;
                for (; Header_End<Demux_Offset; Header_End++)
                    if (Buffer[Header_End  ]==0x00
                     && Buffer[Header_End+1]==0x00
                     && Buffer[Header_End+2]==0x01
                     && Buffer[Header_End+2]==0x0D)
                    break;

                switch (Config->Demux_InitData_Get())
                {
                    case 0 :    //In demux event
                                break; //Will be done in the first demux event
                    case 1 :    //In field
                                {
                                std::string Data_Raw((const char*)(Buffer+Buffer_Offset), (size_t)(Header_End-Buffer_Offset));
                                std::string Data_Base64(Base64::encode(Data_Raw));
                                Fill(Stream_Video, StreamPos_Last, "Demux_InitBytes", Data_Base64);
                                (*Stream_More)[Stream_Video][StreamPos_Last](Ztring().From_Local("Demux_InitBytes"), Info_Options)=__T("N NT");
                                }
                                break;
                    default :   ;
                }

                delete[] InitData_Buffer; InitData_Buffer=NULL;
                InitData_Buffer_Size=0;
            }
        #endif //MEDIAINFO_DEMUX

        Demux_UnpacketizeContainer_Demux(Buffer[Buffer_Offset+3]==0x0F);
    }

    return true;
}
#endif //MEDIAINFO_DEMUX

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Vc1::Read_Buffer_Unsynched()
{
    RefFramesCount=0;
    #if MEDIAINFO_DEMUX
        Demux_IntermediateItemFound=true;
    #endif //MEDIAINFO_DEMUX
}

//***************************************************************************
// Buffer - Per element
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
        if (Buffer_Offset_Temp>=Buffer_Size || Buffer[Buffer_Offset_Temp-1]==0x00)
            Buffer_Offset_Temp--;

        if (start_code==0x0D) //FrameHeader, we need only few bytes
        {
            if (Buffer_Offset_Temp-Buffer_Offset>20)
            {
                //OK, we continue, we have enough for a slice
                Header_Fill_Size(16);
                Buffer_Offset_Temp=0;
                return true;
            }
        }
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
        if (!Synchronize())
        {
            UnSynched_IsNotJunk=true;
            return false;
        }

        if (Buffer_Offset+4>Buffer_Size)
        {
            UnSynched_IsNotJunk=true;
            return false;
        }
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
    //Name
    Element_Name("FrameHeader");
    Element_Info1(Ztring(__T("Frame ")+Ztring::ToZtring(Frame_Count)));
    if (FrameRate)
    {
        Element_Info1C((FrameInfo.PTS!=(int64u)-1), __T("PTS ")+Ztring().Duration_From_Milliseconds(float64_int64s(((float64)FrameInfo.PTS)/1000000+Frame_Count_InThisBlock*1000/FrameRate)));
        Element_Info1C((FrameInfo.DTS!=(int64u)-1), __T("DTS ")+Ztring().Duration_From_Milliseconds(float64_int64s(((float64)FrameInfo.DTS)/1000000)));
    }

    //Counting
    if (File_Offset+Buffer_Offset+Element_Size==File_Size)
        Frame_Count_Valid=Frame_Count; //Finish frames in case of there are less than Frame_Count_Valid frames
    Frame_Count++;
    Frame_Count_InThisBlock++;
    if (Frame_Count_NotParsedIncluded!=(int64u)-1)
        Frame_Count_NotParsedIncluded++;

    //Parsing
    BS_Begin();
    int8u ptype=(int8u)-1;
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
        Param_Info1(Vc1_PictureFormat[PictureFormat]);
        PictureFormat_Count[PictureFormat]++;

        if (PictureFormat==2) //Interlaced Field
        {
            int8u ptype_;
            Get_S1 ( 3, ptype_,                                 "ptype");
            if (ptype_<5)
            {
                Param_Info1(Vc1_Type[Vc1_FieldTypeTable[ptype_][0]]); Element_Info1(Vc1_Type[Vc1_FieldTypeTable[ptype_][0]]); //First field
                Param_Info1(Vc1_Type[Vc1_FieldTypeTable[ptype_][1]]); Element_Info1(Vc1_Type[Vc1_FieldTypeTable[ptype_][1]]); //Second field
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
            size_t ptype_;
            Get_VL (Vc1_ptype, ptype_,                          "ptype"); if (ptype_<5) {Param_Info1(Vc1_Type[Vc1_ptype[ptype_].mapped_to3]); Element_Info1(Vc1_Type[Vc1_ptype[ptype_].mapped_to3]);}
            ptype=(int8u)Vc1_ptype[ptype_].mapped_to3;
        }
        if (RefFramesCount<2 && (ptype==0 || ptype==1))
            RefFramesCount++;
        if (FrameInfo.DTS!=(int64u)-1)
        {
            if (framerate_present)
                FrameInfo.DTS+=float64_int64s(((float64)1000000000)/FrameRate);
        }
        if (FrameInfo.PTS!=(int64u)-1)
        {
            if (PTS_Begin==(int64u)-1 && ptype==0) //IFrame
                PTS_Begin=FrameInfo.PTS;
            if ((ptype==0 || ptype==1) && Frame_Count_InThisBlock<=1) //IFrame or PFrame
                PTS_End=FrameInfo.PTS;
            if ((ptype==0 || ptype==1) || (Frame_Count_InThisBlock>=2 && RefFramesCount>=2)) //IFrame or PFrame or more than 2 RefFrame for BFrames
            {
                if (framerate_present)
                    PTS_End+=float64_int64s(((float64)1000000000)/FrameRate);
            }
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
        if (!Status[IsFilled] && Frame_Count>=Frame_Count_Valid)
        {
            Fill("VC-1");

            if (!IsSub && MediaInfoLib::Config.ParseSpeed_Get()<1)
                Finish("VC-1");
        }

        #if MEDIAINFO_EVENTS
            {
                EVENT_BEGIN (Video, SliceInfo, 0)
                    Event.FieldPosition=Field_Count;
                    Event.SlicePosition=0;
                    switch (ptype)
                    {
                        case 0 :
                                    Event.SliceType=0; break;
                        case 1 :
                                    Event.SliceType=1; break;
                        case 2 :
                        case 3 :
                                    Event.SliceType=2; break;
                        case 4 :
                                    Event.SliceType=3; break;
                        default:
                                    Event.SliceType=(int8u)-1;
                    }
                    Event.Flags=0;
                EVENT_END   ()
            }
        #endif //MEDIAINFO_EVENTS
    FILLING_END();

    Synched=false; //We do not have the complete FrameHeader
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
            Element_Begin1("leaky_bucket");
            Skip_S1( 8,                                         "hrd_full");
            Element_End0();
        }
    TEST_SB_SKIP(                                               "coded_size_flag");
        Info_S2(12, coded_width,                                "coded_width"); Param_Info2((coded_width+1)*2, " pixels");
        Info_S2(12, coded_height,                               "coded_height"); Param_Info2((coded_height+1)*2, " pixels");
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

        #if MEDIAINFO_DEMUX
            if (InitData_Buffer_Size)
            {
                size_t InitData_Buffer_Temp_Size=InitData_Buffer_Size+(size_t)(Header_Size+Element_Size);
                int8u* InitData_Buffer_Temp=new int8u[InitData_Buffer_Temp_Size];
                std::memcpy(InitData_Buffer_Temp, InitData_Buffer, InitData_Buffer_Size);
                std::memcpy(InitData_Buffer_Temp+InitData_Buffer_Size, Buffer+Buffer_Offset-(size_t)Header_Size, (size_t)(Header_Size+Element_Size));

                switch (Config->Demux_InitData_Get())
                {
                    case 0 :    //In demux event
                                break; //Will be done in the first demux event
                    case 1 :    //In field
                                {
                                std::string Data_Raw((char*)InitData_Buffer_Temp, InitData_Buffer_Temp_Size);
                                std::string Data_Base64(Base64::encode(Data_Raw));
                                Fill(Stream_Video, StreamPos_Last, "Demux_InitBytes", Data_Base64);
                                (*Stream_More)[Stream_Video][StreamPos_Last](Ztring().From_Local("Demux_InitBytes"), Info_Options)=__T("N NT");
                                }
                                break;
                    default :   ;
                }

                delete[] InitData_Buffer; InitData_Buffer=NULL;
                delete[] InitData_Buffer_Temp; //InitData_Buffer_Temp=NULL;
                InitData_Buffer_Size=0;
            }
        #endif //MEDIAINFO_DEMUX
    FILLING_END();
}

//---------------------------------------------------------------------------
// Packet "0F"
void File_Vc1::SequenceHeader()
{
    Element_Name("SequenceHeader");

    //Parsing
    BS_Begin();
    Get_S1 ( 2, profile,                                        "profile"); Param_Info1(Vc1_Profile[profile]);
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
        Get_S1 ( 2, colordiff_format,                           "colordiff_format"); Param_Info1(Vc1_ColorimetryFormat[colordiff_format]);
        Skip_S1( 3,                                             "frmrtq_postproc");
        Skip_S1( 5,                                             "bitrtq_postproc");
        Skip_SB(                                                "postprocflag");
        Get_S2 (12, coded_width,                                "max_coded_width"); Param_Info2((coded_width+1)*2, " pixels");
        Get_S2 (12, coded_height,                               "max_coded_height"); Param_Info2((coded_height+1)*2, " pixels");
        Get_SB (    pulldown,                                   "pulldown");
        Get_SB (    interlace,                                  "interlace");
        Get_SB (    tfcntrflag,                                 "tfcntrflag - frame counter");
        Get_SB (    finterpflag,                                "finterpflag");
        Skip_SB(                                                "reserved");
        Get_SB (    psf,                                        "psf - progressive segmented frame");
        TEST_SB_SKIP(                                           "display_ext");
            Info_S2(14, display_x,                              "display_horiz_size"); Param_Info2(display_x+1, " pixels");
            Info_S2(14, display_y,                              "display_vert_size"); Param_Info2(display_y+1, " pixels");
            TEST_SB_SKIP(                                       "aspectratio_flag");
                Get_S1 ( 4, AspectRatio,                        "aspect_ratio"); Param_Info1(Vc1_PixelAspectRatio[AspectRatio]);
                if (AspectRatio==0x0F)
                {
                    Get_S1 ( 8, AspectRatioX,                   "aspect_horiz_size");
                    Get_S1 ( 8, AspectRatioY,                   "aspect_vert_size");
                }
            TEST_SB_END();
            TEST_SB_GET(framerate_present,                      "framerate_flag");
                TESTELSE_SB_GET(framerate_form,                 "framerateind");
                    Get_S2 (16, framerateexp,                   "framerateexp"); Param_Info3((float32)((framerateexp+1)/32.0), 3, " fps");
                TESTELSE_SB_ELSE(                               "framerateind");
                    Get_S1 ( 8, frameratecode_enr,              "frameratenr"); Param_Info1(Vc1_FrameRate_enr(frameratecode_enr));
                    Get_S1 ( 4, frameratecode_dr,               "frameratedr"); Param_Info1(Vc1_FrameRate_dr(frameratecode_dr));
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
                Element_Begin1("leaky_bucket");
                int16u hrd_buffer;
                Skip_S2(16,                                     "hrd_rate");
                Get_S2(16, hrd_buffer,                         "hrd_buffer");
                int32u hrd_buffer_value=(int32u)((hrd_buffer+1)*pow(2.0, 1+buffer_size_exponent)); Param_Info2(hrd_buffer_value, " bytes");
                Element_End0();

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

        //Frame rate
        if (framerate_present)
        {
            if (framerate_form)
                FrameRate=((float32)(framerateexp+1))/(float32)32;
            else if (Vc1_FrameRate_dr(frameratecode_dr))
                FrameRate=Vc1_FrameRate_enr(frameratecode_enr)/Vc1_FrameRate_dr(frameratecode_dr);
        }

        if (From_WMV3)
        {
            if (!Status[IsAccepted])
                Accept("VC-1");
            Finish("VC-1");
        }

        #if MEDIAINFO_DEMUX
            if (InitData_Buffer_Size)
            {
                InitData_Buffer_Size=(size_t)(Header_Size+Element_Size);
                InitData_Buffer=new int8u[InitData_Buffer_Size];
                std::memcpy(InitData_Buffer, Buffer+Buffer_Offset-(size_t)Header_Size, (size_t)(Header_Size+Element_Size));
            }
        #endif //MEDIAINFO_DEMUX
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

