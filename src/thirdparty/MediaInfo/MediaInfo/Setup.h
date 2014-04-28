/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
// All compilation definitions
// Helpers for compilers (precompilation)
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_SetupH
#define MediaInfo_SetupH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//Needed in the whole library
#include "ZenLib/Conf.h"

//***************************************************************************
// General configuration
//***************************************************************************

//---------------------------------------------------------------------------
// Legacy
#if defined(MEDIAINFO_MINIMIZESIZE)
    #if !defined (MEDIAINFO_TRACE_NO) && !defined (MEDIAINFO_TRACE_YES)
        #define MEDIAINFO_TRACE_NO
    #endif
    //#if !defined (MEDIAINFO_FILTER_NO) && !defined (MEDIAINFO_FILTER_YES)
    //    #define MEDIAINFO_FILTER_NO
    //#endif
    //#if !defined (MEDIAINFO_DUPLICATE_NO) && !defined (MEDIAINFO_DUPLICATE_YES)
    //    #define MEDIAINFO_DUPLICATE_NO
    //#endif
    #if !defined (MEDIAINFO_MACROBLOCKS_NO) && !defined (MEDIAINFO_MACROBLOCKS_YES)
        #define MEDIAINFO_MACROBLOCKS_NO
    #endif
    #if !defined (MEDIAINFO_NEXTPACKET_NO) && !defined (MEDIAINFO_NEXTPACKET_YES)
        #define MEDIAINFO_NEXTPACKET_NO
    #endif
    #if !defined (MEDIAINFO_SEEK_NO) && !defined (MEDIAINFO_SEEK_YES)
        #define MEDIAINFO_SEEK_NO
    #endif
    #if !defined (MEDIAINFO_EVENTS_NO) && !defined (MEDIAINFO_EVENTS_YES)
        #define MEDIAINFO_EVENTS_NO
    #endif
    #if !defined (MEDIAINFO_DEMUX_NO) && !defined (MEDIAINFO_DEMUX_YES)
        #define MEDIAINFO_DEMUX_NO
    #endif
    #if !defined (MEDIAINFO_IBI_NO) && !defined (MEDIAINFO_IBI_YES)
        #define MEDIAINFO_IBI_NO
    #endif
#endif
#if defined(MEDIAINFO_EVENTS)
    #undef MEDIAINFO_EVENTS
    #if !defined (MEDIAINFO_EVENTS_NO) && !defined (MEDIAINFO_EVENTS_YES)
        #define MEDIAINFO_EVENTS_YES
    #endif
#endif

//---------------------------------------------------------------------------
// Special configurations
#if defined(MEDIAINFO_MINIMAL_YES)
    #if !defined (MEDIAINFO_TRACE_NO) && !defined (MEDIAINFO_TRACE_YES)
        #define MEDIAINFO_TRACE_NO
    #endif
    #if !defined (MEDIAINFO_FILTER_NO) && !defined (MEDIAINFO_FILTER_YES)
        #define MEDIAINFO_FILTER_NO
    #endif
    #if !defined (MEDIAINFO_DUPLICATE_NO) && !defined (MEDIAINFO_DUPLICATE_YES)
        #define MEDIAINFO_DUPLICATE_NO
    #endif
    #if !defined (MEDIAINFO_MACROBLOCKS_NO) && !defined (MEDIAINFO_MACROBLOCKS_YES)
        #define MEDIAINFO_MACROBLOCKS_NO
    #endif
    #if !defined (MEDIAINFO_NEXTPACKET_NO) && !defined (MEDIAINFO_NEXTPACKET_YES)
        #define MEDIAINFO_NEXTPACKET_NO
    #endif
    #if !defined (MEDIAINFO_SEEK_NO) && !defined (MEDIAINFO_SEEK_YES)
        #define MEDIAINFO_SEEK_NO
    #endif
    #if !defined (MEDIAINFO_EVENTS_NO) && !defined (MEDIAINFO_EVENTS_YES)
        #define MEDIAINFO_EVENTS_NO
    #endif
    #if !defined (MEDIAINFO_DEMUX_NO) && !defined (MEDIAINFO_DEMUX_YES)
        #define MEDIAINFO_DEMUX_NO
    #endif
    #if !defined (MEDIAINFO_IBI_NO) && !defined (MEDIAINFO_IBI_YES)
        #define MEDIAINFO_IBI_NO
    #endif
    #if !defined (MEDIAINFO_DIRECTORY_NO) && !defined (MEDIAINFO_DIRECTORY_YES)
        #define MEDIAINFO_DIRECTORY_NO
    #endif
    #if !defined (MEDIAINFO_LIBCURL_NO) && !defined (MEDIAINFO_LIBCURL_YES)
        #define MEDIAINFO_LIBCURL_NO
    #endif
    #if !defined (MEDIAINFO_LIBMMS_NO) && !defined (MEDIAINFO_LIBMM_YES)
        #define MEDIAINFO_LIBMMS_NO
    #endif
    #if !defined (MEDIAINFO_DVDIF_ANALYZE_NO) && !defined (MEDIAINFO_DVDIF_ANALYZE_YES)
        #define MEDIAINFO_DVDIF_ANALYZE_NO
    #endif
    #if !defined (MEDIAINFO_MPEGTS_DUPLICATE_NO) && !defined (MEDIAINFO_MPEGTS_DUPLICATE_YES)
        #define MEDIAINFO_MPEGTS_DUPLICATE_NO
    #endif
    #if !defined (MEDIAINFO_READTHREAD_NO) && !defined (MEDIAINFO_READTHREAD_YES)
        #define MEDIAINFO_READTHREAD_NO
    #endif
