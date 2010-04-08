// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

#ifndef __ATLEXTMGMT_H__
#define __ATLEXTMGMT_H__

#pragma once
#pragma warning(push)
#pragma warning(disable: 4702)
#include <atlsoap.h>
#include <atlutil.h>
#include <atlsrvres.h>
#include <atlsecurity.h>

//
// You can change the local group that is used for authorizing
// site administrators by #define'ing ATL_DEFAULT_AUTH group
// to something else before including this header file. For
// example:
// #define ATL_DEFAULT_AUTHGRP CSid(_T("My Heros"))
//     Verify that the logged on user is a member of
//     the local group 'My Heros' before allowing them to
//     administrate this site.
//
// #define ATL_DEFAULT_AUTHGRP Sids::World
//     Allow everyone access
//
// #define ATL_DEFAULT_AUTHGRP Sids::Null
//     Allow no one access
//
#ifndef ATL_DEFAULT_AUTHGRP
#define ATL_DEFAULT_AUTHGRP Sids::Admins()
#endif

// If you #define ATL_NO_DEFAULT_AUTHORITY then there will be no authorization
// check before allowing access to management functions. You can also #define
// ATL_NO_DEFAULT_AUTHORITY and then declare you own instance of _Authority
// before #include-ing atlextmgmt.h to use a different authorization scheme.
#ifndef ATL_NO_DEFAULT_AUTHORITY
__declspec(selectany) CDefaultAuth _Authority;
#endif

// You can choose which of the management handlers actually get used by
// #defining the following constants before including this header
// _ATL_THREADPOOL_MANAGEMENT (The thread pool manager web service and web based UI)
// _ATL_STENCILCACHE_MANAGEMENT (The stencil cache manager web service and web based UI)
// _ATL_DLLCACHE_MANAGEMENT (The DLL cache manager service and web based UI)

// You can use the following constants to remove the web based UI if you don't
// want to use it.
// _ATL_THREADPOOL_NOUI (removes the thread pool mgr's stencil handler)
// _ATL_STENCILCACHE_NOUI (removes the stencil cache mgr's stencil handler)
// _ATL_DLLCACHE_NOUI (removes the dll cache mgr's stencil handler)

// You can use the following constants to remove the web service management
// components individually
// _ATL_THREADPOOL_NOWEBSERVICE (removes the thread pool mgr's stencil handler)
// _ATL_STENCILCACHE_NOWEBSERVICE (removes the stencil cache mgr's stencil handler)
// _ATL_DLLCACHE_NOWEBSERVICE (removes the dll cache mgr's stencil handler)


// The following constants declare resource names of stencils included
// as resources in the module that uses this header. These stencils are
// used for the web based UI for the management objects. You can provide
// stencils of your own by including them as resources and redefining these
// constants before including this header.
#ifndef IDR_THREADMGR_SRF
#define IDR_THREADMGR_SRF "THREADMGR.SRF"
#endif

#ifndef IDR_STENCILMGR_SRF
#define IDR_STENCILMGR_SRF "STENCILMGR.SRF"
#endif

#ifndef IDR_DLLMGR_SRF
#define IDR_DLLMGR_SRF "DLLMGR.SRF"
#endif

// A warning so users using the web based UI to manage their extension
// will remember to include the stencil resources in their projects
#if (defined(_ATL_THREADPOOL_MANAGEMENT) && !defined(_ATL_THREADPOOL_NOUI)) ||	(defined(_ATL_STENCILCACHE_MANAGEMENT) && !defined(_ATL_STENCILCACHE_NOUI)) ||	(defined(_ATL_DLLCACHE_MANAGEMENT) && !defined(_ATL_DLLCACHE_NOUI))
#ifndef NO_ATL_MGMT_STENCIL_WARNING
#pragma message("*************** Please Note ***************")
#pragma message("Your usage of atlextmgmt.h requires you to include management")
#pragma message("stencil resources in your module's resource file.")
#pragma message("Please make sure you include atlsrv.rc in your resource file.\r\n")
#endif
#endif

// These constants define the names used for the handler objects for the
// various services. You can change the names by redefining these constants
// before including this header

#ifndef ID_THREADMGR_WEBSERVICE_NAME
#define ID_THREADMGR_WEBSERVICE_NAME "ThreadPoolManager"
#endif

#ifndef ID_THREADMGR_WEBSERVICE_URL
#define ID_THREADMGR_WEBSERVICE_URL "http://www.microsoft.com/vc/atlserver/soap/ThreadPoolManager"
#endif

#ifndef ID_THREADMGR_WEBSERVICE_WSDL
#define ID_THREADMGR_WEBSERVICE_WSDL "GenThreadPoolManagerWSDL"
#endif

#ifndef ID_THREADMGR_SRFHANDLER_NAME
#define ID_THREADMGR_SRFHANDLER_NAME "ThreadMgrSrf"
#endif

#ifndef ID_STENCILCACHEMGR_WEBSERVICE_NAME
#define ID_STENCILCACHEMGR_WEBSERVICE_NAME "StencilCacheManager"
#endif

#ifndef ID_STENCILCACHEMGR_WEBSERVICE_URL
#define ID_STENCILCACHEMGR_WEBSERVICE_URL "http://www.microsoft.com/vc/atlserver/soap/StencilCacheManager"
#endif

#ifndef ID_STENCILCACHEMGR_WEBSERVICE_WSDL
#define ID_STENCILCACHEMGR_WEBSERVICE_WSDL "GenStencilCacheManagerWSDL"
#endif

