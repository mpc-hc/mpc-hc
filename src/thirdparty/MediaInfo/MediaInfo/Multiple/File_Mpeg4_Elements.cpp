/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Contributor: Lionel Duchateau, kurtnoise@free.fr
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
#ifdef MEDIAINFO_MPEG4_YES
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Mpeg4.h"
#if defined(MEDIAINFO_DVDIF_YES)
    #include "MediaInfo/Multiple/File_DvDif.h"
#endif
#if defined(MEDIAINFO_MXF_YES)
    #include "MediaInfo/Multiple/File_Mxf.h"
#endif
#if defined(MEDIAINFO_AIC_YES)
    #include "MediaInfo/Video/File_Aic.h"
#endif
#if defined(MEDIAINFO_AVC_YES)
    #include "MediaInfo/Video/File_Avc.h"
#endif
#if defined(MEDIAINFO_H263_YES)
    #include "MediaInfo/Video/File_H263.h"
#endif
#if defined(MEDIAINFO_HEVC_YES)
    #include "MediaInfo/Video/File_Hevc.h"
#endif
#if defined(MEDIAINFO_MPEGV_YES)
    #include "MediaInfo/Video/File_Mpegv.h"
#endif
#if defined(MEDIAINFO_PRORES_YES)
    #include "MediaInfo/Video/File_ProRes.h"
#endif
#if defined(MEDIAINFO_VC1_YES)
    #include "MediaInfo/Video/File_Vc1.h"
#endif
#if defined(MEDIAINFO_VC3_YES)
    #include "MediaInfo/Video/File_Vc3.h"
#endif
#if defined(MEDIAINFO_AAC_YES)
    #include "MediaInfo/Audio/File_Aac.h"
#endif
#if defined(MEDIAINFO_AC3_YES)
    #include "MediaInfo/Audio/File_Ac3.h"
#endif
#if defined(MEDIAINFO_SMPTEST0337_YES)
    #include "MediaInfo/Audio/File_ChannelGrouping.h"
#endif
#if defined(MEDIAINFO_AMR_YES)
    #include "MediaInfo/Audio/File_Amr.h"
#endif
#if defined(MEDIAINFO_ADPCM_YES)
    #include "MediaInfo/Audio/File_Adpcm.h"
#endif
#if defined(MEDIAINFO_MPEGA_YES)
    #include "MediaInfo/Audio/File_Mpega.h"
#endif
#if defined(MEDIAINFO_PCM_YES)
    #include "MediaInfo/Audio/File_Pcm.h"
#endif
#if defined(MEDIAINFO_CDP_YES)
    #include "MediaInfo/Text/File_Cdp.h"
#endif
#if defined(MEDIAINFO_EIA608_YES)
    #include "MediaInfo/Text/File_Eia608.h"
#endif
#if defined(MEDIAINFO_TIMEDTEXT_YES)
    #include "MediaInfo/Text/File_TimedText.h"
#endif
#if defined(MEDIAINFO_JPEG_YES)
    #include "MediaInfo/Image/File_Jpeg.h"
#endif
#include "MediaInfo/Multiple/File_Mpeg4_TimeCode.h"
#include "ZenLib/FileName.h"
#include "base64.h"
#include <cmath>
#include <zlib.h>
#include <algorithm>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Infos
//***************************************************************************

//---------------------------------------------------------------------------
const char* Mpeg4_Meta_Kind(int32u Kind)
{
    switch (Kind)
    {
        case 0x00 : return "Binary";
        case 0x01 : return "UTF8";
        case 0x02 : return "UTF16";
        case 0x03 : return "Mac String";
        case 0x0E : return "Jpeg";
        case 0x15 : return "Signed Integer"; //the size of the integer is derived from the container size
        case 0x16 : return "Float 32";
        case 0x17 : return "Float 64";
        default   : return "Unknown";
    }
}

//---------------------------------------------------------------------------
const char* Mpeg4_TypeModifierName(int32u TypeModifierName)
{
    switch (TypeModifierName)
    {
        case 0x01 : return "Matrix";
        case 0x02 : return "Clip";
        case 0x03 : return "Volume";
        case 0x04 : return "Audio balance";
        case 0x05 : return "Graphic mode";
        case 0x06 : return "Matrix object";
        case 0x07 : return "Graphics mode object";
        case 0x76696465 : return "Image type";
        default   : return "Unknown";
    }
}

//---------------------------------------------------------------------------
const char* Mpeg4_sample_depends_on[]=
{
    "",
    "this sample does depend on others (not an I picture)",
    "this sample does not depend on others (I picture)",
    "reserved",
};

//---------------------------------------------------------------------------
const char* Mpeg4_sample_is_depended_on[]=
{
    "",
    "other samples depend on this one (not disposable)",
    "no other sample depends on this one (disposable)",
    "reserved",
};

//---------------------------------------------------------------------------
const char* Mpeg4_sample_has_redundancy[]=
{
    "",
    "there is redundant coding in this sample",
    "there is no redundant coding in this sample",
    "reserved",
};

//---------------------------------------------------------------------------
Ztring Mpeg4_Vendor(int32u Vendor)
{
    switch (Vendor)
    {
        case 0x46464D50 : return __T("FFMpeg");
        case 0x4D4F544F : return __T("Motorola");
        case 0x50484C50 : return __T("Philips");
        case 0x6170706C : return __T("Apple");
        case 0x6E6F6B69 : return __T("Nokia");
        case 0x6D6F746F : return __T("Motorola");
        default         : return Ztring().From_CC4(Vendor);
    }
}

//---------------------------------------------------------------------------
const char* Mpeg4_chan(int16u Ordering)
{
    //Source: http://developer.apple.com/library/mac/#documentation/MusicAudio/Reference/CACoreAudioReference/CoreAudioTypes/CompositePage.html
    //Arbitrary decision (0° = Front):
    //Front = 0°-80°
    //Side = 80°-120°
    //Rear = 120°-180°
    switch(Ordering)
    {
        case 100 : return "Front: C";
        case 101 : return "Front: L R";
        case 102 : return "Front: L R (Headphones)"; //With headphones
        case 103 : return "Front: L R (Matrix)"; //With matrix
        case 104 : return "Front: C S";
        case 105 : return "Front: X Y";
        case 106 : return "Front: L R (Binaural)";
        case 107 : return "Front: W X Y Z";
        case 108 : return "Front: L R, Side: L R";
        case 109 : return "Front: L C R, Rear: L R";
        case 110 : return "Front: L C R, Rear: L C R";
        case 111 : return "Front: L C R, Side: L R, Rear: L C R";
        case 112 : return "Front: L R, TopFront: L R, Rear: L R, TopRear: L R";
        case 113 : return "Front: L C R";
        case 114 : return "Front: L C R";
        case 115 : return "Front: L C R, Rear: C";
        case 116 : return "Front: L C R, Rear: C";
        case 117 : return "Front: L C R, Side: L R";
        case 118 : return "Front: L C R, Side: L R";
        case 119 : return "Front: L C R, Side: L R";
        case 120 : return "Front: L C R, Side: L R";
        case 121 : return "Front: L C R, Side: L R, LFE";
        case 122 : return "Front: L C R, Side: L R, LFE";
        case 123 : return "Front: L C R, Side: L R, LFE";
        case 124 : return "Front: L C R, Side: L R, LFE";
        case 125 : return "Front: L C R, Side: L C R";
        case 126 : return "Front: L Lc C Rc R, Side: L R";
        case 127 : return "Front: L Lc Rc R, Side: L R, LFE";
        case 128 : return "Front: L C R, Side: L R, Rear: L R, LFE";
        case 129 : return "Front: L C R, Side: L R, Rear: L R, LFE";
        case 130 : return "Front: L C R, Side: L R, LF, Front: L R (Matrix)";
        case 131 : return "Front: L R, Rear: C";
        case 132 : return "Front: L R, Side: L R";
        case 133 : return "Front: L R, LFE";
        case 134 : return "Front: L R, Rear: C, LFE";
        case 135 : return "Front: L C R, Side: L R, LFE";
        case 136 : return "Front: L C R, LFE";
        case 137 : return "Front: L C R, Rear: C, LFE";
        case 138 : return "Front: L R, Rear: L R, LFE";
        case 139 : return "Front: L C R, Rear: L R, LFE";
        case 140 : return "Front: L C R, Side: L R, Rear: L R";
        case 141 : return "Front: L C R, Side: L R, Rear: C";
        case 142 : return "Front: L C R, Side: L R, Rear: C, LFE";
        case 143 : return "Front: L C R, Side: L R, Rear: L R";
        case 144 : return "Front: L C R, Side: L R, Rear: L C R";
        case 145 : return "Front: Lw L C R Rw, TopFront: L C R, Side: L R, Rear: L C Cd R, LFE: L R"; //d=direct, all must be confirmed
        case 146 : return "Front: Lw L Lc C Rc R Rw, TopFront: L C R, Side: L R, Rear: L C Cd R, LFE: L R, HI, VI, Haptic"; //d=direct, all must be confirmed
        default  : return "";
    }
}

//---------------------------------------------------------------------------
const char* Mpeg4_chan_Layout(int16u Ordering)
{
    //Source: http://developer.apple.com/library/mac/#documentation/MusicAudio/Reference/CACoreAudioReference/CoreAudioTypes/CompositePage.html
    // L - left
    // R - right
    // C - center
    // Ls - left surround
    // Rs - right surround
    // Cs - center surround
    // Rls - rear left surround
    // Rrs - rear right surround
    // Lw - left wide
    // Rw - right wide
    // Lsd - left surround direct
    // Rsd - right surround direct
    // Lc - left center
    // Rc - right center
    // Ts - top surround
    // Vhl - vertical height left
    // Vhc - vertical height center
    // Vhr - vertical height right
    // Lt - left matrix total. for matrix encoded stereo.
    // Rt - right matrix total. for matrix encoded stereo.
    switch(Ordering)
    {
        case 100 : return "C";
        case 101 : return "L R";
        case 102 : return "L R"; //With headphones
        case 103 : return "Lt Rt"; //L R with matrix
        case 104 : return "M S";
        case 105 : return "X Y";
        case 106 : return "L R";
        case 107 : return "W X Y Z";
        case 108 : return "L R Ls Rs";
        case 109 : return "L R Lrs Rrs C";
        case 110 : return "L R Lrs Rrs C Cs";
        case 111 : return "L R Lrs Rrs C Crs, Ls, Rs";
        case 112 : return "L R Lrs Rrs Vhl Vhr, Vhlrs, Vhrrs";
        case 113 : return "L R C";
        case 114 : return "C L R";
        case 115 : return "L R C Cs";
        case 116 : return "C L R Cs";
        case 117 : return "L R C Ls Rs";
        case 118 : return "L R Ls Rs C";
        case 119 : return "L C R Ls Rs";
        case 120 : return "C L R Ls Rs";
        case 121 : return "L R C LFE Ls Rs";
        case 122 : return "L R Ls Rs C LFE";
        case 123 : return "L C R Ls Rs LFE";
        case 124 : return "C L R Ls Rs LFE";
        case 125 : return "L R C LFE Ls Rs Cs";
        case 126 : return "L R C LFE Ls Rs Lc Rc";
        case 127 : return "C Lc Rc L R Ls Rs LFE";
        case 128 : return "L R C LFE Ls R Rls Rrs";
        case 129 : return "L R Ls Rs C LFE Lc Rc";
        case 130 : return "L R C LFE Ls Rs Lt Rt";
        case 131 : return "L R Cs";
        case 132 : return "L R Ls Rs";
        case 133 : return "L R LFE";
        case 134 : return "L R LFE Cs";
        case 135 : return "L R LFE Ls Rs";
        case 136 : return "L R C LFE";
        case 137 : return "L R C LFE Cs";
        case 138 : return "L R Ls Rs LFE";
        case 139 : return "L R Ls Rs C Cs";
        case 140 : return "L R Ls Rs C Rls Rrs";
        case 141 : return "C L R Ls Rs Cs ";
        case 142 : return "C L R Ls Rs Cs LFE";
        case 143 : return "C L R Ls Rs Rls Rrs";
        case 144 : return "C L R Ls Rs Rls Rrs Cs";
        case 145 : return "L R C Vhc Lsd Rsd Ls Rs Vhl Vhr Lw Rw Csd Cs LFE1 LFE2";
        case 146 : return "L R C Vhc Lsd Rsd Ls Rs Vhl Vhr Lw Rw Csd Cs LFE1 LFE2 Lc Rc HI VI Haptic";
        case 147 : return "";
        default  : return "";
    }
}

//---------------------------------------------------------------------------
std::string Mpeg4_chan_ChannelDescription (int64u ChannelLabels)
{
    std::string Text;
    if ((ChannelLabels&0x000E)!=0x0000)
        Text+="Front:";
    if (ChannelLabels&0x0002)
        Text+=" L";
    if (ChannelLabels&0x0008)
        Text+=" C";
    if (ChannelLabels&0x0004)
        Text+=" R";

    if ((ChannelLabels&0x0C00)!=0x0000)
    {
        if (!Text.empty())
            Text+=", ";
        Text+="Side:";
    }
    if (ChannelLabels&0x0400)
        Text+=" L";
    if (ChannelLabels&0x0800)
        Text+=" R";

    if ((ChannelLabels&0x0260)!=0x0000)
    {
        if (!Text.empty())
            Text+=", ";
        Text+="Back:";
    }
    if (ChannelLabels&0x0020)
        Text+=" L";
    if (ChannelLabels&0x0200)
        Text+=" C";
    if (ChannelLabels&0x0040)
        Text+=" R";

    if ((ChannelLabels&0x0010)!=0x0000)
    {
        if (!Text.empty())
            Text+=", ";
        Text+="LFE";
    }

    if (ChannelLabels&0x000000C000000000LL)
    {
        if (!Text.empty())
            Text+=", ";
        Text+="Front:";
    }
    if (ChannelLabels&0x0000004000000000LL)
        Text+=" L";
    if (ChannelLabels&0x0000008000000000LL)
        Text+=" R";
    if (ChannelLabels&0x000000C000000000LL)
        Text+=" (Matrix)";

    return Text;
}

//---------------------------------------------------------------------------
const char* Mpeg4_chan_ChannelDescription_Layout (int32u ChannelLabel)
{
    switch(ChannelLabel)
    {
        case   1 : return "L";
        case   2 : return "R";
        case   3 : return "C";
        case   4 : return "LFE";
        case   5 : return "Ls";
        case   6 : return "Rs";
        case   7 : return "Lc";
        case   8 : return "Rc";
        case   9 : return "Cs";
        case  10 : return "Lsd";
        case  11 : return "Rsd";
        case  12 : return "Tcs";
        case  13 : return "Vhl";
        case  14 : return "Vhc";
        case  15 : return "Vhr";
        case  16 : return "Trs";
        case  17 : return "Trs";
        case  18 : return "Trs";
        case  33 : return "Lrs";
        case  34 : return "Rrs";
        case  35 : return "Lw";
        case  36 : return "Rw";
        case  37 : return "LFE2";
        case  38 : return "Lt";
        case  39 : return "Rt";
        case 200 : return "W";
        case 201 : return "X";
        case 202 : return "Y";
        case 203 : return "Z";
        case 204 : return "M";
        case 205 : return "S";
        case 206 : return "X";
        case 207 : return "Y";
        case 0x10000 : return "Discrete-0";
        case 0x10001 : return "Discrete-1";
        case 0x10002 : return "Discrete-2";
        case 0x10003 : return "Discrete-3";
        case 0x10004 : return "Discrete-4";
        case 0x10005 : return "Discrete-5";
        case 0x10006 : return "Discrete-6";
        case 0x10007 : return "Discrete-7";
        case 0x10008 : return "Discrete-8";
        case 0x10009 : return "Discrete-9";
        case 0x1000A : return "Discrete-10";
        case 0x1000B : return "Discrete-11";
        case 0x1000C : return "Discrete-12";
        case 0x1000D : return "Discrete-13";
        case 0x1000E : return "Discrete-14";
        case 0x1000F : return "Discrete-15";
        default  : return "?";
    }
}

//---------------------------------------------------------------------------
std::string Mpeg4_chan_ChannelBitmap (int32u ChannelBitmap)
{
    std::string Text;
    if ((ChannelBitmap&0x0007)!=0x0000)
        Text+="Front:";
    if (ChannelBitmap&0x0001)
        Text+=" L";
    if (ChannelBitmap&0x0004)
        Text+=" C";
    if (ChannelBitmap&0x0002)
        Text+=" R";

    if ((ChannelBitmap&0x0600)!=0x0000)
        Text+=", Side:";
    if (ChannelBitmap&0x0200)
        Text+=" L";
    if (ChannelBitmap&0x0400)
        Text+=" R";

    if ((ChannelBitmap&0x0130)!=0x0000)
        Text+=", Back:";
    if (ChannelBitmap&0x0010)
        Text+=" L";
    if (ChannelBitmap&0x0100)
        Text+=" C";
    if (ChannelBitmap&0x0020)
        Text+=" R";

    if ((ChannelBitmap&0x0008)!=0x0000)
        Text+=", LFE";

    return Text;
}

//---------------------------------------------------------------------------
const char* Mpeg4_chan_ChannelBitmap_Layout (int32u ChannelBitmap)
{
    switch(ChannelBitmap)
    {
        case   0 : return "L";      // Left
        case   1 : return "R";      // Right
        case   2 : return "C";      // Center
        case   3 : return "LFE";    // LFEScreen
        case   4 : return "Ls";     // LeftSurround / Back Left
        case   5 : return "Rs";     // RightSurround / Back Right
        case   6 : return "?";      // LeftCenter
        case   7 : return "?";      // RightCenter
        case   8 : return "Cs";     // CenterSurround / Back Center
        case   9 : return "Lsd";    // LeftSurroundDirect / Side Left
        case  10 : return "Rsd";    // RightSurroundDirect / Side Right
        case  11 : return "?";      // TopCenterSurround
        case  12 : return "?";      // VerticalHeightLeft / Top Front Left
        case  13 : return "?";      // VerticalHeightCenter / Top Front Center
        case  14 : return "?";      // VerticalHeightRight / Top Front Right"
        case  15 : return "?";      // TopBackLeft
        case  16 : return "?";      // TopBackCenter
        case  17 : return "?";      // TopBackRight
        default  : return "?";
    }
}