#endif

//---------------------------------------------------------------------------
// Optional features
#if !defined(MEDIAINFO_TRACE)
    #if defined(MEDIAINFO_TRACE_NO) && defined(MEDIAINFO_TRACE_YES)
        #undef MEDIAINFO_TRACE_NO //MEDIAINFO_TRACE_YES has priority
    #endif
    #if defined(MEDIAINFO_TRACE_NO)
        #define MEDIAINFO_TRACE 0
    #else
        #define MEDIAINFO_TRACE 1
    #endif
#endif
#if !defined(MEDIAINFO_FILTER)
    #if defined(MEDIAINFO_FILTER_NO) && defined(MEDIAINFO_FILTER_YES)
        #undef MEDIAINFO_FILTER_NO //MEDIAINFO_FILTER_YES has priority
    #endif
    #if defined(MEDIAINFO_FILTER_NO)
        #define MEDIAINFO_FILTER 0
    #else
        #define MEDIAINFO_FILTER 1
    #endif
#endif
#if !defined(MEDIAINFO_DUPLICATE)
    #if defined(MEDIAINFO_DUPLICATE_NO) && defined(MEDIAINFO_DUPLICATE_YES)
        #undef MEDIAINFO_DUPLICATE_NO //MEDIAINFO_DUPLICATE_YES has priority
    #endif
    #if defined(MEDIAINFO_DUPLICATE_NO)
        #define MEDIAINFO_DUPLICATE 0
    #else
        #define MEDIAINFO_DUPLICATE 1
    #endif
#endif
#if !defined(MEDIAINFO_MACROBLOCKS)
    #if defined(MEDIAINFO_MACROBLOCKS_NO) && defined(MEDIAINFO_MACROBLOCKS_YES)
        #undef MEDIAINFO_MACROBLOCKS_NO //MEDIAINFO_MACROBLOCKS_YES has priority
    #endif
    #if defined(MEDIAINFO_MACROBLOCKS_NO)
        #define MEDIAINFO_MACROBLOCKS 0
    #else
        #define MEDIAINFO_MACROBLOCKS 1
    #endif
#endif
#if !defined(MEDIAINFO_NEXTPACKET)
    #if defined(MEDIAINFO_NEXTPACKET_NO) && defined(MEDIAINFO_NEXTPACKET_YES)
        #undef MEDIAINFO_NEXTPACKET_NO //MEDIAINFO_NEXTPACKET_YES has priority
    #endif
    #if defined(MEDIAINFO_NEXTPACKET_NO)
        #define MEDIAINFO_NEXTPACKET 0
    #else
        #define MEDIAINFO_NEXTPACKET 1
    #endif
#endif
#if !defined(MEDIAINFO_SEEK)
    #if defined(MEDIAINFO_SEEK_NO) && defined(MEDIAINFO_SEEK_YES)
        #undef MEDIAINFO_SEEK_NO //MEDIAINFO_SEEK_YES has priority
    #endif
    #if defined(MEDIAINFO_SEEK_NO)
        #define MEDIAINFO_SEEK 0
    #else
        #define MEDIAINFO_SEEK 1
    #endif
#endif
#if !defined(MEDIAINFO_EVENTS)
    #if defined(MEDIAINFO_EVENTS_NO) && defined(MEDIAINFO_EVENTS_YES)
        #undef MEDIAINFO_EVENTS_NO //MEDIAINFO_EVENTS_YES has priority
    #endif
    #if defined(MEDIAINFO_EVENTS_NO)
        #define MEDIAINFO_EVENTS 0
    #else
        #define MEDIAINFO_EVENTS 1
    #endif
#endif
#if !defined(MEDIAINFO_ADVANCED)
    #if defined(MEDIAINFO_ADVANCED_NO) && defined(MEDIAINFO_ADVANCED_YES)
        #undef MEDIAINFO_ADVANCED_NO //MEDIAINFO_ADVANCED_YES has priority
    #endif
    #if defined(MEDIAINFO_ADVANCED_NO)
        #define MEDIAINFO_ADVANCED 0
    #else
        #define MEDIAINFO_ADVANCED 1
    #endif
#endif
#if !defined(MEDIAINFO_MD5)
    #if defined(MEDIAINFO_MD5_NO) && defined(MEDIAINFO_MD5_YES)
        #undef MEDIAINFO_MD5_NO //MEDIAINFO_MD5_YES has priority
    #endif
    #if defined(MEDIAINFO_MD5_NO)
        #define MEDIAINFO_MD5 0
    #else
        #define MEDIAINFO_MD5 1
    #endif
#endif
#if !defined(MEDIAINFO_DEMUX)
    #if !defined(MEDIAINFO_DEMUX_NO) && !defined(MEDIAINFO_DEMUX_YES) && !MEDIAINFO_EVENTS
        #define MEDIAINFO_DEMUX_NO //MEDIAINFO_DEMUX is disabled by default if MEDIAINFO_EVENTS is set to 0
    #endif
    #if defined(MEDIAINFO_DEMUX_NO) && defined(MEDIAINFO_DEMUX_YES)
        #undef MEDIAINFO_DEMUX_NO //MEDIAINFO_DEMUX_YES has priority
    #endif
    #if defined(MEDIAINFO_DEMUX_NO)
        #define MEDIAINFO_DEMUX 0
    #else
        #define MEDIAINFO_DEMUX 1
    #endif
