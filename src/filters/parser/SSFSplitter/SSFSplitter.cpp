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
#include <initguid.h>
#include "SSFSplitter.h"
#include <moreuuids.h>

#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] =
{
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_SSF},
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_PIN sudpPins[] =
{
    {L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesIn), sudPinTypesIn},
    {L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, 0, NULL}
};

const AMOVIESETUP_FILTER sudFilter[] =
{
	{&__uuidof(CSSFSplitterFilter), L"MPC - SSF Splitter", MERIT_NORMAL, countof(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory},
	{&__uuidof(CSSFSourceFilter), L"MPC - SSF Source", MERIT_NORMAL, 0, NULL, CLSID_LegacyAmFilterCategory},
};

CFactoryTemplate g_Templates[] =
{
	{sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CSSFSplitterFilter>, NULL, &sudFilter[0]},
	{sudFilter[1].strName, sudFilter[1].clsID, CreateInstance<CSSFSourceFilter>, NULL, &sudFilter[1]},
};

int g_cTemplates = countof(g_Templates);

STDAPI DllRegisterServer()
{
	SetRegKeyValue(_T("Media Type\\Extensions"), _T(".ssf"), _T("Source Filter"), CStringFromGUID(__uuidof(CSSFSourceFilter)));

	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	DeleteRegKey(_T("Media Type\\Extensions"), _T(".ssf"));

	return AMovieDllRegisterServer2(FALSE);
}

#include "../../FilterApp.h"

CFilterApp theApp;

#endif

namespace ssf
{
	class InputStreamBSF : public InputStream
	{
		CBaseSplitterFile* m_pFile;

	public:
		InputStreamBSF(CBaseSplitterFile* pFile) 
			: m_pFile(pFile)
		{
			ASSERT(m_pFile);
		}

		int InputStreamBSF::NextByte()
		{
			if(!m_pFile) ThrowError(_T("m_pFile is NULL"));
			if(!m_pFile->GetRemaining()) return EOS;
			return (int)m_pFile->BitRead(8);
		}
	};
}

int CSSFSplitterFilter::SegmentItemEx::Compare(const void* a, const void* b)
{
	const SegmentItemEx* si1 = (const SegmentItemEx*)a;
	const SegmentItemEx* si2 = (const SegmentItemEx*)b;
	if(si1->start < si2->start) return -1;
	if(si2->start < si1->start) return +1;
	return 0;
}

//
// CSSFSplitterFilter
//

CSSFSplitterFilter::CSSFSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr)
	: CBaseSplitterFilter(NAME("CSSFSplitterFilter"), pUnk, phr, __uuidof(this))
{
}

HRESULT CSSFSplitterFilter::CreateOutputs(IAsyncReader* pAsyncReader)
{
	CheckPointer(pAsyncReader, E_POINTER);

	HRESULT hr = E_FAIL;

	m_pFile.Free();
	m_pFile.Attach(DNew CBaseSplitterFile(pAsyncReader, hr));
	if(!m_pFile) return E_OUTOFMEMORY;
	if(FAILED(hr)) {m_pFile.Free(); return hr;}

	try {m_ssf.Parse(ssf::InputStreamBSF(m_pFile));}
	catch(ssf::Exception&) {return E_FAIL;}
	
	//

	m_rtNewStart = m_rtCurrent = 0;
	m_rtNewStop = m_rtStop = m_rtDuration = 0;

	if(ssf::Reference* pRootRef = m_ssf.GetRootRef())
	{
		ssf::WCharOutputStream s;
		ssf::StringMapW<float> offset;

		POSITION pos = pRootRef->m_nodes.GetHeadPosition();
		while(pos)
		{
			if(ssf::Definition* pDef = dynamic_cast<ssf::Definition*>(pRootRef->m_nodes.GetNext(pos)))
			{
				try
				{
					ssf::Definition::Time time;

					if(pDef->m_type == L"subtitle" && pDef->GetAsTime(time, offset) && (*pDef)[L"@"].IsValue())
					{
						SegmentItemEx si;
						si.pDef = pDef;
						si.start = time.start.value;
						si.stop = time.stop.value;
						m_subs.AddTail(si);

						// m_ssf.SetTime(pDef, si.start, si.stop);

						continue;
					}
				}
				catch(ssf::Exception&)
				{
				}

				pDef->Dump(s);					
			}
		}

		CStringA hdr = UTF16To8(s.GetString());

		CAtlArray<CMediaType> mts;

		CMediaType mt;
		mt.majortype = MEDIATYPE_Subtitle;
		mt.subtype = MEDIASUBTYPE_SSF;
		mt.formattype = FORMAT_SubtitleInfo;
		SUBTITLEINFO* si = (SUBTITLEINFO*)mt.AllocFormatBuffer(sizeof(SUBTITLEINFO) + hdr.GetLength() + 3);
		memset(si, 0, sizeof(*si));
		si->dwOffset = sizeof(*si);
		BYTE* p = (BYTE*)(si+1);
		p[0] = 0xef; p[1] = 0xbb; p[2] = 0xbf;
		memcpy(&p[3], (LPCSTR)hdr, hdr.GetLength());
		mts.Add(mt);

		CAutoPtr<CBaseSplitterOutputPin> pPinOut(DNew CBaseSplitterOutputPin(mts, L"Output", this, this, &hr));
		EXECUTE_ASSERT(SUCCEEDED(AddOutputPin(0, pPinOut)));

		CAtlArray<SegmentItemEx> subs;
		subs.SetCount(m_subs.GetCount());
		pos = m_subs.GetHeadPosition();
		for(size_t i = 0; pos; i++) subs.SetAt(i, m_subs.GetNext(pos));
		qsort(subs.GetData(), subs.GetCount(), sizeof(SegmentItemEx), SegmentItemEx::Compare);
		m_subs.RemoveAll();
		for(size_t i = 0; i < subs.GetCount(); i++) m_subs.AddTail(subs[i]);

		if(!m_ssf.m_segments.IsEmpty())
		{
			m_rtNewStop = m_rtStop = m_rtDuration = 10000000i64*m_ssf.m_segments.GetTail().m_stop;
		}
	}

	return m_pOutputs.GetCount() > 0 ? S_OK : E_FAIL;
}

bool CSSFSplitterFilter::DemuxInit()
{
	return true;
}

void CSSFSplitterFilter::DemuxSeek(REFERENCE_TIME rt)
{
	if(rt <= 0)
	{
	}
	else
	{
		// TODO
	}
}

bool CSSFSplitterFilter::DemuxLoop()
{
	HRESULT hr = S_OK;

	POSITION pos = m_subs.GetHeadPosition();

	while(pos && SUCCEEDED(hr) && !CheckRequest(NULL))
	{
		SegmentItemEx& si = m_subs.GetNext(pos);

		ssf::WCharOutputStream s;
		si.pDef->Dump(s);
		CStringA& str = UTF16To8(s.GetString());

		CAutoPtr<Packet> p(DNew Packet());

		p->TrackNumber = 0;
		p->bSyncPoint = TRUE; // TODO
		p->rtStart = 10000000i64*si.start;
		p->rtStop = 10000000i64*si.stop;
		p->SetData((LPCSTR)str, str.GetLength());

		hr = DeliverPacket(p);
	}

	return true;
}

//
// CSSFSourceFilter
//

CSSFSourceFilter::CSSFSourceFilter(LPUNKNOWN pUnk, HRESULT* phr)
	: CSSFSplitterFilter(pUnk, phr)
{
	m_clsid = __uuidof(this);
	m_pInput.Free();
}
