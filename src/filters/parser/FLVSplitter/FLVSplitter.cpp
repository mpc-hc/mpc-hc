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
#include "FLVSplitter.h"
#include "../../../DSUtil/DSUtil.h"

#include <InitGuid.h>
#include <moreuuids.h>

#define FLV_AUDIODATA     8
#define FLV_VIDEODATA     9
#define FLV_SCRIPTDATA    18

#define FLV_AUDIO_PCM     0 // Linear PCM, platform endian
#define FLV_AUDIO_ADPCM   1 // ADPCM
#define FLV_AUDIO_MP3     2 // MP3
#define FLV_AUDIO_PCMLE   3 // Linear PCM, little endian
#define FLV_AUDIO_NELLY16 4 // Nellymoser 16 kHz mono
#define FLV_AUDIO_NELLY8  5 // Nellymoser 8 kHz mono
#define FLV_AUDIO_NELLY   6 // Nellymoser
// 7 = G.711 A-law logarithmic PCM (reserved)
// 8 = G.711 mu-law logarithmic PCM (reserved)
// 9 = reserved
#define FLV_AUDIO_AAC     10 // AAC
#define FLV_AUDIO_SPEEX   11 // Speex
// 14 = MP3 8 kHz (reserved)
// 15 = Device-specific sound (reserved)

//#define FLV_VIDEO_JPEG    1 // non-standard? need samples
#define FLV_VIDEO_H263    2 // Sorenson H.263
#define FLV_VIDEO_SCREEN  3 // Screen video
#define FLV_VIDEO_VP6     4 // On2 VP6
#define FLV_VIDEO_VP6A    5 // On2 VP6 with alpha channel
#define FLV_VIDEO_SCREEN2 6 // Screen video version 2
#define FLV_VIDEO_AVC     7 // AVC

#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] = {
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_FLV},
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_PIN sudpPins[] = {
	{L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesIn), sudPinTypesIn},
	{L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, 0, NULL}
};

