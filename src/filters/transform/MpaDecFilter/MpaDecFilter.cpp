/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
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
#include <MMReg.h>
#include <sys/timeb.h>
#include "MpaDecFilter.h"

#include "../../../DSUtil/DSUtil.h"

#ifdef REGISTER_FILTER
#ifdef _WIN64
#ifdef _DEBUG
extern "C" int mingw_app_type = 1; /* 0:console, 1:windows.  */
#endif
void *__imp_toupper = toupper;
void *__imp_time64 = _time64;
#endif

#include <InitGuid.h>
#endif
#include <moreuuids.h>

#include <vector>
#include <ffmpeg/libavcodec/avcodec.h>

#include <libflac/include/FLAC/stream_decoder.h>

#define INT8_PEAK       128
#define INT16_PEAK      32768
#define INT24_PEAK      8388608
#define INT32_PEAK      2147483648

#define INT24_MAX       8388607i32
#define INT24_MIN     (-8388607i32 - 1)

#define AC3_HEADER_SIZE 7
#define MAX_JITTER      1000000i64 // +-100ms jitter is allowed for now 

#if HAS_FFMPEG_AUDIO_DECODERS
typedef struct {
	const CLSID*       clsMinorType;
	const enum CodecID nFFCodec;
} FFMPEG_AUDIO_CODECS;

static const FFMPEG_AUDIO_CODECS ffAudioCodecs[] = {
#if INTERNAL_DECODER_AMR
	// AMR
	{ &MEDIASUBTYPE_AMR,		CODEC_ID_AMR_NB },
	{ &MEDIASUBTYPE_SAMR,		CODEC_ID_AMR_NB },
	{ &MEDIASUBTYPE_SAWB,		CODEC_ID_AMR_WB },
#endif
#if INTERNAL_DECODER_AAC
	// AAC
	{ &MEDIASUBTYPE_AAC,		CODEC_ID_AAC },
	{ &MEDIASUBTYPE_MP4A,		CODEC_ID_AAC },
	{ &MEDIASUBTYPE_mp4a,		CODEC_ID_AAC },
	{ &MEDIASUBTYPE_AAC_ADTS,	CODEC_ID_AAC },
	{ &MEDIASUBTYPE_LATM_AAC,	CODEC_ID_AAC_LATM },
#endif
#if INTERNAL_DECODER_ALAC
	// ALAC
	{ &MEDIASUBTYPE_ALAC,		CODEC_ID_ALAC },
#endif
#if INTERNAL_DECODER_ALS
	// MPEG-4 ALS
	{ &MEDIASUBTYPE_ALS,		CODEC_ID_MP4ALS },
#endif
#if INTERNAL_DECODER_VORBIS
	// Ogg Vorbis
	{ &MEDIASUBTYPE_Vorbis2,	CODEC_ID_VORBIS },
#endif
#if INTERNAL_DECODER_NELLYMOSER
	// NellyMoser
	{ &MEDIASUBTYPE_NELLYMOSER,	CODEC_ID_NELLYMOSER },
#endif
#if INTERNAL_DECODER_ADPCM
	// Qt ADPCM
	{ &MEDIASUBTYPE_IMA4,		CODEC_ID_ADPCM_IMA_QT },
	// FLV ADPCM
	{ &MEDIASUBTYPE_ADPCM_SWF,	CODEC_ID_ADPCM_SWF    },
	// AMV IMA ADPCM
	{ &MEDIASUBTYPE_ADPCM_AMV,	CODEC_ID_ADPCM_IMA_AMV},
#endif
#if INTERNAL_DECODER_MPEGAUDIO
	// MPEG Audio
	{ &MEDIASUBTYPE_MPEG1Packet,		CODEC_ID_MP2 },
	{ &MEDIASUBTYPE_MPEG1Payload,		CODEC_ID_MP2 },
	{ &MEDIASUBTYPE_MPEG1AudioPayload,	CODEC_ID_MP2 },
	{ &MEDIASUBTYPE_MPEG2_AUDIO,		CODEC_ID_MP2 },
	{ &MEDIASUBTYPE_MP3,				CODEC_ID_MP3 },
#endif
#if INTERNAL_DECODER_REALAUDIO
	// RealMedia Audio
	{ &MEDIASUBTYPE_14_4,		CODEC_ID_RA_144	},
	{ &MEDIASUBTYPE_28_8,		CODEC_ID_RA_288	},
	{ &MEDIASUBTYPE_ATRC,		CODEC_ID_ATRAC3	},
	{ &MEDIASUBTYPE_COOK,		CODEC_ID_COOK	},
	{ &MEDIASUBTYPE_SIPR,		CODEC_ID_SIPR	},
	{ &MEDIASUBTYPE_RAAC,		CODEC_ID_AAC	},
	{ &MEDIASUBTYPE_RACP,		CODEC_ID_AAC	},
#endif
#if INTERNAL_DECODER_AC3
	{ &MEDIASUBTYPE_DOLBY_DDPLUS, CODEC_ID_EAC3   },
	{ &MEDIASUBTYPE_DOLBY_TRUEHD, CODEC_ID_TRUEHD },
#endif
	{ &MEDIASUBTYPE_None,		CODEC_ID_NONE },
};
#endif

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] = {
#if INTERNAL_DECODER_MPEGAUDIO
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_MP3},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_MPEG1AudioPayload},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_MPEG1Payload},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_MPEG1Packet},
	{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_MPEG2_AUDIO},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_MPEG2_AUDIO},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_MPEG2_AUDIO},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_MPEG2_AUDIO},
#endif
#if INTERNAL_DECODER_AC3
	{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_DOLBY_AC3},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_DOLBY_AC3},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_DOLBY_AC3},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_DOLBY_AC3},
	{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_DOLBY_DDPLUS},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_DOLBY_DDPLUS},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_DOLBY_DDPLUS},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_DOLBY_DDPLUS},
	{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_DOLBY_TRUEHD},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_DOLBY_TRUEHD},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_DOLBY_TRUEHD},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_DOLBY_TRUEHD},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_WAVE_DOLBY_AC3},
#endif
#if INTERNAL_DECODER_DTS
	{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_DTS},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_DTS},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_DTS},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_DTS},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_WAVE_DTS},
#endif
#if INTERNAL_DECODER_LPCM
	{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_DVD_LPCM_AUDIO},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_DVD_LPCM_AUDIO},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_DVD_LPCM_AUDIO},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_DVD_LPCM_AUDIO},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_HDMV_LPCM_AUDIO},
#endif
#if INTERNAL_DECODER_AAC
	{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_AAC},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_AAC},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_AAC},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_AAC},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_LATM_AAC},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_AAC_ADTS},
	{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_MP4A},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_MP4A},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_MP4A},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_MP4A},
	{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_mp4a},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_mp4a},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_mp4a},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_mp4a},
#endif
#if INTERNAL_DECODER_AMR
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_AMR},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_SAMR},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_SAWB},
#endif
#if INTERNAL_DECODER_PS2AUDIO
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_PS2_PCM},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_PS2_PCM},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PS2_PCM},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_PS2_ADPCM},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_PS2_ADPCM},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PS2_ADPCM},
#endif
#if INTERNAL_DECODER_VORBIS
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_Vorbis2},
#endif
#if INTERNAL_DECODER_FLAC
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_FLAC_FRAMED},
#endif
#if INTERNAL_DECODER_NELLYMOSER
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_NELLYMOSER},
#endif
#if INTERNAL_DECODER_PCM
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_NONE},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_RAW},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_TWOS},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_SOWT},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_IN24},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_IN32},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_FL32},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_FL64},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_IEEE_FLOAT}, // only for 64-bit float PCM
#endif
#if INTERNAL_DECODER_ADPCM
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_IMA4},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_ADPCM_SWF},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_ADPCM_AMV},
#endif
#if INTERNAL_DECODER_REALAUDIO
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_14_4},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_28_8},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_ATRC},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_COOK},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_DNET},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_SIPR},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_RAAC},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_RACP},
#endif
#if INTERNAL_DECODER_ALAC
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_ALAC},
#endif
#if INTERNAL_DECODER_ALS
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_ALS},
#endif
#if !HAS_OTHER_AUDIO_DECODERS && !HAS_FFMPEG_AUDIO_DECODERS
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_None} // just to prevent compilation error
#endif
};

#ifdef REGISTER_FILTER
const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] = {
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_PCM},
};

const AMOVIESETUP_PIN sudpPins[] = {
	{L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, _countof(sudPinTypesIn), sudPinTypesIn},
	{L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, _countof(sudPinTypesOut), sudPinTypesOut}
};

