// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

#ifndef __ATLSMTPCONNECTION_H__
#define __ATLSMTPCONNECTION_H__

#pragma once

#ifndef _ATL_NO_DEFAULT_LIBS
#pragma comment(lib, "ws2_32.lib")
#endif  // !_ATL_NO_DEFAULT_LIBS

#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <atlstr.h>
#include <atlcoll.h>
#include <atlfile.h>
#include <atlmime.h>
#include <atlspriv.h>
#include <atlsmtputil.h>
#include <atlsocket.h>

// SMTP Return Codes
#define ATLSMTP_MAIL_SUCCESS      250
#define ATLSMTP_RCPT_SUCCESS      250
#define ATLSMTP_RCPT_NOT_LOCAL    251
#define ATLSMTP_DATA_INTERMEDIATE 354

#define ATLSMTP_CONN_SUCC "220"
#define ATLSMTP_HELO_SUCC "250"
#define ATLSMTP_MAIL_SUCC "250"
#define ATLSMTP_RCPT_SUCC "250"
#define ATLSMTP_RCPT_NLOC "251"
#define ATLSMTP_DATA_INTM "354"
#define ATLSMTP_DATA_SUCC "250"
#define ATLSMTP_RSET_SUCC "250"

// SMTP flags
#define ATLSMTP_DUMP_SENDER 1
#define ATLSMTP_DUMP_RECIPS 2
#define ATLSMTP_FOR_SEND    4


struct CSMTPWSAStartup
{
private:
	bool m_bInit;

public:
	CSMTPWSAStartup() throw()
		:m_bInit(false)
	{
		Init();
	}

	~CSMTPWSAStartup() throw()
	{
		Uninit();
	}

	bool Init() throw()
	{
		if (m_bInit)
			return true;

		WSADATA wsadata;
		if (WSAStartup(ATLSMTP_WSA_VERSION, &wsadata))
			return false;
		m_bInit = true;
		ATLASSERT(wsadata.wHighVersion >= 2);
		return true;
	}

	bool Uninit() throw()
	{
		if (m_bInit)
			if (WSACleanup())
				return false;
		m_bInit = false;
		return true;
	}
};

__declspec(selectany) CSMTPWSAStartup _g_smtp_init;

#pragma pack(push,_ATL_PACKING)
namespace ATL {

class CSMTPConnection
{
protected:

	// the socket
	SOCKET m_hSocket;

	// the OVERLAPPED struct
	OVERLAPPED m_Overlapped;

public:

	CSMTPConnection() throw()
		:m_hSocket(INVALID_SOCKET)
	{
		// initialize the OVERLAPPED struct
		memset(&m_Overlapped, 0, sizeof(OVERLAPPED));
	}

	~CSMTPConnection() throw()
	{
		Disconnect();
	}

