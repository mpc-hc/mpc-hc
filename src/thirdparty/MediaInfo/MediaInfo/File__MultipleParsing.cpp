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
#include "MediaInfo/File__MultipleParsing.h"
//---------------------------------------------------------------------------
// Multiple
#if defined(MEDIAINFO_AAF_YES)
    #include "MediaInfo/Multiple/File_Aaf.h"
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
#if defined(MEDIAINFO_DPG_YES)
    #include "MediaInfo/Multiple/File_Dpg.h"
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
#if defined(MEDIAINFO_ISM_YES)
    #include "MediaInfo/Multiple/File_Ism.h"
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
#if defined(MEDIAINFO_PTX_YES)
    #include "MediaInfo/Multiple/File_Ptx.h"
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
#if defined(MEDIAINFO_SMPTEST0337_YES)
    #include "MediaInfo/Audio/File_SmpteSt0337.h"
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
#if defined(MEDIAINFO_XM_YES)
    #include "MediaInfo/Audio/File_ExtendedModule.h"
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
#if defined(MEDIAINFO_OPENMG_YES)
    #include "MediaInfo/Audio/File_OpenMG.h"
#endif
#if defined(MEDIAINFO_PCM_YES)
    #include "MediaInfo/Audio/File_Pcm.h"
#endif
#if defined(MEDIAINFO_RKAU_YES)
    #include "MediaInfo/Audio/File_Rkau.h"
#endif
#if defined(MEDIAINFO_S3M_YES)
    #include "MediaInfo/Audio/File_ScreamTracker3.h"
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

//---------------------------------------------------------------------------
// Text
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
#if defined(MEDIAINFO_ARRIRAW_YES)
    #include "MediaInfo/Image/File_ArriRaw.h"
#endif
#if defined(MEDIAINFO_BMP_YES)
    #include "MediaInfo/Image/File_Bmp.h"
#endif
#if defined(MEDIAINFO_DDS_YES)
    #include "MediaInfo/Image/File_Dds.h"
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
#if defined(MEDIAINFO_TIFF_YES)
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
#if !defined(MEDIAINFO_OTHER_NO)
    #include "MediaInfo/File_Other.h"
#endif
#include "MediaInfo/File_Unknown.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Out
//***************************************************************************

