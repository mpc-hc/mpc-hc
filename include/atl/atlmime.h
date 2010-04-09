// This is a part of the Active Template Library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

#ifndef __ATLMIME_H__
#define __ATLMIME_H__

#pragma once

#include <tchar.h>
#include <time.h>
#include <atlbase.h>
#include <mlang.h>
#include <atlfile.h>
#include <atlcoll.h>
#include <atlstr.h>
#include <atlsmtputil.h>
#include <atlenc.h>
#include <atlspriv.h>

#pragma warning(push)
#pragma warning(disable: 4625) // copy constructor could not be generated because a base class copy constructor is inaccessible
#pragma warning(disable: 4626) // assignment operator could not be generated because a base class assignment operator is inaccessible

#ifndef _CPPUNWIND
#pragma warning (push)
#pragma warning(disable: 4702) // unreachable code
#endif // _CPPUNWIND

#pragma pack(push,_ATL_PACKING)
namespace ATL {

#ifndef ATLMIME_SEPARATOR
#define ATLMIME_SEPARATOR "\r\n\r\n--"
#endif//ATLMIME_SEPARATOR

#ifndef ATLMIME_VERSION 
#define ATLMIME_VERSION "MIME-Version: 1.0"
#endif//ATLMIME_VERSION

#ifndef ATLMIME_EMAIL
#define ATLMIME_EMAIL "email"
#endif//ATLMIME_EMAIL

extern __declspec(selectany) const DWORD ATL_MIME_BOUNDARYLEN = 32;
extern __declspec(selectany) const DWORD ATL_MIME_DATE_LEN    = 64;

// Called when message is sent - sets the "Date:" field
inline size_t SetRfc822Time(__out_ecount_part_z_opt(dwLen, return) LPSTR szDate, __in size_t dwLen) throw()
{
	// Max buffer size required(including NULL) - 38
	const size_t s_dwMaxBufferLen = 38;
	if (szDate == NULL)
	{
		return s_dwMaxBufferLen;
	}
	
	if (dwLen < 38)
	{
		return 0;
	}
	static const LPCSTR s_months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
							   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

	static const LPCSTR s_days[7] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

	SYSTEMTIME st;
	DWORD      dwTimeZoneId=TIME_ZONE_ID_UNKNOWN;
	CHAR       cDiff;
	LONG       ltzBias=0;
	LONG       ltzHour;
	LONG       ltzMinute;
	TIME_ZONE_INFORMATION tzi;

	GetLocalTime(&st);

	// Gets TIME_ZONE_INFORMATION
	memset(&tzi, 0, sizeof(tzi));
	dwTimeZoneId = GetTimeZoneInformation(&tzi);
	switch (dwTimeZoneId)
	{
	case TIME_ZONE_ID_STANDARD:
		ltzBias = tzi.Bias + tzi.StandardBias;
		break;

	case TIME_ZONE_ID_DAYLIGHT:
		ltzBias = tzi.Bias + tzi.DaylightBias;
		break;

	case TIME_ZONE_ID_UNKNOWN:
	default:
		ltzBias = tzi.Bias;
		break;
	}

	// Set Hour Minutes and time zone dif
	ltzHour = ltzBias / 60;
	ltzMinute = ltzBias % 60;
	cDiff = (ltzHour < 0) ? '+' : '-';

	int nDay = (st.wDayOfWeek > 6) ? 0 : st.wDayOfWeek;
	int nMonth = st.wMonth = (WORD)((st.wMonth < 1 || st.wMonth > 12) ? 0 : st.wMonth - 1);


	// Constructs RFC 822 format: "ddd, dd mmm yyyy hh:mm:ss +/- hhmm\0"
	sprintf_s(szDate, dwLen, "Date: %3s, %d %3s %4d %02d:%02d:%02d %c%02d%02d",
					  s_days[nDay],                            // "ddd"
					  st.wDay,                                 // "dd"
					  s_months[nMonth],                        // "mmm"
					  st.wYear,                                // "yyyy"
					  st.wHour,                                // "hh"
					  st.wMinute,                              // "mm"
					  st.wSecond,                              // "ss"
					  cDiff,                                   // "+" / "-"
					  abs (ltzHour),                           // "hh"
					  abs (ltzMinute));                        // "mm"
	return s_dwMaxBufferLen;
}

inline DWORD GetContentTypeFromFileName(LPCTSTR szFileName, CSimpleString& strContentType) throw()
{
	if (szFileName == NULL)
	{
		return ERROR_INVALID_DATA;
	}

	DWORD dwErr = ERROR_PATH_NOT_FOUND;
	_ATLTRY
	{
		// get the file extension
		TCHAR szExt[_MAX_EXT];
		Checked::tsplitpath_s(szFileName, NULL, 0, NULL, 0, NULL, 0, szExt, _countof(szExt));
		if (*szExt)
		{
			// Query the content type from the registry
			CRegKey rkContentType;
			dwErr = rkContentType.Open(HKEY_CLASSES_ROOT, szExt, KEY_READ);
			if (dwErr == ERROR_SUCCESS)
			{
				ULONG nChars=0;
				dwErr = rkContentType.QueryStringValue(_T("Content Type"), NULL, &nChars);
				if (dwErr == ERROR_SUCCESS)
				{
					LPTSTR szBuf = strContentType.GetBuffer(nChars);
					dwErr = rkContentType.QueryStringValue(_T("Content Type"), szBuf, &nChars);
					strContentType.ReleaseBuffer(nChars);
				}
			}
		}

		if (dwErr != ERROR_SUCCESS)
		{
			// default to application/octet-stream
			strContentType.SetString(_T("application/octet-stream"), sizeof("application/octet-stream")-1);
		}
	}
	_ATLCATCHALL()
	{
		dwErr = ERROR_OUTOFMEMORY;
	}

	return dwErr;
}

// CMimeBodyPart is an abstract base class for the body parts
// CMimeAttachment, CMimeText, CMimeHeader.
class CMimeBodyPart
{
public:

	virtual ~CMimeBodyPart() = 0 {}

	// WriteData - pure virtual method to dump the data for a body part.
	virtual BOOL WriteData(HANDLE hFile, LPOVERLAPPED pOverlapped, LPCSTR szBoundary, DWORD dwFlags = 0) = 0;

	// GetContentType - pure virtual method to get the content of a body part
	virtual LPCSTR GetContentType() = 0;

	// GetCharset - virtual method to get the character set of a body part
	// (defaults to ATLSMTP_DEFAULT_CSET).
	virtual LPCSTR GetCharset()
	{
		return ATLSMTP_DEFAULT_CSET;
	}

	virtual CMimeBodyPart* Copy() = 0;

protected:

	// MakeMimeHeader - pure virutal method to create a MIME header for a 
	// body part.
	virtual BOOL MakeMimeHeader(CStringA& header, LPCSTR szBoundary) = 0;
}; // class CMimeBodyPart


// This enum is used with the X-Priority part of the message header
enum ATL_MIME_PRIORITY 
{
	ATL_MIME_HIGH_PRIORITY   = 1, 
	ATL_MIME_NORMAL_PRIORITY = 3, 
	ATL_MIME_LOW_PRIORITY    = 5, 
	ATL_MIME_PRIORITY_ERROR  = 0
};


// CMimeHeader describes the basic RFC 822 message header.
// It also serves as the base class for the CMimeMessage object.
class CMimeHeader : public CMimeBodyPart
{
protected:

	// Pointer to MLANG's IMultiLanguage interface.
	// This is used in doing conversion from code pages
	// to MIME-compatible character sets.
	CComPtr<IMultiLanguage> m_spMultiLanguage;

	//Basic Header Parts
	CStringA        m_strFrom;
	CStringA        m_strTo;
	CStringA        m_strCc;
	CStringA        m_strBcc;
	CStringA        m_strSubject;

	//Extended Header Parts
	ATL_MIME_PRIORITY   m_nPriority;
	CStringA        m_XHeader;

	//Display Names
	CStringA        m_strSenderName;

	//MIME Character Sets
	char            m_szSubjectCharset[ATL_MAX_ENC_CHARSET_LENGTH];
	char            m_szSenderCharset[ATL_MAX_ENC_CHARSET_LENGTH];

	//Recipient and CC charsets are encoded in the Add methods

public:

	CMimeHeader() throw()
		:m_nPriority(ATL_MIME_NORMAL_PRIORITY)
	{
		m_szSubjectCharset[0] = '\0';
		m_szSenderCharset[0] = '\0';
	}

	~CMimeHeader() throw()
	{
	}

	// Initialize MLang for multilanguage support
	inline BOOL Initialize(IMultiLanguage* pMultiLanguage = NULL) throw()
	{
		if (pMultiLanguage != NULL)
		{
			m_spMultiLanguage = pMultiLanguage;
		}
		else
		{
			HRESULT hr = m_spMultiLanguage.CoCreateInstance(__uuidof(CMultiLanguage), NULL, CLSCTX_INPROC_SERVER);
			if (hr != S_OK)
				return FALSE;
		}
		return TRUE;
	}

	// Get the content type
	virtual inline LPCSTR GetContentType() throw()
	{
		return "text/plain";
	}

	// Get the character set
	virtual inline LPCSTR GetCharset() throw()
	{
		return "iso-8859-1";
	}

	virtual ATL_NOINLINE CMimeBodyPart* Copy() throw( ... )
	{
		CAutoPtr<CMimeHeader> pNewHeader;
		ATLTRY(pNewHeader.Attach(new CMimeHeader));
		if (pNewHeader)
			*pNewHeader = *this;

		return pNewHeader.Detach();
	}

	const CMimeHeader& operator=(const CMimeHeader& that) throw( ... )
	{
		if (this != &that)
		{
			m_spMultiLanguage = that.m_spMultiLanguage;
			m_strFrom = that.m_strFrom;
			m_strTo = that.m_strTo;
			m_strCc = that.m_strCc;
			m_strSubject = that.m_strSubject;

			m_nPriority = that.m_nPriority;
			m_XHeader = that.m_XHeader;

			m_strSenderName = that.m_strSenderName;

			Checked::strcpy_s(m_szSubjectCharset, ATL_MAX_ENC_CHARSET_LENGTH, that.m_szSubjectCharset);
			Checked::strcpy_s(m_szSenderCharset, ATL_MAX_ENC_CHARSET_LENGTH, that.m_szSenderCharset);
		}

		return *this;
	}

