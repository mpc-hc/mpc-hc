/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// How to:
// To add a new format,
// Fill includes, SelectFromExtension, ListFormats and LibraryIsModified
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

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#include "MediaInfo/Reader/Reader_File.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Multiple
#if defined(MEDIAINFO_BDMV_YES)
    #include "MediaInfo/Multiple/File_Bdmv.h"
#endif
#if defined(MEDIAINFO_CDXA_YES)
    #include "MediaInfo/Multiple/File_Cdxa.h"
#endif
#if defined(MEDIAINFO_DASHMPD_YES)
    #include "MediaInfo/Multiple/File_DashMpd.h"
#endif
#if defined(MEDIAINFO_DCP_YES)
    #include "MediaInfo/Multiple/File_DcpAm.h"
#endif
#if defined(MEDIAINFO_DCP_YES)
    #include "MediaInfo/Multiple/File_DcpCpl.h"
#endif
#if defined(MEDIAINFO_DCP_YES)
    #include "MediaInfo/Multiple/File_DcpPkl.h"
#endif
#if defined(MEDIAINFO_DVDIF_YES)
    #include "MediaInfo/Multiple/File_DvDif.h"
#endif
#if defined(MEDIAINFO_DVDV_YES)
    #include "MediaInfo/Multiple/File_Dvdv.h"
#endif
#if defined(MEDIAINFO_DXW_YES)
    #include "MediaInfo/Multiple/File_Dxw.h"
#endif
#if defined(MEDIAINFO_FLV_YES)
    #include "MediaInfo/Multiple/File_Flv.h"
#endif
#if defined(MEDIAINFO_GXF_YES)
    #include "MediaInfo/Multiple/File_Gxf.h"
#endif
#if defined(MEDIAINFO_HDSF4M_YES)
    #include "MediaInfo/Multiple/File_HdsF4m.h"
#endif
#if defined(MEDIAINFO_HLS_YES)
    #include "MediaInfo/Multiple/File_Hls.h"
#endif
#if defined(MEDIAINFO_IBI_YES)
    #include "MediaInfo/Multiple/File_Ibi.h"
#endif
#if defined(MEDIAINFO_ISM_YES)
    #include "MediaInfo/Multiple/File_Ism.h"
#endif
#if defined(MEDIAINFO_IMF_YES)
    #include "MediaInfo/Multiple/File_ImfCpl.h"
#endif
#if defined(MEDIAINFO_IVF_YES)
    #include "MediaInfo/Multiple/File_Ivf.h"
#endif
#if defined(MEDIAINFO_LXF_YES)
    #include "MediaInfo/Multiple/File_Lxf.h"
#endif
#if defined(MEDIAINFO_MK_YES)
    #include "MediaInfo/Multiple/File_Mk.h"
#endif
#if defined(MEDIAINFO_MPEG4_YES)
    #include "MediaInfo/Multiple/File_Mpeg4.h"
#endif
#if defined(MEDIAINFO_MPEGPS_YES)
    #include "MediaInfo/Multiple/File_MpegPs.h"
#endif
#if defined(MEDIAINFO_MPEGTS_YES) || defined(MEDIAINFO_BDAV_YES) || defined(MEDIAINFO_TSP_YES)
    #include "MediaInfo/Multiple/File_MpegTs.h"
#endif
#if defined(MEDIAINFO_MXF_YES)
    #include "MediaInfo/Multiple/File_Mxf.h"
#endif
#if defined(MEDIAINFO_NUT_YES)
    #include "MediaInfo/Multiple/File_Nut.h"
#endif
#if defined(MEDIAINFO_OGG_YES)
    #include "MediaInfo/Multiple/File_Ogg.h"
#endif
#if defined(MEDIAINFO_P2_YES)
    #include "MediaInfo/Multiple/File_P2_Clip.h"
#endif
#if defined(MEDIAINFO_PMP_YES)
    #include "MediaInfo/Multiple/File_Pmp.h"
#endif
#if defined(MEDIAINFO_RIFF_YES)
    #include "MediaInfo/Multiple/File_Riff.h"
#endif
#if defined(MEDIAINFO_RM_YES)
    #include "MediaInfo/Multiple/File_Rm.h"
#endif
#if defined(MEDIAINFO_SEQUENCEINFO_YES)
    #include "MediaInfo/Multiple/File_SequenceInfo.h"
#endif
#if defined(MEDIAINFO_SKM_YES)
    #include "MediaInfo/Multiple/File_Skm.h"
#endif
#if defined(MEDIAINFO_SWF_YES)
    #include "MediaInfo/Multiple/File_Swf.h"
#endif
#if defined(MEDIAINFO_WM_YES)
    #include "MediaInfo/Multiple/File_Wm.h"
#endif
#if defined(MEDIAINFO_XDCAM_YES)
    #include "MediaInfo/Multiple/File_Xdcam_Clip.h"
#endif
#if defined(MEDIAINFO_DPG_YES)
    #include "MediaInfo/Multiple/File_Dpg.h"
#endif

//---------------------------------------------------------------------------
// Video
#if defined(MEDIAINFO_AVC_YES)
    #include "MediaInfo/Video/File_Avc.h"
#endif
#if defined(MEDIAINFO_AVSV_YES)
    #include "MediaInfo/Video/File_AvsV.h"
