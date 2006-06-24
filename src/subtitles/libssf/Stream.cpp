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

#include "stdafx.h"
#include "Stream.h"

namespace ssf
{
	Stream::Stream()
		: m_encoding(none)
		, m_line(0)
		, m_col(-1)
	{
		
	}

	Stream::~Stream()
	{
	}

	int Stream::NextChar()
	{
		if(m_encoding == none)
		{
			m_encoding = unknown;

			switch(NextByte())
			{
			case 0xef: 
				if(NextByte() == 0xbb && NextByte() == 0xbf) m_encoding = utf8;
				break;
			case 0xff: 
				if(NextByte() == 0xfe) m_encoding = utf16le;
				break;
			case 0xfe:
				if(NextByte() == 0xff) m_encoding = utf16be;
				break;
			}
		}

		if(m_encoding == unknown)
		{
			throw Exception(_T("unknown character encoding, missing BOM"));
		}

		int i, c;

		int cur = NextByte();

		switch(m_encoding)
		{
		case utf8: 
			for(i = 7; i >= 0 && (cur & (1 << i)); i--);
			cur &= (1 << i) - 1;
			while(++i < 7) {c = NextByte(); if(c == EOS) {cur = EOS; break;} cur = (cur << 6) | (c & 0x3f);}
			break;
		case utf16le: 
			c = NextByte();
			if(c == EOS) {cur = EOS; break;}
			cur = (c << 8) | cur;
			break;
		case utf16be: 
			c = NextByte();
			if(c == EOS) {cur = EOS; break;}
			cur = cur | (c << 8);
			break;
		case tchar:
			break;
		}

		return cur;
	}

	int Stream::PushChar()
	{
		int c = NextChar();
		m_queue.AddTail(c);
		return c;
	}

	int Stream::PopChar()
	{
		if(m_queue.IsEmpty()) ThrowError(_T("fatal stream error"));

		int c = m_queue.RemoveHead();

		if(c != EOS)
		{
			if(c == '\n') {m_line++; m_col = -1;}
			m_col++;
		}

		return c;
	}

	int Stream::PeekChar()
	{
		while(m_queue.GetCount() < 2) PushChar();

		ASSERT(m_queue.GetCount() == 2);

		if(m_queue.GetHead() == '/' && m_queue.GetTail() == '/')
		{
			while(!m_queue.IsEmpty()) PopChar();
			int c;
			do {PushChar(); c = PopChar();} while(!(c == '\n' || c == EOS));
			return PeekChar();
		}
		else if(m_queue.GetHead() == '/' && m_queue.GetTail() == '*')
		{
			while(!m_queue.IsEmpty()) PopChar();
			int c1, c2;
			PushChar();
			do {c2 = PushChar(); c1 = PopChar();} while(!((c1 == '*' && c2 == '/') || c1 == EOS));
			PopChar();
			return PeekChar();
		}

		return m_queue.GetHead();
	}

	int Stream::GetChar()
	{
		if(m_queue.GetCount() < 2) PeekChar();
		return PopChar();
	}

	bool Stream::IsWhiteSpace(int c, LPCTSTR morechars)
	{
		return c != 0xa0 && _istspace(c) || morechars && _tcschr(morechars, (TCHAR)c);
	}

	int Stream::SkipWhiteSpace(LPCTSTR morechars)
	{
		int c = PeekChar();
		for(; IsWhiteSpace(c, morechars); c = PeekChar()) 
			GetChar();
		return c;
	}

	void Stream::ThrowError(LPCTSTR fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		int len = _vsctprintf(fmt, args) + 1;
		CString str;
		if(len > 0) _vstprintf_s(str.GetBufferSetLength(len), len, fmt, args);
		va_end(args);

		throw Exception(_T("Error (Ln %d Col %d): %s"), m_line+1, m_col+1, str);
	}

	// FileStream

	FileStream::FileStream(const TCHAR* fn) 
		: m_file(NULL)
	{
		if(_tfopen_s(&m_file, fn, _T("r")) != 0) ThrowError(_T("cannot open file '%s'"), fn);
	}

	FileStream::~FileStream()
	{
		if(m_file) {fclose(m_file); m_file = NULL;}
	}

	int FileStream::NextByte()
	{
		if(!m_file) ThrowError(_T("file pointer is NULL"));
		return fgetc(m_file);
	}

	// MemoryStream

	MemoryStream::MemoryStream(BYTE* pBytes, int len, bool fCopy, bool fFree)
		: m_pBytes(NULL)
		, m_pos(0)
		, m_len(len)
	{
		if(fCopy)
		{
			m_pBytes = new BYTE[len];
			if(m_pBytes) memcpy(m_pBytes, pBytes, len);
			m_fFree = true;
		}
		else
		{
			m_pBytes = pBytes;
			m_fFree = fFree;
		}

		if(!m_pBytes) ThrowError(_T("memory stream pointer is NULL"));
	}

	MemoryStream::~MemoryStream()
	{
		if(m_fFree) delete [] m_pBytes;
		m_pBytes = NULL;
	}

	int MemoryStream::NextByte()
	{
		if(!m_pBytes) ThrowError(_T("memory stream pointer is NULL"));
		if(m_pos >= m_len) return Stream::EOS;
		return (int)m_pBytes[m_pos++];
	}

	// CharacterStream
	
	CharacterStream::CharacterStream(CString str)
		: m_str(str)
		, m_pos(0)
	{
		m_encoding = Stream::tchar;
	}

	int CharacterStream::NextByte()
	{
		if(m_pos >= m_str.GetLength()) return Stream::EOS;
		return m_str[m_pos++];
	}
}