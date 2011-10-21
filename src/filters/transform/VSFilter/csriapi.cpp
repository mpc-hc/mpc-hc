/*
 *	Copyright (C) 2007 Niels Martin Hansen
 *	http://aegisub.net/
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
#include <atlpath.h>
#include "resource.h"
#include "../../../Subtitles/VobSubFile.h"
#include "../../../Subtitles/RTS.h"
#include "../../../Subtitles/SSF.h"
#include "../../../SubPic/MemSubPic.h"

#define CSRIAPI extern "C" __declspec(dllexport)
#define CSRI_OWN_HANDLES
#define CSRI_MAX_SUB 16
typedef const char *csri_rend;
extern "C" struct csri_vsfilter_inst {
	CRenderedTextSubtitle *rts[CSRI_MAX_SUB];
	int sub_num;
	CCritSec *cs;
	CSize script_res;
	CSize screen_res;
	CRect video_rect;
	enum csri_pixfmt pixfmt;
	BYTE *rgb32buf;
};

typedef struct csri_vsfilter_inst csri_inst;
#include "csri.h"

#ifdef _VSMOD
static csri_rend csri_vsfilter = "vsfiltermod";
#else
static csri_rend csri_vsfilter = "vsfilter";
#endif

void inline clear_dirty_rect_rgb32(SubPicDesc *pspd, RECT &rc, DWORD color)
{
	int i = rc.bottom - rc.top,
		l = rc.left,
		j = rc.right - l;

	if (0 == i || 0 == j)
		return;

	BYTE *p = (BYTE *)pspd->bits + (rc.top*pspd->w<<2)+(l<<2);
	do {
		memsetd(p,color,j*4);
		p += (pspd->w<<2);
	} while (--i);
}

extern int c2y_cu;
extern int c2y_cv;
const int cy_cy = int(255.0/219.0*65536+0.5);
const int cy_cy2 = int(255.0/219.0*32768+0.5);

extern int c2y_yb[256];
extern int c2y_yg[256];
extern int c2y_yr[256];
extern int y2c_bu[256];
extern int y2c_gu[256];
extern int y2c_gv[256];
extern int y2c_rv[256];
extern unsigned char* Clip;

void ColorConvInit(bool);
void AlphaBlt_YUY2_MMX(int, int, BYTE*, int, BYTE*, int);
void AlphaBlt_YUY2_SSE2(int, int, BYTE*, int, BYTE*, int);
void AlphaBlt_YUY2_C(int, int, BYTE*, int, BYTE*, int);
STDMETHODIMP alpha_blt_rgb32(RECT *pSrc, RECT *pDst, SubPicDesc *pTarget, SubPicDesc &src);

void inline unlock_dirty_rect(SubPicDesc *pspd)
{
	if(pspd->type >= MSP_YUY2) {// YUV colorspace
		ColorConvInit(pspd->w>1024 || pspd->h>=688);

		if(pspd->type != MSP_AYUV) {
			pspd->vidrect.left &= ~1;
			pspd->vidrect.right = (pspd->vidrect.right+1)&~1;

			if(pspd->type == MSP_YV12 || pspd->type == MSP_IYUV || pspd->type == MSP_NV12 || pspd->type == MSP_NV21) {
				pspd->vidrect.top &= ~1;
				pspd->vidrect.bottom = (pspd->vidrect.bottom+1)&~1;
			}
		}
	}

	int w = pspd->vidrect.right-pspd->vidrect.left, h = pspd->vidrect.bottom-pspd->vidrect.top;

	BYTE* top = (BYTE*)pspd->bits + pspd->pitch*pspd->vidrect.top + pspd->vidrect.left*4;
	BYTE* bottom = top + pspd->pitch*h;

	switch(pspd->type) {
	case MSP_RGB16:
		for(; top < bottom ; top += pspd->pitch) {
			DWORD* s = (DWORD*)top;
			DWORD* e = s + w;
			for(; s < e; s++)
				*s = ((*s>>3)&0x1f000000)|((*s>>8)&0xf800)|((*s>>5)&0x07e0)|((*s>>3)&0x001f);
		}
		break;
	case MSP_RGB15:
		for(; top < bottom; top += pspd->pitch) {
			DWORD* s = (DWORD*)top;
			DWORD* e = s + w;
			for(; s < e; s++)
				*s = ((*s>>3)&0x1f000000)|((*s>>9)&0x7c00)|((*s>>6)&0x03e0)|((*s>>3)&0x001f);
		}
		break;
	case MSP_YUY2:
	case MSP_YV12: case MSP_IYUV:
	case MSP_NV12: case MSP_NV21:
		for(; top < bottom ; top += pspd->pitch) {
			BYTE* s = top;
			BYTE* e = s + w*4;
			for(; s < e; s+=8) {// ARGB ARGB -> AxYU AxYV
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
		for(; top < bottom ; top += pspd->pitch) {
			BYTE* s = top;
			BYTE* e = s + w*4;
			for(; s < e; s+=4) {// ARGB -> AYUV
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
}

CSRIAPI int csri_add_file(csri_inst *inst, const char *filename, struct csri_openflag *flags)
{
	int i = inst->sub_num;
	if (i >= CSRI_MAX_SUB)
		return 0;

	wchar_t *namebuf;
	int namesize = MultiByteToWideChar(CP_OEMCP, 0, filename, -1, NULL, 0);
	if (!namesize) {
		return 0;
	}
	namesize++;
	namebuf = new wchar_t[namesize];
	MultiByteToWideChar(CP_OEMCP, 0, filename, -1, namebuf, namesize);

	inst->rts[i] = new CRenderedTextSubtitle(inst->cs);
	while (flags) {
		if (0 == _stricmp(flags->name,"PAR")) {
			inst->rts[i]->m_ePARCompensationType = CSimpleTextSubtitle::EPCTAccurateSize;
			inst->rts[i]->m_dPARCompensation = flags->data.dval;
		}
		flags = flags->next;
	}
	if (inst->rts[i]->Open(CString(namebuf), DEFAULT_CHARSET)) {
		delete[] namebuf;
		inst->sub_num++;
		return 1;
	} else {
		delete[] namebuf;
		delete inst->rts[i];
		delete inst;
		return 0;
	}
}

CSRIAPI csri_inst *csri_open_file(csri_rend *renderer, const char *filename, struct csri_openflag *flags)
{
	csri_inst *inst = new csri_inst();
	inst->sub_num = 0;
	inst->cs = new CCritSec();
	inst->rgb32buf = NULL;
	if (csri_add_file(inst, filename, flags))
		return inst;
	else {
		delete inst->cs;
		delete inst;
		return 0;
	}
}

CSRIAPI csri_inst *csri_open_mem(csri_rend *renderer, const void *data, size_t length, struct csri_openflag *flags)
{
	// This is actually less effecient than opening a file, since this first writes the memory data to a temp file,
	// then opens that file and parses from that.
	csri_inst *inst = new csri_inst();
	inst->cs = new CCritSec();
	inst->rts[0] = new CRenderedTextSubtitle(inst->cs);
	if (inst->rts[0]->Open((BYTE*)data, (int)length, DEFAULT_CHARSET, _T("CSRI memory subtitles"))) {
		inst->sub_num = 1;
		return inst;
	} else {
		delete inst->rts[0];
		delete inst->cs;
		delete inst;
		return 0;
	}
}

CSRIAPI void csri_close(csri_inst *inst)
{
	if (!inst) {
		return;
	}

	for (int i=inst->sub_num; i; i--)
		delete inst->rts[i-1];
	delete inst->cs;
	if (inst->rgb32buf)
		free(inst->rgb32buf);
	delete inst;
}

CSRIAPI int csri_request_fmt(csri_inst *inst, const struct csri_fmt *fmt)
{
	if (!inst) {
		return -1;
	}

	if (!fmt->width || !fmt->height) {
		return -1;
	}

	// Check if pixel format is supported
	switch (fmt->pixfmt) {
		case CSRI_F_YUY2:
		case CSRI_F_YV12:
		case CSRI_F_NV12:
		case CSRI_F_NV21:
		case CSRI_F_BGR:
			if (inst->rgb32buf)
				free(inst->rgb32buf);
			inst->rgb32buf = (BYTE *)malloc(fmt->width * (fmt->height + 1) * 4);
			memsetd(inst->rgb32buf,0xFF000000,fmt->width * fmt->height * 4);
		case CSRI_F_BGR_:
			inst->pixfmt = fmt->pixfmt;
			break;
		default:
			return -1;
	}
	inst->screen_res = CSize(fmt->width, fmt->height);
	inst->video_rect = CRect(0, 0, fmt->width, fmt->height);
	return 0;
}

CSRIAPI void csri_render(csri_inst *inst, struct csri_frame *frame, double time)
{
	const double arbitrary_framerate = 25.0;
	SubPicDesc spd;
	spd.w = inst->screen_res.cx;
	spd.h = inst->screen_res.cy;
	switch (inst->pixfmt) {
		case CSRI_F_BGR_:
			spd.type = MSP_RGB32;
			spd.bpp = 32;
			spd.bits = frame->planes[0];
			spd.pitch = frame->strides[0];
			break;

		case CSRI_F_BGR:
			spd.type = MSP_RGB24;
			spd.bpp = 24;
			spd.bits = frame->planes[0];
			spd.pitch = frame->strides[0];
			break;

		case CSRI_F_YUY2:
			spd.type = MSP_YUY2;
			spd.bpp = 16;
			spd.bits = frame->planes[0];
			spd.pitch = frame->strides[0];
			break;

		case CSRI_F_YV12:
			spd.type = MSP_YV12;
			spd.bpp = 8;
			spd.bits = frame->planes[0];
			spd.bitsU = frame->planes[2];
			spd.bitsV = frame->planes[1];
			spd.pitch = frame->strides[0];
			spd.pitchUV = frame->strides[1];
			break;

		case CSRI_F_NV12:
			spd.type = MSP_NV12;
			goto SKIP_NV21;
		case CSRI_F_NV21:
			spd.type = MSP_NV21;
		SKIP_NV21:
			spd.bpp = 8;
			spd.bits = frame->planes[0];
			spd.bitsU = frame->planes[1];
			spd.bitsV = frame->planes[1];
			spd.pitch = frame->strides[0];
			spd.pitchUV = frame->strides[1];
			break;

		default:
			// eh?
			return;
	}
	spd.vidrect = inst->video_rect;

	if (MSP_RGB32 != spd.type) {
		SubPicDesc spdrgb32;
		spdrgb32.type = spd.type;
		spdrgb32.bpp = 32;
		spdrgb32.w = spd.w;
		spdrgb32.h = spd.h;
		spdrgb32.bits = inst->rgb32buf;
		spdrgb32.pitch = spd.pitch * 4;
		spdrgb32.vidrect = spd.vidrect;
		for (int i=0; i<inst->sub_num; i++) {
			if (S_OK == inst->rts[i]->Render(spdrgb32, (REFERENCE_TIME)(time*10000000), arbitrary_framerate, spdrgb32.vidrect)) {
				unlock_dirty_rect(&spdrgb32);
				alpha_blt_rgb32(&spd.vidrect, &spd.vidrect, &spd, spdrgb32);
				clear_dirty_rect_rgb32(&spdrgb32,spdrgb32.vidrect,0xFF000000);
			}
		}
	}
	else
		for (int i=0; i<inst->sub_num; i++)
			inst->rts[i]->Render(spd, (REFERENCE_TIME)(time*10000000), arbitrary_framerate, spd.vidrect);
}

// No extensions supported
CSRIAPI void *csri_query_ext(csri_rend *rend, csri_ext_id extname)
{
	return 0;
}

// Get info for renderer
static struct csri_info csri_vsfilter_info = {
#ifdef _DEBUG
#ifdef _VSMOD
	"vsfiltermod_textsub_debug", // name
#else
	"vsfilter_textsub_debug", // name
#endif
	"2.40", // version (assumed version number, svn revision, patchlevel)
#else
#ifdef _VSMOD
	"vsfiltermod_textsub", // name
#else
	"vsfilter_textsub", // name
#endif
	"2.40", // version (assumed version number, svn revision, patchlevel)
#endif
	// 2.38-0611 is base svn 611
	// 2.38-0611-1 is with clipfix and fax/fay patch
	// 2.38-0611-2 adds CSRI
	// 2.38-0611-3 fixes a bug in CSRI and adds fontcrash-fix and float-pos
	// 2.38-0611-4 fixes be1-dots and ugly-fade bugs and adds xbord/ybord/xshad/yshad/blur tags and extends be
	// 2.39 merges with guliverkli2 fork
#ifdef _VSMOD
	"VSFilterMod/TextSub (guliverkli2)", // longname
#else
	"VSFilter/TextSub (MPC-HC)", // longname
#endif
	"Gabest", // author
	"Copyright (c) 2003-2011 by Gabest and others" // copyright
};

CSRIAPI struct csri_info *csri_renderer_info(csri_rend *rend) {
	return &csri_vsfilter_info;
}

// Only one supported, obviously
CSRIAPI csri_rend *csri_renderer_byname(const char *name, const char *specific)
{
	if (strcmp(name, csri_vsfilter_info.name)) {
		return 0;
	}
	if (specific && strcmp(specific, csri_vsfilter_info.specific)) {
		return 0;
	}
	return &csri_vsfilter;
}

// Still just one
CSRIAPI csri_rend *csri_renderer_default()
{
	return &csri_vsfilter;
}

// And no further
CSRIAPI csri_rend *csri_renderer_next(csri_rend *prev)
{
	return 0;
}