//---------------------------------------------------------------------------
File__Analyze* File__MultipleParsing::Parser_Get()
{
    if (Parser.size()!=1)
        return NULL;

    File__Analyze* ToReturn=Parser[0]; //The first parser
    Parser.clear();
    return ToReturn;
}

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File__MultipleParsing::File__MultipleParsing()
:File__Analyze()
{
    #if MEDIAINFO_TRACE
        Trace_DoNotSave=true;
    #endif //MEDIAINFO_TRACE

    // Multiple
    #if defined(MEDIAINFO_AAF_YES)
        Parser.push_back(new File_Aaf());
    #endif
    #if defined(MEDIAINFO_BDAV_YES)
        {File_MpegTs* Temp=new File_MpegTs(); Temp->BDAV_Size=4; Parser.push_back(Temp);}
    #endif
    #if defined(MEDIAINFO_CDXA_YES)
        Parser.push_back(new File_Cdxa());
    #endif
    #if defined(MEDIAINFO_DASHMPD_YES)
        Parser.push_back(new File_DashMpd());
    #endif
    #if defined(MEDIAINFO_DCP_YES)
        Parser.push_back(new File_DcpAm());
    #endif
    #if defined(MEDIAINFO_DCP_YES)
        Parser.push_back(new File_DcpCpl());
    #endif
    #if defined(MEDIAINFO_DCP_YES)
        Parser.push_back(new File_DcpPkl());
    #endif
    #if defined(MEDIAINFO_DPG_YES)
        Parser.push_back(new File_Dpg());
    #endif
    #if defined(MEDIAINFO_DVDIF_YES)
        Parser.push_back(new File_DvDif());
    #endif
    #if defined(MEDIAINFO_DVDV_YES)
        Parser.push_back(new File_Dvdv());
    #endif
    #if defined(MEDIAINFO_DXW_YES)
        Parser.push_back(new File_Dxw());
    #endif
    #if defined(MEDIAINFO_FLV_YES)
        Parser.push_back(new File_Flv());
    #endif
    #if defined(MEDIAINFO_GXF_YES)
        Parser.push_back(new File_Gxf());
    #endif
    #if defined(MEDIAINFO_HDSF4M_YES)
        Parser.push_back(new File_HdsF4m());
    #endif
    #if defined(MEDIAINFO_HLS_YES)
        Parser.push_back(new File_Hls());
    #endif
    #if defined(MEDIAINFO_ISM_YES)
        Parser.push_back(new File_Ism());
    #endif
    #if defined(MEDIAINFO_IVF_YES)
        Parser.push_back(new File_Ivf());
    #endif
    #if defined(MEDIAINFO_LXF_YES)
        Parser.push_back(new File_Lxf());
    #endif
    #if defined(MEDIAINFO_MK_YES)
        Parser.push_back(new File_Mk());
    #endif
    #if defined(MEDIAINFO_MPEG4_YES)
        Parser.push_back(new File_Mpeg4());
    #endif
    #if defined(MEDIAINFO_MPEGPS_YES)
        Parser.push_back(new File_MpegPs());
    #endif
    #if defined(MEDIAINFO_MPEGTS_YES)
        Parser.push_back(new File_MpegTs());
    #endif
    #if defined(MEDIAINFO_TSP_YES)
        {File_MpegTs* Temp=new File_MpegTs(); Temp->TSP_Size=16; Parser.push_back(Temp);}
    #endif
    #if defined(MEDIAINFO_MXF_YES)
        Parser.push_back(new File_Mxf());
    #endif
    #if defined(MEDIAINFO_NUT_YES)
        Parser.push_back(new File_Nut());
    #endif
    #if defined(MEDIAINFO_OGG_YES)
        Parser.push_back(new File_Ogg());
    #endif
    #if defined(MEDIAINFO_P2_YES)
        Parser.push_back(new File_P2_Clip());
    #endif
    #if defined(MEDIAINFO_PMP_YES)
        Parser.push_back(new File_Pmp());
    #endif
    #if defined(MEDIAINFO_RIFF_YES)
        Parser.push_back(new File_Riff());
    #endif
    #if defined(MEDIAINFO_RM_YES)
        Parser.push_back(new File_Rm());
    #endif
    #if defined(MEDIAINFO_SEQUENCEINFO_YES)
        Parser.push_back(new File_SequenceInfo());
    #endif
    #if defined(MEDIAINFO_SKM_YES)
        Parser.push_back(new File_Skm());
    #endif
    #if defined(MEDIAINFO_SWF_YES)
        Parser.push_back(new File_Swf());
    #endif
    #if defined(MEDIAINFO_WM_YES)
        Parser.push_back(new File_Wm());
    #endif
    #if defined(MEDIAINFO_XDCAM_YES)
        Parser.push_back(new File_Xdcam_Clip());
    #endif

    // Video
    #if defined(MEDIAINFO_AVC_YES)
        Parser.push_back(new File_Avc());
    #endif
    #if defined(MEDIAINFO_AVSV_YES)
        Parser.push_back(new File_AvsV());
    #endif
    #if defined(MEDIAINFO_DIRAC_YES)
        Parser.push_back(new File_Dirac());
    #endif
    #if defined(MEDIAINFO_FLIC_YES)
        Parser.push_back(new File_Flic());
    #endif
    #if defined(MEDIAINFO_H263_YES)
        Parser.push_back(new File_H263());
    #endif
    #if defined(MEDIAINFO_MPEG4V_YES)
        Parser.push_back(new File_Mpeg4v());
    #endif
    #if defined(MEDIAINFO_MPEGV_YES)
        Parser.push_back(new File_Mpegv());
    #endif
    #if defined(MEDIAINFO_VC1_YES)
        Parser.push_back(new File_Vc1());
    #endif
    #if defined(MEDIAINFO_Y4M_YES)
        Parser.push_back(new File_Y4m());
    #endif

    // Audio
    #if defined(MEDIAINFO_AAC_YES)
        {File_Aac* Temp=new File_Aac(); Temp->Mode=File_Aac::Mode_ADIF; Parser.push_back(Temp);}
    #endif
    #if defined(MEDIAINFO_AAC_YES)
        {File_Aac* Temp=new File_Aac(); Temp->Mode=File_Aac::Mode_ADTS; Parser.push_back(Temp);}
    #endif
    #if defined(MEDIAINFO_AC3_YES)
        Parser.push_back(new File_Ac3());
    #endif
    #if defined(MEDIAINFO_SMPTEST0337_YES)
        Parser.push_back(new File_SmpteSt0337());
    #endif
    #if defined(MEDIAINFO_ALS_YES)
        Parser.push_back(new File_Als());
    #endif
    #if defined(MEDIAINFO_AMR_YES)
        Parser.push_back(new File_Amr());
    #endif
    #if defined(MEDIAINFO_AMV_YES)
        Parser.push_back(new File_Amv());
    #endif
    #if defined(MEDIAINFO_APE_YES)
        Parser.push_back(new File_Ape());
    #endif
    #if defined(MEDIAINFO_AU_YES)
        Parser.push_back(new File_Au());
    #endif
    #if defined(MEDIAINFO_DTS_YES)
        Parser.push_back(new File_Dts());
    #endif
    #if defined(MEDIAINFO_FLAC_YES)
        Parser.push_back(new File_Flac());
    #endif
    #if defined(MEDIAINFO_IT_YES)
        Parser.push_back(new File_ImpulseTracker());
    #endif
    #if defined(MEDIAINFO_LA_YES)
        Parser.push_back(new File_La());
    #endif
    #if defined(MEDIAINFO_MIDI_YES)
        Parser.push_back(new File_Midi());
    #endif
    #if defined(MEDIAINFO_MOD_YES)
        Parser.push_back(new File_Module());
    #endif
    #if defined(MEDIAINFO_MPC_YES)
        Parser.push_back(new File_Mpc());
    #endif
    #if defined(MEDIAINFO_MPCSV8_YES)
        Parser.push_back(new File_MpcSv8());
    #endif
    #if defined(MEDIAINFO_MPEGA_YES)
        Parser.push_back(new File_Mpega());
    #endif
    #if defined(MEDIAINFO_OPENMG_YES)
        Parser.push_back(new File_OpenMG());
    #endif
    #if defined(MEDIAINFO_RKAU_YES)
        Parser.push_back(new File_Rkau());
    #endif
    #if defined(MEDIAINFO_S3M_YES)
        Parser.push_back(new File_ScreamTracker3());
    #endif
    #if defined(MEDIAINFO_TAK_YES)
        Parser.push_back(new File_Tak());
    #endif
    #if defined(MEDIAINFO_TTA_YES)
        Parser.push_back(new File_Tta());
    #endif
    #if defined(MEDIAINFO_TWINVQ_YES)
        Parser.push_back(new File_TwinVQ());
    #endif
    #if defined(MEDIAINFO_WVPK_YES)
        Parser.push_back(new File_Wvpk());
    #endif
    #if defined(MEDIAINFO_XM_YES)
        Parser.push_back(new File_ExtendedModule());
    #endif

    // Text
    #if defined(MEDIAINFO_N19_YES)
        Parser.push_back(new File_N19());
    #endif
    #if defined(MEDIAINFO_SCC_YES)
        Parser.push_back(new File_Scc());
    #endif
    #if defined(MEDIAINFO_SUBRIP_YES)
        Parser.push_back(new File_SubRip());
    #endif
    #if defined(MEDIAINFO_TTML_YES)
        Parser.push_back(new File_Ttml());
    #endif
    #if defined(MEDIAINFO_OTHERTEXT_YES)
        Parser.push_back(new File_OtherText());
    #endif

    // Image
    #if defined(MEDIAINFO_ARRIRAW_YES)
        Parser.push_back(new File_ArriRaw());
    #endif
    #if defined(MEDIAINFO_BMP_YES)
        Parser.push_back(new File_Bmp());
    #endif
    #if defined(MEDIAINFO_DDS_YES)
        Parser.push_back(new File_Dds());
    #endif
    #if defined(MEDIAINFO_GIF_YES)
        Parser.push_back(new File_Gif());
    #endif
    #if defined(MEDIAINFO_ICO_YES)
        Parser.push_back(new File_Ico());
    #endif
    #if defined(MEDIAINFO_JPEG_YES)
        Parser.push_back(new File_Jpeg());
    #endif
    #if defined(MEDIAINFO_PCX_YES)
        Parser.push_back(new File_Pcx());
    #endif
    #if defined(MEDIAINFO_PNG_YES)
        Parser.push_back(new File_Png());
    #endif
    #if defined(MEDIAINFO_PSD_YES)
        Parser.push_back(new File_Psd());
    #endif
    #if defined(MEDIAINFO_TIFF_YES)
        Parser.push_back(new File_Tiff());
    #endif
    #if defined(MEDIAINFO_TGA_YES)
        Parser.push_back(new File_Tga());
    #endif

    // Archive
    #if defined(MEDIAINFO_7Z_YES)
        Parser.push_back(new File_7z());
    #endif
    #if defined(MEDIAINFO_ACE_YES)
        Parser.push_back(new File_Ace());
    #endif
    #if defined(MEDIAINFO_BZIP2_YES)
        Parser.push_back(new File_Bzip2());
    #endif
    #if defined(MEDIAINFO_ELF_YES)
        Parser.push_back(new File_Elf());
    #endif
    #if defined(MEDIAINFO_GZIP_YES)
        Parser.push_back(new File_Gzip());
    #endif
    #if defined(MEDIAINFO_ISO9660_YES)
        Parser.push_back(new File_Iso9660());
    #endif
    #if defined(MEDIAINFO_MZ_YES)
        Parser.push_back(new File_Mz());
    #endif
    #if defined(MEDIAINFO_RAR_YES)
        Parser.push_back(new File_Rar());
    #endif
    #if defined(MEDIAINFO_TAR_YES)
        Parser.push_back(new File_Tar());
    #endif
    #if defined(MEDIAINFO_ZIP_YES)
        Parser.push_back(new File_Zip());
    #endif

    #if defined(MEDIAINFO_OTHER_YES)
        Parser.push_back(new File_Other());
    #endif
}

