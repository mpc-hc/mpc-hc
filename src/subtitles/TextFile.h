/* 
 *	Copyright (C) 2003-2006 Gabest
 *	http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once

#include <afx.h>

class CTextFile : protected CStdioFile
{
public:
	typedef enum {ASCII, UTF8, LE16, BE16, ANSI} enc;

private:
	enc m_encoding, m_defaultencoding;
	int m_offset;

public:
	CTextFile(enc e = ASCII);

	virtual bool Open(LPCTSTR lpszFileName);
	virtual bool Save(LPCTSTR lpszFileName, enc e /*= ASCII*/);

	void SetEncoding(enc e);
	enc GetEncoding();
	bool IsUnicode();

	// CFile

	CString GetFilePath() const;

	// CStdioFile

    ULONGLONG GetPosition() const;
	ULONGLONG GetLength() const;
	ULONGLONG Seek(LONGLONG lOff, UINT nFrom);

	void WriteString(LPCSTR lpsz/*CStringA str*/);
	void WriteString(LPCWSTR lpsz/*CStringW str*/);
	BOOL ReadString(CStringA& str);
	BOOL ReadString(CStringW& str);
};

class CWebTextFile : public CTextFile
{
	LONGLONG m_llMaxSize;
	CString m_tempfn;

public:
	CWebTextFile(LONGLONG llMaxSize = 1024*1024);

	bool Open(LPCTSTR lpszFileName);
	bool Save(LPCTSTR lpszFileName, enc e /*= ASCII*/);
	void Close();
};

extern CStringW AToW(CStringA str);
extern CStringA WToA(CStringW str);
extern CString AToT(CStringA str);
extern CString WToT(CStringW str);
extern CStringA TToA(CString str);
extern CStringW TToW(CString str);
