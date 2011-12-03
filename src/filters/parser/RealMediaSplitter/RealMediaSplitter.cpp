/*
 *  Copyright (C) 2003-2006 Gabest
 *  http://www.gabest.org
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
#include <Shlwapi.h>
#include <atlpath.h>
#include <MMReg.h>
#include <ks.h>
#include <ksmedia.h>
#include "../../../DSUtil/DSUtil.h"
#include "../../../DSUtil/MediaTypes.h"
#include "RealMediaSplitter.h"
#include "../../../Subtitles/SubtitleInputPin.h"

#include <InitGuid.h>
#include <moreuuids.h>


template<typename T>
static void bswap(T& var)
{
	BYTE* s = (BYTE*)&var;
	for(BYTE* d = s + sizeof(var)-1; s < d; s++, d--) {
		*s ^= *d, *d ^= *s, *s ^= *d;
	}
}

void rvinfo::bswap()
{
	::bswap(dwSize);
	::bswap(w);
	::bswap(h);
	::bswap(bpp);
	::bswap(unk1);
	::bswap(fps);
	::bswap(type1);
	::bswap(type2);
}

void rainfo::bswap()
{
	::bswap(version1);
	::bswap(version2);
	::bswap(header_size);
	::bswap(flavor);
	::bswap(coded_frame_size);
	::bswap(sub_packet_h);
	::bswap(frame_size);
	::bswap(sub_packet_size);
}

void rainfo4::bswap()
{
	__super::bswap();
	::bswap(sample_rate);
	::bswap(sample_size);
	::bswap(channels);
}

void rainfo5::bswap()
{
	__super::bswap();
	::bswap(sample_rate);
	::bswap(sample_size);
	::bswap(channels);
}

using namespace RMFF;

#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] = {
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_PIN sudpPins[] = {
	{L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesIn), sudPinTypesIn},
	{L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, 0, NULL}
};

const AMOVIESETUP_MEDIATYPE sudPinTypesIn2[] = {
	{&MEDIATYPE_Video, &MEDIASUBTYPE_RV20},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_RV30},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_RV40},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_RV41},
};

const AMOVIESETUP_MEDIATYPE sudPinTypesOut2[] = {
	{&MEDIATYPE_Video, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_PIN sudpPins2[] = {
	{L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesIn2), sudPinTypesIn2},
	{L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesOut2), sudPinTypesOut2}
};

const AMOVIESETUP_MEDIATYPE sudPinTypesIn3[] = {
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_14_4},
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_28_8},
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_ATRC},
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_COOK},
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_DNET},
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_SIPR},
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_AAC},
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_RAAC},
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_RACP},
};

const AMOVIESETUP_MEDIATYPE sudPinTypesOut3[] = {
	{&MEDIATYPE_Audio, &MEDIASUBTYPE_PCM},
};

const AMOVIESETUP_PIN sudpPins3[] = {
	{L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesIn3), sudPinTypesIn3},
	{L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesOut3), sudPinTypesOut3}
};

const AMOVIESETUP_FILTER sudFilter[] = {
	{&__uuidof(CRealMediaSplitterFilter), L"MPC RealMedia Splitter", MERIT_NORMAL, countof(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory},
	{&__uuidof(CRealMediaSourceFilter), L"MPC RealMedia Source", MERIT_NORMAL, 0, NULL, CLSID_LegacyAmFilterCategory},
	{&__uuidof(CRealVideoDecoder), L"MPC RealVideo Decoder", MERIT_NORMAL, countof(sudpPins2), sudpPins2, CLSID_LegacyAmFilterCategory},
	{&__uuidof(CRealAudioDecoder), L"MPC RealAudio Decoder", MERIT_NORMAL, countof(sudpPins3), sudpPins3, CLSID_LegacyAmFilterCategory},
};

CFactoryTemplate g_Templates[] = {
	{sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CRealMediaSplitterFilter>, NULL, &sudFilter[0]},
	{sudFilter[1].strName, sudFilter[1].clsID, CreateInstance<CRealMediaSourceFilter>, NULL, &sudFilter[1]},
	{sudFilter[2].strName, sudFilter[2].clsID, CreateInstance<CRealVideoDecoder>, NULL, &sudFilter[2]},
	{sudFilter[3].strName, sudFilter[3].clsID, CreateInstance<CRealAudioDecoder>, NULL, &sudFilter[3]},
};

int g_cTemplates = countof(g_Templates);

STDAPI DllRegisterServer()
{
	RegisterSourceFilter(CLSID_AsyncReader, MEDIASUBTYPE_RealMedia, _T("0,4,,2E524D46"), _T(".rm"), _T(".rmvb"), _T(".ram"), NULL);

	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	UnRegisterSourceFilter(MEDIASUBTYPE_RealMedia);

	return AMovieDllRegisterServer2(FALSE);
}

#include "../../FilterApp.h"

CFilterApp theApp;

#endif

//
// CRealMediaSplitterFilter
//

CRealMediaSplitterFilter::CRealMediaSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr)
	: CBaseSplitterFilter(NAME("CRealMediaSplitterFilter"), pUnk, phr, __uuidof(this))
{
}

CRealMediaSplitterFilter::~CRealMediaSplitterFilter()
{
}

STDMETHODIMP CRealMediaSplitterFilter::QueryFilterInfo(FILTER_INFO* pInfo)
{
	CheckPointer(pInfo, E_POINTER);
	ValidateReadWritePtr(pInfo, sizeof(FILTER_INFO));

	wcscpy(pInfo->achName, L"MPC RealMedia Splitter");
	pInfo->pGraph = m_pGraph;
	if(m_pGraph) {
		m_pGraph->AddRef();
	}

	return S_OK;
}

HRESULT CRealMediaSplitterFilter::CreateOutputs(IAsyncReader* pAsyncReader)
{
	CheckPointer(pAsyncReader, E_POINTER);

	{
		DWORD dw;
		if(FAILED(pAsyncReader->SyncRead(0, 4, (BYTE*)&dw)) || dw != 'FMR.') {
			return E_FAIL;
		}
	}

	HRESULT hr = E_FAIL;

	m_pFile.Free();

	m_pFile.Attach(DNew CRMFile(pAsyncReader, hr));
	if(!m_pFile) {
		return E_OUTOFMEMORY;
	}
	if(FAILED(hr)) {
		m_pFile.Free();
		return hr;
	}

	m_rtNewStart = m_rtCurrent = 0;
	m_rtNewStop = m_rtStop = 0;

	m_rtStop = 10000i64*m_pFile->m_p.tDuration;

	POSITION pos = m_pFile->m_mps.GetHeadPosition();
	while(pos) {
		MediaProperies* pmp = m_pFile->m_mps.GetNext(pos);

		CStringW name;
		name.Format(L"Output %02d", pmp->stream);
		if(!pmp->name.IsEmpty()) {
			name += L" (" + CStringW(pmp->name) + L")";
		}

		CAtlArray<CMediaType> mts;

		CMediaType mt;
		mt.SetSampleSize(max(pmp->maxPacketSize*16/**/, 1));

		if(pmp->mime == "video/x-pn-realvideo") {
			mt.majortype = MEDIATYPE_Video;
			mt.formattype = FORMAT_VideoInfo;

			VIDEOINFOHEADER* pvih = (VIDEOINFOHEADER*)mt.AllocFormatBuffer(sizeof(VIDEOINFOHEADER) + pmp->typeSpecData.GetCount());
			memset(mt.Format(), 0, mt.FormatLength());
			memcpy(pvih + 1, pmp->typeSpecData.GetData(), pmp->typeSpecData.GetCount());

			rvinfo rvi = *(rvinfo*)pmp->typeSpecData.GetData();
			rvi.bswap();

			ASSERT(rvi.dwSize >= FIELD_OFFSET(rvinfo, morewh));
			ASSERT(rvi.fcc1 == 'ODIV');

			mt.subtype = FOURCCMap(rvi.fcc2);
			if(rvi.fps > 0x10000) {
				pvih->AvgTimePerFrame = REFERENCE_TIME(10000000i64 / ((float)rvi.fps/0x10000));
			}
			pvih->dwBitRate = pmp->avgBitRate;
			pvih->bmiHeader.biSize = sizeof(pvih->bmiHeader);
			pvih->bmiHeader.biWidth = rvi.w;
			pvih->bmiHeader.biHeight = rvi.h;
			pvih->bmiHeader.biPlanes = 3;
			pvih->bmiHeader.biBitCount = rvi.bpp;
			pvih->bmiHeader.biCompression = rvi.fcc2;
			pvih->bmiHeader.biSizeImage = rvi.w*rvi.h*3/2;

			mts.Add(mt);

			if(pmp->width > 0 && pmp->height > 0) {
				BITMAPINFOHEADER bmi = pvih->bmiHeader;
				mt.formattype = FORMAT_VideoInfo2;
				VIDEOINFOHEADER2* pvih2 = (VIDEOINFOHEADER2*)mt.ReallocFormatBuffer(sizeof(VIDEOINFOHEADER2) + pmp->typeSpecData.GetCount());
				memset(mt.Format() + FIELD_OFFSET(VIDEOINFOHEADER2, dwInterlaceFlags), 0, mt.FormatLength() - FIELD_OFFSET(VIDEOINFOHEADER2, dwInterlaceFlags));
				memcpy(pvih2 + 1, pmp->typeSpecData.GetData(), pmp->typeSpecData.GetCount());
				pvih2->bmiHeader = bmi;
				pvih2->bmiHeader.biWidth = (DWORD)pmp->width;
				pvih2->bmiHeader.biHeight = (DWORD)pmp->height;
				pvih2->dwPictAspectRatioX = rvi.w;
				pvih2->dwPictAspectRatioY = rvi.h;

				mts.InsertAt(0, mt);
			}
		} else if(pmp->mime == "audio/x-pn-realaudio") {
			mt.majortype = MEDIATYPE_Audio;
			mt.formattype = FORMAT_WaveFormatEx;
			mt.bTemporalCompression = 1;

			WAVEFORMATEX* pwfe = (WAVEFORMATEX*)mt.AllocFormatBuffer(sizeof(WAVEFORMATEX) + pmp->typeSpecData.GetCount());
			memset(mt.Format(), 0, mt.FormatLength());
			memcpy(pwfe + 1, pmp->typeSpecData.GetData(), pmp->typeSpecData.GetCount());

			union {
				DWORD fcc;
				char fccstr[5];
			};

			fcc = 0;
			fccstr[4] = 0;

			BYTE* fmt = pmp->typeSpecData.GetData();
			for(size_t i = 0; i < pmp->typeSpecData.GetCount()-4; i++, fmt++) {
				if(fmt[0] == '.' || fmt[1] == 'r' || fmt[2] == 'a') {
					break;
				}
			}

			rainfo rai = *(rainfo*)fmt;
			rai.bswap();

			BYTE* extra = NULL;

			if(rai.version2 == 4) {
				rainfo4 rai4 = *(rainfo4*)fmt;
				rai4.bswap();
				pwfe->nChannels = rai4.channels;
				pwfe->wBitsPerSample = rai4.sample_size;
				pwfe->nSamplesPerSec = rai4.sample_rate;
				pwfe->nBlockAlign = rai4.frame_size;
				BYTE* p = (BYTE*)((rainfo4*)fmt+1);
				int len = *p++;
				p += len;
				len = *p++;
				ASSERT(len == 4);
				if(len == 4) {
					fcc = MAKEFOURCC(p[0],p[1],p[2],p[3]);
				}
				extra = p + len + 3;
			} else if(rai.version2 == 5) {
				rainfo5 rai5 = *(rainfo5*)fmt;
				rai5.bswap();
				pwfe->nChannels = rai5.channels;
				pwfe->wBitsPerSample = rai5.sample_size;
				pwfe->nSamplesPerSec = rai5.sample_rate;
				pwfe->nBlockAlign = rai5.frame_size;
				fcc = rai5.fourcc3;
				extra = fmt + sizeof(rainfo5) + 4;
			} else {
				continue;
			}

			_strupr(fccstr);

			mt.subtype = FOURCCMap(fcc);

			bswap(fcc);

			switch(fcc) {
				case '14_4':
					pwfe->wFormatTag = WAVE_FORMAT_14_4;
					break;
				case '28_8':
					pwfe->wFormatTag = WAVE_FORMAT_28_8;
					break;
				case 'ATRC':
					pwfe->wFormatTag = WAVE_FORMAT_ATRC;
					break;
				case 'COOK':
					pwfe->wFormatTag = WAVE_FORMAT_COOK;
					break;
				case 'DNET':
					pwfe->wFormatTag = WAVE_FORMAT_DNET;
					break;
				case 'SIPR':
					pwfe->wFormatTag = WAVE_FORMAT_SIPR;
					break;
				case 'RAAC':
					pwfe->wFormatTag = WAVE_FORMAT_RAAC;
					break;
				case 'RACP':
					pwfe->wFormatTag = WAVE_FORMAT_RACP;
					break;
			}

			if(pwfe->wFormatTag) {
				mts.Add(mt);

				if(fcc == 'DNET') {
					mt.subtype = FOURCCMap(pwfe->wFormatTag = WAVE_FORMAT_DOLBY_AC3);
					mts.InsertAt(0, mt);
				} else if(fcc == 'RAAC' || fcc == 'RACP') {
					mt.subtype = FOURCCMap(pwfe->wFormatTag = WAVE_FORMAT_AAC);
					int extralen = *(DWORD*)extra;
					extra += 4;
					::bswap(extralen);
					ASSERT(*extra == 2);	// always 2? why? what does it mean?
					if(*extra == 2) {
						extra++;
						extralen--;
						WAVEFORMATEX* pwfe = (WAVEFORMATEX*)mt.ReallocFormatBuffer(sizeof(WAVEFORMATEX) + extralen);
						pwfe->cbSize = extralen;
						memcpy(pwfe + 1, extra, extralen);
					} else {
						WAVEFORMATEX* pwfe = (WAVEFORMATEX*)mt.ReallocFormatBuffer(sizeof(WAVEFORMATEX) + 5);
						pwfe->cbSize = MakeAACInitData((BYTE*)(pwfe+1), 0, pwfe->nSamplesPerSec, pwfe->nChannels);
					}
					mts.InsertAt(0, mt);
				}
			}
		} else if(pmp->mime == "logical-fileinfo") {
			CAtlMap<CStringA, CStringA, CStringElementTraits<CStringA> > lfi;
			CStringA key, value;

			BYTE* p = pmp->typeSpecData.GetData();
			BYTE* end = p + pmp->typeSpecData.GetCount();
			p += 8;

			DWORD cnt = p <= end-4 ? *(DWORD*)p : 0;
			bswap(cnt);
			p += 4;

			if(cnt > 0xffff) {	// different format?
				p += 2;
				cnt = p <= end-4 ? *(DWORD*)p : 0;
				bswap(cnt);
				p += 4;
			}

			while(p < end-4 && cnt-- > 0) {
				BYTE* base = p;
				DWORD len = *(DWORD*)p;
				bswap(len);
				p += 4;
				if(base + len > end) {
					break;
				}

				p++;
				WORD keylen = *(WORD*)p;
				bswap(keylen);
				p += 2;
				memcpy(key.GetBufferSetLength(keylen), p, keylen);
				p += keylen;

				p+=4;
				WORD valuelen = *(WORD*)p;
				bswap(valuelen);
				p += 2;
				memcpy(value.GetBufferSetLength(valuelen), p, valuelen);
				p += valuelen;

				ASSERT(p == base + len);
				p = base + len;

				lfi[key] = value;
			}

			POSITION pos = lfi.GetStartPosition();
			while(pos) {
				lfi.GetNextAssoc(pos, key, value);

				int n = 0;
				if(key.Find("CHAPTER") == 0 && key.Find("TIME") == key.GetLength()-4
						&& (n = strtol(key.Mid(7), NULL, 10)) > 0) {
					int h, m, s, ms;
					char c;
					if(7 != sscanf(value, "%d%c%d%c%d%c%d", &h, &c, &m, &c, &s, &c, &ms)) {
						continue;
					}

					key.Format("CHAPTER%02dNAME", n);
					if(!lfi.Lookup(key, value) || value.IsEmpty()) {
						value.Format("Chapter %d", n);
					}

					ChapAppend(
						((((REFERENCE_TIME)h*60+m)*60+s)*1000+ms)*10000,
						CStringW(CString(value)));
				}
			}
		}

		if(mts.IsEmpty()) {
			TRACE(_T("Unsupported RealMedia stream (%d): %s\n"), pmp->stream, CString(pmp->mime));
			continue;
		}

		HRESULT hr;

		CAutoPtr<CBaseSplitterOutputPin> pPinOut(DNew CRealMediaSplitterOutputPin(mts, name, this, this, &hr));
		if(SUCCEEDED(AddOutputPin((DWORD)pmp->stream, pPinOut))) {
			if(!m_rtStop) {
				m_pFile->m_p.tDuration = max(m_pFile->m_p.tDuration, pmp->tDuration);
			}
		}
	}

	pos = m_pFile->m_subs.GetHeadPosition();
	for(DWORD stream = 0; pos; stream++) {
		CRMFile::subtitle& s = m_pFile->m_subs.GetNext(pos);

		CStringW name;
		name.Format(L"Subtitle %02d", stream);
		if(!s.name.IsEmpty()) {
			name += L" (" + CStringW(CString(s.name)) + L")";
		}

		CMediaType mt;
		mt.SetSampleSize(1);
		mt.majortype = MEDIATYPE_Text;

		CAtlArray<CMediaType> mts;
		mts.Add(mt);

		HRESULT hr;

		CAutoPtr<CBaseSplitterOutputPin> pPinOut(DNew CRealMediaSplitterOutputPin(mts, name, this, this, &hr));
		AddOutputPin((DWORD)~stream, pPinOut);
	}

	m_rtDuration = m_rtNewStop = m_rtStop = 10000i64*m_pFile->m_p.tDuration;

	SetProperty(L"TITL", CStringW(m_pFile->m_cd.title));
	SetProperty(L"AUTH", CStringW(m_pFile->m_cd.author));
	SetProperty(L"CPYR", CStringW(m_pFile->m_cd.copyright));
	SetProperty(L"DESC", CStringW(m_pFile->m_cd.comment));

	return m_pOutputs.GetCount() > 0 ? S_OK : E_FAIL;
}

