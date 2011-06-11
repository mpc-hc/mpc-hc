/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2011 see AUTHORS
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "stdafx.h"
#include <math.h>
#include <atlbase.h>
#include <mmreg.h>
#include "MpaDecFilter.h"

#include "../../../DSUtil/DSUtil.h"

#include <initguid.h>
#include <moreuuids.h>

#include <vector>
#include "PODtypes.h"
#include "avcodec.h"

#include "faad2/include/neaacdec.h"
#include "FLAC/stream_decoder.h"

#define INT24_MAX					0x7FFFFF
#define EAC3_FRAME_TYPE_RESERVED	3
#define AC3_HEADER_SIZE				7

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] = {
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_MP3},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_MPEG1AudioPayload},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_MPEG1Payload},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_MPEG1Packet},
	{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_MPEG2_AUDIO},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_MPEG2_AUDIO},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_MPEG2_AUDIO},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_MPEG2_AUDIO},
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
	{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_DTS},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_DTS},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_DTS},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_DTS},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_WAVE_DTS},
	{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_DVD_LPCM_AUDIO},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_DVD_LPCM_AUDIO},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_DVD_LPCM_AUDIO},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_DVD_LPCM_AUDIO},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_HDMV_LPCM_AUDIO},
	{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_AAC},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_AAC},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_AAC},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_AAC},
	{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_MP4A},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_MP4A},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_MP4A},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_MP4A},
	{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_mp4a},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_mp4a},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_mp4a},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_mp4a},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_AMR},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_SAMR},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_SAWB},
	{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_PS2_PCM},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_PS2_PCM},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_PS2_PCM},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PS2_PCM},
	{&MEDIATYPE_DVD_ENCRYPTED_PACK, &MEDIASUBTYPE_PS2_ADPCM},
	{&MEDIATYPE_MPEG2_PACK,			&MEDIASUBTYPE_PS2_ADPCM},
	{&MEDIATYPE_MPEG2_PES,			&MEDIASUBTYPE_PS2_ADPCM},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PS2_ADPCM},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_Vorbis2},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_FLAC_FRAMED},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_NELLYMOSER},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_NONE},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_RAW},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_TWOS},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_SOWT},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_IN24},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_IN32},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_FL32},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_FL64},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_IN24_le},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_IN32_le},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_FL32_le},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_PCM_FL64_le},
	{&MEDIATYPE_Audio,				&MEDIASUBTYPE_IMA4},
};

#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesOut[] = {
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_PCM},
};

const AMOVIESETUP_PIN sudpPins[] = {
	{L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesIn), sudPinTypesIn},
	{L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesOut), sudPinTypesOut}
};