	// Set the priority of the message
	inline BOOL SetPriority(ATL_MIME_PRIORITY nPriority) throw()
	{
		if (nPriority < 0)
			return FALSE;
		m_nPriority = nPriority;
		return TRUE;
	}

	// Get the priority of the message
	inline ATL_MIME_PRIORITY GetPriority() throw()
	{
		return m_nPriority;
	}

	// Set the display (friendly) name for the header
	inline BOOL SetSenderName(LPCTSTR szName, UINT uiCodePage = 0) throw()
	{
		if (szName == NULL)
			return FALSE;

		CHeapPtr<char> szNamePtr;
		UINT nLen(0);

		BOOL bRet = AtlMimeConvertString(m_spMultiLanguage, uiCodePage, szName, &szNamePtr, &nLen);
		if (bRet)
		{
			_ATLTRY
			{
				m_strSenderName.Empty();
				m_strSenderName.Append(szNamePtr, (int) nLen);
			}
			_ATLCATCHALL()
			{
				return FALSE;
			}
			bRet = AtlMimeCharsetFromCodePage(m_szSenderCharset, uiCodePage, m_spMultiLanguage, ATL_MAX_ENC_CHARSET_LENGTH);
		}

		return bRet;
	}

	// Get the display (friendly) name for the sender
	inline LPCSTR GetSenderName() throw()
	{
		return m_strSenderName;
	}

	// Append a user defined header (should not contain CRLF)
	inline BOOL AppendUserDefinedHeader(LPCTSTR szHeaderName, LPCTSTR szHeader, UINT uiCodePage = 0) throw()
	{
		if ((szHeader == NULL) || (szHeaderName == NULL))
			return FALSE;

		_ATLTRY
		{
			CHeapPtr<char> szName;
			UINT nLen(0);

			BOOL bRet = AtlMimeConvertString(m_spMultiLanguage, uiCodePage, szHeader, &szName, &nLen);
			if (bRet)
			{
				// get the charset
				char szCharset[ATL_MAX_ENC_CHARSET_LENGTH];			
				bRet = AtlMimeCharsetFromCodePage(szCharset, uiCodePage, m_spMultiLanguage, ATL_MAX_ENC_CHARSET_LENGTH);

				if (bRet)
				{
					CStringA str;
					str.Append(szName, (int)nLen);

					// encode the string
					CHeapPtr<char> szBuf;
					DWORD dwReqLen = QEncodeGetRequiredLength(str.GetLength(), 
								ATL_MAX_ENC_CHARSET_LENGTH);

					if (szBuf.Allocate(dwReqLen) == false)
					{
						return FALSE;
					}

					DWORD dwLength(0);
					BOOL bEncoded = FALSE;
					if (!GetEncodedString(str, szCharset, szBuf, dwReqLen, dwLength, bEncoded))
					{
						return FALSE;
					}

					// add to m_XHeader
					m_XHeader += CT2CA(szHeaderName);
					m_XHeader.Append(": ", 2);
					m_XHeader.Append(szBuf, dwLength);
					m_XHeader.Append("\r\n", 2);
				}
			}

			return bRet;
		}
		_ATLCATCHALL()
		{
			return FALSE;
		}
	}

	// Add a recipient ("To:" line)
	inline BOOL AddRecipient(LPCTSTR szAddress, LPCTSTR szName = NULL, UINT uiCodePage = 0) throw()
	{
		return AddRecipientHelper(m_strTo, szAddress, szName, uiCodePage);
	}

	// Get the recipients string ("To:" line)
	inline LPCSTR GetRecipients() throw()
	{
		return m_strTo;
	}

	// Clear all recipients ("To:" line)
	inline BOOL ClearRecipients() throw()
	{
		m_strTo.Empty();
		return TRUE;
	}

	// Add a recipient ("CC:" line)
	inline BOOL AddCc(LPCTSTR szAddress, LPCTSTR szName = NULL, UINT uiCodePage = 0) throw()
	{
		return AddRecipientHelper(m_strCc, szAddress, szName, uiCodePage);
	}

	// Get the recipients string ("CC:" line)
	inline LPCSTR GetCc() throw()
	{
		return m_strCc;
	}

	// Clear the recipients string ("CC:" line)
	inline BOOL ClearCc() throw()
	{
		m_strCc.Empty();
		return TRUE;
	}

	// Add a Bcc recipient (not output as part of message)
	inline BOOL AddBcc(LPCTSTR szAddress) throw()
	{
		if (szAddress == NULL)
		{
			return FALSE;
		}

		_ATLTRY
		{
			CStringA str = m_strBcc;

			if (m_strBcc.GetLength() > 0)
				str.Append(",", 1);

			str += CT2CA(szAddress);

			m_strBcc = str;

			return TRUE;
		}
		_ATLCATCHALL()
		{
			return FALSE;
		}
	}

	// Get the recipients string (Bcc part)
	inline LPCSTR GetBcc() throw()
	{
		return m_strBcc;
	}

	// Clear the recipients string (Bcc part)
	inline BOOL ClearBcc() throw()
	{
		m_strBcc.Empty();
		return TRUE;
	}


	inline DWORD GetRequiredRecipientsStringLength() throw()
	{
		DWORD dwRet = m_strTo.GetLength();
		if (m_strCc.GetLength())
		{
			dwRet += dwRet ? 1 : 0;
			dwRet += m_strCc.GetLength();
		}
		if (m_strBcc.GetLength())
		{
			dwRet += dwRet ? 1 : 0;
			dwRet += m_strBcc.GetLength();
		}
		dwRet++;
		return dwRet;
	}

	// returns the recipients string to be (addresses only, in comma separated format)
	ATL_NOINLINE BOOL GetRecipientsString(__out_ecount_part_z(*pdwLen, *pdwLen) LPSTR szRecip, __inout LPDWORD pdwLen) throw()
	{
		if ( (szRecip == NULL) || (pdwLen == NULL) )
		{
			return FALSE;
		}

		if ( *pdwLen < GetRequiredRecipientsStringLength())
		{
			*pdwLen = GetRequiredRecipientsStringLength();
			return FALSE;
		}

		DWORD dwMaxLen = *pdwLen;
		*pdwLen = 0;

		DWORD dwLen = 0;
		DWORD dwTotalLen = 0;
		if (m_strTo.GetLength() > 0)
		{
			dwLen = *pdwLen - dwTotalLen;
			if (AtlMimeMakeRecipientsString(m_strTo, szRecip, &dwLen) != TRUE)
			{
				return FALSE;
			}
			szRecip+= dwLen;
			dwTotalLen = dwLen;
		}

		if (m_strCc.GetLength() > 0)
		{
			if (dwTotalLen)
			{
				*szRecip++ = ',';
				dwTotalLen++;
			}
			dwLen = *pdwLen - dwTotalLen;
			if (AtlMimeMakeRecipientsString(m_strCc, szRecip, &dwLen) != TRUE)
			{
				return FALSE;
			}
			szRecip+= dwLen;
			dwTotalLen+= dwLen;
		}

		if (m_strBcc.GetLength() > 0)
		{
			dwLen = m_strBcc.GetLength();
			if (dwTotalLen)
			{
				*szRecip++ = ',';
				dwTotalLen++;
			}
			dwLen = *pdwLen - dwTotalLen;
			Checked::memcpy_s(szRecip, dwMaxLen-dwTotalLen, m_strBcc, dwLen);
			szRecip+= dwLen;
			dwTotalLen+= dwLen;
		}

		*szRecip = '\0';
		*pdwLen = dwTotalLen;

		return TRUE;
	}


	// Get the sender
	inline LPCSTR GetSender() throw()
	{
		return m_strFrom;
	}

	// Set the sender
	inline BOOL SetSender(LPCTSTR szSender) throw()
	{
		if (szSender == NULL)
			return FALSE;

		_ATLTRY
		{
			m_strFrom = CT2CA(szSender);
			return TRUE;
		}
		_ATLCATCHALL()
		{
			return FALSE;
		}
	}

	// Set the subject
	inline BOOL SetSubject(LPCTSTR szSubject, UINT uiCodePage = 0) throw()
	{
		if (szSubject == NULL)
			return FALSE;

		_ATLTRY
		{
			CHeapPtr<char> szName;
			UINT nLen(0);

			BOOL bRet = AtlMimeConvertString(m_spMultiLanguage, uiCodePage, szSubject, &szName, &nLen);
			if (bRet)
			{
				m_strSubject.Empty();
				m_strSubject.Append(szName, (int)nLen);
				bRet = AtlMimeCharsetFromCodePage(m_szSubjectCharset, uiCodePage, m_spMultiLanguage, ATL_MAX_ENC_CHARSET_LENGTH);
			}

			return bRet;
		}
		_ATLCATCHALL()
		{
			return FALSE;
		}
	}

	// Get the subject
	inline LPCSTR GetSubject() throw()
	{
		return (LPCSTR)m_strSubject;
	}