bool CRealMediaSplitterFilter::DemuxInit()
{
	SetThreadName((DWORD)-1, "CRealMediaSplitterFilter");

	if(!m_pFile) {
		return false;
	}

	// reindex if needed

	if(m_pFile->m_irs.GetCount() == 0) {
		m_nOpenProgress = 0;
		m_rtDuration = 0;

		int stream = m_pFile->GetMasterStream();

		UINT32 tLastStart = (UINT32)-1;
		UINT32 nPacket = 0;

		POSITION pos = m_pFile->m_dcs.GetHeadPosition();
		while(pos && !m_fAbort) {
			DataChunk* pdc = m_pFile->m_dcs.GetNext(pos);

			m_pFile->Seek(pdc->pos);

			for(UINT32 i = 0; i < pdc->nPackets && !m_fAbort; i++, nPacket++) {
				UINT64 filepos = m_pFile->GetPos();

				HRESULT hr;

				MediaPacketHeader mph;
				if(S_OK != (hr = m_pFile->Read(mph, false))) {
					break;
				}

				m_rtDuration = max((__int64)(10000i64*mph.tStart), m_rtDuration);

				if(mph.stream == stream && (mph.flags&MediaPacketHeader::PN_KEYFRAME_FLAG) && tLastStart != mph.tStart) {
					CAutoPtr<IndexRecord> pir(DNew IndexRecord);
					pir->tStart = mph.tStart;
					pir->ptrFilePos = (UINT32)filepos;
					pir->packet = nPacket;
					m_pFile->m_irs.AddTail(pir);

					tLastStart = mph.tStart;
				}

				m_nOpenProgress = m_pFile->GetPos()*100/m_pFile->GetLength();

				DWORD cmd;
				if(CheckRequest(&cmd)) {
					if(cmd == CMD_EXIT) {
						m_fAbort = true;
					} else {
						Reply(S_OK);
					}
				}
			}
		}

		m_nOpenProgress = 100;

		if(m_fAbort) {
			m_pFile->m_irs.RemoveAll();
		}

		m_fAbort = false;
	}

	m_seekpos = NULL;
	m_seekpacket = 0;
	m_seekfilepos = 0;

	return true;
}

void CRealMediaSplitterFilter::DemuxSeek(REFERENCE_TIME rt)
{
	if(rt <= 0) {
		m_seekpos = m_pFile->m_dcs.GetHeadPosition();
		m_seekpacket = 0;
		m_seekfilepos = m_pFile->m_dcs.GetHead()->pos;
	} else {
		m_seekpos = NULL;

		POSITION pos = m_pFile->m_irs.GetTailPosition();
		while(pos && !m_seekpos) {
			IndexRecord* pir = m_pFile->m_irs.GetPrev(pos);
			if(pir->tStart <= rt/10000) {
				m_seekpacket = pir->packet;

				pos = m_pFile->m_dcs.GetTailPosition();
				while(pos && !m_seekpos) {
					POSITION tmp = pos;

					DataChunk* pdc = m_pFile->m_dcs.GetPrev(pos);

					if(pdc->pos <= pir->ptrFilePos) {
						m_seekpos = tmp;
						m_seekfilepos = pir->ptrFilePos;

						POSITION pos = m_pFile->m_dcs.GetHeadPosition();
						while(pos != m_seekpos) {
							m_seekpacket -= m_pFile->m_dcs.GetNext(pos)->nPackets;
						}
					}
				}

				// search the closest keyframe to the seek time (commented out 'cause rm seems to index all of its keyframes...)
				/*
								if(m_seekpos)
								{
									DataChunk* pdc = m_pFile->m_dcs.GetAt(m_seekpos);

									m_pFile->Seek(m_seekfilepos);

									REFERENCE_TIME seektime = -1;
									UINT32 seekstream = -1;

									for(UINT32 i = m_seekpacket; i < pdc->nPackets; i++)
									{
										UINT64 filepos = m_pFile->GetPos();

										MediaPacketHeader mph;
										if(S_OK != m_pFile->Read(mph, false))
											break;

										if(seekstream == -1) seekstream = mph.stream;
										if(seekstream != mph.stream) continue;

										if(seektime == 10000i64*mph.tStart) continue;
										if(rt < 10000i64*mph.tStart) break;

										if((mph.flags&MediaPacketHeader::PN_KEYFRAME_FLAG))
										{
											m_seekpacket = i;
											m_seekfilepos = filepos;
											seektime = 10000i64*mph.tStart;
										}
									}
								}
				*/
			}
		}

		if(!m_seekpos) {
			m_seekpos = m_pFile->m_dcs.GetHeadPosition();
			m_seekpacket = 0;
			m_seekfilepos = m_pFile->m_dcs.GetAt(m_seekpos)->pos;
		}
	}
}