#endif
#if MEDIAINFO_DEMUX && !MEDIAINFO_EVENTS
    pragma error MEDIAINFO_DEMUX can be set to 1 only if MEDIAINFO_EVENTS is set to 1
#endif
#if !defined(MEDIAINFO_IBI)
    #if defined(MEDIAINFO_IBI_NO) && defined(MEDIAINFO_IBI_YES)
        #undef MEDIAINFO_IBI_NO //MEDIAINFO_IBI_YES has priority
    #endif
    #if defined(MEDIAINFO_IBI_NO)
        #define MEDIAINFO_IBI 0
    #else
        #define MEDIAINFO_IBI 1
    #endif
#endif
#if !defined(MEDIAINFO_READTHREAD)
    #if defined(MEDIAINFO_READTHREAD_NO) && defined(MEDIAINFO_READTHREAD_YES)
        #undef MEDIAINFO_READTHREAD_NO //MEDIAINFO_READTHREAD_YES has priority
    #endif
    #if defined(MEDIAINFO_READTHREAD_NO) || !defined(WINDOWS) //Currently supported only on Windows TODO: add support of non Windows OS
        #define MEDIAINFO_READTHREAD 0
    #else
        #define MEDIAINFO_READTHREAD 1
    #endif
#endif

//***************************************************************************
// Precise configuration
//***************************************************************************

//---------------------------------------------------------------------------
// Readers
#if !defined(MEDIAINFO_READER_NO) && !defined(MEDIAINFO_DIRECTORY_NO) && !defined(MEDIAINFO_DIRECTORY_YES)
    #define MEDIAINFO_DIRECTORY_YES
#endif
#if !defined(MEDIAINFO_READER_NO) && !defined(MEDIAINFO_FILE_NO) && !defined(MEDIAINFO_FILE_YES)
    #define MEDIAINFO_FILE_YES
#endif
#if !defined(MEDIAINFO_READER_NO) && !defined(MEDIAINFO_LIBCURL_NO) && !defined(MEDIAINFO_LIBCURL_YES)
    #define MEDIAINFO_LIBCURL_YES
#endif
#if !defined(MEDIAINFO_READER_NO) && !defined(MEDIAINFO_LIBMMS_NO) && !defined(MEDIAINFO_LIBMMS_YES)
    #ifndef WINDOWS
        #define MEDIAINFO_LIBMMS_YES
    #endif //WINDOWS
#endif

//---------------------------------------------------------------------------
// All in one for no parsers
#if defined(MEDIAINFO_ALL_NO) && !defined(MEDIAINFO_MULTI_NO)
    #define MEDIAINFO_MULTI_NO
#endif
#if defined(MEDIAINFO_ALL_NO) && !defined(MEDIAINFO_VIDEO_NO)
    #define MEDIAINFO_VIDEO_NO
#endif
#if defined(MEDIAINFO_ALL_NO) && !defined(MEDIAINFO_AUDIO_NO)
    #define MEDIAINFO_AUDIO_NO
#endif
#if defined(MEDIAINFO_ALL_NO) && !defined(MEDIAINFO_TEXT_NO)
    #define MEDIAINFO_TEXT_NO
#endif
#if defined(MEDIAINFO_ALL_NO) && !defined(MEDIAINFO_IMAGE_NO)
    #define MEDIAINFO_IMAGE_NO
#endif
#if defined(MEDIAINFO_ALL_NO) && !defined(MEDIAINFO_ARCHIVE_NO)
    #define MEDIAINFO_ARCHIVE_NO
#endif
#if defined(MEDIAINFO_ALL_NO) && !defined(MEDIAINFO_TAG_NO)
    #define MEDIAINFO_TAG_NO
#endif

//---------------------------------------------------------------------------
// Multiple
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_REFERENCES_NO) && !defined(MEDIAINFO_REFERENCES_YES)
    #define MEDIAINFO_REFERENCES_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_AAF_NO) && !defined(MEDIAINFO_AAF_YES)
    #define MEDIAINFO_AAF_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_ANCILLARY_NO) && !defined(MEDIAINFO_ANCILLARY_YES)
    #define MEDIAINFO_ANCILLARY_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_BDAV_NO) && !defined(MEDIAINFO_BDAV_YES)
    #define MEDIAINFO_BDAV_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_BDMV_NO) && !defined(MEDIAINFO_BDMV_YES)
    #define MEDIAINFO_BDMV_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_CDXA_NO) && !defined(MEDIAINFO_CDXA_YES)
    #define MEDIAINFO_CDXA_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_DASHMPD_NO) && !defined(MEDIAINFO_DASHMPD_YES)
    #define MEDIAINFO_DASHMPD_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_DCP_NO) && !defined(MEDIAINFO_DCP_YES)
    #define MEDIAINFO_DCP_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_DVDIF_NO) && !defined(MEDIAINFO_DVDIF_YES)
    #define MEDIAINFO_DVDIF_YES
