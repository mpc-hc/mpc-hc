// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

#ifndef __ATLSIFACE_H__
#define __ATLSIFACE_H__

#pragma once
#include <atlcoll.h>
#include <httpext.h>
#include <atlserr.h>
#include <atlcom.h>
#include <string.h>
#include <atlpath.h>

#pragma pack(push,_ATL_PACKING)
namespace ATL{

// Forward declarations of custom data types used in 
// interfaces declared in this file.
struct AtlServerRequest;
class CIsapiWorker;
__interface IAtlMemMgr;
class CCookie;

// Forward declarations of all interfaces declared in this file.
__interface IWriteStream;
__interface IHttpFile;
__interface IHttpServerContext;
__interface IHttpRequestLookup;
__interface IRequestHandler;
__interface ITagReplacer;
__interface IIsapiExtension;
__interface IPageCacheControl;
__interface IRequestStats;
__interface IBrowserCaps;
__interface IBrowserCapsSvc;


// ATLS Interface declarations.

// IWriteStream
// Interface for writing to a stream.
__interface IWriteStream
{
	HRESULT WriteStream(LPCSTR szOut, int nLen, DWORD *pdwWritten);
	HRESULT FlushStream();
};

// IHttpFile
// This is an interface that provides for basic accessor
// functionality for files (see CHttpRequestFile).
__interface IHttpFile
{
	LPCSTR GetParamName();
	LPCSTR GetFileName();
	LPCSTR GetFullFileName();
	LPCSTR GetContentType();
	LPCSTR GetTempFileName();
	ULONGLONG GetFileSize();
	void Free();
};

// IHttpServerContext
// This interface encapsulates the capabilities of the web server and provides information about
// the current request being handled. See CServerContext for implementation.
__interface ATL_NO_VTABLE __declspec(uuid("813F3F00-3881-11d3-977B-00C04F8EE25E")) 
	IHttpServerContext : public IUnknown
{
	HRESULT  STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv);
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();

	LPCSTR GetRequestMethod();
	LPCSTR GetQueryString();
	LPCSTR GetPathInfo();
	LPCSTR GetPathTranslated();
	LPCSTR GetScriptPathTranslated();
	DWORD GetTotalBytes();
	DWORD GetAvailableBytes();
	BYTE *GetAvailableData();
	LPCSTR GetContentType();
	BOOL GetServerVariable(__in_z LPCSTR pszVariableName,
									__out_ecount_part_opt(*pdwSize, *pdwSize) LPSTR pvBuffer, __inout DWORD *pdwSize);
	BOOL GetImpersonationToken(HANDLE * pToken);
	BOOL WriteClient(void *pvBuffer, DWORD *pdwBytes);
	BOOL AsyncWriteClient(void *pvBuffer, DWORD *pdwBytes);
	BOOL ReadClient(void *pvBuffer, DWORD *pdwSize);
	BOOL AsyncReadClient(void *pvBuffer, DWORD *pdwSize);
	BOOL SendRedirectResponse(LPCSTR pszRedirectUrl);
	BOOL SendResponseHeader(LPCSTR pszHeader, LPCSTR pszStatusCode,
							BOOL fKeepConn);
	BOOL DoneWithSession(DWORD dwHttpStatusCode);
	BOOL RequestIOCompletion(PFN_HSE_IO_COMPLETION pfn, DWORD *pdwContext);
	BOOL TransmitFile(HANDLE hFile, PFN_HSE_IO_COMPLETION pfn, void *pContext, 
		LPCSTR szStatusCode, DWORD dwBytesToWrite, DWORD dwOffset,
		void *pvHead, DWORD dwHeadLen, void *pvTail,
		DWORD dwTailLen, DWORD dwFlags);
	BOOL AppendToLog(LPCSTR szMessage, DWORD* pdwLen);
	BOOL MapUrlToPathEx(LPCSTR szLogicalPath, DWORD dwLen, HSE_URL_MAPEX_INFO *pumInfo);
};

