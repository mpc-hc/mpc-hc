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


class CJpegEncoder
{
	static const int ColorComponents = 3;

	int m_w, m_h;
	BYTE* m_p;

	unsigned int m_bbuff, m_bwidth;
	bool PutBit(int b, int n);
	void Flush();
	int GetBitWidth(short q);

	void WriteSOI();
	void WriteDQT();
	void WriteSOF0();
	void WriteDHT();
	void WriteSOS();
	void WriteEOI();

protected:
	virtual bool PutByte(BYTE b) = 0;
	virtual bool PutBytes(const void* pData, size_t len) = 0;
	virtual bool Encode(const BYTE* dib);

public:
	CJpegEncoder();
};

class CJpegEncoderFile : public CJpegEncoder
{
	CString m_fn;
	FILE* m_file;

protected:
	bool PutByte(BYTE b);
	bool PutBytes(const void* pData, size_t len);

public:
	CJpegEncoderFile(LPCTSTR fn);

	bool Encode(const BYTE* dib);
};

class CJpegEncoderMem : public CJpegEncoder
{
	CAtlArray<BYTE>* m_pdata;

protected:
	bool PutByte(BYTE b);
	bool PutBytes(const void* pData, size_t len);

public:
	CJpegEncoderMem();

	bool Encode(const BYTE* dib, CAtlArray<BYTE>& data);
};
