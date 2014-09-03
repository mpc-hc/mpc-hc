/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// http://www.ffmpeg.org/~michael/ffv1.html
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
#if defined(MEDIAINFO_FFV1_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Video/File_Ffv1.h"
#include "ZenLib/BitStream.h"
//---------------------------------------------------------------------------

#include <algorithm>
using namespace std;

//---------------------------------------------------------------------------
namespace MediaInfoLib
{

//***************************************************************************
// RangeCoder
//***************************************************************************

//---------------------------------------------------------------------------
RangeCoder::RangeCoder (const int8u* Buffer, size_t Buffer_Size, const state_transitions default_state_transition)
{
    //Assign buffer
    Buffer_Cur=Buffer;
    Buffer_End=Buffer+Buffer_Size;

    //Init
    if (Buffer_Size>=2)
    {
        Current=BigEndian2int16u(Buffer_Cur);
        Buffer_Cur+=2;
        Mask=0xFF00;
    }
    else
    {
        Current=0;
        Mask=0;
    }

    //Assign StateTransitions
    std::memcpy (one_state, default_state_transition, state_transitions_size);
    zero_state[0]=0;
    for (size_t i=1; i<state_transitions_size; i++)
        zero_state[i]=-one_state[state_transitions_size-i];
}

//---------------------------------------------------------------------------
bool RangeCoder::get_rac(int8u States[])
{
    //Here is some black magic... But it works. TODO: better understanding of the algorithm and maybe optimization
    int16u Mask2=(int16u)((((int32u)Mask) * (*States)) >> 8);
    Mask-=Mask2;
    bool Value;
    if (Current<Mask)
    {
        *States=zero_state[*States];
        Value=false;
    }
    else
    {
        Current-=Mask;
        Mask=Mask2;
        *States=one_state[*States];
        Value=true;
    }

    // Next byte
    if (Mask<0x100)
    {
        if (Buffer_Cur >= Buffer_End)
        {
            //Problem
            Current=0;
            Mask=0;
            return false;
        }
        Mask<<=8;
        Current<<=8;
        Current|=*Buffer_Cur;
        Buffer_Cur++;
    }

    return Value;
}

//---------------------------------------------------------------------------
int8u RangeCoder::get_symbol_u(states &States)
{
    if (get_rac(States))
        return 0;

    int8u e=0;
    while (get_rac((States+1+min(e, (int8u)9)))) // 1..10
        e++;

    int8u a=1;
    if (e)
    {
        do
        {
            --e;
            a<<=1;
            if (get_rac((States+22+min(e, (int8u)9))))  // 22..31
                ++a;
        }
        while (e);
    }
    return a;
}

//---------------------------------------------------------------------------
int8u RangeCoder::get_symbol_s(states &States)
{
    if (get_rac(States))
        return 0;

    int8u e=0;
    while (get_rac(States+1+min(e, (int8u)9))) // 1..10
        e++;

    int8u a=1;
    if (e)
    {
        int8u i = e;
        do
        {
            --i;
            a<<=1;
            if (get_rac((States+22+min(i, (int8u)9))))  // 22..31
                ++a;
        }
        while (i);
    }

    if (get_rac((States+11+min(e, (int8u)10)))) // 11..21
        return -((int8s)a);
    else
        return a;
}

//***************************************************************************
// Info
//***************************************************************************

const char* Ffv1_coder_type(int8u coder_type)
{
    switch (coder_type)
    {
        case 0 :
                return "Golomb Rice";
        case 1 :
        case 2 :
                return "Range Coder";
        default:
                return "";
    }
}

const string Ffv1_colorspace_type(int8u colorspace_type, bool chroma_planes, bool alpha_plane)
{
    string ToReturn;
    switch (colorspace_type)
    {
        case 0 :
                    ToReturn=chroma_planes?"YUV":"Y";
                    break;
        case 1 :    ToReturn="RGB"; break;
        default:    return string();
    }

    if (alpha_plane)
        ToReturn+='A';

    return ToReturn;
}

const state_transitions Ffv1_default_state_transition =
{
      0,  0,  0,  0,  0,  0,  0,  0, 20, 21, 22, 23, 24, 25, 26, 27,
     28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 37, 38, 39, 40, 41, 42,
     43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 56, 57,
     58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73,
     74, 75, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88,
     89, 90, 91, 92, 93, 94, 94, 95, 96, 97, 98, 99,100,101,102,103,
    104,105,106,107,108,109,110,111,112,113,114,114,115,116,117,118,
    119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,133,
    134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,
    150,151,152,152,153,154,155,156,157,158,159,160,161,162,163,164,
    165,166,167,168,169,170,171,171,172,173,174,175,176,177,178,179,
    180,181,182,183,184,185,186,187,188,189,190,190,191,192,194,194,
    195,196,197,198,199,200,201,202,202,204,205,206,207,208,209,209,
    210,211,212,213,215,215,216,217,218,219,220,220,222,223,224,225,
    226,227,227,229,229,230,231,232,234,234,235,236,237,238,239,240,
    241,242,243,244,245,246,247,248,248,  0,  0,  0,  0,  0,  0,  0,
};

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Ffv1::File_Ffv1()
:File__Analyze()
{
    //Configuration
    ParserName=__T("FFV1");
    IsRawStream=true;

    //In
    IsOutOfBandData=false;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Ffv1::Streams_Accept()
{
    Stream_Prepare(Stream_Video);
    Fill(Stream_Video, 0, Video_Format, "FFV1");
}

//***************************************************************************
// RangeCoder
//***************************************************************************

#if MEDIAINFO_TRACE
//---------------------------------------------------------------------------
void File_Ffv1::Get_RC (states &States, bool &Info, const char* Name)
{
    Info=RC->get_rac(States);

    if (Trace_Activated)
    {
        Element_Offset=RC->Buffer_Cur-Buffer;
        Param(Name, Info);
    }
}

//---------------------------------------------------------------------------
void File_Ffv1::Get_RU (states &States, int8u &Info, const char* Name)
{
    Info=RC->get_symbol_u(States);

    if (Trace_Activated)
        Param(Name, Info);
}

//---------------------------------------------------------------------------
void File_Ffv1::Get_RS (states &States, int8s &Info, const char* Name)
{
    Info=RC->get_symbol_s(States);

    if (Trace_Activated)
        Param(Name, Info);
}

//---------------------------------------------------------------------------
void File_Ffv1::Skip_RC (states &States, const char* Name)
{
    if (Trace_Activated)
    {
        int8u Info=RC->get_rac(States);
        Element_Offset=RC->Buffer_Cur-Buffer;
        Param(Name, Info);
    }
    else
        RC->get_rac(States);
}

//---------------------------------------------------------------------------
void File_Ffv1::Skip_RU (states &States, const char* Name)
{
    if (Trace_Activated)
        Param(Name, RC->get_symbol_u(States));
    else
        RC->get_symbol_u(States);
}

//---------------------------------------------------------------------------
void File_Ffv1::Skip_RS (states &States, const char* Name)
{
    if (Trace_Activated)
        Param(Name, RC->get_symbol_s(States));
    else
        RC->get_symbol_s(States);
}

#else //MEDIAINFO_TRACE
//---------------------------------------------------------------------------
void File_Ffv1::Get_RC_ (states &States, bool &Info)
{
    Info=RC->get_rac(States);
}

//---------------------------------------------------------------------------
void File_Ffv1::Get_RU_ (states &States, int8u &Info)
{
    Info=RC->get_symbol_u(States);
}

//---------------------------------------------------------------------------
void File_Ffv1::Get_RS_ (states &States, int8s &Info)
{
    Info=RC->get_symbol_s(States);
}

//---------------------------------------------------------------------------
void File_Ffv1::Skip_RC_ (states &States)
{
    RC->get_rac(States);
}

//---------------------------------------------------------------------------
void File_Ffv1::Skip_RU_ (states &States)
{
    RC->get_symbol_u(States);
}

//---------------------------------------------------------------------------
void File_Ffv1::Skip_RS_ (states &States)
{
    RC->get_symbol_s(States);
}

#endif //MEDIAINFO_TRACE

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Ffv1::Read_Buffer_Continue()
{
    if (Buffer_Size<2)
    {
        Reject();
        return;
    }

    Accept();



    RC = new RangeCoder(Buffer, Buffer_Size, Ffv1_default_state_transition);
    states KeyStates;
    memset(KeyStates, 128, states_size);

    if (!IsOutOfBandData)
    {
        bool keyframe;
        Get_RC (KeyStates, keyframe,                            "keyframe");
    }

    FrameHeader();

    Skip_XX(Element_Size-Element_Offset,                        "Other data");

    Frame_Count++;
    Finish();
}

//***************************************************************************
// Elements
//***************************************************************************

//---------------------------------------------------------------------------
void File_Ffv1::FrameHeader()
{
    //Parsing
    states States;
    memset(States, 128, states_size);
    int8u version, micro_version=0, coder_type, colorspace_type, bits_per_raw_sample=8, chroma_h_shift, chroma_v_shift, num_h_slices_minus1=0, num_v_slices_minus1=0;
    bool chroma_planes, alpha_plane;
    Get_RU (States, version,                                    "version");
    if (( IsOutOfBandData && version<=1)
     || (!IsOutOfBandData && version> 1))
    {
        Trusted_IsNot("Invalid version in global header");
        return;
    }
    if (version>2)
    {
        RC->Buffer_End-=4;
        Get_RU (States, micro_version,                          "micro_version");
    }
    Get_RU (States, coder_type,                                 "coder_type");
    if (coder_type == 2) //Range coder with custom state transition table
    {
        for (int16u i = 1; i < state_transitions_size; i++)
        {
            Info_RS(States, StateTransition,                    "state_transition_delta"); Param_Info1(StateTransition+RC->one_state[i]);
        }
    }
    Get_RU (States, colorspace_type,                            "colorspace_type");
    if (version)
    {
        Get_RU (States, bits_per_raw_sample,                    "bits_per_raw_sample");
        if (bits_per_raw_sample==0)
            bits_per_raw_sample=8; //I don't know the reason, 8-bit is coded 0 and 10-bit coded 10 (not 2?).
    }
    Get_RC (States, chroma_planes,                              "chroma_planes");
    Get_RU (States, chroma_h_shift,                             "log2(h_chroma_subsample)");
    Get_RU (States, chroma_v_shift,                             "log2(v_chroma_subsample)");
    Get_RC (States, alpha_plane,                                "alpha_plane");
    if (version>1)
    {
        Get_RU (States, num_h_slices_minus1,                    "num_h_slices_minus1");
        Get_RU (States, num_v_slices_minus1,                    "num_v_slices_minus1");
        IsOutOfBandData=false;
    }

    if (Frame_Count==0)
    {
        Ztring Version=__T("Version ")+Ztring::ToZtring(version);
        if (version>2)
        {
            Version+=__T('.');
            Version+=Ztring::ToZtring(micro_version);
        }
        Fill(Stream_Video, 0, Video_Format_Version, Version);
        Fill(Stream_Video, 0, Video_BitDepth, bits_per_raw_sample);
        Fill(Stream_Video, 0, "coder_type", Ffv1_coder_type(coder_type));
        Fill(Stream_Video, 0, Video_ColorSpace, Ffv1_colorspace_type(colorspace_type, chroma_planes, alpha_plane));
        if (colorspace_type==0 && chroma_planes)
        {
            string ChromaSubsampling;
            switch (chroma_h_shift)
            {
                case 0 :
                        switch (chroma_v_shift)
                        {
                            case 0 : ChromaSubsampling="4:4:4"; break;
                            default: ;
                        }
                        break;
                case 1 :
                        switch (chroma_v_shift)
                        {
                            case 0 : ChromaSubsampling="4:2:2"; break;
                            case 1 : ChromaSubsampling="4:2:0"; break;
                            default: ;
                        }
                        break;
                case 2 :
                        switch (chroma_v_shift)
                        {
                            case 0 : ChromaSubsampling="4:1:1"; break;
                            case 1 : ChromaSubsampling="4:1:0"; break;
                            case 2 : ChromaSubsampling="4:1:0 (4x4)"; break;
                            default: ;
                        }
                        break;
                default: ;
            }
            if (!ChromaSubsampling.empty() && alpha_plane)
                ChromaSubsampling+=":4";
            Fill(Stream_Video, 0, Video_ChromaSubsampling, ChromaSubsampling);
        }
    }
}

} //NameSpace

#endif //MEDIAINFO_FFV1_YES