// IHttpRequestLookup
// This interface is designed to allow one map to chain to another map.
// The interface is implemented by the CHttpThunkMap and CHttpRequest classes.
// Pointers to this interface are passed around by CRequestHandlerT and CHtmlTagReplacer.
// dwType - the type of item being requested
__interface ATL_NO_VTABLE __declspec(uuid("A5990B44-FF74-4bfe-B66D-F9E7E9F42D42")) 
	IHttpRequestLookup : public IUnknown
{
	POSITION GetFirstQueryParam(LPCSTR *ppszName, LPCSTR *ppszValue);
	POSITION GetNextQueryParam(POSITION pos, LPCSTR *ppszName, LPCSTR *ppszValue);

	POSITION GetFirstFormVar(LPCSTR *ppszName, LPCSTR *ppszValue);
	POSITION GetNextFormVar(POSITION pos, LPCSTR *ppszName, LPCSTR *ppszValue);

	POSITION GetFirstFile(LPCSTR *ppszName, IHttpFile **ppFile);
	POSITION GetNextFile(POSITION pos, LPCSTR *ppszName, IHttpFile **ppFile);

	HRESULT GetServerContext(IHttpServerContext **ppOut);
};


// IRequestHandler
// This interface is impelemented by clients who want to be request handlers in an
// atl server application. Server default implementations are provided in ATL, including
// IRequestHandlerImpl (atlisapi.h) and CRequestHandlerT (atlstencil.h)
__interface ATL_NO_VTABLE __declspec(uuid("D57F8D0C-751A-4223-92BC-0B29F65D2453")) 
IRequestHandler : public IUnknown
{
	HTTP_CODE GetFlags(DWORD *pdwStatus);
	HTTP_CODE InitializeHandler(AtlServerRequest *pRequestInfo, IServiceProvider *pProvider);
	HTTP_CODE InitializeChild(AtlServerRequest *pRequestInfo, IServiceProvider *pProvider, IHttpRequestLookup *pLookup);
	HTTP_CODE HandleRequest(AtlServerRequest *pRequestInfo, IServiceProvider *pProvider);
	void UninitializeHandler();
};

// ITagReplacer
// This interface defines the methods necessary for server response file processing.
__interface ATL_NO_VTABLE __declspec(uuid("8FF5E90C-8CE0-43aa-96C4-3BF930837512")) 
	ITagReplacer : public IUnknown
{
	HTTP_CODE FindReplacementOffset(LPCSTR szMethodName, DWORD *pdwMethodOffset, 
						LPCSTR szObjectName, DWORD *pdwObjOffset, DWORD *pdwMap, void **ppvParam, IAtlMemMgr *pMemMgr);
	HTTP_CODE RenderReplacement(DWORD dwFnOffset, DWORD dwObjOffset, DWORD dwMap, void *pvParam);
	HRESULT GetContext(REFIID riid, void** ppv);
	IWriteStream *SetStream(IWriteStream *pStream);
};


struct CStencilState;

// IIsapiExtension
// Tnis is the interface to the ISAPI extension of a running ATL Server web
// application. Provides request handler clients with access to functions of the
// ISAPI server.
__interface __declspec(uuid("79DD4A27-D820-4fa6-954D-E1DFC2C05978"))
	IIsapiExtension : public IUnknown
{
	BOOL DispatchStencilCall(AtlServerRequest *pRequestInfo);
	void RequestComplete(AtlServerRequest *pRequestInfo, DWORD hStatus, DWORD dwSubStatus);
	BOOL OnThreadAttach();
	void OnThreadTerminate();
	BOOL QueueRequest(AtlServerRequest *pRequestInfo);
	CIsapiWorker *GetThreadWorker();
	BOOL SetThreadWorker(CIsapiWorker *pWorker);
	HTTP_CODE LoadRequestHandler(LPCSTR szDllPath, LPCSTR szHandlerName, IHttpServerContext *pServerContext,
		HINSTANCE *phInstance, IRequestHandler **ppHandler);
	HRESULT AddService(REFGUID guidService, REFIID riid, IUnknown *punk, HINSTANCE hInstance);
	HRESULT RemoveService(REFGUID guidService, REFIID riid);
	HTTP_CODE LoadDispatchFile(LPCSTR szFileName, AtlServerRequest *pRequestInfo);

	AtlServerRequest* CreateRequest();
	void FreeRequest(AtlServerRequest* pRequest);
	HTTP_CODE TransferRequest(
		AtlServerRequest *pRequest, 
		IServiceProvider *pServiceProvider,
		IWriteStream *pWriteStream,
		IHttpRequestLookup *pLookup,
		LPCSTR szNewUrl,
		WORD nCodePage,
		bool bContinueAfterProcess,
		CStencilState *pState);
};