	// Dump the header to hFile
	virtual inline BOOL WriteData(HANDLE hFile, LPOVERLAPPED pOverlapped, LPCSTR /*szBoundary*/, DWORD dwFlags = 0) throw()
	{
		if (pOverlapped == NULL)
		{
			return FALSE;
		}

		int nMaxSendLen = GetRequiredBufferSize(ATLSMTP_MAX_LINE_LENGTH-4);
		CHeapPtr<char> spSendBuffer;
		if (!spSendBuffer.Allocate(nMaxSendLen))
			return FALSE;
		
		// choose QEncode here, because the max QEncodeGetRequiredLength will always
		// return a value greater than BEncodeGetRequiredLength
		int nBufLen = __max(QEncodeGetRequiredLength(m_strSubject.GetLength(), 
								ATL_MAX_ENC_CHARSET_LENGTH),
						  QEncodeGetRequiredLength(m_strSenderName.GetLength(), 
								ATL_MAX_ENC_CHARSET_LENGTH)+m_strFrom.GetLength()+2);

		CHeapPtr<char> spBuf;
		if (!spBuf.Allocate(nBufLen))
			return FALSE;

		int nMaxLen = nBufLen;
		DWORD dwOffset = 0;

		char szDate[ATL_MIME_DATE_LEN];

		SetRfc822Time(szDate, ATL_MIME_DATE_LEN);
		char *pSendBuffer = spSendBuffer;

		DWORD dwLength = (DWORD) strlen(szDate);
		
		if(dwLength > ATLSMTP_MAX_LINE_LENGTH -2 -dwOffset )
			return FALSE;
		
		Checked::memcpy_s(pSendBuffer+dwOffset, nMaxSendLen-dwOffset, szDate, dwLength);
		dwOffset += dwLength;
		*(pSendBuffer+dwOffset++) = '\r';
		*(pSendBuffer+dwOffset++) = '\n';

		int dwHeaderPartLength = 0;
		*spBuf = '\0';

		// Get the sender name
		BOOL bRet = TRUE;
		BOOL bEncoded = FALSE;
		if (m_strSenderName.GetLength() > 0)
		{
			bRet = GetEncodedString(m_strSenderName, m_szSenderCharset, spBuf, nBufLen, dwLength, bEncoded);
			dwHeaderPartLength += dwLength;
		}

		// Get the sender email address
		if (bRet && m_strFrom.GetLength() > 0)
		{
				
			if (dwHeaderPartLength != 0)
			{
				if(dwHeaderPartLength + 1 > nBufLen)
					return FALSE;
			
				*(spBuf+dwHeaderPartLength++) = ' ';
			}
			
			if(dwHeaderPartLength + m_strFrom.GetLength() + 2 > nBufLen)
				return FALSE;

			*(spBuf+dwHeaderPartLength++) = '<';
			if (dwHeaderPartLength < 0 || dwHeaderPartLength > nMaxLen)
			{
				return FALSE;
			}
			Checked::memcpy_s(spBuf+dwHeaderPartLength, nMaxLen-dwHeaderPartLength, (LPCSTR)m_strFrom, m_strFrom.GetLength());
			dwHeaderPartLength+= m_strFrom.GetLength();
			*(spBuf+dwHeaderPartLength++) = '>';
		}

		// Output the "From: " line
		if (bRet && dwHeaderPartLength != 0)
		{
			const char szFrom[] = "From: ";
			if(sizeof(szFrom)/sizeof(szFrom[0])-1 > ATLSMTP_MAX_LINE_LENGTH -2 -dwOffset )
				return FALSE;
			if (dwOffset > static_cast<DWORD>(nMaxSendLen))
			{
				return FALSE;
			}
			Checked::memcpy_s(pSendBuffer+dwOffset, nMaxSendLen-dwOffset, szFrom, _countof(szFrom)-1);
			dwOffset+= (sizeof(szFrom)/sizeof(szFrom[0])-1) ;
			DWORD dwWritten = ATLSMTP_MAX_LINE_LENGTH - 2 - dwOffset;
			bRet = FormatField((LPBYTE)(char*)spBuf, dwHeaderPartLength, (LPBYTE)(pSendBuffer+dwOffset), &dwWritten, dwFlags);
			dwOffset += dwWritten;
			*(pSendBuffer+dwOffset++) = '\r';
			*(pSendBuffer+dwOffset++) = '\n';
		}

		// Output the subject
		if (bRet && m_strSubject.GetLength() > 0)
		{
			dwLength = 0;
			bRet = GetEncodedString(m_strSubject, m_szSubjectCharset, spBuf, nBufLen, dwLength, bEncoded);
			if (bRet && dwLength != 0)
			{
				const char szSubject[] = "Subject: ";
				if(sizeof(szSubject)/sizeof(szSubject[0])-1 > ATLSMTP_MAX_LINE_LENGTH -2 -dwOffset )
					return FALSE;
				if (dwOffset > static_cast<DWORD>(nMaxSendLen))
				{
					return FALSE;
				}
				Checked::memcpy_s(pSendBuffer+dwOffset, nMaxSendLen-dwOffset, szSubject, _countof(szSubject)-1);
				dwOffset+= (sizeof(szSubject)/sizeof(szSubject[0])-1);
				DWORD dwWritten = ATLSMTP_MAX_LINE_LENGTH - 2 - dwOffset;
				bRet = FormatField((LPBYTE)(char*)spBuf, dwLength, (LPBYTE)(pSendBuffer+dwOffset), &dwWritten, dwFlags);
				dwOffset += dwWritten;
				*(pSendBuffer+dwOffset++) = '\r';
				*(pSendBuffer+dwOffset++) = '\n';
			}
		}

		// Output the "To:" line
		if (bRet && m_strTo.GetLength() > 0)
		{
			const char szTo[] = "To: ";
			if(sizeof(szTo)/sizeof(szTo[0])-1 > ATLSMTP_MAX_LINE_LENGTH -2 -dwOffset )
					return FALSE;
			if (dwOffset > static_cast<DWORD>(nMaxSendLen))
			{
				return FALSE;
			}
			Checked::memcpy_s(pSendBuffer+dwOffset, nMaxSendLen-dwOffset, szTo, _countof(szTo)-1);
			dwOffset+= (sizeof(szTo)/sizeof(szTo[0]) -1);
			DWORD dwWritten = ATLSMTP_MAX_LINE_LENGTH - 2 - dwOffset;
			bRet = FormatRecipients((LPBYTE)((LPCSTR)m_strTo), m_strTo.GetLength(), (LPBYTE)(pSendBuffer+dwOffset), &dwWritten);
			dwOffset+= dwWritten;
			*(pSendBuffer+dwOffset++) = '\r';
			*(pSendBuffer+dwOffset++) = '\n';
		}

		// Output the "CC:" line
		if (bRet && m_strCc.GetLength() > 0)
		{
			const char szCC[] = "CC: ";
			if(sizeof(szCC)/sizeof(szCC[0])-1 > ATLSMTP_MAX_LINE_LENGTH -2 -dwOffset )
				return FALSE;
			if (dwOffset > static_cast<DWORD>(nMaxSendLen))
			{
				return FALSE;
			}
			Checked::memcpy_s(pSendBuffer+dwOffset, nMaxSendLen-dwOffset, szCC, _countof(szCC)-1);
			dwOffset+= (sizeof(szCC)/sizeof(szCC[0]) -1);
			DWORD dwWritten = ATLSMTP_MAX_LINE_LENGTH - 2 - dwOffset;
			bRet = FormatRecipients((LPBYTE)((LPCSTR)m_strCc), m_strCc.GetLength(), (LPBYTE)(pSendBuffer+dwOffset), &dwWritten);
			dwOffset+= dwWritten;
			*(pSendBuffer+dwOffset++) = '\r';
			*(pSendBuffer+dwOffset++) = '\n';
		}

		// Send the header
		if (bRet && dwOffset)
			bRet = AtlSmtpSendAndWait(hFile, pSendBuffer, dwOffset, pOverlapped);

		return bRet;
	}

protected:

	// Make the mime header
	virtual inline BOOL MakeMimeHeader(CStringA& /*header*/, LPCSTR /*szBoundary*/) throw()
	{
		// The message header does not have its own MIME header
		ATLASSERT(FALSE);
		return TRUE;
	}

	// Get an encoded string for a header field
	inline BOOL GetEncodedString(__in CStringA& headerString, __in LPCSTR szCharset, __out_ecount_part_z(nBufLen, dwLength) LPSTR szBuf, __in int nBufLen, __out DWORD& dwLength, __out BOOL& bEncoded) throw()
	{
//		BOOL bEncoded = FALSE;
		bEncoded = FALSE;
		if (m_spMultiLanguage.p)
		{
			// only encode if there are 8bit characters
			int nExtendedChars = GetExtendedChars(headerString, headerString.GetLength());
			if (nExtendedChars)
			{
				// choose smallest encoding
				if (((nExtendedChars*100)/headerString.GetLength()) < 17)
				{
					int nEncCnt = 0;
					if (!QEncode((LPBYTE)((LPCSTR)headerString), headerString.GetLength(), szBuf, &nBufLen, szCharset, &nEncCnt))
					{
						return FALSE;
					}

					//if no unsafe characters were encountered, just output it
					if (nEncCnt != 0)
					{
						bEncoded = TRUE;
					}
				}
				else
				{
					if (!BEncode((LPBYTE)((LPCSTR)headerString), headerString.GetLength(), szBuf, &nBufLen, szCharset))
					{
						return FALSE;
					}

					bEncoded = TRUE;
				}
			}
		}

		if (!bEncoded)
		{
			// there was no encoding
			dwLength = (DWORD) headerString.GetLength();
			if(dwLength > DWORD(nBufLen))
				return FALSE;
			Checked::memcpy_s(szBuf, nBufLen, headerString, dwLength);
		}
		else
		{
			dwLength = nBufLen;
		}
		return TRUE;
	}