//---------------------------------------------------------------------------
File__MultipleParsing::~File__MultipleParsing()
{
    for (size_t Pos=0; Pos<Parser.size(); Pos++)
        delete Parser[Pos]; //Parser[Pos]=NULL
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File__MultipleParsing::Streams_Finish()
{
    if (Parser.size()!=1)
        return;

    Parser[0]->Open_Buffer_Finalize();
    #if MEDIAINFO_TRACE
        Details=Parser[0]->Details;
    #endif //MEDIAINFO_TRACE
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File__MultipleParsing::Read_Buffer_Init()
{
    //Parsing
    for (size_t Pos=0; Pos<Parser.size(); Pos++)
    {
        //Parsing
        #if MEDIAINFO_TRACE
            Parser[Pos]->Init(Config, Details, Stream, Stream_More);
        #else //MEDIAINFO_TRACE
            Parser[Pos]->Init(Config, Stream, Stream_More);
        #endif //MEDIAINFO_TRACE
        Parser[Pos]->File_Name=File_Name;
        Parser[Pos]->Open_Buffer_Init(File_Size);
    }
}

//---------------------------------------------------------------------------
void File__MultipleParsing::Read_Buffer_Unsynched()
{
    //Parsing
    for (size_t Pos=0; Pos<Parser.size(); Pos++)
        Parser[Pos]->Open_Buffer_Unsynch();
}

//---------------------------------------------------------------------------
void File__MultipleParsing::Read_Buffer_Continue()
{
    //Parsing
    for (size_t Pos=0; Pos<Parser.size(); Pos++)
    {
        //Parsing
        Parser[Pos]->Open_Buffer_Continue(Buffer+Buffer_Offset, (size_t)Element_Size);
        if (File_Offset+Buffer_Size==File_Size)
            Parser[Pos]->Open_Buffer_Finalize();

        //Testing if the parser failed
        if (Parser[Pos]->Status[IsFinished] && !Parser[Pos]->Status[IsAccepted])
        {
            delete Parser[Pos];
            Parser.erase(Parser.begin()+Pos);
            Pos--; //for the next position

            if (Parser.empty())
            {
                File__Analyze* Temp=new File_Unknown(); Parser.push_back(Temp);
                Read_Buffer_Init();
            }
        }
        else
        {
            //If Parser is found, erasing all the other parsers
            if (Parser.size()>1 && Parser[Pos]->Status[IsAccepted])
            {
                File__Analyze* Temp=Parser[Pos];
                for (size_t To_Delete_Pos=0; To_Delete_Pos<Parser.size(); To_Delete_Pos++)
                    if (To_Delete_Pos!=Pos)
                        delete Parser[To_Delete_Pos]; //Parser[Pos]=NULL
                Parser.clear();
                Parser.push_back(Temp);
                Pos=0;
            }

            if (Parser.size()==1)
            {
                //Status
                if (!Status[IsAccepted] && Parser[Pos]->Status[IsAccepted])
                    Status[IsAccepted]=true;
                if (!Status[IsFilled] && Parser[Pos]->Status[IsFilled])
                    Status[IsFilled]=true;
                if (!Status[IsUpdated] && Parser[Pos]->Status[IsUpdated])
                    Status[IsUpdated]=true;
                if (!Status[IsFinished] && Parser[Pos]->Status[IsFinished])
                    Status[IsFinished]=true;

                //Positionning if requested
                if (Parser[0]->File_GoTo!=(int64u)-1)
                   File_GoTo=Parser[0]->File_GoTo;
            }
        }
    }
}

} //NameSpace
