// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

#ifndef __ATLHTTP_INL__
#define __ATLHTTP_INL__

#include <errno.h>

#pragma warning(push)
#pragma warning(disable: 4061) // enumerate 'enum value' in switch of enum 'enum type' is not explicitly handled by a case label
#pragma warning(disable: 4062) // enumerate 'enum value' in switch of enum 'enum type' is not handled

namespace ATL
{

/////////////////////////////////////////////////////////////////////////////////
//
// CAtlHttpClient
// Implementation of CAtlHttpClient member functions
//
/////////////////////////////////////////////////////////////////////////////////
template <class TSocketClass>
inline CAtlHttpClientT<TSocketClass>::CAtlHttpClientT() throw()
{
	InitializeObject();
}

// Sets this object to a known state.
template <class TSocketClass>
inline void CAtlHttpClientT<TSocketClass>::InitializeObject() throw()
{
	Close(); // will close the socket if it's already open
	ResetRequest();
	SetSilentLogonOk(FALSE);
}

template <class TSocketClass>
inline void CAtlHttpClientT<TSocketClass>::ResetRequest() throw()
{
	// reset all data that has to do with the current request
	m_HeaderMap.RemoveAll();
	m_current.Empty();
	m_urlCurrent.Clear();
	m_strMethod.Empty();
	m_nStatus = ATL_INVALID_STATUS;
	m_dwBodyLen = 0;
	m_dwHeaderLen = 0;
	m_dwHeaderStart = 0;
	m_pCurrent = NULL;
	m_pNavData = NULL;
	m_LastResponseParseError = RR_NOT_READ;
	m_pEnd = NULL;

}


// Use this function to retrieve an entity from a server via an HTTP
// request. This function will either request a connection from the
// server specified in the szURL parameter or request a connection from
// the proxy server. If a proxy server is to be used, you must call
// SetProxy prior to calling this function to specify the proxy server
// being used. Once the connection is established, an HTTP request 
// is built and sent to the HTTP server. An attempt to read the HTTP
// response is then made. If the response is successfully read, the
// response will be parsed and stored in this class instance. The 
// headers can be parsed via the LookupHeader function and the body
// of the response can be retrieved using the GetBody function. You
// can also retrieve the contents of the entire response by calling
// GetResponse.
template <class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::Navigate(

				LPCTSTR szUrl,
				ATL_NAVIGATE_DATA *pNavData
			) throw(...)
{
	if (!szUrl || *szUrl == _T('\0'))
		return false;

	CUrl url;
	TCHAR szTmp[ATL_URL_MAX_URL_LENGTH];
	if(!AtlEscapeUrl(szUrl,szTmp,0,ATL_URL_MAX_URL_LENGTH-1,ATL_URL_BROWSER_MODE))
		return false;

	if(!url.CrackUrl(szTmp))
		return false;

	// Navigate
	return Navigate(&url, pNavData);
}

template <class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::Navigate(
			LPCTSTR szServer,
			LPCTSTR szPath, 
			ATL_NAVIGATE_DATA *pNavData
			) throw(...)
{
	// Create a URL
	if (!szServer || *szServer == _T('\0'))
		return false;
	if (!szPath || *szPath == _T('\0'))
		return false;
	CUrl url;
	url.SetScheme(ATL_URL_SCHEME_HTTP);
	url.SetHostName(szServer);
	url.SetUrlPath(szPath);
	if (pNavData)
		url.SetPortNumber(pNavData->nPort); 
	else
		url.SetPortNumber(ATL_URL_DEFAULT_HTTP_PORT);

	TCHAR szUrl[ATL_URL_MAX_URL_LENGTH];
	DWORD dwMaxLen = ATL_URL_MAX_URL_LENGTH;
	if (!url.CreateUrl(szUrl, &dwMaxLen))
		return false;

	// Navigate
	return Navigate(szUrl, pNavData);
}

template <class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::Navigate(
			const CUrl *pUrl,
			ATL_NAVIGATE_DATA *pData
			)  throw(...)
{
	bool bRet = false;
	if (!pUrl)
		return false;

	ResetRequest();
	
	CAtlNavigateData default_nav_data;
	if (!pData)
		m_pNavData = &default_nav_data;
	else
		m_pNavData = pData;

	ATLASSUME(m_pNavData);

	_ATLTRY
	{
		m_strMethod = m_pNavData->szMethod;
	}
	_ATLCATCHALL()
	{
		return false;
	}

	SetSocketTimeout(m_pNavData->dwTimeout);

	// set m_urlCurrent
	if (!SetDefaultUrl(pUrl, m_pNavData->nPort))
		return false;
	DWORD dwSent = 0;
	CString strRequest;
	CString strExtraInfo;

	if (!BuildRequest(&strRequest, 
					m_pNavData->szMethod,
					m_pNavData->szExtraHeaders))
	{
		return false;
	}

	
	if (!ConnectSocket())
		return false;

	LPCTSTR szTRequest = strRequest;
	CT2CA strARequest(szTRequest);
	DWORD dwRequestLen = (DWORD)strlen(strARequest);
	DWORD dwAvailable = dwRequestLen + m_pNavData->dwDataLen;

	if (m_pNavData->dwFlags & ATL_HTTP_FLAG_SEND_CALLBACK)
	{
		dwSent = WriteWithCallback(strARequest, dwRequestLen);
	}
	else if (!m_pNavData->pData)
		dwSent = WriteWithNoData(strARequest, dwRequestLen);
	else if (m_pNavData->pData && (m_pNavData->dwFlags & ATL_HTTP_FLAG_SEND_BLOCKS))
	{
		dwSent = WriteWithChunks(strARequest, dwRequestLen);
	}
	else if(m_pNavData->pData)
	{
		dwSent = WriteWithData(strARequest, dwRequestLen);
	}


	// make sure everything was sent
	if (dwSent == dwAvailable)
	{
		// Read the response
		if (RR_OK == ReadHttpResponse())
		{
			// if navigation isn't complete, try to complete
			// it based on the status code and flags
			if ((m_pNavData->dwFlags & ATL_HTTP_FLAG_PROCESS_RESULT)&&
				!ProcessStatus(m_pNavData->dwFlags))
			{
				bRet = false;
			}
			else
				bRet = true;
		}
		else
			bRet = false;
	}

	if (!bRet)
		Close(); // some kind of failure happened, close the socket.

	m_pNavData = NULL;
	return bRet;
}

template <class TSocketClass>
inline DWORD CAtlHttpClientT<TSocketClass>::WriteWithNoData(LPCSTR pRequest, DWORD dwRequestLen)
{
	ATLASSUME(m_pNavData);
	WSABUF Buffer;
	Buffer.buf = (char*)pRequest;
	Buffer.len = (int)dwRequestLen;
	DWORD dwWritten = 0;
	Write(&Buffer, 1, &dwWritten);
	if (m_pNavData->pfnSendStatusCallback)
		m_pNavData->pfnSendStatusCallback(dwWritten, m_pNavData->m_lParamSend);
	return dwWritten;
}

// The entity body will be retrieved from the client by calling their
// callback function.
template <class TSocketClass>
inline DWORD CAtlHttpClientT<TSocketClass>::WriteWithCallback(LPCSTR pRequest, DWORD dwRequestLen)
{
	ATLASSUME(m_pNavData);
	if (!(m_pNavData->pfnChunkCallback &&
		(m_pNavData->dwFlags & ATL_HTTP_FLAG_SEND_CALLBACK)))
		return 0; // error, must have flag set and callback function

	// write the request
	DWORD dwTotalWritten = 0;
	WSABUF Buffer;
	Buffer.buf = (char*)pRequest;
	Buffer.len = (int)dwRequestLen;
	DWORD dwWritten = 0;
	Write(&Buffer, 1, &dwWritten);
	if (m_pNavData->pfnSendStatusCallback)
		if (!m_pNavData->pfnSendStatusCallback(dwWritten, m_pNavData->m_lParamSend))
			return 0;
	if (!dwWritten)
		return 0; // failure
	dwTotalWritten += dwWritten;

	// start writing data;
	while (m_pNavData->pfnChunkCallback((BYTE**)&Buffer.buf, (DWORD*)&Buffer.len, m_pNavData->m_lParamChunkCB) &&
			Buffer.len > 0 &&
			Buffer.buf != NULL)
	{
		Write(&Buffer, 1, &dwWritten);
		if (dwWritten != Buffer.len)
			return 0;
		if (m_pNavData->pfnSendStatusCallback)
			if (!m_pNavData->pfnSendStatusCallback(dwWritten, m_pNavData->m_lParamSend))
				return 0;
		dwTotalWritten += dwWritten;
	}
	return dwTotalWritten;
}

template <class TSocketClass>
inline DWORD CAtlHttpClientT<TSocketClass>::WriteWithChunks(LPCSTR pRequest, DWORD dwRequestLen)
{
	ATLASSUME(m_pNavData);
	if (!(m_pNavData->dwSendBlockSize > 0 && (m_pNavData->dwFlags & ATL_HTTP_FLAG_SEND_BLOCKS)))
		return 0; // error, must have flag set and callback function

	// write the request
	DWORD dwTotalWritten = 0;
	WSABUF Buffer;
	Buffer.buf = (char*)pRequest;
	Buffer.len = (int)dwRequestLen;
	DWORD dwWritten = 0;
	Write(&Buffer, 1, &dwWritten);
	if (m_pNavData->pfnSendStatusCallback)
		if (!m_pNavData->pfnSendStatusCallback(dwWritten, m_pNavData->m_lParamSend))
			return 0;
	if (!dwWritten)
		return 0; // failure
	dwTotalWritten += dwWritten;

	// start writing data;
	DWORD dwDataWritten = 0;
	DWORD dwDataLeft = m_pNavData->dwDataLen;
	while (dwDataLeft)
	{
		Buffer.buf = (char*)(m_pNavData->pData + dwDataWritten);
		Buffer.len = __min(dwDataLeft, m_pNavData->dwSendBlockSize);
		Write(&Buffer, 1, &dwWritten);
		if (dwWritten != Buffer.len)
			return 0;
		if (m_pNavData->pfnSendStatusCallback)
			if (!m_pNavData->pfnSendStatusCallback(dwWritten, m_pNavData->m_lParamSend))
				return false;
		dwTotalWritten += dwWritten;
		dwDataWritten += dwWritten;
		dwDataLeft -= dwWritten;
	}
	return dwTotalWritten;
}

template <class TSocketClass>
inline DWORD CAtlHttpClientT<TSocketClass>::WriteWithData(LPCSTR pRequest, DWORD dwRequestLen)
{
	WSABUF Buffers[2];
	Buffers[0].buf = (char*)pRequest;
	Buffers[0].len = dwRequestLen;
	Buffers[1].buf = (char*)m_pNavData->pData;
	Buffers[1].len = m_pNavData->dwDataLen;

	DWORD dwWritten = 0;
	Write(Buffers, 2, &dwWritten);
	if (m_pNavData->pfnSendStatusCallback)
		m_pNavData->pfnSendStatusCallback(dwWritten, m_pNavData->m_lParamSend);

	return dwWritten;
}

template <class TSocketClass>
bool CAtlHttpClientT<TSocketClass>::NavigateChunked(
				LPCTSTR szServer,
				LPCTSTR szPath,
				ATL_NAVIGATE_DATA *pNavData
				) throw()
{
	// Create a URL
	if (!szServer || *szServer == _T('\0'))
		return false;
	if (!szPath || *szPath == _T('\0'))
		return false;

	if (!pNavData)
	{
		// To do chunked navigation you must specify an
		// ATL_NAVIGATE_DATA structure that has the pfnChunkCallback
		// member filled out.
		ATLASSERT(FALSE);
		return false;
	}
	CUrl url;
	url.SetScheme(ATL_URL_SCHEME_HTTP);
	url.SetHostName(szServer);
	url.SetUrlPath(szPath);
	if (pNavData)
		url.SetPortNumber(pNavData->nPort); 
	else
		url.SetPortNumber(ATL_URL_DEFAULT_HTTP_PORT);

	TCHAR szUrl[ATL_URL_MAX_URL_LENGTH];
	DWORD dwMaxLen = ATL_URL_MAX_URL_LENGTH;
	if (!url.CreateUrl(szUrl, &dwMaxLen))
		return false;

	// Navigate
	return NavigateChunked(szUrl, pNavData);
}

template <class TSocketClass>
bool CAtlHttpClientT<TSocketClass>::NavigateChunked(
			LPCTSTR szURL,
			ATL_NAVIGATE_DATA *pNavData
			) throw()
{
	if (!szURL || *szURL == _T('\0'))
		return false;

	ResetRequest();

	ATLASSERT(pNavData);

	CUrl url;
	if (!url.CrackUrl(szURL))
		return false;

	// Navigate
	return NavigateChunked(&url, pNavData);
}

template <class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::NavigateChunked(
		const CUrl *pUrl,
		ATL_NAVIGATE_DATA *pNavData
		) throw()
{
	if (!pUrl)
		return false;

	if (!pNavData)
	{
		// To do chunked navigation you must specify an
		// ATL_NAVIGATE_DATA structure that has the pfnChunkCallback
		// member filled out.
		ATLASSERT(FALSE);
		return false;
	}

	m_pNavData = pNavData;
	if (!pNavData->pfnChunkCallback)
		return false;

	bool bRet = true;
	
	_ATLTRY
	{
		m_strMethod = m_pNavData->szMethod;
	}
	_ATLCATCHALL()
	{
		return false;
	}

	SetSocketTimeout(m_pNavData->dwTimeout);

	// set m_urlCurrent
	if (!SetDefaultUrl(pUrl, m_pNavData->nPort))
		return false;


	DWORD dwSent = 0;
	CString strRequest;
	CString strExtraInfo;

	if (!BuildRequest(&strRequest,
					m_pNavData->szMethod,
					m_pNavData->szExtraHeaders // extra headers
					))
	{
		return false;
	}

	if (!ConnectSocket())
		return false;

	WSABUF Buffers[3];

	_ATLTRY
	{
		CT2A pRequest(strRequest);

		Buffers[0].buf = (char*)pRequest;
		Buffers[0].len = strRequest.GetLength();

		// send the first buffer which is the request
		if (!Write(Buffers, 1, &dwSent))
		{
			Close();
			return false;
		}
	}
	_ATLCATCHALL()
	{
		Close();
		return false;
	}
	Buffers[0].buf = NULL;
	Buffers[0].len = 0;

	CStringA strChunkSize;

	Buffers[2].buf = "\r\n";
	Buffers[2].len = 2;
	int z = 0;

	// start sending the chunks
	do
	{
		z++;
		Buffers[1].buf = NULL;
		Buffers[1].len = 0;
		if (m_pNavData->pfnChunkCallback((BYTE**)&Buffers[1].buf, &Buffers[1].len, 
			m_pNavData->m_lParamChunkCB))
		{
			_ATLTRY
			{
				if (Buffers[1].len > 0)
				{
					// send the chunk
					strChunkSize.Format("%x\r\n", Buffers[1].len);
					Buffers[0].buf = (char*)(LPCSTR)strChunkSize;
					Buffers[0].len = strChunkSize.GetLength();
					if (!Write(Buffers, 3, &dwSent))
					{
						bRet = false;
						break;
					}
				}
				else if (Buffers[1].len == 0)
				{
					strChunkSize = "0\r\n\r\n\r\n";
					Buffers[0].buf = (char*)(LPCSTR)strChunkSize;
					Buffers[0].len = strChunkSize.GetLength();
					if (!Write(Buffers, 1, &dwSent))
					{
						bRet = false;
						break;
					}
					break;
				}
			}
			_ATLCATCHALL()
			{
				bRet = false;
				break;
			}
		}
		else
		{
			bRet = false;
			break; // something went wrong in callback
		}
	}while (Buffers[1].len > 0);

	strRequest.ReleaseBuffer();

	if (bRet)
	{
		// Read the response
		if (RR_OK == ReadHttpResponse())
		{
			// if navigation isn't complete, try to complete
			// it based on the status code and flags
			if ((m_pNavData->dwFlags & ATL_HTTP_FLAG_PROCESS_RESULT)
				&& !ProcessStatus(m_pNavData->dwFlags))
			{
				bRet = false;
			}
			bRet = true;
		}
		else
			bRet = false;
	}

	if (!bRet)
		Close();

	return bRet;
}

template <class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::ConnectSocket() throw()
{
	bool bRet=false;
	// connect to the correct server
	if (GetProxy())
	{
		//if we're using a proxy connect to the proxy
		bRet=Connect(m_strProxy, m_nProxyPort);
	}
	else
	{
		bRet=Connect(m_urlCurrent.GetHostName(),m_urlCurrent.GetPortNumber()); // connect to the server
	}		
	return bRet;
}


template <class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::BuildRequest(/*out*/CString *pstrRequest,
										LPCTSTR szMethod,
										LPCTSTR szExtraHeaders)  throw()
{
	if (!m_pNavData)
		return false;
	_ATLTRY
	{
		// build up the request
		CString strRequest = szMethod;
		strRequest += _T(" ");
		if (GetProxy())
		{
			TCHAR buffURL[ATL_URL_MAX_URL_LENGTH];
			DWORD dwSize = ATL_URL_MAX_URL_LENGTH;
			m_urlCurrent.CreateUrl(buffURL, &dwSize);
			strRequest += buffURL;

			strRequest += ATL_HTTP_HEADER_PROXY;
			CString strHost;
			if (m_urlCurrent.GetPortNumber() != ATL_URL_DEFAULT_HTTP_PORT)
				strHost.Format(_T("Host: %s:%d\r\n"), m_urlCurrent.GetHostName(), m_urlCurrent.GetPortNumber());
			else
				strHost.Format(_T("Host: %s\r\n"), m_urlCurrent.GetHostName());
			strRequest += strHost;

			if (m_pNavData->dwDataLen>0)
			{
				CString strCL;
				strCL.Format(_T("Content-Length: %d\r\n"), m_pNavData->dwDataLen);
				strRequest += strCL;
			}

			if (m_pNavData->szDataType)
			{
				strRequest += _T("Content-Type: ");
				strRequest += m_pNavData->szDataType;
				strRequest += _T("\r\n");
			}

			if (m_pNavData->szExtraHeaders)
				strRequest += szExtraHeaders;
			strRequest += ATL_HTTP_USERAGENT;
		}
		else
		{
			strRequest += m_urlCurrent.GetUrlPath();
			strRequest += m_urlCurrent.GetExtraInfo();
			strRequest += ATL_HTTP_HEADER;

			if (m_pNavData->dwDataLen > 0)
			{
				CString strCL;
				strCL.Format(_T("Content-Length: %d\r\n"), m_pNavData->dwDataLen);
				strRequest += strCL;
			}

			if (m_pNavData->szDataType && 
				*m_pNavData->szDataType)
			{
				strRequest += _T("Content-Type: ");
				strRequest += m_pNavData->szDataType;
				strRequest += _T("\r\n");
			}

			if (szExtraHeaders)
				strRequest += szExtraHeaders;


			CString strHost;
			strHost.Format(_T("Host: %s\r\n"), m_urlCurrent.GetHostName());
			strRequest += strHost;
			strRequest += ATL_HTTP_USERAGENT;
		}
		strRequest += _T("\r\n");


		*pstrRequest = strRequest;
		return true;
	}
	_ATLCATCHALL()
	{
		return false;
	}
}

template <class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::ReadSocket() throw()
{
	bool bRet = false;
	unsigned char read_buff[ATL_READ_BUFF_SIZE];
	int dwSize = ATL_READ_BUFF_SIZE;

	// read some data
	for (int i = 0; i < ATL_HTTP_CLIENT_EMPTY_READ_RETRIES; ++i)
	{
		bRet = Read(read_buff, (DWORD*)&dwSize);
		if (!bRet)
			return bRet;

		// notify user
		if (m_pNavData)
		{
			if (m_pNavData->pfnReadStatusCallback)
				bRet = m_pNavData->pfnReadStatusCallback(dwSize, m_pNavData->m_lParamRead);
			if (!bRet)
				return bRet;
		}

		if (dwSize > 0)
		{
			// append the data to our internal buffer
			// m_current holds bytes (not UNICODE!)
			
			if (!m_current.Append((LPCSTR)read_buff, dwSize))
				return FALSE;

			m_pEnd = ((BYTE*)(LPCSTR)m_current) + m_current.GetLength();
			m_pCurrent = (BYTE*)(LPCSTR)m_current;
			break;
		}
 		bRet = false; // nothing was read
	}

	return bRet;
}

// Starts searching for a complete header set at
// m_pCurrent. This function will only move m_pCurrent
// if a complete set is found. Returns the header beginning
// optionally.
template <class TSocketClass>
inline unsigned char* CAtlHttpClientT<TSocketClass>::FindHeaderEnd(unsigned char** ppBegin) throw()
{
	if (!m_pCurrent)
		return NULL;

	BYTE *pCurr = m_pCurrent;
	BYTE *pBegin = m_pCurrent;
	int nLen = m_current.GetLength();

	if (pCurr >= (BYTE*)(LPCSTR)m_current + m_current.GetLength())
		return NULL; // no more chars in buffer
	// look for the end of the header (the \r\n\r\n)
	while (pCurr <= (pBegin + nLen - ATL_HEADER_END_LEN))
	{

		if (* ((UNALIGNED  DWORD*)pCurr)==ATL_DW_HEADER_END)
		{
			// set m_pCurrent pointer to the end of the header
			m_pCurrent = pCurr + ATL_HEADER_END_LEN;
			if (ppBegin)
				*ppBegin = pBegin;
			return m_pCurrent;
		}
		pCurr++;
	}
	return NULL;
}

// Call this function after sending an HTTP request over the socket. The complete
// HTTP response will be read. This function will also parse
// response headers into the response header map.
template <class TSocketClass>
inline typename CAtlHttpClientT<TSocketClass>::HTTP_RESPONSE_READ_STATUS CAtlHttpClientT<TSocketClass>::ReadHttpResponse() 
{
	// Read until we at least have the response headers
	HTTP_RESPONSE_READ_STATUS result = RR_OK;
	readstate state = rs_init;
	unsigned char *pBodyBegin = NULL;
	unsigned char *pHeaderBegin = NULL;
	m_current.Empty();
	m_pCurrent = NULL;
	m_LastResponseParseError = RR_OK;

	while (state != rs_complete)
	{
		switch(state)
		{
		case rs_init:
			m_HeaderMap.RemoveAll();
			m_nStatus = ATL_INVALID_STATUS;
			m_dwHeaderLen = 0;
			m_dwBodyLen = 0;
			state = rs_readheader;
			// fall through

		case rs_readheader:

			// read from the socket until we have a complete set of headers.
			pBodyBegin = FindHeaderEnd(&pHeaderBegin);
			if (!pBodyBegin)
			{
				if (!ReadSocket())
				{
					// Either reading from the socket failed, or there
					// was not data to read. Set the nav status to error
					// and change the state to complete.
					state = rs_complete;
					result = RR_READSOCKET_FAILED;
					break;
				}
				else
					break; // loop back and FindHeaderEnd again.
			}
			// we have a complete set of headers
			m_dwHeaderLen = (DWORD)(pBodyBegin-pHeaderBegin);
			m_dwHeaderStart = (DWORD)(pHeaderBegin - (BYTE*)(LPCSTR)m_current);
			// fall through
			state = rs_scanheader;

		case rs_scanheader:
			// set m_nStatus and check for valid status
			ParseStatusLine(pHeaderBegin);
			// failed to set m_nStatus;
			if (m_nStatus == ATL_INVALID_STATUS)
			{
				state = rs_complete;
				result = RR_STATUS_INVALID;
				break;
			}

			else if (m_nStatus == 100) // continue
			{
				state = rs_init;
				break;
			}

			// crack all the headers and put them into a header map. We've already
			// done the check to make sure we have a complete set of headers in 
			// rs_readheader above
			if (ATL_HEADER_PARSE_COMPLETE != CrackResponseHeader((LPCSTR)pHeaderBegin, 
				(LPCSTR*)&pBodyBegin))
			{
				// something bad happened while parsing the headers!
				state = rs_complete;
				result = RR_PARSEHEADERS_FAILED;
				break;
			}
			state = rs_readbody;
			// fall through

		case rs_readbody:
			// headers are parsed and cracked, we're ready to read the rest
			// of the response. 
			if (IsMsgBodyChunked())
			{
				if (!ReadChunkedBody())
				{
					result = RR_READCHUNKEDBODY_FAILED;
					state = rs_complete;
					break;
				}
			}
			else
			if (!ReadBody(GetContentLength(), m_current.GetLength()-(m_dwHeaderStart+m_dwHeaderLen)))
				result = RR_READBODY_FAILED;
			state = rs_complete;
			//fall through

		case rs_complete:
			// clean up the connection if the server requested a close;
			DisconnectIfRequired();
			break;
		}
	}
	m_LastResponseParseError = result;
	return result;
}

template <class TSocketClass>
inline typename CAtlHttpClientT<TSocketClass>::HTTP_RESPONSE_READ_STATUS CAtlHttpClientT<TSocketClass>::GetResponseStatus()
{
	return m_LastResponseParseError;
}

// Checks to see if the server has closed the connection.
// If it has, we create a new socket and reconnect it to
// the current server. This also clears the contents of the
// current response buffer.
template <class TSocketClass>
inline void CAtlHttpClientT<TSocketClass>::ResetConnection() throw()
{
	ReconnectIfRequired();
	m_HeaderMap.RemoveAll();
	m_current.Empty();
	m_nStatus = ATL_INVALID_STATUS;
	m_AuthTypes.RemoveAll(); // the server will keep sending back www-authenticate
							 // headers until the connection is authorized
}

// Takes action based on the flags passed and the current
// status for this object.
template <class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::ProcessStatus(DWORD dwFlags) throw()
{
	switch(m_nStatus)
	{
	case 200: // In all these cases there is no further action
	case 201: // to take. Any additional informaion is returned
	case 202: // in the entity body.
	case 203:
	case 204:
	case 205:
	case 206:
	case 304:
	case 305:
		return true;
		break;
	case 301:
	case 302:
	case 303:
		if (dwFlags & ATL_HTTP_FLAG_AUTO_REDIRECT)
			return ProcessObjectMoved();
		break;
	case 401: // auth required
			return NegotiateAuth(false);
		break;
	case 407: // proxy auth required
			return NegotiateAuth(true);
		break;

	}
	return false;
}

// Looks up the value of a response header in the header map. Call with
// NULL szBuffer to have length of the required buffer placed in 
// pdwLen on output.

// szName is the name of the header to look up.
// szBuffer is the buffer that will contain the looked up string.
// pdwLen contains the length of szBuffer in characters on input and the length
// of the string including NULL terminator in characters on output.
template<class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::GetHeaderValue(LPCTSTR szName, CString& strValue) const throw()
{
	_ATLTRY
	{
		return m_HeaderMap.Lookup(szName, strValue);
	}
	_ATLCATCHALL()
	{
		return false;
	}
}

template<class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::GetHeaderValue(__in_z LPCTSTR szName, __out_ecount_part_z_opt(*pdwLen, *pdwLen) LPTSTR szBuffer, __inout DWORD *pdwLen) const throw()
{
	CString strValue;
	bool bRet = GetHeaderValue(szName, strValue);
	DWORD nLen = strValue.GetLength();
	if (!bRet)
		return false;

	if ((pdwLen && *pdwLen < nLen+1) ||
		(!szBuffer && pdwLen) )
	{
		*pdwLen = nLen+1;
		return true;
	}

	if (!szBuffer)
		return false;

	Checked::tcsncpy_s(szBuffer, nLen+1, (LPCTSTR)strValue, _TRUNCATE);
	if (pdwLen)
		*pdwLen = nLen+1;
	return true;
}

// Adds an authorization object to use for a particular scheme.
// This will overwrite an existing entry if an object for the 
// same scheme has already been set.
template<class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::AddAuthObj(LPCTSTR szScheme,
				CAtlBaseAuthObject *pObject, IAuthInfo *pInfo/*=NULL*/) throw()
{
	if (!pObject)
		return false;

	pObject->Init(this, pInfo);

	_ATLTRY
	{
		POSITION pos = m_AuthMap.SetAt(szScheme, pObject);
		if (!pos)
			return false;
	}
	_ATLCATCHALL()
	{
		return false;
	}

	return true;
}

// Tries to find an authorization object to use for a particular
// scheme
template<class TSocketClass>
inline const CAtlBaseAuthObject* CAtlHttpClientT<TSocketClass>::FindAuthObject(LPCTSTR szScheme) throw()
{
	CAtlBaseAuthObject *pObject = NULL;
	if (m_AuthMap.Lookup(szScheme, pObject))
	{
		return const_cast<const CAtlBaseAuthObject*>(pObject);
	}
	return NULL;
}

// Removes an existing authorization object from the map.
template<class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::RemoveAuthObject(LPCTSTR szScheme) throw()
{
	return m_AuthMap.RemoveKey(szScheme);
}

// Sets the current proxy server and port
template<class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::SetProxy(LPCTSTR szProxy, short nProxyPort) throw()
{
	if (!szProxy)
	{
		if (!LookupRegProxy())
			return false;
	}
	else
	{
		_ATLTRY
		{
			m_strProxy = szProxy;
			m_nProxyPort = nProxyPort;
		}
		_ATLCATCHALL()
		{
			return false;
		}
	}
	return true;
}

// Removes the current proxy settings.
template<class TSocketClass>
inline void CAtlHttpClientT<TSocketClass>::RemoveProxy() throw()
{
		m_strProxy.Empty();
		m_nProxyPort = ATL_URL_INVALID_PORT_NUMBER;
}

// retrieves the current proxy
template<class TSocketClass>
inline LPCTSTR CAtlHttpClientT<TSocketClass>::GetProxy() const throw()
{
	if (m_strProxy.GetLength())
		return (LPCTSTR)m_strProxy;
	return NULL;
}

template<class TSocketClass>
inline short CAtlHttpClientT<TSocketClass>::GetProxyPort() const throw()
{
	return m_nProxyPort;
}

// Gets the contents of the entire response buffer.
template<class TSocketClass>
inline const BYTE* CAtlHttpClientT<TSocketClass>::GetResponse() throw()
{
	return (const BYTE*)(LPCSTR)m_current;
}

template<class TSocketClass>
inline DWORD CAtlHttpClientT<TSocketClass>::GetResponseLength() throw()
{
	return m_current.GetLength();
}

// Gets the length in bytes of the body of the
// current response
template<class TSocketClass>
inline DWORD CAtlHttpClientT<TSocketClass>::GetBodyLength() const throw()
{
	return m_dwBodyLen;
}

// Gets the contents of the body of the current response. This
// is the response without the headers. 
template<class TSocketClass>
inline const BYTE* CAtlHttpClientT<TSocketClass>::GetBody() throw()
{
	return (BYTE*)((LPCSTR)m_current + m_dwHeaderLen + m_dwHeaderStart);
}

// Get the length of the header part of the response in bytes.
template<class TSocketClass>
inline DWORD CAtlHttpClientT<TSocketClass>::GetRawResponseHeaderLength() throw()
{
	return m_dwHeaderLen >= 2 ? m_dwHeaderLen-2 : 0; // m_dwHeaderLen includes the final \r\n
}

// buffer must include space for null terminator.
// on input, pdwLen specifies the size of szBuffer,
// on output, pdwLen holds the number of bytes copied
// to szBuffer, or the required size of szBuffer if 
// szBuffer wasn't big enough
template<class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::GetRawResponseHeader(LPBYTE szBuffer, DWORD *pdwLen) throw()
{
	if (!pdwLen)
		return false;

	DWORD header_len = GetRawResponseHeaderLength();
	if (header_len == 0)
		return false;

	if (!szBuffer || *pdwLen < header_len+1)
	{
		*pdwLen = header_len+1;
		return false;
	}

	Checked::memcpy_s(szBuffer, *pdwLen, (BYTE*)(LPCSTR)m_current, header_len);
	szBuffer[header_len]='\0';

	*pdwLen = header_len+1;
	return true;
}

// Gets the current URL object.
template<class TSocketClass>
inline LPCURL CAtlHttpClientT<TSocketClass>::GetCurrentUrl() const throw()
{
	return (LPCURL)&m_urlCurrent;
}

template<class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::SetDefaultUrl(  LPCTSTR szUrl, 
											short nPortNumber) throw()
{
	return _SetDefaultUrl(szUrl,nPortNumber);
}

template<class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::SetDefaultUrl(  LPCURL pUrl, 
											short nPortNumber) throw()
{
	m_urlCurrent = *pUrl;
	return _SetDefaultUrl(NULL, nPortNumber);
}

template<class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::SetDefaultMethod(LPCTSTR szMethod) throw()

{
	_ATLTRY
	{
		m_strMethod = szMethod;
		return true;
	}
	_ATLCATCHALL()
	{
		return false;
	}
}

template<class TSocketClass>
inline DWORD CAtlHttpClientT<TSocketClass>::GetFlags() const throw()
{
	if (m_pNavData)
		return m_pNavData->dwFlags;
	else
		return ATL_HTTP_FLAG_INVALID_FLAGS;
}

template<class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::LookupRegProxy() throw()
{
	// attempt to look it up from the registry
	CRegKey rkProxy;
	ULONG nChars = ATL_URL_MAX_URL_LENGTH+1;
	TCHAR szUrl[ATL_URL_MAX_URL_LENGTH+1] = { 0 };

	DWORD dwErr = rkProxy.Open(HKEY_CURRENT_USER, 
		_T("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"), KEY_READ);
	if (dwErr == ERROR_SUCCESS)
	{
		dwErr = rkProxy.QueryStringValue(_T("ProxyServer"), szUrl, &nChars);
	}
	if (dwErr == ERROR_SUCCESS)
	{
		CUrl url;
		if (url.CrackUrl(szUrl))
		{
			if (url.GetScheme()==ATL_URL_SCHEME_UNKNOWN)
			{
				// without the scheme name (e.g. proxy:80)
				m_strProxy = url.GetSchemeName();
				m_nProxyPort = (short)_ttoi(url.GetHostName());
				return true;
			}
			else if (url.GetHostName())
			{
				// with the scheme (e.g. http://proxy:80)
				m_strProxy = url.GetHostName();
				m_nProxyPort = url.GetPortNumber();
				return true;
			}
		}
	}
	return false;
}

template<class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::DisconnectIfRequired() throw()
{
	CString strValue;
	if (GetHeaderValue(_T("Connection"), strValue) && !strValue.CompareNoCase(_T("close")))
	{
		Close();
	}

	return true;
}

class CInitializeCOMThread
{
public:
	CInitializeCOMThread()
		: m_bCoInit(FALSE),m_bShouldUninit(FALSE)
	{		
		//At this point the Thread can be uninit, init to STA or init to MTA.
		//CoInitialize can always fail unexpectedly.		
		HRESULT hr = ::CoInitialize(NULL);		
		if (SUCCEEDED(hr))
		{
			m_bCoInit=TRUE;
			m_bShouldUninit=TRUE;
		} else if (hr == RPC_E_CHANGED_MODE)
		{	
			m_bCoInit=TRUE;
		}
	}
	~CInitializeCOMThread()
	{
		if (m_bShouldUninit)
		{
			::CoUninitialize();
		}
	}
	BOOL IsInitialized() {  return m_bCoInit; }
protected:
	BOOL m_bCoInit;
	BOOL m_bShouldUninit;
};
// Tries to find an authorization object that meets
template<class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::NegotiateAuth(bool bProxy) throw()
{
	//Test if can silently pass user credentials to server.
	if (!m_bSilentLogonOk)
	{	
		//Call CoInit, because ATL Http code cannot assume it has already been called by the user.
		CInitializeCOMThread initThread;
		if (initThread.IsInitialized())
		{
			HRESULT hr = S_OK;
			CComPtr<IInternetSecurityManager> spSecurityMgr;
			
			hr = CoCreateInstance(CLSID_InternetSecurityManager, NULL, CLSCTX_INPROC_SERVER,
									IID_IInternetSecurityManager, (void**)&spSecurityMgr);
			if (SUCCEEDED(hr))
			{		
				TCHAR szUrl[ATL_URL_MAX_URL_LENGTH];
				DWORD dwMaxLen = ATL_URL_MAX_URL_LENGTH;
				if (!m_urlCurrent.CreateUrl(szUrl, &dwMaxLen))
				{
					return false;
				}
				
				CStringW strUrlW(szUrl);
				DWORD dwPolicy=0xFFFF;		
				hr=spSecurityMgr->ProcessUrlAction(strUrlW.GetString(),
												URLACTION_CREDENTIALS_USE,
												reinterpret_cast<BYTE*>(&dwPolicy),
												sizeof(dwPolicy),
												NULL,
												0,
												PUAF_NOUI,
												NULL);

				if (FAILED(hr) || dwPolicy != URLPOLICY_CREDENTIALS_SILENT_LOGON_OK)
				{
					return false;
				}
			}
			else
			{
				// CoCreateInstance failed, return false
				return false;
			}
		}
		else
		{
			// CoInit failed, return false
			return false;
		}
	}

	// szAuthHeaderValue should contain a comma separated list
	// of authentication types
	CAtlBaseAuthObject *pAuthObj = NULL;
	bool bRet = false;
	for (size_t i = 0; i<m_AuthTypes.GetCount(); i++)
	{
		_ATLTRY
		{
			CString strName = m_AuthTypes[i];
			int nSpace = strName.Find(_T(' '));
			if (nSpace!=-1)
				strName.SetAt(nSpace,0);

			if (m_AuthMap.Lookup(strName, pAuthObj) &&
				!pAuthObj->m_bFailed)
				bRet = pAuthObj->Authenticate(m_AuthTypes[i], bProxy);

			if (bRet)
				return bRet;
		}
		_ATLCATCHALL()
		{
			bRet = false;
		}
	}
	return bRet;
}

template<class TSocketClass>
inline long CAtlHttpClientT<TSocketClass>::GetContentLength() throw()
{
	CString strValue;
	if (GetHeaderValue(_T("Content-Length"), strValue))
	{
		TCHAR *pStop = NULL;
		return _tcstol(strValue, &pStop, 10);
	}
	else
		return -1;
}

template<class TSocketClass>
inline LPCSTR CAtlHttpClientT<TSocketClass>::NextLine(BYTE* pCurr) throw()
{
	if (!pCurr)
		return NULL;

	while ( pCurr < m_pEnd && *pCurr && !(*pCurr == '\r' && *(pCurr+1) == '\n'))
		pCurr++;

	if (pCurr >= m_pEnd)
		return NULL;

//	if (pCurr < m_pEnd-4)
//		if (!memcmp(pCurr, ATL_HEADER_END, 4))
			//return NULL;

	return (LPCSTR)(pCurr+2);
}

template<class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::IsMsgBodyChunked() throw()
{
	CString strValue;
	return (
			GetHeaderValue(_T("Transfer-Encoding"), strValue) &&
			!strValue.CompareNoCase(_T("chunked"))
			);

}

// finds the end of an individual header field pointed to by
// pszStart. Header fields can be multi-line with multi-line 
// header fields being a line that starts with some kind of 
// white space.
template<class TSocketClass>
inline LPCSTR CAtlHttpClientT<TSocketClass>::FindEndOfHeader(LPCSTR pszStart) throw()
{
	// move through all the lines until we come to one
	// that doesn't start with white space
	LPCSTR pLineStart = pszStart;
	LPCSTR pHeaderEnd = NULL;

	do 
	{
		pLineStart = NextLine((BYTE*)pLineStart);
	}while (pLineStart && isspace(static_cast<unsigned char>(*pLineStart)) && strncmp(pLineStart-2, ATL_HEADER_END, ATL_HEADER_END_LEN));

	if (pLineStart > (LPCSTR)m_pEnd)
		return NULL; // ran out of data in the buffer without finding the end of a line
	                 // or the end of the headers.

	if (pLineStart)
		pHeaderEnd = pLineStart-2;
	else
		pHeaderEnd = NULL;

	return pHeaderEnd;
}

template<class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::DecodeHeader(LPCSTR pHeaderStart, LPCSTR pHeaderEnd) throw()
{
	_ATLTRY
	{
		if (!pHeaderStart || !pHeaderEnd)
			return false;
		LPCSTR pTemp = pHeaderStart;
		while (*pTemp != ATL_FIELDNAME_DELIMITER && pTemp < pHeaderEnd)
			pTemp++;
		if (*pTemp == ATL_FIELDNAME_DELIMITER)
		{
			char szName[ATL_MAX_FIELDNAME_LEN];
			char szValue[ATL_MAX_VALUE_LEN];
			int nLen = (int)(pTemp-pHeaderStart) ;
			ATLASSERT(nLen < ATL_MAX_FIELDNAME_LEN);
			if (nLen >= ATL_MAX_FIELDNAME_LEN)
				return false; // won't fit in the buffer.
			Checked::memcpy_s(szName, ATL_MAX_FIELDNAME_LEN, pHeaderStart, nLen);
			szName[nLen]=0;

			pTemp++; // move past delimiter;
			while (isspace(static_cast<unsigned char>(*pTemp)) && pTemp < pHeaderEnd)
				pTemp++;

			nLen = (int)(pHeaderEnd-pTemp);
			ATLASSERT(nLen < ATL_MAX_VALUE_LEN);
			if (nLen >= ATL_MAX_VALUE_LEN)
				return false; // won't fit in the buffer
			Checked::memcpy_s(szValue, ATL_MAX_VALUE_LEN, pTemp, nLen);
			szValue[nLen]=0;

			CString strExist;
			CA2T pszName(szName);
			CA2T pszValue(szValue);

			if (!_tcsicmp(pszName, _T("www-authenticate")) ||
				!_tcsicmp(pszName, _T("proxy-authenticate")))
			{
				m_AuthTypes.Add(pszValue);
			}

			if (!m_HeaderMap.Lookup(pszName, strExist))
				m_HeaderMap.SetAt(pszName, pszValue);
			else
			{   
				// field-values for headers with the same name can be appended
				// per rfc2068 4.2, we do the appending so we don't have to
				// store/lookup duplicate keys.
				strExist += ',';
				strExist += pszValue;
				m_HeaderMap.SetAt(pszName, (LPCTSTR)strExist);
			}

			// if it's a set-cookie header notify users so they can do 
			// somthing with it.
			if (!_tcsicmp(pszName, _T("set-cookie")))
				OnSetCookie(pszValue);
		}

		return true;
	}
	_ATLCATCHALL()
	{
		return false;
	}
}

template<class TSocketClass>
inline void CAtlHttpClientT<TSocketClass>::OnSetCookie(LPCTSTR) throw()
{
	return;
}

template<class TSocketClass>
inline LPCSTR CAtlHttpClientT<TSocketClass>::ParseStatusLine(BYTE* pBuffer) throw()
{
	if (!pBuffer)
		return NULL;
	if (m_pEnd <= pBuffer)
		return NULL;

	// find the first space'
	while (pBuffer < m_pEnd && !isspace(static_cast<unsigned char>(*pBuffer)))
		pBuffer++;

	if (pBuffer >= m_pEnd)
		return NULL;

	// move past the space
	while (pBuffer < m_pEnd && isspace(static_cast<unsigned char>(*pBuffer)))
		pBuffer++;

	if (pBuffer >= m_pEnd)
		return NULL;

	// pBuffer better be pointing at the status code now
	LPCSTR pEnd = NULL;
	if (*pBuffer >= '0' && *pBuffer <= '9')
	{
		// probably a good status code
		errno_t errnoValue = AtlStrToNum(&m_nStatus, (LPSTR)pBuffer, (LPSTR*)&pEnd, 10);
		if (errnoValue == ERANGE)
			return NULL; // bad status code
	}
	else 
		return FALSE; // bad status code;

	if (!pEnd)
		return FALSE; // bad status code;

	pBuffer = (BYTE*)pEnd;
	// move to end of line
	while (pBuffer < m_pEnd && *pBuffer !=  '\n')
		pBuffer++;

	if (pBuffer >= m_pEnd)
		return NULL;

	// set the return pointing to the first 
	// character after our status line.
	return (LPCSTR)++pBuffer;
}


// pBuffer should start at the first character
// after the status line.
template<class TSocketClass>
inline int CAtlHttpClientT<TSocketClass>::CrackResponseHeader(LPCSTR pBuffer, /*out*/ LPCSTR *pEnd) throw()
{
	// read up to the double /r/n
	LPCSTR pszStartSearch = pBuffer;
	if (!pEnd)
		return ATL_HEADER_PARSE_HEADERERROR;

	*pEnd = NULL;
	if (pszStartSearch == NULL)
		return ATL_HEADER_PARSE_HEADERERROR;

	// start parsing headers
	LPCSTR pHeaderStart = ParseStatusLine((BYTE*)pBuffer);
	if (!pHeaderStart)
		return ATL_HEADER_PARSE_HEADERERROR;
	LPCSTR pHeaderEnd = NULL;

	while (pHeaderStart && *pHeaderStart && pHeaderStart < (LPCSTR)m_pEnd)
	{
		pHeaderEnd = FindEndOfHeader(pHeaderStart);
		if (!pHeaderEnd)
			break; // error

		DecodeHeader(pHeaderStart, pHeaderEnd);

		if (!strncmp(pHeaderEnd, ATL_HEADER_END, strlen(ATL_HEADER_END)))
		{
			*pEnd = pHeaderEnd + strlen(ATL_HEADER_END);
			break;      // we're done
		}
		else
			pHeaderStart = pHeaderEnd+2;
	}

	return ATL_HEADER_PARSE_COMPLETE;
}

// Reads the body if the encoding is not chunked.
template<class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::ReadBody(int nContentLen, int nCurrentBodyLen) throw()
{
	// nCurrentBodyLen is the length of the body that has already been read
	// nContentLen is the value of Content-Length
	// current is the buffer that will contain the entire response
	bool bRet = true;
	ATLASSUME(m_pNavData);
	if (!m_pNavData)
		return false;

	CTempBuffer<BYTE, 512> readbuff;
	DWORD dwReadBuffSize = 0;
	DWORD dwRead = 0;
	if (m_pNavData->dwReadBlockSize)
	{
		ATLTRY(readbuff.Allocate(m_pNavData->dwReadBlockSize));
		dwReadBuffSize = m_pNavData->dwReadBlockSize;
	}
	else
	{
		ATLTRY(readbuff.Allocate(ATL_READ_BUFF_SIZE));
		dwReadBuffSize = ATL_READ_BUFF_SIZE;
	}

	if (readbuff.operator BYTE*() == NULL)
		return false;

	if (nContentLen != -1) // We know the content length.
	{
		// read the rest of the body.
		while (nCurrentBodyLen < nContentLen)
		{
			dwRead = dwReadBuffSize;
			// loop while dwRead == 0
			for (int nRetry = 0; nRetry < ATL_HTTP_CLIENT_EMPTY_READ_RETRIES; ++nRetry)
			{
				if (!Read(readbuff, &dwRead))
					return false;
	
				// notify user
				if (m_pNavData)
				{
					if (m_pNavData->pfnReadStatusCallback)
						if (!m_pNavData->pfnReadStatusCallback(dwRead, m_pNavData->m_lParamRead))
							return false;
				}
	
				if (dwRead == 0)
					continue;
				nCurrentBodyLen += dwRead;
				if (!m_current.Append((LPCSTR)(BYTE*)readbuff, dwRead))
				{
					ATLASSERT(0);
					return false; // error!
				}
				m_pEnd = ((BYTE*)(LPCSTR)m_current) + m_current.GetLength();
				break;
			}
			if (dwRead == 0)
				return false;
		}
		m_dwBodyLen = nCurrentBodyLen;
	}
	else // We don't know content length. All we can do is
	{    // read until there is nothing else to read.
		int nRetries = 0;
		while (1)
		{
			dwRead = dwReadBuffSize;
			if (Read((BYTE*)readbuff, (DWORD*)&dwRead))
			{
				// notify user
				if (m_pNavData)
				{
					if (m_pNavData->pfnReadStatusCallback)
						bRet = m_pNavData->pfnReadStatusCallback(dwRead, m_pNavData->m_lParamRead);
					if (!bRet)
						return bRet;
				}

				if (dwRead == 0)
				{
					if (nRetries++ < ATL_HTTP_CLIENT_EMPTY_READ_RETRIES)
						continue;
					break;
				}

				nRetries = 0;
				nCurrentBodyLen += dwRead;
				if (!m_current.Append((LPCSTR)(BYTE*)readbuff, dwRead))
					return false;
				m_pEnd = ((BYTE*)(LPCSTR)m_current) + m_current.GetLength();
			}
			else 
			{
				// notify user
				if (m_pNavData)
				{
					if (m_pNavData->pfnReadStatusCallback)
						bRet = m_pNavData->pfnReadStatusCallback(dwRead, m_pNavData->m_lParamRead);
					if (!bRet)
						return bRet;
				}

				bRet = true;
				break;
			}
		}
		m_dwBodyLen = nCurrentBodyLen;
	}
	return bRet;
}


// This function moves pBuffStart only on success. On success, pBuffStart is moved
// to the element past the last element we consumed.
template<class TSocketClass>
inline typename CAtlHttpClientT<TSocketClass>::CHUNK_LEX_RESULT CAtlHttpClientT<TSocketClass>::get_chunked_size(char *&pBuffStart, char *&pBuffEnd, long* pnChunkSize) throw()
{
	CHUNK_LEX_RESULT result = LEX_ERROR;
	char *pStop = NULL;

	if (pBuffStart >= pBuffEnd)
		result = LEX_OUTOFDATA;
	else
	{
		long nResult = 0;
		errno_t errnoValue = AtlStrToNum(&nResult, pBuffStart, &pStop, 16);
		if (errnoValue != ERANGE &&
			nResult >= 0 &&
			nResult < 0xFFFFFFFF &&
			pStop <= pBuffEnd &&
			*pStop == '\r')
		{
			// move pBuffStart
			// return chunk size
			*pnChunkSize = nResult;
			pBuffStart = pStop;
			result = LEX_OK;
		}
		if (*pStop != '\r')
		{
			result = LEX_OUTOFDATA; // not enough data in the buffer
		}
	}
	return result;
}

template<class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::move_leftover_bytes(	__in_ecount(nLen) char *pBufferStart, 
								__in int nLen, 
								__deref_inout_ecount(nLen) char *&pBuffStart, 
								__deref_inout char *& pBuffEnd) throw()
{
	bool bRet = true;
	Checked::memcpy_s(pBufferStart, (pBuffEnd-pBuffStart), pBuffStart, nLen);
	return bRet;
}

template<class TSocketClass>
inline typename CAtlHttpClientT<TSocketClass>::CHUNK_LEX_RESULT CAtlHttpClientT<TSocketClass>::get_chunked_data(char *&pBufferStart,
								  char *&pBufferEnd,
								  long nChunkSize,
								  char **ppDataStart,
								  long *pnDataLen) throw()
{
	CHUNK_LEX_RESULT result = LEX_ERROR;
	if (pBufferStart + nChunkSize - 1 < pBufferEnd)
	{
		*ppDataStart = pBufferStart;
		*pnDataLen = nChunkSize;
		pBufferStart = pBufferStart + nChunkSize;
		result = LEX_OK;
	}
	else if (pBufferStart + nChunkSize - 1 >= pBufferEnd)
		result = LEX_OUTOFDATA;

	return result;
}

template<class TSocketClass>
inline typename CAtlHttpClientT<TSocketClass>::CHUNK_LEX_RESULT CAtlHttpClientT<TSocketClass>::consume_chunk_trailer(char *&pBufferStart, char *pBufferEnd)
{
	CHUNK_LEX_RESULT result = LEX_ERROR;
	if (pBufferStart >= pBufferEnd)
		return result;

	char *pHeaderEnd = NULL;
	char *pTemp = pBufferStart;
	// check for empty trailer, this means there are no more trailers
	if ( (pTemp < pBufferEnd && *pTemp == '\r') &&
			(pTemp+1 < pBufferEnd && *(pTemp+1) == '\n'))
	{
		pBufferStart += 2;
		return LEX_TRAILER_COMPLETE;
	}

	while (pTemp <= pBufferEnd)
	{
		if ( (pTemp < pBufferEnd && *pTemp == '\r') &&
			 (pTemp+1 < pBufferEnd && *(pTemp+1) == '\n'))
		{
			 pHeaderEnd = pTemp; // success case
			 result = LEX_OK;
			 break;
		}
		pTemp++;
	}

	if (result == LEX_OK)
	{
		DecodeHeader(pBufferStart, pHeaderEnd);
		pBufferStart = pHeaderEnd + 2;
	}
	else if (result != LEX_OK &&
		pTemp > pBufferEnd)
		result = LEX_OUTOFDATA;
	return result;
}

template<class TSocketClass>
inline typename CAtlHttpClientT<TSocketClass>::CHUNK_LEX_RESULT CAtlHttpClientT<TSocketClass>::consume_chunk_footer(char *&pBufferStart, char *&pBufferEnd)
{
	CHUNK_LEX_RESULT result = LEX_ERROR;
	if (pBufferStart < pBufferEnd &&
		(pBufferStart+1) <= pBufferEnd)
	{
		if ( *pBufferStart == '\r' &&   
			 *(pBufferStart+1) == '\n')
		{
			pBufferStart += 2;
			result = LEX_OK;
		}
	}
	else
		result = LEX_OUTOFDATA;
	return result;
}

#define CHUNK_BUFF_SIZE 2048

template<class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::ReadChunkedBody() throw()
{
	// At this point, m_current contains the headers, up to and including the \r\n\r\n,
	// plus any additional data that might have been read off the socket. So, we need
	// to copy off the additional data into our read buffer before we start parsing the
	// chunks.
#ifdef _DEBUG
	// nReadCount, keeps track of how many socket reads we do.
	int nReadCount = 0;
#endif

	// nChunkBuffCarryOver
	// When we run out of data in the input buffer, this is the
	// count of bytes that are left in the input that could not
	// be lexed into anything useful. We copy this many bytes to
	// the top of the input buffer before we fill the input buffer
	// with more bytes from the socket
	long nChunkBuffCarryOver = 0;

	// nChunkSize
	// The size of the next chunk to be read from the input buffer.
	long nChunkSize = 0;

	// t_chunk_buffer
	// The heap allocated buffer that we holds data
	// read from the socket. We will increase the size
	// of this buffer to 2 times the max chunk size we
	// need to read if we have to.
	CHeapPtr<char> t_chunk_buffer;

	// nTChunkBuffSize
	// Keeps track of the allocated size of t_chunk_buffer.
	// This size will change if we need to read chunks bigger
	// than the currently allocated size of t_chunk_buffer.
	long nTChunkBuffSize = CHUNK_BUFF_SIZE;

	// chunk_buffer & chunk_buffer_end
	// Keeps track of the current location
	// in t_chunk_buffer that we are lexing input from.
	// chunk_buffer_end is the end of the input buffer we
	// are lexing from. chunk_buffer_end is used as a marker
	// to make sure we don't read past the end of our input buffer
	char *chunk_buffer, *chunk_buffer_end;

	// cstate
	// The current state of the chunk parsing state machine. We
	// start out reading the size of the first chunk.
	CHUNK_STATE cstate = READ_CHUNK_SIZE;

	// cresult
	// Holds the value of the result of a lexing operation performed
	// on the input buffer.
	CHUNK_LEX_RESULT cresult = LEX_OK;

	CAtlIsapiBuffer<> result_buffer;

	// Initialize pointers and allocate the chunk buffer.
	chunk_buffer = chunk_buffer_end = NULL;
	if( !t_chunk_buffer.Allocate(nTChunkBuffSize) )
		return false;

	// copy the headers into a temporary buffer.
	result_buffer.Append(m_current + m_dwHeaderStart, m_dwHeaderLen);

	// calculate number of bytes left in m_current past the headers
	long leftover_in_m_current = m_current.GetLength() - (m_dwHeaderStart + m_dwHeaderLen);

	// copy the extra bytes that might have been read into m_current into the chunk buffer
	if (leftover_in_m_current > 0)
	{
		if (leftover_in_m_current > nTChunkBuffSize)
		{
			if( ! t_chunk_buffer.Reallocate(leftover_in_m_current) )
				return false;
		}

		chunk_buffer = (char*)t_chunk_buffer;
		Checked::memcpy_s(chunk_buffer, leftover_in_m_current, ((LPCSTR)m_current)+ m_dwHeaderStart + m_dwHeaderLen, leftover_in_m_current);
		chunk_buffer_end = chunk_buffer + leftover_in_m_current;
	}

	m_current.Empty();
	m_dwBodyLen = 0;
	m_dwHeaderStart = 0;

	// as we start the state machine, we should be either pointing at the first
	// byte of chunked response or nothing, in which case we will need to get 
	// more data from the socket.
	nChunkSize = 0;

	bool bDone = false;

	while(!bDone)
	{
		// if we run out of data during processing, chunk_buffer
		// get set to null
		if (!chunk_buffer ||
			chunk_buffer >= chunk_buffer_end)
		{
			// we ran out of data in our input buffer, we need
			// to read more from the socket.
			DWORD dwReadBuffSize = nTChunkBuffSize - nChunkBuffCarryOver;
			chunk_buffer = t_chunk_buffer;
			if (!Read((const unsigned char*)(chunk_buffer+nChunkBuffCarryOver), &dwReadBuffSize))
			{
				ATLTRACE("ReadChunkedBody: Error reading from socket (%d)\n", GetLastError());
				return false;
			}
			else if(dwReadBuffSize == 0)
			{
				ATLTRACE("ReadChunkedBody: The socket read timed out and no bytes were read from the socket.\n");
				return false;
			}
#ifdef _DEBUG
			ATLTRACE("ReadChunkedBody read %d bytes from socket. Reads %d \n", dwReadBuffSize, ++nReadCount);
#endif
			chunk_buffer_end = chunk_buffer + nChunkBuffCarryOver + dwReadBuffSize;
			nChunkBuffCarryOver = 0;
		}

		switch(cstate)
		{
		case READ_CHUNK_SIZE:
			{
				cresult = get_chunked_size(chunk_buffer, chunk_buffer_end, &nChunkSize);
				switch(cresult)
				{
				case LEX_ERROR:
					ATLTRACE("ReadChunkedBody Failed retrieving chunk size\n");
					return false;
					break;
				case LEX_OUTOFDATA:
					nChunkBuffCarryOver = (long)(chunk_buffer_end - chunk_buffer);
					if (!move_leftover_bytes((char*)t_chunk_buffer, nChunkBuffCarryOver, 
										chunk_buffer, chunk_buffer_end))
					{
						ATLTRACE("failed to move leftover chunk data to head of buffer\n");
						return false;
					}
					chunk_buffer = chunk_buffer_end = NULL;
					break;
				case LEX_OK:
					if (nChunkSize == 0)
					{
						cstate = CHUNK_READ_DATA_COMPLETE;
					}
					else if (nChunkSize + 2 > nTChunkBuffSize)
					{
						char *pBuffStart = (char*)t_chunk_buffer;
						int nReadSoFar = (int)(chunk_buffer - pBuffStart);
						int nTotal = (int)(chunk_buffer_end - pBuffStart);			
						if( FAILED(::ATL::AtlMultiply(&nTChunkBuffSize, nChunkSize, 2L)))
						{
							return false;
						}
						t_chunk_buffer.Reallocate(nTChunkBuffSize);
						pBuffStart = (char*)t_chunk_buffer;
						chunk_buffer = pBuffStart + nReadSoFar;
						chunk_buffer_end = pBuffStart + nTotal;
						cstate = READ_CHUNK_SIZE_FOOTER;
						m_dwBodyLen += nChunkSize;
					}
					else
					{
						// everything is OK. move to next state
						cstate = READ_CHUNK_SIZE_FOOTER;
						m_dwBodyLen += nChunkSize;
					}
					break;
				default:
					ATLASSERT(0);
					return false;
					break;
				}
			}
			break;
		case READ_CHUNK_DATA:
			{
				char *pDataStart = NULL;
				long nDataLen = 0;
				cresult = LEX_OK;
				cresult = get_chunked_data(chunk_buffer, chunk_buffer_end,
											nChunkSize, &pDataStart, &nDataLen);
				switch(cresult)
				{
				case LEX_ERROR:
					ATLTRACE("ReadChunkedBody failed to retrieve chunk data\n");
					return false;
					break;
				case LEX_OUTOFDATA:
					nChunkBuffCarryOver = (long)(chunk_buffer_end - chunk_buffer);
					if (!move_leftover_bytes((char*)t_chunk_buffer, nChunkBuffCarryOver, 
										chunk_buffer, chunk_buffer_end))
					{
						ATLTRACE("failed to move leftover chunk data to head of buffer\n");
						return false;
					}
					chunk_buffer = chunk_buffer_end = NULL;
					break;
				case LEX_OK:
					result_buffer.Append(pDataStart, nDataLen);
					cstate = READ_CHUNK_DATA_FOOTER;
					break;
				default:
					ATLASSERT(0);
					return false;
				}
			}
			break;
			case READ_CHUNK_SIZE_FOOTER:
			case READ_CHUNK_DATA_FOOTER:
			{
				cresult = consume_chunk_footer(chunk_buffer, chunk_buffer_end);
				switch(cresult)
				{
				case LEX_OK:
					cstate = (cstate == READ_CHUNK_SIZE_FOOTER) ? READ_CHUNK_DATA : READ_CHUNK_SIZE;
					break;
				case LEX_ERROR:
					ATLTRACE("Error consuming chunk footer!\n");
					return false;
					break;
				case LEX_OUTOFDATA:
					nChunkBuffCarryOver = (long)(chunk_buffer_end - chunk_buffer);
					if (!move_leftover_bytes((char*)t_chunk_buffer, nChunkBuffCarryOver, 
										chunk_buffer, chunk_buffer_end))
					{
						ATLTRACE("failed to move leftover chunk data to head of buffer\n");
						return false;
					}
					chunk_buffer = chunk_buffer_end = NULL;
					break;
				default:
					ATLASSERT(0);
					return false;

				}
			}
			break;
			case CHUNK_READ_DATA_COMPLETE:
			{
				// We read the chunk of size 0
				// consume the chunk footer.
				DWORD dwLen = 0;
				cresult = consume_chunk_footer(chunk_buffer, chunk_buffer_end);
				if (GetHeaderValue((_T("Trailer")), NULL, &dwLen))
				{
					cstate = READ_CHUNK_TRAILER; // start reading trailer headers
					break;
				}
				else
					bDone = true;
			}
			break;
			case READ_CHUNK_TRAILER:
				cresult = consume_chunk_trailer(chunk_buffer, chunk_buffer_end);
				switch(cresult)
				{
				case LEX_OK:
					cstate = READ_CHUNK_TRAILER; // keep reading
					break;
				case LEX_ERROR:
					ATLTRACE("Error consuming chunk trailers!\n");
					return false;
					break;
				case LEX_OUTOFDATA:
					nChunkBuffCarryOver = (long)(chunk_buffer_end - chunk_buffer);
					if (!move_leftover_bytes((char*)t_chunk_buffer, nChunkBuffCarryOver, 
										chunk_buffer, chunk_buffer_end))
					{
						ATLTRACE("failed to move leftover chunk data to head of buffer\n");
						return false;
					}
					chunk_buffer = chunk_buffer_end = NULL;
					break;
				case LEX_TRAILER_COMPLETE:
					return true;
					break;
				default:
					ATLASSERT(0);
					return false;



				}
				break;

		}
	}
	if (!m_current.Append((LPCSTR)result_buffer))
		return false;
		
	m_pEnd = ((BYTE*)(LPCSTR)m_current) + m_current.GetLength();
	
	return true;
}

template<class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::ReconnectIfRequired() throw()
{
	CString strValue;
	// if we have a keep-alive header then return true
	// else we have to close and re-open the connection
	if (GetHeaderValue(_T("Connection"), strValue))
	{
		if (!strValue.CompareNoCase(_T("keep-alive")))
			return true; // server said keep connection open.
	}
	else
	{
		return true; // there was no 'Connection' header
	}

	if (!strValue.CompareNoCase(_T("close")))
	{
		Close();
		ConnectSocket();
	}   
	return false;
}

// Complete relative URLs and URLs
// that have a missing path. These are common with redirect headers.
// http://www.microsoft.com becomes http://www.microsoft.com/
// localstart.asp becomes whatever our current (m_urlCurrent) 
// path is plus localstart.asp
template<class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::CompleteURL(CString& strURL) throw()
{
	_ATLTRY
	{
		CString strUrlTemp = strURL;
		strUrlTemp.Trim();
		CUrl url;
		bool bErr = false;
		if (url.CrackUrl(strUrlTemp))
		{
			return true; // URL is already valid
		}


		// if we have a scheme and a host name but no
		// path, then add the path of '/'
		if (url.GetScheme() == ATL_URL_SCHEME_HTTP &&
			url.GetHostNameLength() > 0 &&
			!url.GetUrlPathLength() )
		{
			url.SetUrlPath(_T("/"));
			bErr = true;
		} 
		// if we have leading / (absolute path) (ex: /Test/bbb.asp) we can concatinate it 
		// to it to our current URL (m_urlCurrent) scheme and host 		
		else if (strUrlTemp[0] ==  _T('/'))
		{
			url = m_urlCurrent;
			url.SetUrlPath(strUrlTemp);
			bErr = true;
		}
		// relative path (ex: bbb.asp) - we don't have a valid url
		// and the first char is not /
		// Get the url from our current URL (m_urlCurrent) and add
		// our relative paths
		else
		{
			CString szPath;
			url = m_urlCurrent;

			if (!url.GetUrlPathLength())
			{
				szPath = _T('/'); // current URL has no path!
			}
			else
			{
				szPath = url.GetUrlPath();
			}
				
			// back up to the first / and insert our current url
			int pos = szPath.ReverseFind(_T('/'));
			if(pos == -1)
			{
				return false;
			}
				
			szPath.GetBufferSetLength(pos+1);
			szPath.ReleaseBuffer();

			szPath += strURL;
			url.SetUrlPath(szPath);
			bErr = true;
		}
		if (!bErr)
		{
			return bErr;
		}
		DWORD dwLen = ATL_URL_MAX_PATH_LENGTH;

		return url.CreateUrl(strURL.GetBuffer(ATL_URL_MAX_PATH_LENGTH),
			&dwLen) ? true : false;
	}
	_ATLCATCHALL()
	{
		return false;
	}
}

template<class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::ProcessObjectMoved() throw()
{
	_ATLTRY
	{
		// look for a location header
		CString strValue;
		CString strURLNew;
		if (GetHeaderValue(_T("Location"), strValue))
		{			
			CString strRedirectReqHeaders=m_pNavData->szExtraHeaders;	
			ReconnectIfRequired();
			m_HeaderMap.RemoveAll();
			m_current.Empty();


			// create a new URL based on what is in the
			// Location header and set it as this object's 
			// default Url
			strURLNew = strValue;
			CompleteURL(strURLNew);
			CString strCurrHostName = m_urlCurrent.GetHostName();
			ATL_URL_PORT nCurrPort=m_urlCurrent.GetPortNumber();
			 
			SetDefaultUrl((LPCTSTR)strURLNew, m_urlCurrent.GetPortNumber());
			//If redirected (new url in strURLNew) to different host (server) or port, need a new socket.
			if (m_urlCurrent.GetHostName()!=strCurrHostName || m_urlCurrent.GetPortNumber()!=nCurrPort)
			{
				Close();
				ConnectSocket();
			}
			// build up a request. 			
			CString strRequest;
			BuildRequest(&strRequest,
						m_strMethod,
						strRedirectReqHeaders.GetString());

			// send the request
			DWORD dwSent = strRequest.GetLength();
			DWORD dwAvailable = dwSent;
			if (!Write((BYTE*)((LPCSTR)CT2A(strRequest.GetBuffer(dwAvailable))), &dwSent))
				return false;
			strRequest.ReleaseBuffer();

			if (dwSent != dwAvailable)
				return false;

			// read the response
			if (RR_OK == ReadHttpResponse())
			{
				if (m_pNavData)
					ProcessStatus(m_pNavData->dwFlags);
			}
		}
		return true;
	}
	_ATLCATCHALL()
	{
		return false;
	}
}

template<class TSocketClass>
inline bool CAtlHttpClientT<TSocketClass>::_SetDefaultUrl(LPCTSTR szURL, short nPort) throw()
{

	if (szURL)
		if (!m_urlCurrent.CrackUrl(szURL)) // re-inits the field of the CUrl first
			return false;

	ATL_URL_SCHEME currScheme = m_urlCurrent.GetScheme();
	if ( currScheme != ATL_URL_SCHEME_HTTP &&
		 !TSocketClass::SupportsScheme(currScheme) )
		return false; // only support HTTP

	if (!m_urlCurrent.GetUrlPathLength())
	{
		// no path, default to /
		m_urlCurrent.SetUrlPath(_T("/"));
	}

	if (!m_urlCurrent.GetHostNameLength())
	{
		// no server name
		return false;
	}

	if (m_urlCurrent.GetPortNumber() == ATL_URL_INVALID_PORT_NUMBER)
		m_urlCurrent.SetPortNumber(nPort);
	return true;
}

template<class TSocketClass>
inline int CAtlHttpClientT<TSocketClass>::GetStatus() throw()
{
	return m_nStatus;
}

template<class TSocketClass>
inline LPCTSTR CAtlHttpClientT<TSocketClass>::GetMethod() throw()
{
	return m_strMethod;
}

template<class TSocketClass>
inline BYTE* CAtlHttpClientT<TSocketClass>::GetPostData() throw()
{
	if (m_pNavData)
		return m_pNavData->pData;
	return NULL;
}

template<class TSocketClass>
inline DWORD CAtlHttpClientT<TSocketClass>::GetPostDataLen() throw()
{
	if (m_pNavData)
		return m_pNavData->dwDataLen;
	return 0;
}

template<class TSocketClass>
inline LPCTSTR CAtlHttpClientT<TSocketClass>::GetPostDataType() throw()
{
	if (m_pNavData)
		return m_pNavData->szDataType;
	return NULL;
}

template<class TSocketClass>
inline DWORD CAtlHttpClientT<TSocketClass>::GetLastError() throw()
{
	return m_dwLastError;
}

template<class TSocketClass>
inline const SOCKET& CAtlHttpClientT<TSocketClass>::GetSocket() throw()
{
	return const_cast<const SOCKET&>(m_socket);
}

template<class TSocketClass>
inline void CAtlHttpClientT<TSocketClass>::Close() throw()
{
	TSocketClass::Close();
}

template<class TSocketClass>
inline DWORD CAtlHttpClientT<TSocketClass>::SetSocketTimeout(DWORD dwNewTimeout) throw()
{
	return TSocketClass::SetSocketTimeout(dwNewTimeout);
}

template<class TSocketClass>
inline DWORD CAtlHttpClientT<TSocketClass>::GetSocketTimeout() throw()
{
	return TSocketClass::GetSocketTimeout();
}

template<class TSocketClass>
inline void CAtlHttpClientT<TSocketClass>::AuthProtocolFailed(LPCTSTR szProto) throw()
{
	CAtlBaseAuthObject *pAuthObj = NULL;
	_ATLTRY
	{
		if (m_AuthMap.Lookup(szProto, pAuthObj) && pAuthObj)
		{
			pAuthObj->m_bFailed = true;
		}
	}
	_ATLCATCHALL()
	{
	}
}

template<class TSocketClass>
inline const ATL_NAVIGATE_DATA* CAtlHttpClientT<TSocketClass>::GetCurrentNavdata()
{
	return m_pNavData;
}


/////////////////////////////////////////////////////////////////////////////////
//
// CNTLMAuthObject
// NTLM Security Authorization functions 
//
/////////////////////////////////////////////////////////////////////////////////
inline CNTLMAuthObject::CNTLMAuthObject() throw() :
	m_pSocket(NULL),
	m_nMaxTokenSize(0),
	m_pAuthInfo(NULL),
	m_bProxy(false)
{
	SecInvalidateHandle(&m_hCredentials)
}

inline CNTLMAuthObject::CNTLMAuthObject(IAuthInfo *pAuthInfo) throw() :
	m_pSocket(NULL),
	m_nMaxTokenSize(0),
	m_pAuthInfo(pAuthInfo)
{
	SecInvalidateHandle(&m_hCredentials)
}

inline CNTLMAuthObject::~CNTLMAuthObject() throw()
{
	if (!ATL_IS_INVALIDCREDHANDLE(m_hCredentials))
		FreeCredentialsHandle(&m_hCredentials);
}

inline void CNTLMAuthObject::Init(CAtlHttpClient *pSocket, IAuthInfo *pAuthInfo) throw()
{
	m_pSocket = pSocket;
	SetAuthInfo(pAuthInfo);
}

inline void CNTLMAuthObject::SetAuthInfo(IAuthInfo *pAuthInfo) throw()
{
	m_pAuthInfo = pAuthInfo;
}

inline bool CNTLMAuthObject::Authenticate(LPCTSTR /*szAuthTypes*/, bool bProxy) throw()
{
	m_bProxy = bProxy;
	if (AcquireCredHandle())
		return DoNTLMAuthenticate();
	return false;
}

inline bool CNTLMAuthObject::AcquireCredHandle() throw()
{
	PSecPkgInfo pPackageInfo = NULL;
	SECURITY_STATUS SecurityStatus = SEC_E_OK;

	// Acquire a credentials handle on the NTLM security package
	SecurityStatus = QuerySecurityPackageInfo(ATL_HTTP_AUTHTYPE_NTLM,
							&pPackageInfo);

	if (SecurityStatus != SEC_E_OK)
		return false;

	void *pAuthData = NULL;
	CSecAuthIdentity CA;
	if (m_pAuthInfo)
	{
		// if m_pAuthInfo has been set then the caller wants us
		// to get credentials from them.
		if (CA.Init(m_pAuthInfo))
			pAuthData = static_cast<void*>(&CA);
	}

	SecurityStatus = AcquireCredentialsHandle(
					0,
					pPackageInfo->Name,
					SECPKG_CRED_OUTBOUND,
					0,
					pAuthData,
					0,
					0,
					&m_hCredentials,
					&m_ts
					);

	m_nMaxTokenSize = pPackageInfo->cbMaxToken;
	FreeContextBuffer(pPackageInfo);
	return SecurityStatus == SEC_E_OK ? true : false;
}

inline bool CNTLMAuthObject::DoNTLMAuthenticate() throw()
{
	bool bRet = false;
						
	m_CurrentRequestData = (*(const_cast<const ATL_NAVIGATE_DATA*>(m_pSocket->GetCurrentNavdata())));
	// make sure we have a good credentials handle
	ATLASSERT(!ATL_IS_INVALIDCREDHANDLE(m_hCredentials));
	if (ATL_IS_INVALIDCREDHANDLE(m_hCredentials))
		return false;

	SECURITY_STATUS SecurityStatus = SEC_E_OK;

	unsigned long ContextAttributes = 0;
	CSecBufferDesc OutBufferDesc;
	CtxtHandle SecurityContext;
	SecInvalidateHandle(&SecurityContext);

	// Create a SecBufferDesc with one buffer of m_nMaxTokenSize
	if (!OutBufferDesc.AddBuffers(1, m_nMaxTokenSize))
		return false;

	SecurityStatus = InitializeSecurityContext(
				&m_hCredentials,
				0,
				NULL,			
				ISC_REQ_CONNECTION,
				0,
				0,
				0,
				0,
				&SecurityContext,
				OutBufferDesc,
				&ContextAttributes,
				&m_ts
				);

	if ( (SecurityStatus == SEC_I_COMPLETE_NEEDED) ||
		 (SecurityStatus == SEC_I_COMPLETE_AND_CONTINUE) )
	{
		SecurityStatus = CompleteAuthToken( &SecurityContext, (PSecBufferDesc)OutBufferDesc);
	}

	if (IS_ERROR(SecurityStatus))
		return false;

	// create an Authentication header with the contents of the
	// security buffer and send it to the HTTP server. The output
	// buffer will be pointing to a buffer that contains the 
	// response from the HTTP server on return.
	LPSTR pszbuff = NULL;
	if (!SendSecurityInfo(OutBufferDesc.Buffers(0), &pszbuff) || !pszbuff)
		return false;

	CString strVal;
	if (!m_pSocket->GetHeaderValue(m_bProxy ? g_pszProxyAuthenticate : g_pszWWWAuthenticate, strVal))
		return false; // wrong authentication type

	LPCTSTR szResponsecode = strVal;
	TCHAR pszcode[ATL_AUTH_HDR_SIZE];
	if (szResponsecode)
	{
		// first four characters better be 'NTLM'
		if (_tcsncicmp(szResponsecode, _T("NTLM"), 4) != 0)
			return false;

		// skip NTLM
		szResponsecode += 4;

		// skip space
		while (*szResponsecode && _AtlIsHttpSpace(*szResponsecode))
			szResponsecode++;

		// find end of header
		LPCTSTR pszend = szResponsecode;
		while (*pszend && *pszend != _T('\r'))
			pszend++;
		bRet = false;
		if (pszend)
		{
			// copy authentication data to our buffer
			// and base64decode it.
			int nlen = (int)(pszend-szResponsecode);
			Checked::memcpy_s(pszcode, ATL_AUTH_HDR_SIZE, szResponsecode, nlen*sizeof(TCHAR));
			pszcode[pszend-szResponsecode]=0;

			// re-use OutBufferDesc here since we'll need to need
			// a SecBufferDesc to pass to the next call to InitializeSecurityContext
			// anyways.
			if(!OutBufferDesc.Buffers(0)->ClearBuffer(m_nMaxTokenSize))
				return false;
				
			_ATLTRY
			{
				CT2A pszcode_a(pszcode);
				bRet = Base64Decode(pszcode_a,
									(int) strlen(pszcode_a), 
									(BYTE*)OutBufferDesc.Buffers(0)->pvBuffer,
									(int*) &OutBufferDesc.Buffers(0)->cbBuffer) != FALSE;
			}
			_ATLCATCHALL()
			{
				bRet = false;
			}
		}

		if (!bRet)
			return false;

		// Create buffers for the challenge data
		CSecBufferDesc *InBufferDesc = &OutBufferDesc;
		CSecBufferDesc OutBufferDesc2;
		if (!OutBufferDesc2.AddBuffers(1, m_nMaxTokenSize))
			return false;

		// Process the challenge response from the server
		SecurityStatus = InitializeSecurityContext(
					0,
					&SecurityContext,
					NULL,
					0,
					0,
					0 ,
					InBufferDesc,
					0,
					&SecurityContext,
					OutBufferDesc2,
					&ContextAttributes,
					&m_ts
					);

		if (IS_ERROR(SecurityStatus))
			return false;

		pszbuff = NULL;
		if (SendSecurityInfo(OutBufferDesc2.Buffers(0), &pszbuff))
		{
			// at this point we should be authenticated and either have the page
			// we requested or be getting re-directed to another page under our
			// authorization. Either way, we don't want to go through authorization
			// code again if we are not authorized to prevent recursive authorization
			// so we tell the client not to try this protocol again.
			if (m_pSocket->GetStatus() == 401 ||
				m_pSocket->GetStatus() == 407)
			{
				// Authorization with this protocol failed.
				// don't try it again.
				m_pSocket->AuthProtocolFailed(_T("NTLM"));
			}
			bRet = m_pSocket->ProcessStatus(m_pSocket->GetFlags());
		}
	}

	return bRet;
}
inline bool CNTLMAuthObject::GetCredentialNames(CString& theName)
{
	if (ATL_IS_INVALIDCREDHANDLE(m_hCredentials))
		return false;

	SecPkgCredentials_Names spcn;
	if(!IS_ERROR(QueryCredentialsAttributes(&m_hCredentials, 
		SECPKG_CRED_ATTR_NAMES, (void*)&spcn)))
	{
		theName = spcn.sUserName;
		return true;
	}
	return false;
}

inline bool CNTLMAuthObject::SendSecurityInfo(SecBuffer *pSecBuffer, LPSTR *pszBuffer) throw()
{
	ATLASSERT(pSecBuffer);
	ATLASSUME(m_pSocket);
	ATLASSERT(pszBuffer);

	int nDest = ATL_AUTH_HDR_SIZE;
	char auth_b64encoded[ATL_AUTH_HDR_SIZE];
	char auth_header[ATL_AUTH_HDR_SIZE];
	const char *pszFmtStr = m_bProxy ? m_pszFmtProxy : m_pszFmtWWW;

	if (!pSecBuffer || !pSecBuffer->pvBuffer || !pszBuffer)
		return false;
	*pszBuffer = 0;

	// Base64Encode will fail gracefully if buffer not big enough
	if (Base64Encode((BYTE*)pSecBuffer->pvBuffer, pSecBuffer->cbBuffer,
		auth_b64encoded, &nDest, ATL_BASE64_FLAG_NOCRLF))
	{
		if (nDest < ATL_AUTH_HDR_SIZE)
		{
			auth_b64encoded[nDest]=0;
			// make sure we have enough room in our header buffer
			if ( (strlen(pszFmtStr)-2 + nDest) < ATL_AUTH_HDR_SIZE)
				sprintf_s(auth_header, ATL_AUTH_HDR_SIZE, pszFmtStr, auth_b64encoded);
			else
				return false;
		}
		else
			return false;
	}
	else
		return false;

	// reset the connection if required
	m_pSocket->ResetConnection();

	// Resend the request with the authorization information
	LPCURL pUrl = m_pSocket->GetCurrentUrl();   
	bool bRet = false;

	TCHAR szUrl[ATL_URL_MAX_URL_LENGTH];
	DWORD dwMaxLen = ATL_URL_MAX_URL_LENGTH;
	if( ! pUrl->CreateUrl(szUrl, &dwMaxLen) )
		return false;

	_ATLTRY
	{
		CA2CT hdr(auth_header);
		CAtlNavigateData navigate_data(m_CurrentRequestData);
		// append authorization header to extra headers
		CString strHeaders = navigate_data.GetExtraHeaders();
		strHeaders += hdr;
		navigate_data.SetExtraHeaders(strHeaders);
		navigate_data.RemoveFlags(ATL_HTTP_FLAG_PROCESS_RESULT);

		bRet = m_pSocket->Navigate( szUrl, &navigate_data);
	}
	_ATLCATCHALL()
	{
		bRet = false;
	}
	if (bRet)
		*pszBuffer = (LPSTR)m_pSocket->GetResponse();
	return bRet;
}

/////////////////////////////////////////////////////////////////////////////////
//
// CBasicAuthObject
// BASIC Security Authorization functions 
//
/////////////////////////////////////////////////////////////////////////////////
inline bool CBasicAuthObject::DoBasicAuthenticate() throw()
{
	bool bRet = false;
	ATLASSUME(m_pClient);
	ATLASSUME(m_pAuthInfo);
	// Create an authentication string
	CTempBuffer<TCHAR, (_ATL_MAX_AUTH_BUFF*2)+2> auth_string;
	CAuthInfoBuffType buffUID;
	CAuthInfoBuffType buffPWD;

	DWORD dwUID=0,dwPWD=0;
	if (!_AtlGetAuthInfoHelper(m_pAuthInfo, &IAuthInfo::GetPassword, buffPWD, &dwPWD) ||
		!_AtlGetAuthInfoHelper(m_pAuthInfo, &IAuthInfo::GetUsername, buffUID, &dwUID))
		return false;

	_ATLTRY
	{
		if (!auth_string.Allocate((_ATL_MAX_AUTH_BUFF*2)+2))
			return false;

		Checked::tcscpy_s(auth_string, _ATL_MAX_AUTH_BUFF, buffUID);
		Checked::tcscat_s(auth_string, _ATL_MAX_AUTH_BUFF, _T(":"));
		Checked::tcscat_s(auth_string, _ATL_MAX_AUTH_BUFF, buffPWD);

		// Base64 encode the auth string
		char *auth_string_enc = NULL;
		CTempBuffer<char, 512> auth_string_buff;
		CT2A auth_string_a(auth_string);

		int nLen = Base64EncodeGetRequiredLength((int)strlen((LPSTR)auth_string_a));
		auth_string_buff.Allocate(nLen+1);
		if (!((char*)auth_string_buff))
			return false;

		auth_string_enc = (char*)auth_string_buff;
		if (!Base64Encode((const BYTE*)(LPSTR)auth_string_a, (int)strlen((LPSTR)auth_string_a),
						  auth_string_enc, &nLen, ATL_BASE64_FLAG_NOCRLF))
			return false;
		auth_string_buff[nLen]=0;

		// Format the Authentication header
		int nLenFmt = (m_bProxy ? (int)strlen(m_pszFmtProxy) : (int)strlen(m_pszFmtWWW)) + 2;
		nLen += nLenFmt;
		++nLen; // Space for '\0'
		
		CTempBuffer<char, 512> auth_header_buff;
		ATLTRY(auth_header_buff.Allocate(nLen));
		if (!((char*)auth_header_buff))
			return false;

		char *auth_header = (char*)auth_header_buff;
		Checked::strcpy_s(auth_header, nLen, m_bProxy ? m_pszFmtProxy : m_pszFmtWWW);
		Checked::strcat_s(auth_header, nLen, auth_string_enc);
		Checked::strcat_s(auth_header, nLen, "\r\n");

		// Resend the request with the authorization information
		LPCURL pUrl = m_pClient->GetCurrentUrl();
		TCHAR szUrl[ATL_URL_MAX_URL_LENGTH];
		DWORD dwMaxLen = ATL_URL_MAX_URL_LENGTH;
		pUrl->CreateUrl(szUrl, &dwMaxLen);

		// reset the connection if required
		m_pClient->ResetConnection();

		CA2T hdr(auth_header);
		CAtlNavigateData navigate_data(*(const_cast<const ATL_NAVIGATE_DATA*>(m_pClient->GetCurrentNavdata())));
		// append authorization header to extra headers
		CString strHeaders = navigate_data.GetExtraHeaders();
		strHeaders += hdr;
		navigate_data.SetExtraHeaders(strHeaders);
		navigate_data.RemoveFlags(ATL_HTTP_FLAG_PROCESS_RESULT);
		bRet = m_pClient->Navigate( szUrl,
									&navigate_data);
	}
	_ATLCATCHALL()
	{
		bRet = false;
	}

	if (bRet)
	{
		// Request was successfully sent. Process the result.
		if (m_pClient->GetStatus() == 401 ||
			m_pClient->GetStatus() == 407)
		{
			// Authorization with this protocol failed.
			// don't try it again.
			m_pClient->AuthProtocolFailed(_T("basic"));
		}
		bRet = m_pClient->ProcessStatus(m_pClient->GetFlags());
	}
	return bRet;
}

inline CBasicAuthObject::CBasicAuthObject() throw()
{
	m_pClient = NULL;
	m_pAuthInfo = NULL;
	m_szRealm[0] = 0;
	m_bProxy = false;
}

inline CBasicAuthObject::CBasicAuthObject(IAuthInfo *pAuthInfo) throw()
{
	m_pAuthInfo = pAuthInfo;
	m_pClient = NULL;
}

inline void CBasicAuthObject::SetAuthInfo(IAuthInfo *pAuthInfo) throw()
{
	m_pAuthInfo = pAuthInfo;
}

// Called by the CAtlHttpClient class to 
// authenticate a user.
inline bool CBasicAuthObject::Authenticate(LPCTSTR szAuthTypes, bool bProxy) throw()
{
	if (lstrlen(szAuthTypes) > ATL_AUTH_HDR_SIZE)
		return false;

	m_bProxy = bProxy;

	if (!CrackRealm(szAuthTypes))
		return false;
	return DoBasicAuthenticate();
}

inline LPCTSTR CBasicAuthObject::GetRealm() throw()
{
	return const_cast<LPCTSTR>(m_szRealm);
}

// Called by the CAtlHttpClient class to initialize
// this authentication object.
inline void CBasicAuthObject::Init(CAtlHttpClient *pSocket, IAuthInfo *pAuthInfo) throw()
{
	ATLASSERT(pSocket);
	m_pClient = pSocket;
	if (pAuthInfo)
		SetAuthInfo(pAuthInfo);
}

inline bool CBasicAuthObject::CrackRealm(LPCTSTR szHeader) throw()
{
	// szHeader is pointing at the
	// "basic" in the header
	// see if realm is available
	const TCHAR *pStart = szHeader;

	// skip "basic"
	pStart += 5;

	// skip space
	while (*pStart && _AtlIsHttpSpace(*pStart))
		pStart++;

	// are we pointing at 'realm'?
	if ((*pStart == 'r' || *pStart == 'R') &&
		(*(pStart+1) == 'e' || *(pStart+1) == 'E') &&
		(*(pStart+2) == 'a' || *(pStart+2) == 'A') &&
		(*(pStart+3) == 'l' || *(pStart+3) == 'L') &&
		(*(pStart+4) == 'm' || *(pStart+4) == 'M'))
	{
		// skip 'realm'
		pStart += 5;

		// skip space
		while (*pStart && _AtlIsHttpSpace(*pStart))
			pStart++;

		// skip '='
		if (*pStart && *pStart == _T('='))
			pStart++;
		else
			return false; // invalid realm

		// skip space
		while (*pStart && _AtlIsHttpSpace(*pStart))
			pStart++;

		// skip quotes if they are there
		if (*pStart == '\"')
			pStart++;

		const TCHAR *pEnd = pStart;
		while (*pEnd && *pEnd != '\"')
		{
			if (*pEnd == '\\' && *(pEnd + 1)) // escaped character, skip it
				pEnd += 2;
			else
			   pEnd++;
	   	}

		if (*pEnd == '\"' && *(pEnd+1) != '\0')
			return false; //trailing junk after the quoted realm

		if (*pEnd=='\0' || *pEnd =='\"')
		{
			int nLen = (int)(pEnd-pStart);
			if (nLen < MAX_REALM_LEN)
			{
				Checked::tcsncpy_s(m_szRealm, _countof(m_szRealm), pStart, nLen);
				m_szRealm[nLen]=0;
				if (!AtlUnescapeUrl(m_szRealm, m_szRealm, NULL, MAX_REALM_LEN))
					return false; // error unescaping the string
			}
			else
				return false;
		}
	}
	return true;
}

inline CAtlBaseAuthObject::CAtlBaseAuthObject()
{
	m_bFailed = false;
}


inline CAtlNavigateData::CAtlNavigateData() throw()
{
	dwFlags =   ATL_HTTP_FLAG_AUTO_REDIRECT|
				ATL_HTTP_FLAG_PROCESS_RESULT|
				ATL_HTTP_FLAG_SEND_BLOCKS;
	szExtraHeaders = NULL;
	szMethod = ATL_HTTP_METHOD_GET;
	nPort = ATL_URL_DEFAULT_HTTP_PORT;
	pData = NULL;
	dwDataLen = 0;
	szDataType = NULL;
	dwTimeout = ATL_SOCK_TIMEOUT;
	dwSendBlockSize = ATL_HTTP_DEFAULT_BLOCK_SIZE;
	dwReadBlockSize = ATL_HTTP_DEFAULT_BLOCK_SIZE;
	pfnChunkCallback = NULL;
	pfnSendStatusCallback = NULL;
	pfnReadStatusCallback = NULL;
	m_lParamSend = 0;
	m_lParamRead = 0;
}

inline CAtlNavigateData::CAtlNavigateData(const CAtlNavigateData &rhs)
{
	this->operator=(rhs);
}

inline CAtlNavigateData::CAtlNavigateData(const ATL_NAVIGATE_DATA &rhs)
{
	this->operator=(rhs);
}

inline CAtlNavigateData& CAtlNavigateData::operator=(const CAtlNavigateData &rhs)
{
	return this->operator=(static_cast<const ATL_NAVIGATE_DATA&>(rhs));
}

inline CAtlNavigateData& CAtlNavigateData::operator=(const ATL_NAVIGATE_DATA &rhs)
{
	dwFlags = rhs.dwFlags;
	szExtraHeaders = rhs.szExtraHeaders;
	szMethod = rhs.szMethod;
	nPort = rhs.nPort;
	pData = rhs.pData;
	dwDataLen = rhs.dwDataLen;
	szDataType = rhs.szDataType;
	dwTimeout = rhs.dwTimeout;
	dwSendBlockSize = rhs.dwSendBlockSize;
	dwReadBlockSize = rhs.dwReadBlockSize;
	pfnChunkCallback = rhs.pfnChunkCallback;
	pfnSendStatusCallback = rhs.pfnSendStatusCallback;
	pfnReadStatusCallback = rhs.pfnReadStatusCallback;
	m_lParamSend = rhs.m_lParamSend;
	m_lParamRead = rhs.m_lParamRead;
	return *this;
}

inline DWORD CAtlNavigateData::SetFlags(DWORD dwNewFlags) throw()
{
	// check for mutually exclusive flags
	if ((dwNewFlags & ATL_HTTP_FLAG_SEND_CALLBACK) &&
		(dwNewFlags & ATL_HTTP_FLAG_SEND_BLOCKS))
	{
		ATLASSERT(0);
		return ATL_HTTP_FLAG_INVALID_FLAGS;
	}

	DWORD dwOldFlags = dwFlags;
	dwFlags = dwNewFlags;
	return dwOldFlags;
}

inline DWORD CAtlNavigateData::GetFlags() throw()
{
	return dwFlags;
}

inline DWORD CAtlNavigateData::AddFlags(DWORD dwFlagsToAdd) throw()
{
		// check for mutually exclusive flags
	if (
		((dwFlagsToAdd & ATL_HTTP_FLAG_SEND_CALLBACK) &&
		 (dwFlags & ATL_HTTP_FLAG_SEND_BLOCKS)) ||
		((dwFlagsToAdd & ATL_HTTP_FLAG_SEND_BLOCKS) &&
		 (dwFlags & ATL_HTTP_FLAG_SEND_CALLBACK))
	   )
	{
		ATLASSERT(0);
		return ATL_HTTP_FLAG_INVALID_FLAGS;
	}

	DWORD dwOldFlags = dwFlags;
	dwFlags |= dwFlagsToAdd;
	return dwOldFlags;
}

inline DWORD CAtlNavigateData::RemoveFlags(DWORD dwFlagsToRemove) throw()
{
	DWORD dwOldFlags = dwFlags;
	dwFlags &= ~dwFlagsToRemove;
	return dwOldFlags;
}

inline LPCTSTR CAtlNavigateData::SetExtraHeaders(LPCTSTR szNewHeaders) throw()
{
	LPCTSTR szold = szExtraHeaders;
	szExtraHeaders = szNewHeaders;
	return szold;
}

inline LPCTSTR CAtlNavigateData::GetExtraHeaders() throw()
{
	return szExtraHeaders;  
}
inline LPCTSTR CAtlNavigateData::SetMethod(LPCTSTR szNewMethod) throw()
{
	LPCTSTR szold = szMethod;
	szMethod = szNewMethod;
	return szold;
}
inline LPCTSTR CAtlNavigateData::GetMethod() throw()
{
	return szMethod;
}
inline short CAtlNavigateData::SetPort(short newPort) throw()
{
	short oldport = nPort;
	nPort = newPort;
	return oldport;
}
inline short CAtlNavigateData::GetPort() throw()
{
	return nPort;
}
inline void CAtlNavigateData::SetPostData(BYTE *pd, DWORD len, LPCTSTR type) throw()
{
	pData = pd;
	dwDataLen = len;
	szDataType = type;
}

inline DWORD CAtlNavigateData::SetSocketTimeout(DWORD dwNewTimeout) throw()
{
	DWORD dwold = dwTimeout;
	dwTimeout = dwNewTimeout;
	return dwold;
}
inline DWORD CAtlNavigateData::GetSocketTimeout() throw()
{
	return dwTimeout;
}
inline DWORD CAtlNavigateData::SetSendBlockSize(DWORD dwNewBlockSize) throw()
{
	DWORD dwold = dwSendBlockSize;
	dwSendBlockSize = dwNewBlockSize;
	return dwold;
}
inline DWORD CAtlNavigateData::GetSendBlockSize() throw()
{
	return dwSendBlockSize;
}

inline DWORD CAtlNavigateData::SetReadBlockSize(DWORD dwNewBlockSize) throw()
{
	DWORD dwold = dwReadBlockSize;
	dwReadBlockSize = dwNewBlockSize;
	return dwold;
}

inline DWORD CAtlNavigateData::GetReadBlockSize() throw()
{
	return dwReadBlockSize;
}

inline PFNATLCHUNKEDCB CAtlNavigateData::SetChunkCallback(PFNATLCHUNKEDCB pfn, DWORD_PTR dwParam) throw()
{
	PFNATLCHUNKEDCB pold = pfnChunkCallback;
	pfnChunkCallback = pfn;
	m_lParamChunkCB = dwParam;
	return pold;
}
inline PFNATLCHUNKEDCB CAtlNavigateData::GetChunkCallback() throw()
{
	return pfnChunkCallback;
}

inline PFNATLSTATUSCALLBACK CAtlNavigateData::SetSendStatusCallback(PFNATLSTATUSCALLBACK pfn, DWORD_PTR dwData) throw()
{
	PFNATLSTATUSCALLBACK pold = pfnSendStatusCallback;
	pfnSendStatusCallback = pfn;
	m_lParamSend = dwData;
	return pold;
}

inline PFNATLSTATUSCALLBACK CAtlNavigateData::GetSendStatusCallback() throw()
{
	return pfnSendStatusCallback;
}

inline PFNATLSTATUSCALLBACK CAtlNavigateData::SetReadStatusCallback(PFNATLSTATUSCALLBACK pfn, DWORD_PTR dwData) throw()
{
	PFNATLSTATUSCALLBACK pOld = pfnReadStatusCallback;
	pfnReadStatusCallback = pfn;
	m_lParamRead = dwData;
	return pOld;
}

inline PFNATLSTATUSCALLBACK CAtlNavigateData::GetReadStatusCallback() throw()
{
	return pfnReadStatusCallback;
}

} // namespace ATL

#pragma warning(pop)

#endif // __ATLHTTP_INL__