#endif
#if defined(MEDIAINFO_DIRAC_YES)
    #include "MediaInfo/Video/File_Dirac.h"
#endif
#if defined(MEDIAINFO_FLIC_YES)
    #include "MediaInfo/Video/File_Flic.h"
#endif
#if defined(MEDIAINFO_H263_YES)
    #include "MediaInfo/Video/File_H263.h"
#endif
#if defined(MEDIAINFO_HEVC_YES)
    #include "MediaInfo/Video/File_Hevc.h"
#endif
#if defined(MEDIAINFO_MPEG4V_YES)
    #include "MediaInfo/Video/File_Mpeg4v.h"
#endif
#if defined(MEDIAINFO_MPEGV_YES)
    #include "MediaInfo/Video/File_Mpegv.h"
#endif
#if defined(MEDIAINFO_VC1_YES)
    #include "MediaInfo/Video/File_Vc1.h"
#endif
#if defined(MEDIAINFO_VC3_YES)
    #include "MediaInfo/Video/File_Vc3.h"
#endif
#if defined(MEDIAINFO_Y4M_YES)
    #include "MediaInfo/Video/File_Y4m.h"
#endif

//---------------------------------------------------------------------------
// Audio
#if defined(MEDIAINFO_AAC_YES)
    #include "MediaInfo/Audio/File_Aac.h"
#endif
#if defined(MEDIAINFO_AC3_YES)
    #include "MediaInfo/Audio/File_Ac3.h"
#endif
#if defined(MEDIAINFO_ALS_YES)
    #include "MediaInfo/Audio/File_Als.h"
#endif
#if defined(MEDIAINFO_AMR_YES)
    #include "MediaInfo/Audio/File_Amr.h"
#endif
#if defined(MEDIAINFO_AMV_YES)
    #include "MediaInfo/Audio/File_Amv.h"
#endif
#if defined(MEDIAINFO_APE_YES)
    #include "MediaInfo/Audio/File_Ape.h"
#endif
#if defined(MEDIAINFO_AU_YES)
    #include "MediaInfo/Audio/File_Au.h"
#endif
#if defined(MEDIAINFO_DTS_YES)
    #include "MediaInfo/Audio/File_Dts.h"
#endif
#if defined(MEDIAINFO_DOLBYE_YES)
    #include "MediaInfo/Audio/File_DolbyE.h"
#endif
#if defined(MEDIAINFO_FLAC_YES)
    #include "MediaInfo/Audio/File_Flac.h"
#endif
#if defined(MEDIAINFO_IT_YES)
    #include "MediaInfo/Audio/File_ImpulseTracker.h"
#endif
#if defined(MEDIAINFO_LA_YES)
    #include "MediaInfo/Audio/File_La.h"
#endif
#if defined(MEDIAINFO_MIDI_YES)
    #include "MediaInfo/Audio/File_Midi.h"
#endif
#if defined(MEDIAINFO_MOD_YES)
    #include "MediaInfo/Audio/File_Module.h"
#endif
#if defined(MEDIAINFO_MPC_YES)
    #include "MediaInfo/Audio/File_Mpc.h"
#endif
#if defined(MEDIAINFO_MPCSV8_YES)
    #include "MediaInfo/Audio/File_MpcSv8.h"
#endif
#if defined(MEDIAINFO_MPEGA_YES)
    #include "MediaInfo/Audio/File_Mpega.h"
#endif
#if defined(MEDIAINFO_RKAU_YES)
    #include "MediaInfo/Audio/File_Rkau.h"
#endif
#if defined(MEDIAINFO_S3M_YES)
    #include "MediaInfo/Audio/File_ScreamTracker3.h"
#endif
#if defined(MEDIAINFO_SMPTEST0337_YES)
    #include "MediaInfo/Audio/File_SmpteSt0337.h"
#endif
#if defined(MEDIAINFO_RKAU_YES)
    #include "MediaInfo/Audio/File_Rkau.h"
#endif
#if defined(MEDIAINFO_TAK_YES)
    #include "MediaInfo/Audio/File_Tak.h"
#endif
#if defined(MEDIAINFO_TTA_YES)
    #include "MediaInfo/Audio/File_Tta.h"
#endif
#if defined(MEDIAINFO_TWINVQ_YES)
    #include "MediaInfo/Audio/File_TwinVQ.h"
#endif
#if defined(MEDIAINFO_WVPK_YES)
    #include "MediaInfo/Audio/File_Wvpk.h"
#endif
#if defined(MEDIAINFO_XM_YES)
    #include "MediaInfo/Audio/File_ExtendedModule.h"
#endif

//---------------------------------------------------------------------------
// Text
#if defined(MEDIAINFO_EIA608_YES)
    #include "MediaInfo/Text/File_Eia608.h"
#endif
#if defined(MEDIAINFO_N19_YES)
    #include "MediaInfo/Text/File_N19.h"
#endif
#if defined(MEDIAINFO_SCC_YES)
    #include "MediaInfo/Text/File_Scc.h"
#endif
#if defined(MEDIAINFO_SUBRIP_YES)
    #include "MediaInfo/Text/File_SubRip.h"
#endif
#if defined(MEDIAINFO_TTML_YES)
    #include "MediaInfo/Text/File_Ttml.h"
