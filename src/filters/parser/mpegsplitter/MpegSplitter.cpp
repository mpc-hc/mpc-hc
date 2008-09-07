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
#include <mmreg.h>
#include <initguid.h>
#include <dmodshow.h>
#include "MpegSplitter.h"
#include <moreuuids.h>


#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] =
{
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_MPEG1System},
//	{&MEDIATYPE_Stream, &MEDIASUBTYPE_MPEG1VideoCD}, // cdxa filter should take care of this
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_MPEG2_PROGRAM},
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_MPEG2_TRANSPORT},
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_MPEG2_PVA},
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_PIN sudpPins[] =
{
    {L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesIn), sudPinTypesIn},
    {L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, 0, NULL},
};

const AMOVIESETUP_FILTER sudFilter[] =
{
	{&__uuidof(CMpegSplitterFilter), L"MPC - Mpeg Splitter (Gabest)", MERIT_NORMAL+1 /*MERIT_NORMAL+1*/, countof(sudpPins), sudpPins},
	{&__uuidof(CMpegSourceFilter), L"MPC - Mpeg Source (Gabest)", MERIT_DO_NOT_USE, 0, NULL},
};

CFactoryTemplate g_Templates[] =
{
	{sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CMpegSplitterFilter>, NULL, &sudFilter[0]},
	{sudFilter[1].strName, sudFilter[1].clsID, CreateInstance<CMpegSourceFilter>, NULL, &sudFilter[1]},
};

int g_cTemplates = countof(g_Templates);

STDAPI DllRegisterServer()
{
	DeleteRegKey(_T("Media Type\\Extensions\\"), _T(".ts"));

	RegisterSourceFilter(CLSID_AsyncReader, MEDIASUBTYPE_MPEG1System, _T("0,16,FFFFFFFFF100010001800001FFFFFFFF,000001BA2100010001800001000001BB"), NULL);
	RegisterSourceFilter(CLSID_AsyncReader, MEDIASUBTYPE_MPEG2_PROGRAM, _T("0,5,FFFFFFFFC0,000001BA40"), NULL);
	RegisterSourceFilter(CLSID_AsyncReader, MEDIASUBTYPE_MPEG2_PVA, _T("0,8,fffffc00ffe00000,4156000055000000"), NULL);

	CAtlList<CString> chkbytes;
	chkbytes.AddTail(_T("0,1,,47,188,1,,47,376,1,,47"));
	chkbytes.AddTail(_T("4,1,,47,196,1,,47,388,1,,47"));
	chkbytes.AddTail(_T("0,4,,54467263,1660,1,,47")); // TFrc
	RegisterSourceFilter(CLSID_AsyncReader, MEDIASUBTYPE_MPEG2_TRANSPORT, chkbytes, NULL);	

	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
//	UnRegisterSourceFilter(MEDIASUBTYPE_MPEG1System);
//	UnRegisterSourceFilter(MEDIASUBTYPE_MPEG2_PROGRAM);

	return AMovieDllRegisterServer2(FALSE);
}

#include "..\..\FilterApp.h"

CFilterApp theApp;

#endif

//
// CMpegSplitterFilter
//

CMpegSplitterFilter::CMpegSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr, const CLSID& clsid)
	: CBaseSplitterFilter(NAME("CMpegSplitterFilter"), pUnk, phr, clsid)
	, m_pPipoBimbo(false)
{
}

STDMETHODIMP CMpegSplitterFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);

    return 
		QI(IAMStreamSelect)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP CMpegSplitterFilter::GetClassID(CLSID* pClsID)
{
	CheckPointer (pClsID, E_POINTER);

	if (m_pPipoBimbo)
	{
		memcpy (pClsID, &CLSID_WMAsfReader, sizeof (GUID));
		return S_OK;
	}
	else
		return __super::GetClassID(pClsID);
}

STDMETHODIMP CMpegSplitterFilter::Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt)
{
	if (wcslen (pszFileName) > 0)
	{
		WCHAR		Drive[5];
		WCHAR		Dir[MAX_PATH];
		WCHAR		Filename[MAX_PATH];
		WCHAR		Ext[10];
		
		if (_wsplitpath_s (pszFileName, Drive, countof(Drive), Dir, countof(Dir), Filename, countof(Filename), Ext, countof(Ext)) == 0)
		{
			CString		strClipInfo;
			strClipInfo.Format (_T("%s\\%s\\..\\CLIPINF\\%s.clpi"), Drive, Dir, Filename);
			m_ClipInfo.ReadInfo (strClipInfo);
		}
	}
	
	return __super::Load (pszFileName, pmt);
}

