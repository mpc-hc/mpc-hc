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
#include "SSF.h"

CRenderedSSF::CRenderedSSF(CCritSec* pLock)
	: ISubPicProviderImpl(pLock)
{
}

CRenderedSSF::~CRenderedSSF()
{
}

bool CRenderedSSF::Open(CString fn, CString name)
{
	m_fn.Empty();
	m_name.Empty();
	m_psf.Free();

	if(name.IsEmpty())
	{
		CString str = fn;
		str.Replace('\\', '/');
		name = str.Left(str.ReverseFind('.'));
		name = name.Mid(name.ReverseFind('/')+1);
		name = name.Mid(name.ReverseFind('.')+1);
	}

	try
	{
		if(Open(ssf::FileStream(fn), name)) 
		{
			m_fn = fn;
			return true;
		}
	}
	catch(ssf::Exception& e)
	{
		TRACE(_T("%s\n"), e.ToString());
	}

	return false;	
}

bool CRenderedSSF::Open(ssf::Stream& s, CString name)
{
	m_fn.Empty();
	m_name.Empty();
	m_psf.Free();

	try
	{
		m_psf.Attach(new ssf::SubtitleFile());
		m_psf->Parse(s);
#ifdef DEBUG
		m_psf->Dump(ssf::PLow);
		double at = 0;
		for(int i = 9000; i < 12000; i += 10)
		{
			double at = (double)i/1000;
			CAutoPtrList<ssf::Subtitle> subs;
			m_psf->Lookup(at, subs);
			POSITION pos = subs.GetHeadPosition();
			while(pos)
			{
				const ssf::Subtitle* s = subs.GetNext(pos);

				POSITION pos = s->m_text.GetHeadPosition();
				while(pos)
				{
					const ssf::Text& t = s->m_text.GetNext(pos);
					TRACE(_T("%.3f: [%.2f] %s\n"), at, t.style.font.size, t.str);
				}
			}
		}
#endif
		m_name = name;
		return true;
	}
	catch(ssf::Exception& e)
	{
		TRACE(_T("%s\n"), e.ToString());
	}

	return false;
}

STDMETHODIMP CRenderedSSF::NonDelegatingQueryInterface(REFIID riid, void** ppv)
{
    CheckPointer(ppv, E_POINTER);
    *ppv = NULL;

    return 
		QI(IPersist)
		QI(ISubStream)
		QI(ISubPicProvider)
		__super::NonDelegatingQueryInterface(riid, ppv);
}

// ISubPicProvider

STDMETHODIMP_(POSITION) CRenderedSSF::GetStartPosition(REFERENCE_TIME rt, double fps)
{
	size_t k;
	return m_psf && m_psf->m_segments.Lookup((double)rt/10000000, k) ? (POSITION)(++k) : NULL;
}

STDMETHODIMP_(POSITION) CRenderedSSF::GetNext(POSITION pos)
{
	size_t k = (size_t)pos;
	return m_psf && m_psf->m_segments.GetSegment(k) ? (POSITION)(++k) : NULL;
}

STDMETHODIMP_(REFERENCE_TIME) CRenderedSSF::GetStart(POSITION pos, double fps)
{
	size_t k = (size_t)pos-1;
	const ssf::SubtitleFile::Segment* s = m_psf ? m_psf->m_segments.GetSegment(k) : NULL;
	return s ? (REFERENCE_TIME)(s->m_start*10000000) : 0;
}

STDMETHODIMP_(REFERENCE_TIME) CRenderedSSF::GetStop(POSITION pos, double fps)
{
	CheckPointer(m_psf, 0);

	size_t k = (size_t)pos-1;
	const ssf::SubtitleFile::Segment* s = m_psf ? m_psf->m_segments.GetSegment(k) : NULL;
	return s ? (REFERENCE_TIME)(s->m_stop*10000000) : 0;
}

STDMETHODIMP_(bool) CRenderedSSF::IsAnimated(POSITION pos)
{
	return true;
}

STDMETHODIMP CRenderedSSF::Render(SubPicDesc& spd, REFERENCE_TIME rt, double fps, RECT& bbox)
{
	CheckPointer(m_psf, E_UNEXPECTED);

	CAutoLock csAutoLock(m_pLock);

	double at = (double)rt/10000000;

	CAutoPtrList<ssf::Subtitle> subs;
	m_psf->Lookup(at, subs);

	POSITION pos = subs.GetHeadPosition();
	while(pos)
	{
		ssf::Subtitle* s = subs.GetNext(pos);

		// TODO: render s...

		TRACE(_T("start: %.3f, stop: %.3f\n"), s->m_time.start, s->m_time.stop);

		POSITION pos = s->m_text.GetHeadPosition();
		while(pos)
		{
			ssf::Text& t = s->m_text.GetNext(pos);
/*

			ssf::Placement& p = t.style.placement;
			TRACE(_T("%.1f: %d [%.0f-%.0f-%.0f-%.0f, %f, %.0f-%.0f-%.0f-%.0f] '%s'\n"), 
				at, 
				t.style.fill.id,
				t.style.font.color[0], t.style.font.color[1], t.style.font.color[2], t.style.font.color[3],
				t.style.fill.width, 
				t.style.fill.color[0], t.style.fill.color[1], t.style.fill.color[2], t.style.fill.color[3],
				t.str);
*/
		}

		TRACE(_T("------------\n"));
	}

	return E_NOTIMPL;
}

// IPersist

STDMETHODIMP CRenderedSSF::GetClassID(CLSID* pClassID)
{
	return pClassID ? *pClassID = __uuidof(this), S_OK : E_POINTER;
}

// ISubStream

STDMETHODIMP_(int) CRenderedSSF::GetStreamCount()
{
	return 1;
}

STDMETHODIMP CRenderedSSF::GetStreamInfo(int iStream, WCHAR** ppName, LCID* pLCID)
{
	if(iStream != 0) return E_INVALIDARG;

	if(ppName)
	{
		if(!(*ppName = (WCHAR*)CoTaskMemAlloc((m_name.GetLength()+1)*sizeof(WCHAR))))
			return E_OUTOFMEMORY;

		wcscpy(*ppName, CStringW(m_name));
	}

	if(pLCID)
	{
		*pLCID = 0; // TODO
	}

	return S_OK;
}

STDMETHODIMP_(int) CRenderedSSF::GetStream()
{
	return 0;
}

STDMETHODIMP CRenderedSSF::SetStream(int iStream)
{
	return iStream == 0 ? S_OK : E_FAIL;
}

STDMETHODIMP CRenderedSSF::Reload()
{
	CAutoLock csAutoLock(m_pLock);

	return !m_fn.IsEmpty() && Open(m_fn, m_name) ? S_OK : E_FAIL;
}
