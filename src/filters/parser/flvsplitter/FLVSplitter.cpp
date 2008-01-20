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

#include "StdAfx.h"
#include "FLVSplitter.h"
#include "..\..\..\DSUtil\DSUtil.h"

#include <initguid.h>
#include <moreuuids.h>

#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] =
{
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_FLV},
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_PIN sudpPins[] =
{
	{L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesIn), sudPinTypesIn},
	{L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, 0, NULL}
};

const AMOVIESETUP_MEDIATYPE sudPinTypesIn2[] =
{
	{&MEDIATYPE_Video, &MEDIASUBTYPE_FLV4},
	{&MEDIATYPE_Video, &MEDIASUBTYPE_VP62},
};

const AMOVIESETUP_MEDIATYPE sudPinTypesOut2[] =
{
	{&MEDIATYPE_Video, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_PIN sudpPins2[] =
{
    {L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesIn2), sudPinTypesIn2},
    {L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesOut2), sudPinTypesOut2}
};

const AMOVIESETUP_FILTER sudFilter[] =
{
	{&__uuidof(CFLVSplitterFilter), L"FLV Splitter", MERIT_NORMAL, countof(sudpPins), sudpPins},
	{&__uuidof(CFLVSourceFilter), L"FLV Source", MERIT_NORMAL, 0, NULL},
};

CFactoryTemplate g_Templates[] =
{
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

#include "..\..\FilterApp.h"

CFilterApp theApp;

#endif

//
// CFLVSplitterFilter
//

CFLVSplitterFilter::CFLVSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr)
	: CBaseSplitterFilter(NAME("CFLVSplitterFilter"), pUnk, phr, __uuidof(this))
{
}

bool CFLVSplitterFilter::ReadTag(Tag& t)
{
	if(m_pFile->GetRemaining() < 15) 
		return false;

	t.PreviousTagSize = (UINT32)m_pFile->BitRead(32);
	t.TagType = (BYTE)m_pFile->BitRead(8);
	t.DataSize = (UINT32)m_pFile->BitRead(24);
	t.TimeStamp = (UINT32)m_pFile->BitRead(24);
	t.Reserved = (UINT32)m_pFile->BitRead(32);

	return m_pFile->GetRemaining() >= t.DataSize;
}

bool CFLVSplitterFilter::ReadTag(AudioTag& at)
{
	if(!m_pFile->GetRemaining()) 
		return false;

	at.SoundFormat = (BYTE)m_pFile->BitRead(4);
	at.SoundRate = (BYTE)m_pFile->BitRead(2);
	at.SoundSize = (BYTE)m_pFile->BitRead(1);
	at.SoundType = (BYTE)m_pFile->BitRead(1);

	return true;
}

bool CFLVSplitterFilter::ReadTag(VideoTag& vt)
{
	if(!m_pFile->GetRemaining()) 
		return false;

	vt.FrameType = (BYTE)m_pFile->BitRead(4);
	vt.CodecID = (BYTE)m_pFile->BitRead(4);

	return true;
}

bool CFLVSplitterFilter::ReadTag(VideoTweak& vt)
{
	if(!m_pFile->GetRemaining()) 
		return false;

	vt.x = (BYTE)m_pFile->BitRead(4);
	vt.y = (BYTE)m_pFile->BitRead(4);

	return true;
}

bool CFLVSplitterFilter::Sync(__int64& pos)
{
	m_pFile->Seek(pos);

	while(m_pFile->GetRemaining() >= 11)
	{
		int limit = m_pFile->GetRemaining();

		BYTE b;
		do {b = m_pFile->BitRead(8);}
		while(b != 8 && b != 9 && limit-- > 0);

		pos = m_pFile->GetPos();

		UINT32 DataSize = (UINT32)m_pFile->BitRead(24);
		UINT32 TimeStamp = (UINT32)m_pFile->BitRead(24);
		UINT32 Reserved = (UINT32)m_pFile->BitRead(32);

		__int64 next = m_pFile->GetPos() + DataSize;
		
		if(next <= m_pFile->GetLength())
		{
			m_pFile->Seek(next);

			if(next == m_pFile->GetLength() || m_pFile->BitRead(32) == DataSize + 11)
			{
				m_pFile->Seek(pos -= 5);
				return true;
			}
		}

		m_pFile->Seek(pos);
	}

	return false;
}

