/* 
 * $Id$
 *
 * (C) 2006-2007 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include <math.h>
#include <atlbase.h>
#include <mmreg.h>

#include "PODtypes.h"
#include "avcodec.h"

#include <initguid.h>
#include "MPCVideoDecFilter.h"
#include "VideoDecOutputPin.h"
#include "CpuId.h"

extern "C"
{
	#include "FfmpegContext.h"
}

#include "..\..\..\DSUtil\DSUtil.h"
#include "..\..\..\DSUtil\MediaTypes.h"
#include "..\..\parser\mpegsplitter\MpegSplitter.h"

#include <moreuuids.h>

#include "DXVADecoderH264.h"



/////
#define MAX_SUPPORTED_MODE			5
#define MPCVD_CAPTION				_T("MPC Video decoder")

typedef struct
{
	const int			PicEntryNumber;
	const UINT			ConfigBitstreamRawMin;
	const GUID*			Decoder[MAX_SUPPORTED_MODE];
	const WORD			RestrictedMode[MAX_SUPPORTED_MODE];
} DXVA_PARAMS;

typedef struct
{
  const CLSID*			clsMinorType;
  const enum CodecID	nFFCodec;
  const int				fourcc;
  const DXVA_PARAMS*	DXVAModes;

  int					DXVAModeCount()		
  {
	  if (!DXVAModes) return 0;
	  for (int i=0; i<MAX_SUPPORTED_MODE; i++)
	  {
		  if (DXVAModes->Decoder[i] == &GUID_NULL) return i;
	  }
	  return MAX_SUPPORTED_MODE;
  }
} FFMPEG_CODECS;


// DXVA modes supported for Mpeg2	TODO
DXVA_PARAMS		DXVA_Mpeg2 =
{
	14,		// PicEntryNumber
	1,		// ConfigBitstreamRawMin
	{ &DXVA_ModeMPEG2_A,			&DXVA_ModeMPEG2_C,				&GUID_NULL },
	{ DXVA_RESTRICTED_MODE_MPEG2_A,  DXVA_RESTRICTED_MODE_MPEG2_C,	 0 }
};

// DXVA modes supported for H264
DXVA_PARAMS		DXVA_H264 =
{
	16,		// PicEntryNumber
	2,		// ConfigBitstreamRawMin
	{ &DXVA2_ModeH264_E, &DXVA2_ModeH264_F, &GUID_NULL },
	{ DXVA_RESTRICTED_MODE_H264_E,	 0}
};

// DXVA modes supported for VC1
DXVA_PARAMS		DXVA_VC1 =
{
	14,		// PicEntryNumber
	1,		// ConfigBitstreamRawMin
	{ &DXVA2_ModeVC1_D,				&GUID_NULL },
	{ DXVA_RESTRICTED_MODE_VC1_D,	 0}
};

FFMPEG_CODECS		ffCodecs[] =
{
	// Flash video
	{ &MEDIASUBTYPE_FLV1, CODEC_ID_FLV1, MAKEFOURCC('F','L','V','1'),	NULL },
	{ &MEDIASUBTYPE_flv1, CODEC_ID_FLV1, MAKEFOURCC('f','l','v','1'),	NULL },

	// VP5
	{ &MEDIASUBTYPE_VP50, CODEC_ID_VP5,  MAKEFOURCC('V','P','5','0'),	NULL },
	{ &MEDIASUBTYPE_vp50, CODEC_ID_VP5,  MAKEFOURCC('v','p','5','0'),	NULL },

	// VP6
	{ &MEDIASUBTYPE_VP60, CODEC_ID_VP6,  MAKEFOURCC('V','P','6','0'),	NULL },
	{ &MEDIASUBTYPE_vp60, CODEC_ID_VP6,  MAKEFOURCC('v','p','6','0'),	NULL },
	{ &MEDIASUBTYPE_VP61, CODEC_ID_VP6,  MAKEFOURCC('V','P','6','1'),	NULL },
	{ &MEDIASUBTYPE_vp61, CODEC_ID_VP6,  MAKEFOURCC('v','p','6','1'),	NULL },
	{ &MEDIASUBTYPE_VP62, CODEC_ID_VP6,  MAKEFOURCC('V','P','6','2'),	NULL },
	{ &MEDIASUBTYPE_vp62, CODEC_ID_VP6,  MAKEFOURCC('v','p','6','2'),	NULL },
	{ &MEDIASUBTYPE_FLV4, CODEC_ID_VP6F, MAKEFOURCC('F','L','V','4'),	NULL },
	{ &MEDIASUBTYPE_flv4, CODEC_ID_VP6F, MAKEFOURCC('f','l','v','4'),	NULL },
	{ &MEDIASUBTYPE_VP6F, CODEC_ID_VP6F, MAKEFOURCC('V','P','6','F'),	NULL },
	{ &MEDIASUBTYPE_vp6f, CODEC_ID_VP6F, MAKEFOURCC('v','p','6','f'),	NULL },
	{ &MEDIASUBTYPE_VP6A, CODEC_ID_VP6A, MAKEFOURCC('V','P','6','A'),	NULL },
	{ &MEDIASUBTYPE_vp6a, CODEC_ID_VP6A, MAKEFOURCC('v','p','6','a'),	NULL },

	// Xvid
	{ &MEDIASUBTYPE_XVID, CODEC_ID_MPEG4,  MAKEFOURCC('X','V','I','D'),	NULL },
	{ &MEDIASUBTYPE_xvid, CODEC_ID_MPEG4,  MAKEFOURCC('x','v','i','d'),	NULL },
	{ &MEDIASUBTYPE_XVIX, CODEC_ID_MPEG4,  MAKEFOURCC('X','V','I','X'),	NULL },
	{ &MEDIASUBTYPE_xvix, CODEC_ID_MPEG4,  MAKEFOURCC('x','v','i','x'),	NULL },

	// DivX
	{ &MEDIASUBTYPE_DX50, CODEC_ID_MPEG4,  MAKEFOURCC('D','X','5','0'),	NULL },
	{ &MEDIASUBTYPE_dx50, CODEC_ID_MPEG4,  MAKEFOURCC('d','x','5','0'),	NULL },
	{ &MEDIASUBTYPE_DIVX, CODEC_ID_MPEG4,  MAKEFOURCC('D','I','V','X'),	NULL },
	{ &MEDIASUBTYPE_divx, CODEC_ID_MPEG4,  MAKEFOURCC('d','i','v','x'),	NULL },

	// Other MPEG-4
	{ &MEDIASUBTYPE_MP4V, CODEC_ID_MPEG4,  MAKEFOURCC('M','P','4','V'),	NULL },
	{ &MEDIASUBTYPE_mp4v, CODEC_ID_MPEG4,  MAKEFOURCC('m','p','4','v'),	NULL },
	{ &MEDIASUBTYPE_M4S2, CODEC_ID_MPEG4,  MAKEFOURCC('M','4','S','2'),	NULL },
	{ &MEDIASUBTYPE_m4s2, CODEC_ID_MPEG4,  MAKEFOURCC('m','4','s','2'),	NULL },
	{ &MEDIASUBTYPE_MP4S, CODEC_ID_MPEG4,  MAKEFOURCC('M','P','4','S'),	NULL },
	{ &MEDIASUBTYPE_mp4s, CODEC_ID_MPEG4,  MAKEFOURCC('m','p','4','s'),	NULL },
	{ &MEDIASUBTYPE_3IV1, CODEC_ID_MPEG4,  MAKEFOURCC('3','I','V','1'),	NULL },
	{ &MEDIASUBTYPE_3iv1, CODEC_ID_MPEG4,  MAKEFOURCC('3','i','v','1'),	NULL },
	{ &MEDIASUBTYPE_3IV2, CODEC_ID_MPEG4,  MAKEFOURCC('3','I','V','2'),	NULL },
	{ &MEDIASUBTYPE_3iv2, CODEC_ID_MPEG4,  MAKEFOURCC('3','i','v','2'),	NULL },
	{ &MEDIASUBTYPE_3IVX, CODEC_ID_MPEG4,  MAKEFOURCC('3','I','V','X'),	NULL },
	{ &MEDIASUBTYPE_3ivx, CODEC_ID_MPEG4,  MAKEFOURCC('3','i','v','x'),	NULL },
	{ &MEDIASUBTYPE_BLZ0, CODEC_ID_MPEG4,  MAKEFOURCC('B','L','Z','0'),	NULL },
	{ &MEDIASUBTYPE_blz0, CODEC_ID_MPEG4,  MAKEFOURCC('b','l','z','0'),	NULL },
	{ &MEDIASUBTYPE_DM4V, CODEC_ID_MPEG4,  MAKEFOURCC('D','M','4','V'),	NULL },
	{ &MEDIASUBTYPE_dm4v, CODEC_ID_MPEG4,  MAKEFOURCC('d','m','4','v'),	NULL },
	{ &MEDIASUBTYPE_FFDS, CODEC_ID_MPEG4,  MAKEFOURCC('F','F','D','S'),	NULL },
	{ &MEDIASUBTYPE_ffds, CODEC_ID_MPEG4,  MAKEFOURCC('f','f','d','s'),	NULL },
	{ &MEDIASUBTYPE_FVFW, CODEC_ID_MPEG4,  MAKEFOURCC('F','V','F','W'),	NULL },
	{ &MEDIASUBTYPE_fvfw, CODEC_ID_MPEG4,  MAKEFOURCC('f','v','f','w'),	NULL },
	{ &MEDIASUBTYPE_DXGM, CODEC_ID_MPEG4,  MAKEFOURCC('D','X','G','M'),	NULL },
	{ &MEDIASUBTYPE_dxgm, CODEC_ID_MPEG4,  MAKEFOURCC('d','x','g','m'),	NULL },
	{ &MEDIASUBTYPE_FMP4, CODEC_ID_MPEG4,  MAKEFOURCC('F','M','P','4'),	NULL },
	{ &MEDIASUBTYPE_fmp4, CODEC_ID_MPEG4,  MAKEFOURCC('f','m','p','4'),	NULL },
	{ &MEDIASUBTYPE_HDX4, CODEC_ID_MPEG4,  MAKEFOURCC('H','D','X','4'),	NULL },
	{ &MEDIASUBTYPE_hdx4, CODEC_ID_MPEG4,  MAKEFOURCC('h','d','x','4'),	NULL },
	{ &MEDIASUBTYPE_LMP4, CODEC_ID_MPEG4,  MAKEFOURCC('L','M','P','4'),	NULL },
	{ &MEDIASUBTYPE_lmp4, CODEC_ID_MPEG4,  MAKEFOURCC('l','m','p','4'),	NULL },
	{ &MEDIASUBTYPE_NDIG, CODEC_ID_MPEG4,  MAKEFOURCC('N','D','I','G'),	NULL },
	{ &MEDIASUBTYPE_ndig, CODEC_ID_MPEG4,  MAKEFOURCC('n','d','i','g'),	NULL },
	{ &MEDIASUBTYPE_RMP4, CODEC_ID_MPEG4,  MAKEFOURCC('R','M','P','4'),	NULL },
	{ &MEDIASUBTYPE_rmp4, CODEC_ID_MPEG4,  MAKEFOURCC('r','m','p','4'),	NULL },
	{ &MEDIASUBTYPE_SMP4, CODEC_ID_MPEG4,  MAKEFOURCC('S','M','P','4'),	NULL },
	{ &MEDIASUBTYPE_smp4, CODEC_ID_MPEG4,  MAKEFOURCC('s','m','p','4'),	NULL },
	{ &MEDIASUBTYPE_SEDG, CODEC_ID_MPEG4,  MAKEFOURCC('S','E','D','G'),	NULL },
	{ &MEDIASUBTYPE_sedg, CODEC_ID_MPEG4,  MAKEFOURCC('s','e','d','g'),	NULL },
	{ &MEDIASUBTYPE_UMP4, CODEC_ID_MPEG4,  MAKEFOURCC('U','M','P','4'),	NULL },
	{ &MEDIASUBTYPE_ump4, CODEC_ID_MPEG4,  MAKEFOURCC('u','m','p','4'),	NULL },
	{ &MEDIASUBTYPE_WV1F, CODEC_ID_MPEG4,  MAKEFOURCC('W','V','1','F'),	NULL },
	{ &MEDIASUBTYPE_wv1f, CODEC_ID_MPEG4,  MAKEFOURCC('w','v','1','f'),	NULL },

	// WMV1/2/3
	{ &MEDIASUBTYPE_WMV1, CODEC_ID_WMV1,  MAKEFOURCC('W','M','V','1'),	NULL },
	{ &MEDIASUBTYPE_wmv1, CODEC_ID_WMV1,  MAKEFOURCC('w','m','v','1'),	NULL },
	{ &MEDIASUBTYPE_WMV2, CODEC_ID_WMV2,  MAKEFOURCC('W','M','V','2'),	NULL },
	{ &MEDIASUBTYPE_wmv2, CODEC_ID_WMV2,  MAKEFOURCC('w','m','v','2'),	NULL },
	{ &MEDIASUBTYPE_WMV3, CODEC_ID_WMV3,  MAKEFOURCC('W','M','V','3'),	NULL },
	{ &MEDIASUBTYPE_wmv3, CODEC_ID_WMV3,  MAKEFOURCC('w','m','v','3'),	NULL },

	// MSMPEG-4
	{ &MEDIASUBTYPE_DIV3, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('D','I','V','3'),	NULL },
	{ &MEDIASUBTYPE_div3, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('d','i','v','3'),	NULL },
	{ &MEDIASUBTYPE_DVX3, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('D','V','X','3'),	NULL },
	{ &MEDIASUBTYPE_dvx3, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('d','v','x','3'),	NULL },
	{ &MEDIASUBTYPE_MP43, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('M','P','4','3'),	NULL },
	{ &MEDIASUBTYPE_mp43, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('m','p','4','3'),	NULL },
	{ &MEDIASUBTYPE_COL1, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('C','O','L','1'),	NULL },
	{ &MEDIASUBTYPE_col1, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('c','o','l','1'),	NULL },
	{ &MEDIASUBTYPE_DIV4, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('D','I','V','4'),	NULL },
	{ &MEDIASUBTYPE_div4, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('d','i','v','4'),	NULL },
	{ &MEDIASUBTYPE_DIV5, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('D','I','V','5'),	NULL },
	{ &MEDIASUBTYPE_div5, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('d','i','v','5'),	NULL },
	{ &MEDIASUBTYPE_DIV6, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('D','I','V','6'),	NULL },
	{ &MEDIASUBTYPE_div6, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('d','i','v','6'),	NULL },
	{ &MEDIASUBTYPE_AP41, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('A','P','4','1'),	NULL },
	{ &MEDIASUBTYPE_ap41, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('a','p','4','1'),	NULL },
	{ &MEDIASUBTYPE_MPG3, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('M','P','G','3'),	NULL },
	{ &MEDIASUBTYPE_mpg3, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('m','p','g','3'),	NULL },
	{ &MEDIASUBTYPE_DIV2, CODEC_ID_MSMPEG4V2,  MAKEFOURCC('D','I','V','2'),	NULL },
	{ &MEDIASUBTYPE_div2, CODEC_ID_MSMPEG4V2,  MAKEFOURCC('d','i','v','2'),	NULL },
	{ &MEDIASUBTYPE_MP42, CODEC_ID_MSMPEG4V2,  MAKEFOURCC('M','P','4','2'),	NULL },
	{ &MEDIASUBTYPE_mp42, CODEC_ID_MSMPEG4V2,  MAKEFOURCC('m','p','4','2'),	NULL },
	{ &MEDIASUBTYPE_MPG4, CODEC_ID_MSMPEG4V1,  MAKEFOURCC('M','P','G','4'),	NULL },
	{ &MEDIASUBTYPE_mpg4, CODEC_ID_MSMPEG4V1,  MAKEFOURCC('m','p','g','4'),	NULL },
	{ &MEDIASUBTYPE_DIV1, CODEC_ID_MSMPEG4V1,  MAKEFOURCC('D','I','V','1'),	NULL },
	{ &MEDIASUBTYPE_div1, CODEC_ID_MSMPEG4V1,  MAKEFOURCC('d','i','v','1'),	NULL },
	{ &MEDIASUBTYPE_MP41, CODEC_ID_MSMPEG4V1,  MAKEFOURCC('M','P','4','1'),	NULL },
	{ &MEDIASUBTYPE_mp41, CODEC_ID_MSMPEG4V1,  MAKEFOURCC('m','p','4','1'),	NULL },

	// AMV Video
	{ &MEDIASUBTYPE_AMVV, CODEC_ID_AMV,  MAKEFOURCC('A','M','V','V'),	NULL },

	// H264/AVC
	{ &MEDIASUBTYPE_H264, CODEC_ID_H264, MAKEFOURCC('H','2','6','4'),	&DXVA_H264 },
	{ &MEDIASUBTYPE_h264, CODEC_ID_H264, MAKEFOURCC('h','2','6','4'),	&DXVA_H264 },
	{ &MEDIASUBTYPE_X264, CODEC_ID_H264, MAKEFOURCC('X','2','6','4'),	&DXVA_H264 },
	{ &MEDIASUBTYPE_x264, CODEC_ID_H264, MAKEFOURCC('x','2','6','4'),	&DXVA_H264 },
	{ &MEDIASUBTYPE_VSSH, CODEC_ID_H264, MAKEFOURCC('V','S','S','H'),	&DXVA_H264 },
	{ &MEDIASUBTYPE_vssh, CODEC_ID_H264, MAKEFOURCC('v','s','s','h'),	&DXVA_H264 },
	{ &MEDIASUBTYPE_DAVC, CODEC_ID_H264, MAKEFOURCC('D','A','V','C'),	&DXVA_H264 },
	{ &MEDIASUBTYPE_davc, CODEC_ID_H264, MAKEFOURCC('d','a','v','c'),	&DXVA_H264 },
	{ &MEDIASUBTYPE_PAVC, CODEC_ID_H264, MAKEFOURCC('P','A','V','C'),	&DXVA_H264 },
	{ &MEDIASUBTYPE_pavc, CODEC_ID_H264, MAKEFOURCC('p','a','v','c'),	&DXVA_H264 },
	{ &MEDIASUBTYPE_AVC1, CODEC_ID_H264, MAKEFOURCC('A','V','C','1'),	&DXVA_H264 },
	{ &MEDIASUBTYPE_avc1, CODEC_ID_H264, MAKEFOURCC('a','v','c','1'),	&DXVA_H264 },

	// SVQ3
	{ &MEDIASUBTYPE_SVQ3, CODEC_ID_SVQ3, MAKEFOURCC('S','V','Q','3'),	NULL },

	// SVQ1
	{ &MEDIASUBTYPE_SVQ1, CODEC_ID_SVQ1, MAKEFOURCC('S','V','Q','1'),	NULL },

	// H263
	{ &MEDIASUBTYPE_H263, CODEC_ID_H263, MAKEFOURCC('H','2','6','3'),	NULL },
	{ &MEDIASUBTYPE_h263, CODEC_ID_H263, MAKEFOURCC('h','2','6','3'),	NULL },

	// Theora
	{ &MEDIASUBTYPE_THEORA, CODEC_ID_THEORA, MAKEFOURCC('T','H','E','O'),	NULL },
	{ &MEDIASUBTYPE_theora, CODEC_ID_THEORA, MAKEFOURCC('t','h','e','o'),	NULL },

	// WVC1
	{ &MEDIASUBTYPE_WVC1, CODEC_ID_VC1,  MAKEFOURCC('W','V','C','1'),	&DXVA_VC1 },
	{ &MEDIASUBTYPE_wvc1, CODEC_ID_VC1,  MAKEFOURCC('w','v','c','1'),	&DXVA_VC1 }
};

const AMOVIESETUP_MEDIATYPE CMPCVideoDecFilter::sudPinTypesIn[] =
{
	// Flash video
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_FLV1   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_flv1   },

	// VP5
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP50   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_vp50   },

	// VP6
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP60   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_vp60   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP61   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_vp61   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP62   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_vp62   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_FLV4   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_flv4   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP6F   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_vp6f   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP6A   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_vp6a   },

	// Xvid
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_XVID   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_xvid   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_XVIX   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_xvix   },

	// DivX
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DX50   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_dx50   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DIVX   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_divx   },

	// Other MPEG-4
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MP4V   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_mp4v   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_M4S2   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_m4s2   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MP4S   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_mp4s   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_3IV1   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_3iv1   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_3IV2   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_3iv2   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_3IVX   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_3ivx   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_BLZ0   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_blz0   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DM4V   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_dm4v   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_FFDS   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_ffds   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_FVFW   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_fvfw   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DXGM   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_dxgm   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_FMP4   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_fmp4   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_HDX4   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_hdx4   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_LMP4   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_lmp4   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_NDIG   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_ndig   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_RMP4   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_rmp4   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_SMP4   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_smp4   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_SEDG   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_sedg   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_UMP4   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_ump4   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_WV1F   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_wv1f   },

	// WMV1/2/3
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_WMV1   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_wmv1   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_WMV2   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_wmv2   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_WMV3   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_wmv3   },

	// MSMPEG-4
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DIV3   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_div3   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DVX3   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_dvx3   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MP43   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_mp43   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_COL1   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_col1   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DIV4   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_div4   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DIV5   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_div5   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DIV6   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_div6   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_AP41   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_ap41   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MPG3   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_mpg3   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DIV2   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_div2   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MP42   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_mp42   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MPG4   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_mpg4   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DIV1   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_div1   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MP41   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_mp41   },

	// AMV Video
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_AMVV   },

	// H264/AVC
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_H264   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_h264   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_X264   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_x264   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VSSH   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_vssh   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_DAVC   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_davc   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_PAVC   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_pavc   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_AVC1   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_avc1   },

	// SVQ3
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_SVQ3   },

	// SVQ1
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_SVQ1   },

	// H263
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_H263   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_h263   },

	// Theora
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_THEORA },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_theora },

	// VC1
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_WVC1   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_wvc1   }
};
const int CMPCVideoDecFilter::sudPinTypesInCount = countof(CMPCVideoDecFilter::sudPinTypesIn);


const AMOVIESETUP_MEDIATYPE CMPCVideoDecFilter::sudPinTypesOut[] =
{
	{&MEDIATYPE_Video, &MEDIASUBTYPE_NV12},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_NV24}
};
const int CMPCVideoDecFilter::sudPinTypesOutCount = countof(CMPCVideoDecFilter::sudPinTypesOut);

CMPCVideoDecFilter::CMPCVideoDecFilter(LPUNKNOWN lpunk, HRESULT* phr) 
	: CBaseVideoFilter(NAME("MPC - Video decoder"), lpunk, phr, __uuidof(this))
{
	if(phr) *phr = S_OK;

	if (m_pOutput)	delete m_pOutput;
	if(!(m_pOutput = new CVideoDecOutputPin(NAME("CVideoDecOutputPin"), this, phr, L"Output"))) *phr = E_OUTOFMEMORY;

	m_pCpuId				= new CCpuId();
	m_pAVCodec				= NULL;
	m_pAVCtx				= NULL;
	m_pFrame				= NULL;
	m_nCodecNb				= -1;
	m_rtAvrTimePerFrame		= 0;
	m_bReorderBFrame		= true;
	m_DXVADecoderGUID		= GUID_NULL;
	m_nActiveCodecs			= MPCVD_FLASH|MPCVD_VC1|MPCVD_XVID|MPCVD_DIVX|MPCVD_WMV|MPCVD_MSMPEG4|MPCVD_H263|MPCVD_SVQ3|MPCVD_AMVV|MPCVD_THEORA;

	m_nWorkaroundBug		= FF_BUG_AUTODETECT;
	m_nErrorConcealment		= FF_EC_DEBLOCK | FF_EC_GUESS_MVS;

	m_nThreadNumber			= m_pCpuId->GetProcessorNumber();
	m_nDiscardMode			= AVDISCARD_DEFAULT;
	m_nErrorResilience		= FF_ER_CAREFUL;
	m_nIDCTAlgo				= FF_IDCT_AUTO;
	m_bEnableDXVA			= true;
	m_bEnableFfmpeg			= true;
	m_bDXVACompatible		= true;
	m_nCompatibilityMode	= 0;
	m_pFFBuffer				= NULL;
	m_nFFBufferSize			= 0;
	m_nWidth				= 0;
	m_nHeight				= 0;

	m_nDXVAMode				= MODE_SOFTWARE;
	m_pDXVADecoder			= NULL;
	m_pVideoOutputFormat	= NULL;
	m_nVideoOutputCount		= 0;
	m_hDevice				= INVALID_HANDLE_VALUE;

	CRegKey key;
	if(ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Gabest\\Filters\\MPC Video Decoder"), KEY_READ))
	{
		DWORD dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("ThreadNumber"), dw)) m_nThreadNumber = dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("DiscardMode"), dw)) m_nDiscardMode = dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("ErrorResilience"), dw)) m_nErrorResilience = dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("IDCTAlgo"), dw)) m_nIDCTAlgo = dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("EnableDXVA"), dw)) m_bEnableDXVA = !!dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("CompatibilityMode"), dw)) m_nCompatibilityMode = dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("ActiveCodecs"), dw)) m_nActiveCodecs = dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("EnableFfmpeg"), dw)) m_bEnableFfmpeg = !!dw;		
	}

	ff_avcodec_default_get_buffer		= avcodec_default_get_buffer;
	ff_avcodec_default_release_buffer	= avcodec_default_release_buffer;
	ff_avcodec_default_reget_buffer		= avcodec_default_reget_buffer;

	avcodec_init();
	avcodec_register_all();
	av_log_set_callback(LogLibAVCodec);

	DetectVideoCard();

#ifdef _DEBUG
	// Check codec definition table
	int		nCodecs	  = countof(ffCodecs);
	int		nPinTypes = countof(sudPinTypesIn);
	ASSERT (nCodecs == nPinTypes);
	for (int i=0; i<nPinTypes; i++)
		ASSERT (ffCodecs[i].clsMinorType == sudPinTypesIn[i].clsMinorType);
#endif
}


void CMPCVideoDecFilter::DetectVideoCard()
{
	IDirect3D9* pD3D9;
	m_nPCIVendor = 0;
	m_nPCIDevice = 0;

	if (pD3D9 = Direct3DCreate9(D3D_SDK_VERSION)) {
		D3DADAPTER_IDENTIFIER9 adapterIdentifier;
		if (pD3D9->GetAdapterIdentifier(0, 0, &adapterIdentifier) == S_OK) {
			m_nPCIVendor = adapterIdentifier.VendorId;
			m_nPCIDevice = adapterIdentifier.DeviceId;
			m_strDeviceDescription = adapterIdentifier.Description;
			m_strDeviceDescription.AppendFormat (_T(" (%d)"), m_nPCIVendor);
		}
		pD3D9->Release();
	}
}


CMPCVideoDecFilter::~CMPCVideoDecFilter()
{
	Cleanup();

	SAFE_DELETE(m_pCpuId);
}

HRESULT CMPCVideoDecFilter::IsVideoInterlaced()
{
	return false;
};


void CMPCVideoDecFilter::GetOutputSize(int& w, int& h, int& arx, int& ary)
{
	w = PictWidth();
	h = PictHeightRounded();
}

int CMPCVideoDecFilter::PictWidth()
{
	return m_nWidth; 
}

int CMPCVideoDecFilter::PictHeight()
{
	// Picture height should be rounded to 16 for DXVA
	return m_nHeight;
}

int CMPCVideoDecFilter::PictHeightRounded()
{
	// Picture height should be rounded to 16 for DXVA
	return ((m_nHeight + 15) / 16) * 16;
}


int CMPCVideoDecFilter::FindCodec(const CMediaType* mtIn)
{
	for (int i=0; i<countof(ffCodecs); i++)
		if (mtIn->subtype == *ffCodecs[i].clsMinorType)
		{
#ifndef REGISTER_FILTER
			return i;
#else
			bool	bCodecActivated = true;
			switch (ffCodecs[i].nFFCodec)
			{
			case CODEC_ID_FLV1 :
			case CODEC_ID_VP5  :
			case CODEC_ID_VP6  :
			case CODEC_ID_VP6F :
			case CODEC_ID_VP6A :
				bCodecActivated = (m_nActiveCodecs & MPCVD_FLASH) != 0;
				break;
			case CODEC_ID_MPEG4 :
				if ((*ffCodecs[i].clsMinorType == MEDIASUBTYPE_XVID) || 
					(*ffCodecs[i].clsMinorType == MEDIASUBTYPE_xvid) ||
					(*ffCodecs[i].clsMinorType == MEDIASUBTYPE_XVIX) ||
					(*ffCodecs[i].clsMinorType == MEDIASUBTYPE_xvix) )
				{
					bCodecActivated = (m_nActiveCodecs & MPCVD_XVID) != 0;
				}
				else if ((*ffCodecs[i].clsMinorType == MEDIASUBTYPE_DX50) || 
					(*ffCodecs[i].clsMinorType == MEDIASUBTYPE_dx50) ||
					(*ffCodecs[i].clsMinorType == MEDIASUBTYPE_DIVX) ||
					(*ffCodecs[i].clsMinorType == MEDIASUBTYPE_divx) )
				{
					bCodecActivated = (m_nActiveCodecs & MPCVD_DIVX) != 0;
				}
				break;
			case CODEC_ID_WMV1 :
			case CODEC_ID_WMV2 :
			case CODEC_ID_WMV3 :
				bCodecActivated = (m_nActiveCodecs & MPCVD_WMV) != 0;
				break;
			case CODEC_ID_MSMPEG4V3 :
			case CODEC_ID_MSMPEG4V2 :
			case CODEC_ID_MSMPEG4V1 :
				bCodecActivated = (m_nActiveCodecs & MPCVD_MSMPEG4) != 0;
				break;
			case CODEC_ID_H264 :
				bCodecActivated = (m_nActiveCodecs & MPCVD_H264) != 0;
				break;
			case CODEC_ID_SVQ3 :
			case CODEC_ID_SVQ1 :
				bCodecActivated = (m_nActiveCodecs & MPCVD_SVQ3) != 0;
				break;
			case CODEC_ID_H263 :
				bCodecActivated = (m_nActiveCodecs & MPCVD_H263) != 0;
				break;
			case CODEC_ID_THEORA :
				bCodecActivated = (m_nActiveCodecs & MPCVD_THEORA) != 0;
				break;
			case CODEC_ID_VC1 :
				bCodecActivated = (m_nActiveCodecs & MPCVD_VC1) != 0;
				break;
			case CODEC_ID_AMV :
				bCodecActivated = (m_nActiveCodecs & MPCVD_AMVV) != 0;
				break;
			default :
				ASSERT(FALSE);
			}
			return (bCodecActivated ? i : -1);
#endif
		}

	return -1;
}

void CMPCVideoDecFilter::Cleanup()
{
	SAFE_DELETE (m_pDXVADecoder);

	// Release FFMpeg
	if (m_pAVCtx)
	{
		if (m_pAVCtx->intra_matrix)			free(m_pAVCtx->intra_matrix);
		if (m_pAVCtx->inter_matrix)			free(m_pAVCtx->inter_matrix);
		if (m_pAVCtx->intra_matrix_luma)	free(m_pAVCtx->intra_matrix_luma);
		if (m_pAVCtx->intra_matrix_chroma)	free(m_pAVCtx->intra_matrix_chroma);
		if (m_pAVCtx->inter_matrix_luma)	free(m_pAVCtx->inter_matrix_luma);
		if (m_pAVCtx->inter_matrix_chroma)	free(m_pAVCtx->inter_matrix_chroma);
		if (m_pAVCtx->extradata)			free((unsigned char*)m_pAVCtx->extradata);
		if (m_pFFBuffer)					free(m_pFFBuffer);

		if (m_pAVCtx->slice_offset)			av_free(m_pAVCtx->slice_offset);
		if (m_pAVCtx->codec)				avcodec_close(m_pAVCtx);

		if ((m_nThreadNumber > 1) && IsMultiThreadSupported (ffCodecs[m_nCodecNb].nFFCodec))
			avcodec_thread_free (m_pAVCtx);

		av_free(m_pAVCtx);
	}
	if (m_pFrame)	av_free(m_pFrame);

	m_pAVCodec		= NULL;
	m_pAVCtx		= NULL;
	m_pFrame		= NULL;
	m_pFFBuffer		= NULL;
	m_nFFBufferSize	= 0;
	m_nCodecNb		= -1;
	SAFE_DELETE_ARRAY (m_pVideoOutputFormat);

	// Release DXVA ressources
	if (m_hDevice != INVALID_HANDLE_VALUE)
	{
		m_pDeviceManager->CloseDeviceHandle(m_hDevice);
		m_hDevice = INVALID_HANDLE_VALUE;
	}

	m_pDeviceManager		= NULL;
	m_pDecoderService		= NULL;
	m_pDecoderRenderTarget	= NULL;
}

void CMPCVideoDecFilter::CalcAvgTimePerFrame()
{
	CMediaType &mt	= m_pInput->CurrentMediaType();
	if (mt.formattype==FORMAT_VideoInfo)
		m_rtAvrTimePerFrame = ((VIDEOINFOHEADER*)mt.pbFormat)->AvgTimePerFrame;
	else if (mt.formattype==FORMAT_VideoInfo2)
		m_rtAvrTimePerFrame = ((VIDEOINFOHEADER2*)mt.pbFormat)->AvgTimePerFrame;
	else if (mt.formattype==FORMAT_MPEGVideo)
		m_rtAvrTimePerFrame = ((MPEG1VIDEOINFO*)mt.pbFormat)->hdr.AvgTimePerFrame;
	else if (mt.formattype==FORMAT_MPEG2Video)
		m_rtAvrTimePerFrame = ((MPEG2VIDEOINFO*)mt.pbFormat)->hdr.AvgTimePerFrame;
	else
	{
		ASSERT (FALSE);
		m_rtAvrTimePerFrame	= 1;
	}

	m_rtAvrTimePerFrame = max (1, m_rtAvrTimePerFrame);
}

void CMPCVideoDecFilter::LogLibAVCodec(void* par,int level,const char *fmt,va_list valist)
{
#ifdef _DEBUG
	//AVCodecContext*	m_pAVCtx = (AVCodecContext*) par;

	char		Msg [500];
	vsnprintf (Msg, sizeof(Msg), fmt, valist);
//	TRACE("AVLIB : %s", Msg);
#endif
}

void CMPCVideoDecFilter::OnGetBuffer(AVFrame *pic)
{
	// Callback from FFMpeg to store Ref Time in frame (needed to have correct rtStart after avcodec_decode_video calls)
	pic->rtStart	= m_rtStart;
}

STDMETHODIMP CMPCVideoDecFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	return
		QI(IMPCVideoDecFilter)
		QI(ISpecifyPropertyPages)
		QI(ISpecifyPropertyPages2)
		 __super::NonDelegatingQueryInterface(riid, ppv);
}




HRESULT CMPCVideoDecFilter::CheckInputType(const CMediaType* mtIn)
{
	for (int i=0; i<sizeof(sudPinTypesIn)/sizeof(AMOVIESETUP_MEDIATYPE); i++)
	{
		if ((mtIn->majortype == *sudPinTypesIn[i].clsMajorType) && 
			(mtIn->subtype == *sudPinTypesIn[i].clsMinorType))
			return S_OK;
	}

	return VFW_E_TYPE_NOT_ACCEPTED;
}


bool CMPCVideoDecFilter::IsMultiThreadSupported(int nCodec)
{
	return (nCodec==CODEC_ID_H264);
}


HRESULT CMPCVideoDecFilter::SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt)
{
	int		nNewCodec;

	if (direction == PINDIR_INPUT)
	{
		nNewCodec = FindCodec(pmt);
		if (nNewCodec == -1) return VFW_E_TYPE_NOT_ACCEPTED;

		if (nNewCodec != m_nCodecNb)
		{
			m_nCodecNb	= nNewCodec;

			m_bReorderBFrame	= true;
			m_pAVCodec			= avcodec_find_decoder(ffCodecs[nNewCodec].nFFCodec);
			CheckPointer (m_pAVCodec, VFW_E_UNSUPPORTED_VIDEO);

			m_pAVCtx	= avcodec_alloc_context();
			CheckPointer (m_pAVCtx,	  E_POINTER);

			if ((m_nThreadNumber > 1) && IsMultiThreadSupported (ffCodecs[m_nCodecNb].nFFCodec))
				avcodec_thread_init(m_pAVCtx, m_nThreadNumber);
			m_pFrame = avcodec_alloc_frame();
			CheckPointer (m_pFrame,	  E_POINTER);

			if(pmt->formattype == FORMAT_VideoInfo)
			{
				VIDEOINFOHEADER*	vih = (VIDEOINFOHEADER*)pmt->pbFormat;
				m_pAVCtx->width		= vih->bmiHeader.biWidth;
				m_pAVCtx->height	= abs(vih->bmiHeader.biHeight);
				m_pAVCtx->codec_tag	= vih->bmiHeader.biCompression;
			}
			else if(pmt->formattype == FORMAT_VideoInfo2)
			{
				VIDEOINFOHEADER2*	vih2 = (VIDEOINFOHEADER2*)pmt->pbFormat;
				m_pAVCtx->width		= vih2->bmiHeader.biWidth;
				m_pAVCtx->height	= abs(vih2->bmiHeader.biHeight);
				m_pAVCtx->codec_tag	= vih2->bmiHeader.biCompression;
			}
			else if(pmt->formattype == FORMAT_MPEGVideo)
			{
				MPEG1VIDEOINFO*		mpgv = (MPEG1VIDEOINFO*)pmt->pbFormat;
				m_pAVCtx->width		= mpgv->hdr.bmiHeader.biWidth;
				m_pAVCtx->height	= abs(mpgv->hdr.bmiHeader.biHeight);
				m_pAVCtx->codec_tag	= mpgv->hdr.bmiHeader.biCompression;
			}
			else if(pmt->formattype == FORMAT_MPEG2Video)
			{
				MPEG2VIDEOINFO*		mpg2v = (MPEG2VIDEOINFO*)pmt->pbFormat;
				m_pAVCtx->width		= mpg2v->hdr.bmiHeader.biWidth;
				m_pAVCtx->height	= abs(mpg2v->hdr.bmiHeader.biHeight);
				m_pAVCtx->codec_tag	= mpg2v->hdr.bmiHeader.biCompression;

				if (mpg2v->hdr.bmiHeader.biCompression == NULL)
				{
					m_pAVCtx->codec_tag = pmt->subtype.Data1;
				}
				else if ( (m_pAVCtx->codec_tag == MAKEFOURCC('a','v','c','1')) || (m_pAVCtx->codec_tag == MAKEFOURCC('A','V','C','1')))
				{
					m_pAVCtx->nal_length_size = mpg2v->dwFlags;
					m_bReorderBFrame = false;
				}
			}
			m_nWidth	= m_pAVCtx->width;
			m_nHeight	= m_pAVCtx->height;
			
			m_pAVCtx->intra_matrix			= (uint16_t*)calloc(sizeof(uint16_t),64);
			m_pAVCtx->inter_matrix			= (uint16_t*)calloc(sizeof(uint16_t),64);
			m_pAVCtx->intra_matrix_luma		= (uint16_t*)calloc(sizeof(uint16_t),16);
			m_pAVCtx->intra_matrix_chroma	= (uint16_t*)calloc(sizeof(uint16_t),16);
			m_pAVCtx->inter_matrix_luma		= (uint16_t*)calloc(sizeof(uint16_t),16);
			m_pAVCtx->inter_matrix_chroma	= (uint16_t*)calloc(sizeof(uint16_t),16);
			m_pAVCtx->codec_tag				= ffCodecs[nNewCodec].fourcc;
			m_pAVCtx->workaround_bugs		= m_nWorkaroundBug;
			m_pAVCtx->error_concealment		= m_nErrorConcealment;
			m_pAVCtx->error_resilience		= m_nErrorResilience;
			m_pAVCtx->idct_algo				= m_nIDCTAlgo;
			m_pAVCtx->skip_loop_filter		= (AVDiscard)m_nDiscardMode;
			m_pAVCtx->dsp_mask				= FF_MM_FORCE | m_pCpuId->GetFeatures();

			m_pAVCtx->postgain				= 1.0f;
			m_pAVCtx->scenechange_factor	= 1;
			m_pAVCtx->debug_mv				= 0;
	#ifdef _DEBUG
			//m_pAVCtx->debug					= FF_DEBUG_PICT_INFO | FF_DEBUG_STARTCODE | FF_DEBUG_PTS;
	#endif
			
			m_pAVCtx->self					= this;
			m_pAVCtx->get_buffer			= get_buffer;

			AllocExtradata (m_pAVCtx, pmt);
			ConnectTo (m_pAVCtx);
			CalcAvgTimePerFrame();

			if (avcodec_open(m_pAVCtx, m_pAVCodec)<0)
				return VFW_E_INVALIDMEDIATYPE;

			if (ffCodecs[m_nCodecNb].nFFCodec == CODEC_ID_H264)
			{
				int		nCompat;
				nCompat = FFH264CheckCompatibility (PictWidth(), PictHeightRounded(), m_pAVCtx, (BYTE*)m_pAVCtx->extradata, m_pAVCtx->extradata_size);
				switch (nCompat)
				{
				case 1 :	// SAR not supported
					 m_bDXVACompatible = false;
					 if (m_nCompatibilityMode & 1) MessageBox (NULL, _T("DXVA : SAR is not supported"), MPCVD_CAPTION, MB_OK);
					 if (m_nCompatibilityMode & 2) m_bDXVACompatible = true;
					 break;
				case 2 :	// Too much ref frames
					 m_bDXVACompatible = false;
					 if (m_nCompatibilityMode & 1) MessageBox (NULL, _T("DXVA : too much ref frame"), MPCVD_CAPTION, MB_OK);
					 if (m_nCompatibilityMode & 4) m_bDXVACompatible = true;
					 break;
				}
			}
			
			// Force single thread for DXVA !
			if (IsDXVASupported())
				avcodec_thread_init(m_pAVCtx, 1);

			BuildDXVAOutputFormat();
		}
	}

	return __super::SetMediaType(direction, pmt);
}


VIDEO_OUTPUT_FORMATS DXVAFormats[] =
{
	{&MEDIASUBTYPE_NV12, 1, 12, 'avxd'},	// DXVA2
	{&MEDIASUBTYPE_NV12, 1, 12, 'AVXD'},
	{&MEDIASUBTYPE_NV12, 1, 12, 'AVxD'},
	{&MEDIASUBTYPE_NV12, 1, 12, 'AvXD'}
};

VIDEO_OUTPUT_FORMATS SoftwareFormats[] =
{
	{&MEDIASUBTYPE_YV12, 3, 12, '21VY'},	// Software
	{&MEDIASUBTYPE_I420, 3, 12, '024I'},
	{&MEDIASUBTYPE_IYUV, 3, 12, 'VUYI'},
	{&MEDIASUBTYPE_YUY2, 1, 16, '2YUY'}
};


bool CMPCVideoDecFilter::IsDXVASupported()
{
	if ((m_nCodecNb != -1) && 
		(ffCodecs[m_nCodecNb].DXVAModes != NULL) &&	// Supported by Codec ?
		 m_bEnableDXVA &&							// Enable by user ?
		 m_bDXVACompatible)							// File compatible ?
	{
		return true;
	}

	return false;
}


void CMPCVideoDecFilter::BuildDXVAOutputFormat()
{
	int			nPos = 0;

	SAFE_DELETE_ARRAY (m_pVideoOutputFormat);

	m_nVideoOutputCount = (IsDXVASupported() ? ffCodecs[m_nCodecNb].DXVAModeCount() + countof (DXVAFormats) : 0) +
						  (m_bEnableFfmpeg   ? countof(SoftwareFormats) : 0);

	m_pVideoOutputFormat	= new VIDEO_OUTPUT_FORMATS[m_nVideoOutputCount];

	if (IsDXVASupported())
	{
		// Dynamic DXVA media types for DXVA1
		for (nPos=0; nPos<ffCodecs[m_nCodecNb].DXVAModeCount(); nPos++)
		{
			m_pVideoOutputFormat[nPos].subtype			= ffCodecs[m_nCodecNb].DXVAModes->Decoder[nPos];
			m_pVideoOutputFormat[nPos].biCompression	= 'avxd';
			m_pVideoOutputFormat[nPos].biBitCount		= 12;
			m_pVideoOutputFormat[nPos].biPlanes			= 1;
		}

		// Static list for DXVA2
		memcpy (&m_pVideoOutputFormat[nPos], DXVAFormats, sizeof(DXVAFormats));
		nPos += countof (DXVAFormats);
	}

	// Software rendering
	if (m_bEnableFfmpeg)
		memcpy (&m_pVideoOutputFormat[nPos], SoftwareFormats, sizeof(SoftwareFormats));
}


int CMPCVideoDecFilter::GetPicEntryNumber()
{
	if (IsDXVASupported())
		return ffCodecs[m_nCodecNb].DXVAModes->PicEntryNumber;
	else
		return 0;
}


void CMPCVideoDecFilter::GetOutputFormats (int& nNumber, VIDEO_OUTPUT_FORMATS** ppFormats)
{
	nNumber		= m_nVideoOutputCount;
	*ppFormats	= m_pVideoOutputFormat;
}



void CMPCVideoDecFilter::AllocExtradata(AVCodecContext* pAVCtx, const CMediaType* pmt)
{
	const BYTE*		data = NULL;
	unsigned int	size = 0;

	if (pmt->formattype==FORMAT_VideoInfo)
	{
		size = pmt->cbFormat-sizeof(VIDEOINFOHEADER);
		data = size?pmt->pbFormat+sizeof(VIDEOINFOHEADER):NULL;
	}
	else if (pmt->formattype==FORMAT_VideoInfo2)
	{
		size = pmt->cbFormat-sizeof(VIDEOINFOHEADER2);
		data = size?pmt->pbFormat+sizeof(VIDEOINFOHEADER2):NULL;
	}
	else if (pmt->formattype==FORMAT_MPEGVideo)
	{
		MPEG1VIDEOINFO*		mpeg1info = (MPEG1VIDEOINFO*)pmt->pbFormat;
		if (mpeg1info->cbSequenceHeader)
		{
			size = mpeg1info->cbSequenceHeader;
			data = mpeg1info->bSequenceHeader;
		}
	}
	else if (pmt->formattype==FORMAT_MPEG2Video)
	{
		MPEG2VIDEOINFO*		mpeg2info = (MPEG2VIDEOINFO*)pmt->pbFormat;
		if (mpeg2info->cbSequenceHeader)
		{
			size = mpeg2info->cbSequenceHeader;
			data = (const uint8_t*)mpeg2info->dwSequenceHeader;
		}
	}
	else if (pmt->formattype==FORMAT_VorbisFormat2)
	{
		const VORBISFORMAT2 *vf2=(const VORBISFORMAT2*)pmt->pbFormat;
		size=pmt->cbFormat-sizeof(VORBISFORMAT2);
		data=size?pmt->pbFormat+sizeof(VORBISFORMAT2):NULL;
	}

	if (size)
	{
		pAVCtx->extradata_size	= size;
		pAVCtx->extradata		= (const unsigned char*)calloc(1,size+FF_INPUT_BUFFER_PADDING_SIZE);
		memcpy((void*)pAVCtx->extradata, data, size);
	}
}


HRESULT CMPCVideoDecFilter::CompleteConnect(PIN_DIRECTION direction, IPin* pReceivePin)
{
	LOG(_T("CMPCVideoDecFilter::CompleteConnect"));

	if (direction==PINDIR_INPUT && m_pOutput->IsConnected())
	{
		ReconnectOutput (m_nWidth, m_nHeight);
	}
	else if ( (direction==PINDIR_OUTPUT) && IsDXVASupported() )		 
	{

		if (m_nDXVAMode == MODE_DXVA1)
			m_pDXVADecoder->ConfigureDXVA1();	// TODO : check errors!
		else if (SUCCEEDED (ConfigureDXVA2 (pReceivePin)) &&	SUCCEEDED (SetEVRForDXVA2 (pReceivePin)) )
			m_nDXVAMode  = MODE_DXVA2;

		// TODO :
		CLSID	ClsidSourceFilter = GetCLSID(m_pInput->GetConnected());
		if((ClsidSourceFilter == __uuidof(CMpegSourceFilter)) || (ClsidSourceFilter == __uuidof(CMpegSplitterFilter)))
			m_bReorderBFrame = false;
	}

	if ((m_pOutput->CurrentMediaType().subtype == MEDIASUBTYPE_NV12) && (m_nDXVAMode == MODE_SOFTWARE))
		return VFW_E_INVALIDMEDIATYPE;

	return __super::CompleteConnect (direction, pReceivePin);
}


HRESULT CMPCVideoDecFilter::DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties)
{
	if (UseDXVA2())
	{
		HRESULT					hr;
		ALLOCATOR_PROPERTIES	Actual;

		if(m_pInput->IsConnected() == FALSE) return E_UNEXPECTED;

		pProperties->cBuffers = GetPicEntryNumber();

		if(FAILED(hr = pAllocator->SetProperties(pProperties, &Actual))) 
			return hr;

		return pProperties->cBuffers > Actual.cBuffers || pProperties->cbBuffer > Actual.cbBuffer
			? E_FAIL
			: NOERROR;
	}
	else
		return __super::DecideBufferSize (pAllocator, pProperties);
}


HRESULT CMPCVideoDecFilter::NewSegment(REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, double dRate)
{
	CAutoLock cAutoLock(&m_csReceive);
	m_nPosB = 1;
	memset (&m_BFrames, 0, sizeof(m_BFrames));

	if (m_pDXVADecoder)
		m_pDXVADecoder->Flush();
	return __super::NewSegment (rtStart, rtStop, dRate);
}


HRESULT CMPCVideoDecFilter::BreakConnect(PIN_DIRECTION dir)
{
	if (dir == PINDIR_INPUT)
	{
		Cleanup();
	}

	return __super::BreakConnect (dir);
}


HRESULT CMPCVideoDecFilter::SoftwareDecode(IMediaSample* pIn, BYTE* pDataIn, int nSize, REFERENCE_TIME& rtStart, REFERENCE_TIME& rtStop)
{
	HRESULT			hr;
	int				got_picture;
	int				used_bytes;

	if (m_pAVCtx->has_b_frames)
	{
		m_BFrames[m_nPosB].rtStart	= rtStart;
		m_BFrames[m_nPosB].rtStop	= rtStop;
		m_nPosB						= 1-m_nPosB;
	}

	while (nSize > 0)
	{
		if (nSize+FF_INPUT_BUFFER_PADDING_SIZE > m_nFFBufferSize)
		{
			m_nFFBufferSize = nSize+FF_INPUT_BUFFER_PADDING_SIZE;
			m_pFFBuffer		= (BYTE*)realloc(m_pFFBuffer, m_nFFBufferSize);
		}

		// Required number of additionally allocated bytes at the end of the input bitstream for decoding.
		// This is mainly needed because some optimized bitstream readers read
		// 32 or 64 bit at once and could read over the end.<br>
		// Note: If the first 23 bits of the additional bytes are not 0, then damaged
		// MPEG bitstreams could cause overread and segfault.
		memcpy(m_pFFBuffer, pDataIn, nSize);
		memset(m_pFFBuffer+nSize,0,FF_INPUT_BUFFER_PADDING_SIZE);

		used_bytes = avcodec_decode_video (m_pAVCtx, m_pFrame, &got_picture, m_pFFBuffer, nSize);
		if (!got_picture || !m_pFrame->data[0]) return S_OK;
		if(pIn->IsPreroll() == S_OK || rtStart < 0) return S_OK;

		CComPtr<IMediaSample>	pOut;
		BYTE*					pDataOut = NULL;

		if(FAILED(hr = GetDeliveryBuffer(m_pAVCtx->width, m_pAVCtx->height, &pOut)) || FAILED(hr = pOut->GetPointer(&pDataOut)))
			return hr;
	
		rtStart = m_pFrame->rtStart;
		rtStop  = m_pFrame->rtStart + m_rtAvrTimePerFrame;

		// Re-order B-frames if needed
		if (m_pAVCtx->has_b_frames && m_bReorderBFrame)
		{
			rtStart	= m_BFrames [m_nPosB].rtStart;
			rtStop	= m_BFrames [m_nPosB].rtStop;
		}

		pOut->SetTime(&rtStart, &rtStop);
		pOut->SetMediaTime(NULL, NULL);

		CopyBuffer(pDataOut, m_pFrame->data, m_pAVCtx->width, m_pAVCtx->height, m_pFrame->linesize[0], MEDIASUBTYPE_I420, false);

#ifdef _DEBUG
		static REFERENCE_TIME	rtLast = 0;
		//TRACE ("Deliver : %10I64d - %10I64d   (%10I64d)  {%10I64d}\n", rtStart, rtStop, 
		//			rtStop - rtStart, rtStart - rtLast);
		rtLast = rtStart;
#endif

		hr = m_pOutput->Deliver(pOut);

		nSize	-= used_bytes;
		pDataIn += used_bytes;
	}

	return hr;
}


HRESULT CMPCVideoDecFilter::Transform(IMediaSample* pIn)
{
	CAutoLock cAutoLock(&m_csReceive);
	HRESULT			hr;
	BYTE*			pDataIn;
	int				nSize;
	REFERENCE_TIME	rtStart;
	REFERENCE_TIME	rtStop;

	if(FAILED(hr = pIn->GetPointer(&pDataIn)))
		return hr;

	nSize		= pIn->GetActualDataLength();
	hr			= pIn->GetTime(&rtStart, &rtStop);
	m_rtStart	= rtStart;

	if (rtStop <= rtStart)
		rtStop = rtStart + m_rtAvrTimePerFrame;
	
//	DumpBuffer (pDataIn, nSize);
//	TRACE ("Receive : %10I64d - %10I64d   (%10I64d)  Size=%d\n", rtStart, rtStop, rtStop - rtStart, nSize);

	//char		strMsg[300];
	//FILE* hFile = fopen ("d:\\receive.txt", "at");
	//sprintf (strMsg, "Receive : %10I64d - %10I64d   Size=%d\n", (rtStart + m_rtAvrTimePerFrame/2) / m_rtAvrTimePerFrame, rtStart, nSize);
	//fwrite (strMsg, strlen(strMsg), 1, hFile);
	//fclose (hFile);

	switch (m_nDXVAMode)
	{
	case MODE_SOFTWARE :
		hr = SoftwareDecode (pIn, pDataIn, nSize, rtStart, rtStop);
		break;
	case MODE_DXVA1 :
	case MODE_DXVA2 :
		CheckPointer (m_pDXVADecoder, E_UNEXPECTED);
		hr = m_pDXVADecoder->DecodeFrame (pDataIn, nSize, rtStart, rtStop);
		break;
	default :
		ASSERT (FALSE);
		hr = E_UNEXPECTED;
	}

	return hr;
}

void CMPCVideoDecFilter::FillInVideoDescription(DXVA2_VideoDesc *pDesc)
{
	memset (pDesc, 0, sizeof(DXVA2_VideoDesc));
	pDesc->SampleWidth			= PictWidth();
	pDesc->SampleHeight			= PictHeightRounded();
	pDesc->Format				= D3DFMT_A8R8G8B8;
	pDesc->UABProtectionLevel	= 1;
}

BOOL CMPCVideoDecFilter::IsSupportedDecoderMode(const GUID& mode)
{
	if (IsDXVASupported())
	{
		for (int i=0; i<MAX_SUPPORTED_MODE; i++)
		{
			if (*ffCodecs[m_nCodecNb].DXVAModes->Decoder[i] == GUID_NULL) 
				break;
			else if (*ffCodecs[m_nCodecNb].DXVAModes->Decoder[i] == mode)
				return true;
		}
	}

	return false;
}

BOOL CMPCVideoDecFilter::IsSupportedDecoderConfig(const D3DFORMAT nD3DFormat, const DXVA2_ConfigPictureDecode& config)
{
	bool	bRet = false;

	// TODO : not finished
	bRet = ((config.ConfigBitstreamRaw >= ffCodecs[m_nCodecNb].DXVAModes->ConfigBitstreamRawMin) && 
			(nD3DFormat				   == MAKEFOURCC('N', 'V', '1', '2')) );

	LOG (_T("IsSupportedDecoderConfig  0x%08x  %d"), nD3DFormat, bRet);
	return bRet;
}

HRESULT CMPCVideoDecFilter::FindDXVA2DecoderConfiguration(IDirectXVideoDecoderService *pDecoderService,
													 const GUID& guidDecoder, 
													 DXVA2_ConfigPictureDecode *pSelectedConfig,
													 BOOL *pbFoundDXVA2Configuration)
{
    HRESULT hr = S_OK;
    UINT cFormats = 0;
    UINT cConfigurations = 0;

    D3DFORMAT                   *pFormats = NULL;           // size = cFormats
    DXVA2_ConfigPictureDecode   *pConfig = NULL;            // size = cConfigurations

    // Find the valid render target formats for this decoder GUID.
    hr = pDecoderService->GetDecoderRenderTargets(guidDecoder, &cFormats, &pFormats);
	LOG (_T("GetDecoderRenderTargets => %d"), cFormats);

    if (SUCCEEDED(hr))
    {
        // Look for a format that matches our output format.
        for (UINT iFormat = 0; iFormat < cFormats;  iFormat++)
        {
			LOG (_T("Try to negociate => 0x%08x"), pFormats[iFormat]);

            // Fill in the video description. Set the width, height, format, and frame rate.
            FillInVideoDescription(&m_VideoDesc); // Private helper function.
            m_VideoDesc.Format = pFormats[iFormat];

            // Get the available configurations.
            hr = pDecoderService->GetDecoderConfigurations(guidDecoder, &m_VideoDesc, NULL, &cConfigurations, &pConfig);

            if (FAILED(hr))
            {
                continue;
            }

            // Find a supported configuration.
            for (UINT iConfig = 0; iConfig < cConfigurations; iConfig++)
            {
                if (IsSupportedDecoderConfig(pFormats[iFormat], pConfig[iConfig]))
                {
                    // This configuration is good.
                    *pbFoundDXVA2Configuration = TRUE;
                    *pSelectedConfig = pConfig[iConfig];
                    break;
                }
            }

            CoTaskMemFree(pConfig);
        } // End of formats loop.
    }

    CoTaskMemFree(pFormats);

    // Note: It is possible to return S_OK without finding a configuration.
    return hr;
}


HRESULT CMPCVideoDecFilter::ConfigureDXVA2(IPin *pPin)
{
    HRESULT hr						 = S_OK;
    UINT    cDecoderGuids			 = 0;
    BOOL    bFoundDXVA2Configuration = FALSE;
    GUID    guidDecoder				 = GUID_NULL;

    DXVA2_ConfigPictureDecode config;
    ZeroMemory(&config, sizeof(config));

    CComPtr<IMFGetService>					pGetService;
    CComPtr<IDirect3DDeviceManager9>		pDeviceManager;
    CComPtr<IDirectXVideoDecoderService>	pDecoderService;
    GUID*									pDecoderGuids = NULL;
    HANDLE									hDevice = INVALID_HANDLE_VALUE;

    // Query the pin for IMFGetService.
    hr = pPin->QueryInterface(__uuidof(IMFGetService), (void**)&pGetService);

    // Get the Direct3D device manager.
    if (SUCCEEDED(hr))
    {
        hr = pGetService->GetService(
            MR_VIDEO_ACCELERATION_SERVICE,
            __uuidof(IDirect3DDeviceManager9),
            (void**)&pDeviceManager);
    }

    // Open a new device handle.
    if (SUCCEEDED(hr))
    {
        hr = pDeviceManager->OpenDeviceHandle(&hDevice);
    } 

    // Get the video decoder service.
    if (SUCCEEDED(hr))
    {
        hr = pDeviceManager->GetVideoService(
                hDevice, 
                __uuidof(IDirectXVideoDecoderService), 
                (void**)&pDecoderService);
    }

    // Get the decoder GUIDs.
    if (SUCCEEDED(hr))
    {
        hr = pDecoderService->GetDecoderDeviceGuids(&cDecoderGuids, &pDecoderGuids);
    }

    if (SUCCEEDED(hr))
    {
        // Look for the decoder GUIDs we want.
        for (UINT iGuid = 0; iGuid < cDecoderGuids; iGuid++)
        {
            // Do we support this mode?
            if (!IsSupportedDecoderMode(pDecoderGuids[iGuid]))
            {
                continue;
            }

            // Find a configuration that we support. 
            hr = FindDXVA2DecoderConfiguration(pDecoderService, pDecoderGuids[iGuid], &config, &bFoundDXVA2Configuration);

            if (FAILED(hr))
            {
                break;
            }

            if (bFoundDXVA2Configuration)
            {
                // Found a good configuration. Save the GUID.
                guidDecoder = pDecoderGuids[iGuid];
            }
        }
    }

	if (pDecoderGuids) CoTaskMemFree(pDecoderGuids);
    if (!bFoundDXVA2Configuration)
    {
        hr = E_FAIL; // Unable to find a configuration.
    }

    if (SUCCEEDED(hr))
    {
        // Store the things we will need later.
		m_pDeviceManager	= pDeviceManager;
        m_pDecoderService	= pDecoderService;

        m_DXVA2Config		= config;
        m_DXVADecoderGUID	= guidDecoder;
        m_hDevice			= hDevice;
    }

    if (FAILED(hr))
    {
        if (hDevice != INVALID_HANDLE_VALUE)
        {
            pDeviceManager->CloseDeviceHandle(hDevice);
        }
    }

    return hr;
}


HRESULT CMPCVideoDecFilter::SetEVRForDXVA2(IPin *pPin)
{
    HRESULT hr = S_OK;

    CComPtr<IMFGetService>						pGetService;
    CComPtr<IDirectXVideoMemoryConfiguration>	pVideoConfig;

    // Query the pin for IMFGetService.
    hr = pPin->QueryInterface(__uuidof(IMFGetService), (void**)&pGetService);

    // Get the IDirectXVideoMemoryConfiguration interface.
    if (SUCCEEDED(hr))
    {
        hr = pGetService->GetService(
            MR_VIDEO_ACCELERATION_SERVICE,
            __uuidof(IDirectXVideoMemoryConfiguration),
            (void**)&pVideoConfig);
    }

    // Notify the EVR. 
    if (SUCCEEDED(hr))
    {
        DXVA2_SurfaceType surfaceType;

        for (DWORD iTypeIndex = 0; ; iTypeIndex++)
        {
            hr = pVideoConfig->GetAvailableSurfaceTypeByIndex(iTypeIndex, &surfaceType);
            
            if (FAILED(hr))
				break;

            if (surfaceType == DXVA2_SurfaceType_DecoderRenderTarget)
            {
                hr = pVideoConfig->SetSurfaceType(DXVA2_SurfaceType_DecoderRenderTarget);
                break;
            }
        }
    }

    return hr;
}


HRESULT CMPCVideoDecFilter::CreateDXVA2Decoder(UINT nNumRenderTargets, IDirect3DSurface9** pDecoderRenderTargets)
{
	HRESULT							hr;
	CComPtr<IDirectXVideoDecoder>	pDirectXVideoDec;
	
	m_pDecoderRenderTarget	= NULL;

	if (m_pDXVADecoder) m_pDXVADecoder->SetDirectXVideoDec (NULL);

	hr = m_pDecoderService->CreateVideoDecoder (m_DXVADecoderGUID, &m_VideoDesc, &m_DXVA2Config, 
								pDecoderRenderTargets, nNumRenderTargets, &pDirectXVideoDec);

	if (SUCCEEDED (hr))
	{
		if (!m_pDXVADecoder)
		{
			m_pDXVADecoder	= CDXVADecoder::CreateDecoder (this, pDirectXVideoDec, &m_DXVADecoderGUID, GetPicEntryNumber());
			if (m_pDXVADecoder) m_pDXVADecoder->SetExtraData ((BYTE*)m_pAVCtx->extradata, m_pAVCtx->extradata_size);
		}

		m_pDXVADecoder->SetDirectXVideoDec (pDirectXVideoDec);
	}

	return hr;
}


HRESULT CMPCVideoDecFilter::FindDXVA1DecoderConfiguration(IAMVideoAccelerator* pAMVideoAccelerator, const GUID* guidDecoder, DDPIXELFORMAT* pPixelFormat)
{
	HRESULT			hr				= E_FAIL;
	DWORD			dwFormats		= 0;
	DDPIXELFORMAT*	pPixelFormats	= NULL;


	pAMVideoAccelerator->GetUncompFormatsSupported (guidDecoder, &dwFormats, NULL);
	if (dwFormats > 0)
	{
	    // Find the valid render target formats for this decoder GUID.
		pPixelFormats = new DDPIXELFORMAT[dwFormats];
		hr = pAMVideoAccelerator->GetUncompFormatsSupported (guidDecoder, &dwFormats, pPixelFormats);
		if (SUCCEEDED(hr))
		{
			// Look for a format that matches our output format.
			for (DWORD iFormat = 0; iFormat < dwFormats; iFormat++)
			{
				if (pPixelFormats[iFormat].dwFourCC == MAKEFOURCC ('N', 'V', '1', '2'))
				{
					memcpy (pPixelFormat, &pPixelFormats[iFormat], sizeof(DDPIXELFORMAT));
					SAFE_DELETE_ARRAY(pPixelFormats)
					return S_OK;
				}
			}

			SAFE_DELETE_ARRAY(pPixelFormats);
			hr = E_FAIL;
		}
	}

	return hr;
}

HRESULT CMPCVideoDecFilter::CheckDXVA1Decoder(const GUID *pGuid)
{
	if (m_nCodecNb != -1)
	{
		for (int i=0; i<MAX_SUPPORTED_MODE; i++)
			if (*ffCodecs[m_nCodecNb].DXVAModes->Decoder[i] == *pGuid)
				return S_OK;
	}

	return E_INVALIDARG;
}

void CMPCVideoDecFilter::SetDXVA1Params(const GUID* pGuid, DDPIXELFORMAT* pPixelFormat)
{
	m_DXVADecoderGUID		= *pGuid;
	memcpy (&m_PixelFormat, pPixelFormat, sizeof (DDPIXELFORMAT));
}

WORD CMPCVideoDecFilter::GetDXVA1RestrictedMode()
{
	if (m_nCodecNb != -1)
	{
		for (int i=0; i<MAX_SUPPORTED_MODE; i++)
			if (*ffCodecs[m_nCodecNb].DXVAModes->Decoder[i] == m_DXVADecoderGUID)
				return ffCodecs[m_nCodecNb].DXVAModes->RestrictedMode [i];
	}

	return DXVA_RESTRICTED_MODE_UNRESTRICTED;
}

HRESULT CMPCVideoDecFilter::CreateDXVA1Decoder(IAMVideoAccelerator*  pAMVideoAccelerator, const GUID* pDecoderGuid, DWORD dwSurfaceCount)
{
	SAFE_DELETE (m_pDXVADecoder);

	if (!m_bEnableDXVA) return E_FAIL;

	m_nDXVAMode			= MODE_DXVA1;
	m_DXVADecoderGUID	= *pDecoderGuid;
	m_pDXVADecoder		= CDXVADecoder::CreateDecoder (this, pAMVideoAccelerator, &m_DXVADecoderGUID, dwSurfaceCount);
	if (m_pDXVADecoder) m_pDXVADecoder->SetExtraData ((BYTE*)m_pAVCtx->extradata, m_pAVCtx->extradata_size);

	return S_OK;
}



// ISpecifyPropertyPages2

STDMETHODIMP CMPCVideoDecFilter::GetPages(CAUUID* pPages)
{
	CheckPointer(pPages, E_POINTER);

#ifdef REGISTER_FILTER
	pPages->cElems		= 2;
#else
	pPages->cElems		= 1;
#endif

	pPages->pElems		= (GUID*)CoTaskMemAlloc(sizeof(GUID) * pPages->cElems);
	pPages->pElems[0]	= __uuidof(CMPCVideoDecSettingsWnd);
	if (pPages->cElems>1) pPages->pElems[1]	= __uuidof(CMPCVideoDecCodecWnd);

	return S_OK;
}

STDMETHODIMP CMPCVideoDecFilter::CreatePage(const GUID& guid, IPropertyPage** ppPage)
{
	CheckPointer(ppPage, E_POINTER);

	if(*ppPage != NULL) return E_INVALIDARG;

	HRESULT hr;

	if(guid == __uuidof(CMPCVideoDecSettingsWnd))
	{
		(*ppPage = new CInternalPropertyPageTempl<CMPCVideoDecSettingsWnd>(NULL, &hr))->AddRef();
	}
	else if(guid == __uuidof(CMPCVideoDecCodecWnd))
	{
		(*ppPage = new CInternalPropertyPageTempl<CMPCVideoDecCodecWnd>(NULL, &hr))->AddRef();
	}

	return *ppPage ? S_OK : E_FAIL;
}


// IFfmpegDecFilter
STDMETHODIMP CMPCVideoDecFilter::Apply()
{
	CRegKey key;
	if(ERROR_SUCCESS == key.Create(HKEY_CURRENT_USER, _T("Software\\Gabest\\Filters\\MPC Video Decoder")))
	{
		key.SetDWORDValue(_T("ThreadNumber"), m_nThreadNumber);
		key.SetDWORDValue(_T("DiscardMode"), m_nDiscardMode);
		key.SetDWORDValue(_T("ErrorResilience"), m_nErrorResilience);
		key.SetDWORDValue(_T("IDCTAlgo"), m_nIDCTAlgo);
		key.SetDWORDValue(_T("EnableDXVA"), m_bEnableDXVA);
		key.SetDWORDValue(_T("ActiveCodecs"), m_nActiveCodecs);
		key.SetDWORDValue(_T("EnableFfmpeg"), m_bEnableFfmpeg);
	}
	return S_OK;
}

STDMETHODIMP CMPCVideoDecFilter::SetThreadNumber(int nValue)
{
	CAutoLock cAutoLock(&m_csProps);
	m_nThreadNumber = nValue;
	return S_OK;
}
STDMETHODIMP_(int) CMPCVideoDecFilter::GetThreadNumber()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_nThreadNumber;
}
STDMETHODIMP CMPCVideoDecFilter::SetEnableDXVA(bool fValue)
{
	CAutoLock cAutoLock(&m_csProps);
	m_bEnableDXVA = fValue;
	return S_OK;
}
STDMETHODIMP_(bool) CMPCVideoDecFilter::GetEnableDXVA()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_bEnableDXVA;
}
STDMETHODIMP CMPCVideoDecFilter::SetDiscardMode(int nValue)
{
	CAutoLock cAutoLock(&m_csProps);
	m_nDiscardMode = nValue;
	return S_OK;
}
STDMETHODIMP_(int) CMPCVideoDecFilter::GetDiscardMode()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_nDiscardMode;
}
STDMETHODIMP CMPCVideoDecFilter::SetErrorResilience(int nValue)
{
	CAutoLock cAutoLock(&m_csProps);
	m_nErrorResilience = nValue;
	return S_OK;
}
STDMETHODIMP_(int) CMPCVideoDecFilter::GetErrorResilience()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_nErrorResilience;
}
STDMETHODIMP CMPCVideoDecFilter::SetIDCTAlgo(int nValue)
{
	CAutoLock cAutoLock(&m_csProps);
	m_nIDCTAlgo = nValue;
	return S_OK;
}
STDMETHODIMP_(int) CMPCVideoDecFilter::GetIDCTAlgo()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_nIDCTAlgo;
}
STDMETHODIMP_(GUID*) CMPCVideoDecFilter::GetDXVADecoderGuid()
{
	if (m_pGraph == NULL) 
		return NULL;
	else
		return &m_DXVADecoderGUID;
}
STDMETHODIMP CMPCVideoDecFilter::SetActiveCodecs(MPC_VIDEO_CODEC nValue)
{
	CAutoLock cAutoLock(&m_csProps);
	m_nActiveCodecs = (int)nValue;
	return S_OK;
}
STDMETHODIMP_(MPC_VIDEO_CODEC) CMPCVideoDecFilter::GetActiveCodecs()
{
	CAutoLock cAutoLock(&m_csProps);
	return (MPC_VIDEO_CODEC)m_nActiveCodecs;
}
STDMETHODIMP CMPCVideoDecFilter::SetEnableFfmpeg(bool fValue)
{
	CAutoLock cAutoLock(&m_csProps);
	m_bEnableFfmpeg = fValue;
	return S_OK;
}
STDMETHODIMP_(bool) CMPCVideoDecFilter::GetEnableFfmpeg()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_bEnableFfmpeg;
}

STDMETHODIMP_(LPCTSTR) CMPCVideoDecFilter::GetVideoCardDescription()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_strDeviceDescription;
}