const AMOVIESETUP_FILTER sudFilter[] = {
	{&__uuidof(CMpaDecFilter), L"MPC - MPA Decoder Filter", /*MERIT_DO_NOT_USE*/0x40000001, countof(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory},
};

CFactoryTemplate g_Templates[] = {
	{sudFilter[0].strName, &__uuidof(CMpaDecFilter), CreateInstance<CMpaDecFilter>, NULL, &sudFilter[0]},
	{L"CMpaDecPropertyPage", &__uuidof(CMpaDecSettingsWnd), CreateInstance<CInternalPropertyPageTempl<CMpaDecSettingsWnd> >},
};

int g_cTemplates = countof(g_Templates);

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
	{2, {0, 1,-1,-1,-1,-1,-1,-1}, 0},	// A52_CHANNEL
	{1, {0,-1,-1,-1,-1,-1,-1,-1}, 0}, // A52_MONO
	{2, {0, 1,-1,-1,-1,-1,-1,-1}, 0}, // A52_STEREO
	{3, {0, 2, 1,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER}, // A52_3F
	{3, {0, 1, 2,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_CENTER}, // A52_2F1R
	{4, {0, 2, 1, 3,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_CENTER}, // A52_3F1R
	{4, {0, 1, 2, 3,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT}, // A52_2F2R
	{5, {0, 2, 1, 3, 4,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT}, // A52_3F2R
	{1, {0,-1,-1,-1,-1,-1,-1,-1}, 0}, // A52_CHANNEL1
	{1, {0,-1,-1,-1,-1,-1,-1,-1}, 0}, // A52_CHANNEL2
	{2, {0, 1,-1,-1,-1,-1,-1,-1}, 0}, // A52_DOLBY

	{3, {1, 2, 0,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY},	// A52_CHANNEL|A52_LFE
	{2, {1, 0,-1,-1,-1,-1,-1,-1}, SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY}, // A52_MONO|A52_LFE
	{3, {1, 2, 0,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY}, // A52_STEREO|A52_LFE
	{4, {1, 3, 2, 0,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY}, // A52_3F|A52_LFE
	{4, {1, 2, 0, 3,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_CENTER}, // A52_2F1R|A52_LFE
	{5, {1, 3, 2, 0, 4,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_CENTER}, // A52_3F1R|A52_LFE
	{5, {1, 2, 0, 3, 4,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT}, // A52_2F2R|A52_LFE
	{6, {1, 3, 2, 0, 4, 5,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT}, // A52_3F2R|A52_LFE
	{2, {1, 0,-1,-1,-1,-1,-1,-1}, SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY}, // A52_CHANNEL1|A52_LFE
	{2, {1, 0,-1,-1,-1,-1,-1,-1}, SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY}, // A52_CHANNEL2|A52_LFE
	{3, {1, 2, 0,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY}, // A52_DOLBY|A52_LFE
},
s_scmap_dts[2*10] = {
	{1, {0,-1,-1,-1,-1,-1,-1,-1}, 0}, // DTS_MONO
	{2, {0, 1,-1,-1,-1,-1,-1,-1}, 0},	// DTS_CHANNEL
	{2, {0, 1,-1,-1,-1,-1,-1,-1}, 0}, // DTS_STEREO
	{2, {0, 1,-1,-1,-1,-1,-1,-1}, 0}, // DTS_STEREO_SUMDIFF
	{2, {0, 1,-1,-1,-1,-1,-1,-1}, 0}, // DTS_STEREO_TOTAL
	{3, {1, 2, 0,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER}, // DTS_3F
	{3, {0, 1, 2,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_CENTER}, // DTS_2F1R
	{4, {1, 2, 0, 3,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_CENTER}, // DTS_3F1R
	{4, {0, 1, 2, 3,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT}, // DTS_2F2R
	{5, {1, 2, 0, 3, 4,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT}, // DTS_3F2R

	{2, {0, 1,-1,-1,-1,-1,-1,-1}, SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY}, // DTS_MONO|DTS_LFE
	{3, {0, 1, 2,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY},	// DTS_CHANNEL|DTS_LFE
	{3, {0, 1, 2,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY}, // DTS_STEREO|DTS_LFE
	{3, {0, 1, 2,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY}, // DTS_STEREO_SUMDIFF|DTS_LFE
	{3, {0, 1, 2,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY}, // DTS_STEREO_TOTAL|DTS_LFE
	{4, {1, 2, 0, 3,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY}, // DTS_3F|DTS_LFE
	{4, {0, 1, 3, 2,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_CENTER}, // DTS_2F1R|DTS_LFE
	{5, {1, 2, 0, 4, 3,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_CENTER}, // DTS_3F1R|DTS_LFE
	{5, {0, 1, 4, 2, 3,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT}, // DTS_2F2R|DTS_LFE
	{6, {1, 2, 0, 5, 3, 4,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT}, // DTS_3F2R|DTS_LFE
},
s_scmap_vorbis[6] = {
	{1, {0,-1,-1,-1,-1,-1,-1,-1}, 0}, // 1F
	{2, {0, 1,-1,-1,-1,-1,-1,-1}, 0},	// 2F
	{3, {0, 2, 1,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER}, // 2F1R
	{4, {0, 1, 2, 3,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT}, // 2F2R
	{5, {0, 2, 1, 3, 4,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT}, // 3F2R
	{6, {0, 2, 1, 5, 3, 4,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT}, // 3F2R + LFE
},
s_scmap_hdmv[] = {
	//    FL  FR  FC  LFe BL  BR  FLC FRC
	{0, {-1,-1,-1,-1,-1,-1,-1,-1 }, 0},		// INVALID
	{1, { 0,-1,-1,-1,-1,-1,-1,-1 }, 0},		// Mono			M1, 0
	{0, {-1,-1,-1,-1,-1,-1,-1,-1 }, 0},		// INVALID
	{2, { 0, 1,-1,-1,-1,-1,-1,-1 }, 0},		// Stereo		FL, FR
	{4, { 0, 1, 2,-1,-1,-1,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER},															// 3/0			FL, FR, FC
	{4, { 0, 1, 2,-1,-1,-1,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_LOW_FREQUENCY},															// 2/1			FL, FR, Surround
	{4, { 0, 1, 2, 3,-1,-1,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY},										// 3/1			FL, FR, FC, Surround
	{4, { 0, 1, 2, 3,-1,-1,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},											// 2/2			FL, FR, BL, BR
	{6, { 0, 1, 2, 3, 4,-1,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},						// 3/2			FL, FR, FC, BL, BR
	{6, { 0, 1, 2, 5, 3, 4,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},// 3/2+LFe		FL, FR, FC, BL, BR, LFe
	{8, { 0, 1, 2, 3, 6, 4, 5,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT},	// 3/4			FL, FR, FC, BL, Bls, Brs, BR
	{8, { 0, 1, 2, 7, 4, 5, 3, 6 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT},// 3/4+LFe		FL, FR, FC, BL, Bls, Brs, BR, LFe
},
m_scmap_default[] = {
	//    FL  FR  FC  LFe BL  BR  FLC FRC
	{1, { 0,-1,-1,-1,-1,-1,-1,-1 }, 0},		// Mono			M1, 0
	{2, { 0, 1,-1,-1,-1,-1,-1,-1 }, 0},		// Stereo		FL, FR
	{3, { 0, 1, 2,-1,-1,-1,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER},															// 3/0			FL, FR, FC
	{4, { 0, 1, 2, 3,-1,-1,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY},										// 3/1			FL, FR, FC, Surround
	{5, { 0, 1, 2, 3, 4,-1,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},						// 3/2			FL, FR, FC, BL, BR
	{6, { 0, 1, 2, 3, 4, 5,-1,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},// 3/2+LFe		FL, FR, FC, BL, BR, LFe
	{7, { 0, 1, 2, 3, 4, 5, 6,-1 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT|SPEAKER_BACK_CENTER},	// 3/4			FL, FR, FC, BL, Bls, Brs, BR
	{8, { 0, 1, 2, 3, 6, 7, 4, 5 }, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_SIDE_LEFT|SPEAKER_SIDE_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},// 3/4+LFe		FL, FR, FC, BL, Bls, Brs, BR, LFe
},
m_ffmpeg_ac3[] = {
	//    FL  FR  FC  LFe BL  BR  FLC FRC
	{2, {0, 1,-1,-1,-1,-1,-1,-1}, 0},	// AC3_CHMODE_DUALMONO
	{1, {0,-1,-1,-1,-1,-1,-1,-1}, 0},	// AC3_CHMODE_MONO
	{2, {0, 1,-1,-1,-1,-1,-1,-1}, 0},	// AC3_CHMODE_STEREO
	{3, {0, 2, 1,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER}, // AC3_CHMODE_3F
	{3, {0, 1, 2,-1,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_CENTER},	// AC3_CHMODE_2F1R
	{4, {0, 2, 1, 3,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_CENTER},					// AC3_CHMODE_3F1R
	{4, {0, 1, 2, 3,-1,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},						// AC3_CHMODE_2F2R
	{5, {0, 2, 1, 3, 4,-1,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},// AC3_CHMODE_3F2R

	// LFe
	{6, {0, 1, 2, 3, 4, 5,-1,-1}, SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT|SPEAKER_FRONT_CENTER|SPEAKER_LOW_FREQUENCY|SPEAKER_BACK_LEFT|SPEAKER_BACK_RIGHT},// AC3_CHMODE_3F2R
};
#pragma warning(default : 4245)

CMpaDecFilter::CMpaDecFilter(LPUNKNOWN lpunk, HRESULT* phr)
	: CTransformFilter(NAME("CMpaDecFilter"), lpunk, __uuidof(this))
	, m_iSampleFormat(SF_PCM16)
	, m_fNormalize(false)
	, m_boost(1)
{
	if(phr) {
		*phr = S_OK;
	}

	m_pInput = DNew CMpaDecInputPin(this, phr, L"In");
	if(!m_pInput) {
		*phr = E_OUTOFMEMORY;
	}
	if(FAILED(*phr)) {
		return;
	}

	m_pOutput = DNew CTransformOutputPin(NAME("CTransformOutputPin"), this, phr, L"Out");
	if(!m_pOutput) {
		*phr = E_OUTOFMEMORY;
	}
	if(FAILED(*phr))  {
		delete m_pInput, m_pInput = NULL;
		return;
	}

	m_iSpeakerConfig[ac3] = A52_STEREO;
	m_iSpeakerConfig[dts] = DTS_STEREO;
	m_iSpeakerConfig[aac] = AAC_STEREO;
	m_fDynamicRangeControl[ac3] = false;
	m_fDynamicRangeControl[dts] = false;
	m_fDynamicRangeControl[aac] = false;
	m_DolbyDigitalMode			= DD_Unknown;
#if defined(REGISTER_FILTER) | HAS_FFMPEG_AUDIO_DECODERS
	m_pAVCodec					= NULL;
	m_pAVCtx					= NULL;
	m_pParser					= NULL;
	m_pPCMData					= NULL;
#endif
#if defined(REGISTER_FILTER) | INTERNAL_DECODER_FLAC
	memset (&m_flac, 0, sizeof(m_flac));
#endif

#if defined(REGISTER_FILTER) | HAS_FFMPEG_AUDIO_DECODERS
	m_pFFBuffer					= NULL;
	m_nFFBufferSize				= 0;
#endif

#ifdef REGISTER_FILTER
	CRegKey key;
	if(ERROR_SUCCESS == key.Open(HKEY_CURRENT_USER, _T("Software\\Gabest\\Filters\\MPEG Audio Decoder"), KEY_READ)) {
		DWORD dw;
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("SampleFormat"), dw)) {
			m_iSampleFormat = (MPCSampleFormat)dw;
		}
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("Normalize"), dw)) {
			m_fNormalize = !!dw;
		}
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("Boost"), dw)) {
			m_boost = *(float*)&dw;
		}
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("Ac3SpeakerConfig"), dw)) {
			m_iSpeakerConfig[ac3] = (int)dw;
		}
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("DtsSpeakerConfig"), dw)) {
			m_iSpeakerConfig[dts] = (int)dw;
		}
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("AacSpeakerConfig"), dw)) {
			m_iSpeakerConfig[aac] = (int)dw;
		}
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("Ac3DynamicRangeControl"), dw)) {
			m_fDynamicRangeControl[ac3] = !!dw;
		}
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("DtsDynamicRangeControl"), dw)) {
			m_fDynamicRangeControl[dts] = !!dw;
		}
		if(ERROR_SUCCESS == key.QueryDWORDValue(_T("AacDynamicRangeControl"), dw)) {
			m_fDynamicRangeControl[aac] = !!dw;
		}
	}
#else
	DWORD dw;
	m_iSampleFormat = (MPCSampleFormat)AfxGetApp()->GetProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("SampleFormat"), m_iSampleFormat);
	m_fNormalize = !!AfxGetApp()->GetProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("Normalize"), m_fNormalize);
	dw = AfxGetApp()->GetProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("Boost"), *(DWORD*)&m_boost);
	m_boost = *(float*)&dw;
	m_iSpeakerConfig[ac3] = AfxGetApp()->GetProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("Ac3SpeakerConfig"), m_iSpeakerConfig[ac3]);
	m_iSpeakerConfig[dts] = AfxGetApp()->GetProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("DtsSpeakerConfig"), m_iSpeakerConfig[dts]);
	m_iSpeakerConfig[aac] = AfxGetApp()->GetProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("AacSpeakerConfig"), m_iSpeakerConfig[aac]);
	m_fDynamicRangeControl[ac3] = !!AfxGetApp()->GetProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("Ac3DynamicRangeControl"), m_fDynamicRangeControl[ac3]);
	m_fDynamicRangeControl[dts] = !!AfxGetApp()->GetProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("DtsDynamicRangeControl"), m_fDynamicRangeControl[dts]);
	m_fDynamicRangeControl[aac] = !!AfxGetApp()->GetProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("AacDynamicRangeControl"), m_fDynamicRangeControl[aac]);
#endif
}

CMpaDecFilter::~CMpaDecFilter()
{
#if defined(REGISTER_FILTER) | HAS_FFMPEG_AUDIO_DECODERS
	if (m_pFFBuffer) {
		free(m_pFFBuffer);
	}
	m_nFFBufferSize	= 0;
#endif
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
	m_sample_max = 0.1f;
	return __super::EndFlush();
}

HRESULT CMpaDecFilter::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	CAutoLock cAutoLock(&m_csReceive);
	m_buff.RemoveAll();
	m_sample_max = 0.1f;
	m_ps2_state.sync = false;
	m_DolbyDigitalMode = DD_Unknown;
#if defined(REGISTER_FILTER) | HAS_FFMPEG_AUDIO_DECODERS
	if (m_pAVCtx) {
		avcodec_flush_buffers (m_pAVCtx);
	}
#endif
#if defined(REGISTER_FILTER) | INTERNAL_DECODER_FLAC
	if (m_flac.pDecoder) {
		FLAC__stream_decoder_flush((FLAC__StreamDecoder*) m_flac.pDecoder);
	}
#endif
	return __super::NewSegment(tStart, tStop, dRate);
}

HRESULT CMpaDecFilter::Receive(IMediaSample* pIn)
{
	CAutoLock cAutoLock(&m_csReceive);

	HRESULT hr;

	AM_SAMPLE2_PROPERTIES* const pProps = m_pInput->SampleProps();
	if(pProps->dwStreamId != AM_STREAM_MEDIA) {
		return m_pOutput->Deliver(pIn);
	}

	AM_MEDIA_TYPE* pmt;
	if(SUCCEEDED(pIn->GetMediaType(&pmt)) && pmt) {
		CMediaType mt(*pmt);
		m_pInput->SetMediaType(&mt);
		DeleteMediaType(pmt);
		pmt = NULL;
		m_sample_max = 0.1f;
#if defined(REGISTER_FILTER) | INTERNAL_DECODER_AAC
		m_aac_state.init(mt);
#endif
#if defined(REGISTER_FILTER) | INTERNAL_DECODER_VORBIS
		m_vorbis.init(mt);
#endif
		m_DolbyDigitalMode = DD_Unknown;
	}

	BYTE* pDataIn = NULL;
	if(FAILED(hr = pIn->GetPointer(&pDataIn))) {
		return hr;
	}

	long len = pIn->GetActualDataLength();

	(static_cast<CDeCSSInputPin*>(m_pInput))->StripPacket(pDataIn, len);

	REFERENCE_TIME rtStart = _I64_MIN, rtStop = _I64_MIN;
	hr = pIn->GetTime(&rtStart, &rtStop);

	if(pIn->IsDiscontinuity() == S_OK) {
		m_fDiscontinuity = true;
		m_buff.RemoveAll();
		m_sample_max = 0.1f;
		// ASSERT(SUCCEEDED(hr)); // what to do if not?
		if(FAILED(hr)) {
			TRACE(_T("mpa: disc. w/o timestamp\n"));    // lets wait then...
			return S_OK;
		}
		m_rtStart = rtStart;
	}

	const GUID& subtype = m_pInput->CurrentMediaType().subtype;

	BOOL bNoJitterControl = false;
	if(subtype == MEDIASUBTYPE_AMR || subtype == MEDIASUBTYPE_SAMR || subtype == MEDIASUBTYPE_SAWB) {
		bNoJitterControl = true;
	}

	if(SUCCEEDED(hr) && _abs64((m_rtStart - rtStart)) > 1000000i64  && !bNoJitterControl) { // +-100ms jitter is allowed for now 
		m_buff.RemoveAll(); 
		m_rtStart = rtStart; 
	} 

	int bufflen = m_buff.GetCount();
	m_buff.SetCount(bufflen + len, 4096);
	memcpy(m_buff.GetData() + bufflen, pDataIn, len);
	len += bufflen;

#if defined(REGISTER_FILTER) | INTERNAL_DECODER_AMR
	if(subtype == MEDIASUBTYPE_AMR || subtype == MEDIASUBTYPE_SAMR) {
		hr = ProcessFFmpeg(CODEC_ID_AMR_NB);
	} else if(subtype == MEDIASUBTYPE_SAWB) {
		hr = ProcessFFmpeg(CODEC_ID_AMR_WB);
	}
#else
	if(0) {}
#endif
#if defined(REGISTER_FILTER) | INTERNAL_DECODER_LPCM
	else if(subtype == MEDIASUBTYPE_DVD_LPCM_AUDIO) {
		hr = ProcessLPCM();
	} else if(subtype == MEDIASUBTYPE_HDMV_LPCM_AUDIO) {
		hr = ProcessHdmvLPCM(pIn->IsSyncPoint());
	}
#endif
#if defined(REGISTER_FILTER) | INTERNAL_DECODER_AC3
	else if(subtype == MEDIASUBTYPE_DOLBY_AC3 ||
			subtype == MEDIASUBTYPE_WAVE_DOLBY_AC3 ||
			subtype == MEDIASUBTYPE_DOLBY_DDPLUS ||
			subtype == MEDIASUBTYPE_DOLBY_TRUEHD) {
		hr = ProcessAC3();
	}
#endif
#if defined(REGISTER_FILTER) | INTERNAL_DECODER_DTS
	else if(subtype == MEDIASUBTYPE_DTS || subtype == MEDIASUBTYPE_WAVE_DTS) {
		hr = ProcessDTS();
	}
#endif
#if defined(REGISTER_FILTER) | INTERNAL_DECODER_AAC
	else if(subtype == MEDIASUBTYPE_AAC || subtype == MEDIASUBTYPE_MP4A || subtype == MEDIASUBTYPE_mp4a) {
		hr = ProcessAAC();
	}
#endif
#if defined(REGISTER_FILTER) | INTERNAL_DECODER_PS2AUDIO
	else if(subtype == MEDIASUBTYPE_PS2_PCM) {
		hr = ProcessPS2PCM();
	} else if(subtype == MEDIASUBTYPE_PS2_ADPCM) {
		hr = ProcessPS2ADPCM();
	}
#endif
#if defined(REGISTER_FILTER) | INTERNAL_DECODER_VORBIS
	else if(subtype == MEDIASUBTYPE_Vorbis2) {
		hr = ProcessVorbis();
	}
#endif
#if defined(REGISTER_FILTER) | INTERNAL_DECODER_FLAC
	else if(subtype == MEDIASUBTYPE_FLAC_FRAMED) {
		hr = ProcessFlac();
	}
#endif
#if defined(REGISTER_FILTER) | INTERNAL_DECODER_NELLYMOSER
	else if(subtype == MEDIASUBTYPE_NELLYMOSER) {
		hr = ProcessFFmpeg(CODEC_ID_NELLYMOSER);
	}
#endif
#if defined(REGISTER_FILTER) | INTERNAL_DECODER_PCM
	else if(subtype == MEDIASUBTYPE_PCM_NONE ||
			subtype == MEDIASUBTYPE_PCM_RAW) {
		if(m_buff.GetCount() < 480) {
			return S_OK;
		}
		hr = ProcessPCMraw();
	} else if(subtype == MEDIASUBTYPE_PCM_TWOS) {
		if(m_buff.GetCount() < 960) {
			return S_OK;
		}
		hr = ProcessPCMintBE();
	} else if(subtype == MEDIASUBTYPE_PCM_SOWT) {
		if(m_buff.GetCount() < 960) {
			return S_OK;
		}
		hr = ProcessPCMintLE();
	} else if(subtype == MEDIASUBTYPE_PCM_IN24 ||
			  subtype == MEDIASUBTYPE_PCM_IN32) {
		if(m_buff.GetCount() < 1920) {
			return S_OK;
		}
		hr = ProcessPCMintBE();
	} else if(subtype == MEDIASUBTYPE_PCM_IN24_le ||
			  subtype == MEDIASUBTYPE_PCM_IN32_le) {
		if(m_buff.GetCount() < 1920) {
			return S_OK;
		}
		hr = ProcessPCMintLE();
	} else if(subtype == MEDIASUBTYPE_PCM_FL32 ||
			  subtype == MEDIASUBTYPE_PCM_FL64) {
		if(m_buff.GetCount() < 3840) {
			return S_OK;
		}
		hr = ProcessPCMfloatBE();
	} else if(subtype == MEDIASUBTYPE_PCM_FL32_le ||
			  subtype == MEDIASUBTYPE_PCM_FL64_le) {
		if(m_buff.GetCount() < 3840) {
			return S_OK;
		}
		hr = ProcessPCMfloatLE();
	}
#endif
#if defined(REGISTER_FILTER) | INTERNAL_DECODER_IMA4
	else if(subtype == MEDIASUBTYPE_IMA4) {
		hr = ProcessFFmpeg(CODEC_ID_ADPCM_IMA_QT);
	}
#endif
#if defined(REGISTER_FILTER) | INTERNAL_DECODER_MPEGAUDIO
	else { // if(.. the rest ..)
		hr = ProcessMPA();
	}
#endif

	return hr;
}

#if defined(REGISTER_FILTER) | INTERNAL_DECODER_LPCM
HRESULT CMpaDecFilter::ProcessLPCM()
{
	WAVEFORMATEX* wfein = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();

	if (wfein->nChannels < 1 || wfein->nChannels > 8) {
		return ERROR_NOT_SUPPORTED;
	}

	scmap_t*		remap		= &m_scmap_default [wfein->nChannels-1];
	int				nChannels	= wfein->nChannels;

	BYTE*			pDataIn		= m_buff.GetData();
	int BytesPerDoubleSample	= (wfein->wBitsPerSample * 2)/8;
	int BytesPerDoubleChannelSample = BytesPerDoubleSample * nChannels;
	int				nInBytes	= m_buff.GetCount();
	int				len			= (nInBytes / BytesPerDoubleChannelSample) * (BytesPerDoubleChannelSample); // We always code 2 samples at a time

	CAtlArray<float> pBuff;
	pBuff.SetCount((len/BytesPerDoubleSample) * 2);

	float*	pDataOut = pBuff.GetData();

	switch (wfein->wBitsPerSample) {
		case 16 : {
			long nSamples = len/(BytesPerDoubleChannelSample);
			int16 Temp[2][8];
			for (int i=0; i<nSamples; i++) {
				for(int j = 0; j < nChannels; j++) {
					uint16 All = *((uint16 *)pDataIn);
					pDataIn += 2;
					int16 Part1 = (All & 0xFF) << 8 | (All & 0xFF00) >> 8;
					Temp[0][j] = Part1;
				}
				for(int j = 0; j < nChannels; j++) {
					uint16 All = *((uint16 *)pDataIn);
					pDataIn += 2;
					int16 Part1 = (All & 0xFF) << 8 | (All & 0xFF00) >> 8;
					Temp[1][j] = Part1;
				}

				for(int j = 0; j < nChannels; j++) {
					int		nRemap = remap->ch[j];
					*pDataOut = float(Temp[0][nRemap]) / float(SHRT_MAX);
					++pDataOut;
				}
				for(int j = 0; j < nChannels; j++) {
					int		nRemap = remap->ch[j];
					*pDataOut = float(Temp[1][nRemap]) / float(SHRT_MAX);
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
				for(int j = 0; j < nChannels; j++) {
					uint32 All = *((uint16 *)pDataIn);
					pDataIn += 2;
					uint32 Part1 = (All & 0xFF) << 24 | (All & 0xFF00) << 8;
					Temp[0][j] = Part1;
				}
				for(int j = 0; j < nChannels; j++) {
					uint32 All = *((uint16 *)pDataIn);
					pDataIn += 2;
					uint32 Part1 = (All & 0xFF) << 24 | (All & 0xFF00) << 8;
					Temp[1][j] = Part1;
				}

				// Continue with lower bits
				for(int j = 0; j < nChannels; j++) {
					uint32 All = *((uint8 *)pDataIn);
					pDataIn += 1;
					Temp[0][j] = int32(Temp[0][j] | (All << 8)) >> 8;
				}
				for(int j = 0; j < nChannels; j++) {
					uint32 All = *((uint8 *)pDataIn);
					pDataIn += 1;
					Temp[1][j] = int32(Temp[1][j] | (All << 8)) >> 8;
				}

				// Convert into float
				for(int j = 0; j < nChannels; j++) {
					int		nRemap = remap->ch[j];
					*pDataOut = float(Temp[0][nRemap]) / float(1<<23);
					++pDataOut;
				}
				for(int j = 0; j < nChannels; j++) {
					int		nRemap = remap->ch[j];
					*pDataOut = float(Temp[1][nRemap]) / float(1<<23);
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
				for(int j = 0; j < nChannels; j++) {
					uint32 All = *((uint16 *)pDataIn);
					pDataIn += 2;
					uint32 Part1 = (All & 0xFF) << 24 | (All & 0xFF00) << 8;
					Temp[0][j] = Part1;
				}
				for(int j = 0; j < nChannels; j++) {
					uint32 All = *((uint16 *)pDataIn);
					pDataIn += 2;
					uint32 Part1 = (All & 0xFF) << 24 | (All & 0xFF00) << 8;
					Temp[1][j] = Part1;
				}

				// Continue with lower bits
				for(int j = 0; j < nChannels; j++) {
					uint32 All = *((uint8 *)pDataIn);
					pDataIn += 1;
					Temp[0][j] = int32(Temp[0][j] | ((All&0xf0) << 8)) >> 8;
					Temp[1][j] = int32(Temp[1][j] | ((All&0x0f) << 12)) >> 8;
				}

				// Convert into float
				for(int j = 0; j < nChannels; j++) {
					int		nRemap = remap->ch[j];
					*pDataOut = float(Temp[0][nRemap]) / float(1<<23);
					++pDataOut;
				}
				for(int j = 0; j < nChannels; j++) {
					int		nRemap = remap->ch[j];
					*pDataOut = float(Temp[1][nRemap]) / float(1<<23);
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

	scmap_t* remap		= &s_scmap_hdmv [wfein->channel_conf];
	int	nChannels		= wfein->nChannels;
	int xChannels		= nChannels + (nChannels % 2);
	int BytesPerSample	= (wfein->wBitsPerSample + 7) / 8;
	int BytesPerFrame	= BytesPerSample * xChannels;

	BYTE* pDataIn		= m_buff.GetData();
	int len				= m_buff.GetCount() - (m_buff.GetCount() % BytesPerFrame);
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
				for(int j = 0; j < nChannels; j++) {
					BYTE nRemap = remap->ch[j];
					*pDataOut = (float)(short)(pDataIn[nRemap*2]<<8 | pDataIn[nRemap*2+1]) / 32768;
					pDataOut++;
				}
				pDataIn += xChannels*2;
			}
			break;
		case 24 :
		case 20 :
			for (int i=0; i<nFrames; i++) {
				for(int j = 0; j < nChannels; j++) {
					BYTE nRemap = remap->ch[j];
					*pDataOut = (float)(long)(pDataIn[nRemap*3]<<24 | pDataIn[nRemap*3+1]<<16 | pDataIn[nRemap*3+2]<<8) / 0x80000000;
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

#if defined(REGISTER_FILTER) | INTERNAL_DECODER_AC3
HRESULT CMpaDecFilter::ProcessA52(BYTE* p, int buffsize, int& size, bool& fEnoughData)
{
	int flags, sample_rate, bit_rate;

	if((size = a52_syncinfo(p, &flags, &sample_rate, &bit_rate)) > 0) {
		//			TRACE(_T("ac3: size=%d, flags=%08x, sample_rate=%d, bit_rate=%d\n"), size, flags, sample_rate, bit_rate);

		fEnoughData = size <= buffsize;

		if(fEnoughData) {
			int iSpeakerConfig = GetSpeakerConfig(ac3);

			if(iSpeakerConfig < 0) {
				HRESULT hr;
				if(S_OK != (hr = Deliver(p, size, bit_rate, 0x0001))) {
					return hr;
				}
			} else {
				flags = iSpeakerConfig&(A52_CHANNEL_MASK|A52_LFE);
				flags |= A52_ADJUST_LEVEL;

				sample_t level = 1, gain = 1, bias = 0;
				level *= gain;

				if(a52_frame(m_a52_state, p, &flags, &level, bias) == 0) {
					if(GetDynamicRangeControl(ac3)) {
						a52_dynrng(m_a52_state, NULL, NULL);
					}

					int scmapidx = min(flags&A52_CHANNEL_MASK, countof(s_scmap_ac3)/2);
					scmap_t& scmap = s_scmap_ac3[scmapidx + ((flags&A52_LFE)?(countof(s_scmap_ac3)/2):0)];

					CAtlArray<float> pBuff;
					pBuff.SetCount(6*256*scmap.nChannels);
					float* p = pBuff.GetData();

					int i = 0;

					for(; i < 6 && a52_block(m_a52_state) == 0; i++) {
						sample_t* samples = a52_samples(m_a52_state);

						for(int j = 0; j < 256; j++, samples++) {
							for(int ch = 0; ch < scmap.nChannels; ch++) {
								ASSERT(scmap.ch[ch] != -1);
								*p++ = (float)(*(samples + 256*scmap.ch[ch]) / level);
							}
						}
					}

					if(i == 6) {
						HRESULT hr;
						if(S_OK != (hr = Deliver(pBuff, sample_rate, scmap.nChannels, scmap.dwChannelMask))) {
							return hr;
						}
					}
				}
			}
		}
	}

	return S_OK;
}

#if 0	// Old AC3 ! (to remove later...)

HRESULT CMpaDecFilter::ProcessAC3()
{
	BYTE* p = m_buff.GetData();
	BYTE* base = p;
	BYTE* end = p + m_buff.GetCount();

	while(end - p >= 7) {
		int size = 0, flags, sample_rate, bit_rate;

		if((size = a52_syncinfo(p, &flags, &sample_rate, &bit_rate)) > 0) {
			//			TRACE(_T("ac3: size=%d, flags=%08x, sample_rate=%d, bit_rate=%d\n"), size, flags, sample_rate, bit_rate);

			bool fEnoughData = p + size <= end;

			if(fEnoughData) {
				int iSpeakerConfig = GetSpeakerConfig(ac3);

				if(iSpeakerConfig < 0) {
					HRESULT hr;
					if(S_OK != (hr = Deliver(p, size, bit_rate, 0x0001))) {
						return hr;
					}
				} else {
					flags = iSpeakerConfig&(A52_CHANNEL_MASK|A52_LFE);
					flags |= A52_ADJUST_LEVEL;

					sample_t level = 1, gain = 1, bias = 0;
					level *= gain;

					if(a52_frame(m_a52_state, p, &flags, &level, bias) == 0) {
						if(GetDynamicRangeControl(ac3)) {
							a52_dynrng(m_a52_state, NULL, NULL);
						}

						int scmapidx = min(flags&A52_CHANNEL_MASK, countof(s_scmap_ac3)/2);
						scmap_t& scmap = s_scmap_ac3[scmapidx + ((flags&A52_LFE)?(countof(s_scmap_ac3)/2):0)];

						CAtlArray<float> pBuff;
						pBuff.SetCount(6*256*scmap.nChannels);
						float* p = pBuff.GetData();

						int i = 0;

						for(; i < 6 && a52_block(m_a52_state) == 0; i++) {
							sample_t* samples = a52_samples(m_a52_state);

							for(int j = 0; j < 256; j++, samples++) {
								for(int ch = 0; ch < scmap.nChannels; ch++) {
									ASSERT(scmap.ch[ch] != -1);
									*p++ = (float)(*(samples + 256*scmap.ch[ch]) / level);
								}
							}
						}

						if(i == 6) {
							HRESULT hr;
							if(S_OK != (hr = Deliver(pBuff, sample_rate, scmap.nChannels, scmap.dwChannelMask))) {
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

			if(!fEnoughData) {
				break;
			}
		} else {
			p++;
		}
	}

	m_buff.SetCount(end - p);

	return S_OK;
}

#else

HRESULT CMpaDecFilter::ProcessAC3()
{
	HRESULT hr;
	BYTE* p = m_buff.GetData();
	BYTE* base = p;
	BYTE* end = p + m_buff.GetCount();

	while(p < end && end - p >= AC3_HEADER_SIZE) {
		int		size = 0;
		bool	fEnoughData = true;

		if (m_DolbyDigitalMode != DD_TRUEHD && m_DolbyDigitalMode != DD_MLP && (*((__int16*)p) == 0x770b)) {	/* AC3-EAC3 syncword */
			BYTE	bsid = p[5] >> 3;
			if ((m_DolbyDigitalMode != DD_EAC3) && bsid <= 12) {
				m_DolbyDigitalMode = DD_AC3;
				if (FAILED (hr = ProcessA52 (p, end-p, size, fEnoughData))) {
					return hr;
				}
			} else if (bsid <= 16) {
				DeliverFFmpeg(CODEC_ID_EAC3, p, end-p, size);
				if (size > 0) {
					m_DolbyDigitalMode = DD_EAC3;
				}
			} else {
				p++;
				continue;
			}
		} else if ( (*((__int32*)(p+4)) == 0xba6f72f8) ||	// True HD major sync frame
					m_DolbyDigitalMode == DD_TRUEHD ) {
			int		nLenght = (((p[0]<<8) + p[1]) & 0x0FFF)*2;

			m_DolbyDigitalMode = DD_TRUEHD;

			if (nLenght >= 4) {
				DeliverFFmpeg(CODEC_ID_TRUEHD, p, end-p, size);
				if (size<0) {
					size = end-p;
				}
			}
		} else if ( (*((__int32*)(p+4)) == 0xbb6f72f8) ||
					m_DolbyDigitalMode == DD_MLP ) {	// MLP
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
		if(!fEnoughData) {
			break;
		}
	}

	m_buff.SetCount(end - p);

	return S_OK;
}
#endif
#endif /* INTERNAL_DECODER_AC3 */

#if defined(REGISTER_FILTER) | HAS_FFMPEG_AUDIO_DECODERS
HRESULT CMpaDecFilter::ProcessFFmpeg(int nCodecId)
{
	HRESULT hr;
	BYTE* p = m_buff.GetData();
	BYTE* base = p;
	BYTE* end = p + m_buff.GetCount();

	int		size = 0;
	hr = DeliverFFmpeg(nCodecId, p, end-p, size);
	if (size <= 0) {
		return S_OK;
	}
	p += size;
	memmove(base, p, end - p);
	end = base + (end - p);
	p = base;
	m_buff.SetCount(end - p);

	return hr;
}
#endif /* HAS_FFMPEG_AUDIO_DECODERS */

#if defined(REGISTER_FILTER) | INTERNAL_DECODER_DTS
HRESULT CMpaDecFilter::ProcessDTS()
{
	BYTE* p = m_buff.GetData();
	BYTE* base = p;
	BYTE* end = p + m_buff.GetCount();

	while(end - p >= 14) {
		int size = 0, flags, sample_rate, bit_rate, frame_length;

		if((size = dts_syncinfo(m_dts_state, p, &flags, &sample_rate, &bit_rate, &frame_length)) > 0) {
			//			TRACE(_T("dts: size=%d, flags=%08x, sample_rate=%d, bit_rate=%d, frame_length=%d\n"), size, flags, sample_rate, bit_rate, frame_length);
			bit_rate = int (size * 8i64 * sample_rate / frame_length); // calculate actual bitrate

			bool fEnoughData = p + size <= end;

			if(fEnoughData) {
				int iSpeakerConfig = GetSpeakerConfig(dts);

				if(iSpeakerConfig < 0) {
					HRESULT hr;
					if(S_OK != (hr = Deliver(p, size, bit_rate, 0x000b))) {
						return hr;
					}
				} else {
					flags = iSpeakerConfig&(DTS_CHANNEL_MASK|DTS_LFE);
					flags |= DTS_ADJUST_LEVEL;

					sample_t level = 1, gain = 1, bias = 0;
					level *= gain;

					if(dts_frame(m_dts_state, p, &flags, &level, bias) == 0) {
						if(GetDynamicRangeControl(dts)) {
							dts_dynrng(m_dts_state, NULL, NULL);
						}

						int scmapidx = min(flags&DTS_CHANNEL_MASK, countof(s_scmap_dts)/2);
						scmap_t& scmap = s_scmap_dts[scmapidx + ((flags&DTS_LFE)?(countof(s_scmap_dts)/2):0)];

						int blocks = dts_blocks_num(m_dts_state);

						CAtlArray<float> pBuff;
						pBuff.SetCount(blocks*256*scmap.nChannels);
						float* p = pBuff.GetData();

						int i = 0;

						for(; i < blocks && dts_block(m_dts_state) == 0; i++) {
							sample_t* samples = dts_samples(m_dts_state);

							for(int j = 0; j < 256; j++, samples++) {
								for(int ch = 0; ch < scmap.nChannels; ch++) {
									ASSERT(scmap.ch[ch] != -1);
									*p++ = (float)(*(samples + 256*scmap.ch[ch]) / level);
								}
							}
						}

						if(i == blocks) {
							HRESULT hr;
							if(S_OK != (hr = Deliver(pBuff, sample_rate, scmap.nChannels, scmap.dwChannelMask))) {
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

			if(!fEnoughData) {
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

#if defined(REGISTER_FILTER) | INTERNAL_DECODER_AAC
HRESULT CMpaDecFilter::ProcessAAC()
{
	int iSpeakerConfig = GetSpeakerConfig(aac);

	NeAACDecConfigurationPtr c = NeAACDecGetCurrentConfiguration(m_aac_state.h);
	c->downMatrix = iSpeakerConfig;
	NeAACDecSetConfiguration(m_aac_state.h, c);

	NeAACDecFrameInfo info;
	float* src = (float*)NeAACDecDecode(m_aac_state.h, &info, m_buff.GetData(), m_buff.GetCount());
	m_buff.RemoveAll();
	//if(!src) return E_FAIL;
	if(info.error) {
		m_aac_state.init(m_pInput->CurrentMediaType());
	}
	if(!src || info.samples == 0) {
		return S_OK;
	}

	// HACK: bug in faad2 with mono sources?
	if(info.channels == 2 && info.channel_position[1] == UNKNOWN_CHANNEL) {
		info.channel_position[0] = FRONT_CHANNEL_LEFT;
		info.channel_position[1] = FRONT_CHANNEL_RIGHT;
	}

	CAtlArray<float> pBuff;
	pBuff.SetCount(info.samples);
	float* dst = pBuff.GetData();

	CAtlMap<int, int> chmask;
	chmask[FRONT_CHANNEL_CENTER] = SPEAKER_FRONT_CENTER;
	chmask[FRONT_CHANNEL_LEFT] = SPEAKER_FRONT_LEFT;
	chmask[FRONT_CHANNEL_RIGHT] = SPEAKER_FRONT_RIGHT;
	chmask[SIDE_CHANNEL_LEFT] = SPEAKER_SIDE_LEFT;
	chmask[SIDE_CHANNEL_RIGHT] = SPEAKER_SIDE_RIGHT;
	chmask[BACK_CHANNEL_LEFT] = SPEAKER_BACK_LEFT;
	chmask[BACK_CHANNEL_RIGHT] = SPEAKER_BACK_RIGHT;
	chmask[BACK_CHANNEL_CENTER] = SPEAKER_BACK_CENTER;
	chmask[LFE_CHANNEL] = SPEAKER_LOW_FREQUENCY;

	DWORD dwChannelMask = 0;
	for(int i = 0; i < info.channels; i++) {
		if(info.channel_position[i] == UNKNOWN_CHANNEL) {
			ASSERT(0);
			return E_FAIL;
		}
		dwChannelMask |= chmask[info.channel_position[i]];
	}

	int chmap[countof(info.channel_position)];
	memset(chmap, 0, sizeof(chmap));

	for(unsigned char i = 0; i < info.channels; ++i) {
		unsigned int ch = 0, mask = chmask[info.channel_position[i]];

		for(unsigned int j = 0; j < 32; ++j) {
			if(dwChannelMask & (1 << j)) {
				if((unsigned int)(1 << j) == mask) {
					chmap[i] = ch;
					break;
				}
				++ch;
			}
		}
	}

	if(info.channels <= 2) {
		dwChannelMask = 0;
	}

	for(int j = 0; j < info.samples; j += info.channels, dst += info.channels)
		for(int i = 0; i < info.channels; i++) {
			dst[chmap[i]] = *src++;
		}

	HRESULT hr;
	if(S_OK != (hr = Deliver(pBuff, info.samplerate, info.channels, dwChannelMask))) {
		return hr;
	}

	return S_OK;
}
#endif /* INTERNAL_DECODER_AAC */

#if defined(REGISTER_FILTER) | INTERNAL_DECODER_PCM
HRESULT CMpaDecFilter::ProcessPCMraw() //'raw '
{
	WAVEFORMATEX* wfe = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();
	int nSamples = m_buff.GetCount() * 8 / wfe->wBitsPerSample;

	CAtlArray<float> pBuff;
	pBuff.SetCount(nSamples);
	float* f = pBuff.GetData();

	switch(wfe->wBitsPerSample) {
		case 8: { //unsigned 8-bit
			BYTE* b = m_buff.GetData();
			for(int i = 0; i < nSamples; i++) {
				f[i] = (float)(CHAR)(b[i] - 128) / 128;
			}
		}
		break;
		case 16: { //signed big-endian 16 bit
			USHORT* d = (USHORT*)m_buff.GetData();//signed take as an unsigned to shift operations.
			for(int i = 0; i < nSamples; i++) {
				f[i] = (float)(SHORT)(d[i] << 8 | d[i] >> 8) / 32768;
			}
		}
		break;
	}

	HRESULT hr;
	if(S_OK != (hr = Deliver(pBuff, wfe->nSamplesPerSec, wfe->nChannels))) {
		return hr;
	}

	m_buff.RemoveAll();
	return S_OK;
}

HRESULT CMpaDecFilter::ProcessPCMintBE() //'twos', big-endian 'in24' and 'in32'
{
	WAVEFORMATEX* wfe = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();
	int nSamples = m_buff.GetCount() * 8 / wfe->wBitsPerSample;

	CAtlArray<float> pBuff;
	pBuff.SetCount(nSamples);
	float* f = pBuff.GetData();

	switch(wfe->wBitsPerSample) {
		case 8: { //signed 8-bit
			CHAR* b = (CHAR*)m_buff.GetData();
			for(int i = 0; i < nSamples; i++) {
				f[i] = (float)b[i] / 128;
			}
		}
		break;
		case 16: { //signed big-endian 16-bit
			USHORT* d = (USHORT*)m_buff.GetData();//signed take as an unsigned to shift operations.
			for(int i = 0; i < nSamples; i++) {
				f[i] = (float)(SHORT)(d[i] << 8 | d[i] >> 8) / 32768;
			}
		}
		break;
		case 24: { //signed big-endian 24-bit
			BYTE* b = (BYTE*)m_buff.GetData();
			for(int i = 0; i < nSamples; i++) {
				f[i] = (float)(signed int)((unsigned int)b[3*i] << 24 |
										   (unsigned int)b[3*i+1] << 16 |
										   (unsigned int)b[3*i+2] << 8) / 2147483648;
			}
		}
		break;
		case 32: { //signed big-endian 32-bit
			UINT* q = (UINT*)m_buff.GetData();//signed take as an unsigned to shift operations.
			for(int i = 0; i < nSamples; i++) {
				f[i] = (float)(INT)(q[i] >> 24 |
									(q[i] & 0x00ff0000) >> 8 |
									(q[i] & 0x0000ff00) << 8 |
									q[i] << 24) / 2147483648;
			}
		}
		break;
	}

	HRESULT hr;
	if(S_OK != (hr = Deliver(pBuff, wfe->nSamplesPerSec, wfe->nChannels))) {
		return hr;
	}

	m_buff.RemoveAll();
	return S_OK;
}

HRESULT CMpaDecFilter::ProcessPCMintLE() //'sowt', little-endian 'in24' and 'in32'
{
	WAVEFORMATEX* wfe = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();
	int nSamples = m_buff.GetCount() * 8 / wfe->wBitsPerSample;

	CAtlArray<float> pBuff;
	pBuff.SetCount(nSamples);
	float* f = pBuff.GetData();

	switch(wfe->wBitsPerSample) {
		case 8: { //signed 8-bit
			CHAR* b = (CHAR*)m_buff.GetData();
			for(int i = 0; i < nSamples; i++) {
				f[i] = (float)b[i] / 128;
			}
		}
		break;
		case 16: { //signed little-endian 16-bit
			SHORT* d = (SHORT*)m_buff.GetData();
			for(int i = 0; i < nSamples; i++) {
				f[i] = (float)d[i] / 32768;
			}
		}
		break;
		case 24: { //signed little-endian 32-bit
			BYTE* b = (BYTE*)m_buff.GetData();
			for(int i = 0; i < nSamples; i++) {
				f[i] = (float)(signed int)((unsigned int)b[3*i] << 8 |
										   (unsigned int)b[3*i+1] << 16 |
										   (unsigned int)b[3*i+2] << 24) / 2147483648;
			}
		}
		break;
		case 32: { //signed little-endian 32-bit
			INT* q = (INT*)m_buff.GetData();
			for(int i = 0; i < nSamples; i++) {
				f[i] = (float)q[i] / 2147483648;
			}
		}
		break;
	}

	HRESULT hr;
	if(S_OK != (hr = Deliver(pBuff, wfe->nSamplesPerSec, wfe->nChannels))) {
		return hr;
	}

	m_buff.RemoveAll();
	return S_OK;
}

HRESULT CMpaDecFilter::ProcessPCMfloatBE() //big-endian 'fl32' and 'fl64'
{
	WAVEFORMATEX* wfe = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();
	int nSamples = m_buff.GetCount() * 8 / wfe->wBitsPerSample;

	CAtlArray<float> pBuff;
	pBuff.SetCount(nSamples);
	float* f = pBuff.GetData();

	switch(wfe->wBitsPerSample) {
		case 32: {
			unsigned int* q = (unsigned int*)m_buff.GetData();
			unsigned int* vf = (unsigned int*)f;
			for(int i = 0; i < nSamples; i++) {
				vf[i] = q[i] >> 24 |
						(q[i] & 0x00ff0000) >> 8 |
						(q[i] & 0x0000ff00) << 8 |
						q[i] << 24;
			}
		}
		break;
		case 64: {
			unsigned __int64* q = (unsigned __int64*)m_buff.GetData();
			unsigned __int64 x;
			for(int i = 0; i < nSamples; i++) {
				x =	q[i] >>56 |
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
	if(S_OK != (hr = Deliver(pBuff, wfe->nSamplesPerSec, wfe->nChannels))) {
		return hr;
	}

	m_buff.RemoveAll();
	return S_OK;
}

HRESULT CMpaDecFilter::ProcessPCMfloatLE() //little-endian 'fl32' and 'fl64'
{
	WAVEFORMATEX* wfe = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();
	int nSamples = m_buff.GetCount() * 8 / wfe->wBitsPerSample;

	CAtlArray<float> pBuff;
	pBuff.SetCount(nSamples);
	float* f = pBuff.GetData();

	switch(wfe->wBitsPerSample) {
		case 32: {
			float* q = (float*)m_buff.GetData();
			for(int i = 0; i < nSamples; i++) {
				f[i] = q[i];
			}
		}
		break;
		case 64: {
			double* q = (double*)m_buff.GetData();
			for(int i = 0; i < nSamples; i++) {
				f[i] = (float)q[i];
			}
		}
		break;
	}

	HRESULT hr;
	if(S_OK != (hr = Deliver(pBuff, wfe->nSamplesPerSec, wfe->nChannels))) {
		return hr;
	}

	m_buff.RemoveAll();
	return S_OK;
}
#endif /* INTERNAL_DECODER_PCM */

#if defined(REGISTER_FILTER) | INTERNAL_DECODER_PS2AUDIO
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

	while(end - p >= size) {
		DWORD* dw = (DWORD*)p;

		if(dw[0] == 'dhSS') {
			p += dw[1] + 8;
		} else if(dw[0] == 'dbSS') {
			p += 8;
			m_ps2_state.sync = true;
		} else {
			if(m_ps2_state.sync) {
				short* s = (short*)p;

				for(int i = 0; i < samples; i++)
					for(int j = 0; j < channels; j++) {
						f[i*channels+j] = (float)s[j*samples+i] / 32768;
					}
			} else {
				for(int i = 0, j = samples*channels; i < j; i++) {
					f[i] = 0;
				}
			}

			HRESULT hr;
			if(S_OK != (hr = Deliver(pBuff, wfe->nSamplesPerSec, wfe->nChannels))) {
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
	UNUSED_ALWAYS(unk);

	if(tbl_index >= 10) {
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

	for(int i = 0; i < 28; i++) {
		short input = (short)(((pin[2+i/2] >> ((i&1) << 2)) & 0xf) << 12) >> shift;
		double output = a * tbl[1] + b * tbl[0] + input;

		a = b;
		b = output;

		*pout++ = output / SHRT_MAX;
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

	while(end - p >= size) {
		DWORD* dw = (DWORD*)p;

		if(dw[0] == 'dhSS') {
			p += dw[1] + 8;
		} else if(dw[0] == 'dbSS') {
			p += 8;
			m_ps2_state.sync = true;
		} else {
			if(m_ps2_state.sync) {
				double* tmp = DNew double[samples*channels];

				for(int channel = 0, j = 0, k = 0; channel < channels; channel++, j += wfe->dwInterleave)
					for(int i = 0; i < wfe->dwInterleave; i += 16, k += 28) {
						decodeps2adpcm(m_ps2_state, channel, p + i + j, tmp + k);
					}

				for(int i = 0, k = 0; i < samples; i++)
					for(int j = 0; j < channels; j++, k++) {
						f[k] = (float)tmp[j*samples+i];
					}

				delete [] tmp;
			} else {
				for(int i = 0, j = samples*channels; i < j; i++) {
					f[i] = 0;
				}
			}

			HRESULT hr;
			if(S_OK != (hr = Deliver(pBuff, wfe->nSamplesPerSec, wfe->nChannels))) {
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

#if defined(REGISTER_FILTER) | INTERNAL_DECODER_VORBIS
HRESULT CMpaDecFilter::ProcessVorbis()
{
	if(m_vorbis.vi.channels < 1 || m_vorbis.vi.channels > 6) {
		return E_FAIL;
	}

	if(m_buff.IsEmpty()) {
		return S_OK;
	}

	HRESULT hr = S_OK;

	ogg_packet op;
	memset(&op, 0, sizeof(op));
	op.packet = m_buff.GetData();
	op.bytes = m_buff.GetCount();
	op.b_o_s = 0;
	op.packetno = m_vorbis.packetno++;

	if(vorbis_synthesis(&m_vorbis.vb, &op) == 0) {
		vorbis_synthesis_blockin(&m_vorbis.vd, &m_vorbis.vb);

		int samples;
		float** pcm;

		while((samples = vorbis_synthesis_pcmout(&m_vorbis.vd, &pcm)) > 0) {
			const scmap_t& scmap = s_scmap_vorbis[m_vorbis.vi.channels-1];

			CAtlArray<float> pBuff;
			pBuff.SetCount(samples * scmap.nChannels);
			float* dst = pBuff.GetData();

			for(int j = 0, ch = scmap.nChannels; j < ch; j++) {
				float* src = pcm[scmap.ch[j]];
				for(int i = 0; i < samples; i++) {
					dst[j + i*ch] = src[i];
				}
				//	dst[j + i*ch] = (float)max(min(src[i], 1<<24), -1<<24) / (1<<24);
			}

			if(S_OK != (hr = Deliver(pBuff, m_vorbis.vi.rate, scmap.nChannels, scmap.dwChannelMask))) {
				break;
			}

			vorbis_synthesis_read(&m_vorbis.vd, samples);
		}
	}

	m_buff.RemoveAll();

	return hr;
}
#endif /* INTERNAL_DECODER_VORBIS */

static inline float fscale(mad_fixed_t sample)
{
	if(sample >= MAD_F_ONE) {
		sample = MAD_F_ONE - 1;
	} else if(sample < -MAD_F_ONE) {
		sample = -MAD_F_ONE;
	}

	return (float)sample / (1 << MAD_F_FRACBITS);
}

#if defined(REGISTER_FILTER) | INTERNAL_DECODER_FLAC
HRESULT CMpaDecFilter::ProcessFlac()
{
	WAVEFORMATEX* wfein = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();
	UNUSED_ALWAYS(wfein);

	FLAC__stream_decoder_process_single ((FLAC__StreamDecoder*) m_flac.pDecoder);
	return m_flac.hr;
}
#endif /* INTERNAL_DECODER_FLAC */

#if defined(REGISTER_FILTER) | INTERNAL_DECODER_MPEGAUDIO
HRESULT CMpaDecFilter::ProcessMPA()
{
	mad_stream_buffer(&m_stream, m_buff.GetData(), m_buff.GetCount());

	while(1) {
		if(mad_frame_decode(&m_frame, &m_stream) == -1) {
			if(m_stream.error == MAD_ERROR_BUFLEN) {
				memmove(m_buff.GetData(), m_stream.this_frame, m_stream.bufend - m_stream.this_frame);
				m_buff.SetCount(m_stream.bufend - m_stream.this_frame);
				break;
			}

			if( m_stream.error == MAD_ERROR_BADDATAPTR) {
				TRACE(_T("MAD MAD_ERROR_BADDATAPTR\n"));
				continue;
			}


			if(!MAD_RECOVERABLE(m_stream.error)) {
				TRACE(_T("*m_stream.error == %d\n"), m_stream.error);
				return E_FAIL;
			}

			// FIXME: the renderer doesn't like this
			// m_fDiscontinuity = true;

			continue;
		}
		/*
		// TODO: needs to be tested... (has anybody got an external mpeg audio decoder?)
		HRESULT hr;
		if(S_OK != (hr = Deliver(
		   (BYTE*)m_stream.this_frame,
		   m_stream.next_frame - m_stream.this_frame,
		   m_frame.header.bitrate,
		   m_frame.header.layer == 1 ? 0x0004 : 0x0005)))
			return hr;
		continue;
		*/
		mad_synth_frame(&m_synth, &m_frame);

		WAVEFORMATEX* wfein = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();
		if(wfein->nChannels != m_synth.pcm.channels || wfein->nSamplesPerSec != m_synth.pcm.samplerate) {
			TRACE(_T("MAD channels %d %d samplerate %d %d \n"),wfein->nChannels , m_synth.pcm.channels, wfein->nSamplesPerSec , m_synth.pcm.samplerate);
			//Some time this does happened - need more testing ...
			//continue;
		}

		const mad_fixed_t* left_ch   = m_synth.pcm.samples[0];
		const mad_fixed_t* right_ch  = m_synth.pcm.samples[1];

		CAtlArray<float> pBuff;
		pBuff.SetCount(m_synth.pcm.length*m_synth.pcm.channels);

		float* pDataOut = pBuff.GetData();
		for(unsigned short i = 0; i < m_synth.pcm.length; i++) {
			*pDataOut++ = fscale(*left_ch++);
			if(m_synth.pcm.channels == 2) {
				*pDataOut++ = fscale(*right_ch++);
			}
		}

		HRESULT hr;
		if(S_OK != (hr = Deliver(pBuff, m_synth.pcm.samplerate, m_synth.pcm.channels))) {
			return hr;
		}
	}

	return S_OK;
}
#endif /* INTERNAL_DECODER_MPEGAUDIO */

HRESULT CMpaDecFilter::GetDeliveryBuffer(IMediaSample** pSample, BYTE** pData)
{
	HRESULT hr;

	*pData = NULL;
	if(FAILED(hr = m_pOutput->GetDeliveryBuffer(pSample, NULL, NULL, 0))
			|| FAILED(hr = (*pSample)->GetPointer(pData))) {
		return hr;
	}

	AM_MEDIA_TYPE* pmt = NULL;
	if(SUCCEEDED((*pSample)->GetMediaType(&pmt)) && pmt) {
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

	if(FAILED(hr = ReconnectOutput(nSamples, mt))) {
		return hr;
	}

	CComPtr<IMediaSample> pOut;
	BYTE* pDataOut = NULL;
	if(FAILED(GetDeliveryBuffer(&pOut, &pDataOut))) {
		return E_FAIL;
	}

	REFERENCE_TIME rtDur = 10000000i64*nSamples/wfe->nSamplesPerSec;
	REFERENCE_TIME rtStart = m_rtStart, rtStop = m_rtStart + rtDur;
	m_rtStart += rtDur;
	//TRACE(_T("CMpaDecFilter: %I64d - %I64d\n"), rtStart/10000, rtStop/10000);
	if(rtStart < 0 /*200000*/ /* < 0, FIXME: 0 makes strange noises */) {
		return S_OK;
	}

	if(hr == S_OK) {
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

	// TODO: move this into the audio switcher
	float sample_mul = 1;
	if(m_fNormalize) {
		for(int i = 0, len = pBuff.GetCount(); i < len; i++) {
			float f = *pDataIn++;
			if(f < 0) {
				f = -f;
			}
			if(m_sample_max < f) {
				m_sample_max = f;
			}
		}
		sample_mul = 1.0f / m_sample_max;
		pDataIn = pBuff.GetData();
	}

	bool fBoost = m_boost > 1;
	double boost = 1+log10(m_boost);

	for(int i = 0, len = pBuff.GetCount(); i < len; i++) {
		float f = *pDataIn++;

		// TODO: move this into the audio switcher

		if(m_fNormalize) {
			f *= sample_mul;
		}

		if(fBoost) {
			f *= boost;
		}

		if(f < -1) {
			f = -1;
		} else if(f > 1) {
			f = 1;
		}

#define round(x) ((x) > 0 ? (x) + 0.5 : (x) - 0.5)

		switch(sf) {
			default:
			case SF_PCM16:
				*(short*)pDataOut = (short)round(f * SHRT_MAX);
				pDataOut += sizeof(short);
				break;
			case SF_PCM24: {
				DWORD i24 = (DWORD)(int)round(f * ((1<<23)-1));
				*pDataOut++ = (BYTE)(i24);
				*pDataOut++ = (BYTE)(i24>>8);
				*pDataOut++ = (BYTE)(i24>>16);
			}
			break;
			case SF_PCM32:
				*(int*)pDataOut = (int)round((double)f * INT_MAX);
				pDataOut += sizeof(int);
				break;
			case SF_FLOAT32:
				*(float*)pDataOut = f;
				pDataOut += sizeof(float);
				break;
		}
	}

	return m_pOutput->Deliver(pOut);
}

HRESULT CMpaDecFilter::Deliver(BYTE* pBuff, int size, int bit_rate, BYTE type)
{
	HRESULT hr;
	bool padded = false;

	CMediaType mt = CreateMediaTypeSPDIF();
	WAVEFORMATEX* wfe = (WAVEFORMATEX*)mt.Format();

	int length = 0;
	while(length < size+sizeof(WORD)*4) {
		length += 0x800;
	}
	int size2 = 1i64 * wfe->nBlockAlign * wfe->nSamplesPerSec * size*8 / bit_rate;
	while(length < size2) {
		length += 0x800;
	}
	if(length > size2) {
		padded = true;
	}

	if(FAILED(hr = ReconnectOutput(length / wfe->nBlockAlign, mt))) {
		return hr;
	}

	CComPtr<IMediaSample> pOut;
	BYTE* pDataOut = NULL;
	if(FAILED(GetDeliveryBuffer(&pOut, &pDataOut))) {
		return E_FAIL;
	}

	WORD* pDataOutW = (WORD*)pDataOut;
	pDataOutW[0] = 0xf872;
	pDataOutW[1] = 0x4e1f;
	pDataOutW[2] = type;

	REFERENCE_TIME rtDur;

	if(!padded) {
		pDataOutW[3] = size*8;
		_swab((char*)pBuff, (char*)&pDataOutW[4], size);
	} else {
		pDataOutW[3] = length*8;
		_swab((char*)pBuff, (char*)&pDataOutW[4], length);
	}
	rtDur = 10000000i64 * size*8 / bit_rate;
	REFERENCE_TIME rtStart = m_rtStart, rtStop = m_rtStart + rtDur;
	m_rtStart += rtDur;

	if(rtStart < 0) {
		return S_OK;
	}

	if(hr == S_OK) {
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
	if(!pPin) {
		return E_NOINTERFACE;
	}

	CComPtr<IMemAllocator> pAllocator;
	if(FAILED(hr = pPin->GetAllocator(&pAllocator)) || !pAllocator) {
		return hr;
	}

	ALLOCATOR_PROPERTIES props, actual;
	if(FAILED(hr = pAllocator->GetProperties(&props))) {
		return hr;
	}

	WAVEFORMATEX* wfe = (WAVEFORMATEX*)mt.Format();
	long cbBuffer = nSamples * wfe->nBlockAlign;

	if(mt != m_pOutput->CurrentMediaType() || cbBuffer > props.cbBuffer) {
		if(cbBuffer > props.cbBuffer) {
			props.cBuffers = 4;
			props.cbBuffer = cbBuffer*3/2;

			if(FAILED(hr = m_pOutput->DeliverBeginFlush())
					|| FAILED(hr = m_pOutput->DeliverEndFlush())
					|| FAILED(hr = pAllocator->Decommit())
					|| FAILED(hr = pAllocator->SetProperties(&props, &actual))
					|| FAILED(hr = pAllocator->Commit())) {
				return hr;
			}

			if(props.cBuffers > actual.cBuffers || props.cbBuffer > actual.cbBuffer) {
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
	switch(sf) {
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
	if(dwChannelMask == 0 && (sf == SF_PCM24 || sf == SF_PCM32)) {
		dwChannelMask = nChannels == 2 ? (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT) : SPEAKER_FRONT_CENTER;
	}

	if(dwChannelMask) {
		wfex.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		wfex.Format.cbSize = sizeof(wfex) - sizeof(wfex.Format);
		wfex.dwChannelMask = dwChannelMask;
		wfex.Samples.wValidBitsPerSample = wfex.Format.wBitsPerSample;
		wfex.SubFormat = mt.subtype;
	}

	mt.SetFormat((BYTE*)&wfex, sizeof(wfex.Format) + wfex.Format.cbSize);

	return mt;
}

CMediaType CMpaDecFilter::CreateMediaTypeSPDIF()
{
	CMediaType mt = CreateMediaType(SF_PCM16, 48000, 2);
	((WAVEFORMATEX*)mt.pbFormat)->wFormatTag = WAVE_FORMAT_DOLBY_AC3_SPDIF;
	return mt;
}

HRESULT CMpaDecFilter::CheckInputType(const CMediaType* mtIn)
{
#if defined(REGISTER_FILTER) | INTERNAL_DECODER_LPCM
	if(mtIn->subtype == MEDIASUBTYPE_DVD_LPCM_AUDIO) {
		WAVEFORMATEX* wfe = (WAVEFORMATEX*)mtIn->Format();
		if(wfe->nChannels < 1 || wfe->nChannels > 8 || (wfe->wBitsPerSample != 16 && wfe->wBitsPerSample != 20 && wfe->wBitsPerSample != 24)) {
			return VFW_E_TYPE_NOT_ACCEPTED;
		}
	} else if(mtIn->subtype == MEDIASUBTYPE_HDMV_LPCM_AUDIO) {
		WAVEFORMATEX* wfe = (WAVEFORMATEX*)mtIn->Format();
		UNUSED_ALWAYS(wfe);
		return S_OK;
	}
#else
	if(0) {}
#endif
#if defined(REGISTER_FILTER) | INTERNAL_DECODER_PS2AUD
	else if(mtIn->subtype == MEDIASUBTYPE_PS2_ADPCM) {
		WAVEFORMATEXPS2* wfe = (WAVEFORMATEXPS2*)mtIn->Format();
		UNUSED_ALWAYS(wfe);
		if(wfe->dwInterleave & 0xf) { // has to be a multiple of the block size (16 bytes)
			return VFW_E_TYPE_NOT_ACCEPTED;
		}
	}
#endif
#if defined(REGISTER_FILTER) | INTERNAL_DECODER_VORBIS
	else if(mtIn->subtype == MEDIASUBTYPE_Vorbis2) {
		if(!m_vorbis.init(*mtIn)) {
			return VFW_E_TYPE_NOT_ACCEPTED;
		}
	}
#endif
#if defined(REGISTER_FILTER) | INTERNAL_DECODER_FLAC
	else if(mtIn->subtype == MEDIASUBTYPE_FLAC_FRAMED) {
		return S_OK;
	}
#endif
#if defined(REGISTER_FILTER) | INTERNAL_DECODER_NELLYMOSER
	else if(mtIn->subtype == MEDIASUBTYPE_NELLYMOSER) {
		return S_OK;
	}
#endif
#if defined(REGISTER_FILTER) | INTERNAL_DECODER_AAC
	else if(mtIn->subtype == MEDIASUBTYPE_AAC) {
		// Reject invalid AAC stream on connection
		if (!m_aac_state.init(*mtIn)) {
			return VFW_E_TYPE_NOT_ACCEPTED;
		}
	}
#endif

	for(int i = 0; i < countof(sudPinTypesIn); i++) {
		if(*sudPinTypesIn[i].clsMajorType == mtIn->majortype
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
	if(m_pInput->IsConnected() == FALSE) {
		return E_UNEXPECTED;
	}

	CMediaType& mt = m_pInput->CurrentMediaType();
	WAVEFORMATEX* wfe = (WAVEFORMATEX*)mt.Format();
	UNUSED_ALWAYS(wfe);

	pProperties->cBuffers = 4;
	// pProperties->cbBuffer = 1;
	pProperties->cbBuffer = 48000*6*(32/8)/10; // 48KHz 6ch 32bps 100ms
	pProperties->cbAlign = 1;
	pProperties->cbPrefix = 0;

	HRESULT hr;
	ALLOCATOR_PROPERTIES Actual;
	if(FAILED(hr = pAllocator->SetProperties(pProperties, &Actual))) {
		return hr;
	}

	return pProperties->cBuffers > Actual.cBuffers || pProperties->cbBuffer > Actual.cbBuffer
		   ? E_FAIL
		   : NOERROR;
}

HRESULT CMpaDecFilter::GetMediaType(int iPosition, CMediaType* pmt)
{
	if(m_pInput->IsConnected() == FALSE) {
		return E_UNEXPECTED;
	}

	if(iPosition < 0) {
		return E_INVALIDARG;
	}
	if(iPosition > 0) {
		return VFW_S_NO_MORE_ITEMS;
	}

	CMediaType mt = m_pInput->CurrentMediaType();
	const GUID& subtype = mt.subtype;
	WAVEFORMATEX* wfe = (WAVEFORMATEX*)mt.Format();
	if (wfe == NULL) {
		return E_INVALIDARG;
	}

#if defined(REGISTER_FILTER) | INTERNAL_DECODER_AC3 | INTERNAL_DECODER_DTS
	if(GetSpeakerConfig(ac3) < 0 && (subtype == MEDIASUBTYPE_DOLBY_AC3 ||
									 subtype == MEDIASUBTYPE_WAVE_DOLBY_AC3 ||
									 subtype == MEDIASUBTYPE_DOLBY_DDPLUS ||
									 subtype == MEDIASUBTYPE_DOLBY_TRUEHD)
			|| GetSpeakerConfig(dts) < 0 && (subtype == MEDIASUBTYPE_DTS || subtype == MEDIASUBTYPE_WAVE_DTS)) {
		*pmt = CreateMediaTypeSPDIF();
	}
#else
	if(0) {}
#endif
#if defined(REGISTER_FILTER) | INTERNAL_DECODER_VORBIS
	else if(subtype == MEDIASUBTYPE_Vorbis2) {
		*pmt = CreateMediaType(GetSampleFormat(), m_vorbis.vi.rate, m_vorbis.vi.channels);
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
	if(FAILED(hr)) {
		return hr;
	}

#if defined(REGISTER_FILTER) | INTERNAL_DECODER_AC3
	m_a52_state = a52_init(0);
#endif

#if defined(REGISTER_FILTER) | INTERNAL_DECODER_DTS
	m_dts_state = dts_init(0);
#endif

#if defined(REGISTER_FILTER) | INTERNAL_DECODER_AAC
	m_aac_state.init(m_pInput->CurrentMediaType());
#endif

#if defined(REGISTER_FILTER) | INTERNAL_DECODER_MPEGAUDIO
	mad_stream_init(&m_stream);
	mad_frame_init(&m_frame);
	mad_synth_init(&m_synth);
	mad_stream_options(&m_stream, 0/*options*/);
#endif

	m_ps2_state.reset();
#if defined(REGISTER_FILTER) | INTERNAL_DECODER_FLAC
	FlacInitDecoder();
#endif

	m_fDiscontinuity = false;

	m_sample_max = 0.1f;

	return S_OK;
}

HRESULT CMpaDecFilter::StopStreaming()
{
#if defined(REGISTER_FILTER) | INTERNAL_DECODER_AC3
	a52_free(m_a52_state);
#endif

#if defined(REGISTER_FILTER) | INTERNAL_DECODER_DTS
	dts_free(m_dts_state);
#endif

#if defined(REGISTER_FILTER) | INTERNAL_DECODER_MPEGAUDIO
	mad_synth_finish(&m_synth);
	mad_frame_finish(&m_frame);
	mad_stream_finish(&m_stream);
#endif
#if defined(REGISTER_FILTER) | INTERNAL_DECODER_FLAC
	flac_stream_finish();
#endif
#if defined(REGISTER_FILTER) | HAS_FFMPEG_AUDIO_DECODERS
	ffmpeg_stream_finish();
#endif

	return __super::StopStreaming();
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

STDMETHODIMP CMpaDecFilter::SetNormalize(bool fNormalize)
{
	CAutoLock cAutoLock(&m_csProps);
	if(m_fNormalize != fNormalize) {
		m_sample_max = 0.1f;
	}
	m_fNormalize = fNormalize;
	return S_OK;
}

STDMETHODIMP_(bool) CMpaDecFilter::GetNormalize()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_fNormalize;
}

STDMETHODIMP CMpaDecFilter::SetSpeakerConfig(enctype et, int sc)
{
	CAutoLock cAutoLock(&m_csProps);
	if(et >= 0 && et < etlast) {
		m_iSpeakerConfig[et] = sc;
	}
	return S_OK;
}

STDMETHODIMP_(int) CMpaDecFilter::GetSpeakerConfig(enctype et)
{
	CAutoLock cAutoLock(&m_csProps);
	if(et >= 0 && et < etlast) {
		return m_iSpeakerConfig[et];
	}
	return -1;
}

STDMETHODIMP CMpaDecFilter::SetDynamicRangeControl(enctype et, bool fDRC)
{
	CAutoLock cAutoLock(&m_csProps);
	if(et >= 0 && et < etlast) {
		m_fDynamicRangeControl[et] = fDRC;
	} else {
		return E_INVALIDARG;
	}
	return S_OK;
}

STDMETHODIMP_(bool) CMpaDecFilter::GetDynamicRangeControl(enctype et)
{
	CAutoLock cAutoLock(&m_csProps);
	if(et >= 0 && et < etlast) {
		return m_fDynamicRangeControl[et];
	}
	return false;
}

STDMETHODIMP CMpaDecFilter::SetBoost(float boost)
{
	CAutoLock cAutoLock(&m_csProps);
	m_boost = max(boost, 1);
	return S_OK;
}

STDMETHODIMP_(float) CMpaDecFilter::GetBoost()
{
	CAutoLock cAutoLock(&m_csProps);
	return m_boost;
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
	if(ERROR_SUCCESS == key.Create(HKEY_CURRENT_USER, _T("Software\\Gabest\\Filters\\MPEG Audio Decoder"))) {
		key.SetDWORDValue(_T("SampleFormat"), m_iSampleFormat);
		key.SetDWORDValue(_T("Normalize"), m_fNormalize);
		key.SetDWORDValue(_T("Boost"), *(DWORD*)&m_boost);
		key.SetDWORDValue(_T("Ac3SpeakerConfig"), m_iSpeakerConfig[ac3]);
		key.SetDWORDValue(_T("DtsSpeakerConfig"), m_iSpeakerConfig[dts]);
		key.SetDWORDValue(_T("AacSpeakerConfig"), m_iSpeakerConfig[aac]);
		key.SetDWORDValue(_T("Ac3DynamicRangeControl"), m_fDynamicRangeControl[ac3]);
		key.SetDWORDValue(_T("DtsDynamicRangeControl"), m_fDynamicRangeControl[dts]);
		key.SetDWORDValue(_T("AacDynamicRangeControl"), m_fDynamicRangeControl[aac]);
	}
#else
	AfxGetApp()->WriteProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("SampleFormat"), m_iSampleFormat);
	AfxGetApp()->WriteProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("Normalize"), m_fNormalize);
	AfxGetApp()->WriteProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("Boost"), *(DWORD*)&m_boost);
	AfxGetApp()->WriteProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("Ac3SpeakerConfig"), m_iSpeakerConfig[ac3]);
	AfxGetApp()->WriteProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("DtsSpeakerConfig"), m_iSpeakerConfig[dts]);
	AfxGetApp()->WriteProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("AacSpeakerConfig"), m_iSpeakerConfig[aac]);
	AfxGetApp()->WriteProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("Ac3DynamicRangeControl"), m_fDynamicRangeControl[ac3]);
	AfxGetApp()->WriteProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("DtsDynamicRangeControl"), m_fDynamicRangeControl[dts]);
	AfxGetApp()->WriteProfileInt(_T("Filters\\MPEG Audio Decoder"), _T("AacDynamicRangeControl"), m_fDynamicRangeControl[aac]);
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

	if(*ppPage != NULL) {
		return E_INVALIDARG;
	}

	HRESULT hr;

	if(guid == __uuidof(CMpaDecSettingsWnd)) {
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

#if defined(REGISTER_FILTER) | INTERNAL_DECODER_AAC
//
// aac_state_t
//

aac_state_t::aac_state_t() : h(NULL), freq(0), channels(0)
{
	open();
}
aac_state_t::~aac_state_t()
{
	close();
}

bool aac_state_t::open()
{
	close();
	h = NeAACDecOpen();
	if(!h) {
		return false;
	}
	NeAACDecConfigurationPtr c = NeAACDecGetCurrentConfiguration(h);
	c->outputFormat = FAAD_FMT_FLOAT;
	NeAACDecSetConfiguration(h, c);
	return true;
}

void aac_state_t::close()
{
	if(h) {
		NeAACDecClose(h);
	}
	h = NULL;
}

bool aac_state_t::init(const CMediaType& mt)
{
	if(mt.subtype != MEDIASUBTYPE_AAC
			&& mt.subtype != MEDIASUBTYPE_MP4A
			&& mt.subtype != MEDIASUBTYPE_mp4a) {
		return true;    // nothing to do
	}

	open();
	const WAVEFORMATEX* wfe = (WAVEFORMATEX*)mt.Format();
	return !NeAACDecInit2(h, (BYTE*)(wfe+1), wfe->cbSize, &freq, &channels);
}
#endif /* INTERNAL_DECODER_AAC */

#if defined(REGISTER_FILTER) | INTERNAL_DECODER_VORBIS
//
// vorbis_state_t
//

vorbis_state_t::vorbis_state_t()
{
	memset(&vd, 0, sizeof(vd));
	memset(&vb, 0, sizeof(vb));
	memset(&vc, 0, sizeof(vc));
	memset(&vi, 0, sizeof(vi));
}

vorbis_state_t::~vorbis_state_t()
{
	clear();
}

void vorbis_state_t::clear()
{
	vorbis_block_clear(&vb);
	vorbis_dsp_clear(&vd);
	vorbis_comment_clear(&vc);
	vorbis_info_clear(&vi);
}

bool vorbis_state_t::init(const CMediaType& mt)
{
	if(mt.subtype != MEDIASUBTYPE_Vorbis2) {
		return true;    // nothing to do
	}

	clear();

	vorbis_info_init(&vi);
	vorbis_comment_init(&vc);

	VORBISFORMAT2* vf = (VORBISFORMAT2*)mt.Format();
	BYTE* fmt = mt.Format();

	packetno = 0;

	memset(&op, 0, sizeof(op));
	op.packet = (fmt += sizeof(*vf));
	op.bytes = vf->HeaderSize[0];
	op.b_o_s = 1;
	op.packetno = packetno++;

	if(vorbis_synthesis_headerin(&vi, &vc, &op) < 0) {
		return false;
	}

	memset(&op, 0, sizeof(op));
	op.packet = (fmt += vf->HeaderSize[0]);
	op.bytes = vf->HeaderSize[1];
	op.b_o_s = 0;
	op.packetno = packetno++;

	if(vorbis_synthesis_headerin(&vi, &vc, &op) < 0) {
		return false;
	}

	memset(&op, 0, sizeof(op));
	op.packet = (fmt += vf->HeaderSize[1]);
	op.bytes = vf->HeaderSize[2];
	op.b_o_s = 0;
	op.packetno = packetno++;

	if(vorbis_synthesis_headerin(&vi, &vc, &op) < 0) {
		return false;
	}

	postgain = 1.0;

	if(vorbis_comment_query_count(&vc, "LWING_GAIN")) {
		postgain = atof(vorbis_comment_query(&vc, "LWING_GAIN", 0));
	}

	if(vorbis_comment_query_count(&vc, "POSTGAIN")) {
		postgain = atof(vorbis_comment_query(&vc, "POSTGAIN", 0));
	}

	if(vorbis_comment_query_count(&vc, "REPLAYGAIN_TRACK_GAIN")) {
		postgain = pow(10.0, atof(vorbis_comment_query(&vc, "REPLAYGAIN_TRACK_GAIN", 0)) / 20.0);
	}

	vorbis_synthesis_init(&vd, &vi);
	vorbis_block_init(&vd, &vb);

	return true;
}
#endif /* INTERNAL_DECODER_VORBIS */

#if defined(REGISTER_FILTER) | INTERNAL_DECODER_FLAC

#pragma region Flac callback

void CMpaDecFilter::FlacFillBuffer(BYTE buffer[], size_t *bytes)
{
	UINT			nSize = min (*bytes, m_buff.GetCount());

	if (nSize > 0) {
		memcpy_s (buffer, *bytes, m_buff.GetData(), nSize);
		memmove(m_buff.GetData(), m_buff.GetData() + nSize, m_buff.GetCount() - nSize);
		m_buff.SetCount(m_buff.GetCount() - nSize);

	}
	*bytes = nSize;
}

void CMpaDecFilter::FlacDeliverBuffer  (unsigned blocksize, const __int32 * const buffer[])
{
	WAVEFORMATEX*		wfein = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();
	CAtlArray<float>	pBuff;

	pBuff.SetCount (blocksize * wfein->nChannels);
	float*	pDataOut = pBuff.GetData();

	scmap_t& scmap = m_scmap_default[wfein->nChannels-1];

	switch (wfein->wBitsPerSample) {
		case 16 :
			for(unsigned i = 0; i < blocksize; i++) {
				for(int nChannel = 0; nChannel < wfein->nChannels; nChannel++) {
					FLAC__int16		nVal = (FLAC__int16)buffer[nChannel][i];
					*pDataOut = (float)nVal / SHRT_MAX;
					pDataOut++;
				}
			}
			break;
		case 20 :
		case 24 :
			for(unsigned i = 0; i < blocksize; i++) {
				for(int nChannel = 0; nChannel < wfein->nChannels; nChannel++) {
					FLAC__int32		nVal = (FLAC__int32)buffer[nChannel][i];
					*pDataOut = (float)nVal / INT24_MAX;
					pDataOut++;
				}
			}
			break;
	}

	m_flac.hr = Deliver(pBuff, wfein->nSamplesPerSec, wfein->nChannels, scmap.dwChannelMask);
}


static FLAC__StreamDecoderReadStatus StreamDecoderRead(const FLAC__StreamDecoder *decoder, FLAC__byte buffer[], size_t *bytes, void *client_data)
{
	CMpaDecFilter*	pThis = static_cast<CMpaDecFilter*> (client_data);

	pThis->FlacFillBuffer (buffer, bytes);

	return (*bytes == 0) ?  FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM : FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

static FLAC__StreamDecoderWriteStatus StreamDecoderWrite(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data)
{
	CMpaDecFilter*	pThis = static_cast<CMpaDecFilter*> (client_data);

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

#if defined(REGISTER_FILTER) | HAS_FFMPEG_AUDIO_DECODERS

#pragma region FFmpeg decoder

// Version 1 : using av_parser_parse !
#if 0
HRESULT CMpaDecFilter::DeliverFFmpeg(int nCodecId, BYTE* p, int buffsize, int& size)
{
	HRESULT		hr = S_OK;

	size = 0;
	if (!m_pAVCtx || nCodecId != m_pAVCtx->codec_id)
		if (!InitFFmpeg (nCodecId)) {
			return E_FAIL;
		}

	while (buffsize > 0) {
		BYTE*	pParserData;
		int		nParserLength	= AVCODEC_MAX_AUDIO_FRAME_SIZE;
		int		nPCMLength		= AVCODEC_MAX_AUDIO_FRAME_SIZE;
		int		nRet;

		if (m_pAVCtx->codec_id != CODEC_ID_MLP) {
			// Parse buffer
			nRet = av_parser_parse( m_pParser, m_pAVCtx, (uint8_t**)&pParserData, &nParserLength,
									(const uint8_t*)p, buffsize, AV_NOPTS_VALUE, AV_NOPTS_VALUE);
			if (nRet<0 || (nRet==0 && nParserLength==0)) {
				return S_OK;
			}

			buffsize	-= nRet;
			p			+= nRet;
			size		+= nRet;

			// Decode frame
			if (nParserLength > 0) {
				nRet = avcodec_decode_audio2(m_pAVCtx, (int16_t*)m_pPCMData, &nPCMLength, (const uint8_t*)pParserData, nParserLength);
				if (nRet<0 || (nRet==0 &&nPCMLength==0)) {
					continue;
				}
			} else {
				continue;
			}
		} else {
			// No parsing for MLP : decode only
			nRet = avcodec_decode_audio2(m_pAVCtx, (int16_t*)m_pPCMData, &nPCMLength, (const uint8_t*)p, buffsize);
			if (nRet<0 || (nRet==0 && nParserLength==0)) {
				return S_OK;
			}

			buffsize	-= nRet;
			p			+= nRet;
			size		+= nRet;
		}

		if (nPCMLength > 0) {
			WAVEFORMATEX*		wfein = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();
			CAtlArray<float>	pBuff;
			int					nRemap;
			float*				pDataOut;

			nRemap = FFGetChannelMap (m_pAVCtx);
			if (nRemap >=0) {
				scmap_t& scmap = s_scmap_ac3[nRemap];

				switch (m_pAVCtx->sample_fmt) {
					case SAMPLE_FMT_S16 :
						pBuff.SetCount (nPCMLength / 2);
						pDataOut = pBuff.GetData();

						for (int i=0; i<pBuff.GetCount()/m_pAVCtx->channels; i++) {
							for(int ch=0; ch<m_pAVCtx->channels; ch++) {
								*pDataOut = (float)((int16_t*)m_pPCMData) [scmap.ch[ch]+i*m_pAVCtx->channels] / SHRT_MAX;
								pDataOut++;
							}
						}
						break;

					case SAMPLE_FMT_S32 :
						pBuff.SetCount (nPCMLength / 4);
						pDataOut = pBuff.GetData();

						for (int i=0; i<pBuff.GetCount()/m_pAVCtx->channels; i++) {
							for(int ch=0; ch<m_pAVCtx->channels; ch++) {
								*pDataOut = (float)((int32_t*)m_pPCMData) [scmap.ch[ch]+i*m_pAVCtx->channels] / INT_MAX;
								pDataOut++;
							}
						}
						break;
					default :
						ASSERT(FALSE);
						break;
				}
				hr = Deliver(pBuff, m_pAVCtx->sample_rate, m_pAVCtx->channels, scmap.dwChannelMask);
			}
		}
	}

	return hr;
}

#else
FF_EXPORT void av_init_packet(AVPacket *pkt);

HRESULT CMpaDecFilter::DeliverFFmpeg(int nCodecId, BYTE* p, int buffsize, int& size)
{
	HRESULT		hr			= S_OK;
	int			nPCMLength	= 0;
	if (!m_pAVCtx || nCodecId != m_pAVCtx->codec_id)
		if (!InitFFmpeg (nCodecId)) {
			size = 0;
			return E_FAIL;
		}
	BYTE* pDataInBuff = p;
	CAtlArray<float>	pBuffOut;
	scmap_t* scmap = NULL;

	AVPacket avpkt;
	av_init_packet(&avpkt);

	while (buffsize > 0) {
		nPCMLength	= AVCODEC_MAX_AUDIO_FRAME_SIZE;
		if (buffsize+FF_INPUT_BUFFER_PADDING_SIZE > m_nFFBufferSize) {
			m_nFFBufferSize = buffsize+FF_INPUT_BUFFER_PADDING_SIZE;
			m_pFFBuffer		= (BYTE*)realloc(m_pFFBuffer, m_nFFBufferSize);

		}

		// Required number of additionally allocated bytes at the end of the input bitstream for decoding.
		// This is mainly needed because some optimized bitstream readers read
		// 32 or 64 bit at once and could read over the end.<br>
		// Note: If the first 23 bits of the additional bytes are not 0, then damaged
		// MPEG bitstreams could cause overread and segfault.
		memcpy(m_pFFBuffer, pDataInBuff, buffsize);
		memset(m_pFFBuffer+buffsize,0,FF_INPUT_BUFFER_PADDING_SIZE);

		avpkt.data = (uint8_t *)m_pFFBuffer;
		avpkt.size = buffsize;

		int used_byte = avcodec_decode_audio3(m_pAVCtx, (int16_t*)m_pPCMData, &nPCMLength, &avpkt);

		if(used_byte < 0 ) {
			size = used_byte;
			return S_OK;
		}
		if(used_byte == 0 && nPCMLength <= 0 ) {
			size = used_byte;
			return S_OK;
		}
		size += used_byte;//

		if (nPCMLength > 0) {
			//WAVEFORMATEX*		wfein = (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();
			CAtlArray<float>	pBuff;
			int					nRemap;
			float*				pDataOut;

			nRemap = FFGetChannelMap (m_pAVCtx);
			if (nRemap >=0) {


				switch (nCodecId) {
					case CODEC_ID_EAC3 :
						scmap = &m_ffmpeg_ac3[FFGetChannelMap(m_pAVCtx)];
						break;
					default :
						scmap = &m_scmap_default[m_pAVCtx->channels-1];
						break;
				}

				switch (m_pAVCtx->sample_fmt) {
					case SAMPLE_FMT_S16 :
						pBuff.SetCount (nPCMLength / 2);
						pDataOut = pBuff.GetData();

						for (size_t i=0; i<pBuff.GetCount()/m_pAVCtx->channels; i++) {
							for(int ch=0; ch<m_pAVCtx->channels; ch++) {
								*pDataOut = (float)((int16_t*)m_pPCMData) [scmap->ch[ch]+i*m_pAVCtx->channels] / SHRT_MAX;
								pDataOut++;
							}
						}
						break;

					case SAMPLE_FMT_S32 :
						pBuff.SetCount (nPCMLength / 4);
						pDataOut = pBuff.GetData();

						for (size_t i=0; i<pBuff.GetCount()/m_pAVCtx->channels; i++) {
							for(int ch=0; ch<m_pAVCtx->channels; ch++) {
								*pDataOut = (float)((int32_t*)m_pPCMData) [scmap->ch[ch]+i*m_pAVCtx->channels] / INT_MAX;
								pDataOut++;
							}
						}
						break;
					default :
						ASSERT(FALSE);
						break;
				}

				if(pBuff.GetCount() > 0) {
					int idx_start = pBuffOut.GetCount();
					pBuffOut.SetCount( idx_start + pBuff.GetCount()  );
					for(int i = 0; i< pBuff.GetCount(); i++) {
						pBuffOut[idx_start+i] = pBuff[i];
					}
				}

			}
		}

		buffsize	-= used_byte;
		pDataInBuff += used_byte;
	}
	if(pBuffOut.GetCount() > 0 && scmap) {
		hr = Deliver(pBuffOut, m_pAVCtx->sample_rate, scmap->nChannels, scmap->dwChannelMask);
	}
	return hr;
}
#endif

bool CMpaDecFilter::InitFFmpeg(int nCodecId)
{
	WAVEFORMATEX*	wfein	= (WAVEFORMATEX*)m_pInput->CurrentMediaType().Format();
	bool			bRet	= false;

	avcodec_init();
	avcodec_register_all();
#ifdef _DEBUG
	av_log_set_callback(LogLibAVCodec);
#endif

	if (m_pAVCodec) {
		ffmpeg_stream_finish();
	}

	m_pAVCodec					= avcodec_find_decoder((CodecID)nCodecId);
	if (m_pAVCodec) {
		if (nCodecId==CODEC_ID_AMR_NB || nCodecId== CODEC_ID_AMR_WB) {
			wfein->nChannels = 1;
			wfein->nSamplesPerSec = 8000;
		}

		m_pAVCtx						= avcodec_alloc_context();
		m_pAVCtx->sample_rate			= wfein->nSamplesPerSec;
		m_pAVCtx->channels				= wfein->nChannels;
		m_pAVCtx->bit_rate				= wfein->nAvgBytesPerSec*8;
		m_pAVCtx->bits_per_coded_sample	= wfein->wBitsPerSample;
		m_pAVCtx->block_align			= wfein->nBlockAlign;
		m_pAVCtx->flags				   |= CODEC_FLAG_TRUNCATED;

		m_pAVCtx->codec_id		= (CodecID)nCodecId;
		m_pParser				= av_parser_init(nCodecId);

		if (avcodec_open(m_pAVCtx,m_pAVCodec)>=0) {
			m_pPCMData	= (BYTE*)FF_aligned_malloc (AVCODEC_MAX_AUDIO_FRAME_SIZE+FF_INPUT_BUFFER_PADDING_SIZE, 64);
			bRet		= true;
		}
	}

	if (!bRet) {
		ffmpeg_stream_finish();
	}

	return bRet;
}

void CMpaDecFilter::LogLibAVCodec(void* par,int level,const char *fmt,va_list valist)
{
	char		Msg [500];
	vsnprintf_s (Msg, sizeof(Msg), _TRUNCATE, fmt, valist);
	TRACE("AVLIB : %s", Msg);
}

void CMpaDecFilter::ffmpeg_stream_finish()
{
	m_pAVCodec	= NULL;
	if (m_pAVCtx) {
		avcodec_close (m_pAVCtx);
		av_free (m_pAVCtx);
		m_pAVCtx	= NULL;
	}

	if (m_pParser) {
		av_parser_close (m_pParser);
		m_pParser	= NULL;
	}

	if (m_pPCMData) {
		FF_aligned_free (m_pPCMData);
	}
}

#pragma endregion

#endif /* HAS_FFMPEG_AUDIO_DECODERS */