const AMOVIESETUP_FILTER sudFilter[] = {
	{&__uuidof(CMpaDecFilter), MPCAudioDecName, /*MERIT_DO_NOT_USE*/0x40000001, _countof(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory},
};

CFactoryTemplate g_Templates[] = {
	{sudFilter[0].strName, &__uuidof(CMpaDecFilter), CreateInstance<CMpaDecFilter>, NULL, &sudFilter[0]},
	{L"CMpaDecPropertyPage", &__uuidof(CMpaDecSettingsWnd), CreateInstance<CInternalPropertyPageTempl<CMpaDecSettingsWnd> >},
};

int g_cTemplates = _countof(g_Templates);

STDAPI DllRegisterServer()
{
	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2(FALSE);
}

//

#include "../../FilterApp.h"

CFilterApp theApp;

#endif

// dshow: left, right, center, LFE, left surround, right surround
// ac3: LFE, left, center, right, left surround, right surround
// dts: center, left, right, left surround, right surround, LFE

// lets see how we can map these things to dshow (oh the joy!)

#pragma warning(disable : 4245)
static struct scmap_t {
	WORD nChannels;
	BYTE ch[8];
	DWORD dwChannelMask;
}

s_scmap_ac3[2*11] = {
	{2, {0, 1,-1,-1,-1,-1,-1,-1}, 0}, // A52_CHANNEL
	{1, {0,-1,-1,-1,-1,-1,-1,-1}, 0}, // A52_MONO
	{2, {0, 1,-1,-1,-1,-1,-1,-1}, 0}, // A52_STEREO
	{3, {0, 2, 1,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER}, // A52_3F
	{3, {0, 1, 2,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_CENTER},  // A52_2F1R
	{4, {0, 2, 1, 3,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_CENTER}, // A52_3F1R
	{4, {0, 1, 2, 3,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},     // A52_2F2R
	{5, {0, 2, 1, 3, 4,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT}, // A52_3F2R
	{1, {0,-1,-1,-1,-1,-1,-1,-1}, 0}, // A52_CHANNEL1
	{1, {0,-1,-1,-1,-1,-1,-1,-1}, 0}, // A52_CHANNEL2
	{2, {0, 1,-1,-1,-1,-1,-1,-1}, 0}, // A52_DOLBY

	{3, {1, 2, 0,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY}, // A52_CHANNEL|A52_LFE
	{2, {1, 0,-1,-1,-1,-1,-1,-1}, SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY}, // A52_MONO|A52_LFE
	{3, {1, 2, 0,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY}, // A52_STEREO|A52_LFE
	{4, {1, 3, 2, 0,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY}, // A52_3F|A52_LFE
	{4, {1, 2, 0, 3,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_CENTER},  // A52_2F1R|A52_LFE
	{5, {1, 3, 2, 0, 4,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_CENTER}, // A52_3F1R|A52_LFE
	{5, {1, 2, 0, 3, 4,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},     // A52_2F2R|A52_LFE
	{6, {1, 3, 2, 0, 4, 5,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT}, // A52_3F2R|A52_LFE
	{2, {1, 0,-1,-1,-1,-1,-1,-1}, SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY}, // A52_CHANNEL1|A52_LFE
	{2, {1, 0,-1,-1,-1,-1,-1,-1}, SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY}, // A52_CHANNEL2|A52_LFE
	{3, {1, 2, 0,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY}, // A52_DOLBY|A52_LFE
},

s_scmap_dts[2*10] = {
	{1, {0,-1,-1,-1,-1,-1,-1,-1}, 0}, // DTS_MONO
	{2, {0, 1,-1,-1,-1,-1,-1,-1}, 0}, // DTS_CHANNEL
	{2, {0, 1,-1,-1,-1,-1,-1,-1}, 0}, // DTS_STEREO
	{2, {0, 1,-1,-1,-1,-1,-1,-1}, 0}, // DTS_STEREO_SUMDIFF
	{2, {0, 1,-1,-1,-1,-1,-1,-1}, 0}, // DTS_STEREO_TOTAL
	{3, {1, 2, 0,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER}, // DTS_3F
	{3, {0, 1, 2,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_CENTER},  // DTS_2F1R
	{4, {1, 2, 0, 3,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_CENTER}, // DTS_3F1R
	{4, {0, 1, 2, 3,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},     // DTS_2F2R
	{5, {1, 2, 0, 3, 4,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT}, // DTS_3F2R

	{2, {0, 1,-1,-1,-1,-1,-1,-1}, SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY}, // DTS_MONO|DTS_LFE
	{3, {0, 1, 2,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY}, // DTS_CHANNEL|DTS_LFE
	{3, {0, 1, 2,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY}, // DTS_STEREO|DTS_LFE
	{3, {0, 1, 2,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY}, // DTS_STEREO_SUMDIFF|DTS_LFE
	{3, {0, 1, 2,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY}, // DTS_STEREO_TOTAL|DTS_LFE
	{4, {1, 2, 0, 3,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY}, // DTS_3F|DTS_LFE
	{4, {0, 1, 3, 2,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_CENTER},  // DTS_2F1R|DTS_LFE
	{5, {1, 2, 0, 4, 3,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_CENTER}, // DTS_3F1R|DTS_LFE
	{5, {0, 1, 4, 2, 3,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},     // DTS_2F2R|DTS_LFE
	{6, {1, 2, 0, 5, 3, 4,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT}, // DTS_3F2R|DTS_LFE
},

s_scmap_hdmv[] = {
	//   FL FR FC LFe BL BR FLC FRC
	{0, {-1,-1,-1,-1,-1,-1,-1,-1 }, 0}, // INVALID
	{1, { 0,-1,-1,-1,-1,-1,-1,-1 }, 0}, // Mono    M1, 0
	{0, {-1,-1,-1,-1,-1,-1,-1,-1 }, 0}, // INVALID
	{2, { 0, 1,-1,-1,-1,-1,-1,-1 }, 0}, // Stereo  FL, FR
	{4, { 0, 1, 2,-1,-1,-1,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER},															// 3/0      FL, FR, FC
	{4, { 0, 1, 2,-1,-1,-1,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY},															// 2/1      FL, FR, Surround
	{4, { 0, 1, 2, 3,-1,-1,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY},										// 3/1      FL, FR, FC, Surround
	{4, { 0, 1, 2, 3,-1,-1,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},											// 2/2      FL, FR, BL, BR
	{6, { 0, 1, 2, 3, 4,-1,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},						// 3/2      FL, FR, FC, BL, BR
	{6, { 0, 1, 2, 5, 3, 4,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},// 3/2+LFe  FL, FR, FC, BL, BR, LFe
	{8, { 0, 1, 2, 3, 6, 4, 5,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT},// 3/4  FL, FR, FC, BL, Bls, Brs, BR
	{8, { 0, 1, 2, 7, 4, 5, 3, 6 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT},// 3/4+LFe  FL, FR, FC, BL, Bls, Brs, BR, LFe
},

m_scmap_default[] = {
	//   FL FR FC LFe BL BR FLC FRC
	{1, { 0,-1,-1,-1,-1,-1,-1,-1 }, 0}, // Mono    M1, 0
	{2, { 0, 1,-1,-1,-1,-1,-1,-1 }, 0}, // Stereo  FL, FR
	{3, { 0, 1, 2,-1,-1,-1,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER},															// 3/0      FL, FR, FC
	{4, { 0, 1, 2, 3,-1,-1,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY},										// 3/1      FL, FR, FC, Surround
	{5, { 0, 1, 2, 3, 4,-1,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},						// 3/2      FL, FR, FC, BL, BR
	{6, { 0, 1, 2, 3, 4, 5,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},// 3/2+LFe  FL, FR, FC, BL, BR, LFe
	{7, { 0, 1, 2, 3, 4, 5, 6,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT|SPEAKER_BACK_CENTER},// 3/4  FL, FR, FC, BL, Bls, Brs, BR
	{8, { 0, 1, 2, 3, 6, 7, 4, 5 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},// 3/4+LFe  FL, FR, FC, BL, Bls, Brs, BR, LFe
},

m_scmap_truehd_51 =
{6, { 0, 1, 2, 3, 4, 5,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT},
m_scmap_truehd_71 =
{8, { 0, 1, 2, 3, 4, 5, 6, 7 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},

m_ffmpeg_ac3[] = {
	//  FL FR FC LFe BL BR FLC FRC
	{2, {0, 1,-1,-1,-1,-1,-1,-1}, 0}, // AC3_CHMODE_DUALMONO
	{1, {0,-1,-1,-1,-1,-1,-1,-1}, 0}, // AC3_CHMODE_MONO
	{2, {0, 1,-1,-1,-1,-1,-1,-1}, 0}, // AC3_CHMODE_STEREO
	{3, {0, 2, 1,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER}, // AC3_CHMODE_3F
	{3, {0, 1, 2,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_CENTER},  // AC3_CHMODE_2F1R
	{4, {0, 2, 1, 3,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_CENTER}, // AC3_CHMODE_3F1R
	{4, {0, 1, 2, 3,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},     // AC3_CHMODE_2F2R
	{5, {0, 2, 1, 3, 4,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},// AC3_CHMODE_3F2R

	// LFe
	{6, {0, 1, 2, 3, 4, 5,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},// AC3_CHMODE_3F2R
};
#pragma warning(default : 4245)

CMpaDecFilter::CMpaDecFilter(LPUNKNOWN lpunk, HRESULT* phr)
	: CTransformFilter(NAME("CMpaDecFilter"), lpunk, __uuidof(this))
	, m_iSampleFormat(SF_PCM16)
	, m_bResync(false)
{
	if (phr) {
		*phr = S_OK;
	}

	m_pInput = DNew CMpaDecInputPin(this, phr, L"In");
	if (!m_pInput) {
		*phr = E_OUTOFMEMORY;
	}
	if (FAILED(*phr)) {
		return;
	}

	m_pOutput = DNew CTransformOutputPin(NAME("CTransformOutputPin"), this, phr, L"Out");
	if (!m_pOutput) {
		*phr = E_OUTOFMEMORY;
	}
	if (FAILED(*phr))  {
		delete m_pInput, m_pInput = NULL;
		return;
	}

	m_iSpeakerConfig[ac3] = A52_STEREO;
	m_iSpeakerConfig[dts] = DTS_STEREO;
	m_fDynamicRangeControl[ac3] = false;
	m_fDynamicRangeControl[dts] = false;
	m_DolbyDigitalMode			= DD_Unknown;
#if defined(REGISTER_FILTER) || HAS_FFMPEG_AUDIO_DECODERS
	m_pAVCodec					= NULL;
	m_pAVCtx					= NULL;
	m_pParser					= NULL;
	m_pFrame					= NULL;
	m_pFFBuffer					= NULL;
	m_nFFBufferSize				= 0;
#endif

#if defined(REGISTER_FILTER) || INTERNAL_DECODER_FLAC
	memset (&m_flac, 0, sizeof(m_flac));
#endif

#if defined(REGISTER_FILTER) || INTERNAL_DECODER_AC3
	m_a52_state = NULL;
#endif

#if defined(REGISTER_FILTER) || INTERNAL_DECODER_DTS
	m_dts_state = NULL;
#endif

#ifdef REGISTER_FILTER
	CRegKey key;
	if (ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Gabest\\Filters\\MPEG Audio Decoder"), KEY_READ)) {
		DWORD dw;
		if (ERROR_SUCCESS == key.QueryDWORDValue(_T("SampleFormat"), dw)) {
			m_iSampleFormat = (MPCSampleFormat)dw;
		}
		if (ERROR_SUCCESS == key.QueryDWORDValue(_T("Ac3SpeakerConfig"), dw)) {
			m_iSpeakerConfig[ac3] = (int)dw;
		}
		if (ERROR_SUCCESS == key.QueryDWORDValue(_T("DtsSpeakerConfig"), dw)) {
			m_iSpeakerConfig[dts] = (int)dw;
		}
		if (ERROR_SUCCESS == key.QueryDWORDValue(_T("Ac3DynamicRangeControl"), dw)) {
			m_fDynamicRangeControl[ac3] = !!dw;
		}
		if (ERROR_SUCCESS == key.QueryDWORDValue(_T("DtsDynamicRangeControl"), dw)) {
			m_fDynamicRangeControl[dts] = !!dw;
		}
	}
#else
	m_iSampleFormat = (MPCSampleFormat)AfxGetApp()->GetProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("SampleFormat"), m_iSampleFormat);
	m_iSpeakerConfig[ac3] = AfxGetApp()->GetProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("Ac3SpeakerConfig"), m_iSpeakerConfig[ac3]);
	m_iSpeakerConfig[dts] = AfxGetApp()->GetProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("DtsSpeakerConfig"), m_iSpeakerConfig[dts]);
	m_fDynamicRangeControl[ac3] = !!AfxGetApp()->GetProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("Ac3DynamicRangeControl"), m_fDynamicRangeControl[ac3]);
	m_fDynamicRangeControl[dts] = !!AfxGetApp()->GetProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("DtsDynamicRangeControl"), m_fDynamicRangeControl[dts]);
#endif
}

CMpaDecFilter::~CMpaDecFilter()
{
	StopStreaming();
}

STDMETHODIMP CMpaDecFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	return
		QI(IMpaDecFilter)
		QI(ISpecifyPropertyPages)
		QI(ISpecifyPropertyPages2)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT CMpaDecFilter::EndOfStream()
{
	CAutoLock cAutoLock(&m_csReceive);
	return __super::EndOfStream();
}

HRESULT CMpaDecFilter::BeginFlush()
{
	return __super::BeginFlush();
}

HRESULT CMpaDecFilter::EndFlush()
{
	CAutoLock cAutoLock(&m_csReceive);
	m_buff.RemoveAll();
	return __super::EndFlush();
}

HRESULT CMpaDecFilter::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	CAutoLock cAutoLock(&m_csReceive);
	m_buff.RemoveAll();
	m_ps2_state.sync = false;
	m_DolbyDigitalMode = DD_Unknown;
#if defined(REGISTER_FILTER) || HAS_FFMPEG_AUDIO_DECODERS
	if (m_pAVCtx) {
		avcodec_flush_buffers (m_pAVCtx);
	}
#endif
#if defined(REGISTER_FILTER) || INTERNAL_DECODER_FLAC
	if (m_flac.pDecoder) {
		FLAC__stream_decoder_flush((FLAC__StreamDecoder*) m_flac.pDecoder);
	}
#endif

	m_bResync = true;

	return __super::NewSegment(tStart, tStop, dRate);
}

#if defined(REGISTER_FILTER) || HAS_FFMPEG_AUDIO_DECODERS
enum CodecID CMpaDecFilter::FindCodec(const GUID subtype)
{
	for (int i=0; i<_countof(ffAudioCodecs); i++)
		if (subtype == *ffAudioCodecs[i].clsMinorType) {
			return ffAudioCodecs[i].nFFCodec;
		}

	return CODEC_ID_NONE;
}
#endif


HRESULT CMpaDecFilter::Receive(IMediaSample* pIn)
{
	CAutoLock cAutoLock(&m_csReceive);

	HRESULT hr;

	AM_SAMPLE2_PROPERTIES* const pProps = m_pInput->SampleProps();
	if (pProps->dwStreamId != AM_STREAM_MEDIA) {
		return m_pOutput->Deliver(pIn);
	}

	AM_MEDIA_TYPE* pmt;
	if (SUCCEEDED(pIn->GetMediaType(&pmt)) && pmt) {
		CMediaType mt(*pmt);
		m_pInput->SetMediaType(&mt);
		DeleteMediaType(pmt);
		pmt = NULL;
		m_DolbyDigitalMode = DD_Unknown;
	}

	BYTE* pDataIn = NULL;
	if (FAILED(hr = pIn->GetPointer(&pDataIn))) {
		return hr;
	}

	long len = pIn->GetActualDataLength();

	(static_cast<CDeCSSInputPin*>(m_pInput))->StripPacket(pDataIn, len);

	REFERENCE_TIME rtStart = _I64_MIN, rtStop = _I64_MIN;
	hr = pIn->GetTime(&rtStart, &rtStop);

#if 0
	if (SUCCEEDED(hr)) {
		TRACE(_T("CMpaDecFilter::Receive(): rtStart = %10I64d, rtStop = %10I64d\n"), rtStart, rtStop);
	} else {
		TRACE(_T("CMpaDecFilter::Receive(): frame without timestamp\n"));
	}
#endif

	if (pIn->IsDiscontinuity() == S_OK) {
		m_fDiscontinuity = true;
		m_buff.RemoveAll();
		m_rtStart = rtStart;
		m_bResync = true;
		if (FAILED(hr)) {
			TRACE(_T("CMpaDecFilter::Receive() : Discontinuity without timestamp\n"));
			return S_OK;
		}
	}

	const GUID& subtype = m_pInput->CurrentMediaType().subtype;

	if ((subtype == MEDIASUBTYPE_COOK && (S_OK == pIn->IsSyncPoint())) || ((_abs64((m_rtStart - rtStart)) > MAX_JITTER) && ((subtype != MEDIASUBTYPE_COOK) && (subtype != MEDIASUBTYPE_ATRC) && (subtype != MEDIASUBTYPE_SIPR)))) {
		m_bResync = true;
	}

	if (SUCCEEDED(hr) && m_bResync) {
		m_buff.RemoveAll();
		m_rtStart = rtStart;
		m_bResync = false;
	}

	int bufflen = m_buff.GetCount();
	m_buff.SetCount(bufflen + len, 4096);
	memcpy(m_buff.GetData() + bufflen, pDataIn, len);
	len += bufflen;

#if defined(REGISTER_FILTER) || HAS_FFMPEG_AUDIO_DECODERS
	enum CodecID nCodecId = FindCodec(subtype);
	if (nCodecId != CODEC_ID_NONE) {
		return ProcessFFmpeg(nCodecId);
	}
#endif
	if (0) {} // needed if decoders are disabled below
#if defined(REGISTER_FILTER) || INTERNAL_DECODER_LPCM
	else if (subtype == MEDIASUBTYPE_DVD_LPCM_AUDIO) {
		hr = ProcessLPCM();
	} else if (subtype == MEDIASUBTYPE_HDMV_LPCM_AUDIO) {
		hr = ProcessHdmvLPCM(!!pIn->IsSyncPoint());
	}
#endif
#if defined(REGISTER_FILTER) || INTERNAL_DECODER_AC3
	else if (subtype == MEDIASUBTYPE_DOLBY_AC3 ||
			 subtype == MEDIASUBTYPE_WAVE_DOLBY_AC3 ||
			 subtype == MEDIASUBTYPE_DNET) {
		hr = ProcessAC3();
	}
#endif
#if defined(REGISTER_FILTER) || INTERNAL_DECODER_DTS
	else if (subtype == MEDIASUBTYPE_DTS || subtype == MEDIASUBTYPE_WAVE_DTS) {
		hr = ProcessDTS();
	}
#endif
#if defined(REGISTER_FILTER) || INTERNAL_DECODER_PS2AUDIO
	else if (subtype == MEDIASUBTYPE_PS2_PCM) {
		hr = ProcessPS2PCM();
	} else if (subtype == MEDIASUBTYPE_PS2_ADPCM) {
		hr = ProcessPS2ADPCM();
	}
#endif
#if defined(REGISTER_FILTER) || INTERNAL_DECODER_FLAC
	else if (subtype == MEDIASUBTYPE_FLAC_FRAMED) {
		hr = ProcessFlac();
	}
#endif
#if defined(REGISTER_FILTER) || INTERNAL_DECODER_PCM
	else if (subtype == MEDIASUBTYPE_PCM_NONE ||
			 subtype == MEDIASUBTYPE_PCM_RAW) {
		hr = ProcessPCMraw();
	} else if (subtype == MEDIASUBTYPE_PCM_TWOS ||
			   subtype == MEDIASUBTYPE_PCM_IN24 ||
			   subtype == MEDIASUBTYPE_PCM_IN32) {
		hr = ProcessPCMintBE();
	} else if (subtype == MEDIASUBTYPE_PCM_SOWT) {
		hr = ProcessPCMintLE();
	} else if (subtype == MEDIASUBTYPE_PCM_FL32 ||
			   subtype == MEDIASUBTYPE_PCM_FL64) {
		hr = ProcessPCMfloatBE();
	} else if (subtype == MEDIASUBTYPE_IEEE_FLOAT) {
		hr = ProcessPCMfloatLE();
	}
#endif

	return hr;
}

#if defined(REGISTER_FILTER) || INTERNAL_DECODER_LPCM
HRESULT CMpaDecFilter::ProcessLPCM()
{
	WAVEFORMATEX* wfein = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();

	if (wfein->nChannels < 1 || wfein->nChannels > 8) {
		return ERROR_NOT_SUPPORTED;
	}

	scmap_t* remap     = &m_scmap_default [wfein->nChannels-1];
	int      nChannels = wfein->nChannels;

	BYTE*    pDataIn   = m_buff.GetData();
	int BytesPerDoubleSample        = (wfein->wBitsPerSample * 2)/8;
	int BytesPerDoubleChannelSample = BytesPerDoubleSample * nChannels;
	int      nInBytes  = m_buff.GetCount();
	int      len       = (nInBytes / BytesPerDoubleChannelSample) * (BytesPerDoubleChannelSample); // We always code 2 samples at a time

	CAtlArray<float> pBuff;
	pBuff.SetCount((len/BytesPerDoubleSample) * 2);

	float* pDataOut = pBuff.GetData();

	switch (wfein->wBitsPerSample) {
		case 16 : {
			long nSamples = len/(BytesPerDoubleChannelSample);
			int16 Temp[2][8];
			for (int i=0; i<nSamples; i++) {
				for (int j = 0; j < nChannels; j++) {
					uint16 All = *((uint16 *)pDataIn);
					pDataIn += 2;
					int16 Part1 = (All & 0xFF) << 8 | (All & 0xFF00) >> 8;
					Temp[0][j] = Part1;
				}
				for (int j = 0; j < nChannels; j++) {
					uint16 All = *((uint16 *)pDataIn);
					pDataIn += 2;
					int16 Part1 = (All & 0xFF) << 8 | (All & 0xFF00) >> 8;
					Temp[1][j] = Part1;
				}

				for (int j = 0; j < nChannels; j++) {
					BYTE nRemap = remap->ch[j];
					*pDataOut = float(Temp[0][nRemap]) / INT16_PEAK;
					++pDataOut;
				}
				for (int j = 0; j < nChannels; j++) {
					BYTE nRemap = remap->ch[j];
					*pDataOut = float(Temp[1][nRemap]) / INT16_PEAK;
					++pDataOut;
				}
			}
		}
		break;

		case 24 : {
			long nSamples = len/(BytesPerDoubleChannelSample);
			int32 Temp[2][8];
			for (int i=0; i<nSamples; i++) {
				// Start by upper 16 bits
				for (int j = 0; j < nChannels; j++) {
					uint32 All = *((uint16 *)pDataIn);
					pDataIn += 2;
					uint32 Part1 = (All & 0xFF) << 24 | (All & 0xFF00) << 8;
					Temp[0][j] = Part1;
				}
				for (int j = 0; j < nChannels; j++) {
					uint32 All = *((uint16 *)pDataIn);
					pDataIn += 2;
					uint32 Part1 = (All & 0xFF) << 24 | (All & 0xFF00) << 8;
					Temp[1][j] = Part1;
				}

				// Continue with lower bits
				for (int j = 0; j < nChannels; j++) {
					uint32 All = *((uint8 *)pDataIn);
					pDataIn += 1;
					Temp[0][j] = int32(Temp[0][j] | (All << 8)) >> 8;
				}
				for (int j = 0; j < nChannels; j++) {
					uint32 All = *((uint8 *)pDataIn);
					pDataIn += 1;
					Temp[1][j] = int32(Temp[1][j] | (All << 8)) >> 8;
				}

				// Convert into float
				for (int j = 0; j < nChannels; j++) {
					BYTE nRemap = remap->ch[j];
					*pDataOut = float(Temp[0][nRemap]) / INT24_PEAK;
					++pDataOut;
				}
				for (int j = 0; j < nChannels; j++) {
					BYTE nRemap = remap->ch[j];
					*pDataOut = float(Temp[1][nRemap]) / INT24_PEAK;
					++pDataOut;
				}
			}
		}
		break;
		case 20 : {
			long nSamples = len/(BytesPerDoubleChannelSample);
			int32 Temp[2][8];
			for (int i=0; i<nSamples; i++) {
				// Start by upper 16 bits
				for (int j = 0; j < nChannels; j++) {
					uint32 All = *((uint16 *)pDataIn);
					pDataIn += 2;
					uint32 Part1 = (All & 0xFF) << 24 | (All & 0xFF00) << 8;
					Temp[0][j] = Part1;
				}
				for (int j = 0; j < nChannels; j++) {
					uint32 All = *((uint16 *)pDataIn);
					pDataIn += 2;
					uint32 Part1 = (All & 0xFF) << 24 | (All & 0xFF00) << 8;
					Temp[1][j] = Part1;
				}

				// Continue with lower bits
				for (int j = 0; j < nChannels; j++) {
					uint32 All = *((uint8 *)pDataIn);
					pDataIn += 1;
					Temp[0][j] = int32(Temp[0][j] | ((All&0xf0) << 8)) >> 8;
					Temp[1][j] = int32(Temp[1][j] | ((All&0x0f) << 12)) >> 8;
				}

				// Convert into float
				for (int j = 0; j < nChannels; j++) {
					BYTE nRemap = remap->ch[j];
					*pDataOut = float(Temp[0][nRemap]) / INT24_PEAK;
					++pDataOut;
				}
				for (int j = 0; j < nChannels; j++) {
					BYTE nRemap = remap->ch[j];
					*pDataOut = float(Temp[1][nRemap]) / INT24_PEAK;
					++pDataOut;
				}
			}
		}
		break;
	}

	memmove(m_buff.GetData(), pDataIn, m_buff.GetCount() - len );
	m_buff.SetCount(m_buff.GetCount() - len);

	return Deliver(pBuff, wfein->nSamplesPerSec, wfein->nChannels, remap->dwChannelMask);
}


HRESULT CMpaDecFilter::ProcessHdmvLPCM(bool bAlignOldBuffer) // Blu ray LPCM
{
	WAVEFORMATEX_HDMV_LPCM* wfein = (WAVEFORMATEX_HDMV_LPCM*)m_pInput->CurrentMediaType().Format();

	scmap_t* remap     = &s_scmap_hdmv [wfein->channel_conf];
	int nChannels      = wfein->nChannels;
	int xChannels      = nChannels + (nChannels % 2);
	int BytesPerSample = (wfein->wBitsPerSample + 7) / 8;
	int BytesPerFrame  = BytesPerSample * xChannels;

	BYTE* pDataIn      = m_buff.GetData();
	int len            = m_buff.GetCount() - (m_buff.GetCount() % BytesPerFrame);
	if (bAlignOldBuffer) {
		m_buff.SetCount(len);
	}
	int nFrames = len/xChannels/BytesPerSample;

	CAtlArray<float> pBuff;
	pBuff.SetCount(nFrames*nChannels); //nSamples
	float*	pDataOut = pBuff.GetData();

	switch (wfein->wBitsPerSample) {
		case 16 :
			for (int i=0; i<nFrames; i++) {
				for (int j = 0; j < nChannels; j++) {
					BYTE nRemap = remap->ch[j];
					*pDataOut = (float)(int16_t)(pDataIn[nRemap*2]<<8 | pDataIn[nRemap*2+1]) / INT16_PEAK;
					pDataOut++;
				}
				pDataIn += xChannels*2;
			}
			break;
		case 24 :
		case 20 :
			for (int i=0; i<nFrames; i++) {
				for (int j = 0; j < nChannels; j++) {
					BYTE nRemap = remap->ch[j];
					*pDataOut = (float)(int32_t)(pDataIn[nRemap*3]<<24 | pDataIn[nRemap*3+1]<<16 | pDataIn[nRemap*3+2]<<8) / INT32_PEAK;
					pDataOut++;
				}
				pDataIn += xChannels*3;
			}
			break;
	}
	memmove(m_buff.GetData(), pDataIn, m_buff.GetCount() - len );
	m_buff.SetCount(m_buff.GetCount() - len);

	return Deliver(pBuff, wfein->nSamplesPerSec, wfein->nChannels, remap->dwChannelMask);
}
#endif /* INTERNAL_DECODER_LPCM */

#if defined(REGISTER_FILTER) || INTERNAL_DECODER_AC3
HRESULT CMpaDecFilter::ProcessA52(BYTE* p, int buffsize, int& size, bool& fEnoughData)
{
	int flags, sample_rate, bit_rate;

	if ((size = a52_syncinfo(p, &flags, &sample_rate, &bit_rate)) > 0) {
		// TRACE(_T("ac3: size=%d, flags=%08x, sample_rate=%d, bit_rate=%d\n"), size, flags, sample_rate, bit_rate);

		fEnoughData = size <= buffsize;

		if (fEnoughData) {
			int iSpeakerConfig = GetSpeakerConfig(ac3);

			if (iSpeakerConfig < 0) {
				HRESULT hr;
				if (S_OK != (hr = DeliverBitstream(p, size, sample_rate, 1536, 0x0001))) {
					return hr;
				}
			} else {
				flags = iSpeakerConfig&(A52_CHANNEL_MASK|A52_LFE);
				flags |= A52_ADJUST_LEVEL;

				sample_t level = 1, gain = 1, bias = 0;
				level *= gain;

				if (a52_frame(m_a52_state, p, &flags, &level, bias) == 0) {
					if (GetDynamicRangeControl(ac3)) {
						a52_dynrng(m_a52_state, NULL, NULL);
					}

					int scmapidx = min(flags&A52_CHANNEL_MASK, _countof(s_scmap_ac3)/2);
					scmap_t& scmap = s_scmap_ac3[scmapidx + ((flags&A52_LFE)?(_countof(s_scmap_ac3)/2):0)];

					CAtlArray<float> pBuff;
					pBuff.SetCount(6*256*scmap.nChannels);
					float* p = pBuff.GetData();

					int i = 0;

					for (; i < 6 && a52_block(m_a52_state) == 0; i++) {
						sample_t* samples = a52_samples(m_a52_state);

						for (int j = 0; j < 256; j++, samples++) {
							for (int ch = 0; ch < scmap.nChannels; ch++) {
								ASSERT(scmap.ch[ch] != -1);
								*p++ = (float)(*(samples + 256*scmap.ch[ch]) / level);
							}
						}
					}

					if (i == 6) {
						HRESULT hr;
						if (S_OK != (hr = Deliver(pBuff, sample_rate, scmap.nChannels, scmap.dwChannelMask))) {
							return hr;
						}
					}
				}
			}
		}
	}

	return S_OK;
}

HRESULT CMpaDecFilter::ProcessAC3()
{
	HRESULT hr;
	BYTE* p = m_buff.GetData();
	BYTE* base = p;
	BYTE* end = p + m_buff.GetCount();

	while (p < end && end - p >= AC3_HEADER_SIZE) {
		int		size = 0;
		bool	fEnoughData = true;

		if (m_DolbyDigitalMode != DD_TRUEHD && m_DolbyDigitalMode != DD_MLP && (*((__int16*)p) == 0x770b)) { /* AC3-EAC3 syncword */
			BYTE bsid = p[5] >> 3;
			if (bsid <= 10) {
				if (FAILED (hr = ProcessA52 (p, end-p, size, fEnoughData))) {
					return hr;
				}
				m_DolbyDigitalMode = DD_AC3;
			} else if (bsid <= 16) {
				DeliverFFmpeg(CODEC_ID_EAC3, p, end-p, size);
				if (size > 0) {
					m_DolbyDigitalMode = DD_EAC3;
				}
			} else {
				p++;
				continue;
			}
		} else if ( (*((__int32*)(p+4)) == 0xba6f72f8) || // TrueHD major sync frame
					m_DolbyDigitalMode == DD_TRUEHD ) {
			int		nLenght = (((p[0]<<8) + p[1]) & 0x0FFF)*2;

			m_DolbyDigitalMode = DD_TRUEHD;

			if (nLenght >= 4) {
				DeliverFFmpeg(CODEC_ID_TRUEHD, p, end-p, size);
				if (size<0) {
					size = end-p;
				}
			}
		} else if ( (*((__int32*)(p+4)) == 0xbb6f72f8) || // MLP major sync frame
					m_DolbyDigitalMode == DD_MLP ) {
			int		nLenght = (((p[0]<<8) + p[1]) & 0x0FFF)*2;

			m_DolbyDigitalMode = DD_MLP;

			if (nLenght >= 4) {
				DeliverFFmpeg(CODEC_ID_MLP, p, end-p, size);
				if (size<0) {
					size = end-p;
				}
			}
		} else {
			p++;
			continue;
		}

		// Update buffer position
		if (fEnoughData) {
			ASSERT (size <= end-p);
			if (size <= 0 || size > end-p) {
				break;
			}
			p += size;
		}
		memmove(base, p, end - p);
		end = base + (end - p);
		p = base;
		if (!fEnoughData) {
			break;
		}
	}

	m_buff.SetCount(end - p);

	return S_OK;
}
#endif /* INTERNAL_DECODER_AC3 */

#if defined(REGISTER_FILTER) || HAS_FFMPEG_AUDIO_DECODERS
HRESULT CMpaDecFilter::ProcessFFmpeg(enum CodecID nCodecId)
{
	HRESULT hr;
	BYTE* p = m_buff.GetData();
	BYTE* base = p;
	BYTE* end = p + m_buff.GetCount();

	int size = 0;
	hr = DeliverFFmpeg(nCodecId, p, end-p, size);
	if (FAILED(hr)) {
		if (!(nCodecId == CODEC_ID_AAC || nCodecId == CODEC_ID_AAC_LATM)) {
			m_buff.RemoveAll();
			m_bResync = true;
		}
		return S_OK;
	}

	if (size <= 0) {
		return hr;
	}

	m_DolbyDigitalMode = (nCodecId == CODEC_ID_TRUEHD) ? DD_TRUEHD : (nCodecId == CODEC_ID_EAC3) ? DD_EAC3 : DD_Unknown;

	p += size;
	memmove(base, p, end - p);
	end = base + (end - p);
	p = base;
	m_buff.SetCount(end - p);

	return hr;
}
#endif /* HAS_FFMPEG_AUDIO_DECODERS */

#if defined(REGISTER_FILTER) || INTERNAL_DECODER_DTS
HRESULT CMpaDecFilter::ProcessDTS()
{
	BYTE* p = m_buff.GetData();
	BYTE* base = p;
	BYTE* end = p + m_buff.GetCount();

	while (end - p >= 14) {
		int size = 0, flags, sample_rate, bit_rate, frame_length;

		if ((size = dts_syncinfo(m_dts_state, p, &flags, &sample_rate, &bit_rate, &frame_length)) > 0) {
			// TRACE(_T("dts: size=%d, flags=%08x, sample_rate=%d, bit_rate=%d, frame_length=%d\n"), size, flags, sample_rate, bit_rate, frame_length);
			bit_rate = int (size * 8i64 * sample_rate / frame_length); // calculate actual bitrate

			bool fEnoughData = p + size <= end;

			if (fEnoughData) {
				int iSpeakerConfig = GetSpeakerConfig(dts);

				if (iSpeakerConfig < 0) {
					HRESULT hr;
					if (S_OK != (hr = DeliverBitstream(p, size, sample_rate, frame_length, 0x000b))) {
						return hr;
					}
				} else {
					flags = iSpeakerConfig&(DTS_CHANNEL_MASK|DTS_LFE);
					flags |= DTS_ADJUST_LEVEL;

					sample_t level = 1, gain = 1, bias = 0;
					level *= gain;

					if (dts_frame(m_dts_state, p, &flags, &level, bias) == 0) {
						if (GetDynamicRangeControl(dts)) {
							dts_dynrng(m_dts_state, NULL, NULL);
						}

						int scmapidx = min(flags&DTS_CHANNEL_MASK, _countof(s_scmap_dts)/2);
						scmap_t& scmap = s_scmap_dts[scmapidx + ((flags&DTS_LFE)?(_countof(s_scmap_dts)/2):0)];

						int blocks = dts_blocks_num(m_dts_state);

						CAtlArray<float> pBuff;
						pBuff.SetCount(blocks*256*scmap.nChannels);
						float* p = pBuff.GetData();

						int i = 0;

						for (; i < blocks && dts_block(m_dts_state) == 0; i++) {
							sample_t* samples = dts_samples(m_dts_state);

							for (int j = 0; j < 256; j++, samples++) {
								for (int ch = 0; ch < scmap.nChannels; ch++) {
									ASSERT(scmap.ch[ch] != -1);
									*p++ = (float)(*(samples + 256*scmap.ch[ch]) / level);
								}
							}
						}

						if (i == blocks) {
							HRESULT hr;
							if (S_OK != (hr = Deliver(pBuff, sample_rate, scmap.nChannels, scmap.dwChannelMask))) {
								return hr;
							}
						}
					}
				}

				p += size;
			}

			memmove(base, p, end - p);
			end = base + (end - p);
			p = base;

			if (!fEnoughData) {
				break;
			}
		} else {
			p++;
		}
	}

	m_buff.SetCount(end - p);

	return S_OK;
}
#endif /* INTERNAL_DECODER_DTS */

#if defined(REGISTER_FILTER) || INTERNAL_DECODER_PCM
HRESULT CMpaDecFilter::ProcessPCMraw() //'raw '
{
	WAVEFORMATEX* wfe = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();
	unsigned int nSamples = m_buff.GetCount() * 8 / wfe->wBitsPerSample;

	CAtlArray<float> pBuff;
	pBuff.SetCount(nSamples);
	float* f = pBuff.GetData();

	switch (wfe->wBitsPerSample) {
		case 8: { //unsigned 8-bit
			uint8_t* b = (uint8_t*)m_buff.GetData();
			for (unsigned int i = 0; i < nSamples; i++) {
				f[i] = (float)(int8_t)(b[i] + 128) / INT8_PEAK;
			}
		}
		break;
		case 16: { //signed big-endian 16 bit
			uint16_t* d = (uint16_t*)m_buff.GetData();//signed take as an unsigned to shift operations.
			for (unsigned int i = 0; i < nSamples; i++) {
				f[i] = (float)(int16_t)(d[i] << 8 | d[i] >> 8) / INT16_PEAK;
			}
		}
		break;
	}

	HRESULT hr;
	if (S_OK != (hr = Deliver(pBuff, wfe->nSamplesPerSec, wfe->nChannels))) {
		return hr;
	}

	m_buff.RemoveAll();
	return S_OK;
}

HRESULT CMpaDecFilter::ProcessPCMintBE() //'twos', big-endian 'in24' and 'in32'
{
	WAVEFORMATEX* wfe = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();
	unsigned int nSamples = m_buff.GetCount() * 8 / wfe->wBitsPerSample;

	CAtlArray<float> pBuff;
	pBuff.SetCount(nSamples);
	float* f = pBuff.GetData();

	switch (wfe->wBitsPerSample) {
		case 8: { //signed 8-bit
			int8_t* b = (int8_t*)m_buff.GetData();
			for (unsigned int i = 0; i < nSamples; i++) {
				f[i] = (float)b[i] / INT8_PEAK;
			}
		}
		break;
		case 16: { //signed big-endian 16-bit
			uint16_t* d = (uint16_t*)m_buff.GetData();//signed take as an unsigned to shift operations.
			for (unsigned int i = 0; i < nSamples; i++) {
				f[i] = (float)(int16_t)(d[i] << 8 | d[i] >> 8) / INT16_PEAK;
			}
		}
		break;
		case 24: { //signed big-endian 24-bit
			uint8_t* b = (uint8_t*)m_buff.GetData();
			for (unsigned int i = 0; i < nSamples; i++) {
				f[i] = (float)(int32_t)((uint32_t)b[3*i]   << 24 |
										(uint32_t)b[3*i+1] << 16 |
										(uint32_t)b[3*i+2] << 8) / INT32_PEAK;
			}
		}
		break;
		case 32: { //signed big-endian 32-bit
			uint32_t* q = (uint32_t*)m_buff.GetData();//signed take as an unsigned to shift operations.
			for (unsigned int i = 0; i < nSamples; i++) {
				f[i] = (float)(int32_t)(q[i] >> 24 |
										(q[i] & 0x00ff0000) >> 8 |
										(q[i] & 0x0000ff00) << 8 |
										q[i] << 24) / INT32_PEAK;
			}
		}
		break;
	}

	HRESULT hr;
	if (S_OK != (hr = Deliver(pBuff, wfe->nSamplesPerSec, wfe->nChannels))) {
		return hr;
	}

	m_buff.RemoveAll();
	return S_OK;
}

HRESULT CMpaDecFilter::ProcessPCMintLE() //'sowt', little-endian 'in24' and 'in32'
{
	WAVEFORMATEX* wfe = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();
	unsigned int nSamples = m_buff.GetCount() * 8 / wfe->wBitsPerSample;

	CAtlArray<float> pBuff;
	pBuff.SetCount(nSamples);
	float* f = pBuff.GetData();

	switch (wfe->wBitsPerSample) {
		case 8: { //signed 8-bit
			int8_t* b = (int8_t*)m_buff.GetData();
			for (unsigned int i = 0; i < nSamples; i++) {
				f[i] = (float)b[i] / INT8_PEAK;
			}
		}
		break;
		case 16: { //signed little-endian 16-bit
			int16_t* d = (int16_t*)m_buff.GetData();
			for (unsigned int i = 0; i < nSamples; i++) {
				f[i] = (float)d[i] / INT16_PEAK;
			}
		}
		break;
		case 24: { //signed little-endian 32-bit
			uint8_t* b = (uint8_t*)m_buff.GetData();
			for (unsigned int i = 0; i < nSamples; i++) {
				f[i] = (float)(int32_t)((uint32_t)b[3*i]   << 8  |
										(uint32_t)b[3*i+1] << 16 |
										(uint32_t)b[3*i+2] << 24) / INT32_PEAK;
			}
		}
		break;
		case 32: { //signed little-endian 32-bit
			int32_t* q = (int32_t*)m_buff.GetData();
			for (unsigned int i = 0; i < nSamples; i++) {
				f[i] = (float)q[i] / INT32_PEAK;
			}
		}
		break;
	}

	HRESULT hr;
	if (S_OK != (hr = Deliver(pBuff, wfe->nSamplesPerSec, wfe->nChannels))) {
		return hr;
	}

	m_buff.RemoveAll();
	return S_OK;
}

HRESULT CMpaDecFilter::ProcessPCMfloatBE() //big-endian 'fl32' and 'fl64'
{
	WAVEFORMATEX* wfe = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();
	unsigned int nSamples = m_buff.GetCount() * 8 / wfe->wBitsPerSample;

	CAtlArray<float> pBuff;
	pBuff.SetCount(nSamples);
	float* f = pBuff.GetData();

	switch (wfe->wBitsPerSample) {
		case 32: {
			uint32_t* q  = (uint32_t*)m_buff.GetData();
			uint32_t* vf = (uint32_t*)f;
			for (unsigned int i = 0; i < nSamples; i++) {
				vf[i] = q[i] >> 24 |
						(q[i] & 0x00ff0000) >> 8 |
						(q[i] & 0x0000ff00) << 8 |
						q[i] << 24;
			}
		}
		break;
		case 64: {
			uint64_t* q = (uint64_t*)m_buff.GetData();
			uint64_t x;
			for (unsigned int i = 0; i < nSamples; i++) {
				x = q[i] >>56 |
					(q[i] & 0x00FF000000000000) >> 40 |
					(q[i] & 0x0000FF0000000000) >> 24 |
					(q[i] & 0x000000FF00000000) >>  8 |
					(q[i] & 0x00000000FF000000) <<  8 |
					(q[i] & 0x0000000000FF0000) << 24 |
					(q[i] & 0x000000000000FF00) << 40 |
					q[i] << 56;
				f[i] = (float)*(double*)&x;
			}
		}
		break;
	}

	HRESULT hr;
	if (S_OK != (hr = Deliver(pBuff, wfe->nSamplesPerSec, wfe->nChannels))) {
		return hr;
	}

	m_buff.RemoveAll();
	return S_OK;
}

HRESULT CMpaDecFilter::ProcessPCMfloatLE() //little-endian 'fl32' and 'fl64'
{
	WAVEFORMATEX* wfe = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();
	unsigned int nSamples = m_buff.GetCount() * 8 / wfe->wBitsPerSample;

	CAtlArray<float> pBuff;
	pBuff.SetCount(nSamples);
	float* f = pBuff.GetData();

	switch (wfe->wBitsPerSample) {
		case 32: {
			float* q = (float*)m_buff.GetData();
			memcpy(f, q, nSamples * 4);
		}
		break;
		case 64: {
			double* q = (double*)m_buff.GetData();
			for (unsigned int i = 0; i < nSamples; i++) {
				f[i] = (float)q[i];
			}
		}
		break;
	}

	HRESULT hr;
	if (S_OK != (hr = Deliver(pBuff, wfe->nSamplesPerSec, wfe->nChannels))) {
		return hr;
	}

	m_buff.RemoveAll();
	return S_OK;
}
#endif /* INTERNAL_DECODER_PCM */

#if defined(REGISTER_FILTER) || INTERNAL_DECODER_PS2AUDIO
HRESULT CMpaDecFilter::ProcessPS2PCM()
{
	BYTE* p = m_buff.GetData();
	BYTE* base = p;
	BYTE* end = p + m_buff.GetCount();

	WAVEFORMATEXPS2* wfe = (WAVEFORMATEXPS2*)m_pInput->CurrentMediaType().Format();
	int size = wfe->dwInterleave*wfe->nChannels;
	int samples = wfe->dwInterleave/(wfe->wBitsPerSample>>3);
	int channels = wfe->nChannels;

	CAtlArray<float> pBuff;
	pBuff.SetCount(samples*channels);
	float* f = pBuff.GetData();

	while (end - p >= size) {
		DWORD* dw = (DWORD*)p;

		if (dw[0] == 'dhSS') {
			p += dw[1] + 8;
		} else if (dw[0] == 'dbSS') {
			p += 8;
			m_ps2_state.sync = true;
		} else {
			if (m_ps2_state.sync) {
				short* s = (short*)p;

				for (int i = 0; i < samples; i++)
					for (int j = 0; j < channels; j++) {
						f[i*channels+j] = (float)s[j*samples+i] / INT16_PEAK;
					}
			} else {
				for (int i = 0, j = samples*channels; i < j; i++) {
					f[i] = 0;
				}
			}

			HRESULT hr;
			if (S_OK != (hr = Deliver(pBuff, wfe->nSamplesPerSec, wfe->nChannels))) {
				return hr;
			}

			p += size;

			memmove(base, p, end - p);
			end = base + (end - p);
			p = base;
		}
	}

	m_buff.SetCount(end - p);

	return S_OK;
}

static void decodeps2adpcm(ps2_state_t& s, int channel, BYTE* pin, double* pout)
{
	int tbl_index = pin[0]>>4;
	int shift = pin[0]&0xf;
	int unk = pin[1]; // ?
	UNREFERENCED_PARAMETER(unk);

	if (tbl_index >= 10) {
		ASSERT(0);
		return;
	}
	// if(unk == 7) {ASSERT(0); return;} // ???

	static double s_tbl[] = {
		0.0, 0.0, 0.9375, 0.0, 1.796875, -0.8125, 1.53125, -0.859375, 1.90625, -0.9375,
		0.0, 0.0, -0.9375, 0.0, -1.796875, 0.8125, -1.53125, 0.859375 -1.90625, 0.9375
	};

	double* tbl = &s_tbl[tbl_index*2];
	double& a = s.a[channel];
	double& b = s.b[channel];

	for (int i = 0; i < 28; i++) {
		short input = (short)(((pin[2+i/2] >> ((i&1) << 2)) & 0xf) << 12) >> shift;
		double output = a * tbl[1] + b * tbl[0] + input;

		a = b;
		b = output;

		*pout++ = output / INT16_PEAK;
	}
}

HRESULT CMpaDecFilter::ProcessPS2ADPCM()
{
	BYTE* p = m_buff.GetData();
	BYTE* base = p;
	BYTE* end = p + m_buff.GetCount();

	WAVEFORMATEXPS2* wfe = (WAVEFORMATEXPS2*)m_pInput->CurrentMediaType().Format();
	int size = wfe->dwInterleave*wfe->nChannels;
	int samples = wfe->dwInterleave * 14 / 16 * 2;
	int channels = wfe->nChannels;

	CAtlArray<float> pBuff;
	pBuff.SetCount(samples*channels);
	float* f = pBuff.GetData();

	while (end - p >= size) {
		DWORD* dw = (DWORD*)p;

		if (dw[0] == 'dhSS') {
			p += dw[1] + 8;
		} else if (dw[0] == 'dbSS') {
			p += 8;
			m_ps2_state.sync = true;
		} else {
			if (m_ps2_state.sync) {
				double* tmp = DNew double[samples*channels];

				for (int channel = 0, j = 0, k = 0; channel < channels; channel++, j += wfe->dwInterleave)
					for (DWORD i = 0; i < wfe->dwInterleave; i += 16, k += 28) {
						decodeps2adpcm(m_ps2_state, channel, p + i + j, tmp + k);
					}

				for (int i = 0, k = 0; i < samples; i++)
					for (int j = 0; j < channels; j++, k++) {
						f[k] = (float)tmp[j*samples+i];
					}

				delete [] tmp;
			} else {
				for (int i = 0, j = samples*channels; i < j; i++) {
					f[i] = 0;
				}
			}

			HRESULT hr;
			if (S_OK != (hr = Deliver(pBuff, wfe->nSamplesPerSec, wfe->nChannels))) {
				return hr;
			}

			p += size;
		}
	}

	memmove(base, p, end - p);
	end = base + (end - p);
	p = base;

	m_buff.SetCount(end - p);

	return S_OK;
}
#endif /* INTERNAL_DECODER_PS2AUDIO */

#if defined(REGISTER_FILTER) || INTERNAL_DECODER_FLAC
HRESULT CMpaDecFilter::ProcessFlac()
{
	WAVEFORMATEX* wfein = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();
	UNREFERENCED_PARAMETER(wfein);

	FLAC__stream_decoder_process_single ((FLAC__StreamDecoder*) m_flac.pDecoder);
	return m_flac.hr;
}
#endif /* INTERNAL_DECODER_FLAC */

HRESULT CMpaDecFilter::GetDeliveryBuffer(IMediaSample** pSample, BYTE** pData)
{
	HRESULT hr;

	*pData = NULL;
	if (FAILED(hr = m_pOutput->GetDeliveryBuffer(pSample, NULL, NULL, 0))
			|| FAILED(hr = (*pSample)->GetPointer(pData))) {
		return hr;
	}

	AM_MEDIA_TYPE* pmt = NULL;
	if (SUCCEEDED((*pSample)->GetMediaType(&pmt)) && pmt) {
		CMediaType mt = *pmt;
		m_pOutput->SetMediaType(&mt);
		DeleteMediaType(pmt);
		pmt = NULL;
	}

	return S_OK;
}

HRESULT CMpaDecFilter::Deliver(CAtlArray<float>& pBuff, DWORD nSamplesPerSec, WORD nChannels, DWORD dwChannelMask)
{
	HRESULT hr;

	MPCSampleFormat sf = GetSampleFormat();

	CMediaType mt = CreateMediaType(sf, nSamplesPerSec, nChannels, dwChannelMask);
	WAVEFORMATEX* wfe = (WAVEFORMATEX*)mt.Format();

	int nSamples = pBuff.GetCount()/wfe->nChannels;

	if (FAILED(hr = ReconnectOutput(nSamples, mt))) {
		return hr;
	}

	CComPtr<IMediaSample> pOut;
	BYTE* pDataOut = NULL;
	if (FAILED(GetDeliveryBuffer(&pOut, &pDataOut))) {
		return E_FAIL;
	}

	REFERENCE_TIME rtDur = 10000000i64*nSamples/wfe->nSamplesPerSec;
	REFERENCE_TIME rtStart = m_rtStart, rtStop = m_rtStart + rtDur;
	m_rtStart += rtDur;
	//TRACE(_T("CMpaDecFilter: %I64d - %I64d\n"), rtStart/10000, rtStop/10000);
	if (rtStart < 0 /*200000*/ /* < 0, FIXME: 0 makes strange noises */) {
		return S_OK;
	}

	if (hr == S_OK) {
		m_pOutput->SetMediaType(&mt);
		pOut->SetMediaType(&mt);
	}

	pOut->SetTime(&rtStart, &rtStop);
	pOut->SetMediaTime(NULL, NULL);

	pOut->SetPreroll(FALSE);
	pOut->SetDiscontinuity(m_fDiscontinuity);
	m_fDiscontinuity = false;
	pOut->SetSyncPoint(TRUE);

	pOut->SetActualDataLength(pBuff.GetCount()*wfe->wBitsPerSample/8);

	WAVEFORMATEX* wfeout = (WAVEFORMATEX*)m_pOutput->CurrentMediaType().Format();
	ASSERT(wfeout->nChannels == wfe->nChannels);
	ASSERT(wfeout->nSamplesPerSec == wfe->nSamplesPerSec);
	UNREFERENCED_PARAMETER(wfeout);

	float* pDataIn = pBuff.GetData();

#define f16max (float(INT16_MAX)/INT16_PEAK)
#define f24max (float(INT24_MAX)/INT24_PEAK)
#define d32max (double(INT32_MAX)/INT32_PEAK)
#define round_f(x) ((x) > 0 ? (x) + 0.5f : (x) - 0.5f)
#define round_d(x) ((x) > 0 ? (x) + 0.5 : (x) - 0.5)

	for (unsigned int i = 0, len = pBuff.GetCount(); i < len; i++) {
		float f = *pDataIn++;

		if (f < -1) f = -1;
		switch (sf) {
			default:
			case SF_PCM16:
				if (f > f16max) f = f16max;
				*(int16_t*)pDataOut = (int16_t)round_f(f * INT16_PEAK);
				pDataOut += sizeof(int16_t);
				break;
			case SF_PCM24: {
				if (f > f24max) f = f24max;
				DWORD i24 = (DWORD)(int32_t)round_f(f * INT24_PEAK);
				*pDataOut++ = (BYTE)(i24);
				*pDataOut++ = (BYTE)(i24>>8);
				*pDataOut++ = (BYTE)(i24>>16);
			}
			break;
			case SF_PCM32: {
				double d = (double)f;
				if (d > d32max) d = d32max;
				*(int32_t*)pDataOut = (int32_t)round_d(d * INT32_PEAK);
				pDataOut += sizeof(int32_t);
			}
			break;
			case SF_FLOAT32:
				if (f > 1) f = 1;
				*(float*)pDataOut = f;
				pDataOut += sizeof(float);
				break;
		}
	}

	return m_pOutput->Deliver(pOut);
}

HRESULT CMpaDecFilter::DeliverBitstream(BYTE* pBuff, int size, int sample_rate, int samples, BYTE type)
{
	HRESULT hr;
	bool isDTSWAV = false;

	int length = 0;

	if (type == 0x0b) { // DTS
		if (size == 4096 && sample_rate == 44100 && samples == 1024) { // DTSWAV
			length = size;
			isDTSWAV = true;
		}
		else while (length < size+16) {
				length += 2048;
			}
	} else { //if (type == 0x01) { // AC3
		length = samples*4;
	}

	CMediaType mt;
	if (isDTSWAV) {
		mt = CreateMediaTypeSPDIF(sample_rate);
	} else {
		mt = CreateMediaTypeSPDIF();
	}

	if (FAILED(hr = ReconnectOutput(length, mt))) {
		return hr;
	}

	CComPtr<IMediaSample> pOut;
	BYTE* pDataOut = NULL;
	if (FAILED(GetDeliveryBuffer(&pOut, &pDataOut))) {
		return E_FAIL;
	}

	if (isDTSWAV) {
		memcpy(pDataOut, pBuff, size);
	}
	else {
		WORD* pDataOutW = (WORD*)pDataOut;
		pDataOutW[0] = 0xf872;
		pDataOutW[1] = 0x4e1f;
		pDataOutW[2] = type;
		pDataOutW[3] = size*8;
		_swab((char*)pBuff, (char*)&pDataOutW[4], size+1); //if the size is odd, the function "_swab" lose the last byte. need add one.
	}

	REFERENCE_TIME rtDur;
	rtDur = 10000000i64 * samples / sample_rate;
	REFERENCE_TIME rtStart = m_rtStart, rtStop = m_rtStart + rtDur;
	m_rtStart += rtDur;

	if (rtStart < 0) {
		return S_OK;
	}

	if (hr == S_OK) {
		m_pOutput->SetMediaType(&mt);
		pOut->SetMediaType(&mt);
	}

	pOut->SetTime(&rtStart, &rtStop);
	pOut->SetMediaTime(NULL, NULL);

	pOut->SetPreroll(FALSE);
	pOut->SetDiscontinuity(m_fDiscontinuity);
	m_fDiscontinuity = false;
	pOut->SetSyncPoint(TRUE);

	pOut->SetActualDataLength(length);

	return m_pOutput->Deliver(pOut);
}

HRESULT CMpaDecFilter::ReconnectOutput(int nSamples, CMediaType& mt)
{
	HRESULT hr;

	CComQIPtr<IMemInputPin> pPin = m_pOutput->GetConnected();
	if (!pPin) {
		return E_NOINTERFACE;
	}

	CComPtr<IMemAllocator> pAllocator;
	if (FAILED(hr = pPin->GetAllocator(&pAllocator)) || !pAllocator) {
		return hr;
	}

	ALLOCATOR_PROPERTIES props, actual;
	if (FAILED(hr = pAllocator->GetProperties(&props))) {
		return hr;
	}

	WAVEFORMATEX* wfe = (WAVEFORMATEX*)mt.Format();
	long cbBuffer = nSamples * wfe->nBlockAlign;

	if (mt != m_pOutput->CurrentMediaType() || cbBuffer > props.cbBuffer) {
		if (cbBuffer > props.cbBuffer) {
			props.cBuffers = 4;
			props.cbBuffer = cbBuffer*3/2;

			if (FAILED(hr = m_pOutput->DeliverBeginFlush())
					|| FAILED(hr = m_pOutput->DeliverEndFlush())
					|| FAILED(hr = pAllocator->Decommit())
					|| FAILED(hr = pAllocator->SetProperties(&props, &actual))
					|| FAILED(hr = pAllocator->Commit())) {
				return hr;
			}

			if (props.cBuffers > actual.cBuffers || props.cbBuffer > actual.cbBuffer) {
				NotifyEvent(EC_ERRORABORT, hr, 0);
				return E_FAIL;
			}
		}

		return S_OK;
	}

	return S_FALSE;
}

CMediaType CMpaDecFilter::CreateMediaType(MPCSampleFormat sf, DWORD nSamplesPerSec, WORD nChannels, DWORD dwChannelMask)
{
	CMediaType mt;

	mt.majortype = MEDIATYPE_Audio;
	mt.subtype = sf == SF_FLOAT32 ? MEDIASUBTYPE_IEEE_FLOAT : MEDIASUBTYPE_PCM;
	mt.formattype = FORMAT_WaveFormatEx;

	WAVEFORMATEXTENSIBLE wfex;
	memset(&wfex, 0, sizeof(wfex));
	WAVEFORMATEX* wfe = &wfex.Format;
	wfe->wFormatTag = (WORD)mt.subtype.Data1;
	wfe->nChannels = nChannels;
	wfe->nSamplesPerSec = nSamplesPerSec;
	switch (sf) {
		default:
		case SF_PCM16:
			wfe->wBitsPerSample = 16;
			break;
		case SF_PCM24:
			wfe->wBitsPerSample = 24;
			break;
		case SF_PCM32:
		case SF_FLOAT32:
			wfe->wBitsPerSample = 32;
			break;
	}
	wfe->nBlockAlign = wfe->nChannels*wfe->wBitsPerSample/8;
	wfe->nAvgBytesPerSec = wfe->nSamplesPerSec*wfe->nBlockAlign;
	mt.SetSampleSize (wfe->wBitsPerSample*wfe->nChannels/8);

	// FIXME: 24/32 bit only seems to work with WAVE_FORMAT_EXTENSIBLE
	if (dwChannelMask == 0 && (sf == SF_PCM24 || sf == SF_PCM32)) {
		dwChannelMask = nChannels == 2 ? (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT) : SPEAKER_FRONT_CENTER;
	}

	if (dwChannelMask) {
		wfex.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		wfex.Format.cbSize = sizeof(wfex) - sizeof(wfex.Format);
		wfex.dwChannelMask = dwChannelMask;
		wfex.Samples.wValidBitsPerSample = wfex.Format.wBitsPerSample;
		wfex.SubFormat = mt.subtype;
	}

	mt.SetFormat((BYTE*)&wfex, sizeof(wfex.Format) + wfex.Format.cbSize);

	return mt;
}

CMediaType CMpaDecFilter::CreateMediaTypeSPDIF(DWORD nSamplesPerSec)
{
	CMediaType mt = CreateMediaType(SF_PCM16, nSamplesPerSec, 2);
	((WAVEFORMATEX*)mt.pbFormat)->wFormatTag = WAVE_FORMAT_DOLBY_AC3_SPDIF;
	return mt;
}

HRESULT CMpaDecFilter::CheckInputType(const CMediaType* mtIn)
{
	if (0) {}
#if defined(REGISTER_FILTER) || INTERNAL_DECODER_LPCM
	else if (mtIn->subtype == MEDIASUBTYPE_DVD_LPCM_AUDIO) {
		WAVEFORMATEX* wfe = (WAVEFORMATEX*)mtIn->Format();
		if (wfe->nChannels < 1 || wfe->nChannels > 8 || (wfe->wBitsPerSample != 16 && wfe->wBitsPerSample != 20 && wfe->wBitsPerSample != 24)) {
			return VFW_E_TYPE_NOT_ACCEPTED;
		}
	}
#endif
#if defined(REGISTER_FILTER) || INTERNAL_DECODER_PS2AUDIO
	else if (mtIn->subtype == MEDIASUBTYPE_PS2_ADPCM) {
		WAVEFORMATEXPS2* wfe = (WAVEFORMATEXPS2*)mtIn->Format();
		if (wfe->dwInterleave & 0xf) { // has to be a multiple of the block size (16 bytes)
			return VFW_E_TYPE_NOT_ACCEPTED;
		}
	}
#endif
#if defined(REGISTER_FILTER) || INTERNAL_DECODER_PCM
	else if (mtIn->subtype == MEDIASUBTYPE_IEEE_FLOAT) {
		WAVEFORMATEX* wfe = (WAVEFORMATEX*)mtIn->Format();
		if (wfe->wBitsPerSample != 64) {    // only for 64-bit float PCM
			return VFW_E_TYPE_NOT_ACCEPTED; // not needed any decoders for 32-bit float
		}
	}
#endif

	for (int i = 0; i < _countof(sudPinTypesIn); i++) {
		if (*sudPinTypesIn[i].clsMajorType == mtIn->majortype
				&& *sudPinTypesIn[i].clsMinorType == mtIn->subtype) {
			return S_OK;
		}
	}

	return VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CMpaDecFilter::CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut)
{
	return SUCCEEDED(CheckInputType(mtIn))
		   && mtOut->majortype == MEDIATYPE_Audio && mtOut->subtype == MEDIASUBTYPE_PCM
		   || mtOut->majortype == MEDIATYPE_Audio && mtOut->subtype == MEDIASUBTYPE_IEEE_FLOAT
		   ? S_OK
		   : VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CMpaDecFilter::DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties)
{
	if (m_pInput->IsConnected() == FALSE) {
		return E_UNEXPECTED;
	}

	CMediaType& mt = m_pInput->CurrentMediaType();
	WAVEFORMATEX* wfe = (WAVEFORMATEX*)mt.Format();
	UNREFERENCED_PARAMETER(wfe);

	pProperties->cBuffers = 4;
	// pProperties->cbBuffer = 1;
	pProperties->cbBuffer = 48000*6*(32/8)/10; // 48KHz 6ch 32bps 100ms
	pProperties->cbAlign = 1;
	pProperties->cbPrefix = 0;

	HRESULT hr;
	ALLOCATOR_PROPERTIES Actual;
	if (FAILED(hr = pAllocator->SetProperties(pProperties, &Actual))) {
		return hr;
	}

	return pProperties->cBuffers > Actual.cBuffers || pProperties->cbBuffer > Actual.cbBuffer
		   ? E_FAIL
		   : NOERROR;
}

HRESULT CMpaDecFilter::GetMediaType(int iPosition, CMediaType* pmt)
{
	if (m_pInput->IsConnected() == FALSE) {
		return E_UNEXPECTED;
	}

	if (iPosition < 0) {
		return E_INVALIDARG;
	}
	if (iPosition > 0) {
		return VFW_S_NO_MORE_ITEMS;
	}

	CMediaType mt = m_pInput->CurrentMediaType();
	const GUID& subtype = mt.subtype;
	WAVEFORMATEX* wfe = (WAVEFORMATEX*)mt.Format();
	if (wfe == NULL) {
		return E_INVALIDARG;
	}

#if defined(REGISTER_FILTER) || INTERNAL_DECODER_AC3 || INTERNAL_DECODER_DTS
	if (GetSpeakerConfig(ac3) < 0 && (subtype == MEDIASUBTYPE_DOLBY_AC3 ||
									  subtype == MEDIASUBTYPE_WAVE_DOLBY_AC3 ||
									  subtype == MEDIASUBTYPE_DOLBY_DDPLUS ||
									  subtype == MEDIASUBTYPE_DOLBY_TRUEHD)
			|| GetSpeakerConfig(dts) < 0 && (subtype == MEDIASUBTYPE_DTS || subtype == MEDIASUBTYPE_WAVE_DTS)) {
		*pmt = CreateMediaTypeSPDIF();
	}
#else
	if (0) {}
#endif
#if defined(REGISTER_FILTER) || INTERNAL_DECODER_VORBIS
	else if (subtype == MEDIASUBTYPE_Vorbis2) {
		VORBISFORMAT2 *vf2 = (VORBISFORMAT2 *)mt.Format();
		scmap_t* scmap = &m_scmap_default[vf2->Channels-1];
		*pmt = CreateMediaType(GetSampleFormat(), vf2->SamplesPerSec, (WORD)vf2->Channels, scmap->dwChannelMask);
	}
#endif
	else {
		*pmt = CreateMediaType(GetSampleFormat(), wfe->nSamplesPerSec, min(2, wfe->nChannels));
	}

	return S_OK;
}

HRESULT CMpaDecFilter::StartStreaming()
{
	HRESULT hr = __super::StartStreaming();
	if (FAILED(hr)) {
		return hr;
	}

#if defined(REGISTER_FILTER) || INTERNAL_DECODER_AC3
	m_a52_state = a52_init(0);
#endif

#if defined(REGISTER_FILTER) || INTERNAL_DECODER_DTS
	m_dts_state = dts_init(0);
#endif

	m_ps2_state.reset();
#if defined(REGISTER_FILTER) || INTERNAL_DECODER_FLAC
	FlacInitDecoder();
#endif

	m_fDiscontinuity = false;

	return S_OK;
}

HRESULT CMpaDecFilter::StopStreaming()
{
#if defined(REGISTER_FILTER) || INTERNAL_DECODER_AC3
	if (m_a52_state != NULL) {
		a52_free(m_a52_state);
		m_a52_state = NULL;
	}
#endif

#if defined(REGISTER_FILTER) || INTERNAL_DECODER_DTS
	if (m_dts_state != NULL) {
		dts_free(m_dts_state);
		m_dts_state = NULL;
	}
#endif

#if defined(REGISTER_FILTER) || INTERNAL_DECODER_FLAC
	flac_stream_finish();
#endif

#if defined(REGISTER_FILTER) || HAS_FFMPEG_AUDIO_DECODERS
	ffmpeg_stream_finish();
#endif

	return __super::StopStreaming();
}

HRESULT	CMpaDecFilter::SetMediaType(PIN_DIRECTION dir, const CMediaType *pmt)
{
#if defined(REGISTER_FILTER) || HAS_FFMPEG_AUDIO_DECODERS
	if (dir == PINDIR_INPUT) {
		enum CodecID nCodecId = FindCodec(pmt->subtype);
		if (nCodecId != CODEC_ID_NONE && !InitFFmpeg(nCodecId)) {
			return VFW_E_TYPE_NOT_ACCEPTED;
		}
	}
#endif

	return __super::SetMediaType(dir, pmt);
}

// IMpaDecFilter

STDMETHODIMP CMpaDecFilter::SetSampleFormat(MPCSampleFormat sf)
{
	CAutoLock cAutoLock(&m_csProps);
	m_iSampleFormat = sf;
	return S_OK;
}

STDMETHODIMP_(MPCSampleFormat) CMpaDecFilter::GetSampleFormat()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_iSampleFormat;
}

STDMETHODIMP CMpaDecFilter::SetSpeakerConfig(enctype et, int sc)
{
	CAutoLock cAutoLock(&m_csProps);
	if (et >= 0 && et < etlast) {
		m_iSpeakerConfig[et] = sc;
	}
	return S_OK;
}

STDMETHODIMP_(int) CMpaDecFilter::GetSpeakerConfig(enctype et)
{
	CAutoLock cAutoLock(&m_csProps);
	if (et >= 0 && et < etlast) {
		return m_iSpeakerConfig[et];
	}
	return -1;
}

STDMETHODIMP CMpaDecFilter::SetDynamicRangeControl(enctype et, bool fDRC)
{
	CAutoLock cAutoLock(&m_csProps);
	if (et >= 0 && et < etlast) {
		m_fDynamicRangeControl[et] = fDRC;
	} else {
		return E_INVALIDARG;
	}
	return S_OK;
}

STDMETHODIMP_(bool) CMpaDecFilter::GetDynamicRangeControl(enctype et)
{
	CAutoLock cAutoLock(&m_csProps);
	if (et >= 0 && et < etlast) {
		return m_fDynamicRangeControl[et];
	}
	return false;
}

STDMETHODIMP_(DolbyDigitalMode) CMpaDecFilter::GetDolbyDigitalMode()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_DolbyDigitalMode;
}

STDMETHODIMP CMpaDecFilter::SaveSettings()
{
	CAutoLock cAutoLock(&m_csProps);
#ifdef REGISTER_FILTER
	CRegKey key;
	if (ERROR_SUCCESS == key.Create(HKEY_CURRENT_USER, _T("Software\\Gabest\\Filters\\MPEG Audio Decoder"))) {
		key.SetDWORDValue(_T("SampleFormat"), m_iSampleFormat);
		key.SetDWORDValue(_T("Ac3SpeakerConfig"), m_iSpeakerConfig[ac3]);
		key.SetDWORDValue(_T("DtsSpeakerConfig"), m_iSpeakerConfig[dts]);
		key.SetDWORDValue(_T("Ac3DynamicRangeControl"), m_fDynamicRangeControl[ac3]);
		key.SetDWORDValue(_T("DtsDynamicRangeControl"), m_fDynamicRangeControl[dts]);
	}
#else
	AfxGetApp()->WriteProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("SampleFormat"), m_iSampleFormat);
	AfxGetApp()->WriteProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("Ac3SpeakerConfig"), m_iSpeakerConfig[ac3]);
	AfxGetApp()->WriteProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("DtsSpeakerConfig"), m_iSpeakerConfig[dts]);
	AfxGetApp()->WriteProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("Ac3DynamicRangeControl"), m_fDynamicRangeControl[ac3]);
	AfxGetApp()->WriteProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("DtsDynamicRangeControl"), m_fDynamicRangeControl[dts]);
#endif

	return S_OK;
}

// ISpecifyPropertyPages2

STDMETHODIMP CMpaDecFilter::GetPages(CAUUID* pPages)
{
	CheckPointer(pPages, E_POINTER);

	pPages->cElems = 1;
	pPages->pElems = (GUID*)CoTaskMemAlloc(sizeof(GUID) * pPages->cElems);
	pPages->pElems[0] = __uuidof(CMpaDecSettingsWnd);

	return S_OK;
}

STDMETHODIMP CMpaDecFilter::CreatePage(const GUID& guid, IPropertyPage** ppPage)
{
	CheckPointer(ppPage, E_POINTER);

	if (*ppPage != NULL) {
		return E_INVALIDARG;
	}

	HRESULT hr;

	if (guid == __uuidof(CMpaDecSettingsWnd)) {
		(*ppPage = DNew CInternalPropertyPageTempl<CMpaDecSettingsWnd>(NULL, &hr))->AddRef();
	}

	return *ppPage ? S_OK : E_FAIL;
}

//
// CMpaDecInputPin
//

CMpaDecInputPin::CMpaDecInputPin(CTransformFilter* pFilter, HRESULT* phr, LPWSTR pName)
	: CDeCSSInputPin(NAME("CMpaDecInputPin"), pFilter, phr, pName)
{
}

#if defined(REGISTER_FILTER) || INTERNAL_DECODER_FLAC

#pragma region Flac callback

void CMpaDecFilter::FlacFillBuffer(BYTE buffer[], size_t *bytes)
{
	UINT nSize = min (*bytes, m_buff.GetCount());

	if (nSize > 0) {
		memcpy_s (buffer, *bytes, m_buff.GetData(), nSize);
		memmove(m_buff.GetData(), m_buff.GetData() + nSize, m_buff.GetCount() - nSize);
		m_buff.SetCount(m_buff.GetCount() - nSize);

	}
	*bytes = nSize;
}

void CMpaDecFilter::FlacDeliverBuffer  (unsigned blocksize, const __int32 * const buffer[])
{
	WAVEFORMATEX*    wfein = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();
	CAtlArray<float> pBuff;

	pBuff.SetCount (blocksize * wfein->nChannels);
	float*	pDataOut = pBuff.GetData();

	scmap_t& scmap = m_scmap_default[wfein->nChannels-1];

	switch (wfein->wBitsPerSample) {
		case 16 :
			for (unsigned i = 0; i < blocksize; i++) {
				for (int nChannel = 0; nChannel < wfein->nChannels; nChannel++) {
					FLAC__int16 nVal = (FLAC__int16)buffer[nChannel][i];
					*pDataOut = (float)nVal / INT16_PEAK;
					pDataOut++;
				}
			}
			break;
		case 20 :
		case 24 :
			for (unsigned i = 0; i < blocksize; i++) {
				for (int nChannel = 0; nChannel < wfein->nChannels; nChannel++) {
					FLAC__int32		nVal = (FLAC__int32)buffer[nChannel][i];
					*pDataOut = (float)nVal / INT24_PEAK;
					pDataOut++;
				}
			}
			break;
	}

	m_flac.hr = Deliver(pBuff, wfein->nSamplesPerSec, wfein->nChannels, scmap.dwChannelMask);
}


static FLAC__StreamDecoderReadStatus StreamDecoderRead(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data)
{
	CMpaDecFilter* pThis = static_cast<CMpaDecFilter*> (client_data);

	pThis->FlacFillBuffer (buffer, bytes);

	return (*bytes == 0) ?  FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM : FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

static FLAC__StreamDecoderWriteStatus StreamDecoderWrite(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data)
{
	CMpaDecFilter* pThis = static_cast<CMpaDecFilter*> (client_data);

	pThis->FlacDeliverBuffer (frame->header.blocksize, buffer);

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void StreamDecoderError(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
}


static void StreamDecoderMetadata(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
}

void CMpaDecFilter::FlacInitDecoder()
{
	if (!m_flac.pDecoder) {
		m_flac.pDecoder = FLAC__stream_decoder_new();
		if (m_flac.pDecoder) {
			FLAC__stream_decoder_init_stream ((FLAC__StreamDecoder*)m_flac.pDecoder,
											  StreamDecoderRead,
											  NULL,
											  NULL,
											  NULL,
											  NULL,
											  StreamDecoderWrite,
											  StreamDecoderMetadata,
											  StreamDecoderError,
											  this);
		}
	} else {
		FLAC__stream_decoder_reset ((FLAC__StreamDecoder*)m_flac.pDecoder);
	}
}


void CMpaDecFilter::flac_stream_finish()
{
	if (m_flac.pDecoder) {
		FLAC__stream_decoder_delete ((FLAC__StreamDecoder*)m_flac.pDecoder);
		m_flac.pDecoder = NULL;
	}
}

#pragma endregion

#endif /* INTERNAL_DECODER_FLAC */

#if defined(REGISTER_FILTER) || HAS_FFMPEG_AUDIO_DECODERS

#pragma region FFmpeg decoder

// Copy the given data into our buffer, including padding, so broken decoders do not overread and crash
#define COPY_TO_BUFFER(data, size) { \
  if (size + FF_INPUT_BUFFER_PADDING_SIZE > m_nFFBufferSize) { \
    m_nFFBufferSize = size+FF_INPUT_BUFFER_PADDING_SIZE; \
    m_pFFBuffer = (BYTE*)av_realloc(m_pFFBuffer, m_nFFBufferSize); \
    if (!m_pFFBuffer) { \
      m_nFFBufferSize = 0; \
      return E_FAIL; \
    } \
  }\
  memcpy(m_pFFBuffer, data, size); \
  memset(m_pFFBuffer+size, 0, FF_INPUT_BUFFER_PADDING_SIZE); \
}

HRESULT CMpaDecFilter::DeliverFFmpeg(enum CodecID nCodecId, BYTE* p, int buffsize, int& size)
{
	HRESULT hr = S_OK;
	int got_frame = 0;
	if (!m_pAVCtx || nCodecId != m_pAVCtx->codec_id) {
		if (!InitFFmpeg (nCodecId)) {
			size = 0;
			return E_FAIL;
		}
	}

	bool b_use_parse = m_pParser && ((nCodecId == CODEC_ID_TRUEHD) ? ((buffsize > 2000) ? true : false) : true); // Dirty hack for use with MPC MPEGSplitter

	BYTE* pDataInBuff = p;
	CAtlArray<float> pBuffOut;
	scmap_t* scmap = NULL;

	BYTE *tmpProcessBuf = NULL;

	AVPacket avpkt;
	av_init_packet(&avpkt);

	if (m_raData.deint_id == MAKEFOURCC('r','n','e','g') || m_raData.deint_id == MAKEFOURCC('r','p','i','s')) {

		int w   = m_raData.coded_frame_size;
		int h   = m_raData.sub_packet_h;
		int sps = m_raData.sub_packet_size;
		int len = w * h;

		if (buffsize >= len) {
			tmpProcessBuf = (BYTE *)av_mallocz(len + FF_INPUT_BUFFER_PADDING_SIZE);
			if (sps > 0 && m_raData.deint_id == MAKEFOURCC('r','n','e','g')) { // COOK and ATRAC codec
				const BYTE *srcBuf = pDataInBuff;
				for (int y = 0; y < h; y++) {
					for (int x = 0, w2 = w / sps; x < w2; x++) {
						memcpy(tmpProcessBuf + sps*(h*x+((h+1)/2)*(y&1)+(y>>1)), srcBuf, sps);
						srcBuf += sps;
					}
				}
			} else if (m_raData.deint_id == MAKEFOURCC('r','p','i','s')) { // SIPR codec
				memcpy(tmpProcessBuf, pDataInBuff, len);

				// http://mplayerhq.hu/pipermail/mplayer-dev-eng/2002-August/010569.html
				static BYTE sipr_swaps[38][2]= {
					{0,63},{1,22},{2,44},{3,90},{5,81},{7,31},{8,86},{9,58},{10,36},{12,68},
					{13,39},{14,73},{15,53},{16,69},{17,57},{19,88},{20,34},{21,71},{24,46},
					{25,94},{26,54},{28,75},{29,50},{32,70},{33,92},{35,74},{38,85},{40,56},
					{42,87},{43,65},{45,59},{48,79},{49,93},{51,89},{55,95},{61,76},{67,83},
					{77,80}
				};

				int bs=h*w*2/96; // nibbles per subpacket
				for (int n=0; n<38; n++) {
					int i=bs*sipr_swaps[n][0];
					int o=bs*sipr_swaps[n][1];
					// swap nibbles of block 'i' with 'o'
					for (int j=0; j<bs; j++) {
						int x=(i&1) ? (tmpProcessBuf[(i>>1)]>>4) : (tmpProcessBuf[(i>>1)]&15);
						int y=(o&1) ? (tmpProcessBuf[(o>>1)]>>4) : (tmpProcessBuf[(o>>1)]&15);
						if (o&1) {
							tmpProcessBuf[(o>>1)]=(tmpProcessBuf[(o>>1)]&0x0F)|(x<<4);
						} else {
							tmpProcessBuf[(o>>1)]=(tmpProcessBuf[(o>>1)]&0xF0)|x;
						}
						if (i&1) {
							tmpProcessBuf[(i>>1)]=(tmpProcessBuf[(i>>1)]&0x0F)|(y<<4);
						} else {
							tmpProcessBuf[(i>>1)]=(tmpProcessBuf[(i>>1)]&0xF0)|y;
						}
						++i;
						++o;
					}
				}
			}
			pDataInBuff = tmpProcessBuf;
			buffsize = len;
		} else {
			size = 0;
			return S_OK;
		}
	}

	while (buffsize > 0) {
		got_frame = 0;
		COPY_TO_BUFFER(pDataInBuff, buffsize);

		if (b_use_parse) {
			BYTE *pOut = NULL;
			int pOut_size = 0;
			int used_bytes = av_parser_parse2(m_pParser, m_pAVCtx, &pOut, &pOut_size, m_pFFBuffer, buffsize, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
			if (used_bytes < 0) {
				TRACE(_T("CMpaDecFilter::DeliverFFmpeg() - audio parsing failed (ret: %d)\n"), -used_bytes);
				goto fail;
			} else if (used_bytes == 0 && pOut_size == 0) {
				TRACE(_T("CMpaDecFilter::DeliverFFmpeg() - could not process buffer while parsing\n"));
				break;
			}

			if (used_bytes > 0) {
				size		+= used_bytes;
				buffsize	-= used_bytes;
				pDataInBuff += used_bytes;
			}
			if (pOut_size > 0) {
				COPY_TO_BUFFER(pOut, pOut_size);
				avpkt.data = (uint8_t *)m_pFFBuffer;
				avpkt.size = pOut_size;

				int ret2 = avcodec_decode_audio4(m_pAVCtx, m_pFrame, &got_frame, &avpkt);
				if (ret2 < 0) {
					TRACE(_T("CMpaDecFilter::DeliverFFmpeg() - decoding failed despite successfull parsing\n"));
					m_bResync = true;
					continue;
				}
			} else {
				continue;
			}
		} else {
			avpkt.data = (uint8_t *)m_pFFBuffer;
			avpkt.size = buffsize;

			int used_bytes = avcodec_decode_audio4(m_pAVCtx, m_pFrame, &got_frame, &avpkt);

			if (used_bytes < 0 || (used_bytes == 0 && !got_frame)) {
				TRACE(_T("CMpaDecFilter::DeliverFFmpeg() - decoding failed\n"));
				size = used_bytes;
				goto fail;
			} else if (used_bytes == 0) {
				TRACE(_T("CMpaDecFilter::DeliverFFmpeg() - could not process buffer while decoding\n"));
				break;
			} else if (m_pAVCtx->channels>8) {
				// sometimes avcodec_decode_audio4 can not identify the garbage and produces incorrect data.
				// this code does not solve the problem, it only reduces the likelihood of crash.
				// do it better!
				got_frame = 0;
			}

			size        += used_bytes;
			buffsize    -= used_bytes;
			pDataInBuff += used_bytes;
		}

		if (got_frame) {
			CAtlArray<float> pBuff;
			int              nRemap;
			float*           pDataOut;

			nRemap = FFGetChannelMap (m_pAVCtx);
			if (nRemap >=0) {

				switch (nCodecId) {
					case CODEC_ID_EAC3:
						scmap = &m_ffmpeg_ac3[FFGetChannelMap(m_pAVCtx)];
						break;
					case CODEC_ID_TRUEHD:
						if (m_pAVCtx->channels== 6) {
							scmap = &m_scmap_truehd_51;
							break;
						} else if (m_pAVCtx->channels== 8) {
							scmap = &m_scmap_truehd_71;
							break;
						}
					default:
						scmap = &m_scmap_default[m_pAVCtx->channels-1];
						break;
				}

				DWORD dwPCMSize = m_pFrame->nb_samples * scmap->nChannels * av_get_bytes_per_sample(m_pAVCtx->sample_fmt);
				switch (m_pAVCtx->sample_fmt) {
					case AV_SAMPLE_FMT_S16 :
						pBuff.SetCount (dwPCMSize / 2);
						pDataOut = pBuff.GetData();

						for (size_t i=0; i<pBuff.GetCount()/m_pAVCtx->channels; i++) {
							for (int ch=0; ch<m_pAVCtx->channels; ch++) {
								*pDataOut = (float)((int16_t*)m_pFrame->data[0]) [scmap->ch[ch]+i*m_pAVCtx->channels] / INT16_PEAK;
								pDataOut++;
							}
						}
						break;

					case AV_SAMPLE_FMT_S32 :
						pBuff.SetCount (dwPCMSize / 4);
						pDataOut = pBuff.GetData();

						for (size_t i=0; i<pBuff.GetCount()/m_pAVCtx->channels; i++) {
							for (int ch=0; ch<m_pAVCtx->channels; ch++) {
								*pDataOut = (float)((int32_t*)m_pFrame->data[0]) [scmap->ch[ch]+i*m_pAVCtx->channels] / INT32_PEAK;
								pDataOut++;
							}
						}
						break;
					case AV_SAMPLE_FMT_FLT:
						pBuff.SetCount (dwPCMSize / 4);
						pDataOut = pBuff.GetData();
						memcpy(pDataOut, m_pFrame->data[0], dwPCMSize);
						break;
					default :
						ASSERT(FALSE);
						break;
				}

				if (pBuff.GetCount() > 0) {
					hr = Deliver(pBuff, m_pAVCtx->sample_rate, scmap->nChannels, scmap->dwChannelMask);
					if (FAILED(hr)) {
						TRACE(_T("CMpaDecFilter::DeliverFFmpeg() - Deliver failed\n"));
						goto fail;
					}
				}

			}
		}
	}

	av_free(tmpProcessBuf);
	return hr;
fail:
	av_free(tmpProcessBuf);
	return E_FAIL;
}

bool CMpaDecFilter::InitFFmpeg(enum CodecID nCodecId)
{
	if (nCodecId == CODEC_ID_NONE) {
		return false;
	}

	bool bRet = false;

	avcodec_register_all();
	av_log_set_callback(LogLibAVCodec);

	if (m_pAVCodec) {
		ffmpeg_stream_finish();
	}
	switch (nCodecId) {
		case CODEC_ID_MP1 :
			m_pAVCodec = avcodec_find_decoder_by_name("mp1float");
		case CODEC_ID_MP2 :
			m_pAVCodec = avcodec_find_decoder_by_name("mp2float");
		case CODEC_ID_MP3 :
			m_pAVCodec = avcodec_find_decoder_by_name("mp3float");
		default :
			m_pAVCodec = avcodec_find_decoder(nCodecId);
	}

	if (m_pAVCodec) {
		DWORD nSamples, nBytesPerSec;
		WORD nChannels, nBitsPerSample, nBlockAlign;
		audioFormatTypeHandler((BYTE *)m_pInput->CurrentMediaType().Format(), m_pInput->CurrentMediaType().FormatType(), &nSamples, &nChannels, &nBitsPerSample, &nBlockAlign, &nBytesPerSec);

		if (nCodecId == CODEC_ID_AMR_NB || nCodecId == CODEC_ID_AMR_WB) {
			nChannels = 1;
			nSamples  = 8000;
		}

		m_pAVCtx                        = avcodec_alloc_context3(m_pAVCodec);
		CheckPointer(m_pAVCtx, false);

		m_pAVCtx->sample_rate           = nSamples;
		m_pAVCtx->channels              = nChannels;
		m_pAVCtx->bit_rate              = nBytesPerSec << 3;
		m_pAVCtx->bits_per_coded_sample = nBitsPerSample;
		m_pAVCtx->block_align           = nBlockAlign;

		m_pAVCtx->err_recognition       = AV_EF_CAREFUL;
		m_pAVCtx->codec_id              = nCodecId;
		if (m_pAVCodec->capabilities & CODEC_CAP_TRUNCATED) {
			m_pAVCtx->flags            |= CODEC_FLAG_TRUNCATED;
		}

		if (nCodecId != CODEC_ID_AAC && nCodecId != CODEC_ID_AAC_LATM) {
			m_pParser = av_parser_init(nCodecId);
		}

		const void *format = m_pInput->CurrentMediaType().Format();
		GUID format_type = m_pInput->CurrentMediaType().formattype;
		DWORD formatlen = m_pInput->CurrentMediaType().cbFormat;
		unsigned extralen = 0;
		getExtraData((BYTE *)format, &format_type, formatlen, NULL, &extralen);

		memset(&m_raData, 0, sizeof(m_raData));

		if (extralen) {
			if (nCodecId == CODEC_ID_COOK || nCodecId == CODEC_ID_ATRAC3 || nCodecId == CODEC_ID_SIPR) {
				uint8_t *extra = (uint8_t *)av_mallocz(extralen + FF_INPUT_BUFFER_PADDING_SIZE);
				getExtraData((BYTE *)format, &format_type, formatlen, extra, NULL);

				if (extra[0] == '.' && extra[1] == 'r' && extra[2] == 'a' && extra[3] == 0xfd) {
					HRESULT hr = ParseRealAudioHeader(extra, extralen);
					av_freep(&extra);
					if (FAILED(hr)) {
						return false;
					}
					if (nCodecId == CODEC_ID_SIPR) {
						if (m_raData.flavor > 3) {
							TRACE(_T("CMpaDecFilter::InitFFmpeg() : Invalid SIPR flavor (%d)"), m_raData.flavor);
							return false;
						}
						static BYTE sipr_subpk_size[4] = { 29, 19, 37, 20 };
						m_pAVCtx->block_align = sipr_subpk_size[m_raData.flavor];
					}
				} else {
					// Try without any processing?
					m_pAVCtx->extradata_size = extralen;
					m_pAVCtx->extradata      = extra;
				}
			} else {
				m_pAVCtx->extradata_size = extralen;
				m_pAVCtx->extradata      = (uint8_t *)av_mallocz(m_pAVCtx->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE);
				getExtraData((BYTE *)format, &format_type, formatlen, (BYTE *)m_pAVCtx->extradata, NULL);
			}
		}

		if (avcodec_open2(m_pAVCtx, m_pAVCodec, NULL)>=0) {
			m_pFrame = avcodec_alloc_frame();
			bRet     = true;
		}
	}

	if (!bRet) {
		ffmpeg_stream_finish();
	}

	return bRet;
}

void CMpaDecFilter::LogLibAVCodec(void* par,int level,const char *fmt,va_list valist)
{
#if defined(_DEBUG) && 0
	char		Msg [500];
	vsnprintf_s (Msg, sizeof(Msg), _TRUNCATE, fmt, valist);
	TRACE("AVLIB : %s", Msg);
#endif
}

void CMpaDecFilter::ffmpeg_stream_finish()
{
	m_pAVCodec = NULL;
	if (m_pAVCtx) {
		if (m_pAVCtx->extradata) {
			av_freep(&m_pAVCtx->extradata);
		}
		if (m_pAVCtx->codec) {
			avcodec_close(m_pAVCtx);
		}
		av_freep(&m_pAVCtx);
	}

	if (m_pParser) {
		av_parser_close(m_pParser);
		m_pParser = NULL;
	}

	if (m_pFrame) {
		av_freep(&m_pFrame);
	}

	if (m_pFFBuffer) {
		av_freep(&m_pFFBuffer);
	}
	m_nFFBufferSize = 0;
}

#ifndef AV_RB16
#   define AV_RB16(x)                           \
    ((((const uint8_t*)(x))[0] << 8) |          \
      ((const uint8_t*)(x))[1])
#endif

#ifndef AV_RB32
#   define AV_RB32(x)                                \
    (((uint32_t)((const uint8_t*)(x))[0] << 24) |    \
               (((const uint8_t*)(x))[1] << 16) |    \
               (((const uint8_t*)(x))[2] <<  8) |    \
                ((const uint8_t*)(x))[3])
#endif

HRESULT CMpaDecFilter::ParseRealAudioHeader(const BYTE *extra, const int extralen)
{
	const uint8_t *fmt = extra+4;
	uint16_t version = AV_RB16(fmt);
	fmt += 2;
	if (version == 3) {
		TRACE(_T("RealAudio Header version 3 unsupported\n"));
		return VFW_E_UNSUPPORTED_AUDIO;
	} else if (version == 4 || version == 5 && extralen > 50) {
		// main format block
		fmt += 2;  // word - unused (always 0)
		fmt += 4;  // byte[4] - .ra4/.ra5 signature
		fmt += 4;  // dword - unknown
		fmt += 2;  // word - Version2
		fmt += 4;  // dword - header size
		m_raData.flavor = AV_RB16(fmt); fmt += 2;  // word - codec flavor
		m_raData.coded_frame_size = AV_RB32(fmt); fmt += 4;  // dword - coded frame size
		fmt += 12; // byte[12] - unknown
		m_raData.sub_packet_h = AV_RB16(fmt); fmt += 2;  // word - sub packet h
		fmt += 2;  // word - frame size
		m_raData.sub_packet_size = m_pAVCtx->block_align = AV_RB16(fmt); fmt += 2;  // word - subpacket size
		fmt += 2;  // word - unknown
		// 6 Unknown bytes in ver 5
		if (version == 5) {
			fmt += 6;
		}
		// Audio format block
		fmt += 8;
		// Tag info in v4
		if (version == 4) {
			int len = *fmt++;
			m_raData.deint_id = AV_RB32(fmt); fmt += len;
			len = *fmt++;
			fmt += len;
		} else if (version == 5) {
			m_raData.deint_id = AV_RB32(fmt); fmt += 4;
			fmt += 4;
		}
		fmt += 3;
		if (version == 5) {
			fmt++;
		}

		int ra_extralen = AV_RB32(fmt);
		if (ra_extralen > 0)  {
			m_pAVCtx->extradata_size = ra_extralen;
			m_pAVCtx->extradata      = (uint8_t *)av_mallocz(ra_extralen + FF_INPUT_BUFFER_PADDING_SIZE);
			memcpy((void *)m_pAVCtx->extradata, fmt+4, ra_extralen);
		}
	} else {
		TRACE(_T("Unknown RealAudio Header version: %d\n"), version);
		return VFW_E_UNSUPPORTED_AUDIO;
	}

	return S_OK;
}

#pragma endregion

#endif /* HAS_FFMPEG_AUDIO_DECODERS */
