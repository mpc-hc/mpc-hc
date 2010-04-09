// Copyright 2003-2006 Gabest
// http://www.gabest.org
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html

#include "stdafx.h"
#include <math.h>
#include "DirectVobSubFilter.h"
#include "../../../DSUtil/DSUtil.h"
#include "../../../DSUtil/MediaTypes.h"

#include <initguid.h>
#include <moreuuids.h>

extern int c2y_yb[256];
extern int c2y_yg[256];
extern int c2y_yr[256];
extern void ColorConvInit();

void BltLineRGB32(DWORD* d, BYTE* sub, int w, const GUID& subtype)
{
	if(subtype == MEDIASUBTYPE_YV12 || subtype == MEDIASUBTYPE_I420 || subtype == MEDIASUBTYPE_IYUV)
	{
		BYTE* db = (BYTE*)d;
		BYTE* dbtend = db + w;

		for(; db < dbtend; sub+=4, db++)
		{
			if(sub[3] < 0xff)
			{
				int y = (c2y_yb[sub[0]] + c2y_yg[sub[1]] + c2y_yr[sub[2]] + 0x108000) >> 16; 
				*db = y; // w/o colors 
			}
		}
	}
	else if(subtype == MEDIASUBTYPE_YUY2)
	{
		WORD* ds = (WORD*)d;
		WORD* dstend = ds + w;

		for(; ds < dstend; sub+=4, ds++)
		{
			if(sub[3] < 0xff)
			{
				int y = (c2y_yb[sub[0]] + c2y_yg[sub[1]] + c2y_yr[sub[2]] + 0x108000) >> 16; 
				*ds = 0x8000|y; // w/o colors 
			}
		}
	}
	else if(subtype == MEDIASUBTYPE_RGB555)
	{
		WORD* ds = (WORD*)d;
		WORD* dstend = ds + w;

		for(; ds < dstend; sub+=4, ds++)
		{
			if(sub[3] < 0xff)
			{
				*ds = ((*((DWORD*)sub)>>9)&0x7c00)|((*((DWORD*)sub)>>6)&0x03e0)|((*((DWORD*)sub)>>3)&0x001f);
			}
		}
	}
	else if(subtype == MEDIASUBTYPE_RGB565)
	{
		WORD* ds = (WORD*)d;
		WORD* dstend = ds + w;

		for(; ds < dstend; sub+=4, ds++)
		{
			if(sub[3] < 0xff)
			{
				*ds = ((*((DWORD*)sub)>>8)&0xf800)|((*((DWORD*)sub)>>5)&0x07e0)|((*((DWORD*)sub)>>3)&0x001f);
			}
		}
	}
	else if(subtype == MEDIASUBTYPE_RGB24)
	{
		BYTE* dt = (BYTE*)d;
		BYTE* dstend = dt + w*3;

		for(; dt < dstend; sub+=4, dt+=3)
		{
			if(sub[3] < 0xff)
			{
				dt[0] = sub[0];
				dt[1] = sub[1];
				dt[2] = sub[2];
			}
		}
	}
	else if(subtype == MEDIASUBTYPE_RGB32 || subtype == MEDIASUBTYPE_ARGB32)
	{
		DWORD* dstend = d + w;

		for(; d < dstend; sub+=4, d++)
		{
			if(sub[3] < 0xff) *d = *((DWORD*)sub)&0xffffff;
		}
	}
}