	// Helper function for adding recipients
	inline BOOL AddRecipientHelper(CStringA& str, LPCTSTR szAddress, LPCTSTR szName = NULL, UINT uiCodePage = 0) throw()
	{
		if ((szAddress == NULL) && (szName == NULL))
		{
			return FALSE;
		}

		_ATLTRY
		{
			if (szName)
			{
				CHeapPtr<char> szNamePtr;
				UINT nLen(0);

				BOOL bRet = AtlMimeConvertString(m_spMultiLanguage, uiCodePage, szName, &szNamePtr, &nLen);
				if (bRet)
				{
					CStringA Name(szNamePtr, (int)nLen);

					char szCharset[ATL_MAX_ENC_CHARSET_LENGTH];

					if (!AtlMimeCharsetFromCodePage(szCharset, uiCodePage, m_spMultiLanguage, ATL_MAX_ENC_CHARSET_LENGTH))
					{
						return FALSE;
					}

					CFixedStringT<CStringA, 256> strBuf;

					int nBufLen = QEncodeGetRequiredLength(Name.GetLength(), 
								ATL_MAX_ENC_CHARSET_LENGTH)+1;

					char * szBuf = strBuf.GetBuffer(nBufLen);
					if (szBuf == NULL)
					{
						return FALSE;
					}

					DWORD dwLength = 0;
					BOOL bEncoded = FALSE;
					if (!GetEncodedString(Name, szCharset, szBuf, nBufLen, dwLength, bEncoded))
					{
						strBuf.ReleaseBuffer();
						return FALSE;
					}

					strBuf.ReleaseBuffer(dwLength);

					// append comma if there are existing recipients
					if (str.GetLength() != 0)
					{
						str.Append(", ", 2);
					}

					if (bEncoded == FALSE)
					{
						// need to escape the string if no encoding
						strBuf.Replace("\\", "\\\\");
						strBuf.Replace("\"", "\\\"");

						// wrap the unescaped name in quotes
						str.Append("\"", 1);
					}
					str += strBuf;
					if (bEncoded == FALSE)
					{
						// close quote
						str.Append("\"", 1);
					}
				}
				else
				{
					return bRet;
				}
			}

			if (szAddress)
			{
				if (szName)
				{
					str.Append(" ", 1);
				}
				else
				{
					// append comma if there are existing recipients
					if (str.GetLength() != 0)
					{
						str.Append(", ", 2);
					}
				}
				str.Append("<", 1);
				str += CT2CA(szAddress);
				str.Append(">", 1);
			}
			return TRUE;
		}
		_ATLCATCHALL()
		{
			return FALSE;
		}
	}

	// Get the formatted header information
	inline BOOL FormatField(LPBYTE pbSrcData, int nSrcLen, LPBYTE pbDest, 
		DWORD* pnBufLen, DWORD dwFlags = 0) throw()
	{
		if(pnBufLen == NULL)
			return FALSE;
			
		int nRead = 0;

		// 9 is the length of the maximum field name : "Subject :"
		// we set that here for simplicity
		int nLineLen = 9;
		DWORD nWritten = 0;

		//subtract 2 from these because it's easier for when we have
		//to break lines with a CRLF (and tab if necessary)
		int nMaxLineLength = ATLSMTP_MAX_LINE_LENGTH-3;
		while (nRead < nSrcLen)
		{
			//if we're at the end of the line, break it
			if (nLineLen == nMaxLineLength)
			{
				if( nWritten + 2  > *pnBufLen)
					return FALSE;
					
				*pbDest++ = '\r';
				*pbDest++ = '\n';
				nWritten+= 2;
				nLineLen = -1;

				if ((dwFlags & ATLSMTP_FORMAT_SMTP))
				{
					if(nWritten + 1 > *pnBufLen)
						return FALSE;
						
					*pbDest++ = '\t';
					nWritten++;
					nLineLen++;
				}
			}

			//if we hit a CRLF, reset nLineLen
			if (*pbSrcData == '\n' && nRead > 0 && *(pbSrcData-1) == '\r')
			{
				nLineLen = -1;
			}

			if(nWritten + 1 > *pnBufLen)
				return FALSE;
				
			*pbDest++ = *pbSrcData++;
			nRead++;
			nWritten++;
			nLineLen++;
		}

		*pnBufLen = (DWORD)nWritten;

		return TRUE;
	}


	// Get the formatted recipient information
	inline BOOL FormatRecipients(LPBYTE pbSrcData, int nSrcLen, LPBYTE pbDest, 
		DWORD* pnBufLen) throw()
	{
		
		if(pnBufLen == NULL)
			return FALSE;
			
		int nRead    = 0;
		DWORD nWritten = 0;

		while (nRead < nSrcLen)
		{
			if (*pbSrcData == ',')
			{
				if(nWritten + 4 > *pnBufLen)
					return FALSE;
					
				*pbDest++ = *pbSrcData++;
				nRead++;
				if (nRead+1 <= nSrcLen && *pbSrcData == ' ')
				{
					pbSrcData++;
					nRead++;
				}
				*pbDest++ = '\r';
				*pbDest++ = '\n';
				*pbDest++ = '\t';
				nWritten+= 4;

				continue;
			}

			if(nWritten + 1 > *pnBufLen)
				return FALSE;
				
			*pbDest++ = *pbSrcData++;
			nRead++;
			nWritten++;
		}

		*pnBufLen = nWritten;

		return TRUE;
	}

	// Get the required buffer size for the header
	inline int GetRequiredBufferSize(int nMaxLineLength) throw()
	{
		const static DWORD DATELINE    = 27;
		const static DWORD FROMLINE    = 10;
		const static DWORD TOLINE      = 6;
		const static DWORD CCLINE      = 6;
		const static DWORD SUBJECTLINE = 11;

		//data lengths (QEncoding potentially takes up more space than BEncoding,
		//so default to it)
		int nRequiredLength = QEncodeGetRequiredLength(m_strSenderName.GetLength(), ATL_MAX_ENC_CHARSET_LENGTH)
			+QEncodeGetRequiredLength(m_strSubject.GetLength(), ATL_MAX_ENC_CHARSET_LENGTH);
		nRequiredLength += m_strFrom.GetLength()+m_strTo.GetLength()+m_strCc.GetLength();

		//Add space for date
		nRequiredLength += DATELINE;

		//Add space for From: line
		nRequiredLength += FROMLINE;

		//Add space for To: line
		nRequiredLength += TOLINE;

		//Add space for Cc: line
		nRequiredLength += CCLINE;

		//Add space for Subject: line
		nRequiredLength += SUBJECTLINE;

		//Add space for line breaks and tabs
		nRequiredLength += 3*(nRequiredLength/nMaxLineLength);

		//Trailing CRLF
		nRequiredLength += 2;

		return nRequiredLength;
	}

}; // class CMimeHeader


// CMimeAttachment is an abstract base class for MIME message attachments.
// It serves as a base class for CMimeFileAttachment and CMimeRawAttachment
class CMimeAttachment : public CMimeBodyPart
{
protected:

	// the encoding scheme (ATLSMTP_BASE64_ENCODE, ATLSMTP_UUENCODE, ATLSMTP_QP_ENCODE)
	int      m_nEncodingScheme;

	// the content type of the attachment
	CStringA m_ContentType;

	// the character set
	char     m_szCharset[ATL_MAX_ENC_CHARSET_LENGTH];

	// the encode string ("base64", "quoted-printable", "uuencode")
	char     *m_pszEncodeString;

	// the display name of the attachment
	TCHAR    m_szDisplayName[_MAX_FNAME];

public:
	CMimeAttachment() throw()
		:m_nEncodingScheme(ATLSMTP_BASE64_ENCODE), m_pszEncodeString(NULL)
	{
		m_szCharset[0] = 0;
		m_szDisplayName[0] = 0;
	}

	virtual ~CMimeAttachment() throw()
	{
	}

	// CMimeFileAttachment and CMimeRawAttachment have to handle their own dumping
	virtual inline BOOL WriteData(HANDLE hFile, LPOVERLAPPED pOverlapped, LPCSTR szBoundary, DWORD dwFlags = 0) = 0;

	// Set the encoding scheme of the attachment
	inline BOOL SetEncodingScheme(int nScheme) throw()
	{
		if (nScheme != ATLSMTP_BASE64_ENCODE && nScheme != ATLSMTP_UUENCODE && nScheme != ATLSMTP_QP_ENCODE)
		{
			return FALSE;
		}

		m_nEncodingScheme = nScheme;
		return TRUE;
	}

	// Set the Content-Type of the attachment
	inline BOOL SetContentType(LPCTSTR szContent) throw()
	{
		_ATLTRY
		{
			m_ContentType = CT2CA(szContent);
			return TRUE;
		}
		_ATLCATCHALL()
		{
			return FALSE;
		}
	}

	// Get the content type of the attachment
	virtual inline LPCSTR GetContentType() throw()
	{
		return m_ContentType;
	}

	// Get the character set of the attachment
	virtual inline LPCSTR GetCharset() throw()
	{
		return m_szCharset;
	}

	virtual ATL_NOINLINE CMimeBodyPart* Copy() = 0;

	const CMimeAttachment& operator=(const CMimeAttachment& that) throw( ... )
	{
		if (this != &that)
		{
			m_nEncodingScheme = that.m_nEncodingScheme;
			m_ContentType = that.m_ContentType;
			Checked::strcpy_s(m_szCharset, ATL_MAX_ENC_CHARSET_LENGTH, that.m_szCharset);
			m_pszEncodeString = that.m_pszEncodeString;
			Checked::tcscpy_s(m_szDisplayName, _countof(m_szDisplayName), that.m_szDisplayName);
		}

		return *this;
	}

protected:

	// Make the MIME header for the attachment
	virtual inline BOOL MakeMimeHeader(CStringA& header, LPCSTR szBoundary) throw()
	{
		// if no display name is specified, default to "rawdata"
		return MakeMimeHeader(header, szBoundary, _T("rawdata"));
	}