//

HRESULT CMpegSplitterFilter::DemuxNextPacket(REFERENCE_TIME rtStartOffset)
{
	HRESULT hr;
	BYTE b;

	if(m_pFile->m_type == CMpegSplitterFile::ps || m_pFile->m_type == CMpegSplitterFile::es)
	{
		if(!m_pFile->NextMpegStartCode(b))
			return S_FALSE;

		if(b == 0xba) // program stream header
		{
			CMpegSplitterFile::pshdr h;
			if(!m_pFile->Read(h)) return S_FALSE;
		}
		else if(b == 0xbb) // program stream system header
		{
			CMpegSplitterFile::pssyshdr h;
			if(!m_pFile->Read(h)) return S_FALSE;
		}
		else if(b >= 0xbd && b < 0xf0) // pes packet
//		else if((b >= 0xbd && b < 0xf0) || (b == 0xfd)) // pes packet	TODO EVO SUPPORT
		{
			CMpegSplitterFile::peshdr h;
			if(!m_pFile->Read(h, b) || !h.len) return S_FALSE;

			if(h.type == CMpegSplitterFile::mpeg2 && h.scrambling) {ASSERT(0); return E_FAIL;}

			__int64 pos = m_pFile->GetPos();

			DWORD TrackNumber = m_pFile->AddStream(0, b, h.len);

			if(GetOutputPin(TrackNumber))
			{
				CAutoPtr<Packet> p(new Packet());
				p->TrackNumber = TrackNumber;
				p->bSyncPoint = !!h.fpts;
				p->bAppendable = !h.fpts;
				p->rtStart = h.fpts ? (h.pts - rtStartOffset) : Packet::INVALID_TIME;
				p->rtStop = p->rtStart+1;
				p->SetCount(h.len - (size_t)(m_pFile->GetPos() - pos));
				m_pFile->ByteRead(p->GetData(), h.len - (m_pFile->GetPos() - pos));
				hr = DeliverPacket(p);
			}

			m_pFile->Seek(pos + h.len);
		}
	}
	else if(m_pFile->m_type == CMpegSplitterFile::ts)
	{
		CMpegSplitterFile::trhdr h;
		if(!m_pFile->Read(h)) 
			return S_FALSE;

		//if(h.scrambling) {ASSERT(0); m_pFile->Seek(h.next); return S_FALSE;}

		__int64 pos = m_pFile->GetPos();

		if(h.payload && h.payloadstart)
		{
			m_pFile->UpdatePrograms(h);
		}

		if(h.payload && h.pid >= 16 && h.pid < 0x1fff && !h.scrambling)
		{
			DWORD TrackNumber = h.pid;

			CMpegSplitterFile::peshdr h2;
			if(h.payloadstart && m_pFile->NextMpegStartCode(b, 4) && m_pFile->Read(h2, b)) // pes packet
			{
				if(h2.type == CMpegSplitterFile::mpeg2 && h2.scrambling) {ASSERT(0); return E_FAIL;}
				TrackNumber = m_pFile->AddStream(h.pid, b, h.bytes - (DWORD)(m_pFile->GetPos() - pos));
			}

			if(GetOutputPin(TrackNumber))
			{
				CAutoPtr<Packet> p(new Packet());
				p->TrackNumber = TrackNumber;
				p->bSyncPoint = !!h2.fpts;
				p->bAppendable = !h2.fpts;
				p->rtStart = h2.fpts ? (h2.pts - rtStartOffset) : Packet::INVALID_TIME;
				p->rtStop = p->rtStart+1;
				p->SetCount(h.bytes - (size_t)(m_pFile->GetPos() - pos));
				m_pFile->ByteRead(p->GetData(), h.bytes - (m_pFile->GetPos() - pos));

				hr = DeliverPacket(p);
			}
		}

		m_pFile->Seek(h.next);
	}
	else if(m_pFile->m_type == CMpegSplitterFile::pva)
	{
		CMpegSplitterFile::pvahdr h;
		if(!m_pFile->Read(h))
			return S_FALSE;

		DWORD TrackNumber = h.streamid;

		__int64 pos = m_pFile->GetPos();

		if(GetOutputPin(TrackNumber))
		{
			CAutoPtr<Packet> p(new Packet());
			p->TrackNumber = TrackNumber;
			p->bSyncPoint = !!h.fpts;
			p->bAppendable = !h.fpts;
			p->rtStart = h.fpts ? (h.pts - rtStartOffset) : Packet::INVALID_TIME;
			p->rtStop = p->rtStart+1;
			p->SetCount(h.length);
			m_pFile->ByteRead(p->GetData(), h.length);
			hr = DeliverPacket(p);
		}

		m_pFile->Seek(pos + h.length);
	}

	return S_OK;
}