#ifndef ID_STENCILCACHEMGR_SRFHANDLER_NAME
#define ID_STENCILCACHEMGR_SRFHANDLER_NAME "StencilMgrSrf"
#endif

#ifndef ID_DLLCACHEMGR_WEBSERVICE_NAME
#define ID_DLLCACHEMGR_WEBSERVICE_NAME "DllCacheManager"
#endif

#ifndef ID_DLLCACHEMGR_WEBSERVICE_URL
#define ID_DLLCACHEMGR_WEBSERVICE_URL "http://www.microsoft.com/vc/atlserver/soap/DllCacheManager"
#endif

#ifndef ID_DLLCACHEMGR_WEBSERVICE_WSDL
#define ID_DLLCACHEMGR_WEBSERVICE_WSDL "GenDllCacheManagerWSDL"
#endif


#ifndef ID_DLLCACHEMGR_SRFHANDLER_NAME
#define ID_DLLCACHEMGR_SRFHANDLER_NAME "DllMgrSrf"
#endif

#pragma pack(push,_ATL_PACKING)
namespace ATL
{

[emitidl(restricted)];

#define ATL_COLOR_TR1			RGB(0xd2, 0xff, 0xff)
#define ATL_COLOR_TR2			RGB(0xd2, 0xff, 0xd2)
#define ATL_COLOR_BODYBG		RGB(0xec, 0xf9, 0xec)

// _AtlRedirectToPage builds up a redirect URL from the
// current request plus a Handler= specification and
// redirects the user's browser to that page.
inline HTTP_CODE _AtlRedirectToPage(
    IHttpServerContext *pContext,
    CHttpRequest& request,
    CHttpResponse& response,
    const char *szHandler)
{
    ATLENSURE(pContext);
    CStringA strRedirect("http://");

    char buff[ATL_URL_MAX_URL_LENGTH];
    DWORD dwLen = static_cast<DWORD>(_countof(buff));
    if(!pContext->GetServerVariable("SERVER_NAME", buff, &dwLen))
    {
        return HTTP_FAIL;
    }
    buff[_countof(buff)-1] = '\0';
    strRedirect += buff;

    dwLen = static_cast<DWORD>(_countof(buff));
    if(!request.GetUrl(buff, &dwLen))
    {
        return HTTP_FAIL;
    }
    buff[_countof(buff)-1] = '\0';
    strRedirect += buff;
    strRedirect += szHandler;

    if(strRedirect.GetLength() >= ATL_URL_MAX_URL_LENGTH)
    {
        return HTTP_FAIL;
    }

    BOOL bOK = response.Redirect(strRedirect.GetString());

    return bOK ? HTTP_SUCCESS_NO_PROCESS : HTTP_FAIL;
}

#ifdef _ATL_THREADPOOL_MANAGEMENT
///////////////////////////////////////////////////////////////////////
// Thread pool management

[ uuid("44e9962a-5207-4d2a-a466-5f08a76e0e5d"), object ]
__interface IThreadPoolMgr
{
    [id(0)] STDMETHOD(SetSize)([in] int nNumThreads);
    [id(1)] STDMETHOD(GetSize)([out, retval] int *pnNumThreads);

};


class CThreadPoolMgrObject
{
public:
    CThreadPoolMgrObject() throw()
    {
    }

    HRESULT SetSize(int nNumThreads) throw()
    {
        if(!m_spThreadPoolConfig)
            return E_UNEXPECTED;

        CRevertThreadToken revert;
        if(!revert.Initialize())
            return E_FAIL;

        HRESULT hr = m_spThreadPoolConfig->SetSize(nNumThreads);

        DWORD dwErr = revert.Restore();
        if(dwErr)
            return AtlHresultFromWin32(dwErr);

        return hr;
    }


    HRESULT GetSize(int *pnNumThreads) throw()
    {
        if(!m_spThreadPoolConfig)
            return E_UNEXPECTED;

        return m_spThreadPoolConfig->GetSize(pnNumThreads);

    }

    HTTP_CODE Initialize(IServiceProvider *pProvider) throw()
    {
        ATLASSERT(pProvider); // should never be NULL
        if(!pProvider)
            return HTTP_ERROR(500, ISE_SUBERR_UNEXPECTED);

        if(m_spThreadPoolConfig)
            return HTTP_SUCCESS; // already initialized

        pProvider->QueryService(__uuidof(IThreadPoolConfig), &m_spThreadPoolConfig);
        return m_spThreadPoolConfig ? HTTP_SUCCESS : HTTP_ERROR(500, ISE_SUBERR_UNEXPECTED);
    }

private:
    CComPtr<IThreadPoolConfig> m_spThreadPoolConfig;
};

#ifndef _ATL_THREADPOOL_NOWEBSERVICE
#pragma warning(push)
#pragma warning(disable:4199)
[
    soap_handler(
        name=      ID_THREADMGR_WEBSERVICE_NAME,
        namespace= ID_THREADMGR_WEBSERVICE_URL,
        protocol=  "soap"
    ),
    request_handler(
        name= ID_THREADMGR_WEBSERVICE_NAME,
        sdl=  ID_THREADMGR_WEBSERVICE_WSDL
    )
]
class CThreadPoolManager :
    public IThreadPoolMgr
{
#pragma warning(pop)
public:
    [soap_method]
    STDMETHOD(SetSize)(int nNumThreads)
    {
        return m_PoolMgr.SetSize(nNumThreads);
    }

    [soap_method]
    STDMETHOD(GetSize)(int *pnNumThreads)
    {
        return m_PoolMgr.GetSize(pnNumThreads);
    }

    // override HandleRequest to Initialize our m_spServiceProvider
    // and to handle authorizing the client.
    HTTP_CODE HandleRequest(AtlServerRequest *pRequestInfo, IServiceProvider *pProvider)
    {
        HTTP_CODE hcErr = m_PoolMgr.Initialize(pProvider);
        if(hcErr != HTTP_SUCCESS)
            return hcErr;

        // Make sure caller is authorized on this system
        __if_exists(_Authority)
        {
            hcErr = HTTP_FAIL;
            ATLTRY(hcErr = _Authority.IsAuthorized(pRequestInfo, ATL_DEFAULT_AUTHGRP))
        }
        if(hcErr == HTTP_SUCCESS)
        {
            hcErr = __super::HandleRequest(pRequestInfo, pProvider);
        }
        return hcErr;
    }
private:
    CThreadPoolMgrObject m_PoolMgr;
};
#endif //_ATL_THREADPOOL_NOWEBSERVICE

#ifndef _ATL_THREADPOOL_NOUI
#define INVALID_COMMAND_ID -1
#define MAX_COMMAND_ID 64

[request_handler(name=ID_THREADMGR_SRFHANDLER_NAME)]
class CThreadMgrStencil
{
public:
    CThreadMgrStencil() :
        m_nColor(ATL_COLOR_TR1)
    {

    }