// IPageCacheControl
// This interface controls the cacheability of the current page
__interface ATL_NO_VTABLE __declspec(uuid("9868BFC0-D44D-4154-931C-D186EC0C45D5")) 
	IPageCacheControl : public IUnknown
{
	HRESULT GetExpiration(FILETIME *pftExpiration);
	HRESULT SetExpiration(FILETIME ftExpiration);
	BOOL IsCached();
	BOOL Cache(BOOL bCache);
};

// IRequestStats
// Used to query request statistics from a running ATL server ISAPI application.
__interface ATL_NO_VTABLE __declspec(uuid("2B75C68D-0DDF-48d6-B58A-CC7C2387A6F2"))
	IRequestStats : public IUnknown
{
	long GetTotalRequests();
	long GetFailedRequests();
	long GetAvgResponseTime();
	long GetCurrWaiting();
	long GetMaxWaiting();
	long GetActiveThreads();
};

// IBrowserCaps
// Interface that provides information about a particular web brorwser.
// See atlutil.h and the ATL Browser Capabilities service for information
// about this interface's implementation
__interface __declspec(uuid("3339FCE2-99BC-4985-A702-4ABC8304A995"))
	IBrowserCaps : public IUnknown
{
	HRESULT GetPropertyString(BSTR bstrProperty, BSTR * pbstrOut);
	HRESULT GetBooleanPropertyValue(BSTR bstrProperty, BOOL* pbOut);
	HRESULT GetBrowserName(BSTR * pbstrName);
	HRESULT GetPlatform(BSTR * pbstrPlatform);
	HRESULT GetVersion(BSTR * pbstrVersion);
	HRESULT GetMajorVer(BSTR * pbstrMajorVer);
	HRESULT GetMinorVer(BSTR * pbstrMinorVer);
	HRESULT SupportsFrames(BOOL* pbFrames);
	HRESULT SupportsTables(BOOL* pbTables);
	HRESULT SupportsCookies(BOOL* pbCookies);
	HRESULT SupportsBackgroundSounds(BOOL* pbBackgroundSounds);
	HRESULT SupportsVBScript(BOOL* pbVBScript);
	HRESULT SupportsJavaScript(BOOL* pbJavaScript);
	HRESULT SupportsJavaApplets(BOOL* pbJavaApplets);
	HRESULT SupportsActiveXControls(BOOL* pbActiveXControls);
	HRESULT SupportsCDF(BOOL* pbCDF);
	HRESULT SupportsAuthenticodeUpdate(BOOL* pbAuthenticodeUpdate);
	HRESULT IsBeta(BOOL* pbIsBeta);
	HRESULT IsCrawler(BOOL* pbIsCrawler);
	HRESULT IsAOL(BOOL* pbIsAOL);
	HRESULT IsWin16(BOOL* pbIsWin16);
	HRESULT IsAK(BOOL* pbIsAK);
	HRESULT IsSK(BOOL* pbIsSK);
	HRESULT IsUpdate(BOOL* pbIsUpdate);
};

// IBrowserCapsSvc.
// Interface on the browser caps service. Used by clients to query a running
// instance of the browser capabilities service for information about a user's web
// browser. See atlutil.h for implementation of the browser capabilities services.
__interface __declspec(uuid("391E7418-863B-430e-81BB-1312ED2FF3E9"))
	IBrowserCapsSvc : public IUnknown
{
	HRESULT GetCaps(IHttpServerContext * pContext, IBrowserCaps ** ppOut);
	HRESULT GetCapsUserAgent(BSTR bstrAgent, IBrowserCaps ** ppOut);
};