const AMOVIESETUP_MEDIATYPE sudPinTypesOut2[] = {
	{&MEDIATYPE_Video, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_FILTER sudFilter[] = {
	{&__uuidof(CFLVSplitterFilter), FlvSplitterName, MERIT_NORMAL, countof(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory},
	{&__uuidof(CFLVSourceFilter), FlvSourceName, MERIT_NORMAL, 0, NULL, CLSID_LegacyAmFilterCategory},
};

CFactoryTemplate g_Templates[] = {
	{sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CFLVSplitterFilter>, NULL, &sudFilter[0]},
	{sudFilter[1].strName, sudFilter[1].clsID, CreateInstance<CFLVSourceFilter>, NULL, &sudFilter[1]},
};

int g_cTemplates = countof(g_Templates);

STDAPI DllRegisterServer()
{
	DeleteRegKey(_T("Media Type\\Extensions\\"), _T(".flv"));

	RegisterSourceFilter(CLSID_AsyncReader, MEDIASUBTYPE_FLV, _T("0,4,,464C5601"), NULL);

	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	UnRegisterSourceFilter(MEDIASUBTYPE_FLV);

	return AMovieDllRegisterServer2(FALSE);
}

#include "../../FilterApp.h"

CFilterApp theApp;

#endif

//
// CFLVSplitterFilter
//

CFLVSplitterFilter::CFLVSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr)
	: CBaseSplitterFilter(NAME("CFLVSplitterFilter"), pUnk, phr, __uuidof(this))
{
}

STDMETHODIMP CFLVSplitterFilter::QueryFilterInfo(FILTER_INFO* pInfo)
{
	CheckPointer(pInfo, E_POINTER);
	ValidateReadWritePtr(pInfo, sizeof(FILTER_INFO));

	if (m_pName && m_pName[0]==L'M' && m_pName[1]==L'P' && m_pName[2]==L'C') {
		(void)StringCchCopyW(pInfo->achName, NUMELMS(pInfo->achName), m_pName);
	} else {
		wcscpy(pInfo->achName, FlvSourceName);
	}
	pInfo->pGraph = m_pGraph;
	if (m_pGraph) {
		m_pGraph->AddRef();
	}

	return S_OK;
}

bool CFLVSplitterFilter::ReadTag(Tag& t)
{
	if (m_pFile->GetRemaining() < 15) {
		return false;
	}

	t.PreviousTagSize = (UINT32)m_pFile->BitRead(32);
	t.TagType = (BYTE)m_pFile->BitRead(8);
	t.DataSize = (UINT32)m_pFile->BitRead(24);
	t.TimeStamp = (UINT32)m_pFile->BitRead(24);
	t.TimeStamp |= (UINT32)m_pFile->BitRead(8) << 24;
	t.StreamID = (UINT32)m_pFile->BitRead(24);

	return m_pFile->GetRemaining() >= t.DataSize;
}

bool CFLVSplitterFilter::ReadTag(AudioTag& at)
{
	if (!m_pFile->GetRemaining()) {
		return false;
	}

	at.SoundFormat = (BYTE)m_pFile->BitRead(4);
	at.SoundRate = (BYTE)m_pFile->BitRead(2);
	at.SoundSize = (BYTE)m_pFile->BitRead(1);
	at.SoundType = (BYTE)m_pFile->BitRead(1);

	return true;
}

bool CFLVSplitterFilter::ReadTag(VideoTag& vt)
{
	if (!m_pFile->GetRemaining()) {
		return false;
	}

	vt.FrameType = (BYTE)m_pFile->BitRead(4);
	vt.CodecID = (BYTE)m_pFile->BitRead(4);

	return true;
}

#ifndef NOVIDEOTWEAK
bool CFLVSplitterFilter::ReadTag(VideoTweak& vt)
{
	if (!m_pFile->GetRemaining()) {
		return false;
	}

	vt.x = (BYTE)m_pFile->BitRead(4);
	vt.y = (BYTE)m_pFile->BitRead(4);

	return true;
}
#endif

bool CFLVSplitterFilter::Sync(__int64& pos)
{
	m_pFile->Seek(pos);

	while (m_pFile->GetRemaining() >= 15) {
		__int64 limit = m_pFile->GetRemaining();
		while (true) {
			BYTE b = m_pFile->BitRead(8);
			if (b == FLV_AUDIODATA || b == FLV_VIDEODATA) {
				break;
			}
			if (--limit < 15) {
				return false;
			}
		}

		pos = m_pFile->GetPos() - 5;
		m_pFile->Seek(pos);

		Tag ct;
		if (ReadTag(ct)) {
			__int64 next = m_pFile->GetPos() + ct.DataSize;
			if (next == m_pFile->GetLength() - 4) {
				m_pFile->Seek(pos);
				return true;
			} else if (next <= m_pFile->GetLength() - 19) {
				m_pFile->Seek(next);
				Tag nt;
				if (ReadTag(nt) && (nt.TagType == FLV_AUDIODATA || nt.TagType == FLV_VIDEODATA || nt.TagType == FLV_SCRIPTDATA)) {
					if ((nt.PreviousTagSize == ct.DataSize + 11) ||
							(m_IgnorePrevSizes &&
							 nt.TimeStamp >= ct.TimeStamp &&
							 nt.TimeStamp - ct.TimeStamp <= 1000)) {
						m_pFile->Seek(pos);
						return true;
					}
				}
			}
		}

		m_pFile->Seek(pos + 5);
	}

	return false;
}

HRESULT CFLVSplitterFilter::CreateOutputs(IAsyncReader* pAsyncReader)
{
	CheckPointer(pAsyncReader, E_POINTER);

	HRESULT hr = E_FAIL;

	m_pFile.Free();
	m_pFile.Attach(DNew CBaseSplitterFileEx(pAsyncReader, hr, DEFAULT_CACHE_LENGTH, false));
	if (!m_pFile) {
		return E_OUTOFMEMORY;
	}
	if (FAILED(hr)) {
		m_pFile.Free();
		return hr;
	}

	m_rtNewStart = m_rtCurrent = 0;
	m_rtNewStop = m_rtStop = m_rtDuration = 0;

	if (m_pFile->BitRead(24) != 'FLV' || m_pFile->BitRead(8) != 1) {
		return E_FAIL;
	}

	EXECUTE_ASSERT(m_pFile->BitRead(5) == 0); // TypeFlagsReserved
	bool fTypeFlagsAudio = !!m_pFile->BitRead(1);
	EXECUTE_ASSERT(m_pFile->BitRead(1) == 0); // TypeFlagsReserved
	bool fTypeFlagsVideo = !!m_pFile->BitRead(1);
	m_DataOffset = (UINT32)m_pFile->BitRead(32);

	// doh, these flags aren't always telling the truth
	fTypeFlagsAudio = fTypeFlagsVideo = true;

	Tag t;
	AudioTag at;
	VideoTag vt;

	UINT32 prevTagSize = 0;
	m_IgnorePrevSizes = false;

	m_pFile->Seek(m_DataOffset);

	for (int i = 0; ReadTag(t) && (fTypeFlagsVideo || fTypeFlagsAudio); i++) {
		if (!t.DataSize) continue; // skip empty Tag

		UINT64 next = m_pFile->GetPos() + t.DataSize;

		CStringW name;

		CMediaType mt;
		CMediaType ff_mtype;
		mt.SetSampleSize(1);
		mt.subtype = GUID_NULL;

		if (i != 0 && t.PreviousTagSize != prevTagSize) {
			m_IgnorePrevSizes = true;
		}
		prevTagSize = t.DataSize + 11;

		if (t.TagType == FLV_AUDIODATA && t.DataSize != 0 && fTypeFlagsAudio) {
			UNREFERENCED_PARAMETER(at);
			AudioTag at;
			name = L"Audio";

			if (ReadTag(at)) {
				int dataSize = t.DataSize - 1;

				fTypeFlagsAudio = false;

				mt.majortype = MEDIATYPE_Audio;
				mt.formattype = FORMAT_WaveFormatEx;
				WAVEFORMATEX* wfe = (WAVEFORMATEX*)mt.AllocFormatBuffer(sizeof(WAVEFORMATEX));
				memset(wfe, 0, sizeof(WAVEFORMATEX));
				wfe->nSamplesPerSec = 44100*(1<<at.SoundRate)/8;
				wfe->wBitsPerSample = 8*(at.SoundSize+1);
				wfe->nChannels = at.SoundType+1;

				switch (at.SoundFormat) {
					case FLV_AUDIO_PCM:
					case FLV_AUDIO_PCMLE:
						mt.subtype = FOURCCMap(wfe->wFormatTag = WAVE_FORMAT_PCM);
						name += L" PCM";
						break;
					case FLV_AUDIO_ADPCM:
						mt.subtype = FOURCCMap(wfe->wFormatTag = WAVE_FORMAT_ADPCM_SWF);
						name += L" ADPCM";
						break;
					case FLV_AUDIO_MP3:
						mt.subtype = FOURCCMap(wfe->wFormatTag = WAVE_FORMAT_MP3);
						name += L" MP3";

						{
							CBaseSplitterFileEx::mpahdr h;
							CMediaType mt2;
							if (m_pFile->Read(h, 4, false, &mt2)) {
								mt = mt2;
							}
						}
						break;
					case FLV_AUDIO_NELLY16:
						mt.subtype = FOURCCMap(MAKEFOURCC('N','E','L','L'));
						wfe->nSamplesPerSec = 16000;
						name += L" Nellimoser";
						break;
					case FLV_AUDIO_NELLY8:
						mt.subtype = FOURCCMap(MAKEFOURCC('N','E','L','L'));
						wfe->nSamplesPerSec = 8000;
						name += L" Nellimoser";
						break;
					case FLV_AUDIO_NELLY:
						mt.subtype = FOURCCMap(MAKEFOURCC('N','E','L','L'));
						name += L" Nellimoser";
						break;
					case FLV_AUDIO_AAC: {
						if (dataSize < 1 || m_pFile->BitRead(8) != 0) { // packet type 0 == aac header
							fTypeFlagsAudio = true;
							break;
						}
						name += L" AAC";

						const int sampleRates[] = {
							96000, 88200, 64000, 48000, 44100, 32000, 24000,
							22050, 16000, 12000, 11025, 8000, 7350
						};
						const int channels[] = {
							0, 1, 2, 3, 4, 5, 6, 8
						};

						__int64 configOffset = m_pFile->GetPos();
						UINT32 configSize = dataSize - 1;
						if (configSize < 2) {
							break;
						}

						// Might break depending on the AAC profile, see ff_mpeg4audio_get_config in ffmpeg's mpeg4audio.c
						m_pFile->BitRead(5);
						int iSampleRate = m_pFile->BitRead(4);
						int iChannels = m_pFile->BitRead(4);
						if (iSampleRate > 12 || iChannels > 7) {
							break;
						}

						wfe = (WAVEFORMATEX*)mt.AllocFormatBuffer(sizeof(WAVEFORMATEX) + configSize);
						memset(wfe, 0, mt.FormatLength());
						wfe->nSamplesPerSec = sampleRates[iSampleRate];
						wfe->wBitsPerSample = 16;
						wfe->nChannels = channels[iChannels];
						wfe->cbSize = configSize;

						m_pFile->Seek(configOffset);
						m_pFile->ByteRead((BYTE*)(wfe+1), configSize);

						mt.subtype = FOURCCMap(wfe->wFormatTag = WAVE_FORMAT_AAC);
					}

				}
			}
		} else if (t.TagType == FLV_VIDEODATA && t.DataSize != 0 && fTypeFlagsVideo) {
			UNREFERENCED_PARAMETER(vt);
			VideoTag vt;
			if (ReadTag(vt) && vt.FrameType == 1) {
				int dataSize = t.DataSize - 1;

				fTypeFlagsVideo = false;
				name = L"Video";

				mt.majortype = MEDIATYPE_Video;
				mt.formattype = FORMAT_VideoInfo;
				VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)mt.AllocFormatBuffer(sizeof(VIDEOINFOHEADER));
				memset(vih, 0, sizeof(VIDEOINFOHEADER));

				BITMAPINFOHEADER* bih = &vih->bmiHeader;

				int w, h, arx, ary;

				switch (vt.CodecID) {
					case FLV_VIDEO_H263:   // H.263
						if (m_pFile->BitRead(17) != 1) {
							break;
						}

						m_pFile->BitRead(13); // Version (5), TemporalReference (8)

						switch (BYTE PictureSize = (BYTE)m_pFile->BitRead(3)) { // w00t
							case 0:
							case 1:
								vih->bmiHeader.biWidth = (WORD)m_pFile->BitRead(8*(PictureSize+1));
								vih->bmiHeader.biHeight = (WORD)m_pFile->BitRead(8*(PictureSize+1));
								break;
							case 2:
							case 3:
							case 4:
								vih->bmiHeader.biWidth = 704 / PictureSize;
								vih->bmiHeader.biHeight = 576 / PictureSize;
								break;
							case 5:
							case 6:
								PictureSize -= 3;
								vih->bmiHeader.biWidth = 640 / PictureSize;
								vih->bmiHeader.biHeight = 480 / PictureSize;
								break;
						}

						if (!vih->bmiHeader.biWidth || !vih->bmiHeader.biHeight) {
							break;
						}

						mt.subtype = FOURCCMap(vih->bmiHeader.biCompression = '1VLF');
						name += L" H.263";

						break;
					case FLV_VIDEO_SCREEN: {
						m_pFile->BitRead(4);
						vih->bmiHeader.biWidth  = m_pFile->BitRead(12);
						m_pFile->BitRead(4);
						vih->bmiHeader.biHeight = m_pFile->BitRead(12);

						if (!vih->bmiHeader.biWidth || !vih->bmiHeader.biHeight) {
							break;
						}

						vih->bmiHeader.biSize = sizeof(vih->bmiHeader);
						vih->bmiHeader.biPlanes = 1;
						vih->bmiHeader.biBitCount = 24;
						vih->bmiHeader.biSizeImage = vih->bmiHeader.biWidth * vih->bmiHeader.biHeight * 3;

						mt.subtype = FOURCCMap(vih->bmiHeader.biCompression = '1VSF');
						name += L" Screen";

						break;
					}
					case FLV_VIDEO_VP6A:  // VP6 with alpha
						m_pFile->BitRead(24);
					case FLV_VIDEO_VP6: { // VP6
#ifdef NOVIDEOTWEAK
						m_pFile->BitRead(8);
#else
						VideoTweak fudge;
						ReadTag(fudge);
#endif

						if (m_pFile->BitRead(1)) {
							// Delta (inter) frame
							fTypeFlagsVideo = true;
							break;
						}
						m_pFile->BitRead(6);
						bool fSeparatedCoeff = !!m_pFile->BitRead(1);
						m_pFile->BitRead(5);
						int filterHeader = m_pFile->BitRead(2);
						m_pFile->BitRead(1);
						if (fSeparatedCoeff || !filterHeader) {
							m_pFile->BitRead(16);
						}

						h = m_pFile->BitRead(8) * 16;
						w = m_pFile->BitRead(8) * 16;

						ary = m_pFile->BitRead(8) * 16;
						arx = m_pFile->BitRead(8) * 16;

						if (arx && arx != w || ary && ary != h) {
							VIDEOINFOHEADER2* vih2 = (VIDEOINFOHEADER2*)mt.AllocFormatBuffer(sizeof(VIDEOINFOHEADER2));
							memset(vih2, 0, sizeof(VIDEOINFOHEADER2));
							vih2->dwPictAspectRatioX = arx;
							vih2->dwPictAspectRatioY = ary;
							bih = &vih2->bmiHeader;
							mt.formattype = FORMAT_VideoInfo2;
							vih = (VIDEOINFOHEADER *)vih2;
						}

						bih->biWidth = w;
						bih->biHeight = h;
#ifndef NOVIDEOTWEAK
						SetRect(&vih->rcSource, 0, 0, w - fudge.x, h - fudge.y);
						SetRect(&vih->rcTarget, 0, 0, w - fudge.x, h - fudge.y);
#endif

						mt.subtype = FOURCCMap(bih->biCompression = '4VLF');
						name += L" VP6";

						break;
					}
					case FLV_VIDEO_AVC: { // H.264
						if (dataSize < 4 || m_pFile->BitRead(8) != 0) { // packet type 0 == avc header
							fTypeFlagsVideo = true;
							break;
						}
						m_pFile->BitRead(24); // composition time

						__int64 headerOffset = m_pFile->GetPos();
						UINT32 headerSize = dataSize - 4;
						BYTE *headerData = DNew BYTE[headerSize];

						m_pFile->ByteRead(headerData, headerSize);

						m_pFile->Seek(headerOffset + 9);

						mt.formattype = FORMAT_MPEG2Video;
						MPEG2VIDEOINFO* vih = (MPEG2VIDEOINFO*)mt.AllocFormatBuffer(FIELD_OFFSET(MPEG2VIDEOINFO, dwSequenceHeader) + headerSize);
						memset(vih, 0, mt.FormatLength());
						vih->hdr.bmiHeader.biSize = sizeof(vih->hdr.bmiHeader);
						vih->hdr.bmiHeader.biPlanes = 1;
						vih->hdr.bmiHeader.biBitCount = 24;
						vih->dwFlags = (headerData[4] & 0x03) + 1; // nal length size

						vih->dwProfile = (BYTE)m_pFile->BitRead(8);
						m_pFile->BitRead(8);
						vih->dwLevel = (BYTE)m_pFile->BitRead(8);
						m_pFile->UExpGolombRead(); // seq_parameter_set_id
						UINT64 chroma_format_idc = 0;
						if (vih->dwProfile >= 100) { // high profile
							chroma_format_idc = m_pFile->UExpGolombRead();
							if (chroma_format_idc == 3) { // chroma_format_idc
								m_pFile->BitRead(1);    // residue_transform_flag
							}
							m_pFile->UExpGolombRead(); // bit_depth_luma_minus8
							m_pFile->UExpGolombRead(); // bit_depth_chroma_minus8
							m_pFile->BitRead(1); // qpprime_y_zero_transform_bypass_flag
							if (m_pFile->BitRead(1)) // seq_scaling_matrix_present_flag
								for (int i = 0; i < 8; i++)
									if (m_pFile->BitRead(1)) // seq_scaling_list_present_flag
										for (int j = 0, size = i < 6 ? 16 : 64, next = 8; j < size && next != 0; ++j) {
											next = (next + m_pFile->SExpGolombRead() + 256) & 255;
										}
						}
						m_pFile->UExpGolombRead(); // log2_max_frame_num_minus4
						UINT64 pic_order_cnt_type = m_pFile->UExpGolombRead();
						if (pic_order_cnt_type == 0) {
							m_pFile->UExpGolombRead(); // log2_max_pic_order_cnt_lsb_minus4
						} else if (pic_order_cnt_type == 1) {
							m_pFile->BitRead(1); // delta_pic_order_always_zero_flag
							m_pFile->SExpGolombRead(); // offset_for_non_ref_pic
							m_pFile->SExpGolombRead(); // offset_for_top_to_bottom_field
							UINT64 num_ref_frames_in_pic_order_cnt_cycle = m_pFile->UExpGolombRead();
							for (int i = 0; i < num_ref_frames_in_pic_order_cnt_cycle; i++) {
								m_pFile->SExpGolombRead();    // offset_for_ref_frame[i]
							}
						}
						m_pFile->UExpGolombRead(); // num_ref_frames
						m_pFile->BitRead(1); // gaps_in_frame_num_value_allowed_flag
						UINT64 pic_width_in_mbs_minus1 = m_pFile->UExpGolombRead();
						UINT64 pic_height_in_map_units_minus1 = m_pFile->UExpGolombRead();
						BYTE frame_mbs_only_flag = (BYTE)m_pFile->BitRead(1);
						if (!frame_mbs_only_flag) {
							m_pFile->BitRead(1); // mb_adaptive_frame_field_flag
						}
						m_pFile->BitRead(1); // direct_8x8_inference_flag
						BYTE crop = m_pFile->BitRead(1); // frame_cropping_flag
						UINT64 crop_left = 0;
						UINT64 crop_right = 0;
						UINT64 crop_top = 0;
						UINT64 crop_bottom = 0;

						if (crop) {
							crop_left   = m_pFile->UExpGolombRead(); // frame_cropping_rect_left_offset
							crop_right  = m_pFile->UExpGolombRead(); // frame_cropping_rect_right_offset
							crop_top    = m_pFile->UExpGolombRead(); // frame_cropping_rect_top_offset
							crop_bottom = m_pFile->UExpGolombRead(); // frame_cropping_rect_bottom_offset
						}
						struct sar {
							BYTE num;
							BYTE den;
						} sar;

						if (m_pFile->BitRead(1)) {						// vui_parameters_present_flag
							if (m_pFile->BitRead(1)) {					// aspect_ratio_info_present_flag
								BYTE aspect_ratio_idc = m_pFile->BitRead(8); // aspect_ratio_idc
								if (255==(BYTE)aspect_ratio_idc) {
									sar.num = m_pFile->BitRead(16);				// sar_width
									sar.den = m_pFile->BitRead(16);				// sar_height
								} else if (aspect_ratio_idc < 17) {
									sar.num = pixel_aspect[aspect_ratio_idc][0];
									sar.den = pixel_aspect[aspect_ratio_idc][1];
								} else {
									return false;
								}
							} else {
								sar.num = 1;
								sar.den = 1;
							}
						}
						UINT64 mb_Width = pic_width_in_mbs_minus1 + 1;
						UINT64 mb_Height = (pic_height_in_map_units_minus1 + 1) * (2 - frame_mbs_only_flag);
						BYTE CHROMA444 = (chroma_format_idc == 3);

						UINT64 Width, Height;
						Width = 16 * mb_Width - (2>>CHROMA444) * min(crop_right, (8<<CHROMA444)-1);
						if (frame_mbs_only_flag) {
							Height = 16 * mb_Height - (2>>CHROMA444) * min(crop_bottom, (8<<CHROMA444)-1);
						} else {
							Height = 16 * mb_Height - (4>>CHROMA444) * min(crop_bottom, (8<<CHROMA444)-1);
						}
						if (!sar.num) sar.num = 1;
						if (!sar.den) sar.den = 1;
						CSize aspect(Width * sar.num, Height * sar.den);
						int lnko = LNKO(aspect.cx, aspect.cy);
						if (lnko > 1) {
							aspect.cx /= lnko, aspect.cy /= lnko;
						}

						vih->hdr.dwPictAspectRatioX = aspect.cx;
						vih->hdr.dwPictAspectRatioY = aspect.cy;
						vih->hdr.bmiHeader.biWidth = Width;
						vih->hdr.bmiHeader.biHeight = Height;

						BYTE* src = (BYTE*)headerData + 5;
						BYTE* dst = (BYTE*)vih->dwSequenceHeader;
						BYTE* src_end = (BYTE*)headerData + headerSize;
						BYTE* dst_end = (BYTE*)vih->dwSequenceHeader + headerSize;
						int spsCount = *(src++) & 0x1F;
						int ppsCount = -1;

						vih->cbSequenceHeader = 0;

						while (src < src_end - 1) {
							if (spsCount == 0 && ppsCount == -1) {
								ppsCount = *(src++);
								continue;
							}

							if (spsCount > 0) {
								spsCount--;
							} else if (ppsCount > 0) {
								ppsCount--;
							} else {
								break;
							}

							int len = ((src[0] << 8) | src[1]) + 2;
							if (src + len > src_end || dst + len > dst_end) {
								ASSERT(0);
								break;
							}
							memcpy(dst, src, len);
							src += len;
							dst += len;
							vih->cbSequenceHeader += len;
						}

						delete[] headerData;

						mt.subtype = FOURCCMap(vih->hdr.bmiHeader.biCompression = '1CVA');
						name += L" H.264";

						break;
					}
					default:
						fTypeFlagsVideo = true;
				}
			}
		}

		if (mt.subtype != GUID_NULL) {
			CAtlArray<CMediaType> mts;
			if (mt.subtype == FOURCCMap(MAKEFOURCC('A','S','W','F'))) {
				mts.InsertAt(0, ff_mtype);
			}
			mts.Add(mt);
			CAutoPtr<CBaseSplitterOutputPin> pPinOut(DNew CBaseSplitterOutputPin(mts, name, this, this, &hr));
			EXECUTE_ASSERT(SUCCEEDED(AddOutputPin(t.TagType, pPinOut)));
		}

		m_pFile->Seek(next);
	}

	if (m_pFile->IsRandomAccess()) {
		__int64 pos = max(m_DataOffset, m_pFile->GetLength() - 256 * 1024);

		if (Sync(pos)) {
			Tag t;
			AudioTag at;
			VideoTag vt;

			while (ReadTag(t)) {
				UINT64 next = m_pFile->GetPos() + t.DataSize;

				if (t.TagType == FLV_AUDIODATA && ReadTag(at) || t.TagType == FLV_VIDEODATA && ReadTag(vt)) {
					m_rtDuration = max(m_rtDuration, 10000i64 * t.TimeStamp);
				}

				m_pFile->Seek(next);
			}
		}
	}

	m_rtNewStop = m_rtStop = m_rtDuration;

	return m_pOutputs.GetCount() > 0 ? S_OK : E_FAIL;
}

bool CFLVSplitterFilter::DemuxInit()
{
	SetThreadName((DWORD)-1, "CFLVSplitterFilter");
	return true;
}

void CFLVSplitterFilter::DemuxSeek(REFERENCE_TIME rt)
{
	if (!m_rtDuration || rt <= 0) {
		m_pFile->Seek(m_DataOffset);
	} else if (!m_IgnorePrevSizes) {
		NormalSeek(rt);
	} else {
		AlternateSeek(rt);
	}
}

void CFLVSplitterFilter::NormalSeek(REFERENCE_TIME rt)
{
	bool fAudio = !!GetOutputPin(FLV_AUDIODATA);
	bool fVideo = !!GetOutputPin(FLV_VIDEODATA);

	__int64 pos = m_DataOffset + 1.0 * rt / m_rtDuration * (m_pFile->GetLength() - m_DataOffset);

	if (!Sync(pos)) {
		ASSERT(0);
		m_pFile->Seek(m_DataOffset);
		return;
	}

	Tag t;
	AudioTag at;
	VideoTag vt;

	while (ReadTag(t)) {
		if (10000i64 * t.TimeStamp >= rt) {
			m_pFile->Seek(m_pFile->GetPos() - 15);
			break;
		}

		m_pFile->Seek(m_pFile->GetPos() + t.DataSize);
	}

	while (m_pFile->GetPos() >= m_DataOffset && (fAudio || fVideo) && ReadTag(t)) {
		UINT64 prev = m_pFile->GetPos() - 15 - t.PreviousTagSize - 4;

		if (10000i64 * t.TimeStamp <= rt) {
			if (t.TagType == FLV_AUDIODATA && ReadTag(at)) {
				fAudio = false;
			} else if (t.TagType == FLV_VIDEODATA && ReadTag(vt) && vt.FrameType == 1) {
				fVideo = false;
			}
		}

		m_pFile->Seek(prev);
	}

	if (fAudio || fVideo) {
		ASSERT(0);
		m_pFile->Seek(m_DataOffset);
	}
}

void CFLVSplitterFilter::AlternateSeek(REFERENCE_TIME rt)
{
	bool hasAudio = !!GetOutputPin(FLV_AUDIODATA);
	bool hasVideo = !!GetOutputPin(FLV_VIDEODATA);

	__int64 estimPos = m_DataOffset + 1.0 * rt / m_rtDuration * (m_pFile->GetLength() - m_DataOffset);

	while (true) {
		estimPos -= 256 * 1024;
		if (estimPos < m_DataOffset) estimPos = m_DataOffset;

		bool foundAudio = !hasAudio;
		bool foundVideo = !hasVideo;
		__int64 bestPos = estimPos;

		if (Sync(bestPos)) {
			Tag t;
			AudioTag at;
			VideoTag vt;

			while (ReadTag(t) && t.TimeStamp * 10000i64 < rt) {
				__int64 cur = m_pFile->GetPos() - 15;
				__int64 next = cur + 15 + t.DataSize;

				if (hasAudio && t.TagType == FLV_AUDIODATA && ReadTag(at)) {
					foundAudio = true;
					if (!hasVideo) {
						bestPos = cur;
					}
				} else if (hasVideo && t.TagType == FLV_VIDEODATA && ReadTag(vt) && vt.FrameType == 1) {
					foundVideo = true;
					bestPos = cur;
				}

				m_pFile->Seek(next);
			}
		}

		if (foundAudio && foundVideo) {
			m_pFile->Seek(bestPos);
			return;
		} else if (estimPos == m_DataOffset) {
			m_pFile->Seek(m_DataOffset);
			return;
		}

	}
}

bool CFLVSplitterFilter::DemuxLoop()
{
	HRESULT hr = S_OK;

	CAutoPtr<Packet> p;

	Tag t;
	AudioTag at = {};
	VideoTag vt = {};

	while (SUCCEEDED(hr) && !CheckRequest(NULL) && m_pFile->GetRemaining()) {
		if (!ReadTag(t)) {
			break;
		}

		__int64 next = m_pFile->GetPos() + t.DataSize;

		if ((t.DataSize > 0) && (t.TagType == FLV_AUDIODATA && ReadTag(at) || t.TagType == FLV_VIDEODATA && ReadTag(vt))) {
			UINT32 tsOffset = 0;
			if (t.TagType == FLV_VIDEODATA) {
				if (vt.FrameType == 5) {
					goto NextTag;    // video info/command frame
				}
				if (vt.CodecID == FLV_VIDEO_VP6) {
					m_pFile->BitRead(8);
				} else if (vt.CodecID == FLV_VIDEO_VP6A) {
					m_pFile->BitRead(32);
				} else if (vt.CodecID == FLV_VIDEO_AVC) {
					if (m_pFile->BitRead(8) != 1) {
						goto NextTag;
					}
					// Tag timestamps specify decode time, this is the display time offset
					tsOffset = m_pFile->BitRead(24);
					tsOffset = (tsOffset + 0xff800000) ^ 0xff800000; // sign extension
				}
			}
			if (t.TagType == FLV_AUDIODATA && at.SoundFormat == FLV_AUDIO_AAC) {
				if (m_pFile->BitRead(8) != 1) {
					goto NextTag;
				}
			}
			__int64 dataSize = next - m_pFile->GetPos();
			if (dataSize <= 0) {
				goto NextTag;
			}
			p.Attach(DNew Packet());
			p->TrackNumber = t.TagType;
			p->rtStart = 10000i64 * (t.TimeStamp + tsOffset);
			p->rtStop = p->rtStart + 1;
			p->bSyncPoint = t.TagType == FLV_VIDEODATA ? vt.FrameType == 1 : true;
			p->SetCount(dataSize);
			m_pFile->ByteRead(p->GetData(), p->GetCount());
			hr = DeliverPacket(p);
		}

NextTag:
		m_pFile->Seek(next);
	}

	return true;
}

//
// CFLVSourceFilter
//

CFLVSourceFilter::CFLVSourceFilter(LPUNKNOWN pUnk, HRESULT* phr)
	: CFLVSplitterFilter(pUnk, phr)
{
	m_clsid = __uuidof(this);
	m_pInput.Free();
}
