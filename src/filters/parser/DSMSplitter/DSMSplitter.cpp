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
#include "DSMSplitter.h"
#include "../../../DSUtil/DSUtil.h"

#include <initguid.h>
#include <moreuuids.h>

#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] =
{
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_DirectShowMedia},
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_NULL},
};

const AMOVIESETUP_PIN sudpPins[] =
{
	{L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, countof(sudPinTypesIn), sudPinTypesIn},
	{L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, 0, NULL}
};

const AMOVIESETUP_FILTER sudFilter[] =
{
	{&__uuidof(CDSMSplitterFilter), L"MPC - DSM Splitter", MERIT_NORMAL, countof(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory},
	{&__uuidof(CDSMSourceFilter), L"MPC - DSM Source", MERIT_NORMAL, 0, NULL, CLSID_LegacyAmFilterCategory},
};

CFactoryTemplate g_Templates[] =
{
	{sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CDSMSplitterFilter>, NULL, &sudFilter[0]},
	{sudFilter[1].strName, sudFilter[1].clsID, CreateInstance<CDSMSourceFilter>, NULL, &sudFilter[1]},
};

int g_cTemplates = countof(g_Templates);

STDAPI DllRegisterServer()
{
	CString str;
	str.Format(_T("0,%d,,%%0%dI64x"), DSMSW_SIZE, DSMSW_SIZE*2);
	str.Format(CString(str), DSMSW);

	RegisterSourceFilter(
		CLSID_AsyncReader, 
		MEDIASUBTYPE_DirectShowMedia, 
		str, NULL);

	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	UnRegisterSourceFilter(MEDIASUBTYPE_DirectShowMedia);

	return AMovieDllRegisterServer2(FALSE);
}

#include "../../FilterApp.h"

CFilterApp theApp;

#endif

//
// CDSMSplitterFilter
//

CDSMSplitterFilter::CDSMSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr)
	: CBaseSplitterFilter(NAME("CDSMSplitterFilter"), pUnk, phr, __uuidof(this))
{
}

CDSMSplitterFilter::~CDSMSplitterFilter()
{
}

static int compare_id(const void* id1, const void* id2) {return (int)*(BYTE*)id1 - (int)*(BYTE*)id2;}

HRESULT CDSMSplitterFilter::CreateOutputs(IAsyncReader* pAsyncReader)
{
	CheckPointer(pAsyncReader, E_POINTER);

	HRESULT hr = E_FAIL;

	m_pFile.Free();
	m_pFile.Attach(DNew CDSMSplitterFile(pAsyncReader, hr, *this, *this));
	if(!m_pFile) return E_OUTOFMEMORY;
	if(FAILED(hr)) {m_pFile.Free(); return hr;}

	m_rtNewStart = m_rtCurrent = 0;
	m_rtNewStop = m_rtStop = m_rtDuration = m_pFile->m_rtDuration;

	CAtlArray<BYTE> ids;

	POSITION pos = m_pFile->m_mts.GetStartPosition();
	while(pos)
	{
		BYTE id;
		CMediaType mt;
		m_pFile->m_mts.GetNextAssoc(pos, id, mt);
		ids.Add(id);
	}

	qsort(ids.GetData(), ids.GetCount(), sizeof(BYTE), compare_id);

	for(int i = 0; i < ids.GetCount(); i++)
	{
		BYTE id = ids[i];
		CMediaType& mt = m_pFile->m_mts[id];

		CStringW name, lang;
		name.Format(L"Output %02d", id);

		CAtlArray<CMediaType> mts;
		mts.Add(mt);

		CAutoPtr<CBaseSplitterOutputPin> pPinOut(DNew CBaseSplitterOutputPin(mts, name, this, this, &hr));
	
		name.Empty();

		pos = m_pFile->m_sim[id].GetStartPosition();
		while(pos)
		{
			CStringA key; CStringW value;
			m_pFile->m_sim[id].GetNextAssoc(pos, key, value);
			pPinOut->SetProperty(CStringW(key), value);

			if(key == "NAME") name = value;
			if(key == "LANG") if((lang = ISO6392ToLanguage(CStringA(CString(value)))).IsEmpty()) lang = value;
		}

		if(!name.IsEmpty() || !lang.IsEmpty())
		{
			if(!name.IsEmpty()) {if(!lang.IsEmpty()) name += L" (" + lang + L")";}
			else if(!lang.IsEmpty()) name = lang;
			pPinOut->SetName(name);
		}

		EXECUTE_ASSERT(SUCCEEDED(AddOutputPin(id, pPinOut)));
	}

	pos = m_pFile->m_fim.GetStartPosition();
	while(pos)
	{
		CStringA key; CStringW value;
		m_pFile->m_fim.GetNextAssoc(pos, key, value);
		SetProperty(CStringW(key), value);
	}

	for(int i = 0; i < m_resources.GetCount(); i++)
	{
		const CDSMResource& r = m_resources[i];
		if(r.mime == "application/x-truetype-font")
			m_fontinst.InstallFont(r.data);
	}

	return m_pOutputs.GetCount() > 0 ? S_OK : E_FAIL;
}

bool CDSMSplitterFilter::DemuxInit()
{
	SetThreadName((DWORD)-1, "CDSMSplitterFilter");
	return true;
}

void CDSMSplitterFilter::DemuxSeek(REFERENCE_TIME rt)
{
	m_pFile->Seek(m_pFile->FindSyncPoint(rt));
}

bool CDSMSplitterFilter::DemuxLoop()
{
	HRESULT hr = S_OK;

	while(SUCCEEDED(hr) && !CheckRequest(NULL) && m_pFile->GetRemaining())
	{
		dsmp_t type;
		UINT64 len;

		if(!m_pFile->Sync(type, len))
			continue;

		__int64 pos = m_pFile->GetPos();

		if(type == DSMP_SAMPLE)
		{
			CAutoPtr<Packet> p(DNew Packet());
			if(m_pFile->Read(len, p))
			{
				if(p->rtStart != Packet::INVALID_TIME)
				{
					p->rtStart -= m_pFile->m_rtFirst;
					p->rtStop -= m_pFile->m_rtFirst;
				}

				hr = DeliverPacket(p);
			}
		}

		m_pFile->Seek(pos + len);
	}

	return(true);
}

// IKeyFrameInfo

STDMETHODIMP CDSMSplitterFilter::GetKeyFrameCount(UINT& nKFs)
{
	CheckPointer(m_pFile, E_UNEXPECTED);
	nKFs = m_pFile->m_sps.GetCount();
	return S_OK;
}

STDMETHODIMP CDSMSplitterFilter::GetKeyFrames(const GUID* pFormat, REFERENCE_TIME* pKFs, UINT& nKFs)
{
	CheckPointer(pFormat, E_POINTER);
	CheckPointer(pKFs, E_POINTER);
	CheckPointer(m_pFile, E_UNEXPECTED);

	if(*pFormat != TIME_FORMAT_MEDIA_TIME) return E_INVALIDARG;

	// these aren't really the keyframes, but quicky accessable points in the stream
	for(nKFs = 0; nKFs < m_pFile->m_sps.GetCount(); nKFs++)
		pKFs[nKFs] = m_pFile->m_sps[nKFs].rt;

	return S_OK;
}

//
// CDSMSourceFilter
//

CDSMSourceFilter::CDSMSourceFilter(LPUNKNOWN pUnk, HRESULT* phr)
	: CDSMSplitterFilter(pUnk, phr)
{
	m_clsid = __uuidof(this);
	m_pInput.Free();
}
