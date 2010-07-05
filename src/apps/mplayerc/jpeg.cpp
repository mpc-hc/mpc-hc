/*
 * $Id$
 *
 * (C) 2003-2006 Gabest
 * (C) 2006-2010 see AUTHORS
 *
 * This file is part of mplayerc.
 *
 * Mplayerc is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mplayerc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"
#include <math.h>
#include "jpeg.h"
#include "jpeg_tables.h"


bool CJpegEncoder::PutBit(int b, int n)
{
	if(n > 24 || n <= 0) return(false);

	m_bbuff <<= n;
	m_bbuff |= b & ((1 << n) - 1);
	m_bwidth += n;

	while(m_bwidth >= 8)
	{
		BYTE c = (BYTE)(m_bbuff >> (m_bwidth - 8));
		PutByte(c);
		if(c == 0xff) PutByte(0);
		m_bwidth -= 8;
	}

	return(true);
}

void CJpegEncoder::Flush()
{
	if(m_bwidth > 0)
	{
		BYTE c = m_bbuff << (8 - m_bwidth);
		PutByte(c);
		if(c == 0xff) PutByte(0);
	}

	m_bbuff = m_bwidth = 0;
}

///////

int CJpegEncoder::GetBitWidth(short q)
{
	if(q == 0) return(0);
	if(q < 0) q = -q;

	int width = 15;
	for(; !(q&0x4000); q <<= 1, width--);
	return(width);
}

///////

void CJpegEncoder::WriteSOI()
{
	PutByte(0xff);
	PutByte(0xd8);
}

void CJpegEncoder::WriteDQT()
{
	PutByte(0xff);
	PutByte(0xdb);

	WORD size = 2 + 2*(65 + 64*0);
	PutByte(size>>8);
	PutByte(size&0xff);

	for(int c = 0; c < 2; c++)
	{
		PutByte(c);
		PutBytes(quanttbl[c], 64);
	}
}

void CJpegEncoder::WriteSOF0()
{
	PutByte(0xff);
	PutByte(0xc0);

	WORD size = 8 + 3*ColorComponents;
	PutByte(size>>8);
	PutByte(size&0xff);

	PutByte(8); // precision

	PutByte(m_h>>8);
	PutByte(m_h&0xff);
	PutByte(m_w>>8);
	PutByte(m_w&0xff);

	PutByte(ColorComponents); // color components

	PutByte(1); // component id
	PutByte(0x11); // hor | ver sampling factor
	PutByte(0); // quant. tbl. id

	PutByte(2); // component id
	PutByte(0x11); // hor | ver sampling factor
	PutByte(1); // quant. tbl. id

	PutByte(3); // component id
	PutByte(0x11); // hor | ver sampling factor
	PutByte(1); // quant. tbl. id
}

void CJpegEncoder::WriteDHT()
{
	PutByte(0xff);
	PutByte(0xc4);

	WORD size = 0x01A2; // 2 + n*(17+mi);
	PutByte(size>>8);
	PutByte(size&0xff);

	PutByte(0x00); // tbl class (DC) | tbl id
	PutBytes(DCVLC_NumByLength[0], 16);
	for(int i = 0; i < 12; i++) PutByte(i);

	PutByte(0x01); // tbl class (DC) | tbl id
	PutBytes(DCVLC_NumByLength[1], 16);
	for(int i = 0; i < 12; i++) PutByte(i);

	PutByte(0x10); // tbl class (AC) | tbl id
	PutBytes(ACVLC_NumByLength[0], 16);
	PutBytes(ACVLC_Data[0], sizeof(ACVLC_Data[0]));

	PutByte(0x11); // tbl class (AC) | tbl id
	PutBytes(ACVLC_NumByLength[1], 16);
	PutBytes(ACVLC_Data[1], sizeof(ACVLC_Data[1]));
}

// float(1.0 / sqrt(2.0))
#define invsq2 0.70710678118654f
#define PI 3.14159265358979

void CJpegEncoder::WriteSOS()
{
	PutByte(0xff);
	PutByte(0xda);

	WORD size = 6 + 2*ColorComponents;
	PutByte(size>>8);
	PutByte(size&0xff);

	PutByte(ColorComponents); // color components: 3

	PutByte(1); // component id
	PutByte(0x00); // DC | AC huff tbl

	PutByte(2); // component id
	PutByte(0x11); // DC | AC huff tbl

	PutByte(3); // component id
	PutByte(0x11); // DC | AC huff tbl

	PutByte(0); // ss, first AC
	PutByte(63); // se, last AC

	PutByte(0); // ah | al

	static float cosuv[8][8][8][8];

	// oh yeah, we don't need no fast dct :)
	for(int v = 0; v < 8; v++)
		for(int u = 0; u < 8; u++)
			for(int j = 0; j < 8; j++)
				for(int i = 0; i < 8; i++)
					cosuv[v][u][j][i] = (float)(cos((2*i+1)*u*PI/16) * cos((2*j+1)*v*PI/16));

	int prevDC[3] = {0, 0, 0};

	for(int y = 0; y < m_h; y += 8)
	{
		int jj = min(m_h - y, 8);

		for(int x = 0; x < m_w; x += 8)
		{
			int ii = min(m_w - x, 8);

			for(int c = 0; c < ColorComponents; c++)
			{
				int cc = !!c;

				int ACs = 0;

				static short block[64];

				for(int zigzag = 0; zigzag < 64; zigzag++)
				{
					BYTE u = zigzagU[zigzag];
					BYTE v = zigzagV[zigzag];

					float F = 0;
					/*
										for(int j = 0; j < jj; j++)
											for(int i = 0; i < ii; i++)
												F += (signed char)m_p[((y+j)*m_w + (x+i))*4 + c] * cosuv[v][u][j][i];
					*/
					for(int j = 0; j < jj; j++)
					{
						signed char* p = (signed char*)&m_p[((y+j)*m_w + x)*4 + c];
						for(int i = 0; i < ii; i++, p += 4)
							F += *p * cosuv[v][u][j][i];
					}

					float cu = !u ? invsq2 : 1.0f;
					float cv = !v ? invsq2 : 1.0f;

					block[zigzag] = short(2.0 / 8.0 * cu * cv * F) / quanttbl[cc][zigzag];
				}

				short DC = block[0] - prevDC[c];
				prevDC[c] = block[0];

				int size = GetBitWidth(DC);
				PutBit(DCVLC[cc][size], DCVLC_Size[cc][size]);

				if(DC < 0) DC = DC - 1;
				PutBit(DC, size);

				int j;
				for(j = 64; j > 1 && !block[j-1]; j--);

				for(int i = 1; i < j; i++)
				{
					short AC = block[i];

					if(AC == 0)
					{
						if(++ACs == 16)
						{
							PutBit(ACVLC[cc][15][0], ACVLC_Size[cc][15][0]);
							ACs = 0;
						}
					}
					else
					{
						int size = GetBitWidth(AC);
						PutBit(ACVLC[cc][ACs][size], ACVLC_Size[cc][ACs][size]);

						if(AC < 0) AC--;
						PutBit(AC, size);

						ACs = 0;
					}
				}

				if(j < 64) PutBit(ACVLC[cc][0][0], ACVLC_Size[cc][0][0]);
			}
		}
	}

	Flush();
}

