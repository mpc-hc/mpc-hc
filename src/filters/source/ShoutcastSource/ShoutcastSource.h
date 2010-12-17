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

#pragma once

#include <atlbase.h>
#include <atlutil.h>
#include <afxinet.h>
#include <afxsock.h>
#include <qnetwork.h>

class __declspec(uuid("68F540E9-766F-44d2-AB07-E26CC6D27A79"))
	CShoutcastSource
	: public CSource
	, public IFileSourceFilter
	, public IAMFilterMiscFlags
	, public IAMOpenProgress
	, public IAMMediaContent
{
	CStringW m_fn;

public:
	CShoutcastSource(LPUNKNOWN lpunk, HRESULT* phr);
	virtual ~CShoutcastSource();

	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void** ppv);

	// IFileSourceFilter
	STDMETHODIMP Load(LPCOLESTR pszFileName, const AM_MEDIA_TYPE* pmt);
	STDMETHODIMP GetCurFile(LPOLESTR* ppszFileName, AM_MEDIA_TYPE* pmt);

	// IAMFilterMiscFlags
	STDMETHODIMP_(ULONG) GetMiscFlags();

	// IAMOpenProgress
	STDMETHODIMP QueryProgress(LONGLONG* pllTotal, LONGLONG* pllCurrent);
	STDMETHODIMP AbortOperation();

	// IAMMediaContent
	STDMETHODIMP GetTypeInfoCount(UINT* pctinfo) {
		return E_NOTIMPL;
	}
	STDMETHODIMP GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo) {
		return E_NOTIMPL;
	}
	STDMETHODIMP GetIDsOfNames(REFIID riid, OLECHAR** rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid) {
		return E_NOTIMPL;
	}
	STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr) {
		return E_NOTIMPL;
	}
	STDMETHODIMP get_AuthorName(BSTR* pbstrAuthorName) {
		return E_NOTIMPL;
	}
	STDMETHODIMP get_Title(BSTR* pbstrTitle);
	STDMETHODIMP get_Rating(BSTR* pbstrRating) {
		return E_NOTIMPL;
	}
	STDMETHODIMP get_Description(BSTR* pbstrDescription) {
		return E_NOTIMPL;
	}
	STDMETHODIMP get_Copyright(BSTR* pbstrCopyright) {
		return E_NOTIMPL;
	}
	STDMETHODIMP get_BaseURL(BSTR* pbstrBaseURL) {
		return E_NOTIMPL;
	}
	STDMETHODIMP get_LogoURL(BSTR* pbstrLogoURL) {
		return E_NOTIMPL;
	}
	STDMETHODIMP get_LogoIconURL(BSTR* pbstrLogoURL) {
		return E_NOTIMPL;
	}
	STDMETHODIMP get_WatermarkURL(BSTR* pbstrWatermarkURL) {
		return E_NOTIMPL;
	}
	STDMETHODIMP get_MoreInfoURL(BSTR* pbstrMoreInfoURL) {
		return E_NOTIMPL;
	}
	STDMETHODIMP get_MoreInfoBannerImage(BSTR* pbstrMoreInfoBannerImage) {
		return E_NOTIMPL;
	}
	STDMETHODIMP get_MoreInfoBannerURL(BSTR* pbstrMoreInfoBannerURL) {
		return E_NOTIMPL;
	}
	STDMETHODIMP get_MoreInfoText(BSTR* pbstrMoreInfoText) {
		return E_NOTIMPL;
	}
};

class CShoutcastStream : public CSourceStream
{
	class mp3frame
	{
	public:
		DWORD len;
		BYTE* pData;
		REFERENCE_TIME rtStart, rtStop;
		CString title;
		mp3frame(DWORD len = 0) {
			this->len = len;
			pData = len ? DNew BYTE[len] : NULL;
			rtStart = rtStop = 0;
		}
		mp3frame(const mp3frame& f) {
			*this = f;
		}
		~mp3frame() {
			delete pData;
		}
		mp3frame& operator = (const mp3frame& f) {
			if(this != &f) {
				len = f.len;
				pData = f.pData;
				rtStart = f.rtStart;
				rtStop = f.rtStop;
				title = f.title;
				((mp3frame*)&f)->pData = NULL;
			}
			return *this;
		}
	};

	class mp3queue : public CAtlList<mp3frame>, public CCritSec {} m_queue;

	class CShoutcastSocket : public CSocket
	{
		DWORD m_nBytesRead;

	public:
		CShoutcastSocket() {
			m_metaint = m_bitrate = m_freq = m_channels = 0;
		}
		int Receive(void* lpBuf, int nBufLen, int nFlags = 0);

		DWORD m_metaint, m_bitrate, m_freq, m_channels;
		CString m_title, m_url;
		bool Connect(CUrl& url);
		bool FindSync();
	} m_socket;

	HANDLE m_hSocketThread;

	CUrl m_url;

	bool m_fBuffering;
	CString m_title;

public:
	CShoutcastStream(const WCHAR* wfn, CShoutcastSource* pParent, HRESULT* phr);
	virtual ~CShoutcastStream();

	bool fExitThread;
	UINT SocketThreadProc();

	void EmptyBuffer();
	LONGLONG GetBufferFullness();
	CString GetTitle();

	HRESULT DecideBufferSize(IMemAllocator* pIMemAlloc, ALLOCATOR_PROPERTIES* pProperties);
	HRESULT FillBuffer(IMediaSample* pSample);
	HRESULT CheckMediaType(const CMediaType* pMediaType);
	HRESULT GetMediaType(int iPosition, CMediaType* pmt);

	STDMETHODIMP Notify(IBaseFilter* pSender, Quality q) {
		return E_NOTIMPL;
	}

	HRESULT OnThreadCreate();
	HRESULT OnThreadDestroy();
	HRESULT Inactive();
	HRESULT Pause();
};
