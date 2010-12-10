/*
 * $Id$
 *
 * (C) 2010 see AUTHORS
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

#pragma once

#include "internal_filter_config.h"

enum
{
	SOURCE_FILTER,
	DECODER,
	DXVA_DECODER,
	FFMPEG_DECODER
};

enum SOURCE_FILTER
{
#if INTERNAL_SOURCEFILTER_CDDA
	SRC_CDDA,
#endif
#if INTERNAL_SOURCEFILTER_CDXA
	SRC_CDXA,
#endif
#if INTERNAL_SOURCEFILTER_VTS
	SRC_VTS,
#endif
#if INTERNAL_SOURCEFILTER_FLIC
	SRC_FLIC,
#endif
#if INTERNAL_SOURCEFILTER_DVSOURCE
	SRC_D2V,
#endif
#if INTERNAL_SOURCEFILTER_DTSAC3
	SRC_DTSAC3,
#endif
#if INTERNAL_SOURCEFILTER_MATROSKA
	SRC_MATROSKA,
#endif
#if INTERNAL_SOURCEFILTER_SHOUTCAST
	SRC_SHOUTCAST,
#endif
#if INTERNAL_SOURCEFILTER_REALMEDIA
	SRC_REALMEDIA,
#endif
#if INTERNAL_SOURCEFILTER_AVI
	SRC_AVI,
#endif
#if INTERNAL_SOURCEFILTER_OGG
	SRC_OGG,
#endif
#if INTERNAL_SOURCEFILTER_MPEG
	SRC_MPEG,
#endif
#if INTERNAL_SOURCEFILTER_MPEGAUDIO
	SRC_MPA,
#endif
#if INTERNAL_SOURCEFILTER_DSM
	SRC_DSM,
#endif
	SRC_SUBS,
#if INTERNAL_SOURCEFILTER_MP4
	SRC_MP4,
#endif
#if INTERNAL_SOURCEFILTER_FLV
	SRC_FLV,
#endif
#if INTERNAL_SOURCEFILTER_FLAC
	SRC_FLAC,
#endif
	SRC_LAST
};

enum DECODER
{
#if INTERNAL_DECODER_MPEG1
	TRA_MPEG1,
#endif
#if INTERNAL_DECODER_MPEG2
	TRA_MPEG2,
#endif
#if INTERNAL_DECODER_REALVIDEO
	TRA_RV,
#endif
#if INTERNAL_DECODER_REALAUDIO
	TRA_RA,
#endif
#if INTERNAL_DECODER_MPEGAUDIO
	TRA_MPA,
#endif
#if INTERNAL_DECODER_DTS
	TRA_DTS,
	TRA_LPCM,
#endif
#if INTERNAL_DECODER_AC3
	TRA_AC3,
#endif
#if INTERNAL_DECODER_AAC
	TRA_AAC,
#endif
#if INTERNAL_DECODER_PS2AUDIO
	TRA_PS2AUD,
#endif
#if INTERNAL_DECODER_VORBIS
	TRA_VORBIS,
#endif
#if INTERNAL_DECODER_FLAC
	TRA_FLAC,
#endif
#if INTERNAL_DECODER_NELLYMOSER
	TRA_NELLY,
#endif
#if INTERNAL_DECODER_AMR
	TRA_AMR,
#endif
#if INTERNAL_DECODER_PCM
	TRA_PCM,
#endif
	TRA_LAST
};

enum DXVA_DECODER
{
#if INTERNAL_DECODER_H264_DXVA
	TRA_DXVA_H264,
#endif
#if INTERNAL_DECODER_VC1_DXVA
	TRA_DXVA_VC1,
#endif
#if INTERNAL_DECODER_MPEG2_DXVA
	TRA_DXVA_MPEG2,
#endif
	TRA_DXVA_LAST,
// dummy values (needed in FGManager.cpp)
#if !INTERNAL_DECODER_H264_DXVA & INTERNAL_DECODER_H264
	TRA_DXVA_H264,
#endif
#if !INTERNAL_DECODER_VC1_DXVA & INTERNAL_DECODER_VC1
	TRA_DXVA_VC1,
#endif
};

enum FFMPEG_DECODER
{
#if INTERNAL_DECODER_H264
	FFM_H264,
#endif
#if INTERNAL_DECODER_VC1
	FFM_VC1,
#endif
#if INTERNAL_DECODER_FLV
	FFM_FLV4,
#endif
#if INTERNAL_DECODER_VP6
	FFM_VP62,
#endif
#if INTERNAL_DECODER_VP8
	FFM_VP8,
#endif
#if INTERNAL_DECODER_XVID
	FFM_XVID,
#endif
#if INTERNAL_DECODER_DIVX
	FFM_DIVX,
#endif
#if INTERNAL_DECODER_MSMPEG4
	FFM_MSMPEG4,
#endif
#if INTERNAL_DECODER_WMV
	FFM_WMV,
#endif
#if INTERNAL_DECODER_SVQ
	FFM_SVQ3,
#endif
#if INTERNAL_DECODER_H263
	FFM_H263,
#endif
#if INTERNAL_DECODER_THEORA
	FFM_THEORA,
#endif
#if INTERNAL_DECODER_AMVV
	FFM_AMVV,
#endif
	FFM_LAST,
// dummy values (needed in FGManager.cpp)
#if !INTERNAL_DECODER_H264 & INTERNAL_DECODER_H264_DXVA
	FFM_H264,
#endif
#if !INTERNAL_DECODER_VC1 & INTERNAL_DECODER_VC1_DXVA
	FFM_VC1,
#endif
};