//

HRESULT CMpegSplitterFilter::CreateOutputs(IAsyncReader* pAsyncReader)
{
	CheckPointer(pAsyncReader, E_POINTER);

	HRESULT hr = E_FAIL;

	m_pFile.Free();

	m_pFile.Attach(new CMpegSplitterFile(pAsyncReader, hr, m_ClipInfo.IsHdmv()));
	if(!m_pFile) return E_OUTOFMEMORY;
	if(FAILED(hr)) {m_pFile.Free(); return hr;}

	// Create
	if (m_ClipInfo.IsHdmv())
	{
		for (int i=0; i<m_ClipInfo.GetStreamNumber(); i++)
		{
			CHdmvClipInfo::Stream*		stream = m_ClipInfo.GetStreamByIndex (i);
			if (stream->stream_coding_type == 0x90)
			{
				m_pFile->AddHdmvPGStream (stream->stream_PID, stream->language_code);
			}
		}
	}

	m_rtNewStart = m_rtCurrent = 0;
	m_rtNewStop = m_rtStop = m_rtDuration = 0;

	for(int i = 0; i < countof(m_pFile->m_streams); i++)
	{
		POSITION pos = m_pFile->m_streams[i].GetHeadPosition();
		while(pos)
		{
			CMpegSplitterFile::stream& s = m_pFile->m_streams[i].GetNext(pos);

			CAtlArray<CMediaType> mts;
			mts.Add(s.mt);

			CStringW name = CMpegSplitterFile::CStreamList::ToString(i);

			HRESULT hr;
			CAutoPtr<CBaseSplitterOutputPin> pPinOut(new CMpegSplitterOutputPin(mts, name, this, this, &hr));
			if (i == CMpegSplitterFile::subpic)
				((CMpegSplitterOutputPin*)pPinOut.m_p)->SetMaxShift (_I64_MAX);
			if(S_OK == AddOutputPin(s, pPinOut))
				break;
		}
	}

	if(m_pFile->IsRandomAccess() && m_pFile->m_rate)
	{
		m_rtNewStop = m_rtStop = m_rtDuration = 10000000i64 * m_pFile->GetLength() / m_pFile->m_rate;
	}

	return m_pOutputs.GetCount() > 0 ? S_OK : E_FAIL;
}

bool CMpegSplitterFilter::DemuxInit()
{
	if(!m_pFile) return(false);

	m_rtStartOffset = 0;

	return(true);
}

