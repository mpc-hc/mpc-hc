//	VirtualDub - Video processing and capture application
//	Graphics support library
//	Copyright (C) 1998-2009 Avery Lee
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include <stdafx.h>
#include <float.h>
#include <math.h>
#include <vd2/system/vdalloc.h>
#include <vd2/system/vdstl.h>
#include <vd2/system/memory.h>
#include <vd2/system/math.h>
#include <vd2/system/cpuaccel.h>
#include <vd2/Kasumi/pixmap.h>
#include <vd2/Kasumi/pixmaputils.h>
#include <vd2/Kasumi/resample.h>
#include "uberblit_gen.h"

///////////////////////////////////////////////////////////////////////////
//
// the resampler (finally)
//
///////////////////////////////////////////////////////////////////////////

class VDPixmapResampler : public IVDPixmapResampler {
public:
	VDPixmapResampler();
	~VDPixmapResampler();

	void SetSplineFactor(double A) { mSplineFactor = A; }
	void SetFilters(FilterMode h, FilterMode v, bool interpolationOnly);
	bool Init(uint32 dw, uint32 dh, int dstformat, uint32 sw, uint32 sh, int srcformat);
	bool Init(const vdrect32f& dstrect, uint32 dw, uint32 dh, int dstformat, const vdrect32f& srcrect, uint32 sw, uint32 sh, int srcformat);
	void Shutdown();

	void Process(const VDPixmap& dst, const VDPixmap& src);

protected:
	void ApplyFilters(VDPixmapUberBlitterGenerator& gen, uint32 dw, uint32 dh, float xoffset, float yoffset, float xfactor, float yfactor);

	vdautoptr<IVDPixmapBlitter> mpBlitter;
	vdautoptr<IVDPixmapBlitter> mpBlitter2;
	double				mSplineFactor;
	FilterMode			mFilterH;
	FilterMode			mFilterV;
	bool				mbInterpOnly;

	vdrect32			mDstRectPlane0;
	vdrect32			mDstRectPlane12;
};

IVDPixmapResampler *VDCreatePixmapResampler() { return new VDPixmapResampler; }

VDPixmapResampler::VDPixmapResampler()
	: mSplineFactor(-0.6)
	, mFilterH(kFilterCubic)
	, mFilterV(kFilterCubic)
	, mbInterpOnly(false)
{
}

VDPixmapResampler::~VDPixmapResampler() {
	Shutdown();
}

void VDPixmapResampler::SetFilters(FilterMode h, FilterMode v, bool interpolationOnly) {
	mFilterH = h;
	mFilterV = v;
	mbInterpOnly = interpolationOnly;
}

bool VDPixmapResampler::Init(uint32 dw, uint32 dh, int dstformat, uint32 sw, uint32 sh, int srcformat) {
	vdrect32f rSrc(0.0f, 0.0f, (float)sw, (float)sh);
	vdrect32f rDst(0.0f, 0.0f, (float)dw, (float)dh);
	return Init(rDst, dw, dh, dstformat, rSrc, sw, sh, srcformat);
}