#endif
#if defined(MEDIAINFO_OTHERTEXT_YES)
    #include "MediaInfo/Text/File_OtherText.h"
#endif

//---------------------------------------------------------------------------
// Image
#if defined(MEDIAINFO_BMP_YES)
    #include "MediaInfo/Image/File_Bmp.h"
#endif
#if defined(MEDIAINFO_DPX_YES)
    #include "MediaInfo/Image/File_Dpx.h"
#endif
#if defined(MEDIAINFO_EXR_YES)
    #include "MediaInfo/Image/File_Exr.h"
#endif
#if defined(MEDIAINFO_GIF_YES)
    #include "MediaInfo/Image/File_Gif.h"
#endif
#if defined(MEDIAINFO_ICO_YES)
    #include "MediaInfo/Image/File_Ico.h"
#endif
#if defined(MEDIAINFO_JPEG_YES)
    #include "MediaInfo/Image/File_Jpeg.h"
#endif
#if defined(MEDIAINFO_PCX_YES)
    #include "MediaInfo/Image/File_Pcx.h"
#endif
#if defined(MEDIAINFO_PNG_YES)
    #include "MediaInfo/Image/File_Png.h"
#endif
#if defined(MEDIAINFO_PSD_YES)
    #include "MediaInfo/Image/File_Psd.h"
#endif
#if defined(MEDIAINFO_TIFF_YES)
    #include "MediaInfo/Image/File_Tiff.h"
#endif
#if defined(MEDIAINFO_TGA_YES)
    #include "MediaInfo/Image/File_Tga.h"
#endif

//---------------------------------------------------------------------------
// Archive
#if defined(MEDIAINFO_7Z_YES)
    #include "MediaInfo/Archive/File_7z.h"
#endif
#if defined(MEDIAINFO_ACE_YES)
    #include "MediaInfo/Archive/File_Ace.h"
#endif
#if defined(MEDIAINFO_BZIP2_YES)
    #include "MediaInfo/Archive/File_Bzip2.h"
#endif
#if defined(MEDIAINFO_ELF_YES)
    #include "MediaInfo/Archive/File_Elf.h"
#endif
#if defined(MEDIAINFO_GZIP_YES)
    #include "MediaInfo/Archive/File_Gzip.h"
#endif
#if defined(MEDIAINFO_ISO9660_YES)
    #include "MediaInfo/Archive/File_Iso9660.h"
#endif
#if defined(MEDIAINFO_MZ_YES)
    #include "MediaInfo/Archive/File_Mz.h"
#endif
#if defined(MEDIAINFO_RAR_YES)
    #include "MediaInfo/Archive/File_Rar.h"
#endif
#if defined(MEDIAINFO_TAR_YES)
    #include "MediaInfo/Archive/File_Tar.h"
#endif
#if defined(MEDIAINFO_ZIP_YES)
    #include "MediaInfo/Archive/File_Zip.h"
#endif

//---------------------------------------------------------------------------
// Other
#if defined(MEDIAINFO_OTHER_YES)
    #include "MediaInfo/File_Other.h"
#endif
#if defined(MEDIAINFO_UNKNOWN_YES)
    #include "MediaInfo/File_Unknown.h"
#endif
#if defined(MEDIAINFO_DUMMY_YES)
    #include "MediaInfo/File_Dummy.h"