	// Attempt to connect to the socket
	// lpszHostName - the host name to connect to
	BOOL Connect(LPCTSTR lpszHostName, DWORD dwTimeout = 10000) throw()
	{
		ATLASSERT(lpszHostName != NULL);

		// If we're already connected
		if (Connected())
		{
			return FALSE;
		}

		if (!_g_smtp_init.Init())
		{
			return FALSE;
		}

		CSocketAddr address;
		if (address.FindAddr(lpszHostName, IPPORT_SMTP, 0, PF_UNSPEC, SOCK_STREAM, 0))
		{
			return FALSE;
		}

		ADDRINFOT *pAI;
		
		BOOL bRet = FALSE;
		int nIndex = 0;
		while ((pAI = address.GetAddrInfo(nIndex++)) != NULL)
		{
			// create the socket
			m_hSocket = WSASocket(pAI->ai_family, pAI->ai_socktype, pAI->ai_protocol, NULL, 0, WSA_FLAG_OVERLAPPED);

			if (m_hSocket == INVALID_SOCKET)
			{
				return FALSE;
			}

			bRet = FALSE;
			WSAEVENT hEventConnect = WSACreateEvent();
			if (hEventConnect != NULL)
			{
				if (SOCKET_ERROR != WSAEventSelect(m_hSocket, hEventConnect, FD_CONNECT))
				{
					if (WSAConnect(m_hSocket, pAI->ai_addr, (int)pAI->ai_addrlen, 
							NULL, NULL, NULL, NULL))
					{
						if (WSAGetLastError() == WSAEWOULDBLOCK)
						{
							DWORD dwWait = WaitForSingleObject((HANDLE) hEventConnect, dwTimeout);
							if (dwWait == WAIT_OBJECT_0)
							{
								// make sure there were no connection errors.
								WSANETWORKEVENTS wse;
								ZeroMemory(&wse, sizeof(wse));
								WSAEnumNetworkEvents(m_hSocket, NULL, &wse);
								if (wse.iErrorCode[FD_CONNECT_BIT]==0)
								{
									bRet = TRUE;
								}
							}
						}
					}
				}

				// we're done with the event
				WSACloseEvent(hEventConnect);
			}
			if (bRet)
			{
				break;
			}
			
			shutdown(m_hSocket, SD_BOTH);
			closesocket(m_hSocket);
			m_hSocket = INVALID_SOCKET;
		}
		

		// Create an event for asynchronous I/O
		if (bRet)
		{
			ATLASSUME(m_Overlapped.hEvent == NULL);
			m_Overlapped.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
			if (m_Overlapped.hEvent == NULL)
			{
				bRet = FALSE;
			}
		}

		char szBuf[ATLSMTP_MAX_LINE_LENGTH+1];
		int nBufLen = ATLSMTP_MAX_LINE_LENGTH;
		if (bRet)
		{
			// See if the connect returns success
			bRet = AtlSmtpReadData((HANDLE)m_hSocket, szBuf, &nBufLen, &m_Overlapped);
			if (bRet)
			{
				if (strncmp(szBuf, ATLSMTP_CONN_SUCC, ATLSMTP_RETCODE_LEN))
				{
					bRet = FALSE;
				}
			}
		}

		char szLocalHost[ATLSMTP_MAX_SERVER_NAME_LENGTH+1];

		// gethostname should return 0 on success
		if (bRet && gethostname(szLocalHost, ATLSMTP_MAX_SERVER_NAME_LENGTH))
		{
			bRet = FALSE;
		}

		// Send HELO command and get reply
		if (bRet)
		{
			nBufLen = sprintf_s(szBuf, ATLSMTP_MAX_LINE_LENGTH+1, "HELO %s\r\n", szLocalHost);
			if (nBufLen > 0)
			{
				bRet = AtlSmtpSendAndCheck((HANDLE)m_hSocket, szBuf, nBufLen, szBuf, &nBufLen, 
										ATLSMTP_MAX_LINE_LENGTH, ATLSMTP_HELO_SUCC, &m_Overlapped);
			}
			else
			{
				bRet = FALSE;
			}
		}

		if (!bRet)
		{
			if (m_Overlapped.hEvent != NULL)
				CloseHandle(m_Overlapped.hEvent);
			shutdown(m_hSocket, SD_BOTH);
			closesocket(m_hSocket);
			m_hSocket = INVALID_SOCKET;
		}
		
		return bRet;
	}

	// Disconnect the socket
	inline BOOL Disconnect() throw()
	{
		if (!Connected())
		{
			return FALSE;
		}

		// shutdown should return 0 on success
		if (shutdown(m_hSocket, SD_BOTH))
		{
			return FALSE;
		}

		// closesocket should return 0 on success
		if (closesocket(m_hSocket))
		{
			return FALSE;
		}

		// close the handle to the overlapped event
		CloseHandle(m_Overlapped.hEvent);
		m_hSocket = INVALID_SOCKET;
		memset((void*)&m_Overlapped, 0, sizeof(OVERLAPPED));
		return TRUE;
	}

	// Are we connected?
	inline BOOL Connected() throw()
	{
		return (m_hSocket != INVALID_SOCKET ? TRUE : FALSE);
	}