bool VDPixmapResampler::Init(const vdrect32f& dstrect0, uint32 dw, uint32 dh, int dstformat, const vdrect32f& srcrect0, uint32 sw, uint32 sh, int srcformat) {
	Shutdown();

	if (dstformat != srcformat)
		return false;
	
	switch(srcformat) {
		case nsVDPixmap::kPixFormat_XRGB8888:
		case nsVDPixmap::kPixFormat_Y8:
		case nsVDPixmap::kPixFormat_Y8_FR:
		case nsVDPixmap::kPixFormat_YUV444_Planar:
		case nsVDPixmap::kPixFormat_YUV444_Planar_FR:
		case nsVDPixmap::kPixFormat_YUV444_Planar_709:
		case nsVDPixmap::kPixFormat_YUV444_Planar_709_FR:
		case nsVDPixmap::kPixFormat_YUV422_Planar:
		case nsVDPixmap::kPixFormat_YUV422_Planar_FR:
		case nsVDPixmap::kPixFormat_YUV422_Planar_709:
		case nsVDPixmap::kPixFormat_YUV422_Planar_709_FR:
		case nsVDPixmap::kPixFormat_YUV420_Planar:
		case nsVDPixmap::kPixFormat_YUV420_Planar_FR:
		case nsVDPixmap::kPixFormat_YUV420_Planar_709:
		case nsVDPixmap::kPixFormat_YUV420_Planar_709_FR:
		case nsVDPixmap::kPixFormat_YUV411_Planar:
		case nsVDPixmap::kPixFormat_YUV411_Planar_FR:
		case nsVDPixmap::kPixFormat_YUV411_Planar_709:
		case nsVDPixmap::kPixFormat_YUV411_Planar_709_FR:
		case nsVDPixmap::kPixFormat_YUV410_Planar:
		case nsVDPixmap::kPixFormat_YUV410_Planar_FR:
		case nsVDPixmap::kPixFormat_YUV410_Planar_709:
		case nsVDPixmap::kPixFormat_YUV410_Planar_709_FR:
			break;

		default:
			return false;
	}

	// convert destination flips to source flips
	vdrect32f dstrect(dstrect0);
	vdrect32f srcrect(srcrect0);

	if (dstrect.left > dstrect.right) {
		std::swap(dstrect.left, dstrect.right);
		std::swap(srcrect.left, srcrect.right);
	}

	if (dstrect.top > dstrect.bottom) {
		std::swap(dstrect.top, dstrect.bottom);
		std::swap(srcrect.top, srcrect.bottom);
	}

	// compute source step factors
	float xfactor = (float)srcrect.width()  / (float)dstrect.width();
	float yfactor = (float)srcrect.height() / (float)dstrect.height();

	// clip destination rect
	if (dstrect.left < 0) {
		float clipx1 = -dstrect.left;
		srcrect.left += xfactor * clipx1;
		dstrect.left = 0.0f;
	}

	if (dstrect.top < 0) {
		float clipy1 = -dstrect.top;
		srcrect.top += yfactor * clipy1;
		dstrect.top = 0.0f;
	}

	float clipx2 = dstrect.right - (float)dw;
	if (clipx2 > 0) {
		srcrect.right -= xfactor * clipx2;
		dstrect.right = (float)dw;
	}

	float clipy2 = dstrect.bottom - (float)dh;
	if (clipy2 > 0) {
		srcrect.bottom -= yfactor * clipy2;
		dstrect.bottom = (float)dh;
	}

	// compute plane 0 dest rect in integral quanta
	const VDPixmapFormatInfo& formatInfo = VDPixmapGetInfo(dstformat);
	mDstRectPlane0.left		= VDCeilToInt(dstrect.left	 - 0.5f);
	mDstRectPlane0.top		= VDCeilToInt(dstrect.top	 - 0.5f);
	mDstRectPlane0.right	= VDCeilToInt(dstrect.right	 - 0.5f);
	mDstRectPlane0.bottom	= VDCeilToInt(dstrect.bottom - 0.5f);

	// compute plane 0 stepping parameters
	float xoffset = (((float)mDstRectPlane0.left + 0.5f) - dstrect.left) * xfactor + srcrect.left;
	float yoffset = (((float)mDstRectPlane0.top  + 0.5f) - dstrect.top ) * yfactor + srcrect.top;

	// compute plane 1/2 dest rect and stepping parameters
	float xoffset2 = 0.0f;
	float yoffset2 = 0.0f;

	if (formatInfo.auxbufs > 0) {
		float xf2 = (float)(1 << formatInfo.auxwbits);
		float yf2 = (float)(1 << formatInfo.auxhbits);
		float invxf2 = 1.0f / xf2;
		float invyf2 = 1.0f / yf2;

		// convert source and dest rects to plane 1/2 space
		vdrect32f srcrect2(srcrect);
		vdrect32f dstrect2(dstrect);

		srcrect2.scale(invxf2, invyf2);
		dstrect2.scale(invxf2, invyf2);

		switch(srcformat) {
		case nsVDPixmap::kPixFormat_YUV444_Planar:
		case nsVDPixmap::kPixFormat_YUV444_Planar_FR:
		case nsVDPixmap::kPixFormat_YUV444_Planar_709:
		case nsVDPixmap::kPixFormat_YUV444_Planar_709_FR:
			break;
		case nsVDPixmap::kPixFormat_YUV422_Planar:
		case nsVDPixmap::kPixFormat_YUV422_Planar_FR:
		case nsVDPixmap::kPixFormat_YUV422_Planar_709:
		case nsVDPixmap::kPixFormat_YUV422_Planar_709_FR:
			srcrect2.translate(0.25f, 0.0f);
			dstrect2.translate(0.25f, 0.0f);
			break;
		case nsVDPixmap::kPixFormat_YUV420_Planar:
		case nsVDPixmap::kPixFormat_YUV420_Planar_FR:
		case nsVDPixmap::kPixFormat_YUV420_Planar_709:
		case nsVDPixmap::kPixFormat_YUV420_Planar_709_FR:
			srcrect2.translate(0.25f, 0.0f);
			dstrect2.translate(0.25f, 0.0f);
			break;
		case nsVDPixmap::kPixFormat_YUV411_Planar:
		case nsVDPixmap::kPixFormat_YUV411_Planar_FR:
		case nsVDPixmap::kPixFormat_YUV411_Planar_709:
		case nsVDPixmap::kPixFormat_YUV411_Planar_709_FR:
			srcrect2.translate(0.375f, 0.0f);
			dstrect2.translate(0.375f, 0.0f);
			break;
		case nsVDPixmap::kPixFormat_YUV410_Planar:
		case nsVDPixmap::kPixFormat_YUV410_Planar_FR:
		case nsVDPixmap::kPixFormat_YUV410_Planar_709:
		case nsVDPixmap::kPixFormat_YUV410_Planar_709_FR:
			break;
		default:
			VDASSERT(false);
		}

		mDstRectPlane12.left	= VDCeilToInt(dstrect2.left		- 0.5f);
		mDstRectPlane12.top		= VDCeilToInt(dstrect2.top		- 0.5f);
		mDstRectPlane12.right	= VDCeilToInt(dstrect2.right	- 0.5f);
		mDstRectPlane12.bottom	= VDCeilToInt(dstrect2.bottom	- 0.5f);

		xoffset2 = (((float)mDstRectPlane12.left + 0.5f) - dstrect2.left) * xfactor + srcrect2.left;
		yoffset2 = (((float)mDstRectPlane12.top  + 0.5f) - dstrect2.top ) * yfactor + srcrect2.top;
	}

	VDPixmapUberBlitterGenerator gen;

	switch(srcformat) {
		case nsVDPixmap::kPixFormat_XRGB8888:
			gen.ldsrc(0, 0, 0, 0, sw, sh, VDPixmapGetFormatTokenFromFormat(srcformat), sw*4);
			ApplyFilters(gen, mDstRectPlane0.width(), mDstRectPlane0.height(), xoffset, yoffset, xfactor, yfactor);
			break;

		case nsVDPixmap::kPixFormat_Y8:
			gen.ldsrc(0, 0, 0, 0, sw, sh, kVDPixType_8, sw);
			ApplyFilters(gen, mDstRectPlane0.width(), mDstRectPlane0.height(), xoffset, yoffset, xfactor, yfactor);
			break;

		case nsVDPixmap::kPixFormat_YUV444_Planar:
		case nsVDPixmap::kPixFormat_YUV422_Planar:
		case nsVDPixmap::kPixFormat_YUV420_Planar:
		case nsVDPixmap::kPixFormat_YUV411_Planar:
		case nsVDPixmap::kPixFormat_YUV410_Planar:
		case nsVDPixmap::kPixFormat_YUV444_Planar_FR:
		case nsVDPixmap::kPixFormat_YUV422_Planar_FR:
		case nsVDPixmap::kPixFormat_YUV420_Planar_FR:
		case nsVDPixmap::kPixFormat_YUV411_Planar_FR:
		case nsVDPixmap::kPixFormat_YUV410_Planar_FR:
		case nsVDPixmap::kPixFormat_YUV444_Planar_709:
		case nsVDPixmap::kPixFormat_YUV422_Planar_709:
		case nsVDPixmap::kPixFormat_YUV420_Planar_709:
		case nsVDPixmap::kPixFormat_YUV411_Planar_709:
		case nsVDPixmap::kPixFormat_YUV410_Planar_709:
		case nsVDPixmap::kPixFormat_YUV444_Planar_709_FR:
		case nsVDPixmap::kPixFormat_YUV422_Planar_709_FR:
		case nsVDPixmap::kPixFormat_YUV420_Planar_709_FR:
		case nsVDPixmap::kPixFormat_YUV411_Planar_709_FR:
		case nsVDPixmap::kPixFormat_YUV410_Planar_709_FR:
			gen.ldsrc(0, 0, 0, 0, sw, sh, kVDPixType_8, sw);
			ApplyFilters(gen, mDstRectPlane0.width(), mDstRectPlane0.height(), xoffset, yoffset, xfactor, yfactor);

			{
				const VDPixmapFormatInfo& info = VDPixmapGetInfo(dstformat);
				uint32 subsw = -(-(sint32)sw >> info.auxwbits);
				uint32 subsh = -(-(sint32)sh >> info.auxhbits);

				VDPixmapUberBlitterGenerator gen2;
				gen2.ldsrc(0, 0, 0, 0, subsw, subsh, kVDPixType_8, subsw);
				ApplyFilters(gen2, mDstRectPlane12.width(), mDstRectPlane12.height(), xoffset2, yoffset2, xfactor, yfactor);
				mpBlitter2 = gen2.create();
				if (!mpBlitter2)
					return false;
			}
			break;
	}

	mpBlitter = gen.create();
	if (!mpBlitter)
		return false;

	return true;
}

