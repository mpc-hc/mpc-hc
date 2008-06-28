// multisz.cpp: implementation of the CMultiSz class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "sniffusb.h"
#include "multisz.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMultiSz::CMultiSz()
{
	m_MultiSzBuf = NULL;
	m_MultiSzLen = 0;
	m_MultiSzPos = 0;
}

CMultiSz::CMultiSz(const CMultiSz& other)
{
	*this = other;
}

CMultiSz::~CMultiSz()
{
	if (m_MultiSzBuf != NULL)
		free (m_MultiSzBuf);
}

BOOL CMultiSz::SetBuffer(BYTE *buf, DWORD len)
{
	BYTE * newBuf;

	// check the content of "buf"

	if (len < 0)
	{
		TRACE("Error : len < 0 [len = %d], using len = 0\n",len);
		len = 0;
	}
	else if (len != 0 && len < 2)
	{
		TRACE("Error : len = %d, using len = 0\n",len);
		len = 0;
	}
	else if (len >= 2)
	{
		// check the content of buf

		int i, i_zero = -1;
		char c, lastc = buf[0];

		// i_zero is the index of the last null character or -1

		for (i=1;i<len;i++)
		{
			c = buf[i];

			if (c == 0)
				i_zero = i;

			if (c == 0 && lastc == 0 && i+1 != len)
			{
				TRACE("Error: len = %d, reallen = %d, using len = %d\n",len,i+1,i+1);
				len = i+1;
				break;
			}

			lastc = c;
		}

		if (buf[len-1] != 0 || buf[len-2] != 0)
		{
			if (i_zero < 0)
				i_zero = 0;

			TRACE("Error: missing two 0 at end of buffer, using len = %d\n",i_zero+2);

			buf[i_zero    ] = 0;
			buf[i_zero + 1] = 0;

			len = i_zero + 2;
		}
	}


	newBuf = (BYTE *) realloc(m_MultiSzBuf, len);
	if (newBuf == NULL)
		return FALSE;

	m_MultiSzBuf = newBuf;
	m_MultiSzLen = len;

	if (buf != NULL && len > 0)
		memcpy(m_MultiSzBuf, buf, m_MultiSzLen);

	return TRUE;
}

void CMultiSz::FirstPosition(int * position) const
{
	* position = 0;
}

void CMultiSz::First()
{
	FirstPosition(&m_MultiSzPos);
}

const char * CMultiSz::NextPosition(int * position) const
{
	const char * str;

	// check if the buffer is empty

	if (m_MultiSzLen < 1)
		return NULL;

	// check if we are already at the end of the list

	if (m_MultiSzBuf[*position] == 0)
		return NULL;

	// get the current string

	str = (const char *) m_MultiSzBuf + *position;

	// compute the starting byte position of the next string

	*position += strlen(str) + 1;

	return str;
}

const char * CMultiSz::Next()
{
	return NextPosition(&m_MultiSzPos);
}

BOOL CMultiSz::AddString(const char *str)
{
	// check if the string is already present

	if (FindString(str))
		return TRUE;

	// if the buffer was empty, add another byte
	if (m_MultiSzLen == 0)
		m_MultiSzLen ++;

	// if not present, add the new string to the end

	DWORD newMultiSzLen = m_MultiSzLen + strlen(str) + 1;
	BYTE * newMultiSzBuf = (BYTE *) realloc(m_MultiSzBuf, newMultiSzLen);
	if (newMultiSzBuf == NULL)
		return FALSE;

	strcpy((char *)newMultiSzBuf + m_MultiSzLen - 1, str);
	newMultiSzBuf[newMultiSzLen - 1] = 0;

	m_MultiSzLen = newMultiSzLen;
	m_MultiSzBuf = newMultiSzBuf;

	return TRUE;
}

BOOL CMultiSz::RemoveString(const char *str)
{
	const char * s;
	int curPos = 0;

	// check if the string is already present

	First();

	while ((s = Next()) != NULL)
	{
		if (strcmp(str,s) == 0)
		{
			// m_MultiSzPos is initialized to point to the
			// beginning of the next string

			memcpy(m_MultiSzBuf + curPos , m_MultiSzBuf + m_MultiSzPos , m_MultiSzLen - m_MultiSzPos);

			m_MultiSzLen -= m_MultiSzPos - curPos;

			return TRUE;
		}

		// save the position of the beginning of the next
		// string

		curPos = m_MultiSzPos;
	}

	// if not, returns TRUE
	return TRUE;
}

const CMultiSz& CMultiSz::operator = (const CMultiSz& other)
{
	m_MultiSzBuf = NULL;
	if (other.m_MultiSzLen > 0)
	{
		m_MultiSzBuf = (BYTE *) realloc(m_MultiSzBuf, other.m_MultiSzLen);
		if (m_MultiSzBuf != NULL)
			memcpy(m_MultiSzBuf, other.m_MultiSzBuf, other.m_MultiSzLen);
	}

	m_MultiSzLen = other.m_MultiSzLen;
	m_MultiSzPos = other.m_MultiSzPos;

	return *this;
}

BOOL CMultiSz::FindString(const char *str) const
{
	const char * s;
	int position;

	// check if the string is already present

	FirstPosition(&position);
	while ((s = NextPosition(&position)) != NULL)
	{
		if (strcmp(str,s) == 0)
			return TRUE;
	}

	return FALSE;
}

void CMultiSz::DisplayBuffer()
{
	char msg [ 1000 ];
	char ns [ 10 ];
	int i;

	msg [0] = 0;
	for (i=0;i<m_MultiSzLen; i++)
	{
		if (strlen(msg) > sizeof(msg) - 10)
		{
			MessageBox(NULL,_T("Truncating buffer"),_T("DisplayBuffer"),MB_OK);
			break;
		}

		char c = m_MultiSzBuf[i];
		if (c < ' ' || c > 0x7f)
			c = ' ';

		sprintf(ns, "%02x/%c ",m_MultiSzBuf [i], c);
		strcat(msg,ns);
	}

	MessageBox(NULL,msg,_T("DisplayBuffer"),MB_OK);
}

