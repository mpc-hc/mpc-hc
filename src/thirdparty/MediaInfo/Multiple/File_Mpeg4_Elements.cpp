// File_Mpeg4 - Info for MPEG-4 files
// Copyright (C) 2005-2010 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR .  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// Elements part
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
#ifdef MEDIAINFO_MPEG4_YES
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Mpeg4.h"
#if defined(MEDIAINFO_DVDIF_YES)
    #include "MediaInfo/Multiple/File_DvDif.h"
#endif
#if defined(MEDIAINFO_AVC_YES)
    #include "MediaInfo/Video/File_Avc.h"
#endif
#if defined(MEDIAINFO_MPEGV_YES)
    #include "MediaInfo/Video/File_Mpegv.h"
#endif
#if defined(MEDIAINFO_VC1_YES)
    #include "MediaInfo/Video/File_Vc1.h"
#endif
#if defined(MEDIAINFO_AC3_YES)
    #include "MediaInfo/Audio/File_Ac3.h"
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
#if defined(MEDIAINFO_JPEG_YES)
    #include "MediaInfo/Image/File_Jpeg.h"
#endif
#include "MediaInfo/Multiple/File_Mpeg4_TimeCode.h"
#include "ZenLib/Base64/base64.h"
#include <cmath>
#include <zlib.h>
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
        case 0x46464D50 : return _T("FFMpeg");
        case 0x4D4F544F : return _T("Motorola");
        case 0x50484C50 : return _T("Philips");
        case 0x6170706C : return _T("Apple");
        case 0x6E6F6B69 : return _T("Nokia");
        case 0x6D6F746F : return _T("Motorola");
        default         : return Ztring().From_CC4(Vendor);
    }
}

//---------------------------------------------------------------------------
const char* Mpeg4_chan(int16u Ordering)
{
    //TODO: http://developer.apple.com/mac/library/documentation/MusicAudio/Reference/CAFSpec/CAF_spec/CAF_spec.html#//apple_ref/doc/uid/TP40001862-CH210-BCGECJAJ
    switch(Ordering)
    {
        case 100 : return "Front: C";
        case 101 : return "Front: L R";
        case 102 : return "Front: L R"; //With headphones
        case 103 : return "Front: L R"; //With matrix
        case 104 : return "Front: MidSide";
        case 105 : return "Front: XY";
        case 106 : return "Front: Binaural";
        case 107 : return "Front: WXYZ";
        case 108 : return "Front: L R, Side: L R";
        case 109 : return "Front: L C R, Rear: L R";
        case 110 : return "Front: L C R, Rear: L C R";
        case 111 : return "Front: L C R, Side: L R, Rear: L C R";
        case 112 : return "Front: L R, TopFront: L R, Rear: L R, TopRear: L R";
		default  : return "";
    }
}

//---------------------------------------------------------------------------
std::string Mpeg4_chan_ChannelDescription (int32u ChannelLabels)
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
        Text+=", Side:";
    if (ChannelLabels&0x0400)
        Text+=" L";
    if (ChannelLabels&0x0800)
        Text+=" R";

    if ((ChannelLabels&0x0260)!=0x0000)
        Text+=", Back:";
    if (ChannelLabels&0x0020)
        Text+=" L";
    if (ChannelLabels&0x0200)
        Text+=" C";
    if (ChannelLabels&0x0040)
        Text+=" R";

    if ((ChannelLabels&0x0010)!=0x0000)
        Text+=", LFE";

    return Text;
}

//***************************************************************************
// Constants
//***************************************************************************

//---------------------------------------------------------------------------
namespace Elements
{
    const int64u free=0x66726565;
    const int64u ftyp=0x66747970;
    const int64u ftyp_qt=0x71742020;
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
    const int64u moov_trak_mdia_hdlr_MPEG=0x4D504547;
    const int64u moov_trak_mdia_hdlr_sbtl=0x7362746C;
    const int64u moov_trak_mdia_hdlr_soun=0x736F756E;
    const int64u moov_trak_mdia_hdlr_subp=0x73756270;
    const int64u moov_trak_mdia_hdlr_text=0x74657874;
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
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_avcC=0x61766343;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_bitr=0x62697472;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_btrt=0x62747274;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_clap=0x636C6170;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_chan=0x6368616E;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_colr=0x636F6C72;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_d263=0x64323633;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_dac3=0x64616333;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_dec3=0x64656333;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_damr=0x64616D72;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_esds=0x65736473;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_idfm=0x6964666D;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_idfm_atom=0x61746F6D;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_idfm_qtat=0x71746174;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_idfm_fxat=0x66786174;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_idfm_priv=0x70726976;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_idfm_subs=0x73756273;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_idfm_cspc=0x63737063;
    const int64u moov_trak_mdia_minf_stbl_stsd_xxxx_pasp=0x70617370;
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
    const int64u moov_trak_mdia_minf_vmhd=0x766D6864;
    const int64u moov_trak_tapt=0x74617074;
    const int64u moov_trak_tapt_clef=0x636C6566;
    const int64u moov_trak_tapt_enof=0x656E6F66;
    const int64u moov_trak_tapt_prof=0x70726F66;
    const int64u moov_trak_tkhd=0x746B6864;
    const int64u moov_trak_tref=0x74726566;
    const int64u moov_trak_tref_dpnd=0x64706E64;
    const int64u moov_trak_tref_ipir=0x69706972;
    const int64u moov_trak_tref_hint=0x68696E74;
    const int64u moov_trak_tref_mpod=0x6D706F64;
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
    const int64u PICT=0x50494354;
    const int64u pckg=0x70636B67;
    const int64u pnot=0x706E6F74;
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
    LIST_SKIP(free)
    ATOM(ftyp)
    ATOM(idat)
    ATOM(idsc)
    ATOM(jp2c)
    LIST(jp2h)
        ATOM_BEGIN
        ATOM(jp2h_ihdr)
        ATOM(jp2h_colr)
        ATOM_END
    LIST(mdat)
        ATOM_BEGIN
        ATOM_DEFAULT(mdat_xxxx)
        ATOM_END_DEFAULT
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
                ATOM_BEGIN
                LIST_DEFAULT (moov_meta_ilst_xxxx)
                    ATOM_BEGIN
                    ATOM (moov_meta_ilst_xxxx_data)
                    ATOM (moov_meta_ilst_xxxx_mean)
                    ATOM (moov_meta_ilst_xxxx_name)
                    ATOM_END
                ATOM_END_DEFAULT
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
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_avcC)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_bitr)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_btrt)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_chan)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_clap)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_colr)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_d263)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_dac3)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_dec3)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_damr)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_esds)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_idfm)
                                ATOM(moov_trak_mdia_minf_stbl_stsd_xxxx_pasp)
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
                        ATOM_END
                    ATOM(moov_trak_mdia_minf_vmhd)
                    ATOM_END
                ATOM_END
            LIST(moov_trak_tapt)
                ATOM_BEGIN
                ATOM(moov_trak_tapt_clef)
                ATOM(moov_trak_tapt_prof)
                ATOM(moov_trak_tapt_enof)
                ATOM_END
            ATOM(moov_trak_tkhd)
            LIST(moov_trak_tref)
                ATOM_BEGIN
                ATOM(moov_trak_tref_dpnd)
                ATOM(moov_trak_tref_ipir)
                ATOM(moov_trak_tref_hint)
                ATOM(moov_trak_tref_mpod)
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
                    ATOM_BEGIN
                    LIST_DEFAULT (moov_udta_meta_ilst_xxxx);
                        ATOM_BEGIN
                        ATOM (moov_udta_meta_ilst_xxxx_data);
                        ATOM (moov_udta_meta_ilst_xxxx_mean);
                        ATOM (moov_udta_meta_ilst_xxxx_name);
                        ATOM_END
                    ATOM_END_DEFAULT
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
    ATOM(PICT)
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
        Param_Info(_INFO); \
    } \