    [tag_name("GetSize")]
    HTTP_CODE GetSize()
    {
        int nSize = 0;
        HRESULT hr = m_PoolMgr.GetSize(&nSize);
        if(SUCCEEDED(hr))
        {
            m_HttpResponse << nSize;
        }
        else
            m_HttpResponse << "size not found";

        return HTTP_SUCCESS;
    }

    [tag_name("GetTRColor")]
    HTTP_CODE GetTRColor()
    {
        m_nColor = (m_nColor == ATL_COLOR_TR1) ? ATL_COLOR_TR2 : ATL_COLOR_TR1;
        TCHAR cr[8];
        if(RGBToHtml(m_nColor, cr, sizeof(cr)))
            m_HttpResponse << cr;

        return HTTP_SUCCESS;
    }

    [tag_name("GetBodyColor")]
    HTTP_CODE GetBodyColor()
    {
        TCHAR cr[8];
        if(RGBToHtml(ATL_COLOR_BODYBG, cr, sizeof(cr)))
            m_HttpResponse << cr;
        return HTTP_SUCCESS;
    }


    HTTP_CODE ValidateAndExchange() throw()
    {
        _ATLTRY
        {
            // Initialize the thread pool manager instance. Internally
            // the initialize function will only intialize it's data structures
            // once.
            HTTP_CODE hcErr = m_PoolMgr.Initialize(m_spServiceProvider);
            if(hcErr != HTTP_SUCCESS)
                return hcErr;

            __if_exists(_Authority)
            {
                // Make sure caller is authorized on this system
                hcErr = HTTP_FAIL;
                ATLTRY(hcErr = _Authority.IsAuthorized(m_pRequestInfo, ATL_DEFAULT_AUTHGRP))
                if(hcErr != HTTP_SUCCESS)
                    return hcErr;
            }


            m_HttpResponse.SetContentType("text/html");

            CString strHandler, strOptParam;
            int nCmdToExec = INVALID_COMMAND_ID;

            if(m_HttpRequest.GetMethod() == CHttpRequest::HTTP_METHOD_POST)
            {
                // check to see if we have a "Method" form variable and can execute a command
                DWORD dwErr = m_HttpRequest.FormVars.Exchange("Method", &strHandler);
                if(dwErr == VALIDATION_S_OK)
                {
                    if(strHandler == _T("ExecuteCommand"))
                    {
                        // get the value of the command parameter so we can execute it
                        dwErr = m_HttpRequest.FormVars.Validate("command", &nCmdToExec, 0, MAX_COMMAND_ID);
                        if(dwErr == VALIDATION_S_OK)
                        {
                            // get the optional parameter if it's there.
                            m_HttpRequest.FormVars.Validate("DynValue", &strOptParam, 0, MAX_COMMAND_ID);

                            hcErr = ExecCommand(nCmdToExec, strOptParam);
                            return hcErr;
                        }
                    }
                }
            }

            // If we had a proper command to execute, we would have done it by now.
            // Just handle like it's a normal request to view the thread count.
            hcErr = LoadStencilResource(m_hInstHandler, IDR_THREADMGR_SRF);
            return hcErr;

        }
        _ATLCATCHALL()
        {
            return HTTP_FAIL;
        }
    }

    HTTP_CODE ExecCommand(int nCmdToExec, CString& strOptParam)
    {
        switch(nCmdToExec)
        {
        case 0:
            TCHAR *pStop = NULL;
            int nValue = _tcstoul(strOptParam, &pStop, 10);
            m_PoolMgr.SetSize(nValue);
            break;
        };

        return _AtlRedirectToPage(
                   m_spServerContext,
                   m_HttpRequest,
                   m_HttpResponse,
                   "?Handler=" ID_THREADMGR_SRFHANDLER_NAME
               );
    }
private:
    CThreadPoolMgrObject m_PoolMgr;
    long m_nColor;
    CString m_strUrl;

};


#endif // _ATL_THREADPOOL_NOUI
#endif // _ATL_THREADPOOL_MANAGEMENT

#ifdef _ATL_STENCILCACHE_MANAGEMENT
//////////////////////////////////////////////////////////////////////
// Stencil cache management
class CStencilCacheMgrObject
{
public:
    CStencilCacheMgrObject()
    {

    }