#endif
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
extern MediaInfo_Config Config;
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
bool MediaInfo_Internal::SelectFromExtension (const String &Parser)
{
    CriticalSectionLocker CSL(CS);

    //Clear last value
    delete Info; Info=NULL;

    //Searching the right File_*
             if (0) {} //For #defines

    // Multiple
    #if defined(MEDIAINFO_BDAV_YES)
        else if (Parser==__T("Bdav"))       {Info=new File_MpegTs(); ((File_MpegTs*)Info)->BDAV_Size=4;}
    #endif
    #if defined(MEDIAINFO_BDMV_YES)
        else if (Parser==__T("Bdmv"))        Info=new File_Bdmv();
    #endif
    #if defined(MEDIAINFO_CDXA_YES)
        else if (Parser==__T("Cdxa"))        Info=new File_Cdxa();
    #endif
    #if defined(MEDIAINFO_DASHMPD_YES)
        else if (Parser==__T("DashMpd"))     Info=new File_DashMpd();
    #endif
    #if defined(MEDIAINFO_DCP_YES)
        else if (Parser==__T("DcpAm"))       Info=new File_DcpAm();
    #endif
    #if defined(MEDIAINFO_DCP_YES)
        else if (Parser==__T("DcpCpl"))      Info=new File_DcpCpl();
    #endif
    #if defined(MEDIAINFO_DCP_YES)
        else if (Parser==__T("DcpPkg"))      Info=new File_DcpPkl();
    #endif
    #if defined(MEDIAINFO_DVDIF_YES)
        else if (Parser==__T("DvDif"))        Info=new File_DvDif();
    #endif
    #if defined(MEDIAINFO_DVDV_YES)
        else if (Parser==__T("Dvdv"))        Info=new File_Dvdv();
    #endif
    #if defined(MEDIAINFO_DXW_YES)
        else if (Parser==__T("Dxw"))         Info=new File_Dxw();
    #endif
    #if defined(MEDIAINFO_FLV_YES)
        else if (Parser==__T("Flv"))         Info=new File_Flv();
    #endif
    #if defined(MEDIAINFO_GXF_YES)
        else if (Parser==__T("Gxf"))         Info=new File_Gxf();
    #endif
    #if defined(MEDIAINFO_HDSF4M_YES)
        else if (Parser==__T("HdsF4m"))      Info=new File_HdsF4m();
    #endif
    #if defined(MEDIAINFO_HLS_YES)
        else if (Parser==__T("Hls"))         Info=new File_Hls();
    #endif
    #if defined(MEDIAINFO_IVF_YES)
        else if (Parser==__T("Ivf"))         Info=new File_Ivf();
    #endif
    #if defined(MEDIAINFO_ISM_YES)
        else if (Parser==__T("Ism"))         Info=new File_Ism();
    #endif
    #if defined(MEDIAINFO_LXF_YES)
        else if (Parser==__T("Lxf"))         Info=new File_Lxf();
    #endif
    #if defined(MEDIAINFO_MK_YES)
        else if (Parser==__T("Mk"))          Info=new File_Mk();
    #endif
    #if defined(MEDIAINFO_MPEG4_YES)
        else if (Parser==__T("Mpeg4"))       Info=new File_Mpeg4();
    #endif
    #if defined(MEDIAINFO_MPEGPS_YES)
        else if (Parser==__T("MpegPs"))      Info=new File_MpegPs();
    #endif
    #if defined(MEDIAINFO_MPEGTS_YES)
        else if (Parser==__T("MpegTs"))      Info=new File_MpegTs();
    #endif
    #if defined(MEDIAINFO_MXF_YES)
        else if (Parser==__T("Mxf"))         Info=new File_Mxf();
    #endif
    #if defined(MEDIAINFO_NUT_YES)
        else if (Parser==__T("Nut"))         Info=new File_Nut();
    #endif
    #if defined(MEDIAINFO_OGG_YES)
        else if (Parser==__T("Ogg"))         Info=new File_Ogg();
    #endif
    #if defined(MEDIAINFO_P2_YES)
        else if (Parser==__T("P2_Clip"))     Info=new File_P2_Clip();
    #endif
    #if defined(MEDIAINFO_PMP_YES)
        else if (Parser==__T("Pmp"))         Info=new File_Pmp();
    #endif
    #if defined(MEDIAINFO_RIFF_YES)
        else if (Parser==__T("Riff"))        Info=new File_Riff();
    #endif
    #if defined(MEDIAINFO_RM_YES)
        else if (Parser==__T("Rm"))          Info=new File_Rm();
    #endif
    #if defined(MEDIAINFO_SEQUENCEINFO_YES)
        else if (Parser==__T("SequenceInfo")) Info=new File_SequenceInfo();
    #endif
    #if defined(MEDIAINFO_SKM_YES)
        else if (Parser==__T("Skm"))         Info=new File_Skm();
    #endif
    #if defined(MEDIAINFO_SWF_YES)
        else if (Parser==__T("Swf"))         Info=new File_Swf();
    #endif
    #if defined(MEDIAINFO_WM_YES)
        else if (Parser==__T("Wm"))          Info=new File_Wm();
    #endif
    #if defined(MEDIAINFO_XDCAM_YES)
        else if (Parser==__T("Xdcam_Clip"))   Info=new File_Xdcam_Clip();
    #endif
    #if defined(MEDIAINFO_DPG_YES)
        else if (Parser==__T("Dpg"))         Info=new File_Dpg();
    #endif

    // Video
    #if defined(MEDIAINFO_AVC_YES)
        else if (Parser==__T("Avc"))         Info=new File_Avc();
    #endif
    #if defined(MEDIAINFO_HEVC_YES)
        else if (Parser==__T("Hevc"))         Info=new File_Hevc();
    #endif
    #if defined(MEDIAINFO_AVSV_YES)
        else if (Parser==__T("AvsV"))        Info=new File_AvsV();
    #endif
    #if defined(MEDIAINFO_DIRAC_YES)
        else if (Parser==__T("Dirac"))       Info=new File_Dirac();
    #endif
    #if defined(MEDIAINFO_FLIC_YES)
        else if (Parser==__T("Flic"))        Info=new File_Flic();
    #endif
    #if defined(MEDIAINFO_H263_YES)
        else if (Parser==__T("H263"))        Info=new File_H263();
    #endif
    #if defined(MEDIAINFO_MPEG4V_YES)
        else if (Parser==__T("Mpeg4v"))      Info=new File_Mpeg4v();
    #endif
    #if defined(MEDIAINFO_MPEGV_YES)
        else if (Parser==__T("Mpegv"))       Info=new File_Mpegv();
    #endif
    #if defined(MEDIAINFO_VC1_YES)
        else if (Parser==__T("Vc1"))         Info=new File_Vc1();
    #endif
    #if defined(MEDIAINFO_VC3_YES)
        else if (Parser==__T("Vc3"))         Info=new File_Vc3();
    #endif
    #if defined(MEDIAINFO_Y4M_YES)
        else if (Parser==__T("Y4m"))         Info=new File_Y4m();
    #endif

    // Audio
    #if defined(MEDIAINFO_AAC_YES)
        else if (Parser==__T("Adts"))       {Info=new File_Aac(); ((File_Aac*)Info)->Mode=File_Aac::Mode_ADTS;} // Prioritization against ADIF
    #endif
    #if defined(MEDIAINFO_AC3_YES)
        else if (Parser==__T("Ac3"))         Info=new File_Ac3();
    #endif
    #if defined(MEDIAINFO_SMPTEST0337_YES)
        else if (Parser==__T("Aes3"))        Info=new File_SmpteSt0337();
    #endif
    #if defined(MEDIAINFO_ALS_YES)
        else if (Parser==__T("Als"))         Info=new File_Als();
    #endif
    #if defined(MEDIAINFO_AMR_YES)
        else if (Parser==__T("Amr"))         Info=new File_Amr();
    #endif
    #if defined(MEDIAINFO_AMV_YES)
        else if (Parser==__T("Amv"))         Info=new File_Amv();
    #endif
    #if defined(MEDIAINFO_APE_YES)
        else if (Parser==__T("Ape"))         Info=new File_Ape();
    #endif
    #if defined(MEDIAINFO_AU_YES)
        else if (Parser==__T("Au"))          Info=new File_Au();
    #endif
    #if defined(MEDIAINFO_DTS_YES)
        else if (Parser==__T("Dts"))         Info=new File_Dts();
    #endif
    #if defined(MEDIAINFO_DOLBYE_YES)
        else if (Parser==__T("DolbyE"))      Info=new File_DolbyE();
    #endif
    #if defined(MEDIAINFO_FLAC_YES)
        else if (Parser==__T("Flac"))        Info=new File_Flac();
    #endif
    #if defined(MEDIAINFO_IT_YES)
        else if (Parser==__T("It"))          Info=new File_ImpulseTracker();
    #endif
    #if defined(MEDIAINFO_LA_YES)
        else if (Parser==__T("La"))          Info=new File_La();
    #endif
    #if defined(MEDIAINFO_MIDI_YES)
        else if (Parser==__T("Midi"))        Info=new File_Midi();
    #endif
    #if defined(MEDIAINFO_MOD_YES)
        else if (Parser==__T("Mod"))         Info=new File_Module();
    #endif
    #if defined(MEDIAINFO_MPC_YES)
        else if (Parser==__T("Mpc"))         Info=new File_Mpc();
    #endif
    #if defined(MEDIAINFO_MPCSV8_YES)
        else if (Parser==__T("Mpc"))         Info=new File_MpcSv8();
    #endif
    #if defined(MEDIAINFO_MPEGA_YES)
        else if (Parser==__T("Mpega"))       Info=new File_Mpega();
    #endif
    #if defined(MEDIAINFO_AU_YES)
        else if (Parser==__T("Au"))          Info=new File_Au();
    #endif
    #if defined(MEDIAINFO_RKAU_YES)
        else if (Parser==__T("Rkau"))         Info=new File_Rkau();
    #endif
    #if defined(MEDIAINFO_S3M_YES)
        else if (Parser==__T("S3m"))         Info=new File_ScreamTracker3();
    #endif
    #if defined(MEDIAINFO_TAK_YES)
        else if (Parser==__T("Tak"))         Info=new File_Tak();
    #endif
    #if defined(MEDIAINFO_TTA_YES)
        else if (Parser==__T("Tta"))         Info=new File_Tta();
    #endif
    #if defined(MEDIAINFO_TWINVQ_YES)
        else if (Parser==__T("TwinVQ"))      Info=new File_TwinVQ();
    #endif
    #if defined(MEDIAINFO_WVPK_YES)
        else if (Parser==__T("Wvpk"))        Info=new File_Wvpk();
    #endif
    #if defined(MEDIAINFO_XM_YES)
        else if (Parser==__T("Xm"))          Info=new File_ExtendedModule();
    #endif

    // Text
    #if defined(MEDIAINFO_EIA608_YES)
        else if (Parser==__T("CEA-608"))     Info=new File_Eia608();
        else if (Parser==__T("EIA-608"))     Info=new File_Eia608();
    #endif
    #if defined(MEDIAINFO_N19_YES)
        else if (Parser==__T("N19"))         Info=new File_N19();
    #endif
    #if defined(MEDIAINFO_SCC_YES)
        else if (Parser==__T("SCC"))         Info=new File_Scc();
    #endif
    #if defined(MEDIAINFO_SUBRIP_YES)
        else if (Parser==__T("SubRip"))      Info=new File_SubRip();
    #endif
    #if defined(MEDIAINFO_TTML_YES)
        else if (Parser==__T("TTML"))        Info=new File_Ttml();
    #endif
    #if defined(MEDIAINFO_OTHERTEXT_YES)
        else if (Parser==__T("OtherText"))   Info=new File_OtherText();
    #endif

    // Image
    #if defined(MEDIAINFO_GIF_YES)
        else if (Parser==__T("Gif"))         Info=new File_Gif();
    #endif
    #if defined(MEDIAINFO_BMP_YES)
        else if (Parser==__T("Bmp"))         Info=new File_Bmp();
    #endif
    #if defined(MEDIAINFO_DPX_YES)
        else if (Parser==__T("Dpx"))         Info=new File_Dpx();
    #endif
    #if defined(MEDIAINFO_EXR_YES)
        else if (Parser==__T("Exr"))         Info=new File_Exr();
    #endif
    #if defined(MEDIAINFO_ICO_YES)
        else if (Parser==__T("Ico"))         Info=new File_Ico();
    #endif
    #if defined(MEDIAINFO_JPEG_YES)
        else if (Parser==__T("Jpeg"))        Info=new File_Jpeg();
    #endif
    #if defined(MEDIAINFO_PCX_YES)
        else if (Parser==__T("PCX"))         Info=new File_Pcx();
    #endif
    #if defined(MEDIAINFO_PNG_YES)
        else if (Parser==__T("Png"))         Info=new File_Png();
    #endif
    #if defined(MEDIAINFO_PSD_YES)
        else if (Parser==__T("Psd"))         Info=new File_Psd();
    #endif
    #if defined(MEDIAINFO_TIFF_YES)
        else if (Parser==__T("Tiff"))        Info=new File_Tiff();
    #endif
    #if defined(MEDIAINFO_TGA_YES)
        else if (Parser==__T("Tga"))         Info=new File_Tga();
    #endif

    // Archive
    #if defined(MEDIAINFO_7Z_YES)
        else if (Parser==__T("7z"))          Info=new File_7z();
    #endif
    #if defined(MEDIAINFO_ACE_YES)
        else if (Parser==__T("Ace"))         Info=new File_Ace();
    #endif
    #if defined(MEDIAINFO_BZIP2_YES)
        else if (Parser==__T("Bzip2"))       Info=new File_Bzip2();
    #endif
    #if defined(MEDIAINFO_ELF_YES)
        else if (Parser==__T("Elf"))         Info=new File_Elf();
    #endif
    #if defined(MEDIAINFO_GZIP_YES)
        else if (Parser==__T("Gzip"))        Info=new File_Gzip();
    #endif
    #if defined(MEDIAINFO_ISO9660_YES)
        else if (Parser==__T("Iso9660"))     Info=new File_Iso9660();
    #endif
    #if defined(MEDIAINFO_MZ_YES)
        else if (Parser==__T("Mz"))          Info=new File_Mz();
    #endif
    #if defined(MEDIAINFO_RAR_YES)
        else if (Parser==__T("Rar"))         Info=new File_Rar();
    #endif
    #if defined(MEDIAINFO_TAR_YES)
        else if (Parser==__T("Tar"))         Info=new File_Tar();
    #endif
    #if defined(MEDIAINFO_ZIP_YES)
        else if (Parser==__T("Zip"))         Info=new File_Zip();
    #endif

    // Other
    #if defined(MEDIAINFO_OTHER_YES)
        else if (Parser==__T("Other"))       Info=new File_Other();
    #endif

    //No parser
        else
            return false;

    return true;
}

