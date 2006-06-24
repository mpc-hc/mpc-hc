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

#include "Exception.h"

namespace ssf
{
	class Stream
	{
	public:
		enum {EOS = -1};
		enum {none, unknown, utf8, utf16le, utf16be, tchar} m_encoding;

	private:
		int m_line, m_col;

		CAtlList<int> m_queue;
		int PushChar(), PopChar();

		int NextChar();

	protected:
		virtual int NextByte() = 0;

	public:
		Stream();
		virtual ~Stream();

		int PeekChar(), GetChar();

		static bool IsWhiteSpace(int c, LPCTSTR morechars = NULL);
		int SkipWhiteSpace(LPCTSTR morechars = NULL);

		void ThrowError(LPCTSTR fmt, ...);
	};

	class FileStream : public Stream
	{
		FILE* m_file;

	protected:
		int NextByte();

	public:
		FileStream(const TCHAR* fn);
		~FileStream();
	};

	class MemoryStream : public Stream
	{
		BYTE* m_pBytes;
		int m_pos, m_len;
		bool m_fFree;

	protected:
		int NextByte();

	public:
		MemoryStream(BYTE* pBytes, int len, bool fCopy, bool fFree);
		~MemoryStream();
	};

	class CharacterStream : public Stream
	{
		CString m_str;
		int m_pos;

	protected:
		int NextByte();

	public:
		CharacterStream(CString str);
	};
}