void CJpegEncoder::WriteEOI()
{
	PutByte(0xff);
	PutByte(0xd9);
}

//

CJpegEncoder::CJpegEncoder()
{
}

bool CJpegEncoder::Encode(const BYTE* dib)
{
	m_bbuff = m_bwidth = 0;

	BITMAPINFO* bi = (BITMAPINFO*)dib;

	int bpp = bi->bmiHeader.biBitCount;

	if(bpp != 16 && bpp != 24 && bpp != 32) // 16 & 24 not tested!!! there may be some alignment problems when the row size is not 4*something in bytes
		return false;

	m_w = bi->bmiHeader.biWidth;
	m_h = abs(bi->bmiHeader.biHeight);
	m_p = DNew BYTE[m_w*m_h*4];

	const BYTE* src = dib + sizeof(bi->bmiHeader);
	if(bi->bmiHeader.biBitCount <= 8)
	{
		if(bi->bmiHeader.biClrUsed) src += bi->bmiHeader.biClrUsed * sizeof(bi->bmiColors[0]);
		else src += (1 << bi->bmiHeader.biBitCount) * DWORD(sizeof(bi->bmiColors[0]));
	}

	int srcpitch = m_w*(bpp>>3);
	int dstpitch = m_w*4;

	BitBltFromRGBToRGB(
		m_w, m_h,
		m_p, dstpitch, 32,
		(BYTE*)src + srcpitch*(m_h-1), -srcpitch, bpp);

	BYTE* p = m_p;
	for(BYTE* e = p + m_h*dstpitch; p < e; p += 4)
	{
		int r = p[2], g = p[1], b = p[0];

		p[0] = (BYTE)min(max(0.2990*r+0.5870*g+0.1140*b, 0), 255) - 128;
		p[1] = (BYTE)min(max(-0.1687*r-0.3313*g+0.5000*b + 128, 0), 255) - 128;
		p[2] = (BYTE)min(max(0.5000*r-0.4187*g-0.0813*b + 128, 0), 255) - 128;
	}

	if(quanttbl[0][0] == 16)
	{
		for(int i = 0; i < countof(quanttbl); i++)
			for(int j = 0; j < countof(quanttbl[0]); j++)
				quanttbl[i][j] >>= 2; // the default quantization table contains a little too large values
	}

	WriteSOI();
	WriteDQT();
	WriteSOF0();
	WriteDHT();
	WriteSOS();
	WriteEOI();

	delete [] m_p;

	return true;
}

//////////

CJpegEncoderFile::CJpegEncoderFile(LPCTSTR fn)
{
	m_fn = fn;
	m_file = NULL;
}

bool CJpegEncoderFile::PutByte(BYTE b)
{
	return fputc(b, m_file) != EOF;
}

bool CJpegEncoderFile::PutBytes(const void* pData, size_t len)
{
	return fwrite(pData, 1, len, m_file) == len;
}

bool CJpegEncoderFile::Encode(const BYTE* dib)
{
	m_file = _tfopen(m_fn, _T("wb"));
	if(!m_file)
		return false;
	bool ret = __super::Encode(dib);
	fclose(m_file);
	m_file = NULL;
	return ret;
}

//////////

CJpegEncoderMem::CJpegEncoderMem()
{
}

bool CJpegEncoderMem::PutByte(BYTE b)
{
	m_pdata->Add(b); // yeah... a bit unbuffered, for now
	return true;
}

bool CJpegEncoderMem::PutBytes(const void* pData, size_t len)
{
	CAtlArray<BYTE> moredata;
	moredata.SetCount(len);
	memcpy(moredata.GetData(), pData, len);
	m_pdata->Append(moredata);
	return true;
}

bool CJpegEncoderMem::Encode(const BYTE* dib, CAtlArray<BYTE>& data)
{
	m_pdata = &data;
	return __super::Encode(dib);
}