class CBrowserCapsSvc : public IBrowserCapsSvc, 
						public CComObjectRootEx<CComSingleThreadModel>
{
public:
	virtual ~CBrowserCapsSvc()
	{
	}
	
	BEGIN_COM_MAP(CBrowserCapsSvc)
		COM_INTERFACE_ENTRY(IBrowserCapsSvc)
	END_COM_MAP()

	__success(SUCCEEDED(return)) __checkReturn HRESULT GetCaps(__in IHttpServerContext * pContext, __deref_out_opt IBrowserCaps ** ppOut)
	{
		if (!pContext)
			return E_POINTER;

		if (!ppOut)
			return E_POINTER;

		*ppOut = NULL;

		char szUserAgent[256];
		DWORD dwSize = sizeof(szUserAgent);
		if (!pContext->GetServerVariable("HTTP_USER_AGENT", szUserAgent, &dwSize))
			return E_FAIL;

		return GetCapsUserAgent(CComBSTR(szUserAgent), ppOut);
	}

	__success(SUCCEEDED(return)) __checkReturn HRESULT GetCapsUserAgent(__in BSTR bstrAgent, __deref_out IBrowserCaps ** ppOut)
	{
        if (::SysStringLen(bstrAgent) == 0 || ppOut == NULL)
        {
			return E_POINTER;
        }

		*ppOut = NULL;

		BrowserCaps* pCaps = NULL;

		_ATLTRY
		{
			CW2CT szUserAgent(bstrAgent);

			if (!m_mapAgent.Lookup(szUserAgent, pCaps))
			{
				pCaps = NULL;
				for (size_t i=0; i<m_caps.GetCount(); i++)
				{
					BrowserCaps& caps = m_caps[i];
					if (IsEqualAgentString(caps.m_strUserAgent, szUserAgent))
					{
						pCaps = &caps;
						break;
					}
				}
			}
		}
		_ATLCATCHALL()
		{
			return E_FAIL;
		}

#pragma warning(push)
#pragma warning(disable: 6014)
		if (pCaps != NULL)
		{
			CComObjectNoLock<CBrowserCaps> *pRet = NULL;

			ATLTRY(pRet = new CComObjectNoLock<CBrowserCaps>);

			if (!pRet)
				return E_OUTOFMEMORY;
			pRet->AddRef();

			HRESULT hr = pRet->Initialize(pCaps);
			if (FAILED(hr))
			{
				pRet->Release();
				return hr;
			}

			*ppOut = pRet;
			return S_OK;
		}
#pragma warning(pop)

		return E_FAIL;
	}

	__checkReturn HRESULT Initialize(__in HINSTANCE hInstance) throw()
	{
		// tries loading browscap.ini from the same directory as the module
		if (hInstance != NULL)
		{
			_ATLTRY
			{
				CPath strBrowscapPath;

				LPTSTR sz = strBrowscapPath.m_strPath.GetBuffer(MAX_PATH);
				UINT nChars = ::GetModuleFileName(hInstance, sz, MAX_PATH);
				strBrowscapPath.m_strPath.ReleaseBuffer(nChars);
				if (nChars != 0 &&
					nChars != MAX_PATH &&
					strBrowscapPath.RemoveFileSpec())
				{
					strBrowscapPath += _T("\\browscap.ini");
					if (SUCCEEDED(Load(strBrowscapPath)))
						return S_OK;
				}
			}
			_ATLCATCHALL()
			{
				return E_FAIL;
			}
		}

		// falls back to the system browscap.ini if previous Load failed
		return Load();
	}

	HRESULT Uninitialize()
	{
		Clear();
		return S_OK;
	}

private:
	static bool IsEqualAgentString(__in LPCTSTR szPattern, __in LPCTSTR szInput)
	{
		while (*szPattern && *szInput && (*szPattern == *szInput || *szPattern == '?'))
		{
			szPattern++;
			szInput++;
		}

		if (*szPattern == *szInput)
		{
			return true;
		}

		if (*szPattern == '*')
		{
			szPattern++;
			if (!*szPattern)
			{
				return true;
			}
			while(*szInput)
			{
				if (IsEqualAgentString(szPattern, szInput))
				{
					return true;
				}

				szInput++;
			}
		}

		return false;
	}

	__checkReturn HRESULT Load(__in_opt LPCTSTR szPath = NULL)
	{
		_ATLTRY
		{
			Clear();

			CString strBrowscapPath(szPath);

			// use default load path if a path isn't specified
			if (strBrowscapPath.IsEmpty())
			{
				LPTSTR sz = strBrowscapPath.GetBuffer(MAX_PATH);
				UINT nChars = ::GetSystemDirectory(sz, MAX_PATH);
				strBrowscapPath.ReleaseBuffer(nChars);
				if (nChars == 0 || nChars == MAX_PATH)
					return E_FAIL;

				strBrowscapPath += _T("\\inetsrv\\browscap.ini");
			}

			size_t nCurrent = 16384;
			CHeapPtr<TCHAR> data;

			if (!data.Allocate(nCurrent))
				return E_OUTOFMEMORY;

			// load the list of all the user agents
			bool bRetrieved = false;

			do
			{
				DWORD dwRetrieved = ::GetPrivateProfileSectionNames(data, (DWORD) nCurrent, strBrowscapPath);
				if (dwRetrieved == 0)
				{
					return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);			
				}
				else if (dwRetrieved < nCurrent-2)
				{
					bRetrieved = true;
				}
				else if(SIZE_MAX/2<nCurrent)
				{
					return E_OUTOFMEMORY;
				}
				else if (!data.Reallocate(nCurrent *= 2))
				{
					return E_OUTOFMEMORY;
				}
			} while (!bRetrieved);

			// figure out how many user agents there are
			// and set them in the structure
			LPTSTR sz = data;
			int nSections = 0;
			while (*sz)
			{
				nSections++;
				sz += (lstrlen(sz)+1);
			}

			if (!m_caps.SetCount(nSections))
				return E_OUTOFMEMORY;

			sz = data;
			nSections = 0;
			while (*sz)
			{
				BrowserCaps& caps = m_caps[nSections++];
				caps.m_strUserAgent = sz;
				m_mapAgent[caps.m_strUserAgent] = &caps;
				sz += (caps.m_strUserAgent.GetLength()+1);
			}

			// for each user agent, load the properties
			for (size_t i=0; i<m_caps.GetCount(); i++)
			{
				bRetrieved = false;
				BrowserCaps& caps = m_caps[i];
				caps.m_pParent = NULL;

				do
				{
					DWORD dwRetrieved = ::GetPrivateProfileSection(caps.m_strUserAgent, data, (DWORD) nCurrent, strBrowscapPath);
					if (dwRetrieved == 0)
					{
						return AtlHresultFromWin32(ERROR_FILE_NOT_FOUND);
					}
					else if (dwRetrieved < nCurrent-2)
					{
						bRetrieved = true;
					}	
					else if(SIZE_MAX/2<nCurrent)
					{
						return E_OUTOFMEMORY;
					}
					else if (!data.Reallocate(nCurrent *= 2))
					{
						return E_OUTOFMEMORY;
					}
				} while (!bRetrieved);

				sz = data;
				while (*sz)
				{
					CString str = sz;
					int nChar = str.Find('=');
					if (nChar != -1)
					{
						CString strPropName = str.Left(nChar);
						CString strPropVal = str.Mid(nChar+1);
						strPropName.Trim();
						strPropVal.Trim();
						caps.m_props.SetAt(strPropName, strPropVal);

						// if it's the parent property, set up the parent pointer
						if (strPropName.CompareNoCase(_T("parent")) == 0)
						{
							BrowserCaps* pParent = NULL;
							if (m_mapAgent.Lookup(strPropVal, pParent))
								caps.m_pParent = pParent;
						}
					}
					sz += (str.GetLength()+1);
				}
			}
		}
		_ATLCATCHALL()
		{
			return E_FAIL;
		}

		return S_OK;
	}

	void Clear()
	{
		m_caps.RemoveAll();
		m_mapAgent.RemoveAll();
	}

	friend class CBrowserCaps;

	struct BrowserCaps
	{
		CString m_strUserAgent; // user agent string to match against (with wildcards)
		BrowserCaps* m_pParent;
		CAtlMap<CString, CString, CStringElementTraitsI<CString>, CStringElementTraits<CString> > m_props;
	};

	// map from UserAgent string to caps
	// used for non-wildcard lookup and parent lookup
	CAtlMap<CString, BrowserCaps*, CStringElementTraits<CString> > m_mapAgent;

	// all of the caps
	CAtlArray<BrowserCaps> m_caps;

	class CBrowserCaps : public IBrowserCaps, public CComObjectRootEx<CComSingleThreadModel>
	{
	public:

		BEGIN_COM_MAP(CBrowserCaps)
			COM_INTERFACE_ENTRY(IBrowserCaps)
		END_COM_MAP()

		CBrowserCaps()
		{
		}

		HRESULT Initialize(__in CBrowserCapsSvc::BrowserCaps * pCaps)
		{
			m_pCaps = pCaps;
			return S_OK;
		}

		__checkReturn HRESULT GetPropertyString(__in BSTR bstrProperty, __out BSTR * pbstrOut)
		{
			_ATLTRY
			{
				ATLASSUME(m_pCaps);
				if (!m_pCaps)
					return E_UNEXPECTED;

				if (!pbstrOut)
					return E_POINTER;

				*pbstrOut = NULL;

				CString strName(bstrProperty);
				CString strVal;

				CBrowserCapsSvc::BrowserCaps * pCaps = m_pCaps;
				while (pCaps)
				{
					if (pCaps->m_props.Lookup(strName, strVal))
					{
						CComBSTR bstrVal(strVal);
						*pbstrOut = bstrVal.Detach();
						return S_OK;
					}

					pCaps = pCaps->m_pParent;
				}

				return S_FALSE;
			}
			_ATLCATCHALL()
			{
				return E_FAIL;
			}
		}

		__checkReturn HRESULT GetBooleanPropertyValue(__in BSTR bstrProperty, __out BOOL* pbOut)
		{
			if (!pbOut)
				return E_POINTER;

			CComBSTR bstrOut;
			HRESULT hr = GetPropertyString(bstrProperty, &bstrOut);
			if (FAILED(hr) || S_FALSE == hr)
				return hr;

			if (_wcsicmp(bstrOut, L"true") == 0)
				*pbOut = TRUE;
			else
				*pbOut = FALSE;

			return S_OK;
		}

		__checkReturn HRESULT GetBrowserName(__out BSTR * pbstrName)
		{
			return GetPropertyString(CComBSTR(L"browser"), pbstrName);
		}

		__checkReturn HRESULT GetPlatform(__out BSTR * pbstrPlatform)
		{
			return GetPropertyString(CComBSTR(L"platform"), pbstrPlatform);
		}

		__checkReturn HRESULT GetVersion(__out BSTR * pbstrVersion)
		{
			return GetPropertyString(CComBSTR(L"version"), pbstrVersion);
		}

		__checkReturn HRESULT GetMajorVer(__out BSTR * pbstrMajorVer)
		{
			return GetPropertyString(CComBSTR(L"majorver"), pbstrMajorVer);
		}

		__checkReturn HRESULT GetMinorVer(__out BSTR * pbstrMinorVer)
		{
			return GetPropertyString(CComBSTR(L"minorver"), pbstrMinorVer);
		}

		__checkReturn HRESULT SupportsFrames(__out BOOL* pbFrames)
		{
			return GetBooleanPropertyValue(CComBSTR(L"frames"), pbFrames);
		}

		__checkReturn HRESULT SupportsTables(__out BOOL* pbTables)
		{
			return GetBooleanPropertyValue(CComBSTR(L"tables"), pbTables);
		}
		__checkReturn HRESULT SupportsCookies(__out BOOL* pbCookies)
		{
			return GetBooleanPropertyValue(CComBSTR(L"cookies"), pbCookies);
		}
		__checkReturn HRESULT SupportsBackgroundSounds(__out BOOL* pbBackgroundSounds)
		{
			return GetBooleanPropertyValue(CComBSTR(L"backgroundsounds"), pbBackgroundSounds);
		}
		__checkReturn HRESULT SupportsVBScript(__out BOOL* pbVBScript)
		{
			return GetBooleanPropertyValue(CComBSTR(L"vbscript"), pbVBScript);
		}
		__checkReturn HRESULT SupportsJavaScript(__out BOOL* pbJavaScript)
		{
			return GetBooleanPropertyValue(CComBSTR(L"javascript"), pbJavaScript);
		}
		__checkReturn HRESULT SupportsJavaApplets(__out BOOL* pbJavaApplets)
		{
			return GetBooleanPropertyValue(CComBSTR(L"javaapplets"), pbJavaApplets);
		}
		__checkReturn HRESULT SupportsActiveXControls(__out BOOL* pbActiveXControls)
		{
			return GetBooleanPropertyValue(CComBSTR(L"ActiveXControls"), pbActiveXControls);
		}
		__checkReturn HRESULT SupportsCDF(__out BOOL* pbCDF)
		{
			return GetBooleanPropertyValue(CComBSTR(L"CDF"), pbCDF);
		}
		__checkReturn HRESULT SupportsAuthenticodeUpdate(__out BOOL* pbAuthenticodeUpdate)
		{
			return GetBooleanPropertyValue(CComBSTR(L"AuthenticodeUpdate"), pbAuthenticodeUpdate);
		}
		__checkReturn HRESULT IsBeta(__out BOOL* pbIsBeta)
		{
			return GetBooleanPropertyValue(CComBSTR(L"beta"), pbIsBeta);
		}
		__checkReturn HRESULT IsCrawler(__out BOOL* pbIsCrawler)
		{
			return GetBooleanPropertyValue(CComBSTR(L"Crawler"), pbIsCrawler);
		}
		__checkReturn HRESULT IsAOL(__out BOOL* pbIsAOL)
		{
			return GetBooleanPropertyValue(CComBSTR(L"AOL"), pbIsAOL);
		}
		__checkReturn HRESULT IsWin16(__out BOOL* pbIsWin16)
		{
			return GetBooleanPropertyValue(CComBSTR(L"Win16"), pbIsWin16);
		}
		__checkReturn HRESULT IsAK(__out BOOL* pbIsAK)
		{
			return GetBooleanPropertyValue(CComBSTR(L"AK"), pbIsAK);
		}
		__checkReturn HRESULT IsSK(__out BOOL* pbIsSK)
		{
			return GetBooleanPropertyValue(CComBSTR(L"SK"), pbIsSK);
		}
		__checkReturn HRESULT IsUpdate(__out BOOL* pbIsUpdate) 
		{
			return GetBooleanPropertyValue(CComBSTR(L"Update"), pbIsUpdate);
		}

	private:
		CBrowserCapsSvc::BrowserCaps * m_pCaps;
	};
};

