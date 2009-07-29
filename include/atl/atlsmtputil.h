// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

#ifndef __ATLSMTPUTIL_H__
#define __ATLSMTPUTIL_H__

#pragma once

#if (defined(_WINSOCKAPI_) && !defined(_WINSOCK2API_))
#error <atlsmtputil.h> requires <winsock2.h> -- include <winsock2.h> before you include <windows.h> or <winsock.h>
#endif
#include <winsock2.h>
#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <tchar.h>
#include <atlstr.h>
#include <winnls.h>
#include <atlspriv.h>

//=======================================================================
//defines for SMTPMail module
//=======================================================================

//If overlapped I/O is desired, need 2.0 or greater
#define ATLSMTP_WSA_VERSION  ATL_WINSOCK_VER

//The maximum number of characters on a SMTP line
#define ATLSMTP_MAX_LINE_LENGTH 1000

#define ATLSMTP_MAX_SERVER_NAME_LENGTH 256

//Encoding schemes
#define ATLSMTP_BASE64_ENCODE 0
#define ATLSMTP_UUENCODE      1
#define ATLSMTP_QP_ENCODE     2

//I/O Defines
#define ATLSMTP_READBUFFER_SIZE        4096
#define ATLSMTP_GET_LINES              100


//Miscellaneous defines
#define ATLSMTP_SEND_FILE   1
#define ATLSMTP_FORMAT_SMTP 8

#define ATLSMTP_RETCODE_LEN 3