	// Send a message from a file
	// lpszFileName - the file containing the message
	// lpszRecipients - the recipients to send to (optional - if not specified, the recipients specified
	//		in the file will be used
	// lpszSender - the sender (optional - if not specified, the recipients specified in the file
	//		will be used
	BOOL SendMessage(LPCTSTR lpszFileName, LPCTSTR lpszRecipients = NULL, LPCTSTR lpszSender = NULL) throw()
	{
		if (!Connected())
		{
			return FALSE;
		}

		//Try to open the file
		CAtlFile readFile;
		if (FAILED(readFile.Create(lpszFileName, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL)))
		{
			return FALSE;
		}

		char szBuf[ATLSMTP_MAX_LINE_LENGTH+1];
		int nBufLen = ATLSMTP_MAX_LINE_LENGTH;
		BOOL bDumpedSender = FALSE;

		//If the caller specifies the sender, rather than having an existing one in the file...
		if (lpszSender)
		{
			nBufLen = sprintf_s(szBuf, ATLSMTP_MAX_LINE_LENGTH+1, 
				"MAIL FROM:<%s>\r\n", (LPCSTR) CT2CA(lpszSender));
			if ((nBufLen < 0) || 
				(!AtlSmtpSendAndCheck((HANDLE)m_hSocket, szBuf, nBufLen, szBuf, &nBufLen, ATLSMTP_MAX_LINE_LENGTH, 
					ATLSMTP_MAIL_SUCC, &m_Overlapped)))
			{
				return FALSE;
			}
			bDumpedSender = TRUE;
		}
		nBufLen = ATLSMTP_MAX_LINE_LENGTH;

#ifdef ATLSMTP_DOUBLE_BUFFERED
		char buffer1[ATLSMTP_READBUFFER_SIZE];
		char buffer2[ATLSMTP_READBUFFER_SIZE];
		char* currBuffer = buffer1;
		char* prevBuffer = NULL;

		int nCurrBuffer = 0;
		DWORD dwPrevLength = 0;
		DWORD dwWritten = 0;
#else
		char bakBuffer[ATLSMTP_READBUFFER_SIZE];
		char* currBuffer = bakBuffer;

#endif // ATLSMTP_DOUBLE_BUFFERED
		DWORD dwRead = 0;
		DWORD dwBytesInBuffer = 0;
		DWORD dwBufPos = 0;

		//first handle the MAIL FROM and RCPT TO commands
		BOOL bDumpedRecipients = FALSE;
		BOOL bRet = TRUE;
		while (bRet)
		{
			int nRetCode = 0;

			//if we have dumped the sender, and we have extra recipients to send,
			//and we haven't alredy done so, do it
			if (lpszRecipients && !bDumpedRecipients && bDumpedSender)
			{
				bRet = DumpRecipients((HANDLE)m_hSocket, CT2A(lpszRecipients), &m_Overlapped, ATLSMTP_FOR_SEND);
			}

			if (bRet)
			{
				dwRead = 0;
				BOOL bFullLine = FALSE;
				bRet = ReadLine(readFile, currBuffer, szBuf, &dwBytesInBuffer, &dwBufPos,
					ATLSMTP_READBUFFER_SIZE, ATLSMTP_MAX_LINE_LENGTH, &dwRead, &bFullLine);
				if (dwRead == 0 || bFullLine == FALSE)
					bRet = FALSE;
			}

			if (bRet)
			{
				bRet = AtlSmtpSendAndWait((HANDLE)m_hSocket, szBuf, (int)(dwRead), &m_Overlapped);
			}

			if (bRet)
			{
				nBufLen = ATLSMTP_MAX_LINE_LENGTH;
				bRet = AtlSmtpReadData((HANDLE)m_hSocket, szBuf, &nBufLen, &m_Overlapped);
			}

			if (bRet)
			{	
				nRetCode = atoi(szBuf);
				//if the command is equal to ATLSMTP_MAIL_SUCC (or RCPT_SUCC: they are equivalent)
				if (nRetCode == ATLSMTP_MAIL_SUCCESS || nRetCode == ATLSMTP_RCPT_NOT_LOCAL || nRetCode == ATLSMTP_RCPT_SUCCESS)
				{
					bDumpedSender = TRUE;
					continue;
				}

				//If the command is equal to the data intermediate success code,
				//break out of the loop
				if (nRetCode == ATLSMTP_DATA_INTERMEDIATE)
					break;
			}

			//otherwise, we got an error code
			CancelMessage();
			return FALSE;
		}

		dwRead = dwBytesInBuffer;
		currBuffer+= dwBufPos;
		DWORD dwErr = 0;
		do
		{
			dwErr = 0;

			//Try to send the data
#ifdef ATLSMTP_DOUBLE_BUFFERED
			if (!AtlSmtpSendOverlapped((HANDLE)m_hSocket, currBuffer, dwRead, prevBuffer, dwPrevLength, &m_Overlapped))
			{
				bRet = FALSE;
				break;
			}
#else
			if (!AtlSmtpSendAndWait((HANDLE)m_hSocket, currBuffer, dwRead, &m_Overlapped))
			{
				bRet = FALSE;
				break;
			}
#endif // ATLSMTP_DOUBLE_BUFFERED

			//swap the current and previous buffer
#ifdef ATLSMTP_DOUBLE_BUFFERED
			prevBuffer = currBuffer;
			currBuffer = (nCurrBuffer == 0 ? buffer2 : buffer1);
			nCurrBuffer = (nCurrBuffer == 0 ? 1 : 0);
			dwPrevLength = dwBytesInBuffer;
#else
			currBuffer = bakBuffer;
#endif // ATLSMTP_DOUBLE_BUFFERED

			if (FAILED(readFile.Read(currBuffer, ATLSMTP_READBUFFER_SIZE, dwRead)))
			{
				bRet = FALSE;
				break;
			}
		} while (dwRead != 0);

		//ensure that the last of the data is sent
#ifdef ATLSMTP_DOUBLE_BUFFERED
		if (!GetOverlappedResult((HANDLE)m_hSocket, &m_Overlapped, &dwWritten, TRUE))
		{
			if ((dwErr = GetLastError()) != ERROR_IO_PENDING && dwErr != ERROR_IO_INCOMPLETE)
				bRet = FALSE;
			else if (dwWritten < dwPrevLength)
				bRet = AtlSmtpSendAndWait((HANDLE)m_hSocket, prevBuffer+dwWritten, dwPrevLength-dwWritten, &m_Overlapped);
		}
#endif // ATLSMTP_DOUBLE_BUFFERED


		if (bRet)
		{
			// End the message with a CRLF.CRLF
			nBufLen = sprintf_s(szBuf, _countof(szBuf), "\r\n.\r\n");
			if (!AtlSmtpSendAndCheck((HANDLE)m_hSocket, szBuf, nBufLen, 
				szBuf, &nBufLen, ATLSMTP_MAX_LINE_LENGTH, ATLSMTP_DATA_SUCC, &m_Overlapped))
			{
				bRet = FALSE;
			}
		}

		return bRet;
	}