//---------------------------------------------------------------------------
#if !defined(MEDIAINFO_READER_NO)
int MediaInfo_Internal::ListFormats(const String &File_Name)
{
    // Multiple
    #if defined(MEDIAINFO_BDAV_YES)
        delete Info; Info=new File_MpegTs(); ((File_MpegTs*)Info)->BDAV_Size=4; if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_BDMV_YES)
        delete Info; Info=new File_Bdmv();               if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_CDXA_YES)
        delete Info; Info=new File_Cdxa();               if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_DASHMPD_YES)
        delete Info; Info=new File_DashMpd();            if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_DCP_YES)
        delete Info; Info=new File_DcpAm();              if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_DCP_YES)
        delete Info; Info=new File_DcpCpl();             if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_DCP_YES)
        delete Info; Info=new File_DcpPkl();             if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_DVDIF_YES)
        delete Info; Info=new File_DvDif();              if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_DVDV_YES)
        delete Info; Info=new File_Dvdv();               if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_DXW_YES)
        delete Info; Info=new File_Dxw();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_FLV_YES)
        delete Info; Info=new File_Flv();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_GXF_YES)
        delete Info; Info=new File_Gxf();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_HDSF4M_YES)
        delete Info; Info=new File_HdsF4m();             if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_HLS_YES)
        delete Info; Info=new File_Hls();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_IBI_YES)
        delete Info; Info=new File_Ibi();                 if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_ISM_YES)
        delete Info; Info=new File_Ism();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_IVF_YES)
        delete Info; Info=new File_Ivf();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_LXF_YES)
        delete Info; Info=new File_Lxf();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_MK_YES)
        delete Info; Info=new File_Mk();                 if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_MPEG4_YES)
        delete Info; Info=new File_Mpeg4();              if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_MPEGPS_YES)
        delete Info; Info=new File_MpegPs();             if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_MPEGTS_YES)
        delete Info; Info=new File_MpegTs();             if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_MPLI_YES)
        delete Info; Info=new File_Mpli();               if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_MXF_YES)
        delete Info; Info=new File_Mxf();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_NUT_YES)
        delete Info; Info=new File_Nut();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_OGG_YES)
        delete Info; Info=new File_Ogg();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_P2_YES)
        delete Info; Info=new File_P2_Clip();            if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_PMP_YES)
        delete Info; Info=new File_Pmp();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_RIFF_YES)
        delete Info; Info=new File_Riff();               if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_RM_YES)
        delete Info; Info=new File_Rm();                 if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_SEQUENCEINFO_YES)
        delete Info; Info=new File_SequenceInfo();       if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_SKM_YES)
        delete Info; Info=new File_Skm();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_SWF_YES)
        delete Info; Info=new File_Swf();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_TSP_YES)
        delete Info; Info=new File_MpegTs(); ((File_MpegTs*)Info)->TSP_Size=16; if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_WM_YES)
        delete Info; Info=new File_Wm();                 if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_XDCAM_YES)
        delete Info; Info=new File_Xdcam_Clip();         if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_DPG_YES)
        delete Info; Info=new File_Dpg();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif

    // Video
    #if defined(MEDIAINFO_AVC_YES)
        delete Info; Info=new File_Avc();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_HEVC_YES)
        delete Info; Info=new File_Hevc();               if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_AVSV_YES)
        delete Info; Info=new File_AvsV();               if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_DIRAC_YES)
        delete Info; Info=new File_Dirac();              if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_FLIC_YES)
        delete Info; Info=new File_Flic();               if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_H263_YES)
        delete Info; Info=new File_H263();               if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_MPEG4V_YES)
        delete Info; Info=new File_Mpeg4v();             if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_MPEGV_YES)
        delete Info; Info=new File_Mpegv();              if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_VC1_YES)
        delete Info; Info=new File_Vc1();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_VC3_YES)
        delete Info; Info=new File_Vc3();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_Y4M_YES)
        delete Info; Info=new File_Y4m();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif

    // Audio
    #if defined(MEDIAINFO_AAC_YES)
        delete Info; Info=new File_Aac();               if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_AC3_YES)
        delete Info; Info=new File_Ac3();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_SMPTEST0337_YES)
        delete Info; Info=new File_SmpteSt0337();        if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_ALS_YES)
        delete Info; Info=new File_Als();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_AMR_YES)
        delete Info; Info=new File_Amr();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_AMV_YES)
        delete Info; Info=new File_Amv();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_APE_YES)
        delete Info; Info=new File_Ape();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_AU_YES)
        delete Info; Info=new File_Au();                 if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_DTS_YES)
        delete Info; Info=new File_Dts();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
