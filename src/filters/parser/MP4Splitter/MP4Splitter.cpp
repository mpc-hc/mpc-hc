/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
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
#include "Mmreg.h"
#include "MP4Splitter.h"
#include "../../../DSUtil/DSUtil.h"

#include <initguid.h>
#include <moreuuids.h>

#include "Ap4.h"
#include "Ap4File.h"
#include "Ap4StssAtom.h"
#include "Ap4StsdAtom.h"
#include "Ap4IsmaCryp.h"
#include "Ap4AvcCAtom.h"
#include "Ap4ChplAtom.h"
#include "Ap4FtabAtom.h"
#include "Ap4MetaData.h"

#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] =
{
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_MP4},
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_PIN sudpPins[] =
{
	{L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesIn), sudPinTypesIn},
	{L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, 0, NULL}
};

const AMOVIESETUP_FILTER sudFilter[] =
{
	{&__uuidof(CMP4SplitterFilter), L"MPC - MP4 Splitter", MERIT_NORMAL, countof(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory},
	{&__uuidof(CMP4SourceFilter), L"MPC - MP4 Source", MERIT_NORMAL, 0, NULL, CLSID_LegacyAmFilterCategory},
	{&__uuidof(CMPEG4VideoSplitterFilter), L"MPC - MPEG4 Video Splitter", MERIT_NORMAL, countof(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory},
	{&__uuidof(CMPEG4VideoSourceFilter), L"MPC - MPEG4 Video Source", MERIT_NORMAL, 0, NULL, CLSID_LegacyAmFilterCategory},	
};

CFactoryTemplate g_Templates[] =
{
	{sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CMP4SplitterFilter>, NULL, &sudFilter[0]},
	{sudFilter[1].strName, sudFilter[1].clsID, CreateInstance<CMP4SourceFilter>, NULL, &sudFilter[1]},
	{sudFilter[2].strName, sudFilter[2].clsID, CreateInstance<CMPEG4VideoSplitterFilter>, NULL, &sudFilter[2]},
	{sudFilter[3].strName, sudFilter[3].clsID, CreateInstance<CMPEG4VideoSourceFilter>, NULL, &sudFilter[3]},
};

int g_cTemplates = countof(g_Templates);

STDAPI DllRegisterServer()
{
	DeleteRegKey(_T("Media Type\\Extensions\\"), _T(".mp4"));
	DeleteRegKey(_T("Media Type\\Extensions\\"), _T(".mov"));

	CAtlList<CString> chkbytes;

	// mp4
	chkbytes.AddTail(_T("4,4,,66747970")); // ftyp
	chkbytes.AddTail(_T("4,4,,6d6f6f76")); // moov
	chkbytes.AddTail(_T("4,4,,6d646174")); // mdat
	chkbytes.AddTail(_T("4,4,,736b6970")); // skip
	chkbytes.AddTail(_T("4,12,ffffffff00000000ffffffff,77696465027fe3706d646174")); // wide ? mdat

	// mpeg4 video
	chkbytes.AddTail(_T("3,3,,000001"));

	RegisterSourceFilter(CLSID_AsyncReader, MEDIASUBTYPE_MP4, chkbytes, NULL);

	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	UnRegisterSourceFilter(MEDIASUBTYPE_MP4);

	return AMovieDllRegisterServer2(FALSE);
}

#include "../../FilterApp.h"

CFilterApp theApp;

#endif

AP4_Result GetDataBuffer (AP4_Atom* atom, AP4_DataBuffer& db)
{
	AP4_MemoryByteStream*	mbs = new AP4_MemoryByteStream(db);
	AP4_Result				res;

	res = atom->Write (*mbs);
	mbs->Release();		// Release delete the object!

	return res;
}


//
// CMP4SplitterFilter
//

CMP4SplitterFilter::CMP4SplitterFilter(LPUNKNOWN pUnk, HRESULT* phr)
	: CBaseSplitterFilter(NAME("CMP4SplitterFilter"), pUnk, phr, __uuidof(this))
{
}

CMP4SplitterFilter::~CMP4SplitterFilter()
{
}

HRESULT CMP4SplitterFilter::CreateOutputs(IAsyncReader* pAsyncReader)
{
	CheckPointer(pAsyncReader, E_POINTER);

	HRESULT hr = E_FAIL;

	m_trackpos.RemoveAll();

	m_pFile.Free();
	m_pFile.Attach(DNew CMP4SplitterFile(pAsyncReader, hr));
	if(!m_pFile) return E_OUTOFMEMORY;
	if(FAILED(hr)) {m_pFile.Free(); return hr;}

	m_rtNewStart = m_rtCurrent = 0;
	m_rtNewStop = m_rtStop = m_rtDuration = 0;

	m_framesize.SetSize(640, 480);

	if(AP4_Movie* movie = (AP4_Movie*)m_pFile->GetMovie())
	{
		for(AP4_List<AP4_Track>::Item* item = movie->GetTracks().FirstItem();
			item;
			item = item->GetNext())
		{
			AP4_Track* track = item->GetData();

			if(track->GetType() != AP4_Track::TYPE_VIDEO 
			&& track->GetType() != AP4_Track::TYPE_AUDIO
			&& track->GetType() != AP4_Track::TYPE_TEXT
			&& track->GetType() != AP4_Track::TYPE_SUBP)
				continue;

			AP4_Sample sample;

			if(!AP4_SUCCEEDED(track->GetSample(0, sample)) || sample.GetDescriptionIndex() == 0xFFFFFFFF)
				continue;

			CStringW TrackName = UTF8To16(track->GetTrackName().GetChars());
			CStringA TrackLanguage = track->GetTrackLanguage().GetChars();

			CAtlArray<CMediaType> mts;

			CMediaType mt;
			mt.SetSampleSize(1);

			VIDEOINFOHEADER* vih = NULL;
			WAVEFORMATEX* wfe = NULL;

			AP4_DataBuffer empty;

			if(AP4_SampleDescription* desc = track->GetSampleDescription(sample.GetDescriptionIndex()))
			{
				AP4_MpegSampleDescription* mpeg_desc = NULL;

				if(desc->GetType() == AP4_SampleDescription::TYPE_MPEG)
				{
					mpeg_desc = dynamic_cast<AP4_MpegSampleDescription*>(desc);
				}
				else if(desc->GetType() == AP4_SampleDescription::TYPE_PROTECTED)
				{
					AP4_ProtectedSampleDescription* isma_desc = dynamic_cast<AP4_ProtectedSampleDescription*>(desc);
					mpeg_desc = dynamic_cast<AP4_MpegSampleDescription*>(isma_desc->GetOriginalSampleDescription());
				}

				if(AP4_MpegVideoSampleDescription* video_desc = 
					dynamic_cast<AP4_MpegVideoSampleDescription*>(mpeg_desc))
				{
					const AP4_DataBuffer& di = video_desc->GetDecoderInfo();
					//if(!di) di = &empty;

					mt.majortype = MEDIATYPE_Video;
					mt.formattype = FORMAT_VideoInfo;
					vih = (VIDEOINFOHEADER*)mt.AllocFormatBuffer(sizeof(VIDEOINFOHEADER) + di.GetDataSize());
					memset(vih, 0, mt.FormatLength());
					vih->dwBitRate = video_desc->GetAvgBitrate()/8;
					vih->bmiHeader.biSize = sizeof(vih->bmiHeader);
					vih->bmiHeader.biWidth = (LONG)video_desc->GetWidth();
					vih->bmiHeader.biHeight = (LONG)video_desc->GetHeight();
					memcpy(vih + 1, di.GetData(), di.GetDataSize());

					switch(video_desc->GetObjectTypeId())
					{
					case AP4_OTI_MPEG4_VISUAL:
						mt.subtype = FOURCCMap('v4pm');
						mt.formattype = FORMAT_MPEG2Video;
						{
						MPEG2VIDEOINFO* vih = (MPEG2VIDEOINFO*)mt.AllocFormatBuffer(FIELD_OFFSET(MPEG2VIDEOINFO, dwSequenceHeader) + di.GetDataSize());
						memset(vih, 0, mt.FormatLength());
						vih->hdr.bmiHeader.biSize = sizeof(vih->hdr.bmiHeader);
						vih->hdr.bmiHeader.biWidth = (LONG)video_desc->GetWidth();
						vih->hdr.bmiHeader.biHeight = (LONG)video_desc->GetHeight();
						vih->hdr.bmiHeader.biCompression = 'v4pm';
						vih->hdr.bmiHeader.biPlanes = 1;
						vih->hdr.bmiHeader.biBitCount = 24;
						vih->hdr.dwPictAspectRatioX = vih->hdr.bmiHeader.biWidth;
						vih->hdr.dwPictAspectRatioY = vih->hdr.bmiHeader.biHeight;
						vih->cbSequenceHeader = di.GetDataSize();
						memcpy(vih->dwSequenceHeader, di.GetData(), di.GetDataSize());
						mts.Add(mt);
						mt.subtype = FOURCCMap(vih->hdr.bmiHeader.biCompression = 'V4PM');
						mts.Add(mt);
						}
						break;
					case AP4_OTI_MPEG2_VISUAL_SIMPLE:
					case AP4_OTI_MPEG2_VISUAL_MAIN:
					case AP4_OTI_MPEG2_VISUAL_SNR:
					case AP4_OTI_MPEG2_VISUAL_SPATIAL:
					case AP4_OTI_MPEG2_VISUAL_HIGH:
					case AP4_OTI_MPEG2_VISUAL_422:
						mt.subtype = MEDIASUBTYPE_MPEG2_VIDEO;
						{
						m_pFile->Seek(sample.GetOffset());
						CBaseSplitterFileEx::seqhdr h;
						CMediaType mt2;
						if(m_pFile->Read(h, sample.GetSize(), &mt2))
							mt = mt2;
						}
						mts.Add(mt);
						break;
					case AP4_OTI_MPEG1_VISUAL: // ???
						mt.subtype = MEDIASUBTYPE_MPEG1Payload;
						{
						m_pFile->Seek(sample.GetOffset());
						CBaseSplitterFileEx::seqhdr h;
						CMediaType mt2;
						if(m_pFile->Read(h, sample.GetSize(), &mt2))
							mt = mt2;
						}
						mts.Add(mt);
						break;
					}

					if(mt.subtype == GUID_NULL)
					{
						TRACE(_T("Unknown video OBI: %02x\n"), video_desc->GetObjectTypeId());
					}
				}
				else if(AP4_MpegAudioSampleDescription* audio_desc = 
					dynamic_cast<AP4_MpegAudioSampleDescription*>(mpeg_desc))
				{
					const AP4_DataBuffer& di = audio_desc->GetDecoderInfo();
					//if(!di) di = &empty;

					mt.majortype = MEDIATYPE_Audio;
					mt.formattype = FORMAT_WaveFormatEx;

					wfe = (WAVEFORMATEX*)mt.AllocFormatBuffer(sizeof(WAVEFORMATEX) + di.GetDataSize());
					memset(wfe, 0, mt.FormatLength());
					wfe->nSamplesPerSec = audio_desc->GetSampleRate();
					wfe->nAvgBytesPerSec = audio_desc->GetAvgBitrate()/8;
					wfe->nChannels = audio_desc->GetChannelCount();
					wfe->wBitsPerSample = audio_desc->GetSampleSize();
					wfe->cbSize = (WORD)di.GetDataSize();
					memcpy(wfe + 1, di.GetData(), di.GetDataSize());

					switch(audio_desc->GetObjectTypeId())
					{
					case AP4_OTI_MPEG4_AUDIO:
					case AP4_OTI_MPEG2_AAC_AUDIO_MAIN: // ???
					case AP4_OTI_MPEG2_AAC_AUDIO_LC: // ???
					case AP4_OTI_MPEG2_AAC_AUDIO_SSRP: // ???
						mt.subtype = FOURCCMap(wfe->wFormatTag = WAVE_FORMAT_AAC);
						if(wfe->cbSize >= 2) wfe->nChannels = (((BYTE*)(wfe+1))[1]>>3) & 0xf;
						mts.Add(mt);
						break;
					case AP4_OTI_MPEG2_PART3_AUDIO: // ???
					case AP4_OTI_MPEG1_AUDIO:
						mt.subtype = FOURCCMap(wfe->wFormatTag = WAVE_FORMAT_MP3);
						{
						m_pFile->Seek(sample.GetOffset());
						CBaseSplitterFileEx::mpahdr h;
						CMediaType mt2;
						if(m_pFile->Read(h, sample.GetSize(), false, &mt2))
							mt = mt2;
						}
						mts.Add(mt);
						break;
					}

					if(mt.subtype == GUID_NULL)
					{
						TRACE(_T("Unknown audio OBI: %02x\n"), audio_desc->GetObjectTypeId());
					}
				}
				else if(AP4_MpegSystemSampleDescription* system_desc = 
					dynamic_cast<AP4_MpegSystemSampleDescription*>(desc))
				{
					const AP4_DataBuffer& di = system_desc->GetDecoderInfo();
					//if(!di) di = &empty;

					switch(system_desc->GetObjectTypeId())
					{
					case AP4_NERO_VOBSUB:
						if(di.GetDataSize() >= 16*4)
						{
							CSize size(720, 576);
							if(AP4_TkhdAtom* tkhd = dynamic_cast<AP4_TkhdAtom*>(track->GetTrakAtom()->GetChild(AP4_ATOM_TYPE_TKHD)))
							{
								size.cx = tkhd->GetWidth()>>16;
								size.cy = tkhd->GetHeight()>>16;
							}

							const AP4_Byte* pal = di.GetData();
							CAtlList<CStringA> sl;
							for(int i = 0; i < 16*4; i += 4)
							{
								BYTE y = (pal[i+1]-16)*255/219;
								BYTE u = pal[i+2];
								BYTE v = pal[i+3];
								BYTE r = (BYTE)min(max(1.0*y + 1.4022*(v-128), 0), 255);
								BYTE g = (BYTE)min(max(1.0*y - 0.3456*(u-128) - 0.7145*(v-128), 0), 255);
								BYTE b = (BYTE)min(max(1.0*y + 1.7710*(u-128), 0) , 255);
								CStringA str;
								str.Format("%02x%02x%02x", r, g, b);
								sl.AddTail(str);
							}

							CStringA hdr;
							hdr.Format(
								"# VobSub index file, v7 (do not modify this line!)\n"
								"size: %dx%d\n"
								"palette: %s\n",
								size.cx, size.cy,
								Implode(sl, ','));
							
							mt.majortype = MEDIATYPE_Subtitle;
							mt.subtype = MEDIASUBTYPE_VOBSUB;
							mt.formattype = FORMAT_SubtitleInfo;
							SUBTITLEINFO* si = (SUBTITLEINFO*)mt.AllocFormatBuffer(sizeof(SUBTITLEINFO) + hdr.GetLength());
							memset(si, 0, mt.FormatLength());
							si->dwOffset = sizeof(SUBTITLEINFO);
							strcpy_s(si->IsoLang, countof(si->IsoLang), CStringA(TrackLanguage));
							wcscpy_s(si->TrackName, countof(si->TrackName), TrackName);
							memcpy(si + 1, (LPCSTR)hdr, hdr.GetLength());
							mts.Add(mt);
						}
						break;
					}

					if(mt.subtype == GUID_NULL)
					{
						TRACE(_T("Unknown audio OBI: %02x\n"), system_desc->GetObjectTypeId());
					}
				}
				else if(desc->GetType() == AP4_SampleDescription::TYPE_AVC)
				{
					AP4_AvcSampleDescription* avc1 = dynamic_cast<AP4_AvcSampleDescription*>(desc);
					
					const AP4_DataBuffer& di = avc1->GetRawBytes();

					const AP4_Byte* data = di.GetData();
					AP4_Size size = di.GetDataSize();

					mt.majortype = MEDIATYPE_Video;
					mt.subtype = FOURCCMap('1cva');
					mt.formattype = FORMAT_MPEG2Video;

					MPEG2VIDEOINFO* vih = (MPEG2VIDEOINFO*)mt.AllocFormatBuffer(FIELD_OFFSET(MPEG2VIDEOINFO, dwSequenceHeader) + size - 7);
					memset(vih, 0, mt.FormatLength());
					vih->hdr.bmiHeader.biSize = sizeof(vih->hdr.bmiHeader);
					vih->hdr.bmiHeader.biWidth = (LONG)avc1->GetWidth();
					vih->hdr.bmiHeader.biHeight = (LONG)avc1->GetHeight();
					vih->hdr.bmiHeader.biCompression = '1cva';
					vih->hdr.bmiHeader.biPlanes = 1;
					vih->hdr.bmiHeader.biBitCount = 24;
					vih->hdr.dwPictAspectRatioX = vih->hdr.bmiHeader.biWidth;
					vih->hdr.dwPictAspectRatioY = vih->hdr.bmiHeader.biHeight;
					if (item->GetData()->GetSampleCount() > 1)
						vih->hdr.AvgTimePerFrame = MulDiv(item->GetData()->GetDurationMs(), 10000, item->GetData()->GetSampleCount()-1);
					vih->dwProfile = data[1];
					vih->dwLevel = data[3];
					vih->dwFlags = (data[4] & 3) + 1;

					vih->cbSequenceHeader = 0;

					BYTE* src = (BYTE*)data + 5;
					BYTE* dst = (BYTE*)vih->dwSequenceHeader;

					BYTE* src_end = (BYTE*)data + size;
					BYTE* dst_end = (BYTE*)vih->dwSequenceHeader + size;

					for(int i = 0; i < 2; i++)
					{
						for(int n = *src++ & 0x1f; n > 0; n--)
						{
							int len = ((src[0] << 8) | src[1]) + 2;
							if(src + len > src_end || dst + len > dst_end) {ASSERT(0); break;}
							memcpy(dst, src, len);
							src += len; 
							dst += len;
							vih->cbSequenceHeader += len;
						}
					}

					mts.Add(mt);

					mt.subtype = FOURCCMap(vih->hdr.bmiHeader.biCompression = '1CVA');
					mts.Add(mt);
				}
			}
			if(AP4_StsdAtom* stsd = dynamic_cast<AP4_StsdAtom*>(
				track->GetTrakAtom()->FindChild("mdia/minf/stbl/stsd")))
			{

				for(AP4_List<AP4_Atom>::Item* item = stsd->GetChildren().FirstItem(); 
					item; 
					item = item->GetNext())
				{
					AP4_Atom* atom = item->GetData();
					AP4_DataBuffer db;

					GetDataBuffer (atom, db);

					AP4_Atom::Type type = atom->GetType();

					if((type & 0xffff0000) == AP4_ATOM_TYPE('m', 's', 0, 0))
					{
						type &= 0xffff;
					}
					else if(type == AP4_ATOM_TYPE__MP3)
					{
						type = 0x0055;
					}
					else if((type == AP4_ATOM_TYPE_AC_3) || (type == AP4_ATOM_TYPE_SAC3))
					{
						type = 0x2000;
					}
					else
					{
						type = 
							((type >> 24) & 0x000000ff) |
							((type >>  8) & 0x0000ff00) |
							((type <<  8) & 0x00ff0000) |
							((type << 24) & 0xff000000);
					}

					if(AP4_VisualSampleEntry* vse = dynamic_cast<AP4_VisualSampleEntry*>(atom))
					{
						mt.majortype = MEDIATYPE_Video;
						mt.subtype = FOURCCMap(type);
						mt.formattype = FORMAT_VideoInfo;
						vih = (VIDEOINFOHEADER*)mt.AllocFormatBuffer(sizeof(VIDEOINFOHEADER)+db.GetDataSize());
						memset(vih, 0, mt.FormatLength());
						vih->bmiHeader.biSize = sizeof(vih->bmiHeader);
						vih->bmiHeader.biWidth = (LONG)vse->GetWidth();
						vih->bmiHeader.biHeight = (LONG)vse->GetHeight();
						vih->bmiHeader.biCompression = type;
						memcpy(vih+1, db.GetData(), db.GetDataSize());
						mts.Add(mt);

						char buff[5];
						memcpy(buff, &type, 4);
						buff[4] = 0;

						strlwr((char*)&buff);
						AP4_Atom::Type typelwr = *(AP4_Atom::Type*)buff;

						if(typelwr != type)
						{
							mt.subtype = FOURCCMap(vih->bmiHeader.biCompression = typelwr);
							mts.Add(mt);
						}

						strupr((char*)&buff);
						AP4_Atom::Type typeupr = *(AP4_Atom::Type*)buff;

						if(typeupr != type)
						{
							mt.subtype = FOURCCMap(vih->bmiHeader.biCompression = typeupr);
							mts.Add(mt);
						}

						break;
					}
					else if(AP4_AudioSampleEntry* ase = dynamic_cast<AP4_AudioSampleEntry*>(atom))
					{
						mt.majortype = MEDIATYPE_Audio;
						mt.subtype = FOURCCMap(type);
						mt.formattype = FORMAT_WaveFormatEx;
						wfe = (WAVEFORMATEX*)mt.AllocFormatBuffer(sizeof(WAVEFORMATEX) + db.GetDataSize());
						memset(wfe, 0, mt.FormatLength());
						if(!(type & 0xffff0000)) wfe->wFormatTag = (WORD)type;
						wfe->nSamplesPerSec = ase->GetSampleRate();
						wfe->nChannels = ase->GetChannelCount();
						wfe->wBitsPerSample = ase->GetSampleSize();
						wfe->nBlockAlign = ase->GetBytesPerFrame();
						wfe->cbSize = db.GetDataSize();
						memcpy(wfe+1, db.GetData(), db.GetDataSize());
						mts.Add(mt);
						break;
					}

					else if(dynamic_cast<AP4_TextSampleEntry*>(atom)
						|| dynamic_cast<AP4_Tx3gSampleEntry*>(atom))
					{
						mt.majortype = MEDIATYPE_Subtitle;
						mt.subtype = MEDIASUBTYPE_ASS2;
						mt.formattype = FORMAT_SubtitleInfo;
						CStringA hdr;
						hdr.Format(
							"[Script Info]\n"
							"ScriptType: v4.00++\n"
							"ScaledBorderAndShadow: yes\n"
							"PlayResX: %d\n"
							"PlayResY: %d\n"
							"[V4++ Styles]\n"
							"Style: Text,Arial,12,&H00ffffff,&H0000ffff,&H00000000,&H80000000,0,0,0,0,100,100,0,0.00,3,0,0,2,0,0,0,0,1,1\n",
							// "Style: Text,Arial,12,&H00ffffff,&H0000ffff,&H00000000,&H80000000,0,0,0,0,100,100,0,0.00,1,0,0,2,0,0,0,0,1,1\n",
							m_framesize.cx, 
							m_framesize.cy);
						SUBTITLEINFO* si = (SUBTITLEINFO*)mt.AllocFormatBuffer(sizeof(SUBTITLEINFO) + hdr.GetLength());
						memset(si, 0, mt.FormatLength());
						si->dwOffset = sizeof(SUBTITLEINFO);
						strcpy_s(si->IsoLang, countof(si->IsoLang), CStringA(TrackLanguage));
						wcscpy_s(si->TrackName, countof(si->TrackName), TrackName);
						memcpy(si + 1, (LPCSTR)hdr, hdr.GetLength());
						mts.Add(mt);
					}

				}
			}

			if(mts.IsEmpty()) continue;

			REFERENCE_TIME rtDuration = 10000i64 * track->GetDurationMs();
			if(m_rtDuration < rtDuration) m_rtDuration = rtDuration;

			DWORD id = track->GetId();

			CStringW name, lang;
			name.Format(L"Output %d", id);

			if(!TrackName.IsEmpty())
			{
				name = TrackName;
			}

			if(!TrackLanguage.IsEmpty())
			{
				if(TrackLanguage != L"und") name += " (" + TrackLanguage + ")";
			}

			for(int i = 0, j = mts.GetCount(); i < j; i++)
			{
				BITMAPINFOHEADER bih;
				if(ExtractBIH(&mts[i], &bih))
				{
					m_framesize.cx = bih.biWidth;
					m_framesize.cy = abs(bih.biHeight);
				}
			}

			CAutoPtr<CBaseSplitterOutputPin> pPinOut(DNew CBaseSplitterOutputPin(mts, name, this, this, &hr));

			if(!TrackName.IsEmpty()) pPinOut->SetProperty(L"NAME", TrackName);
			if(!TrackLanguage.IsEmpty()) pPinOut->SetProperty(L"LANG", CStringW(TrackLanguage));

			EXECUTE_ASSERT(SUCCEEDED(AddOutputPin(id, pPinOut)));

			m_trackpos[id] = trackpos();

			if(mts.GetCount() == 1 && mts[0].subtype == MEDIASUBTYPE_ASS2)
			{
				LPCWSTR postfix = L" (plain text)";

				mts[0].subtype = MEDIASUBTYPE_UTF8;

				SUBTITLEINFO* si = (SUBTITLEINFO*)mts[0].ReallocFormatBuffer(sizeof(SUBTITLEINFO));
				wcscat(si->TrackName, postfix);

				id ^= 0x80402010; // FIXME: until fixing, let's hope there won't be another track like this...

				CAutoPtr<CBaseSplitterOutputPin> pPinOut(DNew CBaseSplitterOutputPin(mts, name + postfix, this, this, &hr));

				if(!TrackName.IsEmpty()) pPinOut->SetProperty(L"NAME", TrackName + postfix);
				if(!TrackLanguage.IsEmpty()) pPinOut->SetProperty(L"LANG", CStringW(TrackLanguage));

				EXECUTE_ASSERT(SUCCEEDED(AddOutputPin(id, pPinOut)));
			}
		}

		if(AP4_ChplAtom* chpl = dynamic_cast<AP4_ChplAtom*>(movie->GetMoovAtom()->FindChild("udta/chpl")))
		{
			AP4_Array<AP4_ChplAtom::AP4_Chapter>& chapters = chpl->GetChapters();

			for(AP4_Cardinal i = 0; i < chapters.ItemCount(); i++)
			{
				AP4_ChplAtom::AP4_Chapter& chapter = chapters[i];
				ChapAppend(chapter.Time, UTF8To16(ConvertMBCS(chapter.Name.GetChars(), ANSI_CHARSET, CP_UTF8))); // this is b0rked, thx to nero :P
			}

			ChapSort();
		}

		if(AP4_ContainerAtom* ilst = dynamic_cast<AP4_ContainerAtom*>(movie->GetMoovAtom()->FindChild("udta/meta/ilst")))
		{
			CStringW title, artist, writer, album, year, appl, desc, gen, track;

			for(AP4_List<AP4_Atom>::Item* item = ilst->GetChildren().FirstItem();
				item;
				item = item->GetNext())
			{
				if(AP4_ContainerAtom* atom = dynamic_cast<AP4_ContainerAtom*>(item->GetData()))
				{
					if(AP4_DataAtom* data = dynamic_cast<AP4_DataAtom*>(atom->GetChild(AP4_ATOM_TYPE_DATA)))
					{
						// TODO : BENTOMIGRATION
						//const AP4_DataBuffer* db = data->GetData();

						//if(atom->GetType() == AP4_ATOM_TYPE_TRKN)
						//{
						//	if(db->GetDataSize() >= 4)
						//	{
						//		unsigned short n = (db->GetData()[2] << 8) | db->GetData()[3];
						//		if(n > 0 && n < 100) track.Format(L"%02d", n);
						//		else if(n >= 100) track.Format(L"%d", n);
						//	}
						//}
						//else
						//{
						//	CStringW str = UTF8To16(CStringA((LPCSTR)db->GetData(), db->GetDataSize()));

						//	switch(atom->GetType())
						//	{
						//	case AP4_ATOM_TYPE_NAM: title = str; break;
						//	case AP4_ATOM_TYPE_ART: artist = str; break;
						//	case AP4_ATOM_TYPE_WRT: writer = str; break;
						//	case AP4_ATOM_TYPE_ALB: album = str; break;
						//	case AP4_ATOM_TYPE_DAY: year = str; break;
						//	case AP4_ATOM_TYPE_TOO: appl = str; break;
						//	case AP4_ATOM_TYPE_CMT: desc = str; break;
						//	case AP4_ATOM_TYPE_GEN: gen = str; break;
						//	}
						//}
					}
				}
			}

			if(!title.IsEmpty())
			{
				if(!track.IsEmpty()) title = track + L" - " + title;
				if(!album.IsEmpty()) title = album + L" - " + title;
				if(!year.IsEmpty()) title += L" - " +  year;
				if(!gen.IsEmpty()) title += L" - " + gen;
				SetProperty(L"TITL", title);
			}

			if(!artist.IsEmpty()) SetProperty(L"AUTH", artist);
			else if(!writer.IsEmpty()) SetProperty(L"AUTH", writer);

			if(!appl.IsEmpty()) SetProperty(L"APPL", appl);

			if(!desc.IsEmpty()) SetProperty(L"DESC", desc);
		}
	}

	m_rtNewStop = m_rtStop = m_rtDuration;

	return m_pOutputs.GetCount() > 0 ? S_OK : E_FAIL;
}

bool CMP4SplitterFilter::DemuxInit()
{
	SetThreadName((DWORD)-1, "CMP4SplitterFilter");
	AP4_Movie* movie = (AP4_Movie*)m_pFile->GetMovie();

	POSITION pos = m_trackpos.GetStartPosition();
	while(pos)
	{
		CAtlMap<AP4_Ordinal, trackpos>::CPair* pPair = m_trackpos.GetNext(pos);

		pPair->m_value.index = 0;
		pPair->m_value.ts = 0;

		AP4_Track* track = movie->GetTrack(pPair->m_key);

		AP4_Sample sample;
		if(AP4_SUCCEEDED(track->GetSample(0, sample)))
			pPair->m_value.ts = sample.GetCts();
	}

	return true;
}

void CMP4SplitterFilter::DemuxSeek(REFERENCE_TIME rt)
{
	REFERENCE_TIME ts = (rt / 10000);

	AP4_Movie* movie = (AP4_Movie*)m_pFile->GetMovie();

	POSITION pos = m_trackpos.GetStartPosition();
	while(pos)
	{
		CAtlMap<AP4_Ordinal, trackpos>::CPair* pPair = m_trackpos.GetNext(pos);

		AP4_Track* track = movie->GetTrack(pPair->m_key);

		if(AP4_FAILED(track->GetSampleIndexForTimeStampMs(ts, pPair->m_value.index)))
			pPair->m_value.index = 0;

		AP4_Sample sample;
		if(AP4_SUCCEEDED(track->GetSample(pPair->m_value.index, sample)))
			pPair->m_value.ts = sample.GetCts();

		// FIXME: slow search & stss->m_Entries is private

		if(AP4_StssAtom* stss = dynamic_cast<AP4_StssAtom*>(track->GetTrakAtom()->FindChild("mdia/minf/stbl/stss")))
		{
			const AP4_Array<AP4_UI32>&	Entries = stss->GetEntries();
			if(Entries.ItemCount() > 0)
			{
				AP4_Cardinal i = (AP4_Cardinal)-1;
				while(++i < Entries.ItemCount() && Entries[i]-1 <= pPair->m_value.index);
				if(i > 0) i--;
				pPair->m_value.index = Entries[i]-1;
			}
		}
	}
}

struct SSACharacter {CString pre, post; WCHAR c;}; 

static CStringW ConvertTX3GToSSA(
	CStringW str, 
	const AP4_Tx3gSampleEntry::AP4_Tx3gDescription& desc, 
	CStringW font,
	const AP4_Byte* mods, 
	int size, 
	CSize framesize,
	CPoint translation,
	int durationms,
	CRect& rbox)
{
	int str_len = str.GetLength();

	SSACharacter* chars = DNew SSACharacter[str_len];
	for(int i = 0; i < str_len; i++) chars[i].c = str[i];
	str.Empty();

	//

	rbox.SetRect(desc.TextBox.Left, desc.TextBox.Top, desc.TextBox.Right, desc.TextBox.Bottom);

	int align = 2;
	signed char h = (signed char)desc.HorizontalJustification;
	signed char v = (signed char)desc.VerticalJustification;
	if(h == 0 && v < 0) align = 1;
	else if(h > 0 && v < 0) align = 2;
	else if(h < 0 && v < 0) align = 3;
	else if(h == 0 && v > 0) align = 4;
	else if(h > 0 && v > 0) align = 5;
	else if(h < 0 && v > 0) align = 6;
	else if(h == 0 && v == 0) align = 7;
	else if(h > 0 && v == 0) align = 8;
	else if(h < 0 && v == 0) align = 9;
	str.Format(L"{\\an%d}%s", align, CStringW(str));

	if(!font.CompareNoCase(L"serif")) font = L"Times New Roman";
	else if(!font.CompareNoCase(L"sans-serif")) font = L"Arial";
	else if(!font.CompareNoCase(L"monospace")) font = L"Courier New";
	str.Format(L"{\\fn%s}%s", font, CStringW(str));	

	const AP4_Byte* fclr = (const AP4_Byte*)&desc.Style.Font.Color;

	CStringW font_color;
	font_color.Format(L"{\\1c%02x%02x%02x\\1a%02x}", fclr[2], fclr[1], fclr[0], 255 - fclr[3]);
	str = font_color + str;

	str.Format(L"%s{\\2c%02x%02x%02x\\2a%02x}", CString(str), fclr[2], fclr[1], fclr[0], 255 - fclr[3]);

	CStringW font_size;
	font_size.Format(L"{\\fs%d}", desc.Style.Font.Size);
	str = font_size + str;

	CStringW font_flags;
	font_flags.Format(L"{\\b%d\\i%d\\u%d}", 
		(desc.Style.Font.Face&1) ? 1 : 0,
		(desc.Style.Font.Face&2) ? 1 : 0,
		(desc.Style.Font.Face&4) ? 1 : 0);
	str = font_flags + str;

	//

	const AP4_Byte* hclr = (const AP4_Byte*)&desc.BackgroundColor;

	while(size > 8)
	{
		DWORD tag_size = (mods[0]<<24)|(mods[1]<<16)|(mods[2]<<8)|(mods[3]); mods += 4;
		DWORD tag = (mods[0]<<24)|(mods[1]<<16)|(mods[2]<<8)|(mods[3]); mods += 4;

		size -= tag_size;
		tag_size -= 8;
		const AP4_Byte* next = mods + tag_size;

		if(tag == 'styl')
		{
			WORD styl_count = (mods[0]<<8)|(mods[1]); mods += 2;

			while(styl_count-- > 0)
			{
				WORD start = (mods[0]<<8)|(mods[1]); mods += 2;
				WORD end = (mods[0]<<8)|(mods[1]); mods += 2;
				WORD font_id = (mods[0]<<8)|(mods[1]); mods += 2;
				WORD flags = mods[0]; mods += 1;
				WORD size = mods[0]; mods += 1;
				const AP4_Byte* color = mods; mods += 4;
				UNUSED_ALWAYS(font_id);

				if(end > str_len) end = str_len;

				if(start < end)
				{
					CStringW s;

					s.Format(L"{\\1c%02x%02x%02x\\1a%02x}", color[2], color[1], color[0], 255 - color[3]);
					chars[start].pre += s;
					chars[end-1].post += font_color;

					s.Format(L"{\\fs%d}", size);
					chars[start].pre += s;
					chars[end-1].post += font_size;

					s.Format(L"{\\b%d\\i%d\\u%d}", (flags&1) ? 1 : 0, (flags&2) ? 1 : 0, (flags&4) ? 1 : 0);
					chars[start].pre += s; 
					chars[end-1].post += font_flags;
				}
			}
		}
		else if(tag == 'hclr')
		{
			hclr = mods;
		}
		else if(tag == 'hlit')
		{
			WORD start = (mods[0]<<8)|(mods[1]); mods += 2;
			WORD end = (mods[0]<<8)|(mods[1]); mods += 2;

			if(end > str_len) end = str_len;

			if(start < end)
			{
				CStringW s;

				s.Format(L"{\\3c%02x%02x%02x\\3a%02x}", hclr[2], hclr[1], hclr[0], 255 - hclr[3]);
				chars[start].pre += s;
				chars[end-1].post += font_color;

				chars[start].pre += L"{\\bord0.1}";
				chars[end-1].post += L"{\\bord}";
			}
		}
		else if(tag == 'blnk')
		{
			WORD start = (mods[0]<<8)|(mods[1]); mods += 2;
			WORD end = (mods[0]<<8)|(mods[1]); mods += 2;

			if(end > str_len) end = str_len;

			if(start < end)
			{
				// cheap...

				for(int i = 0, alpha = 255; i < durationms; i += 750, alpha = 255 - alpha)
				{
					CStringW s;
					s.Format(L"{\\t(%d, %d, \\alpha&H%02x&)}", i, i + 750, alpha);
					chars[start].pre += s;
				}

				chars[end-1].post += L"{\\alpha}";
			}
		}
		else if(tag == 'tbox')
		{
			rbox.top = (mods[0]<<8)|(mods[1]); mods += 2;
			rbox.left = (mods[0]<<8)|(mods[1]); mods += 2;
			rbox.bottom = (mods[0]<<8)|(mods[1]); mods += 2;
			rbox.right = (mods[0]<<8)|(mods[1]); mods += 2;
		}
		else if(tag == 'krok' && !(desc.DisplayFlags & 0x800))
		{
			DWORD start_time = (mods[0]<<24)|(mods[1]<<16)|(mods[2]<<8)|(mods[3]); mods += 4;
			WORD krok_count = (mods[0]<<8)|(mods[1]); mods += 2;

			while(krok_count-- > 0)
			{
				DWORD end_time = (mods[0]<<24)|(mods[1]<<16)|(mods[2]<<8)|(mods[3]); mods += 4;
				WORD start = (mods[0]<<8)|(mods[1]); mods += 2;
				WORD end = (mods[0]<<8)|(mods[1]); mods += 2;

				if(end > str_len) end = str_len;

				if(start < end)
				{
					CStringW s;

					s.Format(L"{\\kt%d\\kf%d}", start_time/10, (end_time - start_time)/10);
					chars[start].pre += s;
					s.Format(L"{\\1c%02x%02x%02x\\1a%02x}", hclr[2], hclr[1], hclr[0], 255 - hclr[3]);
					chars[start].pre += s;
					chars[end-1].post += L"{\\kt}" + font_color;
				}

				start_time = end_time;
			}
		}

		mods = next;
	}

	// continous karaoke

	if(desc.DisplayFlags & 0x800)
	{
		CStringW s;

		s.Format(L"{\\1c%02x%02x%02x\\1a%02x}", hclr[2], hclr[1], hclr[0], 255 - hclr[3]);
		str += s;

		int breaks = 0;

		for(int i = 0; i <= str_len; i++)
		{
			if(chars[i].c == '\n' /*|| chars[i].c == ' '*/)
			{
				breaks++;
			}
		}

		if(str_len > breaks)
		{
			float dur = (float)max(durationms - 500, 0) / 10;

			for(int i = 0, j = 0; i <= str_len; i++)
			{
				if(i == str_len || chars[i].c == '\n' /*|| chars[i].c == ' '*/)
				{
					s.Format(L"{\\kf%d}", (int)(dur * (i - j) / (str_len - breaks)));
					chars[j].pre += s;
					j = i+1;
				}
			}
		}
	}

	//

	for(int i = 0; i < str_len; i++) 
	{
		str += chars[i].pre;
		str += chars[i].c;
		if(desc.DisplayFlags & 0x20000) str += L"\\N";
		str += chars[i].post;
	}

	delete [] chars;

	//

	if(rbox.IsRectEmpty()) rbox.SetRect(0, 0, framesize.cx, framesize.cy);
	rbox.OffsetRect(translation);

	CRect rbox2 = rbox;
	rbox2.DeflateRect(2, 2);

	CRect r(0,0,0,0);
	if(rbox2.Height() > 0) {r.top = rbox2.top; r.bottom = framesize.cy - rbox2.bottom;}
	if(rbox2.Width() > 0) {r.left = rbox2.left; r.right = framesize.cx - rbox2.right;}

	CStringW hdr;
	hdr.Format(L"0,0,Text,,%d,%d,%d,%d,,{\\clip(%d,%d,%d,%d)}", 
		r.left, r.right, r.top, r.bottom,
		rbox.left, rbox.top, rbox.right, rbox.bottom);

	//

	return hdr + str;
}

bool CMP4SplitterFilter::DemuxLoop()
{
	HRESULT hr = S_OK;

	AP4_Movie* movie = (AP4_Movie*)m_pFile->GetMovie();

	while(SUCCEEDED(hr) && !CheckRequest(NULL))
	{
		CAtlMap<AP4_Ordinal, trackpos>::CPair* pPairNext = NULL;
		REFERENCE_TIME rtNext = 0;

		POSITION pos = m_trackpos.GetStartPosition();
		while(pos)
		{
			CAtlMap<AP4_Ordinal, trackpos>::CPair* pPair = m_trackpos.GetNext(pos);

			AP4_Track* track = movie->GetTrack(pPair->m_key);

			CBaseSplitterOutputPin* pPin = GetOutputPin((DWORD)track->GetId());
			if(!pPin->IsConnected()) continue;

			REFERENCE_TIME rt = (REFERENCE_TIME)(10000000.0 / track->GetMediaTimeScale() * pPair->m_value.ts);

			if(pPair->m_value.index < track->GetSampleCount() && (!pPairNext || rt < rtNext))
			{
				pPairNext = pPair;
				rtNext = rt;
			}
		}

		if(!pPairNext) break;

		AP4_Track* track = movie->GetTrack(pPairNext->m_key);

		CBaseSplitterOutputPin* pPin = GetOutputPin((DWORD)track->GetId());

		AP4_Sample sample;
		AP4_DataBuffer data;

		if(pPin && pPin->IsConnected() && AP4_SUCCEEDED(track->ReadSample(pPairNext->m_value.index, sample, data)))
		{
			const CMediaType& mt = pPin->CurrentMediaType();

			CAutoPtr<Packet> p(DNew Packet());
			p->TrackNumber = (DWORD)track->GetId();
			p->rtStart = (REFERENCE_TIME)(10000000.0 / track->GetMediaTimeScale() * sample.GetCts());
			p->rtStop = p->rtStart + (REFERENCE_TIME)(10000000.0 / track->GetMediaTimeScale() * sample.GetDuration());
			p->bSyncPoint = TRUE;

			// FIXME: slow search & stss->m_Entries is private

			if(AP4_StssAtom* stss = dynamic_cast<AP4_StssAtom*>(track->GetTrakAtom()->FindChild("mdia/minf/stbl/stss")))
			{
				if(stss->GetEntries().ItemCount() > 0)
				{
					p->bSyncPoint = FALSE;

					AP4_Cardinal i = (AP4_Cardinal)-1;
					while(++i < stss->GetEntries().ItemCount())
						if(stss->GetEntries()[i]-1 == pPairNext->m_value.index)
							p->bSyncPoint = TRUE;
				}
			}

			//

			if(track->GetType() == AP4_Track::TYPE_AUDIO && data.GetDataSize() == 1)
			{
				WAVEFORMATEX* wfe = (WAVEFORMATEX*)mt.Format();

				int nBlockAlign = 1200;

				if(wfe->nBlockAlign > 1)
				{
					nBlockAlign = wfe->nBlockAlign;
					pPairNext->m_value.index -= pPairNext->m_value.index % wfe->nBlockAlign;
				}

//				p->rtStop = p->rtStart;
				AP4_UI32	nAP4SampleDuration = 0;

				int fFirst = true;

				while(AP4_SUCCEEDED(track->ReadSample(pPairNext->m_value.index, sample, data)))
				{
					AP4_Size size = data.GetDataSize();
					const AP4_Byte* ptr = data.GetData();
					for(int i = 0; i < size; i++) p->Add(ptr[i]);

//int x = p->GetCount();
//if (x>4 && p->GetAt(x-4)==0x91  && p->GetAt(x-3)==0x11  && p->GetAt(x-2)==0x19  && p->GetAt(x-1)==0x90)
//	printf("gotcha");

					if(fFirst) {p->rtStart = p->rtStop = (REFERENCE_TIME)(10000000.0 / track->GetMediaTimeScale() * sample.GetCts()); fFirst = false;}
					nAP4SampleDuration += sample.GetDuration();

					if(pPairNext->m_value.index+1 >= track->GetSampleCount() || p->GetCount() >= nBlockAlign)
						break;

					pPairNext->m_value.index++;
				}

				p->rtStop = p->rtStart + (REFERENCE_TIME)(10000000.0 / track->GetMediaTimeScale() * nAP4SampleDuration);
			}
			else if(track->GetType() == AP4_Track::TYPE_TEXT)
			{
				CStringA dlgln_bkg, dlgln_plaintext;

				const AP4_Byte* ptr = data.GetData();
				AP4_Size avail = data.GetDataSize();

				if(avail > 2)
				{
					AP4_UI16 size = (ptr[0] << 8) | ptr[1];

					if(size <= avail-2)
					{
						CStringA str;

						if(size >= 2 && ptr[2] == 0xfe && ptr[3] == 0xff)
						{
							CStringW wstr = CStringW((LPCWSTR)&ptr[2], size/2);
							for(int i = 0; i < wstr.GetLength(); i++) wstr.SetAt(i, ((WORD)wstr[i] >> 8) | ((WORD)wstr[i] << 8));
							str = UTF16To8(wstr);
						}
						else
						{
							str = CStringA((LPCSTR)&ptr[2], size);
						}

						CStringA dlgln = str;

						if(mt.subtype == MEDIASUBTYPE_ASS2)
						{
							AP4_SampleDescription* desc = track->GetSampleDescription(sample.GetDescriptionIndex());
							UNUSED_ALWAYS(desc);

							dlgln = "0,0,Text,,0000,0000,0000,0000,," + str;
							dlgln_plaintext = str;

							CPoint translation(0, 0);
							if(AP4_TkhdAtom* tkhd = dynamic_cast<AP4_TkhdAtom*>(track->GetTrakAtom()->GetChild(AP4_ATOM_TYPE_TKHD)))
							{
								float x, y;
								tkhd->GetTranslation(x, y);
								translation.SetPoint((int)x, (int)y);
							}
						}

						dlgln.Replace("\r", "");
						dlgln.Replace("\n", "\\N");

						p->SetData((LPCSTR)dlgln, dlgln.GetLength());
					}
				}

				if(!dlgln_bkg.IsEmpty())
				{
					CAutoPtr<Packet> p2(DNew Packet());
					p2->TrackNumber = p->TrackNumber;
					p2->rtStart = p->rtStart;
					p2->rtStop = p->rtStop;
					p2->bSyncPoint = p->bSyncPoint;
					p2->SetData((LPCSTR)dlgln_bkg, dlgln_bkg.GetLength());
					hr = DeliverPacket(p2);
				}

				if(!dlgln_plaintext.IsEmpty())
				{
					CAutoPtr<Packet> p2(DNew Packet());
					p2->TrackNumber = p->TrackNumber ^ 0x80402010;
					p2->rtStart = p->rtStart;
					p2->rtStop = p->rtStop;
					p2->bSyncPoint = p->bSyncPoint;
					p2->SetData((LPCSTR)dlgln_plaintext, dlgln_plaintext.GetLength());
					hr = DeliverPacket(p2);
				}
			}
			else
			{
				p->SetData(data.GetData(), data.GetDataSize());
			}

			hr = DeliverPacket(p);
		}

		{
			AP4_Sample sample;
			if(AP4_SUCCEEDED(track->GetSample(++pPairNext->m_value.index, sample)))
				pPairNext->m_value.ts = sample.GetCts();
		}

	}

	return true;
}

// IKeyFrameInfo

STDMETHODIMP CMP4SplitterFilter::GetKeyFrameCount(UINT& nKFs)
{
	CheckPointer(m_pFile, E_UNEXPECTED);

	if(!m_pFile) return E_UNEXPECTED;

	AP4_Movie* movie = (AP4_Movie*)m_pFile->GetMovie();

	POSITION pos = m_trackpos.GetStartPosition();
	while(pos)
	{
		CAtlMap<AP4_Ordinal, trackpos>::CPair* pPair = m_trackpos.GetNext(pos);

		AP4_Track* track = movie->GetTrack(pPair->m_key);

		if(track->GetType() != AP4_Track::TYPE_VIDEO)
			continue;

		if(AP4_StssAtom* stss = dynamic_cast<AP4_StssAtom*>(track->GetTrakAtom()->FindChild("mdia/minf/stbl/stss")))
		{
			nKFs = stss->GetEntries().ItemCount();
			return S_OK;
		}
	}

	return E_FAIL;
}

STDMETHODIMP CMP4SplitterFilter::GetKeyFrames(const GUID* pFormat, REFERENCE_TIME* pKFs, UINT& nKFs)
{
	CheckPointer(pFormat, E_POINTER);
	CheckPointer(pKFs, E_POINTER);
	CheckPointer(m_pFile, E_UNEXPECTED);

	if(*pFormat != TIME_FORMAT_MEDIA_TIME) return E_INVALIDARG;

	if(!m_pFile) return E_UNEXPECTED;

	AP4_Movie* movie = (AP4_Movie*)m_pFile->GetMovie();

	POSITION pos = m_trackpos.GetStartPosition();
	while(pos)
	{
		CAtlMap<AP4_Ordinal, trackpos>::CPair* pPair = m_trackpos.GetNext(pos);

		AP4_Track* track = movie->GetTrack(pPair->m_key);

		if(track->GetType() != AP4_Track::TYPE_VIDEO)
			continue;

		if(AP4_StssAtom* stss = dynamic_cast<AP4_StssAtom*>(track->GetTrakAtom()->FindChild("mdia/minf/stbl/stss")))
		{
			nKFs = 0;

			for(AP4_Cardinal i = 0; i < stss->GetEntries().ItemCount(); i++)
			{
				AP4_Sample sample;
				if(AP4_SUCCEEDED(track->GetSample(stss->GetEntries()[i]-1, sample)))
					pKFs[nKFs++] = 10000000i64 * sample.GetCts() / track->GetMediaTimeScale();
			}

			return S_OK;
		}
	}

	return E_FAIL;
}

//
// CMP4SourceFilter
//

CMP4SourceFilter::CMP4SourceFilter(LPUNKNOWN pUnk, HRESULT* phr)
	: CMP4SplitterFilter(pUnk, phr)
{
	m_clsid = __uuidof(this);
	m_pInput.Free();
}

//
// CMPEG4VideoSplitterFilter
//

CMPEG4VideoSplitterFilter::CMPEG4VideoSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr)
	: CBaseSplitterFilter(NAME("CMPEG4VideoSplitterFilter"), pUnk, phr, __uuidof(this))
{
}

void CMPEG4VideoSplitterFilter::SkipUserData()
{
	m_pFile->BitByteAlign();
	while(m_pFile->BitRead(32, true) == 0x000001b2)
		while(m_pFile->BitRead(24, true) != 0x000001)
			m_pFile->BitRead(8);
}

HRESULT CMPEG4VideoSplitterFilter::CreateOutputs(IAsyncReader* pAsyncReader)
{
	CheckPointer(pAsyncReader, E_POINTER);

	HRESULT hr = E_FAIL;

	m_pFile.Free();
	m_pFile.Attach(DNew CBaseSplitterFileEx(pAsyncReader, hr));
	if(!m_pFile) return E_OUTOFMEMORY;
	if(FAILED(hr)) {m_pFile.Free(); return hr;}

	m_rtNewStart = m_rtCurrent = 0;
	m_rtNewStop = m_rtStop = m_rtDuration = 0;

	// TODO

	DWORD width = 0;
	DWORD height = 0;
	BYTE parx = 1;
	BYTE pary = 1;
	REFERENCE_TIME atpf = 400000;

	if(m_pFile->BitRead(24, true) != 0x000001)
		return E_FAIL;

	BYTE id;
	while(m_pFile->NextMpegStartCode(id, 1024 - m_pFile->GetPos()))
	{
		if(id == 0xb5)
		{
			BYTE is_visual_object_identifier = (BYTE)m_pFile->BitRead(1);

			if(is_visual_object_identifier)
			{
				BYTE visual_object_verid = (BYTE)m_pFile->BitRead(4);
				BYTE visual_object_priority = (BYTE)m_pFile->BitRead(3);
				UNUSED_ALWAYS(visual_object_verid);
				UNUSED_ALWAYS(visual_object_priority);
			}

			BYTE visual_object_type = (BYTE)m_pFile->BitRead(4);
			
			if(visual_object_type == 1 || visual_object_type == 2)
			{
				BYTE video_signal_type = (BYTE)m_pFile->BitRead(1);
				
				if(video_signal_type)
				{
					BYTE video_format = (BYTE)m_pFile->BitRead(3);
					BYTE video_range = (BYTE)m_pFile->BitRead(1);
					BYTE colour_description = (BYTE)m_pFile->BitRead(1);
					UNUSED_ALWAYS(video_format);
					UNUSED_ALWAYS(video_range);
					UNUSED_ALWAYS(colour_description);

					if(colour_description)
					{
						BYTE colour_primaries = (BYTE)m_pFile->BitRead(8);
						BYTE transfer_characteristics = (BYTE)m_pFile->BitRead(8);
						BYTE matrix_coefficients = (BYTE)m_pFile->BitRead(8);
						UNUSED_ALWAYS(colour_primaries);
						UNUSED_ALWAYS(transfer_characteristics);
						UNUSED_ALWAYS(matrix_coefficients);
					}
				}
			}

			SkipUserData();

			if(visual_object_type == 1)
			{
				if(m_pFile->BitRead(24) != 0x000001)
					break;

				BYTE video_object_start_code = (BYTE)m_pFile->BitRead(8);
				if(video_object_start_code < 0x00 || video_object_start_code > 0x1f)
					break;

				if(m_pFile->BitRead(24) != 0x000001)
					break;

				BYTE video_object_layer_start_code = (DWORD)m_pFile->BitRead(8);
				if(video_object_layer_start_code < 0x20 || video_object_layer_start_code > 0x2f)
					break;

				BYTE random_accessible_vol = (BYTE)m_pFile->BitRead(1);
				BYTE video_object_type_indication = (BYTE)m_pFile->BitRead(8);
				UNUSED_ALWAYS(random_accessible_vol);

				if(video_object_type_indication == 0x12) // Fine Granularity Scalable
					break; // huh

				BYTE is_object_layer_identifier = (BYTE)m_pFile->BitRead(1);

				BYTE video_object_layer_verid = 0;

				if(is_object_layer_identifier)
				{
					video_object_layer_verid = (BYTE)m_pFile->BitRead(4);
					BYTE video_object_layer_priority = (BYTE)m_pFile->BitRead(3);
				}

				BYTE aspect_ratio_info = (BYTE)m_pFile->BitRead(4);

				switch(aspect_ratio_info)
				{
				default: ASSERT(0); break;
				case 1: parx = 1; pary = 1; break;
				case 2: parx = 12; pary = 11; break;
				case 3: parx = 10; pary = 11; break;
				case 4: parx = 16; pary = 11; break;
				case 5: parx = 40; pary = 33; break;
				case 15: parx = (BYTE)m_pFile->BitRead(8); pary = (BYTE)m_pFile->BitRead(8); break;
				}

				BYTE vol_control_parameters = (BYTE)m_pFile->BitRead(1);

				if(vol_control_parameters)
				{
					BYTE chroma_format = (BYTE)m_pFile->BitRead(2);
					BYTE low_delay = (BYTE)m_pFile->BitRead(1);
					BYTE vbv_parameters = (BYTE)m_pFile->BitRead(1);
					UNUSED_ALWAYS(chroma_format);
					UNUSED_ALWAYS(low_delay);

					if(vbv_parameters)
					{
						WORD first_half_bit_rate = (WORD)m_pFile->BitRead(15);
						if(!m_pFile->BitRead(1)) break;
						WORD latter_half_bit_rate = (WORD)m_pFile->BitRead(15);
						if(!m_pFile->BitRead(1)) break;
						WORD first_half_vbv_buffer_size = (WORD)m_pFile->BitRead(15);
						if(!m_pFile->BitRead(1)) break;

						BYTE latter_half_vbv_buffer_size = (BYTE)m_pFile->BitRead(3);
						WORD first_half_vbv_occupancy = (WORD)m_pFile->BitRead(11);
						if(!m_pFile->BitRead(1)) break;
						WORD latter_half_vbv_occupancy = (WORD)m_pFile->BitRead(15);
						if(!m_pFile->BitRead(1)) break;
					}
				}

				BYTE video_object_layer_shape = (BYTE)m_pFile->BitRead(2);

				if(video_object_layer_shape == 3 && video_object_layer_verid != 1)
				{
					BYTE video_object_layer_shape_extension = (BYTE)m_pFile->BitRead(4);
				}

				if(!m_pFile->BitRead(1)) break;
				WORD vop_time_increment_resolution = (WORD)m_pFile->BitRead(16);
				if(!m_pFile->BitRead(1)) break;
				BYTE fixed_vop_rate = (BYTE)m_pFile->BitRead(1);

				if(fixed_vop_rate)
				{
					int bits = 0;
					for(WORD i = vop_time_increment_resolution; i; i /= 2)
						bits++;

					WORD fixed_vop_time_increment = m_pFile->BitRead(bits);

					if(fixed_vop_time_increment)
					{
						atpf = 10000000i64 * fixed_vop_time_increment / vop_time_increment_resolution;
					}
				}

				if(video_object_layer_shape != 2)
				{
					if(video_object_layer_shape == 0)
					{
						if(!m_pFile->BitRead(1)) break;
						width = (WORD)m_pFile->BitRead(13);
						if(!m_pFile->BitRead(1)) break;
						height = (WORD)m_pFile->BitRead(13);
						if(!m_pFile->BitRead(1)) break;
					}

					BYTE interlaced = (BYTE)m_pFile->BitRead(1);
					BYTE obmc_disable = (BYTE)m_pFile->BitRead(1);

					// ...
				}
			}
		}
		else if(id == 0xb6)
		{
			m_seqhdrsize = m_pFile->GetPos() - 4;			
		}
	}

	if(!width || !height) 
		return E_FAIL;

	CAtlArray<CMediaType> mts;

	CMediaType mt;
	mt.SetSampleSize(1);

	mt.majortype = MEDIATYPE_Video;
	mt.subtype = FOURCCMap('v4pm');
	mt.formattype = FORMAT_MPEG2Video;
	MPEG2VIDEOINFO* vih = (MPEG2VIDEOINFO*)mt.AllocFormatBuffer(FIELD_OFFSET(MPEG2VIDEOINFO, dwSequenceHeader) + m_seqhdrsize);
	memset(vih, 0, mt.FormatLength());
	vih->hdr.bmiHeader.biSize = sizeof(vih->hdr.bmiHeader);
	vih->hdr.bmiHeader.biWidth = width;
	vih->hdr.bmiHeader.biHeight = height;
	vih->hdr.bmiHeader.biCompression = 'v4pm';
	vih->hdr.bmiHeader.biPlanes = 1;
	vih->hdr.bmiHeader.biBitCount = 24;
	vih->hdr.AvgTimePerFrame = atpf;
	vih->hdr.dwPictAspectRatioX = width*parx;
	vih->hdr.dwPictAspectRatioY = height*pary;
	vih->cbSequenceHeader = m_seqhdrsize;
	m_pFile->Seek(0);
	m_pFile->ByteRead((BYTE*)vih->dwSequenceHeader, m_seqhdrsize);
	mts.Add(mt);
	mt.subtype = FOURCCMap(vih->hdr.bmiHeader.biCompression = 'V4PM');
	mts.Add(mt);

	CAutoPtr<CBaseSplitterOutputPin> pPinOut(DNew CBaseSplitterOutputPin(mts, L"Video", this, this, &hr));
	EXECUTE_ASSERT(SUCCEEDED(AddOutputPin(0, pPinOut)));

	m_rtNewStop = m_rtStop = m_rtDuration;

	return m_pOutputs.GetCount() > 0 ? S_OK : E_FAIL;
}

bool CMPEG4VideoSplitterFilter::DemuxInit()
{
	SetThreadName(-1, "CMPEG4VideoSplitterFilter");
	return true;
}

void CMPEG4VideoSplitterFilter::DemuxSeek(REFERENCE_TIME rt)
{
	ASSERT(rt == 0);

	m_pFile->Seek(m_seqhdrsize);
}

bool CMPEG4VideoSplitterFilter::DemuxLoop()
{
	HRESULT hr = S_OK;

	CAutoPtr<Packet> p;

	REFERENCE_TIME rt = 0;
	REFERENCE_TIME atpf = ((MPEG2VIDEOINFO*)GetOutputPin(0)->CurrentMediaType().Format())->hdr.AvgTimePerFrame;

	DWORD sync = ~0;

	while(SUCCEEDED(hr) && !CheckRequest(NULL) && m_pFile->GetRemaining())
	{
		for(int i = 0; i < 65536; i++) // don't call CheckRequest so often
		{
			bool eof = !m_pFile->GetRemaining();

			if(p && !p->IsEmpty() && (m_pFile->BitRead(32, true) == 0x000001b6 || eof))
			{
				hr = DeliverPacket(p);
			}

			if(eof) break;

			if(!p)
			{
				p.Attach(DNew Packet());
				p->SetCount(0, 1024);
				p->TrackNumber = 0;
				p->rtStart = rt; 
				p->rtStop = rt + atpf;
				p->bSyncPoint = FALSE;
				rt += atpf; 
				// rt = Packet::INVALID_TIME;
			}

			BYTE b;
			m_pFile->ByteRead(&b, 1);
			p->Add(b);
		}
	}

	return true;
}

//
// CMPEG4VideoSourceFilter
//

CMPEG4VideoSourceFilter::CMPEG4VideoSourceFilter(LPUNKNOWN pUnk, HRESULT* phr)
	: CMPEG4VideoSplitterFilter(pUnk, phr)
{
	m_clsid = __uuidof(this);
	m_pInput.Free();
}