	// Make the MIME header with the specified filename
	virtual inline BOOL MakeMimeHeader(CStringA& header, LPCSTR szBoundary, LPCTSTR szFileName)
	{
		ATLENSURE(szBoundary != NULL);
		ATLASSERT(szFileName != NULL);
		ATLASSUME(m_pszEncodeString != NULL);

		char szBegin[256];
		if (*szBoundary)
		{
			// this is not the only body part
			Checked::memcpy_s(szBegin, 256, ATLMIME_SEPARATOR, sizeof(ATLMIME_SEPARATOR));
			Checked::memcpy_s(szBegin+6, 250, szBoundary, ATL_MIME_BOUNDARYLEN);
			*(szBegin+(ATL_MIME_BOUNDARYLEN+6)) = '\0';
		}
		else
		{
			// this is the only body part, so output the MIME header
			Checked::memcpy_s(szBegin, 256, ATLMIME_VERSION, sizeof(ATLMIME_VERSION));
		}

		// Get file name with the path stripped out
		TCHAR szFile[MAX_PATH+1];
		TCHAR szExt[_MAX_EXT+1];
		Checked::tsplitpath_s(szFileName, NULL, 0, NULL, 0, szFile, _countof(szFile), szExt, _countof(szExt));
		Checked::tcscat_s(szFile, _countof(szFile), szExt);

		_ATLTRY
		{
			CT2CAEX<MAX_PATH+1> szFileNameA(szFile);

			CStringA szDisplayName(szFile);
			if (m_szDisplayName[0] != '\0')
			{
				szDisplayName = CT2CAEX<_MAX_FNAME+1>(m_szDisplayName);
			}

			header.Format("%s\r\nContent-Type: %s;\r\n\tcharset=\"%s\"\r\n\tname=\"%s\"\r\n"
				"Content-Transfer-Encoding: %s\r\nContent-Disposition: attachment;\r\n\tfilename=\"%s\"\r\n\r\n",
				szBegin, (LPCSTR) m_ContentType, m_szCharset, (LPCSTR) szDisplayName, m_pszEncodeString, (LPCSTR) szFileNameA); 
			return TRUE;
		}
		_ATLCATCHALL()
		{
			return FALSE;
		}
	}

	// Get encoding information
	inline BOOL GetEncodingInformation(int* pnRequiredLength, int* pnLineLength)
	{
		ATLENSURE(pnRequiredLength != NULL);
		ATLENSURE(pnLineLength != NULL);

		switch(m_nEncodingScheme)
		{
			case ATLSMTP_BASE64_ENCODE:
				m_pszEncodeString = "base64";
				*pnLineLength = ATLSMTP_MAX_BASE64_LINE_LENGTH;
				*pnRequiredLength = Base64EncodeGetRequiredLength(ATLSMTP_MAX_BASE64_LINE_LENGTH);
				break;
			case ATLSMTP_UUENCODE:
				m_pszEncodeString ="uuencode";
				*pnLineLength = ATLSMTP_MAX_UUENCODE_LINE_LENGTH;
				*pnRequiredLength = UUEncodeGetRequiredLength(ATLSMTP_MAX_UUENCODE_LINE_LENGTH);
				break;
			case ATLSMTP_QP_ENCODE:
				m_pszEncodeString = "quoted-printable";
				*pnLineLength = ATLSMTP_MAX_QP_LINE_LENGTH;
				*pnRequiredLength = QPEncodeGetRequiredLength(ATLSMTP_MAX_QP_LINE_LENGTH);
				break;
			default:
				return FALSE;
		}
		return TRUE;
	}

}; // class CMimeAttachment


// CMimeFileAttachment represents a MIME file attachment body part
class CMimeFileAttachment : public CMimeAttachment
{

protected:
	// The filename
	TCHAR m_szFileName[MAX_PATH+1];

public:
	CMimeFileAttachment() throw()
	{
		m_szFileName[0] = 0;
	}

	virtual ATL_NOINLINE CMimeBodyPart* Copy() throw( ... )
	{
		CAutoPtr<CMimeFileAttachment> pNewAttachment;
		ATLTRY(pNewAttachment.Attach(new CMimeFileAttachment));
		if (pNewAttachment)
			*pNewAttachment = *this;

		return pNewAttachment.Detach();
	}

	const CMimeFileAttachment& operator=(const CMimeFileAttachment& that) throw( ... )
	{
		if (this != &that)
		{
			CMimeAttachment::operator=(that);
			Checked::tcscpy_s(m_szFileName, _countof(m_szFileName), that.m_szFileName);
		}

		return *this;
	}


	// Initialize the file attachment
	// szFileName - the actual file name
	// szDisplayName - the display name for the file (optional)
	// pMultiLanguage - the IMulitLanguage pointer for codepage to charset conversion (optional)
	// uiCodePage - the code page (optional)
	inline BOOL Initialize(LPCTSTR szFileName, LPCTSTR szDisplayName = NULL, IMultiLanguage* pMultiLanguage = NULL, UINT uiCodePage = 0) throw()
	{
		if (!AtlMimeCharsetFromCodePage(m_szCharset, uiCodePage, pMultiLanguage, ATL_MAX_ENC_CHARSET_LENGTH))
			return FALSE;

		if( _tcslen(szFileName) > MAX_PATH )
		{
			return FALSE;
		}
		Checked::tcscpy_s(m_szFileName, _countof(m_szFileName), szFileName);

		if (szDisplayName)
		{
			// use the user-specified display name
			size_t nLen = _tcslen(szDisplayName)+1;
			if (nLen <= _countof(m_szDisplayName))
			{
				Checked::tcscpy_s(m_szDisplayName, _countof(m_szDisplayName), szDisplayName);
			}
			else
			{
				Checked::tcsncpy_s(m_szDisplayName, _countof(m_szDisplayName), szDisplayName, _countof(m_szDisplayName) - 4);
				Checked::tcscpy_s(m_szDisplayName + _countof(m_szDisplayName) - 4, 4, _T("..."));
			}
		}
		else
		{
			// otherwise there is no display name
			*m_szDisplayName = '\0';
		}
		return TRUE;
	}

	// Dump the data for the file attachment
	virtual inline BOOL WriteData(HANDLE hFile, LPOVERLAPPED pOverlapped, LPCSTR szBoundary, DWORD dwFlags = 0) throw()
	{
		if ((pOverlapped == NULL) || (szBoundary == NULL))
		{
			return FALSE;
		}

		int nLineLength = 0;
		int nRequiredLength = 0;

		if (!GetEncodingInformation(&nRequiredLength, &nLineLength))
			return FALSE;

		//Try to open the file that is being attached
		CAtlFile readFile;
		if (FAILED(readFile.Create(m_szFileName, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING)))
			return FALSE;

		//Make the mime header
		CStringA header;
		if (!MakeMimeHeader(header, szBoundary, m_szFileName))
		{
			return FALSE;
		}

		//Try to send the mime header
		if (!AtlSmtpSendAndWait(hFile, ((LPCSTR)header), header.GetLength(), pOverlapped))
		{
			return FALSE;
		}

		int nGetLines = ATLSMTP_GET_LINES;

		nRequiredLength *= nGetLines;

		//dwToGet is the total number of characters to attempt to get
		DWORD dwToGet = (DWORD)nGetLines*nLineLength;

		//allocate the data array
		CHeapPtr<BYTE> spData;
		if (!spData.Allocate(dwToGet+1))
			return FALSE;

// if double buffering is defined, create two buffers
#ifdef ATLSMTP_DOUBLE_BUFFERED
		CHeapPtr<char> buffer1;
		if (!buffer1.Allocate(nRequiredLength+3))
			return FALSE;

		CHeapPtr<char> buffer2;
		if (!buffer2.Allocate(nRequiredLength+3))
			return FALSE;

		char* currBuffer = buffer1;
		char* prevBuffer = NULL;
		int nCurrBuffer = 0;
		DWORD dwPrevLength = 0;
#else
		CHeapPtr<char> currBuffer;
		if (!currBuffer.Allocate(nRequiredLength+3))
			return FALSE;

#endif // ATLSMTP_DOUBLE_BUFFERED

		int nEncodedLength = nRequiredLength;
		BOOL bRet = FALSE;
		DWORD dwRead = 0;
		DWORD dwTotalRead = 0;
		DWORD dwCurrRead = 0;

		do
		{
			do 
			{
				//Read a chunk of data from the file increment buffer offsets and amount to read
				//based on what's already been read in this iteration of the loop
				HRESULT hr = readFile.Read(((LPBYTE)spData)+dwCurrRead, dwToGet-dwCurrRead, dwRead);
				if (FAILED(hr))
				{
					if (hr != AtlHresultFromWin32(ERROR_MORE_DATA))
					{
						return FALSE;
					}
				}
				dwCurrRead += dwRead;

			} while (dwRead != 0 && dwCurrRead < dwToGet);

			//reset nEncodedLength
			nEncodedLength = nRequiredLength;
			switch (m_nEncodingScheme)
			{
				case ATLSMTP_BASE64_ENCODE:
					//if we are at the end of input (dwCurrRead < dwToGet), output the trailing padding if necessary
					//(ATL_FLAG_NONE)
					bRet = Base64Encode(spData, dwCurrRead, currBuffer, &nEncodedLength, 
						(dwCurrRead < dwToGet ? ATL_BASE64_FLAG_NONE: ATL_BASE64_FLAG_NOPAD));
					//Base64Encoding needs explicit CRLF added
					if (dwCurrRead < dwToGet)
					{
						currBuffer[nEncodedLength++] = '\r';
						currBuffer[nEncodedLength++] = '\n';
					}
					break;
				case ATLSMTP_UUENCODE:
					//if we are at the beginning of the input, output the header (ATL_UUENCODE_HEADER)
					//if we are the end of input (dwCurrRead < dwToGet), output the 'end'
					//we are encoding for purposes of sending mail, so stuff dots (ATL_UUENCODE_DOT)
					bRet = UUEncode(spData, dwCurrRead, currBuffer, &nEncodedLength, m_szFileName,
									(dwTotalRead > 0 ? 0 : ATLSMTP_UUENCODE_HEADER) | 
									(dwCurrRead < dwToGet ? ATLSMTP_UUENCODE_END : 0) | 
									((dwFlags & ATLSMTP_FORMAT_SMTP) ? ATLSMTP_UUENCODE_DOT : 0));
					break;
				case ATLSMTP_QP_ENCODE:
					//we are encoding for purposes of sending mail, so stuff dots
					bRet = QPEncode(spData, dwCurrRead, currBuffer, &nEncodedLength, 
									((dwFlags & ATLSMTP_FORMAT_SMTP) ? ATLSMTP_QPENCODE_DOT : 0) |
									(dwCurrRead < dwToGet ? 0 : ATLSMTP_QPENCODE_TRAILING_SOFT));
					break;
			}
			//try to send the encoded data
#ifdef ATLSMTP_DOUBLE_BUFFERED
			if (bRet)
			{
				bRet = AtlSmtpSendOverlapped(hFile, currBuffer, nEncodedLength, 
					prevBuffer, dwPrevLength, pOverlapped);
			}

			//swap the buffers
			dwPrevLength = nEncodedLength;
			prevBuffer = currBuffer;
			currBuffer = (nCurrBuffer == 0 ? buffer2 : buffer1);
			nCurrBuffer = (nCurrBuffer == 0 ? 1 : 0);
#else
			if (bRet)
			{
				bRet = AtlSmtpSendAndWait(hFile, currBuffer, nEncodedLength, pOverlapped);
			}
#endif // ATLSMTP_DOUBLE_BUFFERED

			dwTotalRead += dwCurrRead;
			if (dwRead != 0)
				dwCurrRead = 0;

			nEncodedLength = nRequiredLength;

		} while (dwRead != 0 && bRet);

		//ensure that the last Send sent all the data
#ifdef ATLSMTP_DOUBLE_BUFFERED
		DWORD dwWritten = 0, dwErr = 0;
		if (!GetOverlappedResult(hFile, pOverlapped, &dwWritten, TRUE))
		{
			if ((dwErr = GetLastError()) != ERROR_IO_PENDING && dwErr != ERROR_IO_INCOMPLETE)
			{
				bRet = FALSE;
			}
			else if (dwWritten < dwPrevLength)
			{
				bRet = AtlSmtpSendAndWait(hFile, prevBuffer+dwWritten, 
					dwPrevLength-dwWritten, pOverlapped);
			}
		}
#endif // ATLSMTP_DOUBLE_BUFFERED

		//for uuencoding, if the last chunk read was of size dwToGet, but it was also the end of the file,
		//the "end" keyword will not get encoded, so a check is necessary
		if (m_nEncodingScheme == ATLSMTP_UUENCODE && dwCurrRead == dwToGet)
		{
			bRet = UUEncode(spData, 0, currBuffer, &nEncodedLength, m_szFileName, 
							(dwFlags & ATLSMTP_FORMAT_SMTP ? ATLSMTP_UUENCODE_DOT : 0) |
							ATLSMTP_UUENCODE_END);
			if (bRet)
			{
				bRet = AtlSmtpSendAndWait(hFile, currBuffer, nEncodedLength, pOverlapped);
			}
		}

		return bRet;
	}
}; // class CMimeFileAttachment

// CMimeRawAttachment represents a file attachment MIME body part.
// The data provided is not a file, but a blob of raw data.
class CMimeRawAttachment : public CMimeAttachment
{
protected:

	//the raw data
	void* m_pvRaw;

	//the length
	DWORD m_dwLength;

	//whether or not we own it
	bool  m_bShared;

public:
	CMimeRawAttachment() throw()
		:m_dwLength(0), m_bShared(false), m_pvRaw(NULL)
	{
	}

	~CMimeRawAttachment() throw()
	{
		//If we own the raw data, free it
		if (!m_bShared && m_pvRaw)
			free(m_pvRaw);
	}

	virtual ATL_NOINLINE CMimeBodyPart* Copy() throw( ... )
	{
		CAutoPtr<CMimeRawAttachment> pNewAttachment;
		ATLTRY(pNewAttachment.Attach(new CMimeRawAttachment));
		if (pNewAttachment)
			*pNewAttachment = *this;

		return pNewAttachment.Detach();
	}

	const CMimeRawAttachment& operator=(const CMimeRawAttachment& that) throw( ... )
	{
		if (this != &that)
		{
			CMimeAttachment::operator=(that);
			if (!m_bShared && m_pvRaw)
				free(m_pvRaw);

			m_bShared = that.m_bShared;
			m_dwLength = that.m_dwLength;

			if (m_bShared)
			{
				m_pvRaw = that.m_pvRaw;
			}
			else
			{
				m_pvRaw = malloc(m_dwLength);
				if (m_pvRaw)
				{
					Checked::memcpy_s(m_pvRaw, m_dwLength, that.m_pvRaw, m_dwLength);
				}
			}
		}

		return *this;
	}

	// Initialize the attachment
	// pData - the data
	// nDataLength - the size of pData in BYTEs
	// bCopyData - flag specifying whether CMimeRawAttachment should make a copy of the data (optional)
	// pMultiLanguage - the IMultiLanguage pointer for codepage to character set conversion (optional)
	// uiCodePage - the codepage (optional)
	inline BOOL Initialize(void* pData, DWORD nDataLength, BOOL bCopyData = TRUE, LPCTSTR szDisplayName = NULL, 
		IMultiLanguage* pMultiLanguage = NULL, UINT uiCodePage = 0) throw()
	{
		// if we're already attached to some data, and it's not shared, free it
		if (m_pvRaw && !m_bShared)
			free(m_pvRaw);
		m_pvRaw = NULL;

		m_dwLength = nDataLength;
		if (bCopyData)
		{
			m_pvRaw = calloc(sizeof(BYTE),m_dwLength);
			if (!m_pvRaw)
			{
				return FALSE;
			}
			Checked::memcpy_s(m_pvRaw, m_dwLength, pData, m_dwLength);
			m_bShared = false;
		}
		else
		{
			m_pvRaw = pData;
			m_bShared = true;
		}

		if (!AtlMimeCharsetFromCodePage(m_szCharset, uiCodePage, pMultiLanguage, ATL_MAX_ENC_CHARSET_LENGTH))
			return FALSE;

		if (szDisplayName)
		{
			// use the user-specified display name
			Checked::tcscpy_s(m_szDisplayName, _countof(m_szDisplayName), szDisplayName);
			m_szDisplayName[_countof(m_szDisplayName)-1] = 0;
		}
		else
		{
			// no display name
			*m_szDisplayName = '\0';
		}
		return TRUE;
	}

	// Output the data--similar to CFileAttachment::WriteData
	// See CFileAttachment::WriteData for comments
	virtual inline BOOL WriteData(HANDLE hFile, LPOVERLAPPED pOverlapped, LPCSTR szBoundary, DWORD dwFlags = 0) throw()
	{
		if ((pOverlapped == NULL) || (szBoundary == NULL))
		{
			return FALSE;
		}

		if (!m_pvRaw)
			return FALSE;

		int nLineLength = 0, nRequiredLength = 0;
		if (!GetEncodingInformation(&nRequiredLength, &nLineLength))
			return FALSE;

		CStringA header;

		if (!MakeMimeHeader(header, szBoundary))
		{
			return FALSE;
		}

		if (!AtlSmtpSendAndWait(hFile, ((LPCSTR)header), header.GetLength(), pOverlapped))
		{
			return FALSE;
		}

		int nGetLines = ATLSMTP_GET_LINES;
		DWORD dwCurrChunk = 0;
		nRequiredLength *= nGetLines;
		DWORD dwToGet = (DWORD)nGetLines*nLineLength;
		int nDestLen = nRequiredLength;
		BOOL bRet = FALSE;
		DWORD dwRead = 0;
#ifdef ATLSMTP_DOUBLE_BUFFERED
		CHeapPtr<char> buffer1;
		if (!buffer1.Allocate(nRequiredLength+3))
			return FALSE;

		CHeapPtr<char> buffer2;
		if (!buffer2.Allocate(nRequiredLength+3))
			return FALSE;

		char* currBuffer = buffer1;
		char* prevBuffer = NULL;
		int nCurrBuffer = 0;
		DWORD dwPrevLength = 0;
#else
		CHeapPtr<char> currBuffer;
		if (!currBuffer.Allocate(nRequiredLength+3))
			return FALSE;
#endif // ATLSMTP_DOUBLE_BUFFERED

		do 
		{
			if ((m_dwLength-dwRead) <= dwToGet)
				dwCurrChunk = m_dwLength-dwRead;
			else
				dwCurrChunk = dwToGet;
			switch(m_nEncodingScheme)
			{
				case ATLSMTP_BASE64_ENCODE:
					bRet = Base64Encode(((LPBYTE)(m_pvRaw))+dwRead, dwCurrChunk, currBuffer, &nDestLen, 
						(dwRead < m_dwLength) ? ATL_BASE64_FLAG_NONE : ATL_BASE64_FLAG_NOPAD);
					if (dwRead+dwCurrChunk == m_dwLength)
					{
						currBuffer[nDestLen++] = '\r';
						currBuffer[nDestLen++] = '\n';
					}
					break;
				case ATLSMTP_UUENCODE:
					bRet = UUEncode(((LPBYTE)(m_pvRaw))+dwRead, dwCurrChunk, currBuffer, &nDestLen, _T("rawdata"), 
									(dwRead > 0 ? 0 : ATLSMTP_UUENCODE_HEADER) | 
									(dwRead+dwCurrChunk == m_dwLength ? ATLSMTP_UUENCODE_END : 0) | 
									((dwFlags & ATLSMTP_FORMAT_SMTP) ? ATLSMTP_UUENCODE_DOT : 0));
					break;
				case ATLSMTP_QP_ENCODE:
					bRet = QPEncode(((LPBYTE)(m_pvRaw))+dwRead, dwCurrChunk, currBuffer, &nDestLen, 
									((dwFlags & ATLSMTP_FORMAT_SMTP) ? ATLSMTP_QPENCODE_DOT : 0) | 
									(dwRead+dwCurrChunk == m_dwLength ? 0 : ATLSMTP_QPENCODE_TRAILING_SOFT));
					break;
			}
			if (!bRet)
				break;
#ifdef ATLSMTP_DOUBLE_BUFFERED
			bRet = AtlSmtpSendOverlapped(hFile, currBuffer, nDestLen, prevBuffer, dwPrevLength, pOverlapped);
			dwPrevLength = (DWORD)nDestLen;
			prevBuffer = currBuffer;
			currBuffer = (nCurrBuffer == 0 ? buffer2 : buffer1);
			nCurrBuffer = (nCurrBuffer == 0 ? 1 : 0);
#else
			bRet = AtlSmtpSendAndWait(hFile, currBuffer, nDestLen, pOverlapped);
#endif // ATLSMTP_DOUBLE_BUFFERED

			nDestLen = nRequiredLength;
			dwRead += dwCurrChunk;
		} while (bRet && (dwRead < m_dwLength));

		//ensure all data is sent from prevBuffer
#ifdef ATLSMTP_DOUBLE_BUFFERED
		DWORD dwWritten = 0, dwErr = 0;
		if (!GetOverlappedResult(hFile, pOverlapped, &dwWritten, TRUE))
		{
			if ((dwErr = GetLastError()) != ERROR_IO_PENDING && dwErr != ERROR_IO_INCOMPLETE)
				bRet = FALSE;
			else if (dwWritten < dwPrevLength)
				bRet = AtlSmtpSendAndWait(hFile, prevBuffer+dwWritten, dwPrevLength-dwWritten, pOverlapped);
		}
#endif // ATLSMTP_DOUBLE_BUFFERED

		return bRet;
	}
}; // class CMimeRawAttachment


// CMimeText - represents a text body part in MIME body
class CMimeText : public CMimeBodyPart
{
protected:

	// the text
	CHeapPtr<char> m_szText;

	// the character set
	char     m_szCharset[ATL_MAX_ENC_CHARSET_LENGTH];

	// the text length
	int      m_nTextLen;

public:
	CMimeText() throw()
		:m_nTextLen(0)
	{
		Checked::strcpy_s(m_szCharset, ATL_MAX_ENC_CHARSET_LENGTH, ATLSMTP_DEFAULT_CSET);
	}

	virtual ~CMimeText() throw()
	{
	}

	// Get the content type
	virtual inline LPCSTR GetContentType() throw()
	{
		return "text/plain";
	}

	// Get the character set
	virtual inline LPCSTR GetCharset() throw()
	{
		return m_szCharset;
	}

	virtual ATL_NOINLINE CMimeBodyPart* Copy() throw( ... )
	{
		CAutoPtr<CMimeText> pNewText;
		ATLTRY(pNewText.Attach(new CMimeText));
		if (pNewText)
			*pNewText = *this;

		return pNewText.Detach();
	}

	const CMimeText& operator=(const CMimeText& that) throw( ... )
	{
		if (this != &that)
		{
			m_nTextLen = that.m_nTextLen;
			Checked::strcpy_s(m_szCharset, ATL_MAX_ENC_CHARSET_LENGTH, that.m_szCharset);
			m_szText.Free();
			if (m_szText.AllocateBytes(m_nTextLen) != false)
			{
				Checked::memcpy_s((char *)m_szText, m_nTextLen, (char *)that.m_szText, m_nTextLen);
			}
		}

		return *this;
	}

	// Initialize the body part
	// szText - the text (required)
	// nTextLen - the text length in bytes (optional--if not specified a _tcslen will be done)
	// pMultiLanguage - the IMultiLanguagte pointer for converting codepages to MIME character sets (optional)
	// uiCodePage - the codepage
	inline BOOL Initialize(LPCTSTR szText, int nTextLen = -1, IMultiLanguage* pMultiLanguage = NULL, UINT uiCodePage = 0) throw()
	{
		BOOL bRet = TRUE;

		// if IMultiLanguage is there, respect the codepage
		if (pMultiLanguage)
		{
			CHeapPtr<char> szTextPtr;
			UINT nLen(0);

			bRet = AtlMimeConvertString(pMultiLanguage, uiCodePage, szText, &szTextPtr, &nLen);
			if (bRet)
			{
				m_szText.Free();
				m_szText.Attach(szTextPtr.Detach());
				m_nTextLen = nLen;
			}
		}
		else // no multilanguage support
		{
			if (nTextLen < 0)
			{
				nTextLen = (int) _tcslen(szText);
				nTextLen*= sizeof(TCHAR);
			}

			m_szText.Free();
			if (m_szText.AllocateBytes(nTextLen) != false)
			{
				Checked::memcpy_s((char *)m_szText, nTextLen, szText, nTextLen);
				m_nTextLen = nTextLen;
			}
		}

		if (bRet)
		{
			bRet = AtlMimeCharsetFromCodePage(m_szCharset, uiCodePage, pMultiLanguage, ATL_MAX_ENC_CHARSET_LENGTH);
		}

		return bRet;
	}

	// Dump the data to hFile
	virtual inline BOOL WriteData(HANDLE hFile, LPOVERLAPPED pOverlapped, LPCSTR szBoundary, DWORD dwFlags = 0) throw()
	{
		if ((pOverlapped == NULL) || (szBoundary == NULL))
		{
			return FALSE;
		}

		CStringA strHeader;
		char sendBuffer[ATLSMTP_READBUFFER_SIZE];
		LPSTR pSendBuffer = sendBuffer;
		LPSTR szText = m_szText;

		if (!MakeMimeHeader(strHeader, szBoundary))
		{
			return FALSE;
		}

		//copy the header into the sendbuffer
		int nWritten = strHeader.GetLength();
		if(nWritten > ATLSMTP_READBUFFER_SIZE)
			return FALSE;
			
		Checked::memcpy_s(pSendBuffer, ATLSMTP_READBUFFER_SIZE, (LPCSTR)strHeader, nWritten);
		pSendBuffer+= nWritten;
		int nRead = 0;
		int nLineLen = 0;

		//subtract 2 from these because it's easier for when we have
		//to break lines with a CRLF
		int nMaxLineLength = ATLSMTP_MAX_LINE_LENGTH-2;
		int nMaxBufferSize = ATLSMTP_READBUFFER_SIZE-2;
		while (nRead <= m_nTextLen)
		{
			//if the buffer is full or we've reached the end of the text, 
			//send it
			if (nWritten >= nMaxBufferSize || nRead == m_nTextLen)
			{
				if (!AtlSmtpSendAndWait(hFile, sendBuffer, nWritten, pOverlapped))
					return FALSE;
				nWritten = 0;
				pSendBuffer = sendBuffer;
				if (nRead == m_nTextLen)
				{
					break; // job done, no need to run the code below
				}
			}

			//if we're at the end of the line, break it
			if (nLineLen == nMaxLineLength)
			{
				if(nWritten + 2 > ATLSMTP_READBUFFER_SIZE)
					return FALSE;
				*pSendBuffer++ = '\r';
				*pSendBuffer++ = '\n';
				nWritten+= 2;
				nLineLen = -1;
				continue;
			}

			//stuff dots at the start of the line
			if (nLineLen == 0 && (dwFlags & ATLSMTP_FORMAT_SMTP) && *szText == '.')
			{
				if(nWritten + 1 > ATLSMTP_READBUFFER_SIZE)
					return FALSE;
				*pSendBuffer++ = '.';
				nWritten++;
				nLineLen++;
				continue;
			}

			//if we hit a CRLF, reset nLineLen
			if (*szText == '\n' && nRead > 0 && *(szText-1) == '\r')
				nLineLen = -1;

			if(nWritten + 1 > ATLSMTP_READBUFFER_SIZE)
				return FALSE;
			*pSendBuffer++ = (*szText++);
			nRead++;
			nWritten++;
			nLineLen++;
		}

		return TRUE;
	}

protected:

	// Make the MIME header
	virtual inline BOOL MakeMimeHeader(CStringA& header, LPCSTR szBoundary) throw()
	{
		char szBegin[256];
		if (*szBoundary)
		{
			// this is not the only body part
			Checked::memcpy_s(szBegin, sizeof(szBegin), ATLMIME_SEPARATOR, sizeof(ATLMIME_SEPARATOR));
			Checked::memcpy_s(szBegin+6, sizeof(szBegin)-6, szBoundary, ATL_MIME_BOUNDARYLEN);
			*(szBegin+(ATL_MIME_BOUNDARYLEN+6)) = '\0';
		}
		else
		{
			// this is the only body part, so output the full MIME header
			Checked::memcpy_s(szBegin, sizeof(szBegin), ATLMIME_VERSION, sizeof(ATLMIME_VERSION));
		}

		_ATLTRY
		{
			header.Format("%s\r\nContent-Type: text/plain;\r\n\tcharset=\"%s\"\r\nContent-Transfer-Encoding: 8bit\r\n\r\n", 
				szBegin, m_szCharset);
			return TRUE;
		}
		_ATLCATCHALL()
		{
			return FALSE;
		}
	}
}; // class CMimeText


// CMimeMessage - the MIME message class.  Represents a full MIME message
class CMimeMessage : public CMimeHeader
{
protected:

	// The list of the MIME body parts
	CAutoPtrList<CMimeBodyPart> m_BodyParts;

	// The display name of the message
	char m_szDisplayName[MAX_PATH+1];

public:
	CMimeMessage(IMultiLanguage *pMultiLanguage = NULL) throw()
	{
		Initialize(pMultiLanguage);
		Checked::memcpy_s(m_szDisplayName, MAX_PATH+1, ATLMIME_EMAIL, sizeof(ATLMIME_EMAIL));
	}

	virtual ~CMimeMessage() throw()
	{
		RemoveParts();
	}

	void RemoveParts() throw()
	{
		m_BodyParts.RemoveAll();
	}


	virtual ATL_NOINLINE CMimeBodyPart* Copy() throw( ... )
	{
		CAutoPtr<CMimeMessage> pNewMessage;
		ATLTRY(pNewMessage.Attach(new CMimeMessage));
		if (pNewMessage)
			*pNewMessage = *this;

		return pNewMessage.Detach();
	}


	const CMimeMessage& operator=(const CMimeMessage& that) throw( ... )
	{
		if (this != &that)
		{
			CMimeHeader::operator=(that);
			Checked::strcpy_s(m_szDisplayName, MAX_PATH+1, that.m_szDisplayName);

			RemoveParts();
			POSITION pos = that.m_BodyParts.GetHeadPosition();
			while (pos != NULL)
			{
				CAutoPtr<CMimeBodyPart> pCopy(that.m_BodyParts.GetNext(pos)->Copy());
				if (pCopy)
				{
					m_BodyParts.AddTail(pCopy);
				}
			}
		}

		return *this;
	}