HRESULT CFLVSplitterFilter::CreateOutputs(IAsyncReader* pAsyncReader)
{
	CheckPointer(pAsyncReader, E_POINTER);

	HRESULT hr = E_FAIL;

	m_pFile.Free();
	m_pFile.Attach(new CBaseSplitterFileEx(pAsyncReader, hr, DEFAULT_CACHE_LENGTH, false));
	if(!m_pFile) return E_OUTOFMEMORY;
	if(FAILED(hr)) {m_pFile.Free(); return hr;}

	m_rtNewStart = m_rtCurrent = 0;
	m_rtNewStop = m_rtStop = m_rtDuration = 0;

	if(m_pFile->BitRead(24) != 'FLV' || m_pFile->BitRead(8) != 1)
		return E_FAIL;

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

	m_pFile->Seek(m_DataOffset);

	for(int i = 0; ReadTag(t) && (fTypeFlagsVideo || fTypeFlagsAudio) && i < 100; i++)
	{
		UINT64 next = m_pFile->GetPos() + t.DataSize;

		CStringW name;

		CMediaType mt;
		mt.SetSampleSize(1);
		mt.subtype = GUID_NULL;

		if(t.TagType == 8 && fTypeFlagsAudio)
		{
			UNREFERENCED_PARAMETER(at);
			AudioTag at;
			if(ReadTag(at))
			{
				fTypeFlagsAudio = false;
				name = L"Audio";

				mt.majortype = MEDIATYPE_Audio;
				mt.formattype = FORMAT_WaveFormatEx;
				WAVEFORMATEX* wfe = (WAVEFORMATEX*)mt.AllocFormatBuffer(sizeof(WAVEFORMATEX));
				memset(wfe, 0, sizeof(WAVEFORMATEX));
				wfe->nSamplesPerSec = 44100*(1<<at.SoundRate)/8;
				wfe->wBitsPerSample = 8*(at.SoundSize+1);
				wfe->nChannels = 1*(at.SoundType+1);
				
				switch(at.SoundFormat)
				{
				case 0: // FLV_CODECID_PCM_BE
					mt.subtype = FOURCCMap(wfe->wFormatTag = WAVE_FORMAT_PCM);
					break;
				case 1: // FLV_CODECID_ADPCM
					mt.subtype = FOURCCMap(MAKEFOURCC('A','S','W','F'));
					break;
				case 2:	// FLV_CODECID_MP3
					mt.subtype = FOURCCMap(wfe->wFormatTag = WAVE_FORMAT_MP3);

					{
						CBaseSplitterFileEx::mpahdr h;
						CMediaType mt2;
						if(m_pFile->Read(h, 4, false, &mt2))
							mt = mt2;
					}
					break;
				case 3 :	// FLV_CODECID_PCM_LE
					// ToDo
					break;
				case 4 :	// unknown
					break;
				case 5 :	// FLV_CODECID_NELLYMOSER_8HZ_MONO
					mt.subtype = FOURCCMap(MAKEFOURCC('N','E','L','L'));
					break;
				case 6 :	// FLV_CODECID_NELLYMOSER
					mt.subtype = FOURCCMap(MAKEFOURCC('N','E','L','L'));
					break;
				}
			}
		}
		else if(t.TagType == 9 && fTypeFlagsVideo)
		{
			UNREFERENCED_PARAMETER(vt);
			VideoTag vt;
			if(ReadTag(vt) && vt.FrameType == 1)
			{
				fTypeFlagsVideo = false;
				name = L"Video";

				mt.majortype = MEDIATYPE_Video;
				mt.formattype = FORMAT_VideoInfo;
				VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)mt.AllocFormatBuffer(sizeof(VIDEOINFOHEADER));
				memset(vih, 0, sizeof(VIDEOINFOHEADER));

				BITMAPINFOHEADER* bih = &vih->bmiHeader;

				int w, h, arx, ary;

				switch(vt.CodecID)
				{
				case 2: 
					if(m_pFile->BitRead(17) != 1) break;

					m_pFile->BitRead(13); // Version (5), TemporalReference (8)

					switch(BYTE PictureSize = (BYTE)m_pFile->BitRead(3)) // w00t
					{
					case 0: case 1:
						vih->bmiHeader.biWidth = (WORD)m_pFile->BitRead(8*(PictureSize+1));
						vih->bmiHeader.biHeight = (WORD)m_pFile->BitRead(8*(PictureSize+1));
						break;
					case 2: case 3: case 4: 
						vih->bmiHeader.biWidth = 704 / PictureSize;
						vih->bmiHeader.biHeight = 576 / PictureSize;
						break;
					case 5: case 6: 
						PictureSize -= 3;
						vih->bmiHeader.biWidth = 640 / PictureSize;
						vih->bmiHeader.biHeight = 480 / PictureSize;
						break;
					}

					if(!vih->bmiHeader.biWidth || !vih->bmiHeader.biHeight) break;

					mt.subtype = FOURCCMap(vih->bmiHeader.biCompression = '1VLF');

					break;

				case 5:
					m_pFile->BitRead(24);
				case 4:
					
					VideoTweak fudge;
					ReadTag(fudge);
				
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

					if(arx && arx != w || ary && ary != h)
					{
						VIDEOINFOHEADER2* vih2 = (VIDEOINFOHEADER2*)mt.AllocFormatBuffer(sizeof(VIDEOINFOHEADER2));
						memset(vih2, 0, sizeof(VIDEOINFOHEADER2));
						vih2->dwPictAspectRatioX = arx;
						vih2->dwPictAspectRatioY = ary;
						bih = &vih2->bmiHeader;						
					}

					bih->biWidth = w;
					bih->biHeight = h;
					SetRect(&vih->rcSource, 0, 0, w - fudge.x, h - fudge.y);
					SetRect(&vih->rcTarget, 0, 0, w - fudge.x, h - fudge.y);

					mt.subtype = FOURCCMap(bih->biCompression = '4VLF');

					break;
				}
			}
		}
			
		if(mt.subtype != GUID_NULL)
		{
			CAtlArray<CMediaType> mts;
			mts.Add(mt);
			CAutoPtr<CBaseSplitterOutputPin> pPinOut(new CBaseSplitterOutputPin(mts, name, this, this, &hr));
			EXECUTE_ASSERT(SUCCEEDED(AddOutputPin(t.TagType, pPinOut)));
		}

		m_pFile->Seek(next);
	}

	if(m_pFile->IsRandomAccess())
	{
		__int64 pos = max(m_DataOffset, m_pFile->GetLength() - 65536);

		if(Sync(pos))
		{
			Tag t;
			AudioTag at;
			VideoTag vt;

			while(ReadTag(t))
			{
				UINT64 next = m_pFile->GetPos() + t.DataSize;

				if(t.TagType == 8 && ReadTag(at) || t.TagType == 9 && ReadTag(vt))
				{
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
	return true;
}

void CFLVSplitterFilter::DemuxSeek(REFERENCE_TIME rt)
{
	if(!m_rtDuration || rt <= 0)
	{
		m_pFile->Seek(m_DataOffset);
	}
	else
	{
		bool fAudio = !!GetOutputPin(8);
		bool fVideo = !!GetOutputPin(9);

		__int64 pos = m_DataOffset + 1.0 * rt / m_rtDuration * (m_pFile->GetLength() - m_DataOffset);

		if(!Sync(pos))
		{
			ASSERT(0);
			m_pFile->Seek(m_DataOffset);
			return;
		}

		Tag t;
		AudioTag at;
		VideoTag vt;

		while(ReadTag(t))
		{
			if(10000i64 * t.TimeStamp >= rt)
			{
				m_pFile->Seek(m_pFile->GetPos() - 15);
				break;
			}

			m_pFile->Seek(m_pFile->GetPos() + t.DataSize);
		}

		while(m_pFile->GetPos() >= m_DataOffset && (fAudio || fVideo) && ReadTag(t))
		{
			UINT64 prev = m_pFile->GetPos() - 15 - t.PreviousTagSize - 4;

			if(10000i64 * t.TimeStamp <= rt)
			{
				if(t.TagType == 8 && ReadTag(at))
				{
					fAudio = false;
				}
				else if(t.TagType == 9 && ReadTag(vt) && vt.FrameType == 1)
				{
					fVideo = false;
				}
			}

			m_pFile->Seek(prev);
		}

		if(fAudio || fVideo)
		{
			ASSERT(0);
			m_pFile->Seek(m_DataOffset);
		}
	}
}

bool CFLVSplitterFilter::DemuxLoop()
{
	HRESULT hr = S_OK;

	CAutoPtr<Packet> p;

	Tag t;
	AudioTag at;
	VideoTag vt;

	while(SUCCEEDED(hr) && !CheckRequest(NULL) && m_pFile->GetRemaining())
	{
		if(!ReadTag(t)) break;

		UINT64 next = m_pFile->GetPos() + t.DataSize;

		if((t.DataSize > 0) && (t.TagType == 8 && ReadTag(at) || t.TagType == 9 && ReadTag(vt)))
		{
			p.Attach(new Packet());
			p->TrackNumber = t.TagType;
			p->rtStart = 10000i64 * t.TimeStamp; 
			p->rtStop = p->rtStart + 1;
			p->bSyncPoint = t.TagType == 9 ? vt.FrameType == 1 : true;
			if(t.TagType == 9 && vt.CodecID == 4) m_pFile->BitRead(8);
			if(t.TagType == 9 && vt.CodecID == 5) m_pFile->BitRead(32);
			p->SetCount(next - m_pFile->GetPos());
			m_pFile->ByteRead(p->GetData(), p->GetCount());
			hr = DeliverPacket(p); 
		}

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