#pragma pack(push,_ATL_PACKING)
namespace ATL
{

//=======================================================================
// Miscellaneous Utility Functions
//=======================================================================
//A list of recipients in a string must by separated by one
//of the following characters
inline BOOL AtlSmtpIsRecipientDelimiter(char ch) throw()
{
	return (ch == ',' || ch == ';' || ch == ' ' || ch == '\0');
}

//Send data to hFile and wait for it to finish sending
inline BOOL AtlSmtpSendAndWait(HANDLE hFile, LPCSTR lpData, int nDataLength, LPOVERLAPPED pOverlapped) throw()
{
	ATLASSERT(lpData != NULL);
	ATLENSURE_RETURN_VAL(pOverlapped != NULL, FALSE);

	DWORD dwWritten = 0, dwErr = 0;
	int nRet = 0, nBufPos = 0;

	//write all the data
	do 
	{
		//Write a chunk of data, offsetting the buffer and amount to write by what's already 
		//been written
		nRet = WriteFile(hFile, (void*)(lpData+nBufPos), nDataLength-nBufPos, &dwWritten, pOverlapped);
		if (!nRet && (dwErr = GetLastError()) != ERROR_IO_INCOMPLETE && dwErr != ERROR_IO_PENDING)
			return FALSE;

		//Get the result of the write operation (wait for it)
		nRet = GetOverlappedResult(hFile, pOverlapped, &dwWritten, TRUE);
		if (!nRet)
			return FALSE;

		//Need to update offsets when writing to a file
		pOverlapped->Offset += dwWritten;
		nBufPos += dwWritten;

	} while (nBufPos < nDataLength);
	return TRUE;
}


//Read up to nDestLen bytes from hFile, keep reading while there's more data and there's
//room in the buffer
inline BOOL AtlSmtpReadData(__in HANDLE hFile, __out_ecount_part_z(*pnDestLen, *pnDestLen) LPSTR lpData, __inout int* pnDestLen, __in LPOVERLAPPED pOverlapped) 
{
	ATLASSERT(lpData != NULL);
	ATLASSERT(pnDestLen != NULL);
	ATLENSURE(pOverlapped != NULL);

	DWORD dwRead = 0, dwErr = 0;
	int nBufPos = 0;
	do
	{
		//REad a chunk of data, offsetting the buffer and amount to read by what's already been read
		int nRet = ReadFile(hFile, (void*)(lpData+nBufPos), (*pnDestLen)-nBufPos, &dwRead, pOverlapped);
		if (!nRet && (dwErr = GetLastError()) != ERROR_MORE_DATA && dwErr != ERROR_IO_PENDING && dwErr != ERROR_IO_INCOMPLETE)
			return FALSE;

		//Get the result of the read operation (wait for it)
		nRet = GetOverlappedResult(hFile, pOverlapped, &dwRead, TRUE);
		if (!nRet)
			return FALSE;

		//Handle offsets when reading from a file
		pOverlapped->Offset += dwRead;
		nBufPos += dwRead;
	} while (nBufPos < *pnDestLen && dwErr == ERROR_MORE_DATA);
	*pnDestLen = nBufPos;
	return TRUE;
}


//Used in sending encoded data
//lpData is the data to be sent now
//lpPrev is a pointer to the buffer that the previous call was made on
//This allows the new buffer (lpData) to be filled while lpPrev is being sent
//If all the data in lpPrev had not finished sending, we complete the send and wait
inline BOOL AtlSmtpSendOverlapped(HANDLE hFile, LPCSTR lpData, int nDataLength, LPCSTR lpPrev, DWORD dwPrevLength, LPOVERLAPPED pOverlapped)
{
	ATLASSERT(lpData != NULL);
	ATLENSURE(pOverlapped != NULL);

	DWORD dwWritten = 0, dwErr = 0, dwBufPos = 0;
	int nRet = 0;

	//Get the results of the previous call (if any)
	if (lpPrev && (!GetOverlappedResult(hFile, pOverlapped, &dwWritten, FALSE) || dwWritten < dwPrevLength))
	{
		//If any error but IO_INCOMPLETE, return failure
		if ((dwErr = GetLastError()) != ERROR_SUCCESS  && dwErr != ERROR_IO_INCOMPLETE && dwErr != ERROR_IO_PENDING)
		{
			return FALSE;
		}
		//Finish writing lpPrev if we need to
		while (dwBufPos < dwPrevLength)
		{
			//Get the result of the previous write (wait for it)
			nRet = GetOverlappedResult(hFile, pOverlapped, &dwWritten, TRUE);
			if (!nRet || (dwBufPos += dwWritten) == dwPrevLength)
			{
				if ((dwErr = GetLastError()) != ERROR_IO_INCOMPLETE && dwErr != ERROR_IO_PENDING)
					break;
			}

			//If we are writing to a file, we need to update the offsets
			pOverlapped->Offset += dwWritten;
			if(dwBufPos>dwPrevLength)
			{
				/* shouldn't happen */
				ATLASSERT(false);
				break;
			}
			nRet = WriteFile(hFile, (void*)(lpPrev+dwBufPos), dwPrevLength-dwBufPos, &dwWritten, pOverlapped);

			//If any error but IO_PENDING and IO_INCOMPLETE, break
			if (!nRet && (dwErr = GetLastError()) != ERROR_IO_PENDING && dwErr != ERROR_IO_INCOMPLETE)
				break;
		}
		if (dwBufPos < dwPrevLength)
			return FALSE;
	}

	//Now that all the previous data has been sent, start sending the current data
	nRet = WriteFile(hFile, (void*)lpData, nDataLength, &dwWritten, pOverlapped);
	GetOverlappedResult(hFile, pOverlapped, &dwWritten, FALSE);

	pOverlapped->Offset += dwWritten;

	//If any error but IO_PENDING
	if (!nRet && (dwErr = GetLastError()) != ERROR_IO_PENDING && dwErr != ERROR_IO_INCOMPLETE)
		return FALSE;
	return TRUE;
}


//Send a SMTP command and read the response
//return TRUE if it matches szResponse, FALSE otherwise
inline BOOL AtlSmtpSendAndCheck(__in HANDLE hFile, __in LPCSTR lpData, __in int nDataLength, __out_ecount_part(nMaxResponseLength, *pnResponseLength) LPSTR lpResponse, __out int* pnResponseLength, __in int nMaxResponseLength, 
							 __in_z LPCSTR szResponse, __in LPOVERLAPPED pOverlapped) throw()
{
	ATLASSERT(lpData != NULL);
	ATLASSERT(lpResponse != NULL);
	ATLASSERT(pnResponseLength != NULL);

	BOOL bRet = AtlSmtpSendAndWait(hFile, lpData, nDataLength, pOverlapped);
	if (bRet)
	{
		*pnResponseLength = nMaxResponseLength;
		bRet = AtlSmtpReadData(hFile, lpResponse, pnResponseLength, pOverlapped);
	}
	if (!bRet || strncmp((char*)lpResponse, szResponse, ATLSMTP_RETCODE_LEN))
		return FALSE;
	return TRUE;
}

} // namespace ATL
#pragma pack(pop)

#endif // __ATLSMTPUTIL_H__