void CMpegSplitterFilter::DemuxSeek(REFERENCE_TIME rt)
{
	CAtlList<CMpegSplitterFile::stream>* pMasterStream = m_pFile->GetMasterStream();

	if(!pMasterStream) {ASSERT(0); return;}

	if(m_pFile->IsStreaming())
	{
		m_pFile->Seek(max(0, m_pFile->GetLength() - 100*1024));
		m_rtStartOffset = m_pFile->m_rtMin + m_pFile->NextPTS(pMasterStream->GetHead());
		return;
	}

	REFERENCE_TIME rtPreroll = 10000000;
	
	if(rt <= rtPreroll || m_rtDuration <= 0)
	{
		m_pFile->Seek(0);
	}
	else
	{
		__int64 len = m_pFile->GetLength();
		__int64 seekpos = (__int64)(1.0*rt/m_rtDuration*len);
		__int64 minseekpos = _I64_MAX;

		REFERENCE_TIME rtmax = rt - rtPreroll;
		REFERENCE_TIME rtmin = rtmax - 5000000;

		if(m_rtStartOffset == 0)
		for(int i = 0; i < countof(m_pFile->m_streams)-1; i++)
		{
			POSITION pos = m_pFile->m_streams[i].GetHeadPosition();
			while(pos)
			{
				DWORD TrackNum = m_pFile->m_streams[i].GetNext(pos);

				CBaseSplitterOutputPin* pPin = GetOutputPin(TrackNum);
				if(pPin && pPin->IsConnected())
				{
					m_pFile->Seek(seekpos);

					REFERENCE_TIME pdt = _I64_MIN;

					for(int j = 0; j < 10; j++)
					{
						REFERENCE_TIME rt = m_pFile->NextPTS(TrackNum);
						// TRACE(_T("[%d/%04x]: rt=%I64d, fp=%I64d\n"), i, TrackNum, rt, m_pFile->GetPos());

						if(rt < 0) break;

						REFERENCE_TIME dt = rt - rtmax;
						if(dt > 0 && dt == pdt) dt = 10000000i64;

						// TRACE(_T("dt=%I64d\n"), dt);

						if(rtmin <= rt && rt <= rtmax || pdt > 0 && dt < 0)
						{
							// TRACE(_T("minseekpos: %I64d -> "), minseekpos);
							minseekpos = min(minseekpos, m_pFile->GetPos());
							// TRACE(_T("%I64d\n"), minseekpos);
							break;
						}

						m_pFile->Seek(m_pFile->GetPos() - (__int64)(1.0*dt/m_rtDuration*len));
		
						pdt = dt;
					}
				}
			}
		}

		if(minseekpos != _I64_MAX)
		{
			seekpos = minseekpos;
		}
		else
		{
			// this file is probably screwed up, try plan B, seek simply by bitrate

			rt -= rtPreroll;
			seekpos = (__int64)(1.0*rt/m_rtDuration*len);
			m_pFile->Seek(seekpos);
			m_rtStartOffset = m_pFile->m_rtMin + m_pFile->NextPTS(pMasterStream->GetHead()) - rt;
		}

		m_pFile->Seek(seekpos);
	}
}

bool CMpegSplitterFilter::DemuxLoop()
{
	REFERENCE_TIME rtStartOffset = m_rtStartOffset ? m_rtStartOffset : m_pFile->m_rtMin;

	HRESULT hr = S_OK;
	while(SUCCEEDED(hr) && !CheckRequest(NULL))
	{
		if((hr = m_pFile->HasMoreData(1024*500)) == S_OK)
			if((hr = DemuxNextPacket(rtStartOffset)) == S_FALSE)
				Sleep(1);
	}

	return(true);
}

// IAMStreamSelect

STDMETHODIMP CMpegSplitterFilter::Count(DWORD* pcStreams)
{
	CheckPointer(pcStreams, E_POINTER);

	*pcStreams = 0;

	for(int i = 0; i < countof(m_pFile->m_streams); i++)
		(*pcStreams) += m_pFile->m_streams[i].GetCount();

	return S_OK;
}

STDMETHODIMP CMpegSplitterFilter::Enable(long lIndex, DWORD dwFlags)
{
	if(!(dwFlags & AMSTREAMSELECTENABLE_ENABLE))
		return E_NOTIMPL;

	for(int i = 0, j = 0; i < countof(m_pFile->m_streams); i++)
	{
		int cnt = m_pFile->m_streams[i].GetCount();
		
		if(lIndex >= j && lIndex < j+cnt)
		{
			lIndex -= j;

			POSITION pos = m_pFile->m_streams[i].FindIndex(lIndex);
			if(!pos) return E_UNEXPECTED;

			CMpegSplitterFile::stream& to = m_pFile->m_streams[i].GetAt(pos);

			pos = m_pFile->m_streams[i].GetHeadPosition();
			while(pos)
			{
				CMpegSplitterFile::stream& from = m_pFile->m_streams[i].GetNext(pos);
				if(!GetOutputPin(from)) continue;

				HRESULT hr;
				if(FAILED(hr = RenameOutputPin(from, to, &to.mt)))
					return hr;

				if(const CMpegSplitterFile::program* p = m_pFile->FindProgram(to.pid))
				{
					for(int k = 0; k < countof(m_pFile->m_streams); k++)
					{
						if(k == i) continue;

						pos = m_pFile->m_streams[k].GetHeadPosition();
						while(pos)
						{
							CMpegSplitterFile::stream& from = m_pFile->m_streams[k].GetNext(pos);
							if(!GetOutputPin(from)) continue;

							for(int l = 0; l < countof(p->pid); l++)
							{
								if(const CMpegSplitterFile::stream* s = m_pFile->m_streams[k].FindStream(p->pid[l]))
								{
									if(from != *s) hr = RenameOutputPin(from, *s, &s->mt);
									break;
								}
							}
						}
					}
				}

				return S_OK;
			}
		}

		j += cnt;
	}

	return S_FALSE;
}