void VDPixmapResampler::Shutdown() {
	mpBlitter = NULL;
	mpBlitter2 = NULL;
}

void VDPixmapResampler::Process(const VDPixmap& dst, const VDPixmap& src) {
	if (!mpBlitter)
		return;

	switch(dst.format) {
		case nsVDPixmap::kPixFormat_XRGB8888:
		case nsVDPixmap::kPixFormat_Y8:
			mpBlitter->Blit(dst, &mDstRectPlane0, src);
			break;

		case nsVDPixmap::kPixFormat_YUV444_Planar:
		case nsVDPixmap::kPixFormat_YUV422_Planar:
		case nsVDPixmap::kPixFormat_YUV420_Planar:
		case nsVDPixmap::kPixFormat_YUV411_Planar:
		case nsVDPixmap::kPixFormat_YUV410_Planar:
		case nsVDPixmap::kPixFormat_YUV444_Planar_FR:
		case nsVDPixmap::kPixFormat_YUV422_Planar_FR:
		case nsVDPixmap::kPixFormat_YUV420_Planar_FR:
		case nsVDPixmap::kPixFormat_YUV411_Planar_FR:
		case nsVDPixmap::kPixFormat_YUV410_Planar_FR:
		case nsVDPixmap::kPixFormat_YUV444_Planar_709:
		case nsVDPixmap::kPixFormat_YUV422_Planar_709:
		case nsVDPixmap::kPixFormat_YUV420_Planar_709:
		case nsVDPixmap::kPixFormat_YUV411_Planar_709:
		case nsVDPixmap::kPixFormat_YUV410_Planar_709:
		case nsVDPixmap::kPixFormat_YUV444_Planar_709_FR:
		case nsVDPixmap::kPixFormat_YUV422_Planar_709_FR:
		case nsVDPixmap::kPixFormat_YUV420_Planar_709_FR:
		case nsVDPixmap::kPixFormat_YUV411_Planar_709_FR:
		case nsVDPixmap::kPixFormat_YUV410_Planar_709_FR:
			// blit primary plane
			mpBlitter->Blit(dst, &mDstRectPlane0, src);

			// slice and blit secondary planes
			{
				const VDPixmapFormatInfo& formatInfo = VDPixmapGetInfo(dst.format);
				VDPixmap pxdst;
				pxdst.format	= nsVDPixmap::kPixFormat_Y8;
				pxdst.w			= -(-dst.w >> formatInfo.auxwbits);
				pxdst.h			= -(-dst.h >> formatInfo.auxhbits);
				pxdst.pitch		= dst.pitch2;
				pxdst.data		= dst.data2;

				VDPixmap pxsrc;
				pxsrc.format	= nsVDPixmap::kPixFormat_Y8;
				pxsrc.w			= -(-src.w >> formatInfo.auxwbits);
				pxsrc.h			= -(-src.h >> formatInfo.auxhbits);
				pxsrc.pitch		= src.pitch2;
				pxsrc.data		= src.data2;

				mpBlitter2->Blit(pxdst, &mDstRectPlane12, pxsrc);

				pxdst.pitch		= dst.pitch3;
				pxdst.data		= dst.data3;
				pxsrc.pitch		= src.pitch3;
				pxsrc.data		= src.data3;
				mpBlitter2->Blit(pxdst, &mDstRectPlane12, pxsrc);
			}
			break;
	}
}