typedef DWORD_PTR HSESSIONENUM;

// ISession
// Interface on a single client session. Used to access variables in the client 
// session in the session state services. See atlsession.h for implementation of
// this interface.
__interface __declspec(uuid("DEB69BE3-7AC9-4a13-9519-266C1EA3AB39")) 
	ISession : public IUnknown
{
	STDMETHOD(SetVariable)(LPCSTR szName, VARIANT NewVal);
	STDMETHOD(GetVariable)(LPCSTR szName, VARIANT *pVal);
	STDMETHOD(GetCount)(long *pnCount);
	STDMETHOD(RemoveVariable)(LPCSTR szName);
	STDMETHOD(RemoveAllVariables)();
	STDMETHOD(BeginVariableEnum)(POSITION *pPOS, HSESSIONENUM *phEnumHandle);
	STDMETHOD(GetNextVariable)(POSITION *pPOS, VARIANT *pVal, HSESSIONENUM hEnum, LPSTR szName, DWORD dwLen);
	STDMETHOD(CloseEnum)(HSESSIONENUM hEnumHandle);
	STDMETHOD(IsExpired)();
	STDMETHOD(SetTimeout)(unsigned __int64 dwNewTimeout);
}; //ISession


// ISessionStateService
// Interface on the session state service for an ISAPI application. Request
// handler objects will use this interface to access user sessions. See
// atlsession.h for implementation of this interface.
__interface __declspec(uuid("C5740C4F-0C6D-4b43-92C4-2AF778F35DDE"))
	ISessionStateService : public IUnknown
{
	STDMETHOD(CreateNewSession)(LPSTR szNewID, DWORD *pdwSize, ISession** ppSession);
	STDMETHOD(CreateNewSessionByName)(LPSTR szNewID, ISession** ppSession);
	STDMETHOD(GetSession)(LPCSTR szID, ISession **ppSession);
	STDMETHOD(CloseSession)(LPCSTR szID);
};

// ISessionStateControl
// Interface used by session state service to get information about the service.
// Currently you can get the count of active sessions and the current default
// timeout for a session.
__interface __declspec(uuid("6C7F5F56-6CBD-49ee-9797-4C837D4C527A"))
	ISessionStateControl : public IUnknown
{
	STDMETHOD(SetSessionTimeout)(unsigned __int64 nTimeout);
	STDMETHOD(GetSessionTimeout)(unsigned __int64 *pnTimeout);
	STDMETHOD(GetSessionCount)(DWORD *pnSessionCount);
}; 

}; // namespace ATL
#pragma pack(pop)

#endif // __ATLSIFACE_H__