    HRESULT GetCurrentEntryCount(__int64 *pdwSize)
    {
        ATLASSUME(m_spMemCacheStats);
        if(!pdwSize)
            return E_INVALIDARG;

        DWORD dwValue;
        HRESULT hr = m_spMemCacheStats->GetCurrentEntryCount(&dwValue);
        if(hr == S_OK)
        {
            *pdwSize = dwValue;
        }
        return hr;
    }

    HRESULT ClearStats()
    {
        ATLENSURE(m_spMemCacheStats);
        return m_spMemCacheStats->ClearStats();
    }

    HRESULT GetHitCount(__int64 *pdwSize)
    {
        ATLENSURE(m_spMemCacheStats);
        if(!pdwSize)
            return E_INVALIDARG;

        DWORD dwValue;
        HRESULT hr = m_spMemCacheStats->GetHitCount(&dwValue);
        if(hr == S_OK)
        {
            *pdwSize = dwValue;
        }
        return hr;
    }

    HRESULT GetMissCount(__int64 *pdwSize)
    {
        ATLENSURE(m_spMemCacheStats);
        if(!pdwSize)
            return E_INVALIDARG;

        DWORD dwValue;

        HRESULT hr = m_spMemCacheStats->GetMissCount(&dwValue);
        if(hr == S_OK)
        {
            *pdwSize = dwValue;
        }
        return hr;
    }

    HRESULT GetCurrentAllocSize(__int64 *pdwSize)
    {
        ATLENSURE(m_spMemCacheStats);
        if(!pdwSize)
            return E_INVALIDARG;

        DWORD dwValue;

        HRESULT hr = m_spMemCacheStats->GetCurrentAllocSize(&dwValue);
        if(hr == S_OK)
        {
            *pdwSize = dwValue;
        }
        return hr;
    }

    HRESULT GetMaxAllocSize(__int64 *pdwSize)
    {
        ATLENSURE(m_spMemCacheStats);
        if(!pdwSize)
            return E_INVALIDARG;

        DWORD dwValue;

        HRESULT hr = m_spMemCacheStats->GetMaxAllocSize(&dwValue);
        if(hr == S_OK)
        {
            *pdwSize = dwValue;
        }
        return hr;
    }


    HRESULT GetMaxEntryCount(__int64 *pdwSize)
    {
        ATLENSURE(m_spMemCacheStats);
        if(!pdwSize)
            return E_INVALIDARG;

        DWORD dwValue;

        HRESULT hr = m_spMemCacheStats->GetMaxEntryCount(&dwValue);
        if(hr == S_OK)
        {
            *pdwSize = dwValue;
        }
        return hr;
    }


    HRESULT RemoveStencil(__int64 hStencil)
    {
        ATLENSURE(m_spStencilCacheControl);
        return m_spStencilCacheControl->RemoveStencil((const HCACHEITEM)hStencil);
    }

    HRESULT RemoveStencilByName(BSTR szStencil) throw()
    {
        ATLENSURE_RETURN(m_spStencilCacheControl);
        return m_spStencilCacheControl->RemoveStencilByName(CW2A(szStencil));
    }


    HRESULT RemoveAllStencils()
    {
        ATLENSURE(m_spStencilCacheControl);
        return m_spStencilCacheControl->RemoveAllStencils();
    }

    // we show lifespan in milliseconds in the UI so we have to
    // do the conversion to 100ns intervals here.
    HRESULT SetDefaultLifespan(unsigned __int64 dwdwLifespan)
    {
        ATLENSURE(m_spStencilCacheControl);
        // convert to 100ns intervals
        return m_spStencilCacheControl->SetDefaultLifespan(dwdwLifespan * CFileTime::Millisecond);
    }

    HRESULT GetDefaultLifespan(unsigned __int64 *pdwdwLifespan)
    {
        ATLENSURE(m_spStencilCacheControl);
        ATLENSURE(pdwdwLifespan != NULL);
        *pdwdwLifespan = 0;
        unsigned __int64 dwls = 0;
        HRESULT hr = m_spStencilCacheControl->GetDefaultLifespan(&dwls);

        // convert to milliseconds
        if(SUCCEEDED(hr))
        {
            dwls /= CFileTime::Millisecond;
            *pdwdwLifespan = dwls;
        }

        return hr;
    }

