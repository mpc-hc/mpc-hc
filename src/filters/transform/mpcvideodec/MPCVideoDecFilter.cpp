/* 
 * $Id: MPCVideoDecFilter.cpp 249 2007-09-26 11:07:22Z casimir666 $
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

#include "..\..\..\DSUtil\DSUtil.h"

#include <moreuuids.h>

#include "DXVADecoderH264.h"

#undef free
#include <malloc.h>


/////
#define MAX_SUPPORTED_MODE			5
typedef struct
{
  const CLSID*			clsMinorType;
  const enum CodecID	nFFCodec;
  const int				fourcc;
  const	bool			bSupportDXVA;
  const GUID*			SupportedMode[MAX_SUPPORTED_MODE];
} FFMPEG_CODECS;


const FFMPEG_CODECS		ffCodecs[] =
{
	// Flash video
	{ &MEDIASUBTYPE_FLV1, CODEC_ID_FLV1, MAKEFOURCC('F','L','V','1'),	false, { &GUID_NULL } },

	// VP5
	{ &MEDIASUBTYPE_VP50, CODEC_ID_VP5,  MAKEFOURCC('V','P','5','0'),	false, { &GUID_NULL } },

	// VP6
	{ &MEDIASUBTYPE_VP60, CODEC_ID_VP6,  MAKEFOURCC('V','P','6','0'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_VP61, CODEC_ID_VP6,  MAKEFOURCC('V','P','6','1'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_VP62, CODEC_ID_VP6,  MAKEFOURCC('V','P','6','2'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_FLV4, CODEC_ID_VP6F, MAKEFOURCC('F','L','V','4'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_VP6F, CODEC_ID_VP6F, MAKEFOURCC('V','P','6','F'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_VP6A, CODEC_ID_VP6A, MAKEFOURCC('V','P','6','A'),	false, { &GUID_NULL } },

	// Mpeg2
	{ &MEDIASUBTYPE_MPEG2_VIDEO, CODEC_ID_MPEG2VIDEO, MAKEFOURCC('M','P','G','2'),	true, { /*&DXVA2_ModeMPEG2_MoComp,*/ &DXVA2_ModeMPEG2_VLD, &GUID_NULL } },

	// Xvid
	{ &MEDIASUBTYPE_XVID, CODEC_ID_MPEG4,  MAKEFOURCC('X','V','I','D'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_xvid, CODEC_ID_MPEG4,  MAKEFOURCC('x','v','i','d'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_XVIX, CODEC_ID_MPEG4,  MAKEFOURCC('X','V','I','X'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_xvix, CODEC_ID_MPEG4,  MAKEFOURCC('x','v','i','x'),	false, { &GUID_NULL } },

	// DivX
	{ &MEDIASUBTYPE_DX50, CODEC_ID_MPEG4,  MAKEFOURCC('D','X','5','0'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_dx50, CODEC_ID_MPEG4,  MAKEFOURCC('d','x','5','0'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_DIVX, CODEC_ID_MPEG4,  MAKEFOURCC('D','I','V','X'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_divx, CODEC_ID_MPEG4,  MAKEFOURCC('d','i','v','x'),	false, { &GUID_NULL } },

	// Other MPEG-4
	{ &MEDIASUBTYPE_MP4V, CODEC_ID_MPEG4,  MAKEFOURCC('M','P','4','V'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_mp4v, CODEC_ID_MPEG4,  MAKEFOURCC('m','p','4','v'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_M4S2, CODEC_ID_MPEG4,  MAKEFOURCC('M','4','S','2'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_m4s2, CODEC_ID_MPEG4,  MAKEFOURCC('m','4','s','2'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_MP4S, CODEC_ID_MPEG4,  MAKEFOURCC('M','P','4','S'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_mp4s, CODEC_ID_MPEG4,  MAKEFOURCC('m','p','4','s'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_3IV1, CODEC_ID_MPEG4,  MAKEFOURCC('3','I','V','1'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_3iv1, CODEC_ID_MPEG4,  MAKEFOURCC('3','i','v','1'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_3IV2, CODEC_ID_MPEG4,  MAKEFOURCC('3','I','V','2'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_3iv2, CODEC_ID_MPEG4,  MAKEFOURCC('3','i','v','2'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_3IVX, CODEC_ID_MPEG4,  MAKEFOURCC('3','I','V','X'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_3ivx, CODEC_ID_MPEG4,  MAKEFOURCC('3','i','v','x'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_BLZ0, CODEC_ID_MPEG4,  MAKEFOURCC('B','L','Z','0'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_blz0, CODEC_ID_MPEG4,  MAKEFOURCC('b','l','z','0'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_DM4V, CODEC_ID_MPEG4,  MAKEFOURCC('D','M','4','V'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_dm4v, CODEC_ID_MPEG4,  MAKEFOURCC('d','m','4','v'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_FFDS, CODEC_ID_MPEG4,  MAKEFOURCC('F','F','D','S'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_ffds, CODEC_ID_MPEG4,  MAKEFOURCC('f','f','d','s'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_FVFW, CODEC_ID_MPEG4,  MAKEFOURCC('F','V','F','W'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_fvfw, CODEC_ID_MPEG4,  MAKEFOURCC('f','v','f','w'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_DXGM, CODEC_ID_MPEG4,  MAKEFOURCC('D','X','G','M'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_dxgm, CODEC_ID_MPEG4,  MAKEFOURCC('d','x','g','m'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_FMP4, CODEC_ID_MPEG4,  MAKEFOURCC('F','M','P','4'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_fmp4, CODEC_ID_MPEG4,  MAKEFOURCC('f','m','p','4'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_HDX4, CODEC_ID_MPEG4,  MAKEFOURCC('H','D','X','4'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_hdx4, CODEC_ID_MPEG4,  MAKEFOURCC('h','d','x','4'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_LMP4, CODEC_ID_MPEG4,  MAKEFOURCC('L','M','P','4'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_lmp4, CODEC_ID_MPEG4,  MAKEFOURCC('l','m','p','4'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_NDIG, CODEC_ID_MPEG4,  MAKEFOURCC('N','D','I','G'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_ndig, CODEC_ID_MPEG4,  MAKEFOURCC('n','d','i','g'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_RMP4, CODEC_ID_MPEG4,  MAKEFOURCC('R','M','P','4'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_rmp4, CODEC_ID_MPEG4,  MAKEFOURCC('r','m','p','4'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_SMP4, CODEC_ID_MPEG4,  MAKEFOURCC('S','M','P','4'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_smp4, CODEC_ID_MPEG4,  MAKEFOURCC('s','m','p','4'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_SEDG, CODEC_ID_MPEG4,  MAKEFOURCC('S','E','D','G'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_sedg, CODEC_ID_MPEG4,  MAKEFOURCC('s','e','d','g'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_UMP4, CODEC_ID_MPEG4,  MAKEFOURCC('U','M','P','4'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_ump4, CODEC_ID_MPEG4,  MAKEFOURCC('u','m','p','4'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_WV1F, CODEC_ID_MPEG4,  MAKEFOURCC('W','V','1','F'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_wv1f, CODEC_ID_MPEG4,  MAKEFOURCC('w','v','1','f'),	false, { &GUID_NULL } },

	// MSMPEG-4
	{ &MEDIASUBTYPE_DIV3, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('D','I','V','3'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_div3, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('d','i','v','3'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_DVX3, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('D','V','X','3'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_dvx3, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('d','v','x','3'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_MP43, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('M','P','4','3'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_mp43, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('m','p','4','3'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_COL1, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('C','O','L','1'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_col1, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('c','o','l','1'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_DIV4, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('D','I','V','4'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_div4, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('d','i','v','4'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_DIV5, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('D','I','V','5'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_div5, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('d','i','v','5'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_DIV6, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('D','I','V','6'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_div6, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('d','i','v','6'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_AP41, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('A','P','4','1'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_ap41, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('a','p','4','1'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_MPG3, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('M','P','G','3'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_mpg3, CODEC_ID_MSMPEG4V3,  MAKEFOURCC('m','p','g','3'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_DIV2, CODEC_ID_MSMPEG4V2,  MAKEFOURCC('D','I','V','2'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_div2, CODEC_ID_MSMPEG4V2,  MAKEFOURCC('d','i','v','2'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_MP42, CODEC_ID_MSMPEG4V2,  MAKEFOURCC('M','P','4','2'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_mp42, CODEC_ID_MSMPEG4V2,  MAKEFOURCC('m','p','4','2'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_MPG4, CODEC_ID_MSMPEG4V1,  MAKEFOURCC('M','P','G','4'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_mpg4, CODEC_ID_MSMPEG4V1,  MAKEFOURCC('m','p','g','4'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_DIV1, CODEC_ID_MSMPEG4V1,  MAKEFOURCC('D','I','V','1'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_div1, CODEC_ID_MSMPEG4V1,  MAKEFOURCC('d','i','v','1'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_MP41, CODEC_ID_MSMPEG4V1,  MAKEFOURCC('M','P','4','1'),	false, { &GUID_NULL } },
	{ &MEDIASUBTYPE_mp41, CODEC_ID_MSMPEG4V1,  MAKEFOURCC('m','p','4','1'),	false, { &GUID_NULL } },

	// AMV Video
	{ &MEDIASUBTYPE_AMVV, CODEC_ID_AMV,  MAKEFOURCC('A','M','V','V'),	false, { &GUID_NULL } },

	// H264/AVC
	{ &MEDIASUBTYPE_H264, CODEC_ID_H264, MAKEFOURCC('H','2','6','4'),	true, { &DXVA2_ModeH264_E, &GUID_NULL } },
	{ &MEDIASUBTYPE_h264, CODEC_ID_H264, MAKEFOURCC('h','2','6','4'),	true, { &DXVA2_ModeH264_E, &GUID_NULL } },
	{ &MEDIASUBTYPE_X264, CODEC_ID_H264, MAKEFOURCC('X','2','6','4'),	true, { &DXVA2_ModeH264_E, &GUID_NULL } },
	{ &MEDIASUBTYPE_x264, CODEC_ID_H264, MAKEFOURCC('x','2','6','4'),	true, { &DXVA2_ModeH264_E, &GUID_NULL } },
	{ &MEDIASUBTYPE_VSSH, CODEC_ID_H264, MAKEFOURCC('V','S','S','H'),	true, { &DXVA2_ModeH264_E, &GUID_NULL } },
	{ &MEDIASUBTYPE_vssh, CODEC_ID_H264, MAKEFOURCC('v','s','s','h'),	true, { &DXVA2_ModeH264_E, &GUID_NULL } },
	{ &MEDIASUBTYPE_DAVC, CODEC_ID_H264, MAKEFOURCC('D','A','V','C'),	true, { &DXVA2_ModeH264_E, &GUID_NULL } },
	{ &MEDIASUBTYPE_davc, CODEC_ID_H264, MAKEFOURCC('d','a','v','c'),	true, { &DXVA2_ModeH264_E, &GUID_NULL } },
	{ &MEDIASUBTYPE_PAVC, CODEC_ID_H264, MAKEFOURCC('P','A','V','C'),	true, { &DXVA2_ModeH264_E, &GUID_NULL } },
	{ &MEDIASUBTYPE_pavc, CODEC_ID_H264, MAKEFOURCC('p','a','v','c'),	true, { &DXVA2_ModeH264_E, &GUID_NULL } },
	{ &MEDIASUBTYPE_AVC1, CODEC_ID_H264, MAKEFOURCC('A','V','C','1'),	true, { &DXVA2_ModeH264_E, &GUID_NULL } },
	{ &MEDIASUBTYPE_avc1, CODEC_ID_H264, MAKEFOURCC('a','v','c','1'),	true, { &DXVA2_ModeH264_E, &GUID_NULL } },

	// SVQ3
	{ &MEDIASUBTYPE_SVQ3, CODEC_ID_SVQ3, MAKEFOURCC('S','V','Q','3'),	true, { &DXVA2_ModeH264_E, &GUID_NULL } },

	// SVQ1
	{ &MEDIASUBTYPE_SVQ1, CODEC_ID_SVQ1, MAKEFOURCC('S','V','Q','1'),	true, { &GUID_NULL } },

	// WVC1
	{ &MEDIASUBTYPE_WVC1, CODEC_ID_VC1,  MAKEFOURCC('W','V','C','1'),	true, { &GUID_NULL } },
};

const AMOVIESETUP_MEDIATYPE CMPCVideoDecFilter::sudPinTypesIn[] =
{
	// Flash video
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_FLV1   },

	// VP5
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP50   },

	// VP6
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP60   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP61   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP62   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_FLV4   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP6F   },
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_VP6A   },

	// Mpeg2
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_MPEG2_VIDEO   },

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

	// VC1
	{ &MEDIATYPE_Video, &MEDIASUBTYPE_WVC1   },
};
const int CMPCVideoDecFilter::sudPinTypesInCount = countof(CMPCVideoDecFilter::sudPinTypesIn);


const AMOVIESETUP_MEDIATYPE CMPCVideoDecFilter::sudPinTypesOut[] =
{
	{&MEDIATYPE_Video, &MEDIASUBTYPE_NV12},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_NV24},
};
const int CMPCVideoDecFilter::sudPinTypesOutCount = countof(CMPCVideoDecFilter::sudPinTypesOut);


CMPCVideoDecFilter::CMPCVideoDecFilter(LPUNKNOWN lpunk, HRESULT* phr) 
	: CBaseVideoFilter(NAME("MPC - Video decoder"), lpunk, phr, __uuidof(this))
{
	if(phr) *phr = S_OK;

	if (m_pOutput)	delete m_pOutput;
	if(!(m_pOutput = new CVideoDecOutputPin(NAME("CVideoDecOutputPin"), this, phr, L"Output"))) *phr = E_OUTOFMEMORY;

/*	CRegKey key;
	if(ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Gabest\\Filters\\MPEG Audio Decoder"), KEY_READ))
	{
		DWORD dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("SampleFormat"), dw)) m_iSampleFormat = (SampleFormat)dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("Normalize"), dw)) m_fNormalize = !!dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("Boost"), dw)) m_boost = *(float*)&dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("Ac3SpeakerConfig"), dw)) m_iSpeakerConfig[ac3] = (int)dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("DtsSpeakerConfig"), dw)) m_iSpeakerConfig[dts] = (int)dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("AacSpeakerConfig"), dw)) m_iSpeakerConfig[aac] = (int)dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("Ac3DynamicRangeControl"), dw)) m_fDynamicRangeControl[ac3] = !!dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("DtsDynamicRangeControl"), dw)) m_fDynamicRangeControl[dts] = !!dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("AacDynamicRangeControl"), dw)) m_fDynamicRangeControl[aac] = !!dw;
	}*/

	m_pCpuId			= new CCpuId();
	m_pAVCodec			= NULL;
	m_pAVCtx			= NULL;
	m_pFrame			= NULL;
	m_nCodecNb			= -1;

	m_nWorkaroundBug	= 1;		// TODO : add config in property page
	m_nErrorConcealment	= 3;
	m_nErrorResilience	= 1;
	m_nThreadNumber		= 1; //m_CpuId.GetProcessorNumber();

	m_bUseDXVA2			= false;
	m_pDXVADecoder		= NULL;

#ifdef __USE_FFMPEG_DLL	
	// Test with ffmpeg as DLL
	HMODULE		hLib = LoadLibrary (_T("libavcodec.dll"));
	if (hLib)
	{
		ff_avcodec_init				= (FUNC_AVCODEC_INIT)			GetProcAddress (hLib, "avcodec_init");
		ff_avcodec_register_all		= (FUNC_AVCODEC_REGISTER_ALL)	GetProcAddress (hLib, "avcodec_register_all");
		ff_avcodec_find_decoder		= (FUNC_AVCODEC_FIND_DECODER)	GetProcAddress (hLib, "avcodec_find_decoder");
		ff_avcodec_alloc_context	= (FUNC_AVCODEC_ALLOC_CONTEXT)	GetProcAddress (hLib, "avcodec_alloc_context");
		ff_avcodec_alloc_frame		= (FUNC_AVCODEC_ALLOC_FRAME)	GetProcAddress (hLib, "avcodec_alloc_frame");
		ff_avcodec_open				= (FUNC_AVCODEC_OPEN)			GetProcAddress (hLib, "avcodec_open");
		ff_avcodec_decode_video		= (FUNC_AVCODEC_DECODE_VIDEO)	GetProcAddress (hLib, "avcodec_decode_video");
		ff_av_log_set_callback		= (FUNC_AV_LOG_SET_CALLBACK)	GetProcAddress (hLib, "av_log_set_callback");
		ff_avcodec_close			= (FUNC_AVCODEC_CLOSE)			GetProcAddress (hLib, "avcodec_close");
		ff_avcodec_thread_free		= (FUNC_AVCODEC_THREAD_FREE)	GetProcAddress (hLib, "avcodec_thread_free");
		ff_avcodec_thread_init		= (FUNC_AVCODEC_THREAD_INIT)	GetProcAddress (hLib, "avcodec_thread_init");
		ff_av_free					= (FUNC_AV_FREE)				GetProcAddress (hLib, "av_free");
	}
#else
	// Test with ffmpeg as static library
	ff_avcodec_init				= avcodec_init;
	ff_avcodec_register_all		= avcodec_register_all;
	ff_avcodec_find_decoder		= avcodec_find_decoder;
	ff_avcodec_alloc_context	= avcodec_alloc_context;
	ff_avcodec_alloc_frame		= avcodec_alloc_frame;
	ff_avcodec_open				= avcodec_open;
	ff_avcodec_decode_video		= avcodec_decode_video;
	ff_av_log_set_callback		= av_log_set_callback;
	ff_avcodec_close			= avcodec_close;
	ff_avcodec_thread_free		= avcodec_thread_free;
	ff_avcodec_thread_init		= avcodec_thread_init;
	ff_av_free					= av_free;
#endif

	ff_avcodec_init();
	ff_avcodec_register_all();
	ff_av_log_set_callback(LogLibAVCodec);
}



CMPCVideoDecFilter::~CMPCVideoDecFilter()
{
	Cleanup();

	delete m_pCpuId;

	/*
	CRegKey key;
	if(ERROR_SUCCESS == key.Create(HKEY_CURRENT_USER, _T("Software\\Gabest\\Filters\\MPEG Audio Decoder")))
	{
		key.SetDWORDValue(_T("SampleFormat"), m_iSampleFormat);
		key.SetDWORDValue(_T("Normalize"), m_fNormalize);
		key.SetDWORDValue(_T("Boost"), *(DWORD*)&m_boost);
		key.SetDWORDValue(_T("Ac3SpeakerConfig"), m_iSpeakerConfig[ac3]);
		key.SetDWORDValue(_T("DtsSpeakerConfig"), m_iSpeakerConfig[dts]);
		key.SetDWORDValue(_T("AacSpeakerConfig"), m_iSpeakerConfig[aac]);
		key.SetDWORDValue(_T("Ac3DynamicRangeControl"), m_fDynamicRangeControl[ac3]);
		key.SetDWORDValue(_T("DtsDynamicRangeControl"), m_fDynamicRangeControl[dts]);
		key.SetDWORDValue(_T("AacDynamicRangeControl"), m_fDynamicRangeControl[aac]);
	}*/
}


int CMPCVideoDecFilter::PictWidth()
{
	return m_pAVCtx ? m_pAVCtx->width  : 0; 
}


int CMPCVideoDecFilter::PictHeight()
{
	return m_pAVCtx ? m_pAVCtx->height : 0;
}

int CMPCVideoDecFilter::FindCodec(const CMediaType* mtIn)
{
	for (int i=0; i<countof(ffCodecs); i++)
		if (mtIn->subtype == *ffCodecs[i].clsMinorType)
			return i;

	return -1;
}

void CMPCVideoDecFilter::Cleanup()
{
	if (m_pDXVADecoder)
	{
		delete m_pDXVADecoder;
		m_pDXVADecoder = NULL;
	}

	if (m_pAVCtx)
	{
		if (m_pAVCtx->intra_matrix)			free(m_pAVCtx->intra_matrix);
		if (m_pAVCtx->inter_matrix)			free(m_pAVCtx->inter_matrix);
		if (m_pAVCtx->intra_matrix_luma)	free(m_pAVCtx->intra_matrix_luma);
		if (m_pAVCtx->intra_matrix_chroma)	free(m_pAVCtx->intra_matrix_chroma);
		if (m_pAVCtx->inter_matrix_luma)	free(m_pAVCtx->inter_matrix_luma);
		if (m_pAVCtx->inter_matrix_chroma)	free(m_pAVCtx->inter_matrix_chroma);

		if (m_pAVCtx->slice_offset) ff_av_free(m_pAVCtx->slice_offset);
		if (m_pAVCtx) ff_avcodec_close(m_pAVCtx);

		if ((m_nThreadNumber > 1) && IsMultiThreadSupported (ffCodecs[m_nCodecNb].nFFCodec))
			ff_avcodec_thread_free (m_pAVCtx);

		ff_av_free(m_pAVCtx);
	}
	if (m_pFrame)	ff_av_free(m_pFrame);

	m_pAVCodec	= NULL;
	m_pAVCtx	= NULL;
	m_pFrame	= NULL;
	m_nCodecNb	= -1;
}

void CMPCVideoDecFilter::LogLibAVCodec(void* par,int level,const char *fmt,va_list valist)
{
#ifdef _DEBUG
	//AVCodecContext*	m_pAVCtx = (AVCodecContext*) par;

	//char		Msg [500];
	//snprintf (Msg, sizeof(Msg), fmt, valist);
	//TRACE("AVLIB : %s", Msg);
#endif
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
	return (nCodec==CODEC_ID_MPEG1VIDEO) || (nCodec==CODEC_ID_MPEG2VIDEO) || (nCodec==CODEC_ID_H264);
}


HRESULT CMPCVideoDecFilter::SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt)
{
	int		nNewCodec;

	nNewCodec = FindCodec(pmt);
	if ((direction == PINDIR_INPUT) && (nNewCodec != -1) && (nNewCodec != m_nCodecNb))
	{
		m_nCodecNb	= nNewCodec;

		m_pAVCodec	= ff_avcodec_find_decoder(ffCodecs[nNewCodec].nFFCodec);
		CheckPointer (m_pAVCodec, VFW_E_UNSUPPORTED_VIDEO);

		m_pAVCtx	= ff_avcodec_alloc_context();
		CheckPointer (m_pAVCtx,	  E_POINTER);

		if ((m_nThreadNumber > 1) && IsMultiThreadSupported (ffCodecs[m_nCodecNb].nFFCodec))
			ff_avcodec_thread_init(m_pAVCtx, m_nThreadNumber);
		m_pFrame = ff_avcodec_alloc_frame();
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

			if (m_pAVCtx->codec_tag == MAKEFOURCC('a','v','c','1'))
			{
				m_pAVCtx->nal_length_size = mpg2v->dwFlags;
			}
		}
		
		m_pAVCtx->intra_matrix			= (uint16_t*)calloc(sizeof(uint16_t),64);
		m_pAVCtx->inter_matrix			= (uint16_t*)calloc(sizeof(uint16_t),64);
		m_pAVCtx->intra_matrix_luma		= (uint16_t*)calloc(sizeof(uint16_t),16);
		m_pAVCtx->intra_matrix_chroma	= (uint16_t*)calloc(sizeof(uint16_t),16);
		m_pAVCtx->inter_matrix_luma		= (uint16_t*)calloc(sizeof(uint16_t),16);
		m_pAVCtx->inter_matrix_chroma	= (uint16_t*)calloc(sizeof(uint16_t),16);
		m_pAVCtx->codec_tag				= ffCodecs[nNewCodec].fourcc;
		m_pAVCtx->workaround_bugs		= m_nWorkaroundBug;	// TODO !! 
		m_pAVCtx->error_concealment		= m_nErrorConcealment;
		m_pAVCtx->error_resilience		= m_nErrorResilience;
		m_pAVCtx->dsp_mask				= FF_MM_FORCE | m_pCpuId->GetFeatures();

		m_pAVCtx->postgain				= 1.0f;
		m_pAVCtx->scenechange_factor	= 1;
		m_pAVCtx->debug_mv				= 1;

		AllocExtradata (m_pAVCtx, pmt);		

		if (ff_avcodec_open(m_pAVCtx, m_pAVCodec)<0)
			return VFW_E_INVALIDMEDIATYPE;
	}

	return __super::SetMediaType(direction, pmt);
}


HRESULT CMPCVideoDecFilter::GetMediaType(int iPosition, CMediaType* pmt)
{
	HRESULT			hr			= __super::GetMediaType (iPosition, pmt);
	CMediaType&		pmtInput	= m_pInput->CurrentMediaType();

	if (hr == S_OK)
	{
		VIDEOINFOHEADER* vih      = (VIDEOINFOHEADER*)pmt->Format();
		VIDEOINFOHEADER* vihInput = (VIDEOINFOHEADER*)pmtInput.Format();

		if (vih && vihInput)
		{
			memcpy (&vih->rcSource, &vihInput->rcSource, sizeof(RECT));
			memcpy (&vih->rcTarget, &vihInput->rcTarget, sizeof(RECT));
		}
	}

	return hr;
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
	if ( (direction==PINDIR_OUTPUT) &&
		 (m_nCodecNb != -1) &&
		 ffCodecs[m_nCodecNb].bSupportDXVA &&
		 SUCCEEDED (ConfigureDXVA2 (pReceivePin)) &&
		 SUCCEEDED (SetEVRForDXVA2 (pReceivePin)) )
	{
		m_bUseDXVA2  = true;
	}

	return __super::CompleteConnect (direction, pReceivePin);
}


HRESULT CMPCVideoDecFilter::DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties)
{
	if (m_bUseDXVA2)
	{
		HRESULT					hr;
		ALLOCATOR_PROPERTIES	Actual;

		if(m_pInput->IsConnected() == FALSE) return E_UNEXPECTED;

		pProperties->cBuffers += m_DecoderConfig.ConfigMinRenderTargetBuffCount;

		if(FAILED(hr = pAllocator->SetProperties(pProperties, &Actual))) 
			return hr;

		return pProperties->cBuffers > Actual.cBuffers || pProperties->cbBuffer > Actual.cbBuffer
			? E_FAIL
			: NOERROR;
	}
	else
		return __super::DecideBufferSize (pAllocator, pProperties);
}


HRESULT CMPCVideoDecFilter::Transform(IMediaSample* pIn)
{
	HRESULT			hr;
	BYTE*			pDataIn = NULL;
	int				nSize;
	int				got_picture;
	REFERENCE_TIME	rtStart = _I64_MIN, rtStop = _I64_MIN;

	if(FAILED(hr = pIn->GetPointer(&pDataIn)))
		return hr;

	nSize	= pIn->GetActualDataLength();
	hr		= pIn->GetTime(&rtStart, &rtStop);

	//FILE*	File = fopen ("e:\\temp\\h264.bin", "wb");
	//fwrite (pDataIn, nSize, 1, File);
	//fclose (File);

	if (!m_bUseDXVA2)
	{
		int		used_bytes;

		m_pAVCtx->parserRtStart=&rtStart;
		
		while (nSize > 0)
		{
			used_bytes = ff_avcodec_decode_video (m_pAVCtx, m_pFrame, &got_picture, pDataIn, nSize);
			if (!got_picture || !m_pFrame->data[0]) return S_OK;

			if(pIn->IsPreroll() == S_OK || rtStart < 0)
				return S_OK;

			CComPtr<IMediaSample>	pOut;
			BYTE*					pDataOut = NULL;
			if(FAILED(hr = GetDeliveryBuffer(m_pAVCtx->width, m_pAVCtx->height, &pOut)) || FAILED(hr = pOut->GetPointer(&pDataOut)))
				return hr;


			//rtStart = m_pFrame->rtStart;
			//rtStop = m_pFrame->rtStart + 1;
			TRACE ("CMPCVideoDecFilter::Transform    %ld\n", rtStart/400000);

			pOut->SetTime(&rtStart, &rtStop);
			pOut->SetMediaTime(NULL, NULL);
			pOut->SetDiscontinuity(pIn->IsDiscontinuity() == S_OK);

			CopyBuffer(pDataOut, m_pFrame->data, m_pAVCtx->width, m_pAVCtx->height, m_pFrame->linesize[0], MEDIASUBTYPE_I420, false);

			hr = m_pOutput->Deliver(pOut);

			nSize	-= used_bytes;
			pDataIn += used_bytes;
		}

		return hr;
	}
	else
	{
		CheckPointer (m_pDXVADecoder, E_UNEXPECTED);
		CComPtr<IMediaSample>		pOut;

		hr = m_pOutput->GetDeliveryBuffer(&pOut, 0, 0, 0);

		hr = m_pDXVADecoder->DecodeFrame (pDataIn, nSize, pOut);

		pOut->SetTime(&rtStart, &rtStop);
		pOut->SetDiscontinuity(pIn->IsDiscontinuity() == S_OK);
		hr = m_pOutput->Deliver(pOut);
		LOG(_T("m_pOutput->Deliver  hr=0x%08x"), hr);
		return hr;
	}
}

void CMPCVideoDecFilter::FillInVideoDescription(DXVA2_VideoDesc *pDesc)
{
	memset (&m_VideoDesc, 0, sizeof(m_VideoDesc));
	pDesc->SampleWidth	= m_pAVCtx->width;
	pDesc->SampleHeight	= m_pAVCtx->height;
	pDesc->Format       = D3DFMT_YUY2;
}

BOOL CMPCVideoDecFilter::IsSupportedDecoderMode(const GUID& mode)
{
	if (ffCodecs[m_nCodecNb].bSupportDXVA)
	{
		for (int i=0; i<MAX_SUPPORTED_MODE; i++)
		{
			if (*ffCodecs[m_nCodecNb].SupportedMode[i] == GUID_NULL) 
				break;
			else if (*ffCodecs[m_nCodecNb].SupportedMode[i] == mode)
				return true;
		}
	}

	return false;
}

BOOL CMPCVideoDecFilter::IsSupportedDecoderConfig(const D3DFORMAT nD3DFormat, const DXVA2_ConfigPictureDecode& config)
{
	bool	bRet;

	bRet = ((config.ConfigBitstreamRaw == 2) && (nD3DFormat == MAKEFOURCC('N', 'V', '1', '2')) );

	LOG (_T("IsSupportedDecoderConfig  0x%08x  %d"), nD3DFormat, bRet);
	return bRet;
}

HRESULT CMPCVideoDecFilter::FindDecoderConfiguration(IDirectXVideoDecoderService *pDecoderService,
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

    if (SUCCEEDED(hr))
    {
        // Look for a format that matches our output format.
        for (UINT iFormat = 0; iFormat < cFormats;  iFormat++)
        {
            // Fill in the video description. Set the width, height, format, and frame rate.
            FillInVideoDescription(&m_VideoDesc); // Private helper function.
            m_VideoDesc.Format = pFormats[iFormat];

            // Get the available configurations.
            hr = pDecoderService->GetDecoderConfigurations(guidDecoder, &m_VideoDesc, NULL, &cConfigurations, &pConfig);

            if (FAILED(hr))
            {
                break;
            }

            // Find a supported configuration.
            for (UINT iConfig = 0; iConfig < cConfigurations; iConfig++)
            {
                if (IsSupportedDecoderConfig(pFormats[iConfig], pConfig[iConfig]))
                {
                    // This configuration is good.
                    *pbFoundDXVA2Configuration = TRUE;
                    *pSelectedConfig = pConfig[iConfig];
                    break;
                }
            }

            CoTaskMemFree(pConfig);
            break;

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
            hr = FindDecoderConfiguration(pDecoderService, pDecoderGuids[iGuid], &config, &bFoundDXVA2Configuration);

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

    if (!bFoundDXVA2Configuration)
    {
        hr = E_FAIL; // Unable to find a configuration.
    }

    if (SUCCEEDED(hr))
    {
        // Store the things we will need later.
		m_pDeviceManager	= pDeviceManager;
        m_pDecoderService	= pDecoderService;

        m_DecoderConfig		= config;
        m_DecoderGuid		= guidDecoder;
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
	HRESULT		hr;
	m_pDecoderRenderTarget	= NULL;
	CComPtr<IDirectXVideoDecoder>	pDecoder;

	//hr = m_pDecoderService->CreateSurface (m_pAVCtx->width,m_pAVCtx->height, 0, D3DFMT_A8R8G8B8, 
	//							D3DPOOL_DEFAULT, 0, DXVA2_VideoDecoderRenderTarget, &m_pDecoderRenderTarget, NULL);

	hr = m_pDecoderService->CreateVideoDecoder (m_DecoderGuid, &m_VideoDesc, &m_DecoderConfig, 
								pDecoderRenderTargets, nNumRenderTargets, &pDecoder);

	if (SUCCEEDED (hr))
		m_pDXVADecoder	= CDXVADecoder::CreateDecoder (this, pDecoder, &m_DecoderGuid);

	return hr;
}


// ISpecifyPropertyPages2

STDMETHODIMP CMPCVideoDecFilter::GetPages(CAUUID* pPages)
{
	CheckPointer(pPages, E_POINTER);

	pPages->cElems		= 1;
	pPages->pElems		= (GUID*)CoTaskMemAlloc(sizeof(GUID) * pPages->cElems);
	pPages->pElems[0]	= __uuidof(CMPCVideoDecSettingsWnd);

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

	return *ppPage ? S_OK : E_FAIL;
}


// IFfmpegDecFilter