#endif
#if defined(MEDIAINFO_DVDIF_YES) && !defined(MEDIAINFO_DVDIF_ANALYZE_NO) && !defined(MEDIAINFO_DVDIF_ANALYZE_YES)
    #define MEDIAINFO_DVDIF_ANALYZE_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_DVDV_NO) && !defined(MEDIAINFO_DVDV_YES)
    #define MEDIAINFO_DVDV_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_DXW_NO) && !defined(MEDIAINFO_DXW_YES)
    #define MEDIAINFO_DXW_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_FLV_NO) && !defined(MEDIAINFO_FLV_YES)
    #define MEDIAINFO_FLV_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_GXF_NO) && !defined(MEDIAINFO_GXF_YES)
    #define MEDIAINFO_GXF_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_HDSF4M_NO) && !defined(MEDIAINFO_HDSF4M_YES)
    #define MEDIAINFO_HDSF4M_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_HLS_NO) && !defined(MEDIAINFO_HLS_YES)
    #define MEDIAINFO_HLS_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_ISM_NO) && !defined(MEDIAINFO_ISM_YES)
    #define MEDIAINFO_ISM_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_IVF_NO) && !defined(MEDIAINFO_IVF_YES)
    #define MEDIAINFO_IVF_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_IBI_NO) && !defined(MEDIAINFO_IBI_YES)
    #define MEDIAINFO_IBI_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_LXF_NO) && !defined(MEDIAINFO_LXF_YES)
    #define MEDIAINFO_LXF_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_MK_NO) && !defined(MEDIAINFO_MK_YES)
    #define MEDIAINFO_MK_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_MPEG4_NO) && !defined(MEDIAINFO_MPEG4_YES)
    #define MEDIAINFO_MPEG4_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_MPEGPS_NO) && !defined(MEDIAINFO_MPEGPS_YES)
    #define MEDIAINFO_MPEGPS_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_MPEGTS_NO) && !defined(MEDIAINFO_MPEGTS_YES)
    #define MEDIAINFO_MPEGTS_YES
#endif
#if defined(MEDIAINFO_MPEGTS_YES) && !defined(MEDIAINFO_MPEGTS_PCR_NO) && !defined(MEDIAINFO_MPEGTS_PCR_YES)
    #define MEDIAINFO_MPEGTS_PCR_YES
#endif
#if defined(MEDIAINFO_MPEGTS_YES) && !defined(MEDIAINFO_MPEGTS_PESTIMESTAMP_NO) && !defined(MEDIAINFO_MPEGTS_PESTIMESTAMP_YES)
    #define MEDIAINFO_MPEGTS_PESTIMESTAMP_YES
#endif
#if defined(MEDIAINFO_MPEGTS_YES) && !defined(MEDIAINFO_MPEGTS_DUPLICATE_NO) && !defined(MEDIAINFO_MPEGTS_DUPLICATE_YES)
    #define MEDIAINFO_MPEGTS_DUPLICATE_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_MXF_NO) && !defined(MEDIAINFO_MXF_YES)
    #define MEDIAINFO_MXF_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_NUT_NO) && !defined(MEDIAINFO_NUT_YES)
    #define MEDIAINFO_NUT_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_OGG_NO) && !defined(MEDIAINFO_OGG_YES)
    #define MEDIAINFO_OGG_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_P2_NO) && !defined(MEDIAINFO_P2_YES)
    #define MEDIAINFO_P2_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_PMP_NO) && !defined(MEDIAINFO_PMP_YES)
    #define MEDIAINFO_PMP_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_PTX_NO) && !defined(MEDIAINFO_PTX_YES)
    #define MEDIAINFO_PTX_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_RIFF_NO) && !defined(MEDIAINFO_RIFF_YES)
    #define MEDIAINFO_RIFF_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_RM_NO) && !defined(MEDIAINFO_RM_YES)
    #define MEDIAINFO_RM_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_SEQUENCEINFO_NO) && !defined(MEDIAINFO_SEQUENCEINFO_YES)
    #define MEDIAINFO_SEQUENCEINFO_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_SKM_NO) && !defined(MEDIAINFO_SKM_YES)
    #define MEDIAINFO_SKM_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_SWF_NO) && !defined(MEDIAINFO_SWF_YES)
    #define MEDIAINFO_SWF_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_TSP_NO) && !defined(MEDIAINFO_TSP_YES)
    #define MEDIAINFO_TSP_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_UMF_NO) && !defined(MEDIAINFO_UMF_YES)
    #define MEDIAINFO_UMF_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_WM_NO) && !defined(MEDIAINFO_WM_YES)
    #define MEDIAINFO_WM_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_XDCAM_NO) && !defined(MEDIAINFO_XDCAM_YES)
    #define MEDIAINFO_XDCAM_YES
#endif
#if !defined(MEDIAINFO_MULTI_NO) && !defined(MEDIAINFO_DPG_NO) && !defined(MEDIAINFO_DPG_YES)
    #define MEDIAINFO_DPG_YES
#endif

//---------------------------------------------------------------------------
// Video
#if !defined(MEDIAINFO_VIDEO_NO) && !defined(MEDIAINFO_AIC_NO) && !defined(MEDIAINFO_AIC_YES)
    #define MEDIAINFO_AIC_YES
#endif
#if !defined(MEDIAINFO_VIDEO_NO) && !defined(MEDIAINFO_AFDBARDATA_NO) && !defined(MEDIAINFO_AFDBARDATA_YES)
    #define MEDIAINFO_AFDBARDATA_YES