	// Send the message
	// msg - the CMimeMessage to send
	// lpszSender - the sender 
	inline BOOL SendMessage(CMimeMessage& msg, LPCTSTR lpszRecipients = NULL, LPCTSTR lpszSender = NULL) throw()
	{
		if (!Connected())
		{
			return FALSE;
		}

		char szBuf[ATLSMTP_MAX_LINE_LENGTH+1];

		//Send MAIL FROM command and get reply
		int nBufLen = sprintf_s(szBuf, ATLSMTP_MAX_LINE_LENGTH+1, "MAIL FROM:<%s>\r\n", 
			(lpszSender ? (LPCSTR) CT2CA(lpszSender) : msg.GetSender()));
		if ((nBufLen < 0) ||
			(!AtlSmtpSendAndCheck((HANDLE)m_hSocket, szBuf, nBufLen, 
				szBuf, &nBufLen, ATLSMTP_MAX_LINE_LENGTH, ATLSMTP_MAIL_SUCC, &m_Overlapped)))
		{
			return FALSE;
		}

		BOOL bRet = TRUE;
		if (!lpszRecipients)
		{
			LPSTR lpszRecipientsA = NULL;
			DWORD dwLen = msg.GetRequiredRecipientsStringLength();
			lpszRecipientsA = static_cast<LPSTR>(calloc(sizeof(char),dwLen));
			if (!lpszRecipientsA || msg.GetRecipientsString(lpszRecipientsA, &dwLen) == FALSE)
			{
				bRet = FALSE;
			}
			if (bRet)
				bRet = DumpRecipients((HANDLE)m_hSocket, lpszRecipientsA, &m_Overlapped, ATLSMTP_FOR_SEND);
			free(lpszRecipientsA);
		}
		else
		{
			bRet = DumpRecipients((HANDLE)m_hSocket, CT2CA(lpszRecipients), 
						&m_Overlapped, ATLSMTP_FOR_SEND);
		}

		//Begin the data output
		if (bRet)
		{
			nBufLen = sprintf_s(szBuf, _countof(szBuf), "DATA\r\n");
			bRet = AtlSmtpSendAndCheck((HANDLE)m_hSocket, szBuf, nBufLen, 
						szBuf, &nBufLen, ATLSMTP_MAX_LINE_LENGTH, ATLSMTP_DATA_INTM, &m_Overlapped);
		}

		if (!bRet)
			CancelMessage();

		//Attempt to write the data to the socket
		if (bRet)
		{
			bRet = msg.WriteData((HANDLE)m_hSocket, &m_Overlapped, NULL, ATLSMTP_FORMAT_SMTP);
		}

		if (bRet)
		{
			//End the message with a <CRLF>.<CRLF>
			nBufLen = sprintf_s(szBuf, _countof(szBuf), "\r\n.\r\n");
			if (!AtlSmtpSendAndCheck((HANDLE)m_hSocket, szBuf, nBufLen, 
					szBuf, &nBufLen, ATLSMTP_MAX_LINE_LENGTH, ATLSMTP_DATA_SUCC, &m_Overlapped))
			{
				return FALSE;
			}
		}

		return bRet;
	}

