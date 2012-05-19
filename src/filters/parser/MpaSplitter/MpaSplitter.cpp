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
#include <MMReg.h>
#ifdef REGISTER_FILTER
#include <InitGuid.h>
#endif
#include "MpaSplitter.h"
#include <moreuuids.h>

#ifdef REGISTER_FILTER

const AMOVIESETUP_MEDIATYPE sudPinTypesIn[] = {
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_MPEG1Audio},
	{&MEDIATYPE_Stream, &MEDIASUBTYPE_NULL}
};

const AMOVIESETUP_PIN sudpPins[] = {
	{L"Input", FALSE, FALSE, FALSE, FALSE, &CLSID_NULL, NULL, _countof(sudPinTypesIn), sudPinTypesIn},
	{L"Output", FALSE, TRUE, FALSE, FALSE, &CLSID_NULL, NULL, 0, NULL}
};

const AMOVIESETUP_FILTER sudFilter[] = {
	{&__uuidof(CMpaSplitterFilter), MpaSplitterName, MERIT_NORMAL+1, _countof(sudpPins), sudpPins, CLSID_LegacyAmFilterCategory},
	{&__uuidof(CMpaSourceFilter), MpaSourceName, MERIT_NORMAL+1, 0, NULL, CLSID_LegacyAmFilterCategory},
};

CFactoryTemplate g_Templates[] = {
	{sudFilter[0].strName, sudFilter[0].clsID, CreateInstance<CMpaSplitterFilter>, NULL, &sudFilter[0]},
	{sudFilter[1].strName, sudFilter[1].clsID, CreateInstance<CMpaSourceFilter>, NULL, &sudFilter[1]},
};

int g_cTemplates = _countof(g_Templates);

STDAPI DllRegisterServer()
{
	CAtlList<CString> chkbytes;
	chkbytes.AddTail(_T("0,2,FFE0,FFE0"));
	chkbytes.AddTail(_T("0,10,FFFFFF00000080808080,49443300000000000000"));
	RegisterSourceFilter(CLSID_AsyncReader, MEDIASUBTYPE_MPEG1Audio, chkbytes, NULL);

	return AMovieDllRegisterServer2(TRUE);
}

STDAPI DllUnregisterServer()
{
	return AMovieDllRegisterServer2(FALSE);
}

#include "../../FilterApp.h"

CFilterApp theApp;

#endif

//
// CMpaSplitterFilter
//

CMpaSplitterFilter::CMpaSplitterFilter(LPUNKNOWN pUnk, HRESULT* phr)
	: CBaseSplitterFilter(NAME("CMpaSplitterFilter"), pUnk, phr, __uuidof(this))
{
}

STDMETHODIMP CMpaSplitterFilter::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
	CheckPointer(ppv, E_POINTER);

	return
		__super::NonDelegatingQueryInterface(riid, ppv);
}

STDMETHODIMP CMpaSplitterFilter::QueryFilterInfo(FILTER_INFO* pInfo)
{
	CheckPointer(pInfo, E_POINTER);
	ValidateReadWritePtr(pInfo, sizeof(FILTER_INFO));

	if (m_pName && m_pName[0]==L'M' && m_pName[1]==L'P' && m_pName[2]==L'C') {
		(void)StringCchCopyW(pInfo->achName, NUMELMS(pInfo->achName), m_pName);
	} else {
		wcscpy_s(pInfo->achName, MpaSourceName);
	}
	pInfo->pGraph = m_pGraph;
	if (m_pGraph) {
		m_pGraph->AddRef();
	}

	return S_OK;
}

HRESULT CMpaSplitterFilter::CreateOutputs(IAsyncReader* pAsyncReader)
{
	CheckPointer(pAsyncReader, E_POINTER);

	HRESULT hr = E_FAIL;

	m_pFile.Free();

	m_pFile.Attach(DNew CMpaSplitterFile(pAsyncReader, hr));
	if (!m_pFile) {
		return E_OUTOFMEMORY;
	}
	if (FAILED(hr)) {
		m_pFile.Free();
		return hr;
	}

	CAtlArray<CMediaType> mts;
	mts.Add(m_pFile->GetMediaType());

	CAutoPtr<CBaseSplitterOutputPin> pPinOut(DNew CBaseSplitterOutputPin(mts, L"Audio", this, this, &hr));
	AddOutputPin(0, pPinOut);

	m_rtNewStart = m_rtCurrent = 0;
	m_rtNewStop = m_rtStop = m_rtDuration = m_pFile->GetDuration();

	CStringW str, title;
	if (m_pFile->m_tags.Lookup('TIT2', str)) {
		title = str;
	}
	if (m_pFile->m_tags.Lookup('TYER', str) && !title.IsEmpty() && !str.IsEmpty()) {
		title += L" (" + str + L")";
	}
	if (!title.IsEmpty()) {
		SetProperty(L"TITL", title);
	}
	if (m_pFile->m_tags.Lookup('TPE1', str)) {
		SetProperty(L"AUTH", str);
	}
	if (m_pFile->m_tags.Lookup('TCOP', str)) {
		SetProperty(L"CPYR", str);
	}
	if (m_pFile->m_tags.Lookup('COMM', str)) {
		SetProperty(L"DESC", str);
	}

	return m_pOutputs.GetCount() > 0 ? S_OK : E_FAIL;
}

STDMETHODIMP CMpaSplitterFilter::GetDuration(LONGLONG* pDuration)
{
	CheckPointer(pDuration, E_POINTER);
	CheckPointer(m_pFile, VFW_E_NOT_CONNECTED);

	*pDuration = m_pFile->GetDuration();

	return S_OK;
}

bool CMpaSplitterFilter::DemuxInit()
{
	SetThreadName((DWORD)-1, "CMpaSplitterFilter");
	if (!m_pFile) {
		return false;
	}

	// TODO

	return true;
}

void CMpaSplitterFilter::DemuxSeek(REFERENCE_TIME rt)
{
	__int64 startpos = m_pFile->GetStartPos();
	__int64 endpos = m_pFile->GetEndPos();

	if (rt <= 0 || m_pFile->GetDuration() <= 0) {
		m_pFile->Seek(startpos);
		m_rtStart = 0;
	} else {
		m_pFile->Seek(startpos + (__int64)((1.0 * rt / m_pFile->GetDuration()) * (endpos - startpos)));
		m_rtStart = rt;
	}

}

bool CMpaSplitterFilter::DemuxLoop()
{
	HRESULT hr = S_OK;

	int FrameSize;
	REFERENCE_TIME rtDuration;

	while (SUCCEEDED(hr) && !CheckRequest(NULL) && m_pFile->GetPos() < m_pFile->GetEndPos() - 9) {
		if (!m_pFile->Sync(FrameSize, rtDuration)) {
			Sleep(1);
			continue;
		}

		CAutoPtr<Packet> p(DNew Packet());
		p->SetCount(FrameSize);
		m_pFile->ByteRead(p->GetData(), FrameSize);

		p->TrackNumber = 0;
		p->rtStart = m_rtStart;
		p->rtStop = m_rtStart + rtDuration;
		p->bSyncPoint = TRUE;

		hr = DeliverPacket(p);

		m_rtStart += rtDuration;
	}

	return true;
}

//
// CMpaSourceFilter
//

CMpaSourceFilter::CMpaSourceFilter(LPUNKNOWN pUnk, HRESULT* phr)
	: CMpaSplitterFilter(pUnk, phr)
{
	m_clsid = __uuidof(this);
	m_pInput.Free();
}