void VDPixmapResampler::ApplyFilters(VDPixmapUberBlitterGenerator& gen, uint32 dw, uint32 dh, float xoffset, float yoffset, float xfactor, float yfactor) {
	switch(mFilterH) {
		case kFilterPoint:
			gen.pointh(xoffset, xfactor, dw);
			break;

		case kFilterLinear:
			gen.linearh(xoffset, xfactor, dw, mbInterpOnly);
			break;

		case kFilterCubic:
			gen.cubich(xoffset, xfactor, dw, (float)mSplineFactor, mbInterpOnly);
			break;

		case kFilterLanczos3:
			gen.lanczos3h(xoffset, xfactor, dw);
			break;
	}

	switch(mFilterV) {
		case kFilterPoint:
			gen.pointv(yoffset, yfactor, dh);
			break;

		case kFilterLinear:
			gen.linearv(yoffset, yfactor, dh, mbInterpOnly);
			break;

		case kFilterCubic:
			gen.cubicv(yoffset, yfactor, dh, (float)mSplineFactor, mbInterpOnly);
			break;

		case kFilterLanczos3:
			gen.lanczos3v(yoffset, yfactor, dh);
			break;
	}
}

bool VDPixmapResample(const VDPixmap& dst, const VDPixmap& src, IVDPixmapResampler::FilterMode filter) {
	VDPixmapResampler r;

	r.SetFilters(filter, filter, false);

	if (!r.Init(dst.w, dst.h, dst.format, src.w, src.h, src.format))
		return false;

	r.Process(dst, src);
	return true;
}