#endif
#if !defined(MEDIAINFO_VIDEO_NO) && !defined(MEDIAINFO_AVC_NO) && !defined(MEDIAINFO_AVC_YES)
    #define MEDIAINFO_AVC_YES
#endif
#if !defined(MEDIAINFO_VIDEO_NO) && !defined(MEDIAINFO_AVSV_NO) && !defined(MEDIAINFO_AVSV_YES)
    #define MEDIAINFO_AVSV_YES
#endif
#if !defined(MEDIAINFO_VIDEO_NO) && !defined(MEDIAINFO_CANOPUS_NO) && !defined(MEDIAINFO_CANOPUS_YES)
    #define MEDIAINFO_CANOPUS_YES
#endif
#if !defined(MEDIAINFO_VIDEO_NO) && !defined(MEDIAINFO_DIRAC_NO) && !defined(MEDIAINFO_DIRAC_YES)
    #define MEDIAINFO_DIRAC_YES
#endif
#if !defined(MEDIAINFO_VIDEO_NO) && !defined(MEDIAINFO_FLIC_NO) && !defined(MEDIAINFO_FLIC_YES)
    #define MEDIAINFO_FLIC_YES
#endif
#if !defined(MEDIAINFO_VIDEO_NO) && !defined(MEDIAINFO_FRAPS_NO) && !defined(MEDIAINFO_FRAPS_YES)
    #define MEDIAINFO_FRAPS_YES
#endif
#if !defined(MEDIAINFO_VIDEO_NO) && !defined(MEDIAINFO_H263_NO) && !defined(MEDIAINFO_H263_YES)
    #define MEDIAINFO_H263_YES
#endif
#if !defined(MEDIAINFO_VIDEO_NO) && !defined(MEDIAINFO_HEVC_NO) && !defined(MEDIAINFO_HEVC_YES)
    #define MEDIAINFO_HEVC_YES
#endif
#if !defined(MEDIAINFO_VIDEO_NO) && !defined(MEDIAINFO_LAGARITH_NO) && !defined(MEDIAINFO_LAGARITH_YES)
    #define MEDIAINFO_LAGARITH_YES
#endif
#if !defined(MEDIAINFO_VIDEO_NO) && !defined(MEDIAINFO_MPEG4V_NO) && !defined(MEDIAINFO_MPEG4V_YES)
    #define MEDIAINFO_MPEG4V_YES
#endif
#if !defined(MEDIAINFO_VIDEO_NO) && !defined(MEDIAINFO_MPEGV_NO) && !defined(MEDIAINFO_MPEGV_YES)
    #define MEDIAINFO_MPEGV_YES
#endif
#if !defined(MEDIAINFO_VIDEO_NO) && !defined(MEDIAINFO_PRORES_NO) && !defined(MEDIAINFO_PRORES_YES)
    #define MEDIAINFO_PRORES_YES
#endif
#if !defined(MEDIAINFO_VIDEO_NO) && !defined(MEDIAINFO_VC1_NO) && !defined(MEDIAINFO_VC1_YES)
    #define MEDIAINFO_VC1_YES
#endif
#if !defined(MEDIAINFO_VIDEO_NO) && !defined(MEDIAINFO_VC3_NO) && !defined(MEDIAINFO_VC3_YES)
    #define MEDIAINFO_VC3_YES
#endif
#if !defined(MEDIAINFO_VIDEO_NO) && !defined(MEDIAINFO_TIMECODE_NO) && !defined(MEDIAINFO_TIMECODE_YES)
    #define MEDIAINFO_TIMECODE_YES
#endif
#if !defined(MEDIAINFO_VIDEO_NO) && !defined(MEDIAINFO_THEORA_NO) && !defined(MEDIAINFO_THEORA_YES)
    #define MEDIAINFO_THEORA_YES
#endif
#if !defined(MEDIAINFO_VIDEO_NO) && !defined(MEDIAINFO_VP8_NO) && !defined(MEDIAINFO_VP8_YES)
    #define MEDIAINFO_VP8_YES
#endif
#if !defined(MEDIAINFO_VIDEO_NO) && !defined(MEDIAINFO_Y4M_NO) && !defined(MEDIAINFO_Y4M_YES)
    #define MEDIAINFO_Y4M_YES
#endif