bool CRealMediaSplitterFilter::DemuxLoop()
{
	HRESULT hr = S_OK;
	POSITION pos;

	pos = m_pFile->m_subs.GetHeadPosition();
	for(DWORD stream = 0; pos && SUCCEEDED(hr) && !CheckRequest(NULL); stream++) {
		CRMFile::subtitle& s = m_pFile->m_subs.GetNext(pos);

		CAutoPtr<Packet> p(DNew Packet);

		p->TrackNumber = ~stream;
		p->bSyncPoint = TRUE;
		p->rtStart = 0;
		p->rtStop = 1;

		p->SetCount((4+1) + (2+4+(s.name.GetLength()+1)*2) + (2+4+s.data.GetLength()));
		BYTE* ptr = p->GetData();

		strcpy((char*)ptr, "GAB2");
		ptr += 4+1;

		*(WORD*)ptr = 2;
		ptr += 2;
		*(DWORD*)ptr = (s.name.GetLength()+1)*2;
		ptr += 4;
		wcscpy((WCHAR*)ptr, CStringW(s.name));
		ptr += (s.name.GetLength()+1)*2;

		*(WORD*)ptr = 4;
		ptr += 2;
		*(DWORD*)ptr = s.data.GetLength();
		ptr += 4;
		memcpy((char*)ptr, s.data, s.data.GetLength());
		ptr += s.name.GetLength();

		hr = DeliverPacket(p);
	}

	pos = m_seekpos;
	while(pos && SUCCEEDED(hr) && !CheckRequest(NULL)) {
		DataChunk* pdc = m_pFile->m_dcs.GetNext(pos);

		m_pFile->Seek(m_seekfilepos > 0 ? m_seekfilepos : pdc->pos);

		for(UINT32 i = m_seekpacket; i < pdc->nPackets && SUCCEEDED(hr) && !CheckRequest(NULL); i++) {
			MediaPacketHeader mph;
			if(S_OK != (hr = m_pFile->Read(mph))) {
				break;
			}

			CAutoPtr<Packet> p(DNew Packet);
			p->TrackNumber = mph.stream;
			p->bSyncPoint = !!(mph.flags&MediaPacketHeader::PN_KEYFRAME_FLAG);
			p->rtStart = 10000i64*(mph.tStart);
			p->rtStop = p->rtStart+1;
			p->Copy(mph.pData);
			hr = DeliverPacket(p);
		}

		m_seekpacket = 0;
		m_seekfilepos = 0;
	}

	return true;
}

// IKeyFrameInfo

STDMETHODIMP CRealMediaSplitterFilter::GetKeyFrameCount(UINT& nKFs)
{
	if(!m_pFile) {
		return E_UNEXPECTED;
	}
	nKFs = m_pFile->m_irs.GetCount();
	return S_OK;
}

STDMETHODIMP CRealMediaSplitterFilter::GetKeyFrames(const GUID* pFormat, REFERENCE_TIME* pKFs, UINT& nKFs)
{
	CheckPointer(pFormat, E_POINTER);
	CheckPointer(pKFs, E_POINTER);

	if(!m_pFile) {
		return E_UNEXPECTED;
	}
	if(*pFormat != TIME_FORMAT_MEDIA_TIME) {
		return E_INVALIDARG;
	}

	UINT nKFsTmp = 0;
	POSITION pos = m_pFile->m_irs.GetHeadPosition();
	for(int i = 0; pos && nKFsTmp < nKFs; i++) {
		pKFs[nKFsTmp++] = 10000i64*m_pFile->m_irs.GetNext(pos)->tStart;
	}
	nKFs = nKFsTmp;

	return S_OK;
}

//
// CRealMediaSplitterOutputPin
//

CRealMediaSplitterOutputPin::CRealMediaSplitterOutputPin(CAtlArray<CMediaType>& mts, LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr)
	: CBaseSplitterOutputPin(mts, pName, pFilter, pLock, phr)
{
}

CRealMediaSplitterOutputPin::~CRealMediaSplitterOutputPin()
{
}

HRESULT CRealMediaSplitterOutputPin::DeliverEndFlush()
{
	{
		CAutoLock cAutoLock(&m_csQueue);
		m_segments.Clear();
	}

	return __super::DeliverEndFlush();
}

HRESULT CRealMediaSplitterOutputPin::DeliverSegments()
{
	HRESULT hr;

	if(m_segments.GetCount() == 0) {
		m_segments.Clear();
		return S_OK;
	}

	CAutoPtr<Packet> p(DNew Packet());

	p->TrackNumber = (DWORD)-1;
	p->bDiscontinuity = m_segments.fDiscontinuity;
	p->bSyncPoint = m_segments.fSyncPoint;
	p->rtStart = m_segments.rtStart;
	p->rtStop = m_segments.rtStart+1;

	DWORD len = 0, total = 0;
	POSITION pos = m_segments.GetHeadPosition();
	while(pos) {
		segment* s = m_segments.GetNext(pos);
		len = max(len, s->offset + s->data.GetCount());
		total += s->data.GetCount();
	}
	ASSERT(len == total);
	len += 1 + 2*4*(!m_segments.fMerged ? m_segments.GetCount() : 1);

	p->SetCount(len);

	BYTE* pData = p->GetData();

	*pData++ = m_segments.fMerged ? 0 : m_segments.GetCount()-1;

	if(m_segments.fMerged) {
		*((DWORD*)pData) = 1;
		pData += 4;
		*((DWORD*)pData) = 0;
		pData += 4;
	} else {
		pos = m_segments.GetHeadPosition();
		while(pos) {
			*((DWORD*)pData) = 1;
			pData += 4;
			*((DWORD*)pData) = m_segments.GetNext(pos)->offset;
			pData += 4;
		}
	}

	pos = m_segments.GetHeadPosition();
	while(pos) {
		segment* s = m_segments.GetNext(pos);
		memcpy(pData + s->offset, s->data.GetData(), s->data.GetCount());
	}

	hr = __super::DeliverPacket(p);

	m_segments.Clear();

	return hr;
}

HRESULT CRealMediaSplitterOutputPin::DeliverPacket(CAutoPtr<Packet> p)
{
	HRESULT hr = S_OK;

	ASSERT(p->rtStart < p->rtStop);

	if(m_mt.subtype == MEDIASUBTYPE_WAVE_DOLBY_AC3) {
		WORD* s = (WORD*)p->GetData();
		WORD* e = s + p->GetCount()/2;
		while(s < e) {
			bswap(*s++);
		}
	}

	if(m_mt.subtype == MEDIASUBTYPE_RV10 || m_mt.subtype == MEDIASUBTYPE_RV20
			|| m_mt.subtype == MEDIASUBTYPE_RV30 || m_mt.subtype == MEDIASUBTYPE_RV40
			|| m_mt.subtype == MEDIASUBTYPE_RV41) {
		CAutoLock cAutoLock(&m_csQueue);

		int len = p->GetCount();
		BYTE* pIn = p->GetData();
		BYTE* pInOrg = pIn;

		if(m_segments.rtStart != p->rtStart) {
			if(S_OK != (hr = DeliverSegments())) {
				return hr;
			}
		}

		if(!m_segments.fDiscontinuity && p->bDiscontinuity) {
			m_segments.fDiscontinuity = true;
		}
		m_segments.fSyncPoint = !!p->bSyncPoint;
		m_segments.rtStart = p->rtStart;

		while(pIn - pInOrg < len) {
			BYTE hdr = *pIn++;
			DWORD packetlen = 0, packetoffset = 0;

			if((hdr&0xc0) == 0x40) {
				pIn++;
				packetlen = len - (pIn - pInOrg);
			} else {
				if((hdr&0x40) == 0) {
					pIn++; //BYTE subseq = (*pIn++)&0x7f;
				}

#define GetWORD(var) \
					var = (var<<8)|(*pIn++); \
					var = (var<<8)|(*pIn++); \
 
				GetWORD(packetlen);
				if(packetlen&0x8000) {
					m_segments.fMerged = true;
				}
				if((packetlen&0x4000) == 0) {
					GetWORD(packetlen);
					packetlen &= 0x3fffffff;
				} else {
					packetlen &= 0x3fff;
				}

				GetWORD(packetoffset);
				if((packetoffset&0x4000) == 0) {
					GetWORD(packetoffset);
					packetoffset &= 0x3fffffff;
				} else {
					packetoffset &= 0x3fff;
				}

#undef GetWORD

				if((hdr&0xc0) == 0xc0) {
					m_segments.rtStart = 10000i64*packetoffset - m_rtStart, packetoffset = 0;
				} else if((hdr&0xc0) == 0x80) {
					packetoffset = packetlen - packetoffset;
				}

				pIn++; //BYTE seqnum = *pIn++;
			}

			int len2 = min(len - (pIn - pInOrg), packetlen - packetoffset);

			CAutoPtr<segment> s(DNew segment);
			s->offset = packetoffset;
			s->data.SetCount(len2);
			memcpy(s->data.GetData(), pIn, len2);
			m_segments.AddTail(s);

			pIn += len2;

			if((hdr&0x80) || packetoffset+len2 >= packetlen) {
				if(S_OK != (hr = DeliverSegments())) {
					return hr;
				}
			}
		}
	} else if(m_mt.subtype == MEDIASUBTYPE_RAAC || m_mt.subtype == MEDIASUBTYPE_RACP
			  || m_mt.subtype == MEDIASUBTYPE_AAC) {
		BYTE* ptr = p->GetData()+2;

		CAtlList<WORD> sizes;
		int total = 0;
		int remaining = p->GetCount()-2;
		int expected = *(ptr-1)>>4;

		while(total < remaining) {
			int size = (ptr[0]<<8)|(ptr[1]);
			sizes.AddTail(size);
			total += size;
			ptr += 2;
			remaining -= 2;
			expected--;
		}

		ASSERT(total == remaining);
		ASSERT(expected == 0);

		WAVEFORMATEX* wfe = (WAVEFORMATEX*)m_mt.pbFormat;
		REFERENCE_TIME rtDur = 10240000000i64/wfe->nSamplesPerSec * (wfe->cbSize>2?2:1);
		REFERENCE_TIME rtStart = p->rtStart;
		BOOL bDiscontinuity = p->bDiscontinuity;

		POSITION pos = sizes.GetHeadPosition();
		while(pos) {
			WORD size = sizes.GetNext(pos);

			CAutoPtr<Packet> p(DNew Packet);
			p->bDiscontinuity = bDiscontinuity;
			p->bSyncPoint = true;
			p->rtStart = rtStart;
			p->rtStop = rtStart += rtDur;
			p->SetData(ptr, size);
			ptr += size;
			bDiscontinuity = false;
			if(S_OK != (hr = __super::DeliverPacket(p))) {
				break;
			}
		}
	} else {
		hr = __super::DeliverPacket(p);
	}

	return hr;
}

//
// CRealMediaSourceFilter
//

CRealMediaSourceFilter::CRealMediaSourceFilter(LPUNKNOWN pUnk, HRESULT* phr)
	: CRealMediaSplitterFilter(pUnk, phr)
{
	m_clsid = __uuidof(this);
	m_pInput.Free();
}

//
// CRMFile
//