//---------------------------------------------------------------------------
const char* Mpeg4_jp2h_METH(int8u METH)
{
    switch (METH)
    {
        case 0x01 : return "Enumerated colourspace";
        case 0x02 : return "Restricted ICC profile";
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* Mpeg4_jp2h_EnumCS(int32u EnumCS)
{
    switch (EnumCS)
    {
        case 0x10 : return "RGB"; //sRGB
        case 0x11 : return "Y";
        case 0x12 : return "YUV"; //sYUV
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* Mpegv_colour_primaries(int8u colour_primaries);
const char* Mpegv_transfer_characteristics(int8u transfer_characteristics);
const char* Mpegv_matrix_coefficients(int8u matrix_coefficients);

//---------------------------------------------------------------------------
// DTS
#if defined(MEDIAINFO_DTS_YES)
    extern const char*  DTS_ChannelPositions[16];
    extern const char*  DTS_ChannelPositions2[16];
    extern std::string DTS_HD_SpeakerActivityMask (int16u SpeakerActivityMask);
    extern std::string DTS_HD_SpeakerActivityMask2 (int16u SpeakerActivityMask);
#endif //defined(MEDIAINFO_DTS_YES)

//***************************************************************************
// Constants
//***************************************************************************

//---------------------------------------------------------------------------
namespace Elements
{
    const int64u bloc=0x626C6F63;
    const int64u cdat=0x63646174;
    const int64u cdt2=0x63647432;
    const int64u free=0x66726565;
    const int64u ftyp=0x66747970;
    const int64u ftyp_qt=0x71742020;
    const int64u ftyp_dash=0x64617368;
    const int64u ftyp_isom=0x69736F6D;
    const int64u ftyp_caqv=0x63617176;
    const int64u idat=0x69646174;
    const int64u idsc=0x69647363;
    const int64u jp2c=0x6A703263;
    const int64u jp2h=0x6A703268;
    const int64u jp2h_ihdr=0x69686472;
    const int64u jp2h_colr=0x636F6C72;
    const int64u mdat=0x6D646174;
    const int64u mfra=0x6D667261;
    const int64u mfra_mfro=0x6D66726F;
    const int64u mfra_tfra=0x74667261;
    const int64u moof=0x6D6F6F66;
    const int64u moof_mfhd=0x6D666864;
    const int64u moof_traf=0x74726166;
    const int64u moof_traf_sdtp=0x73647470;
    const int64u moof_traf_tfhd=0x74666864;
    const int64u moof_traf_trun=0x7472756E;
    const int64u moov=0x6D6F6F76;
    const int64u moov_ainf=0x61696E66;
    const int64u moov_cmov=0x636D6F76;
    const int64u moov_cmov_cmvd=0x636D7664;
    const int64u moov_cmov_dcom=0x64636F6D;
    const int64u moov_cmov_dcom_zlib=0x7A6C6962;
    const int64u moov_ctab=0x63746162;
    const int64u moov_iods=0x696F6473;
    const int64u moov_meta=0x6D657461;
    const int64u moov_meta______=0x2D2D2D2D;
    const int64u moov_meta___day=0xA9646179;
    const int64u moov_meta__disk=0x6469736B;
    const int64u moov_meta__trkn=0x74726B6E;
    const int64u moov_meta__trng=0x74726E67;
    const int64u moov_meta__covr=0x636F7672;
    const int64u moov_meta__gnre=0x676E7265;
    const int64u moov_meta_bxml=0x62786D6C;
    const int64u moov_meta_hdlr=0x68646C72;
    const int64u moov_meta_hdlr_mdir=0x6D646972;
    const int64u moov_meta_hdlr_mdta=0x6D647461;
    const int64u moov_meta_hdlr_mp7b=0x6D703762;
    const int64u moov_meta_hdlr_mp7t=0x6D703774;
    const int64u moov_meta_keys=0x6B657973;
    const int64u moov_meta_keys_mdta=0x6D647461;
    const int64u moov_meta_ilst=0x696C7374;
    const int64u moov_meta_ilst_xxxx_data=0x64617461;
    const int64u moov_meta_ilst_xxxx_mean=0x6D65616E;
    const int64u moov_meta_ilst_xxxx_name=0x6E616D65;
    const int64u moov_meta_xml=0x786D6C20;
    const int64u moov_mvex=0x6D766578;
    const int64u moov_mvex_mehd=0x6D656864;
    const int64u moov_mvex_trex=0x74726578;
    const int64u moov_mvhd=0x6D766864;
    const int64u moov_trak=0x7472616B;
    const int64u moov_trak_edts=0x65647473;
    const int64u moov_trak_edts_elst=0x656C7374;
    const int64u moov_trak_load=0x6C6F6164;
    const int64u moov_trak_mdia=0x6D646961;
    const int64u moov_trak_mdia_hdlr=0x68646C72;
    const int64u moov_trak_mdia_hdlr_clcp=0x636C6370;
    const int64u moov_trak_mdia_hdlr_MPEG=0x4D504547;
    const int64u moov_trak_mdia_hdlr_sbtl=0x7362746C;
    const int64u moov_trak_mdia_hdlr_soun=0x736F756E;
    const int64u moov_trak_mdia_hdlr_subp=0x73756270;
    const int64u moov_trak_mdia_hdlr_text=0x74657874;
    const int64u moov_trak_mdia_hdlr_tmcd=0x746D6364;
    const int64u moov_trak_mdia_hdlr_vide=0x76696465;
    const int64u moov_trak_mdia_imap=0x696D6170;
    const int64u moov_trak_mdia_imap_sean=0x7365616E;
    const int64u moov_trak_mdia_imap_sean___in=0x0000696E;
    const int64u moov_trak_mdia_imap_sean___in___ty=0x00007479;
    const int64u moov_trak_mdia_imap_sean___in_dtst=0x64747374;
    const int64u moov_trak_mdia_imap_sean___in_obid=0x6F626964;
    const int64u moov_trak_mdia_mdhd=0x6D646864;
    const int64u moov_trak_mdia_minf=0x6D696E66;
    const int64u moov_trak_mdia_minf_code=0x636F6465;
    const int64u moov_trak_mdia_minf_code_sean=0x7365616E;
    const int64u moov_trak_mdia_minf_code_sean_RU_A=0x52552A41;
    const int64u moov_trak_mdia_minf_dinf=0x64696E66;
    const int64u moov_trak_mdia_minf_dinf_url_=0x75726C20;
    const int64u moov_trak_mdia_minf_dinf_urn_=0x75726E20;
    const int64u moov_trak_mdia_minf_dinf_dref=0x64726566;
    const int64u moov_trak_mdia_minf_dinf_dref_alis=0x616C6973;
    const int64u moov_trak_mdia_minf_dinf_dref_rsrc=0x72737263;
    const int64u moov_trak_mdia_minf_dinf_dref_url_=0x75726C20;
    const int64u moov_trak_mdia_minf_dinf_derf_urn_=0x75726E20;
    const int64u moov_trak_mdia_minf_gmhd=0x676D6864;
    const int64u moov_trak_mdia_minf_gmhd_gmin=0x676D696E;
    const int64u moov_trak_mdia_minf_gmhd_tmcd=0x746D6364;
    const int64u moov_trak_mdia_minf_gmhd_tmcd_tcmi=0x74636D69;
    const int64u moov_trak_mdia_minf_gmhd_tcmi=0x74636D69;
    const int64u moov_trak_mdia_minf_hint=0x68696E74;
    const int64u moov_trak_mdia_minf_hdlr=0x68646C72;
    const int64u moov_trak_mdia_minf_hmhd=0x686D6864;
    const int64u moov_trak_mdia_minf_nmhd=0x6E6D6864;
    const int64u moov_trak_mdia_minf_smhd=0x736D6864;
    const int64u moov_trak_mdia_minf_stbl=0x7374626C;
    const int64u moov_trak_mdia_minf_stbl_co64=0x636F3634;
    const int64u moov_trak_mdia_minf_stbl_cslg=0x63736C67;
    const int64u moov_trak_mdia_minf_stbl_ctts=0x63747473;
    const int64u moov_trak_mdia_minf_stbl_sdtp=0x73647470;
    const int64u moov_trak_mdia_minf_stbl_stco=0x7374636F;
    const int64u moov_trak_mdia_minf_stbl_stdp=0x73746470;
    const int64u moov_trak_mdia_minf_stbl_stps=0x73747073;
    const int64u moov_trak_mdia_minf_stbl_stsc=0x73747363;
    const int64u moov_trak_mdia_minf_stbl_stsd=0x73747364;
    const int64u moov_trak_mdia_minf_stbl_stsd_alac=0x616C6163;
    const int64u moov_trak_mdia_minf_stbl_stsd_mp4a=0x6D703461;
    const int64u moov_trak_mdia_minf_stbl_stsd_mp4s=0x6D703473;
    const int64u moov_trak_mdia_minf_stbl_stsd_mp4v=0x6D703476;
    const int64u moov_trak_mdia_minf_stbl_stsd_text=0x74657874;
    const int64u moov_trak_mdia_minf_stbl_stsd_tmcd=0x746D6364;
    const int64u moov_trak_mdia_minf_stbl_stsd_tmcd_name=0x6E616D65;
    const int64u moov_trak_mdia_minf_stbl_stsd_tx3g=0x74783367;
    const int64u moov_trak_mdia_minf_stbl_stsd_tx3g_ftab=0x66746162;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_alac=0x616C6163;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_ACLR=0x41434C52;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_APRG=0x41505247;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_ARES=0x41524553;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_avcC=0x61766343;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_bitr=0x62697472;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_btrt=0x62747274;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_clap=0x636C6170;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_chan=0x6368616E;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_colr=0x636F6C72;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_d263=0x64323633;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_dac3=0x64616333;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_damr=0x64616D72;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_dec3=0x64656333;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_ddts=0x64647473;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_dvc1=0x64766331;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_esds=0x65736473;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_fiel=0x6669656C;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_glbl=0x676C626C;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_hvcC=0x68766343;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_idfm=0x6964666D;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_idfm_atom=0x61746F6D;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_idfm_qtat=0x71746174;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_idfm_fxat=0x66786174;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_idfm_priv=0x70726976;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_idfm_subs=0x73756273;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_idfm_cspc=0x63737063;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_jp2h=0x6A703268;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_jp2h_colr=0x636F6C72;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_jp2h_ihdr=0x69686472;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_pasp=0x70617370;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_sinf=0x73696E66;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_sinf_frma=0x66726D61;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_sinf_imif=0x696D6966;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_sinf_schi=0x73636869;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_sinf_schm=0x7363686D;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_wave=0x77617665;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_wave_acbf=0x61636266;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_wave_enda=0x656E6461;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_wave_frma=0x66726D61;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_wave_samr=0x73616D72;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_wave_srcq=0x73726371;
    const int64u moov_trak_mdia_minf_stbl_stsh=0x73747368;
    const int64u moov_trak_mdia_minf_stbl_stss=0x73747373;
    const int64u moov_trak_mdia_minf_stbl_stsz=0x7374737A;
    const int64u moov_trak_mdia_minf_stbl_stts=0x73747473;
    const int64u moov_trak_mdia_minf_stbl_stz2=0x73747A32;
    const int64u moov_trak_mdia_minf_vmhd=0x766D6864;
    const int64u moov_trak_meta=0x6D657461;
    const int64u moov_trak_meta______=0x2D2D2D2D;
    const int64u moov_trak_meta___day=0xA9646179;
    const int64u moov_trak_meta__disk=0x6469736B;
    const int64u moov_trak_meta__trkn=0x74726B6E;
    const int64u moov_trak_meta__trng=0x74726E67;
    const int64u moov_trak_meta__covr=0x636F7672;
    const int64u moov_trak_meta__gnre=0x676E7265;
    const int64u moov_trak_meta_bxml=0x62786D6C;
    const int64u moov_trak_meta_hdlr=0x68646C72;
    const int64u moov_trak_meta_hdlr_mdir=0x6D646972;
    const int64u moov_trak_meta_hdlr_mdta=0x6D647461;
    const int64u moov_trak_meta_hdlr_mp7b=0x6D703762;
    const int64u moov_trak_meta_hdlr_mp7t=0x6D703774;
    const int64u moov_trak_meta_keys=0x6B657973;
    const int64u moov_trak_meta_keys_mdta=0x6D647461;
    const int64u moov_trak_meta_ilst=0x696C7374;
    const int64u moov_trak_meta_ilst_xxxx_data=0x64617461;
    const int64u moov_trak_meta_ilst_xxxx_mean=0x6D65616E;
    const int64u moov_trak_meta_ilst_xxxx_name=0x6E616D65;
    const int64u moov_trak_meta_xml=0x786D6C20;
    const int64u moov_trak_tapt=0x74617074;
    const int64u moov_trak_tapt_clef=0x636C6566;
    const int64u moov_trak_tapt_enof=0x656E6F66;
    const int64u moov_trak_tapt_prof=0x70726F66;
    const int64u moov_trak_tkhd=0x746B6864;
    const int64u moov_trak_tref=0x74726566;
    const int64u moov_trak_tref_dpnd=0x64706E64;
    const int64u moov_trak_tref_chap=0x63686170;
    const int64u moov_trak_tref_clcp=0x636C6370;
    const int64u moov_trak_tref_fall=0x66616C6C;
    const int64u moov_trak_tref_folw=0x666F6C77;
    const int64u moov_trak_tref_forc=0x666F7263;
    const int64u moov_trak_tref_hint=0x68696E74;
    const int64u moov_trak_tref_ipir=0x69706972;
    const int64u moov_trak_tref_mpod=0x6D706F64;
    const int64u moov_trak_tref_scpt=0x73637074;
    const int64u moov_trak_tref_ssrc=0x73737263;
    const int64u moov_trak_tref_sync=0x73796E63;
    const int64u moov_trak_tref_tmcd=0x746D6364;
    const int64u moov_udta=0x75647461;
    const int64u moov_udta_AllF=0x416C6C46;
    const int64u moov_udta_chpl=0x6368706C;
    const int64u moov_udta_clsf=0x636C7366;
    const int64u moov_udta_DcMD=0x44634D44;
    const int64u moov_udta_DcMD_Cmbo=0x436D626F;
    const int64u moov_udta_DcMD_DcME=0x44634D45;
    const int64u moov_udta_DcMD_DcME_Keyw=0x4B657977;
    const int64u moov_udta_DcMD_DcME_Mtmd=0x4D746D64;
    const int64u moov_udta_DcMD_DcME_Rate=0x52617465;
    const int64u moov_udta_FIEL=0x4649454C;
    const int64u moov_udta_FXTC=0x46585443;
    const int64u moov_udta_hinf=0x68696E66;
    const int64u moov_udta_hinv=0x68696E76;
    const int64u moov_udta_hnti=0x686E7469;
    const int64u moov_udta_hnti_rtp=0x72747020;
    const int64u moov_udta_ID32=0x49443332;
    const int64u moov_udta_kywd=0x6B797764;
    const int64u moov_udta_loci=0x6C6F6369;
    const int64u moov_udta_LOOP=0x4C4F4F50;
    const int64u moov_udta_MCPS=0x4D435053;
    const int64u moov_udta_meta=0x6D657461;
    const int64u moov_udta_meta_hdlr=0x68646C72;
    const int64u moov_udta_meta_ilst=0x696C7374;
    const int64u moov_udta_meta_ilst_xxxx_data=0x64617461;
    const int64u moov_udta_meta_ilst_xxxx_mean=0x6D65616E;
    const int64u moov_udta_meta_ilst_xxxx_name=0x6E616D65;
    const int64u moov_udta_ndrm=0x6E64726D;
    const int64u moov_udta_nsav=0x6E736176;
    const int64u moov_udta_ptv =0x70747620;
    const int64u moov_udta_rtng=0x72746E67;
    const int64u moov_udta_Sel0=0x53656C30;
    const int64u moov_udta_tags=0x74616773;
    const int64u moov_udta_tags_meta=0x6D657461;
    const int64u moov_udta_tags_tseg=0x74736567;
    const int64u moov_udta_tags_tseg_tshd=0x74736864;
    const int64u moov_udta_WLOC=0x574C4F43;
    const int64u moov_udta_XMP_=0x584D505F;
    const int64u moov_udta_yrrc=0x79727263;
    const int64u pdin=0x7064696E;
    const int64u PICT=0x50494354;
    const int64u pckg=0x70636B67;
    const int64u pnot=0x706E6F74;
    const int64u RDAO=0x5244414F;
    const int64u RDAS=0x52444153;
    const int64u RDVO=0x5244564F;
    const int64u RDVS=0x52445653;
    const int64u RED1=0x52454431;
    const int64u REDA=0x52454441;
    const int64u REDV=0x52454456;
    const int64u REOB=0x52454F42;
    const int64u skip=0x736B6970;
    const int64u wide=0x77696465;
}

//---------------------------------------------------------------------------
const char* Mpeg4_Description(int32u Description)
{
    switch (Description)
    {
        case Elements::moov_trak_mdia_minf_stbl_stsd_xxxx_idfm_atom : return "Classic atom structure";
        case Elements::moov_trak_mdia_minf_stbl_stsd_xxxx_idfm_qtat : return "QT atom structure";
        case Elements::moov_trak_mdia_minf_stbl_stsd_xxxx_idfm_fxat : return "Effect";
        case Elements::moov_trak_mdia_minf_stbl_stsd_xxxx_idfm_priv : return "Private";
        case Elements::moov_trak_mdia_minf_stbl_stsd_xxxx_idfm_subs : return "Substitute if main codec not available";
        case Elements::moov_trak_mdia_minf_stbl_stsd_xxxx_idfm_cspc : return "Native pixel format";
        default                                                     : return "Unknown";
    }
}

//***************************************************************************
// Format
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mpeg4::Data_Parse()
{
    //mdat
    if (IsParsing_mdat)
    {
        mdat_xxxx();
        return;
    }

    //Parsing
    DATA_BEGIN
    ATOM(bloc)
    ATOM(cdat)
    ATOM(cdt2)
    LIST_SKIP(free)
    ATOM(ftyp)
    ATOM(idat)
    ATOM(idsc)
    ATOM(jp2c)
    LIST(jp2h)
        ATOM_BEGIN
        ATOM(jp2h_colr)
        ATOM(jp2h_ihdr)
        ATOM_END
    LIST(mdat)
        ATOM_DEFAULT_ALONE(mdat_xxxx)
    LIST(mfra)
        ATOM_BEGIN
        ATOM(mfra_mfro)
        ATOM(mfra_tfra)
        ATOM_END
    LIST(moof)
        ATOM_BEGIN
        ATOM(moof_mfhd)
        LIST(moof_traf)
            ATOM_BEGIN
            ATOM(moof_traf_sdtp)
            ATOM(moof_traf_tfhd)
            ATOM(moof_traf_trun)
            ATOM_END
        ATOM_END
    LIST(moov)
        ATOM_BEGIN
        ATOM(moov_ainf)
        LIST(moov_cmov)
            ATOM_BEGIN
            ATOM(moov_cmov_dcom)
            ATOM(moov_cmov_cmvd)
            ATOM_END
        ATOM(moov_ctab)
        ATOM(moov_iods)
        LIST(moov_meta)
            ATOM_BEGIN
            ATOM(moov_meta_bxml)
            LIST(moov_meta_keys)
                ATOM_BEGIN
                ATOM(moov_meta_keys_mdta)
                ATOM_END
            ATOM(moov_meta_hdlr)
            LIST(moov_meta_ilst)
                LIST_DEFAULT_ALONE_BEGIN (moov_meta_ilst_xxxx)
                    ATOM_BEGIN
                    ATOM (moov_meta_ilst_xxxx_data)
                    ATOM (moov_meta_ilst_xxxx_mean)
                    ATOM (moov_meta_ilst_xxxx_name)
                    ATOM_END
                LIST_DEFAULT_ALONE_END
            ATOM(moov_meta_xml)
            ATOM_END
        LIST(moov_mvex)
            ATOM_BEGIN
            ATOM(moov_mvex_mehd)
            ATOM(moov_mvex_trex)
            ATOM_END
        ATOM(moov_mvhd)
        LIST(moov_trak)
            ATOM_BEGIN
            LIST(moov_trak_edts)
                ATOM_BEGIN
                ATOM(moov_trak_edts_elst)
                ATOM_END
            ATOM(moov_trak_load)
            LIST(moov_trak_mdia)
                ATOM_BEGIN
                ATOM(moov_trak_mdia_hdlr)
                LIST(moov_trak_mdia_imap)
                    ATOM_BEGIN
                    LIST(moov_trak_mdia_imap_sean)
                        ATOM_BEGIN
                        LIST(moov_trak_mdia_imap_sean___in)
                            ATOM_BEGIN
                            ATOM(moov_trak_mdia_imap_sean___in___ty)
                            ATOM(moov_trak_mdia_imap_sean___in_dtst)
                            ATOM(moov_trak_mdia_imap_sean___in_obid)
                            ATOM_END
                        ATOM_END
                    ATOM_END
                ATOM(moov_trak_mdia_mdhd)
                LIST(moov_trak_mdia_minf)
                    ATOM_BEGIN
                    LIST(moov_trak_mdia_minf_code)
                        ATOM_BEGIN
                        LIST(moov_trak_mdia_minf_code_sean)
                            ATOM_BEGIN
                            ATOM(moov_trak_mdia_minf_code_sean_RU_A)
                            ATOM_END
                        ATOM_END
                    LIST(moov_trak_mdia_minf_dinf)
                        ATOM_BEGIN
                        ATOM(moov_trak_mdia_minf_dinf_url_)
                        ATOM(moov_trak_mdia_minf_dinf_urn_)
                        LIST(moov_trak_mdia_minf_dinf_dref)
                            ATOM_BEGIN
                            ATOM(moov_trak_mdia_minf_dinf_dref_alis)
                            ATOM(moov_trak_mdia_minf_dinf_dref_rsrc)
                            ATOM(moov_trak_mdia_minf_dinf_url_)
                            ATOM(moov_trak_mdia_minf_dinf_urn_)
                            ATOM_END
                        ATOM_END
                    ATOM(moov_trak_mdia_minf_hdlr)
                    LIST(moov_trak_mdia_minf_gmhd)
                        ATOM_BEGIN
                        ATOM(moov_trak_mdia_minf_gmhd_gmin)
                        LIST(moov_trak_mdia_minf_gmhd_tmcd)
                            ATOM_BEGIN
                            ATOM(moov_trak_mdia_minf_gmhd_tmcd_tcmi)
                            ATOM_END
                        ATOM(moov_trak_mdia_minf_gmhd_tcmi)
                        ATOM_END
                    ATOM(moov_trak_mdia_minf_hint)
                    ATOM(moov_trak_mdia_minf_hmhd)
                    ATOM(moov_trak_mdia_minf_nmhd)
                    ATOM(moov_trak_mdia_minf_smhd)
                    LIST(moov_trak_mdia_minf_stbl)
                        ATOM_BEGIN
                        ATOM(moov_trak_mdia_minf_stbl_co64)
                        ATOM(moov_trak_mdia_minf_stbl_cslg)
                        ATOM(moov_trak_mdia_minf_stbl_ctts)
                        ATOM(moov_trak_mdia_minf_stbl_sdtp)
                        ATOM(moov_trak_mdia_minf_stbl_stco)
                        ATOM(moov_trak_mdia_minf_stbl_stdp)
                        ATOM(moov_trak_mdia_minf_stbl_stps)
                        ATOM(moov_trak_mdia_minf_stbl_stsc)
                        LIST(moov_trak_mdia_minf_stbl_stsd)
                            ATOM_BEGIN
                            ATOM(moov_trak_mdia_minf_stbl_stsd_text)
                            LIST(moov_trak_mdia_minf_stbl_stsd_tmcd)
                                ATOM_BEGIN
                                ATOM(moov_trak_mdia_minf_stbl_stsd_tmcd_name)
                                ATOM_END
                            LIST(moov_trak_mdia_minf_stbl_stsd_tx3g)
                                ATOM_BEGIN
                                ATOM(moov_trak_mdia_minf_stbl_stsd_tx3g_ftab)
                                ATOM_END
                            LIST_DEFAULT(moov_trak_mdia_minf_stbl_stsd_xxxx)
                                ATOM_BEGIN
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_alac)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_ACLR)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_APRG)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_ARES)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_avcC)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_bitr)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_btrt)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_chan)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_clap)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_colr)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_d263)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_dac3)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_damr)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_dec3)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_ddts)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_dvc1)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_esds)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_fiel)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_glbl)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_hvcC)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_idfm)
                                LIST(moov_trak_mdia_minf_stbl_stsd_xxxx_jp2h)
                                    ATOM_BEGIN
                                    ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_jp2h_colr)
                                    ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_jp2h_ihdr)
                                    ATOM_END
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_pasp)
                                LIST(moov_trak_mdia_minf_stbl_stsd_xxxx_sinf)
                                    ATOM_BEGIN
                                    ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_sinf_frma)
                                    ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_sinf_imif)
                                    ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_sinf_schi)
                                    ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_sinf_schm)
                                    ATOM_END
                                LIST(moov_trak_mdia_minf_stbl_stsd_xxxx_wave)
                                    ATOM_BEGIN
                                    ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_esds)
                                    ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_wave_acbf)
                                    ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_wave_enda)
                                    ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_wave_frma)
                                    ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_wave_samr)
                                    ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_wave_srcq)
                                    ATOM_DEFAULT(moov_trak_mdia_minf_stbl_stsd_xxxx_wave_xxxx)
                                    ATOM_END_DEFAULT
                                ATOM_END
                            ATOM_END_DEFAULT
                        ATOM(moov_trak_mdia_minf_stbl_stsh)
                        ATOM(moov_trak_mdia_minf_stbl_stss)
                        ATOM(moov_trak_mdia_minf_stbl_stsz)
                        ATOM(moov_trak_mdia_minf_stbl_stts)
                        ATOM(moov_trak_mdia_minf_stbl_stz2)
                        ATOM_END
                    ATOM(moov_trak_mdia_minf_vmhd)
                    ATOM_END
                ATOM_END
            /* Need additional tests
            LIST(moov_trak_meta)
                ATOM_BEGIN
                ATOM(moov_trak_meta_bxml)
                LIST(moov_trak_meta_keys)
                    ATOM_BEGIN
                    ATOM(moov_trak_meta_keys_mdta)
                    ATOM_END
                ATOM(moov_trak_meta_hdlr)
                LIST(moov_trak_meta_ilst)
                    ATOM_BEGIN
                    LIST_DEFAULT (moov_trak_meta_ilst_xxxx)
                        ATOM_BEGIN
                        ATOM (moov_trak_meta_ilst_xxxx_data)
                        ATOM (moov_trak_meta_ilst_xxxx_mean)
                        ATOM (moov_trak_meta_ilst_xxxx_name)
                        ATOM_END
                    ATOM_END_DEFAULT
                ATOM(moov_trak_meta_xml)
                ATOM_END
            */
            LIST(moov_trak_tapt)
                ATOM_BEGIN
                ATOM(moov_trak_tapt_clef)
                ATOM(moov_trak_tapt_prof)
                ATOM(moov_trak_tapt_enof)
                ATOM_END
            ATOM(moov_trak_tkhd)
            LIST(moov_trak_tref)
                ATOM_BEGIN
                ATOM(moov_trak_tref_chap)
                ATOM(moov_trak_tref_clcp)
                ATOM(moov_trak_tref_dpnd)
                ATOM(moov_trak_tref_fall)
                ATOM(moov_trak_tref_folw)
                ATOM(moov_trak_tref_forc)
                ATOM(moov_trak_tref_ipir)
                ATOM(moov_trak_tref_hint)
                ATOM(moov_trak_tref_mpod)
                ATOM(moov_trak_tref_scpt)
                ATOM(moov_trak_tref_ssrc)
                ATOM(moov_trak_tref_sync)
                ATOM(moov_trak_tref_tmcd)
                ATOM_END
            ATOM_END
        LIST(moov_udta)
            ATOM_BEGIN
            ATOM(moov_udta_AllF)
            ATOM(moov_udta_chpl)
            ATOM(moov_udta_clsf)
            LIST(moov_udta_DcMD)
                ATOM_BEGIN
                ATOM(moov_udta_DcMD_Cmbo)
                LIST(moov_udta_DcMD_DcME)
                    ATOM_BEGIN
                    ATOM(moov_udta_DcMD_DcME_Keyw)
                    ATOM(moov_udta_DcMD_DcME_Mtmd)
                    ATOM(moov_udta_DcMD_DcME_Rate)
                    ATOM_END
                ATOM_END
            ATOM(moov_udta_FIEL)
            ATOM(moov_udta_FXTC)
            ATOM(moov_udta_hinf)
            ATOM(moov_udta_hinv)
            LIST(moov_udta_hnti)
                ATOM_BEGIN
                ATOM(moov_udta_hnti_rtp)
                ATOM_END
            ATOM(moov_udta_ID32)
            ATOM(moov_udta_kywd)
            ATOM(moov_udta_loci)
            ATOM(moov_udta_LOOP)
            ATOM(moov_udta_MCPS)
            LIST(moov_udta_meta)
                ATOM_BEGIN
                ATOM(moov_udta_meta_hdlr)
                LIST(moov_udta_meta_ilst)
                    LIST_DEFAULT_ALONE_BEGIN (moov_udta_meta_ilst_xxxx)
                        ATOM_BEGIN
                        ATOM (moov_udta_meta_ilst_xxxx_data);
                        ATOM (moov_udta_meta_ilst_xxxx_mean);
                        ATOM (moov_udta_meta_ilst_xxxx_name);
                        ATOM_END
                    LIST_DEFAULT_ALONE_END
                ATOM_END
            ATOM(moov_udta_ndrm)
            ATOM(moov_udta_nsav)
            ATOM(moov_udta_ptv )
            ATOM(moov_udta_rtng)
            ATOM(moov_udta_Sel0)
            LIST(moov_udta_tags)
                ATOM_BEGIN
                ATOM(moov_udta_tags_meta)
                LIST(moov_udta_tags_tseg)
                    ATOM_BEGIN
                    ATOM(moov_udta_tags_tseg_tshd);
                    ATOM_END
                ATOM_END
            ATOM(moov_udta_WLOC)
            ATOM(moov_udta_XMP_)
            ATOM(moov_udta_yrrc)
            ATOM_DEFAULT (moov_udta_xxxx); //User data
            ATOM_END_DEFAULT
        ATOM_END
    ATOM(pdin)
    ATOM(PICT)
    ATOM(RDAO)
    ATOM(RDAS)
    ATOM(RDVO)
    ATOM(RDVS)
    ATOM(RED1)
    ATOM(REDA)
    ATOM(REDV)
    ATOM(REOB)
    ATOM(pckg)
    ATOM(pnot)
    LIST_SKIP(skip)
    LIST_SKIP(wide)
    DATA_END
}

//***************************************************************************
// Elements
//***************************************************************************

#define NAME_VERSION_FLAG(ELEMENT_NAME) \
    Element_Name(ELEMENT_NAME); \
    int32u Flags; \
    int8u Version; \
    { \
        Get_B1(Version,                                         "Version"); \
        Get_B3(Flags,                                           "Flags"); \
    } \

#define INTEGRITY_VERSION(_VERSION) \
    if (Version>_VERSION) \
    { \
        Skip_XX(Element_Size-Element_Offset,                    "Unknown data"); \
        return; \
    } \

#define INTEGRITY(TOVALIDATE, ERRORTEXT) \
    if (!(TOVALIDATE)) \
    { \
        Trusted_IsNot(ERRORTEXT); \
        return; \
    } \

#define INTEGRITY_SIZE1(ELEMENT_SIZE0) \
    INTEGRITY_VERSION(0); \
    if (Element_Size!=ELEMENT_SIZE0+4) \
    { \
        Trusted_IsNot("Size is wrong"); \
        return; \
    } \

#define INTEGRITY_SIZE2(ELEMENT_SIZE0, ELEMENT_SIZE1) \
    INTEGRITY_VERSION(1); \
    if (Version==0 && Element_Size!=ELEMENT_SIZE0 \
     || Version==1 && Element_Size!=ELEMENT_SIZE1) \
    { \
        Trusted_IsNot("Size is wrong"); \
        return; \
    } \

#define INTEGRITY_SIZE_ATLEAST1(ELEMENT_SIZE0) \
    INTEGRITY_VERSION(0); \
    if (Version==0 && Element_Size<ELEMENT_SIZE0) \
    { \
        Trusted_IsNot("Size is wrong"); \
        return; \
    } \

#define INTEGRITY_SIZE_ATLEAST2(ELEMENT_SIZE0, ELEMENT_SIZE1) \
    INTEGRITY_VERSION(1); \
    if (Version==0 && Element_Size<ELEMENT_SIZE0 \
     || Version==1 && Element_Size<ELEMENT_SIZE1) \
    { \
        Trusted_IsNot("Size is wrong"); \
        return; \
    } \

#define Get_B_DEPENDOFVERSION(_INFO, _NAME) \
    { \
        if (Version==0) \
        { \
            int32u Info; \
            Get_B4(Info,                                        _NAME); \
            _INFO=Info; \
        } \
        else \
            Get_B8(_INFO,                                       _NAME); \
    } \

#define Get_DATE1904_DEPENDOFVERSION(_INFO, _NAME) \
    { \
        if (Version==0) \
        { \
            int32u Temp; \
            Get_B4(Temp,                                        _NAME); \
            _INFO.Date_From_Seconds_1904(Temp); \
        } \
        else \
        { \
            int64u Temp; \
            Get_B8(Temp,                                        _NAME); \
            _INFO.Date_From_Seconds_1904(Temp); \
        } \
        Param_Info1(_INFO); \
    } \

//-------------------------------------------------------------------------
void File_Mpeg4::bloc()
{
    NAME_VERSION_FLAG("Base Location");

    //Parsing
    Skip_XX(256,                                                "baseLocation");
    Skip_XX(256,                                                "purchaseLocation");
    Skip_XX(512,                                                "Reserved");
}

