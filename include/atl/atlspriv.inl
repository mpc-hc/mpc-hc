// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

/////////////////////////////////////////////////////////////////////////////////
//
// ZEvtSyncSocket
// ************ This is an implementation only class ************
// Class ZEvtSyncSocket is a non-supported, implementation only 
// class used by the ATL HTTP client class CAtlHttpClient. Do not
// use this class in your code. Use of this class is not supported by Microsoft.
//
/////////////////////////////////////////////////////////////////////////////////

#ifndef __ATLSPRIV_INL__
#define __ATLSPRIV_INL__

#pragma once

#pragma warning(push)
#pragma warning(disable:4312)

inline ZEvtSyncSocket::ZEvtSyncSocket() 
{
	m_dwCreateFlags = WSA_FLAG_OVERLAPPED;
	m_hEventRead = m_hEventWrite = m_hEventConnect = NULL;
	m_socket = INVALID_SOCKET;
	m_bConnected = false;
	m_dwLastError = 0;
	m_dwSocketTimeout = ATL_SOCK_TIMEOUT;
	g_HttpInit.Init();
}

inline ZEvtSyncSocket::~ZEvtSyncSocket() 
{
	Close();
}

inline ZEvtSyncSocket::operator SOCKET() 
{
	return m_socket;
}

inline void ZEvtSyncSocket::Close()
{
	if (m_socket != INVALID_SOCKET)
	{
		m_bConnected = false;
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		Term();
	}
}

inline void ZEvtSyncSocket::Term() 
{
	if (m_hEventRead)
	{
		WSACloseEvent(m_hEventRead);
		m_hEventRead = NULL;
	}
	if (m_hEventWrite)
	{
		WSACloseEvent(m_hEventWrite);
		m_hEventWrite = NULL;
	}
	if (m_hEventConnect)
	{
		WSACloseEvent(m_hEventConnect);
		m_hEventConnect = NULL;
	}
	m_socket = INVALID_SOCKET;
}

inline bool ZEvtSyncSocket::Create(const ADDRINFOT* pAI, WORD wFlags)
{
	return Create(pAI->ai_family, pAI->ai_socktype, pAI->ai_protocol, wFlags);
}

inline bool ZEvtSyncSocket::Create(int af, int st, int proto, WORD wFlags) 
{
	bool bRet = true;
	if (m_socket != INVALID_SOCKET)
	{
		m_dwLastError = WSAEALREADY;
		return false; // Must close this socket first
	}

	m_socket = WSASocket(af, st, proto, NULL, 0,
		wFlags | m_dwCreateFlags);
	if (m_socket == INVALID_SOCKET)
	{
		m_dwLastError = ::WSAGetLastError();
		bRet = false;
	}
	else
		bRet = Init(m_socket, NULL);
	return bRet;
}

inline bool ZEvtSyncSocket::Connect(LPCTSTR szAddr, unsigned short nPort) throw()
{
	if (m_bConnected)
		return true;

	bool bRet = true;
	CSocketAddr address;
	// Find address information
	if ((m_dwLastError = address.FindAddr(szAddr, nPort, 0, PF_UNSPEC, SOCK_STREAM, 0)) != ERROR_SUCCESS)
	{
		bRet = false;
	}
	else
	{
		bRet = Connect(address.GetAddrInfo());
	}
	return bRet;
}

inline bool ZEvtSyncSocket::Connect(const ADDRINFOT *pAI)
{
	if (m_socket == INVALID_SOCKET && !Create(pAI))
		return false;

	return Connect((SOCKADDR*)pAI->ai_addr, (int)pAI->ai_addrlen);
}

inline bool ZEvtSyncSocket::Connect(const SOCKADDR* psa, int len) 
{
	if (m_bConnected)
		return true; // already connected

	DWORD dwLastError;
	bool bRet = true;

	// if you try to connect the socket without
	// creating it first it's reasonable to automatically
	// try the create for you.
	if (m_socket == INVALID_SOCKET)
		return false;

	if (WSAConnect(m_socket, 
		psa, len,
		NULL, NULL, NULL, NULL))
	{
		dwLastError = WSAGetLastError();
		if (dwLastError != WSAEWOULDBLOCK)
		{
			m_dwLastError = dwLastError;
			bRet = false;
		}
		else
		{
			dwLastError = WaitForSingleObject((HANDLE)m_hEventConnect, m_dwSocketTimeout);
			if (dwLastError == WAIT_OBJECT_0)
			{
				// make sure there were no connection errors.
				WSANETWORKEVENTS wse;
				ZeroMemory(&wse, sizeof(wse));
				WSAEnumNetworkEvents(m_socket, NULL, &wse);
				if (wse.iErrorCode[FD_CONNECT_BIT]!=0)
				{
					m_dwLastError = (DWORD)(wse.iErrorCode[FD_CONNECT_BIT]);
					bRet = false;
				}
			}
			else
				bRet = false;
		}

	}

	m_bConnected = bRet;
	return bRet;
}