CRMFile::CRMFile(IAsyncReader* pAsyncReader, HRESULT& hr)
	: CBaseSplitterFile(pAsyncReader, hr)
{
	if(FAILED(hr)) {
		return;
	}
	hr = Init();
}

template<typename T>
HRESULT CRMFile::Read(T& var)
{
	HRESULT hr = ByteRead((BYTE*)&var, sizeof(var));
	bswap(var);
	return hr;
}

HRESULT CRMFile::Read(ChunkHdr& hdr)
{
	memset(&hdr, 0, sizeof(hdr));
	HRESULT hr;
	if(S_OK != (hr = Read(hdr.object_id))
			|| S_OK != (hr = Read(hdr.size))
			|| S_OK != (hr = Read(hdr.object_version))) {
		return hr;
	}
	return S_OK;
}

HRESULT CRMFile::Read(MediaPacketHeader& mph, bool fFull)
{
	memset(&mph, 0, FIELD_OFFSET(MediaPacketHeader, pData));
	mph.stream = (UINT16)-1;

	HRESULT hr;

	UINT16 object_version;
	if(S_OK != (hr = Read(object_version))) {
		return hr;
	}
	if(object_version != 0 && object_version != 1) {
		return S_OK;
	}

	UINT8 flags;
	if(S_OK != (hr = Read(mph.len))
			|| S_OK != (hr = Read(mph.stream))
			|| S_OK != (hr = Read(mph.tStart))
			|| S_OK != (hr = Read(mph.reserved))
			|| S_OK != (hr = Read(flags))) {
		return hr;
	}
	mph.flags = (MediaPacketHeader::flag_t)flags;

	LONG len = mph.len;
	len -= sizeof(object_version);
	len -= FIELD_OFFSET(MediaPacketHeader, flags);
	len -= sizeof(flags);
	ASSERT(len >= 0);
	len = max(len, 0);

	if(fFull) {
		mph.pData.SetCount(len);
		if(mph.len > 0 && S_OK != (hr = ByteRead(mph.pData.GetData(), len))) {
			return hr;
		}
	} else {
		Seek(GetPos() + len);
	}

	return S_OK;
}

HRESULT CRMFile::Init()
{
	Seek(0);

	bool fFirstChunk = true;

	HRESULT hr;

	ChunkHdr hdr;
	while(GetRemaining() && S_OK == (hr = Read(hdr))) {
		__int64 pos = GetPos() - sizeof(hdr);

		if(fFirstChunk && hdr.object_id != '.RMF') {
			return E_FAIL;
		}

		fFirstChunk = false;

		if(pos + hdr.size > GetLength() && hdr.object_id != 'DATA') {	// truncated?
			break;
		}

		if(hdr.object_id == 0x2E7261FD) {	// '.ra+0xFD'
			return E_FAIL;
		}

		if(hdr.object_version == 0) {
			switch(hdr.object_id) {
				case '.RMF':
					if(S_OK != (hr = Read(m_fh.version))) {
						return hr;
					}
					if(hdr.size == 0x10) {
						WORD w = 0;
						if(S_OK != (hr = Read(w))) {
							return hr;
						}
						m_fh.nHeaders = w;
					} else if(S_OK != (hr = Read(m_fh.nHeaders))) {
						return hr;
					}
					break;
				case 'CONT':
					UINT16 slen;
					if(S_OK != (hr = Read(slen))) {
						return hr;
					}
					if(slen > 0 && S_OK != (hr = ByteRead((BYTE*)m_cd.title.GetBufferSetLength(slen), slen))) {
						return hr;
					}
					if(S_OK != (hr = Read(slen))) {
						return hr;
					}
					if(slen > 0 && S_OK != (hr = ByteRead((BYTE*)m_cd.author.GetBufferSetLength(slen), slen))) {
						return hr;
					}
					if(S_OK != (hr = Read(slen))) {
						return hr;
					}
					if(slen > 0 && S_OK != (hr = ByteRead((BYTE*)m_cd.copyright.GetBufferSetLength(slen), slen))) {
						return hr;
					}
					if(S_OK != (hr = Read(slen))) {
						return hr;
					}
					if(slen > 0 && S_OK != (hr = ByteRead((BYTE*)m_cd.comment.GetBufferSetLength(slen), slen))) {
						return hr;
					}
					break;
				case 'PROP':
					if(S_OK != (hr = Read(m_p.maxBitRate))) {
						return hr;
					}
					if(S_OK != (hr = Read(m_p.avgBitRate))) {
						return hr;
					}
					if(S_OK != (hr = Read(m_p.maxPacketSize))) {
						return hr;
					}
					if(S_OK != (hr = Read(m_p.avgPacketSize))) {
						return hr;
					}
					if(S_OK != (hr = Read(m_p.nPackets))) {
						return hr;
					}
					if(S_OK != (hr = Read(m_p.tDuration))) {
						return hr;
					}
					if(S_OK != (hr = Read(m_p.tPreroll))) {
						return hr;
					}
					if(S_OK != (hr = Read(m_p.ptrIndex))) {
						return hr;
					}
					if(S_OK != (hr = Read(m_p.ptrData))) {
						return hr;
					}
					if(S_OK != (hr = Read(m_p.nStreams))) {
						return hr;
					}
					UINT16 flags;
					if(S_OK != (hr = Read(flags))) {
						return hr;
					}
					m_p.flags = (Properies::flags_t)flags;
					break;
				case 'MDPR': {
					CAutoPtr<MediaProperies> mp(DNew MediaProperies);
					if(S_OK != (hr = Read(mp->stream))) {
						return hr;
					}
					if(S_OK != (hr = Read(mp->maxBitRate))) {
						return hr;
					}
					if(S_OK != (hr = Read(mp->avgBitRate))) {
						return hr;
					}
					if(S_OK != (hr = Read(mp->maxPacketSize))) {
						return hr;
					}
					if(S_OK != (hr = Read(mp->avgPacketSize))) {
						return hr;
					}
					if(S_OK != (hr = Read(mp->tStart))) {
						return hr;
					}
					if(S_OK != (hr = Read(mp->tPreroll))) {
						return hr;
					}
					if(S_OK != (hr = Read(mp->tDuration))) {
						return hr;
					}
					UINT8 slen;
					if(S_OK != (hr = Read(slen))) {
						return hr;
					}
					if(slen > 0 && S_OK != (hr = ByteRead((BYTE*)mp->name.GetBufferSetLength(slen), slen))) {
						return hr;
					}
					if(S_OK != (hr = Read(slen))) {
						return hr;
					}
					if(slen > 0 && S_OK != (hr = ByteRead((BYTE*)mp->mime.GetBufferSetLength(slen), slen))) {
						return hr;
					}
					UINT32 tsdlen;
					if(S_OK != (hr = Read(tsdlen))) {
						return hr;
					}
					mp->typeSpecData.SetCount(tsdlen);
					if(tsdlen > 0 && S_OK != (hr = ByteRead(mp->typeSpecData.GetData(), tsdlen))) {
						return hr;
					}
					mp->width = mp->height = 0;
					mp->interlaced = mp->top_field_first = false;
					m_mps.AddTail(mp);
					break;
				}
				case 'DATA': {
					CAutoPtr<DataChunk> dc(DNew DataChunk);
					if(S_OK != (hr = Read(dc->nPackets))) {
						return hr;
					}
					if(S_OK != (hr = Read(dc->ptrNext))) {
						return hr;
					}
					dc->pos = GetPos();
					m_dcs.AddTail(dc);
					GetDimensions();
					break;
				}
				case 'INDX': {
					IndexChunkHeader ich;
					if(S_OK != (hr = Read(ich.nIndices))) {
						return hr;
					}
					if(S_OK != (hr = Read(ich.stream))) {
						return hr;
					}
					if(S_OK != (hr = Read(ich.ptrNext))) {
						return hr;
					}
					int stream = GetMasterStream();
					while(ich.nIndices-- > 0) {
						UINT16 object_version;
						if(S_OK != (hr = Read(object_version))) {
							return hr;
						}
						if(object_version == 0) {
							CAutoPtr<IndexRecord> ir(DNew IndexRecord);
							if(S_OK != (hr = Read(ir->tStart))) {
								return hr;
							}
							if(S_OK != (hr = Read(ir->ptrFilePos))) {
								return hr;
							}
							if(S_OK != (hr = Read(ir->packet))) {
								return hr;
							}
							if(ich.stream == stream) {
								m_irs.AddTail(ir);
							}
						}
					}
					break;
				}
				case '.SUB':
					if(hdr.size > sizeof(hdr)) {
						int size = hdr.size - sizeof(hdr);
						CAutoVectorPtr<char> buff;
						if(!buff.Allocate(size)) {
							return E_OUTOFMEMORY;
						}
						char* p = buff;
						if(S_OK != (hr = ByteRead((BYTE*)p, size))) {
							return hr;
						}
						for(char* end = p + size; p < end; ) {
							subtitle s;
							s.name = p;
							p += s.name.GetLength()+1;
							CStringA len(p);
							p += len.GetLength()+1;
							s.data = CStringA(p, strtol(len, NULL, 10));
							p += s.data.GetLength();
							m_subs.AddTail(s);
						}
					}
					break;
			}
		}

		if(hdr.object_id == 'CONT' && BitRead(32, true) == 'DATA') {
			hdr.size = GetPos() - pos;
		}

		ASSERT(hdr.object_id == 'DATA'
			   || GetPos() == pos + hdr.size
			   || GetPos() == pos + sizeof(hdr));

		pos += hdr.size;
		if(pos > GetPos()) {
			Seek(pos);
		}
	}

	return S_OK;
}

#define GetBits(n) GetBits2(n, p, bit_offset, bit_buffer)

unsigned int GetBits2(int n, unsigned char*& p, unsigned int& bit_offset, unsigned int& bit_buffer)
{
	unsigned int ret = ((unsigned int)bit_buffer >> (32-(n)));

	bit_offset += n;
	bit_buffer <<= n;
	if(bit_offset > (32-16)) {
		p += bit_offset >> 3;
		bit_offset &= 7;
		bit_buffer = (unsigned int)p[0] << 24;
		bit_buffer |= (unsigned int)p[1] << 16;
		bit_buffer |= (unsigned int)p[2] << 8;
		bit_buffer |= (unsigned int)p[3];
		bit_buffer <<= bit_offset;
	}

	return ret;
}

void GetDimensions(unsigned char* p, unsigned int* wi, unsigned int* hi)
{
	unsigned int w, h, c;

	const unsigned int cw[8] = {160, 176, 240, 320, 352, 640, 704, 0};
	const unsigned int ch1[8] = {120, 132, 144, 240, 288, 480, 0, 0};
	const unsigned int ch2[4] = {180, 360, 576, 0};

	unsigned int bit_offset = 0;
	unsigned int bit_buffer = *(unsigned int*)p;
	bswap(bit_buffer);

	GetBits(13);

	GetBits(13);

	w = cw[GetBits(3)];
	if(w == 0) {
		do {
			c = GetBits(8);
			w += (c << 2);
		} while(c == 255);
	}

	c = GetBits(3);

	h = ch1[c];
	if(h == 0) {
		c = ((c << 1) | GetBits(1)) & 3;

		h = ch2[c];
		if(h == 0) {
			do {
				c = GetBits(8);
				h += (c << 2);
			} while(c == 255);
		}
	}

	*wi = w;
	*hi = h;
}