    HTTP_CODE Initialize(IServiceProvider *pProvider) throw()
    {

        ATLASSERT(pProvider); // should never be NULL
        if(!pProvider)
            return HTTP_ERROR(500, ISE_SUBERR_UNEXPECTED);


        if(m_spMemCacheStats && m_spStencilCacheControl)
            return HTTP_SUCCESS; // already initialized

        CComPtr<IStencilCache> spStencilCache;
        pProvider->QueryService(__uuidof(IStencilCache), &spStencilCache);
        if(spStencilCache)
        {
            if(!m_spMemCacheStats)
            {
                spStencilCache->QueryInterface(__uuidof(IMemoryCacheStats),
                                               (void**)&m_spMemCacheStats);
            }
            if(!m_spStencilCacheControl)
            {
                spStencilCache->QueryInterface(__uuidof(IStencilCacheControl),
                                               (void**)&m_spStencilCacheControl);
            }
        }

        return (m_spMemCacheStats && m_spStencilCacheControl)
               ? HTTP_SUCCESS : HTTP_ERROR(500, ISE_SUBERR_UNEXPECTED);
    }

private:
    CComPtr<IMemoryCacheStats> m_spMemCacheStats;
    CComPtr<IStencilCacheControl> m_spStencilCacheControl;
};


#ifndef _ATL_STENCILCACHE_NOWEBSERVICE

[ uuid("3813895C-4C4C-41df-95F4-12220140B164"), object ]
__interface IStencilCacheMgr
{
    // data access
    [id(0)] STDMETHOD(GetCurrentEntryCount)([out, retval] __int64 *pdwSize);
    [id(1)] STDMETHOD(GetHitCount)([out, retval] __int64 *pdwSize);
    [id(2)] STDMETHOD(GetMissCount)([out, retval] __int64 *pdwSize);
    [id(3)] STDMETHOD(GetCurrentAllocSize)([out, retval] __int64 *pdwSize);
    [id(4)] STDMETHOD(GetMaxAllocSize)([out, retval] __int64 *pdwSize);
    [id(5)] STDMETHOD(GetMaxEntryCount)([out, retval] __int64 *pdwSize);
    [id(6)] STDMETHOD(GetDefaultLifespan)([out, retval] unsigned __int64 *pdwdwLifespan);

    // commands
    [id(7)] STDMETHOD(ClearStats)();
    [id(8)] STDMETHOD(RemoveStencil)([in] __int64 hStencil);
    [id(9)] STDMETHOD(RemoveStencilByName)([in] BSTR szStencil);
    [id(10)] STDMETHOD(RemoveAllStencils)();
    [id(11)] STDMETHOD(SetDefaultLifespan)([in] unsigned __int64 dwdwLifespan);
};

#pragma warning(push)
#pragma warning(disable:4199)
[
    soap_handler(name=		ID_STENCILCACHEMGR_WEBSERVICE_NAME,
                 namespace=	ID_STENCILCACHEMGR_WEBSERVICE_URL,
                 protocol=	"soap"
                ),
    request_handler(
        name=		ID_STENCILCACHEMGR_WEBSERVICE_NAME,
        sdl=		ID_STENCILCACHEMGR_WEBSERVICE_WSDL)
]
class CStencilCacheManager :
    public IStencilCacheMgr
{
#pragma warning(pop)
public:
    [ soap_method ]
    STDMETHOD(GetCurrentEntryCount)(__int64 *pdwSize)
    {
        return m_MgrObj.GetCurrentEntryCount(pdwSize);
    }

    [ soap_method ]
    STDMETHOD(ClearStats)()
    {
        return m_MgrObj.ClearStats();
    }

    [ soap_method ]
    STDMETHOD(GetHitCount)(__int64 *pdwSize)
    {
        return m_MgrObj.GetHitCount(pdwSize);
    }

    [ soap_method ]
    STDMETHOD(GetMissCount)(__int64 *pdwSize)
    {
        return m_MgrObj.GetMissCount(pdwSize);
    }

    [ soap_method ]
    STDMETHOD(GetCurrentAllocSize)(__int64 *pdwSize)
    {
        return m_MgrObj.GetCurrentAllocSize(pdwSize);
    }

    [ soap_method ]
    STDMETHOD(GetMaxAllocSize)(__int64 *pdwSize)
    {
        return m_MgrObj.GetMaxAllocSize(pdwSize);
    }

    [ soap_method ]
    STDMETHOD(GetMaxEntryCount)(__int64 *pdwSize)
    {
        return m_MgrObj.GetMaxEntryCount(pdwSize);
    }

    [ soap_method ]
    STDMETHOD(RemoveStencil)(__int64 hStencil)
    {
        return m_MgrObj.RemoveStencil(hStencil);
    }

    [ soap_method ]
    STDMETHOD(RemoveStencilByName)(BSTR bstrStencil)
    {
        return m_MgrObj.RemoveStencilByName(bstrStencil);
    }

    [ soap_method ]
    STDMETHOD(RemoveAllStencils)()
    {
        return m_MgrObj.RemoveAllStencils();
    }

    // we show lifespan in milliseconds in the UI.
    // m_MgrObj handles the conversion to 100ns intervals.
    [ soap_method ]
    STDMETHOD(SetDefaultLifespan)(unsigned __int64 dwdwLifespan)
    {
        return m_MgrObj.SetDefaultLifespan(dwdwLifespan);
    }

    [ soap_method ]
    STDMETHOD(GetDefaultLifespan)(unsigned __int64 *pdwdwLifespan)
    {
        return m_MgrObj.GetDefaultLifespan(pdwdwLifespan);
    }

    HTTP_CODE HandleRequest(AtlServerRequest *pRequestInfo, IServiceProvider *pProvider)
    {
        HTTP_CODE hcErr = m_MgrObj.Initialize(pProvider);
        if(hcErr != HTTP_SUCCESS)
            return hcErr;

        __if_exists(_Authority)
        {
            // Make sure caller is authorized on this system
            hcErr = HTTP_FAIL;
            ATLTRY(hcErr = _Authority.IsAuthorized(pRequestInfo, ATL_DEFAULT_AUTHGRP))
        }
        if(hcErr == HTTP_SUCCESS)
        {
            hcErr = __super::HandleRequest(pRequestInfo, pProvider);
        }
        return hcErr;
    }
private:
    CStencilCacheMgrObject m_MgrObj;
};
#endif //_ATL_STENCILCACHE_NOWEBSERVICE
#ifndef _ATL_STENCILCACHE_NOUI
typedef HRESULT(CStencilCacheMgrObject::*PFNGETDATA)(__int64 *pdwSize);

struct CCache_data
{
    PFNGETDATA m_pfn;
    char m_sz[128];
};

#define INVALID_DATA_PTR ((DWORD_PTR) -1)
#define INVALID_COMMAND_ID -1
#define MAX_COMMAND_ID 64
#define ATL_STENCILCACHECMD_CLEARALLSTATS		0
#define ATL_STENCILCACHECMD_REMOVESTENCIL		1
#define ATL_STENCILCACHECMD_REMOVEALLSTENCILS	2
#define ATL_STENCILCACHECMD_SETDEFLIFESPAN		3

[request_handler(name=ID_STENCILCACHEMGR_SRFHANDLER_NAME)]
class CStencilMgr
{
public:
    CStencilMgr()
    {
        m_pData = (CCache_data*)INVALID_DATA_PTR;
        m_nColor = ATL_COLOR_TR1;
    }