//---------------------------------------------------------------------------
// Audio
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_AAC_NO) && !defined(MEDIAINFO_AAC_YES)
    #define MEDIAINFO_AAC_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_AC3_NO) && !defined(MEDIAINFO_AC3_YES)
    #define MEDIAINFO_AC3_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_ADPCM_NO) && !defined(MEDIAINFO_ADPCM_YES)
    #define MEDIAINFO_ADPCM_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_ALS_NO) && !defined(MEDIAINFO_ALS_YES)
    #define MEDIAINFO_ALS_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_LATM_NO) && !defined(MEDIAINFO_LATM_YES)
    #define MEDIAINFO_LATM_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_AMR_NO) && !defined(MEDIAINFO_AMR_YES)
    #define MEDIAINFO_AMR_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_AMV_NO) && !defined(MEDIAINFO_AMV_YES)
    #define MEDIAINFO_AMV_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_APE_NO) && !defined(MEDIAINFO_APE_YES)
    #define MEDIAINFO_APE_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_AU_NO) && !defined(MEDIAINFO_AU_YES)
    #define MEDIAINFO_AU_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_CELT_NO) && !defined(MEDIAINFO_CELT_YES)
    #define MEDIAINFO_CELT_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_DOLBYE_NO) && !defined(MEDIAINFO_DOLBYE_YES)
    #define MEDIAINFO_DOLBYE_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_DTS_NO) && !defined(MEDIAINFO_DTS_YES)
    #define MEDIAINFO_DTS_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_FLAC_NO) && !defined(MEDIAINFO_FLAC_YES)
    #define MEDIAINFO_FLAC_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_IT_NO) && !defined(MEDIAINFO_IT_YES)
    #define MEDIAINFO_IT_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_LA_NO) && !defined(MEDIAINFO_LA_YES)
    #define MEDIAINFO_LA_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_MIDO_NO) && !defined(MEDIAINFO_MIDO_YES)
    #define MEDIAINFO_MIDI_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_MOD_NO) && !defined(MEDIAINFO_MOD_YES)
    #define MEDIAINFO_MOD_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_MPC_NO) && !defined(MEDIAINFO_MPC_YES)
    #define MEDIAINFO_MPC_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_MPCSV8_NO) && !defined(MEDIAINFO_MPCSV8_YES)
    #define MEDIAINFO_MPCSV8_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_MPEGA_NO) && !defined(MEDIAINFO_MPEGA_YES)
    #define MEDIAINFO_MPEGA_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_OPENMG_NO) && !defined(MEDIAINFO_OPENMG_YES)
    #define MEDIAINFO_OPENMG_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_OPUS_NO) && !defined(MEDIAINFO_OPUS_YES)
    #define MEDIAINFO_OPUS_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_PCM_NO) && !defined(MEDIAINFO_PCM_YES)
    #define MEDIAINFO_PCM_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_PCMM2TS_NO) && !defined(MEDIAINFO_PCMM2TS_YES)
    #define MEDIAINFO_PCMM2TS_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_PCMVOB_NO) && !defined(MEDIAINFO_PCMVOB_YES)
    #define MEDIAINFO_PCMVOB_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_PS2A_NO) && !defined(MEDIAINFO_PS2A_YES)
    #define MEDIAINFO_PS2A_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_RKAU_NO) && !defined(MEDIAINFO_RKAU_YES)
    #define MEDIAINFO_RKAU_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_S3M_NO) && !defined(MEDIAINFO_S3M_YES)
    #define MEDIAINFO_S3M_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_SPEEX_NO) && !defined(MEDIAINFO_SPEEX_YES)
    #define MEDIAINFO_SPEEX_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_SMPTEST0302_NO) && !defined(MEDIAINFO_SMPTEST0302_YES)
    #define MEDIAINFO_SMPTEST0302_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_SMPTEST0331_NO) && !defined(MEDIAINFO_SMPTEST0331_YES)
    #define MEDIAINFO_SMPTEST0331_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_SMPTEST0337_NO) && !defined(MEDIAINFO_SMPTEST0337_YES)
    #define MEDIAINFO_SMPTEST0337_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_TAK_NO) && !defined(MEDIAINFO_TAK_YES)
    #define MEDIAINFO_TAK_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_TTA_NO) && !defined(MEDIAINFO_TTA_YES)
    #define MEDIAINFO_TTA_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_TWINVQ_NO) && !defined(MEDIAINFO_TWINVQ_YES)
    #define MEDIAINFO_TWINVQ_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_VORBIS_NO) && !defined(MEDIAINFO_VORBIS_YES)
    #define MEDIAINFO_VORBIS_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_WVPK_NO) && !defined(MEDIAINFO_WVPK_YES)
    #define MEDIAINFO_WVPK_YES
#endif
#if !defined(MEDIAINFO_AUDIO_NO) && !defined(MEDIAINFO_XM_NO) && !defined(MEDIAINFO_XM_YES)
    #define MEDIAINFO_XM_YES
#endif

//---------------------------------------------------------------------------
// Text
#if !defined(MEDIAINFO_TEXT_NO) && !defined(MEDIAINFO_ARIBSTDB24B37_NO) && !defined(MEDIAINFO_ARIBSTDB24B37_YES)
    #define MEDIAINFO_ARIBSTDB24B37_YES
#endif
#if !defined(MEDIAINFO_TEXT_NO) && !defined(MEDIAINFO_CDP_NO) && !defined(MEDIAINFO_CDP_YES)
    #define MEDIAINFO_CDP_YES
#endif
#if !defined(MEDIAINFO_TEXT_NO) && !defined(MEDIAINFO_CMML_NO) && !defined(MEDIAINFO_CMML_YES)
    #define MEDIAINFO_CMML_YES
#endif
#if !defined(MEDIAINFO_TEXT_NO) && !defined(MEDIAINFO_DVBSUBTITLE_NO) && !defined(MEDIAINFO_DVBSUBTITLE_YES)
    #define MEDIAINFO_DVBSUBTITLE_YES
#endif
#if !defined(MEDIAINFO_TEXT_NO) && !defined(MEDIAINFO_DTVCCTRANSPORT_NO) && !defined(MEDIAINFO_DTVCCTRANSPORT_YES)
    #define MEDIAINFO_DTVCCTRANSPORT_YES