void GetDimensions_X10(unsigned char* p, unsigned int* wi, unsigned int* hi,
					   bool *interlaced, bool *top_field_first, bool *repeat_field)
{
	unsigned int w, h, c;

	const unsigned int cw[8] = {160, 176, 240, 320, 352, 640, 704, 0};
	const unsigned int ch1[8] = {120, 132, 144, 240, 288, 480, 0, 0};
	const unsigned int ch2[4] = {180, 360, 576, 0};

	unsigned int bit_offset = 0;
	unsigned int bit_buffer = *(unsigned int*)p;
	bswap(bit_buffer);

	GetBits(9);

	*interlaced = false;
	*top_field_first = false;
	*repeat_field = false;
	c = GetBits(1);
	if (c) {
		c = GetBits(1);
		if (c) {
			*interlaced = true;
		}
		c = GetBits(1);
		if (c) {
			*top_field_first = true;
		}
		c = GetBits(1);
		if (c) {
			*repeat_field = true;
		}

		c = GetBits(1);
		c = GetBits(1);
		if (c) {
			GetBits(2);
		}
	}

	GetBits(16);

	w = cw[GetBits(3)];
	if(w == 0) {
		do {
			c = GetBits(8);
			w += (c << 2);
		} while(c == 255);
	}

	c = GetBits(3);

	h = ch1[c];
	if(h == 0) {
		c = ((c << 1) | GetBits(1)) & 3;

		h = ch2[c];
		if(h == 0) {
			do {
				c = GetBits(8);
				h += (c << 2);
			} while(c == 255);
		}
	}

	*wi = w;
	*hi = h;
}

void CRMFile::GetDimensions()
{
	POSITION pos = m_mps.GetHeadPosition();
	while(pos) {
		UINT64 filepos = GetPos();

		MediaProperies* pmp = m_mps.GetNext(pos);
		if(pmp->mime == "video/x-pn-realvideo") {
			pmp->width = pmp->height = 0;

			rvinfo rvi = *(rvinfo*)pmp->typeSpecData.GetData();
			rvi.bswap();

			if(rvi.fcc2 != '04VR' && rvi.fcc2 != '14VR') {
				continue;
			}

			MediaPacketHeader mph;
			while(S_OK == Read(mph)) {
				if(mph.stream != pmp->stream || mph.len == 0
						|| !(mph.flags&MediaPacketHeader::PN_KEYFRAME_FLAG)) {
					continue;
				}

				BYTE* p = mph.pData.GetData();
				BYTE* p0 = p;
				int len = mph.pData.GetCount();

				BYTE hdr = *p++;
				DWORD packetlen = 0, packetoffset = 0;

				if((hdr&0xc0) == 0x40) {
					packetlen = len - (++p - p0);
				} else {
					if((hdr&0x40) == 0) {
						p++;
					}

#define GetWORD(var) \
						var = (var<<8)|(*p++); \
						var = (var<<8)|(*p++); \
 
					GetWORD(packetlen);
					if((packetlen&0x4000) == 0) {
						GetWORD(packetlen);
						packetlen &= 0x3fffffff;
					} else {
						packetlen &= 0x3fff;
					}

					GetWORD(packetoffset);
					if((packetoffset&0x4000) == 0) {
						GetWORD(packetoffset);
						packetoffset &= 0x3fffffff;
					} else {
						packetoffset &= 0x3fff;
					}

#undef GetWORD

					if((hdr&0xc0) == 0xc0) {
						packetoffset = 0;
					} else if((hdr&0xc0) == 0x80) {
						packetoffset = packetlen - packetoffset;
					}

					p++;
				}

				len = min(len - (p - p0), packetlen - packetoffset);

				if(len > 0) {
					bool repeat_field;
					if(rvi.fcc2 == '14VR') {
						::GetDimensions_X10(p, &pmp->width, &pmp->height, &pmp->interlaced, &pmp->top_field_first, &repeat_field);
					} else {
						::GetDimensions(p, &pmp->width, &pmp->height);
					}

					if(rvi.w == pmp->width && rvi.h == pmp->height) {
						pmp->width = pmp->height = 0;
					}

					break;
				}
			}
		}

		Seek(filepos);
	}
}

int CRMFile::GetMasterStream()
{
	int s1 = -1, s2 = -1;

	POSITION pos = m_mps.GetHeadPosition();
	while(pos) {
		MediaProperies* pmp = m_mps.GetNext(pos);
		if(pmp->mime == "video/x-pn-realvideo") {
			s1 = pmp->stream;
			break;
		} else if(pmp->mime == "audio/x-pn-realaudio" && s2 == -1) {
			s2 = pmp->stream;
		}
	}

	if(s1 == -1) {
		s1 = s2;
	}

	return s1;
}


////////////////////////////

//
// CRealVideoDecoder
//

CRealVideoDecoder::CRealVideoDecoder(LPUNKNOWN lpunk, HRESULT* phr)
	: CBaseVideoFilter(NAME("CRealVideoDecoder"), lpunk, phr, __uuidof(this))
	, m_hDrvDll(NULL)
	, m_dwCookie(0)
	, m_lastBuffSizeDim(0)
{
}

CRealVideoDecoder::~CRealVideoDecoder()
{
	if(m_hDrvDll) {
		FreeLibrary(m_hDrvDll);
	}
}

HRESULT CRealVideoDecoder::InitRV(const CMediaType* pmt)
{
	FreeRV();

	HRESULT hr = VFW_E_TYPE_NOT_ACCEPTED;

	rvinfo rvi = *(rvinfo*)(pmt->Format() + (pmt->formattype == FORMAT_VideoInfo ? sizeof(VIDEOINFOHEADER) : sizeof(VIDEOINFOHEADER2)));
	rvi.bswap();

#pragma pack(push, 1)
	struct {
		WORD unk1, w, h, unk3;
		DWORD unk2, subformat, unk5, format;
	} i =
	{11, rvi.w, rvi.h, 0, 0, rvi.type1, 1, rvi.type2};
#pragma pack(pop)

	if(FAILED(hr = RVInit(&i, &m_dwCookie))) {
		return hr;
	}

	if(rvi.fcc2 <= '03VR' && rvi.type2 >= 0x20200002) {
		int nWidthHeight = (1+((rvi.type1>>16)&7));
		UINT32* pWH = DNew UINT32[nWidthHeight*2];
		pWH[0] = rvi.w;
		pWH[1] = rvi.h;
		for(int i = 2; i < nWidthHeight*2; i++) {
			pWH[i] = rvi.morewh[i-2]*4;
		}
#pragma pack(push, 1)
		struct {
			UINT32 data1;
			UINT32 data2;
			UINT32* dimensions;
		} cmsg_data =
		{0x24, nWidthHeight, pWH};
#pragma pack(pop)
		hr = RVCustomMessage(&cmsg_data, m_dwCookie);
		delete [] pWH;
	}

	return hr;
}

void CRealVideoDecoder::FreeRV()
{
	if(m_dwCookie) {
		RVFree(m_dwCookie);
		m_dwCookie = 0;
	}
}

HRESULT CRealVideoDecoder::Transform(IMediaSample* pIn)
{
	CAutoLock cAutoLock(&m_csReceive);

	HRESULT hr;

	BYTE* pDataIn = NULL;
	if(FAILED(hr = pIn->GetPointer(&pDataIn))) {
		return hr;
	}

	long len = pIn->GetActualDataLength();
	if(len <= 0) {
		return S_OK;    // nothing to do
	}

	REFERENCE_TIME rtStart, rtStop;
	pIn->GetTime(&rtStart, &rtStop);

	rtStart += m_tStart;

	int offset = 1+((*pDataIn)+1)*8;

#pragma pack(push, 1)
	struct {
		DWORD len, unk1, chunks;
		DWORD* extra;
		DWORD unk2, timestamp;
	} transform_in =
	{len - offset, 0, *pDataIn, (DWORD*)(pDataIn+1), 0, (DWORD)(rtStart/10000)};
	struct {
		DWORD unk1, unk2, timestamp, w, h;
	} transform_out =
	{0,0,0,0,0};
#pragma pack(pop)

	pDataIn += offset;

	if(m_fDropFrames && m_timestamp+1 == transform_in.timestamp) {
		m_timestamp = transform_in.timestamp;
		return S_OK;
	}


	unsigned int tmp1, tmp2;
	bool interlaced = false, tmp3, tmp4;
	::GetDimensions_X10(pDataIn, &tmp1, &tmp2, &interlaced, &tmp3, &tmp4);

	int size =  tmp1*tmp2;
	if( m_lastBuffSizeDim < size  ) {
		_aligned_free(m_pI420);
		_aligned_free(m_pI420Tmp);

		m_lastBuffSizeDim = size;
		ATLTRACE("resize out put buff %d" ,size);
		m_pI420 = static_cast<BYTE*>(_aligned_malloc(size*3/2, 16));
		if ( m_pI420 ) {
			ATLTRACE(" m_pI420.Allocated 1" );
			memset(m_pI420, 0, size);
			ATLTRACE(" m_pI420.Allocated 2" );
			memset(m_pI420 + size, 0x80, size/2);
			ATLTRACE(" m_pI420.Allocated 3" );
		} else {
			ATLTRACE(" m_pI420.Allocate fail %d" ,size*3/2);
			return S_OK;
		}
		m_pI420Tmp = static_cast<BYTE*>(_aligned_malloc(size*3/2, 16));
		if ( m_pI420Tmp ) {
			ATLTRACE(" m_pI420Tmp.Allocated 1" );
			memset(m_pI420Tmp, 0, size);
			ATLTRACE(" m_pI420Tmp.Allocated 2" );
			memset(m_pI420Tmp + size, 0x80, size/2);
			ATLTRACE(" m_pI420Tmp.Allocated 3" );
		} else {
			ATLTRACE(" m_pI420Tmp.Allocate fail %d" ,size*3/2);
			return S_OK;
		}
	}

	hr = RVTransform(pDataIn, (BYTE*)m_pI420, &transform_in, &transform_out, m_dwCookie);

	m_timestamp = transform_in.timestamp;

	if(FAILED(hr)) {
		TRACE(_T("RV returned an error code!!!\n"));
		ASSERT(!(transform_out.unk1&1));	// error allowed when the "render" flag is not set
		//		return hr;
	}

	if(pIn->IsPreroll() == S_OK || rtStart < 0 || !(transform_out.unk1&1)) {
		return S_OK;
	}

	CComPtr<IMediaSample> pOut;
	BYTE* pDataOut = NULL;
	if(/*FAILED(hr = GetDeliveryBuffer(transform_out.w, transform_out.h, &pOut)) // TODO
	&& */ FAILED(hr = GetDeliveryBuffer(m_w, m_h, &pOut))
		|| FAILED(hr = pOut->GetPointer(&pDataOut))) {
		return hr;
	}

	BYTE* pI420[3] = {m_pI420, m_pI420Tmp, NULL};

	if(interlaced) {
		int size = m_w*m_h;
		DeinterlaceBlend(pI420[1], pI420[0], m_w, m_h, m_w, m_w);
		DeinterlaceBlend(pI420[1]+size, pI420[0]+size, m_w/2, m_h/2, m_w/2, m_w/2);
		DeinterlaceBlend(pI420[1]+size*5/4, pI420[0]+size*5/4, m_w/2, m_h/2, m_w/2, m_w/2);
		pI420[2] = pI420[1], pI420[1] = pI420[0], pI420[0] = pI420[2];
	}

	if(transform_out.w != m_w || transform_out.h != m_h) {
		Resize(pI420[0], transform_out.w, transform_out.h, pI420[1], m_w, m_h);
		// only one of these can be true, and when it happens the result image must be in the tmp buffer
		if(transform_out.w == m_w || transform_out.h == m_h) {
			pI420[2] = pI420[1], pI420[1] = pI420[0], pI420[0] = pI420[2];
		}
	}

	rtStart = 10000i64*transform_out.timestamp - m_tStart;
	rtStop = rtStart + 1;
	pOut->SetTime(&rtStart, /*NULL*/&rtStop);

	pOut->SetDiscontinuity(pIn->IsDiscontinuity() == S_OK);

	CopyBuffer(pDataOut, pI420[0], m_w, m_h, m_w, MEDIASUBTYPE_I420);

	DbgLog((LOG_TRACE, 0, _T("V: rtStart=%I64d, rtStop=%I64d, disc=%d, sync=%d"),
			rtStart, rtStop, pOut->IsDiscontinuity() == S_OK, pOut->IsSyncPoint() == S_OK));

	return m_pOutput->Deliver(pOut);
}