//    Too many false-positives
//    #if defined(MEDIAINFO_DOLBYE_YES)
//        delete Info; Info=new File_DolbyE();             if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
//    #endif
    #if defined(MEDIAINFO_FLAC_YES)
        delete Info; Info=new File_Flac();               if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_IT_YES)
        delete Info; Info=new File_ImpulseTracker();     if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_LA_YES)
        delete Info; Info=new File_La();                 if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_MIDI_YES)
        delete Info; Info=new File_Midi();               if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_MOD_YES)
        delete Info; Info=new File_Module();             if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_MPC_YES)
        delete Info; Info=new File_Mpc();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_MPCSV8_YES)
        delete Info; Info=new File_MpcSv8();             if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_MPEGA_YES)
        delete Info; Info=new File_Mpega();              if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_RKAU_YES)
        delete Info; Info=new File_Rkau();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_TAK_YES)
        delete Info; Info=new File_Tak();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_S3M_YES)
        delete Info; Info=new File_ScreamTracker3();     if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_TTA_YES)
        delete Info; Info=new File_Tta();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_TWINVQ_YES)
        delete Info; Info=new File_TwinVQ();             if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_WVPK_YES)
        delete Info; Info=new File_Wvpk();               if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_XM_YES)
        delete Info; Info=new File_ExtendedModule();     if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif

    // Text
    #if defined(MEDIAINFO_N19_YES)
        delete Info; Info=new File_N19();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_SCC_YES)
        delete Info; Info=new File_Scc();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_SUBRIP_YES)
        delete Info; Info=new File_SubRip();             if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_TTML_YES)
        delete Info; Info=new File_Ttml();               if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_OTHERTEXT_YES)
        delete Info; Info=new File_OtherText();          if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif

    // Image
    #if defined(MEDIAINFO_BMP_YES)
        delete Info; Info=new File_Bmp();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_DPX_YES)
        delete Info; Info=new File_Dpx();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_GIF_YES)
        delete Info; Info=new File_Gif();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_ICO_YES)
        delete Info; Info=new File_Ico();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_JPEG_YES)
        delete Info; Info=new File_Jpeg();               if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_PCX_YES)
        delete Info; Info=new File_Pcx();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_PNG_YES)
        delete Info; Info=new File_Png();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_PSD_YES)
        delete Info; Info=new File_Psd();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_TIFF_YES)
        delete Info; Info=new File_Tiff();               if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_TGA_YES)
        delete Info; Info=new File_Tga();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif

    // Archive
    #if defined(MEDIAINFO_ACE_YES)
        delete Info; Info=new File_Ace();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_7Z_YES)
        delete Info; Info=new File_7z();                 if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_BZIP2_YES)
        delete Info; Info=new File_Bzip2();              if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_ELF_YES)
        delete Info; Info=new File_Elf();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_GZIP_YES)
        delete Info; Info=new File_Gzip();               if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_ISO9660_YES)
        delete Info; Info=new File_Iso9660();            if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_MZ_YES)
        delete Info; Info=new File_Mz();                 if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_RAR_YES)
        delete Info; Info=new File_Rar();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_TAR_YES)
        delete Info; Info=new File_Tar();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif
    #if defined(MEDIAINFO_ZIP_YES)
        delete Info; Info=new File_Zip();                if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif

    // Other
    #if !defined(MEDIAINFO_OTHER_NO)
        delete Info; Info=new File_Other();              if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    #endif

    // Default (empty)
        delete Info; Info=new File_Unknown();            if (((Reader_File*)Reader)->Format_Test_PerParser(this, File_Name)>0) return 1;
    return 0;
}
#endif //!defined(MEDIAINFO_READER_NO)