	// Set the display name of the message
	inline BOOL SetDisplayName(LPCTSTR szDisplayName) throw()
	{
		if (szDisplayName == NULL)
		{
			return FALSE;
		}

		_ATLTRY
		{
			CT2CA szDisplayNameA(szDisplayName);
		 	if (szDisplayNameA == NULL || strlen(szDisplayNameA) > MAX_PATH)
		 		return FALSE;
			Checked::strcpy_s(m_szDisplayName, MAX_PATH+1, szDisplayNameA);
			return TRUE;
		}
		_ATLCATCHALL()
		{
			return FALSE;
		}
	}

	// Add some text to the message at position nPos in the body parts list
	// szText - the text
	// nTextLen - the size of the text in bytes (optional - if not specified a _tcslen will be done)
	// nPos - the position in the message at which to insert the text (optional)
	// uiCodePage - the codepage (optional)
	inline BOOL AddText(LPCTSTR szText, int nTextLen = -1, int nPos = 1, UINT uiCodePage = 0) throw()
	{
		if (szText == NULL)
			return FALSE;

		if (nPos < 1)
		{
			nPos = 1;
		}

		CAutoPtr<CMimeBodyPart> spNewText;
		CMimeText *pNewText = NULL;
		ATLTRY(spNewText.Attach(pNewText = new CMimeText()));
		if (!spNewText || !pNewText)
			return FALSE;

		BOOL bRet = pNewText->Initialize(szText, nTextLen, m_spMultiLanguage, uiCodePage);
		if (bRet)
		{
			_ATLTRY
			{
				POSITION currPos = m_BodyParts.FindIndex(nPos-1);

					if (!currPos)
					{
						if (!m_BodyParts.AddTail(spNewText))
							bRet = FALSE;
					}
					else
					{
						if (!m_BodyParts.InsertBefore(currPos, spNewText))
							bRet = FALSE;
					}

			}
			_ATLCATCHALL()
			{
				bRet = FALSE;
			}
		}

		return bRet;
	}

	// Dump the data
	virtual BOOL WriteData(HANDLE hFile, LPOVERLAPPED pOverlapped, LPCSTR szBoundary=NULL, DWORD dwFlags = 0) throw()
	{	
		if (pOverlapped == NULL)
		{
			return FALSE;
		}

		// Make the MIME boundary for this message
		char szBoundaryBuf[ATL_MIME_BOUNDARYLEN+1];
		if(MakeBoundary(szBoundaryBuf,ATL_MIME_BOUNDARYLEN+1) == FALSE)
			return FALSE;

		// if the passed boundary is valid, this is an attached message
		if (szBoundary && *szBoundary != '\0')
		{
			_ATLTRY
			{
				// output the MIME header for a message attachment
				CStringA strHeader;
				strHeader.Format("\r\n\r\n--%s\r\nContent-Type: message/rfc822\r\n\tname=\"%s\"\r\nContent-Transfer-Encoding: 8bit\r\n"
					"Content-Disposition: attachment;\r\n\tfilename=\"%s\"\r\n\r\n", 
					szBoundary, m_szDisplayName, m_szDisplayName);

				if (!AtlSmtpSendAndWait(hFile, ((LPCSTR)strHeader), strHeader.GetLength(), pOverlapped))
				{
					return FALSE;
				}
			}
			_ATLCATCHALL()
			{
				return FALSE;
			}
		}

		if (!CMimeHeader::WriteData(hFile, pOverlapped, szBoundaryBuf, dwFlags))
			return FALSE;

		// Create and output the header
		CStringA strHeader;

		if (!MakeMimeHeader(strHeader, szBoundaryBuf))
		{
			return FALSE;
		}

		if (!AtlSmtpSendAndWait(hFile, ((LPCSTR)strHeader), strHeader.GetLength(), pOverlapped))
		{
			return FALSE;
		}

		CMimeBodyPart* pCurrPart;
		POSITION currPos = m_BodyParts.GetHeadPosition();

		//Dump the body parts
		while (currPos != NULL)
		{
			pCurrPart = m_BodyParts.GetAt(currPos);
			if (!pCurrPart->WriteData(hFile, pOverlapped, szBoundaryBuf, dwFlags))
			{
				return FALSE;
			}
			m_BodyParts.GetNext(currPos);
		}

		char szBuf[ATL_MIME_BOUNDARYLEN+(sizeof("\r\n\r\n--%s--\r\n"))];
		//output a trailing boundary
		if (*szBoundaryBuf)
		{
			int nBufLen = sprintf_s(szBuf, ATL_MIME_BOUNDARYLEN+(sizeof("\r\n\r\n--%s--\r\n")),
				"\r\n\r\n--%s--\r\n", szBoundaryBuf);
			if ((nBufLen < 0) || (!AtlSmtpSendAndWait(hFile, szBuf, nBufLen, pOverlapped)))
			{
				return FALSE;
			}
		}

		return TRUE;
	}

	// Attach a file.
	// szFileName - the filename
	// szDisplayName - the display name (optional)
	// szContentType - the content type (optional - defaults to NULL -- lookup will be attempted, otherwise default to application/octet-stream)
	// nEncodingScheme - the encoding scheme to use for the attachment (optional - defaults to base64
	// uiCodePage - the codepage (optional)
	inline BOOL AttachFile(LPCTSTR szFileName, LPCTSTR szDisplayName = NULL, LPCTSTR szContentType = NULL, 
		int nEncodingScheme = ATLSMTP_BASE64_ENCODE, UINT uiCodepage = 0)
	{
		if (szFileName == NULL)
			return FALSE;

		CAutoPtr<CMimeBodyPart> spFileAttach;
		CMimeFileAttachment* pFileAttach = NULL;
		ATLTRY(spFileAttach.Attach(pFileAttach = new CMimeFileAttachment()));
		if (!spFileAttach || !pFileAttach)
			return FALSE;

		BOOL bRet = pFileAttach->Initialize(szFileName, szDisplayName, m_spMultiLanguage, uiCodepage);

		if (bRet)
			bRet = pFileAttach->SetEncodingScheme(nEncodingScheme);

		CString strContentType;
		if (bRet && (szContentType == NULL))
		{
			if (GetContentTypeFromFileName(szFileName, strContentType) != ERROR_OUTOFMEMORY)
			{
				szContentType = strContentType;
			}
			else
			{
				bRet = FALSE;
			}
		}

		_ATLTRY
		{
			if (bRet)
			{
				bRet = pFileAttach->SetContentType(szContentType);
				if (bRet)
				{
					if (!m_BodyParts.AddTail(spFileAttach))
					{
						bRet = FALSE;
					}
				}
			}
		}
		_ATLCATCHALL()
		{
			bRet = FALSE;
		}

		return bRet;
	}

	// Attach some raw data
	// pRawData - the data
	// nDataLength - the size of the data in bytes
	// nEncodingScheme - the encoding scheme to use for the attachment (optional - defaults to base64
	// uiCodePage - the codepage (optional)
	inline BOOL AttachRaw(void* pRawData, DWORD dwDataLength, int nEncodingScheme = ATLSMTP_BASE64_ENCODE, BOOL bCopyData = TRUE, 
		LPCTSTR szDisplayName = NULL, LPCTSTR szContentType = _T("application/octet-stream"), UINT uiCodepage = 0)
	{
		if (!pRawData)
			return FALSE;

		CAutoPtr<CMimeBodyPart> spRawAttach;
		CMimeRawAttachment* pRawAttach;
		ATLTRY(spRawAttach.Attach(pRawAttach = new CMimeRawAttachment()));
		if (!spRawAttach)
		{
			return FALSE;
		}

		BOOL bRet = pRawAttach->Initialize(pRawData, dwDataLength, bCopyData, szDisplayName, m_spMultiLanguage, uiCodepage);

		if (bRet)
			bRet = pRawAttach->SetEncodingScheme(nEncodingScheme);
		if (bRet)
			bRet = pRawAttach->SetContentType(szContentType);

		_ATLTRY
		{
		if (bRet)
			if(!m_BodyParts.AddTail(spRawAttach))
				bRet = FALSE;
		}
		_ATLCATCHALL()
		{
			bRet = FALSE;
		}

		return bRet;
	}

	// Attach a CMimeMessage
	// pMsg - pointer to the Msg object
	inline BOOL AttachMessage(CMimeMessage* pMsg) throw( ... )
	{
		if (!pMsg)
			return FALSE;

		_ATLTRY
		{
			CAutoPtr<CMimeBodyPart> spMsg(pMsg->Copy());
			if (!m_BodyParts.AddTail(spMsg))
				return FALSE;
		}
		_ATLCATCHALL()
		{
			return FALSE;
		}

		return TRUE;
	}

protected:
	// Make the MIME header
	virtual inline BOOL MakeMimeHeader(CStringA& header, LPCSTR szBoundary) throw()
	{
		_ATLTRY
		{
			if (!*szBoundary)
			{
				header.Format("X-Priority: %d\r\n%s", m_nPriority, (LPCSTR) m_XHeader);
			}
			else if (m_BodyParts.GetCount() > 1)
			{
				header.Format("X-Priority: %d\r\n%sMIME-Version: 1.0\r\nContent-Type: multipart/mixed;\r\n\tboundary=\"%s\"\r\n", 
					m_nPriority, (LPCSTR) m_XHeader, szBoundary);
			}
			return TRUE;
		}
		_ATLCATCHALL()
		{
			return FALSE;
		}
	}

	// Make the MIME boundary
	inline BOOL MakeBoundary(__out_ecount_z(nBufLen) LPSTR szBoundary, __in int nBufLen)
	{
		ATLENSURE(szBoundary != NULL);
			
		if(nBufLen < 1)
		{
			return FALSE;
		}

		if (m_BodyParts.GetCount() < 2)
		{
			*szBoundary = '\0';
		}
		else 
		{
			int ret = sprintf_s(szBoundary, nBufLen, "------=_Next_Part_%.10u.%.3u", GetTickCount(), rand()%1000); 
			if (ret == -1 || ret >= nBufLen)
				return FALSE;
		}
		return TRUE;
	}

}; // class CMimeMessage

} // namespace ATL
#pragma pack(pop)

#ifndef _CPPUNWIND
#pragma warning (pop)
#endif //_CPPUNWIND

#pragma warning(pop)

#endif // __ATLMIME_H__