void CRealVideoDecoder::Resize(BYTE* pIn, DWORD wi, DWORD hi, BYTE* pOut, DWORD wo, DWORD ho)
{
	int si = wi*hi, so = wo*ho;
	ASSERT(((si*so)&3) == 0);

	if(wi < wo) {
		ResizeWidth(pIn, wi, hi, pOut, wo, ho);
		ResizeWidth(pIn + si, wi/2, hi/2, pOut + so, wo/2, ho/2);
		ResizeWidth(pIn + si + si/4, wi/2, hi/2, pOut + so + so/4, wo/2, ho/2);
		if(hi == ho) {
			return;
		}
		ResizeHeight(pOut, wo, hi, pIn, wo, ho);
		ResizeHeight(pOut + so, wo/2, hi/2, pIn + so, wo/2, ho/2);
		ResizeHeight(pOut + so + so/4, wo/2, hi/2, pIn + so + so/4, wo/2, ho/2);
	} else if(hi < ho) {
		ResizeHeight(pIn, wi, hi, pOut, wo, ho);
		ResizeHeight(pIn + si, wi/2, hi/2, pOut + so, wo/2, ho/2);
		ResizeHeight(pIn + si + si/4, wi/2, hi/2, pOut + so + so/4, wo/2, ho/2);
		if(wi == wo) {
			return;
		}
		ASSERT(0);	// this is uncreachable code, but anyway... looks nice being so symmetric
		ResizeWidth(pOut, wi, ho, pIn, wo, ho);
		ResizeWidth(pOut + so, wi/2, ho/2, pIn + so, wo/2, ho/2);
		ResizeWidth(pOut + so + so/4, wi/2, ho/2, pIn + so + so/4, wo/2, ho/2);
	}
}

void CRealVideoDecoder::ResizeWidth(BYTE* pIn, DWORD wi, DWORD hi, BYTE* pOut, DWORD wo, DWORD ho)
{
	for(DWORD y = 0; y < hi; y++, pIn += wi, pOut += wo) {
		if(wi == wo) {
			memcpy(pOut, pIn, wo);
		} else {
			ResizeRow(pIn, wi, 1, pOut, wo, 1);
		}
	}
}

void CRealVideoDecoder::ResizeHeight(BYTE* pIn, DWORD wi, DWORD hi, BYTE* pOut, DWORD wo, DWORD ho)
{
	if(hi == ho) {
		memcpy(pOut, pIn, wo*ho);
	} else {
		for(DWORD x = 0; x < wo; x++, pIn++, pOut++) {
			ResizeRow(pIn, hi, wo, pOut, ho, wo);
		}
	}
}

void CRealVideoDecoder::ResizeRow(BYTE* pIn, DWORD wi, DWORD dpi, BYTE* pOut, DWORD wo, DWORD dpo)
{
	ASSERT(wi < wo);

	if(dpo == 1) {
		for(DWORD i = 0, j = 0, dj = (wi<<16)/wo; i < wo-1; i++, pOut++, j += dj)
			//			pOut[i] = pIn[j>>16];
		{
			BYTE* p = &pIn[j>>16];
			DWORD jf = j&0xffff;
			*pOut = ((p[0]*(0xffff-jf) + p[1]*jf) + 0x7fff) >> 16;
		}

		*pOut = pIn[wi-1];
	} else {
		for(DWORD i = 0, j = 0, dj = (wi<<16)/wo; i < wo-1; i++, pOut += dpo, j += dj)
			//			*pOut = pIn[dpi*(j>>16)];
		{
			BYTE* p = &pIn[dpi*(j>>16)];
			DWORD jf = j&0xffff;
			*pOut = ((p[0]*(0xffff-jf) + p[dpi]*jf) + 0x7fff) >> 16;
		}

		*pOut = pIn[dpi*(wi-1)];
	}
}

HRESULT CRealVideoDecoder::CheckInputType(const CMediaType* mtIn)
{
	if(mtIn->majortype != MEDIATYPE_Video
			|| mtIn->subtype != MEDIASUBTYPE_RV20
			&& mtIn->subtype != MEDIASUBTYPE_RV30
			&& mtIn->subtype != MEDIASUBTYPE_RV40
			&& mtIn->subtype != MEDIASUBTYPE_RV41) {
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	if(mtIn->formattype == FORMAT_VideoInfo2) {
		VIDEOINFOHEADER2* vih2 = (VIDEOINFOHEADER2*)mtIn->Format();
		if(vih2->dwPictAspectRatioX < vih2->bmiHeader.biWidth
				|| vih2->dwPictAspectRatioY < vih2->bmiHeader.biHeight) {
			return VFW_E_TYPE_NOT_ACCEPTED;
		}
	}

	if(!m_pInput->IsConnected()) {
		if(m_hDrvDll) {
			FreeLibrary(m_hDrvDll);
			m_hDrvDll = NULL;
		}

		CAtlList<CString> paths;
		CString olddll, newdll, oldpath, newpath;

		olddll.Format(_T("drv%c3260.dll"), (TCHAR)((mtIn->subtype.Data1>>16)&0xff));
		newdll =
			mtIn->subtype == FOURCCMap('14VR') ? _T("drvi.dll") :
			mtIn->subtype == FOURCCMap('02VR') ? _T("drv2.dll") :
			_T("drvc.dll");

		CRegKey key;
		TCHAR buff[_MAX_PATH];
		ULONG len = sizeof(buff);
		if(ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, _T("Software\\RealNetworks\\Preferences\\DT_Codecs"), KEY_READ)
				&& ERROR_SUCCESS == key.QueryStringValue(NULL, buff, &len) && _tcslen(buff) > 0) {
			oldpath = buff;
			TCHAR c = oldpath[oldpath.GetLength()-1];
			if(c != '\\' && c != '/') {
				oldpath += '\\';
			}
			key.Close();
		}
		len = sizeof(buff);
		if(ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, _T("Helix\\HelixSDK\\10.0\\Preferences\\DT_Codecs"), KEY_READ)
				&& ERROR_SUCCESS == key.QueryStringValue(NULL, buff, &len) && _tcslen(buff) > 0) {
			newpath = buff;
			TCHAR c = newpath[newpath.GetLength()-1];
			if(c != '\\' && c != '/') {
				newpath += '\\';
			}
			key.Close();
		}

		if(!newpath.IsEmpty()) {
			paths.AddTail(newpath + newdll);
		}
		if(!oldpath.IsEmpty()) {
			paths.AddTail(oldpath + newdll);
		}
		paths.AddTail(newdll);	// default dll paths
		if(!newpath.IsEmpty()) {
			paths.AddTail(newpath + olddll);
		}
		if(!oldpath.IsEmpty()) {
			paths.AddTail(oldpath + olddll);
		}
		paths.AddTail(olddll);	// default dll paths

		POSITION pos = paths.GetHeadPosition();
		do {
			if (pos) {
				m_hDrvDll = LoadLibrary(paths.GetNext(pos));
			}
		} while(pos && !m_hDrvDll);

		if(m_hDrvDll) {
			RVCustomMessage = (PRVCustomMessage)GetProcAddress(m_hDrvDll, "RV20toYUV420CustomMessage");
			RVFree = (PRVFree)GetProcAddress(m_hDrvDll, "RV20toYUV420Free");
			RVHiveMessage = (PRVHiveMessage)GetProcAddress(m_hDrvDll, "RV20toYUV420HiveMessage");
			RVInit = (PRVInit)GetProcAddress(m_hDrvDll, "RV20toYUV420Init");
			RVTransform = (PRVTransform)GetProcAddress(m_hDrvDll, "RV20toYUV420Transform");

			if(!RVCustomMessage) {
				RVCustomMessage = (PRVCustomMessage)GetProcAddress(m_hDrvDll, "RV40toYUV420CustomMessage");
			}
			if(!RVFree) {
				RVFree = (PRVFree)GetProcAddress(m_hDrvDll, "RV40toYUV420Free");
			}
			if(!RVHiveMessage) {
				RVHiveMessage = (PRVHiveMessage)GetProcAddress(m_hDrvDll, "RV40toYUV420HiveMessage");
			}
			if(!RVInit) {
				RVInit = (PRVInit)GetProcAddress(m_hDrvDll, "RV40toYUV420Init");
			}
			if(!RVTransform) {
				RVTransform = (PRVTransform)GetProcAddress(m_hDrvDll, "RV40toYUV420Transform");
			}
		}

		if(!m_hDrvDll || !RVCustomMessage
				|| !RVFree || !RVHiveMessage
				|| !RVInit || !RVTransform) {
			return VFW_E_TYPE_NOT_ACCEPTED;
		}

		if(FAILED(InitRV(mtIn))) {
			return VFW_E_TYPE_NOT_ACCEPTED;
		}
	}

	return S_OK;
}

HRESULT CRealVideoDecoder::CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut)
{
	if(m_pOutput && m_pOutput->IsConnected()) {
		BITMAPINFOHEADER bih1, bih2;
		if(ExtractBIH(mtOut, &bih1) && ExtractBIH(&m_pOutput->CurrentMediaType(), &bih2)
				&& abs(bih1.biHeight) != abs(bih2.biHeight)) {
			return VFW_E_TYPE_NOT_ACCEPTED;
		}
	}

	return __super::CheckTransform(mtIn, mtOut);
}

HRESULT CRealVideoDecoder::StartStreaming()
{
	const CMediaType& mt = m_pInput->CurrentMediaType();
	if(FAILED(InitRV(&mt))) {
		return E_FAIL;
	}

	int size = m_w*m_h;
	m_lastBuffSizeDim = size;
	m_pI420 = static_cast<BYTE*>(_aligned_malloc(size*3/2, 16));
	memset(m_pI420, 0, size);
	memset(m_pI420 + size, 0x80, size/2);
	m_pI420Tmp = static_cast<BYTE*>(_aligned_malloc(size*3/2, 16));
	memset(m_pI420Tmp, 0, size);
	memset(m_pI420Tmp + size, 0x80, size/2);

	return __super::StartStreaming();
}

HRESULT CRealVideoDecoder::StopStreaming()
{
	_aligned_free(m_pI420);
	_aligned_free(m_pI420Tmp);

	FreeRV();

	return __super::StopStreaming();
}

HRESULT CRealVideoDecoder::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	CAutoLock cAutoLock(&m_csReceive);

	m_timestamp = (DWORD)~0;
	m_fDropFrames = false;

	DWORD tmp[2] = {20, 0};
	RVHiveMessage(tmp, m_dwCookie);

	m_tStart = tStart;
	return __super::NewSegment(tStart, tStop, dRate);
}