    HTTP_CODE ValidateAndExchange() throw()
    {
        _ATLTRY
        {
            HTTP_CODE hcErr = m_MgrObj.Initialize(m_spServiceProvider);
            if(hcErr != HTTP_SUCCESS)
                return hcErr;

            __if_exists(_Authority)
            {
                // Make sure caller is authorized on this system
                hcErr = HTTP_FAIL;
                ATLTRY(hcErr = _Authority.IsAuthorized(m_pRequestInfo, ATL_DEFAULT_AUTHGRP))
                if(hcErr != HTTP_SUCCESS)
                    return hcErr;
            }
            m_HttpResponse.SetContentType("text/html");

            // check to see if we have a "Handler" form variable
            CString strHandler, strOptParam;
            int nCmdToExec;

            if(m_HttpRequest.GetMethod() == CHttpRequest::HTTP_METHOD_POST)
            {
                DWORD dwErr = m_HttpRequest.FormVars.Exchange("Method", &strHandler);
                if(dwErr == VALIDATION_S_OK)
                {
                    if(strHandler == _T("ExecuteCommand"))
                    {
                        // get the value of the command parameter so we can execute it
                        dwErr = m_HttpRequest.FormVars.Validate("command", &nCmdToExec, 0, MAX_COMMAND_ID);
                        if(dwErr == VALIDATION_S_OK)
                        {
                            // get the optional parameter if it's there.
                            m_HttpRequest.FormVars.Validate("DynValue", &strOptParam, 0, MAX_COMMAND_ID);
                            hcErr = ExecCommand(nCmdToExec, strOptParam);
                            return hcErr;
                        }
                    }
                }
            }
            hcErr = LoadStencilResource(m_hInstHandler, IDR_STENCILMGR_SRF);
            return hcErr;
        }
        _ATLCATCHALL()
        {
            return HTTP_FAIL;
        }
    }

    HTTP_CODE ExecCommand(int nCmdToExec, CString& strOptParam)
    {
        switch(nCmdToExec)
        {
        case ATL_STENCILCACHECMD_CLEARALLSTATS:
            m_MgrObj.ClearStats();
            break;

        case ATL_STENCILCACHECMD_REMOVESTENCIL:
            m_MgrObj.RemoveStencilByName(strOptParam.AllocSysString());
            break;

        case ATL_STENCILCACHECMD_REMOVEALLSTENCILS:
            m_MgrObj.RemoveAllStencils();
            break;

        case ATL_STENCILCACHECMD_SETDEFLIFESPAN:
            TCHAR *pStop = NULL;
            m_MgrObj.SetDefaultLifespan(_tcstoul(strOptParam, &pStop, 10));
            break;
        };

        return _AtlRedirectToPage(
                   m_spServerContext,
                   m_HttpRequest,
                   m_HttpResponse,
                   "?Handler=" ID_STENCILCACHEMGR_SRFHANDLER_NAME
               );

    }

    [tag_name("GetNextStencilCacheStats")]
    HTTP_CODE GetNextStencilCacheStats()
    {
        if(m_pData == (CCache_data*)INVALID_DATA_PTR)
        {
            m_pData = GetCacheData();
            return HTTP_SUCCESS;
        }
        m_pData++;

        if(m_pData->m_pfn != NULL)
            return HTTP_SUCCESS;

        m_pData = (CCache_data*)INVALID_DATA_PTR;
        return HTTP_S_FALSE;

    }

    [tag_name("GetCacheValue")]
    HTTP_CODE GetCacheValue()
    {
        ATLENSURE(m_pData);
        ATLENSURE(m_pData != (CCache_data*)INVALID_DATA_PTR);
        m_HttpResponse << m_pData->m_sz;
        return HTTP_SUCCESS;
    }

    [tag_name("GetCacheQuantity")]
    HTTP_CODE GetCacheQuantity()
    {
        ATLENSURE(m_pData);
        ATLENSURE(m_pData != (CCache_data*)INVALID_DATA_PTR);
        __int64 dwValue = 0;
        PFNGETDATA pfn = m_pData->m_pfn;
        ATLENSURE(pfn);
        CStencilCacheMgrObject *pMgr = &m_MgrObj;
        (pMgr->*pfn)(&dwValue);

        m_HttpResponse << dwValue;
        return HTTP_SUCCESS;
    }

    [tag_name("GetTRColor")]
    HTTP_CODE GetTRColor()
    {
        m_nColor = (m_nColor == ATL_COLOR_TR1) ? ATL_COLOR_TR2 : ATL_COLOR_TR1;
        TCHAR cr[8];
        if(RGBToHtml(m_nColor, cr, sizeof(cr)))
            m_HttpResponse << cr;

        return HTTP_SUCCESS;
    }