#endif
#if !defined(MEDIAINFO_TEXT_NO) && !defined(MEDIAINFO_EIA608_NO) && !defined(MEDIAINFO_EIA608_YES)
    #define MEDIAINFO_EIA608_YES
#endif
#if !defined(MEDIAINFO_TEXT_NO) && !defined(MEDIAINFO_EIA708_NO) && !defined(MEDIAINFO_EIA708_YES)
    #define MEDIAINFO_EIA708_YES
#endif
#if !defined(MEDIAINFO_TEXT_NO) && !defined(MEDIAINFO_N19_NO) && !defined(MEDIAINFO_N19_YES)
    #define MEDIAINFO_N19_YES
#endif
#if !defined(MEDIAINFO_TEXT_NO) && !defined(MEDIAINFO_KATE_NO) && !defined(MEDIAINFO_KATE_YES)
    #define MEDIAINFO_KATE_YES
#endif
#if !defined(MEDIAINFO_TEXT_NO) && !defined(MEDIAINFO_PGS_NO) && !defined(MEDIAINFO_PGS_YES)
    #define MEDIAINFO_PGS_YES
#endif
#if !defined(MEDIAINFO_TEXT_NO) && !defined(MEDIAINFO_SCC_NO) && !defined(MEDIAINFO_SCC_YES)
    #define MEDIAINFO_SCC_YES
#endif
#if !defined(MEDIAINFO_TEXT_NO) && !defined(MEDIAINFO_SCTE20_NO) && !defined(MEDIAINFO_SCTE20_YES)
    #define MEDIAINFO_SCTE20_YES
#endif
#if !defined(MEDIAINFO_TEXT_NO) && !defined(MEDIAINFO_SUBRIP_NO) && !defined(MEDIAINFO_SUBRIP_YES)
    #define MEDIAINFO_SUBRIP_YES
#endif
#if !defined(MEDIAINFO_TEXT_NO) && !defined(MEDIAINFO_TELETEXT_NO) && !defined(MEDIAINFO_TELETEXT_YES)
    #define MEDIAINFO_TELETEXT_YES
#endif
#if !defined(MEDIAINFO_TEXT_NO) && !defined(MEDIAINFO_TIMEDTEXT_NO) && !defined(MEDIAINFO_TIMEDTEXT_YES)
    #define MEDIAINFO_TIMEDTEXT_YES
#endif
#if !defined(MEDIAINFO_TEXT_NO) && !defined(MEDIAINFO_TTML_NO) && !defined(MEDIAINFO_TTML_YES)
    #define MEDIAINFO_TTML_YES
#endif
#if !defined(MEDIAINFO_TEXT_NO) && !defined(MEDIAINFO_OTHERTEXT_NO) && !defined(MEDIAINFO_OTHERTEXT_YES)
    #define MEDIAINFO_OTHERTEXT_YES
#endif

//---------------------------------------------------------------------------
// Image
#if !defined(MEDIAINFO_IMAGE_NO) && !defined(MEDIAINFO_ARRIRAW_NO) && !defined(MEDIAINFO_ARRIRAW_YES)
    #define MEDIAINFO_ARRIRAW_YES
#endif
#if !defined(MEDIAINFO_IMAGE_NO) && !defined(MEDIAINFO_BMP_NO) && !defined(MEDIAINFO_BMP_YES)
    #define MEDIAINFO_BMP_YES
#endif
#if !defined(MEDIAINFO_IMAGE_NO) && !defined(MEDIAINFO_DDS_NO) && !defined(MEDIAINFO_DDS_YES)
    #define MEDIAINFO_DDS_YES
#endif
#if !defined(MEDIAINFO_IMAGE_NO) && !defined(MEDIAINFO_DPX_NO) && !defined(MEDIAINFO_DPX_YES)
    #define MEDIAINFO_DPX_YES
#endif
#if !defined(MEDIAINFO_IMAGE_NO) && !defined(MEDIAINFO_EXR_NO) && !defined(MEDIAINFO_EXR_YES)
    #define MEDIAINFO_EXR_YES
#endif
#if !defined(MEDIAINFO_IMAGE_NO) && !defined(MEDIAINFO_GIF_NO) && !defined(MEDIAINFO_GIF_YES)
    #define MEDIAINFO_GIF_YES
#endif
#if !defined(MEDIAINFO_IMAGE_NO) && !defined(MEDIAINFO_ICO_NO) && !defined(MEDIAINFO_ICO_YES)
    #define MEDIAINFO_ICO_YES
#endif
#if !defined(MEDIAINFO_IMAGE_NO) && !defined(MEDIAINFO_JPEG_NO) && !defined(MEDIAINFO_JPEG_YES)
    #define MEDIAINFO_JPEG_YES
#endif
#if !defined(MEDIAINFO_IMAGE_NO) && !defined(MEDIAINFO_PCX_NO) && !defined(MEDIAINFO_PCX_YES)
    #define MEDIAINFO_PCX_YES
#endif
#if !defined(MEDIAINFO_IMAGE_NO) && !defined(MEDIAINFO_PNG_NO) && !defined(MEDIAINFO_PNG_YES)
    #define MEDIAINFO_PNG_YES
#endif
#if !defined(MEDIAINFO_IMAGE_NO) && !defined(MEDIAINFO_PSD_NO) && !defined(MEDIAINFO_PSD_YES)
    #define MEDIAINFO_PSD_YES