#ifdef WIN64
// For CPUID usage
#include "../../../dsutil/vd.h"
#include <emmintrin.h>
#endif
/* ResX2 */
void Scale2x(const GUID& subtype, BYTE* d, int dpitch, BYTE* s, int spitch, int w, int h)
{
#ifdef WIN64
	// CPUID from VDub
	bool fSSE2 = !!(g_cpuid.m_flags & CCpuID::sse2);
#endif

	if(subtype == MEDIASUBTYPE_YV12 || subtype == MEDIASUBTYPE_I420 || subtype == MEDIASUBTYPE_IYUV)
	{
		BYTE* s1;
		BYTE* s2;
		BYTE* d1;

		for(s1 = s, s2 = s + h*spitch, d1 = d; s1 < s2; d1 += dpitch) // TODO: replace this mess with mmx code
		{
			BYTE* stmp = s1 + spitch;
			BYTE* dtmp = d1 + dpitch;

			for(BYTE* s3 = s1 + (w-1); s1 < s3; s1 += 1, d1 += 2)
			{
				d1[0] = s1[0]; 
				d1[1] = (s1[0]+s1[1])>>1;
			}

			d1[0] = d1[1] = s1[0]; 

			s1 += 1;
			d1 += 2;

			s1 = stmp;
			d1 = dtmp;
		}

		AvgLines8(d, h*2, dpitch);
	}
	else if(subtype == MEDIASUBTYPE_YUY2)
	{
		unsigned __int64 __0xffffffff00000000 = 0xffffffff00000000;
		unsigned __int64 __0x00000000ffffffff = 0x00000000ffffffff;
		unsigned __int64 __0x00ff00ff00ff00ff = 0x00ff00ff00ff00ff;

		BYTE* s1;
		BYTE* s2;
		BYTE* d1;

		for(s1 = s, s2 = s + h*spitch, d1 = d; s1 < s2; d1 += dpitch)
		{
			BYTE* stmp = s1 + spitch;
			BYTE* dtmp = d1 + dpitch;

			// row0, 4 pixels: y1|u1|y2|v1|y3|u2|y4|v2
			// ->
			// row0, 8 pixels: y1|u1|(y1+y2)/2|v1|y2|(u1+u2)/2|(y2+y3)/2|(v1+v2)/2

#ifdef WIN64
			if(fSSE2)
			{
				__m128i mm4 = _mm_cvtsi64_si128(__0x00ff00ff00ff00ff);
				__m128i mm5 = _mm_cvtsi64_si128(__0x00000000ffffffff);
				__m128i mm6 = _mm_cvtsi64_si128(__0xffffffff00000000);
				for(BYTE* s3 = s1 + ((w>>1)-1)*4; s1 < s3; s1 += 4, d1 += 8)
				{
					__m128i mm0 = _mm_cvtsi64_si128(*(size_t*)s1); //movq	mm0, [esi]
					__m128i mm2 = _mm_move_epi64(mm0);			//movq	mm2, mm0
					mm0 = _mm_and_si128(mm0, mm4);				//pand	mm0, mm4	// mm0 = 00y400y300y200y1
					mm2 = _mm_srli_epi16(mm2, 8);				//psrlw	mm2, 8		// mm2 = 00u200v200u100v1
					__m128i mm1 = _mm_move_epi64(mm0);			//movq	mm1, mm0
					mm0 = _mm_and_si128(mm0, mm5);				//pand	mm0, mm5	// mm0 = 0000000000y200y1
					mm1 = _mm_slli_epi64(mm1, 16);				//psllq	mm1, 16
					mm1 = _mm_and_si128(mm1, mm6);				//pand	mm1, mm6	// mm1 = 00y300y200000000
					mm1 = _mm_or_si128(mm1, mm0);				//por	mm1, mm0	// mm1 = 00y300y200y200y1
					mm0 = _mm_unpacklo_epi8(mm0, mm0);			//punpcklwd mm0, mm0	// mm0 = 00y200y200y100y1
					mm0 = _mm_adds_epi16(mm0,mm1);				//paddw	mm0, mm1
					mm0 = _mm_srli_epi16(mm0, 1);				//psrlw	mm0, 1		// mm0 = (mm0 + mm1) / 2
					mm1 = _mm_move_epi64(mm2);					//movq	mm1, mm2
					mm1 = _mm_unpacklo_epi32(mm1, mm1);			//punpckldq	mm1, mm1 // mm1 = 00u100v100u100v1
					mm1 = _mm_adds_epi16(mm1,mm2);				//paddw	mm1, mm2
					mm1 = _mm_srli_epi16(mm1, 1);				//psrlw	mm1, 1		// mm1 = (mm1 + mm2) / 2
					mm1 = _mm_slli_epi64(mm1, 8);				//psllw	mm1, 8
					mm1 = _mm_or_si128(mm0, mm1);				//por		mm0, mm1	// mm0 = (v1+v2)/2|(y2+y3)/2|(u1+u2)/2|y2|v1|(y1+y2)/2|u1|y1
					*(size_t*)d1 = (size_t)_mm_cvtsi128_si64(mm0);		//movq	[edi], mm0
				}
			}
			else
			{
				for(BYTE* s3 = s1 + ((w>>1)-1)*4; s1 < s3; s1 += 4, d1 += 8)
				{
					d1[0] = s1[0]; 
					d1[1] = s1[1]; 
					d1[2] = (s1[0]+s1[2])>>1;
					d1[3] = s1[3];

					d1[4] = s1[2];
					d1[5] = (s1[1]+s1[5])>>1;
					d1[6] = (s1[2]+s1[4])>>1;
					d1[7] = (s1[3]+s1[7])>>1;
				}
			}
#else
			__asm
			{
				mov		esi, s1
				mov		edi, d1

				mov		ecx, w
				shr		ecx, 1
				dec		ecx

				movq	mm4, __0x00ff00ff00ff00ff
				movq	mm5, __0x00000000ffffffff
				movq	mm6, __0xffffffff00000000
row_loop1:
				movq	mm0, [esi]
				movq	mm2, mm0

				pand	mm0, mm4	// mm0 = 00y400y300y200y1
				psrlw	mm2, 8		// mm2 = 00u200v200u100v1


				movq	mm1, mm0

				pand	mm0, mm5	// mm0 = 0000000000y200y1

				psllq	mm1, 16
				pand	mm1, mm6	// mm1 = 00y300y200000000

				por		mm1, mm0	// mm1 = 00y300y200y200y1

				punpcklwd mm0, mm0	// mm0 = 00y200y200y100y1

				paddw	mm0, mm1
				psrlw	mm0, 1		// mm0 = (mm0 + mm1) / 2


				movq	mm1, mm2
				punpckldq	mm1, mm1 // mm1 = 00u100v100u100v1

				paddw	mm1, mm2
				psrlw	mm1, 1		// mm1 = (mm1 + mm2) / 2


				psllw	mm1, 8
				por		mm0, mm1	// mm0 = (v1+v2)/2|(y2+y3)/2|(u1+u2)/2|y2|v1|(y1+y2)/2|u1|y1

				movq	[edi], mm0

				lea		esi, [esi+4]
				lea		edi, [edi+8]

				dec		ecx
				jnz		row_loop1

				mov		s1, esi
				mov		d1, edi
			};
#endif
			*d1++ = s1[0];
			*d1++ = s1[1];
			*d1++ =(s1[0]+s1[2])>>1;
			*d1++ = s1[3];

			*d1++ = s1[2];
			*d1++ = s1[1];
			*d1++ = s1[2];
			*d1++ = s1[3];

			s1 += 4;

			s1 = stmp;
			d1 = dtmp;
		}

		AvgLines8(d, h*2, dpitch);
	}
	else if(subtype == MEDIASUBTYPE_RGB555)
	{
		BYTE* s1;
		BYTE* s2;
		BYTE* d1;

		for(s1 = s, s2 = s + h*spitch, d1 = d; s1 < s2; d1 += dpitch) // TODO: replace this mess with mmx code
		{
			BYTE* stmp = s1 + spitch;
			BYTE* dtmp = d1 + dpitch;

			for(BYTE* s3 = s1 + (w-1)*2; s1 < s3; s1 += 2, d1 += 4)
			{
				*((WORD*)d1) = *((WORD*)s1);
				*((WORD*)d1+1) = 
					((((*((WORD*)s1)&0x7c00) + (*((WORD*)s1+1)&0x7c00)) >> 1)&0x7c00)|
					((((*((WORD*)s1)&0x03e0) + (*((WORD*)s1+1)&0x03e0)) >> 1)&0x03e0)|
					((((*((WORD*)s1)&0x001f) + (*((WORD*)s1+1)&0x001f)) >> 1)&0x001f);
			}

			*((WORD*)d1) = *((WORD*)s1);
			*((WORD*)d1+1) = *((WORD*)s1);

			s1 += 2;
			d1 += 4;

			s1 = stmp;
			d1 = dtmp;
		}

		AvgLines555(d, h*2, dpitch);
	}
	else if(subtype == MEDIASUBTYPE_RGB565)
	{
		BYTE* s1;
		BYTE* s2;
		BYTE* d1;

		for(s1 = s, s2 = s + h*spitch, d1 = d; s1 < s2; d1 += dpitch) // TODO: replace this mess with mmx code
		{
			BYTE* stmp = s1 + spitch;
			BYTE* dtmp = d1 + dpitch;

			for(BYTE* s3 = s1 + (w-1)*2; s1 < s3; s1 += 2, d1 += 4)
			{
				*((WORD*)d1) = *((WORD*)s1);
				*((WORD*)d1+1) = 
					((((*((WORD*)s1)&0xf800) + (*((WORD*)s1+1)&0xf800)) >> 1)&0xf800)|
					((((*((WORD*)s1)&0x07e0) + (*((WORD*)s1+1)&0x07e0)) >> 1)&0x07e0)|
					((((*((WORD*)s1)&0x001f) + (*((WORD*)s1+1)&0x001f)) >> 1)&0x001f);
			}

			*((WORD*)d1) = *((WORD*)s1);
			*((WORD*)d1+1) = *((WORD*)s1);

			s1 += 2;
			d1 += 4;

			s1 = stmp;
			d1 = dtmp;
		}

		AvgLines565(d, h*2, dpitch);
	}
	else if(subtype == MEDIASUBTYPE_RGB24)
	{
		BYTE* s1;
		BYTE* s2;
		BYTE* d1;

		for(s1 = s, s2 = s + h*spitch, d1 = d; s1 < s2; d1 += dpitch) // TODO: replace this mess with mmx code
		{
			BYTE* stmp = s1 + spitch;
			BYTE* dtmp = d1 + dpitch;

			for(BYTE* s3 = s1 + (w-1)*3; s1 < s3; s1 += 3, d1 += 6)
			{
				d1[0] = s1[0]; 
				d1[1] = s1[1]; 
				d1[2] = s1[2];
				d1[3] = (s1[0]+s1[3])>>1;
				d1[4] = (s1[1]+s1[4])>>1;
				d1[5] = (s1[2]+s1[5])>>1;
			}

			d1[0] = d1[3] = s1[0]; 
			d1[1] = d1[4] = s1[1]; 
			d1[2] = d1[5] = s1[2];

			s1 += 3;
			d1 += 6;

			s1 = stmp;
			d1 = dtmp;
		}

		AvgLines8(d, h*2, dpitch);
	}
	else if(subtype == MEDIASUBTYPE_RGB32 || subtype == MEDIASUBTYPE_ARGB32)
	{
		BYTE* s1;
		BYTE* s2;
		BYTE* d1;

		for(s1 = s, s2 = s + h*spitch, d1 = d; s1 < s2; d1 += dpitch)
		{
			BYTE* stmp = s1 + spitch;
			BYTE* dtmp = d1 + dpitch;

#ifdef WIN64
			if(fSSE2) // SSE2 code
			{
				__m128i mm_zero = _mm_setzero_si128();//pxor	mm0, mm0
				for(BYTE* s3 = s1 + (w-1)*4; s1 < s3; s1 += 4, d1 += 8)
				{
					
					__m128i mm1 = _mm_cvtsi64_si128(*(size_t*)s1); //movq	mm1, [esi]
					__m128i mm2 = _mm_move_epi64(mm1);			//movq	mm2, mm1

					mm1 = _mm_unpacklo_epi8(mm1,mm_zero);//punpcklbw mm1, mm0	// mm1 = 00xx00r100g100b1
					mm2 = _mm_unpacklo_epi8(mm2,mm_zero);//punpckhbw mm2, mm0	// mm2 = 00xx00r200g200b2

					mm2 = _mm_adds_epi16(mm2,mm1);		//paddw	mm2, mm1
					mm2 = _mm_srli_epi16(mm2, 1);		//psrlw	mm2, 1		// mm2 = (mm1 + mm2) / 2

					mm1 = _mm_packus_epi16(mm1,mm2);	//packuswb	mm1, mm2

					*(size_t*)d1=(size_t)_mm_cvtsi128_si64(mm1);//movq	[edi], mm1
				}
			}
			else
			{
				for(BYTE* s3 = s1 + (w-1)*4; s1 < s3; s1 += 3, d1 += 6)
				{
					d1[0] = s1[0]; 
					d1[1] = s1[1]; 
					d1[2] = s1[2];
					d1[3] = s1[3];

					d1[4] = (s1[0]+s1[4])>>1;
					d1[5] = (s1[1]+s1[5])>>1;
					d1[6] = (s1[2]+s1[6])>>1;
					d1[7] = (s1[4]+s1[7])>>1;
				}
			}
#else
			__asm
			{
				mov		esi, s1
				mov		edi, d1

				mov		ecx, w
				dec		ecx

				pxor	mm0, mm0
row_loop3:
				movq	mm1, [esi]
				movq	mm2, mm1

				punpcklbw mm1, mm0	// mm1 = 00xx00r100g100b1
				punpckhbw mm2, mm0	// mm2 = 00xx00r200g200b2

				paddw	mm2, mm1
				psrlw	mm2, 1		// mm2 = (mm1 + mm2) / 2

				packuswb	mm1, mm2

				movq	[edi], mm1

				lea		esi, [esi+4]
				lea		edi, [edi+8]

				dec		ecx
				jnz		row_loop3

				mov		s1, esi
				mov		d1, edi
			};
#endif

			*((DWORD*)d1) = *((DWORD*)s1);
			*((DWORD*)d1+1) = *((DWORD*)s1);

			s1 += 4;
			d1 += 8;

			s1 = stmp;
			d1 = dtmp;
		}

		AvgLines8(d, h*2, dpitch);
	}

#ifndef WIN64
	__asm emms;
#endif
}

