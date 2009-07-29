// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

#ifndef __ATLSOAP_H__
#define __ATLSOAP_H__

#pragma once

#if (defined(_WINSOCKAPI_) && !defined(_WINSOCK2API_))
	#error require winsock2.h -- include <winsock2.h> before you include <windows.h>
#endif

#if ((_WIN32_WINNT < 0x0400) && (_WIN32_WINDOWS <= 0x0400))
	#error require _WIN32_WINNT >= 0x0400 or _WIN32_WINDOWS > 0x0400
#endif

#ifndef ATLSOAP_TRACE
	#ifdef _ATLSOAP_TRACE_XML
		#define ATLSOAP_TRACE(__data, __len) AtlSoapTraceXML(__data, __len)
	#else
		#define ATLSOAP_TRACE(__data, __len) __noop
	#endif
#endif // ATLSOAP_TRACE

// override this macro to ATL_BASE64_FLAG_NOCRLF if you do 
// not want Base64-encoded binary data to contain CRLFs
#ifndef ATLSOAP_BASE64_FLAGS
	#define ATLSOAP_BASE64_FLAGS ATL_BASE64_FLAG_NONE
#endif // ATLSOAP_BASE64_FLAGS

[ emitidl(restricted) ];
#include <winsock2.h>
#include <atlstr.h>
#include <atlcoll.h>
#include <atlbase.h>
#include <msxml2.h>
#include <atlenc.h>
#include <fcntl.h>
#include <float.h>
#include <math.h>
#include <limits>
#include <atlisapi.h>
#include <atlstencil.h>
#include <atlhttp.h>
#include <atlhttp.inl>

#pragma warning(push)
#pragma warning(disable: 4625) // copy constructor could not be generated because a base class copy constructor is inaccessible
#pragma warning(disable: 4626) // assignment operator could not be generated because a base class assignment operator is inaccessible
#pragma warning(disable: 4061) // enumerate 'enum value' in switch of enum 'enum type' is not explicitly handled by a case label

#ifndef _CPPUNWIND
#pragma warning(disable: 4702) // unreachable code
#endif // _CPPUNWIND

#ifndef ATLSOAP_NOWININET
	#include <wininet.h>
	#ifndef ATLSOAPINET_CLIENT
		#define ATLSOAPINET_CLIENT _T("VCSoapClient")
	#endif
#endif

#ifndef _ATL_NO_DEFAULT_LIBS
#pragma comment(lib, "msxml2.lib")
	#ifndef ATLSOAP_NOWININET
		#pragma comment(lib, "wininet.lib")
	#endif
#endif

#define _ATLSOAP_MAKEWIDESTR( str ) L ## str
#define ATLSOAP_MAKEWIDESTR( str ) _ATLSOAP_MAKEWIDESTR( str )