    [tag_name("GetBodyColor")]
    HTTP_CODE GetBodyColor()
    {
        TCHAR cr[8];
        if(RGBToHtml(ATL_COLOR_BODYBG, cr, sizeof(cr)))
            m_HttpResponse << cr;
        return HTTP_SUCCESS;
    }
private:
    static CCache_data* GetCacheData()
    {
        static CCache_data cache_data[] =
        {
            {(PFNGETDATA)&CStencilCacheMgrObject::GetCurrentEntryCount, "Current Cache Entry Count(stencils)"},
            {(PFNGETDATA)&CStencilCacheMgrObject::GetHitCount, "Cache Hit Count(stencils)"},
            {(PFNGETDATA)&CStencilCacheMgrObject::GetMissCount, "Cache Miss Count(stencils)"},
            {(PFNGETDATA)&CStencilCacheMgrObject::GetCurrentAllocSize, "Cache memory allocation(bytes)"},
            {(PFNGETDATA)&CStencilCacheMgrObject::GetMaxAllocSize, "Cache maximum allocation size(bytes)"},
            {(PFNGETDATA)&CStencilCacheMgrObject::GetMaxEntryCount, "Cache maximum entry count(stencils)"},
            {(PFNGETDATA)&CStencilCacheMgrObject::GetDefaultLifespan, "Default stencil lifespan(ms)"},
            {NULL, NULL}
        };
        return cache_data;
    }

    CStencilCacheMgrObject m_MgrObj;
    CCache_data *m_pData;
    long m_nColor;
};
//__declspec(selectany) CComObjectGlobal<CStencilCacheManager> CStencilMgr::m_cachemgr;
#endif // _ATL_STENCILCACHE_NOUI
#endif // _ATL_STENCILCACHE_MANAGEMENT

//////////////////////////////////////////////////////////////////////
// DLL cache management
#ifdef _ATL_DLLCACHE_MANAGEMENT


#ifndef _ATL_DLLCACHE_NOWEBSERVICE
[export]
#endif
struct _DLL_CACHE_ENTRY
{
    DWORD hInstDll;
    DWORD dwRefs;
    BSTR szDllName;
};


class CDllMgrObject
{
public:
    HRESULT GetEntries(DWORD dwCount, _DLL_CACHE_ENTRY *pEntries, DWORD *pdwCopied)
    {
        ATLASSUME(m_spDllCache);
        HRESULT hr = E_FAIL;
        DLL_CACHE_ENTRY *pe = NULL;

        if(!m_spDllCache)
            return E_UNEXPECTED;

        if(dwCount != 0 && pEntries == NULL)
            return E_UNEXPECTED; // asking for entries but no place to put them

        if(!pdwCopied)
            return E_POINTER;
        *pdwCopied = 0;

        if(dwCount)
        {
            pe = new DLL_CACHE_ENTRY[dwCount];
            if(!pe)
                return E_OUTOFMEMORY;
        }

        hr = m_spDllCache->GetEntries(dwCount, pe, pdwCopied);
        if(hr == S_OK && dwCount != 0 && pEntries != NULL)
        {
            // SysAllocString our path strings
            for(DWORD i = 0; i < *pdwCopied; i++)
            {
                pEntries[i].hInstDll = (DWORD)(DWORD_PTR)pe[i].hInstDll;
                pEntries[i].dwRefs = pe[i].dwRefs;
                pEntries[i].szDllName = ::SysAllocString(CA2W(pe[i].szDllName));
            }
        }

        delete [] pe;
        return hr;
    }


    HRESULT GetEntryCount(DWORD *pdwCount)
    {
        ATLASSUME(m_spDllCache);
        if(!m_spDllCache)
            return E_UNEXPECTED;

        return m_spDllCache->GetEntries(0, NULL, pdwCount);
    }

    HTTP_CODE Initialize(IServiceProvider *pProvider)
    {
        ATLASSERT(pProvider); // should never be NULL
        if(!pProvider)
            return HTTP_ERROR(500, ISE_SUBERR_UNEXPECTED);

        if(m_spDllCache)
            return HTTP_SUCCESS; // already initialized

        pProvider->QueryService(__uuidof(IDllCache), &m_spDllCache);
        return m_spDllCache ? HTTP_SUCCESS : HTTP_ERROR(500, ISE_SUBERR_UNEXPECTED);
    }

private:
    CComPtr<IDllCache> m_spDllCache;

}; // CDllMgrObject


#ifndef _ATL_DLLCACHE_NOWEBSERVICE
// _DLL_CACHE_ENTRY is our own version of DLL_CACHE_ENTRY(atlcache.h) that
// uses a BSTR instead of a fixed length string for the szDllName for compatiblility
// with our SOAP implementation.
[ uuid("A0C00AF8-CEA5-46b9-97ED-FDEE55B583EF"), object ]
__interface IDllCacheMgr
{
    [id(0)] STDMETHOD(GetEntries)([in] DWORD dwCount, [out] _DLL_CACHE_ENTRY *pEntries, [out, retval] DWORD *pdwCopied);
    [id(1)] STDMETHOD(GetEntryCount)([out, retval] DWORD *pdwCount);

};


#pragma warning(push)
#pragma warning(disable:4199)
[
    soap_handler(
        name=		ID_DLLCACHEMGR_WEBSERVICE_NAME,
        namespace=	ID_DLLCACHEMGR_WEBSERVICE_URL,
        protocol=	"soap"
    ),
    request_handler(
        name=		ID_DLLCACHEMGR_WEBSERVICE_NAME,
        sdl=		ID_DLLCACHEMGR_WEBSERVICE_WSDL
    )
]
class CDllCacheManager :
    public IDllCacheMgr
{
#pragma warning(pop)
public:
    [soap_method]
    HRESULT GetEntries(DWORD dwCount, _DLL_CACHE_ENTRY *pEntries, DWORD *pdwCopied)
    {
        return m_MgrObj.GetEntries(dwCount, pEntries, pdwCopied);
    }

    [soap_method]
    STDMETHOD(GetEntryCount)(DWORD *pdwCount)
    {
        return m_MgrObj.GetEntries(0, NULL, pdwCount);
    }

    HTTP_CODE HandleRequest(AtlServerRequest *pRequestInfo, IServiceProvider *pProvider)
    {
        HTTP_CODE hcErr = m_MgrObj.Initialize(pProvider);
        if(hcErr != HTTP_SUCCESS)
            return hcErr;

        __if_exists(_Authority)
        {
            // Make sure caller is authorized on this system
            hcErr = HTTP_FAIL;
            ATLTRY(hcErr = _Authority.IsAuthorized(pRequestInfo, ATL_DEFAULT_AUTHGRP))
        }
        if(hcErr == HTTP_SUCCESS)
        {
            hcErr = __super::HandleRequest(pRequestInfo, pProvider);
        }
        return hcErr;
    }

protected:
    CDllMgrObject m_MgrObj;
};
#endif _ATL_DLLCACHE_NOWEBSERVICE

#ifndef _ATL_DLLCACHE_NOUI
#define INVALID_INDEX -1

[
    request_handler(name=ID_DLLCACHEMGR_SRFHANDLER_NAME)
]
class CDllCacheMgr
{
public:
    CDllCacheMgr() : m_nColor(ATL_COLOR_TR1),
        m_nEnumCount(INVALID_INDEX),
        m_nEnumIndex(INVALID_INDEX),
        m_pEntries(NULL)
    {

    }