	// Send a chunk of raw data
	inline BOOL SendRaw(LPCTSTR lpszRawData, DWORD dwLen, LPCTSTR lpszRecipients, LPCTSTR lpszSender) throw()
	{
		ATLASSERT(lpszRawData != NULL);
		ATLASSERT(lpszRecipients != NULL);
		ATLASSERT(lpszSender != NULL);

		if (!Connected())
			return FALSE;

		char szBuf[ATLSMTP_MAX_LINE_LENGTH+1];

		//Send MAIL FROM command and get reply
		int nBufLen = sprintf_s(szBuf, ATLSMTP_MAX_LINE_LENGTH+1, 
			"MAIL FROM:<%s>\r\n", (LPCSTR) CT2CA(lpszSender));
		if ((nBufLen < 0) || 
			(!AtlSmtpSendAndCheck((HANDLE)m_hSocket, szBuf, nBufLen, 
				szBuf, &nBufLen, ATLSMTP_MAX_LINE_LENGTH, ATLSMTP_MAIL_SUCC, &m_Overlapped)))
		{
			return FALSE;
		}

		BOOL bRet = DumpRecipients((HANDLE)m_hSocket, CT2CA(lpszRecipients),
						&m_Overlapped, ATLSMTP_FOR_SEND);

		// Begin the data output
		if (bRet)
		{
			nBufLen = sprintf_s(szBuf, _countof(szBuf), "DATA\r\n");
			bRet = AtlSmtpSendAndCheck((HANDLE)m_hSocket, szBuf, nBufLen,
						szBuf, &nBufLen, ATLSMTP_MAX_LINE_LENGTH, ATLSMTP_DATA_INTM, &m_Overlapped);
		}

		if (!bRet)
			CancelMessage();

		if (bRet)
		{
			bRet = AtlSmtpSendAndWait((HANDLE)m_hSocket, (LPSTR)(lpszRawData), dwLen, &m_Overlapped);
		}

		if (bRet)
		{
			//End the message with a <CRLF>.<CRLF>
			nBufLen = sprintf_s(szBuf, _countof(szBuf), "\r\n.\r\n");
			if (!AtlSmtpSendAndCheck((HANDLE)m_hSocket, szBuf, nBufLen, 
					szBuf, &nBufLen, ATLSMTP_MAX_LINE_LENGTH, ATLSMTP_DATA_SUCC, &m_Overlapped))
			{
				return FALSE;
			}
		}

		return bRet;
	}