STDMETHODIMP CMpegSplitterFilter::Info(long lIndex, AM_MEDIA_TYPE** ppmt, DWORD* pdwFlags, LCID* plcid, DWORD* pdwGroup, WCHAR** ppszName, IUnknown** ppObject, IUnknown** ppUnk)
{
	for(int i = 0, j = 0; i < countof(m_pFile->m_streams); i++)
	{
		int cnt = m_pFile->m_streams[i].GetCount();
		
		if(lIndex >= j && lIndex < j+cnt)
		{
			lIndex -= j;
			
			POSITION pos = m_pFile->m_streams[i].FindIndex(lIndex);
			if(!pos) return E_UNEXPECTED;

			CMpegSplitterFile::stream&	s = m_pFile->m_streams[i].GetAt(pos);
			CHdmvClipInfo::Stream*		pStream = m_ClipInfo.FindStream (s.pid);

			if(ppmt) *ppmt = CreateMediaType(&s.mt);
			if(pdwFlags) *pdwFlags = GetOutputPin(s) ? (AMSTREAMSELECTINFO_ENABLED|AMSTREAMSELECTINFO_EXCLUSIVE) : 0;
			if(plcid) *plcid = pStream ? pStream->lcid : 0;
			if(pdwGroup) *pdwGroup = i;
			if(ppObject) *ppObject = NULL;
			if(ppUnk) *ppUnk = NULL;

			
			if(ppszName)
			{
				CStringW name = CMpegSplitterFile::CStreamList::ToString(i);

				CStringW str;

				if (pStream)
				{
					CString lang;
					int len = GetLocaleInfo(pStream->lcid, LOCALE_SENGLANGUAGE, lang.GetBuffer(64), 64);
					lang.ReleaseBufferSetLength(max(len-1, 0));
					str.Format (L"%s (%s - %s)", name, lang, pStream->Format());
				}
				else if (i == CMpegSplitterFile::subpic && s.pid == NO_SUBTITLE_PID)
				{
					str		= _T("No subtitles");
					*plcid	= LCID_NOSUBTITLES;
				}
				else
					str.Format(L"%s (%04x,%02x,%02x)", name, s.pid, s.pesid, s.ps1id); // TODO: make this nicer

				*ppszName = (WCHAR*)CoTaskMemAlloc((str.GetLength()+1)*sizeof(WCHAR));
				if(*ppszName == NULL) return E_OUTOFMEMORY;
				wcscpy_s(*ppszName, str.GetLength()+1, str);
			}
		}

		j += cnt;
	}

	return S_OK;
}

//
// CMpegSourceFilter
//

CMpegSourceFilter::CMpegSourceFilter(LPUNKNOWN pUnk, HRESULT* phr, const CLSID& clsid)
	: CMpegSplitterFilter(pUnk, phr, clsid)
{
	m_pInput.Free();
}

//
// CMpegSplitterOutputPin
//

CMpegSplitterOutputPin::CMpegSplitterOutputPin(CAtlArray<CMediaType>& mts, LPCWSTR pName, CBaseFilter* pFilter, CCritSec* pLock, HRESULT* phr)
	: CBaseSplitterOutputPin(mts, pName, pFilter, pLock, phr)
	, m_fHasAccessUnitDelimiters(false)
	, m_rtMaxShift(50000000)
{
}

