/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2012 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
		enum encoding_t {none, unknown, utf8, utf16le, utf16be, wchar};

	protected:
		int m_line, m_col;
		encoding_t m_encoding;

	public:
		Stream();
		virtual ~Stream();

		static bool IsWhiteSpace(int c, LPCWSTR morechars = NULL);

		void ThrowError(LPCTSTR fmt, ...);
	};

	class InputStream : public Stream
	{
		CAtlList<int> m_queue;
		int PushChar(), PopChar();

		int NextChar();

	protected:
		virtual int NextByte() = 0;

	public:
		InputStream();
		~InputStream();

		int PeekChar(), GetChar();

		int SkipWhiteSpace(LPCWSTR morechars = NULL);
	};

	class FileInputStream : public InputStream
	{
		FILE* m_file;

	protected:
		int NextByte();

	public:
		FileInputStream(const TCHAR* fn);
		~FileInputStream();
	};

	class MemoryInputStream : public InputStream
	{
		BYTE* m_pBytes;
		int m_pos, m_len;
		bool m_fFree;

	protected:
		int NextByte();

	public:
		MemoryInputStream(BYTE* pBytes, int len, bool fCopy, bool fFree);
		~MemoryInputStream();
	};

	class WCharInputStream : public InputStream
	{
		CStringW m_str;
		int m_pos;

	protected:
		int NextByte();

	public:
		WCharInputStream(CStringW str);
	};

	class OutputStream : public Stream
	{
		bool m_bof;

	protected:
		virtual void NextByte(int b) = 0;

	public:
		OutputStream(encoding_t e);
		virtual ~OutputStream();

		void PutChar(WCHAR c);
		void PutString(LPCWSTR fmt, ...);
	};

	class WCharOutputStream : public OutputStream
	{
		CStringW m_str;

	protected:
		void NextByte(int b);

	public:
		WCharOutputStream();

		const CStringW& GetString() {
			return m_str;
		}
	};

	class DebugOutputStream : public OutputStream
	{
		CStringW m_str;

	protected:
		void NextByte(int b);

	public:
		DebugOutputStream();
		~DebugOutputStream();
	};
}