HRESULT CDirectVobSubFilter::Copy(BYTE* pSub, BYTE* pIn, CSize sub, CSize in, int bpp, const GUID& subtype, DWORD black)
{
	int wIn = in.cx, hIn = in.cy, pitchIn = wIn*bpp>>3;
	int wSub = sub.cx, hSub = sub.cy, pitchSub = wSub*bpp>>3;
	bool fScale2x = wIn*2 <= wSub;

	if(fScale2x) wIn <<= 1, hIn <<= 1;

	int left = ((wSub - wIn)>>1)&~1;
	int mid = wIn;
	int right = left + ((wSub - wIn)&1);

	int dpLeft = left*bpp>>3;
	int dpMid = mid*bpp>>3;
	int dpRight = right*bpp>>3;

	ASSERT(wSub >= wIn);

	{
		int i = 0, j = 0;

		j += (hSub - hIn) >> 1;

		for(; i < j; i++, pSub += pitchSub)
		{
			memsetd(pSub, black, dpLeft+dpMid+dpRight);
		}

		j += hIn;

		if(hIn > hSub)
			pIn += pitchIn * ((hIn - hSub) >> (fScale2x?2:1));

		if(fScale2x)
		{
			Scale2x(subtype, 
				pSub + dpLeft, pitchSub, pIn, pitchIn, 
				in.cx, (min(j, hSub) - i) >> 1);
            
			for(ptrdiff_t k = min(j, hSub); i < k; i++, pIn += pitchIn, pSub += pitchSub)
			{
				memsetd(pSub, black, dpLeft);
				memsetd(pSub + dpLeft+dpMid, black, dpRight);
			}
		}
		else
		{
			for(ptrdiff_t k = min(j, hSub); i < k; i++, pIn += pitchIn, pSub += pitchSub)
			{
				memsetd(pSub, black, dpLeft);
				memcpy(pSub + dpLeft, pIn, dpMid);
				memsetd(pSub + dpLeft+dpMid, black, dpRight);
			}
		}

		j = hSub;

		for(; i < j; i++, pSub += pitchSub)
		{
			memsetd(pSub, black, dpLeft+dpMid+dpRight);
		}
	}

	return NOERROR;
}