HRESULT CRealVideoDecoder::AlterQuality(Quality q)
{
	if(q.Late > 500*10000i64) {
		m_fDropFrames = true;
	}
	if(q.Late <= 0) {
		m_fDropFrames = false;
	}
	//	TRACE(_T("CRealVideoDecoder::AlterQuality: Type=%d, Proportion=%d, Late=%I64d, TimeStamp=%I64d\n"), q.Type, q.Proportion, q.Late, q.TimeStamp);
	return E_NOTIMPL;
}

/////////////////////////

//
// CRealAudioDecoder
//

CRealAudioDecoder::CRealAudioDecoder(LPUNKNOWN lpunk, HRESULT* phr)
	: CTransformFilter(NAME("CRealAudioDecoder"), lpunk, __uuidof(this))
	, m_hDrvDll(NULL)
	, m_dwCookie(0)
{
	if(phr) {
		*phr = S_OK;
	}
}

CRealAudioDecoder::~CRealAudioDecoder()
{
	//	FreeRA();
	if(m_hDrvDll) {
		FreeLibrary(m_hDrvDll);
	}
}

HRESULT CRealAudioDecoder::InitRA(const CMediaType* pmt)
{
	FreeRA();

	HRESULT hr;

	if(RAOpenCodec2 && FAILED(hr = RAOpenCodec2(&m_dwCookie, m_dllpath))
			|| RAOpenCodec && FAILED(hr = RAOpenCodec(&m_dwCookie))) {
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	WAVEFORMATEX* pwfe = (WAVEFORMATEX*)pmt->Format();

	// someone might be doing cbSize = sizeof(WAVEFORMATEX), chances of
	// cbSize being really sizeof(WAVEFORMATEX) is less than this,
	// especially with our rm splitter ;)
	DWORD cbSize = pwfe->cbSize;
	if(cbSize == sizeof(WAVEFORMATEX)) {
		ASSERT(0);
		cbSize = 0;
	}

	WORD wBitsPerSample = pwfe->wBitsPerSample;
	if(!wBitsPerSample) {
		wBitsPerSample = 16;
	}

#pragma pack(push, 1)
	struct {
		DWORD freq;
		WORD bpsample, channels, quality;
		DWORD bpframe, packetsize, extralen;
		void* extra;
	} initdata = {
		pwfe->nSamplesPerSec, wBitsPerSample, pwfe->nChannels, 100,
		0, 0, 0, NULL
	};
#pragma pack(pop)

	CAutoVectorPtr<BYTE> pBuff;

	if(pmt->subtype == MEDIASUBTYPE_AAC) {
		pBuff.Allocate(cbSize+1);
		pBuff[0] = 0x02;
		memcpy(pBuff+1, pwfe+1, cbSize);
		initdata.extralen = cbSize+1;
		initdata.extra = pBuff;
	} else {
		if(pmt->FormatLength() <= sizeof(WAVEFORMATEX) + cbSize) {	// must have type_specific_data appended
			return VFW_E_TYPE_NOT_ACCEPTED;
		}

		BYTE* fmt = pmt->Format() + sizeof(WAVEFORMATEX) + cbSize;

		for(int i = 0, len = pmt->FormatLength() - (sizeof(WAVEFORMATEX) + cbSize); i < len-4; i++, fmt++) {
			if(fmt[0] == '.' || fmt[1] == 'r' || fmt[2] == 'a') {
				break;
			}
		}

		m_rai = *(rainfo*)fmt;
		m_rai.bswap();

		BYTE* p;

		if(m_rai.version2 == 4) {
			p = (BYTE*)((rainfo4*)fmt+1);
			int len = *p++;
			p += len;
			len = *p++;
			p += len;
			ASSERT(len == 4);
		} else if(m_rai.version2 == 5) {
			p = (BYTE*)((rainfo5*)fmt+1);
		} else {
			return VFW_E_TYPE_NOT_ACCEPTED;
		}

		p += 3;
		if(m_rai.version2 == 5) {
			p++;
		}

		initdata.bpframe = m_rai.sub_packet_size;
		initdata.packetsize = m_rai.coded_frame_size;
		initdata.extralen = min((pmt->Format() + pmt->FormatLength()) - (p + 4), *(DWORD*)p);
		initdata.extra = p + 4;
	}

	if(FAILED(hr = RAInitDecoder(m_dwCookie, &initdata))) {
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	if(RASetPwd) {
		RASetPwd(m_dwCookie, "Ardubancel Quazanga");
	}

	if(RASetFlavor && FAILED(hr = RASetFlavor(m_dwCookie, m_rai.flavor))) {
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	return S_OK;
}

void CRealAudioDecoder::FreeRA()
{
	if(m_dwCookie) {
		RAFreeDecoder(m_dwCookie);
		RACloseCodec(m_dwCookie);
		m_dwCookie = 0;
	}
}

HRESULT CRealAudioDecoder::Receive(IMediaSample* pIn)
{
	CAutoLock cAutoLock(&m_csReceive);

	HRESULT hr;

	AM_SAMPLE2_PROPERTIES* const pProps = m_pInput->SampleProps();
	if(pProps->dwStreamId != AM_STREAM_MEDIA) {
		return m_pOutput->Deliver(pIn);
	}

	BYTE* pDataIn = NULL;
	if(FAILED(hr = pIn->GetPointer(&pDataIn))) {
		return hr;
	}
	BYTE* pDataInOrg = pDataIn;
	UNUSED_ALWAYS(pDataInOrg);

	long len = pIn->GetActualDataLength();
	if(len <= 0) {
		return S_OK;
	}

	REFERENCE_TIME rtStart, rtStop;
	pIn->GetTime(&rtStart, &rtStop);
	/*
		if(pIn->IsPreroll() == S_OK || rtStart < 0)
			return S_OK;
	*/

	if(S_OK == pIn->IsSyncPoint()) {
		m_bufflen = 0;
		m_rtBuffStart = rtStart;
		m_fBuffDiscontinuity = pIn->IsDiscontinuity() == S_OK;
	}

	BYTE* src = NULL;
	BYTE* dst = NULL;

	int w = m_rai.coded_frame_size;
	int h = m_rai.sub_packet_h;
	int sps = m_rai.sub_packet_size;

	if(m_pInput->CurrentMediaType().subtype == MEDIASUBTYPE_RAAC
			|| m_pInput->CurrentMediaType().subtype == MEDIASUBTYPE_RACP
			|| m_pInput->CurrentMediaType().subtype == MEDIASUBTYPE_AAC) {
		src = pDataIn;
		dst = pDataIn + len;
		w = len;
	} else {
		memcpy(&m_buff[m_bufflen], pDataIn, len);
		m_bufflen += len;

		len = w*h;

		if(m_bufflen >= len) {
			ASSERT(m_bufflen == len);

			src = m_buff;
			dst = m_buff + len;

			if(sps > 0
					&& (m_pInput->CurrentMediaType().subtype == MEDIASUBTYPE_COOK
						|| m_pInput->CurrentMediaType().subtype == MEDIASUBTYPE_ATRC)) {
				for(int y = 0; y < h; y++) {
					for(int x = 0, w2 = w / sps; x < w2; x++) {
						// TRACE(_T("--- %d, %d\n"), (h*x+((h+1)/2)*(y&1)+(y>>1)), sps*(h*x+((h+1)/2)*(y&1)+(y>>1)));
						memcpy(dst + sps*(h*x+((h+1)/2)*(y&1)+(y>>1)), src, sps);
						src += sps;
					}
				}

				src = m_buff + len;
				dst = m_buff + len*2;
			} else if(m_pInput->CurrentMediaType().subtype == MEDIASUBTYPE_SIPR) {
				// http://mplayerhq.hu/pipermail/mplayer-dev-eng/2002-August/010569.html

				static BYTE sipr_swaps[38][2]= {
					{0,63},{1,22},{2,44},{3,90},{5,81},{7,31},{8,86},{9,58},{10,36},{12,68},
					{13,39},{14,73},{15,53},{16,69},{17,57},{19,88},{20,34},{21,71},{24,46},
					{25,94},{26,54},{28,75},{29,50},{32,70},{33,92},{35,74},{38,85},{40,56},
					{42,87},{43,65},{45,59},{48,79},{49,93},{51,89},{55,95},{61,76},{67,83},
					{77,80}
				};

				int bs=h*w*2/96;	// nibbles per subpacket
				for(int n=0; n<38; n++) {
					int i=bs*sipr_swaps[n][0];
					int o=bs*sipr_swaps[n][1];
					// swap nibbles of block 'i' with 'o'      TODO: optimize
					for(int j=0; j<bs; j++) {
						int x=(i&1) ? (src[(i>>1)]>>4) : (src[(i>>1)]&15);
						int y=(o&1) ? (src[(o>>1)]>>4) : (src[(o>>1)]&15);
						if(o&1) {
							src[(o>>1)]=(src[(o>>1)]&0x0F)|(x<<4);
						} else {
							src[(o>>1)]=(src[(o>>1)]&0xF0)|x;
						}
						if(i&1) {
							src[(i>>1)]=(src[(i>>1)]&0x0F)|(y<<4);
						} else {
							src[(i>>1)]=(src[(i>>1)]&0xF0)|y;
						}
						++i;
						++o;
					}
				}
			}

			m_bufflen = 0;
		}
	}

	rtStart = m_rtBuffStart;

	for(; src < dst; src += w) {
		CComPtr<IMediaSample> pOut;
		BYTE* pDataOut = NULL;
		if(FAILED(hr = m_pOutput->GetDeliveryBuffer(&pOut, NULL, NULL, 0))
				|| FAILED(hr = pOut->GetPointer(&pDataOut))) {
			return hr;
		}

		AM_MEDIA_TYPE* pmt;
		if(SUCCEEDED(pOut->GetMediaType(&pmt)) && pmt) {
			CMediaType mt(*pmt);
			m_pOutput->SetMediaType(&mt);
			DeleteMediaType(pmt);
		}

		hr = RADecode(m_dwCookie, src, w, pDataOut, &len, -1);

		if(FAILED(hr)) {
			TRACE(_T("RA returned an error code!!!\n"));
			continue;
			//			return hr;
		}

		WAVEFORMATEX* pwfe = (WAVEFORMATEX*)m_pOutput->CurrentMediaType().Format();

		rtStop = rtStart + 1000i64*len/pwfe->nAvgBytesPerSec*10000;
		pOut->SetTime(&rtStart, &rtStop);
		pOut->SetMediaTime(NULL, NULL);

		pOut->SetDiscontinuity(m_fBuffDiscontinuity);
		m_fBuffDiscontinuity = false;
		pOut->SetSyncPoint(TRUE);

		pOut->SetActualDataLength(len);

		DbgLog((LOG_TRACE, 0, _T("A: rtStart=%I64d, rtStop=%I64d, disc=%d, sync=%d"),
				rtStart, rtStop, pOut->IsDiscontinuity() == S_OK, pOut->IsSyncPoint() == S_OK));

		if(rtStart >= 0 && S_OK != (hr = m_pOutput->Deliver(pOut))) {
			return hr;
		}

		rtStart = rtStop;
	}

	m_rtBuffStart = rtStart;

	return S_OK;
}

HRESULT CRealAudioDecoder::CheckInputType(const CMediaType* mtIn)
{
	if(mtIn->majortype != MEDIATYPE_Audio
			|| mtIn->subtype != MEDIASUBTYPE_14_4
			&& mtIn->subtype != MEDIASUBTYPE_28_8
			&& mtIn->subtype != MEDIASUBTYPE_ATRC
			&& mtIn->subtype != MEDIASUBTYPE_COOK
			&& mtIn->subtype != MEDIASUBTYPE_DNET
			&& mtIn->subtype != MEDIASUBTYPE_SIPR
			&& mtIn->subtype != MEDIASUBTYPE_RAAC
			&& mtIn->subtype != MEDIASUBTYPE_RACP
			&& mtIn->subtype != MEDIASUBTYPE_AAC) {
		return VFW_E_TYPE_NOT_ACCEPTED;
	}

	if(!m_pInput->IsConnected()) {
		if(m_hDrvDll) {
			FreeLibrary(m_hDrvDll);
			m_hDrvDll = NULL;
		}

		CAtlList<CString> paths;
		CString olddll, newdll, oldpath, newpath;

		TCHAR fourcc[5] = {
			(TCHAR)((mtIn->subtype.Data1>>0)&0xff),
			(TCHAR)((mtIn->subtype.Data1>>8)&0xff),
			(TCHAR)((mtIn->subtype.Data1>>16)&0xff),
			(TCHAR)((mtIn->subtype.Data1>>24)&0xff),
			0
		};

		if(!_tcscmp(_T("RACP"), fourcc) || !_tcscmp(_T("\xff"), fourcc)) {
			_tcscpy(fourcc, _T("RAAC"));
		}

		olddll.Format(_T("%s3260.dll"), fourcc);
		newdll.Format(_T("%s.dll"), fourcc);

		CRegKey key;
		TCHAR buff[_MAX_PATH];
		ULONG len = sizeof(buff);
		if(ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, _T("Software\\RealNetworks\\Preferences\\DT_Codecs"), KEY_READ)
				&& ERROR_SUCCESS == key.QueryStringValue(NULL, buff, &len) && _tcslen(buff) > 0) {
			oldpath = buff;
			TCHAR c = oldpath[oldpath.GetLength()-1];
			if(c != '\\' && c != '/') {
				oldpath += '\\';
			}
			key.Close();
		}
		len = sizeof(buff);
		if(ERROR_SUCCESS == key.Open(HKEY_CLASSES_ROOT, _T("Helix\\HelixSDK\\10.0\\Preferences\\DT_Codecs"), KEY_READ)
				&& ERROR_SUCCESS == key.QueryStringValue(NULL, buff, &len) && _tcslen(buff) > 0) {
			newpath = buff;
			TCHAR c = newpath[newpath.GetLength()-1];
			if(c != '\\' && c != '/') {
				newpath += '\\';
			}
			key.Close();
		}

		if(!newpath.IsEmpty()) {
			paths.AddTail(newpath + newdll);
		}
		if(!oldpath.IsEmpty()) {
			paths.AddTail(oldpath + newdll);
		}
		paths.AddTail(newdll);	// default dll paths
		if(!newpath.IsEmpty()) {
			paths.AddTail(newpath + olddll);
		}
		if(!oldpath.IsEmpty()) {
			paths.AddTail(oldpath + olddll);
		}
		paths.AddTail(olddll);	// default dll paths

		POSITION pos = paths.GetHeadPosition();
		do {
			if (pos) {
				m_hDrvDll = LoadLibrary(paths.GetNext(pos));
			}
		} while(pos && !m_hDrvDll);

		if(m_hDrvDll) {
			RACloseCodec = (PCloseCodec)GetProcAddress(m_hDrvDll, "RACloseCodec");
			RADecode = (PDecode)GetProcAddress(m_hDrvDll, "RADecode");
			RAFlush = (PFlush)GetProcAddress(m_hDrvDll, "RAFlush");
			RAFreeDecoder = (PFreeDecoder)GetProcAddress(m_hDrvDll, "RAFreeDecoder");
			RAGetFlavorProperty = (PGetFlavorProperty)GetProcAddress(m_hDrvDll, "RAGetFlavorProperty");
			RAInitDecoder = (PInitDecoder)GetProcAddress(m_hDrvDll, "RAInitDecoder");
			RAOpenCodec = (POpenCodec)GetProcAddress(m_hDrvDll, "RAOpenCodec");
			RAOpenCodec2 = (POpenCodec2)GetProcAddress(m_hDrvDll, "RAOpenCodec2");
			RASetFlavor = (PSetFlavor)GetProcAddress(m_hDrvDll, "RASetFlavor");
			RASetDLLAccessPath = (PSetDLLAccessPath)GetProcAddress(m_hDrvDll, "RASetDLLAccessPath");
			RASetPwd = (PSetPwd)GetProcAddress(m_hDrvDll, "RASetPwd");
		}

		if(!m_hDrvDll || !RACloseCodec || !RADecode /*|| !RAFlush*/
				|| !RAFreeDecoder || !RAGetFlavorProperty || !RAInitDecoder
				|| !(RAOpenCodec || RAOpenCodec2) /*|| !RASetFlavor*/) {
			return VFW_E_TYPE_NOT_ACCEPTED;
		}

		if(m_hDrvDll) {
			char buff[_MAX_PATH];
			GetModuleFileNameA(m_hDrvDll, buff, MAX_PATH);
			CPathA p(buff);
			p.RemoveFileSpec();
			p.AddBackslash();
			m_dllpath = p.m_strPath;
			if(RASetDLLAccessPath) {
				RASetDLLAccessPath("DT_Codecs=" + m_dllpath);
			}
		}

		if(FAILED(InitRA(mtIn))) {
			return VFW_E_TYPE_NOT_ACCEPTED;
		}
	}

	return S_OK;
}

HRESULT CRealAudioDecoder::CheckTransform(const CMediaType* mtIn, const CMediaType* mtOut)
{
	return mtIn->majortype == MEDIATYPE_Audio && (mtIn->subtype == MEDIASUBTYPE_14_4
			|| mtIn->subtype == MEDIASUBTYPE_28_8
			|| mtIn->subtype == MEDIASUBTYPE_ATRC
			|| mtIn->subtype == MEDIASUBTYPE_COOK
			|| mtIn->subtype == MEDIASUBTYPE_DNET
			|| mtIn->subtype == MEDIASUBTYPE_SIPR
			|| mtIn->subtype == MEDIASUBTYPE_RAAC
			|| mtIn->subtype == MEDIASUBTYPE_RACP
			|| mtIn->subtype == MEDIASUBTYPE_AAC)
		   && mtOut->majortype == MEDIATYPE_Audio && mtOut->subtype == MEDIASUBTYPE_PCM
		   ? S_OK
		   : VFW_E_TYPE_NOT_ACCEPTED;
}

HRESULT CRealAudioDecoder::DecideBufferSize(IMemAllocator* pAllocator, ALLOCATOR_PROPERTIES* pProperties)
{
	if(m_pInput->IsConnected() == FALSE) {
		return E_UNEXPECTED;
	}

	CComPtr<IMemAllocator> pAllocatorIn;
	m_pInput->GetAllocator(&pAllocatorIn);
	if(!pAllocatorIn) {
		return E_UNEXPECTED;
	}

	WAVEFORMATEX* pwfe = (WAVEFORMATEX*)m_pOutput->CurrentMediaType().Format();

	WORD wBitsPerSample = pwfe->wBitsPerSample;
	if(!wBitsPerSample) {
		wBitsPerSample = 16;
	}

	// ok, maybe this is too much...
	pProperties->cBuffers = 8;
	pProperties->cbBuffer = pwfe->nChannels*pwfe->nSamplesPerSec*wBitsPerSample>>3; // nAvgBytesPerSec;
	pProperties->cbAlign = 1;
	pProperties->cbPrefix = 0;

	HRESULT hr;
	ALLOCATOR_PROPERTIES Actual;
	if(FAILED(hr = pAllocator->SetProperties(pProperties, &Actual))) {
		return hr;
	}

	return(pProperties->cBuffers > Actual.cBuffers || pProperties->cbBuffer > Actual.cbBuffer
		   ? E_FAIL
		   : NOERROR);
}

HRESULT CRealAudioDecoder::GetMediaType(int iPosition, CMediaType* pmt)
{
	if(m_pInput->IsConnected() == FALSE) {
		return E_UNEXPECTED;
	}

	*pmt = m_pInput->CurrentMediaType();
	pmt->subtype = MEDIASUBTYPE_PCM;
	WAVEFORMATEX* pwfe = (WAVEFORMATEX*)pmt->ReallocFormatBuffer(sizeof(WAVEFORMATEX));

	if(iPosition < 0) {
		return E_INVALIDARG;
	}
	if(iPosition > (pwfe->nChannels > 2 && pwfe->nChannels <= 6 ? 1 : 0)) {
		return VFW_S_NO_MORE_ITEMS;
	}

	if(!pwfe->wBitsPerSample) {
		pwfe->wBitsPerSample = 16;
	}

	pwfe->cbSize = 0;
	pwfe->wFormatTag = WAVE_FORMAT_PCM;
	pwfe->nBlockAlign = pwfe->nChannels*pwfe->wBitsPerSample>>3;
	pwfe->nAvgBytesPerSec = pwfe->nSamplesPerSec*pwfe->nBlockAlign;

	if(iPosition == 0 && pwfe->nChannels > 2 && pwfe->nChannels <= 6) {
		static DWORD chmask[] = {
			KSAUDIO_SPEAKER_DIRECTOUT,
			KSAUDIO_SPEAKER_MONO,
			KSAUDIO_SPEAKER_STEREO,
			KSAUDIO_SPEAKER_STEREO|SPEAKER_FRONT_CENTER,
			KSAUDIO_SPEAKER_QUAD,
			KSAUDIO_SPEAKER_QUAD|SPEAKER_FRONT_CENTER,
			KSAUDIO_SPEAKER_5POINT1
		};

		WAVEFORMATEXTENSIBLE* pwfex = (WAVEFORMATEXTENSIBLE*)pmt->ReallocFormatBuffer(sizeof(WAVEFORMATEXTENSIBLE));
		pwfex->Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
		pwfex->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
		pwfex->dwChannelMask = chmask[pwfex->Format.nChannels];
		pwfex->Samples.wValidBitsPerSample = pwfex->Format.wBitsPerSample;
		pwfex->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
	}

	return S_OK;
}

HRESULT CRealAudioDecoder::StartStreaming()
{
	int w = m_rai.coded_frame_size;
	int h = m_rai.sub_packet_h;
	int sps = m_rai.sub_packet_size;
	UNUSED_ALWAYS(sps);

	int len = w*h;

	m_buff.Allocate(len*2);
	m_bufflen = 0;
	m_rtBuffStart = 0;

	return __super::StartStreaming();
}

HRESULT CRealAudioDecoder::StopStreaming()
{
	m_buff.Free();
	m_bufflen = 0;

	return __super::StopStreaming();
}

HRESULT CRealAudioDecoder::EndOfStream()
{
	return __super::EndOfStream();
}

HRESULT CRealAudioDecoder::BeginFlush()
{
	return __super::BeginFlush();
}

HRESULT CRealAudioDecoder::EndFlush()
{
	return __super::EndFlush();
}

HRESULT CRealAudioDecoder::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	CAutoLock cAutoLock(&m_csReceive);
	m_tStart = tStart;
	m_bufflen = 0;
	m_rtBuffStart = 0;
	return __super::NewSegment(tStart, tStop, dRate);
}
