// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

#ifndef __ATLSHAREDSVC_H__
#define __ATLSHAREDSVC_H__

#pragma once

#include <atltime.h>
#include <atlsoap.h>
#pragma pack(push,_ATL_PACKING)
namespace ATL{

#ifndef ATL_SHAREDBLOBCACHE_TIMEOUT
	#define ATL_SHAREDBLOBCACHE_TIMEOUT 36000000000 // in 100 nano second intervals
													 // each entry will be free'd if 
													// no access in 1 hour.
#endif

// Interface used by to access the shared blob cache.
[ uuid("AB4AF9CD-8DB1-4974-A617-CF0449578FB9"), object ]
__interface ISharedBlobCache
{
	[id(0)] STDMETHOD(AddItem)([in] BSTR szItemName, [in] BSTR szData);
	[id(1)] STDMETHOD(GetItem)([in] BSTR szItemName, [out,retval] BSTR *szData); 
};

class CSharedCache: 
	public CBlobCache<CWorkerThread<>, CStdStatClass >,
	public IMemoryCacheClient,
	public ISharedBlobCache
{
	typedef CBlobCache<CWorkerThread<>, CStdStatClass > basecache;
public:

	// IMemoryCacheClient method, frees data in the memory cache.
	STDMETHOD( Free )(const void *pvData)
	{
		if (pvData)
		{
			::SysFreeString((BSTR)pvData);
		}
		return S_OK;
	}


	STDMETHODIMP AddItem(BSTR szItemName, BSTR szData)
	{

		HRESULT hr = E_UNEXPECTED;

		// We make a copy of the BSTR and stick it in the cache.
		// The BSTR will be freed in our IMemoryCacheClient::Free
		// implementation above.
		BSTR szEntry = SysAllocString(szData);
		if(szEntry)
		{
			USES_CONVERSION_EX;
			// create a time span and for the entry
			CFileTime tm = CFileTime::GetCurrentTime();
			CFileTimeSpan span;
			span.SetTimeSpan(ATL_SHAREDBLOBCACHE_TIMEOUT);
			tm += span;
			HCACHEITEM h;
			hr = basecache::Add(OLE2A_EX(szItemName, _ATL_SAFE_ALLOCA_DEF_THRESHOLD), szEntry, sizeof(BSTR), 
				&tm, _AtlBaseModule.m_hInst, &h, static_cast<IMemoryCacheClient*>(this));

			if (hr == S_OK)
			{
				// On successful add, we have to release our 
				// reference on the entry.
				basecache::ReleaseEntry(h);
			}
		}
		return hr;
	}

	STDMETHODIMP GetItem(BSTR szItemName, BSTR *szData)
	{
		USES_CONVERSION_EX;
		HRESULT hr = E_UNEXPECTED;
		HCACHEITEM hEntry = NULL;

		if (!szItemName || !szData)
			return hr;

		hr = basecache::LookupEntry(OLE2A_EX(szItemName, _ATL_SAFE_ALLOCA_DEF_THRESHOLD), &hEntry);
		if (hr == S_OK)
		{
			void *pData = NULL;
			DWORD dwSize = 0;
			hr = basecache::GetData(hEntry, &pData, &dwSize);
			if (hr == S_OK)
			{
				// make a copy of the string
				*szData = ::SysAllocString((BSTR)pData);
			}
			basecache::ReleaseEntry(hEntry);
		}
		return hr;
	}


	STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
	{
		HRESULT hr = E_NOINTERFACE;
		if (InlineIsEqualGUID(__uuidof(IMemoryCacheClient), riid)||
			InlineIsEqualGUID(__uuidof(IUnknown), riid))
		{
			*ppv = static_cast<void*>(static_cast<IMemoryCacheClient*>(this));
			hr = S_OK;
		}
		else if( InlineIsEqualGUID(__uuidof(ISharedBlobCache), riid))
		{
			*ppv = static_cast<void*>(static_cast<ISharedBlobCache*>(this));
			hr = S_OK;
		}
		return hr;
	}
	ULONG STDMETHODCALLTYPE AddRef()
	{
		return 1;
	}
	ULONG STDMETHODCALLTYPE Release()
	{
		return 1;
	}
};


// This class implements the SOAP interface for the shared blob cache.
[
	soap_handler(
					name="SharedBlobCache", 
					namespace="http://www.microsoft.com/vc/atlserver/soap/SharedBlobCache",
					protocol="soap"
				),
	request_handler(
					name="SharedBlobCache",
					sdl="GenSharedBlobCacheWSDL"
					)
]
class CSharedCacheHandler:
	public ISharedBlobCache
{
public:
	[soap_method]
	STDMETHOD(AddItem)(BSTR szItemName, BSTR szData)
	{
		if (!m_spMemCache)
			return E_UNEXPECTED;
		return m_spMemCache->AddItem(szItemName, szData);
	}

	[soap_method]
	STDMETHOD(GetItem)(BSTR szItemName, BSTR *szData)
	{	
		if (!m_spMemCache)
			return E_UNEXPECTED;
		return m_spMemCache->GetItem(szItemName, szData);
	}

	HTTP_CODE Initialize(IServiceProvider *pProvider)
	{
		ATLASSERT(pProvider); // should never be NULL
		if (!pProvider)
			return HTTP_ERROR(500, ISE_SUBERR_UNEXPECTED);

		if (m_spMemCache)
			return HTTP_SUCCESS; // already initialized

		pProvider->QueryService(__uuidof(ISharedBlobCache), &m_spMemCache);
		return m_spMemCache ? HTTP_SUCCESS : HTTP_ERROR(500, ISE_SUBERR_UNEXPECTED);
	}

	// override HandleRequest to Initialize our m_spServiceProvider
	// and to handle authorizing the client.
	HTTP_CODE HandleRequest(AtlServerRequest *pRequestInfo, IServiceProvider *pProvider)
	{
		HTTP_CODE dwErr = Initialize(pProvider);
		if (dwErr != HTTP_SUCCESS)
			return dwErr;

		dwErr = CSoapHandler<CSharedCacheHandler>::HandleRequest(pRequestInfo,
								pProvider);
		return dwErr;
	}
	CComPtr<ISharedBlobCache> m_spMemCache;
};

} //ATL

#pragma pack(pop)

#endif // __ATLSHAREDSVC_H__