    [tag_name("GetTRColor")]
    HTTP_CODE GetTRColor()
    {
        m_nColor = (m_nColor == ATL_COLOR_TR1) ? ATL_COLOR_TR2 : ATL_COLOR_TR1;
        TCHAR cr[8];
        if(RGBToHtml(m_nColor, cr, sizeof(cr)))
            m_HttpResponse << cr;

        return HTTP_SUCCESS;
    }

    [tag_name("GetBodyColor")]
    HTTP_CODE GetBodyColor()
    {
        TCHAR cr[8];
        if(RGBToHtml(ATL_COLOR_BODYBG, cr, sizeof(cr)))
            m_HttpResponse << cr;
        return HTTP_SUCCESS;
    }


    [tag_name("GetNumEntries")]
    HTTP_CODE GetNumEntries()
    {
        DWORD dwEntries = 0;
        m_MgrObj.GetEntryCount(&dwEntries);
        m_HttpResponse << dwEntries;
        return HTTP_SUCCESS;
    }


    [tag_name("EnumEntries")]
    HTTP_CODE EnumEntries()
    {
        // we lock the cache while we enum entries so no entries
        // will be removed during the enumeration request.
        if(m_nEnumIndex == INVALID_INDEX)
        {
            // set up for the iteration
            m_MgrObj.GetEntryCount((DWORD*)&m_nEnumCount);
            if(!m_nEnumCount)
                return HTTP_S_FALSE; // nothing to enum

            m_pEntries = new _DLL_CACHE_ENTRY[m_nEnumCount];
            if(!m_pEntries)
                return HTTP_ERROR(500, ISE_SUBERR_OUTOFMEM);

            DWORD dwFetched = INVALID_INDEX;

            if(S_OK != m_MgrObj.GetEntries(m_nEnumCount, m_pEntries, &dwFetched))
                return HTTP_ERROR(500, ISE_SUBERR_UNEXPECTED);

            m_nEnumIndex = 0;
            return HTTP_SUCCESS;
        }

        m_nEnumIndex++;
        if(m_nEnumIndex < m_nEnumCount)
            return HTTP_SUCCESS; // continue iterating

        else
        {
            // done, clean up
            for(int i = 0; i < m_nEnumCount; i++)
            {
                ::SysFreeString(m_pEntries[i].szDllName);
            }
            delete [] m_pEntries;
            m_pEntries = NULL;
            m_nEnumCount = INVALID_INDEX;
            m_nEnumIndex = INVALID_INDEX;
            return HTTP_S_FALSE; // terminate iterations.
        }
    }

    [tag_name("GetDllName")]
    HTTP_CODE GetDllName()
    {
        m_HttpResponse << m_pEntries[m_nEnumIndex].szDllName;
        return HTTP_SUCCESS;
    }

    [tag_name("GetDllReferences")]
    HTTP_CODE GetDllReferences()
    {
        m_HttpResponse << m_pEntries[m_nEnumIndex].dwRefs;
        return HTTP_SUCCESS;
    }

    HTTP_CODE ValidateAndExchange()
    {

        HTTP_CODE hcErr = m_MgrObj.Initialize(m_spServiceProvider);
        if(hcErr != HTTP_SUCCESS)
            return hcErr;

        __if_exists(_Authority)
        {
            // Make sure caller is authorized on this system
            hcErr = HTTP_FAIL;
            ATLTRY(hcErr = _Authority.IsAuthorized(m_pRequestInfo, ATL_DEFAULT_AUTHGRP))
            if(hcErr != HTTP_SUCCESS)
                return hcErr;
        }
        hcErr = LoadStencilResource(m_hInstHandler, IDR_DLLMGR_SRF);
        m_HttpResponse.SetContentType("text/html");
        return hcErr;

    }

    CDllMgrObject m_MgrObj;
    long m_nColor;
    int m_nEnumCount;
    int m_nEnumIndex;
    _DLL_CACHE_ENTRY *m_pEntries;

};

#endif // _ATL_DLLCACHE_NOUI
#endif // _ATL_DLLCACHE_MANAGEMENT

}; // ATL

#pragma pack(pop)
#pragma warning(pop)
#endif // __ATLEXTMGMT_H__