#pragma pack(push,_ATL_PACKING)
namespace ATL
{

ATL_NOINLINE inline void AtlSoapTraceXML(LPBYTE pdwData, DWORD dwLen)
{
	HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdOut != INVALID_HANDLE_VALUE)
	{
		DWORD dwWritten;
		WriteFile(hStdOut, 
			"\n-----------------------------------------------------------------\n",
			sizeof("\n-----------------------------------------------------------------\n")-1, 
			&dwWritten, NULL);

		WriteFile(hStdOut, pdwData, dwLen, &dwWritten, NULL);

		WriteFile(hStdOut,
			"\n-----------------------------------------------------------------\n",
			sizeof("\n-----------------------------------------------------------------\n")-1, 
			&dwWritten, NULL);
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// IStreamImpl - stub IStream implementation class
//
////////////////////////////////////////////////////////////////////////////////

class IStreamImpl : public IStream
{
public:

	HRESULT __stdcall Read(void * /*pDest*/, ULONG /*nMaxLen*/, ULONG * /*pnRead*/)
	{
		return E_NOTIMPL;
	}

	HRESULT __stdcall Write(const void * /*pv*/, ULONG /*cb*/, ULONG * /*pcbWritten*/)
	{
		return E_NOTIMPL;
	}

	HRESULT __stdcall Seek(LARGE_INTEGER /*dlibMove*/, DWORD /*dwOrigin*/, 
		ULARGE_INTEGER * /*pLibNewPosition*/)
	{
		return E_NOTIMPL;
	}

	HRESULT __stdcall SetSize(ULARGE_INTEGER /*libNewSize*/)
	{
		return E_NOTIMPL;
	}

	HRESULT __stdcall CopyTo(IStream * /*pStream*/, ULARGE_INTEGER /*cb*/, 
		ULARGE_INTEGER * /*pcbRead*/, ULARGE_INTEGER * /*pcbWritten*/)
	{
		return E_NOTIMPL;
	}

	HRESULT __stdcall Commit(DWORD /*grfCommitFlags*/)
	{
		return E_NOTIMPL;
	}

	HRESULT __stdcall Revert()
	{
		return E_NOTIMPL;
	}

	HRESULT __stdcall LockRegion(ULARGE_INTEGER /*libOffset*/, ULARGE_INTEGER /*cb*/, DWORD /*dwLockType*/)
	{
		return E_NOTIMPL;
	}

	HRESULT __stdcall UnlockRegion(ULARGE_INTEGER /*libOffset*/, ULARGE_INTEGER /*cb*/, DWORD /*dwLockType*/)
	{
		return E_NOTIMPL;
	}

	HRESULT __stdcall Stat(STATSTG * /*pstatstg*/, DWORD /*grfStatFlag*/)
	{
		return E_NOTIMPL;
	}

	HRESULT __stdcall Clone(IStream ** /*ppstm*/)
	{
		return E_NOTIMPL;
	}
}; // class IStreamImpl

////////////////////////////////////////////////////////////////////////////////
//
// CStreamOnServerContext
//
////////////////////////////////////////////////////////////////////////////////

class CStreamOnServerContext : public IStreamImpl
{
public:

	HRESULT __stdcall QueryInterface(REFIID riid, void **ppv)
	{
		if (ppv == NULL)
		{
			return E_POINTER;
		}

		*ppv = NULL;

		if (InlineIsEqualGUID(riid, IID_IUnknown) ||
			InlineIsEqualGUID(riid, IID_IStream) ||
			InlineIsEqualGUID(riid, IID_ISequentialStream))
		{
			*ppv = static_cast<IStream *>(this);
			return S_OK;
		}

		return E_NOINTERFACE;
	}

	ULONG __stdcall AddRef()
	{
		return 1;
	}

	ULONG __stdcall Release()
	{
		return 1;
	}

private:

	IHttpServerContext * m_pServerContext;
	DWORD m_dwBytesRead;

public:

	CStreamOnServerContext(IHttpServerContext *pServerContext = NULL)
		: m_pServerContext(pServerContext), m_dwBytesRead(0)
	{
	}

	void SetServerContext(IHttpServerContext *pServerContext)
	{
		ATLASSUME( m_pServerContext == NULL );

		m_pServerContext = pServerContext;
	}

	HRESULT __stdcall Read(void *pDest, ULONG nMaxLen, ULONG *pnRead)
	{
		ATLENSURE( pDest != NULL );
		ATLASSUME( m_pServerContext != NULL );

		DWORD dwToRead = __min(m_pServerContext->GetTotalBytes()-m_dwBytesRead, nMaxLen);
		if (ReadClientData(m_pServerContext, (LPSTR) pDest, &dwToRead, m_dwBytesRead) != FALSE)
		{
			m_dwBytesRead+= dwToRead;

			if (pnRead != NULL)
			{
				*pnRead = dwToRead;
			}

			return S_OK;
		}

		ATLTRACE( _T("ATLSOAP: CStreamOnServerContext::Read -- ReadClientData failed.\r\n") );

		return E_FAIL;
	}
}; // class CStreamOnServerContext

////////////////////////////////////////////////////////////////////////////////
//
// CReadStreamOnSocket
//
////////////////////////////////////////////////////////////////////////////////

template <typename TSocketClass>
class CReadStreamOnSocket : public IStreamImpl
{
public:

	HRESULT __stdcall QueryInterface(REFIID riid, void **ppv)
	{
		if (ppv == NULL)
		{
			return E_POINTER;
		}

		*ppv = NULL;

		if (InlineIsEqualGUID(riid, IID_IUnknown) ||
			InlineIsEqualGUID(riid, IID_IStream) ||
			InlineIsEqualGUID(riid, IID_ISequentialStream))
		{
			*ppv = static_cast<IStream *>(this);
			return S_OK;
		}

		return E_NOINTERFACE;
	}

	ULONG __stdcall AddRef()
	{
		return 1;
	}

	ULONG __stdcall Release()
	{
		return 1;
	}

private:

	CAtlHttpClientT<TSocketClass> * m_pSocket;
	LPCSTR m_szBuffer;
	LPCSTR m_szCurr;
	long m_nBodyLen;

public:

	CReadStreamOnSocket()
		: m_pSocket(NULL), m_szBuffer(NULL), m_szCurr(NULL), m_nBodyLen(0)
	{
	}

	BOOL Init(CAtlHttpClientT<TSocketClass> *pSocket)
	{
		ATLENSURE( pSocket != NULL );

		m_pSocket = pSocket;
		m_szBuffer = (LPCSTR) pSocket->GetBody();

		ATLSOAP_TRACE( (LPBYTE) pSocket->GetBody(), pSocket->GetBodyLength() );

		if (m_szBuffer != NULL)
		{
			m_szCurr = m_szBuffer;
			m_nBodyLen = pSocket->GetBodyLength();
			if (m_nBodyLen != 0)
			{
				return TRUE;
			}
		}

		ATLTRACE( _T("ATLSOAP: CReadStreamOnSocket::Init failed.\r\n") );

		return FALSE;
	}

	HRESULT __stdcall Read(void *pDest, ULONG nMaxLen, ULONG *pnRead)
	{
		ATLASSERT( pDest != NULL );
		ATLASSUME( m_pSocket != NULL );
		ATLASSUME( m_szBuffer != NULL );

		if (pnRead != NULL)
		{
			*pnRead = 0;
		}

		long nRead = (int) (m_szCurr-m_szBuffer);
		if (nRead < m_nBodyLen)
		{
			long nLength = __min((int)(m_nBodyLen-nRead), (LONG) nMaxLen);
			Checked::memcpy_s(pDest, nMaxLen, m_szCurr, nLength);
			m_szCurr+= nLength;

			if (pnRead != NULL)
			{
				*pnRead = (ULONG) nLength;
			}
		}

		return S_OK;
	}
}; // class CReadStreamOnSocket

////////////////////////////////////////////////////////////////////////////////
//
// CWriteStreamOnCString
//
////////////////////////////////////////////////////////////////////////////////

class CWriteStreamOnCString : public IWriteStream
{

public:
	CStringA m_str;

	virtual ~CWriteStreamOnCString()
	{
	}

	HRESULT WriteStream(LPCSTR szOut, int nLen, LPDWORD pdwWritten)
	{
		ATLENSURE_RETURN( szOut != NULL );

		if (nLen < 0)
		{
			nLen = (int) strlen(szOut);
		}

		_ATLTRY
		{
			m_str.Append(szOut, nLen);
		}
		_ATLCATCHALL()
		{
			return E_OUTOFMEMORY;
		}

		if (pdwWritten != NULL)
		{
			*pdwWritten = (DWORD) nLen;
		}

		return S_OK;
	}

	HRESULT FlushStream()
	{
		return S_OK;
	}

	void Cleanup()
	{
		m_str.Empty();
	}
}; // class CWriteStreamOnCString

////////////////////////////////////////////////////////////////////////////////
//
// Namespaces
//
////////////////////////////////////////////////////////////////////////////////

#define SOAPENV_NAMESPACEA "http://schemas.xmlsoap.org/soap/envelope/"
#define SOAPENV_NAMESPACEW ATLSOAP_MAKEWIDESTR( SOAPENV_NAMESPACEA )

#define SOAPENC_NAMESPACEA "http://schemas.xmlsoap.org/soap/encoding/"
#define SOAPENC_NAMESPACEW ATLSOAP_MAKEWIDESTR( SOAPENC_NAMESPACEA )

#define XSI_NAMESPACEA  "http://www.w3.org/2001/XMLSchema-instance"
#define XSI_NAMESPACEW  ATLSOAP_MAKEWIDESTR( XSI_NAMESPACEA )

#define XSD_NAMESPACEA  "http://www.w3.org/2001/XMLSchema"
#define XSD_NAMESPACEW  ATLSOAP_MAKEWIDESTR( XSD_NAMESPACEA )

#ifndef ATLSOAP_GENERIC_NAMESPACE
#define ATLSOAP_GENERIC_NAMESPACE L"http://www.tempuri.org"
#endif

////////////////////////////////////////////////////////////////////////////////
//
// Helpers
//
////////////////////////////////////////////////////////////////////////////////

inline HRESULT GetAttribute(
	__in ISAXAttributes *pAttributes, 
	__in_ecount(cchName) const wchar_t *wszAttrName, __in int cchName, 
	__out_ecount_part(*pcchValue, *pcchValue) const wchar_t **pwszValue, __inout int *pcchValue,
	__in_ecount_opt(cchNamespace) wchar_t *wszNamespace = NULL, __in int cchNamespace = 0)
{
	if (!pAttributes || !wszAttrName || !pwszValue || !pcchValue)
	{
		return E_INVALIDARG;
	}

	*pwszValue = NULL;
	*pcchValue = 0;
	if (!wszNamespace)
	{
		return (pAttributes->getValueFromQName(wszAttrName, cchName, pwszValue, pcchValue) == S_OK ? S_OK : E_FAIL);
	}
	return (pAttributes->getValueFromName(wszNamespace, cchNamespace, 
		wszAttrName, cchName, pwszValue, pcchValue) == S_OK ? S_OK : E_FAIL);
}

inline HRESULT GetAttribute(
	__in ISAXAttributes *pAttributes, 
	__in_ecount(cchName) const wchar_t *wszAttrName, __in int cchName, 
	__inout CStringW &strValue,
	__in_ecount_opt(cchNamespace) wchar_t *wszNamespace = NULL, __in int cchNamespace = 0)
{
	const wchar_t *wszValue = NULL;
	int cchValue = 0;

	if (!pAttributes || !wszAttrName)
	{
		return E_INVALIDARG;
	}

	HRESULT hr;
	if (!wszNamespace)
	{
		hr = (pAttributes->getValueFromQName(wszAttrName, cchName, &wszValue, &cchValue) == S_OK ? S_OK : E_FAIL);
	}
	else
	{
		hr = (pAttributes->getValueFromName(wszNamespace, cchNamespace, 
			wszAttrName, cchName, &wszValue, &cchValue) == S_OK ? S_OK : E_FAIL);
	}

	if (hr == S_OK)
	{
		_ATLTRY
		{
			strValue.SetString(wszValue, cchValue);
		}
		_ATLCATCHALL()
		{
			ATLTRACE( _T("ATLSOAP: GetAttribute -- out of memory.\r\n") );

			hr = E_OUTOFMEMORY;
		}
	}

	return hr;
}

inline const wchar_t *SkipWhitespace(const wchar_t *wsz)
{
	while (*wsz && iswspace(*wsz))
		++wsz;
	return wsz;
}

} // namespace ATL
#pragma pack(pop)

////////////////////////////////////////////////////////////////////////////////
//
// BLOB data type - use this struct when you want to send BLOB data
//   the attribute provider and proxy generator will only properly special
//   case blob data when using this struct.
//
////////////////////////////////////////////////////////////////////////////////

[ export ]
typedef struct _tagATLSOAP_BLOB
{
	unsigned long size;
	unsigned char *data;
} ATLSOAP_BLOB;

#ifndef _ATL_SOAP_NO_PARAMETER_VALIDATIONS
#define _ATL_VALIDATE_PARAMETER_END(p)\
	do			\
	{			\
		if(*(p) !='\0') \
			return E_FAIL; \
	} while(0)
#else
#define _ATL_VALIDATE_PARAMETER_END(p) 
#endif

// All non-integral types have specializations which
// will be called. The following function will be called
// only for integral types

#pragma push_macro("max")
#pragma push_macro("min")
#undef max
#undef min
template <typename T>
inline HRESULT AtlGetSAXValue(T * pVal , const wchar_t * wsz , int cch )
{
	__int64 nVal = *pVal;
	if (FAILED(AtlGetSAXValue(&nVal, wsz, cch)))
		return E_FAIL;

#ifndef _ATL_SOAP_NO_PARAMETER_VALIDATIONS
	if(nVal < std::numeric_limits<T>::min() || nVal > std::numeric_limits<T>::max())
		return E_FAIL;
#endif

	*pVal = T(nVal);
	return S_OK;


}
#pragma pop_macro("max")
#pragma pop_macro("min")

////////////////////////////////////////////////////////////////////////////////
//
// AtlGetXMLValue (for IXMLDOMDocument) - get the real type from the XML data
//
///////////////////////////////////////////////////////////////////////////////

//
// generic IXMLDOMNode template function
// delegates to AtlGetSAXValue
//
template <typename T>
inline HRESULT AtlGetXMLValue(IXMLDOMNode *pParam, T *pVal)
{
	CComBSTR bstrVal;
	HRESULT hr = AtlGetXMLValue(pParam, &bstrVal);
	if (SUCCEEDED(hr))
	{
		hr = AtlGetSAXValue(pVal, bstrVal, bstrVal.Length());
	}

	return hr;
}

// specialization for BSTR
template <>
inline HRESULT AtlGetXMLValue<BSTR>(IXMLDOMNode *pParam, BSTR *pbstrVal)
{
	if (pParam == NULL)
	{
		return E_INVALIDARG;
	}
	if (pbstrVal == NULL)
	{
		return E_POINTER;
	}

	CComPtr<IXMLDOMNode> spChild;
	if (pParam->get_firstChild(&spChild) == S_OK)
	{
		CComPtr<IXMLDOMNode> spXmlChild;
		if (spChild->get_firstChild(&spXmlChild) == S_OK)
		{
			return (pParam->get_xml(pbstrVal) == S_OK ? S_OK : E_FAIL);
		}
	}

	return (pParam->get_text(pbstrVal) == S_OK) ? S_OK : E_FAIL;
}

////////////////////////////////////////////////////////////////////////////////
//
// AtlGetSAXValue - (for SAX or generic) get the real type from the XML data
//
////////////////////////////////////////////////////////////////////////////////

template <>
inline HRESULT AtlGetSAXValue<bool>(bool *pVal, __in_z const wchar_t *wsz, int cch)
{
	ATLENSURE( wsz != NULL );

	if (!pVal)
	{
		return E_POINTER;
	}

	*pVal = false;

	HRESULT hr = E_FAIL;
	switch (wsz[0])
	{
		case L'1':
		{
			if (cch==1)
			{
				*pVal = true;
				hr = S_OK;
			}
			break;
		}
		case L'0':
		{
			if (cch==1)
			{
				*pVal = false;
				hr = S_OK;
			}
			break;
		}
		case L't':
		{
			if (cch==sizeof("true")-1 && !wcsncmp(wsz, L"true", cch))
			{
				*pVal = true;
				hr = S_OK;
			}
			break;
		}
		case L'f':
		{
			if (cch==sizeof("false")-1 && !wcsncmp(wsz, L"false", cch))
			{
				*pVal = false;
				hr = S_OK;
			}
			break;
		}
	}

	return hr;
}

template <>
inline HRESULT AtlGetSAXValue<__int64>(__int64 *pVal, __in_z const wchar_t *wsz, int cch)
{
	ATLENSURE_RETURN( wsz != NULL );

	if (!pVal)
	{
		return E_POINTER;
	}

	_ATLTRY
	{
		CFixedStringT<CStringW, 1024> wstr(wsz, cch);
		const wchar_t *pStart = ATL::SkipWhitespace(static_cast<LPCWSTR>(wstr));
		const wchar_t *pEnd;

		__int64 i = 0;
		errno_t errnoValue = AtlStrToNum(&i, pStart, const_cast<wchar_t **>(&pEnd), 10);
		if (errnoValue == ERANGE)
		{
			return E_FAIL;//overflow or underflow case 
		}
		pEnd = ATL::SkipWhitespace(pEnd);
		_ATL_VALIDATE_PARAMETER_END(pEnd);
		*pVal = i;
	}
	_ATLCATCHALL()
	{
		return E_OUTOFMEMORY;
	}

	return S_OK;
}

template <>
inline HRESULT AtlGetSAXValue<unsigned __int64>(unsigned __int64 *pVal, __in_z const wchar_t *wsz, int cch)
{
	ATLENSURE_RETURN( wsz != NULL );

	if (!pVal)
	{
		return E_POINTER;
	}

	_ATLTRY
	{
		CFixedStringT<CStringW, 1024> wstr(wsz, cch);
		const wchar_t *pStart = ATL::SkipWhitespace(static_cast<LPCWSTR>(wstr));
		const wchar_t *pEnd;

		unsigned __int64 i = 0;
		errno_t errnoValue = AtlStrToNum(&i, pStart, const_cast<wchar_t **>(&pEnd), 10);
		if (errnoValue == ERANGE)
		{
			 return E_FAIL;//overflow or underflow case 
		}
		pEnd = ATL::SkipWhitespace(pEnd);
		_ATL_VALIDATE_PARAMETER_END(pEnd);
		*pVal = i;
	}
	_ATLCATCHALL()
	{
		return E_OUTOFMEMORY;
	}

	return S_OK;
}
template <>
inline HRESULT AtlGetSAXValue<double>(double *pVal, __in_z const wchar_t *wsz, int cch)
{
	ATLENSURE_RETURN( wsz != NULL );

	if (!pVal)
	{
		return E_POINTER;
	}

	if ((cch == 3) && (wsz[0]==L'I') && (!wcsncmp(wsz, L"INF", cch)))
	{
		*(((int *) pVal)+0) = 0x0000000;
		*(((int *) pVal)+1) = 0x7FF00000;
	}
	else if ((cch == 3) && (wsz[0]==L'N') && (!wcsncmp(wsz, L"NaN", cch)))
	{
		*(((int *) pVal)+0) = 0x0000000;
		*(((int *) pVal)+1) = 0xFFF80000;
	}
	else if ((cch == 4) && (wsz[1]==L'I') && (!wcsncmp(wsz, L"-INF", cch)))
	{
		*(((int *) pVal)+0) = 0x0000000;
		*(((int *) pVal)+1) = 0xFFF00000;
	}
	else
	{
		errno_t errnoValue = 0;

		_ATLTRY
		{
			CFixedStringT<CStringW, 1024> wstr(wsz, cch);
			const wchar_t *pStart = ATL::SkipWhitespace(static_cast<LPCWSTR>(wstr));
			const wchar_t *pEnd;
			double d = 0.0;
			errnoValue = AtlStrToNum(&d, pStart, const_cast<wchar_t **>(&pEnd));
			pEnd = ATL::SkipWhitespace(pEnd);
			_ATL_VALIDATE_PARAMETER_END(pEnd);
			*pVal = d;
		}
		_ATLCATCHALL()
		{
			return E_OUTOFMEMORY;
		}

		if ((*pVal == -HUGE_VAL) || (*pVal == HUGE_VAL) || (errnoValue == ERANGE))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

template <>
inline HRESULT AtlGetSAXValue<float>(float *pVal, __in_z const wchar_t *wsz, int cch)
{
	ATLASSERT( wsz != NULL );

	if (!pVal)
	{
		return E_POINTER;
	}

	double d = *pVal;
	if (SUCCEEDED(AtlGetSAXValue(&d, wsz, cch)))
	{
#ifdef _ATL_SOAP_PARAMETER_VALIDATIONS
		if(d > FLT_MAX || d < -FLT_MAX)
			return E_FAIL;
#endif
		*pVal = (float) d;
		return S_OK;
	}

	return E_FAIL;
}

template <>
inline HRESULT AtlGetSAXValue<BSTR>(BSTR *pVal, __in_z const wchar_t *wsz, int cch)
{
	ATLASSERT( wsz != NULL );

	if (pVal == NULL)
	{
		return E_POINTER;
	}

	*pVal = SysAllocStringLen(wsz, cch);

	return ((*pVal != NULL) ? S_OK : E_OUTOFMEMORY);
}

inline HRESULT AtlGetSAXBlobValue(
	ATLSOAP_BLOB *pVal, 
	const wchar_t *wsz, 
	int cch, 
	IAtlMemMgr *pMemMgr, 
	bool bHex = false)
{
	ATLENSURE_RETURN( wsz != NULL );
	ATLENSURE_RETURN( pMemMgr != NULL );

	if (pVal == NULL)
	{
		return E_POINTER;
	}

	if (pVal->data != NULL)
	{
		return E_INVALIDARG;
	}

	pVal->data = NULL;
	pVal->size = 0;

	int nLength = AtlUnicodeToUTF8(wsz, cch, NULL, 0);

	if (nLength != 0)
	{
		char * pSrc = (char *) pMemMgr->Allocate(nLength);
		if (pSrc != NULL)
		{
			nLength = AtlUnicodeToUTF8(wsz, cch, pSrc, nLength);
			if (nLength != 0)
			{
				pVal->data = (unsigned char *) pMemMgr->Allocate(nLength);
				if (pVal->data != NULL)
				{
					BOOL bRet;
					int nDataLength = nLength;
					if (!bHex)
					{
						bRet = Base64Decode(pSrc, nLength, pVal->data, &nDataLength);
					}
					else
					{
						bRet = AtlHexDecode(pSrc, nLength, pVal->data, &nDataLength);
					}
					if (bRet)
					{
						pVal->size = nDataLength;
					}
				}
			}

			pMemMgr->Free(pSrc);
		}
	}

	if (pVal->size == 0)
	{
		if (pVal->data != NULL)
		{
			pMemMgr->Free(pVal->data);
			pVal->data = NULL;
		}
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
//
// AtlGenXMLValue template and specializations
//
////////////////////////////////////////////////////////////////////////////////

template <typename T>
inline HRESULT AtlGenXMLValue(__in IWriteStream *pStream, __in T *pVal)
{
	if ((pStream == NULL) || (pVal == NULL))
	{
		return E_INVALIDARG;
	}

	//
	// delegate to CWriteStreamHelper
	//
	CWriteStreamHelper s(pStream);

	return (s.Write(*pVal) == TRUE ? S_OK : E_FAIL);
}

#ifdef _NATIVE_WCHAR_T_DEFINED
template <>
inline HRESULT AtlGenXMLValue<wchar_t>(__in IWriteStream *pStream, __in wchar_t *pVal)
{
	return AtlGenXMLValue(pStream, (unsigned short *)pVal);
}
#endif

template <>
inline HRESULT AtlGenXMLValue<wchar_t *>(__in IWriteStream *pStream, __deref_inout_z wchar_t **pVal)
{
	if ((pStream == NULL) || (*pVal == NULL))
	{
		return E_INVALIDARG;
	}

	wchar_t *wszWrite = *pVal;
	int nSrcLen = (int)wcslen(*pVal);
	int nCnt = EscapeXML(*pVal, nSrcLen, NULL, 0);
	if (nCnt > nSrcLen)
	{
		nCnt++;
		wszWrite = (wchar_t *)calloc((nCnt),sizeof(wchar_t));
		if (wszWrite == NULL)
		{
			return E_OUTOFMEMORY;
		}

		nCnt = EscapeXML(*pVal, nSrcLen, wszWrite, nCnt);
		if (nCnt == 0)
		{
			free(wszWrite);
			return E_FAIL;
		}
		wszWrite[nCnt] = L'\0';
		nSrcLen = nCnt;
	}

	nCnt = AtlUnicodeToUTF8(wszWrite, nSrcLen, NULL, 0);
	HRESULT hr = E_FAIL;
	if ((nCnt == 0) || (nCnt == nSrcLen))
	{
		CWriteStreamHelper s(pStream);

		hr = (s.Write(wszWrite) == TRUE ? S_OK : E_FAIL);
	}
	else
	{
		nCnt++;
		CHeapPtr<char> szWrite;
		szWrite.AllocateBytes((size_t)(nCnt));
		if (szWrite != NULL)
		{
			nCnt = AtlUnicodeToUTF8(wszWrite, nSrcLen, szWrite, nCnt);
			if (nCnt != 0)
			{
				hr = pStream->WriteStream(szWrite, nCnt, NULL);
			}
		}
		else
		{
			ATLTRACE( _T("ATLSOAP: AtlGenXMLValue<wchar_t *> -- out of memory.\r\n") );

			hr = E_OUTOFMEMORY;
		}
	}

	if (wszWrite != *pVal)
	{
		free(wszWrite);
	}

	return hr;
}

template <>
inline HRESULT AtlGenXMLValue<double>(IWriteStream *pStream, double *pVal)
{
	if ((pStream == NULL) || (pVal == NULL))
	{
		return E_INVALIDARG;
	}

	HRESULT hr;
	switch (_fpclass(*pVal))
	{
		case _FPCLASS_SNAN: 
		case _FPCLASS_QNAN:
		{
			hr = pStream->WriteStream("NaN", 3, NULL);
			break;
		}
		case _FPCLASS_NINF:
		{
			hr = pStream->WriteStream("-INF", 4, NULL);
			break;
		}
		case _FPCLASS_PINF:
		{
			hr = pStream->WriteStream("INF", 3, NULL);
			break;
		}
		case _FPCLASS_NZ:
		{
			hr = pStream->WriteStream("-0", 2, NULL);
			break;
		}
		default:
		{
            /***
            * 2 = sign + decimal point
            * ndec = decimal digits
            * 5 = exponent letter (e or E), exponent sign, three digits exponent
            * 1 = extra space for rounding
            * 1 = string terminator '\0'
            ***/
            const int ndec = 512;
            CHAR szBuf[ndec+9];
            szBuf[0] = '\0';
            Checked::gcvt_s(szBuf, _countof(szBuf), *pVal, ndec);
            size_t nLen = strlen(szBuf);
            if (nLen && szBuf[nLen-1] == '.')
            {
                szBuf[--nLen] = '\0';
            }

            hr = pStream->WriteStream(szBuf, (int)nLen, NULL);
            break;
        }
    }

    return hr;
}

template <>
inline HRESULT AtlGenXMLValue<float>(IWriteStream *pStream, float *pVal)
{
	if ((pStream == NULL) || (pVal == NULL))
	{
		return E_INVALIDARG;
	}

	double d = *pVal;

	return AtlGenXMLValue(pStream, &d);
}

template <>
inline HRESULT AtlGenXMLValue<bool>(IWriteStream *pStream, bool *pVal)
{
	if ((pStream == NULL) || (pVal == NULL))
	{
		return E_INVALIDARG;
	}

	if (*pVal == true)
	{
		return pStream->WriteStream("true", sizeof("true")-1, NULL);
	}

	return pStream->WriteStream("false", sizeof("false")-1, NULL);
}

inline HRESULT AtlGenXMLBlobValue(
	IWriteStream *pStream, 
	ATLSOAP_BLOB *pVal, 
	IAtlMemMgr *pMemMgr, 
	bool bHex = false)
{
	if ((pStream == NULL) || (pVal == NULL) || (pMemMgr == NULL))
	{
		return E_INVALIDARG;
	}

	HRESULT hr = E_FAIL;
	int nLength;
	if (!bHex)
	{
		nLength = Base64EncodeGetRequiredLength(pVal->size, ATLSOAP_BASE64_FLAGS);
	}
	else
	{
		nLength = AtlHexEncodeGetRequiredLength(pVal->size);
	}

	char *pEnc = (char *) pMemMgr->Allocate(nLength);
	if (pEnc != NULL)
	{
		BOOL bRet;
		if (!bHex)
		{
			bRet = Base64Encode(pVal->data, pVal->size, pEnc, &nLength, ATLSOAP_BASE64_FLAGS);
		}
		else
		{
			bRet = AtlHexEncode(pVal->data, pVal->size, pEnc, &nLength);
		}
		if (bRet)
		{
			hr = pStream->WriteStream(pEnc, nLength, NULL);
		}

		pMemMgr->Free(pEnc);
	}

	return hr;
}

template <typename T>
inline HRESULT AtlCleanupValue(T * /*pVal*/)
{
	return S_OK;
}

inline HRESULT AtlCleanupBlobValue(ATLSOAP_BLOB *pVal, IAtlMemMgr *pMemMgr)
{
	if ((pVal == NULL) || (pMemMgr == NULL))
	{
		return E_INVALIDARG;
	}

	if (pVal->data != NULL)
	{
		pMemMgr->Free(pVal->data);
		pVal->data = NULL;
		pVal->size = 0;
	}

	return S_OK;
}

template <>
inline HRESULT AtlCleanupValue<ATLSOAP_BLOB>(ATLSOAP_BLOB *pVal)
{
	ATLTRACE( _T("Warning: AtlCleanupValue<ATLSOAP_BLOB> was called -- assuming CRT allocator.\r\n") );

	if (pVal == NULL)
	{
		return E_INVALIDARG;
	}

	if (pVal->data != NULL)
	{
		free(pVal->data);
		pVal->data = NULL;
		pVal->size = 0;
	}

	return S_OK;
}

template <>
inline HRESULT AtlCleanupValue<BSTR>(BSTR *pVal)
{
	if (pVal == NULL)
	{
		// should never happen
		ATLASSERT( FALSE );
		return E_INVALIDARG;
	}

	if ((*pVal) != NULL)
	{
		// null strings are okay
		SysFreeString(*pVal);
		*pVal = NULL;
	}

	return S_OK;
}

template <typename T>
inline HRESULT AtlCleanupValueEx(T *pVal, IAtlMemMgr *pMemMgr)
{
	pMemMgr;

	return AtlCleanupValue(pVal);
}

template <>
inline HRESULT AtlCleanupValueEx<ATLSOAP_BLOB>(ATLSOAP_BLOB *pVal, IAtlMemMgr *pMemMgr)
{
	return AtlCleanupBlobValue(pVal, pMemMgr);
}

// single dimensional arrays
template <typename T>
inline HRESULT AtlCleanupArray(T *pArray, int nCnt)
{
	if (pArray == NULL)
	{
		return E_INVALIDARG;
	}

	for (int i=0; i<nCnt; i++)
	{
		AtlCleanupValue(&pArray[i]);
	}

	return S_OK;
}


template <typename T>
inline HRESULT AtlCleanupArrayEx(T *pArray, int nCnt, IAtlMemMgr *pMemMgr)
{
	if (pArray == NULL)
	{
		return E_INVALIDARG;
	}

	for (int i=0; i<nCnt; i++)
	{
		AtlCleanupValueEx(&pArray[i], pMemMgr);
	}

	return S_OK;
}


// multi-dimensional arrays
template <typename T>
inline HRESULT AtlCleanupArrayMD(T *pArray, const int *pDims)
{
	if ((pArray == NULL) || (pDims == NULL))
	{
		return E_INVALIDARG;
	}

	// calculate size
	int nCnt = 1;
	for (int i=1; i<=pDims[0]; i++)
	{
		nCnt*= pDims[i];
	}

	return AtlCleanupArray(pArray, nCnt);
}

template <typename T>
inline HRESULT AtlCleanupArrayMDEx(T *pArray, const int *pDims, IAtlMemMgr *pMemMgr)
{
	if ((pArray == NULL) || (pDims == NULL))
	{
		return E_INVALIDARG;
	}

	// calculate size
	int nCnt = 1;
	for (int i=1; i<=pDims[0]; i++)
	{
		nCnt*= pDims[i];
	}

	return AtlCleanupArrayEx(pArray, nCnt, pMemMgr);
}


#pragma pack(push,_ATL_PACKING)
namespace ATL
{

////////////////////////////////////////////////////////////////////////////////
//
// CSAXSoapErrorHandler
//
////////////////////////////////////////////////////////////////////////////////

class CSAXSoapErrorHandler : public ISAXErrorHandler
{
private:

	CFixedStringT<CStringW, 256> m_strParseError;

public:
	virtual ~CSAXSoapErrorHandler()
	{
	}

	HRESULT __stdcall QueryInterface(REFIID riid, void **ppv)
	{
		if (!ppv)
		{
			return E_POINTER;
		}

		if (InlineIsEqualGUID(riid, __uuidof(ISAXErrorHandler)) ||
			InlineIsEqualGUID(riid, __uuidof(IUnknown)))
		{
			*ppv = static_cast<ISAXErrorHandler*>(this);
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	ULONG __stdcall AddRef()
	{
		return 1;
	}

	ULONG __stdcall Release()
	{
		return 1;
	}

	const CStringW& GetParseError()
	{
		return m_strParseError;
	}

	HRESULT __stdcall error( 
		ISAXLocator *pLocator,
		const wchar_t *wszErrorMessage,
		HRESULT hrErrorCode)
	{
		(pLocator);
		(wszErrorMessage);
		(hrErrorCode);

		ATLTRACE( _T("ATLSOAP: parse error: %ws\r\n"), wszErrorMessage );

		_ATLTRY
		{
			m_strParseError = wszErrorMessage;
		}
		_ATLCATCHALL()
		{
			return E_FAIL;
		}

		return hrErrorCode;
	}

	HRESULT __stdcall fatalError(
		ISAXLocator  *pLocator,
		const wchar_t *wszErrorMessage,
		HRESULT hrErrorCode)
	{
		(pLocator);
		(wszErrorMessage);
		(hrErrorCode);

		ATLTRACE( _T("ATLSOAP: fatal parse error: %ws\r\n"), wszErrorMessage );

		_ATLTRY
		{
			m_strParseError = wszErrorMessage;
		}
		_ATLCATCHALL()
		{
			return E_FAIL;
		}

		return hrErrorCode;
	}

	HRESULT __stdcall ignorableWarning(
		ISAXLocator  *pLocator,
		const wchar_t *wszErrorMessage,
		HRESULT hrErrorCode)
	{
		(pLocator);
		(wszErrorMessage);
		(hrErrorCode);

		ATLTRACE( _T("ATLSOAP: ignorable warning: %ws\r\n"), wszErrorMessage );

		return hrErrorCode;
	}
};

////////////////////////////////////////////////////////////////////////////////
//
// ISAXContentHandlerImpl
//
////////////////////////////////////////////////////////////////////////////////

class ISAXContentHandlerImpl : 
	public ISAXContentHandler
{
public:

	//
	// ISAXContentHandler interface
	//

	HRESULT __stdcall putDocumentLocator(ISAXLocator  * /*pLocator*/)
	{
		return S_OK;
	}

	HRESULT __stdcall startDocument()
	{
		return S_OK;
	}

	HRESULT __stdcall endDocument()
	{
		return S_OK;
	}

	HRESULT __stdcall startPrefixMapping(
		 const wchar_t  * /*wszPrefix*/,
		 int /*cchPrefix*/,
		 const wchar_t  * /*wszUri*/,
		 int /*cchUri*/)
	{
		return S_OK;
	}

	HRESULT __stdcall endPrefixMapping( 
		 const wchar_t  * /*wszPrefix*/,
		 int /*cchPrefix*/)
	{
		return S_OK;
	}

	HRESULT __stdcall startElement( 
		 const wchar_t  * /*wszNamespaceUri*/,
		 int /*cchNamespaceUri*/,
		 const wchar_t  * /*wszLocalName*/,
		 int /*cchLocalName*/,
		 const wchar_t  * /*wszQName*/,
		 int /*cchQName*/,
		 ISAXAttributes  * /*pAttributes*/)
	{
		return S_OK;
	}

	HRESULT __stdcall endElement( 
		 const wchar_t  * /*wszNamespaceUri*/,
		 int /*cchNamespaceUri*/,
		 const wchar_t  * /*wszLocalName*/,
		 int /*cchLocalName*/,
		 const wchar_t  * /*wszQName*/,
		 int /*cchQName*/)
	{
		return S_OK;
	}

	HRESULT __stdcall characters( 
		 const wchar_t  * /*wszChars*/,
		 int /*cchChars*/)
	{
		return S_OK;
	}

	HRESULT __stdcall ignorableWhitespace( 
		 const wchar_t  * /*wszChars*/,
		 int /*cchChars*/)
	{
		return S_OK;
	}

	HRESULT __stdcall processingInstruction( 
		 const wchar_t  * /*wszTarget*/,
		 int /*cchTarget*/,
		 const wchar_t  * /*wszData*/,
		 int /*cchData*/)
	{
		return S_OK;
	}

	HRESULT __stdcall skippedEntity( 
		 const wchar_t  * /*wszName*/,
		 int /*cchName*/)
	{
		return S_OK;
	}
}; // class ISAXContentHandlerImpl

////////////////////////////////////////////////////////////////////////////////
//
// SAX skip element handler utility class
// (skip an element and all its child elements)
//
////////////////////////////////////////////////////////////////////////////////

class CSkipHandler : public ISAXContentHandlerImpl
{
public:
	virtual ~CSkipHandler()
	{
	}
	
	HRESULT __stdcall QueryInterface(REFIID riid, void **ppv)
	{
		if (ppv == NULL)
		{
			return E_POINTER;
		}

		*ppv = NULL;

		if (InlineIsEqualGUID(riid, IID_IUnknown) ||
			InlineIsEqualGUID(riid, IID_ISAXContentHandler))
		{
			*ppv = static_cast<ISAXContentHandler *>(this);
			return S_OK;
		}

		return E_NOINTERFACE;
	}

	ULONG __stdcall AddRef()
	{
		return 1;
	}

	ULONG __stdcall Release()
	{
		return 1;
	}

private:

	DWORD m_dwReset;
	CComPtr<ISAXXMLReader> m_spReader;
	CComPtr<ISAXContentHandler> m_spParent;

	DWORD DisableReset(DWORD dwCnt = 1)
	{
		m_dwReset += dwCnt;

		return m_dwReset;
	}

	DWORD EnableReset()
	{
		if (m_dwReset > 0)
		{
			--m_dwReset;
		}

		return m_dwReset;
	}

public:

	CSkipHandler(ISAXContentHandler *pParent = NULL, ISAXXMLReader *pReader = NULL)
		: m_spParent(pParent), m_spReader(pReader), m_dwReset(1)
	{
	}

	void SetParent(ISAXContentHandler *pParent)
	{
		m_spParent = pParent;
	}
	void DetachParent()
	{		
		m_spParent.Detach();
	}

	void SetReader(ISAXXMLReader *pReader)
	{
		m_spReader = pReader;
	}

	HRESULT __stdcall startElement( 
		 const wchar_t  * /*wszNamespaceUri*/,
		 int /*cchNamespaceUri*/,
		 const wchar_t  * /*wszLocalName*/,
		 int /*cchLocalName*/,
		 const wchar_t  * /*wszQName*/,
		 int /*cchQName*/,
		 ISAXAttributes  * /*pAttributes*/)
	{
		DisableReset();
		return S_OK;
	}

	HRESULT __stdcall endElement( 
		 const wchar_t  * /*wszNamespaceUri*/,
		 int /*cchNamespaceUri*/,
		 const wchar_t  * /*wszLocalName*/,
		 int /*cchLocalName*/,
		 const wchar_t  * /*wszQName*/,
		 int /*cchQName*/)
	{
		if (EnableReset() == 0)
		{
			m_spReader->putContentHandler(m_spParent);
		}

		return S_OK;
	}
}; // class CSkipHandler


////////////////////////////////////////////////////////////////////////////////
//
// SAX string builder class
//
////////////////////////////////////////////////////////////////////////////////

class CSAXStringBuilder : public ISAXContentHandlerImpl
{
public:

	HRESULT __stdcall QueryInterface(REFIID riid, void **ppv)
	{
		if (ppv == NULL)
		{
			return E_POINTER;
		}

		*ppv = NULL;

		if (InlineIsEqualGUID(riid, IID_IUnknown) ||
			InlineIsEqualGUID(riid, IID_ISAXContentHandler))
		{
			*ppv = static_cast<ISAXContentHandler *>(this);
			return S_OK;
		}

		return E_NOINTERFACE;
	}

	ULONG __stdcall AddRef()
	{
		return 1;
	}

	ULONG __stdcall Release()
	{
		return 1;
	}

private:

	ISAXContentHandler * m_pParent;
	ISAXXMLReader * m_pReader;
	DWORD m_dwReset;
	CFixedStringT<CStringW, 64> m_str;

	DWORD DisableReset(DWORD dwReset = 1)
	{
		m_dwReset+= dwReset;

		return m_dwReset;
	}

	DWORD EnableReset()
	{
		if (m_dwReset > 0)
		{
			--m_dwReset;
		}

		return m_dwReset;
	}

public:

	CSAXStringBuilder(ISAXXMLReader *pReader = NULL, ISAXContentHandler *pParent = NULL)
		:m_pReader(pReader), m_pParent(pParent), m_dwReset(0)
	{
	}
	
	virtual ~CSAXStringBuilder()
	{
	}

	void SetReader(ISAXXMLReader *pReader)
	{
		m_pReader = pReader;
	}

	void SetParent(ISAXContentHandler *pParent)
	{
		m_pParent = pParent;
	}

	const CStringW& GetString()
	{
		return m_str;
	}

	void Clear()
	{
		m_str.Empty();
		m_dwReset = 0;
	}

	HRESULT __stdcall startElement( 
		 const wchar_t  * /*wszNamespaceUri*/,
		 int /*cchNamespaceUri*/,
		 const wchar_t  * /*wszLocalName*/,
		 int /*cchLocalName*/,
		 const wchar_t  *wszQName,
		 int cchQName,
		 ISAXAttributes  *pAttributes)
	{
		if (m_dwReset == 0)
		{
			// if there is unescaped, nested XML, must disable 
			// an additional time for the first element
			DisableReset();
		}
		DisableReset();

		int nAttrs = 0;
		HRESULT hr = pAttributes->getLength(&nAttrs);

		_ATLTRY
		{
			if (SUCCEEDED(hr))
			{
				m_str.Append(L"<", 1);
				m_str.Append(wszQName, cchQName);

				const wchar_t *wszAttrNamespaceUri = NULL;
				const wchar_t *wszAttrLocalName = NULL;
				const wchar_t *wszAttrQName = NULL;
				const wchar_t *wszAttrValue = NULL;
				int cchAttrUri = 0;
				int cchAttrLocalName = 0;
				int cchAttrQName = 0;
				int cchAttrValue = 0;

				for (int i=0; i<nAttrs; i++)
				{
					hr = pAttributes->getName(i, &wszAttrNamespaceUri, &cchAttrUri, 
						&wszAttrLocalName, &cchAttrLocalName, &wszAttrQName, &cchAttrQName);

					if (FAILED(hr))
					{
						ATLTRACE( _T("ATLSOAP: CSAXStringBuilder::startElement -- MSXML error.\r\n") );

						break;
					}

					m_str.Append(L" ", 1);
					m_str.Append(wszAttrQName, cchAttrQName);

					hr = pAttributes->getValue(i, &wszAttrValue, &cchAttrValue);

					if (FAILED(hr))
					{
						ATLTRACE( _T("ATLSOAP: CSAXStringBuilder::startElement -- MSXML error.\r\n") );

						break;
					}

					m_str.Append(L"=\"", sizeof("=\"")-1);
					if (cchAttrValue != 0)
					{
						m_str.Append(wszAttrValue, cchAttrValue);
					}
					m_str.Append(L"\"", 1);
				}

				if (SUCCEEDED(hr))
				{
					m_str.Append(L">", 1);
				}
			}
		}
		_ATLCATCHALL()
		{
			ATLTRACE( _T("ATLSOAP: CSAXStringBuilder::startElement -- out of memory.\r\n") );

			hr = E_OUTOFMEMORY;
		}

		return hr;
	}

	HRESULT __stdcall endElement( 
		 const wchar_t  * wszNamespaceUri,
		 int cchNamespaceUri,
		 const wchar_t  * wszLocalName,
		 int cchLocalName,
		 const wchar_t  *wszQName,
		 int cchQName)
	{
		HRESULT hr = S_OK;
		_ATLTRY
		{
			if (EnableReset() == 0)
			{
				hr = m_pParent->characters((LPCWSTR) m_str, m_str.GetLength());
				if (SUCCEEDED(hr))
				{
					hr = m_pParent->endElement(wszNamespaceUri, cchNamespaceUri,
							wszLocalName, cchLocalName, wszQName, cchQName);
				}

				m_pReader->putContentHandler(m_pParent);
			}

			if (m_dwReset > 0)
			{
				m_str.Append(L"</", 2);
				m_str.Append(wszQName, cchQName);
				m_str.Append(L">", 1);
			}
		}
		_ATLCATCHALL()
		{
			ATLTRACE( _T("ATLSOAP: CSAXStringBuilder::endElement -- out of memory.\r\n") );

			hr = E_OUTOFMEMORY;
		}

		return hr;
	}

	HRESULT __stdcall characters(
		 const wchar_t  *wszChars,
		 int cchChars)
	{
		_ATLTRY
		{
			m_str.Append(wszChars, cchChars);
		}
		_ATLCATCHALL()
		{
			ATLTRACE( _T("ATLSOAP: CSAXStringBuilder::characters -- out of memory.\r\n") );

			return E_OUTOFMEMORY;
		}

		return S_OK;
	}

	HRESULT __stdcall ignorableWhitespace( 
		 const wchar_t  *wszChars,
		 int cchChars)
	{
		_ATLTRY
		{
			m_str.Append(wszChars, cchChars);
		}
		_ATLCATCHALL()
		{
			ATLTRACE( _T("ATLSOAP: CSAXStringBuilder::ignorableWhitespace -- out of memory.\r\n") );

			return E_OUTOFMEMORY;
		}

		return S_OK;
	}
}; // class CSAXStringBuilder

} // namespace ATL
#pragma pack(pop)

////////////////////////////////////////////////////////////////////////////////
//
// SOAP data structure definitions
//
////////////////////////////////////////////////////////////////////////////////

//
// *****************************  WARNING  *****************************
// THESE STRUCTURES ARE INTERNAL ONLY, FOR USE WITH THE ATL SERVER SOAP 
// ATTRIBUTES. USERS SHOULD NOT USE THESE TYPES DIRECTLY. ABSOLUTELY NO 
// GUARANTEES ARE MADE ABOUT BACKWARD COMPATIBILITY FOR DIRECT USE OF 
// THESE TYPES.
//

////////////////////////////////////////////////////////////////////////////////
//
// BEGIN PRIVATE DEFINITIONS
//
////////////////////////////////////////////////////////////////////////////////

inline HRESULT AtlSoapGetArraySize(ISAXAttributes *pAttributes, size_t *pnSize, 
	const wchar_t **pwszTypeStart = NULL, const wchar_t **pwszTypeEnd = NULL)
{
	if (pnSize == NULL)
	{
		return E_POINTER;
	}

	if (pAttributes == NULL)
	{
		return E_INVALIDARG;
	}

	*pnSize = 0;

	HRESULT hr = S_OK;

	_ATLTRY
	{
		const wchar_t *wszTmp;
		int cch;

		hr = GetAttribute(pAttributes, L"arrayType", sizeof("arrayType")-1, 
			&wszTmp, &cch, SOAPENC_NAMESPACEW, sizeof(SOAPENC_NAMESPACEA)-1);

		if ((SUCCEEDED(hr)) && (wszTmp != NULL))
		{
			hr = E_FAIL;

			CFixedStringT<CStringW, 1024> wstrArrayType(wszTmp, cch);
			const wchar_t *wsz = static_cast<LPCWSTR>(wstrArrayType);

			const wchar_t *wszTypeStart = NULL;
			const wchar_t *wszTypeEnd = NULL;

			// skip spaces
			while (iswspace(*wsz) != 0)
			{
				wsz++;
			}

			// no need to walk the string if the caller is not interested
			if ((pwszTypeStart != NULL) && (pwszTypeEnd != NULL))
			{
				wszTypeStart = wsz;
				wszTypeEnd = wcschr(wszTypeStart, L':');
				if (wszTypeEnd != NULL)
				{
					wszTypeStart = wszTypeEnd+1;
				}
			}

			// SOAP Section 5 encodings are of the form:
			//   <soap_enc namespace>:arrayType="<type_qname>[dim1(,dim_i)*]
			//   for example: SOAP-ENC:arrayType="xsd:string[2,4]"

			wsz = wcschr(wsz, L'[');
			if (wsz != NULL)
			{
				wszTypeEnd = wsz-1;
				if (wsz[1] == ']')
				{
					return S_FALSE;
				}

				*pnSize = 1;

				// get the size of each dimension
				while (wsz != NULL)
				{
					wsz++;
					int nDim = _wtoi(wsz);
					if (nDim < 0)
					{
						hr = E_FAIL;
						break;
					}
					*pnSize *= (size_t) nDim;
					if (!nDim)
					{
						break;
					}

					wsz = wcschr(wsz, L',');
				}

				if ((pwszTypeStart != NULL) && (pwszTypeEnd != NULL))
				{
					*pwszTypeStart = wszTypeStart;
					*pwszTypeEnd = wszTypeEnd;
				}

				hr = S_OK;
			}
		}
		else
		{
			// not a section-5 encoding
			hr = S_FALSE;
		}
	}
	_ATLCATCHALL()
	{
		hr = E_OUTOFMEMORY;
	}

	return hr;
}

inline size_t AtlSoapGetArrayDims(const int *pDims)
{
	if (pDims == NULL)
	{
		return 0;
	}

	size_t nRet = 1;
	for (int i=1; i<=pDims[0]; i++)
	{
		nRet *= pDims[i];
	}

	return nRet;
}

enum SOAPFLAGS
{
	SOAPFLAG_NONE           = 0x00000000,
	SOAPFLAG_IN             = 0x00000001,
	SOAPFLAG_OUT            = 0x00000002,
	SOAPFLAG_RETVAL         = 0x00000004,
	SOAPFLAG_DYNARR         = 0x00000008,
	SOAPFLAG_FIXEDARR       = 0x00000010,
	SOAPFLAG_MUSTUNDERSTAND = 0x00000020,
	SOAPFLAG_UNKSIZE        = 0x00000040,
	SOAPFLAG_READYSTATE     = 0x00000080,
	SOAPFLAG_FIELD          = 0x00000100,
	SOAPFLAG_NOMARSHAL      = 0x00000200,
	SOAPFLAG_NULLABLE       = 0x00000400,
	SOAPFLAG_DOCUMENT       = 0x00000800,
	SOAPFLAG_RPC            = 0x00001000,
	SOAPFLAG_LITERAL        = 0x00002000,
	SOAPFLAG_ENCODED        = 0x00004000,
	SOAPFLAG_PID            = 0x00008000,
	SOAPFLAG_PAD            = 0x00010000,
	SOAPFLAG_CHAIN          = 0x00020000,
	SOAPFLAG_SIZEIS         = 0x00040000,
	SOAPFLAG_DYNARRWRAPPER  = 0x00080000
};

enum SOAPMAPTYPE
{
	SOAPMAP_ERR = 0,
	SOAPMAP_ENUM,
	SOAPMAP_FUNC,
	SOAPMAP_STRUCT,
	SOAPMAP_UNION,
	SOAPMAP_HEADER,
	SOAPMAP_PARAM
};

struct _soapmap;

struct _soapmapentry
{
	ULONG nHash;
	const char * szField;
	const WCHAR * wszField;
	int cchField;
	int nVal;
	DWORD dwFlags;

	size_t nOffset;
	const int * pDims;

	const _soapmap * pChain;

	int nSizeIs;

	ULONG nNamespaceHash;
	const char *szNamespace;
	const wchar_t *wszNamespace;
	int cchNamespace;
};

struct _soapmap
{
	ULONG nHash;
	const char * szName;
	const wchar_t * wszName;
	int cchName;
	int cchWName;
	SOAPMAPTYPE mapType;
	const _soapmapentry * pEntries;
	size_t nElementSize;
	size_t nElements;
	int nRetvalIndex;

	DWORD dwCallFlags;

	ULONG nNamespaceHash;
	const char *szNamespace;
	const wchar_t *wszNamespace;
	int cchNamespace;
};

enum SOAPTYPES
{
	SOAPTYPE_ERR = -2,
	SOAPTYPE_UNK = -1,
	SOAPTYPE_STRING = 0,
	SOAPTYPE_BOOLEAN,
	SOAPTYPE_FLOAT,
	SOAPTYPE_DOUBLE,
	SOAPTYPE_DECIMAL,
	SOAPTYPE_DURATION,
	SOAPTYPE_HEXBINARY,
	SOAPTYPE_BASE64BINARY,
	SOAPTYPE_ANYURI,
	SOAPTYPE_ID,
	SOAPTYPE_IDREF,
	SOAPTYPE_ENTITY,
	SOAPTYPE_NOTATION,
	SOAPTYPE_QNAME,
	SOAPTYPE_NORMALIZEDSTRING,
	SOAPTYPE_TOKEN,
	SOAPTYPE_LANGUAGE,
	SOAPTYPE_IDREFS,
	SOAPTYPE_ENTITIES,
	SOAPTYPE_NMTOKEN,
	SOAPTYPE_NMTOKENS,
	SOAPTYPE_NAME,
	SOAPTYPE_NCNAME,
	SOAPTYPE_INTEGER,
	SOAPTYPE_NONPOSITIVEINTEGER,
	SOAPTYPE_NEGATIVEINTEGER,
	SOAPTYPE_LONG,
	SOAPTYPE_INT,
	SOAPTYPE_SHORT,
	SOAPTYPE_BYTE,
	SOAPTYPE_NONNEGATIVEINTEGER,
	SOAPTYPE_UNSIGNEDLONG,
	SOAPTYPE_UNSIGNEDINT,
	SOAPTYPE_UNSIGNEDSHORT,
	SOAPTYPE_UNSIGNEDBYTE,
	SOAPTYPE_POSITIVEINTEGER,
	SOAPTYPE_DATETIME,
	SOAPTYPE_TIME,
	SOAPTYPE_DATE,
	SOAPTYPE_GMONTH,
	SOAPTYPE_GYEARMONTH,
	SOAPTYPE_GYEAR,
	SOAPTYPE_GMONTHDAY,
	SOAPTYPE_GDAY,

	SOAPTYPE_USERBASE = 0x00001000
};

inline ULONG AtlSoapHashStr(const char * sz)
{
	ULONG nHash = 0;
	while (*sz != 0)
	{
		nHash = (nHash<<5)+nHash+(*sz);
		sz++;
	}

	return nHash;
}

inline ULONG AtlSoapHashStr(const wchar_t * sz)
{
	ULONG nHash = 0;
	while (*sz != 0)
	{
		nHash = (nHash<<5)+nHash+(*sz);
		sz++;
	}

	return nHash;
}

inline ULONG AtlSoapHashStr(const char * sz, int cch)
{
	ULONG nHash = 0;
	for (int i=0; i<cch; i++)
	{
		nHash = (nHash<<5)+nHash+(*sz);
		sz++;
	}

	return nHash;
}

inline ULONG AtlSoapHashStr(const wchar_t * sz, int cch)
{
	ULONG nHash = 0;
	for (int i=0; i<cch; i++)
	{
		nHash = (nHash<<5)+nHash+(*sz);
		sz++;
	}

	return nHash;
}

inline size_t AtlSoapGetElementSize(SOAPTYPES type)
{
	size_t nRet;
	switch (type)
	{
		case SOAPTYPE_BOOLEAN:
			nRet = sizeof(bool);
			break;
		case SOAPTYPE_FLOAT:
			nRet = sizeof(float);
			break;
		case SOAPTYPE_DOUBLE: 
		case SOAPTYPE_DECIMAL:
			nRet = sizeof(double);
			break;
		case SOAPTYPE_HEXBINARY:
		case SOAPTYPE_BASE64BINARY:
			nRet = sizeof(ATLSOAP_BLOB);
			break;
		case SOAPTYPE_INTEGER: 
		case SOAPTYPE_NONPOSITIVEINTEGER:
		case SOAPTYPE_NEGATIVEINTEGER:
		case SOAPTYPE_LONG:
			nRet = sizeof(__int64);
			break;
		case SOAPTYPE_INT:
			nRet = sizeof(int);
			break;
		case SOAPTYPE_SHORT:
			nRet = sizeof(short);
			break;
		case SOAPTYPE_BYTE:
			nRet = sizeof(char);
			break;
		case SOAPTYPE_POSITIVEINTEGER:
		case SOAPTYPE_NONNEGATIVEINTEGER:
		case SOAPTYPE_UNSIGNEDLONG:
			nRet = sizeof(unsigned __int64);
			break;
		case SOAPTYPE_UNSIGNEDINT:
			nRet = sizeof(unsigned int);
			break;
		case SOAPTYPE_UNSIGNEDSHORT:
			nRet = sizeof(unsigned short);
			break;
		case SOAPTYPE_UNSIGNEDBYTE:
			nRet = sizeof(unsigned char);
			break;
		default:
			if ((type != SOAPTYPE_ERR) && (type != SOAPTYPE_UNK) && (type != SOAPTYPE_USERBASE))
			{
				// treat as string
				nRet = sizeof(BSTR);
			}
			else
			{
				ATLTRACE( _T("ATLSOAP: AtlSoapGetElementSize -- internal error.\r\n") );
				// should never get here
				ATLASSERT( FALSE );
				nRet = 0;
			}
			break;
	}

	return nRet;
}

inline HRESULT AtlSoapGetElementValue(const wchar_t *wsz, int cch, 
	void *pVal, SOAPTYPES type, IAtlMemMgr *pMemMgr)
{
	HRESULT hr = E_FAIL;

	switch (type)
	{
		case SOAPTYPE_BOOLEAN:
			hr = AtlGetSAXValue((bool *)pVal, wsz, cch);
			break;
		case SOAPTYPE_FLOAT:
			hr = AtlGetSAXValue((float *)pVal, wsz, cch);
			break;
		case SOAPTYPE_DOUBLE: 
		case SOAPTYPE_DECIMAL:
			hr = AtlGetSAXValue((double *)pVal, wsz, cch);
			break;
		case SOAPTYPE_HEXBINARY:
			hr = AtlGetSAXBlobValue((ATLSOAP_BLOB *)pVal, wsz, cch, pMemMgr, true);
			break;
		case SOAPTYPE_BASE64BINARY:
			hr = AtlGetSAXBlobValue((ATLSOAP_BLOB *)pVal, wsz, cch, pMemMgr, false);
			break;

		case SOAPTYPE_INTEGER: 
		case SOAPTYPE_NONPOSITIVEINTEGER:
		case SOAPTYPE_NEGATIVEINTEGER:
		case SOAPTYPE_LONG:
			hr = AtlGetSAXValue((__int64 *)pVal, wsz, cch);
			break;
		case SOAPTYPE_INT:
			hr = AtlGetSAXValue((int *)pVal, wsz, cch);
			break;
		case SOAPTYPE_SHORT:
			hr = AtlGetSAXValue((short *)pVal, wsz, cch);
			break;
		case SOAPTYPE_BYTE:
			hr = AtlGetSAXValue((char *)pVal, wsz, cch);
			break;
		case SOAPTYPE_POSITIVEINTEGER:
		case SOAPTYPE_NONNEGATIVEINTEGER:
		case SOAPTYPE_UNSIGNEDLONG:
			hr = AtlGetSAXValue((unsigned __int64 *)pVal, wsz, cch);
			break;
		case SOAPTYPE_UNSIGNEDINT:
			hr = AtlGetSAXValue((unsigned int *)pVal, wsz, cch);
			break;
		case SOAPTYPE_UNSIGNEDSHORT:
			hr = AtlGetSAXValue((unsigned short *)pVal, wsz, cch);
			break;
		case SOAPTYPE_UNSIGNEDBYTE:
			hr = AtlGetSAXValue((unsigned char *)pVal, wsz, cch);
			break;
		default:
			if ((type != SOAPTYPE_ERR) && (type != SOAPTYPE_UNK) && (type != SOAPTYPE_USERBASE))
			{
				hr = AtlGetSAXValue((BSTR *)pVal, wsz, cch);
			}
#ifdef _DEBUG
			else
			{
				ATLTRACE( _T("ATLSOAP: AtlSoapGetElementValue -- internal error.\r\n") );

				// should never get here
				ATLASSERT( FALSE );
			}
#endif
			break;
	}

	return hr;
}

inline HRESULT AtlSoapGenElementValue(void *pVal, IWriteStream *pStream, SOAPTYPES type, IAtlMemMgr *pMemMgr)
{
	HRESULT hr = E_FAIL;

	switch (type)
	{
		case SOAPTYPE_BOOLEAN:
			hr = AtlGenXMLValue(pStream, (bool *)pVal);
			break;
		case SOAPTYPE_FLOAT:
			hr = AtlGenXMLValue(pStream, (float *)pVal);
			break;
		case SOAPTYPE_DOUBLE: 
		case SOAPTYPE_DECIMAL:
			hr = AtlGenXMLValue(pStream, (double *)pVal);
			break;
		case SOAPTYPE_HEXBINARY:
			hr = AtlGenXMLBlobValue(pStream, (ATLSOAP_BLOB *)pVal, pMemMgr, true);
			break;
		case SOAPTYPE_BASE64BINARY:
			hr = AtlGenXMLBlobValue(pStream, (ATLSOAP_BLOB *)pVal, pMemMgr, false);
			break;

		case SOAPTYPE_INTEGER: 
		case SOAPTYPE_NONPOSITIVEINTEGER:
		case SOAPTYPE_NEGATIVEINTEGER:
		case SOAPTYPE_LONG:
			hr = AtlGenXMLValue(pStream, (__int64 *)pVal);
			break;
		case SOAPTYPE_INT:
			hr = AtlGenXMLValue(pStream, (int *)pVal);
			break;
		case SOAPTYPE_SHORT:
			hr = AtlGenXMLValue(pStream, (short *)pVal);
			break;
		case SOAPTYPE_BYTE:
			hr = AtlGenXMLValue(pStream, (char *)pVal);
			break;
		case SOAPTYPE_POSITIVEINTEGER:
		case SOAPTYPE_NONNEGATIVEINTEGER:
		case SOAPTYPE_UNSIGNEDLONG:
			hr = AtlGenXMLValue(pStream, (unsigned __int64 *)pVal);
			break;
		case SOAPTYPE_UNSIGNEDINT:
			hr = AtlGenXMLValue(pStream, (unsigned int *)pVal);
			break;
		case SOAPTYPE_UNSIGNEDSHORT:
			hr = AtlGenXMLValue(pStream, (unsigned short *)pVal);
			break;
		case SOAPTYPE_UNSIGNEDBYTE:
			hr = AtlGenXMLValue(pStream, (unsigned char *)pVal);
			break;
		default:
			if ((type != SOAPTYPE_ERR) && (type != SOAPTYPE_UNK) && (type != SOAPTYPE_USERBASE))
			{
				hr = AtlGenXMLValue(pStream, (BSTR *)pVal);
			}
#ifdef _DEBUG
			else
			{
				ATLTRACE( _T("ATLSOAP: AtlSoapGenElementValue -- internal error.\r\n" ) );

				// should never get here
				ATLASSERT( FALSE );
			}
#endif
			break;
	}
	return hr;
}

inline HRESULT AtlSoapCleanupElement(void *pVal, SOAPTYPES type, IAtlMemMgr *pMemMgr)
{
	HRESULT hr = S_OK;

	switch (type)
	{
		case SOAPTYPE_BOOLEAN:
		case SOAPTYPE_FLOAT:
		case SOAPTYPE_DOUBLE: 
		case SOAPTYPE_DECIMAL:
		case SOAPTYPE_INT:
		case SOAPTYPE_INTEGER: 
		case SOAPTYPE_NONPOSITIVEINTEGER:
		case SOAPTYPE_NEGATIVEINTEGER:
		case SOAPTYPE_LONG:
		case SOAPTYPE_SHORT:
		case SOAPTYPE_BYTE:
		case SOAPTYPE_POSITIVEINTEGER:
		case SOAPTYPE_NONNEGATIVEINTEGER:
		case SOAPTYPE_UNSIGNEDLONG:
		case SOAPTYPE_UNSIGNEDINT:
		case SOAPTYPE_UNSIGNEDSHORT:
		case SOAPTYPE_UNSIGNEDBYTE:
			break;

		case SOAPTYPE_HEXBINARY:
		case SOAPTYPE_BASE64BINARY:
			hr = AtlCleanupBlobValue((ATLSOAP_BLOB *)pVal, pMemMgr);
			break;

		default:
			if ((type != SOAPTYPE_ERR) && (type != SOAPTYPE_UNK) && (type != SOAPTYPE_USERBASE))
			{
				// treat as string
				hr = AtlCleanupValue((BSTR *)pVal);
			}
#ifdef _DEBUG
			else
			{
				ATLTRACE( _T("ATLSOAP: AtlSoapCleanupElement -- internal error.\r\n" ) );

				// should never get here
				ATLASSERT( FALSE );
			}
#endif
			break;
	}

	return hr;
}

////////////////////////////////////////////////////////////////////////////////
//
// END PRIVATE DEFINITIONS
//
////////////////////////////////////////////////////////////////////////////////

#define SOAP_ENVELOPEA "Envelope"
#define SOAP_ENVELOPEW ATLSOAP_MAKEWIDESTR( SOAP_ENVELOPEA )

#define SOAP_HEADERA   "Header"
#define SOAP_HEADERW   ATLSOAP_MAKEWIDESTR( SOAP_HEADERA )

#define SOAP_BODYA     "Body"
#define SOAP_BODYW     ATLSOAP_MAKEWIDESTR( SOAP_BODYA )


#pragma pack(push,_ATL_PACKING)
namespace ATL
{

//
// SOAP fault helpers
//

enum SOAP_ERROR_CODE
{ 
	SOAP_E_UNK=0,
	SOAP_E_VERSION_MISMATCH=100,
	SOAP_E_MUST_UNDERSTAND=200,
	SOAP_E_CLIENT=300,
	SOAP_E_SERVER=400
};

// forward declaration of CSoapFault
class CSoapFault;

class CSoapFaultParser : public ISAXContentHandlerImpl
{
private:

	CSoapFault *m_pFault;

	DWORD m_dwState;

	const static DWORD STATE_ERROR       = 0;
	const static DWORD STATE_ENVELOPE    = 1;
	const static DWORD STATE_BODY        = 2;
	const static DWORD STATE_START       = 4;
	const static DWORD STATE_FAULTCODE   = 8;
	const static DWORD STATE_FAULTSTRING = 16;
	const static DWORD STATE_FAULTACTOR  = 32;
	const static DWORD STATE_DETAIL      = 64;
	const static DWORD STATE_RESET       = 128;
	const static DWORD STATE_SKIP        = 256;


	CComPtr<ISAXXMLReader> m_spReader;
	CSAXStringBuilder m_stringBuilder;
	CSkipHandler m_skipHandler;

	const wchar_t *m_wszSoapPrefix;
	int m_cchSoapPrefix;

public:
	virtual ~CSoapFaultParser()
	{
		m_skipHandler.DetachParent();		
	}
	
	// IUnknown interface
	HRESULT __stdcall QueryInterface(REFIID riid, void **ppv)
	{
		if (ppv == NULL)
		{
			return E_POINTER;
		}

		*ppv = NULL;

		if (InlineIsEqualGUID(riid, IID_IUnknown) ||
			InlineIsEqualGUID(riid, IID_ISAXContentHandler))
		{
			*ppv = static_cast<ISAXContentHandler *>(this);
			return S_OK;
		}

		return E_NOINTERFACE;
	}

	ULONG __stdcall AddRef()
	{
		return 1;
	}

	ULONG __stdcall Release()
	{
		return 1;
	}

	// constructor

	CSoapFaultParser(CSoapFault *pFault, ISAXXMLReader *pReader)
		:m_pFault(pFault), m_dwState(STATE_ERROR), m_spReader(pReader)
	{
		ATLASSERT( pFault != NULL );
		ATLASSERT( pReader != NULL );
	}

	// ISAXContentHandler interface
	HRESULT __stdcall startElement( 
		 const wchar_t  * wszNamespaceUri,
		 int cchNamespaceUri,
		 const wchar_t  * wszLocalName,
		 int cchLocalName,
		 const wchar_t  * /*wszQName*/,
		 int /*cchQName*/,
		 ISAXAttributes  * /*pAttributes*/)
	{
		struct _faultmap
		{
			const wchar_t *wszTag;
			int cchTag;
			DWORD dwState;
		};

		const static _faultmap s_faultParseMap[] =
		{
			{ L"Envelope", sizeof("Envelope")-1, CSoapFaultParser::STATE_ENVELOPE },
			{ L"Body", sizeof("Body")-1, CSoapFaultParser::STATE_BODY },
			{ L"Header", sizeof("Header")-1, CSoapFaultParser::STATE_BODY },
			{ L"Fault", sizeof("Fault")-1, CSoapFaultParser::STATE_START },
			{ L"faultcode", sizeof("faultcode")-1, CSoapFaultParser::STATE_FAULTCODE },
			{ L"faultstring", sizeof("faultstring")-1, CSoapFaultParser::STATE_FAULTSTRING },
			{ L"faultactor", sizeof("faultactor")-1, CSoapFaultParser::STATE_FAULTACTOR },
			{ L"detail", sizeof("detail")-1, CSoapFaultParser::STATE_DETAIL }
		};

		if (m_spReader.p == NULL)
		{
			ATLTRACE( _T("ATLSOAP: CSoapFaultParser::startElement -- ISAXXMLReader is NULL.\r\n" ) );

			return E_INVALIDARG;
		}

		m_dwState &= ~STATE_RESET;
		for (int i=0; i<(sizeof(s_faultParseMap)/sizeof(s_faultParseMap[0])); i++)
		{
			if ((cchLocalName == s_faultParseMap[i].cchTag) &&
				(!wcsncmp(wszLocalName, s_faultParseMap[i].wszTag, cchLocalName)))
			{
				DWORD dwState = s_faultParseMap[i].dwState;
				if ((dwState & (STATE_START | STATE_ENVELOPE | STATE_BODY)) == 0)
				{
					m_stringBuilder.SetReader(m_spReader);
					m_stringBuilder.SetParent(this);

					m_stringBuilder.Clear();
					m_spReader->putContentHandler( &m_stringBuilder );
				}
				else
				{
					if ((dwState <= m_dwState) || 
						(cchNamespaceUri != sizeof(SOAPENV_NAMESPACEA)-1) ||
						(wcsncmp(wszNamespaceUri, SOAPENV_NAMESPACEW, cchNamespaceUri)))
					{
						ATLTRACE( _T("ATLSOAP: CSoapFaultParser::startElement -- malformed SOAP fault.\r\n" ) );

						return E_FAIL;
					}
				}

				m_dwState = dwState;
				return S_OK;
			}
		}
		if (m_dwState > STATE_START)
		{
			m_dwState = STATE_SKIP;
			m_skipHandler.SetReader(m_spReader);
			m_skipHandler.SetParent(this);

			m_spReader->putContentHandler( &m_skipHandler );
			return S_OK;
		}

		ATLTRACE( _T("ATLSOAP: CSoapFaultParser::startElement -- malformed SOAP fault.\r\n" ) );

		return E_FAIL;
	}

	HRESULT __stdcall startPrefixMapping(
		 const wchar_t  * wszPrefix,
		 int cchPrefix,
		 const wchar_t  * wszUri,
		 int cchUri)
	{
		if ((cchUri == sizeof(SOAPENV_NAMESPACEA)-1) &&
			(!wcsncmp(wszUri, SOAPENV_NAMESPACEW, cchUri)))
		{
			m_wszSoapPrefix = wszPrefix;
			m_cchSoapPrefix = cchPrefix;
		}

		return S_OK;
	}

	HRESULT __stdcall characters( 
		 const wchar_t  * wszChars,
		 int cchChars);
};

extern __declspec(selectany) const int ATLS_SOAPFAULT_CNT = 4;

class CSoapFault
{
private:

	struct _faultcode
	{
		const wchar_t *wsz;
		int cch;
		const wchar_t *wszFaultString;
		int cchFaultString;
		SOAP_ERROR_CODE errCode;
	};

	static const _faultcode s_faultCodes[];

public:

	// members
	SOAP_ERROR_CODE m_soapErrCode;
	CStringW m_strFaultCode;
	CStringW m_strFaultString;
	CStringW m_strFaultActor;
	CStringW m_strDetail;

	CSoapFault()
		: m_soapErrCode(SOAP_E_UNK)
	{
	}

	HRESULT SetErrorCode(
		const wchar_t *wsz, 
		const wchar_t *wszSoapPrefix,
		int cch = -1, 
		int cchSoapPrefix = -1,
		bool bSetFaultString = true)
	{
		if ((wsz == NULL) || (wszSoapPrefix == NULL))
		{
			return E_INVALIDARG;
		}

		if (cch == -1)
		{
			cch = (int) wcslen(wsz);
		}

		while (*wsz && iswspace(*wsz))
		{
			++wsz;
			--cch;
		}

		if (cchSoapPrefix == -1)
		{
			cchSoapPrefix = (int) wcslen(wszSoapPrefix);
		}

		const wchar_t *wszLocalName = wcschr(wsz, L':');
		if (wszLocalName == NULL)
		{
			// faultCode must be QName

			ATLTRACE( _T("ATLSOAP: CSoapFault::SetErrorCode -- faultCode is not a QName.\r\n" ) );

			return E_FAIL;
		}

		// make sure the namespace of the fault is the
		// SOAPENV namespace
		if ((cchSoapPrefix != (int)(wszLocalName-wsz)) ||
			(wcsncmp(wsz, wszSoapPrefix, cchSoapPrefix)))
		{
			ATLTRACE( _T("ATLSOAP: CSoapFault::SetErrorCode -- fault namespace is incorrect.\r\n" ) );

			return E_FAIL;
		}

		wszLocalName++;
		cch -= (int) (wszLocalName-wsz);

		_ATLTRY
		{
			for (int i=0; i<ATLS_SOAPFAULT_CNT; i++)
			{
				if ((cch == s_faultCodes[i].cch) &&
					(!wcsncmp(wszLocalName, s_faultCodes[i].wsz, cch)))
				{
					m_soapErrCode = s_faultCodes[i].errCode;
					if (bSetFaultString != false)
					{
						m_strFaultString.SetString(s_faultCodes[i].wszFaultString, s_faultCodes[i].cchFaultString);
						break;
					}
				}
			}
			if (m_strFaultString.GetLength() == 0)
			{
				m_strFaultCode.SetString(wszLocalName, cch);
			}
		}
		_ATLCATCHALL()
		{
			ATLTRACE( _T("ATLSOAP: CSoapFault::SetErrorCode -- out of memory.\r\n" ) );

			return E_OUTOFMEMORY;
		}

		return S_OK;
	}

	HRESULT ParseFault(IStream *pStream, ISAXXMLReader *pReader = NULL)
	{
		if (pStream == NULL)
		{
			ATLTRACE( _T("ATLSOAP: CSoapFault::ParseFault -- NULL IStream was passed.\r\n" ) );

			return E_INVALIDARG;
		}

		CComPtr<ISAXXMLReader> spReader;
		if (pReader != NULL)
		{
			spReader = pReader;
		}
		else
		{
			if (FAILED(spReader.CoCreateInstance(ATLS_SAXXMLREADER_CLSID, NULL, CLSCTX_INPROC_SERVER)))
			{
				ATLTRACE( _T("ATLSOAP: CSoapFault::ParseFault -- CoCreateInstance of SAXXMLReader failed.\r\n" ) );

				return E_FAIL;
			}
		}

		Clear();
		CSoapFaultParser parser(const_cast<CSoapFault *>(this), spReader);
		spReader->putContentHandler(&parser);

		CComVariant varStream;
		varStream = static_cast<IUnknown*>(pStream);

		HRESULT hr = spReader->parse(varStream);
		spReader->putContentHandler(NULL);
		return hr;
	}

	HRESULT GenerateFault(IWriteStream *pWriteStream)
	{
		if ((pWriteStream == NULL) || (m_soapErrCode == SOAP_E_UNK))
		{
			return E_INVALIDARG;
		}

		ATLASSERT( (m_soapErrCode == SOAP_E_UNK) || 
				   (m_soapErrCode == SOAP_E_VERSION_MISMATCH) ||
				   (m_soapErrCode == SOAP_E_MUST_UNDERSTAND) || 
				   (m_soapErrCode == SOAP_E_CLIENT) ||
				   (m_soapErrCode == SOAP_E_SERVER) );

		HRESULT hr = S_OK;
		_ATLTRY
		{
			const wchar_t *wszFaultCode = NULL;
			if (m_strFaultCode.GetLength() == 0)
			{
				for (int i=0; i<4; i++)
				{
					if (s_faultCodes[i].errCode == m_soapErrCode)
					{
						if (m_strFaultString.GetLength() == 0)
						{
							m_strFaultString.SetString(s_faultCodes[i].wszFaultString, 
								s_faultCodes[i].cchFaultString);
						}

						wszFaultCode = s_faultCodes[i].wsz;
						break;
					}
				}
			}

			if (wszFaultCode == NULL)
			{
				if (m_strFaultCode.GetLength() != 0)
				{
					wszFaultCode = m_strFaultCode;
				}
				else
				{
					ATLTRACE( _T("CSoapFault::GenerateFault -- missing/invalid fault code.\r\n") );
					return E_FAIL;
				}
			}

			const LPCSTR s_szErrorFormat =
				"<SOAP:Envelope xmlns:SOAP=\"" SOAPENV_NAMESPACEA "\">"
				"<SOAP:Body>"
				"<SOAP:Fault>"
				"<faultcode>SOAP:%ws</faultcode>"
				"<faultstring>%ws</faultstring>"
				"%s%ws%s"
				"<detail>%ws</detail>"
				"</SOAP:Fault>"
				"</SOAP:Body>"
				"</SOAP:Envelope>";

			CStringA strFault;
			strFault.Format(s_szErrorFormat, wszFaultCode, m_strFaultString, 
				m_strFaultActor.GetLength() ? "<faultactor>" : "", m_strFaultActor, 
				m_strFaultActor.GetLength() ? "</faultactor>" : "",
				m_strDetail);

			hr = pWriteStream->WriteStream(strFault, strFault.GetLength(), NULL);
		}
		_ATLCATCHALL()
		{
			ATLTRACE( _T("ATLSOAP: CSoapFault::GenerateFault -- out of memory.\r\n" ) );
			hr = E_OUTOFMEMORY;
		}

		return hr;
	}

	void Clear()
	{
		m_soapErrCode = SOAP_E_UNK;
		m_strFaultCode.Empty();
		m_strFaultString.Empty();
		m_strFaultActor.Empty();
		m_strDetail.Empty();
	}
}; // class CSoapFault

#define DECLARE_SOAP_FAULT(__name, __faultstring, __errcode) \
	{ L ## __name, sizeof(__name)-1, L ## __faultstring, sizeof(__faultstring), __errcode },

__declspec(selectany) const CSoapFault::_faultcode CSoapFault::s_faultCodes[] =
{
	DECLARE_SOAP_FAULT("VersionMismatch", "SOAP Version Mismatch Error", SOAP_E_VERSION_MISMATCH)
	DECLARE_SOAP_FAULT("MustUnderstand", "SOAP Must Understand Error", SOAP_E_MUST_UNDERSTAND)
	DECLARE_SOAP_FAULT("Client", "SOAP Invalid Request", SOAP_E_CLIENT)
	DECLARE_SOAP_FAULT("Server", "SOAP Server Application Faulted", SOAP_E_SERVER)
};

ATL_NOINLINE inline HRESULT __stdcall CSoapFaultParser::characters( 
		 const wchar_t  * wszChars,
		 int cchChars)
{
	if (m_pFault == NULL)
	{
		return E_INVALIDARG;
	}

	if (m_dwState & STATE_RESET)
	{
		return S_OK;
	}

	HRESULT hr = E_FAIL;
	_ATLTRY
	{
		switch (m_dwState)
		{
			case STATE_FAULTCODE:
				if (m_pFault->m_soapErrCode == SOAP_E_UNK)
				{
					hr = m_pFault->SetErrorCode(wszChars, m_wszSoapPrefix, 
						cchChars, m_cchSoapPrefix, false);
				}
				break;
			case STATE_FAULTSTRING:
				if (m_pFault->m_strFaultString.GetLength() == 0)
				{
					m_pFault->m_strFaultString.SetString(wszChars, cchChars);
					hr = S_OK;
				}
				break;
			case STATE_FAULTACTOR:
				if (m_pFault->m_strFaultActor.GetLength() == 0)
				{
					m_pFault->m_strFaultActor.SetString(wszChars, cchChars);
					hr = S_OK;
				}
				break;
			case STATE_DETAIL:
				if (m_pFault->m_strDetail.GetLength() == 0)
				{
					m_pFault->m_strDetail.SetString(wszChars, cchChars);
					hr = S_OK;
				}
				break;
			case STATE_START: case STATE_ENVELOPE : case STATE_BODY : case STATE_SKIP:
				hr = S_OK;
				break;
			default:
				// should never get here
				ATLASSERT( FALSE );
				break;
		}
	}
	_ATLCATCHALL()
	{
		ATLTRACE( _T("ATLSOAP: CSoapFaultParser::characters -- out of memory.\r\n" ) );

		hr = E_OUTOFMEMORY;
	}

	m_dwState |= STATE_RESET;

	return hr;
}

////////////////////////////////////////////////////////////////////////////////
//
// CSoapRootHandler - the class that does most of the work
//
////////////////////////////////////////////////////////////////////////////////

#ifndef ATLSOAP_STACKSIZE
	// 16 will be plenty for the 99% case
	#define ATLSOAP_STACKSIZE 16
#endif

#ifndef ATLSOAP_GROWARRAY
	#define ATLSOAP_GROWARRAY 10
#endif

class CSoapRootHandler : public ISAXContentHandlerImpl
{
private:

	friend class _CSDLGenerator;

	//
	// state constants
	//
	const static DWORD SOAP_START        = 0;
	const static DWORD SOAP_ENVELOPE     = 1;
	const static DWORD SOAP_HEADERS      = 2;
	const static DWORD SOAP_BODY         = 3;
	const static DWORD SOAP_PARAMS       = 4;
	const static DWORD SOAP_CALLED       = 5;
	const static DWORD SOAP_RESPONSE     = 6;
	const static DWORD SOAP_HEADERS_DONE = 7;

	//
	// hash values for SOAP namespaces and elements
	//
	const static ULONG SOAP_ENV = 0x5D3574E2;
	const static ULONG SOAP_ENC = 0xBD62724B;
	const static ULONG ENVELOPE = 0xDBE6009E;
	const static ULONG HEADER   = 0xAF4DFFC9;
	const static ULONG BODY     = 0x0026168E;

	//
	// XSD Names
	//
	struct XSDEntry
	{
		wchar_t * wszName;
		char * szName;
		int cchName;
	};

	const static XSDEntry s_xsdNames[];

	//
	// CBitVector - a dynamically sized bit vector class
	//
	class CBitVector
	{
	private:

		// 64 bits will handle the 99% case
		unsigned __int64 m_nBits;

		// when we need to grow
		unsigned __int64 * m_pBits;

		size_t m_nSize;

		bool Grow(size_t nIndex)
		{
			// Think carefully
			// In our current implementation, CHAR_BIT==8, and sizeof(m_nBits)==8. Easy to confuse the two.

			// We do math in bits, so this is our max size
			ATLENSURE(nIndex<SIZE_MAX/((sizeof(m_nBits)*CHAR_BIT)));

			// round up to nearest 64 bits
			size_t nAllocSizeBits = nIndex+((sizeof(m_nBits)*CHAR_BIT)-(nIndex%(sizeof(m_nBits)*CHAR_BIT)));
			size_t nAllocSizeBytes = nAllocSizeBits/CHAR_BIT;

			if (m_pBits != &m_nBits)
			{
				unsigned __int64 * pNewBits=NULL;
				pNewBits = (unsigned __int64 *) realloc(m_pBits, nAllocSizeBytes );
				if(!pNewBits)
				{
					return false;
				}
				m_pBits=pNewBits;
			}
			else
			{
				m_pBits = (unsigned __int64 *) malloc(nAllocSizeBytes );
				if (m_pBits != NULL)
				{
					Checked::memcpy_s(m_pBits, nAllocSizeBytes, &m_nBits, sizeof(m_nBits));
				}
			}

			if (m_pBits != NULL)
			{
				// set new bits to 0
				memset(m_pBits+(m_nSize/(CHAR_BIT*sizeof(m_nBits))), 0x00, (nAllocSizeBits-m_nSize)/CHAR_BIT);
				m_nSize = nAllocSizeBits;
				return true;
			}

			ATLTRACE( _T("ATLSOAP: CBitVector::Grow -- out of memory.\r\n" ) );

			return false;
		}

	public:

		CBitVector()
			: m_nBits(0), m_nSize(sizeof(m_nBits)*CHAR_BIT)
		{
			m_pBits = &m_nBits;
		}

		CBitVector(const CBitVector&)
		{
			m_pBits = &m_nBits;
		}

		const CBitVector& operator=(const CBitVector& that)
		{
			if (this != &that)
			{
				m_pBits = &m_nBits;
			}

			return *this;
		}

		bool GetBit(size_t nIndex) const
		{
			if (nIndex >= m_nSize)
			{
				return false;
			}

			size_t i = nIndex/(sizeof(m_nBits)*CHAR_BIT);
			size_t nBits = nIndex-i*(sizeof(m_nBits)*CHAR_BIT);
			return ((m_pBits[i] >> nBits) & 0x01);
		}

		bool SetBit(size_t nIndex)
		{
			if (nIndex >= m_nSize)
			{
				if (!Grow(nIndex))
				{
					return false;
				}
			}

			size_t i = nIndex/(sizeof(m_nBits)*CHAR_BIT);
			size_t nBits = nIndex-i*(sizeof(m_nBits)*CHAR_BIT);
			m_pBits[i] |= (((unsigned __int64) 1) << nBits);

			return true;
		}

		void Clear()
		{
			if (m_pBits == &m_nBits)
			{
				m_nBits = 0;
			}
			else
			{
				memset(m_pBits, 0x00, (m_nSize/CHAR_BIT));
			}
		}

		~CBitVector()
		{
			if (m_pBits != &m_nBits)
			{
				free(m_pBits);
			}

			m_pBits = &m_nBits;
			m_nSize = sizeof(m_nBits)*CHAR_BIT;
		}

		void RelocateFixup()
		{
			if (m_nSize <= sizeof(m_nBits)*CHAR_BIT)
			{
				m_pBits = &m_nBits;
			}
		}
	}; // class CBitVector

	//
	// Parsing State
	//
	struct ParseState
	{
		void *pvElement;
		DWORD dwFlags;
		size_t nAllocSize;
		size_t nExpectedElements;
		size_t nElement;
		const _soapmap *pMap;
		const _soapmapentry *pEntry;

		// mark when we get an item
		CBitVector vec;

		size_t nDepth;

		ParseState(void *pvElement_ = NULL, DWORD dwFlags_ = 0, 
			size_t nAllocSize_ = 0, size_t nExpectedElements_ = 0, 
			size_t nElement_ = 0, const _soapmap *pMap_ = NULL, 
			const _soapmapentry *pEntry_ = NULL)
			: pvElement(pvElement_), dwFlags(dwFlags_), nAllocSize(nAllocSize_),
			  nExpectedElements(nExpectedElements_), nElement(nElement_), pMap(pMap_),
			  pEntry(pEntry_), nDepth(0)
		{
			vec.Clear();
		}

		ParseState(const ParseState& that)
		{
			pvElement = that.pvElement;
			dwFlags = that.dwFlags;
			nAllocSize = that.nAllocSize;
			nExpectedElements = that.nExpectedElements;
			nElement = that.nElement;
			pMap = that.pMap;
			pEntry = that.pEntry;
			nDepth = that.nDepth;
			vec.Clear();
		}

		~ParseState()
		{
			pvElement = NULL;
			dwFlags = 0;
			nAllocSize = 0;
			nExpectedElements = 0;
			nElement = 0;
			pMap = NULL;
			pEntry = NULL;
			nDepth = 0;
			vec.Clear();
		}

		void RelocateFixup()
		{
			vec.RelocateFixup();
		}
	}; // struct ParseState

	class CParseStateElementTraits : public CDefaultElementTraits<ParseState>
	{
	public:
		// CBitVector relocate fixup
		static void RelocateElements( ParseState* pDest, ParseState* pSrc, size_t nElements )
		{
			CDefaultElementTraits<ParseState>::RelocateElements(pDest, pSrc, nElements);

			// fixup CBitVector
			for (size_t i=0; i<nElements; i++)
			{
				pDest[i].RelocateFixup();
			}
		}
	};

	class CResponseGenerator
	{
	public:
		HRESULT StartEnvelope(IWriteStream *pStream)
		{
			ATLENSURE_RETURN( pStream != NULL );

			return pStream->WriteStream("<soap:Envelope "
				"xmlns:soap=\"" SOAPENV_NAMESPACEA "\" "
				"xmlns:xsi=\"" XSI_NAMESPACEA "\" "
				"xmlns:xsd=\"" XSD_NAMESPACEA "\" "
				"xmlns:soapenc=\"" SOAPENC_NAMESPACEA "\">",

				sizeof("<soap:Envelope "
					"xmlns:soap=\"" SOAPENV_NAMESPACEA "\" "
					"xmlns:xsi=\"" XSI_NAMESPACEA "\" "
					"xmlns:xsd=\"" XSD_NAMESPACEA "\" "
					"xmlns:soapenc=\"" SOAPENC_NAMESPACEA "\">")-1,

				NULL);
		}

		HRESULT StartHeaders(IWriteStream *pStream, const _soapmap *pMap)
		{
			ATLENSURE_RETURN( pStream != NULL );
			ATLENSURE_RETURN( pMap != NULL );

			HRESULT hr = pStream->WriteStream("<soap:Header", sizeof("<soap:Header")-1, NULL);
			if (SUCCEEDED(hr))
			{
				if ((pMap->dwCallFlags & (SOAPFLAG_RPC | SOAPFLAG_ENCODED)) != 
					(SOAPFLAG_RPC | SOAPFLAG_ENCODED))
				{
					// qualify document/literal by default
					// For this version, ATL Server will not respect 
					// the elementForm* attributes in an XSD schema

					hr = pStream->WriteStream(" xmlns=\"", sizeof(" xmlns=\"")-1, NULL);
					if (SUCCEEDED(hr))
					{
						hr = pStream->WriteStream(pMap->szNamespace, pMap->cchNamespace, NULL);
						if (SUCCEEDED(hr))
						{
							hr = pStream->WriteStream("\">", sizeof("\">")-1, NULL);
						}
					}					
				}
				else
				{
					// rpc/encoded
					hr = pStream->WriteStream(">", sizeof(">")-1, NULL);
				}
			}
			return hr;
		}

		HRESULT EndHeaders(IWriteStream *pStream)
		{
			ATLENSURE_RETURN( pStream != NULL );

			return pStream->WriteStream("</soap:Header>", sizeof("</soap:Header>")-1, NULL);
		}

		virtual HRESULT StartBody(IWriteStream *pStream)
		{
			ATLENSURE_RETURN( pStream != NULL );

			return pStream->WriteStream(
				"<soap:Body>", sizeof("<soap:Body>")-1, NULL);
		}

		HRESULT EndBody(IWriteStream *pStream)
		{
			ATLENSURE_RETURN( pStream != NULL );

			return pStream->WriteStream("</soap:Body>", sizeof("</soap:Body>")-1, NULL);
		}

		HRESULT EndEnvelope(IWriteStream *pStream)
		{
			ATLENSURE_RETURN( pStream != NULL );

			return pStream->WriteStream("</soap:Envelope>", sizeof("</soap:Envelope>")-1, NULL);
		}

		virtual HRESULT StartMap(IWriteStream *pStream, const _soapmap *pMap, bool bClient) = 0;
		virtual HRESULT EndMap(IWriteStream *pStream, const _soapmap *pMap, bool bClient) = 0;

		virtual HRESULT StartEntry(IWriteStream *pStream, const _soapmap *pMap, const _soapmapentry *pEntry)
		{
			ATLENSURE_RETURN( pStream != NULL );
			ATLENSURE_RETURN( pEntry != NULL );

			// output name
			HRESULT hr = pStream->WriteStream("<", 1, NULL);
			if (SUCCEEDED(hr))
			{
				const char *szHeaderNamespace = NULL;
				int cchHeaderNamespace = 0;

				if ((pMap != NULL) && (pMap->mapType == SOAPMAP_HEADER) &&
					((pEntry->pChain != NULL) && 
					 (pEntry->pChain->szNamespace !=NULL)) ||
					(pEntry->szNamespace != NULL))
				{
					hr = pStream->WriteStream("snp:", sizeof("snp:")-1, NULL);
					if (SUCCEEDED(hr))
					{
						szHeaderNamespace = pEntry->pChain ? 
							pEntry->pChain->szNamespace : pEntry->szNamespace;

						cchHeaderNamespace = pEntry->pChain ? 
							pEntry->pChain->cchNamespace : pEntry->cchNamespace;
					}
				}

				if (SUCCEEDED(hr))
				{
					if ((pEntry->dwFlags & SOAPFLAG_RETVAL)==0)
					{
						hr = pStream->WriteStream(pEntry->szField, pEntry->cchField, NULL);
					}
					else
					{
						hr = pStream->WriteStream("return", sizeof("return")-1, NULL);
					}
					if (SUCCEEDED(hr))
					{
						if (szHeaderNamespace != NULL)
						{
							ATLASSERT( cchHeaderNamespace != 0 );

							hr = pStream->WriteStream(" xmlns:snp=\"", sizeof(" xmlns:snp=\"")-1, NULL);
							if (SUCCEEDED(hr))
							{
								hr = pStream->WriteStream(szHeaderNamespace, cchHeaderNamespace, NULL);
								if (SUCCEEDED(hr))
								{
									hr = pStream->WriteStream("\"", sizeof("\"")-1, NULL);
								}
							}
						}
					}
				}
			}
			if (SUCCEEDED(hr))
			{
				if (pEntry->dwFlags & SOAPFLAG_MUSTUNDERSTAND)
				{
					// output mustUnderstand
					hr = pStream->WriteStream(" soap:mustUnderstand=\"1\"", sizeof(" soap:mustUnderstand=\"1\"")-1, NULL);
				}
			}
			return hr;
		}

		HRESULT EndEntry(IWriteStream *pStream, const _soapmap *pMap, const _soapmapentry *pEntry)
		{
			ATLENSURE_RETURN( pStream != NULL );
			ATLENSURE_RETURN( pEntry != NULL );

			HRESULT hr = pStream->WriteStream("</", 2, NULL);
			if (SUCCEEDED(hr))
			{
				if ((pMap != NULL) && 
					(pMap->mapType == SOAPMAP_HEADER) &&
					((pEntry->pChain != NULL) && 
					 (pEntry->pChain->szNamespace !=NULL)) ||
					(pEntry->szNamespace != NULL))
				{
					hr = pStream->WriteStream("snp:", sizeof("snp:")-1, NULL);
				}
				if ((pEntry->dwFlags & SOAPFLAG_RETVAL)==0)
				{
					hr = pStream->WriteStream(pEntry->szField, pEntry->cchField, NULL);
				}
				else
				{
					hr = pStream->WriteStream("return", sizeof("return")-1, NULL);
				}
				if (SUCCEEDED(hr))
				{
					hr = pStream->WriteStream(">", 1, NULL);
				}
			}
			return hr;
		}
	}; // class CResponseGenerator

	class CDocLiteralGenerator : public CResponseGenerator
	{
	public:

		HRESULT StartMap(IWriteStream *pStream, const _soapmap *pMap, bool bClient)
		{
			ATLENSURE_RETURN( pStream != NULL );
			ATLENSURE_RETURN( pMap != NULL );

			HRESULT hr = S_OK;
			// output type name
			hr = pStream->WriteStream("<", 1, NULL);
			if (SUCCEEDED(hr))
			{
				hr = pStream->WriteStream(pMap->szName, pMap->cchName, NULL);
				if (SUCCEEDED(hr))
				{
					if ((pMap->mapType == SOAPMAP_FUNC) && 
						(bClient == false) && 
						(pMap->dwCallFlags & SOAPFLAG_PID))
					{
						hr = pStream->WriteStream("Response", sizeof("Response")-1, NULL);
						if (FAILED(hr))
						{
							return hr;
						}
					}

					if (pMap->mapType == SOAPMAP_FUNC)
					{
						hr = pStream->WriteStream(" xmlns=\"", sizeof(" xmlns=\"")-1, NULL);
						if (SUCCEEDED(hr))
						{
							hr = pStream->WriteStream(pMap->szNamespace, pMap->cchNamespace, NULL);
							if (SUCCEEDED(hr))
							{
								hr = pStream->WriteStream("\">", sizeof("\">")-1, NULL);
							}
						}
					}
					else
					{
						hr = pStream->WriteStream(">", 1, NULL);
					}
				}
			}
			return hr;
		}

		HRESULT EndMap(IWriteStream *pStream, const _soapmap *pMap, bool bClient)
		{
			ATLENSURE_RETURN( pStream != NULL );
			ATLENSURE_RETURN( pMap != NULL );

			HRESULT hr = pStream->WriteStream("</", sizeof("</")-1, NULL);
			if (SUCCEEDED(hr))
			{
				hr = pStream->WriteStream(pMap->szName, pMap->cchName, NULL);
				if (SUCCEEDED(hr))
				{
					if ((pMap->mapType == SOAPMAP_FUNC) && 
						(bClient == false) && 
						(pMap->dwCallFlags & SOAPFLAG_PID))
					{
						hr = pStream->WriteStream("Response", sizeof("Response")-1, NULL);
						if (FAILED(hr))
						{
							return hr;
						}
					}
					hr = pStream->WriteStream(">", 1, NULL);
				}
			}

			return hr;
		}

	}; // class CDocLiteralGenerator

	class CPIDGenerator : public CDocLiteralGenerator
	{
	};

	class CPADGenerator : public CDocLiteralGenerator
	{
	public:

		virtual HRESULT StartEntry(IWriteStream *pStream, const _soapmap *pMap, const _soapmapentry *pEntry)
		{
			ATLENSURE_RETURN( pStream != NULL );
			ATLENSURE_RETURN( pEntry != NULL );

			HRESULT hr = __super::StartEntry(pStream, pMap, pEntry);
			if (SUCCEEDED(hr) && (pMap->dwCallFlags & SOAPFLAG_PAD))
			{
				hr = pStream->WriteStream(" xmlns=\"", sizeof(" xmlns=\"")-1, NULL);
				if (SUCCEEDED(hr))
				{
					hr = pStream->WriteStream(pMap->szNamespace, pMap->cchNamespace, NULL);
					if (SUCCEEDED(hr))
					{
						hr = pStream->WriteStream("\"", sizeof("\"")-1, NULL);
					}
				}
			}

			return hr;
		}
	}; // class CPADGenerator

	class CRpcEncodedGenerator : public CResponseGenerator
	{
	public:

		HRESULT StartBody(IWriteStream *pStream)
		{
			ATLENSURE_RETURN( pStream != NULL );

			return pStream->WriteStream(
				"<soap:Body soap:encodingStyle=\"" SOAPENC_NAMESPACEA "\">", 
				sizeof("<soap:Body soap:encodingStyle=\"" SOAPENC_NAMESPACEA "\">")-1, NULL);
		}

		HRESULT StartMap(IWriteStream *pStream, const _soapmap *pMap, bool bClient)
		{
			ATLENSURE_RETURN( pStream != NULL );
			ATLENSURE_RETURN( pMap != NULL );

			(bClient); // unused for rpc/encoded

			HRESULT hr = pStream->WriteStream("<snp:", sizeof("<snp:")-1, NULL);
			if (SUCCEEDED(hr))
			{
				hr = pStream->WriteStream(pMap->szName, pMap->cchName, NULL);
				if (SUCCEEDED(hr))
				{
					if (pMap->mapType == SOAPMAP_FUNC)
					{
						hr = pStream->WriteStream(" xmlns:snp=\"", sizeof(" xmlns:snp=\"")-1, NULL);
						if (SUCCEEDED(hr))
						{
							ATLASSERT( pMap->szNamespace != NULL );
							hr = pStream->WriteStream(pMap->szNamespace, pMap->cchNamespace, NULL);
							if (SUCCEEDED(hr))
							{
								hr = pStream->WriteStream("\">", sizeof("\">")-1, NULL);
							}
						}
					}
					else
					{
						hr = pStream->WriteStream(">", 1, NULL);
					}
				}
			}
			return hr;
		}

		HRESULT EndMap(IWriteStream *pStream, const _soapmap *pMap, bool bClient)
		{
			ATLENSURE_RETURN( pStream != NULL );
			ATLENSURE_RETURN( pMap != NULL );

			(bClient); // unused for rpc/encoded

			HRESULT hr = pStream->WriteStream("</snp:", sizeof("</snp:")-1, NULL);
			if (SUCCEEDED(hr))
			{
				hr = pStream->WriteStream(pMap->szName, pMap->cchName, NULL);
				if (SUCCEEDED(hr))
				{
					hr = pStream->WriteStream(">", 1, NULL);
				}
			}

			return hr;
		}
	}; // class CRpcEncodedGenerator

	//
	// members
	//
	CAtlArray<ParseState, CParseStateElementTraits> m_stateStack;
	size_t m_nState;

	DWORD m_dwState;

	CComPtr<ISAXXMLReader> m_spReader;

	CSAXStringBuilder m_stringBuilder;
	CSkipHandler m_skipHandler;

	IAtlMemMgr * m_pMemMgr;

	static CCRTHeap m_crtHeap;

	bool m_bClient;

	void *m_pvParam;

	bool m_bNullCheck;
	bool m_bChildCheck;
	bool m_bCharacters;
	size_t m_nDepth;

	typedef CFixedStringT<CStringW, 16> REFSTRING;

	// used for rpc/encoded messages with href's
	typedef CAtlMap<REFSTRING, ParseState, CStringRefElementTraits<REFSTRING> > REFMAP;
	REFMAP m_refMap;

	//
	// Implementation helpers
	//

	HRESULT PushState(void *pvElement = NULL, const _soapmap *pMap = NULL,
			const _soapmapentry *pEntry = NULL, DWORD dwFlags = 0, size_t nAllocSize = 0, 
			size_t nExpectedElements = 0, size_t nElement = 0)
	{
		if (m_stateStack.IsEmpty())
		{
			// 16 will be plenty for the 99% case
			if (!m_stateStack.SetCount(0, 16))
			{
				ATLTRACE( _T("ATLSOAP: CSoapRootHandler::PushState -- out of memory.\r\n" ) );

				return E_OUTOFMEMORY;
			}
		}

		size_t nCnt = m_stateStack.GetCount();
		m_nState = m_stateStack.Add();
		if (m_stateStack.GetCount() <= nCnt)
		{
			ATLTRACE( _T("ATLSOAP: CSoapRootHandler::PushState -- out of memory.\r\n" ) );

			return E_OUTOFMEMORY;
		}

		ParseState &state = m_stateStack[m_nState];

		state.pvElement = pvElement;
		state.dwFlags = dwFlags;
		state.nAllocSize = nAllocSize;
		state.nExpectedElements = nExpectedElements;
		state.nElement = nElement;
		state.pMap = pMap;
		state.pEntry = pEntry;
		state.nDepth = m_nDepth;

		return S_OK;
	}

	ParseState& GetState()
	{
		return m_stateStack[m_nState];
	}

	void PopState(bool bForce = false)
	{
		if ((m_nState != 0) || (bForce != false))
		{
			m_stateStack.RemoveAt(m_nState);
			--m_nState;
		}
	}

	BOOL IsEqualElement(int cchLocalNameCheck, const wchar_t *wszLocalNameCheck, 
		int cchNamespaceUriCheck, const wchar_t *wszNamespaceUriCheck,
		int cchLocalName, const wchar_t *wszLocalName,
		int cchNamespaceUri, const wchar_t *wszNamespaceUri)
	{
		ATLENSURE(wszLocalName);
		ATLENSURE(wszLocalNameCheck);
		ATLENSURE(wszNamespaceUri);
		ATLENSURE(wszNamespaceUriCheck);
		
		if (cchLocalName == cchLocalNameCheck &&
			cchNamespaceUri == cchNamespaceUriCheck &&
			!wcsncmp(wszLocalName, wszLocalNameCheck, cchLocalName) &&
			!wcsncmp(wszNamespaceUri, wszNamespaceUriCheck, cchNamespaceUri))
		{
			return TRUE;
		}

		return FALSE;
	}

	ATL_FORCEINLINE BOOL IsEqualString(const wchar_t *wszStr1, int cchStr1, const wchar_t *wszStr2, int cchStr2)
	{
		ATLENSURE( wszStr1 != NULL );
		ATLENSURE( wszStr2 != NULL );
		ATLENSURE( cchStr1 >= 0 );
		ATLENSURE( cchStr2 >= 0 );

		if (cchStr1 == cchStr2)
		{
			return !wcsncmp(wszStr1, wszStr2, cchStr2);
		}
		return FALSE;
	}

	ATL_FORCEINLINE BOOL IsEqualStringHash(const wchar_t *wszStr1, int cchStr1, ULONG nHash1, 
		const wchar_t *wszStr2, int cchStr2, ULONG nHash2)
	{
		ATLENSURE( wszStr1 != NULL );
		ATLENSURE( wszStr2 != NULL );
		ATLENSURE( cchStr1 >= 0 );
		ATLENSURE( cchStr2 >= 0 );

		if (nHash1 == nHash2)
		{
			return IsEqualString(wszStr1, cchStr1, wszStr2, cchStr2);
		}

		return FALSE;
	}

	BOOL IsEqualElement(int cchLocalNameCheck, const wchar_t *wszLocalNameCheck, 
		int cchLocalName, const wchar_t *wszLocalName)
	{
		if (cchLocalName == cchLocalNameCheck &&
			!wcsncmp(wszLocalName, wszLocalNameCheck, cchLocalName))
		{
			return TRUE;
		}

		return FALSE;
	}

	void SetOffsetValue(void *pBase, void *pSrc, size_t nOffset)
	{
		void **ppDest = (void **)(((unsigned char *)pBase)+nOffset);
		*ppDest = pSrc;
	}

	bool IsRpcEncoded()
	{
		if ((m_stateStack[0].pMap->dwCallFlags & (SOAPFLAG_RPC | SOAPFLAG_ENCODED)) ==
			(SOAPFLAG_RPC | SOAPFLAG_ENCODED))
		{
			return true;
		}
		return false;
	}


	HRESULT ValidateArrayEntry(
		ParseState& state,
		const wchar_t  *wszLocalName,
		int cchLocalName)
	{
		(cchLocalName);
		(wszLocalName);

		ATLASSERT( state.pEntry != NULL );

		// SOAP Section 5.4.2

		// check number of elements
		if (state.nElement == state.nExpectedElements)
		{
			// too many elements
			if ((state.dwFlags & SOAPFLAG_UNKSIZE)==0)
			{
				ATLTRACE( _T("ATLSOAP: CSoapRootHandler::ValidateArrayEntry -- too many elements.\r\n" ) );
				return E_FAIL;
			}

			ATLASSERT( IsRpcEncoded() == false );

			// see if we need to allocate more
			if (state.nElement == state.nAllocSize)
			{
				unsigned char **ppArr = (unsigned char **)state.pvElement;
				size_t nNewElement=0;
				HRESULT hr=E_FAIL;
				if(FAILED(hr=::ATL::AtlMultiply(&nNewElement, state.nElement, static_cast<size_t>(2))))
				{
					return hr;
				}
				hr = AllocateArray(state.pEntry, (void **)ppArr, __max(nNewElement, ATLSOAP_GROWARRAY), state.nElement);

				if (SUCCEEDED(hr))
				{
					state.nAllocSize = __max((state.nElement)*2, ATLSOAP_GROWARRAY);
				}

				return hr;
			}
		}

		return S_OK;
	}

	HRESULT CheckID(
		const wchar_t *wszNamespaceUri,
		const wchar_t *wszLocalName,
		int cchLocalName,
		ISAXAttributes *pAttributes)
	{
		(cchLocalName);
		(wszLocalName);
		(wszNamespaceUri);
		ATLASSERT( pAttributes != NULL );

		const wchar_t *wsz = NULL;
		int cch = 0;

		HRESULT hr = GetAttribute(pAttributes, L"id", sizeof("id")-1, &wsz, &cch);
		if ((hr == S_OK) && (wsz != NULL))
		{
			const REFMAP::CPair *p = NULL;
			_ATLTRY
			{
				REFSTRING strRef(wsz, cch);
				p = m_refMap.Lookup(strRef);
				if (p == NULL)
				{
					return S_FALSE;
				}
			}
			_ATLCATCHALL()
			{
				ATLTRACE( _T("ATLSOAP: CSoapRootHandler::CheckID -- out of memory.\r\n" ) );

				return E_OUTOFMEMORY;
			}

			ATLASSERT( IsRpcEncoded() == true );

			const ParseState& state = p->m_value;

			// disallow href-chaining
			hr = CheckHref(state.pEntry, state.pvElement, pAttributes);
			if (hr != S_FALSE)
			{
				return E_FAIL;
			}

			hr = S_OK;

			// do array stuff
			if (state.dwFlags & (SOAPFLAG_FIXEDARR | SOAPFLAG_DYNARR))
			{
				hr = GetSection5Info(state, state.pEntry, pAttributes);
			}
			else
			{
				// only structs and arrays are allowed for hrefs
				ATLASSERT( state.pEntry->pChain != NULL );
				ATLASSERT( state.pEntry->pChain->mapType == SOAPMAP_STRUCT );

				// structs must have child entries
				m_bChildCheck = state.pEntry->pChain->nElements != 0;

				if (S_OK != PushState(state.pvElement, state.pEntry->pChain, state.pEntry, 
								state.dwFlags, 0, state.pEntry->pChain->nElements))
				{
					ATLTRACE( _T("ATLSOAP: CSoapRootHandler::CheckID -- out of memory.\n" ) );
					hr = E_OUTOFMEMORY;
				}
			}

			m_refMap.DisableAutoRehash();
			m_refMap.RemoveAtPos(const_cast<REFMAP::CPair*>(p));
			m_refMap.EnableAutoRehash();

			return hr;
		}

		return S_FALSE;
	}

	HRESULT GetElementEntry(
		ParseState& state,
		const wchar_t *wszNamespaceUri,
		const wchar_t *wszLocalName,
		int cchLocalName,
		ISAXAttributes *pAttributes,
		const _soapmapentry **ppEntry)
	{
		ATLENSURE_RETURN( state.pMap != NULL );
		ATLENSURE_RETURN( ppEntry != NULL );

		*ppEntry = NULL;
		const _soapmapentry *pEntries = state.pMap->pEntries;
		DWORD dwIncludeFlags;
		DWORD dwExcludeFlags;

		HRESULT hr = CheckID(wszNamespaceUri, wszLocalName, cchLocalName, pAttributes);
		if (hr != S_FALSE)
		{
			if (hr == S_OK)
			{
				hr = S_FALSE;
			}
			return hr;
		}

		if (m_bClient != false)
		{
			dwIncludeFlags = SOAPFLAG_OUT;
			dwExcludeFlags = SOAPFLAG_IN;
		}
		else
		{
			dwIncludeFlags = SOAPFLAG_IN;
			dwExcludeFlags = SOAPFLAG_OUT;
		}

		ULONG nHash = AtlSoapHashStr(wszLocalName, cchLocalName);

		for (size_t i=0; pEntries[i].nHash != 0; i++)
		{
			if (nHash == pEntries[i].nHash && 
				((pEntries[i].dwFlags & dwIncludeFlags) || 
				 ((pEntries[i].dwFlags & dwExcludeFlags) == 0)) &&
				IsEqualElement(pEntries[i].cchField, pEntries[i].wszField, 
				cchLocalName, wszLocalName)/* &&
				!wcscmp(wszNamespaceUri, wszNamespace)*/)
			{
				// check bit vector

				if (state.vec.GetBit(i) == false)
				{
					if (state.vec.SetBit(i) == false)
					{
						return E_OUTOFMEMORY;
					}
				}
				else
				{
					// already received this element
					ATLTRACE( _T("ATLSOAP: CSoapRootHandler::GetElementEntry -- duplicate element was sent.\r\n" ) );

					return E_FAIL;
				}

				state.nElement++;
				*ppEntry = &pEntries[i];

				return S_OK;
			}
		}

		ATLTRACE( _T("ATLSOAP: CSoapRootHandler::GetElementEntry -- element not found: %.*ws.\r\n" ), cchLocalName, wszLocalName );

		return E_FAIL;
	}

	HRESULT CheckMustUnderstandHeader(ISAXAttributes *pAttributes)
	{
		ATLASSERT( pAttributes != NULL );

		const wchar_t* wszMustUnderstand;
		int cchMustUnderstand;
		bool bMustUnderstand= false;

		if (SUCCEEDED(GetAttribute(pAttributes, L"mustUnderstand", sizeof("mustUnderstand")-1, 
				&wszMustUnderstand, &cchMustUnderstand,
				SOAPENV_NAMESPACEW, sizeof(SOAPENV_NAMESPACEA)-1)) && 
				(wszMustUnderstand != NULL))
		{
			if (FAILED(AtlGetSAXValue(&bMustUnderstand, wszMustUnderstand, cchMustUnderstand)))
			{
				bMustUnderstand = true;
			}
		}

		if (bMustUnderstand == false)
		{
			ATLASSERT( GetReader() != NULL );

			m_skipHandler.SetReader(GetReader());
			m_skipHandler.SetParent(this);

			return GetReader()->putContentHandler( &m_skipHandler );
		}
		else
		{
			SoapFault(SOAP_E_MUST_UNDERSTAND, NULL, 0);
		}

		ATLTRACE( _T("ATLSOAP: CSoapRootHandler::CheckMustUnderstandHeader -- unknown \"mustUnderstand\" SOAP Header was received.\r\n" ) );

		return E_FAIL;
	}

	HRESULT AllocateArray(
		const _soapmapentry *pEntry, 
		void **ppArr, size_t nElements, 
		size_t nCurrElements = 0)
	{
		ATLENSURE_RETURN( ppArr != NULL );
		ATLENSURE_RETURN( pEntry != NULL );

		size_t nElementSize;
		if (pEntry->nVal != SOAPTYPE_UNK)
		{
			nElementSize = AtlSoapGetElementSize((SOAPTYPES) pEntry->nVal);
		}
		else // UDT
		{
			ATLENSURE_RETURN( pEntry->pChain != NULL );
			nElementSize = pEntry->pChain->nElementSize;
		}
		if (nElementSize != 0)
		{
			if (*ppArr == NULL)
			{
				ATLASSERT( nCurrElements == 0 );
				size_t nBytes=0;
				HRESULT hr=S_OK;
				if( FAILED(hr=::ATL::AtlMultiply(&nBytes, nElementSize, nElements)))
				{			
					return hr;
				}
				*ppArr = m_pMemMgr->Allocate(nBytes);
			}
			else // *ppArr != NULL
			{
				ATLASSERT( nCurrElements != 0 );
				size_t nBytes=0;
				HRESULT hr=S_OK;
				if(	FAILED(hr=::ATL::AtlAdd(&nBytes, nElements, nCurrElements)) ||			
					FAILED(hr=::ATL::AtlMultiply(&nBytes, nElementSize, nBytes)))
				{
					return hr;
				}
				*ppArr = m_pMemMgr->Reallocate(*ppArr, nBytes);
			}
		}
		else
		{
			// internal error
			ATLASSERT( FALSE );
			return E_FAIL;
		}

		if (*ppArr == NULL)
		{
			return E_OUTOFMEMORY;
		}

		memset(((unsigned char *)(*ppArr))+(nCurrElements*nElementSize), 0x00, nElements*nElementSize);

		return S_OK;
	}

	HRESULT GetSection5Info(
		const ParseState& state,
		const _soapmapentry *pEntry,
		ISAXAttributes *pAttributes)
	{
		ATLENSURE_RETURN( pEntry != NULL );
		ATLENSURE_RETURN( pAttributes != NULL );

		HRESULT hr;
		if (IsRpcEncoded() != false)
		{
			// check for href
			// we ONLY do this for rpc/encoded (required for interop)
			// NOTE: ATL Server does not support object graphs, so 
			// only single-reference elements are allowed
			hr = CheckHref(pEntry, state.pvElement, pAttributes, 
				pEntry->dwFlags, SOAPFLAG_READYSTATE);
			if (hr != S_FALSE)
			{
				return hr;
			}
		}

		size_t nElements;
		DWORD dwFlags = 0;
		hr = AtlSoapGetArraySize(pAttributes, &nElements);
		if (FAILED(hr))
		{
			return hr;
		}

		size_t nAllocSize = 0;
		size_t nElementsPush = 0;

		if (pEntry->dwFlags & SOAPFLAG_DYNARR)
		{
			// set size_is value
			ATLENSURE_RETURN( state.pMap != NULL );
			int *pnSizeIs = (int *)(((unsigned char *)state.pvElement)+
				(state.pMap->pEntries[pEntry->nSizeIs].nOffset));

			if (hr != S_OK)
			{
				if (IsRpcEncoded())
				{
					// rpc/encoded requires soapenc:arrayType attribute
					return E_FAIL;
				}

				nElements = ATLSOAP_GROWARRAY;
				nAllocSize = ATLSOAP_GROWARRAY;
				dwFlags |= SOAPFLAG_UNKSIZE;
				*pnSizeIs = 0;
			}
			else
			{
				*pnSizeIs = (int)nElements;
				if (nElements == 0)
				{
					// soapenc:arrayType="type[0]"
					// treat as null array

					m_bNullCheck = true;

					// push an emtpy state
					return PushState();
				}

				nElementsPush = nElements;
			}
			void *p = NULL;
			hr = AllocateArray(pEntry, &p, nElements);
			if (hr != S_OK)
			{
				return hr;
			}

			SetOffsetValue(state.pvElement, p, pEntry->nOffset);
		}
		else
		{
			// for fixed-size arrays, we know the number of elements
			ATLASSERT( pEntry->dwFlags & SOAPFLAG_FIXEDARR );
			if (hr == S_OK)
			{
				if (nElements != AtlSoapGetArrayDims(pEntry->pDims))
				{
					return E_FAIL;
				}
			}
			else
			{
				hr = S_OK;
				nElements = AtlSoapGetArrayDims(pEntry->pDims);
			}
			nElementsPush = nElements;
		}

		dwFlags |= pEntry->dwFlags;

		// push element with array flag

		if (S_OK != PushState(((unsigned char *)state.pvElement)+pEntry->nOffset, 
			state.pMap, pEntry, dwFlags & ~SOAPFLAG_READYSTATE, nAllocSize, nElementsPush))
		{
			return E_OUTOFMEMORY;
		}

		m_bChildCheck = true;

		return S_OK;
	}

	void * UpdateArray(ParseState& state, const _soapmapentry *pEntry)
	{
		ATLENSURE(pEntry);

		size_t nSize;
		void *pVal = NULL;

		if (pEntry->nVal != SOAPTYPE_UNK)
		{
			nSize = AtlSoapGetElementSize((SOAPTYPES) pEntry->nVal);
		}
		else
		{
			ATLENSURE( pEntry->pChain != NULL );

			nSize = pEntry->pChain->nElementSize;
		}

		if (state.dwFlags & SOAPFLAG_FIXEDARR)
		{
			unsigned char *ppArr = (unsigned char *)state.pvElement;
			pVal = ppArr+(state.nElement*nSize);
		}
		else
		{
			ATLASSERT( state.dwFlags & SOAPFLAG_DYNARR );

			unsigned char **ppArr = (unsigned char **)state.pvElement;
			pVal = (*ppArr)+(state.nElement*nSize);
			if (state.dwFlags & SOAPFLAG_UNKSIZE)
			{
				ATLASSERT( IsRpcEncoded() == false );

				// need to use the previous state's pvElement to update the size_is value
				ATLASSUME( m_nState > 0 );
				int *pnSizeIs = (int *)(((unsigned char *)m_stateStack[m_nState-1].pvElement)+
					(state.pMap->pEntries[pEntry->nSizeIs].nOffset));

				// update size_is parameter
				*pnSizeIs = (int)(state.nElement+1);
				state.nExpectedElements++;
			}
		}
		state.nElement++;

		return pVal;
	}

	HRESULT ProcessString(const _soapmapentry *pEntry, void *pVal)
	{
		ATLENSURE_RETURN( pEntry != NULL );

		//  set to the string builder class

		ATLASSERT( GetReader() != NULL );

		m_stringBuilder.SetReader(GetReader());
		m_stringBuilder.SetParent(this);

		m_stringBuilder.Clear();
		GetReader()->putContentHandler( &m_stringBuilder );

		if (S_OK != PushState(pVal, NULL, pEntry, SOAPFLAG_READYSTATE | pEntry->dwFlags))
		{
			return E_OUTOFMEMORY;
		}

		return S_OK;
	}


	HRESULT CheckHref(
		const _soapmapentry *pEntry,
		void *pVal,
		ISAXAttributes *pAttributes,
		DWORD dwIncludeFlags = 0,
		DWORD dwExcludeFlags = 0)
	{
		ATLASSERT( pEntry != NULL );
		ATLASSERT( pVal != NULL );
		ATLASSERT( pAttributes != NULL );

		const wchar_t *wsz = NULL;
		int cch = 0;

		HRESULT hr = GetAttribute(pAttributes, L"href", sizeof("href")-1, &wsz, &cch);
		if ((hr == S_OK) && (wsz != NULL))
		{
			// only allow hrefs on structs and arrays
			if (((pEntry->dwFlags & (SOAPFLAG_FIXEDARR | SOAPFLAG_DYNARR))==0) &&
				(pEntry->pChain == NULL || pEntry->pChain->mapType != SOAPMAP_STRUCT))
			{
				ATLTRACE( _T("ATL Server only allows href's on arrays and structs.\r\n") );

				return E_FAIL;
			}

			ATLASSERT( IsRpcEncoded() == true );

			_ATLTRY
			{
				if (*wsz == L'#')
				{
					wsz++;
					cch--;
				}

				REFSTRING strRef(wsz, cch);
				if (m_refMap.Lookup(strRef) != NULL)
				{
					// ATL Server does not support multi-reference objects 
					ATLASSERT( FALSE );
					return E_FAIL;
				}

				ParseState& currState = GetState();
				if ((currState.pEntry != NULL) && (currState.pEntry->dwFlags & (SOAPFLAG_FIXEDARR | SOAPFLAG_DYNARR)))
				{
					// it is an array item
					ATLASSERT( currState.nElement != 0 );

					// exclude array flags for href'd array elements
					dwExcludeFlags |= SOAPFLAG_FIXEDARR | SOAPFLAG_DYNARR;
				}

				ParseState state;
				state.pvElement = pVal;
				state.dwFlags = (pEntry->dwFlags | dwIncludeFlags) & ~dwExcludeFlags;
				state.nExpectedElements = 0;

				state.nElement = 0;
				state.pMap = GetState().pMap;
				state.pEntry = pEntry;

				if (!m_refMap.SetAt(strRef, state))
				{
					ATLTRACE( _T("ATLSOAP: CSoapRootHandler::CheckHref -- out of memory.\r\n" ) );

					return E_OUTOFMEMORY;
				}

				// make sure there are no child elements
				m_bNullCheck = true;

				// push an emtpy state
				return PushState();
			}
			_ATLCATCHALL()
			{
				ATLTRACE( _T("ATLSOAP: CSoapRootHandler::CheckHref -- out of memory.\r\n" ) );

				return E_OUTOFMEMORY;
			}
		}

		return S_FALSE;
	}

	HRESULT ProcessUDT(
		const _soapmapentry *pEntry, 
		void *pVal)
	{
		ATLENSURE_RETURN( pEntry != NULL );
		ATLENSURE_RETURN( pVal != NULL );
		ATLENSURE_RETURN( pEntry->nVal != SOAPTYPE_ERR );
		ATLENSURE_RETURN( pEntry->nVal != SOAPTYPE_USERBASE );

		// if it is a complex type, get the chain entry
		// and push the new state on the stack

		DWORD dwFlags = pEntry->dwFlags;
		if (pEntry->pChain->mapType != SOAPMAP_ENUM)
		{
			// struct
			dwFlags &= ~(SOAPFLAG_FIXEDARR | SOAPFLAG_DYNARR);
			m_bChildCheck = pEntry->pChain->nElements != 0;
		}
		else
		{
			// enum
			dwFlags |= SOAPFLAG_READYSTATE;

			// enums must not have child elements
			m_bNullCheck = true;

			// enums must be specified
			m_bCharacters = true;
		}

		if (S_OK != PushState(pVal, pEntry->pChain, pEntry, dwFlags, 0, pEntry->pChain->nElements))
		{
			return E_OUTOFMEMORY;
		}

		return S_OK;
	}

	HRESULT ChainEntry(
		const ParseState& state,
		const wchar_t  *wszNamespaceUri,
		int cchNamespaceUri,
		const wchar_t  *wszLocalName,
		int cchLocalName,
		ISAXAttributes  *pAttributes)
	{
		ATLENSURE_RETURN( state.pMap != NULL );

		// PAD is only supported on the client
		const _soapmap *pMap = state.pMap;
		if ((pMap->dwCallFlags & SOAPFLAG_CHAIN)==0)
		{
			return S_FALSE;
		}

		ATLENSURE_RETURN( pMap->dwCallFlags & SOAPFLAG_PAD );
		ATLASSUME( m_bClient == true );
		ATLENSURE_RETURN( pMap->nElements == 1 );
		const _soapmapentry *pEntries = pMap->pEntries;
		ATLENSURE_RETURN( pEntries != NULL );

		int nIndex;
		if (pEntries[0].dwFlags & SOAPFLAG_OUT)
		{
			nIndex = 0;
		}
		else
		{
			nIndex = 1;
		}

		const _soapmapentry *pEntry = &pEntries[nIndex];
		ATLENSURE_RETURN( pEntry->nHash != 0 );
		ATLENSURE_RETURN( pEntry->pChain != NULL );

		if (S_OK != PushState(state.pvElement, pEntry->pChain, pEntry, pEntry->dwFlags, 0, pEntry->pChain->nElements))
		{
			return E_OUTOFMEMORY;
		}

		return ProcessParams(wszNamespaceUri, cchNamespaceUri, wszLocalName, cchLocalName, pAttributes);
	}

	HRESULT IsNullEntry(const _soapmapentry *pEntry, ISAXAttributes *pAttributes)
	{
		ATLASSERT( pEntry != NULL );
		ATLASSERT( pAttributes != NULL );

		HRESULT hr = E_FAIL;
		bool bNull = false;
		const wchar_t *wszNull = NULL;
		int cchNull = 0;
		hr = GetAttribute(pAttributes, L"nil", sizeof("nil")-1, &wszNull, &cchNull,
				XSI_NAMESPACEW, sizeof(XSI_NAMESPACEA)-1);
		if ((hr == S_OK) && (wszNull != NULL))
		{
			hr = AtlGetSAXValue(&bNull, wszNull, cchNull);
			if (hr == S_OK)
			{
				if (bNull != false)
				{
					if (pEntry->dwFlags & SOAPFLAG_NULLABLE)
					{
						m_bNullCheck = true;

						// push an emtpy state
						return PushState();
					}

					// non-nullable element
					return E_FAIL;
				}
			}
		}

		return S_FALSE;
	}

	HRESULT ProcessParams(
		const wchar_t  *wszNamespaceUri,
		int cchNamespaceUri,
		const wchar_t  *wszLocalName,
		int cchLocalName,
		ISAXAttributes  *pAttributes)
	{
		(wszNamespaceUri);
		(cchNamespaceUri);

		if (m_stateStack.IsEmpty())
		{
			if (m_dwState == SOAP_HEADERS)
			{
				return CheckMustUnderstandHeader(pAttributes);
			}

			return E_FAIL;
		}

		ParseState &state = GetState();

		ATLASSERT( state.pvElement != NULL );
		HRESULT hr = E_FAIL;
		const _soapmapentry *pEntry = NULL;

		// if array element
		if (state.dwFlags & (SOAPFLAG_FIXEDARR | SOAPFLAG_DYNARR))
		{
			hr = ValidateArrayEntry(state, wszLocalName, cchLocalName);

			if (SUCCEEDED(hr))
			{
				pEntry = state.pEntry;
			}
			else
			{
				return hr;
			}
		}
		else // not an array element
		{
			// special-case for PAD with type=
			hr = ChainEntry(state, wszNamespaceUri, cchNamespaceUri,
				wszLocalName, cchLocalName, pAttributes);

			if (hr == S_FALSE)
			{
				hr = GetElementEntry(state, wszNamespaceUri, wszLocalName, cchLocalName, pAttributes, &pEntry);
				if (hr != S_OK)
				{
					if (hr == S_FALSE)
					{
						hr = S_OK;
					}
					else if (m_dwState == SOAP_HEADERS)
					{
						hr = CheckMustUnderstandHeader(pAttributes);
					}
					return hr;
				}

				ATLASSERT( pEntry != NULL );
			}
			else
			{
				return hr;
			}
		}

		hr = IsNullEntry(pEntry, pAttributes);
		if (hr != S_FALSE)
		{
			return hr;
		}
		hr = S_OK;
		ATLENSURE_RETURN(pEntry);
		// if is array
		if (((pEntry->pDims != NULL) || (pEntry->dwFlags & (SOAPFLAG_FIXEDARR | SOAPFLAG_DYNARR))) && 
			((state.dwFlags & (SOAPFLAG_FIXEDARR | SOAPFLAG_DYNARR)) == 0))
		{
			// get SOAP section-5 info (if it is there)
			return GetSection5Info(state, pEntry, pAttributes);
		}
		else
		{
			// if it is a simple type, push a new (ready) state on the stack
			void *pVal;
			if (state.dwFlags & (SOAPFLAG_FIXEDARR | SOAPFLAG_DYNARR))
			{
				pVal = UpdateArray(state, pEntry);
				ATLASSERT( pVal != NULL );
			}
			else
			{
				pVal = (((unsigned char *)state.pvElement)+pEntry->nOffset);
			}

			if (IsRpcEncoded() != false)
			{
				// check for href
				// we ONLY do this for rpc/encoded (required for interop)
				// NOTE: ATL Server does not support object graphs, so 
				// only single-reference elements are allowed
				hr = CheckHref(pEntry, pVal, pAttributes);
				if (hr != S_FALSE)
				{
					return hr;
				}
				hr = S_OK;
			}

			if (pEntry->nVal != SOAPTYPE_UNK)
			{
				// simple types should not have child elements
				m_bNullCheck = true;

				// if it is a string
				if ((pEntry->nVal == SOAPTYPE_STRING) || (pEntry->nVal == SOAPTYPE_BASE64BINARY))
				{
					hr = ProcessString(pEntry, pVal);
				}
				else
				{
					// expect characters for all non-string simple types
					m_bCharacters = true;

					// basic simple type
					if (S_OK != PushState(pVal, NULL, pEntry, SOAPFLAG_READYSTATE | pEntry->dwFlags))
					{
						hr = E_OUTOFMEMORY;
					}
				}
			}
			else
			{
				hr = ProcessUDT(pEntry, pVal);
				if (pEntry->dwFlags & (SOAPFLAG_DYNARRWRAPPER))
				{
					// We're moving to the **first** entry in the dynamic array wrapper.
					// We know it is the first entry because the dynamic array wrapper is created
					// by sproxy and it guarantees this layouts.
					++m_nDepth;
					ProcessParams (wszNamespaceUri, cchNamespaceUri, pEntry->pChain->pEntries[0].wszField, 
						pEntry->pChain->pEntries[0].cchField, pAttributes);
				}
			}
		}

		return hr;
	}

	size_t GetSizeIsValue(void *pvParam, const _soapmap *pMap, const _soapmapentry *pEntry)
	{
		ATLENSURE( pvParam != NULL );
		ATLENSURE( pMap != NULL );
		ATLENSURE( pEntry != NULL );

		int nSizeIs = pEntry->nSizeIs;
		size_t nOffset = pMap->pEntries[nSizeIs].nOffset;
		void *pVal = ((unsigned char *)pvParam)+nOffset;

		__int64 nVal = 0;
		switch(pMap->pEntries[nSizeIs].nVal)
		{
			case SOAPTYPE_INTEGER: 
			case SOAPTYPE_NONPOSITIVEINTEGER:
			case SOAPTYPE_NEGATIVEINTEGER:
			case SOAPTYPE_LONG:
				nVal = *((__int64 *)pVal);
				break;
			case SOAPTYPE_INT:
				nVal = *((int *)pVal);
				break;
			case SOAPTYPE_SHORT:
				nVal = *((short *)pVal);
				break;
			case SOAPTYPE_BYTE:
				nVal = *((char *)pVal);
				break;
			case SOAPTYPE_POSITIVEINTEGER:
			case SOAPTYPE_NONNEGATIVEINTEGER:
			case SOAPTYPE_UNSIGNEDLONG:
				unsigned __int64 n;
				n = *((unsigned __int64 *)pVal);
				if (n > _I64_MAX)
				{
					// come on ...
					nVal = 0;
				}
				else
				{
					nVal = (__int64)n;
				}
				break;
			case SOAPTYPE_UNSIGNEDINT:
				nVal = *((unsigned int *)pVal);
				break;
			case SOAPTYPE_UNSIGNEDSHORT:
				nVal = *((unsigned short *)pVal);
				break;
			case SOAPTYPE_UNSIGNEDBYTE:
				nVal = *((unsigned char *)pVal);
				break;
			default:
				nVal = 0;
		}

		if (nVal < 0)
		{
			nVal = 0;
		}

		return (size_t) nVal;
	}

	HRESULT GenerateArrayInfo(const _soapmapentry *pEntry, const int *pDims, IWriteStream *pStream)
	{
		ATLENSURE_RETURN( pEntry != NULL );
		ATLENSURE_RETURN( pStream != NULL );

		HRESULT hr = S_OK;
		if (pEntry->nVal != SOAPTYPE_UNK)
		{
			// xsd type
			hr = pStream->WriteStream(" soapenc:arrayType=\"xsd:", 
				sizeof(" soapenc:arrayType=\"xsd:")-1, NULL);
		}
		else
		{
			ATLENSURE_RETURN( pEntry->pChain != NULL );

			hr = pStream->WriteStream(" xmlns:q1=\"", sizeof(" xmlns:q1=\"")-1, NULL);
			if (SUCCEEDED(hr))
			{
				if (pEntry->pChain->szNamespace != NULL)
				{
					hr = pStream->WriteStream(pEntry->pChain->szNamespace, pEntry->pChain->cchNamespace, NULL);
				}
				else
				{
					hr = pStream->WriteStream(GetNamespaceUriA(), -1, NULL);
				}
				if (SUCCEEDED(hr))
				{
					hr = pStream->WriteStream("\"", 1, NULL);
					if (SUCCEEDED(hr))
					{
						hr = pStream->WriteStream(" soapenc:arrayType=\"q1:", 
							sizeof(" soapenc:arrayType=\"q1:")-1, NULL);
					}
				}
			}
		}

		if (FAILED(hr))
		{
			return hr;
		}

		if (pEntry->nVal != SOAPTYPE_UNK)
		{
			hr = pStream->WriteStream(s_xsdNames[pEntry->nVal].szName , 
				s_xsdNames[pEntry->nVal].cchName, NULL);
		}
		else
		{
			ATLASSERT( pEntry->pChain != NULL );

			hr = pStream->WriteStream(pEntry->pChain->szName, pEntry->pChain->cchName, NULL);
		}

		if (FAILED(hr))
		{
			return hr;
		}

		hr = pStream->WriteStream("[", 1, NULL);
		if (FAILED(hr))
		{
			return hr;
		}

		CWriteStreamHelper s( pStream );
		for (int i=1; i<=pDims[0]; i++)
		{
			if (!s.Write(pDims[i]) || 
				((i < pDims[0]) && (S_OK != pStream->WriteStream(", ", 2, NULL))))
			{
				return E_FAIL;
			}
		}

		hr = pStream->WriteStream("]\"", 2, NULL);
		if (FAILED(hr))
		{
			return hr;
		}

		return S_OK;
	}

	HRESULT GenerateXSDWrapper(bool bStart, int nVal, bool bNull, IWriteStream *pStream)
	{
		ATLENSURE_RETURN( pStream != NULL );

		HRESULT hr = pStream->WriteStream((bStart != false) ? "<" : "</", 
			(bStart != false) ? 1 : 2, NULL);
		if (SUCCEEDED(hr))
		{
			hr = pStream->WriteStream(s_xsdNames[nVal].szName, 
				s_xsdNames[nVal].cchName, NULL);
			if ((bNull != false) && (SUCCEEDED(hr)))
			{
				hr = pStream->WriteStream(" xsi:nil=\"1\"", sizeof(" xsi:nil=\"1\"")-1, NULL);
			}
			if (SUCCEEDED(hr))
			{
				hr = pStream->WriteStream(">", 1, NULL);
			}
		}

		return hr;
	}

	HRESULT GenerateGenericWrapper(bool bStart, const _soapmap *pMap, IWriteStream *pStream)
	{
		ATLENSURE_RETURN( pStream != NULL );
		ATLENSURE_RETURN( pMap != NULL );

		HRESULT hr = pStream->WriteStream((bStart != false) ? "<" : "</", 
			(bStart != false) ? 1 : 2, NULL);
		if (SUCCEEDED(hr))
		{
			hr = pStream->WriteStream(pMap->szName, pMap->cchName, NULL);
			if (SUCCEEDED(hr))
			{
				hr = pStream->WriteStream(">", 1, NULL);
			}
		}

		return hr;
	}

	HRESULT GetArrayInformation(
		IWriteStream *pStream, 
		const _soapmap *pMap, 
		const _soapmapentry *pEntry, 
		void *pvParam,
		size_t &nCnt, 
		size_t &nElementSize)
	{
		ATLENSURE_RETURN( pStream != NULL );
		ATLENSURE_RETURN( pMap != NULL );
		ATLENSURE_RETURN( pEntry != NULL );
		ATLENSURE_RETURN( pvParam != NULL );

		const int *pDims = NULL;
		int arrDims[2] = { 0 };

		if (pEntry->dwFlags & SOAPFLAG_FIXEDARR)
		{
			pDims = pEntry->pDims;
		}
		else
		{
			ATLASSERT( pEntry->dwFlags & SOAPFLAG_DYNARR );
			nCnt = GetSizeIsValue(pvParam, pMap, pEntry);

			if (nCnt == 0)
			{
				// array size should only be zero if array is NULL
				// did you forget to set the array size?
				ATLASSERT( FALSE );
				return E_FAIL;
			}

			arrDims[0] = 1;
			arrDims[1] = (int) nCnt;

			pDims = arrDims;
		}

		// output array information
		HRESULT hr = GenerateArrayInfo(pEntry, pDims, pStream);
		if (FAILED(hr))
		{
			return hr;
		}
		if (SUCCEEDED(hr))
		{
			nCnt = AtlSoapGetArrayDims(pDims);

			// did you forget to set the size_is value?
			ATLASSERT( nCnt != 0 );

			if (pEntry->nVal != SOAPTYPE_UNK)
			{
				nElementSize = AtlSoapGetElementSize((SOAPTYPES) pEntry->nVal);
			}
			else
			{
				ATLENSURE_RETURN( pEntry->pChain != NULL );

				nElementSize = pEntry->pChain->nElementSize;
			}
		}

		return hr;
	}

	HRESULT GenerateEnum(IWriteStream *pStream, void *pVal, const _soapmapentry *pEntry, bool bArray)
	{
		ATLENSURE_RETURN( pStream != NULL );
		ATLENSURE_RETURN( pVal != NULL );
		ATLENSURE_RETURN( pEntry != NULL );

		int nVal = *((int *)pVal);
		const _soapmapentry *pEnumEntries = pEntry->pChain->pEntries;

		ATLENSURE_RETURN( pEnumEntries != NULL );
		size_t j;
		HRESULT hr = E_FAIL;
		for (j=0; pEnumEntries[j].nHash != 0; j++)
		{
			if (nVal == pEnumEntries[j].nVal)
			{
				hr = pStream->WriteStream(pEnumEntries[j].szField, pEnumEntries[j].cchField, NULL);
				if ((bArray != false) && (SUCCEEDED(hr)))
				{
					hr = GenerateGenericWrapper(false, pEntry->pChain, pStream);
				}
				break;
			}
		}

		return hr;
	}

	HRESULT GenerateHeaders(CResponseGenerator *pGenerator, const _soapmap *pMap, IWriteStream *pStream)
	{
		ATLENSURE_RETURN( pStream != NULL );
		ATLENSURE_RETURN( pMap != NULL );

		ATLENSURE_RETURN( pGenerator != NULL );

		DWORD dwIncludeFlags = SOAPFLAG_OUT;
		if (m_bClient != false)
		{
			dwIncludeFlags = SOAPFLAG_IN;
		}

		size_t nCnt = 0;
		for (size_t i=0; pMap->pEntries[i].nHash != 0; i++)
		{
			if (pMap->pEntries[i].dwFlags & dwIncludeFlags)
			{
				nCnt++;
			}
		}

		// no headers to be sent
		if (nCnt == 0)
		{
			return S_OK;
		}

		HRESULT hr = pGenerator->StartHeaders(pStream, pMap);
		if (SUCCEEDED(hr))
		{
			hr = GenerateResponseHelper(pGenerator, pMap, GetHeaderValue(), pStream);
			if (SUCCEEDED(hr))
			{
				hr = pGenerator->EndHeaders(pStream);
			}
		}

		return hr;
	}

	bool IsNullElement(const _soapmapentry *pEntry, void *pVal, DWORD dwExcludeFlags=0)
	{
		ATLENSURE( pEntry != NULL );
		ATLENSURE( pVal != NULL );

		bool bNull = false;

		DWORD dwFlags = pEntry->dwFlags & ~dwExcludeFlags;

		if (dwFlags & SOAPFLAG_DYNARR)
		{
			unsigned char **ppArr = (unsigned char **)pVal;
			if (*ppArr == NULL)
			{
				bNull = true;
			}
		}
		else if (pEntry->nVal == SOAPTYPE_STRING)
		{
			BSTR *pBSTR = (BSTR *)pVal;
			if (*pBSTR == NULL)
			{
				bNull = true;
			}
		}
		else if ((pEntry->nVal == SOAPTYPE_BASE64BINARY) || (pEntry->nVal == SOAPTYPE_HEXBINARY))
		{
			if (((ATLSOAP_BLOB *)pVal)->data == NULL)
			{
				bNull = true;
			}
		}

		return bNull;
	}

	HRESULT GenerateNull(IWriteStream *pStream)
	{
		ATLENSURE_RETURN( pStream != NULL );
		return pStream->WriteStream(" xsi:nil=\"1\"/>", sizeof(" xsi:nil=\"1\"/>")-1, NULL);
	}

	HRESULT GenerateResponseHelper(CResponseGenerator *pGenerator, const _soapmap *pMap, void *pvParam, IWriteStream *pStream, 
		bool bArrayElement = false)
	{
		ATLENSURE_RETURN( pGenerator != NULL );
		ATLENSURE_RETURN( pMap != NULL );
		ATLENSURE_RETURN( pStream != NULL );

		HRESULT hr = S_OK;

		if ((bArrayElement != false) && 
			((pMap->dwCallFlags & SOAPFLAG_PAD)==0))
		{
			hr = pGenerator->StartMap(pStream, pMap, m_bClient);
			if (FAILED(hr))
			{
				return hr;
			}
		}

		ATLENSURE_RETURN( pMap->pEntries != NULL );

		const _soapmapentry *pEntries = pMap->pEntries;
		size_t i;

		DWORD dwIncludeFlags;
		DWORD dwExcludeFlags;
		if (m_bClient != false)
		{
			dwIncludeFlags = SOAPFLAG_IN;
			dwExcludeFlags = SOAPFLAG_OUT;
		}
		else
		{
			dwIncludeFlags = SOAPFLAG_OUT;
			dwExcludeFlags = SOAPFLAG_IN;
		}

		for (i=0; pEntries[i].nHash != 0; i++)
		{
			if (((pEntries[i].dwFlags & dwIncludeFlags) ||
				((pEntries[i].dwFlags & dwExcludeFlags)==0)) &&
				((pEntries[i].dwFlags & SOAPFLAG_NOMARSHAL)==0))
			{
				hr = pGenerator->StartEntry(pStream, pMap, &pEntries[i]);
				if (FAILED(hr))
				{
					return hr;
				}

				size_t nElementSize = 0;
				size_t nCnt = 1;

				ATLASSERT( pvParam != NULL );

				void *pvCurrent = ((unsigned char *)pvParam)+pEntries[i].nOffset;

				if (IsNullElement(&pEntries[i], pvCurrent))
				{
					hr = GenerateNull(pStream);
					if (SUCCEEDED(hr))
					{
						continue;
					}
					return hr;
				}

				bool bArray = (pEntries[i].dwFlags & (SOAPFLAG_FIXEDARR | SOAPFLAG_DYNARR)) != 0;
				if (bArray != false)
				{
					hr = GetArrayInformation(pStream, pMap, &pEntries[i], pvParam, nCnt, nElementSize);
				}

				hr = pStream->WriteStream(">", 1, NULL);
				if (FAILED(hr))
				{
					return hr;
				}

				for (size_t nElement=0; nElement<nCnt; nElement++)
				{
					void *pVal;

					// get updated value
					if (bArray != false)
					{
						if (pEntries[i].dwFlags & SOAPFLAG_FIXEDARR)
						{
							unsigned char *ppArr = (unsigned char *)pvCurrent;
							pVal = ppArr+(nElement*nElementSize);
						}
						else
						{
							ATLASSERT( pEntries[i].dwFlags & SOAPFLAG_DYNARR );

							unsigned char **ppArr = (unsigned char **)pvCurrent;
							pVal = (*ppArr)+(nElement*nElementSize);
						}
					}
					else
					{
						pVal = pvCurrent;
					}

					if (pEntries[i].nVal != SOAPTYPE_UNK)
					{
						bool bNull = false;
						if (bArray != false)
						{
							bNull = IsNullElement(&pEntries[i], pVal, SOAPFLAG_DYNARR | SOAPFLAG_FIXEDARR);
							hr = GenerateXSDWrapper(true, pEntries[i].nVal, bNull, pStream);
							if (FAILED(hr))
							{
								return hr;
							}
						}
						if (bNull == false)
						{
							hr = AtlSoapGenElementValue(pVal, pStream, (SOAPTYPES) pEntries[i].nVal, GetMemMgr());
						}
						if ((SUCCEEDED(hr)) && (bArray != false))
						{
							hr = GenerateXSDWrapper(false, pEntries[i].nVal, false, pStream);
						}

						if (FAILED(hr))
						{
							return hr;
						}
					}
					else
					{
						ATLASSERT( pEntries[i].pChain != NULL );

						if (pEntries[i].pChain->mapType != SOAPMAP_ENUM)
						{
							// struct
							hr = GenerateResponseHelper(pGenerator, pEntries[i].pChain, pVal, pStream, bArray);
						}
						else
						{
							if (bArray != false)
							{
								hr = GenerateGenericWrapper(true, pEntries[i].pChain, pStream);
								if (FAILED(hr))
								{
									return hr;
								}
							}

							hr = GenerateEnum(pStream, pVal, &pEntries[i], bArray);
						}
					}
				}

				// output element close
				if (SUCCEEDED(hr))
				{
					hr = pGenerator->EndEntry(pStream, pMap, &pEntries[i]);
				}
			}

			if (FAILED(hr))
			{
				return hr;
			}
		}

		if ((bArrayElement != false) && 
			((pMap->dwCallFlags & SOAPFLAG_PAD)==0))
		{
			// output type name
			hr = pGenerator->EndMap(pStream, pMap, m_bClient);
		}

		return hr;
	}

	void CleanupHelper(const _soapmap *pMap, void *pvParam)
	{
		ATLENSURE( pMap != NULL );
		ATLENSURE( pMap->pEntries != NULL );

		if (pvParam == NULL)
		{
			return;
		}

		const _soapmapentry *pEntries = pMap->pEntries;
		size_t i;

		for (i=0; pEntries[i].nHash != 0; i++)
		{
			if ((m_bClient != false) && ((pEntries[i].dwFlags & SOAPFLAG_OUT)==0))
			{
				// skip in-only headers on the client
				continue;
			}

			void *pvCheck = ((unsigned char *)pvParam)+pEntries[i].nOffset;
			if (IsNullElement(&pEntries[i], pvCheck))
			{
				continue;
			}

			size_t nElementSize = 0;
			size_t nCnt = 1;

			const int *pDims = NULL;
			int arrDims[2] = { 0 };

			bool bArray = (pEntries[i].dwFlags & (SOAPFLAG_FIXEDARR | SOAPFLAG_DYNARR)) != 0;

			if (bArray != false)
			{
				if (pEntries[i].dwFlags & SOAPFLAG_FIXEDARR)
				{
					pDims = pEntries[i].pDims;
				}
				else
				{
					ATLASSERT( pEntries[i].dwFlags & SOAPFLAG_DYNARR );
					nCnt = GetSizeIsValue(pvParam, pMap, &pEntries[i]);

					arrDims[0] = 1;
					arrDims[1] = (int) nCnt;

					pDims = arrDims;
				}

				nCnt = AtlSoapGetArrayDims(pDims);

				if (pEntries[i].nVal != SOAPTYPE_UNK)
				{
					nElementSize = AtlSoapGetElementSize((SOAPTYPES) pEntries[i].nVal);
				}
				else
				{
					ATLENSURE( pEntries[i].pChain != NULL );

					nElementSize = pEntries[i].pChain->nElementSize;
				}					
			}

			void *pvCurrent = ((unsigned char *)pvParam)+pEntries[i].nOffset;

			for (size_t nElement=0; nElement<nCnt; nElement++)
			{
				void *pVal;

				// get updated value
				if (bArray != false)
				{
					if (pEntries[i].dwFlags & SOAPFLAG_FIXEDARR)
					{
						unsigned char *ppArr = (unsigned char *)pvCurrent;
						pVal = ppArr+(nElement*nElementSize);
					}
					else
					{
						ATLASSERT( pEntries[i].dwFlags & SOAPFLAG_DYNARR );

						unsigned char **ppArr = (unsigned char **)pvCurrent;
						if (*ppArr == NULL)
						{							
							break;
						}
						pVal = (*ppArr)+(nElement*nElementSize);
					}
				}
				else
				{
					pVal = pvCurrent;
				}

				if (pEntries[i].nVal != SOAPTYPE_UNK)
				{
					AtlSoapCleanupElement(pVal, (SOAPTYPES) pEntries[i].nVal, GetMemMgr());
				}
				else
				{
					ATLENSURE( pEntries[i].pChain != NULL );

					if (pEntries[i].pChain->mapType != SOAPMAP_ENUM)
					{
						CleanupHelper(pEntries[i].pChain, pVal);
					}
				}
			}

			if (pEntries[i].dwFlags & SOAPFLAG_DYNARR)
			{
				// free it
				unsigned char **ppArr = (unsigned char **)pvCurrent;

				ATLENSURE( ppArr != NULL );

				if (*ppArr != NULL)
				{
					m_pMemMgr->Free(*ppArr);
					*ppArr = NULL;
				}
			}
		}
	}

	const _soapmap * GetSoapMapFromName(
		const wchar_t * wszName, 
		int cchName = -1, 
		const wchar_t * wszNamespaceUri = NULL,
		int cchNamespaceUri = -1,
		int *pnVal = NULL,
		bool bHeader = false)
	{
		(cchNamespaceUri);

		const _soapmap ** pEntry = NULL;

		if (bHeader == false)
		{
			pEntry = GetFunctionMap();
		}
		else
		{
			pEntry = GetHeaderMap();
		}

		if (pEntry == NULL)
		{
			return NULL;
		}

		if (cchName < 0)
		{
			cchName = (int)wcslen(wszName);
		}
		if ((cchNamespaceUri < 0) && (wszNamespaceUri != NULL))
		{
			cchNamespaceUri = (int)wcslen(wszNamespaceUri);
		}

		ULONG nFunctionHash = AtlSoapHashStr(wszName, cchName);
		ULONG nNamespaceHash = wszNamespaceUri ? AtlSoapHashStr(wszNamespaceUri, cchNamespaceUri) : 0;

		int i;
		for (i=0; pEntry[i] != NULL; i++)
		{
			if ((IsEqualStringHash(wszName, cchName, nFunctionHash,
				   pEntry[i]->wszName, pEntry[i]->cchWName, pEntry[i]->nHash) != FALSE) &&
				(!wszNamespaceUri ||
				 IsEqualStringHash(wszNamespaceUri, cchNamespaceUri, nNamespaceHash, 
				   pEntry[i]->wszNamespace, pEntry[i]->cchNamespace, pEntry[i]->nNamespaceHash) != FALSE))
			{
				break;
			}
		}

		if (pnVal != NULL)
		{
			*pnVal = i;
		}
		return pEntry[i];
	}

	HRESULT CheckEndElement(const ParseState& state)
	{
		// check for all elements
		if (state.nElement == state.nExpectedElements)
		{
			return S_OK;
		}

		// error check for fixed arrays
		if (state.dwFlags & SOAPFLAG_FIXEDARR)
		{
			return E_FAIL;
		}

		// check for dynamic arrays
		if (state.dwFlags & SOAPFLAG_DYNARR)
		{
			// check for dynamic arrays with known size
			// (from soap:arrayType attribute)
			if ((state.dwFlags & SOAPFLAG_UNKSIZE)==0)
			{
				return E_FAIL;
			}
		}

		DWORD dwIncludeFlags;
		DWORD dwExcludeFlags;

		if (m_bClient != false)
		{
			dwIncludeFlags = SOAPFLAG_OUT;
			dwExcludeFlags = SOAPFLAG_IN;
		}
		else
		{
			dwIncludeFlags = SOAPFLAG_IN;
			dwExcludeFlags = SOAPFLAG_OUT;
		}

		if (state.pMap != NULL)
		{
			// ensure all omitted elements were nullable elements or nomarshal elements
			const _soapmapentry *pEntries = state.pMap->pEntries;
			for (size_t i=0; pEntries[i].nHash != 0; i++)
			{
				if ((pEntries[i].dwFlags & dwIncludeFlags) ||
					((pEntries[i].dwFlags & dwExcludeFlags)==0))
				{
					if (state.vec.GetBit(i) == false)
					{
						if (((pEntries[i].dwFlags & (SOAPFLAG_NULLABLE | SOAPFLAG_NOMARSHAL))==0) && (pEntries[i].nVal != SOAPTYPE_UNK))
						{
							ATLTRACE( _T("ATLSOAP: CSoapRootHandler::CheckEndElement -- invalid number of elements for parameter/field\r\n") );
							return E_FAIL;
						}
					}
				}
			}
		}

		return S_OK;
	}

	HRESULT CheckSoapHeaders(const ParseState &state)
	{
		DWORD dwIncludeFlags;
		DWORD dwExcludeFlags;

		if (m_bClient != false)
		{
			dwIncludeFlags = SOAPFLAG_OUT;
			dwExcludeFlags = SOAPFLAG_IN;
		}
		else
		{
			dwIncludeFlags = SOAPFLAG_IN;
			dwExcludeFlags = SOAPFLAG_OUT;
		}

		if (state.pMap != NULL)
		{
			ATLASSERT( state.pMap->mapType == SOAPMAP_HEADER );

			// ensure all omitted elements were nullable elements, nomarshal elements, or non-required elements
			const _soapmapentry *pEntries = state.pMap->pEntries;
			for (size_t i=0; pEntries[i].nHash != 0; i++)
			{
				if ((pEntries[i].dwFlags & dwIncludeFlags) ||
					((pEntries[i].dwFlags & dwExcludeFlags)==0))
				{
					if (state.vec.GetBit(i) == false)
					{
						bool bNoOmit = (pEntries[i].dwFlags & (SOAPFLAG_NULLABLE | SOAPFLAG_NOMARSHAL))==0;

						if ((bNoOmit != false) || 
							((bNoOmit != false) && (pEntries[i].dwFlags & SOAPFLAG_MUSTUNDERSTAND)))
						{
							ATLTRACE( _T("ATLSOAP: CSoapRootHandler::CheckSoapHeaders -- missing header\r\n") );
							return E_FAIL;
						}
					}
				}
			}
		}

		return S_OK;
	}

	HRESULT CheckEndHeaders(
		const wchar_t  * wszNamespaceUri,
		int cchNamespaceUri,
		const wchar_t  * wszLocalName,
		int cchLocalName)
	{
		if (IsEqualElement(sizeof(SOAP_HEADERA)-1, SOAP_HEADERW,
					sizeof(SOAPENV_NAMESPACEA)-1, SOAPENV_NAMESPACEW,
					cchLocalName, wszLocalName,
					cchNamespaceUri, wszNamespaceUri))
		{
			m_dwState = SOAP_HEADERS_DONE;
			return S_OK;
		}

		// some sort of error
		ATLTRACE( _T("ATLSOAP: CSoapRootHandler::endElement -- invalid SOAP message format while processing headers.\r\n" ) );

		return E_FAIL;
	}

protected:

	ISAXXMLReader * SetReader(ISAXXMLReader *pReader)
	{
		ISAXXMLReader *pPrevRdr = m_spReader;
		m_spReader = pReader;

		return pPrevRdr;
	}

	ISAXXMLReader * GetReader()
	{
		return m_spReader;
	}

	HRESULT SetSoapMapFromName(
		const wchar_t * wszName, 
		int cchName = -1, 
		const wchar_t * wszNamespaceUri = NULL,
		int cchNamespaceUri = -1,
		bool bHeader = false)
	{
		ATLENSURE_RETURN( wszName != NULL );

		int nVal;
		const _soapmap *pMap = NULL;
		if (m_stateStack.GetCount() != 0)
		{
			ATLASSUME( m_stateStack[0].pMap != NULL );
			nVal = (int) m_stateStack[0].nAllocSize;
			ATLASSERT( GetFunctionMap() != NULL );
			pMap = GetFunctionMap()[nVal];
		}
		else
		{
			pMap = GetSoapMapFromName(wszName, cchName,
				wszNamespaceUri, cchNamespaceUri, &nVal, bHeader);
		}

		if (pMap == NULL)
		{
			ATLTRACE( _T("ATLSOAP: CSoapRootHandler::SetSoapMapFromName -- _soapmap not found for: %.*ws, with namespace %.*ws\r\n"),
				(int)wcslen(wszName), wszName, wszNamespaceUri ? (int)wcslen(wszNamespaceUri) : 0, wszNamespaceUri ? wszNamespaceUri : L"");

			return E_FAIL;
		}

		HRESULT hr = E_OUTOFMEMORY;

		// allocate the parameter struct

		void *pvParam = NULL;
		if (bHeader != false)
		{
			pvParam = GetHeaderValue();
		}
		else 
		{
			if (m_bClient == false)
			{
				m_pvParam = m_pMemMgr->Allocate(pMap->nElementSize);
			}
			pvParam = m_pvParam;
		}

		if (pvParam != NULL)
		{
			if (bHeader == false)
			{
				memset(pvParam, 0x00, pMap->nElementSize);
			}

			// push initial state

			if (m_stateStack.GetCount() != 0)
			{
				m_stateStack.RemoveAll();
			}

			hr = PushState(pvParam, pMap, NULL, 0, nVal, pMap->nElements);

			if (FAILED(hr))
			{
				if ((m_bClient == false) && (bHeader == false))
				{
					m_pMemMgr->Free(pvParam);
				}
			}
		}

#ifdef _DEBUG
		if (hr == E_OUTOFMEMORY)
		{
			ATLTRACE( _T("ATLSOAP: CSoapRootHandler::SetSoapMapFromName -- out of memory.\r\n" ) );
		}
#endif // _DEBUG

		return hr;
	}

	// implementation
	virtual const _soapmap ** GetFunctionMap() = 0;
	virtual const _soapmap ** GetHeaderMap() = 0;
	virtual const wchar_t * GetNamespaceUri() = 0;
	virtual const char * GetServiceName() = 0;
	virtual const char * GetNamespaceUriA() = 0;
	virtual HRESULT CallFunction(
		void *pvParam, 
		const wchar_t *wszLocalName, int cchLocalName,
		size_t nItem) = 0;
	virtual void * GetHeaderValue() = 0;

public:

	CSoapRootHandler(ISAXXMLReader *pReader = NULL)
	:  m_pMemMgr(&m_crtHeap), m_spReader(pReader), m_bClient(false),
	   m_nState(0), m_pvParam(NULL), m_nDepth(0)
	{
		InitHandlerState();
	}
	virtual ~CSoapRootHandler()
	{
		m_skipHandler.DetachParent();		
	}

	IAtlMemMgr * SetMemMgr(IAtlMemMgr *pMemMgr)
	{
		IAtlMemMgr *pPrevMgr = m_pMemMgr;
		m_pMemMgr = pMemMgr;

		return pPrevMgr;
	}

	IAtlMemMgr * GetMemMgr()
	{
		return m_pMemMgr;
	}

	// override this function to do SOAP Fault handling
	virtual HRESULT SoapFault(
		SOAP_ERROR_CODE /*errCode*/, 
		const wchar_t * /*wszDetail*/, 
		int /*cchDetail*/)
	{
		if (m_bClient != false)
		{
			return S_OK;
		}

		// SOAP servers must implement this function
		ATLASSERT( FALSE );
		return E_FAIL;
	}

	//
	// implementation
	//

	void InitHandlerState()
	{
		m_bNullCheck = false;
		m_bCharacters = false;
		m_bChildCheck = false;	
		m_dwState = SOAP_START;		
	}
	HRESULT __stdcall startDocument()
	{
		InitHandlerState();
		return S_OK;
	}

	HRESULT __stdcall startElement( 
		 const wchar_t  *wszNamespaceUri,
		 int cchNamespaceUri,
		 const wchar_t  *wszLocalName,
		 int cchLocalName,
		 const wchar_t  * wszQName,
		 int cchQName,
		 ISAXAttributes  *pAttributes)
	{
		if (m_bNullCheck || m_bCharacters)
		{
			// make sure elements that aren't supposed to have child elements
			// do not have child elements, and where we were expecting
			// characters, we got them
			return E_FAIL;
		}

		m_bChildCheck = false;
		++m_nDepth;

		HRESULT hr = S_OK;
		switch (m_dwState)
		{
			case SOAP_PARAMS: case SOAP_HEADERS:
			{
				hr = ProcessParams(wszNamespaceUri, cchNamespaceUri, wszLocalName, 
					cchLocalName, pAttributes);

				break;
			}
			case SOAP_START: case SOAP_ENVELOPE: case SOAP_HEADERS_DONE:
			{
				ULONG nNamespaceHash = AtlSoapHashStr(wszNamespaceUri, 
					cchNamespaceUri);
				if (nNamespaceHash != SOAP_ENV)
				{
					ATLTRACE( _T("ATLSOAP: CSoapRootHandler::startElement -- incorrect SOAP-ENV namespace.\r\n" ) );

					return E_FAIL;
				}

				ULONG nElementHash = AtlSoapHashStr(wszLocalName, cchLocalName);

				if (nElementHash == ENVELOPE && 
					IsEqualElement(
						sizeof(SOAP_ENVELOPEA)-1, SOAP_ENVELOPEW,
						sizeof(SOAPENV_NAMESPACEA)-1, SOAPENV_NAMESPACEW,
						cchLocalName, wszLocalName,
						cchNamespaceUri, wszNamespaceUri))
				{
					// Envelope must be first element in package

					if (m_dwState != SOAP_START)
					{
						ATLTRACE( _T("ATLSOAP: CSoapRootHandler::startElement -- invalid SOAP message format: \"Envelope\" in unexpected location.\r\n" ) );

						hr = E_FAIL;
					}
					m_dwState = SOAP_ENVELOPE;
				}
				else if (nElementHash == HEADER &&
					IsEqualElement(sizeof(SOAP_HEADERA)-1, SOAP_HEADERW,
						sizeof(SOAPENV_NAMESPACEA)-1, SOAPENV_NAMESPACEW,
						cchLocalName, wszLocalName,
						cchNamespaceUri, wszNamespaceUri))
				{
					if (m_dwState != SOAP_ENVELOPE)
					{
						ATLTRACE( _T("ATLSOAP: CSoapRootHandler::startElement -- invalid SOAP message format: \"Headers\" in unexpected location.\r\n" ) );

						hr = E_FAIL;
					}

					m_dwState = SOAP_HEADERS;
				}
				else if (nElementHash == BODY &&
					IsEqualElement(sizeof(SOAP_BODYA)-1, SOAP_BODYW,
						sizeof(SOAPENV_NAMESPACEA)-1, SOAPENV_NAMESPACEW,
						cchLocalName, wszLocalName,
						cchNamespaceUri, wszNamespaceUri))
				{
					if (m_dwState == SOAP_START)
					{
						ATLTRACE( _T("ATLSOAP: CSoapRootHandler::startElement -- invalid SOAP message format: \"Body\" in unexpected location.\r\n" ) );

						hr = E_FAIL;
					}
					m_dwState = SOAP_BODY;
				}

				break;
			}
			case SOAP_BODY:
			{
				hr = DispatchSoapCall(wszNamespaceUri, cchNamespaceUri,
						wszLocalName, cchLocalName);

				m_dwState = SOAP_PARAMS;

				if (SUCCEEDED(hr))
				{
					if (GetState().pMap->dwCallFlags & SOAPFLAG_PAD)
					{
						hr = startElement(wszNamespaceUri, cchNamespaceUri,
								wszLocalName, cchLocalName, wszQName, cchQName,
								pAttributes);
					}
				}

				break;
			}

#ifdef _DEBUG

			default:
			{
				// should never get here -- internal error
				ATLASSERT( FALSE );
			}

#endif // _DEBUG
		}

		return hr;
	}

	HRESULT __stdcall characters( 
		 const wchar_t  *wszChars,
		 int cchChars)
	{
		m_bCharacters = false;

		// if it is a ready state, get the value
		if (m_stateStack.IsEmpty() == false)
		{
			ParseState& state = GetState();
			if ((state.dwFlags & SOAPFLAG_READYSTATE) &&
				((state.dwFlags & SOAPFLAG_SIZEIS)==0)) // don't marshal struct size_is elements -- should be filled in by array marshaling code
			{
				if ((state.pMap == NULL) || (state.pMap->mapType != SOAPMAP_ENUM))
				{
					return AtlSoapGetElementValue(wszChars, cchChars, 
						state.pvElement, (SOAPTYPES)state.pEntry->nVal, GetMemMgr());
				}
				else
				{
					// enum

					ATLASSERT( state.pMap != NULL );
					ATLASSERT( state.pMap->pEntries != NULL );

					ULONG nHash = AtlSoapHashStr(wszChars, cchChars);
					const _soapmapentry *pEntries = state.pMap->pEntries;

					size_t i;
					for (i=0; pEntries[i].nHash != 0; i++)
					{
						if ((nHash == pEntries[i].nHash) &&
							(cchChars == pEntries[i].cchField) &&
							(!wcsncmp(wszChars, pEntries[i].wszField, cchChars)))
						{
							break;
						}
					}

					if (pEntries[i].nHash != 0)
					{
						*((int *)state.pvElement) = pEntries[i].nVal;
						state.nElement++;
						return S_OK;
					}

					// no matching enum entry found
					ATLTRACE( _T("ATLSOAP: CSoapRootHandler::characters -- no matching enum entry found for: %.*ws.\r\n" ), cchChars, wszChars );

					return E_FAIL;
				}
			}
		}

		// otherwise, ignore

		return S_OK;
	}

	HRESULT __stdcall endElement( 
		 const wchar_t  * wszNamespaceUri,
		 int cchNamespaceUri,
		 const wchar_t  * wszLocalName,
		 int cchLocalName,
		 const wchar_t  * /*wszQName*/,
		 int /*cchQName*/)
	{
		static bool bDynArrWrapper = false;
		if (m_bCharacters)
		{			
			return E_FAIL;
		}

		m_bNullCheck = false;

		if (m_stateStack.IsEmpty() != false)
		{
			return S_OK;
		}

		if (!bDynArrWrapper && (m_nState > 1))
		{
			ParseState prevState = m_stateStack.GetAt(m_nState - 1);
			ParseState curState = m_stateStack.GetAt(m_nState);
			if (prevState.dwFlags & SOAPFLAG_DYNARRWRAPPER)
			{
				bDynArrWrapper = true;
				endElement (wszNamespaceUri, cchNamespaceUri, curState.pEntry->wszField,
				curState.pEntry->cchField, NULL, 0);
			}
		}
		else
		{
			bDynArrWrapper = false;
		}

		--m_nDepth;

		const ParseState& state = GetState();

		if ((m_dwState == SOAP_HEADERS) && (m_stateStack.GetCount() == 1))
		{
			return CheckEndHeaders(wszNamespaceUri, cchNamespaceUri, wszLocalName, cchLocalName);
		}

		if (state.dwFlags & (SOAPFLAG_FIXEDARR | SOAPFLAG_DYNARR))
		{
			if (state.dwFlags & SOAPFLAG_READYSTATE)
			{
				PopState();
			}

			const ParseState& currstate = GetState();
			ATLENSURE_RETURN( currstate.pEntry != NULL );

			if (m_nDepth == (currstate.nDepth-1))
			{
				if (S_OK != CheckEndElement(currstate))
				{
					// invalid number of elements
					ATLTRACE( _T("ATLSOAP: CSoapRootHandler::endElement -- invalid number of array elements for array parameter %.*ws.\r\n"), 
							  currstate.pEntry->cchField, currstate.pEntry->wszField );

					return E_FAIL;
				}

				PopState();
			}
		}
		else
		{
			if (S_OK != CheckEndElement(state))
			{
				return E_FAIL;
			}

			PopState();
		}

		return S_OK;
	}

	HRESULT SetClientStruct(void *pvParam, int nMapIndex)
	{
		ATLENSURE_RETURN( pvParam != NULL );
		ATLENSURE_RETURN( nMapIndex >= 0 );

		// this is the params struct
		// store for later use
		m_pvParam = pvParam;

		const _soapmap ** pEntries = GetHeaderMap();
		ATLENSURE_RETURN( pEntries != NULL );

		// push header value
		return PushState(GetHeaderValue(), pEntries[nMapIndex], NULL, 0, nMapIndex, pEntries[nMapIndex]->nElements);
	}

	void ResetClientState(bool bFull = false)
	{
		m_stateStack.RemoveAll();
		m_nState = 0;
		if (bFull != false)
		{
			m_dwState = SOAP_START;
			m_pvParam = NULL;
		}
	}

	HRESULT CreateReader()
	{
		return m_spReader.CoCreateInstance(ATLS_SAXXMLREADER_CLSID, NULL, CLSCTX_INPROC_SERVER);
	}

	HRESULT InitializeSOAP(IServiceProvider *pProvider)
	{
		HRESULT hr = S_OK;

		if (m_spReader.p == NULL)
		{
			hr = E_FAIL;
			if (pProvider != NULL)
			{
				IAtlMemMgr *pMemMgr = NULL;
				hr = pProvider->QueryService(__uuidof(IAtlMemMgr), 
					__uuidof(IAtlMemMgr), (void **)&pMemMgr);
				if ((SUCCEEDED(hr)) && (pMemMgr != NULL))
				{
					SetMemMgr(pMemMgr);
				}

				hr = pProvider->QueryService(__uuidof(ISAXXMLReader), 
					__uuidof(ISAXXMLReader), (void **)&m_spReader);
			}

			if (FAILED(hr))
			{
				hr = CreateReader();
			}
		}

		if (SUCCEEDED(hr))
		{
			hr = m_spReader->putContentHandler(this);
		}

#ifdef _DEBUG
		else
		{
			ATLTRACE( _T("ATLSOAP: CSoapRootHandler::InitializeSOAP -- failed to get SAXXMLReader.\r\n" ) );
		}
#endif // _DEBUG

		return hr;
	}

	void UninitializeSOAP()
	{
		if (m_spReader.p != NULL)
		{
			m_spReader->putContentHandler(NULL);
			m_spReader.Release();
		}
	}

	virtual HRESULT DispatchSoapCall(const wchar_t *wszNamespaceUri,
		int cchNamespaceUri, const wchar_t *wszLocalName,
		int cchLocalName)
	{
		HRESULT hr = S_OK;

		if (m_stateStack.IsEmpty() == false)
		{
			ATLASSUME( m_stateStack[0].pMap != NULL );

			// check to see if all required and non-nullable SOAP headers were sent
			if (m_stateStack[0].pMap->mapType == SOAPMAP_HEADER)
			{
				hr = CheckSoapHeaders(m_stateStack[0]);
			}
			if (SUCCEEDED(hr))
			{
				hr = SetSoapMapFromName(wszLocalName, cchLocalName, 
						wszNamespaceUri, cchNamespaceUri);
			}
		}
		else
		{
			// get the appropriate function map
			hr = SetSoapMapFromName(wszLocalName, cchLocalName, 
					wszNamespaceUri, cchNamespaceUri);

			if (SUCCEEDED(hr))
			{
				// set the SOAP Header map for the function
				ATLASSUME( m_stateStack.IsEmpty() == false );

				const _soapmap **ppHeaderMap = GetHeaderMap();
				ATLENSURE_RETURN( ppHeaderMap != NULL );

				// create a temporary parse state for checking headers
				ParseState state;
				state.pMap = ppHeaderMap[m_stateStack[0].nAllocSize];
				ATLENSURE_RETURN( state.pMap != NULL );

				// check to see if all required and non-nullable SOAP headers were sent
				hr = CheckSoapHeaders(state);
			}
		}

		return hr;
	}

	virtual HRESULT BeginParse(IStream *pStream)
	{
		ATLASSERT( pStream != NULL );

		CComVariant varStream;
		varStream = static_cast<IUnknown*>(pStream);

		HRESULT hr = m_spReader->parse(varStream);
		if (SUCCEEDED(hr))
		{
			if (m_refMap.GetCount() != 0)
			{
				hr = E_FAIL;
			}
		}
		return hr;
	}

	HRESULT CallFunctionInternal()
	{
		HRESULT hr = E_FAIL;
		const ParseState& state = m_stateStack[0];
		hr = CallFunction(
			state.pvElement, 
			state.pMap->wszName,
			state.pMap->cchWName,
			state.nAllocSize);

		return hr;
	}

	virtual HRESULT GenerateResponse(IWriteStream *pStream)
	{
		ATLASSUME( m_stateStack.IsEmpty() == false );
		ATLASSUME( m_stateStack[0].pMap != NULL );
		ATLASSUME( m_stateStack[0].pvElement != NULL );

		const ParseState& state = m_stateStack[0];

		const _soapmap *pHeaderMap = NULL;
		if (m_bClient == false)
		{
			const _soapmap **ppHeaderMap = GetHeaderMap();
			if (ppHeaderMap != NULL)
			{
				pHeaderMap = ppHeaderMap[state.nAllocSize];
			}
		}
		else
		{
			pHeaderMap = state.pMap;
		}

		const _soapmap *pFuncMap = NULL;
		if (m_bClient == false)
		{
			pFuncMap = state.pMap;
		}
		else
		{
			const _soapmap **ppFuncMap = GetFunctionMap();
			ATLENSURE_RETURN( ppFuncMap != NULL );
			pFuncMap = ppFuncMap[state.nAllocSize];
		}

		ATLENSURE_RETURN( pFuncMap != NULL );

		CRpcEncodedGenerator rpcGen;
		CPADGenerator padGen;
		CPIDGenerator pidGen;

		CResponseGenerator *pGenerator = NULL;

		if ((pFuncMap->dwCallFlags & (SOAPFLAG_RPC | SOAPFLAG_ENCODED)) == (SOAPFLAG_RPC | SOAPFLAG_ENCODED))
		{
			pGenerator = &rpcGen;
		}
		else if (pFuncMap->dwCallFlags & SOAPFLAG_PID)
		{
			ATLASSERT( (pFuncMap->dwCallFlags & (SOAPFLAG_DOCUMENT | SOAPFLAG_LITERAL)) == (SOAPFLAG_DOCUMENT | SOAPFLAG_LITERAL) );
			pGenerator = &pidGen;
		}
		else
		{
			ATLASSERT( (pFuncMap->dwCallFlags & (SOAPFLAG_DOCUMENT | SOAPFLAG_LITERAL)) == (SOAPFLAG_DOCUMENT | SOAPFLAG_LITERAL) );
			ATLASSERT( pFuncMap->dwCallFlags & SOAPFLAG_PAD );
			pGenerator = &padGen;
		}

		HRESULT hr = pGenerator->StartEnvelope(pStream);
		if (SUCCEEDED(hr))
		{
			// generate headers if necessary
			hr = GenerateHeaders(pGenerator, pHeaderMap, pStream);
			if (SUCCEEDED(hr))
			{
				hr = pGenerator->StartBody(pStream);
				if (SUCCEEDED(hr))
				{
					hr = GenerateResponseHelper(pGenerator, pFuncMap, m_pvParam, pStream, true);
					if (SUCCEEDED(hr))
					{
						hr = pGenerator->EndBody(pStream);
						if (SUCCEEDED(hr))
						{
							hr = pGenerator->EndEnvelope(pStream);
						}
					}
				}
			}
		}

		return hr;
	}

	virtual void Cleanup()
	{
		// cleanup headers
		CleanupHeaders();

		if ((m_stateStack.IsEmpty() == false) && (m_pvParam != NULL))
		{
			const _soapmap **ppFuncMap = GetFunctionMap();
			ATLENSURE( ppFuncMap != NULL );

			const _soapmap *pFuncMap = ppFuncMap[m_stateStack[0].nAllocSize];
			ATLENSURE( pFuncMap != NULL );

			CleanupHelper(pFuncMap, m_pvParam);
			if (m_bClient == false)
			{
				m_pMemMgr->Free(m_pvParam);
				m_stateStack.RemoveAll();
			}
		}
	}

	virtual void CleanupHeaders()
	{
		if (m_stateStack.IsEmpty() == false)
		{
			const _soapmap **ppHeaderMap = GetHeaderMap();
			ATLENSURE( ppHeaderMap != NULL );

			const _soapmap *pHeaderMap = ppHeaderMap[m_stateStack[0].nAllocSize];
			ATLENSURE( pHeaderMap != NULL );

			CleanupHelper(pHeaderMap, GetHeaderValue());
		}
	}

	void SetClient(bool bClient)
	{
		m_bClient = bClient;
	}

}; // class CSoapRootHandler

#define DECLARE_XSD_ENTRY( __name ) \
	{ L ## __name, __name, sizeof(__name)-1 },

__declspec(selectany) const CSoapRootHandler::XSDEntry CSoapRootHandler::s_xsdNames[] =
{
	DECLARE_XSD_ENTRY("string")
	DECLARE_XSD_ENTRY("boolean")
	DECLARE_XSD_ENTRY("float")
	DECLARE_XSD_ENTRY("double")
	DECLARE_XSD_ENTRY("decimal")
	DECLARE_XSD_ENTRY("duration")
	DECLARE_XSD_ENTRY("hexBinary")
	DECLARE_XSD_ENTRY("base64Binary")
	DECLARE_XSD_ENTRY("anyURI")
	DECLARE_XSD_ENTRY("ID")
	DECLARE_XSD_ENTRY("IDREF")
	DECLARE_XSD_ENTRY("ENTITY")
	DECLARE_XSD_ENTRY("NOTATION")
	DECLARE_XSD_ENTRY("QName")
	DECLARE_XSD_ENTRY("normalizedString")
	DECLARE_XSD_ENTRY("token")
	DECLARE_XSD_ENTRY("language")
	DECLARE_XSD_ENTRY("IDREFS")
	DECLARE_XSD_ENTRY("ENTITIES")
	DECLARE_XSD_ENTRY("NMTOKEN")
	DECLARE_XSD_ENTRY("NMTOKENS")
	DECLARE_XSD_ENTRY("Name")
	DECLARE_XSD_ENTRY("NCName")
	DECLARE_XSD_ENTRY("integer")
	DECLARE_XSD_ENTRY("nonPositiveInteger")
	DECLARE_XSD_ENTRY("negativeInteger")
	DECLARE_XSD_ENTRY("long")
	DECLARE_XSD_ENTRY("int")
	DECLARE_XSD_ENTRY("short")
	DECLARE_XSD_ENTRY("byte")
	DECLARE_XSD_ENTRY("nonNegativeInteger")
	DECLARE_XSD_ENTRY("unsignedLong")
	DECLARE_XSD_ENTRY("unsignedInt")
	DECLARE_XSD_ENTRY("unsignedShort")
	DECLARE_XSD_ENTRY("unsignedByte")
	DECLARE_XSD_ENTRY("positiveInteger")
	DECLARE_XSD_ENTRY("dateTime")
	DECLARE_XSD_ENTRY("time")
	DECLARE_XSD_ENTRY("date")
	DECLARE_XSD_ENTRY("gMonth")
	DECLARE_XSD_ENTRY("gYearMonth")
	DECLARE_XSD_ENTRY("gYear")
	DECLARE_XSD_ENTRY("gMonthDay")
	DECLARE_XSD_ENTRY("gDay")
};

__declspec(selectany) CCRTHeap CSoapRootHandler::m_crtHeap;

template <typename THandler>
class CSoapHandler : 
	public CSoapRootHandler, 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IRequestHandlerImpl<THandler>
{
protected:

	HTTP_CODE m_hcErr;
	CHttpResponse *m_pHttpResponse;

	// heap for SOAP requests
	CWin32Heap m_heap;

	// default heap is COM heap (SOAP Servers can double as COM objects)
	CComHeap m_comHeap;

public:

	BEGIN_COM_MAP(CSoapHandler<THandler>)
		COM_INTERFACE_ENTRY(ISAXContentHandler)
		COM_INTERFACE_ENTRY(IRequestHandler)
	END_COM_MAP()

	CSoapHandler()
		:m_pHttpResponse(NULL), m_hcErr(HTTP_SUCCESS)
	{
		SetMemMgr(&m_comHeap);
	}

	void SetHttpError(HTTP_CODE hcErr)
	{
		m_hcErr = hcErr;
	}

	HRESULT SoapFault(
		SOAP_ERROR_CODE errCode, 
		const wchar_t *wszDetail,
		int cchDetail)
	{
		ATLASSUME( m_pHttpResponse != NULL );

		SetHttpError(AtlsHttpError(500, SUBERR_NO_PROCESS));

		m_pHttpResponse->ClearHeaders();
		m_pHttpResponse->ClearContent();
		m_pHttpResponse->SetContentType("text/xml");
		m_pHttpResponse->SetStatusCode(500);

		CSoapFault fault;
		if (wszDetail != NULL)
		{
			if (cchDetail < 0)
			{
				cchDetail = (int) wcslen(wszDetail);
			}

			_ATLTRY
			{
				fault.m_strDetail.SetString(wszDetail, cchDetail);
			}
			_ATLCATCHALL()
			{
				ATLTRACE( _T("CSoapHandler::SoapFault -- out of memory.\r\n" ) );

				return E_OUTOFMEMORY;
			}
		}

		fault.m_soapErrCode = errCode;
		fault.GenerateFault(m_pHttpResponse);
		return S_OK;
	}

	HTTP_CODE InitializeHandler(AtlServerRequest *pRequestInfo, IServiceProvider *pProvider)
	{
		m_hcErr = IRequestHandlerImpl<THandler>::InitializeHandler(pRequestInfo, pProvider);
		if (m_hcErr == HTTP_SUCCESS)
		{
			HRESULT hr = InitializeSOAP(m_spServiceProvider);
			if (SUCCEEDED(hr))
			{
				// try to use the per-thread heap
				CIsapiWorker *pWorker = pRequestInfo->pExtension->GetThreadWorker();
				if (pWorker != NULL)
				{
					m_heap.Attach(pWorker->m_hHeap, false);
					SetMemMgr(&m_heap);
				}

				return m_hcErr;
			}
		}

		// some initialization failure
		CHttpResponse HttpResponse(pRequestInfo->pServerContext);
		m_pHttpResponse = &HttpResponse;

		SoapFault(SOAP_E_SERVER, NULL, 0);

		m_pHttpResponse = NULL;

		return m_hcErr;
	}

	HTTP_CODE HandleRequest(AtlServerRequest *pRequestInfo, IServiceProvider * /*pProvider*/)
	{
		// SOAPACTION header is required per the SOAP 1.1
		// mainly so firewalls can filter on it.
		char szBuf[ATL_URL_MAX_URL_LENGTH+1];
		szBuf[0] = '\0';
		DWORD dwLen = ATL_URL_MAX_URL_LENGTH;
		if ( m_spServerContext->GetServerVariable("HTTP_SOAPACTION", szBuf, &dwLen) != FALSE )
		{
			if ( dwLen >= 2 )
			{
				// drop the last "
				szBuf[dwLen-2] = '\0';
				char *szMethod = strrchr(szBuf, '#');
				if (szMethod != NULL)
				{
					_ATLTRY
					{
						// ignore return code here
						SetSoapMapFromName(CA2W( szMethod+1 ), -1, GetNamespaceUri(), -1, true);
					}
					_ATLCATCHALL()
					{
						return AtlsHttpError(500, ISE_SUBERR_OUTOFMEM);
					}
				}
			}
		}
		else
		{
			// SOAP requestion that use the HTTP transport
			// must have a SOAPACTION header.
			return HTTP_ERROR(500, ISE_SUBERR_SOAPNOSOAPACTION);
		}

		// set the header map
		CHttpResponse HttpResponse(pRequestInfo->pServerContext);
		m_pHttpResponse = &HttpResponse;

		CStreamOnServerContext s(pRequestInfo->pServerContext);

#ifdef _DEBUG

		CSAXSoapErrorHandler err;
		GetReader()->putErrorHandler(&err);

#endif // _DEBUG

		HRESULT hr = BeginParse(&s);

#ifdef _DEBUG
		// release the error handler
		GetReader()->putErrorHandler(NULL);
#endif // _DEBUG

		if (FAILED(hr))
		{
			Cleanup();
			if (m_hcErr == HTTP_SUCCESS)
			{
				SoapFault(SOAP_E_CLIENT, NULL, NULL);
			}

			return m_hcErr;
		}

		_ATLTRY
		{
			hr = CallFunctionInternal();
		}
		_ATLCATCHALL()
		{
			// cleanup before propagating user exception
			Cleanup();
			HttpResponse.Detach();
			_ATLRETHROW;
		}

		if (FAILED(hr))
		{
			Cleanup();
			HttpResponse.ClearHeaders();
			HttpResponse.ClearContent();
			if (m_hcErr != HTTP_SUCCESS)
			{
				HttpResponse.SetStatusCode(HTTP_ERROR_CODE(m_hcErr));
				return HTTP_SUCCESS_NO_PROCESS;
			}
			HttpResponse.SetStatusCode(500);
			GenerateAppError(&HttpResponse, hr);
			return AtlsHttpError(500, SUBERR_NO_PROCESS);
		}

		HttpResponse.SetContentType("text/xml");
		hr = GenerateResponse(&HttpResponse);
		Cleanup();
		if (FAILED(hr))
		{
			SoapFault(SOAP_E_SERVER, NULL, 0);
			return m_hcErr;
		}

		return HTTP_SUCCESS;
	}

	virtual ATL_NOINLINE HRESULT GenerateAppError(IWriteStream *pStream, HRESULT hr)
	{
		if (pStream == NULL)
		{
			return E_INVALIDARG;
		}

		LPWSTR pwszMessage = NULL;
		DWORD dwLen = ::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, hr, 0, (LPWSTR) &pwszMessage, 0, NULL);

		if (dwLen == 0)
		{
			pwszMessage = L"Application Error";
		}

		hr = SoapFault(SOAP_E_SERVER, pwszMessage, dwLen ? dwLen : -1);
		if (dwLen != 0)
		{
			::LocalFree(pwszMessage);
		}

		return hr;
	}

	void UninitializeHandler()
	{
		UninitializeSOAP();
	}
};


// client error states
enum SOAPCLIENT_ERROR
{
	SOAPCLIENT_SUCCESS=0,           // everything succeeded
	SOAPCLIENT_INITIALIZE_ERROR,    // initialization failed -- most likely an MSXML installation problem
	SOAPCLIENT_OUTOFMEMORY,         // out of memory
	SOAPCLIENT_GENERATE_ERROR,      // failed in generating the response
	SOAPCLIENT_CONNECT_ERROR,       // failed connecting to server
	SOAPCLIENT_SEND_ERROR,          // failed in sending message
	SOAPCLIENT_SERVER_ERROR,        // server error
	SOAPCLIENT_SOAPFAULT,           // a SOAP Fault was returned by the server
	SOAPCLIENT_PARSEFAULT_ERROR,    // failed in parsing SOAP fault
	SOAPCLIENT_READ_ERROR,          // failed in reading response
	SOAPCLIENT_PARSE_ERROR          // failed in parsing response
};

template <typename TSocketClass = ZEvtSyncSocket>
class CSoapSocketClientT
{
private:

	CUrl m_url;
	CWriteStreamOnCString m_writeStream;
	CReadStreamOnSocket<TSocketClass> m_readStream;
	DWORD m_dwTimeout;

	SOAPCLIENT_ERROR m_errorState;

protected:

	virtual HRESULT GetClientReader(ISAXXMLReader **pReader)
	{
		if (pReader == NULL)
		{
			return E_POINTER;
		}
		*pReader = NULL;

		CComPtr<ISAXXMLReader> spReader;
		HRESULT hr = spReader.CoCreateInstance(ATLS_SAXXMLREADER_CLSID, NULL, CLSCTX_INPROC_SERVER);
		if (SUCCEEDED(hr))
		{
			*pReader = spReader.Detach();
		}
		return hr;
	}

public:

	// note : not shared across stock client implementations
	CAtlHttpClientT<TSocketClass> m_socket;

	CSoapFault m_fault;

	// constructor
	CSoapSocketClientT(LPCTSTR szUrl)
		: m_dwTimeout(0), m_errorState(SOAPCLIENT_SUCCESS)
	{
		TCHAR szTmp[ATL_URL_MAX_URL_LENGTH];
		if(AtlEscapeUrl(szUrl,szTmp,0,ATL_URL_MAX_URL_LENGTH-1,ATL_URL_BROWSER_MODE))
			m_url.CrackUrl(szTmp);
	}

	CSoapSocketClientT(LPCTSTR szServer, LPCTSTR szUri, ATL_URL_PORT nPort=80)
		: m_dwTimeout(0), m_errorState(SOAPCLIENT_SUCCESS)
	{
		ATLASSERT( szServer != NULL );
		ATLASSERT( szUri != NULL );

		m_url.SetUrlPath(szUri);
		m_url.SetHostName(szServer);
		m_url.SetPortNumber(nPort);
	}

	~CSoapSocketClientT()
	{
		CleanupClient();
	}

	SOAPCLIENT_ERROR GetClientError()
	{
		return m_errorState;
	}

	void SetClientError(SOAPCLIENT_ERROR errorState)
	{
		m_errorState = errorState;
	}

	IWriteStream * GetWriteStream()
	{
		return &m_writeStream;
	}

	HRESULT GetReadStream(IStream **ppStream)
	{
		if (ppStream == NULL)
		{
			return E_POINTER;
		}

		*ppStream = &m_readStream;
		return S_OK;
	}

	void CleanupClient()
	{
		m_writeStream.Cleanup();
		m_fault.Clear();
		SetClientError(SOAPCLIENT_SUCCESS);
	}

	HRESULT SendRequest(LPCTSTR szAction)
	{
		HRESULT hr = E_FAIL;
		_ATLTRY
		{	
			// create extra headers to send with request
			CFixedStringT<CString, 256> strExtraHeaders(szAction);
			strExtraHeaders.Append(_T("Accept: text/xml\r\n"), sizeof("Accept: text/xml\r\n")-1);
			CAtlNavigateData navData;
			navData.SetMethod(ATL_HTTP_METHOD_POST);
			navData.SetPort(m_url.GetPortNumber());
			navData.SetExtraHeaders(strExtraHeaders);
			navData.SetPostData((LPBYTE)(LPCSTR) m_writeStream.m_str, m_writeStream.m_str.GetLength(), _T("text/xml; charset=utf-8"));

			ATLSOAP_TRACE( (LPBYTE)(LPCSTR)m_writeStream.m_str, m_writeStream.m_str.GetLength() );

			if (m_dwTimeout != 0)
			{
				navData.SetSocketTimeout(m_dwTimeout);
			}

			if (m_socket.Navigate(&m_url, &navData) != false)
			{
				if (GetStatusCode() == 200)
				{
					hr = (m_readStream.Init(&m_socket) != FALSE ? S_OK : E_FAIL);
					if (hr != S_OK)
					{
						SetClientError(SOAPCLIENT_READ_ERROR);
					}
				}
				else if (GetStatusCode() == 202)
				{
					// for one-way methods
					hr = S_OK;
				}
				else
				{
					SetClientError(SOAPCLIENT_SERVER_ERROR);
				}
			}
			else if (GetStatusCode() == 500)
			{
				SetClientError(SOAPCLIENT_SOAPFAULT);

				// if returned 500, get the SOAP fault
				if (m_readStream.Init(&m_socket) != FALSE)
				{
					CComPtr<ISAXXMLReader> spReader;
					if (SUCCEEDED(GetClientReader(&spReader)))
					{
						CComPtr<IStream> spReadStream;
						if (SUCCEEDED(GetReadStream(&spReadStream)))
						{
							if (FAILED(m_fault.ParseFault(spReadStream, spReader)))
							{
								SetClientError(SOAPCLIENT_PARSEFAULT_ERROR);
							}
						}
					}
				}
			}
			else
			{
				SetClientError(SOAPCLIENT_SEND_ERROR);
			}
		}
		_ATLCATCHALL()
		{
			hr = E_FAIL;
		}

		return hr;
	}

	HRESULT SetUrl(LPCTSTR szUrl)
	{
		TCHAR szTmp[ATL_URL_MAX_URL_LENGTH];
		if(!AtlEscapeUrl(szUrl,szTmp,0,ATL_URL_MAX_URL_LENGTH-1,ATL_URL_BROWSER_MODE))
		{
			return E_FAIL;
		}

		return (m_url.CrackUrl(szTmp) != FALSE) ? S_OK : E_FAIL;
	}

	HRESULT GetUrl(__out_ecount_part_z(*pdwLen, *pdwLen) LPTSTR szUrl, __inout LPDWORD pdwLen)
	{
		if ((szUrl == NULL) || (pdwLen == NULL))
		{
			return E_INVALIDARG;
		}

		return (m_url.CreateUrl(szUrl, pdwLen) != FALSE) ? S_OK : E_FAIL;
	}

	HRESULT SetProxy(LPCTSTR szProxy = NULL, short nProxyPort = 80)
	{
		BOOL bRet = m_socket.SetProxy(szProxy, nProxyPort);
		return (bRet != FALSE) ? S_OK : E_FAIL;
	}

	void SetTimeout(DWORD dwTimeout)
	{
		m_dwTimeout = dwTimeout;
	}

	int GetStatusCode()
	{
		return m_socket.GetStatus();
	}

}; // CSoapSocketClientT

#ifndef ATLSOAP_NOWININET

class CReadStreamOnInet : public IStreamImpl
{
public:

	HRESULT __stdcall QueryInterface(REFIID riid, void **ppv)
	{
		if (ppv == NULL)
		{
			return E_POINTER;
		}

		*ppv = NULL;

		if (InlineIsEqualGUID(riid, IID_IUnknown) ||
			InlineIsEqualGUID(riid, IID_IStream) ||
			InlineIsEqualGUID(riid, IID_ISequentialStream))
		{
			*ppv = static_cast<IStream *>(this);
			return S_OK;
		}

		return E_NOINTERFACE;
	}

	ULONG __stdcall AddRef()
	{
		return 1;
	}

	ULONG __stdcall Release()
	{
		return 1;
	}

private:

	HINTERNET m_hFile;

public:

	CReadStreamOnInet()
		:m_hFile(NULL)
	{
	}

	void Init(HINTERNET hFile)
	{
		m_hFile = hFile;
	}

	HRESULT STDMETHODCALLTYPE Read(void *pDest, ULONG dwMaxLen, ULONG *pdwRead)
	{
		BOOL bRet = InternetReadFile(m_hFile, pDest, dwMaxLen, pdwRead);
		return (bRet != FALSE) ? S_OK : E_FAIL;
	}

}; // CStreamOnInet

class CSoapWininetClient
{
private:

	CUrl m_url;
	CWriteStreamOnCString m_writeStream;
	CReadStreamOnInet m_readStream;
	CString m_strProxy;
	DWORD m_dwTimeout;
	CFixedStringT<CString, ATL_URL_MAX_URL_LENGTH+1> m_strUrl;
	SOAPCLIENT_ERROR m_errorState;

	void CloseAll()
	{
		if (m_hRequest != NULL)
		{
			InternetCloseHandle(m_hRequest);
			m_hRequest = NULL;
		}
		if (m_hConnection != NULL)
		{
			InternetCloseHandle(m_hConnection);
			m_hConnection = NULL;
		}
		if (m_hInternet != NULL)
		{
			InternetCloseHandle(m_hInternet);
			m_hInternet = NULL;
		}
	}

	HRESULT ConnectToServer()
	{
		if (m_hConnection != NULL)
		{
			return S_OK;
		}

		m_hInternet = InternetOpen(
			ATLSOAPINET_CLIENT, 
			m_strProxy.GetLength() ? (INTERNET_OPEN_TYPE_PRECONFIG | INTERNET_OPEN_TYPE_PROXY) : INTERNET_OPEN_TYPE_PRECONFIG,
			m_strProxy.GetLength() ? (LPCTSTR) m_strProxy : NULL,
			NULL, 0);

		if (m_hInternet != NULL)
		{
			if (m_dwTimeout != 0)
			{
				InternetSetOption(m_hInternet, INTERNET_OPTION_CONNECT_TIMEOUT,
					&m_dwTimeout, sizeof(m_dwTimeout));
				InternetSetOption(m_hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT,
					&m_dwTimeout, sizeof(m_dwTimeout));
				InternetSetOption(m_hInternet, INTERNET_OPTION_SEND_TIMEOUT,
					&m_dwTimeout, sizeof(m_dwTimeout));
			}
			m_hConnection = InternetConnect(m_hInternet, m_url.GetHostName(), 
				(INTERNET_PORT) m_url.GetPortNumber(), NULL, NULL,
				INTERNET_SERVICE_HTTP, 0, NULL);

			if (m_hConnection != NULL)
			{
				return S_OK;
			}
		}
		CloseAll();
		return E_FAIL;
	}

protected:

	virtual HRESULT GetClientReader(ISAXXMLReader **pReader)
	{
		if (pReader == NULL)
		{
			return E_POINTER;
		}
		*pReader = NULL;

		CComPtr<ISAXXMLReader> spReader;
		HRESULT hr = spReader.CoCreateInstance(ATLS_SAXXMLREADER_CLSID, NULL, CLSCTX_INPROC_SERVER);
		if (SUCCEEDED(hr))
		{
			*pReader = spReader.Detach();
		}
		return hr;
	}

public:

	// note : not shared across stock client implementations
	HINTERNET m_hInternet;
	HINTERNET m_hConnection;
	HINTERNET m_hRequest;

	CSoapFault m_fault;

	CSoapWininetClient(LPCTSTR szUrl)
		:m_hInternet(NULL), m_hConnection(NULL), m_hRequest(NULL), m_dwTimeout(0), m_errorState(SOAPCLIENT_SUCCESS)
	{
		TCHAR szTmp[ATL_URL_MAX_URL_LENGTH];
		if(AtlEscapeUrl(szUrl,szTmp,0,ATL_URL_MAX_URL_LENGTH-1,ATL_URL_BROWSER_MODE))
		{
			if (m_url.CrackUrl(szTmp) != FALSE)
			{
				SetProxy();
				_ATLTRY
				{
					m_strUrl.SetString(m_url.GetUrlPath(), m_url.GetUrlPathLength());
					m_strUrl.Append(m_url.GetExtraInfo(), m_url.GetExtraInfoLength());
				}
				_ATLCATCHALL()
				{
				}
			}
		}
	}

	CSoapWininetClient(LPCTSTR szServer, LPCTSTR szUri, short nPort=80)
		:m_hInternet(NULL), m_hConnection(NULL), m_hRequest(NULL), m_dwTimeout(0), m_errorState(SOAPCLIENT_SUCCESS)
	{
		if (m_url.SetHostName(szServer) != FALSE)
		{
			if (m_url.SetUrlPath(szUri) != FALSE)
			{
				if (m_url.SetPortNumber((ATL_URL_PORT) nPort) != FALSE)
				{
					_ATLTRY
					{
						m_strUrl.SetString(m_url.GetUrlPath(), m_url.GetUrlPathLength());
						m_strUrl.Append(m_url.GetExtraInfo(), m_url.GetExtraInfoLength());
					}
					_ATLCATCHALL()
					{
					}
				}
			}
		}
	}

	virtual ~CSoapWininetClient()
	{
		CleanupClient();
		CloseAll();
	}

	SOAPCLIENT_ERROR GetClientError()
	{
		return m_errorState;
	}

	void SetClientError(SOAPCLIENT_ERROR errorState)
	{
		m_errorState = errorState;
	}

	IWriteStream * GetWriteStream()
	{
		return &m_writeStream;
	}

	HRESULT GetReadStream(IStream **ppStream)
	{
		if (ppStream == NULL)
		{
			return E_POINTER;
		}

		*ppStream = &m_readStream;
		return S_OK;
	}

	void CleanupClient()
	{
		m_writeStream.Cleanup();
		if (m_hRequest != NULL)
		{
			InternetCloseHandle(m_hRequest);
			m_hRequest = NULL;
		}
		m_fault.Clear();
		SetClientError(SOAPCLIENT_SUCCESS);
	}

	HRESULT SendRequest(LPCTSTR szAction)
	{
		if (ConnectToServer() != S_OK)
		{
			SetClientError(SOAPCLIENT_CONNECT_ERROR);
			return E_FAIL;
		}

		CString strHeaders;
		_ATLTRY
		{
			strHeaders.Append(szAction);
			strHeaders.Append(_T("Content-Type: text/xml; charset=utf-8\r\n"));
		}
		_ATLCATCHALL()
		{
			return E_OUTOFMEMORY;
		}

		static LPCTSTR s_szAcceptTypes[] = { _T("text/*"), NULL };
		m_hRequest = HttpOpenRequest(m_hConnection, _T("POST"), 
			m_strUrl, _T("HTTP/1.0"), NULL,
			s_szAcceptTypes, 
			INTERNET_FLAG_NO_UI | INTERNET_FLAG_KEEP_CONNECTION | ((m_url.GetScheme() == ATL_URL_SCHEME_HTTPS) ? INTERNET_FLAG_SECURE : 0)
			, NULL);

		if (m_hRequest != NULL)
		{
			if (FALSE != HttpSendRequest(m_hRequest, strHeaders, (DWORD) strHeaders.GetLength(),		
				(void *)(LPCSTR)m_writeStream.m_str, m_writeStream.m_str.GetLength()))
			{
				m_readStream.Init(m_hRequest);
				if (GetStatusCode() != HTTP_STATUS_SERVER_ERROR)
				{
					return S_OK;
				}
				else
				{
					SetClientError(SOAPCLIENT_SOAPFAULT);

					CComPtr<ISAXXMLReader> spReader;
					if (SUCCEEDED(GetClientReader(&spReader)))
					{
						CComPtr<IStream> spReadStream;
						if (SUCCEEDED(GetReadStream(&spReadStream)))
						{
							if (FAILED(m_fault.ParseFault(spReadStream, spReader)))
							{
								SetClientError(SOAPCLIENT_PARSEFAULT_ERROR);
							}
						}
					}
				}
			}
		}
		else
		{
			SetClientError(SOAPCLIENT_SEND_ERROR);
		}

		return E_FAIL;
	}

	HRESULT SetUrl(LPCTSTR szUrl)
	{
		CloseAll();
		TCHAR szTmp[ATL_URL_MAX_URL_LENGTH];
		if(!AtlEscapeUrl(szUrl,szTmp,0,ATL_URL_MAX_URL_LENGTH-1,ATL_URL_BROWSER_MODE))
		{
			return E_FAIL;
		}

		if (m_url.CrackUrl(szTmp) != FALSE)
		{
			_ATLTRY
			{
				m_strUrl.SetString(m_url.GetUrlPath(), m_url.GetUrlPathLength());
				m_strUrl.Append(m_url.GetExtraInfo(), m_url.GetExtraInfoLength());
			}
			_ATLCATCHALL()
			{
				return E_OUTOFMEMORY;
			}
			return S_OK;
		}
		return E_FAIL;
	}

	HRESULT GetUrl(LPTSTR szUrl, LPDWORD pdwLen)
	{
		if ((szUrl == NULL) || (pdwLen == NULL))
		{
			return E_INVALIDARG;
		}

		return (m_url.CreateUrl(szUrl, pdwLen) != FALSE) ? S_OK : E_FAIL;
	}

	HRESULT SetProxy(LPCTSTR szProxy = NULL, short nProxyPort = 80)
	{
		_ATLTRY
		{
			if (szProxy && szProxy[0])
			{
				m_strProxy.Format(_T("http=http://%s:%d https=http://%s:%d"), szProxy, nProxyPort, szProxy, nProxyPort);
			}
			else
			{
				m_strProxy.Empty();
			}
		}
		_ATLCATCHALL()
		{
			return E_OUTOFMEMORY;
		}

		return S_OK;
	}

	void SetTimeout(DWORD dwTimeout)
	{
		m_dwTimeout = dwTimeout;
	}

	int GetStatusCode()
	{
		DWORD dwLen = 255;
		TCHAR szBuf[256];
		if (HttpQueryInfo(m_hRequest, HTTP_QUERY_STATUS_CODE, szBuf, &dwLen, NULL))
		{
			szBuf[dwLen] = '\0';
			return _ttoi(szBuf);
		}
		return 0;
	}
}; // CSoapWininetClient
#endif

#ifndef ATLSOAP_NOMSXML_INET
class CSoapMSXMLInetClient
{
private:

	CUrl m_url;
	CWriteStreamOnCString m_writeStream;
	DWORD m_dwTimeout;
	SOAPCLIENT_ERROR m_errorState;

	HRESULT ConnectToServer()
	{
		TCHAR szURL[ATL_URL_MAX_URL_LENGTH];
		DWORD dwLen = ATL_URL_MAX_URL_LENGTH;
		HRESULT hr = E_FAIL;

		if (m_spHttpRequest)
			return S_OK;

		if (!m_url.CreateUrl(szURL, &dwLen))
			return E_FAIL;


		hr = m_spHttpRequest.CoCreateInstance(__uuidof(ServerXMLHTTP30));
		if (hr != S_OK)
			return hr;

		CComVariant vEmpty;
		hr = m_spHttpRequest->open( CComBSTR(L"POST"),
									CComBSTR(szURL),
									CComVariant(VARIANT_BOOL(VARIANT_FALSE)),
									vEmpty,
									vEmpty );
		if (hr != S_OK)
		{
			m_spHttpRequest.Release();
		   return hr;
		}

		return S_OK;
	}

protected:

	virtual HRESULT GetClientReader(ISAXXMLReader **pReader)
	{
		if (pReader == NULL)
		{
			return E_POINTER;
		}
		*pReader = NULL;

		CComPtr<ISAXXMLReader> spReader;
		HRESULT hr = spReader.CoCreateInstance(ATLS_SAXXMLREADER_CLSID, NULL, CLSCTX_INPROC_SERVER);
		if (SUCCEEDED(hr))
		{
			*pReader = spReader.Detach();
		}
		return hr;
	}

public:

	// note : not shared across stock client implementations
	CComPtr<IServerXMLHTTPRequest> m_spHttpRequest;

	CSoapFault m_fault;

	CSoapMSXMLInetClient(LPCTSTR szUrl)
		:m_dwTimeout(0), m_errorState(SOAPCLIENT_SUCCESS)
	{
		m_url.CrackUrl(szUrl);
	}

	CSoapMSXMLInetClient(LPCTSTR szServer, LPCTSTR szUri, short nPort=80)
		: m_dwTimeout(0), m_errorState(SOAPCLIENT_SUCCESS)
	{
		m_url.SetHostName(szServer);
		m_url.SetUrlPath(szUri);
		m_url.SetPortNumber((ATL_URL_PORT) nPort);
	}

	virtual ~CSoapMSXMLInetClient()
	{
		CleanupClient();
	}

	SOAPCLIENT_ERROR GetClientError()
	{
		return m_errorState;
	}

	void SetClientError(SOAPCLIENT_ERROR errorState)
	{
		m_errorState = errorState;
	}

	IWriteStream * GetWriteStream()
	{
		return &m_writeStream;
	}

	HRESULT GetReadStream(IStream **ppStream)
	{
		if (ppStream == NULL)
		{
			return E_POINTER;
		}

		*ppStream = NULL;
		HRESULT hr = E_FAIL;

		if (m_spHttpRequest)
		{
			VARIANT vResponseStream;
			VariantInit(&vResponseStream);
			hr = m_spHttpRequest->get_responseStream(&vResponseStream);
			if (S_OK == hr)
			{
				hr = E_FAIL;
				if ((vResponseStream.vt == VT_UNKNOWN) && (vResponseStream.punkVal != NULL))
				{
					// we return the refcount with the pointer!
					hr = vResponseStream.punkVal->QueryInterface(__uuidof(IStream), (void **)ppStream);
				}
				else
				{
					SetClientError(SOAPCLIENT_READ_ERROR);
				}
			}
			VariantClear(&vResponseStream);
		}
		return hr;
	}

	void CleanupClient()
	{
		m_writeStream.Cleanup();
		m_spHttpRequest.Release();
		m_fault.Clear();
		SetClientError(SOAPCLIENT_SUCCESS);
	}

	HRESULT SendRequest(LPCTSTR szAction)
	{
		if (ConnectToServer() != S_OK)
		{
			SetClientError(SOAPCLIENT_CONNECT_ERROR);
			return E_FAIL;
		}

		// set the action header
		LPCTSTR szColon = _tcschr(szAction, _T(':'));
		if (szColon != NULL)
		{
			do
			{
				szColon++;
			} while (_istspace(static_cast<unsigned char>(*szColon)));

			if (FAILED(m_spHttpRequest->setRequestHeader(
						CComBSTR( L"SOAPAction" ), CComBSTR( szColon ))))
			{
				SetClientError(SOAPCLIENT_SEND_ERROR);
				return E_FAIL;
			}
		} // if SOAPAction header not properly formed, attempt to send anyway

		if (FAILED(m_spHttpRequest->setRequestHeader(CComBSTR( L"Content-Type" ), CComBSTR(L"text/xml; charset=utf-8"))))
		{
			SetClientError(SOAPCLIENT_SEND_ERROR);
			return E_FAIL;
		}

		// set timeout
		if (m_dwTimeout != 0)
		{
			long nTimeout = (long) m_dwTimeout;
			m_spHttpRequest->setTimeouts(nTimeout, nTimeout, nTimeout, nTimeout);
			// reset timeout
			m_dwTimeout = 0;
		}

		CComVariant vBody(m_writeStream.m_str);
		HRESULT hr = m_spHttpRequest->send(vBody);
		if ((SUCCEEDED(hr)) && (GetStatusCode() == 500))
		{
			hr = E_FAIL;
			CComPtr<ISAXXMLReader> spReader;
			if (SUCCEEDED(GetClientReader(&spReader)))
			{
				SetClientError(SOAPCLIENT_SOAPFAULT);

				CComPtr<IStream> spReadStream;
				if (SUCCEEDED(GetReadStream(&spReadStream)))
				{
					if (FAILED(m_fault.ParseFault(spReadStream, spReader)))
					{
						SetClientError(SOAPCLIENT_PARSEFAULT_ERROR);
					}
				}
			}
		}
		else if (FAILED(hr))
		{
			SetClientError(SOAPCLIENT_SEND_ERROR);
		}

		return hr;
	}

	HRESULT SetUrl(LPCTSTR szUrl)
	{
		CleanupClient();
		return (m_url.CrackUrl(szUrl) != FALSE ? S_OK : E_FAIL);
	}

	HRESULT GetUrl(LPTSTR szUrl, LPDWORD pdwLen)
	{
		if ((szUrl == NULL) || (pdwLen == NULL))
		{
			return E_INVALIDARG;
		}

		return (m_url.CreateUrl(szUrl, pdwLen) != FALSE) ? S_OK : E_FAIL;
	}

	void SetTimeout(DWORD dwTimeout)
	{
		m_dwTimeout = dwTimeout;
	}

	int GetStatusCode()
	{
		long lStatus;
		if (m_spHttpRequest->get_status(&lStatus) == S_OK)
		{
			return (int) lStatus;
		}
		return 0;
	}

	HRESULT SetProxy(LPCTSTR szProxy = NULL, short nProxyPort = 80)
	{
		(szProxy);
		(nProxyPort);

		ATLTRACE( _T("CSoapMSXMLInetClient does not support SetProxy") );

		return S_OK;
	}
}; // CSoapMSXMLInetClient
#endif


class _CSDLGenerator : public ITagReplacerImpl<_CSDLGenerator>
{
private:

	typedef CAtlMap<CStringA, const _soapmap *, CStringElementTraits<CStringA> >  WSDLMAP;
	typedef CAtlMap<CStringA, const _soapmapentry *, CStringElementTraits<CStringA> > HEADERMAP;

	HRESULT GenerateWSDLHelper(const _soapmap *pMap, WSDLMAP& structMap, WSDLMAP& enumMap)
	{
		ATLENSURE_RETURN( pMap != NULL );

		const _soapmapentry *pEntries = pMap->pEntries;
		ATLENSURE_RETURN( pEntries != NULL );

		HRESULT hr = S_OK;

		for (int i=0; pEntries[i].nHash != 0; i++)
		{
			if (pEntries[i].nVal == SOAPTYPE_UNK)
			{
				ATLENSURE_RETURN( pEntries[i].pChain != NULL );

				_ATLTRY
				{
					POSITION pos = NULL;
					CStringA strName(pEntries[i].pChain->szName, pEntries[i].pChain->cchName);
					if (pEntries[i].pChain->mapType == SOAPMAP_STRUCT)
					{
						pos = structMap.SetAt(strName, pEntries[i].pChain);
					}
					else if (pEntries[i].pChain->mapType == SOAPMAP_ENUM)
					{
						pos = enumMap.SetAt(strName, pEntries[i].pChain);
					}
					if (pos == NULL)
					{
						hr = E_OUTOFMEMORY;
						break;
					}
				}
				_ATLCATCHALL()
				{
					hr = E_OUTOFMEMORY;
					break;
				}

				hr = GenerateWSDLHelper(pEntries[i].pChain, structMap, enumMap);
				if (FAILED(hr))
				{
					break;
				}
			}
		}

		return hr;
	}

	HTTP_CODE IsUDT(const _soapmapentry *pEntry)
	{
		ATLENSURE( pEntry != NULL );
		return (pEntry->nVal != SOAPTYPE_UNK) ? HTTP_S_FALSE : HTTP_SUCCESS;
	}

	HTTP_CODE GetSoapDims(const _soapmapentry *pEntry)
	{
		ATLENSURE( pEntry != NULL );
		if (pEntry->pDims[0] != 0)
		{
			if (SUCCEEDED(m_pWriteStream->WriteStream("[", 1, NULL)))
			{
				for (int i=1; i<=pEntry->pDims[0]; i++)
				{
					if (m_writeHelper.Write(pEntry->pDims[i]) != FALSE)
					{
						if (i < pEntry->pDims[0])
						{
							if (FAILED(m_pWriteStream->WriteStream(", ", 2, NULL)))
							{
								return HTTP_FAIL;
							}
						}
					}
				}
				if (SUCCEEDED(m_pWriteStream->WriteStream("]", 1, NULL)))
				{
					return HTTP_SUCCESS;
				}
			}
		}
		return HTTP_FAIL;
	}

	const _soapmap **m_pFuncs;
	const _soapmap **m_pHeaders;
	int m_nFunc;
	int m_nParam;
	int m_nHeader;
	WSDLMAP m_structMap;
	WSDLMAP m_enumMap;
	POSITION m_currUDTPos;
	int m_nCurrUDTField;

	HEADERMAP m_headerMap;
	POSITION m_currHeaderPos;

	CWriteStreamHelper m_writeHelper;

	CStringA m_strServiceName;
	CStringA m_strNamespaceUri;

	IWriteStream *m_pWriteStream;

	CComPtr<IHttpServerContext> m_spHttpServerContext;

	DWORD m_dwCallFlags;

protected:

	void SetWriteStream(IWriteStream *pStream)
	{
		m_pWriteStream = pStream;
		m_writeHelper.Attach(m_pWriteStream);
	}

	void SetHttpServerContext(IHttpServerContext *pServerContext)
	{
		m_spHttpServerContext = pServerContext;
	}

	static HTTP_CODE GetSoapType(int nVal, IWriteStream *pStream)
	{
		return (pStream->WriteStream(CSoapRootHandler::s_xsdNames[nVal].szName, 
			CSoapRootHandler::s_xsdNames[nVal].cchName, NULL) == S_OK) ? HTTP_SUCCESS : HTTP_FAIL;
	}


	HRESULT InitializeSDL(CSoapRootHandler *pHdlr)
	{
		m_pFuncs = pHdlr->GetFunctionMap();

		if (m_pFuncs == NULL)
		{
			return E_FAIL;
		}

		ATLASSUME( m_pFuncs[0] != NULL );

		m_dwCallFlags = m_pFuncs[0]->dwCallFlags;

		size_t i;
		for (i=0; m_pFuncs[i] != NULL; i++)
		{
			const _soapmap *pMap = m_pFuncs[i];
			HRESULT hr = GenerateWSDLHelper(pMap, m_structMap, m_enumMap);
			if (FAILED(hr))
			{
				return hr;
			}
		}

		m_pHeaders = pHdlr->GetHeaderMap();
		if (m_pHeaders != NULL)
		{
			for (i=0; m_pHeaders[i] != NULL; i++)
			{
				const _soapmap *pMap = m_pHeaders[i];
				HRESULT hr = GenerateWSDLHelper(pMap, m_structMap, m_enumMap);
				if (FAILED(hr))
				{
					return hr;
				}
			}

			for (i=0; m_pHeaders[i] != NULL; i++)
			{
				const _soapmap *pMap = m_pHeaders[i];
				for (size_t j=0; pMap->pEntries[j].nHash != 0; j++)
				{
					HRESULT hr = S_OK;
					_ATLTRY
					{
						if (m_headerMap.SetAt(pMap->pEntries[j].szField, &pMap->pEntries[j]) == NULL)
						{
							hr = E_OUTOFMEMORY;
						}
					}
					_ATLCATCHALL()
					{
						hr = E_OUTOFMEMORY;
					}
					if (FAILED(hr))
					{
						return hr;
					}
				}
			}
		}

		_ATLTRY
		{
			m_strServiceName = pHdlr->GetServiceName();
			m_strNamespaceUri = pHdlr->GetNamespaceUriA();
		}
		_ATLCATCHALL()
		{
			return E_OUTOFMEMORY;
		}
		return S_OK;
	}

	virtual const char * GetHandlerName() = 0;

public:

	_CSDLGenerator()
		:m_pFuncs(NULL), m_nFunc(-1), m_nParam(-1),
		 m_currUDTPos(NULL), m_nCurrUDTField(-1),
		 m_pWriteStream(NULL), m_nHeader(-1), m_currHeaderPos(NULL)
	{
	}
	virtual ~_CSDLGenerator()
	{
	}

	HTTP_CODE OnGetURL()
	{
		char szURL[ATL_URL_MAX_URL_LENGTH];
		DWORD dwUrlSize = sizeof(szURL);
		char szServer[ATL_URL_MAX_HOST_NAME_LENGTH];
		DWORD dwServerSize = sizeof(szServer);
		char szHttps[16];
		DWORD dwHttpsLen = sizeof(szHttps);
		char szPort[ATL_URL_MAX_PORT_NUMBER_LENGTH+1];
		DWORD dwPortLen = sizeof(szPort);

		if (m_spHttpServerContext->GetServerVariable("URL", szURL, &dwUrlSize) != FALSE)
		{
			if (m_spHttpServerContext->GetServerVariable("SERVER_NAME", szServer, &dwServerSize) != FALSE)
			{
				bool bHttps = false;
				if ((m_spHttpServerContext->GetServerVariable("HTTPS", szHttps, &dwHttpsLen) != FALSE) &&
					(!_stricmp(szHttps, "ON")))
				{
					bHttps = true;
				}

				if (m_spHttpServerContext->GetServerVariable("SERVER_PORT", szPort, &dwPortLen) != FALSE)
				{
					_ATLTRY
					{
						CStringA strUrl;
						strUrl.Format("http%s://%s:%s%s?Handler=%s", bHttps ? "s" : "", szServer, szPort, szURL, GetHandlerName());
						
						CA2W wszUrl(strUrl);
						wchar_t	*pwszUrl = wszUrl;
						HRESULT	hr = AtlGenXMLValue(m_pWriteStream, &pwszUrl);
						return SUCCEEDED(hr) ? HTTP_SUCCESS : HTTP_FAIL;
					}
					_ATLCATCHALL()
					{
						return HTTP_FAIL;
					}
				}
			}
		}
		return HTTP_FAIL;
	}

	HTTP_CODE OnGetNamespace()
	{
		return (m_pWriteStream->WriteStream(m_strNamespaceUri, 
			m_strNamespaceUri.GetLength(), NULL) == S_OK) ? HTTP_SUCCESS : HTTP_FAIL;
	}

	HTTP_CODE OnGetNextFunction()
	{
		m_nFunc++;
		if (m_pFuncs[m_nFunc] == NULL)
		{
			m_nFunc = -1;
			return HTTP_S_FALSE;
		}
		return HTTP_SUCCESS;
	}

	HTTP_CODE OnGetFunctionName()
	{
		return (m_pWriteStream->WriteStream(m_pFuncs[m_nFunc]->szName, 
			m_pFuncs[m_nFunc]->cchName, NULL) == S_OK) ? HTTP_SUCCESS : HTTP_S_FALSE; 
	}

	HTTP_CODE OnGetNextParameter()
	{
		++m_nParam;
		if (m_pFuncs[m_nFunc]->pEntries[m_nParam].nHash != 0)
		{
			if (m_pFuncs[m_nFunc]->pEntries[m_nParam].dwFlags & SOAPFLAG_NOMARSHAL)
			{
				return OnGetNextParameter();
			}
			return HTTP_SUCCESS;
		}
		m_nParam = -1;
		return HTTP_S_FALSE;
	}

	HTTP_CODE OnIsInParameter()
	{
		return (m_pFuncs[m_nFunc]->pEntries[m_nParam].dwFlags & SOAPFLAG_IN) ? HTTP_SUCCESS : HTTP_S_FALSE;
	}

	HTTP_CODE OnGetParameterName()
	{
		HRESULT hr = S_OK;
		if (m_pFuncs[m_nFunc]->pEntries[m_nParam].dwFlags & SOAPFLAG_RETVAL)
		{
			hr = m_pWriteStream->WriteStream("return", sizeof("return")-1, NULL);
		}
		else
		{
			hr = m_pWriteStream->WriteStream(m_pFuncs[m_nFunc]->pEntries[m_nParam].szField, 
					m_pFuncs[m_nFunc]->pEntries[m_nParam].cchField, NULL);
		}

		return (hr == S_OK) ? HTTP_SUCCESS : HTTP_FAIL; 
	}

	HTTP_CODE OnNotIsArrayParameter()
	{
		return (m_pFuncs[m_nFunc]->pEntries[m_nParam].dwFlags & (SOAPFLAG_FIXEDARR | SOAPFLAG_DYNARR)) 
			? HTTP_S_FALSE: HTTP_SUCCESS;
	}

	HTTP_CODE OnIsParameterUDT()
	{
		return IsUDT(&m_pFuncs[m_nFunc]->pEntries[m_nParam]);
	}

	HTTP_CODE OnGetParameterSoapType()
	{
		if (m_pFuncs[m_nFunc]->pEntries[m_nParam].nVal != SOAPTYPE_UNK)
		{
			return GetSoapType(m_pFuncs[m_nFunc]->pEntries[m_nParam].nVal, m_pWriteStream);
		}
		ATLASSUME( m_pFuncs[m_nFunc]->pEntries[m_nParam].pChain != NULL );
		return (m_pWriteStream->WriteStream(m_pFuncs[m_nFunc]->pEntries[m_nParam].pChain->szName, 
			m_pFuncs[m_nFunc]->pEntries[m_nParam].pChain->cchName, NULL) == S_OK) ? HTTP_SUCCESS : HTTP_S_FALSE; 
	}

	HTTP_CODE OnIsParameterDynamicArray()
	{
		return (m_pFuncs[m_nFunc]->pEntries[m_nParam].dwFlags & SOAPFLAG_DYNARR) ? HTTP_SUCCESS: HTTP_S_FALSE;
	}

	HTTP_CODE OnIsArrayParameter()
	{
		return (OnNotIsArrayParameter() != HTTP_SUCCESS) ? HTTP_SUCCESS : HTTP_S_FALSE;
	}

	HTTP_CODE OnIsParameterOneDimensional()
	{
		return (m_pFuncs[m_nFunc]->pEntries[m_nParam].pDims[0] == 1) ? HTTP_SUCCESS : HTTP_S_FALSE;
	}

	HTTP_CODE OnGetParameterArraySize()
	{
		return (m_writeHelper.Write(m_pFuncs[m_nFunc]->pEntries[m_nParam].pDims[1]) != FALSE)
			? HTTP_SUCCESS : HTTP_FAIL;
	}

	HTTP_CODE OnGetParameterArraySoapDims()
	{
		return GetSoapDims(&m_pFuncs[m_nFunc]->pEntries[m_nParam]);
	}

	HTTP_CODE OnIsOutParameter()
	{
		return (m_pFuncs[m_nFunc]->pEntries[m_nParam].dwFlags & SOAPFLAG_OUT) ? HTTP_SUCCESS : HTTP_S_FALSE;
	}

	HTTP_CODE OnGetNextEnum()
	{
		if (m_currUDTPos == NULL)
		{
			m_currUDTPos = m_enumMap.GetStartPosition();
		}
		else
		{
			m_enumMap.GetNext(m_currUDTPos);
		}

		return (m_currUDTPos != NULL) ? HTTP_SUCCESS : HTTP_S_FALSE;
	}

	HTTP_CODE OnGetEnumName()
	{
		const _soapmap *pMap = m_enumMap.GetValueAt(m_currUDTPos);
		return (m_pWriteStream->WriteStream(pMap->szName, pMap->cchName, NULL) == S_OK) ? HTTP_SUCCESS : HTTP_S_FALSE;
	}

	HTTP_CODE OnGetNextEnumElement()
	{
		const _soapmap *pMap = m_enumMap.GetValueAt(m_currUDTPos);
		++m_nCurrUDTField;
		if (pMap->pEntries[m_nCurrUDTField].nHash != 0)
		{
			return HTTP_SUCCESS;
		}
		m_nCurrUDTField = -1;
		return HTTP_S_FALSE;
	}

	HTTP_CODE OnGetEnumElementName()
	{
		const _soapmap *pMap = m_enumMap.GetValueAt(m_currUDTPos);
		return (m_pWriteStream->WriteStream(pMap->pEntries[m_nCurrUDTField].szField,
			pMap->pEntries[m_nCurrUDTField].cchField, NULL) == S_OK) ? HTTP_SUCCESS : HTTP_S_FALSE;
	}

	HTTP_CODE OnGetNextStruct()
	{
		if (m_currUDTPos == NULL)
		{
			m_currUDTPos = m_structMap.GetStartPosition();
		}
		else
		{
			m_structMap.GetNext(m_currUDTPos);
		}

		return (m_currUDTPos != NULL) ? HTTP_SUCCESS : HTTP_S_FALSE;
	}

	HTTP_CODE OnGetStructName()
	{
		const _soapmap *pMap = m_enumMap.GetValueAt(m_currUDTPos);
		return (m_pWriteStream->WriteStream(pMap->szName, pMap->cchName, NULL) == S_OK) ? HTTP_SUCCESS : HTTP_S_FALSE;
	}

	HTTP_CODE OnGetNextStructField()
	{
		const _soapmap *pMap = m_structMap.GetValueAt(m_currUDTPos);
		++m_nCurrUDTField;
		if (pMap->pEntries[m_nCurrUDTField].nHash != 0)
		{
			return HTTP_SUCCESS;
		}
		m_nCurrUDTField = -1;
		return HTTP_S_FALSE;
	}

	HTTP_CODE OnGetStructFieldName()
	{
		const _soapmap *pMap = m_structMap.GetValueAt(m_currUDTPos);
		return (m_pWriteStream->WriteStream(pMap->pEntries[m_nCurrUDTField].szField,
			pMap->pEntries[m_nCurrUDTField].cchField, NULL) == S_OK) ? HTTP_SUCCESS : HTTP_S_FALSE;
	}

	HTTP_CODE OnNotIsArrayField()
	{
		const _soapmap *pMap = m_structMap.GetValueAt(m_currUDTPos);
		return (pMap->pEntries[m_nCurrUDTField].dwFlags & (SOAPFLAG_FIXEDARR | SOAPFLAG_DYNARR)) ? HTTP_S_FALSE : HTTP_SUCCESS;
	}

	HTTP_CODE OnIsFieldUDT()
	{
		const _soapmap *pMap = m_structMap.GetValueAt(m_currUDTPos);
		return IsUDT(&pMap->pEntries[m_nCurrUDTField]);
	}

	HTTP_CODE OnGetStructFieldSoapType()
	{
		const _soapmap *pMap = m_structMap.GetValueAt(m_currUDTPos);
		if (pMap->pEntries[m_nCurrUDTField].nVal != SOAPTYPE_UNK)
		{
			return GetSoapType(pMap->pEntries[m_nCurrUDTField].nVal, m_pWriteStream);
		}
		ATLASSERT( pMap->pEntries[m_nCurrUDTField].pChain != NULL );
		return (m_pWriteStream->WriteStream(pMap->pEntries[m_nCurrUDTField].pChain->szName, 
			pMap->pEntries[m_nCurrUDTField].pChain->cchName, NULL) == S_OK) ? HTTP_SUCCESS : HTTP_S_FALSE; 
	}

	HTTP_CODE OnIsArrayField()
	{
		return (OnNotIsArrayField() != HTTP_SUCCESS) ? HTTP_SUCCESS : HTTP_S_FALSE;
	}

	HTTP_CODE OnIsFieldDynamicArray()
	{
		const _soapmap *pMap = m_structMap.GetValueAt(m_currUDTPos);
		return (pMap->pEntries[m_nCurrUDTField].dwFlags & SOAPFLAG_DYNARR) ? HTTP_SUCCESS : HTTP_S_FALSE;
	}

	HTTP_CODE OnGetFieldSizeIsName()
	{
		const _soapmap *pMap = m_structMap.GetValueAt(m_currUDTPos);
		int nIndex = pMap->pEntries[m_nCurrUDTField].nSizeIs;
		ATLASSERT( nIndex >= 0 );
		return (m_pStream->WriteStream(pMap->pEntries[nIndex].szField, 
			pMap->pEntries[nIndex].cchField, NULL) == S_OK) ? HTTP_SUCCESS : HTTP_S_FALSE;
	}

	HTTP_CODE OnIsFieldOneDimensional()
	{
		const _soapmap *pMap = m_structMap.GetValueAt(m_currUDTPos);
		return (pMap->pEntries[m_nCurrUDTField].pDims[0] == 1) ? HTTP_SUCCESS : HTTP_S_FALSE;
	}

	HTTP_CODE OnGetFieldArraySize()
	{
		const _soapmap *pMap = m_structMap.GetValueAt(m_currUDTPos);
		return (m_writeHelper.Write(pMap->pEntries[m_nCurrUDTField].pDims[1]) != FALSE) ? 
				HTTP_SUCCESS : HTTP_S_FALSE;
	}

	HTTP_CODE OnGetFieldArraySoapDims()
	{
		const _soapmap *pMap = m_structMap.GetValueAt(m_currUDTPos);
		return GetSoapDims(&pMap->pEntries[m_nCurrUDTField]);
	}

	HTTP_CODE OnGetServiceName()
	{
		return (m_pWriteStream->WriteStream(m_strServiceName, 
			m_strServiceName.GetLength(), NULL) == S_OK) ? HTTP_SUCCESS : HTTP_FAIL;
	}

	HTTP_CODE OnGetNextHeader()
	{	
		if (m_currHeaderPos == NULL)
		{
			m_currHeaderPos = m_headerMap.GetStartPosition();
		}
		else
		{
			m_headerMap.GetNext(m_currHeaderPos);
		}

		return (m_currHeaderPos != NULL) ? HTTP_SUCCESS : HTTP_S_FALSE;
	}

	HTTP_CODE OnIsInHeader()
	{
		return (m_pHeaders[m_nFunc]->pEntries[m_nHeader].dwFlags & SOAPFLAG_IN) 
			? HTTP_SUCCESS : HTTP_S_FALSE;
	}

	HTTP_CODE OnIsOutHeader()
	{
		return (m_pHeaders[m_nFunc]->pEntries[m_nHeader].dwFlags & SOAPFLAG_OUT) 
			? HTTP_SUCCESS : HTTP_S_FALSE;
	}

	HTTP_CODE OnIsRequiredHeader()
	{
		return (m_pHeaders[m_nFunc]->pEntries[m_nHeader].dwFlags & SOAPFLAG_MUSTUNDERSTAND) 
			? HTTP_SUCCESS : HTTP_S_FALSE;
	}

	HTTP_CODE OnGetHeaderName()
	{
		const _soapmapentry *pEntry = m_headerMap.GetValueAt(m_currHeaderPos);
		return (m_pWriteStream->WriteStream(pEntry->szField, 
			pEntry->cchField, NULL) == S_OK) ? HTTP_SUCCESS : HTTP_FAIL;
	}

	HTTP_CODE OnNotIsArrayHeader()
	{
		const _soapmapentry *pEntry = m_headerMap.GetValueAt(m_currHeaderPos);
		return (pEntry->dwFlags & SOAPFLAG_FIXEDARR) ? HTTP_S_FALSE : HTTP_SUCCESS;
	}

	HTTP_CODE OnIsHeaderUDT()
	{
		return IsUDT(m_headerMap.GetValueAt(m_currHeaderPos));
	}

	HTTP_CODE OnGetHeaderSoapType()
	{
		const _soapmapentry *pEntry = m_headerMap.GetValueAt(m_currHeaderPos);
		if (pEntry->nVal != SOAPTYPE_UNK)
		{
			return GetSoapType(pEntry->nVal, m_pWriteStream);
		}
		ATLENSURE( pEntry->pChain != NULL );
		return (m_pWriteStream->WriteStream(pEntry->pChain->szName, 
			pEntry->pChain->cchName, NULL) == S_OK) ? HTTP_SUCCESS : HTTP_S_FALSE; 
	}

	HTTP_CODE OnIsHeaderOneDimensional()
	{
		const _soapmapentry *pEntry = m_headerMap.GetValueAt(m_currHeaderPos);
		return (pEntry->pDims[0] == 1) ? HTTP_SUCCESS : HTTP_S_FALSE;
	}

	HTTP_CODE OnGetHeaderArraySize()
	{
		const _soapmapentry *pEntry = m_headerMap.GetValueAt(m_currHeaderPos);
		return (m_writeHelper.Write(pEntry->pDims[1]) != FALSE) ? 
				HTTP_SUCCESS : HTTP_S_FALSE;
	}

	HTTP_CODE OnGetHeaderArraySoapDims()
	{
		return GetSoapDims(m_headerMap.GetValueAt(m_currHeaderPos));
	}

	HTTP_CODE OnGetNextFunctionHeader()
	{
		++m_nHeader;
		if (m_pHeaders[m_nFunc]->pEntries[m_nHeader].nHash != 0)
		{
			if (m_pHeaders[m_nFunc]->pEntries[m_nHeader].dwFlags & SOAPFLAG_NOMARSHAL)
			{
				return OnGetNextHeader();
			}
			return HTTP_SUCCESS;
		}
		m_nHeader = -1;
		return HTTP_S_FALSE;
	}

	HTTP_CODE OnGetFunctionHeaderName()
	{
		return (m_pWriteStream->WriteStream(
					m_pHeaders[m_nFunc]->pEntries[m_nHeader].szField,
					m_pHeaders[m_nFunc]->pEntries[m_nHeader].cchField,
					NULL) == S_OK) ? HTTP_SUCCESS : HTTP_FAIL;
	}

	HTTP_CODE OnIsArrayHeader()
	{
		return (OnNotIsArrayHeader() == HTTP_SUCCESS) ? HTTP_S_FALSE : HTTP_SUCCESS;
	}

	HTTP_CODE OnIsDocumentLiteral()
	{
		if ((m_dwCallFlags & (SOAPFLAG_DOCUMENT | SOAPFLAG_LITERAL)) ==	
			(SOAPFLAG_DOCUMENT | SOAPFLAG_LITERAL))
		{
			return HTTP_SUCCESS;
		}
		return HTTP_S_FALSE;
	}

	HTTP_CODE OnIsRpcEncoded()
	{
		if ((m_dwCallFlags & (SOAPFLAG_RPC | SOAPFLAG_ENCODED)) ==	
			(SOAPFLAG_RPC | SOAPFLAG_ENCODED))
		{
			return HTTP_SUCCESS;
		}
		return HTTP_S_FALSE;
	}

#pragma warning (push)
#pragma warning (disable : 4640)	// construction of local static object is not thread-safe

	BEGIN_REPLACEMENT_METHOD_MAP(_CSDLGenerator)
		REPLACEMENT_METHOD_ENTRY("GetNamespace", OnGetNamespace)
		REPLACEMENT_METHOD_ENTRY("GetNextFunction", OnGetNextFunction)
		REPLACEMENT_METHOD_ENTRY("GetFunctionName", OnGetFunctionName)
		REPLACEMENT_METHOD_ENTRY("GetNextParameter", OnGetNextParameter)
		REPLACEMENT_METHOD_ENTRY("IsInParameter", OnIsInParameter)
		REPLACEMENT_METHOD_ENTRY("GetParameterName", OnGetParameterName)
		REPLACEMENT_METHOD_ENTRY("NotIsArrayParameter", OnNotIsArrayParameter)
		REPLACEMENT_METHOD_ENTRY("IsParameterUDT", OnIsParameterUDT)
		REPLACEMENT_METHOD_ENTRY("GetParameterSoapType", OnGetParameterSoapType)
		REPLACEMENT_METHOD_ENTRY("IsParameterDynamicArray", OnIsParameterDynamicArray)
		REPLACEMENT_METHOD_ENTRY("IsArrayParameter", OnIsArrayParameter)
		REPLACEMENT_METHOD_ENTRY("IsParameterOneDimensional", OnIsParameterOneDimensional)
		REPLACEMENT_METHOD_ENTRY("GetParameterArraySize", OnGetParameterArraySize)
		REPLACEMENT_METHOD_ENTRY("GetParameterArraySoapDims", OnGetParameterArraySoapDims)
		REPLACEMENT_METHOD_ENTRY("IsOutParameter", OnIsOutParameter)
		REPLACEMENT_METHOD_ENTRY("GetNextEnum", OnGetNextEnum)
		REPLACEMENT_METHOD_ENTRY("GetEnumName", OnGetEnumName)
		REPLACEMENT_METHOD_ENTRY("GetNextEnumElement", OnGetNextEnumElement)
		REPLACEMENT_METHOD_ENTRY("GetEnumElementName", OnGetEnumElementName)
		REPLACEMENT_METHOD_ENTRY("GetNextStruct", OnGetNextStruct)
		REPLACEMENT_METHOD_ENTRY("GetStructName", OnGetStructName)
		REPLACEMENT_METHOD_ENTRY("GetNextStructField", OnGetNextStructField)
		REPLACEMENT_METHOD_ENTRY("GetStructFieldName", OnGetStructFieldName)
		REPLACEMENT_METHOD_ENTRY("NotIsArrayField", OnNotIsArrayField)
		REPLACEMENT_METHOD_ENTRY("IsFieldUDT", OnIsFieldUDT)
		REPLACEMENT_METHOD_ENTRY("GetStructFieldSoapType", OnGetStructFieldSoapType)
		REPLACEMENT_METHOD_ENTRY("IsArrayField", OnIsArrayField)
		REPLACEMENT_METHOD_ENTRY("IsFieldOneDimensional", OnIsFieldOneDimensional)
		REPLACEMENT_METHOD_ENTRY("GetFieldArraySize", OnGetFieldArraySize)
		REPLACEMENT_METHOD_ENTRY("GetFieldArraySoapDims", OnGetFieldArraySoapDims)
		REPLACEMENT_METHOD_ENTRY("GetServiceName", OnGetServiceName)
		REPLACEMENT_METHOD_ENTRY("GetURL", OnGetURL)

		REPLACEMENT_METHOD_ENTRY("GetNextHeader", OnGetNextHeader)
		REPLACEMENT_METHOD_ENTRY("GetHeaderName", OnGetHeaderName)
		REPLACEMENT_METHOD_ENTRY("NotIsArrayHeader", OnNotIsArrayHeader)
		REPLACEMENT_METHOD_ENTRY("IsArrayHeader", OnIsArrayHeader)
		REPLACEMENT_METHOD_ENTRY("IsHeaderUDT", OnIsHeaderUDT)
		REPLACEMENT_METHOD_ENTRY("GetHeaderSoapType", OnGetHeaderSoapType)
		REPLACEMENT_METHOD_ENTRY("IsHeaderOneDimensional", OnIsHeaderOneDimensional)
		REPLACEMENT_METHOD_ENTRY("GetHeaderArraySize", OnGetHeaderArraySize)
		REPLACEMENT_METHOD_ENTRY("GetHeaderArraySoapDims", OnGetHeaderArraySoapDims)
		REPLACEMENT_METHOD_ENTRY("GetNextFunctionHeader", OnGetNextFunctionHeader)
		REPLACEMENT_METHOD_ENTRY("GetFunctionHeaderName", OnGetFunctionHeaderName)
		REPLACEMENT_METHOD_ENTRY("IsInHeader", OnIsInHeader)
		REPLACEMENT_METHOD_ENTRY("IsOutHeader", OnIsOutHeader)
		REPLACEMENT_METHOD_ENTRY("IsRequiredHeader", OnIsRequiredHeader)

		REPLACEMENT_METHOD_ENTRY("IsDocumentLiteral", OnIsDocumentLiteral)
		REPLACEMENT_METHOD_ENTRY("IsRpcEncoded", OnIsRpcEncoded)
		REPLACEMENT_METHOD_ENTRY("IsFieldDynamicArray", OnIsFieldDynamicArray)
		REPLACEMENT_METHOD_ENTRY("GetFieldSizeIsName", OnGetFieldSizeIsName)
	END_REPLACEMENT_METHOD_MAP()
	
#pragma warning (pop)

}; // class _CSDLGenerator

template <class THandler, const char *szHandlerName>
class CSDLGenerator :
	public _CSDLGenerator,
	public IRequestHandlerImpl< CSDLGenerator<THandler,szHandlerName> >,
	public CComObjectRootEx<CComSingleThreadModel>
{
private:

public:
	typedef CSDLGenerator<THandler, szHandlerName> _sdlGenerator;

	BEGIN_COM_MAP(_sdlGenerator)
		COM_INTERFACE_ENTRY(IRequestHandler)
		COM_INTERFACE_ENTRY(ITagReplacer)
	END_COM_MAP()

	HTTP_CODE InitializeHandler(AtlServerRequest *pRequestInfo, IServiceProvider *pServiceProvider)
	{
		IRequestHandlerImpl<CSDLGenerator>::InitializeHandler(pRequestInfo, pServiceProvider);

		CComObjectStack<THandler> handler;
		if (FAILED(InitializeSDL(&handler)))
		{
			return HTTP_FAIL;
		}

		CStencil s;
		HTTP_CODE hcErr = s.LoadFromString(s_szAtlsWSDLSrf, (DWORD) strlen(s_szAtlsWSDLSrf));
		if (hcErr == HTTP_SUCCESS)
		{
			hcErr = HTTP_FAIL;
			CHttpResponse HttpResponse(pRequestInfo->pServerContext);
			HttpResponse.SetContentType("text/xml");
			if (s.ParseReplacements(this) != false)
			{
				s.FinishParseReplacements();

				SetStream(&HttpResponse);
				SetWriteStream(&HttpResponse);
				SetHttpServerContext(m_spServerContext);

				ATLASSERT( s.ParseSuccessful() != false );

				hcErr = s.Render(this, &HttpResponse);
			}
		}

		return hcErr;
	}

	const char * GetHandlerName()
	{
		return szHandlerName;
	}
}; // class CSDLGenerator

} // namespace ATL
#pragma pack(pop)

#pragma warning(pop)

#endif // __ATLSOAP_H__