#endif
#if !defined(MEDIAINFO_IMAGE_NO) && !defined(MEDIAINFO_RLE_NO) && !defined(MEDIAINFO_RLE_YES)
    #define MEDIAINFO_RLE_YES
#endif
#if !defined(MEDIAINFO_IMAGE_NO) && !defined(MEDIAINFO_TIFF_NO) && !defined(MEDIAINFO_TIFF_YES)
    #define MEDIAINFO_TIFF_YES
#endif
#if !defined(MEDIAINFO_IMAGE_NO) && !defined(MEDIAINFO_TGA_NO) && !defined(MEDIAINFO_TGA_YES)
    #define MEDIAINFO_TGA_YES
#endif

//---------------------------------------------------------------------------
// Archive
#if !defined(MEDIAINFO_ARCHIVE_NO) && !defined(MEDIAINFO_7Z_NO) && !defined(MEDIAINFO_7Z_YES)
    #define MEDIAINFO_7Z_YES
#endif
#if !defined(MEDIAINFO_ARCHIVE_NO) && !defined(MEDIAINFO_ACE_NO) && !defined(MEDIAINFO_ACE_YES)
    #define MEDIAINFO_ACE_YES
#endif
#if !defined(MEDIAINFO_ARCHIVE_NO) && !defined(MEDIAINFO_BZIP2_NO) && !defined(MEDIAINFO_BZIP2_YES)
    #define MEDIAINFO_BZIP2_YES
#endif
#if !defined(MEDIAINFO_ARCHIVE_NO) && !defined(MEDIAINFO_ELF_NO) && !defined(MEDIAINFO_ELF_YES)
    #define MEDIAINFO_ELF_YES
#endif
#if !defined(MEDIAINFO_ARCHIVE_NO) && !defined(MEDIAINFO_GZIP_NO) && !defined(MEDIAINFO_GZIP_YES)
    #define MEDIAINFO_GZIP_YES
#endif
#if !defined(MEDIAINFO_ARCHIVE_NO) && !defined(MEDIAINFO_ISO9660_NO) && !defined(MEDIAINFO_ISO9660_YES)
    #define MEDIAINFO_ISO9660_YES
#endif
#if !defined(MEDIAINFO_ARCHIVE_NO) && !defined(MEDIAINFO_MZ_NO) && !defined(MEDIAINFO_MZ_YES)
    #define MEDIAINFO_MZ_YES
#endif
#if !defined(MEDIAINFO_ARCHIVE_NO) && !defined(MEDIAINFO_RAR_NO) && !defined(MEDIAINFO_RAR_YES)
    #define MEDIAINFO_RAR_YES
#endif
#if !defined(MEDIAINFO_ARCHIVE_NO) && !defined(MEDIAINFO_TAR_NO) && !defined(MEDIAINFO_TAR_YES)
    #define MEDIAINFO_TAR_YES
#endif
#if !defined(MEDIAINFO_ARCHIVE_NO) && !defined(MEDIAINFO_ZIP_NO) && !defined(MEDIAINFO_ZIP_YES)
    #define MEDIAINFO_ZIP_YES
#endif

//---------------------------------------------------------------------------
// Tag
#if !defined(MEDIAINFO_TAG_NO) && !defined(MEDIAINFO_TAG_NO) && !defined(MEDIAINFO_TAG_YES)
    #define MEDIAINFO_TAG_YES
#endif
#if !defined(MEDIAINFO_TAG_NO) && !defined(MEDIAINFO_APETAG_NO) && !defined(MEDIAINFO_APETAG_YES)
    #define MEDIAINFO_APETAG_YES
#endif
#if !defined(MEDIAINFO_TAG_NO) && !defined(MEDIAINFO_ID3_NO) && !defined(MEDIAINFO_ID3_YES)
    #define MEDIAINFO_ID3_YES
#endif
#if !defined(MEDIAINFO_TAG_NO) && !defined(MEDIAINFO_ID3V2_NO) && !defined(MEDIAINFO_ID3V2_YES)
    #define MEDIAINFO_ID3V2_YES
#endif
#if !defined(MEDIAINFO_TAG_NO) && !defined(MEDIAINFO_LYRICS3_NO) && !defined(MEDIAINFO_LYRICS3_YES)
    #define MEDIAINFO_LYRICS3_YES
#endif
#if !defined(MEDIAINFO_TAG_NO) && !defined(MEDIAINFO_LYRICS3V2_NO) && !defined(MEDIAINFO_LYRICS3V2_YES)
    #define MEDIAINFO_LYRICS3V2_YES
#endif
#if !defined(MEDIAINFO_TAG_NO) && !defined(MEDIAINFO_VORBISCOM_NO) && !defined(MEDIAINFO_VORBISCOM_YES)
    #define MEDIAINFO_VORBISCOM_YES
#endif

//---------------------------------------------------------------------------
// Other
#if !defined(MEDIAINFO_DUMMY_NO)
    //#define MEDIAINFO_DUMMY_YES
#endif
#if !defined(MEDIAINFO_OTHER_NO)
    #define MEDIAINFO_OTHER_YES
#endif
#if !defined(MEDIAINFO_UNKNOWN_NO)
    #define MEDIAINFO_UNKNOWN_YES
#endif

#endif