	inline BOOL SendSimple(LPCTSTR lpszRecipients, LPCTSTR lpszSender, LPCTSTR lpszSubject, LPCTSTR lpszBody, int nTextLen = -1) throw()
	{
		CMimeMessage msg;
		BOOL bRet = msg.SetSubject(lpszSubject);
		if (bRet)
			bRet = msg.AddText(lpszBody, nTextLen);

		CFixedStringT<CString, MAX_PATH> strRecip;
		LPCTSTR szTmp = lpszRecipients;
		LPCTSTR szTmp2 = lpszRecipients;
		while (*szTmp && bRet)
		{
			if (AtlSmtpIsRecipientDelimiter((char) *szTmp2))
			{
				_ATLTRY
				{
					strRecip.SetString(szTmp, (int)(szTmp2-szTmp));
					bRet = msg.AddRecipient((LPCTSTR) strRecip);

					if (*szTmp2)
					{
						while (*szTmp2 && AtlSmtpIsRecipientDelimiter((char) *szTmp2))
						{
							szTmp2++;
						}
					}
					szTmp = szTmp2;
				}
				_ATLCATCHALL()
				{
					bRet = FALSE;
				}
			}
			else
			{
				szTmp2++;
			}
		}

		if (bRet)
			bRet = SendMessage(msg, lpszRecipients, lpszSender);

		return bRet;
	}

	// Save a MIME message to a file
	// lpszFileName - the file name
	// lpszRecipients - the recipients string (optional)
	// lpszSender - the sender (optional)
	// dwFlags - the flags (optional)
	inline BOOL WriteToFile(LPCTSTR lpszFileName, CMimeMessage& msg, LPCTSTR lpszRecipients = NULL, 
		LPCTSTR lpszSender = NULL, DWORD dwFlags = 0) throw()
	{
		//Try to create/open the file
		HANDLE hFile = CreateFile(lpszFileName, GENERIC_WRITE, 0, NULL,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			return FALSE;
		}

		// Use CHandle to close the file handle
		// (CAtlFile does not allow for overlapped I/O)
		CHandle hdlFile(hFile);

		//Create and initialize the OVERLAPPED struct
		OVERLAPPED writeOverlapped;
		memset((void*)&writeOverlapped, 0, sizeof(OVERLAPPED));
		writeOverlapped.hEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
		if (writeOverlapped.hEvent == NULL)
		{
			return FALSE;
		}

		// Use CHandle to close the event handle
		CHandle hdlEvent(writeOverlapped.hEvent);

		char szBuf[ATLSMTP_MAX_LINE_LENGTH+1];
		BOOL bRet = TRUE;

		int nBufLen = 0;

		//if writing to file for purposes of sending, write out the
		//commands as well
		if (lpszSender || (dwFlags & ATLSMTP_DUMP_SENDER))
		{
			nBufLen = sprintf_s(szBuf, ATLSMTP_MAX_LINE_LENGTH+1, "MAIL FROM:<%s>\r\n", 
				(lpszSender ? (LPCSTR) CT2CA(lpszSender) : msg.GetSender()));
			if (nBufLen > 0)
			{
				bRet = AtlSmtpSendAndWait(hFile, szBuf, nBufLen, &writeOverlapped);
			}
			else
			{
				bRet = FALSE;
			}
		}

		if (bRet && (lpszRecipients || (dwFlags & ATLSMTP_DUMP_RECIPS)))
		{
			if (!lpszRecipients)
			{
				LPSTR lpszRecipientsA = NULL;
				DWORD dwLen = msg.GetRequiredRecipientsStringLength();
				lpszRecipientsA = static_cast<LPSTR>(calloc(sizeof(char),dwLen));
				if (!lpszRecipientsA || msg.GetRecipientsString(lpszRecipientsA, &dwLen) == FALSE)
				{
					bRet = FALSE;
				}
				if (bRet)
					bRet = DumpRecipients(hFile, lpszRecipientsA, &writeOverlapped);
				free(lpszRecipientsA);
			}
			else
			{
				bRet = DumpRecipients(hFile, CT2CA(lpszRecipients), &writeOverlapped);
			}
		}

		if (bRet)
		{
			nBufLen = sprintf_s(szBuf, _countof(szBuf), "DATA\r\n");
			bRet = AtlSmtpSendAndWait(hFile, szBuf, nBufLen, &writeOverlapped);
		}

		if (bRet)
		{
			bRet = msg.WriteData(hFile, &writeOverlapped, NULL, ATLSMTP_FORMAT_SMTP);
		}

		return bRet;
	}

protected:

	// disallow copy construction
	CSMTPConnection(const CSMTPConnection&) throw()
	{
		ATLASSERT(FALSE);
	}

	// disallow assignment
	const CSMTPConnection& operator=(const CSMTPConnection&) throw()
	{
		ATLASSERT(FALSE);
		return *this;
	}

	// Tell the server we are aborting the message
	inline BOOL CancelMessage() throw()
	{
		char szBuf[ATLSMTP_MAX_LINE_LENGTH+1];
		int nBufLen  = 0;
		nBufLen = sprintf_s(szBuf, _countof(szBuf), "RSET\r\n");
		if (!AtlSmtpSendAndCheck((HANDLE)m_hSocket, szBuf, nBufLen, szBuf, &nBufLen, ATLSMTP_MAX_LINE_LENGTH, 
			ATLSMTP_RSET_SUCC, &m_Overlapped))
		{
			Disconnect();
			return FALSE;
		}
		return TRUE;
	}

	// Dump the recipients to hFile
	// lpszRecipients - the recipients string
	// pOverlapped - the OVERALAPPED struct
	// dwFlags - the flags
	inline BOOL DumpRecipients(HANDLE hFile, LPCSTR lpszRecipients, LPOVERLAPPED pOverlapped, DWORD dwFlags = 0)
	{
		ATLENSURE(lpszRecipients != NULL);
		ATLASSERT(pOverlapped != NULL);

		char  rcptBuf[ATLSMTP_MAX_LINE_LENGTH-12+1];
		char  szBuf[ATLSMTP_MAX_LINE_LENGTH+1];
		LPSTR tmpBuf = rcptBuf;
		char ch;
		BOOL bRet = TRUE;
		int nMaxLength = ATLSMTP_MAX_LINE_LENGTH;
		int nRetCode = 0;
		size_t nCnt = 0;
		do
		{
			ch = *lpszRecipients;
			if (ch)
				lpszRecipients++;
			if (AtlSmtpIsRecipientDelimiter(ch))
			{
				*tmpBuf = 0;
				int nBufLen = sprintf_s(szBuf, ATLSMTP_MAX_LINE_LENGTH, 
					"RCPT TO:<%s>\r\n", rcptBuf);
				if (nBufLen > 0)
				{
					bRet = AtlSmtpSendAndWait(hFile, szBuf, nBufLen, pOverlapped);
				}
				else
				{
					bRet = FALSE;
				}

				if (bRet && (dwFlags & ATLSMTP_FOR_SEND))
				{
					bRet = AtlSmtpReadData(hFile, szBuf, &nMaxLength, pOverlapped);
					nRetCode = atoi(szBuf);
					if (!bRet || (nRetCode != ATLSMTP_RCPT_SUCCESS && nRetCode != ATLSMTP_RCPT_NOT_LOCAL))
					{
						bRet = FALSE;
						break;
					}
				}
				tmpBuf = rcptBuf;
				nCnt = 0;
				nMaxLength = ATLSMTP_MAX_LINE_LENGTH;
				while (isspace(static_cast<unsigned char>(*lpszRecipients)))
					lpszRecipients++;
				continue;
			}

			if (nCnt >= sizeof(rcptBuf)-1)
			{
				// recipient string too long
				bRet = FALSE;
				break;
			}

			*tmpBuf++ = ch;
			nCnt++;
		} while (ch != '\0');

		return bRet;
	}