//---------------------------------------------------------------------------
bool MediaInfo_Internal::LibraryIsModified ()
{
    #if defined(MEDIAINFO_MULTI_NO) || defined(MEDIAINFO_VIDEO_NO) || defined(MEDIAINFO_AUDIO_NO) || defined(MEDIAINFO_TEXT_NO) || defined(MEDIAINFO_IMAGE_NO) || defined(MEDIAINFO_ARCHIVE_NO) \
     || defined(MEDIAINFO_BDAV_NO) || defined(MEDIAINFO_MK_NO) || defined(MEDIAINFO_OGG_NO) || defined(MEDIAINFO_RIFF_NO) || defined(MEDIAINFO_MPEG4_NO) || defined(MEDIAINFO_MPEGPS_NO) || defined(MEDIAINFO_MPEGTS_NO) || defined(MEDIAINFO_DXW_NO) || defined(MEDIAINFO_FLV_NO) || defined(MEDIAINFO_GXF_NO) || defined(MEDIAINFO_HDSF4M_NO) || defined(MEDIAINFO_HLS_NO) || defined(MEDIAINFO_ISM_NO) || defined(MEDIAINFO_IVF_NO) || defined(MEDIAINFO_LXF_NO) || defined(MEDIAINFO_SWF_NO) || defined(MEDIAINFO_MXF_NO) || defined(MEDIAINFO_NUT_NO) || defined(MEDIAINFO_WM_NO) || defined(MEDIAINFO_QT_NO) || defined(MEDIAINFO_RM_NO) || defined(MEDIAINFO_DVDIF_NO) || defined(MEDIAINFO_DVDV_NO) || defined(MEDIAINFO_CDXA_NO) || defined(MEDIAINFO_DPG_NO) || defined(MEDIAINFO_TSP_NO) \
     || defined(MEDIAINFO_AVC_NO) || defined(MEDIAINFO_AVSV_NO) || defined(MEDIAINFO_HEVC_NO) || defined(MEDIAINFO_MPEG4V_NO) || defined(MEDIAINFO_MPEGV_NO) || defined(MEDIAINFO_FLIC_NO) || defined(MEDIAINFO_THEORA_NO) || defined(MEDIAINFO_Y4M_NO) \
     || defined(MEDIAINFO_AC3_NO) || defined(MEDIAINFO_ADIF_NO) || defined(MEDIAINFO_ADTS_NO) || defined(MEDIAINFO_SMPTEST0337_NO) || defined(MEDIAINFO_AMR_NO) || defined(MEDIAINFO_DTS_NO) || defined(MEDIAINFO_DOLBYE_NO) || defined(MEDIAINFO_FLAC_NO) || defined(MEDIAINFO_APE_NO) || defined(MEDIAINFO_MPC_NO) || defined(MEDIAINFO_MPCSV8_NO) || defined(MEDIAINFO_MPEGA_NO) || defined(MEDIAINFO_TWINVQ_NO) || defined(MEDIAINFO_XM_NO) || defined(MEDIAINFO_MOD_NO) || defined(MEDIAINFO_S3M_NO) || defined(MEDIAINFO_IT_NO) || defined(MEDIAINFO_SPEEX_NO) || defined(MEDIAINFO_TAK_NO) || defined(MEDIAINFO_PS2A_NO) \
     || defined(MEDIAINFO_CMML_NO)  || defined(MEDIAINFO_KATE_NO)  || defined(MEDIAINFO_PGS_NO) || defined(MEDIAINFO_OTHERTEXT_NO) \
     || defined(MEDIAINFO_BMP_NO) || defined(MEDIAINFO_DPX_NO) || defined(MEDIAINFO_EXR_NO) || defined(MEDIAINFO_GIF_NO) || defined(MEDIAINFO_ICO_NO) || defined(MEDIAINFO_JPEG_NO) || defined(MEDIAINFO_PNG_NO) || defined(MEDIAINFO_TGA_NO) || defined(MEDIAINFO_TIFF_NO) \
     || defined(MEDIAINFO_7Z_NO) || defined(MEDIAINFO_ZIP_NO) || defined(MEDIAINFO_RAR_NO) || defined(MEDIAINFO_ACE_NO) || defined(MEDIAINFO_ELF_NO) || defined(MEDIAINFO_MZ_NO) \
     || defined(MEDIAINFO_OTHER_NO) || defined(MEDIAINFO_DUMMY_NO)
        return true;
    #else
        return false;
    #endif
}

//---------------------------------------------------------------------------
void MediaInfo_Internal::CreateDummy (const String&)
{
    #if defined(MEDIAINFO_DUMMY_YES)
        Info=new File_Dummy();
        ((File_Dummy*)Info)->KindOfDummy=Value;
    #endif
}

} //NameSpace