inline bool ZEvtSyncSocket::Write(WSABUF *pBuffers, int nCount, DWORD *pdwSize) 
{
	// if we aren't already connected we'll wait to see if the connect
	// event happens
	if (WAIT_OBJECT_0 != WaitForSingleObject((HANDLE)m_hEventConnect , m_dwSocketTimeout))
	{
		m_dwLastError = WSAENOTCONN;
		return false; // not connected
	}

	// make sure we aren't already writing
	if (WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_hEventWrite, 0))
	{
		m_dwLastError = WSAEINPROGRESS;
		return false; // another write on is blocking this socket
	}

	bool bRet = true;
	*pdwSize = 0;
	WSAOVERLAPPED o;
	m_csWrite.Lock();
	o.hEvent = m_hEventWrite;
	WSAResetEvent(o.hEvent);
	if (WSASend(m_socket, pBuffers, nCount, pdwSize, 0, &o, 0))
	{	
		DWORD dwLastError = WSAGetLastError();
		if (dwLastError != WSA_IO_PENDING)
		{
			m_dwLastError = dwLastError;
			bRet = false;
		}
	}

	// wait for write to complete
	if (bRet)
	{
		if (WaitForSingleObject((HANDLE)m_hEventWrite, m_dwSocketTimeout) == WAIT_OBJECT_0)
		{
			DWORD dwFlags = 0;
			if (WSAGetOverlappedResult(m_socket, &o, pdwSize, FALSE, &dwFlags))
				bRet = true;
			else
			{
				m_dwLastError = ::GetLastError();
				bRet = false;
			}
		}
		else
			bRet = false;
	}

	m_csWrite.Unlock();
	return bRet;
}

inline bool ZEvtSyncSocket::Write(const unsigned char *pBuffIn, DWORD *pdwSize) 
{
	WSABUF buff;
	buff.buf = (char*)pBuffIn;
	buff.len = *pdwSize;
	return Write(&buff, 1, pdwSize);
}

inline bool ZEvtSyncSocket::Read(const unsigned char *pBuff, DWORD *pdwSize) 
{
	// if we aren't already connected we'll wait to see if the connect
	// event happens
	if (WAIT_OBJECT_0 != WaitForSingleObject((HANDLE)m_hEventConnect , m_dwSocketTimeout))
	{
		m_dwLastError = WSAENOTCONN;
		return false; // not connected
	}

	if (WAIT_ABANDONED == WaitForSingleObject((HANDLE)m_hEventRead, 0))
	{
		m_dwLastError = WSAEINPROGRESS;
		return false; // another write on is blocking this socket
	}

	bool bRet = true;
	WSABUF buff;
	buff.buf = (char*)pBuff;
	buff.len = *pdwSize;
	*pdwSize = 0;
	DWORD dwFlags = 0;
	WSAOVERLAPPED o;
	ZeroMemory(&o, sizeof(o));

	// protect against re-entrency
	m_csRead.Lock();
	o.hEvent = m_hEventRead;
	WSAResetEvent(o.hEvent);
	if (WSARecv(m_socket, &buff, 1, pdwSize, &dwFlags, &o, 0))
	{
		DWORD dwLastError = WSAGetLastError();
		if (dwLastError != WSA_IO_PENDING)
		{
			m_dwLastError = dwLastError;
			bRet = false;
		}
	}

	// wait for the read to complete
	if (bRet)
	{
		if (WAIT_OBJECT_0 == WaitForSingleObject((HANDLE)o.hEvent, m_dwSocketTimeout))
		{
			dwFlags = 0;
			if (WSAGetOverlappedResult(m_socket, &o, pdwSize, FALSE, &dwFlags))
				bRet = true;
			else
			{
				m_dwLastError = ::GetLastError();
				bRet = false;
			}
		}
		else
			bRet = false;
	}

	m_csRead.Unlock();
	return bRet;
}

inline bool ZEvtSyncSocket::Init(SOCKET hSocket, void * /*pData=NULL*/) 
{
	ATLASSERT(hSocket != INVALID_SOCKET);

	if (hSocket == INVALID_SOCKET)
	{
		m_dwLastError = WSAENOTSOCK;
		return false;
	}

	m_socket = hSocket;

	// Allocate Events. On error, any open event handles will be closed
	// in the destructor
	if (NULL != (m_hEventRead = WSACreateEvent()))
		if (NULL != (m_hEventWrite = WSACreateEvent()))
			if (NULL != (m_hEventConnect = WSACreateEvent()))
	{
		if (!WSASetEvent(m_hEventWrite) || !WSASetEvent(m_hEventRead))
		{
			m_dwLastError = ::GetLastError();
			return false;
		}

		if (SOCKET_ERROR != WSAEventSelect(m_socket, m_hEventRead, FD_READ))
			if (SOCKET_ERROR != WSAEventSelect(m_socket, m_hEventWrite, FD_WRITE))
				if (SOCKET_ERROR != WSAEventSelect(m_socket, m_hEventConnect, FD_CONNECT))
					return true;
	}
	m_dwLastError = ::GetLastError();
	return false;
}

inline DWORD ZEvtSyncSocket::GetSocketTimeout() throw()
{
	return m_dwSocketTimeout;
}

inline DWORD ZEvtSyncSocket::SetSocketTimeout(DWORD dwNewTimeout) throw()
{
	DWORD dwOldTimeout = m_dwSocketTimeout;
	m_dwSocketTimeout = dwNewTimeout;
	return dwOldTimeout;
}

inline bool ZEvtSyncSocket::SupportsScheme(ATL_URL_SCHEME scheme) throw()
{
	// default only supports HTTP
	return scheme == ATL_URL_SCHEME_HTTP ? true : false;
}


#pragma warning(pop)

#endif // __ATLSPRIV_INL__