CMpegSplitterOutputPin::~CMpegSplitterOutputPin()
{
}

HRESULT CMpegSplitterOutputPin::DeliverNewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
	{
		CAutoLock cAutoLock(this);
		m_rtPrev = Packet::INVALID_TIME;
		m_rtOffset = 0;
	}

	return __super::DeliverNewSegment(tStart, tStop, dRate);
}

HRESULT CMpegSplitterOutputPin::DeliverEndFlush()
{
	{
		CAutoLock cAutoLock(this);
		m_p.Free();
		m_pl.RemoveAll();
	}

	return __super::DeliverEndFlush();
}

HRESULT CMpegSplitterOutputPin::DeliverPacket(CAutoPtr<Packet> p)
{
	CAutoLock cAutoLock(this);

	if(p->rtStart != Packet::INVALID_TIME)
	{
		REFERENCE_TIME rt = p->rtStart + m_rtOffset;

		// Filter invalid PTS (if too different from previous packet)
		if(m_rtPrev != Packet::INVALID_TIME)
		if(_abs64(rt - m_rtPrev) > m_rtMaxShift)
			m_rtOffset += m_rtPrev - rt;

		p->rtStart += m_rtOffset;
		p->rtStop += m_rtOffset;
//TRACE(_T("%I64d, %I64d (%I64d)\n"), p->rtStart, m_rtPrev, m_rtOffset);
		m_rtPrev = p->rtStart;
	}

	if (p->pmt && p->pmt->subtype != m_mt.subtype)
		SetMediaType ((CMediaType*)p->pmt);


	if(m_mt.subtype == MEDIASUBTYPE_AAC) // special code for aac, the currently available decoders only like whole frame samples
	{
		if(m_p && m_p->GetCount() == 1 && m_p->GetAt(0) == 0xff
		&& !(!p->IsEmpty() && (p->GetAt(0) & 0xf6) == 0xf0))
			m_p.Free();

		if(!m_p)
		{
			BYTE* base = p->GetData();
			BYTE* s = base;
			BYTE* e = s + p->GetCount();

			for(; s < e; s++)
			{
				if(*s != 0xff) continue;

				if(s == e-1 || (s[1]&0xf6) == 0xf0)
				{
					memmove(base, s, e - s);
					p->SetCount(e - s);
					m_p = p;
					break;
				}
			}
		}
		else
		{
			m_p->Append(*p);
		}

		while(m_p && m_p->GetCount() > 9)
		{
			BYTE* base = m_p->GetData();
			BYTE* s = base;
			BYTE* e = s + m_p->GetCount();
			//bool layer0 = ((s[3]>>1)&3) == 0;
			int len = ((s[3]&3)<<11)|(s[4]<<3)|(s[5]>>5);
			bool crc = !(s[1]&1);
			s += 7; len -= 7;
			if(crc) s += 2, len -= 2;

			if(e - s < len)
			{
				break;
			}

			if(len <= 0 || e - s >= len + 2 && (s[len] != 0xff || (s[len+1]&0xf6) != 0xf0))
			{
				m_p.Free();
				break;
			}

			CAutoPtr<Packet> p(new Packet());
			p->TrackNumber = m_p->TrackNumber;
			p->bDiscontinuity |= m_p->bDiscontinuity; m_p->bDiscontinuity = false;
			p->bSyncPoint = m_p->rtStart != Packet::INVALID_TIME;
			p->rtStart = m_p->rtStart; m_p->rtStart = Packet::INVALID_TIME;
			p->rtStop = m_p->rtStop; m_p->rtStop = Packet::INVALID_TIME;
			p->pmt = m_p->pmt; m_p->pmt = NULL;
			p->SetData(s, len);
			s += len;
			memmove(base, s, e - s);
			m_p->SetCount(e - s);

			HRESULT hr = __super::DeliverPacket(p);
			if(hr != S_OK) return hr;
		}

		if(m_p && p)
		{
			if(!m_p->bDiscontinuity) m_p->bDiscontinuity = p->bDiscontinuity;
			if(!m_p->bSyncPoint) m_p->bSyncPoint = p->bSyncPoint;
			if(m_p->rtStart == Packet::INVALID_TIME) m_p->rtStart = p->rtStart, m_p->rtStop = p->rtStop;
			if(m_p->pmt) DeleteMediaType(m_p->pmt); m_p->pmt = p->pmt; p->pmt = NULL;
		}

		return S_OK;
	}
	else if(m_mt.subtype == FOURCCMap('1CVA') || m_mt.subtype == FOURCCMap('1cva')) // just like aac, this has to be starting nalus, more can be packed together
	{
		if(!m_p)
		{
			m_p.Attach(new Packet());
			m_p->TrackNumber = p->TrackNumber;
			m_p->bDiscontinuity = p->bDiscontinuity; p->bDiscontinuity = FALSE;
			m_p->bSyncPoint = p->bSyncPoint; p->bSyncPoint = FALSE;
			m_p->rtStart = p->rtStart; p->rtStart = Packet::INVALID_TIME;
			m_p->rtStop = p->rtStop; p->rtStop = Packet::INVALID_TIME;
		}

		m_p->Append(*p);

		BYTE* start = m_p->GetData();
		BYTE* end = start + m_p->GetCount();

		while(start <= end-4 && *(DWORD*)start != 0x01000000) start++;

		while(start <= end-4)
		{
			BYTE* next = start+1;

			while(next <= end-4 && *(DWORD*)next != 0x01000000) next++;

			if(next >= end-4) break;

			int size = next - start;

			CH264Nalu			Nalu;
			Nalu.SetBuffer (start, size, 0);

			CAutoPtr<Packet> p2;

			while (Nalu.ReadNext())
			{
				DWORD	dwNalLength = 
					((Nalu.GetDataLength() >> 24) & 0x000000ff) |
					((Nalu.GetDataLength() >>  8) & 0x0000ff00) |
					((Nalu.GetDataLength() <<  8) & 0x00ff0000) |
					((Nalu.GetDataLength() << 24) & 0xff000000);

				CAutoPtr<Packet> p3(new Packet());
				
				//p2->SetData(start, next - start);
				p3->SetCount (Nalu.GetDataLength()+sizeof(dwNalLength));
				memcpy (p3->GetData(), &dwNalLength, sizeof(dwNalLength));
				memcpy (p3->GetData()+sizeof(dwNalLength), Nalu.GetDataBuffer(), Nalu.GetDataLength());
				
				if (p2 == NULL)
					p2 = p3;
				else
					p2->Append(*p3);
			}

			p2->TrackNumber = m_p->TrackNumber;
			p2->bDiscontinuity = m_p->bDiscontinuity; m_p->bDiscontinuity = FALSE;
			p2->bSyncPoint = m_p->bSyncPoint; m_p->bSyncPoint = FALSE;
			p2->rtStart = m_p->rtStart; m_p->rtStart = Packet::INVALID_TIME;
			p2->rtStop = m_p->rtStop; m_p->rtStop = Packet::INVALID_TIME;
			p2->pmt = m_p->pmt; m_p->pmt = NULL;

			m_pl.AddTail(p2);

			if(p->rtStart != Packet::INVALID_TIME) {m_p->rtStart = p->rtStart; m_p->rtStop = p->rtStop; p->rtStart = Packet::INVALID_TIME;}
			if(p->bDiscontinuity) {m_p->bDiscontinuity = p->bDiscontinuity; p->bDiscontinuity = FALSE;}
			if(p->bSyncPoint) {m_p->bSyncPoint = p->bSyncPoint; p->bSyncPoint = FALSE;}
			if(m_p->pmt) DeleteMediaType(m_p->pmt); m_p->pmt = p->pmt; p->pmt = NULL;

			start = next;
		}
		if(start > m_p->GetData())
		{
			m_p->RemoveAt(0, start - m_p->GetData());
		}

		for(POSITION pos = m_pl.GetHeadPosition(); pos; m_pl.GetNext(pos))
		{
			if(pos == m_pl.GetHeadPosition()) 
				continue;

			Packet* pPacket = m_pl.GetAt(pos);
			BYTE* pData = pPacket->GetData();

			if((pData[4]&0x1f) == 0x09) m_fHasAccessUnitDelimiters = true;

			if((pData[4]&0x1f) == 0x09 || !m_fHasAccessUnitDelimiters && pPacket->rtStart != Packet::INVALID_TIME)
			{
				p = m_pl.RemoveHead();

				while(pos != m_pl.GetHeadPosition())
				{
					CAutoPtr<Packet> p2 = m_pl.RemoveHead();
					p->Append(*p2);
				}

				HRESULT hr = __super::DeliverPacket(p);
				if(hr != S_OK) return hr;
			}
		}

		return S_OK;
	}
	else if(m_mt.subtype == FOURCCMap('1CVW') || m_mt.subtype == FOURCCMap('1cvw')) // just like aac, this has to be starting nalus, more can be packed together
	{
		if(!m_p)
		{
			m_p.Attach(new Packet());
			m_p->TrackNumber = p->TrackNumber;
			m_p->bDiscontinuity = p->bDiscontinuity; p->bDiscontinuity = FALSE;
			m_p->bSyncPoint = p->bSyncPoint; p->bSyncPoint = FALSE;
			m_p->rtStart = p->rtStart; p->rtStart = Packet::INVALID_TIME;
			m_p->rtStop = p->rtStop; p->rtStop = Packet::INVALID_TIME;
		}

		m_p->Append(*p);

		BYTE* start = m_p->GetData();
		BYTE* end = start + m_p->GetCount();

		while(start <= end-4 && *(DWORD*)start != 0x0D010000) start++;

		while(start <= end-4)
		{
			BYTE* next = start+1;

			while(next <= end-4 && *(DWORD*)next != 0x0D010000) next++;

			if(next >= end-4) break;

			int size = next - start - 4;


			CAutoPtr<Packet> p2(new Packet());
			p2->TrackNumber = m_p->TrackNumber;
			p2->bDiscontinuity = m_p->bDiscontinuity; m_p->bDiscontinuity = FALSE;
			p2->bSyncPoint = m_p->bSyncPoint; m_p->bSyncPoint = FALSE;
			p2->rtStart = m_p->rtStart; m_p->rtStart = Packet::INVALID_TIME;
			p2->rtStop = m_p->rtStop; m_p->rtStop = Packet::INVALID_TIME;
			p2->pmt = m_p->pmt; m_p->pmt = NULL;
			p2->SetData(start, next - start);

			HRESULT hr = __super::DeliverPacket(p2);
			if(hr != S_OK) return hr;

			if(p->rtStart != Packet::INVALID_TIME) {m_p->rtStart = p->rtStart; m_p->rtStop = p->rtStop; p->rtStart = Packet::INVALID_TIME;}
			if(p->bDiscontinuity) {m_p->bDiscontinuity = p->bDiscontinuity; p->bDiscontinuity = FALSE;}
			if(p->bSyncPoint) {m_p->bSyncPoint = p->bSyncPoint; p->bSyncPoint = FALSE;}
			if(m_p->pmt) DeleteMediaType(m_p->pmt); m_p->pmt = p->pmt; p->pmt = NULL;

			start = next;
		}

		if(start > m_p->GetData())
		{
			m_p->RemoveAt(0, start - m_p->GetData());
		}

		return S_OK;
	}
	else if (m_mt.subtype == MEDIASUBTYPE_HDMV_LPCM_AUDIO)
	{
		BYTE* start = p->GetData();
		p->SetData(start + 4, p->GetCount() - 4);
	}
	else
	{
		m_p.Free();
		m_pl.RemoveAll();
	}

	return __super::DeliverPacket(p);
}


STDMETHODIMP CMpegSplitterOutputPin::Connect(IPin* pReceivePin, const AM_MEDIA_TYPE* pmt)
{
	HRESULT		hr;
	PIN_INFO	PinInfo;
	GUID		FilterClsid;

	if (SUCCEEDED (pReceivePin->QueryPinInfo (&PinInfo)))
	{
		if (SUCCEEDED (PinInfo.pFilter->GetClassID(&FilterClsid)) && (FilterClsid == CLSID_DMOWrapperFilter))
			((CMpegSplitterFilter*)m_pFilter)->SetPipo(true);
		PinInfo.pFilter->Release();
	}

	hr = __super::Connect (pReceivePin, pmt);
	((CMpegSplitterFilter*)m_pFilter)->SetPipo(false);
	return hr;
}