void CDirectVobSubFilter::PrintMessages(BYTE* pOut)
{
	if(!m_hdc || !m_hbm)
		return;

	ColorConvInit();

	const GUID& subtype = m_pOutput->CurrentMediaType().subtype;

	BITMAPINFOHEADER bihOut;
	ExtractBIH(&m_pOutput->CurrentMediaType(), &bihOut);

	CString msg, tmp;

	if(m_fOSD)
	{
		tmp.Format(_T("in: %dx%d %s\nout: %dx%d %s\n"), 
			m_w, m_h, 
			Subtype2String(m_pInput->CurrentMediaType().subtype),
			bihOut.biWidth, bihOut.biHeight, 
			Subtype2String(m_pOutput->CurrentMediaType().subtype));
		msg += tmp;

		tmp.Format(_T("real fps: %.3f, current fps: %.3f\nmedia time: %d, subtitle time: %d [ms]\nframe number: %d (calculated)\nrate: %.4f\n"), 
			m_fps, m_fMediaFPSEnabled?m_MediaFPS:fabs(m_fps),
			(int)m_tPrev.Millisecs(), (int)(CalcCurrentTime()/10000),
			(int)(m_tPrev.m_time * m_fps / 10000000),
			m_pInput->CurrentRate());
		msg += tmp;

		CAutoLock cAutoLock(&m_csQueueLock);

		if(m_pSubPicQueue)
		{
			int nSubPics = -1;
			REFERENCE_TIME rtNow = -1, rtStart = -1, rtStop = -1;
			m_pSubPicQueue->GetStats(nSubPics, rtNow, rtStart, rtStop);
			tmp.Format(_T("queue stats: %I64d - %I64d [ms]\n"), rtStart/10000, rtStop/10000);
			msg += tmp;

			for(ptrdiff_t i = 0; i < nSubPics; i++)
			{
				m_pSubPicQueue->GetStats(i, rtStart, rtStop);
				tmp.Format(_T("%d: %I64d - %I64d [ms]\n"), i, rtStart/10000, rtStop/10000);
				msg += tmp;
			}

		}
	}

	if(msg.IsEmpty()) return;

	HANDLE hOldBitmap = SelectObject(m_hdc, m_hbm);
	HANDLE hOldFont = SelectObject(m_hdc, m_hfont);

	SetTextColor(m_hdc, 0xffffff);
	SetBkMode(m_hdc, TRANSPARENT);
	SetMapMode(m_hdc, MM_TEXT);

	BITMAP bm;
	GetObject(m_hbm, sizeof(BITMAP), &bm);

	CRect r(0, 0, bm.bmWidth, bm.bmHeight);
	DrawText(m_hdc, msg, _tcslen(msg), &r, DT_CALCRECT|DT_EXTERNALLEADING|DT_NOPREFIX|DT_WORDBREAK);

	r += CPoint(10, 10);
	r &= CRect(0, 0, bm.bmWidth, bm.bmHeight);

	DrawText(m_hdc, msg, _tcslen(msg), &r, DT_LEFT|DT_TOP|DT_NOPREFIX|DT_WORDBREAK);

	BYTE* pIn = (BYTE*)bm.bmBits;
	int pitchIn = bm.bmWidthBytes;
	int pitchOut = bihOut.biWidth * bihOut.biBitCount >> 3;

	if(subtype == MEDIASUBTYPE_YV12 || subtype == MEDIASUBTYPE_I420 || subtype == MEDIASUBTYPE_IYUV)
		pitchOut = bihOut.biWidth;

	pitchIn = (pitchIn+3)&~3;
	pitchOut = (pitchOut+3)&~3;

	if(bihOut.biHeight > 0 && bihOut.biCompression <= 3) // flip if the dst bitmap is flipped rgb (m_hbm is a top-down bitmap, not like the subpictures)
	{
		pOut += pitchOut * (abs(bihOut.biHeight)-1);
		pitchOut = -pitchOut;
	}

	pIn += pitchIn * r.top;
	pOut += pitchOut * r.top;

	for(ptrdiff_t w = min(r.right, m_w), h = r.Height(); h--; pIn += pitchIn, pOut += pitchOut)
	{
		BltLineRGB32((DWORD*)pOut, pIn, w, subtype);
		memsetd(pIn, 0xff000000, r.right*4);
	}

	SelectObject(m_hdc, hOldBitmap);
	SelectObject(m_hdc, hOldFont);
}