//---------------------------------------------------------------------------
void File_Mpeg4::cdat()
{
    Element_Code=Element_Code==Elements::cdat?1:2;

    if (!Status[IsAccepted])
    {
        Accept("EIA-608");
        Fill(Stream_General, 0, General_Format, "Final Cut EIA-608", Unlimited, true, true);
    }
    #ifdef MEDIAINFO_EIA608_YES
        if (Streams[(int32u)Element_Code].Parsers.empty())
        {
            File_Eia608* Parser=new File_Eia608();
            Open_Buffer_Init(Parser);
            Streams[(int32u)Element_Code].Parsers.push_back(Parser);
        }
    #endif //MEDIAINFO_EIA608_YES

    Element_Name(Element_Code==1?"EIA-608-1":"EIA-608-2");

    #if MEDIAINFO_DEMUX
        Demux(Buffer+Buffer_Offset, (size_t)Element_Size, ContentType_MainStream);
        Streams[(int32u)Element_Code].Parsers[0]->FrameInfo=FrameInfo;
    #endif //MEDIAINFO_DEMUX
    while (Element_Offset+2<=Element_Size)
    {
        Open_Buffer_Continue(Streams[(int32u)Element_Code].Parsers[0], Buffer+Buffer_Offset+(size_t)Element_Offset, 2);
        Element_Offset+=2;
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::free()
{
    Element_Name("Free space");

    //Parsing
    #if MEDIAINFO_TRACE
        if (Trace_Activated)
            Param("Data", Ztring("(")+Ztring::ToZtring(Element_TotalSize_Get())+Ztring(" bytes)"));
    #endif //MEDIAINFO_TRACE
    #if MEDIAINFO_MD5
        if (MD5==NULL || (!IsSecondPass && FirstMdatPos<FirstMoovPos))
    #endif //MEDIAINFO_MD5
            GoTo(File_Offset+Buffer_Offset+Element_TotalSize_Get()); //Not using Skip_XX() because we want to skip data we don't have, and Skip_XX() does a test on size of buffer
    #if MEDIAINFO_MD5
        else
            Element_Offset=Element_TotalSize_Get();
    #endif //MEDIAINFO_MD5

    //ISM
    if (moof_traf_base_data_offset==(int64u)-1 && !data_offset_present)
        Stream->second.stco.push_back(File_Offset+Buffer_Offset);
}

//---------------------------------------------------------------------------
void File_Mpeg4::ftyp()
{
    Element_Name("File Type");

    if (Count_Get(Stream_General))
    {
        Skip_XX(Element_Size,                                   "Duplicate ftyp");
        return;
    }

    //Parsing
    std::vector<int32u> ftyps;
    Get_C4 (MajorBrand,                                         "MajorBrand");
    ftyps.push_back(MajorBrand);
    Skip_B4(                                                    "MajorBrandVersion");
    while (Element_Offset<Element_Size)
    {
        int32u CompatibleBrand;
        Get_C4 (CompatibleBrand,                                "CompatibleBrand");
        ftyps.push_back(CompatibleBrand);
    }

    FILLING_BEGIN();
        Accept("MPEG-4");

        Fill(Stream_General, 0, General_Format, "MPEG-4");
        for (size_t Pos=0; Pos<ftyps.size(); Pos++)
            switch (ftyps[Pos])
            {
                case Elements::ftyp_dash : if (Config->File_Names.size()==1)TestContinuousFileNames(1, __T("m4s")); break;
                case Elements::ftyp_caqv : Fill(StreamKind_Last, StreamPos_Last, "Encoded_Application", "Casio Digital Camera"); break;
                default : ;
            }
        CodecID_Fill(Ztring().From_CC4(MajorBrand), Stream_General, 0, InfoCodecID_Format_Mpeg4);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::idat()
{
    Element_Name("QTI");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");

    FILLING_BEGIN();
        Accept("QTI");

        Fill(Stream_General, 0, General_Format, "MPEG-4");
        CodecID_Fill(__T("QTI"), Stream_General, 0, InfoCodecID_Format_Mpeg4);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::idsc()
{
    Element_Name("QTI");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");

    FILLING_BEGIN();
        Accept("QTI");

        Fill(Stream_General, 0, General_Format, "MPEG-4");
        CodecID_Fill(__T("QTI"), Stream_General, 0, InfoCodecID_Format_Mpeg4);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::jp2c()
{
    Element_Name("JPEG 2000 content");

    #if defined(MEDIAINFO_JPEG_YES)
        //Creating the parser
        File_Jpeg MI;
        if (IsSub || Config->File_Names.size()>1) //If contained in another container or several files, this is a video stream
            MI.StreamKind=Stream_Video;
        Open_Buffer_Init(&MI);

        //Demux
        #if MEDIAINFO_DEMUX
            #if MEDIAINFO_EVENTS
                StreamIDs_Width[0]=0;
            #endif //MEDIAINFO_EVENTS
            if (Frame_Count_NotParsedIncluded==(int64u)-1)
                Frame_Count_NotParsedIncluded=0;
            if (Config->Demux_Rate_Get())
            {
                FrameInfo.DTS=float64_int64s(Frame_Count_NotParsedIncluded*1000000000/Config->Demux_Rate_Get());
                FrameInfo.PTS=FrameInfo.DTS;
                FrameInfo.DUR=float64_int64s(1000000000/Config->Demux_Rate_Get());
            }
            Demux(Buffer+Buffer_Offset, (size_t)Element_Size, ContentType_MainStream);
        #endif //MEDIAINFO_DEMUX

        //Parsing
        Open_Buffer_Continue(&MI);

        //Filling
        if (Frame_Count==0)
        {
            Accept("MPEG-4");

            Fill(Stream_General, 0, General_Format, "JPEG 2000", Unlimited, true, true);
            Fill(Stream_General, 0, General_Format_Profile, "MPEG-4");

            Finish(&MI);
            Merge(MI, MI.StreamKind, 0, 0);

            Fill("MPEG-4");
            if (Config->File_Names.size()>1)
            {
                int64u OverHead=Config->File_Sizes[0]-Element_Size;
                Fill(Stream_Video, 0, Video_StreamSize, File_Size-Config->File_Names.size()*OverHead, 10, true);
            }
            if (Config->ParseSpeed<1.0)
                Finish("MPEG-4");
        }
        Frame_Count++;
        if (Frame_Count_NotParsedIncluded!=(int64u)-1)
            Frame_Count_NotParsedIncluded++;
    #endif

}

//---------------------------------------------------------------------------
void File_Mpeg4::jp2h()
{
    Element_Name("JPEG-2000 header");
}

//---------------------------------------------------------------------------
void File_Mpeg4::jp2h_colr()
{
    Element_Name("Color");

    //Parsing
    int8u METH;
    Get_B1 (METH,                                               "METH - Specification method"); Param_Info1(Mpeg4_jp2h_METH(METH));
    Skip_B1(                                                    "PREC - Precedence");
    Skip_B1(                                                    "APPROX - Colourspace approximation");
    switch (METH)
    {
        case 0x01 : {
                    int32u EnumCS;
                    Get_B4 (EnumCS,                             "EnumCS - Enumerated colourspace"); Param_Info1(Mpeg4_jp2h_EnumCS(EnumCS));
                    Fill(StreamKind_Last, 0, "ColorSpace", Mpeg4_jp2h_EnumCS(EnumCS));
                    }
                    break;
        case 0x02 : Skip_XX(Element_Size-Element_Offset,        "PROFILE");
                    break;
        default   : Skip_XX(Element_Size-Element_Offset,        "Unknown");
                    return;
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::jp2h_ihdr()
{
    Element_Name("Header");

    //Parsing
    Skip_B4(                                                    "Height");
    Skip_B4(                                                    "Width");
    Skip_B2(                                                    "NC - Number of components");
    BS_Begin();
    Skip_SB(                                                    "BPC - Bits per component (Sign)");
    Skip_S1(7,                                                  "BPC - Bits per component (Value)");
    BS_End();
    Skip_B1(                                                    "C - Compression type");
    Skip_B1(                                                    "UnkC - Colourspace Unknown");
    Skip_B1(                                                    "IPR - Intellectual Property");
}

//---------------------------------------------------------------------------
void File_Mpeg4::mdat()
{
    #if MEDIAINFO_TRACE
        Trace_Layers_Update(8); //Streams
    #endif //MEDIAINFO_TRACE

    if (!Status[IsAccepted])
    {
        Data_Accept("MPEG-4");

        Fill(Stream_General, 0, General_Format, "QuickTime");
    }
    Element_Name("Data");

    //Sizes
    if (Retrieve(Stream_General, 0, General_HeaderSize).empty())
    {
        Fill(Stream_General, 0, General_HeaderSize, File_Offset+Buffer_Offset-Header_Size);
        Fill(Stream_General, 0, General_DataSize, Element_TotalSize_Get()+Header_Size);
        if (File_Size!=(int64u)-1 && File_Offset+Buffer_Offset+Element_TotalSize_Get()<=File_Size)
            Fill(Stream_General, 0, General_FooterSize, File_Size-(File_Offset+Buffer_Offset+Element_TotalSize_Get()));
        Fill(Stream_General, 0, General_IsStreamable, FirstMoovPos==(int64u)-1?"No":"Yes");
    }

    //Trace
    #if MEDIAINFO_TRACE
        Trace_Layers_Update(0); //Container1
    #endif //MEDIAINFO_TRACE

    if (IsSecondPass && !mdat_Pos.empty() && mdat_Pos.begin()->first<File_Offset+Buffer_Offset+Element_TotalSize_Get())
    {
        //Next piece of data
        mdat_Pos_Temp=mdat_Pos.begin();
        IsParsing_mdat=true;
        mdat_StreamJump();

        return; //Only if have something in this mdat
    }

    //In case of mdat is before moov
    if (FirstMdatPos==(int64u)-1)
    {
        Buffer_Offset-=(size_t)Header_Size;
        Element_Level--;
        BookMark_Set(); //Remembering this place, for stream parsing in phase 2
        Element_Level++;
        Buffer_Offset+=(size_t)Header_Size;

        FirstMdatPos=File_Offset+Buffer_Offset-Header_Size;
    }
    if (File_Offset+Buffer_Offset>LastMdatPos)
        LastMdatPos=File_Offset+Buffer_Offset+Element_TotalSize_Get();

    //Parsing
    #if MEDIAINFO_TRACE
        if (Trace_Activated)
            Param("Data", Ztring("(")+Ztring::ToZtring(Element_TotalSize_Get())+Ztring(" bytes)"));
    #endif //MEDIAINFO_TRACE
    #if MEDIAINFO_MD5
        if (MD5==NULL || (!IsSecondPass && FirstMdatPos<FirstMoovPos))
    #endif //MEDIAINFO_MD5
            GoTo(File_Offset+Buffer_Offset+Element_TotalSize_Get()); //Not using Skip_XX() because we want to skip data we don't have, and Skip_XX() does a test on size of buffer
    #if MEDIAINFO_MD5
        else
            Element_Offset=Element_TotalSize_Get();
    #endif //MEDIAINFO_MD5

    //ISM
    if (moof_traf_base_data_offset==(int64u)-1 && !data_offset_present)
        Stream->second.stco.push_back(File_Offset+Buffer_Offset);
}

//---------------------------------------------------------------------------
void File_Mpeg4::mdat_xxxx()
{
    if (!Element_IsComplete_Get())
    {
        Element_WaitForMoreData();
        return;
    }

    if (!IsSub)
    {
        if (Config->ParseSpeed>=1.0)
            Config->State_Set(((float)Buffer_TotalBytes)/File_Size);
    }

    std::map<int32u, stream>::iterator Stream=Streams.find((int32u)Element_Code);
    if (Stream==Streams.end())
    {
        Skip_XX(Element_Size,                                   "Unknown");
        mdat_StreamJump();
        return;
    }
    #if MEDIAINFO_DEMUX
        if (Streams[(int32u)Element_Code].StreamKind!=Stream_Other && Streams[(int32u)Element_Code].StreamKind!=Stream_Max)
        {
            //DTS
            Frame_Count_NotParsedIncluded=Stream->second.stts_FramePos;
            if (Stream->second.stts_Durations_Pos<Stream->second.stts_Durations.size())
            {
                stream::stts_durations::iterator stts_Duration=Stream->second.stts_Durations.begin()+Stream->second.stts_Durations_Pos;
                FrameInfo.DTS=TimeCode_DtsOffset+(stts_Duration->DTS_Begin+(((int64u)stts_Duration->SampleDuration)*(Frame_Count_NotParsedIncluded-stts_Duration->Pos_Begin)))*1000000000/Stream->second.mdhd_TimeScale;
                FrameInfo.PTS=Streams[(int32u)Element_Code].PtsDtsAreSame?FrameInfo.DTS:(int64u)-1;
                FrameInfo.DUR=((int64u)stts_Duration->SampleDuration)*1000000000/Stream->second.mdhd_TimeScale;
                Streams[(int32u)Element_Code].stts_FramePos++;
                if (Stream->second.stts_FramePos>=stts_Duration->Pos_End)
                    Stream->second.stts_Durations_Pos++;
                if (!Stream->second.stss.empty())
                {
                    Demux_random_access=false;
                    for (size_t Pos=0; Pos<Stream->second.stss.size(); Pos++)
                        if (Stream->second.stss[Pos]==Frame_Count_NotParsedIncluded)
                        {
                            Demux_random_access=true;
                            break;
                        }
                }
                else
                    Demux_random_access=true;
            }
            else
            {
                FrameInfo.DTS=(int64u)-1;
                FrameInfo.DUR=(int64u)-1;
                Streams[(int32u)Element_Code].stts_FramePos++;
            }

            Demux_Level=Streams[(int32u)Element_Code].Demux_Level;
            Demux(Buffer+Buffer_Offset+Streams[(int32u)Element_Code].Demux_Offset, (size_t)(Element_Size-Streams[(int32u)Element_Code].Demux_Offset), ContentType_MainStream);
        }
    #endif //MEDIAINFO_DEMUX

    for (size_t Pos=0; Pos<Stream->second.Parsers.size(); Pos++)
    {
        #if MEDIAINFO_DEMUX
        Stream->second.Parsers[Pos]->FrameInfo=FrameInfo;
        #endif //MEDIAINFO_DEMUX

        Open_Buffer_Continue(Stream->second.Parsers[Pos], Buffer+Buffer_Offset, (size_t)Element_Size);
        #if MEDIAINFO_DEMUX
            if (Config->Demux_EventWasSent && Config->Demux_Unpacketize_Get())
                Stream->second.Demux_EventWasSent=true;
        #endif //MEDIAINFO_DEMUX
        Element_Show();

        #if MEDIAINFO_DEMUX
            if (!Stream->second.IsFilled && Stream->second.Parsers[Pos]->Status[IsFilled])
            {
                if (Stream->second.StreamKind==Stream_Other) //If this is a TimeCode track
                {
                    if (((File_Mpeg4_TimeCode*)Stream->second.Parsers[Pos])->Pos!=(int32u)-1)
                    {
                        for (std::map<int32u, stream>::iterator StreamTemp=Streams.begin(); StreamTemp!=Streams.end(); ++StreamTemp)
                            if (StreamTemp->second.TimeCode_TrackID==Stream->first)
                            {
                                TimeCode_FrameOffset=((File_Mpeg4_TimeCode*)Stream->second.Parsers[Pos])->Pos;
                                TimeCode_DtsOffset=float64_int64s(((float64)TimeCode_FrameOffset)*1000000000*Stream->second.TimeCode->FrameDuration/Stream->second.TimeCode->TimeScale);
                            }
                    }
                }

                Stream->second.IsFilled=true;

                if (Config->ParseSpeed<1)
                {
                    bool File_Offset_Next_IsValid;
                    int64u File_Offset_Next;
                    if (mdat_Pos_Temp!=mdat_Pos.end())
                    {
                        File_Offset_Next=mdat_Pos_Temp->first;
                        File_Offset_Next_IsValid=true;
                    }
                    else
                    {
                        File_Offset_Next=(int64u)-1;
                        File_Offset_Next_IsValid=false;
                    }
                    mdat_pos mdat_Pos_New;
                    for (mdat_pos::iterator mdat_Pos_Item=mdat_Pos.begin(); mdat_Pos_Item!=mdat_Pos.end(); ++mdat_Pos_Item)
                        if (mdat_Pos_Item->second.StreamID!=Stream->first)
                            mdat_Pos_New[mdat_Pos_Item->first]=mdat_Pos_Item->second;
                    mdat_Pos=mdat_Pos_New;
                    if (File_Offset_Next_IsValid)
                        for (mdat_Pos_Temp=mdat_Pos.begin(); mdat_Pos_Temp!=mdat_Pos.end(); ++mdat_Pos_Temp)
                            if (mdat_Pos_Temp->first>=File_Offset_Next)
                                break;
                }
            }
        #endif //MEDIAINFO_DEMUX

        //Multiple parsers
        if (Stream->second.Parsers.size()>1)
        {
            if (!Stream->second.Parsers[Pos]->Status[IsAccepted] && Stream->second.Parsers[Pos]->Status[IsFinished])
            {
                delete *(Stream->second.Parsers.begin()+Pos);
                Stream->second.Parsers.erase(Stream->second.Parsers.begin()+Pos);
                Pos--;
            }
            else if (Stream->second.Parsers.size()>1 && Stream->second.Parsers[Pos]->Status[IsAccepted])
            {
                File__Analyze* Parser=Stream->second.Parsers[Pos];
                for (size_t Pos2=0; Pos2<Stream->second.Parsers.size(); Pos2++)
                {
                    if (Pos2!=Pos)
                        delete *(Stream->second.Parsers.begin()+Pos2);
                }
                Stream->second.Parsers.clear();
                Stream->second.Parsers.push_back(Parser);
            }
        }
    }

    //Next piece of data
    Element_Offset=Element_Size;
    Element_Show();
    mdat_StreamJump();
}

//---------------------------------------------------------------------------
void File_Mpeg4::mdat_StreamJump()
{
    //Finding right file offset
    int64u ToJump=File_Size;
    if (mdat_Pos_Temp!=mdat_Pos.end())
    {
        ToJump=mdat_Pos_Temp->first;
        #if MEDIAINFO_DEMUX
            if (Config->ParseSpeed==1)
            {
                std::map<int64u, int64u>::iterator StreamOffset_Jump_Temp=StreamOffset_Jump.find(ToJump);
                if (StreamOffset_Jump_Temp!=StreamOffset_Jump.end())
                {
                    ToJump=StreamOffset_Jump_Temp->second;
                    mdat_Pos_Temp=mdat_Pos.find(ToJump);
                }
            }
        #endif // MEDIAINFO_DEMUX
    }
    if (ToJump>File_Size)
        ToJump=File_Size;
    if (ToJump!=File_Offset+Buffer_Offset+Element_Size)
    {
        if (!Status[IsAccepted])
            Data_Accept("MPEG-4");
        #if MEDIAINFO_MD5
            if (Config->File_Md5_Get() && ((IsSecondPass && mdat_Pos_NormalParsing) || FirstMoovPos<FirstMdatPos))
                Md5_ParseUpTo=ToJump;
            else
        #endif //MEDIAINFO_MD5
                Data_GoTo(ToJump, "MPEG-4"); //Not just after
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::mfra()
{
    Element_Name("Movie Fragment Random Access");
}

//---------------------------------------------------------------------------
void File_Mpeg4::mfra_mfro()
{
    NAME_VERSION_FLAG("Movie Fragment Random Access Offset");

    //Parsing
    Skip_B4(                                                    "size");
}

//---------------------------------------------------------------------------
void File_Mpeg4::mfra_tfra()
{
    NAME_VERSION_FLAG("Track Fragment Random Access");

    //Parsing
    int32u number_of_entry;
    int8u length_size_of_traf_num, length_size_of_trun_num, length_size_of_sample_num;
    Skip_B4(                                                    "track_ID");
    BS_Begin();
    Skip_S1(26,                                                 "reserved");
    Get_S1 (2, length_size_of_traf_num,                         "length_size_of_traf_num");
    Get_S1 (2, length_size_of_trun_num,                         "length_size_of_trun_num");
    Get_S1 (2, length_size_of_sample_num,                       "length_size_of_sample_num");
    BS_End();
    Get_B4(number_of_entry,                                     "number_of_entry");
    for(int32u Pos=0; Pos<number_of_entry; Pos++)
    {
        Element_Begin1("entry");
        int64u time, moof_offset;
        Get_B_DEPENDOFVERSION(time,                             "time");
        Get_B_DEPENDOFVERSION(moof_offset,                      "moof_offset");
        switch (length_size_of_traf_num)
        {
            case 0 : Skip_B1(                                   "traf_number"); break;
            case 1 : Skip_B2(                                   "traf_number"); break;
            case 2 : Skip_B3(                                   "traf_number"); break;
            case 3 : Skip_B4(                                   "traf_number"); break;
            default: ;
        }
        switch (length_size_of_trun_num)
        {
            case 0 : Skip_B1(                                   "trun_num"); break;
            case 1 : Skip_B2(                                   "trun_num"); break;
            case 2 : Skip_B3(                                   "trun_num"); break;
            case 3 : Skip_B4(                                   "trun_num"); break;
            default: ;
        }
        switch (length_size_of_sample_num)
        {
            case 0 : Skip_B1(                                   "sample_num"); break;
            case 1 : Skip_B2(                                   "sample_num"); break;
            case 2 : Skip_B3(                                   "sample_num"); break;
            case 3 : Skip_B4(                                   "sample_num"); break;
            default: ;
        }
        Element_End0();
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::moof()
{
    Element_Name("Movie Fragment");

    if (IsSecondPass)
    {
        #if MEDIAINFO_TRACE
            if (Trace_Activated)
                Param("Data", Ztring("(")+Ztring::ToZtring(Element_TotalSize_Get())+Ztring(" bytes)"));
        #endif //MEDIAINFO_TRACE
        #if MEDIAINFO_MD5
            if (MD5==NULL || (!IsSecondPass && FirstMdatPos<FirstMoovPos))
        #endif //MEDIAINFO_MD5
                GoTo(File_Offset+Buffer_Offset+Element_TotalSize_Get()); //Not using Skip_XX() because we want to skip data we don't have, and Skip_XX() does a test on size of buffer
        #if MEDIAINFO_MD5
            else
                Element_Offset=Element_TotalSize_Get();
        #endif //MEDIAINFO_MD5
        return;
    }

    IsFragmented=true;
    moof_base_data_offset=File_Offset+Buffer_Offset-Header_Size;
    data_offset_present=true;
}

//---------------------------------------------------------------------------
void File_Mpeg4::moof_mfhd()
{
    NAME_VERSION_FLAG("Movie Fragment Header");

    //Parsing
    Skip_B4(                                                    "sequence_number");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moof_traf()
{
    Element_Name("Track Fragment");

    //Filling
    moof_traf_base_data_offset=(int64u)-1;
    if (Streams.empty())
        Streams[(int32u)-1]; //Creating a virtual track in case there is no track
    Stream=Streams.begin(); //Using first track in case there is no track header
    moof_traf_default_sample_duration=Stream->second.mvex_trex_default_sample_duration;
    moof_traf_default_sample_size=Stream->second.mvex_trex_default_sample_size;
}

//---------------------------------------------------------------------------
void File_Mpeg4::moof_traf_sdtp()
{
    NAME_VERSION_FLAG("Independent and Disposable Samples");

    //Parsing
    //for (int32u Pos=0; Pos<sample_count; Pos++) //sample_count should be taken from stsz or stz2
    while (Element_Offset<Element_Size)
    {
        Element_Begin1("sample");
        BS_Begin();
        Skip_S1(2,                                              "reserved");
        Info_S1(2, sample_depends_on,                           "sample_depends_on"); Param_Info1(Mpeg4_sample_depends_on[sample_depends_on]);
        Info_S1(2, sample_is_depended_on,                       "sample_is_depended_on"); Param_Info1(Mpeg4_sample_is_depended_on[sample_is_depended_on]);
        Info_S1(2, sample_has_redundancy,                       "sample_has_redundancy"); Param_Info1(Mpeg4_sample_has_redundancy[sample_has_redundancy]);
        BS_End();
        Element_End0();
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::moof_traf_tfhd()
{
    NAME_VERSION_FLAG("Track Fragment Header");

    //Parsing
    bool base_data_offset_present, sample_description_index_present, default_sample_duration_present, default_sample_size_present, default_sample_flags_present;
        Get_Flags (Flags,  0, base_data_offset_present,         "base-data-offset-present");
        Get_Flags (Flags,  1, sample_description_index_present, "sample-description-index-present");
        Get_Flags (Flags,  3, default_sample_duration_present,  "default-sample-duration-present");
        Get_Flags (Flags,  4, default_sample_size_present,      "default-sample-size-present");
        Get_Flags (Flags,  5, default_sample_flags_present,     "default-sample-flags-present");
        Skip_Flags(Flags, 16,                                   "duration-is-empty");
    Get_B4 (moov_trak_tkhd_TrackID,                             "track_ID");
    if (base_data_offset_present)
        Get_B8 (moof_traf_base_data_offset,                     "base_data_offset");
    if (sample_description_index_present)
        Skip_B4(                                                "sample_description_index");
    if (default_sample_duration_present)
        Get_B4 (moof_traf_default_sample_duration,              "default_sample_duration");
    if (default_sample_size_present)
        Get_B4 (moof_traf_default_sample_size,                  "default_sample_size");
    if (default_sample_flags_present)
        Skip_B4(                                                "default_sample_flags");

    FILLING_BEGIN();
        Stream=Streams.find(moov_trak_tkhd_TrackID);
        if (Stream==Streams.end())
            Stream=Streams.begin();
        if (!default_sample_duration_present)
            moof_traf_default_sample_duration=Stream->second.mvex_trex_default_sample_duration;
        if (!default_sample_size_present)
            moof_traf_default_sample_size=Stream->second.mvex_trex_default_sample_size;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moof_traf_trun()
{
    NAME_VERSION_FLAG("Track Fragment Run");

    //Parsing
    int32u sample_count;
    bool first_sample_flags_present, sample_duration_present, sample_size_present, sample_flags_present, sample_composition_time_offset_present;
        Get_Flags (Flags,  0, data_offset_present,              "data-offset-present");
        Get_Flags (Flags,  2, first_sample_flags_present,       "first-sample-flags-present");
        Get_Flags (Flags,  8, sample_duration_present,          "sample-duration-present");
        Get_Flags (Flags,  9, sample_size_present,              "sample-size-present");
        Get_Flags (Flags, 10, sample_flags_present,             "sample-flags-present");
        Get_Flags (Flags, 11, sample_composition_time_offset_present, "sample-composition-time-offsets-present");
    Get_B4 (sample_count,                                       "sample_count");
    int64u data_offset_Final=moof_traf_base_data_offset!=(int64u)-1?moof_traf_base_data_offset:moof_base_data_offset;
    if (data_offset_present)
    {
        int32u data_offset;
        Get_B4 (data_offset,                                    "data_offset");
        data_offset_Final+=data_offset;
    }

    //Filling
    if (moof_traf_base_data_offset!=(int64u)-1 || data_offset_present)
        Stream->second.stco.push_back(data_offset_Final);
    stream::stsc_struct Stsc;
    if (Stream->second.stsc.empty())
        Stsc.FirstChunk=1;
    else
        Stsc.FirstChunk=Stream->second.stsc[Stream->second.stsc.size()-1].FirstChunk+1;
    Stsc.SamplesPerChunk=sample_count;
    Stream->second.stsc.push_back(Stsc);
    if (!sample_duration_present)
        moov_trak_mdia_minf_stbl_stts_Common(sample_count, moof_traf_default_sample_duration);
    if (!sample_size_present)
        Stream->second.stsz.resize(Stream->second.stsz.size()+sample_count, moof_traf_default_sample_size);

    if (first_sample_flags_present)
        Skip_B4(                                                "first_sample_flags");
    for (int32u Pos=0; Pos<sample_count; Pos++)
    {
        Element_Begin1("sample");
        if (sample_duration_present)
        {
            int32u sample_duration;
            Get_B4 (sample_duration,                            "sample_duration");

            moov_trak_mdia_minf_stbl_stts_Common(1, sample_duration);
        }
        if (sample_size_present)
        {
            int32u sample_size;
            Get_B4 (sample_size,                                "sample_size");

            //Filling
            Stream->second.stsz_StreamSize+=sample_size;
            Stream->second.stsz_Total.push_back(sample_size);
            if (Stream->second.stsz.size()<FrameCount_MaxPerStream)
                Stream->second.stsz.push_back(sample_size);
        }
        if (sample_flags_present)
            Skip_B4(                                            "sample_flags");
        if (sample_composition_time_offset_present)
            Skip_B4(                                            "sample_composition_time_offset");
        Element_End0();
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov()
{
    if (!Status[IsAccepted])
    {
        Data_Accept("MPEG-4");

        Fill(Stream_General, 0, General_Format, "QuickTime"); //If there is no ftyp atom, this is an old Quictime file
    }
    Element_Name("File header");

    if (IsSecondPass || FirstMoovPos!=(int64u)-1) //Currently, the 1 moov atom is used
    {
        #if MEDIAINFO_TRACE
            if (Trace_Activated)
                Param("Data", Ztring("(")+Ztring::ToZtring(Element_TotalSize_Get())+Ztring(" bytes)"));
        #endif //MEDIAINFO_TRACE
        #if MEDIAINFO_MD5
            if (MD5==NULL || (!IsSecondPass && FirstMdatPos<FirstMoovPos))
        #endif //MEDIAINFO_MD5
                GoTo(File_Offset+Buffer_Offset+Element_TotalSize_Get()); //Not using Skip_XX() because we want to skip data we don't have, and Skip_XX() does a test on size of buffer
        #if MEDIAINFO_MD5
            else
                Element_Offset=Element_TotalSize_Get();
        #endif //MEDIAINFO_MD5
        return;
    }

    if (FirstMoovPos==(int64u)-1)
        FirstMoovPos=File_Offset+Buffer_Offset-Header_Size;
    /*
    else
    {
        //In case of more than 1 moov atom, the last one is used (previous ones are trashed)
        Streams.clear();
        for (size_t StreamKind=Stream_General+1; StreamKind<Stream_Max; StreamKind++)
            while (Count_Get((stream_t)StreamKind))
                Stream_Erase((stream_t)StreamKind, Count_Get((stream_t)StreamKind)-1);
    }
    */
}

//-------------------------------------------------------------------------
void File_Mpeg4::moov_ainf()
{
    NAME_VERSION_FLAG("Asset Information");

    //Parsing
    Skip_C4(                                                    "profile_version");
    Skip_String(Element_Size-Element_Offset,                    "APID");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_cmov()
{
    Element_Name("Compressed file header");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_cmov_cmvd()
{
    Element_Name("Data");

    switch (moov_cmov_dcom_Compressor)
    {
        case Elements::moov_cmov_dcom_zlib : moov_cmov_cmvd_zlib(); break;
        default: Skip_XX(Element_Size,                          "Data");
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_cmov_cmvd_zlib()
{
    Element_Name("Zlib");

    //Parsing
    int32u Dest_Size32;
    Get_B4(Dest_Size32,                                         "Destination size");

    FILLING_BEGIN();
        //Sizes
        unsigned long Source_Size=(unsigned long)(Element_Size-Element_Offset);
        unsigned long Dest_Size=Dest_Size32;

        //Uncompressing
        int8u* Dest=new int8u[Dest_Size];
        if (uncompress((Bytef*)Dest, &Dest_Size, (const Bytef*)Buffer+Buffer_Offset+4, Source_Size)<0)
        {
            Skip_XX(Element_Size,                               "Problem during the decompression");
            delete[] Dest; //Dest=NULL;
            return;
        }

        //Exiting this element
        Skip_XX(Element_Size-Element_Offset,                    "Will be parsed");

        //Configuring level
        std::vector<int64u> Element_Sizes_Sav;
        size_t Element_Level_Sav=Element_Level;
        while(Element_Level)
        {
            Element_Sizes_Sav.push_back(Element_TotalSize_Get());
            Element_End0();
        }

        //Configuring buffer
        const int8u* Buffer_Sav=Buffer;
        size_t Buffer_Size_Sav=Buffer_Size;
        int8u* Buffer_Temp_Sav=Buffer_Temp;
        size_t Buffer_Temp_Size_Sav=Buffer_Temp_Size;
        size_t Buffer_Offset_Sav=Buffer_Offset;
        size_t Buffer_Offset_Temp_Sav=Buffer_Offset_Temp;
        Buffer=NULL;
        Buffer_Size=0;
        Buffer_Temp=NULL;
        Buffer_Temp_Size=0;
        Buffer_Offset=0;
        Buffer_Offset_Temp=0;

        //Configuring file size
        int64u File_Offset_Sav=File_Offset;
        int64u File_Size_Sav=File_Size;
        if (File_Size<File_Offset+Buffer_Offset+Element_Offset+Dest_Size)
            File_Size=File_Offset+Buffer_Offset+Element_Offset+Dest_Size;
        Element_Level++;
        Header_Fill_Size(File_Size);
        Element_Level--;

        //Configuring some status info
        FirstMoovPos=(int64u)-1;

        //Parsing
        Buffer=Dest;
        Buffer_Size=Dest_Size;
        while (Open_Buffer_Continue_Loop());
        delete[] Dest; //Dest=NULL;

        //Resetting file info
        File_Offset=File_Offset_Sav;
        File_Size=File_Size_Sav;

        //Resetting buffer
        Buffer=Buffer_Sav;
        Buffer_Size=Buffer_Size_Sav;
        Buffer_Temp=Buffer_Temp_Sav;
        Buffer_Temp_Size=Buffer_Temp_Size_Sav;
        Buffer_Offset=Buffer_Offset_Sav;
        Buffer_Offset_Temp=Buffer_Offset_Temp_Sav;

        //Configuring level
        while(Element_Level)
            Element_End0();
        Element_Level++;
        Header_Fill_Size(File_Size-(File_Offset+Buffer_Offset));
        Element_Level--;
        while(Element_Level<Element_Level_Sav)
        {
            Element_Begin0();
            Element_Begin0();
            Header_Fill_Size(Element_Sizes_Sav[0]);
            Element_End0();
        }

        //Filling
        Fill(Stream_General, 0, General_Format_Settings, "Compressed header");
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_ctab()
{
    Element_Name("Color Table");

    //Parsing
    int16u Size;
    Skip_B4(                                                    "Color table seed");
    Skip_B2(                                                    "Color table flags");
    Get_B2 (Size,                                               "Color table size");
    for (int16u Pos=0; Pos<=Size; Pos++)
    {
        Skip_B2(                                                "Zero");
        Skip_B2(                                                "Red");
        Skip_B2(                                                "Green");
        Skip_B2(                                                "Blue");
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_cmov_dcom()
{
    Element_Name("Data compressor name");

    //Parsing
    Get_C4 (moov_cmov_dcom_Compressor,                          "Value");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_iods()
{
    NAME_VERSION_FLAG("Initial object descriptor");
    INTEGRITY_VERSION(0);

    FILLING_BEGIN();
        Descriptors();
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_meta()
{
    Element_Name("Metadata");

    //Filling
    moov_meta_hdlr_Type=0;
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_meta_bxml()
{
    NAME_VERSION_FLAG("Binary XML"); //ISO/IEC 14496-12
    INTEGRITY(moov_meta_hdlr_Type==Elements::moov_meta_hdlr_mp7b, "Bad meta type");

    //Parsing
    Skip_XX(Element_Size-Element_Offset,                        "Binary XML dump");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_meta_hdlr()
{
    NAME_VERSION_FLAG("Metadata Header"); //ISO/IEC 14496-12 handler

    //Parsing
    Skip_C4(                                                    "Type (Quicktime)"); //Filled if Quicktime
    Get_C4 (moov_meta_hdlr_Type,                                "Metadata type");
    if (Element_Offset+12<=Element_Size)
    {
        Skip_C4(                                                "Manufacturer");
        Skip_B4(                                                "Component reserved flags"); //Filled if Quicktime
        Skip_B4(                                                "Component reserved flags mask"); //Filled if Quicktime
        if (Element_Offset<Element_Size)
            Skip_Local(Element_Size-Element_Offset,             "Component type name");
    }
    else if (Element_Offset<Element_Size)
        Skip_XX(Element_Size-Element_Offset,                    "Unknown");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_meta_keys()
{
    NAME_VERSION_FLAG("Keys");
    INTEGRITY(moov_meta_hdlr_Type==Elements::moov_meta_hdlr_mdta, "Bad meta type");

    //Parsing
    Skip_B4(                                                    "Count");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_meta_keys_mdta()
{
    Element_Name("Name");
    INTEGRITY(moov_meta_hdlr_Type==Elements::moov_meta_hdlr_mdta, "Bad meta type");

    //Parsing
    std::string Value;
    Get_String(Element_Size, Value,                             "Value");

    //Filling
    moov_udta_meta_keys_List.push_back(Value);
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_meta_ilst()
{
    Element_Name("List");

    //Filling
    moov_udta_meta_keys_ilst_Pos=0;
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_meta_ilst_xxxx()
{
    Element_Name("Element");

    //Filling
    moov_meta_ilst_xxxx_name_Name.clear(); //useful if metadata type = "mdir"
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_meta_ilst_xxxx_data()
{
    Element_Name("Data");

    //Parsing
    int32u Kind, Language;
    Ztring Value;
    Get_B4(Kind,                                                  "Kind"); Param_Info1(Mpeg4_Meta_Kind(Kind));

    //Error detection
    switch (Element_Code_Get(Element_Level-1))
    {
        case Elements::moov_meta__disk :
        case Elements::moov_meta__trkn :
                                         if (Kind)
                                         {
                                             //Not normal
                                             Kind=0x00;
                                         }
        default                        : ;
    }

    switch (Kind)
    {
        case 0x00 : //Binary
                    Get_B4(Language,                            "Language");
                    switch (Element_Code_Get(Element_Level-1))
                    {
                        case Elements::moov_meta__disk :
                            {
                            //Parsing
                            int16u Position, Total;
                            Skip_B2(                            "Reserved");
                            Get_B2(Position,                    "Position");
                            Get_B2(Total,                       "Total");
                            if (Element_Offset+2<=Element_Size)
                                Skip_B2(                        "Reserved"); //Sometimes there are 2 more bytes, unknown

                            //Filling
                            if (Position)
                                Fill(Stream_General, 0, General_Part_Position, Position, 10, true);
                            if (Total)
                                Fill(Stream_General, 0, General_Part_Position_Total, Total, 10, true);
                            }
                            return;
                        case Elements::moov_meta__trkn :
                            {
                            //Parsing
                            int16u Position, Total;
                            Skip_B2(                            "Reserved");
                            Get_B2(Position,                    "Position");
                            Get_B2(Total,                       "Total");
                            if (Element_Offset+2<=Element_Size)
                                Skip_B2(                        "Reserved"); //Sometimes there are 2 more bytes, unknown

                            //Filling
                            if (Position)
                                Fill(Stream_General, 0, General_Track_Position, Position, 10, true);
                            if (Total)
                                Fill(Stream_General, 0, General_Track_Position_Total, Total, 10, true);
                            }
                            return;
                        case Elements::moov_meta__covr :
                            {
                            std::string Data_Raw((const char*)(Buffer+(size_t)(Buffer_Offset+Element_Offset)), (size_t)(Element_Size-Element_Offset));
                            std::string Data_Base64(Base64::encode(Data_Raw));
                            Skip_XX(Element_Size-Element_Offset, "Data");

                            //Filling
                            Fill(Stream_General, 0, General_Cover_Data, Data_Base64);
                            Fill(Stream_General, 0, General_Cover, "Yes");
                            }
                            return;
                        case Elements::moov_meta__gnre :
                            {
                            if (Element_Size-Element_Offset==2)
                            {
                                int16u Genre;
                                Get_B2(Genre,                   "Genre");

                                //Filling
                                if (Genre)
                                    Fill(Stream_General, 0, General_Genre, Genre-1);
                            }
                            else
                                Skip_XX(Element_Size-Element_Offset,"Data");
                            }
                            return;
                        default:
                            Skip_XX(Element_Size-Element_Offset,"To decode!");
                            Value=__T("(Binary)");
                    }
                    break;
        case 0x01 : //UTF-8
                    Get_B4(Language,                            "Language");
                    switch (Element_Code_Get(Element_Level-1))
                    {
                        case Elements::moov_meta___day :
                            {
                            std::string ValueS;
                            Get_String(Element_Size-Element_Offset, ValueS, "Value");
                            Value.Date_From_String(ValueS.c_str(), ValueS.size());
                            }
                            break;
                        default:
                            Get_UTF8(Element_Size-Element_Offset, Value, "Value");
                    }
                     break;
        case 0x02 : //UTF-16
                    Value=__T("UTF-16 encoding not yet supported");
                    break;
        case 0x03 : //Mac String
                    Get_B4(Language,                            "Language"); //To confirm
                    Get_Local(Element_Size-Element_Offset, Value, "Value");
                    break;
        case 0x0D : //JPEG
                    Get_B4(Language,                            "Language");
                    switch (Element_Code_Get(Element_Level-1))
                    {
                        case Elements::moov_meta__covr :
                            {
                            std::string Data_Raw((const char*)(Buffer+(size_t)(Buffer_Offset+Element_Offset)), (size_t)(Element_Size-Element_Offset));
                            std::string Data_Base64(Base64::encode(Data_Raw));
                            Skip_XX(Element_Size-Element_Offset, "Data");

                            //Filling
                            Fill(Stream_General, 0, General_Cover_Data, Data_Base64);
                            Fill(Stream_General, 0, General_Cover, "Yes");
                            }
                            return;
                        default:
                            Value=__T("(Binary)");
                    }
                    break;
        case 0x0E : //PNG
                    Get_B4(Language,                            "Language");
                    switch (Element_Code_Get(Element_Level-1))
                    {
                        case Elements::moov_meta__covr :
                            {
                            std::string Data_Raw((const char*)(Buffer+(size_t)(Buffer_Offset+Element_Offset)), (size_t)(Element_Size-Element_Offset));
                            std::string Data_Base64(Base64::encode(Data_Raw));
                            Skip_XX(Element_Size-Element_Offset, "Data");

                            //Filling
                            Fill(Stream_General, 0, General_Cover_Data, Data_Base64);
                            Fill(Stream_General, 0, General_Cover, "Yes");
                            }
                            return;
                        default:
                            Value=__T("(Binary)");
                    }
                    break;
        case 0x15 : //Signed Integer
                    {
                    Get_B4(Language,                            "Language");
                    switch (Element_Size-Element_Offset)
                    {
                        case 1 : {int8u  ValueI; Get_B1(ValueI, "Value"); Value.From_Number((int8s) ValueI);}; break;
                        case 2 : {int16u ValueI; Get_B2(ValueI, "Value"); Value.From_Number((int16s)ValueI);}; break;
                        case 4 : {int32u ValueI; Get_B4(ValueI, "Value"); Value.From_Number((int32s)ValueI);}; break;
                        case 8 : {int64u ValueI; Get_B8(ValueI, "Value"); Value.From_Number((int64s)ValueI);}; break;
                        default  : Value=__T("Unknown kind of integer value!");
                    }
                    }
                    break;
        case 0x16 : //Float 32
                    Skip_XX(4,                                  "To decode!");
                    //Value=__T("Float32 encoding not yet supported");
                    break;
        case 0x17 : //Float 64
                    Skip_XX(8,                                  "To decode!");
                    //Value=__T("Float64 encoding not yet supported");
                    break;
        default: Value=__T("Unknown kind of value!");
   }

    switch (moov_meta_hdlr_Type)
    {
        case Elements::moov_meta_hdlr_mdir :
            {
                FILLING_BEGIN();
                    std::string Parameter;
                    if (Element_Code_Get(Element_Level-1)==Elements::moov_meta______)
                        Metadata_Get(Parameter, moov_meta_ilst_xxxx_name_Name);
                    else
                        Metadata_Get(Parameter, Element_Code_Get(Element_Level-1));
                    if (Parameter=="Encoded_Application")
                    {
                        if (Value.find(__T("Nero AAC codec"))==0)
                        {
                            ZtringList List; List.Separator_Set(0, __T(" / "));
                            List.Write(Value);
                            Element_Info1(Parameter.c_str());
                            Fill(Stream_Audio, StreamPos_Last, Audio_Encoded_Library_Name, List(0), true);
                            Fill(Stream_Audio, StreamPos_Last, Audio_Encoded_Library_Version, List(1), true);
                            Parameter.clear(); //Set as already filled
                        }
                    }
                    if (Parameter=="cdec")
                    {
                        if (Value.find(__T("ndaudio "))==0)
                        {
                            ZtringList List; List.Separator_Set(0, __T(" / "));
                            List.Write(Value);
                            Element_Info1(Parameter.c_str());
                            Fill(Stream_Audio, StreamPos_Last, Audio_Encoded_Library_Name, "Nero AAC codec", Unlimited, true, true);
                            Fill(Stream_Audio, StreamPos_Last, Audio_Encoded_Library_Name, List(0).substr(8, string::npos), true);
                            Fill(Stream_Audio, StreamPos_Last, Audio_Encoded_Library_Settings, List(1), true);
                            Parameter.clear(); //Set as already filled
                        }
                    }
                    if (Parameter=="Compilation" || Parameter=="HDVideo" || Parameter=="iTunesU" || Parameter=="Podcast")
                    {
                        if (Value==__T("1"))
                            Value=__T("Yes");
                        else
                            Value.clear(); //This is usually set to 0 even if the user did not explicitely indicated something (default)
                    }
                    if (Parameter=="BPM")
                    {
                        if (Value==__T("0"))
                            Value.clear();
                    }
                    if (Parameter=="ContentType")
                    {
                             if (Value==__T("0"))  Value=__T("Movie");
                        else if (Value==__T("1"))  Value=__T("Music");
                        else if (Value==__T("2"))  Value=__T("Audiobook");
                        else if (Value==__T("5"))  Value=__T("Whacked Bookmark");
                        else if (Value==__T("6"))  Value=__T("Music Video");
                        else if (Value==__T("9"))  Value=__T("Short Film");
                        else if (Value==__T("10")) Value=__T("TV Show");
                        else if (Value==__T("11")) Value=__T("Booklet");
                        else if (Value==__T("14")) Value=__T("Ringtone");
                        else if (Value==__T("21")) Value=__T("Podcast");
                        else Value=__T("Unknown Type");
                        Fill(Stream_General, 0, General_ContentType, Value);
                    }
                    if (Parameter=="AppleStoreAccountType")
                    {
                        if (Value==__T("0"))
                            Value=__T("iTunes");
                        else
                            Value=__T("AOL");
                        Fill(Stream_General, 0, General_ServiceName, Value);
                    }
                    if (Parameter=="Rating")
                    {
                             if (Value==__T("0")) Value=__T("None");
                        else if (Value==__T("2")) Value=__T("Clean");
                        else Value=__T("Explicit");
                    }
                    if (Parameter=="AppleStoreCountry")
                    {
                             if (Value==__T("143441")) Value=__T("United States");
                        else if (Value==__T("143442")) Value=__T("France");
                        else if (Value==__T("143443")) Value=__T("Germany");
                        else if (Value==__T("143444")) Value=__T("United Kingdom");
                        else if (Value==__T("143445")) Value=__T("Austria");
                        else if (Value==__T("143446")) Value=__T("Belgium");
                        else if (Value==__T("143447")) Value=__T("Finland");
                        else if (Value==__T("143448")) Value=__T("Greece");
                        else if (Value==__T("143449")) Value=__T("Ireland");
                        else if (Value==__T("143450")) Value=__T("Italy");
                        else if (Value==__T("143451")) Value=__T("Luxembourg");
                        else if (Value==__T("143452")) Value=__T("Netherlands");
                        else if (Value==__T("143453")) Value=__T("Portugal");
                        else if (Value==__T("143454")) Value=__T("Spain");
                        else if (Value==__T("143455")) Value=__T("Canada");
                        else if (Value==__T("143456")) Value=__T("Sweden");
                        else if (Value==__T("143457")) Value=__T("Norway");
                        else if (Value==__T("143458")) Value=__T("Denmark");
                        else if (Value==__T("143459")) Value=__T("Switzerland");
                        else if (Value==__T("143460")) Value=__T("Australia");
                        else if (Value==__T("143461")) Value=__T("New Zealand");
                        else if (Value==__T("143462")) Value=__T("Japan");
                        else Value=__T("Unknown Country");
                    }
                    if (!Parameter.empty())
                    {
                        Element_Info1(Parameter.c_str());
                        Fill(Stream_General, 0, Parameter.c_str(), Value, true);
                    }
                FILLING_END();
            }
            break;
        case Elements::moov_meta_hdlr_mdta :
            if(!moov_udta_meta_keys_List.empty())
            {
                FILLING_BEGIN();
                    std::string Parameter;
                    Metadata_Get(Parameter, moov_udta_meta_keys_List[moov_udta_meta_keys_ilst_Pos<moov_udta_meta_keys_List.size()?moov_udta_meta_keys_ilst_Pos:moov_udta_meta_keys_List.size()-1]);
                    if (Parameter=="com.apple.quicktime.version")
                        Vendor_Version=Value.SubString(__T(""), __T(" "));
                    else if (Parameter=="com.apple.quicktime.player.version")
                        Vendor_Version=Value.SubString(__T(""), __T(" "));
                    else if (Parameter=="com.apple.quicktime.comment")
                        Fill(Stream_General, 0, General_Comment, Value, true);
                    else if (Parameter=="com.apple.quicktime.description")
                        Fill(Stream_General, 0, General_Description, Value, true);
                    else if (Parameter=="com.apple.finalcutstudio.media.uuid")
                        Fill(Stream_General, 0, "Media/UUID", Value);
                    else if (Parameter=="com.apple.finalcutstudio.media.history.uuid")
                        Fill(Stream_General, 0, "Media/History/UUID", Value);
                    else if (!Parameter.empty())
                        Fill(Stream_General, 0, Parameter.c_str(), Value, true);
                FILLING_END();
                moov_udta_meta_keys_ilst_Pos++;
            }
            else
                Param2("Keys atom is missing!", 0);

        case Elements::moov_udta_meta :
            {
                FILLING_BEGIN();
                    std::string Parameter;
                    Metadata_Get(Parameter, (int32u)Element_Code_Get(Element_Level-1));
                    if (!Parameter.empty())
                        Fill(Stream_General, 0, Parameter.c_str(), Value, true);
                FILLING_END();
            }
        default: ;
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_meta_ilst_xxxx_mean()
{
    Element_Name("Mean");

    //Parsing
    Skip_B4(                                                    "Unknown");
    Skip_Local(Element_Size-Element_Offset,                     "Value");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_meta_ilst_xxxx_name()
{
    Element_Name("Name");

    //Parsing
    Skip_B4(                                                    "Unknown");
    Get_String(Element_Size-Element_Offset, moov_meta_ilst_xxxx_name_Name, "Value");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_meta_xml()
{
    NAME_VERSION_FLAG("XML"); //ISO/IEC 14496-12
    INTEGRITY(moov_meta_hdlr_Type==Elements::moov_meta_hdlr_mp7t, "Bad meta type");

    //Parsing
    Skip_XX(Element_Size-Element_Offset,                        "XML dump");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_mvex()
{
    Element_Name("Movie Extends");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_mvex_mehd()
{
    NAME_VERSION_FLAG("Movie Extends Header");

    //Parsing
    int64u fragment_duration;
    Get_B_DEPENDOFVERSION(fragment_duration,                    "fragment_duration");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_mvex_trex()
{
    NAME_VERSION_FLAG("Movie Extends Defaults");

    //Parsing
    int32u default_sample_duration;
    int32u default_sample_size;
    Get_B4 (moov_trak_tkhd_TrackID,                             "track_ID");
    Skip_B4(                                                    "default_sample_description_index");
    Get_B4 (default_sample_duration,                            "default_sample_duration");
    Get_B4 (default_sample_size,                                "default_sample_size");
    Element_Begin1("default_sample_flags");
        BS_Begin();
        Skip_S1(6,                                              "reserved");
        Skip_S1(2,                                              "sample_depends_on");
        Skip_S1(2,                                              "sample_is_depended_on");
        Skip_S1(2,                                              "sample_has_redundancy");
        Skip_S1(3,                                              "sample_padding_value");
        Skip_SB(                                                "sample_is_difference_sample");
        BS_End();
        Skip_B2(                                                "sample_degradation_priority");
    Element_End0();

    FILLING_BEGIN();
        Streams[moov_trak_tkhd_TrackID].mvex_trex_default_sample_duration=default_sample_duration;
        Streams[moov_trak_tkhd_TrackID].mvex_trex_default_sample_size=default_sample_size;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_mvhd()
{
    NAME_VERSION_FLAG("Movie header");

    //Parsing
    Ztring Date_Created, Date_Modified;
    int64u Duration;
    float32 a, b, u, c, d, v, x, y, w;
    int32u Rate;
    int16u Volume;
    Get_DATE1904_DEPENDOFVERSION(Date_Created,                  "Creation time");
    Get_DATE1904_DEPENDOFVERSION(Date_Modified,                 "Modification time");
    Get_B4(TimeScale,                                           "Time scale"); Param_Info1(Ztring::ToZtring(TimeScale)+__T(" Hz"));
    Get_B_DEPENDOFVERSION(Duration,                             "Duration"); Param_Info1C(TimeScale, Ztring::ToZtring((int64u)Duration*1000/TimeScale)+__T(" ms"));
    Get_B4 (Rate,                                               "Preferred rate"); Param_Info1(Ztring::ToZtring(((float32)Rate)/0x10000));
    Get_B2 (Volume,                                             "Preferred volume"); Param_Info1(Ztring::ToZtring(((float32)Volume)/0x100));
    Skip_XX(10,                                                 "Reserved");
    Element_Begin1("Matrix structure");
        Get_BFP4(16, a,                                         "a (width scale)");
        Get_BFP4(16, b,                                         "b (width rotate)");
        Get_BFP4( 2, u,                                         "u (width angle)");
        Get_BFP4(16, c,                                         "c (height rotate)");
        Get_BFP4(16, d,                                         "d (height scale)");
        Get_BFP4( 2, v,                                         "v (height angle)");
        Get_BFP4(16, x,                                         "x (position left)");
        Get_BFP4(16, y,                                         "y (position top)");
        Get_BFP4( 2, w,                                         "w (divider)");
    Element_End0();
    Skip_B4(                                                    "Preview time");
    Skip_B4(                                                    "Preview duration");
    Skip_B4(                                                    "Poster time");
    Skip_B4(                                                    "Selection time");
    Skip_B4(                                                    "Selection duration");
    Skip_B4(                                                    "Current time");
    Skip_B4(                                                    "Next track ID");

    FILLING_BEGIN();
        if (TimeScale)
        {
            //int32u Duration=(int32u)(((float)Duration)/TimeScale*1000);
            //Fill("Duration", Duration);
        }
        if (Date_Created.find(__T('\r'))!=std::string::npos)
            Date_Created.resize(Date_Created.find(__T('\r')));
        if (Date_Created.find(__T('\n'))!=std::string::npos)
            Date_Created.resize(Date_Created.find(__T('\n')));
        Fill(StreamKind_Last, StreamPos_Last, "Encoded_Date", Date_Created);
        if (Date_Modified.find(__T('\r'))!=std::string::npos)
            Date_Modified.resize(Date_Modified.find(__T('\r')));
        if (Date_Modified.find(__T('\n'))!=std::string::npos)
            Date_Modified.resize(Date_Modified.find(__T('\n')));
        Fill(StreamKind_Last, StreamPos_Last, "Tagged_Date", Date_Modified);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak()
{
    Element_Name("Track");

    FILLING_BEGIN();
        Fill_Flush();
        moov_trak_tkhd_TrackID=(int32u)-1;
        moov_trak_tkhd_Width=0;
        moov_trak_tkhd_Height=0;
        moov_trak_tkhd_DisplayAspectRatio=0;
        moov_trak_tkhd_Rotation=0;
        Stream_Prepare(Stream_Max); //clear filling
        Streams.erase((int32u)-1);
        Fill(StreamKind_Last, StreamPos_Last, General_StreamOrder, Streams.size());
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_edts()
{
    Element_Name("Edit");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_edts_elst()
{
    NAME_VERSION_FLAG("Edit List");

    //Parsing
    int32u Count;
    Get_B4 (Count,                                              "Number of entries");
    for (int32u Pos=0; Pos<Count; Pos++)
    {
        stream::edts_struct edts;
        Element_Begin1("Entry");
        Get_B4 (edts.Duration,                                  "Track duration"); Param_Info2C(TimeScale, (int64u)edts.Duration*1000/TimeScale, " ms");
        Get_B4 (edts.Delay,                                     "Media time"); Param_Info2C(TimeScale && (edts.Delay!=(int32u)-1), (int64u)edts.Delay*1000/TimeScale, " ms");
        Get_B4 (edts.Rate,                                      "Media rate"); Param_Info1(((float)edts.Rate)/0x10000);
        Element_End0();

        Streams[moov_trak_tkhd_TrackID].edts.push_back(edts);
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_load()
{
    Element_Name("Preload");

    //Parsing
    Info_B4(PreloadTime,                                        "Preload time"); Param_Info2C(TimeScale, PreloadTime*1000/TimeScale, " ms");
    Info_B4(PreloadFlags,                                       "Flags");
        Skip_Flags(PreloadFlags, 0,                             "PreloadAlways");
        Skip_Flags(PreloadFlags, 1,                             "TrackEnabledPreload");
    Info_B4(HintFlags,                                          "Hint flags");
        Skip_Flags(HintFlags,  2,                               "KeepInBuffer");
        Skip_Flags(HintFlags,  8,                               "HighQuality");
        Skip_Flags(HintFlags, 20,                               "SingleFieldPlayback");
        Skip_Flags(HintFlags, 26,                               "DeinterlaceFields");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia()
{
    Element_Name("Media");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_hdlr()
{
    NAME_VERSION_FLAG("Handler Reference");

    //Parsing
    Ztring Title;
    int32u SubType, Manufacturer;
    int8u Size;
    Skip_C4(                                                    "Component type");
    Get_C4 (SubType,                                            "Component subtype");
    Get_C4 (Manufacturer,                                       "Component manufacturer");
    Skip_B4(                                                    "Component flags");
    Skip_B4(                                                    "Component flags mask");
    if (Element_Offset<Element_Size)
    {
        Peek_B1(Size);
        if (Element_Offset+1+Size==Element_Size)
        {
            Skip_B1(                                                "Component name size");
            Get_Local(Size, Title,                                  "Component name");
        }
        else
        {
            std::string TitleS;
            Get_String(Element_Size-Element_Offset, TitleS,         "Component name");
            Title.From_UTF8(TitleS.c_str());
            if (Title.empty())
                Title.From_Local(TitleS.c_str()); //Trying Local...
        }
        if (Title.find(__T("Handler"))!=std::string::npos || Title.find(__T("vide"))!=std::string::npos || Title.find(__T("soun"))!=std::string::npos)
            Title.clear(); //This is not a Title
    }

    FILLING_BEGIN();
        if (!Title.empty()) Fill(StreamKind_Last, StreamPos_Last, "Title",    Title);
        switch (SubType)
        {
            case Elements::moov_trak_mdia_hdlr_clcp :
                if (StreamKind_Last!=Stream_Text)
                {
                    Stream_Prepare(Stream_Text);
                }
                break;
            case Elements::moov_trak_mdia_hdlr_soun :
                if (StreamKind_Last!=Stream_Audio)
                {
                    Stream_Prepare(Stream_Audio);
                }
                break;
            case Elements::moov_trak_mdia_hdlr_vide :
                if (StreamKind_Last!=Stream_Video)
                {
                    Stream_Prepare(Stream_Video);
                }
                break;
            case Elements::moov_trak_mdia_hdlr_text :
            case Elements::moov_trak_mdia_hdlr_sbtl :
                if (StreamKind_Last!=Stream_Text)
                {
                    if (Streams[moov_trak_tkhd_TrackID].IsChapter)
                        Stream_Prepare(Stream_Menu);
                    else
                    {
                        Stream_Prepare(Stream_Text);
                        if (SubType!=Elements::moov_trak_mdia_hdlr_text)
                            Fill(Stream_Text, StreamPos_Last, Text_MuxingMode, Ztring().From_CC4(SubType));
                    }
                }
                break;
            case Elements::moov_trak_mdia_hdlr_tmcd :
                if (StreamKind_Last!=Stream_Text)
                {
                    Stream_Prepare(Stream_Other);
                    Fill(Stream_Other, StreamPos_Last, Other_Type, "Time code");
                    Fill(Stream_Other, StreamPos_Last, Other_Format, "QuickTime TC");
                }
                break;
            case Elements::moov_trak_mdia_hdlr_subp :
                if (StreamKind_Last!=Stream_Text)
                {
                    Stream_Prepare(Stream_Text);
                    CodecID_Fill(__T("subp"), Stream_Text, StreamPos_Last, InfoCodecID_Format_Mpeg4);
                    Fill(StreamKind_Last, StreamPos_Last, Text_Codec, "subp");
                }
                break;
            case Elements::moov_trak_mdia_hdlr_MPEG :
                mdat_MustParse=true; //Data is in MDAT
                break;
            default: ;
        }
        if (Manufacturer!=0x00000000)
        {
            if (Vendor==0x00000000)
                Vendor=Manufacturer;
            else if (Vendor!=Manufacturer)
                Vendor=0xFFFFFFFF; //Two names, this is two much
        }

        Streams[moov_trak_tkhd_TrackID].StreamKind=StreamKind_Last;
        Streams[moov_trak_tkhd_TrackID].StreamPos=StreamPos_Last;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_imap()
{
    Element_Name("Non-primary source input map");

    //Parsing
    Skip_B4(                                                    "Unknown");
    Skip_B4(                                                    "Unknown");
    Skip_B4(                                                    "Unknown");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_imap_sean()
{
    //Parsing
    Skip_B4(                                                    "Unknown");
    Skip_B4(                                                    "Unknown");
    Skip_B4(                                                    "Unknown");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_imap_sean___in()
{
    Element_Name("Input");

    //Parsing
    Skip_B4(                                                    "Atom ID");
    Skip_B2(                                                    "Zero");
    Skip_B2(                                                    "Number of internal atoms");
    Skip_B4(                                                    "Zero");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_imap_sean___in___ty()
{
    Element_Name("Input type");

    //Parsing
    Info_B4(TypeModifierName,                                   "Type modifier name"); Param_Info1(Mpeg4_TypeModifierName(TypeModifierName));
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_imap_sean___in_dtst()
{
    //Parsing
    Skip_B4(                                                    "Unknown");
    Skip_B4(                                                    "Unknown");
    Skip_B4(                                                    "Unknown");
    Skip_C4(                                                    "Unknown");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_imap_sean___in_obid()
{
    Element_Name("Object ID");

    //Parsing
    Skip_B4(                                                    "Object ID");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_mdhd()
{
    NAME_VERSION_FLAG("Media Header");

    //Parsing
    Ztring Date_Created, Date_Modified;
    int64u Duration;
    int32u TimeScale;
    int16u Language;
    Get_DATE1904_DEPENDOFVERSION(Date_Created,                  "Creation time");
    Get_DATE1904_DEPENDOFVERSION(Date_Modified,                 "Modification time");
    Get_B4(TimeScale,                                           "Time scale");
    Get_B_DEPENDOFVERSION(Duration,                             "Duration"); Param_Info2C(TimeScale, Duration*1000/TimeScale, " ms");
    Get_B2 (Language,                                           "Language"); Param_Info1(Language_Get(Language));
    Skip_B2(                                                    "Quality");

    FILLING_BEGIN();
        Fill(StreamKind_Last, StreamPos_Last, "Language", Language_Get(Language));
        Streams[moov_trak_tkhd_TrackID].mdhd_Duration=Duration;
        Streams[moov_trak_tkhd_TrackID].mdhd_TimeScale=TimeScale;

        //Coherency tests
        if (Streams[moov_trak_tkhd_TrackID].tkhd_Duration*1.01>=Duration*0.99 && Streams[moov_trak_tkhd_TrackID].tkhd_Duration*0.99<=Duration*1.01 && TimeScale && File_Mpeg4::TimeScale!=TimeScale)
        {
            float64 Ratio=((float64)TimeScale)/((float64)File_Mpeg4::TimeScale);
            Streams[moov_trak_tkhd_TrackID].tkhd_Duration=float64_int64s(Streams[moov_trak_tkhd_TrackID].tkhd_Duration/Ratio);
            Clear(StreamKind_Last, StreamPos_Last, "Duration_Source");
            Fill(StreamKind_Last, StreamPos_Last, "Duration", float64_int64s(((float64)Streams[moov_trak_tkhd_TrackID].tkhd_Duration)*1000/((float64)File_Mpeg4::TimeScale)), 10, true);
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf()
{
    Element_Name("Media Information");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_code()
{
    Element_Name("code (found in Avid?)");

    //Parsing
    Skip_B4(                                                    "Unknown");
    Skip_B4(                                                    "Unknown");
    Skip_B4(                                                    "Unknown");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_code_sean()
{
    Element_Name("sean (found in Avid?)");

    //Parsing
    Skip_B4(                                                    "Unknown");
    Skip_B4(                                                    "Unknown");
    Skip_B4(                                                    "Unknown");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_code_sean_RU_A()
{
    Element_Name("RU*A (found in Avid?)");

    //Parsing
    Ztring Path;
    Skip_B4(                                                    "Unknown");
    Skip_B4(                                                    "Unknown");
    Skip_B4(                                                    "Unknown");
    Get_Local(Element_Size-Element_Offset, Path,                "Path?");

    FILLING_BEGIN();
        Streams[moov_trak_tkhd_TrackID].File_Name=Path;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_dinf()
{
    Element_Name("Data Information");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_dinf_url_()
{
    NAME_VERSION_FLAG("Data Location");

    //Parsing
    Skip_Local(Element_Size-Element_Offset,                     "location");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_dinf_urn_()
{
    NAME_VERSION_FLAG("Data Name");

    //Parsing
    Skip_Local(Element_Size-Element_Offset,                     "name TODO location after null string");
    //Skip_Local(Element_Size,                                    location);
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_dinf_dref()
{
    NAME_VERSION_FLAG("Data Reference");

    //Parsing
    Skip_B4(                                                    "entry_count");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_dinf_dref_alis()
{
    NAME_VERSION_FLAG("Alias"); //bit 0 = external/internal data

    //Often empty
    bool IsInternal;
        Get_Flags (Flags,    0, IsInternal,                     "IsInternal");
    if (IsInternal)
        return; //Internal stream, no alias

    //Parsing
    Ztring file_name_string, volume_name_string, Directory_Name;
    int16u record_size, record_version, alias_kind;
    int8u volume_name_string_length, file_name_string_length;
    Element_Begin1("Mac OS Alias Record");
    Skip_B4(                                                    "user type name/app creator code");
    Get_B2 (record_size,                                        "record size");
    Get_B2 (record_version,                                     "record version");
    if (record_version!=2)
    {
        Skip_XX(Element_Size-Element_Offset,                    "unknown");
        return;
    }
    int64u End=Element_Offset-8+record_size;
    if (End>Element_Size)
        End=Element_Size; //Found one file having record_size = the size of the atom, header included
    Get_B2 (alias_kind,                                         "alias kind"); Param_Info1(alias_kind?"directory":"file");
    Get_B1 (volume_name_string_length,                          "volume name string length");
    if (volume_name_string_length>27)
        volume_name_string_length=27;
    Get_Local(volume_name_string_length, volume_name_string,    "volume name string");
    if (volume_name_string_length<27)
        Skip_XX(27-volume_name_string_length,                   "volume name string padding");
    Skip_B4(                                                    "volume created mac local date"); //seconds since beginning 1904 to 2040
    Skip_B2(                                                    "file system type");
    Skip_B2(                                                    "drive type");
    Skip_B4(                                                    "parent directory ID");
    Get_B1 (file_name_string_length,                            "file name string length");
    if (file_name_string_length>99)
        file_name_string_length=99;
    Get_Local(file_name_string_length, file_name_string,        "file name string");
    if (file_name_string_length<63)
        Skip_XX(63-file_name_string_length,                     "file name string padding");
    if (file_name_string_length<=63)
        Skip_B4(                                                "file number");
    else if (file_name_string_length<67)
        Skip_XX(67-file_name_string_length,                     "file name string padding (hack)");
    if (file_name_string_length<=67)
        Skip_B4(                                                "file created mac local date");
    else if (file_name_string_length<71)
        Skip_XX(71-file_name_string_length,                     "file name string padding (hack)");
    if (file_name_string_length<=71)
        Skip_B4(                                                "file type name");
    else if (file_name_string_length<75)
        Skip_XX(75-file_name_string_length,                     "file name string padding (hack)");
    if (file_name_string_length<=75)
        Skip_B4(                                                "file creator name");
    else if (file_name_string_length<79)
        Skip_XX(79-file_name_string_length,                     "file name string padding (hack)");
    if (file_name_string_length<=79)
        Skip_B2(                                                "next level up from alias");
    else if (file_name_string_length<81)
        Skip_XX(81-file_name_string_length,                     "file name string padding (hack)");
    if (file_name_string_length<=81)
        Skip_B2(                                                "next level down to target");
    else if (file_name_string_length<83)
        Skip_XX(83-file_name_string_length,                     "file name string padding (hack)");
    if (file_name_string_length<=83)
        Skip_B4(                                                "volume attributes");
    else if (file_name_string_length<87)
        Skip_XX(87-file_name_string_length,                     "file name string padding (hack)");
    if (file_name_string_length<=87)
        Skip_B2(                                                "volume file system ID");
    else if (file_name_string_length<89)
        Skip_XX(89-file_name_string_length,                     "file name string padding (hack)");
    if (file_name_string_length<=89)
        Skip_XX(10,                                             "Reserved");
    else if (file_name_string_length<99)
        Skip_XX(99-file_name_string_length,                     "file name string padding (hack)");
    while(Element_Offset<End)
    {
        Trusted++;
        int16u type, size;
        Get_B2 (type,                                           "type");
        Get_B2 (size,                                           "size");
        switch (type)
        {
            case 0x0000 :
                        Get_Local(size, Directory_Name,         "Directory Name");
                        break;
            case 0x0002 :
                        Skip_Local(size,                        "Absolute Path");
                        break;
            case 0xFFFF :
                        Skip_XX(End-Element_Offset,             "Padding");
                        break;
            default     :
                        Skip_Local(size,                        "Unknown");
        }
        if (size%2)
            Skip_B1(                                            "Padding");
    }
    Element_End0();

    if (Element_Offset<Element_Size)
        Skip_XX(Element_Size-Element_Offset,                    "Padding");

    FILLING_BEGIN();
        if (Streams[moov_trak_tkhd_TrackID].File_Name.empty()) //Priority to "code" version
        {
            if (!Directory_Name.empty())
            {
                Streams[moov_trak_tkhd_TrackID].File_Name+=Directory_Name;
                Streams[moov_trak_tkhd_TrackID].File_Name+=ZenLib::PathSeparator;
            }
            Streams[moov_trak_tkhd_TrackID].File_Name+=file_name_string;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_dinf_dref_rsrc()
{
    Element_Name("Resource alias");

    //Parsing
    Skip_B4(                                                    "Flags"); //bit 0 = external/internal data
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_gmhd()
{
    Element_Name("Generic Media Header");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_gmhd_gmin()
{
    NAME_VERSION_FLAG("Generic Media Info");

    //Parsing
    Skip_B2(                                                    "Graphics mode");
    Skip_B2(                                                    "Opcolor (red)");
    Skip_B2(                                                    "Opcolor (green)");
    Skip_B2(                                                    "Opcolor (blue)");
    Skip_B2(                                                    "Balance");
    Skip_B2(                                                    "Reserved");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_gmhd_tmcd()
{
    Element_Name("TimeCode");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_gmhd_tcmi()
{
    moov_trak_mdia_minf_gmhd_tmcd_tcmi();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_gmhd_tmcd_tcmi()
{
    NAME_VERSION_FLAG("TimeCode Media Information");

    //Parsing
    int8u FontNameSize;
    bool IsVisual;
        Get_Flags (Flags,    0, IsVisual,                       "IsVisual");
    Skip_B2(                                                    "Text font");
    Info_B2(TextFace,                                           "Text face");
        Skip_Flags(TextFace, 0,                                 "Bold");
        Skip_Flags(TextFace, 1,                                 "Italic");
        Skip_Flags(TextFace, 2,                                 "Underline");
        Skip_Flags(TextFace, 3,                                 "Outline");
        Skip_Flags(TextFace, 4,                                 "Shadow");
        Skip_Flags(TextFace, 5,                                 "Condense");
        Skip_Flags(TextFace, 6,                                 "Extend");
    if (Element_Size>=25 && 25+Buffer[Buffer_Offset+24]==Element_Size)
        Skip_BFP4(16,                                           "Text size"); //Non-Standard, but found in several files
    else
        Skip_B2(                                                "Text size");
    Skip_B2(                                                    "Text color (red)");
    Skip_B2(                                                    "Text color (green)");
    Skip_B2(                                                    "Text color (blue)");
    Skip_B2(                                                    "Background color (red)");
    Skip_B2(                                                    "Background color (green)");
    Skip_B2(                                                    "Background color (blue)");
    Get_B1 (FontNameSize,                                       "Font name size");
    Skip_Local(FontNameSize,                                    "Font name");

    FILLING_BEGIN();
        Streams[moov_trak_tkhd_TrackID].TimeCode_IsVisual=IsVisual;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_hint()
{
    NAME_VERSION_FLAG("Hint");

    //Parsing
    Skip_B2(                                                    "Maximum packet delivery unit");
    Skip_B2(                                                    "Average packet delivery unit");
    Skip_B4(                                                    "Maximum bit rate");
    Skip_B4(                                                    "Average bit rate");
    Skip_B4(                                                    "Reserved");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_hdlr()
{
    moov_trak_mdia_hdlr();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_hmhd()
{
    NAME_VERSION_FLAG("Hint Media Header");

    //Parsing
    Skip_B2(                                                    "maxPDUsize");
    Skip_B2(                                                    "avgPDUsize");
    Skip_B4(                                                    "maxbitrate");
    Skip_B4(                                                    "avgbitrate");
    Skip_B4(                                                    "reserved");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_nmhd()
{
    NAME_VERSION_FLAG("Null Media Header");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_smhd()
{
    NAME_VERSION_FLAG("Sound Media Header");

    //Parsing
    Skip_B2(                                                    "Audio balance");
    Skip_B2(                                                    "Reserved");

    FILLING_BEGIN();
        if (StreamKind_Last!=Stream_Audio)
            Stream_Prepare(Stream_Audio);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_vmhd()
{
    NAME_VERSION_FLAG("Video Media Header")

    //Parsing
    Skip_B2(                                                    "Graphic mode");
    Skip_B2(                                                    "Graphic mode color R");
    Skip_B2(                                                    "Graphic mode color G");
    Skip_B2(                                                    "Graphic mode color B");

    FILLING_BEGIN();
        if (StreamKind_Last!=Stream_Video)
        {
            Stream_Prepare(Stream_Video);
            Streams[moov_trak_tkhd_TrackID].StreamKind=Stream_Video;
            Streams[moov_trak_tkhd_TrackID].StreamPos=StreamPos_Last;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl()
{
    Element_Name("Sample Table");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_co64()
{
    NAME_VERSION_FLAG("Chunk offset");

    int64u Offset;
    int32u Count;
    Get_B4 (Count,                                              "Number of entries");
    for (int32u Pos=0; Pos<Count; Pos++)
    {
        //Too much slow
        /*
        Get_B8 (Offset,                                     "Offset");
        */

        //Faster
        if (Element_Offset+8>Element_Size)
            break; //Problem
        Offset=BigEndian2int64u(Buffer+Buffer_Offset+(size_t)Element_Offset);
        Element_Offset+=8;

        if (Pos<FrameCount_MaxPerStream)
            Streams[moov_trak_tkhd_TrackID].stco.push_back(Offset);
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_cslg()
{
    Element_Name("Composition Shift Least Greatest");

    //Parsing
    Skip_XX(Element_Size,                                       "Unknown");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_ctts()
{
    NAME_VERSION_FLAG("Composition Time To Sample");

    //Parsing
    int32u entry_count;
    Get_B4 (entry_count,                                        "entry_count");

    //Currently no usage
    //for (int32u Pos=0; Pos<entry_count; Pos++)
    {
        //int32u sample_count, sample_offset;

        //Too much slow
        /*
        Get_B4 (sample_count,                                   "sample_count");
        Get_B4 (sample_offset,                                  "sample_offset");
        */

        //Faster
        /*
        if (Element_Offset+8>Element_Size)
            break; //Problem
        sample_count =BigEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset  );
        sample_offset=BigEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset+4);
        Element_Offset+=8;
        */
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_sdtp()
{
    Element_Name("Sample Dependency");

    //Parsing
    Skip_XX(Element_Size,                                       "Unknown");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stco()
{
    NAME_VERSION_FLAG("Chunk offset");

    int32u Count, Offset;
    Get_B4 (Count,                                              "Number of entries");
    for (int32u Pos=0; Pos<Count; Pos++)
    {
        //Too much slow
        /*
        Get_B4 (Offset,                                     "Offset");
        */

        //Faster
        if (Element_Offset+4>Element_Size)
            break; //Problem
        Offset=BigEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset);
        Element_Offset+=4;

        if (Pos<FrameCount_MaxPerStream)
            Streams[moov_trak_tkhd_TrackID].stco.push_back(Offset);
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stdp()
{
    Element_Name("Degradation Priority");

    //Parsing
    int32u sample_count;
    Get_B4 (sample_count,                                       "sample-count");

    for (int32u Pos=0; Pos<sample_count; Pos++)
    {
        Skip_B2(                                                "priority");
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stps()
{
    NAME_VERSION_FLAG("Partial Sync Sample");

    //Parsing
    int32u sample_count;
    Get_B4 (sample_count,                                       "sample-count");

    int32u Offset=1; //By default, begin at 1
    bool stss_PreviouslyEmpty=Streams[moov_trak_tkhd_TrackID].stss.empty();
    for (int32u Pos=0; Pos<sample_count; Pos++)
    {
        int32u sample_number;

        //Too much slow
        /*
        Get_B4 (sample_number,                                  "sample-number");
        */

        //Faster
        if (Element_Offset+4>Element_Size)
            break; //Problem
        sample_number=BigEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset);
        Element_Offset+=4;

        //Coherency testing (first frame is 0 or 1)
        if (sample_number==0 && Offset)
        {
            for (size_t Pos=0; Streams[moov_trak_tkhd_TrackID].stss.size(); Pos++)
                Streams[moov_trak_tkhd_TrackID].stss[Pos]--;
            Offset=0;
        }

        Streams[moov_trak_tkhd_TrackID].stss.push_back(sample_number-Offset);
    }
    if (!stss_PreviouslyEmpty)
        std::sort(Streams[moov_trak_tkhd_TrackID].stss.begin(), Streams[moov_trak_tkhd_TrackID].stss.end());
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsc()
{
    NAME_VERSION_FLAG("Sample To Chunk");

    //Parsing
    int32u Count;
    stream::stsc_struct Stsc;
    Get_B4 (Count,                                              "Number of entries");
    for (int32u Pos=0; Pos<Count; Pos++)
    {
        //Too much slow
        /*
        Element_Begin1("Entry", 12);
        int32u SampleDescriptionId;
        Get_B4 (Stsc.FirstChunk,                                "First chunk");
        Get_B4 (Stsc.SamplesPerChunk,                           "Samples per chunk");
        Get_B4 (SampleDescriptionId,                            "Sample description ID");
        Element_Info1(Stsc.FirstChunk);
        Element_Info1(Stsc.SamplesPerChunk);
        Element_Info1(SampleDescriptionId);
        Element_End0();
        Streams[moov_trak_tkhd_TrackID].stsc.push_back(Stsc);
        */

        //Faster
        if (Pos<FrameCount_MaxPerStream)
        {
            if (Element_Offset+12>Element_Size)
                break; //Problem
            Stsc.FirstChunk     =BigEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset  );
            Stsc.SamplesPerChunk=BigEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset+4);
            Element_Offset+=12;

            Streams[moov_trak_tkhd_TrackID].stsc.push_back(Stsc);
        }
        else
            Element_Offset=Element_Size; //No need
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd()
{
    NAME_VERSION_FLAG("Sample Description");

    //Parsing
    Skip_B4(                                                    "Count");

    //Filling
    moov_trak_mdia_minf_stbl_stsd_Pos=0;
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_text()
{
    Element_Name("Text (Apple)");

    //Parsing
    int8u TextName_Size;
    Skip_B4(                                                    "Reserved");
    Skip_B2(                                                    "Reserved");
    Skip_B2(                                                    "Data reference index");
    Info_B4(Flags,                                              "Display flags");
        Skip_Flags(Flags,  1,                                   "Don't auto scale");
        Skip_Flags(Flags,  3,                                   "Use movie background color");
        Skip_Flags(Flags,  5,                                   "Scroll in");
        Skip_Flags(Flags,  6,                                   "Scroll out");
        Skip_Flags(Flags,  7,                                   "Horizontal scroll");
        Skip_Flags(Flags,  8,                                   "Reverse scroll");
        Skip_Flags(Flags,  9,                                   "Continuous scroll");
        Skip_Flags(Flags, 12,                                   "Drop shadow");
        Skip_Flags(Flags, 13,                                   "Anti-alias");
        Skip_Flags(Flags, 14,                                   "Key text");
    Skip_B4(                                                    "Text justification");
    Skip_B2(                                                    "Background color (Red)");
    Skip_B2(                                                    "Background color (Green)");
    Skip_B2(                                                    "Background color (Blue)");
    Element_Begin1("Default text box");
        Skip_B2(                                                "top");
        Skip_B2(                                                "left");
        Skip_B2(                                                "bottom");
        Skip_B2(                                                "right");
    Element_End0();
    Skip_B8(                                                    "Reserved");
    Skip_B2(                                                    "Font number");
    Info_B2(FontFace,                                           "Font face");
        Skip_Flags(FontFace, 0,                                 "Bold");
        Skip_Flags(FontFace, 1,                                 "Italic");
        Skip_Flags(FontFace, 2,                                 "Underline");
        Skip_Flags(FontFace, 3,                                 "Outline");
        Skip_Flags(FontFace, 4,                                 "Shadow");
        Skip_Flags(FontFace, 5,                                 "Condense");
        Skip_Flags(FontFace, 6,                                 "Extend");
    Skip_B1(                                                    "Reserved");
    Skip_B1(                                                    "Reserved"); //Specs say 16-bits, but not in coherency with my test sample
    Skip_B2(                                                    "Foreground color (Red)");
    Skip_B2(                                                    "Foreground color (Green)");
    Skip_B2(                                                    "Foreground color (Blue)");
    Get_B1 (TextName_Size,                                      "Text name size");
    Skip_Local(TextName_Size,                                   "Text name");

    FILLING_BEGIN();
        CodecID_Fill(__T("text"), StreamKind_Last, StreamPos_Last, InfoCodecID_Format_Mpeg4);
        Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Codec), "text", Unlimited, true, true);
        Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Format), "Timed Text", Unlimited, true, true);

        #ifdef MEDIAINFO_TIMEDTEXT_YES
            File_TimedText* Parser=new File_TimedText;
            int64u Elemen_Code_Save=Element_Code;
            Element_Code=moov_trak_tkhd_TrackID; //Element_Code is use for stream identifier
            Open_Buffer_Init(Parser);
            Element_Code=Elemen_Code_Save;
            Parser->IsChapter=Streams[moov_trak_tkhd_TrackID].IsChapter;
            Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
            mdat_MustParse=true; //Data is in MDAT
        #endif //MEDIAINFO_TIMEDTEXT_YES
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_tmcd()
{
    Element_Name("TimeCode");

    //Parsing
    stream::timecode *tc=new stream::timecode();
    int32u TimeCodeFlags;
    int8u   NumberOfFrames;
    Skip_B4(                                                    "Reserved");
    Skip_B2(                                                    "Reserved");
    Skip_B2(                                                    "Data reference index");
    Skip_B4(                                                    "Reserved");
    Get_B4 (TimeCodeFlags,                                      "Flags (timecode)");
        Get_Flags (TimeCodeFlags, 0, tc->DropFrame,             "Drop frame");
        Get_Flags (TimeCodeFlags, 1, tc->H24,                   "24 hour max ");
        Get_Flags (TimeCodeFlags, 2, tc->NegativeTimes,         "Negative times OK");
        Skip_Flags(TimeCodeFlags, 3,                            "Counter");
    Get_B4 (tc->TimeScale,                                      "Time scale");
    Get_B4 (tc->FrameDuration,                                  "Frame duration");
    Get_B1 (NumberOfFrames,                                     "Number of frames");
    if (Element_Size==Element_Offset+3 || (Element_Size>=Element_Offset+7 && Element_Size>=Element_Offset+7+BigEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset+3)))
        Skip_B3(                                                "Reserved");
    else
        Skip_B1(                                                "Reserved"); //Non-standard but several files are like that

    FILLING_BEGIN();
        //Bug in one file
        if (tc->TimeScale==25 && tc->FrameDuration==100)
            tc->TimeScale=2500;

        if (StreamKind_Last!=Stream_Other)
        {
            Stream_Prepare(Stream_Other);
            Fill(Stream_Other, StreamPos_Last, Other_Type, "Time code");
            Fill(Stream_Other, StreamPos_Last, Other_Format, "QuickTime TC");
            Streams[moov_trak_tkhd_TrackID].StreamKind=Stream_Other;
            Streams[moov_trak_tkhd_TrackID].StreamPos=StreamPos_Last;
        }

        //Filling
        Streams[moov_trak_tkhd_TrackID].TimeCode=tc;

        //Preparing TimeCode parser
        File_Mpeg4_TimeCode* Parser=new File_Mpeg4_TimeCode;
        Open_Buffer_Init(Parser);
        mdat_Pos_ToParseInPriority_StreamIDs.push_back(moov_trak_tkhd_TrackID);
        Streams[moov_trak_tkhd_TrackID].IsPriorityStream=true;
        ((File_Mpeg4_TimeCode*)Parser)->NumberOfFrames=NumberOfFrames; //tc->FrameDuration?(((float64)tc->TimeScale)/tc->FrameDuration):0;
        ((File_Mpeg4_TimeCode*)Parser)->DropFrame=tc->DropFrame;
        ((File_Mpeg4_TimeCode*)Parser)->NegativeTimes=tc->NegativeTimes;
        Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
        mdat_MustParse=true; //Data is in MDAT
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_tmcd_name()
{
    Element_Name("Name (TimeCode)");

    //Parsing
    Ztring Value;
    int16u Size, Language;
    Get_B2(Size,                                                "Size");
    Get_B2(Language,                                            "Language"); Param_Info1(Language_Get(Language));
    if (Size)
    {
        int8u Junk;
        Peek_B1(Junk);
        if (Junk<0x20)
        {
            Skip_B1(                                                "Junk");
            Size--;
        }
    }
    Get_Local(Size, Value,                                      "Value");

    FILLING_BEGIN();
        Fill(Stream_Other, StreamPos_Last, "Title", Value);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_tx3g()
{
    Element_Name("Text");

    //Parsing
    Skip_B4(                                                    "Reserved");
    Skip_B2(                                                    "Reserved");
    Skip_B2(                                                    "Data reference index");
    Info_B4(Flags,                                              "displayFlags");
        Skip_Flags(Flags,  5,                                   "Scroll in");
        Skip_Flags(Flags,  6,                                   "Scroll out");
        Skip_Flags(Flags,  7,                                   "Horizontal scroll");
        Skip_Flags(Flags,  8,                                   "Reverse scroll");
        Skip_Flags(Flags, 10,                                   "Continuous karaoke");
        Skip_Flags(Flags, 17,                                   "write text vertically");
        Skip_Flags(Flags, 18,                                   "fill text region");
    Skip_B1(                                                    "horizontal-justification");
    Skip_B1(                                                    "vertical-justification");
    Skip_B1(                                                    "background-color-rgba (red)");
    Skip_B1(                                                    "background-color-rgba (green)");
    Skip_B1(                                                    "background-color-rgba (blue)");
    Skip_B1(                                                    "background-color-rgba (alpha)");
    Element_Begin1("default-text-box");
        if (Element_Size>42 && CC4(Buffer+Buffer_Offset+38)==Elements::moov_trak_mdia_minf_stbl_stsd_tx3g_ftab)
        {
            Skip_B1(                                            "top"); //Specs say 16-bits, but not in coherency with a test sample
            Skip_B1(                                            "left"); //Specs say 16-bits, but not in coherency with a test sample
            Skip_B1(                                            "bottom"); //Specs say 16-bits, but not in coherency with a test sample
            Skip_B1(                                            "right"); //Specs say 16-bits, but not in coherency with a test sample
        }
        else
        {
            Skip_B2(                                            "top");
            Skip_B2(                                            "left");
            Skip_B2(                                            "bottom");
            Skip_B2(                                            "right");
        }
    Element_End0();
    Element_Begin1("default-style");
        Skip_B2(                                                "startChar");
        Skip_B2(                                                "endChar");
        Skip_B2(                                                "font-ID");
        Skip_B1(                                                "face-style-flags");
        Skip_B1(                                                "font-size");
        Skip_B1(                                                "text-color-rgba (red)");
        Skip_B1(                                                "text-color-rgba (green)");
        Skip_B1(                                                "text-color-rgba (blue)");
        Skip_B1(                                                "text-color-rgba (alpha)");
    Element_End0();

    FILLING_BEGIN();
        CodecID_Fill(__T("tx3g"), StreamKind_Last, StreamPos_Last, InfoCodecID_Format_Mpeg4);
        Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Codec), "tx3g", Unlimited, true, true);
        Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Format), "Timed Text", Unlimited, true, true);

        #ifdef MEDIAINFO_TIMEDTEXT_YES
            File_TimedText* Parser=new File_TimedText;
            int64u Elemen_Code_Save=Element_Code;
            Element_Code=moov_trak_tkhd_TrackID; //Element_Code is use for stream identifier
            Open_Buffer_Init(Parser);
            Element_Code=Elemen_Code_Save;
            Parser->IsChapter=Streams[moov_trak_tkhd_TrackID].IsChapter;
            Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
            mdat_MustParse=true; //Data is in MDAT
        #endif //MEDIAINFO_TIMEDTEXT_YES
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_tx3g_ftab()
{
    Element_Name("Font table");

    //Found strange data in one file, but no specs about this
    if (Element_Size==0x17-8 && (BigEndian2int16u(Buffer+Buffer_Offset)!=1 || BigEndian2int16u(Buffer+Buffer_Offset+4)!=0x17-8-5))
    {
        Skip_XX(Element_Size,                                   "Unknown");
        return;
    }

    //Parsing
    int16u entry_count;
    Get_B2 (entry_count,                                        "entry-count");

    for (int16u Pos=0; Pos<entry_count; Pos++)
    {
        int8u FontName_Length;
        Skip_B2(                                                "font-ID");
        Get_B1 (FontName_Length,                                "font-name-length");
        Skip_Local(FontName_Length,                             "font-name");
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx()
{
    //Parsing
    Skip_B6(                                                    "Reserved");
    Skip_B2(                                                    "Data reference index");

    if (StreamKind_Last==Stream_Max)
        switch (Element_Code)
        {
            case Elements::moov_trak_mdia_minf_stbl_stsd_mp4a : Stream_Prepare(Stream_Audio); break;
            case Elements::moov_trak_mdia_minf_stbl_stsd_mp4v : Stream_Prepare(Stream_Video); break;
            default                                           : ;
        }

    switch (StreamKind_Last)
    {
        case Stream_Video : moov_trak_mdia_minf_stbl_stsd_xxxxVideo(); break;
        case Stream_Audio : moov_trak_mdia_minf_stbl_stsd_xxxxSound(); break;
        case Stream_Text  : moov_trak_mdia_minf_stbl_stsd_xxxxText (); break;
        default           :
                            switch (Element_Code)
                            {
                                case Elements::moov_trak_mdia_minf_stbl_stsd_mp4s : moov_trak_mdia_minf_stbl_stsd_xxxxStream(); break;
                                default                                           : Skip_XX(Element_TotalSize_Get()-Element_Offset, "Unknown");
                            }
    }

    FILLING_BEGIN();
        if (Streams[moov_trak_tkhd_TrackID].Parsers.size()==1 && !Retrieve(StreamKind_Last, StreamPos_Last, "Encryption").empty())
        {
            Finish(Streams[moov_trak_tkhd_TrackID].Parsers[0]);
        }
        moov_trak_mdia_minf_stbl_stsd_Pos++;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxxSound()
{
    Element_Name("Audio");

    int32u SampleRate, Channels, SampleSize, Flags=0;
    int16u Version, ID;
    Get_B2 (Version,                                            "Version");
    Skip_B2(                                                    "Revision level");
    Skip_C4(                                                    "Vendor");
    if (Version<2) // Version 0 or 1
    {
        int16u Channels16, SampleSize16, SampleRate16;
        Get_B2 (Channels16,                                     "Number of channels");
        Get_B2 (SampleSize16,                                   "Sample size");
        Get_B2 (ID,                                             "Compression ID");
        Skip_B2(                                                "Packet size");
        Get_B2 (SampleRate16,                                   "Sample rate"); Param_Info2(SampleRate16, " Hz");
        Skip_B2(                                                "Reserved");
        if (Version>=1)
        {
            Skip_B4(                                            "Samples per packet");
            Skip_B4(                                            "Bytes per packet");
            Skip_B4(                                            "Bytes per frame");
            Skip_B4(                                            "Bytes per sample");
        }
        Channels=Channels16;
        SampleSize=SampleSize16;
        SampleRate=SampleRate16;
    }
    else if (Version==2)
    {
        float64 SampleRateF64;
        Skip_B2(                                                "Reserved (0x0003)");
        Skip_B2(                                                "Reserved (0x0010)");
        Skip_B2(                                                "Reserved (0xFFFE)");
        Skip_B2(                                                "Reserved (0x0000)");
        Skip_B4(                                                "Reserved (0x00010000)");
        Skip_B4(                                                "Size of Struct");
        Get_BF8(SampleRateF64,                                  "Sample rate");
        Get_B4 (Channels,                                       "Number of channels");
        Skip_B4(                                                "Reserved (0x7F000000)");
        Get_B4 (SampleSize,                                     "Sample size");
        Get_B4 (Flags,                                          "Flags");
        Skip_B4(                                                "Bytes per packet");
        Skip_B4(                                                "Frames per packet");

        SampleRate=(int32u)SampleRateF64;
    }
    else
    {
        Skip_XX(Element_Size,                                   "Unknown");
        return;
    }

    //Bug found in one file: sample size is 16 with a 24-bit CodecID ("in24")
    if (Element_Code==0x696E3234 && SampleSize==16)
        SampleSize=24; //Correcting the header
    //Bug found in one file: sample size is 16 with a 32-bit CodecID ("fl32")
    if (Element_Code==0x666C3332 && SampleSize==16)
        SampleSize=32; //Correcting the header

    if (moov_trak_mdia_minf_stbl_stsd_Pos)
        return; //Handling only the first description

    FILLING_BEGIN();
        //samr bug viewed in some files: channels and Sampling rate are wrong
        if (Element_Code==0x73616D72) //"samr"
        {
            SampleRate=8000;
            Channels=1;
        }

        //lpcm puts "1" in the SampleRate field  and Timescale is the real sample size
        if (Element_Code==0x6C70636D && SampleRate==1) //"lpcm"
        {
            SampleRate=Streams[moov_trak_tkhd_TrackID].mdhd_TimeScale;
        }

        std::string Codec;
        Codec.append(1, (char)((Element_Code&0xFF000000)>>24));
        Codec.append(1, (char)((Element_Code&0x00FF0000)>>16));
        if (Codec!="ms") //Normal
        {
            Codec.append(1, (char)((Element_Code&0x0000FF00)>> 8));
            Codec.append(1, (char)((Element_Code&0x000000FF)>> 0));
            if (Codec!="mp4a") //mp4a is for Mpeg4 system
                CodecID_Fill(Ztring(Codec.c_str()), Stream_Audio, StreamPos_Last, InfoCodecID_Format_Mpeg4);
            if (Codec!="raw ")
                Fill(Stream_Audio, StreamPos_Last, Audio_Codec, Codec, false, true);
            else
                Fill(Stream_Audio, StreamPos_Last, Audio_Codec, "PCM", Error, false, true);
            Fill(Stream_Audio, StreamPos_Last, Audio_Codec_CC, Codec, false, true);
            if (Codec=="drms")
                Fill(Stream_Audio, StreamPos_Last, Audio_Encryption, "iTunes");
            if (Codec=="enca")
                Fill(Stream_Audio, StreamPos_Last, Audio_Encryption, "Encrypted");
        }
        else //Microsoft 2CC
        {
            int64u CodecI= ((Element_Code&0x0000FF00ULL)>> 8)
                         + ((Element_Code&0x000000FFULL)>> 0); //FormatTag
            Codec=Ztring().From_Number(CodecI, 16).To_Local();
            CodecID_Fill(Ztring::ToZtring(CodecI, 16), Stream_Audio, StreamPos_Last, InfoCodecID_Format_Riff);
            Fill(Stream_Audio, StreamPos_Last, Audio_Codec, Codec, true);
            Fill(Stream_Audio, StreamPos_Last, Audio_Codec_CC, Codec, true);
        }
        #if defined(MEDIAINFO_AAC_YES)
        if (Version==2 && Element_Code==Elements::moov_trak_mdia_minf_stbl_stsd_mp4a) //This is not normal, but I don't know where is audioObjectType, default to 2 (AAC LC)
        {
            File_Aac* Parser=new File_Aac;
            Parser->AudioSpecificConfig_OutOfBand(SampleRate, 2);
            Parser->Mode=File_Aac::Mode_raw_data_block;
            Parser->FrameIsAlwaysComplete=true;
            Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
        }
        #endif
        #if defined(MEDIAINFO_AMR_YES)
        if (MediaInfoLib::Config.CodecID_Get(Stream_Audio, InfoCodecID_Format_Mpeg4, Ztring(Codec.c_str()), InfoCodecID_Format)==__T("AMR"))
        {
            //Creating the parser
            File_Amr MI;
            MI.Codec=Ztring().From_Local(Codec.c_str());
            Open_Buffer_Init(&MI);

            //Parsing
            Open_Buffer_Continue(&MI, 0);

            //Filling
            Finish(&MI);
            Merge(MI, StreamKind_Last, 0, StreamPos_Last);
        }
        #endif
        #if defined(MEDIAINFO_ADPCM_YES)
        if (MediaInfoLib::Config.CodecID_Get(Stream_Audio, InfoCodecID_Format_Mpeg4, Ztring(Codec.c_str()), InfoCodecID_Format)==__T("ADPCM"))
        {
            //Creating the parser
            File_Adpcm MI;
            MI.Codec=Ztring().From_Local(Codec.c_str());
            Open_Buffer_Init(&MI);

            //Parsing
            Open_Buffer_Continue(&MI, 0);

            //Filling
            Finish(&MI);
            Merge(MI, StreamKind_Last, 0, StreamPos_Last);
        }
        #endif
        #if defined(MEDIAINFO_PCM_YES)
        if (MediaInfoLib::Config.CodecID_Get(Stream_Audio, InfoCodecID_Format_Mpeg4, Ztring(Codec.c_str()), InfoCodecID_Format)==__T("PCM"))
        {
            //Info of stream size
            Streams[moov_trak_tkhd_TrackID].stsz_Sample_Multiplier=Channels*SampleSize/8;
            #if MEDIAINFO_DEMUX
                Streams[moov_trak_tkhd_TrackID].PtsDtsAreSame=true;
            #endif // MEDIAINFO_DEMUX

            //Creating the parser
            if ((Channels==1 && (StreamPos_Last%2)==0)
             || (Streams.find(moov_trak_tkhd_TrackID-1)!=Streams.end() && Streams[moov_trak_tkhd_TrackID-1].IsPcmMono))
            {
                File_ChannelGrouping* Parser=new File_ChannelGrouping;
                if (StreamPos_Last%2 && !Streams[moov_trak_tkhd_TrackID-1].Parsers.empty())
                {
                    Parser->Channel_Pos=1;
                    Parser->Common=((File_ChannelGrouping*)Streams[moov_trak_tkhd_TrackID-1].Parsers[0])->Common;
                    Parser->StreamID=moov_trak_tkhd_TrackID-1;
                    Element_Code=moov_trak_tkhd_TrackID-1;
                }
                else
                {
                    Parser->Channel_Pos=0;
                    Streams[moov_trak_tkhd_TrackID].IsPcmMono=true;
                }
                Parser->Channel_Total=2;
                Parser->SamplingRate=(int16u)SampleRate;
                Parser->BitDepth=(int8u)SampleSize;

                Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
            }

            //PCM parser
            File_Pcm* Parser=new File_Pcm;
            #if MEDIAINFO_DEMUX
                if (Config->Demux_Unpacketize_Get())
                {
                    Parser->Demux_Level=2; //Container
                    Parser->Demux_UnpacketizeContainer=true;
                }
            #endif //MEDIAINFO_DEMUX
            if (Version==2)
            {
                if (Flags&0x01)
                    Parser->Endianness='F';
                else
                {
                    Parser->Endianness=(Flags&0x02)?'B':'L';
                    Parser->Sign=(Flags&0x04)?'S':'U';
                }
            }
            Parser->Codec=Ztring().From_Local(Codec.c_str());
            Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
            Streams[moov_trak_tkhd_TrackID].IsPcm=true;

            #if MEDIAINFO_DEMUX
                Streams[moov_trak_tkhd_TrackID].Demux_Level=4; //Intermediate
            #endif //MEDIAINFO_DEMUX
        }
        #endif
        #if defined(MEDIAINFO_MPEGA_YES)
        if (MediaInfoLib::Config.CodecID_Get(Stream_Audio, InfoCodecID_Format_Mpeg4, Ztring(Codec.c_str()), InfoCodecID_Format)==__T("MPEG Audio"))
        {
            //Creating the parser
            File_Mpega* Parser=new File_Mpega;
            Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
        }
        #endif
        if (Element_Code==0x6F776D61) //"owma"
        {
            //Parsing
            int32u SamplingRate, BytesPerSec;
            int16u CodecID, Channels, Data_Size, Resolution;
            Get_L2 (CodecID,                                            "Codec ID");
            Get_L2 (Channels,                                           "Number of Channels");
            Get_L4 (SamplingRate,                                       "Samples Per Second");
            Get_L4 (BytesPerSec,                                        "Average Number of Bytes Per Second");
            Skip_L2(                                                    "Block Alignment");
            Get_L2 (Resolution,                                         "Bits / Sample");
            Get_L2 (Data_Size,                                          "Codec Specific Data Size");

            //Filling
            Ztring Codec; Codec.From_Number(CodecID, 16);
            Codec.MakeUpperCase();
            CodecID_Fill(Codec, Stream_Audio, StreamPos_Last, InfoCodecID_Format_Riff);
            Fill(Stream_Audio, StreamPos_Last, Audio_Codec, Codec); //May be replaced by codec parser
            Fill(Stream_Audio, StreamPos_Last, Audio_Codec_CC, Codec);
            Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, Channels, 10, true);
            Fill(Stream_Audio, StreamPos_Last, Audio_SamplingRate, SamplingRate, 10, true);
            Fill(Stream_Audio, StreamPos_Last, Audio_BitRate, BytesPerSec*8, 10, true);
            Fill(Stream_Audio, StreamPos_Last, Audio_BitDepth, Resolution, 10, true);

            FILLING_BEGIN();
                //Creating the parser
                     if (0);
                #if defined(MEDIAINFO_MPEGA_YES)
                else if (MediaInfoLib::Config.CodecID_Get(Stream_Audio, InfoCodecID_Format_Riff, Ztring::ToZtring(CodecID, 16))==__T("MPEG Audio"))
                {
                    File_Mpega* Parser=new File_Mpega;
                    Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
                }
                #endif
            FILLING_END();

            //Parsing
            if (Data_Size>0)
            {
                Element_Begin1("Codec Specific Data");
                switch (CodecID)
                {
                    case 0x0161 :
                                    //Parsing
                                    Skip_L4(                            "SamplesPerBlock");
                                    Skip_L2(                            "EncodeOptions");
                                    Skip_L4(                            "SuperBlockAlign");
                                    break;
                    default     : Skip_XX(Data_Size,                    "Unknown");
                }
                Element_End0();
            }

            //Demux
            #if MEDIAINFO_DEMUX
                if (!Config->Demux_Avc_Transcode_Iso14496_15_to_Iso14496_10_Get())
                    switch (Config->Demux_InitData_Get())
                    {
                        case 0 :    //In demux event
                                    Demux_Level=2; //Container
                                    Demux(Buffer+Buffer_Offset+Element_Offset-(18+Data_Size), (size_t)(18+Data_Size), ContentType_Header);
                                    break;
                        case 1 :    //In field
                                    {
                                    std::string Data_Raw((const char*)(Buffer+Buffer_Offset+Element_Offset-(18+Data_Size)), (size_t)(18+Data_Size));
                                    std::string Data_Base64(Base64::encode(Data_Raw));
                                    Fill(Stream_Audio, StreamPos_Last, "Demux_InitBytes", Data_Base64);
                                    (*Stream_More)[Stream_Audio][StreamPos_Last](Ztring().From_Local("Demux_InitBytes"), Info_Options)=__T("N NT");
                                    }
                                    break;
                        default :   ;
                    }
            #endif //MEDIAINFO_DEMUX
        }

        #if MEDIAINFO_DEMUX
            if (Streams[moov_trak_tkhd_TrackID].Parsers.empty() && Config_Demux)
            {
                File__Analyze* Parser=new File__Analyze; //Only for activating Demux
                Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
            }
        #endif //MEDIAINFO_DEMUX
        for (size_t Pos=0; Pos<Streams[moov_trak_tkhd_TrackID].Parsers.size(); Pos++)
        {
            int64u Elemen_Code_Save=Element_Code;
            Element_Code=moov_trak_tkhd_TrackID; //Element_Code is use for stream identifier
            Open_Buffer_Init(Streams[moov_trak_tkhd_TrackID].Parsers[Pos]);
            Element_Code=Elemen_Code_Save;
            mdat_MustParse=true; //Data is in MDAT
        }

        Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, Channels, 10, true);
        if (SampleSize!=0 && Element_Code!=0x6D703461 && (Element_Code&0xFFFF0000)!=0x6D730000 && Retrieve(Stream_Audio, StreamPos_Last, Audio_BitDepth).empty()) //if not mp4a, and not ms*
            Fill(Stream_Audio, StreamPos_Last, Audio_BitDepth, SampleSize, 10, true);
        Fill(Stream_Audio, StreamPos_Last, Audio_SamplingRate, SampleRate, 10, true);

        //Sometimes, more Atoms in this atoms
        if (Element_Offset+8<Element_Size)
            Element_ThisIsAList();
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxxStream()
{
    Element_Name("Stream"); //Defined in ISO-IEC 14496-14

    if (moov_trak_mdia_minf_stbl_stsd_Pos)
        return; //Handling only the first description

    FILLING_BEGIN();
    //Sometimes, more Atoms in this atoms
    if (Element_Offset+8<Element_Size)
        Element_ThisIsAList();
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxxText()
{
    Element_Name("Text");

    if (moov_trak_mdia_minf_stbl_stsd_Pos)
        return; //Handling only the first description

    FILLING_BEGIN();
        Ztring CodecID; CodecID.From_CC4((int32u)Element_Code);
        CodecID_Fill(CodecID, Stream_Text, StreamPos_Last, InfoCodecID_Format_Mpeg4);

        if (MediaInfoLib::Config.CodecID_Get(Stream_Text, InfoCodecID_Format_Mpeg4, CodecID, InfoCodecID_Format)==__T("EIA-608"))
        {
            //Creating the parser
            File_Mpeg4* Parser=new File_Mpeg4;
            Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
        }
        #if defined(MEDIAINFO_CDP_YES)
        if (MediaInfoLib::Config.CodecID_Get(Stream_Text, InfoCodecID_Format_Mpeg4, CodecID, InfoCodecID_Format)==__T("EIA-708"))
        {
            //Creating the parser
            File_Cdp* Parser=new File_Cdp;
            Parser->WithAppleHeader=true;
            Parser->AspectRatio=((float)16)/9; //TODO: this is hardcoded, must adapt it to the real video aspect ratio
            Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
        }
        #endif
        #if MEDIAINFO_DEMUX
            if (Streams[moov_trak_tkhd_TrackID].Parsers.empty() && Config_Demux)
            {
                File__Analyze* Parser=new File__Analyze; //Only for activating Demux
                Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
            }
        #endif //MEDIAINFO_DEMUX
        for (size_t Pos=0; Pos<Streams[moov_trak_tkhd_TrackID].Parsers.size(); Pos++)
        {
            int64u Elemen_Code_Save=Element_Code;
            Element_Code=moov_trak_tkhd_TrackID; //Element_Code is use for stream identifier
            Open_Buffer_Init(Streams[moov_trak_tkhd_TrackID].Parsers[Pos]);
            Element_Code=Elemen_Code_Save;
            mdat_MustParse=true; //Data is in MDAT
        }

        //Sometimes, more Atoms in this atoms
        if (Element_Offset+8<Element_Size)
            Element_ThisIsAList();
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxxVideo()
{
    Element_Name("Video");

    int16u Width, Height, Depth, ColorTableID;
    int8u  CompressorName_Size;
    Skip_B2(                                                    "Version");
    Skip_B2(                                                    "Revision level");
    Skip_C4(                                                    "Vendor");
    Skip_B4(                                                    "Temporal quality");
    Skip_B4(                                                    "Spatial quality");
    Get_B2 (Width,                                              "Width");
    Get_B2 (Height,                                             "Height");
    Skip_B4(                                                    "Horizontal resolution");
    Skip_B4(                                                    "Vertical resolution");
    Skip_B4(                                                    "Data size");
    Skip_B2(                                                    "Frame count");
    Peek_B1(CompressorName_Size);
    if (CompressorName_Size<32)
    {
        //This is pascal string
        Skip_B1(                                                "Compressor name size");
        Skip_Local(CompressorName_Size,                         "Compressor name");
        Skip_XX(32-1-CompressorName_Size,                       "Padding");
    }
    else
        //this is hard-coded 32-byte string
        Skip_Local(32,                                          "Compressor name");
    Get_B2 (Depth,                                              "Depth");
    Get_B2 (ColorTableID,                                       "Color table ID");
    if (ColorTableID==0 && Width && Height) //In one file, if Zero-filled, Color table is not present
        Skip_XX(32,                                             "Color Table");

    if (moov_trak_mdia_minf_stbl_stsd_Pos)
        return; //Handling only the first description

    FILLING_BEGIN();
        std::string Codec;
        Codec.append(1, (char)((Element_Code&0xFF000000)>>24));
        Codec.append(1, (char)((Element_Code&0x00FF0000)>>16));
        Codec.append(1, (char)((Element_Code&0x0000FF00)>> 8));
        Codec.append(1, (char)((Element_Code&0x000000FF)>> 0));
        if (Codec!="mp4v") //mp4v is for Mpeg4 system
            CodecID_Fill(Ztring(Codec.c_str()), Stream_Video, StreamPos_Last, InfoCodecID_Format_Mpeg4);
        Fill(Stream_Video, StreamPos_Last, Video_Codec, Codec, false, true);
        Fill(Stream_Video, StreamPos_Last, Video_Codec_CC, Codec, false, true);
        if (Codec=="drms")
            Fill(Stream_Video, StreamPos_Last, Video_Encryption, "iTunes");
        if (Codec=="encv")
            Fill(Stream_Video, StreamPos_Last, Video_Encryption, "Encrypted");
        if (Width)
            Fill(Stream_Video, StreamPos_Last, Video_Width, Width, 10, true);
        if (Height)
            Fill(Stream_Video, StreamPos_Last, Video_Height, Height, 10, true);
        if (moov_trak_tkhd_DisplayAspectRatio && Height && moov_trak_tkhd_DisplayAspectRatio!=((float32)Width)/Height)
        {
            Fill(Stream_Video, StreamPos_Last, Video_DisplayAspectRatio, moov_trak_tkhd_DisplayAspectRatio, 3, true);
            Fill(Stream_Video, StreamPos_Last, Video_PixelAspectRatio, moov_trak_tkhd_DisplayAspectRatio/Width*Height, 3, true);
        }
        Fill(Stream_Video, StreamPos_Last, Video_Rotation, moov_trak_tkhd_Rotation, 3);
        if (moov_trak_tkhd_Rotation)
            Fill(Stream_Video, StreamPos_Last, Video_Rotation_String, Ztring::ToZtring(moov_trak_tkhd_Rotation, 0)+__T("\xB0")); //degree sign

        //Specific cases
        if (Streams[moov_trak_tkhd_TrackID].Parsers.empty())
        {
            #if defined(MEDIAINFO_DVDIF_YES)
                if (MediaInfoLib::Config.CodecID_Get(Stream_Video, InfoCodecID_Format_Mpeg4, Ztring(Codec.c_str()), InfoCodecID_Format)==__T("DV"))
                {
                    File_DvDif* Parser=new File_DvDif;
                    Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
                }
            #endif
            #if defined(MEDIAINFO_MXF_YES)
                if (Element_Code==0x6D78336E || Element_Code==0x6D783370 || Element_Code==0x6D78356E || Element_Code==0x6D783570) //mx3n, mx3p, mx5n, mx5p
                {
                    Fill(Stream_Video, StreamPos_Last, Video_MuxingMode, "MXF");
                    File_Mxf* Parser=new File_Mxf;
                    Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);

                    #if MEDIAINFO_DEMUX
                        Streams[moov_trak_tkhd_TrackID].Demux_Level=4; //Intermediate
                    #endif //MEDIAINFO_DEMUX
               }
            #endif
            #if defined(MEDIAINFO_AIC_YES)
                if (MediaInfoLib::Config.CodecID_Get(Stream_Video, InfoCodecID_Format_Mpeg4, Ztring().From_CC4((int32u)Element_Code), InfoCodecID_Format)==__T("AIC"))
                {
                    File_Aic* Parser=new File_Aic;
                    Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
                }
            #endif
            #if defined(MEDIAINFO_AVC_YES)
                if (MediaInfoLib::Config.CodecID_Get(Stream_Video, InfoCodecID_Format_Mpeg4, Ztring().From_CC4((int32u)Element_Code), InfoCodecID_Format)==__T("AVC"))
                {
                    File_Avc* Parser=new File_Avc;
                    Parser->FrameIsAlwaysComplete=true;
                    #if MEDIAINFO_DEMUX
                        if (Config->Demux_Avc_Transcode_Iso14496_15_to_Iso14496_10_Get())
                        {
                            Streams[moov_trak_tkhd_TrackID].Demux_Level=4; //Intermediate
                            Parser->Demux_Level=2; //Container
                            Parser->Demux_UnpacketizeContainer=true;
                        }
                    #endif //MEDIAINFO_DEMUX
                    Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
                }
            #endif
            #if defined(MEDIAINFO_H263_YES)
                if (MediaInfoLib::Config.CodecID_Get(Stream_Video, InfoCodecID_Format_Mpeg4, Ztring().From_CC4((int32u)Element_Code), InfoCodecID_Format)==__T("H.263"))
                {
                    File_H263* Parser=new File_H263;
                    Parser->FrameIsAlwaysComplete=true;
                    Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
                }
            #endif
            #if defined(MEDIAINFO_MPEGV_YES)
                if (MediaInfoLib::Config.CodecID_Get(Stream_Video, InfoCodecID_Format_Mpeg4, Ztring().From_CC4((int32u)Element_Code), InfoCodecID_Format)==__T("MPEG Video") && Element_Code!=0x6D78336E && Element_Code!=0x6D783370 && Element_Code!=0x6D78356E && Element_Code!=0x6D783570) //mx3n, mx3p, mx5n, mx5p
                {
                    File_Mpegv* Parser=new File_Mpegv;
                    Parser->FrameIsAlwaysComplete=true;
                    #if MEDIAINFO_ADVANCED
                        Parser->InitDataNotRepeated_Optional=true;
                    #endif // MEDIAINFO_ADVANCED
                    Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
                }
            #endif
            #if defined(MEDIAINFO_PRORES_YES)
                if (MediaInfoLib::Config.CodecID_Get(Stream_Video, InfoCodecID_Format_Mpeg4, Ztring().From_CC4((int32u)Element_Code), InfoCodecID_Format)==__T("ProRes"))
                {
                    File_ProRes* Parser=new File_ProRes;
                    Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
                }
            #endif
            #if defined(MEDIAINFO_VC1_YES)
                if (Element_Code!=0x76632D31 && MediaInfoLib::Config.CodecID_Get(Stream_Video, InfoCodecID_Format_Mpeg4, Ztring().From_CC4((int32u)Element_Code), InfoCodecID_Format)==__T("VC-1")) // If "vc-1" CodecID, there is a dvc1 atom, not using the hack with Sequence Header direct search
                {
                    File_Vc1* Parser=new File_Vc1;
                    Parser->FrameIsAlwaysComplete=true;
                    Open_Buffer_Init(Parser);
                    Open_Buffer_Continue(Parser);
                    Element_Offset=Element_Size;
                    Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
                    mdat_MustParse=true; //Data is in MDAT
                }
            #endif
            #if defined(MEDIAINFO_VC3_YES)
                if (MediaInfoLib::Config.CodecID_Get(Stream_Video, InfoCodecID_Format_Mpeg4, Ztring().From_CC4((int32u)Element_Code), InfoCodecID_Format)==__T("VC-3"))
                {
                    File_Vc3* Parser=new File_Vc3;
                    Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
                }
            #endif
            #if defined(MEDIAINFO_JPEG_YES)
                if (MediaInfoLib::Config.CodecID_Get(Stream_Video, InfoCodecID_Format_Mpeg4, Ztring(Codec.c_str()), InfoCodecID_Format)==__T("JPEG"))
                {
                    File_Jpeg* Parser=new File_Jpeg;
                    Parser->StreamKind=Stream_Video;
                    Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
                }
            #endif
            #if defined(MEDIAINFO_MPEG4_YES)
                if (MediaInfoLib::Config.CodecID_Get(Stream_Video, InfoCodecID_Format_Mpeg4, Ztring(Codec.c_str()), InfoCodecID_Format)==__T("JPEG 2000"))
                {
                    File_Mpeg4* Parser=new File_Mpeg4;
                    Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);

                    #if MEDIAINFO_DEMUX
                        Streams[moov_trak_tkhd_TrackID].Demux_Level=4; //Intermediate
                    #endif //MEDIAINFO_DEMUX
                }
            #endif

            #if MEDIAINFO_DEMUX
                if (Streams[moov_trak_tkhd_TrackID].Parsers.empty() && Config_Demux)
                {
                    File__Analyze* Parser=new File__Analyze; //Only for activating Demux
                    Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
                }
            #endif //MEDIAINFO_DEMUX
            for (size_t Pos=0; Pos<Streams[moov_trak_tkhd_TrackID].Parsers.size(); Pos++)
                if (!Streams[moov_trak_tkhd_TrackID].Parsers[Pos]->Status[IsAccepted])
                {
                    int64u Elemen_Code_Save=Element_Code;
                    Element_Code=moov_trak_tkhd_TrackID; //Element_Code is use for stream identifier
                    Open_Buffer_Init(Streams[moov_trak_tkhd_TrackID].Parsers[Pos]);
                    Element_Code=Elemen_Code_Save;
                    mdat_MustParse=true; //Data is in MDAT
                }
        }

        //RGB(A)
        if (Codec=="raw " || Codec=="rle ")
        {
            if (Depth==1)
            {
                Fill(Stream_Video, StreamPos_Last, Video_ColorSpace, "Y", Unlimited, true, true);
                Fill(Stream_Video, StreamPos_Last, Video_BitDepth, 1);
            }
            else if (Depth<15)
            {
                Fill(Stream_Video, StreamPos_Last, Video_ColorSpace, "RGB", Unlimited, true, true);
                Fill(Stream_Video, StreamPos_Last, Video_BitDepth, 8);
            }
            else if (Depth==32 || Depth==36)
            {
                Fill(Stream_Video, StreamPos_Last, Video_ColorSpace, "RGBA", Unlimited, true, true);
                Fill(Stream_Video, StreamPos_Last, Video_BitDepth, Depth/4);
            }
            else
                Fill(Stream_Video, StreamPos_Last, Video_BitDepth, Depth/3);
        }

        //Descriptors or a list (we can see both!)
        if (Element_Offset+8<=Element_Size
             && ((CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+0)>='A' && CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+0)<='z') || (CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+0)>='0' && CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+0)<='9'))
             && ((CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+1)>='A' && CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+1)<='z') || (CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+1)>='0' && CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+1)<='9'))
             && ((CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+2)>='A' && CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+2)<='z') || (CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+2)>='0' && CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+2)<='9'))
             && ((CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+3)>='A' && CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+3)<='z') || (CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+3)>='0' && CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+3)<='9')))
                Element_ThisIsAList();
        else if (Element_Offset<Element_Size)
            Descriptors();
    FILLING_END();
}

//---------------------------------------------------------------------------
// Source: http://wiki.multimedia.cx/index.php?title=Apple_Lossless_Audio_Coding
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_alac()
{
    Element_Name("ALAC");

    //Parsing
    int32u  bitrate, samplerate;
    int8u   sample_size, channels;
    Skip_B4(                                                    "?");
    Skip_B4(                                                    "max sample per frame");
    Skip_B1(                                                    "?");
    Get_B1 (sample_size,                                        "sample size");
    Skip_B1(                                                    "rice history mult");
    Skip_B1(                                                    "rice initial history");
    Skip_B1(                                                    "rice kmodifier");
    Get_B1 (channels,                                           "channels");
    Skip_B1(                                                    "?");
    Skip_B1(                                                    "?");
    Skip_B4(                                                    "max coded frame size");
    Get_B4 (bitrate,                                            "bitrate");
    Get_B4 (samplerate,                                         "samplerate");

    FILLING_BEGIN_PRECISE();
        if (sample_size)
            Fill(Stream_Audio, StreamPos_Last, Audio_BitDepth, sample_size, 10, true);
        if (channels)
            Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, channels, 10, true);
        if (bitrate)
            Fill(Stream_Audio, StreamPos_Last, Audio_BitRate_Nominal, bitrate, 10, true);
        if (samplerate)
            Fill(Stream_Audio, StreamPos_Last, Audio_SamplingRate, samplerate, 10, true);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_ACLR()
{
    Element_Name("Avid ACLR");

    //Parsing
    Skip_C4(                                                    "tag");
    Skip_C4(                                                    "vers");
    Skip_B4(                                                    "yuv range"); //full 1 / normal 2
    Skip_B4(                                                    "unknown (always 0?)");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_APRG()
{
    Element_Name("Avid APRG");

    //Parsing
    Skip_C4(                                                    "tag");
    Skip_C4(                                                    "vers");
    Skip_B4(                                                    "unknown (always 1?)");
    Skip_B4(                                                    "unknown (always 0?)");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_ARES()
{
    Element_Name("Avid ARES");

    //Parsing
    //Source: http://lists.apple.com/archives/quicktime-api/2012/Mar/msg00039.html
    int32u x1;
    Skip_C4(                                                    "tag");
    Skip_C4(                                                    "vers");
    Skip_B4(                                                    "cid"); //Compression ID, From inspection this is 160 for MPEG 50, 161 for MPEG 40 and 162 for MPEG 30. 171 for raw?
    Skip_B4(                                                    "width");
    Skip_B4(                                                    "height"); //Of a field
    Get_B4 (x1,                                                 "x1");  //1=Progressive, 2=Interlaced
    Skip_B4(                                                    "zero");
    Skip_B4(                                                    "x2"); //4=Interlaced, 5=Progressive 1080, 6 Progressive not 1080?

    switch (x1)
    {
        case 0x01 : Fill(Stream_Video, StreamPos_Last, Video_ScanType, "Progressive", Unlimited, true, true); break;
        case 0x02 : Fill(Stream_Video, StreamPos_Last, Video_ScanType, "Interlaced", Unlimited, true, true); break;
        default   : ;
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_avcC()
{
    Element_Name("AVC decode");

    //Parsing
    int8u Version;
    Get_B1 (Version,                                            "Version");
    if (moov_trak_mdia_minf_stbl_stsd_Pos>1)
    {
        Skip_XX(Element_Size-Element_Offset,                    "Data not analyzed");
        return; //Handling only the first description
    }
    else if (Version==1)
    {
        #ifdef MEDIAINFO_AVC_YES
            for (size_t Pos=0; Pos<Streams[moov_trak_tkhd_TrackID].Parsers.size(); Pos++) //Removing any previous parser (in case of multiple streams in one track, or dummy parser for demux)
                delete Streams[moov_trak_tkhd_TrackID].Parsers[Pos];
            Streams[moov_trak_tkhd_TrackID].Parsers.clear();

            File_Avc* Parser=new File_Avc;
            Parser->FrameIsAlwaysComplete=true;
            #if MEDIAINFO_DEMUX
                Element_Code=moov_trak_tkhd_TrackID;
                if (Config->Demux_Avc_Transcode_Iso14496_15_to_Iso14496_10_Get())
                {
                    Streams[moov_trak_tkhd_TrackID].Demux_Level=4; //Intermediate
                    Parser->Demux_Level=2; //Container
                    Parser->Demux_UnpacketizeContainer=true;
                }
            #endif //MEDIAINFO_DEMUX
            Open_Buffer_Init(Parser);
            Parser->MustParse_SPS_PPS=true;
            Parser->MustSynchronize=false;
            Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
            mdat_MustParse=true; //Data is in MDAT

            //Demux
            #if MEDIAINFO_DEMUX
                if (!Config->Demux_Avc_Transcode_Iso14496_15_to_Iso14496_10_Get())
                    switch (Config->Demux_InitData_Get())
                    {
                        case 0 :    //In demux event
                                    Demux_Level=2; //Container
                                    Demux(Buffer+Buffer_Offset, (size_t)Element_Size, ContentType_Header);
                                    break;
                        case 1 :    //In field
                                    {
                                    std::string Data_Raw((const char*)(Buffer+Buffer_Offset), (size_t)Element_Size);
                                    std::string Data_Base64(Base64::encode(Data_Raw));
                                    Fill(Stream_Video, StreamPos_Last, "Demux_InitBytes", Data_Base64);
                                    (*Stream_More)[Stream_Video][StreamPos_Last](Ztring().From_Local("Demux_InitBytes"), Info_Options)=__T("N NT");
                                    }
                                    break;
                        default :   ;
                    }
            #endif //MEDIAINFO_DEMUX

            //Parsing
            Open_Buffer_Continue(Parser);

            Parser->MustParse_SPS_PPS=false;
            Parser->SizedBlocks=true;  //Now this is SizeBlocks
        #else
            Skip_XX(Element_Size,                               "AVC Data");
        #endif
    }
    else
        Skip_XX(Element_Size,                                   "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_bitr()
{
    Element_Name("BitRate");

    //Parsing
    int32u Avg_Bitrate, Max_Bitrate;
    Get_B4 (Avg_Bitrate,                                        "Avg_Bitrate");
    Get_B4 (Max_Bitrate,                                        "Max_Bitrate");

    if (moov_trak_mdia_minf_stbl_stsd_Pos>1)
        return; //Handling only the first description

    FILLING_BEGIN();
        if (Avg_Bitrate)
            Fill(StreamKind_Last, StreamPos_Last, "BitRate", Avg_Bitrate);
        if (Max_Bitrate)
            Fill(StreamKind_Last, StreamPos_Last, "BitRate_Maximum", Max_Bitrate);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_btrt()
{
    Element_Name("BitRate");

    //Parsing
    int32u maxBitrate, avgBitrate;
    Skip_B4(                                                    "bufferSizeDB");
    Get_B4 (maxBitrate,                                         "maxBitrate");
    Get_B4 (avgBitrate,                                         "avgBitrate");

    if (moov_trak_mdia_minf_stbl_stsd_Pos>1)
        return; //Handling only the first description

    FILLING_BEGIN();
        //if (avgBitrate)
        //    Fill(StreamKind_Last, StreamPos_Last, "BitRate",         avgBitrate); //Not trustable enough, and we have precise bitrate from stream size with
        if (maxBitrate)
            Fill(StreamKind_Last, StreamPos_Last, "BitRate_Maximum", maxBitrate);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_chan()
{
    NAME_VERSION_FLAG("Channels");

    //Parsing
    //From http://developer.apple.com/mac/library/documentation/MusicAudio/Reference/CAFSpec/CAF_spec/CAF_spec.html
    std::string ChannelDescription_Layout;
    int64u ChannelLabels=0;
    int32u ChannelLayoutTag, ChannelBitmap, NumberChannelDescriptions;
    bool ChannelLabels_Valid=true;
    Get_B4 (ChannelLayoutTag,                                   "ChannelLayoutTag");
    Get_B4 (ChannelBitmap,                                      "ChannelBitmap");
    Get_B4 (NumberChannelDescriptions,                          "NumberChannelDescriptions");
    if (ChannelLayoutTag==0) //UseChannelDescriptions
    {
        for (int32u Pos=0; Pos<NumberChannelDescriptions; Pos++)
        {
            int32u ChannelLabel;
            Get_B4 (ChannelLabel,                                   "ChannelLabel");
            if (ChannelLabel<64)
                ChannelLabels|=(((int64u)1)<<ChannelLabel);
            else
                ChannelLabels_Valid=false;
            ChannelDescription_Layout+=Mpeg4_chan_ChannelDescription_Layout(ChannelLabel);
            ChannelDescription_Layout+=__T(' ');
            Skip_B4(                                                "ChannelFlags");
            Skip_BF4(                                               "Coordinates (0)");
            Skip_BF4(                                               "Coordinates (1)");
            Skip_BF4(                                               "Coordinates (2)");
        }
        if (!ChannelDescription_Layout.empty())
            ChannelDescription_Layout.resize(ChannelDescription_Layout.size()-1);
    }

    if (moov_trak_mdia_minf_stbl_stsd_Pos>1)
        return; //Handling only the first description

    FILLING_BEGIN();
        if (ChannelLayoutTag==0) //UseChannelDescriptions
        {
            Fill(Stream_Audio, StreamPos_Last, Audio_ChannelLayout, ChannelDescription_Layout.c_str(), Unlimited, true, true);
            if (ChannelLabels_Valid)
                Fill(Stream_Audio, StreamPos_Last, Audio_ChannelPositions, Mpeg4_chan_ChannelDescription(ChannelLabels), true, true);
            else
                Fill(Stream_Audio, StreamPos_Last, Audio_ChannelPositions, ChannelDescription_Layout);
        }
        else if (ChannelLayoutTag==0x10000) //UseChannelBitmap
        {
            int8u Channels=0;
            for (size_t Bit=0; Bit<18; Bit++)
                if (ChannelBitmap&(1<<Bit))
                {
                    Channels++;
                    ChannelDescription_Layout+=Mpeg4_chan_ChannelBitmap_Layout((int32u)(Bit+1));
                    ChannelDescription_Layout+=__T(' ');
                }
            if (Channels)
            {
                //Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, Channels, 10, true); //Channel count from this atom should not be used as a primary source, it may be wrong
                Fill(Stream_Audio, StreamPos_Last, Audio_ChannelPositions, Mpeg4_chan_ChannelBitmap(ChannelBitmap), true, true);
                if (!ChannelDescription_Layout.empty())
                {
                    ChannelDescription_Layout.resize(ChannelDescription_Layout.size()-1);
                    Fill(Stream_Audio, StreamPos_Last, Audio_ChannelLayout, ChannelDescription_Layout.c_str(), Unlimited, true, true);
                }
            }
        }
        else if (ChannelLayoutTag>0x10000)
        {
            //int16u Channels=ChannelLayoutTag&0x0000FFFF;
            int16u Ordering=(ChannelLayoutTag&0xFFFF0000)>>16;
            //Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, Channels, 10, true); //Channel count from this atom should not be used as a primary source, it may be wrong
            Fill(Stream_Audio, StreamPos_Last, Audio_ChannelPositions, Mpeg4_chan(Ordering), Unlimited, true, true);
            Fill(Stream_Audio, StreamPos_Last, Audio_ChannelLayout, Mpeg4_chan_Layout(Ordering));
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_clap()
{
    Element_Name("Clean Aperture");

    //Parsing
    int32u apertureWidth_N, apertureWidth_D, apertureHeight_N, apertureHeight_D;
    Get_B4 (apertureWidth_N,                                    "apertureWidth_N");
    Get_B4 (apertureWidth_D,                                    "apertureWidth_D");
    Get_B4 (apertureHeight_N,                                   "apertureHeight_N");
    Get_B4 (apertureHeight_D,                                   "apertureHeight_D");
    Skip_B4(                                                    "horizOff_N");
    Skip_B4(                                                    "horizOff_D");
    Skip_B4(                                                    "vertOff_N");
    Skip_B4(                                                    "vertOff_D");

    if (moov_trak_mdia_minf_stbl_stsd_Pos>1)
        return; //Handling only the first description

    FILLING_BEGIN();
        if (apertureWidth_N && apertureWidth_D && apertureHeight_N && apertureHeight_D)
        {
            Streams[moov_trak_tkhd_TrackID].CleanAperture_Width=((float)apertureWidth_N)/apertureWidth_D;
            Streams[moov_trak_tkhd_TrackID].CleanAperture_Height=((float)apertureHeight_N)/apertureHeight_D;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_colr()
{
    Element_Name("Color Parameter");

    //Parsing
    int16u  colour_primaries, transfer_characteristics, matrix_coefficients;
    Skip_C4(                                                    "Color parameter type");
    Get_B2 (colour_primaries,                                   "Primaries index"); Param_Info1(Mpegv_colour_primaries((int8u)colour_primaries));
    Get_B2 (transfer_characteristics,                           "Transfer function index"); Param_Info1(Mpegv_transfer_characteristics((int8u)transfer_characteristics));
    Get_B2 (matrix_coefficients,                                "Matrix index"); Param_Info1(Mpegv_matrix_coefficients((int8u)matrix_coefficients));

    FILLING_BEGIN();
        if (Retrieve(Stream_Video, StreamPos_Last, Video_colour_description_present).empty()) //Using only the first one met
        {
            Fill(Stream_Video, StreamPos_Last, Video_colour_description_present, "Yes");
            Fill(Stream_Video, StreamPos_Last, Video_colour_primaries, Mpegv_colour_primaries((int8u)colour_primaries));
            Fill(Stream_Video, StreamPos_Last, Video_transfer_characteristics, Mpegv_transfer_characteristics((int8u)transfer_characteristics));
            Fill(Stream_Video, StreamPos_Last, Video_matrix_coefficients, Mpegv_matrix_coefficients((int8u)matrix_coefficients));
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_d263()
{
    Element_Name("H263SpecificBox");

    //Parsing
    int32u Vendor;
    int8u  Version, H263_Level, H263_Profile;
    Get_C4 (Vendor,                                             "Encoder vendor");
    Get_B1 (Version,                                            "Encoder version");
    Get_B1 (H263_Level,                                         "H263_Level");
    Get_B1 (H263_Profile,                                       "H263_Profile");

    if (moov_trak_mdia_minf_stbl_stsd_Pos>1)
        return; //Handling only the first description

    Ztring ProfileLevel;
    switch (H263_Profile)
    {
        case 0x00 : ProfileLevel=__T("BaseLine"); break;
        default   : ProfileLevel.From_Number(H263_Profile);
    }
    ProfileLevel+=__T('@');
    ProfileLevel+=Ztring::ToZtring(((float32)H263_Level)/10, 1);
    Fill(Stream_Video, StreamPos_Last, Video_Format_Profile, ProfileLevel);
    Fill(Stream_Video, StreamPos_Last, Video_Encoded_Library_Name, Mpeg4_Vendor(Vendor));
    Fill(Stream_Video, StreamPos_Last, Video_Encoded_Library_Version, Version);
    Fill(Stream_Video, StreamPos_Last, Video_Encoded_Library, Retrieve(Stream_Video, StreamPos_Last, Video_Encoded_Library_Name)+__T(' ')+Ztring::ToZtring(Version));
    Ztring Encoded_Library_String=Retrieve(Stream_Video, StreamPos_Last, Video_Encoded_Library_Name)+(Version?(__T(" Revision ")+Ztring::ToZtring(Version)):Ztring());
    Fill(Stream_Video, StreamPos_Last, Video_Encoded_Library_String, Encoded_Library_String, true);
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_dac3()
{
    Element_Name("AC3SpecificBox");
    Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, "", Unlimited, true, true); //Remove the value (is always wrong in the stsd atom)

    //Parsing
    if (Retrieve(Stream_Audio, StreamPos_Last, Audio_CodecID)==__T("sac3"))
    {
        Element_Info1("Nero specific");
        int8u Version;
        Get_B1 (Version,                                        "Version");
        if (Version==1)
        {
            int8u bsid;
            Get_B1 (bsid,                                       "bsid");
            Skip_XX(Element_Size-Element_Offset,                "unknown");
            #ifdef MEDIAINFO_AC3_YES
                if (Streams[moov_trak_tkhd_TrackID].Parsers.empty())
                {
                    File_Ac3* Parser=new File_Ac3;
                    Open_Buffer_Init(Parser);
                    Parser->Frame_Count_Valid=2;
                    Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
                    mdat_MustParse=true; //Data is in MDAT
                }
            #else
                if (bsid<=0x08)
                    Fill(Stream_Audio, StreamKind_Last, Audio_Format, "AC-3");
                if (bsid>0x0A && bsid<=0x10)
                    Fill(Stream_Audio, StreamKind_Last, Audio_Format,  "E-AC-3");
            #endif
            return;
        }
        else
        {
            Skip_XX(Element_Size,                               "Data");
            return;
        }
    }

    if (moov_trak_mdia_minf_stbl_stsd_Pos>1)
        return; //Handling only the first description

    #ifdef MEDIAINFO_AC3_YES
        if (Streams[moov_trak_tkhd_TrackID].Parsers.empty())
        {
            File_Ac3* Parser=new File_Ac3;
            Open_Buffer_Init(Parser);
            Parser->MustParse_dac3=true;
            Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
            mdat_MustParse=true; //Data is in MDAT

            //Parsing
            Open_Buffer_Continue(Parser);
        }
    #else
        Skip_XX(Element_Size,                                   "AC-3 Data");

        Fill(Stream_Audio, StreamKind_Last, Audio_Format, "AC-3");
    #endif
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_damr()
{
    Element_Name("AMR decode config");

    //Parsing
    int32u Vendor;
    int8u  Version;
    Get_C4 (Vendor,                                             "Encoder vendor");
    Get_B1 (Version,                                            "Encoder version");
    Skip_B2(                                                    "Packet modes");
    Skip_B1(                                                    "Number of packet mode changes");
    Skip_B1(                                                    "Samples per packet");

    if (moov_trak_mdia_minf_stbl_stsd_Pos>1)
        return; //Handling only the first description

    Fill(Stream_Audio, StreamPos_Last, Audio_Encoded_Library_Name, Mpeg4_Vendor(Vendor));
    Fill(Stream_Audio, StreamPos_Last, Audio_Encoded_Library_Version, Version);
    Fill(Stream_Audio, StreamPos_Last, Audio_Encoded_Library, Retrieve(Stream_Audio, StreamPos_Last, Audio_Encoded_Library_Name)+__T(' ')+Ztring::ToZtring(Version));
    Ztring Encoded_Library_String=Retrieve(Stream_Audio, StreamPos_Last, Audio_Encoded_Library_Name)+(Version?(__T(" Revision ")+Ztring::ToZtring(Version)):Ztring());
    Fill(Stream_Audio, StreamPos_Last, Audio_Encoded_Library_String, Encoded_Library_String, true);
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_dec3()
{
    Element_Name("EAC3SpecificBox");
    Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, "", Unlimited, true, true); //Remove the value (is always wrong in the stsd atom)

    if (moov_trak_mdia_minf_stbl_stsd_Pos>1)
    {
        return; //Handling only the first description
    }

    #ifdef MEDIAINFO_AC3_YES
        if (moov_trak_mdia_minf_stbl_stsd_Pos>1)
        {
            Skip_XX(Element_Size,                               "Data not analyzed");
            return; //Handling only the first description
        }

        if (Streams[moov_trak_tkhd_TrackID].Parsers.empty())
        {
            File_Ac3* Parser=new File_Ac3;
            Open_Buffer_Init(Parser);
            Parser->Frame_Count_Valid=2;
            Parser->MustParse_dec3=true;
            Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
            mdat_MustParse=true; //Data is in MDAT

            //Parsing
            Open_Buffer_Continue(Parser);
        }
    #else
        Skip_XX(Element_Size,                                   "E-AC-3 Data");

        Fill(Stream_Audio, StreamKind_Last, Audio_Format, "E-AC-3");
        Fill(Stream_Audio, StreamKind_Last, Audio_Format, "", Unlimited, true, true); //Remove the value (is always wrong in the stsd atom)
    #endif
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_ddts()
{
    Element_Name("DTSSpecificBox");
    Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, "", Unlimited, true, true); //Remove the value (is always wrong in the stsd atom)

    if (moov_trak_mdia_minf_stbl_stsd_Pos>1)
    {
        return; //Handling only the first description
    }

    //Comments about the new CodecIDs having this atom:
    //dtsc: DTS Core only
    //dtse: DTS Express, not having DTS Core
    //dtsh: DTS Core + DTS Lossless (MA), mixed
    //dtsl: DTS Lossless (MA), not having DTS Core

    int32u  DTSSamplingFrequency, maxBitrate, avgBitrate;
    int16u  ChannelLayout;
    int8u   CoreLayout;

    //Parsing
    Get_B4 (DTSSamplingFrequency,                               "DTSSamplingFrequency");
    Get_B4 (maxBitrate,                                         "maxBitrate");
    Get_B4 (avgBitrate,                                         "avgBitrate");
    Skip_B1(                                                    "pcmSampleDepth"); // 16 or 24 bits
    BS_Begin();
        Skip_S1(2,                                              "FrameDuration");  // 0 = 512, 1 = 1024, 2 = 2048, 3 = 4096
        Skip_S1(5,                                              "StreamConstruction");
        Skip_SB(                                                "CoreLFEPresent"); // 0 = none, 1 = LFE exists
        Get_S1 (6,  CoreLayout,                                 "CoreLayout");
        Skip_S2(14,                                             "CoreSize");
        Skip_SB(                                                "StereoDownmix"); // 0 = none, 1 = downmix present
        Skip_S1 (3,                                             "RepresentationType");
        Get_S2 (16, ChannelLayout,                              "ChannelLayout");
        Skip_SB(                                                "MultiAssetFlag"); // 0 = none, 1 = multiple asset
        Skip_SB(                                                "LBRDurationMod"); // 0 = ignore, 1 = Special LBR duration modifier
        Skip_S1(6,                                              "reserved");
    BS_End();

    FILLING_BEGIN();
        if (DTSSamplingFrequency)
            Fill(StreamKind_Last, StreamPos_Last, Audio_SamplingRate, DTSSamplingFrequency, 10, true); //This is the maximum sampling frequency
        if (avgBitrate)
            Fill(StreamKind_Last, StreamPos_Last, Audio_BitRate, avgBitrate);
        if (maxBitrate)
            Fill(StreamKind_Last, StreamPos_Last, Audio_BitRate_Maximum, maxBitrate);
        #if defined(MEDIAINFO_DTS_YES)
        if (ChannelLayout)
        {
            Fill(Stream_Audio, 0, Audio_ChannelPositions, DTS_HD_SpeakerActivityMask(ChannelLayout).c_str());
            Fill(Stream_Audio, 0, Audio_ChannelPositions_String2, DTS_HD_SpeakerActivityMask2(ChannelLayout).c_str());
        }
        if (CoreLayout && CoreLayout<16)
        {
            Fill(Stream_Audio, 0, Audio_ChannelPositions, DTS_ChannelPositions[CoreLayout]);
            Fill(Stream_Audio, 0, Audio_ChannelPositions_String2, DTS_ChannelPositions2[CoreLayout]);
        }
        #endif //defined(MEDIAINFO_DTS_YES)
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_dvc1()
{
    Element_Name("VC1SpecificBox");

    // Parsing
    int32u hrd_buffer, hrd_rate, framerate=0;
    int8u profile, level, frmrtq_pp, bitrtq_pp, dquant, vtransform, overlap, syncmarker, rangered, maxbframes, quantizer, finterpflag, no_interlace, no_multiple_seq, no_multiple_entry, no_slice_code, no_bframe;
    bool cbr, loopfilter, multires, fastuvmc,  extended_mv;

    BS_Begin();
        Get_S1 (4, profile,                                     "Profile");
        Get_S1 (3, level,                                       "Level");
        Skip_SB(                                                "reserved");
    BS_End();

    switch(profile)
    {
        case 0 : // Simple Profile
        case 4 : // Main Profile
                BS_Begin();
                    Get_S1 (3, level,                           "Level");      // should be the same as above
                    Get_SB (cbr,                                "CBR");        // should be equal to 1 if CBR Mode, 0 otherwhise
                    Skip_S1(4,                                  "Reserved");   // should be set to 0
                BS_End();
                Get_B3 (hrd_buffer,                             "HRD_Buffer"); // Buffer Size of Hypothetical Reference Decoder in ms
                Get_B4 (hrd_rate,                               "HRD_Rate");   // Peak Transmisson Rate of the Hypothetical Reference Decoder in bps
                Get_B4 (framerate,                              "FrameRate");  // Rounded FPS of the track
                BS_Begin();
                    Get_S1 (4, profile,                         "Profile");    // should be the same as above
                    Get_S1 (3, frmrtq_pp,                       "Frmrtq_postproc");
                    Get_S1 (5, bitrtq_pp,                       "Bitrtq_postproc");
                    Get_SB (loopfilter,                         "Loopfilter");
                    Skip_SB(                                    "Reserved");   // should be set to 0
                    Get_SB (multires,                           "MultiRes");
                    Skip_SB(                                    "Reserved");   // should be set to 1
                    Get_SB (fastuvmc,                           "Fastuvmc");
                    Get_SB (extended_mv,                        "Extended_mv");
                    Get_S1 (2, dquant,                          "Dquant");
                    Get_S1 (1, vtransform,                      "Vtransform");
                    Skip_SB(                                    "Reserved");   // should be set to 0
                    Get_S1 (1, overlap,                         "Overlap");
                    Get_S1 (1, syncmarker,                      "Syncmarker");
                    Get_S1 (1, rangered,                        "Rangered");
                    Get_S1 (3, maxbframes,                      "MaxBFrames"); // should be equal to 0 with SP
                    Get_S1 (2, quantizer,                       "Quantizer");
                    Get_S1 (1, finterpflag,                     "Finterpflag");
                    Skip_SB(                                    "Reserved");   // should be set to 1
                BS_End();
        break;
        case 12 : // Advanced Profile
                BS_Begin();
                    Get_S1 (3, level,                           "Level");      // should be the same as above
                    Get_SB (cbr,                                "CBR");        // should be equal to 1 if CBR Mode, 0 otherwhise
                    Skip_S1(6,                                  "Reserved");   // should be set to 0
                    Get_S1 (1, no_interlace,                    "No Interlace"); // should be set to 0 for Interlaced Content
                    Get_S1 (1, no_multiple_seq,                 "No Multiple Sequence");
                    Get_S1 (1, no_multiple_entry,               "No Multiple Entry");
                    Get_S1 (1, no_slice_code,                   "No Slice Code");
                    Get_S1 (1, no_bframe,                       "No BFrame");
                    Skip_SB(                                    "Reserved");
                BS_End();
                Get_B4 (framerate,                              "FrameRate");
                Element_Begin1("Sequence HDR");
                    File_Vc1* Parser=new File_Vc1;
                    Parser->FrameIsAlwaysComplete=true;
                    Open_Buffer_Init(Parser);
                    Open_Buffer_Continue(Parser);
                    Element_Offset=Element_Size;
                    Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
                    mdat_MustParse=true; //Data is in MDAT*/
                Element_End0();
        break;
    }

    FILLING_BEGIN();
        #if defined(MEDIAINFO_VC1_YES)
        if ((profile) && (level))
        {
            Ztring Profile, Level;

            switch (profile)
            {
                case 0  : Profile=__T("Simple"); break;
                case 4  : Profile=__T("Main"); break;
                case 12 : Profile=__T("Advanced"); break;
                default : Profile=__T("");
            }
            Profile+=__T("@");

            if (profile == 0) // Simple Profile
            {
                switch (level)
                {
                    case 0  : Level=__T("Low"); break;
                    case 2  : Level=__T("Medium"); break;
                    default : Level=__T("");
                }
            }
            else if (profile == 4) // Main Profile
            {
                switch (level)
                {
                    case 0  : Level=__T("Low"); break;
                    case 2  : Level=__T("Medium"); break;
                    case 4  : Level=__T("High"); break;
                    default : Level=__T("");
                }
            }
            else if (profile == 12) // Advanced Profile
            {
                switch (level)
                {
                    case 0  : Level=__T("L0"); break;
                    case 1  : Level=__T("L1"); break;
                    case 2  : Level=__T("L2"); break;
                    case 3  : Level=__T("L3"); break;
                    case 4  : Level=__T("L4"); break;
                    default : Level=__T("");
                }
            }
            Profile+=Level;
            Fill(Stream_Video, StreamPos_Last, Video_Format_Profile, Profile);
            Fill(Stream_Video, StreamPos_Last, Video_Codec_Profile, Profile);
        }
        #endif //defined(MEDIAINFO_VC1_YES)
        if (framerate && framerate!=(int32u)-1)
            Fill(StreamKind_Last, StreamPos_Last, Video_FrameRate, framerate, 3);
    FILLING_END();
}


//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_esds()
{
    NAME_VERSION_FLAG("ES Descriptor");
    INTEGRITY_VERSION(0);

    if (moov_trak_mdia_minf_stbl_stsd_Pos>1)
        return; //Handling only the first description

    FILLING_BEGIN();
        Descriptors();
        if (Streams[moov_trak_tkhd_TrackID].Parsers.size()==1 && !Retrieve(StreamKind_Last, StreamPos_Last, "Encryption").empty())
        {
            Finish(Streams[moov_trak_tkhd_TrackID].Parsers[0]);
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_fiel()
{
    //Source: http://developer.apple.com/quicktime/icefloe/dispatch019.html#fiel
    //Source: QuickTimeFileFormat2011.pdf:
    // 1 – T is displayed earliest, T is stored first in the file.
    // 6 – B is displayed earliest, B is stored first in the file.
    // 9 – B is displayed earliest, T is stored first in the file.
    //14 – T is displayed earliest, B is stored first in the file.
    Element_Name("Field/Frame Information");

    //Parsing
    int8u  fields, detail;
    Get_B1 (fields,                                             "fields");
    Get_B1 (detail,                                             "detail");

    if (moov_trak_mdia_minf_stbl_stsd_Pos>1)
        return; //Handling only the first description

    FILLING_BEGIN();
        switch(fields)
        {
            case 0x01 : Fill(Stream_Video, StreamPos_Last, Video_ScanType, "Progressive", Unlimited, true, true); break;
            case 0x02 : Fill(Stream_Video, StreamPos_Last, Video_ScanType, "Interlaced", Unlimited, true, true);
                        switch(detail)
                        {
                            case  1  :  // T is displayed earliest, T is stored first in the file.
                            case  9 :   // B is displayed earliest, T is stored first in the file.
                                        Fill(Stream_Video, StreamPos_Last, Video_ScanOrder, "TFF", Unlimited, true, true);
                                        break;
                            case  6  :  // B is displayed earliest, B is stored first in the file.
                            case 14 :   // T is displayed earliest, B is stored first in the file.
                                        Fill(Stream_Video, StreamPos_Last, Video_ScanOrder, "BFF", Unlimited, true, true);
                                        break;
                            default  :  ;
                        }
                        switch(detail)
                        {
                            case  9 :   // B is displayed earliest, T is stored first in the file.
                            case 14 :   // T is displayed earliest, B is stored first in the file.
                                        Streams[moov_trak_tkhd_TrackID].ScanOrder_StoredDisplayedInverted=true;
                                        break;
                            default  :  ;
                        }
                        #ifdef MEDIAINFO_JPEG_YES
                            if (Retrieve(Stream_Video, StreamPos_Last, Video_Format)==__T("JPEG") && Streams[moov_trak_tkhd_TrackID].Parsers.size()==1)
                                ((File_Jpeg*)Streams[moov_trak_tkhd_TrackID].Parsers[0])->Interlaced=true;
                        #endif //MEDIAINFO_JPEG_YES
                        break;
            default   : ;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_glbl()
{
    Element_Name("Global");

    if (Retrieve(Stream_Video, StreamPos_Last, Video_MuxingMode)==__T("MXF"))
    {
        Clear(Stream_Video, StreamPos_Last, Video_MuxingMode);
        for (size_t Pos=0; Pos<Streams[moov_trak_tkhd_TrackID].Parsers.size(); Pos++)
            delete Streams[moov_trak_tkhd_TrackID].Parsers[Pos];
        Streams[moov_trak_tkhd_TrackID].Parsers.clear();
        #if defined(MEDIAINFO_MPEGV_YES)
            File_Mpegv* Parser=new File_Mpegv;
            Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
        #endif //defined(MEDIAINFO_MPEGV_YES)

        //Re-init
        if (!Streams[moov_trak_tkhd_TrackID].Parsers.empty())
        {
            int64u Elemen_Code_Save=Element_Code;
            Element_Code=moov_trak_tkhd_TrackID; //Element_Code is use for stream identifier
            for (size_t Pos=0; Pos<Streams[moov_trak_tkhd_TrackID].Parsers.size(); Pos++)
                Open_Buffer_Init(Streams[moov_trak_tkhd_TrackID].Parsers[Pos]);
            Element_Code=Elemen_Code_Save;
            mdat_MustParse=true; //Data is in MDAT
        }
    }

    //Demux
    #if MEDIAINFO_DEMUX
        Demux_Level=2; //Container
        Demux(Buffer+Buffer_Offset, (size_t)Element_Size, ContentType_MainStream);
    #endif //MEDIAINFO_DEMUX

    //Parsing
    for (size_t Pos=0; Pos<Streams[moov_trak_tkhd_TrackID].Parsers.size(); Pos++)
        Open_Buffer_Continue(Streams[moov_trak_tkhd_TrackID].Parsers[Pos]);
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_hvcC()
{
    Element_Name("HEVCDecoderConfigurationRecord");

    //Parsing
    int8u Version;
    Get_B1 (Version,                                            "Version");
    if (moov_trak_mdia_minf_stbl_stsd_Pos>1)
    {
        Skip_XX(Element_Size-Element_Offset,                    "Data not analyzed");
        return; //Handling only the first description
    }
    else if (Version==1)
    {
        #ifdef MEDIAINFO_HEVC_YES
            for (size_t Pos=0; Pos<Streams[moov_trak_tkhd_TrackID].Parsers.size(); Pos++) //Removing any previous parser (in case of multiple streams in one track, or dummy parser for demux)
                delete Streams[moov_trak_tkhd_TrackID].Parsers[Pos];
            Streams[moov_trak_tkhd_TrackID].Parsers.clear();

            File_Hevc* Parser=new File_Hevc;
            Parser->FrameIsAlwaysComplete=true;
            Open_Buffer_Init(Parser);
            Parser->MustParse_VPS_SPS_PPS=true;
            Parser->MustSynchronize=false;
            Streams[moov_trak_tkhd_TrackID].Parsers.push_back(Parser);
            mdat_MustParse=true; //Data is in MDAT

            //Parsing
            Open_Buffer_Continue(Parser);

            Parser->SizedBlocks=true;  //Now this is SizeBlocks
        #else
            Skip_XX(Element_Size,                               "HEVC Data");
        #endif
    }
    else
        Skip_XX(Element_Size,                                   "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_idfm()
{
    Element_Name("Description");

    Info_C4(Description,                                        "Description"); Param_Info1(Mpeg4_Description(Description));
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_pasp()
{
    Element_Name("Pixel Aspect Ratio");

    //Parsing
    int32u hSpacing, vSpacing;
    Get_B4 (hSpacing,                                           "hSpacing");
    Get_B4 (vSpacing,                                           "vSpacing");

    if (moov_trak_mdia_minf_stbl_stsd_Pos>1)
        return; //Handling only the first description

    FILLING_BEGIN();
        if (vSpacing)
        {
            float32 PixelAspectRatio=(float32)hSpacing/vSpacing;
            Clear(Stream_Video, StreamPos_Last, Video_DisplayAspectRatio);
            Fill(Stream_Video, StreamPos_Last, Video_PixelAspectRatio, PixelAspectRatio, 3, true);
            Streams[moov_trak_tkhd_TrackID].CleanAperture_PixelAspectRatio=PixelAspectRatio; //This is the PAR of the clean aperture
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_sinf()
{
    Element_Name("Protection scheme information box");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_sinf_frma()
{
    Element_Name("Original format box");

    //Parsing
    std::string Codec;
    Get_String(4, Codec,                                        "data_format");

    FILLING_BEGIN();
        CodecID_Fill(Ztring(Codec.c_str()), Stream_Video, StreamPos_Last, InfoCodecID_Format_Mpeg4);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_sinf_imif()
{
    NAME_VERSION_FLAG("IPMP Information box");

    //Parsing
    Descriptors();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_sinf_schi()
{
    Element_Name("scheme information box");

    //Parsing
    Skip_XX(Element_Size,                                       "scheme_specific_data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_sinf_schm()
{
    NAME_VERSION_FLAG("scheme type box");

    //Parsing
    Skip_C4(                                                    "scheme_type");
    Skip_B4(                                                    "scheme_version");
    if (Flags&0x000001)
        Skip_UTF8(Element_Size-Element_Offset,                  "scheme_uri");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_wave()
{
    Element_Name("Wave");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_wave_acbf()
{
    Element_Name("Audio Bitrate Control Mode");

    //Parsing
    Skip_B4(                                                    "Bit Rate Control Mode");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_wave_enda()
{
    //Parsing
    int16u Endianness;
    Get_B2 (Endianness,                                          "Endianness");

    if (moov_trak_mdia_minf_stbl_stsd_Pos>1)
        return; //Handling only the first description

    FILLING_BEGIN();
        if (Streams[moov_trak_tkhd_TrackID].IsPcm)
        {
            if (Streams[moov_trak_tkhd_TrackID].Parsers.size()==1)
                ((File_Pcm*)Streams[moov_trak_tkhd_TrackID].Parsers[0])->Endianness=Endianness?'L':'B';
            if (Streams[moov_trak_tkhd_TrackID].Parsers.size()==2)
            {
                ((File_ChannelGrouping*)Streams[moov_trak_tkhd_TrackID].Parsers[0])->Endianness=Endianness?'L':'B';
                ((File_Pcm*)Streams[moov_trak_tkhd_TrackID].Parsers[1])->Endianness=Endianness?'L':'B';
            }
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_wave_frma()
{
    Element_Name("Data format");

    //Parsing
    int16u Codec_Peek;
    Peek_B2(Codec_Peek);
    if (Codec_Peek==0x6D73) //"ms", Microsoft 2CC
    {
        int16u CodecMS;
        Skip_C2(                                                "Codec_MS");
        Get_B2 (CodecMS,                                        "CC2");

        if (moov_trak_mdia_minf_stbl_stsd_Pos>1)
            return; //Handling only the first description

        FILLING_BEGIN();
            Ztring OldFormat=Retrieve(Stream_Audio, StreamPos_Last, Audio_CodecID);
            Ztring NewFormat=Ztring::ToZtring(CodecMS, 16);
            if (OldFormat!=NewFormat)
            {
                Clear(Stream_Audio, StreamPos_Last, Audio_CodecID);
                CodecID_Fill(NewFormat, Stream_Audio, StreamPos_Last, InfoCodecID_Format_Riff);
            }
            Fill(Stream_Audio, StreamPos_Last, Audio_Codec, CodecMS, 16, true);
            Fill(Stream_Audio, StreamPos_Last, Audio_Codec_CC, CodecMS, 16, true);
        FILLING_END();
    }
    else
    {
        int32u Codec;
        Get_C4(Codec,                                           "Codec");

        if (moov_trak_mdia_minf_stbl_stsd_Pos>1)
            return; //Handling only the first description

        FILLING_BEGIN();
            if (Codec!=0x6D703461) //"mp4a"
            {
                Ztring OldFormat=Retrieve(Stream_Audio, StreamPos_Last, Audio_CodecID);
                Ztring NewFormat=Ztring().From_CC4(Codec);
                if (OldFormat!=NewFormat)
                {
                    Clear(Stream_Audio, StreamPos_Last, Audio_CodecID);
                    CodecID_Fill(NewFormat, Stream_Audio, StreamPos_Last, InfoCodecID_Format_Mpeg4);
                }
            }
            Fill(Stream_Audio, StreamPos_Last, Audio_Codec, Ztring().From_CC4(Codec), true);
            Fill(Stream_Audio, StreamPos_Last, Audio_Codec_CC, Ztring().From_CC4(Codec), true);
        FILLING_END();
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_wave_samr()
{
    Element_Name("AMR decode config");

    //Parsing
    int32u Vendor;
    int8u  Version;
    Get_C4 (Vendor,                                             "Encoder vendor");
    Get_B1 (Version,                                            "Encoder version");
    Skip_XX(Element_Size-Element_Offset,                        "Unknown");

    if (moov_trak_mdia_minf_stbl_stsd_Pos>1)
        return; //Handling only the first description

    Fill(Stream_Audio, StreamPos_Last, Audio_Encoded_Library_Name, Mpeg4_Vendor(Vendor));
    Fill(Stream_Audio, StreamPos_Last, Audio_Encoded_Library_Version, Version);
    Fill(Stream_Audio, StreamPos_Last, Audio_Encoded_Library, Retrieve(Stream_Audio, StreamPos_Last, Audio_Encoded_Library_Name)+__T(' ')+Ztring::ToZtring(Version));
    Ztring Encoded_Library_String=Retrieve(Stream_Audio, StreamPos_Last, Audio_Encoded_Library_Name)+(Version?(__T("Revision")+Ztring::ToZtring(Version)):Ztring());
    Fill(Stream_Audio, StreamPos_Last, Audio_Encoded_Library_String, Encoded_Library_String, true);
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_wave_srcq()
{
    Element_Name("Source Quality");

    //Parsing
    Skip_B4(                                                    "Source Quality");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_wave_xxxx()
{
    if ((Element_Code&0x6D730000)!=0x6D730000)
        return; //Only msxx format is known

    Element_Name("Microsoft Audio");

    //Parsing
    int32u SamplesPerSec, AvgBytesPerSec;
    int16u FormatTag, Channels, BitsPerSample;
    Get_L2 (FormatTag,                                          "FormatTag");
    Get_L2 (Channels,                                           "Channels");
    Get_L4 (SamplesPerSec,                                      "SamplesPerSec");
    Get_L4 (AvgBytesPerSec,                                     "AvgBytesPerSec");
    Skip_L2(                                                    "BlockAlign");
    Get_L2 (BitsPerSample,                                      "BitsPerSample");

    FILLING_BEGIN();
        Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, Channels!=5?Channels:6, 10, true);
        Fill(Stream_Audio, StreamPos_Last, Audio_SamplingRate, SamplesPerSec, 10, true);
        Fill(Stream_Audio, StreamPos_Last, Audio_BitRate_Nominal, AvgBytesPerSec*8, 10, true);
    FILLING_END();

    //Options
    if (Element_Offset+2>Element_Size)
        return; //No options

    //Parsing
    int16u Option_Size;
    Get_L2 (Option_Size,                                        "cbSize");

    //Filling
    if (Option_Size>0)
    {
        Skip_XX(Option_Size,                                    "Unknown");
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsh()
{
    Element_Name("Shadow Sync Sample");

    //Parsing
    int32u entry_count;
    Get_B4 (entry_count,                                        "entry-count");

    for (int32u Pos=0; Pos<entry_count; Pos++)
    {
        Skip_B4(                                                "shadowed-sample-number");
        Skip_B4(                                                "sync-sample-number");
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stss()
{
    NAME_VERSION_FLAG("Sync Sample");

    Streams[moov_trak_tkhd_TrackID].stss.clear();

    //Parsing
    int32u entry_count;
    Get_B4 (entry_count,                                        "entry-count");

    int32u Offset=1; //By default, begin at 1
    bool stss_PreviouslyEmpty=Streams[moov_trak_tkhd_TrackID].stss.empty();
    for (int32u Pos=0; Pos<entry_count; Pos++)
    {
        int32u sample_number;

        //Too much slow
        /*
        Get_B4 (sample_number,                                  "sample-number");
        */

        //Faster
        if (Element_Offset+4>Element_Size)
            break; //Problem
        sample_number=BigEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset);
        Element_Offset+=4;

        //Coherency testing (first frame is 0 or 1)
        if (sample_number==0 && Offset)
        {
            for (size_t Pos=0; Pos<Streams[moov_trak_tkhd_TrackID].stss.size(); Pos++)
                Streams[moov_trak_tkhd_TrackID].stss[Pos]--;
            Offset=0;
        }

        Streams[moov_trak_tkhd_TrackID].stss.push_back(sample_number-Offset);
    }
    if (!stss_PreviouslyEmpty)
        std::sort(Streams[moov_trak_tkhd_TrackID].stss.begin(), Streams[moov_trak_tkhd_TrackID].stss.end());
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsz()
{
    NAME_VERSION_FLAG("Sample Size")

    Stream=Streams.find(moov_trak_tkhd_TrackID);
    int32u Sample_Size, Sample_Count;
    int8u  FieldSize;
    if (Element_Code==Elements::moov_trak_mdia_minf_stbl_stsz)
    {
        Get_B4 (Sample_Size,                                    "Sample Size");
        FieldSize=32;
    }
    else
    {
        Skip_B3(                                                "Reserved");
        Get_B1 (FieldSize,                                      "Field size");
        Sample_Size=0;
    }
    Get_B4 (Sample_Count,                                       "Number of entries");

    if (Sample_Size>0)
    {
        //Detecting wrong stream size with some PCM streams
        if (StreamKind_Last==Stream_Audio)
        {
            const Ztring &Codec=Retrieve(Stream_Audio, StreamPos_Last, Audio_CodecID);
            if (Codec==__T("raw ")
             || MediaInfoLib::Config.CodecID_Get(Stream_Audio, InfoCodecID_Format_Mpeg4, Codec).find(__T("PCM"))==0
             || MediaInfoLib::Config.Codec_Get(Codec, InfoCodec_KindofCodec).find(__T("PCM"))==0)
             {
                int64u Duration=Retrieve(StreamKind_Last, StreamPos_Last, Audio_Duration).To_int64u();
                int64u Resolution=Retrieve(StreamKind_Last, StreamPos_Last, Audio_BitDepth).To_int64u();
                int64u SamplingRate=Retrieve(StreamKind_Last, StreamPos_Last, Audio_SamplingRate).To_int64u();
                int64u Channels=Retrieve(StreamKind_Last, StreamPos_Last, Audio_Channel_s_).To_int64u();
                int64u Stream_Size_Theory=Duration*Resolution*SamplingRate*Channels/8/1000;
                int64u Stream_Size_Real=Sample_Size; Stream_Size_Real*=Sample_Count;
                for (int64u Multiplier=1; Multiplier<=32; Multiplier++)
                    if (Stream_Size_Real*Multiplier>Stream_Size_Theory*0.995 && Stream_Size_Real*Multiplier<Stream_Size_Theory*1.005)
                    {
                        Streams[moov_trak_tkhd_TrackID].stsz_Sample_Multiplier=Multiplier;
                        break;
                    }
             }
        }

        Stream->second.stsz_StreamSize=Sample_Size; Stream->second.stsz_StreamSize*=Sample_Count;
        if (Sample_Size==1)
            Stream->second.stsz_StreamSize*=Streams[moov_trak_tkhd_TrackID].stsz_Sample_Multiplier;
        else
            Stream->second.stsz_Sample_Multiplier=1; //Sampe_Size is correct, no need of stsz_Sample_Multiplier

        Stream->second.stsz_Sample_Size=Sample_Size;
        Stream->second.stsz_Sample_Count=Sample_Count;

        if (Sample_Count>1 && Retrieve(StreamKind_Last, StreamPos_Last, "BitRate_Mode").empty())
            Fill(StreamKind_Last, StreamPos_Last, "BitRate_Mode", "CBR");

    }
    else
    {
        int32u Size;
        /*
        if (FieldSize==4)
            BS_Begin(); //Too much slow
        */
        for (int32u Pos=0; Pos<Sample_Count; Pos++)
        {
            //Too much slow
            /*
            switch(FieldSize)
            {
                case  4 :                Get_S4 (4, Size,       "Size"); break;
                case  8 : {int8u  Size1; Get_B1 (Size1,         "Size"); Size=Size1;} break;
                case 16 : {int16u Size2; Get_B2 (Size2,         "Size"); Size=Size2;} break;
                case 32 : {int32u Size4; Get_B4 (Size4,         "Size"); Size=Size4;} break;
                default : return;
            }
            */

            //Faster
            if (Element_Offset+4>Element_Size)
                break; //Problem
           switch(FieldSize)
            {
                case  4 : if (Sample_Count%2)
                            Size=Buffer[Buffer_Offset+(size_t)Element_Offset]&0x0F;
                          else
                          {
                            Size=Buffer[Buffer_Offset+(size_t)Element_Offset]>>4;
                            Element_Offset++;
                          }
                          break;
               case  8 : Size=BigEndian2int8u (Buffer+Buffer_Offset+(size_t)Element_Offset); Element_Offset++; break;
                case 16 : Size=BigEndian2int16u(Buffer+Buffer_Offset+(size_t)Element_Offset); Element_Offset+=2; break;
                case 32 : Size=BigEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset); Element_Offset+=4; break;
                default : return;
            }

            Stream->second.stsz_StreamSize+=Size;
            Stream->second.stsz_Total.push_back(Size);
            if (Pos<FrameCount_MaxPerStream)
                Stream->second.stsz.push_back(Size);
        }
        /*
        if (FieldSize==4)
            BS_End(); //Too much slow
        */
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stts()
{
    NAME_VERSION_FLAG("Time to Sample");

    //Parsing
    int32u NumberOfEntries;
    Get_B4(NumberOfEntries,                                     "Number of entries");

    Stream=Streams.find(moov_trak_tkhd_TrackID);
    Stream->second.stts_Min=(int32u)-1;
    Stream->second.stts_Max=0;
    Stream->second.stts_FrameCount=0;
    #ifdef MEDIAINFO_DVDIF_ANALYZE_YES
        std::map<int32u, int64u> Duration_FrameCount; //key is duration
        int64u Duration_FrameCount_Max=0;
        int32u Duration_FrameCount_Max_Duration=0;
        if (StreamKind_Last==Stream_Video && Retrieve(Stream_Video, StreamPos_Last, "Format")==__T("DV") && Streams[moov_trak_tkhd_TrackID].Parsers[0] && ((File_DvDif*)Streams[moov_trak_tkhd_TrackID].Parsers[0])->Mpeg4_stts==NULL)
            ((File_DvDif*)Streams[moov_trak_tkhd_TrackID].Parsers[0])->Mpeg4_stts=new File_DvDif::stts;
    #endif //MEDIAINFO_DVDIF_ANALYZE_YES

    #if MEDIAINFO_DEMUX
        Streams[moov_trak_tkhd_TrackID].stts_Durations.clear();
    #endif //MEDIAINFO_DEMUX

    for (int32u Pos=0; Pos<NumberOfEntries; Pos++)
    {
        int32u SampleCount, SampleDuration;
        Get_B4(SampleCount,                                     "Sample Count");
        Get_B4(SampleDuration,                                  "Sample Duration");

        moov_trak_mdia_minf_stbl_stts_Common(SampleCount, SampleDuration, Pos, NumberOfEntries);

        #ifdef MEDIAINFO_DVDIF_ANALYZE_YES
            if (StreamKind_Last==Stream_Video && Retrieve(Stream_Video, StreamPos_Last, "Format")==__T("DV"))
            {
                File_DvDif::stts_part DV_stts_Part;
                DV_stts_Part.Pos_Begin=Stream->second.stts_FrameCount-SampleCount;
                DV_stts_Part.Pos_End=Stream->second.stts_FrameCount;
                DV_stts_Part.Duration=SampleDuration;
                ((File_DvDif*)Streams[moov_trak_tkhd_TrackID].Parsers[0])->Mpeg4_stts->push_back(DV_stts_Part);

                Duration_FrameCount[SampleDuration]+=Stream->second.stts_FrameCount;
                if (Duration_FrameCount_Max<=Duration_FrameCount[SampleDuration])
                {
                    Duration_FrameCount_Max=Duration_FrameCount[SampleDuration];
                    Duration_FrameCount_Max_Duration=SampleDuration;
                }
            }
        #endif //MEDIAINFO_DVDIF_ANALYZE_YES
    }

    FILLING_BEGIN();
        if (StreamKind_Last==Stream_Video)
        {
            Fill(Stream_Video, StreamPos_Last, Video_FrameCount, Stream->second.stts_FrameCount);

            #ifdef MEDIAINFO_DVDIF_ANALYZE_YES
                if (StreamKind_Last==Stream_Video && Retrieve(Stream_Video, StreamPos_Last, "Format")==__T("DV"))
                {
                    //Clean up the "normal" value
                    for (size_t Pos=0; Pos<((File_DvDif*)Streams[moov_trak_tkhd_TrackID].Parsers[0])->Mpeg4_stts->size(); Pos++)
                    {
                        if (((File_DvDif*)Streams[moov_trak_tkhd_TrackID].Parsers[0])->Mpeg4_stts->at(Pos).Duration==Duration_FrameCount_Max_Duration)
                        {
                            ((File_DvDif*)Streams[moov_trak_tkhd_TrackID].Parsers[0])->Mpeg4_stts->erase(((File_DvDif*)Streams[moov_trak_tkhd_TrackID].Parsers[0])->Mpeg4_stts->begin()+Pos);
                            Pos--;
                        }
                    }

                    if (((File_DvDif*)Streams[moov_trak_tkhd_TrackID].Parsers[0])->Mpeg4_stts->empty())
                        {delete ((File_DvDif*)Streams[moov_trak_tkhd_TrackID].Parsers[0])->Mpeg4_stts; ((File_DvDif*)Streams[moov_trak_tkhd_TrackID].Parsers[0])->Mpeg4_stts=NULL;}
                }
            #endif //MEDIAINFO_DVDIF_ANALYZE_YES
        }
    FILLING_END();
}

void File_Mpeg4::moov_trak_mdia_minf_stbl_stts_Common(int32u SampleCount, int32u SampleDuration, int32u Pos, int32u NumberOfEntries)
{
    FILLING_BEGIN();
        stream::stts_struct Stts;
        Stts.SampleCount=SampleCount;
        Stts.SampleDuration=SampleDuration;
        Stream->second.stts.push_back(Stts);
        if (Pos==1 && NumberOfEntries>=2 && NumberOfEntries<=3 && Stream->second.stts_FrameCount==1 && Stts.SampleDuration!=Stream->second.stts_Max && Stream->second.mdhd_TimeScale)
        {
            Stream->second.stts_Duration_FirstFrame=Stream->second.stts[0].SampleDuration;
            Stream->second.stts_Min=Stts.SampleDuration;
            Stream->second.stts_Max=Stts.SampleDuration;
        }
        if (NumberOfEntries>=2 && NumberOfEntries<=3 && Pos+1==NumberOfEntries && Stts.SampleCount==1 && Stream->second.stts_Min==Stream->second.stts_Max && Stts.SampleDuration!=Stream->second.stts_Max && Stream->second.mdhd_TimeScale)
        {
            Stream->second.stts_Duration_LastFrame=Stts.SampleDuration;
        }
        else
        {
            if (Stts.SampleDuration<Stream->second.stts_Min) Stream->second.stts_Min=Stts.SampleDuration;
            if (Stts.SampleDuration>Stream->second.stts_Max) Stream->second.stts_Max=Stts.SampleDuration;
        }
        Stream->second.stts_FrameCount+=Stts.SampleCount;
        if (Stts.SampleDuration<0x80000000)
            Stream->second.stts_Duration+=Stts.SampleCount*Stts.SampleDuration;
        else
            Stream->second.stts_Duration-=Stts.SampleCount*(((int32u)-1)-Stts.SampleDuration+1); //Negative value

        #if MEDIAINFO_DEMUX
            stream::stts_duration stts_Duration;
            stts_Duration.Pos_Begin=Stream->second.stts_FrameCount-Stts.SampleCount;
            stts_Duration.Pos_End=Stream->second.stts_FrameCount;
            stts_Duration.SampleDuration=Stts.SampleDuration;
            if (Streams[moov_trak_tkhd_TrackID].stts_Durations.empty())
                stts_Duration.DTS_Begin=0;
            else
            {
                stream::stts_durations::iterator Previous=Streams[moov_trak_tkhd_TrackID].stts_Durations.end(); --Previous;
                stts_Duration.DTS_Begin=Previous->DTS_End;
            }
            stts_Duration.DTS_End=stts_Duration.DTS_Begin+Stts.SampleCount*Stts.SampleDuration;
            Streams[moov_trak_tkhd_TrackID].stts_Durations.push_back(stts_Duration);
        #endif //MEDIAINFO_DEMUX
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_tapt()
{
    Element_Name("Aperture Mode Dimensions");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_tapt_clef()
{
    NAME_VERSION_FLAG("Clean Aperture Dimensions");

    //Parsing
    Skip_BFP4(16,                                               "cleanApertureWidth");
    Skip_BFP4(16,                                               "cleanApertureHeight");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_tapt_prof()
{
    NAME_VERSION_FLAG("Production Aperture Dimensions");

    //Parsing
    Skip_BFP4(16,                                               "productionApertureWidth");
    Skip_BFP4(16,                                               "productionApertureHeight");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_tapt_enof()
{
    NAME_VERSION_FLAG("Encoded Pixels Dimensions");

    //Parsing
    Skip_BFP4(16,                                               "encodedApertureWidth");
    Skip_BFP4(16,                                               "encodedApertureHeight");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_tkhd()
{
    NAME_VERSION_FLAG("Track Header")

    //Parsing
    Ztring Date_Created, Date_Modified;
    float32 a, b, u, c, d, v, x, y, w;
    int64u Duration;
    int16u Volume;
        Skip_Flags(Flags, 0,                                    "Track Enabled");
        Skip_Flags(Flags, 1,                                    "Track in Movie");
        Skip_Flags(Flags, 2,                                    "Track in Preview");
        Skip_Flags(Flags, 3,                                    "Track in Poster");
    Get_DATE1904_DEPENDOFVERSION(Date_Created,                  "Creation time");
    Get_DATE1904_DEPENDOFVERSION(Date_Modified,                 "Modification time");
    Get_B4 (moov_trak_tkhd_TrackID,                             "Track ID"); Element_Info1(moov_trak_tkhd_TrackID);
    Skip_B4(                                                    "Reserved");
    Get_B_DEPENDOFVERSION(Duration,                             "Duration"); if (TimeScale) {Param_Info2(Duration*1000/TimeScale, " ms"); Element_Info2(Duration*1000/TimeScale, " ms");}
    Skip_B4(                                                    "Reserved");
    Skip_B4(                                                    "Reserved");
    Skip_B2(                                                    "Layer");
    Skip_B2(                                                    "Alternate group");
    Get_B2 (Volume,                                             "Volume"); Param_Info1(Ztring::ToZtring(((float)Volume)/256));
    Skip_B2(                                                    "Reserved");
    Element_Begin1("Matrix structure");
        Get_BFP4(16, a,                                         "a (width scale)");
        Get_BFP4(16, b,                                         "b (width rotate)");
        Get_BFP4( 2, u,                                         "u (width angle)");
        Get_BFP4(16, c,                                         "c (height rotate)");
        Get_BFP4(16, d,                                         "d (height scale)");
        Get_BFP4( 2, v,                                         "v (height angle)");
        Get_BFP4(16, x,                                         "x (position left)");
        Get_BFP4(16, y,                                         "y (position top)");
        Get_BFP4( 2, w,                                         "w (divider)");
    Element_End0();
    Get_BFP4(16, moov_trak_tkhd_Width,                          "Track width");
    Get_BFP4(16, moov_trak_tkhd_Height,                         "Track height");

    FILLING_BEGIN();
        //Case of header is after main part
        std::map<int32u, stream>::iterator Temp=Streams.find((int32u)-1);
        if (Temp!=Streams.end())
        {
            Streams[moov_trak_tkhd_TrackID]=Temp->second;
            Temp->second.Parsers.clear(); //They are a copy, we don't want that the destructor deletes the Parser
            Streams.erase(Temp);
        }

        Fill(StreamKind_Last, StreamPos_Last, "Duration", float64_int64s(((float64)Duration)*1000/TimeScale));
        Fill(StreamKind_Last, StreamPos_Last, "Encoded_Date", Date_Created);
        Fill(StreamKind_Last, StreamPos_Last, "Tagged_Date", Date_Modified);
        Fill(StreamKind_Last, StreamPos_Last, General_ID, moov_trak_tkhd_TrackID, 10, true);
        Streams[moov_trak_tkhd_TrackID].tkhd_Duration=Duration;
        if (moov_trak_tkhd_Height*d)
            moov_trak_tkhd_DisplayAspectRatio=(moov_trak_tkhd_Width*a)/(moov_trak_tkhd_Height*d);
        moov_trak_tkhd_Rotation=(float32)(std::atan2(b, a)*180.0/3.14159);
        if (moov_trak_tkhd_Rotation<0)
            moov_trak_tkhd_Rotation+=360;

        #if MEDIAINFO_EVENTS
            if (StreamIDs_Size>1 && Config->File_ID_OnlyRoot_Get())
                StreamIDs_Width[StreamIDs_Size-1]=Streams.size()<=1?0:8; //If referenced file and more than 1 stream, referenced ID is discarded, else referenced ID is used
        #endif //MEDIAINFO_EVENTS
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_tref()
{
    Element_Name("Track Reference");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_tref_chap()
{
    Element_Name("Chapters List");

    //Parsing
    int32u TrackID;
    while (Element_Offset<Element_Size)
    {
        Get_B4(TrackID,                                         "track-ID");

        FILLING_BEGIN();
            Streams[TrackID].IsChapter=true;
        FILLING_END();
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_tref_clcp()
{
    Element_Name("Closed Caption Track");

    //Parsing
    while (Element_Offset<Element_Size)
        Skip_B4(                                                "track-ID");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_tref_dpnd()
{
    Element_Name("MPEG-4 dependency");

    //Parsing
    while (Element_Offset<Element_Size)
        Skip_B4(                                                "track-ID");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_tref_fall()
{
    Element_Name("Audio Track Reference");

    //Parsing
    while (Element_Offset<Element_Size)
        Skip_B4(                                                "track-ID");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_tref_folw()
{
    Element_Name("Default Track");

    //Parsing
    while (Element_Offset<Element_Size)
        Skip_B4(                                                "track-ID");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_tref_forc()
{
    Element_Name("Forced Subtitle Track");

    //Parsing
    while (Element_Offset<Element_Size)
        Skip_B4(                                                "track-ID");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_tref_hint()
{
    Element_Name("original media");

    //Parsing
    while (Element_Offset<Element_Size)
        Skip_B4(                                                "track-ID");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_tref_ipir()
{
    Element_Name("IPI declarations");

    //Parsing
    while (Element_Offset<Element_Size)
        Skip_B4(                                                "track-ID");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_tref_mpod()
{
    Element_Name("included elementary stream");

    //Parsing
    while (Element_Offset<Element_Size)
        Skip_B4(                                                "track-ID");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_tref_scpt()
{
    Element_Name("transcript");

    //Parsing
    while (Element_Offset<Element_Size)
        Skip_B4(                                                "track-ID");
}


//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_tref_ssrc()
{
    Element_Name("non-primary source (used in other track)");

    //Parsing
    while (Element_Offset<Element_Size)
        Skip_B4(                                                "track-ID");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_tref_sync()
{
    Element_Name("synchronization source");

    //Parsing
    while (Element_Offset<Element_Size)
        Skip_B4(                                                "track-ID");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_tref_tmcd()
{
    Element_Name("TimeCode");

    //Parsing
    int32u TrackID;
    Get_B4(TrackID,                                             "track-ID");

    FILLING_BEGIN();
        Streams[moov_trak_tkhd_TrackID].TimeCode_TrackID=TrackID;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta()
{
    Element_Name("User Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_AllF()
{
    Element_Name("AllF");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_chpl()
{
    Element_Name("Chapters");

    //Parsing
    Ztring Value;
    std::string ValueS;
    int64u Time;
    int8u Size;
    size_t Pos=0;
    Stream_Prepare(Stream_Menu);
    Skip_B8(                                                    "Unknown");
    Skip_B1(                                                    "Chapter Count");
    Fill(Stream_Menu, StreamPos_Last, Menu_Chapters_Pos_Begin, Count_Get(Stream_Menu, StreamPos_Last), 10, true);
    while (Element_Offset<Element_Size)
    {
        Get_B8 (Time,                                           "Time");
        Get_B1 (Size,                                           "Text size");
        Get_String(Size, ValueS,                                "Value");
        Value.From_UTF8(ValueS.c_str());
        if (Value.empty())
            Value.From_Local(ValueS.c_str()); //Trying Local...

        FILLING_BEGIN();
            Fill(Stream_Menu, StreamPos_Last, Ztring().Duration_From_Milliseconds(Time/10000).To_Local().c_str(), Value);
        FILLING_END();

        //Next
        Pos++;
    }
    Fill(Stream_Menu, StreamPos_Last, Menu_Chapters_Pos_End, Count_Get(Stream_Menu, StreamPos_Last), 10, true);
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_clsf()
{
    NAME_VERSION_FLAG("Classification"); //3GP

    //Parsing
    Ztring ClassificationInfo;
    int32u ClassificationEntity;
    int16u Language, ClassificationTable;
    Get_C4(ClassificationEntity,                                "ClassificationEntity");
    Get_C2(ClassificationTable,                                 "ClassificationTable");
    Get_B2(Language,                                            "Language");
    bool Utf8=true;
    if (Element_Offset+2<=Element_Size)
    {
        int16u Utf16;
        Peek_B2(Utf16);
        if (Utf16==0xFEFF)
            Utf8=false;
    }
    if (Utf8)
        Get_UTF8(Element_Size-Element_Offset, ClassificationInfo, "ClassificationInfo");
    else
        Get_UTF16(Element_Size-Element_Offset, ClassificationInfo, "ClassificationInfo");

    FILLING_BEGIN();
        Fill(Stream_General, 0, "Classification", Ztring().From_CC4(ClassificationTable));
        Fill(Stream_General, 0, "Classification_Reason", ClassificationInfo);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_DcMD()
{
    Element_Name("Kodak MetaData");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_DcMD_Cmbo()
{
    Element_Name("Camera byte order");

    //Parsing
    Skip_C2(                                                    "EXIF byte order");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_DcMD_DcME()
{
    Element_Name("DcME?");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_DcMD_DcME_Keyw()
{
    Element_Name("Keywords?");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_DcMD_DcME_Mtmd()
{
    Element_Name("Metadata?");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_DcMD_DcME_Rate()
{
    Element_Name("Rate?");

    //Parsing
    Skip_B2(                                                    "Zero");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_FIEL()
{
    Element_Name("FIEL?");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_FXTC()
{
    Element_Name("Adobe After Effects?");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_hinf()
{
    Element_Name("Hint Format");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_hinv()
{
    Element_Name("Hint Version");

    //Parsing
    Skip_Local(Element_Size,                                    "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_hnti()
{
    Element_Name("Hint Info");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_hnti_rtp()
{
    Element_Name("Real Time");

    //Parsing
    Skip_Local(Element_Size,                                    "Value");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_ID32()
{
    NAME_VERSION_FLAG("ID3v2"); //3GP

    //Parsing
    int16u Language;
    Get_B2(Language,                                            "Language");
    Skip_XX(Element_Size-Element_Offset,                        "ID3v2data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_kywd()
{
    NAME_VERSION_FLAG("Keywords"); //3GP

    //Parsing
    int16u Language;
    int8u KeywordCnt;
    Get_B2(Language,                                            "Language");
    Get_B1(KeywordCnt,                                          "KeywordCnt");
    for (int8u Pos=0; Pos<KeywordCnt; Pos++)
    {
        Ztring KeywordInfo;
        int8u  KeywordSize;
        Get_B1(KeywordSize,                                     "KeywordSize");
        bool Utf8=true;
        if (Element_Offset+2<=Element_Size)
        {
            int16u Utf16;
            Peek_B2(Utf16);
            if (Utf16==0xFEFF)
                Utf8=false;
        }
        if (Utf8)
            Get_UTF8(KeywordSize, KeywordInfo, "KeywordInfo");
        else
            Get_UTF16(KeywordSize, KeywordInfo, "KeywordInfo");

        FILLING_BEGIN();
            Fill(Stream_General, 0, "Keywords", KeywordInfo);
        FILLING_END();
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_loci()
{
    NAME_VERSION_FLAG("Location Information"); //3GP

    //Parsing
    Skip_XX(Element_Size-Element_Offset,                        "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_LOOP()
{
    Element_Name("LOOP");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_MCPS()
{
    Element_Name("Mechanical Copyright Protection Society?");

    //Parsing
    Ztring Encoder;
    Get_Local(Element_Size, Encoder,                            "Value");

    //Filling
    //Fill(Stream_General, 0, General_Encoded_Library, Encoder);
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_meta()
{
    NAME_VERSION_FLAG("Metadata");
    INTEGRITY_VERSION(0);

    //Filling
    moov_meta_hdlr_Type=Elements::moov_udta_meta;
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_meta_hdlr()
{
    moov_meta_hdlr();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_meta_ilst()
{
    moov_meta_ilst();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_meta_ilst_xxxx()
{
    moov_meta_ilst_xxxx();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_meta_ilst_xxxx_data()
{
    moov_meta_ilst_xxxx_data();
}
//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_meta_ilst_xxxx_mean()
{
    moov_meta_ilst_xxxx_mean();
}
//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_meta_ilst_xxxx_name()
{
    moov_meta_ilst_xxxx_name();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_ndrm()
{
    //Parsing
    Skip_XX(Element_Size,                                       "Unknown");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_nsav()
{
    Element_Name("No Save");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_ptv()
{
    Element_Name("Print To Video");

    //Parsing
    Skip_B2(                                                    "Display size");
    Skip_B2(                                                    "Reserved");
    Skip_B2(                                                    "Reserved");
    Skip_B1(                                                    "Slide show");
    Skip_B1(                                                    "Play on open");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_rtng()
{
    NAME_VERSION_FLAG("Rating"); //3GP

    //Parsing
    Ztring RatingInfo;
    int32u RatingEntity, RatingCriteria;
    int16u Language;
    Get_C4(RatingEntity,                                        "RatingEntity");
    Get_C4(RatingCriteria,                                      "RatingCriteria");
    Get_B2(Language,                                            "Language");
    bool Utf8=true;
    if (Element_Offset+2<=Element_Size)
    {
        int16u Utf16;
        Peek_B2(Utf16);
        if (Utf16==0xFEFF)
            Utf8=false;
    }
    if (Utf8)
        Get_UTF8(Element_Size-Element_Offset, RatingInfo, "RatingInfo");
    else
        Get_UTF16(Element_Size-Element_Offset, RatingInfo, "RatingInfo");

    FILLING_BEGIN();
        Fill(Stream_General, 0, General_LawRating, Ztring().From_CC4(RatingCriteria));
        Fill(Stream_General, 0, General_LawRating_Reason, RatingInfo);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_Sel0()
{
    Element_Name("Sel0");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_tags()
{
    Element_Name("Tags");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_tags_meta()
{
    Element_Name("Metadata");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_tags_tseg()
{
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_tags_tseg_tshd()
{
    //Parsing
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_WLOC()
{
    Element_Name("WLOC");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_XMP_()
{
    Element_Name("eXtensible Metadata Platform");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_yrrc()
{
    NAME_VERSION_FLAG("Recording Year"); //3GP

    //Parsing
    int16u RecordingYear;
    Get_B2 (RecordingYear,                                       "RecordingYear");

    FILLING_BEGIN();
        Fill(Stream_General, 0, General_Released_Date, RecordingYear);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_udta_xxxx()
{
    //Getting the method
    std::string Parameter;
    method Method=Metadata_Get(Parameter, Element_Code);
    Element_Info1(Parameter.c_str());

    switch (Method)
    {
        case Method_None :
            {
                Element_Name("Unknown");
            }
            break;
        case Method_Binary :
            {
                Element_Name("Binary");
                Skip_XX(Element_Size,                           "Unknown");
                return;
            }
            break;
        case Method_String :
            {
                Element_Name("Text");

                //Parsing
                Ztring Value;
                int32u Size32=0;
                int16u Size16=0, Language;
                bool IsText=true;
                if (Element_Size<=4)
                    IsText=false;
                else
                {
                    Peek_B4(Size32);
                    if (4+(int64u)Size32>Element_Size)
                    {
                        Size32=0;
                        Peek_B2(Size16);
                        if (4+(int64u)Size16>Element_Size)
                            IsText=false;
                    }
                }
                if (!IsText)
                {
                    Skip_XX(Element_Size,                       "Unknown");
                    return;
                }

                while(Element_Offset<Element_Size)
                {
                    if (Size32)
                    {
                        Get_Local(Size32, Value,                "Value");
                        Get_B4 (Size32,                         "Size");
                    }
                    else
                    {
                        Get_B2 (Size16,                         "Size");
                        Get_B2 (Language,                       "Language"); Param_Info1(Language_Get(Language));
                        Get_Local(Size16, Value,                "Value");
                    }

                    FILLING_BEGIN();
                        if (Retrieve(Stream_General, 0, Parameter.c_str()).empty())
                            Fill(Stream_General, 0, Parameter.c_str(), Value);
                    FILLING_END();

                    if (Element_Offset+1==Element_Size)
                    {
                        int8u Null;
                        Peek_B1(Null);
                        if (Null==0x00)
                            Skip_B1(                            "NULL");
                    }
                    if (Element_Offset+4<=Element_Size && Size32)
                    {
                        int32u Null;
                        Peek_B4(Null);
                        if (Null==0x00000000)
                            Skip_XX(Element_Size-Element_Offset,"Padding");
                    }
                    if (Element_Offset+2<=Element_Size && Size16)
                    {
                        int16u Null;
                        Peek_B2(Null);
                        if (Null==0x0000)
                            Skip_XX(Element_Size-Element_Offset,"Padding");
                    }
                }
            }
            break;
        case Method_String2 :
            {
                NAME_VERSION_FLAG("Text");

                //Parsing
                Ztring Value;
                int16u Language;
                while(Element_Offset<Element_Size)
                {
                    Get_B2(Language,                            "Language"); Param_Info1(Language_Get(Language));
                    bool Utf8=true;
                    if (Element_Offset+2<=Element_Size)
                    {
                        int16u Utf16;
                        Peek_B2(Utf16);
                        if (Utf16==0xFEFF)
                            Utf8=false;
                    }
                    if (Utf8)
                        Get_UTF8(Element_Size-Element_Offset, Value, "Value");
                    else
                        Get_UTF16(Element_Size-Element_Offset, Value, "Value");

                    FILLING_BEGIN();
                       if (Retrieve(Stream_General, 0, Parameter.c_str()).empty())
                            Fill(Stream_General, 0, Parameter.c_str(), Value);
                    FILLING_END();
                }
            }
            break;
        case Method_String3 :
            {
                NAME_VERSION_FLAG("Text");

                //Parsing
                Ztring Value;
                while(Element_Offset<Element_Size)
                {
                    Get_UTF8(Element_Size-Element_Offset, Value,"Value");

                    FILLING_BEGIN();
                       if (Retrieve(Stream_General, 0, Parameter.c_str()).empty())
                            Fill(Stream_General, 0, Parameter.c_str(), Value);
                    FILLING_END();
                }
            }
            break;
        case Method_Integer :
            {
                Element_Name("Integer");

                //Parsing
                Skip_XX(4,                                      "Value");
            }
            break;
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::pdin()
{
    NAME_VERSION_FLAG("Progressive Download Information");

    //Parsing
    while (Element_Offset<Element_Size)
    {
        Skip_B4(                                                "Rate");
        Skip_B4(                                                "Initial Delay");
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::PICT()
{
    Element_Name("QuickDraw picture");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::pckg()
{
    Element_Name("QTCA");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");

    FILLING_BEGIN();
        Accept("QTCA");

        Fill(Stream_General, 0, General_Format, "MPEG-4");
        CodecID_Fill(__T("QTCA"), Stream_General, 0, InfoCodecID_Format_Mpeg4);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::pnot()
{
    Element_Name("Preview");

    //Parsing
    Info_B4(Date_Modified,                                      "Modification date"); Param_Info1(Ztring().Date_From_Seconds_1904(Date_Modified));
    Skip_B2(                                                    "Version number");
    Skip_C4(                                                    "Atom type");
    Skip_B2(                                                    "Atom index");
}

//---------------------------------------------------------------------------
void File_Mpeg4::RDAO()
{
    Element_Name("Audio O?");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::RDAS()
{
    Element_Name("Audio S?");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::RDVO()
{
    Element_Name("Video O?");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::RDVS()
{
    Element_Name("Video S?");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::RED1()
{
    Element_Name("RED Header");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");

    FILLING_BEGIN();
        Accept("R3D");

        Fill(Stream_General, 0, General_Format, "R3D");
        //Source: http://peter.schlaile.de/redcode/

        Finish();
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::REDA()
{
    Element_Name("RED Audio");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");

    Finish();
}

//---------------------------------------------------------------------------
void File_Mpeg4::REDV()
{
    Element_Name("RED Video");

    //Parsing
    /*
    if (Streams[1].Parser==0)
    {
        Streams[1].Parser=new File_Mpeg4();
        Open_Buffer_Init(Streams[1].Parser);
    }
    */

    //Parsing
    Skip_B4(                                                    "Unknown");
    Skip_B4(                                                    "Unknown");
    Skip_B4(                                                    "Unknown");
    Skip_B4(                                                    "Unknown");
    Skip_C4(                                                    "CodecID");
    Skip_B4(                                                    "Unknown");
    //Open_Buffer_Continue(Streams[1].Parser);
    Skip_XX(Element_Size-Element_Offset,                        "Data");

    Finish();
}

//---------------------------------------------------------------------------
void File_Mpeg4::REOB()
{
    Element_Name("OB?");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::skip()
{
    Element_Name("Skip");

    //Parsing
    Skip_XX(Element_Size,                                       "Free");
}

//---------------------------------------------------------------------------
void File_Mpeg4::wide()
{
    Element_Name("Wide");

    //Parsing
    Skip_XX(Element_Size,                                       "Free");
}

//***************************************************************************
// C++
//***************************************************************************

} //NameSpace

#endif //MEDIAINFO_MPEG4_YES