//---------------------------------------------------------------------------
void File_Mpeg4::free()
{
    Element_Name("Free space");

    //Parsing
    Skip_XX(Element_TotalSize_Get(),                            "Free");
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
    int32u MajorBrand;
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
        CodecID_Fill(_T("QTI"), Stream_General, 0, InfoCodecID_Format_Mpeg4);
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
        CodecID_Fill(_T("QTI"), Stream_General, 0, InfoCodecID_Format_Mpeg4);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::jp2c()
{
    Element_Name("JPEG 2000 content");

    #if defined(MEDIAINFO_JPEG_YES)
        //Creating the parser
        File_Jpeg MI;
        Open_Buffer_Init(&MI);

        //Parsing
        Open_Buffer_Continue(&MI);

        //Filling
        Finish(&MI);

        Accept("MPEG-4");

        Fill(Stream_General, 0, General_Format, "JPEG 2000", Unlimited, true, true);
        Fill(Stream_General, 0, General_Format_Profile, "MPEG-4");

        Merge(MI);
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
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::jp2h_ihdr()
{
    Element_Name("Header");

    //Parsing
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::mdat()
{
    #if MEDIAINFO_TRACE
        Trace_Levels.reset(); Trace_Levels.set(8); //Stream
    #endif //MEDIAINFO_TRACE

    if (!Status[IsAccepted])
    {
        Data_Accept("MPEG-4");

        Fill(Stream_General, 0, General_Format, "QuickTime");
    }
    Element_Name("Data");

    //In case of second pass
    if (mdat_MustParse && mdat_Pos.empty())
    {
        //For each stream
        for (std::map<int32u, stream>::iterator Temp=Stream.begin(); Temp!=Stream.end(); Temp++)
        {
            if (Temp->second.Parser)
            {
                //Adding it
                size_t SamplesPerChunk_Pos=0;
                int32u SamplesPerChunk=0;
                size_t Chunk=0;
                size_t stsc_Pos=0; //Sample to Chunk
                int64u Position=(int64u)-1;
                size_t stsz_Sample_Size;
                if (Temp->second.stsz_Sample_Size)
                    stsz_Sample_Size=(size_t)Temp->second.stsz_Sample_Size; //If stsz is not filled, this is a fixed sample size //TODO: handle 64-bit values on 32-bit OS
                else
                    stsz_Sample_Size=Temp->second.stsz.size();
                for (size_t stsz_Pos=0; stsz_Pos<stsz_Sample_Size; stsz_Pos++) //Sample Size
                {
                    //Changing stco/stsc if needed
                    if (SamplesPerChunk_Pos>=SamplesPerChunk)
                    {
                        //Reseting
                        SamplesPerChunk_Pos=0;
                        Chunk++;
                        if (Chunk>Temp->second.stco.size())
                            break;

                        //Count of sample in this Chunk
                        if (stsc_Pos+1<Temp->second.stsc.size() && Chunk>=Temp->second.stsc[stsc_Pos+1].FirstChunk || Position==(int64u)-1)
                        {
                            if (Position!=(int64u)-1)
                            {
                                stsc_Pos++;
                                if (stsc_Pos>=Temp->second.stsc.size())
                                    break;
                            }
                            SamplesPerChunk=Temp->second.stsc[stsc_Pos].SamplesPerChunk;
                        }

                        //Chunk Offset
                        Position=Temp->second.stco[Chunk-1];
                    }

                    mdat_Pos[Position].StreamID=Temp->first;
                    mdat_Pos[Position].Size=Temp->second.stsz_Sample_Size?Temp->second.stsz_Sample_Size:Temp->second.stsz[stsz_Pos];

                    Position+=Temp->second.stsz_Sample_Size?Temp->second.stsz_Sample_Size:Temp->second.stsz[stsz_Pos];
                    SamplesPerChunk_Pos++;
                }
            }
        }
    }

    //Trace
    #if MEDIAINFO_TRACE
        Trace_Levels.set(); //All
    #endif //MEDIAINFO_TRACE

    if (!mdat_Pos.empty() && mdat_Pos.begin()->first<File_Offset+Buffer_Offset+Element_TotalSize_Get())
    {
        //Next piece of data
        IsParsing_mdat=true;
        mdat_StreamJump();
        return; //Only if have something in this mdat
    }

    //In case of mdat is before moov
    if (Stream.empty())
    {
        Buffer_Offset-=(size_t)Header_Size;
        Element_Level--;
        BookMark_Set(); //Remembering this place, for stream parsing in phase 2
        Element_Level++;
        Buffer_Offset+=(size_t)Header_Size;
    }

    //Parsing
    Skip_XX(Element_TotalSize_Get(),                            "Data");
}

//---------------------------------------------------------------------------
void File_Mpeg4::mdat_xxxx()
{
    if (!Element_IsComplete_Get())
    {
        Element_WaitForMoreData();
        return;
    }

    Demux(Buffer+Buffer_Offset, (size_t)Element_Size, ContentType_MainStream);

    if (Stream[(int32u)Element_Code].Parser)
    {
        Open_Buffer_Continue(Stream[(int32u)Element_Code].Parser);
        Element_Offset=Element_Size;
        Element_Show();
    }
    else
        Skip_XX(Element_Size,                                   "Data");

    //Erasing Index if no more needed
    mdat_StreamClear();
    
    //Next piece of data
    mdat_StreamJump();
}

//---------------------------------------------------------------------------
void File_Mpeg4::mdat_StreamClear()
{
    if (Stream[(int32u)Element_Code].Parser==NULL || Stream[(int32u)Element_Code].Parser->Status[IsFinished])
    {
        std::map<int64u, mdat_Pos_Type>::iterator Temp=mdat_Pos.begin();
        while (Temp!=mdat_Pos.end())
        {
            std::map<int64u, mdat_Pos_Type>::iterator Stream_Pos_Temp=Temp;
            bool Useful=true;
            if (Temp->second.StreamID==(int32u)Element_Code)
                Useful=false;

            Temp++;

            if (!Useful)
                mdat_Pos.erase(Stream_Pos_Temp);
        }
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::mdat_StreamJump()
{
    int64u ToJump=File_Size;
    if (!mdat_Pos.empty())
        ToJump=mdat_Pos.begin()->first;
    if (ToJump>File_Size)
        ToJump=File_Size;
    if (ToJump>=File_Offset+Buffer_Offset+Element_TotalSize_Get(Element_Level-1)) //We want always Element mdat
    {
        if (!Status[IsAccepted])
            Data_Accept("MPEG-4");
        Data_GoTo(File_Offset+Buffer_Offset+Element_TotalSize_Get(Element_Level-1), "MPEG-4"); //Not in this chunk
        IsParsing_mdat=false;
    }
    else if (ToJump!=File_Offset+Buffer_Offset+Element_Size)
    {
        if (!Status[IsAccepted])
            Data_Accept("MPEG-4");
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
        Element_Begin("entry");
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
        Element_End();
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::moof()
{
    Element_Name("Movie Fragment");
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
}

//---------------------------------------------------------------------------
void File_Mpeg4::moof_traf_sdtp()
{
    NAME_VERSION_FLAG("Independent and Disposable Samples");

    //Parsing
    //for (int32u Pos=0; Pos<sample_count; Pos++) //sample_count should be taken from stsz or stz2
    while (Element_Offset<Element_Size)
    {
        Element_Begin("sample");
        BS_Begin();
        Skip_S1(2,                                              "reserved");
        Info_S1(2, sample_depends_on,                           "sample_depends_on"); Param_Info(Mpeg4_sample_depends_on[sample_depends_on]);
        Info_S1(2, sample_is_depended_on,                       "sample_is_depended_on"); Param_Info(Mpeg4_sample_is_depended_on[sample_is_depended_on]);
        Info_S1(2, sample_has_redundancy,                       "sample_has_redundancy"); Param_Info(Mpeg4_sample_has_redundancy[sample_has_redundancy]);
        BS_End();
        Element_End();
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
    Skip_B4(                                                    "track_ID");
    if (base_data_offset_present)
        Skip_B8(                                                "base_data_offset");
    if (sample_description_index_present)
        Skip_B4(                                                "sample_description_index");
    if (default_sample_duration_present)
        Skip_B4(                                                "default_sample_duration");
    if (default_sample_size_present)
        Skip_B4(                                                "default_sample_size");
    if (default_sample_flags_present)
        Skip_B4(                                                "default_sample_flags");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moof_traf_trun()
{
    NAME_VERSION_FLAG("Track Fragment Run");

    //Parsing
    int32u sample_count;
    bool data_offset_present, first_sample_flags_present, sample_duration_present, sample_size_present, sample_flags_present, sample_composition_time_offset_present;
        Get_Flags (Flags,  0, data_offset_present,              "data-offset-present");
        Get_Flags (Flags,  2, first_sample_flags_present,       "first-sample-flags-present");
        Get_Flags (Flags,  8, sample_duration_present,          "sample-duration-present");
        Get_Flags (Flags,  9, sample_size_present,              "sample-size-present");
        Get_Flags (Flags, 10, sample_flags_present,             "sample-flags-present");
        Get_Flags (Flags, 11, sample_composition_time_offset_present, "sample-composition-time-offsets-present");
    Get_B4 (sample_count,                                       "sample_count");
    if (data_offset_present)
        Skip_B4(                                                "data_offset");
    if (first_sample_flags_present)
        Skip_B4(                                                "first_sample_flags");
    for (int32u Pos=0; Pos<sample_count; Pos++)
    {
        Element_Begin("sample");
        if (sample_duration_present)
            Skip_B4(                                            "sample_duration");
        if (sample_size_present)
            Skip_B4(                                            "sample_size");
        if (sample_flags_present)
            Skip_B4(                                            "sample_flags");
        if (sample_composition_time_offset_present)
            Skip_B4(                                            "sample_composition_time_offset");
        Element_End();
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

    if (!Stream.empty())
    {
        Skip_XX(Element_TotalSize_Get(),                        "Duplicated moov");
        return;
    }
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
        Skip_XX(Element_Size,                                   "Will be parsed");

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

        //Configuring level
        std::vector<int64u> Element_Sizes_Sav;
        size_t Element_Level_Sav=Element_Level;
        while(Element_Level)
        {
            int64u A=Element_TotalSize_Get();
            Element_Sizes_Sav.push_back(A);
            Element_End();
        }

        //Configuring file size
        int64u File_Size_Sav=File_Size;
        if (File_Size<File_Offset+Buffer_Offset+Element_Offset+Dest_Size)
            File_Size=File_Offset+Buffer_Offset+Element_Offset+Dest_Size;
        Element_Level++;
        Header_Fill_Size(File_Size);
        Element_Level--;

        //Parsing
        Open_Buffer_Continue(Dest, Dest_Size);
        delete[] Dest; //Dest=NULL;

        //Resetting file size
        File_Size=File_Size_Sav;
        while(Element_Level)
            Element_End();
        Element_Level++;
        Header_Fill_Size(File_Size);
        Element_Level--;

        //Configuring level
        while(Element_Level<Element_Level_Sav)
        {
            Element_Begin();
            Element_Begin();
            Header_Fill_Size(Element_Sizes_Sav[0]);
            Element_End();
            //Ztring(), Element_Sizes_Sav[Element_Level_Sav-1-Element_Level]);
        }

        //Resetting buffer
        Buffer=Buffer_Sav;
        Buffer_Size=Buffer_Size_Sav;
        Buffer_Temp=Buffer_Temp_Sav;
        Buffer_Temp_Size=Buffer_Temp_Size_Sav;
        Buffer_Offset=Buffer_Offset_Sav;
        Buffer_Offset_Temp=Buffer_Offset_Temp_Sav;

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
    Skip_C4(                                                    "Manufacturer");
    Skip_B4(                                                    "Component reserved flags"); //Filled if Quicktime
    Skip_B4(                                                    "Component reserved flags mask"); //Filled if Quicktime
    Skip_Local(Element_Size-Element_Offset,                     "Component type name");
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
    Get_B4(Kind,                                                  "Kind"); Param_Info(Mpeg4_Meta_Kind(Kind));
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
                            Fill(Stream_General, 0, General_Part_Position, Position, 10, true);
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
                            Fill(Stream_General, 0, General_Track_Position, Position, 10, true);
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
                            Skip_XX(Element_Size,               "To decode!");
                            Value=_T("(Binary)");
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
                    Value=_T("UTF-16 encoding not yet supported");
                    break;
        case 0x03 : //Mac String
                    Get_B4(Language,                            "Language"); //To confirm
                    Get_Local(Element_Size-Element_Offset, Value, "Value");
                    break;
        case 0x0D : //JPEG
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
                            Value=_T("(Binary)");
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
                        default  : Value=_T("Unknown kind of integer value!");
                    }
                    }
                    break;
        case 0x16 : //Float 32
                    Skip_XX(4,                                  "To decode!");
                    //Value=_T("Float32 encoding not yet supported");
                    break;
        case 0x17 : //Float 64
                    Skip_XX(8,                                  "To decode!");
                    Trusted=1000;
                    //Value=_T("Float64 encoding not yet supported");
                    break;
        default: Value=_T("Unknown kind of value!");
   }

   FILLING_BEGIN();
        switch (moov_meta_hdlr_Type)
        {
            case Elements::moov_meta_hdlr_mdir :
                {
                    std::string Parameter;
                    if (Element_Code_Get(Element_Level-1)==Elements::moov_meta______)
                        Metadata_Get(Parameter, moov_meta_ilst_xxxx_name_Name);
                    else
                        Metadata_Get(Parameter, Element_Code_Get(Element_Level-1));
                    if (!Parameter.empty())
                    {
                        Element_Info(Parameter.c_str());
                        Fill(Stream_General, 0, Parameter.c_str(), Value, true);
                    }
                }
                break;
            case Elements::moov_meta_hdlr_mdta :
                if(!moov_udta_meta_keys_List.empty())
                {
                    std::string Parameter;
                    Metadata_Get(Parameter, moov_udta_meta_keys_List[moov_udta_meta_keys_ilst_Pos<moov_udta_meta_keys_List.size()?moov_udta_meta_keys_ilst_Pos:moov_udta_meta_keys_List.size()-1]);
                    if (Parameter=="com.apple.quicktime.version")
                        Vendor_Version=Value.SubString(_T(""), _T(" "));
                    else if (Parameter=="com.apple.quicktime.player.version")
                        Vendor_Version=Value.SubString(_T(""), _T(" "));
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
                    moov_udta_meta_keys_ilst_Pos++;
                }
                else
                    Param("Keys atom is missing!", 0);

            case Elements::moov_udta_meta :
                {
                    std::string Parameter;
                    Metadata_Get(Parameter, (int32u)Element_Code_Get(Element_Level-1));
                    if (!Parameter.empty())
                        Fill(Stream_General, 0, Parameter.c_str(), Value, true);
                }
            default: ;
        }
   FILLING_END();
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
    Skip_B4(                                                    "track_ID");
    Skip_B4(                                                    "default_sample_description_index");
    Skip_B4(                                                    "default_sample_duration");
    Skip_B4(                                                    "default_sample_size");
    Element_Begin("default_sample_flags", 4);
        BS_Begin();
        Skip_S1(6,                                              "reserved");
        Skip_S1(2,                                              "sample_depends_on");
        Skip_S1(2,                                              "sample_is_depended_on");
        Skip_S1(2,                                              "sample_has_redundancy");
        Skip_S1(3,                                              "sample_padding_value");
        Skip_SB(                                                "sample_is_difference_sample");
        BS_End();
        Skip_B2(                                                "sample_degradation_priority");
    Element_End();
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
    Get_B4(TimeScale,                                           "Time scale"); Param_Info(Ztring::ToZtring(TimeScale)+_T(" Hz"));
    Get_B_DEPENDOFVERSION(Duration,                             "Duration"); Param_Info(Ztring::ToZtring((int64u)Duration*1000/TimeScale)+_T(" ms"));
    Get_B4 (Rate,                                               "Preferred rate"); Param_Info(Ztring::ToZtring(((float32)Rate)/0x10000));
    Get_B2 (Volume,                                             "Preferred volume"); Param_Info(Ztring::ToZtring(((float32)Volume)/0x100));
    Skip_XX(10,                                                 "Reserved");
    Element_Begin("Matrix structure", 36);
        Get_BFP4(16, a,                                         "a (width scale)");
        Get_BFP4(16, b,                                         "b (width rotate)");
        Get_BFP4( 2, u,                                         "u (width angle)");
        Get_BFP4(16, c,                                         "c (height rotate)");
        Get_BFP4(16, d,                                         "d (height scale)");
        Get_BFP4( 2, v,                                         "v (height angle)");
        Get_BFP4(16, x,                                         "x (position left)");
        Get_BFP4(16, y,                                         "y (position top)");
        Get_BFP4( 2, w,                                         "w (divider)");
    Element_End();
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
        if (Date_Created.find(_T('\r'))!=std::string::npos)
            Date_Created.resize(Date_Created.find(_T('\r')));
        if (Date_Created.find(_T('\n'))!=std::string::npos)
            Date_Created.resize(Date_Created.find(_T('\n')));
        Fill(StreamKind_Last, StreamPos_Last, "Encoded_Date", Date_Created);
        if (Date_Modified.find(_T('\r'))!=std::string::npos)
            Date_Modified.resize(Date_Modified.find(_T('\r')));
        if (Date_Modified.find(_T('\n'))!=std::string::npos)
            Date_Modified.resize(Date_Modified.find(_T('\n')));
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
    int32u Count, Duration, Time;
    bool NoMoreEmpty=false;
    Get_B4 (Count,                                              "Number of entries");
    for (int32u Pos=0; Pos<Count; Pos++)
    {
        Element_Begin("Entry", 12);
        Get_B4 (Duration,                                       "Track duration"); Param_Info((int64u)Duration*1000/TimeScale, " ms");
        Get_B4 (Time,                                           "Media time"); if (Time!=(int32u)-1) {Param_Info((int64u)Time*1000/TimeScale, " ms");}
        if (Time==(int32u)-1 && !NoMoreEmpty)
        {
            Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Delay), (int64u)Duration*1000/TimeScale);
            Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Delay_Source), "Container");
        }
        if (Time!=(int32u)-1)
            NoMoreEmpty=true;
        Info_B4(MediaRate,                                      "Media rate"); Param_Info(((float)MediaRate)/0x10000);
        Element_End();
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_load()
{
    Element_Name("Preload");

    //Parsing
    Info_B4(PreloadTime,                                        "Preload time"); Param_Info(PreloadTime*1000/TimeScale, " ms");
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

    FILLING_BEGIN();
        moov_trak_mdia_mdhd_Duration=0;
    FILLING_END();
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
        if (Title.find(_T("Handler"))!=std::string::npos || Title.find(_T("vide"))!=std::string::npos || Title.find(_T("soun"))!=std::string::npos)
            Title.clear(); //This is not a Title
    }

    FILLING_BEGIN();
        if (!Title.empty()) Fill(StreamKind_Last, StreamPos_Last, "Title",    Title);
        switch (SubType)
        {
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
                    Stream_Prepare(Stream_Text);
                    if (SubType!=Elements::moov_trak_mdia_hdlr_text)
                        Fill(Stream_Text, StreamPos_Last, Text_MuxingMode, Ztring().From_CC4(SubType));
                }
                break;
            case Elements::moov_trak_mdia_hdlr_subp :
                if (StreamKind_Last!=Stream_Text)
                {
                    Stream_Prepare(Stream_Text);
                    CodecID_Fill(_T("subp"), Stream_Text, StreamPos_Last, InfoCodecID_Format_Mpeg4);
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

        Stream[moov_trak_tkhd_TrackID].StreamKind=StreamKind_Last;
        Stream[moov_trak_tkhd_TrackID].StreamPos=StreamPos_Last;
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
    Info_B4(TypeModifierName,                                   "Type modifier name"); Param_Info(Mpeg4_TypeModifierName(TypeModifierName));
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
    int16u Language;
    Get_DATE1904_DEPENDOFVERSION(Date_Created,                  "Creation time");
    Get_DATE1904_DEPENDOFVERSION(Date_Modified,                 "Modification time");
    Get_B4(moov_trak_mdia_mdhd_TimeScale,                       "Time scale");
    Get_B_DEPENDOFVERSION(Duration,                             "Duration");
    Get_B2 (Language,                                           "Language"); Param_Info(Language_Get(Language));
    Skip_B2(                                                    "Quality");

    FILLING_BEGIN();
        Fill(StreamKind_Last, StreamPos_Last, "Language", Language_Get(Language));
        if (moov_trak_mdia_mdhd_TimeScale)
        {
            moov_trak_mdia_mdhd_Duration=(int32u)(((float)Duration)/moov_trak_mdia_mdhd_TimeScale*1000);
            Fill(StreamKind_Last, StreamPos_Last, "Duration", moov_trak_mdia_mdhd_Duration);
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
        Stream[moov_trak_tkhd_TrackID].File_Name=Path;
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
    Element_Begin("Mac OS Alias Record");
    Skip_B4(                                                    "user type name/app creator code");
    Get_B2 (record_size,                                        "record size");
    Get_B2 (record_version,                                     "record version");
    if (record_version!=2)
    {
        Skip_XX(Element_Size-Element_Offset,                    "unknown");
        return;
    }
    int64u End=Element_Offset-8+record_size;
    Get_B2 (alias_kind,                                         "alias kind"); Param_Info(alias_kind?"directory":"file");
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
    Element_End(record_size);

    if (Element_Offset<Element_Size)
        Skip_XX(Element_Size-Element_Offset,                    "Padding");

    FILLING_BEGIN();
        if (Stream[moov_trak_tkhd_TrackID].File_Name.empty()) //Priority to "code" version
        {
            if (!Directory_Name.empty())
            {
                Stream[moov_trak_tkhd_TrackID].File_Name=Directory_Name;
                Stream[moov_trak_tkhd_TrackID].File_Name+=ZenLib::PathSeparator;
            }
            Stream[moov_trak_tkhd_TrackID].File_Name+=file_name_string;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_dinf_dref_rsrc()
{
    Element_Name("Ressource alias");

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

    //Filling
    Stream_Prepare(Stream_Menu);
    Fill(Stream_Menu, StreamPos_Last, Menu_Format, "TimeCode");
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
    Skip_BFP4(16,                                               "Text size");
    Skip_B2(                                                    "Text color (red)");
    Skip_B2(                                                    "Text color (green)");
    Skip_B2(                                                    "Text color (blue)");
    Skip_B2(                                                    "Background color (red)");
    Skip_B2(                                                    "Background color (green)");
    Skip_B2(                                                    "Background color (blue)");
    Get_B1 (FontNameSize,                                       "Font name size");
    Skip_Local(FontNameSize,                                    "Font name");

    FILLING_BEGIN();
        Stream[moov_trak_tkhd_TrackID].TimeCode_IsVisual=IsVisual;
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
            Stream_Prepare(Stream_Video);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl()
{
    Element_Name("Sample Table");

    FILLING_BEGIN();
        Buffer_MaximumSize=16*1024*1024; //If we are here, this is really a MPEG-4 file, and some atoms are very bigs...
    FILLING_END();
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

        if (Pos<300 || MediaInfoLib::Config.ParseSpeed_Get()==1.00)
            Stream[moov_trak_tkhd_TrackID].stco.push_back(Offset);
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
    int32u entry_count, sample_count, sample_offset;
    Get_B4 (entry_count,                                        "entry_count");
    for (int32u Pos=0; Pos<entry_count; Pos++)
    {
        //Too much slow
        /*
        Get_B4 (sample_count,                                   "sample_count");
        Get_B4 (sample_offset,                                  "sample_offset");
        */

        //Faster
        if (Element_Offset+8>Element_Size)
            break; //Problem
        sample_count =BigEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset  );
        sample_offset=BigEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset+4);
        Element_Offset+=8;
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

        if (Pos<300 || MediaInfoLib::Config.ParseSpeed_Get()==1.00)
            Stream[moov_trak_tkhd_TrackID].stco.push_back(Offset);
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
    Element_Name("Partial Sync Sample");

    //Parsing
    Skip_XX(Element_Size,                                       "Unknown");
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
        Element_Begin("Entry", 12);
        int32u SampleDescriptionId;
        Get_B4 (Stsc.FirstChunk,                                "First chunk");
        Get_B4 (Stsc.SamplesPerChunk,                           "Samples per chunk");
        Get_B4 (SampleDescriptionId,                            "Sample description ID");
        Element_Info(Stsc.FirstChunk);
        Element_Info(Stsc.SamplesPerChunk);
        Element_Info(SampleDescriptionId);
        Element_End();
        */

        //Faster
        if (Pos<300 || MediaInfoLib::Config.ParseSpeed_Get()==1.00)
        {
            if (Element_Offset+12>Element_Size)
                break; //Problem
            Stsc.FirstChunk     =BigEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset  );
            Stsc.SamplesPerChunk=BigEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset+4);
            Element_Offset+=12;

            Stream[moov_trak_tkhd_TrackID].stsc.push_back(Stsc);
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
    Element_Begin("Default text box");
        Skip_B2(                                                "top");
        Skip_B2(                                                "left");
        Skip_B2(                                                "bottom");
        Skip_B2(                                                "right");
    Element_End();
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
        CodecID_Fill(_T("text"), Stream_Text, StreamPos_Last, InfoCodecID_Format_Mpeg4);
        Fill(StreamKind_Last, StreamPos_Last, Text_Codec, "text", Unlimited, true, true);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_tmcd()
{
    Element_Name("TimeCode");

    //Parsing
    int32u TimeScale, FrameDuration;
    bool DropFrame, H24, NegativeTimes;
    Skip_B4(                                                    "Reserved");
    Skip_B2(                                                    "Reserved");
    Skip_B2(                                                    "Data reference index");
    Skip_B4(                                                    "Reserved (Flags)");
    Info_B4(TimeCodeFlags,                                      "Flags (timecode)");
        Get_Flags (TimeCodeFlags, 0, DropFrame,                 "Drop frame");
        Get_Flags (TimeCodeFlags, 1, H24,                       "24 hour max ");
        Get_Flags (TimeCodeFlags, 2, NegativeTimes,             "Negative times OK");
        Skip_Flags(TimeCodeFlags, 3,                            "Counter");
    Get_B4 (TimeScale,                                          "Time scale");
    Get_B4 (FrameDuration,                                      "Frame duration");
    Skip_B1(                                                    "Number of frames");
    Skip_B1(                                                    "Unknown");

    FILLING_BEGIN();
        //Bug in one file
        if (TimeScale==25 && FrameDuration==100)
            TimeScale=2500;

        //Is it general or for a specific stream?
        bool IsGeneral=true;
        for (std::map<int32u, stream>::iterator Strea=Stream.begin(); Strea!=Stream.end(); Strea++)
            if (Strea->second.TimeCode_TrackID==moov_trak_tkhd_TrackID)
                IsGeneral=false;

        //For each track in the file (but only the last one will be used!)
        for (std::map<int32u, stream>::iterator Strea=Stream.begin(); Strea!=Stream.end(); Strea++)
            if (IsGeneral && Strea->second.StreamKind!=Stream_Max || Strea->second.TimeCode_TrackID==moov_trak_tkhd_TrackID)
            {
                if (Strea->second.StreamKind==Stream_Video)
                {
                    Fill(Stream_Video, Strea->second.StreamPos, Video_Delay_Settings, Ztring(_T("DropFrame="))+(DropFrame?_T("Yes"):_T("No")));
                    Fill(Stream_Video, Strea->second.StreamPos, Video_Delay_Settings, Ztring(_T("24HourMax="))+(H24?_T("Yes"):_T("No")));
                    Fill(Stream_Video, Strea->second.StreamPos, Video_Delay_Settings, Ztring(_T("IsVisual="))+(Stream[moov_trak_tkhd_TrackID].TimeCode_IsVisual?_T("Yes"):_T("No")));
                }
                if (Stream[moov_trak_tkhd_TrackID].Parser==NULL)
                {
                    Stream[moov_trak_tkhd_TrackID].Parser=new File_Mpeg4_TimeCode;
                    Open_Buffer_Init(Stream[moov_trak_tkhd_TrackID].Parser);
                    mdat_MustParse=true; //Data is in MDAT
                    ((File_Mpeg4_TimeCode*)Stream[moov_trak_tkhd_TrackID].Parser)->StreamKind=IsGeneral?Stream_General:Strea->second.StreamKind;
                    ((File_Mpeg4_TimeCode*)Stream[moov_trak_tkhd_TrackID].Parser)->FrameRate=FrameDuration?(((float64)TimeScale)/FrameDuration):0;
                    ((File_Mpeg4_TimeCode*)Stream[moov_trak_tkhd_TrackID].Parser)->NegativeTimes=NegativeTimes;
                    if (!IsGeneral)
                    {
                        Stream[moov_trak_tkhd_TrackID].StreamKind=Strea->second.StreamKind;
                        Stream[moov_trak_tkhd_TrackID].StreamPos=Strea->second.StreamPos;
                    }
                }
            }

        //General stuff
        if (IsGeneral)
            Stream[moov_trak_tkhd_TrackID].StreamKind=Stream_General;
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
    Get_B2(Language,                                            "Language"); Param_Info(Language_Get(Language));
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
        Fill(Stream_General, 0, General_OriginalSourceMedium, Value);
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
    Element_Begin("default-text-box");
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
    Element_End();
    Element_Begin("default-style");
        Skip_B2(                                                "startChar");
        Skip_B2(                                                "endChar");
        Skip_B2(                                                "font-ID");
        Skip_B1(                                                "face-style-flags");
        Skip_B1(                                                "font-size");
        Skip_B1(                                                "text-color-rgba (red)");
        Skip_B1(                                                "text-color-rgba (green)");
        Skip_B1(                                                "text-color-rgba (blue)");
        Skip_B1(                                                "text-color-rgba (alpha)");
    Element_End();

    FILLING_BEGIN();
        CodecID_Fill(_T("tx3g"), Stream_Text, StreamPos_Last, InfoCodecID_Format_Mpeg4);
        Fill(StreamKind_Last, StreamPos_Last, Text_Codec, "tx3g", Unlimited, true, true);
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
    switch (StreamKind_Last)
    {
        case Stream_Audio : moov_trak_mdia_minf_stbl_stsd_xxxxSound(); break;
        case Stream_Video : moov_trak_mdia_minf_stbl_stsd_xxxxVideo(); break;
        default : Skip_XX(Element_TotalSize_Get(),              "Unknown");
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxxSound()
{
    Element_Name("Audio");

    int16u Version, Channels, SampleSize, ID, SampleRate;
    Skip_B4(                                                    "Reserved");
    Skip_B2(                                                    "Reserved");
    Skip_B2(                                                    "Data reference index");
    Get_B2 (Version,                                            "Version");
    Skip_B2(                                                    "Revision level");
    Skip_C4(                                                    "Vendor");
    Get_B2 (Channels,                                           "Number of channels");
    Get_B2 (SampleSize,                                         "Sample size");
    Get_B2 (ID,                                                 "Compression ID");
    Skip_B2(                                                    "Packet size");
    Get_B2 (SampleRate,                                         "Sample rate"); Param_Info(SampleRate, " Hz");
    Skip_B2(                                                    "Reserved");
    if (Version>=1)
    {
        Skip_B4(                                                "Samples per packet");
        Skip_B4(                                                "Bytes per packet");
        Skip_B4(                                                "Bytes per frame");
        Skip_B4(                                                "Bytes per sample");
        if (Version>=2)
        {
            Skip_B4(                                            "Unknown");
            Skip_B4(                                            "Unknown");
            Skip_B4(                                            "Unknown");
            Skip_B4(                                            "Unknown");
            Skip_B4(                                            "Unknown");
        }
    }

    FILLING_BEGIN();
        int32u SampleRate32=SampleRate;    

        //samr bug viewed in some files: channels and Sampling rate are wrong
        if (Element_Code==0x73616D72) //"samr"
        {
            SampleRate32=8000;
            Channels=1;
        }

        //lpcm puts "1" in the SampleRate field  and Timescale is the real sample size
        if (Element_Code==0x6C70636D && SampleRate32==1) //"lpcm"
        {
            SampleRate32=moov_trak_mdia_mdhd_TimeScale;
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
        #if defined(MEDIAINFO_AMR_YES)
        if (MediaInfoLib::Config.CodecID_Get(Stream_Audio, InfoCodecID_Format_Mpeg4, Ztring(Codec.c_str()), InfoCodecID_Format)==_T("AMR"))
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
        if (MediaInfoLib::Config.CodecID_Get(Stream_Audio, InfoCodecID_Format_Mpeg4, Ztring(Codec.c_str()), InfoCodecID_Format)==_T("ADPCM"))
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
        if (MediaInfoLib::Config.CodecID_Get(Stream_Audio, InfoCodecID_Format_Mpeg4, Ztring(Codec.c_str()), InfoCodecID_Format)==_T("PCM"))
        {
            //Creating the parser
            File_Pcm MI;
            MI.Codec=Ztring().From_Local(Codec.c_str());
            Open_Buffer_Init(&MI);

            //Parsing
            Open_Buffer_Continue(&MI, 0);

            //Filling
            Finish(&MI);
            Merge(MI, StreamKind_Last, 0, StreamPos_Last);
        }
        #endif
        #if defined(MEDIAINFO_MPEGA_YES)
        if (MediaInfoLib::Config.CodecID_Get(Stream_Audio, InfoCodecID_Format_Mpeg4, Ztring(Codec.c_str()), InfoCodecID_Format)==_T("MPEG Audio"))
        {
            //Creating the parser
            Stream[moov_trak_tkhd_TrackID].Parser=new File_Mpega;
            Open_Buffer_Init(Stream[moov_trak_tkhd_TrackID].Parser);
            mdat_MustParse=true; //Data is in MDAT
        }
        #endif
        if (Element_Code==0x6F776D61) //"owma"
        {
            Skip_XX(Element_Size-Element_Offset,                "WMA Pro data");
        }

        Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, Channels, 10, true);
        if (SampleSize!=0 && Element_Code!=0x6D703461 && (Element_Code&0xFFFF0000)!=0x6D730000 && Retrieve(Stream_Audio, StreamPos_Last, Audio_Resolution).empty()) //if not mp4a, and not ms*
            Fill(Stream_Audio, StreamPos_Last, Audio_Resolution, SampleSize, 10, true);
        Fill(Stream_Audio, StreamPos_Last, Audio_SamplingRate, SampleRate32);

        //Sometimes, more Atoms in this atoms
        if (Element_Offset+8<Element_Size)
            Element_ThisIsAList();
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxxVideo()
{
    Element_Name("Video");

    int16u Width, Height, ColorTableID;
    int8u  CompressorName_Size;
    Skip_B4(                                                    "Reserved");
    Skip_B2(                                                    "Reserved");
    Skip_B2(                                                    "Data reference index");
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
    Skip_B2(                                                    "Depth");
    Get_B2 (ColorTableID,                                       "Color table ID");
    if (ColorTableID==0)
        Skip_XX(32,                                             "Color Table");

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
        if (Codec=="enca")
            Fill(Stream_Video, StreamPos_Last, Video_Encryption, "Encrypted");
        Fill(Stream_Video, StreamPos_Last, Video_Width, Width, 10, true);
        Fill(Stream_Video, StreamPos_Last, Video_Height, Height, 10, true);
        if (moov_trak_tkhd_DisplayAspectRatio && moov_trak_tkhd_DisplayAspectRatio!=((float32)Width)/Height)
        {
            Fill(Stream_Video, StreamPos_Last, Video_DisplayAspectRatio, moov_trak_tkhd_DisplayAspectRatio, 3, true);
            Fill(Stream_Video, StreamPos_Last, Video_PixelAspectRatio, moov_trak_tkhd_DisplayAspectRatio/Width*Height, 3, true);
        }
        Fill(Stream_Video, StreamPos_Last, Video_Rotation, moov_trak_tkhd_Rotation, 3);
        if (moov_trak_tkhd_Rotation)
            Fill(Stream_Video, StreamPos_Last, Video_Rotation_String, Ztring::ToZtring(moov_trak_tkhd_Rotation, 0)+_T("\xB0")); //degree sign

        //Specific cases
        if (Stream[moov_trak_tkhd_TrackID].Parser==NULL)
        {
            #if defined(MEDIAINFO_DVDIF_YES)
                if (MediaInfoLib::Config.CodecID_Get(Stream_Video, InfoCodecID_Format_Mpeg4, Ztring(Codec.c_str()), InfoCodecID_Format)==_T("DV"))
                {
                    Stream[moov_trak_tkhd_TrackID].Parser=new File_DvDif;
                    Open_Buffer_Init(Stream[moov_trak_tkhd_TrackID].Parser);
                    mdat_MustParse=true; //Data is in MDAT
                }
            #endif
            #if defined(MEDIAINFO_MPEGV_YES)
                if (MediaInfoLib::Config.CodecID_Get(Stream_Video, InfoCodecID_Format_Mpeg4, Ztring().From_CC4((int32u)Element_Code), InfoCodecID_Format)==_T("MPEG Video"))
                {
                    Stream[moov_trak_tkhd_TrackID].Parser=new File_Mpegv;
                    ((File_Mpegv*)Stream[moov_trak_tkhd_TrackID].Parser)->FrameIsAlwaysComplete=true;
                    Open_Buffer_Init(Stream[moov_trak_tkhd_TrackID].Parser);
                    mdat_MustParse=true; //Data is in MDAT
                }
            #endif
            #if defined(MEDIAINFO_VC1_YES)
                if (MediaInfoLib::Config.CodecID_Get(Stream_Video, InfoCodecID_Format_Mpeg4, Ztring().From_CC4((int32u)Element_Code), InfoCodecID_Format)==_T("VC-1"))
                {
                    File_Vc1 MI;
                    MI.FrameIsAlwaysComplete=true;
                    Open_Buffer_Init(&MI);
                    Open_Buffer_Continue(&MI);
                    Element_Offset=Element_Size;
                    Finish(&MI);
                    Merge(MI, Stream_Video, 0, StreamPos_Last);
                }
            #endif
        }

        //Descriptors or a list (we can see both!)
        if (Element_Offset+8<=Element_Size
             && (CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+0)>='A' && CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+0)<='z' || CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+0)>='0' && CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+0)<='9')
             && (CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+1)>='A' && CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+1)<='z' || CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+1)>='0' && CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+1)<='9')
             && (CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+2)>='A' && CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+2)<='z' || CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+2)>='0' && CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+2)<='9')
             && (CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+3)>='A' && CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+3)<='z' || CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+3)>='0' && CC1(Buffer+Buffer_Offset+(size_t)Element_Offset+4+3)<='9'))
                Element_ThisIsAList();
        else if (Element_Offset<Element_Size)
            Descriptors();
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_alac()
{
    Element_Name("ALAC");

    //Parsing
    Skip_B4(                                                    "Reserved");
    Skip_B2(                                                    "Reserved");
    Skip_B2(                                                    "Count");
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_avcC()
{
    Element_Name("AVC decode");

    //Parsing
    int8u Version;
    Get_B1 (Version,                                            "Version");
    if (Version==1)
    {
        #ifdef MEDIAINFO_AVC_YES
            if (Stream[moov_trak_tkhd_TrackID].Parser==NULL)
            {
                Stream[moov_trak_tkhd_TrackID].Parser=new File_Avc;
                #if MEDIAINFO_DEMUX
                    Element_Code=moov_trak_tkhd_TrackID;
                #endif //MEDIAINFO_DEMUX
                Open_Buffer_Init(Stream[moov_trak_tkhd_TrackID].Parser);
                ((File_Avc*)Stream[moov_trak_tkhd_TrackID].Parser)->MustParse_SPS_PPS=true;
                Stream[moov_trak_tkhd_TrackID].Parser->MustSynchronize=false;
                mdat_MustParse=true; //Data is in MDAT

                //Parsing
                Demux(Buffer+(size_t)(Buffer_Offset+Element_Offset), (size_t)(Element_Size-Element_Offset), ContentType_Header);
                Open_Buffer_Continue(Stream[moov_trak_tkhd_TrackID].Parser);

                ((File_Avc*)Stream[moov_trak_tkhd_TrackID].Parser)->SizedBlocks=true;  //Now this is SizeBlocks
            }
            else
            {
                //We parse it, but we don't save information: how to do? Multiple formats in one track.
                File_Avc* MI=new File_Avc;
                Open_Buffer_Init(MI);
                MI->MustParse_SPS_PPS=true;
                MI->MustSynchronize=false;

                //Parsing
                Open_Buffer_Continue(MI);

                //Cleanup
                delete MI; //MI=NULL
            }
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
    //From http://developer.apple.com/mac/library/documentation/MusicAudio/Reference/CAFSpec/CAF_spec/CAF_spec.html#//apple_ref/doc/uid/TP40001862-CH210-DontLinkElementID_25
    int32u ChannelLayoutTag, ChannelBitmap, NumberChannelDescriptions, ChannelLabels=0;
    bool ChannelLabels_Valid=true;
    Get_B4 (ChannelLayoutTag,                                   "ChannelLayoutTag");
    Get_B4 (ChannelBitmap,                                      "ChannelBitmap");
    Get_B4 (NumberChannelDescriptions,                          "NumberChannelDescriptions");
    for (int32u Pos=0; Pos<NumberChannelDescriptions; Pos++)
    {
        int32u ChannelLabel;
        Get_B4 (ChannelLabel,                                   "ChannelLabel");
        if (ChannelLabel<32)
            ChannelLabels|=(1<<ChannelLabel);
        else
            ChannelLabels_Valid=false;
        Skip_B4(                                                "ChannelFlags");
        Skip_BF4(                                               "Coordinates (0)");
        Skip_BF4(                                               "Coordinates (1)");
        Skip_BF4(                                               "Coordinates (2)");
    }

    FILLING_BEGIN();
        if (ChannelLayoutTag==0) //AudioChannelDescriptions
        {
            Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, NumberChannelDescriptions, 10, true);
            if (ChannelLabels_Valid)
                Fill(Stream_Audio, StreamPos_Last, Audio_ChannelPositions, Mpeg4_chan_ChannelDescription(ChannelLabels), true, true);
        }
        else if (ChannelLayoutTag==0x10000) //kCAFChannelLayoutTag_UseChannelBitmap
        {
            int16u Channels=ChannelLayoutTag&0x0000FFFF;
            int16u Ordering=(ChannelLayoutTag&0xFFFF0000)>16;
            Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, Channels, 10, true);
            Fill(Stream_Audio, StreamPos_Last, Audio_ChannelPositions, Mpeg4_chan(Ordering), Unlimited, true, true);
        }
        else
        {
            int8u Channels=0;
            for (size_t Bit=0; Bit<18; Bit++)
                if (ChannelBitmap&(1<<Bit))
                    Channels++;
            if (Channels)
            {
                Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, Channels);
            }
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

    FILLING_BEGIN();
        Stream[moov_trak_tkhd_TrackID].CleanAperture_Width=((float)apertureWidth_N)/apertureWidth_D;
        Stream[moov_trak_tkhd_TrackID].CleanAperture_Height=((float)apertureHeight_N)/apertureHeight_D;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_colr()
{
    Element_Name("Color Parameter");

    //Parsing
    Skip_C4(                                                    "Color parameter type");
    Skip_B2(                                                    "Primaries index");
    Skip_B2(                                                    "Transfer function index");
    Skip_B2(                                                    "Matrix index");
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

    Ztring ProfileLevel;
    switch (H263_Profile)
    {
        case 0x00 : ProfileLevel=_T("BaseLine"); break;
        default   : ProfileLevel.From_Number(H263_Profile);
    }
    ProfileLevel+=_T('@');
    ProfileLevel+=Ztring::ToZtring(((float32)H263_Level)/10, 1);
    Fill(Stream_Video, StreamPos_Last, Video_Format_Profile, ProfileLevel);
    Fill(Stream_Video, StreamPos_Last, Video_Encoded_Library_Name, Mpeg4_Vendor(Vendor));
    Fill(Stream_Video, StreamPos_Last, Video_Encoded_Library_Version, Version);
    Fill(Stream_Video, StreamPos_Last, Video_Encoded_Library, Retrieve(Stream_Video, StreamPos_Last, Video_Encoded_Library_Name)+_T(' ')+Ztring::ToZtring(Version));
    Ztring Encoded_Library_String=Retrieve(Stream_Video, StreamPos_Last, Video_Encoded_Library_Name)+(Version?(_T(" Revision ")+Ztring::ToZtring(Version)):Ztring());
    Fill(Stream_Video, StreamPos_Last, Video_Encoded_Library_String, Encoded_Library_String, true);
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_dac3()
{
    Element_Name("AC-3");
    Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, "", Unlimited, true, true); //Remove the value (is always wrong in the stsd atom)

    //Parsing
    if (Retrieve(Stream_Audio, StreamPos_Last, Audio_CodecID)==_T("sac3"))
    {
        Element_Info("Nero specific");
        int8u Version;
        Get_B1 (Version,                                        "Version");
        if (Version==1)
        {
            int8u bsid;
            Get_B1 (bsid,                                       "bsid");
            Skip_XX(Element_Size-Element_Offset,                "unknown");
            #ifdef MEDIAINFO_AC3_YES
                if (Stream[moov_trak_tkhd_TrackID].Parser==NULL)
                {
                    Stream[moov_trak_tkhd_TrackID].Parser=new File_Ac3;
                    Open_Buffer_Init(Stream[moov_trak_tkhd_TrackID].Parser);
                    ((File_Ac3*)Stream[moov_trak_tkhd_TrackID].Parser)->Frame_Count_Valid=2;
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

    #ifdef MEDIAINFO_AC3_YES
        if (Stream[moov_trak_tkhd_TrackID].Parser==NULL)
        {
            Stream[moov_trak_tkhd_TrackID].Parser=new File_Ac3;
            Open_Buffer_Init(Stream[moov_trak_tkhd_TrackID].Parser);
            ((File_Ac3*)Stream[moov_trak_tkhd_TrackID].Parser)->Frame_Count_Valid=2;
            ((File_Ac3*)Stream[moov_trak_tkhd_TrackID].Parser)->MustParse_dac3=true;
            mdat_MustParse=true; //Data is in MDAT

            //Parsing
            Open_Buffer_Continue(Stream[moov_trak_tkhd_TrackID].Parser);
        }
    #else
        Skip_XX(Element_Size,                                   "AC-3 Data");

        Fill(Stream_Audio, StreamKind_Last, Audio_Format, "AC-3");
    #endif
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_dec3()
{
    Element_Name("E-AC-3");
    Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, "", Unlimited, true, true); //Remove the value (is always wrong in the stsd atom)

    #ifdef MEDIAINFO_AC3_YES
        if (Stream[moov_trak_tkhd_TrackID].Parser==NULL)
        {
            Stream[moov_trak_tkhd_TrackID].Parser=new File_Ac3;
            Open_Buffer_Init(Stream[moov_trak_tkhd_TrackID].Parser);
            ((File_Ac3*)Stream[moov_trak_tkhd_TrackID].Parser)->Frame_Count_Valid=2;
            ((File_Ac3*)Stream[moov_trak_tkhd_TrackID].Parser)->MustParse_dec3=true;
            mdat_MustParse=true; //Data is in MDAT

            //Parsing
            Open_Buffer_Continue(Stream[moov_trak_tkhd_TrackID].Parser);
        }
    #else
        Skip_XX(Element_Size,                                   "E-AC-3 Data");

        Fill(Stream_Audio, StreamKind_Last, Audio_Format, "E-AC-3");
        Fill(Stream_Audio, StreamKind_Last, Audio_Format, "", Unlimited, true, true); //Remove the value (is always wrong in the stsd atom)
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

    Fill(Stream_Audio, StreamPos_Last, Audio_Encoded_Library_Name, Mpeg4_Vendor(Vendor));
    Fill(Stream_Audio, StreamPos_Last, Audio_Encoded_Library_Version, Version);
    Fill(Stream_Audio, StreamPos_Last, Audio_Encoded_Library, Retrieve(Stream_Audio, StreamPos_Last, Audio_Encoded_Library_Name)+_T(' ')+Ztring::ToZtring(Version));
    Ztring Encoded_Library_String=Retrieve(Stream_Audio, StreamPos_Last, Audio_Encoded_Library_Name)+(Version?(_T(" Revision ")+Ztring::ToZtring(Version)):Ztring());
    Fill(Stream_Audio, StreamPos_Last, Audio_Encoded_Library_String, Encoded_Library_String, true);
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_esds()
{
    NAME_VERSION_FLAG("ES Descriptor");
    INTEGRITY_VERSION(0);

    FILLING_BEGIN();
        Descriptors();
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_idfm()
{
    Element_Name("Description");

    Info_C4(Description,                                        "Description"); Param_Info(Mpeg4_Description(Description));
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsd_xxxx_pasp()
{
    Element_Name("Pixel Aspect Ratio");

    //Parsing
    int32u hSpacing, vSpacing;
    Get_B4 (hSpacing,                                           "hSpacing");
    Get_B4 (vSpacing,                                           "vSpacing");

    FILLING_BEGIN();
        if (vSpacing)
        {
            float32 PixelAspectRatio=(float32)hSpacing/vSpacing;
            Clear(Stream_Video, StreamPos_Last, Video_DisplayAspectRatio);
            Fill(Stream_Video, StreamPos_Last, Video_PixelAspectRatio, PixelAspectRatio, 3, true);
            Stream[moov_trak_tkhd_TrackID].CleanAperture_PixelAspectRatio=PixelAspectRatio; //This is the PAR of the clean aperture
        }
    FILLING_END();
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
    Skip_B2(                                                    "Unknown");
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

        FILLING_BEGIN();
            CodecID_Fill(Ztring::ToZtring(CodecMS, 16), Stream_Audio, StreamPos_Last, InfoCodecID_Format_Riff);
            Fill(Stream_Audio, StreamPos_Last, Audio_Codec, CodecMS, 16, true);
            Fill(Stream_Audio, StreamPos_Last, Audio_Codec_CC, CodecMS, 16, true);
        FILLING_END();
    }
    else
    {
        int32u Codec;
        Get_C4(Codec,                                           "Codec");

        FILLING_BEGIN();
            if (Codec!=0x6D703461) //"mp4a"
                CodecID_Fill(Ztring().From_CC4(Codec), Stream_Audio, StreamPos_Last, InfoCodecID_Format_Mpeg4);
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

    Fill(Stream_Audio, StreamPos_Last, Audio_Encoded_Library_Name, Mpeg4_Vendor(Vendor));
    Fill(Stream_Audio, StreamPos_Last, Audio_Encoded_Library_Version, Version);
    Fill(Stream_Audio, StreamPos_Last, Audio_Encoded_Library, Retrieve(Stream_Audio, StreamPos_Last, Audio_Encoded_Library_Name)+_T(' ')+Ztring::ToZtring(Version));
    Ztring Encoded_Library_String=Retrieve(Stream_Audio, StreamPos_Last, Audio_Encoded_Library_Name)+(Version?(_T("Revision")+Ztring::ToZtring(Version)):Ztring());
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

    //Parsing
    int32u entry_count;
    Get_B4 (entry_count,                                        "entry-count");

    for (int32u Pos=0; Pos<entry_count; Pos++)
    {
        Skip_B4(                                                "sample-number");
    }
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stsz()
{
    NAME_VERSION_FLAG("Sample Size")

    int32u Sample_Size, Sample_Count;
    Get_B4 (Sample_Size,                                        "Sample Size");
    Get_B4 (Sample_Count,                                       "Number of entries");

    int64u Stream_Size=0;

    if (Sample_Size>0)
    {
        Stream_Size=Sample_Size; Stream_Size*=Sample_Count;

        Stream[moov_trak_tkhd_TrackID].stsz_Sample_Size=Sample_Size;
        Stream[moov_trak_tkhd_TrackID].stsz_Sample_Count=Sample_Count;
        
        if (Sample_Count>1 && Retrieve(StreamKind_Last, StreamPos_Last, "BitRate_Mode").empty())
            Fill(StreamKind_Last, StreamPos_Last, "BitRate_Mode", "CBR");

        //Detecting wrong stream size with some PCM streams
        if (StreamKind_Last==Stream_Audio)
        {
            const Ztring &Codec=Retrieve(Stream_Audio, StreamPos_Last, Audio_CodecID);
            if (Codec==_T("raw ")
             || MediaInfoLib::Config.Codec_Get(Codec, InfoCodec_KindofCodec).find(_T("PCM"))==0)
             {
                int64u Duration=Retrieve(StreamKind_Last, StreamPos_Last, Audio_Duration).To_int64u();
                int64u Resolution=Retrieve(StreamKind_Last, StreamPos_Last, Audio_Resolution).To_int64u();
                int64u SamplingRate=Retrieve(StreamKind_Last, StreamPos_Last, Audio_SamplingRate).To_int64u();
                int64u Channels=Retrieve(StreamKind_Last, StreamPos_Last, Audio_Channel_s_).To_int64u();
                int64u Stream_Size_Theory=Duration*Resolution*SamplingRate*Channels/8/1000;
                for (int64u Multiplier=1; Multiplier<=32; Multiplier++)
                    if (Stream_Size*Multiplier>Stream_Size_Theory*0.995 && Stream_Size*Multiplier<Stream_Size_Theory*1.005)
                    {
                        Stream_Size*=Multiplier;
                        break;
                    }
             }
        }
    }
    else
    {
        int32u Size;
        int32u Size_Min=(int32u)-1, Size_Max=0;
        for (int32u Pos=0; Pos<Sample_Count; Pos++)
        {
            //Too much slow
            /*
            Get_B4 (Size,                                     "Size");
            */

            //Faster
            if (Element_Offset+4>Element_Size)
                break; //Problem
            Size=BigEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset);
            Element_Offset+=4;

            Stream_Size+=Size;
            if (Size<Size_Min)
                Size_Min=Size;
            if (Size>Size_Max)
                Size_Max=Size;
            if (Pos<300 || MediaInfoLib::Config.ParseSpeed_Get()==1.00)
                Stream[moov_trak_tkhd_TrackID].stsz.push_back(Size);
        }

        if (Retrieve(StreamKind_Last, StreamPos_Last, "BitRate_Mode").empty())
        {
            if (Size_Min*(1.005+0.005)<Size_Max)
                Fill(StreamKind_Last, StreamPos_Last, "BitRate_Mode", "VBR");
            else
                Fill(StreamKind_Last, StreamPos_Last, "BitRate_Mode", "CBR");
        }
    }

    FILLING_BEGIN();
        if (Stream_Size>0)
            Fill(StreamKind_Last, StreamPos_Last, "StreamSize", Stream_Size);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_mdia_minf_stbl_stts()
{
    NAME_VERSION_FLAG("Time to Sample");

    //Parsing
    int32u NumberOfEntries;
    Get_B4(NumberOfEntries,                                     "Number of entries");

    int32u Min=(int32u)-1;
    int32u Max=0;
    int64u FrameCount=0;
    #ifdef MEDIAINFO_DVDIF_ANALYZE_YES
        std::map<int32u, int64u> Duration_FrameCount; //key is duration 
        int64u Duration_FrameCount_Max=0;
        int32u Duration_FrameCount_Max_Duration=0;
        if (StreamKind_Last==Stream_Video && Retrieve(Stream_Video, StreamPos_Last, "Format")==_T("DV") && Stream[moov_trak_tkhd_TrackID].Parser && ((File_DvDif*)Stream[moov_trak_tkhd_TrackID].Parser)->Mpeg4_stts==NULL)
            ((File_DvDif*)Stream[moov_trak_tkhd_TrackID].Parser)->Mpeg4_stts=new File_DvDif::stts;
    #endif //MEDIAINFO_DVDIF_ANALYZE_YES

    for (int32u Pos=0; Pos<NumberOfEntries; Pos++)
    {
        int32u SampleCount, SampleDuration;
        Get_B4(SampleCount,                                     "Sample Count");
        Get_B4(SampleDuration,                                  "Sample Duration");

        FILLING_BEGIN();
            FrameCount+=SampleCount;
            if (NumberOfEntries==2 && Pos+1==NumberOfEntries && FrameCount && SampleCount==1 && Min==Max && SampleDuration!=Max)
            {
                if ((float32)SampleDuration-Min>0)
                    Clear(Stream_Video, StreamPos_Last, Video_Duration);
                if (moov_trak_mdia_mdhd_TimeScale)
                    Fill(Stream_Video, StreamPos_Last, Video_Duration_LastFrame, ((float32)SampleDuration-Min)*1000/moov_trak_mdia_mdhd_TimeScale, 0); //The duration of the frame minus 1 normal frame duration
            }
            else
            {
                if (SampleDuration<Min) Min=SampleDuration;
                if (SampleDuration>Max) Max=SampleDuration;
            }
            #ifdef MEDIAINFO_DVDIF_ANALYZE_YES
                if (StreamKind_Last==Stream_Video && Retrieve(Stream_Video, StreamPos_Last, "Format")==_T("DV"))
                {
                    File_DvDif::stts_part DV_stts_Part;
                    DV_stts_Part.Pos_Begin=FrameCount-SampleCount;
                    DV_stts_Part.Pos_End=FrameCount;
                    DV_stts_Part.Duration=SampleDuration;
                    ((File_DvDif*)Stream[moov_trak_tkhd_TrackID].Parser)->Mpeg4_stts->push_back(DV_stts_Part);

                    Duration_FrameCount[SampleDuration]+=FrameCount;
                    if (Duration_FrameCount_Max<=Duration_FrameCount[SampleDuration])
                    {
                        Duration_FrameCount_Max=Duration_FrameCount[SampleDuration];
                        Duration_FrameCount_Max_Duration=SampleDuration;
                    }
                }
            #endif //MEDIAINFO_DVDIF_ANALYZE_YES
        FILLING_END();
    }

    FILLING_BEGIN();
        if (StreamKind_Last==Stream_Video)
        {
            if (moov_trak_mdia_mdhd_TimeScale && Min && Max)
            {
                if (Min!=Max)
                {
                    Fill(Stream_Video, StreamPos_Last, Video_FrameRate_Minimum, ((float)moov_trak_mdia_mdhd_TimeScale)/Max);
                    Fill(Stream_Video, StreamPos_Last, Video_FrameRate_Maximum, ((float)moov_trak_mdia_mdhd_TimeScale)/Min);
                    Fill(Stream_Video, StreamPos_Last, Video_FrameRate,         ((float)FrameCount)/moov_trak_mdia_mdhd_Duration*1000, 3, true);
                    Fill(Stream_Video, StreamPos_Last, Video_FrameRate_Mode,    "VFR");
                }
                else
                {
                    Fill(Stream_Video, StreamPos_Last, Video_FrameRate,         ((float)moov_trak_mdia_mdhd_TimeScale)/Max, 3, true);
                    Fill(Stream_Video, StreamPos_Last, Video_FrameRate_Mode,    "CFR");
                }
            }
            Fill(Stream_Video, StreamPos_Last, Video_FrameCount, FrameCount);
            
            #ifdef MEDIAINFO_DVDIF_ANALYZE_YES
                if (StreamKind_Last==Stream_Video && Retrieve(Stream_Video, StreamPos_Last, "Format")==_T("DV"))
                {
                    //Clean up the "normal" value
                    for (size_t Pos=0; Pos<((File_DvDif*)Stream[moov_trak_tkhd_TrackID].Parser)->Mpeg4_stts->size(); Pos++)
                    {
                        if (((File_DvDif*)Stream[moov_trak_tkhd_TrackID].Parser)->Mpeg4_stts->at(Pos).Duration==Duration_FrameCount_Max_Duration)
                        {
                            ((File_DvDif*)Stream[moov_trak_tkhd_TrackID].Parser)->Mpeg4_stts->erase(((File_DvDif*)Stream[moov_trak_tkhd_TrackID].Parser)->Mpeg4_stts->begin()+Pos);
                            Pos--;
                        }
                    }

                    if (((File_DvDif*)Stream[moov_trak_tkhd_TrackID].Parser)->Mpeg4_stts->empty())
                        {delete ((File_DvDif*)Stream[moov_trak_tkhd_TrackID].Parser)->Mpeg4_stts; ((File_DvDif*)Stream[moov_trak_tkhd_TrackID].Parser)->Mpeg4_stts=NULL;}
                }
            #endif //MEDIAINFO_DVDIF_ANALYZE_YES
        }
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
    Skip_B4(                                                    "cleanApertureWidth"); //BFP4, but how many bits?
    Skip_B4(                                                    "cleanApertureHeight"); //BFP4, but how many bits?
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_tapt_prof()
{
    NAME_VERSION_FLAG("Production Aperture Dimensions");

    //Parsing
    Skip_B4(                                                    "productionApertureWidth"); //BFP4, but how many bits?
    Skip_B4(                                                    "productionApertureHeight"); //BFP4, but how many bits?
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_tapt_enof()
{
    NAME_VERSION_FLAG("Encoded Pixels Dimensions");

    //Parsing
    Skip_B4(                                                    "encodedApertureWidth"); //BFP4, but how many bits?
    Skip_B4(                                                    "encodedApertureHeight"); //BFP4, but how many bits?
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
    Get_B4 (moov_trak_tkhd_TrackID,                             "Track ID"); Element_Info(moov_trak_tkhd_TrackID);
    Skip_B4(                                                    "Reserved");
    Get_B_DEPENDOFVERSION(Duration,                             "Duration"); Param_Info((int64u)Duration*1000/TimeScale, " ms"); Element_Info(Duration*1000/TimeScale, " ms");
    Skip_B4(                                                    "Reserved");
    Skip_B4(                                                    "Reserved");
    Skip_B2(                                                    "Layer");
    Skip_B2(                                                    "Alternate group");
    Get_B2 (Volume,                                             "Volume"); Param_Info(Ztring::ToZtring(((float)Volume)/256));
    Skip_B2(                                                    "Reserved");
    Element_Begin("Matrix structure", 36);
        Get_BFP4(16, a,                                         "a (width scale)");
        Get_BFP4(16, b,                                         "b (width rotate)");
        Get_BFP4( 2, u,                                         "u (width angle)");
        Get_BFP4(16, c,                                         "c (height rotate)");
        Get_BFP4(16, d,                                         "d (height scale)");
        Get_BFP4( 2, v,                                         "v (height angle)");
        Get_BFP4(16, x,                                         "x (position left)");
        Get_BFP4(16, y,                                         "y (position top)");
        Get_BFP4( 2, w,                                         "w (divider)");
    Element_End();
    Get_BFP4(16, moov_trak_tkhd_Width,                          "Track width");
    Get_BFP4(16, moov_trak_tkhd_Height,                         "Track height");

    FILLING_BEGIN();
        //Case of header is after main part
        if (StreamKind_Last!=Stream_Max)
        {
            std::map<int32u, stream>::iterator Temp=Stream.find((int32u)-1);
            if (Temp!=Stream.end())
            {
                Stream[moov_trak_tkhd_TrackID]=Temp->second;
                Temp->second.Parser=NULL; //It is a copy, we don't want that the destructor deletes the Parser
                Stream.erase(Temp);
            }
        }

        Fill(StreamKind_Last, StreamPos_Last, "Encoded_Date", Date_Created);
        Fill(StreamKind_Last, StreamPos_Last, "Tagged_Date", Date_Modified);
        Fill(StreamKind_Last, StreamPos_Last, General_ID, moov_trak_tkhd_TrackID, 10, true);
        if (moov_trak_tkhd_Height*d)
            moov_trak_tkhd_DisplayAspectRatio=(moov_trak_tkhd_Width*a)/(moov_trak_tkhd_Height*d);
        moov_trak_tkhd_Rotation=(float32)(std::atan2(b, a)*180.0/3.14159);
        if (moov_trak_tkhd_Rotation<0)
            moov_trak_tkhd_Rotation+=360;
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::moov_trak_tref()
{
    Element_Name("Track Reference");
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
void File_Mpeg4::moov_trak_tref_ipir()
{
    Element_Name("IPI declarations");

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
void File_Mpeg4::moov_trak_tref_mpod()
{
    Element_Name("included elementary stream");

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
        Stream[moov_trak_tkhd_TrackID].TimeCode_TrackID=TrackID;
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
        //Value.From_UTF8(ValueS.c_str());
        //if (Value.empty())
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
    Skip_XX(Element_Size,                                       "Data");
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
    Element_Info(Parameter.c_str());

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
                        Get_B2 (Language,                       "Language"); Param_Info(Language_Get(Language));
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
                    Get_B2(Language,                            "Language"); Param_Info(Language_Get(Language));
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
        CodecID_Fill(_T("QTCA"), Stream_General, 0, InfoCodecID_Format_Mpeg4);
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mpeg4::pnot()
{
    Element_Name("Preview");

    //Parsing
    Info_B4(Date_Modified,                                      "Modification date"); Param_Info(Ztring().Date_From_Seconds_1904(Date_Modified));
    Skip_B2(                                                    "Version number");
    Skip_C4(                                                    "Atom type");
    Skip_B2(                                                    "Atom index");
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