	// Implementation - used from ReadLine
	// fills pBuf with up to dwMaxLen bytes
	BOOL FillBuffer(__in HANDLE hFile, __out_ecount_part(dwMaxLen, *pdwLen) LPSTR pBuf, __in DWORD dwMaxLen, __out LPDWORD pdwLen) throw()
	{
		ATLASSERT(hFile != INVALID_HANDLE_VALUE);
		ATLASSERT(pdwLen != NULL);

		DWORD dwRead = 0;
		DWORD dwTotalRead = 0;
		int nRet = 0;

		do 
		{
			nRet = ReadFile(hFile, pBuf, dwMaxLen-dwTotalRead, &dwRead, NULL);
			if (!nRet && GetLastError() != ERROR_HANDLE_EOF)
			{
				return FALSE;
			}

			if (dwRead == 0)
				break;

			dwTotalRead+= dwRead;
		} while (dwTotalRead < dwMaxLen);

		*pdwLen = dwTotalRead;

		return TRUE;
	}

	// Implementation
	// Read a line (terminated by LF) from hFile
	BOOL ReadLine(__in HANDLE hFile, __out_ecount_part_z(dwMaxSrcLen, *pdwSrcLen) LPSTR pSrc, __out_ecount_part_z(dwMaxDestLen, *pdwRead) LPSTR pDest, __inout LPDWORD pdwSrcLen, __inout LPDWORD pdwBufPos, __in DWORD dwMaxSrcLen, 
			__in DWORD dwMaxDestLen, __out_opt LPDWORD pdwRead=NULL, __out_opt LPBOOL pbFullLine=NULL) 
	{
		ATLENSURE(hFile != INVALID_HANDLE_VALUE);
		ATLENSURE(pSrc != NULL);
		ATLENSURE(pDest != NULL);
		ATLENSURE(pdwSrcLen != NULL);
		ATLENSURE(pdwBufPos != NULL);

		BOOL bRet = TRUE;
		DWORD dwLen = 0;
		DWORD dwBufPos = 0;
		DWORD dwSrcLen = *pdwSrcLen;
		LPSTR pSrcCurrent = pSrc + *pdwBufPos;

		while (bRet && dwLen < dwMaxDestLen)
		{
			if (dwSrcLen == 0)
			{
				if (!FillBuffer(hFile, pSrc, dwMaxSrcLen, pdwSrcLen) || *pdwSrcLen == 0)
					break;

				dwBufPos = 0;
				*pdwBufPos = 0;
				dwSrcLen = *pdwSrcLen;
				pSrcCurrent = pSrc;
			}

			--dwSrcLen;
			*pDest = *pSrcCurrent++;
			dwLen++;
			dwBufPos++;
			if (*pDest == '\n')
			{
				break;
			}
			pDest++;
		}

		*pdwSrcLen = dwSrcLen;

		if (pbFullLine)
		{
			if (*pDest != '\n')
				*pbFullLine = FALSE;
			else
				*pbFullLine = TRUE;
		}

		if (pdwRead)
			*pdwRead = dwLen;

		*pdwBufPos += dwBufPos;

		return bRet;
	}

}; // class CSMTPConnection

} // namespace ATL
#pragma pack(pop)

#endif // __ATLSMTPCONNECTION_H__
