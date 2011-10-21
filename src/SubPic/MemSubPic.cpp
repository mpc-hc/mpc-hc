/*
 *  $Id$
 *
 *  (C) 2003-2006 Gabest
 *  (C) 2006-2010 see AUTHORS
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
#include "MemSubPic.h"

// For CPUID usage
#include "../DSUtil/vd.h"
#include <emmintrin.h>

// color conv

unsigned char Clip_base[256*3];
unsigned char* Clip = Clip_base + 256;

const int c2y_cyb = int(0.114*219/255*65536+0.5);
const int c2y_cyg = int(0.587*219/255*65536+0.5);
const int c2y_cyr = int(0.299*219/255*65536+0.5);
const int c2y_cyb_hd = int(0.0722*219/255*65536+0.5);
const int c2y_cyg_hd = int(0.7152*219/255*65536+0.5);
const int c2y_cyr_hd = int(0.2126*219/255*65536+0.5);
int c2y_cu = int(1.0/2.018*1024+0.5);
int c2y_cv = int(1.0/1.596*1024+0.5);

int c2y_yb[256];
int c2y_yg[256];
int c2y_yr[256];

const int y2c_cbu = int(2.018*65536+0.5);
const int y2c_cgu = int(0.391*65536+0.5);
const int y2c_cgv = int(0.813*65536+0.5);
const int y2c_crv = int(1.596*65536+0.5);
const int y2c_cbu_hd = int(2.113*65536+0.5);
const int y2c_cgu_hd = int(0.213*65536+0.5);
const int y2c_cgv_hd = int(0.533*65536+0.5);
const int y2c_crv_hd = int(1.793*65536+0.5);
int y2c_bu[256];
int y2c_gu[256];
int y2c_gv[256];
int y2c_rv[256];

const int cy_cy = int(255.0/219.0*65536+0.5);
const int cy_cy2 = int(255.0/219.0*32768+0.5);

bool fColorConvInitOK = false;
bool useBT709 = false;

void ColorConvInit(bool BT709 = false)
{
	if(fColorConvInitOK && useBT709 == BT709) {
		return;
	}

	int i;

	for(i = 0; i < 256; i++) {
		Clip_base[i] = 0;
		Clip_base[i+256] = i;
		Clip_base[i+512] = 255;
	}

	if (!BT709) {
		for(i = 0; i < 256; i++) {
			c2y_yb[i] = c2y_cyb*i;
			c2y_yg[i] = c2y_cyg*i;
			c2y_yr[i] = c2y_cyr*i;

			y2c_bu[i] = y2c_cbu*(i-128);
			y2c_gu[i] = y2c_cgu*(i-128);
			y2c_gv[i] = y2c_cgv*(i-128);
			y2c_rv[i] = y2c_crv*(i-128);
		}
	} else {
		c2y_cu = int(1.0/2.113*1024+0.5);
		c2y_cv = int(1.0/1.793*1024+0.5);
		for(i = 0; i < 256; i++) {
			c2y_yb[i] = c2y_cyb_hd*i;
			c2y_yg[i] = c2y_cyg_hd*i;
			c2y_yr[i] = c2y_cyr_hd*i;

			y2c_bu[i] = y2c_cbu_hd*(i-128);
			y2c_gu[i] = y2c_cgu_hd*(i-128);
			y2c_gv[i] = y2c_cgv_hd*(i-128);
			y2c_rv[i] = y2c_crv_hd*(i-128);
		}
	}

	fColorConvInitOK = true;
	useBT709 = BT709;
}

#define rgb2yuv(r1,g1,b1,r2,g2,b2) \
	int y1 = (c2y_yb[b1] + c2y_yg[g1] + c2y_yr[r1] + 0x108000) >> 16; \
	int y2 = (c2y_yb[b2] + c2y_yg[g2] + c2y_yr[r2] + 0x108000) >> 16; \
\
	int scaled_y = (y1+y2-32) * cy_cy2; \
\
	unsigned char u = Clip[(((((b1+b2)<<15) - scaled_y) >> 10) * c2y_cu + 0x800000 + 0x8000) >> 16]; \
	unsigned char v = Clip[(((((r1+r2)<<15) - scaled_y) >> 10) * c2y_cv + 0x800000 + 0x8000) >> 16]; \
 
//
// CMemSubPic
//

CMemSubPic::CMemSubPic(SubPicDesc& spd)
	: m_spd(spd)
{
	m_maxsize.SetSize(spd.w, spd.h);
	m_rcDirty.SetRect(0, 0, spd.w, spd.h);
}

CMemSubPic::~CMemSubPic()
{
	delete [] m_spd.bits, m_spd.bits = NULL;
}

// ISubPic

STDMETHODIMP_(void*) CMemSubPic::GetObject()
{
	return (void*)&m_spd;
}

STDMETHODIMP CMemSubPic::GetDesc(SubPicDesc& spd)
{
	spd.type = m_spd.type;
	spd.w = m_size.cx;
	spd.h = m_size.cy;
	spd.bpp = m_spd.bpp;
	spd.pitch = m_spd.pitch;
	spd.bits = m_spd.bits;
	spd.bitsU = m_spd.bitsU;
	spd.bitsV = m_spd.bitsV;
	spd.vidrect = m_vidrect;

	return S_OK;
}

STDMETHODIMP CMemSubPic::CopyTo(ISubPic* pSubPic)
{
	HRESULT hr;
	if(FAILED(hr = __super::CopyTo(pSubPic))) {
		return hr;
	}

	SubPicDesc src, dst;
	if(FAILED(GetDesc(src)) || FAILED(pSubPic->GetDesc(dst))) {
		return E_FAIL;
	}

	int w = m_rcDirty.Width(), h = m_rcDirty.Height();

	BYTE* s = (BYTE*)src.bits + src.pitch*m_rcDirty.top + m_rcDirty.left*4;
	BYTE* d = (BYTE*)dst.bits + dst.pitch*m_rcDirty.top + m_rcDirty.left*4;

	for(ptrdiff_t j = 0; j < h; j++, s += src.pitch, d += dst.pitch) {
		memcpy(d, s, w*4);
	}

	return S_OK;
}

STDMETHODIMP CMemSubPic::ClearDirtyRect(DWORD color)
{
	if(m_rcDirty.IsRectEmpty()) {
		return S_FALSE;
	}

	BYTE* p = (BYTE*)m_spd.bits + m_spd.pitch*m_rcDirty.top + m_rcDirty.left*(m_spd.bpp>>3);
	for(ptrdiff_t j = 0, h = m_rcDirty.Height(); j < h; j++, p += m_spd.pitch) {
		int w = m_rcDirty.Width();
#ifdef _WIN64
		memsetd(p, color, w*4); // nya
#else
		__asm {
			mov eax, color
			mov ecx, w
			mov edi, p
			cld
			rep stosd
		}
#endif
	}

	m_rcDirty.SetRectEmpty();

	return S_OK;
}

STDMETHODIMP CMemSubPic::Lock(SubPicDesc& spd)
{
	return GetDesc(spd);
}

STDMETHODIMP CMemSubPic::Unlock(RECT* pDirtyRect)
{
	m_rcDirty = pDirtyRect ? *pDirtyRect : CRect(0,0,m_spd.w,m_spd.h);

	if(m_rcDirty.IsRectEmpty()) {
		return S_OK;
	}

	if(m_spd.type >= MSP_YUY2) { // YUV colorspace
		ColorConvInit(m_spd.w>1024 || m_spd.h>=688);

		if(m_spd.type != MSP_AYUV) {
			m_rcDirty.left &= ~1;
			m_rcDirty.right = (m_rcDirty.right+1)&~1;

			if(m_spd.type == MSP_YV12 || m_spd.type == MSP_IYUV || m_spd.type == MSP_NV12 || m_spd.type == MSP_NV21) {
				m_rcDirty.top &= ~1;
				m_rcDirty.bottom = (m_rcDirty.bottom+1)&~1;
			}
		}
	}

	int w = m_rcDirty.Width(), h = m_rcDirty.Height();

	BYTE* top = (BYTE*)m_spd.bits + m_spd.pitch*m_rcDirty.top + m_rcDirty.left*4;
	BYTE* bottom = top + m_spd.pitch*h;

	switch(m_spd.type) {
	case MSP_RGB16:
		for(; top < bottom ; top += m_spd.pitch) {
			DWORD* s = (DWORD*)top;
			DWORD* e = s + w;
			for(; s < e; s++) {
				*s = ((*s>>3)&0x1f000000)|((*s>>8)&0xf800)|((*s>>5)&0x07e0)|((*s>>3)&0x001f);
				//				*s = (*s&0xff000000)|((*s>>8)&0xf800)|((*s>>5)&0x07e0)|((*s>>3)&0x001f);
			}
		}
		break;
	case MSP_RGB15:
		for(; top < bottom; top += m_spd.pitch) {
			DWORD* s = (DWORD*)top;
			DWORD* e = s + w;
			for(; s < e; s++) {
				*s = ((*s>>3)&0x1f000000)|((*s>>9)&0x7c00)|((*s>>6)&0x03e0)|((*s>>3)&0x001f);
				//				*s = (*s&0xff000000)|((*s>>9)&0x7c00)|((*s>>6)&0x03e0)|((*s>>3)&0x001f);
			}
		}
		break;
	case MSP_YUY2:
	case MSP_YV12: case MSP_IYUV:
	case MSP_NV12: case MSP_NV21:
		for(; top < bottom ; top += m_spd.pitch) {
			BYTE* s = top;
			BYTE* e = s + w*4;
			for(; s < e; s+=8) { // ARGB ARGB -> AxYU AxYV
				if((s[3]+s[7]) < 0x1fe) {
					s[1] = (c2y_yb[s[0]] + c2y_yg[s[1]] + c2y_yr[s[2]] + 0x108000) >> 16;
					s[5] = (c2y_yb[s[4]] + c2y_yg[s[5]] + c2y_yr[s[6]] + 0x108000) >> 16;

					int scaled_y = (s[1]+s[5]-32) * cy_cy2;

					s[0] = Clip[(((((s[0]+s[4])<<15) - scaled_y) >> 10) * c2y_cu + 0x800000 + 0x8000) >> 16];
					s[4] = Clip[(((((s[2]+s[6])<<15) - scaled_y) >> 10) * c2y_cv + 0x800000 + 0x8000) >> 16];
				} else {
					s[1] = s[5] = 0x10;
					s[0] = s[4] = 0x80;
				}
			}
		}
		break;
	case MSP_AYUV:
		for(; top < bottom ; top += m_spd.pitch) {
			BYTE* s = top;
			BYTE* e = s + w*4;
			for(; s < e; s+=4) { // ARGB -> AYUV
				if(s[3] < 0xff) {
					int y = (c2y_yb[s[0]] + c2y_yg[s[1]] + c2y_yr[s[2]] + 0x108000) >> 16;
					int scaled_y = (y-32) * cy_cy;
					s[1] = Clip[((((s[0]<<16) - scaled_y) >> 10) * c2y_cu + 0x800000 + 0x8000) >> 16];
					s[0] = Clip[((((s[2]<<16) - scaled_y) >> 10) * c2y_cv + 0x800000 + 0x8000) >> 16];
					s[2] = y;
				} else {
					s[0] = s[1] = 0x80;
					s[2] = 0x10;
				}
			}
		}
	}

	return S_OK;
}

#ifdef _WIN64
void AlphaBlt_YUY2_SSE2(int w, int h, BYTE* d, int dstpitch, BYTE* s, int srcpitch)
{
	unsigned int ia;
	DWORD* d2 = (DWORD*)d;

	BYTE* s2 = s;
	BYTE* s2end = s2 + w*4;
	static const __int64 _8181 = 0x0080001000800010LL;
	__m128i mm_8181 = _mm_move_epi64(_mm_cvtsi64_si128(_8181));

	for(ptrdiff_t j = 0; j < h; j++, s += srcpitch, d += dstpitch) {
		for(; s2 < s2end; s2 += 8, d2++) {
			ia = s2[3]+s2[7];
			if(ia < 0x1fe) {
				ia >>= 1;
				unsigned int c = (s2[4]<<24)|(s2[5]<<16)|(s2[0]<<8)|s2[1]; // (v<<24)|(y2<<16)|(u<<8)|y1;
				ia = (ia<<24)|(s2[7]<<16)|(ia<<8)|s2[3];
				// SSE2
				__m128i mm_zero = _mm_setzero_si128();
				__m128i mm_c = _mm_cvtsi32_si128(c);
				mm_c = _mm_unpacklo_epi8(mm_c, mm_zero);
				__m128i mm_d = _mm_cvtsi32_si128(*d2);
				mm_d = _mm_unpacklo_epi8(mm_d, mm_zero);
				__m128i mm_a = _mm_cvtsi32_si128(ia);
				mm_a = _mm_unpacklo_epi8(mm_a, mm_zero);
				mm_a = _mm_srli_epi16(mm_a,1);
				mm_d = _mm_sub_epi16(mm_d,mm_8181);
				mm_d = _mm_mullo_epi16(mm_d,mm_a);
				mm_d = _mm_srai_epi16(mm_d,7);
				mm_d = _mm_adds_epi16(mm_d,mm_c);
				mm_d = _mm_packus_epi16(mm_d,mm_d);
				*d2 = (DWORD)_mm_cvtsi128_si32(mm_d);
			}
		}
	}
}
#endif

#ifndef _WIN64
void AlphaBlt_YUY2_MMX(int w, int h, BYTE* d, int dstpitch, BYTE* s, int srcpitch)
{
	unsigned int ia;
	DWORD* d2 = (DWORD*)d;

	BYTE* s2 = s;
	BYTE* s2end = s2 + w*4;
	__m64 _8181 = {0x0080001000800010LL};


	for(ptrdiff_t j = 0; j < h; j++, s += srcpitch, d += dstpitch) {
		for(; s2 < s2end; s2 += 8, d2++) {
			ia = s2[3]+s2[7];
			if(ia < 0x1fe) {
				ia >>= 1;
				unsigned int c = (s2[4]<<24)|(s2[5]<<16)|(s2[0]<<8)|s2[1]; // (v<<24)|(y2<<16)|(u<<8)|y1;
				ia = (ia<<24)|(s2[7]<<16)|(ia<<8)|s2[3];

				__m64 mm0, mm2, mm3, mm4;
				mm0 = _mm_setzero_si64();
				mm2 = _mm_cvtsi32_si64(c);
				mm2 = _mm_unpacklo_pi8(mm2, mm0);
				mm3 = _mm_cvtsi32_si64(*d2);
				mm3 = _mm_unpacklo_pi8(mm3, mm0);
				mm4 = _mm_cvtsi32_si64(ia);
				mm4 = _mm_unpacklo_pi8(mm4, mm0);
				mm4 = _mm_srli_pi16(mm4, 1);
				mm3 = _mm_sub_pi16(mm3, _8181);
				mm3 = _mm_mullo_pi16(mm3, mm4);
				mm3 = _mm_srai_pi16(mm3, 7);
				mm3 = _mm_add_pi16(mm3, mm2);
				mm3 = _mm_packs_pu16(mm3, mm3);
				*d2 = _mm_cvtsi64_si32(mm3);
			}
		}
	}
}
#endif

void AlphaBlt_YUY2_C(int w, int h, BYTE* d, int dstpitch, BYTE* s, int srcpitch)
{
	unsigned int ia;
	DWORD* d2 = (DWORD*)d;

	BYTE* s2 = s;
	BYTE* s2end = s2 + w*4;
	static const __int64 _8181 = 0x0080001000800010i64;

	for(ptrdiff_t j = 0; j < h; j++, s += srcpitch, d += dstpitch) {
		for(; s2 < s2end; s2 += 8, d2++) {
			ia = s2[3]+s2[7];
			if(ia < 0x1fe) {
				ia >>= 1;
				//unsigned int c = (s2[4]<<24)|(s2[5]<<16)|(s2[0]<<8)|s2[1]; // (v<<24)|(y2<<16)|(u<<8)|y1;

				// YUY2 colorspace fix. rewritten from sse2 asm
				DWORD y1 = (DWORD)(((((*d2&0xff)-0x10)*(s2[3]>>1))>>7)+s2[1])&0xff;			// y1
				DWORD uu = (DWORD)((((((*d2>>8)&0xff)-0x80)*(ia>>1))>>7)+s2[0])&0xff;		// u
				DWORD y2 = (DWORD)((((((*d2>>16)&0xff)-0x10)*(s2[7]>>1))>>7)+s2[5])&0xff;	// y2
				DWORD vv = (DWORD)((((((*d2>>24)&0xff)-0x80)*(ia>>1))>>7)+s2[4])&0xff;		// v
				*d2 = (y1)|(uu<<8)|(y2<<16)|(vv<<24);
			}
		}
	}
}

STDMETHODIMP alpha_blt_rgb32(RECT *pSrc, RECT *pDst, SubPicDesc *pTarget, SubPicDesc &src)
{
	SubPicDesc dst = *pTarget; // copy, because we might modify it

	int w = pSrc->right - pSrc->left, h = pSrc->bottom - pSrc->top;

	BYTE* s = (BYTE*)src.bits + src.pitch*pSrc->top + pSrc->left*4;
	BYTE* d = (BYTE*)dst.bits + dst.pitch*pDst->top + ((pDst->left*dst.bpp)>>3);

	switch(dst.type)
	{
	case MSP_RGBA:
		for(ptrdiff_t j = 0; j < h; j++, s += src.pitch, d += dst.pitch) {
			BYTE* s2 = s;
			BYTE* s2end = s2 + w*4;
			DWORD* d2 = (DWORD*)d;
			for(; s2 < s2end; s2 += 4, d2++) {
				if(s2[3] < 0xff) {
					DWORD bd =0x00000100 -( (DWORD) s2[3]);
					DWORD B = ((*((DWORD*)s2)&0x000000ff)<<8)/bd;
					DWORD V = ((*((DWORD*)s2)&0x0000ff00)/bd)<<8;
					DWORD R = (((*((DWORD*)s2)&0x00ff0000)>>8)/bd)<<16;
					*d2 = B | V | R
						| (0xff000000-(*((DWORD*)s2)&0xff000000))&0xff000000;
				}
			}
		}
		break;
	case MSP_RGB32:
	case MSP_AYUV:
		for(ptrdiff_t j = 0; j < h; j++, s += src.pitch, d += dst.pitch) {
			BYTE* s2 = s;
			BYTE* s2end = s2 + w*4;

			DWORD* d2 = (DWORD*)d;
			for(; s2 < s2end; s2 += 4, d2++) {
#ifdef _WIN64
				DWORD ia = 256-s2[3];
				if(s2[3] < 0xff) {
					*d2 = ((((*d2&0x00ff00ff)*s2[3])>>8) + (((*((DWORD*)s2)&0x00ff00ff)*ia)>>8)&0x00ff00ff)
						| ((((*d2&0x0000ff00)*s2[3])>>8) + (((*((DWORD*)s2)&0x0000ff00)*ia)>>8)&0x0000ff00);
				}
#else
				if(s2[3] < 0xff) {
					*d2 = ((((*d2&0x00ff00ff)*s2[3])>>8) + (*((DWORD*)s2)&0x00ff00ff)&0x00ff00ff)
						| ((((*d2&0x0000ff00)*s2[3])>>8) + (*((DWORD*)s2)&0x0000ff00)&0x0000ff00);
				}
#endif
			}
		}
		break;
	case MSP_RGB24:
		for(ptrdiff_t j = 0; j < h; j++, s += src.pitch, d += dst.pitch) {
			BYTE* s2 = s;
			BYTE* s2end = s2 + w*4;
			BYTE* d2 = d;
			for(; s2 < s2end; s2 += 4, d2 += 3) {
				if(s2[3] < 0xff) {
					d2[0] = ((d2[0]*s2[3])>>8) + s2[0];
					d2[1] = ((d2[1]*s2[3])>>8) + s2[1];
					d2[2] = ((d2[2]*s2[3])>>8) + s2[2];
				}
			}
		}
		break;
	case MSP_RGB16:
		for(ptrdiff_t j = 0; j < h; j++, s += src.pitch, d += dst.pitch) {
			BYTE* s2 = s;
			BYTE* s2end = s2 + w*4;
			WORD* d2 = (WORD*)d;
			for(; s2 < s2end; s2 += 4, d2++) {
				if(s2[3] < 0x1f) {
					*d2 = (WORD)((((((*d2&0xf81f)*s2[3])>>5) + (*(DWORD*)s2&0xf81f))&0xf81f)
						| (((((*d2&0x07e0)*s2[3])>>5) + (*(DWORD*)s2&0x07e0))&0x07e0));
				}
			}
		}
		break;
	case MSP_RGB15:
		for(ptrdiff_t j = 0; j < h; j++, s += src.pitch, d += dst.pitch) {
			BYTE* s2 = s;
			BYTE* s2end = s2 + w*4;
			WORD* d2 = (WORD*)d;
			for(; s2 < s2end; s2 += 4, d2++) {
				if(s2[3] < 0x1f) {
					*d2 = (WORD)((((((*d2&0x7c1f)*s2[3])>>5) + (*(DWORD*)s2&0x7c1f))&0x7c1f)
						| (((((*d2&0x03e0)*s2[3])>>5) + (*(DWORD*)s2&0x03e0))&0x03e0));
				}
			}
		}
		break;
	case MSP_YUY2:
#ifdef _WIN64
		AlphaBlt_YUY2_SSE2(w, h, d, dst.pitch, s, src.pitch);
#else
		AlphaBlt_YUY2_MMX(w, h, d, dst.pitch, s, src.pitch);
#endif
		break;
	case MSP_YV12: case MSP_IYUV:
	case MSP_NV12: case MSP_NV21:
		for(ptrdiff_t j = 0; j < h; j++, s += src.pitch, d += dst.pitch) {
			BYTE* s2 = s;
			BYTE* s2end = s2 + w*4;
			BYTE* d2 = d;
			for(; s2 < s2end; s2 += 4, d2++) {
				if(s2[3] < 0xff)
					d2[0] = (((d2[0]-0x10)*s2[3])>>8) + s2[1];
			}
		}
		break;
	default:
		return E_NOTIMPL;
	}

	int h2 = h/2;
	s = (BYTE*)src.bits + src.pitch*pSrc->top + pSrc->left*4;

	if(dst.type == MSP_YV12 || dst.type == MSP_IYUV) {
		if(!dst.pitchUV)
			dst.pitchUV = abs(dst.pitch)/2;
		if(!dst.bitsU || !dst.bitsV) {
			dst.bitsU = (BYTE*)dst.bits + dst.pitch*dst.h;
			dst.bitsV = dst.bitsU + dst.pitchUV*dst.h/2;

			if(dst.type == MSP_YV12) {
				BYTE* p = dst.bitsU;
				dst.bitsU = dst.bitsV;
				dst.bitsV = p;
			}
		}

		d = dst.bitsU + dst.pitchUV*pDst->top/2 + pDst->left/2;
		if(pDst->top > pDst->bottom) {
			d -= dst.pitchUV*pDst->top;
			dst.pitchUV = -dst.pitchUV;
		}

		int offset = dst.bitsV - dst.bitsU;
		for(ptrdiff_t j = 0; j < h2; j++, s += src.pitch*2, d += dst.pitchUV) {
			BYTE* s2 = s; BYTE* d2 = d;
			BYTE* s2end = s2 + w*4;
			for(; s2 < s2end; s2 += 8, d2++) {
				unsigned int ia = s2[3]+s2[7]+s2[3+src.pitch]+s2[7+src.pitch];
				if(ia < 0x3fc) {
					d2[0] = ((((d2[0]-0x80)*ia)>>9)+s2[0]+s2[src.pitch])>>1;
					d2[offset] = ((((d2[offset]-0x80)*ia)>>9)+s2[4]+s2[src.pitch+4])>>1;
				}
			}
		}
	} else if(dst.type == MSP_NV12 || dst.type == MSP_NV21) {
		if(!dst.pitchUV)
			dst.pitchUV = abs(dst.pitch);
		if(!dst.bitsU) {
			dst.bitsU = (BYTE*)dst.bits + dst.pitch*dst.h;
			d = dst.bitsU + dst.pitchUV*pDst->top/2 + pDst->left;
		}
		if(pDst->top > pDst->bottom) {
			d -= dst.pitchUV*pDst->top;
			dst.pitchUV = -dst.pitchUV;
		}

		if (dst.type == MSP_NV12) {
			for(ptrdiff_t j = 0; j < h2; j++, s += src.pitch*2, d += dst.pitchUV) {
				BYTE* s2 = s; BYTE* d2 = d;
				BYTE* s2end = s2 + w*4;
				for(; s2 < s2end; s2 += 8, d2 += 2) {
					unsigned int ia = s2[3]+s2[7]+s2[3+src.pitch]+s2[7+src.pitch];
					if(ia < 0x3fc) {
						d2[0] = ((((d2[0]-0x80)*ia)>>9)+s2[0]+s2[src.pitch])>>1;
						d2[1] = ((((d2[1]-0x80)*ia)>>9)+s2[4]+s2[src.pitch+4])>>1;
					}
				}
			}
		} else {
			for(ptrdiff_t j = 0; j < h2; j++, s += src.pitch*2, d += dst.pitchUV) {
				BYTE* s2 = s; BYTE* d2 = d;
				BYTE* s2end = s2 + w*4;
				for(; s2 < s2end; s2 += 8, d2 += 2) {
					unsigned int ia = s2[3]+s2[7]+s2[3+src.pitch]+s2[7+src.pitch];
					if(ia < 0x3fc) {
						d2[0] = ((((d2[0]-0x80)*ia)>>9)+s2[4]+s2[src.pitch+4])>>1;
						d2[1] = ((((d2[1]-0x80)*ia)>>9)+s2[0]+s2[src.pitch])>>1;
					}
				}
			}
		}
	}

#ifndef _WIN64
	__asm emms;
#endif
	return S_OK;
}

STDMETHODIMP CMemSubPic::AlphaBlt(RECT* pSrc, RECT* pDst, SubPicDesc* pTarget)
{
	ASSERT(pTarget);

	if(!pSrc || !pDst || !pTarget) {
		return E_POINTER;
	}

	return alpha_blt_rgb32(pSrc, pDst, pTarget, m_spd);
}

//
// CMemSubPicAllocator
//

CMemSubPicAllocator::CMemSubPicAllocator(int type, SIZE maxsize)
: CSubPicAllocatorImpl(maxsize, false, false)
, m_type(type)
, m_maxsize(maxsize)
{
}

// ISubPicAllocatorImpl

bool CMemSubPicAllocator::Alloc(bool fStatic, ISubPic** ppSubPic)
{
	if(!ppSubPic) {
		return false;
	}

	SubPicDesc spd;
	spd.w = m_maxsize.cx;
	spd.h = m_maxsize.cy;
	spd.bpp = 32;
	spd.pitch = (spd.w*spd.bpp)>>3;
	spd.type = m_type;
	spd.bits = DNew BYTE[spd.pitch*spd.h];
	if(!spd.bits) {
		return false;
	}

	*ppSubPic = DNew CMemSubPic(spd);
	if(!(*ppSubPic)) {
		return false;
	}

	(*ppSubPic)->AddRef();

	return true;
}
