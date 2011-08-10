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
#include <vd2/system/vdstl.h>
#include <vd2/system/memory.h>
#include <vd2/system/math.h>
#include <vd2/system/cpuaccel.h>
#include <vd2/Kasumi/pixmap.h>
#include <vd2/Kasumi/pixmaputils.h>
#include <vd2/Kasumi/resample.h>

#include <vd2/Kasumi/resample_kernels.h>
#include "resample_stages_x86.h"
#include "uberblit_resample.h"

namespace {
	sint32 scale32x32_fp16(sint32 x, sint32 y) {
		return (sint32)(((sint64)x * y + 0x8000) >> 16);
	}

	template<class T>
	IVDResamplerSeparableRowStage *RowFactory(double cutoff, float filterFactor) {
		return new T;
	}

	template<class T>
	IVDResamplerSeparableRowStage *RowFactoryLinear(double cutoff, float filterFactor) {
		return new T(VDResamplerLinearFilter(cutoff));
	}

	template<class T>
	IVDResamplerSeparableRowStage *RowFactoryCubic(double cutoff, float filterFactor) {
		return new T(VDResamplerCubicFilter(cutoff, filterFactor));
	}

	template<class T>
	IVDResamplerSeparableRowStage *RowFactoryCubic2(double cutoff, float filterFactor) {
		return new T(filterFactor);
	}

	template<class T>
	IVDResamplerSeparableRowStage *RowFactoryLanczos3(double cutoff, float filterFactor) {
		return new T(VDResamplerLanczos3Filter(cutoff));
	}

	template<class T>
	IVDResamplerSeparableColStage *ColFactory(double cutoff, float filterFactor) {
		return new T;
	}

	template<class T>
	IVDResamplerSeparableColStage *ColFactoryLinear(double cutoff, float filterFactor) {
		return new T(VDResamplerLinearFilter(cutoff));
	}

	template<class T>
	IVDResamplerSeparableColStage *ColFactoryCubic(double cutoff, float filterFactor) {
		return new T(VDResamplerCubicFilter(cutoff, filterFactor));
	}

	template<class T>
	IVDResamplerSeparableColStage *ColFactoryCubic2(double cutoff, float filterFactor) {
		return new T(filterFactor);
	}

	template<class T>
	IVDResamplerSeparableColStage *ColFactoryLanczos3(double cutoff, float filterFactor) {
		return new T(VDResamplerLanczos3Filter(cutoff));
	}
}

///////////////////////////////////////////////////////////////////////////
//
// VDPixmapGenResampleRow
//
///////////////////////////////////////////////////////////////////////////

VDPixmapGenResampleRow::VDPixmapGenResampleRow()
	: mpRowStage(NULL)
	, mpRowStage2(NULL)
{
}

VDPixmapGenResampleRow::~VDPixmapGenResampleRow() {
	if (mpRowStage)
		delete mpRowStage;
}

void VDPixmapGenResampleRow::Init(IVDPixmapGen *src, uint32 srcIndex, uint32 width, float offset, float step, nsVDPixmap::FilterMode filterMode, float filterFactor, bool interpolationOnly) {
	InitSource(src, srcIndex);

	sint32 u0 = (sint32)(offset * 65536.0);
	sint32 dudx = (sint32)(step * 65536.0);

	mAxis.Init(dudx);

	double x_2fc = 1.0;
	if (!interpolationOnly && step > 1.0f)
		x_2fc = 1.0 / step;

	struct SpecialCaseSpanRoutine {
		sint32		mPhase;
		sint32		mStep;
		uint32		mType;
		nsVDPixmap::FilterMode mFilterMode;
		uint32 mCPUFlags;
		IVDResamplerSeparableRowStage *(*mpClassFactory)(double filterCutoff, float filterFactor);
	};

	static const SpecialCaseSpanRoutine kSpecialCaseSpanRoutines[]={
		// Generic
#if defined _M_IX86
		{ +0x0000, 0x008000, kVDPixType_8,		nsVDPixmap::kFilterLinear,		CPUF_SUPPORTS_INTEGER_SSE,	RowFactory<VDResamplerRowStageSeparableLinear8_phaseZeroStepHalf_ISSE> },
#endif

		{ +0x0000, 0x008000, kVDPixType_8,		nsVDPixmap::kFilterLinear,		0,							RowFactory<VDResamplerRowStageSeparableLinear8_phaseZeroStepHalf> },
	};

	long flags = CPUGetEnabledExtensions();
	uint32 type = mpSrc->GetType(mSrcIndex) & kVDPixType_Mask;

	for(int i=0; i<sizeof(kSpecialCaseSpanRoutines)/sizeof(kSpecialCaseSpanRoutines[0]); ++i) {
		const SpecialCaseSpanRoutine& rout = kSpecialCaseSpanRoutines[i];

		if (rout.mType != type)
			continue;

		if (x_2fc < 1.0)
			continue;

		if (rout.mStep != dudx)
			continue;

		if (rout.mPhase != u0)
			continue;

		if (rout.mFilterMode != filterMode)
			continue;

		if ((rout.mCPUFlags & flags) != rout.mCPUFlags)
			continue;

		mpRowStage = rout.mpClassFactory(x_2fc, filterFactor);
		mpRowStage2 = mpRowStage->AsRowStage2();
		break;
	}

	if (!mpRowStage) {
		struct SpanRoutine {
			uint32		mType;
			bool mbInterpOnly;
			nsVDPixmap::FilterMode mFilterMode;
			uint32 mCPUFlags;
			IVDResamplerSeparableRowStage *(*mpClassFactory)(double filterCutoff, float filterFactor);
		};
		
		static const SpanRoutine kSpanRoutines[]={
#if defined _M_IX86
			// X86
			{ kVDPixType_8888,		false,	nsVDPixmap::kFilterPoint,		CPUF_SUPPORTS_MMX,	RowFactory<VDResamplerSeparablePointRowStageMMX> },
			{ kVDPixType_8888,		false,	nsVDPixmap::kFilterPoint,		0,					RowFactory<VDResamplerSeparablePointRowStageX86> },
			{ kVDPixType_8,			false,	nsVDPixmap::kFilterLinear,		CPUF_SUPPORTS_SSE41,	RowFactoryLinear<VDResamplerSeparableTableRowStage8SSE41> },
			{ kVDPixType_8,			false,	nsVDPixmap::kFilterLinear,		CPUF_SUPPORTS_MMX,	RowFactoryLinear<VDResamplerSeparableTableRowStage8MMX> },
			{ kVDPixType_8888,		true,	nsVDPixmap::kFilterLinear,		CPUF_SUPPORTS_MMX,	RowFactory<VDResamplerSeparableLinearRowStageMMX> },
			{ kVDPixType_8888,		false,	nsVDPixmap::kFilterLinear,		CPUF_SUPPORTS_SSE2,	RowFactoryLinear<VDResamplerSeparableTableRowStageSSE2> },
			{ kVDPixType_8888,		false,	nsVDPixmap::kFilterLinear,		CPUF_SUPPORTS_MMX,	RowFactoryLinear<VDResamplerSeparableTableRowStageMMX> },
			{ kVDPixType_8,			false,	nsVDPixmap::kFilterCubic,		CPUF_SUPPORTS_SSE41,	RowFactoryCubic<VDResamplerSeparableTableRowStage8SSE41> },
			{ kVDPixType_8,			false,	nsVDPixmap::kFilterCubic,		CPUF_SUPPORTS_MMX,	RowFactoryCubic<VDResamplerSeparableTableRowStage8MMX> },
			{ kVDPixType_8888,		true,	nsVDPixmap::kFilterCubic,		CPUF_SUPPORTS_MMX,	RowFactoryCubic2<VDResamplerSeparableCubicRowStageMMX> },
			{ kVDPixType_8888,		false,	nsVDPixmap::kFilterCubic,		CPUF_SUPPORTS_SSE2,	RowFactoryCubic<VDResamplerSeparableTableRowStageSSE2> },
			{ kVDPixType_8888,		false,	nsVDPixmap::kFilterCubic,		CPUF_SUPPORTS_MMX,	RowFactoryCubic<VDResamplerSeparableTableRowStageMMX> },
			{ kVDPixType_8,			false,	nsVDPixmap::kFilterLanczos3,		CPUF_SUPPORTS_SSE41,	RowFactoryLanczos3<VDResamplerSeparableTableRowStage8SSE41> },
			{ kVDPixType_8,			false,	nsVDPixmap::kFilterLanczos3,	CPUF_SUPPORTS_MMX,	RowFactoryLanczos3<VDResamplerSeparableTableRowStage8MMX> },
			{ kVDPixType_8888,		false,	nsVDPixmap::kFilterLanczos3,	CPUF_SUPPORTS_SSE2,	RowFactoryLanczos3<VDResamplerSeparableTableRowStageSSE2> },
			{ kVDPixType_8888,		false,	nsVDPixmap::kFilterLanczos3,	CPUF_SUPPORTS_MMX,	RowFactoryLanczos3<VDResamplerSeparableTableRowStageMMX> },
#elif defined _M_AMD64
			// AMD64
			{ kVDPixType_8888,		false,	nsVDPixmap::kFilterLinear,		CPUF_SUPPORTS_SSE2,	RowFactoryLinear<VDResamplerSeparableTableRowStageSSE2> },
			{ kVDPixType_8888,		false,	nsVDPixmap::kFilterCubic,		CPUF_SUPPORTS_SSE2,	RowFactoryCubic<VDResamplerSeparableTableRowStageSSE2> },
			{ kVDPixType_8888,		false,	nsVDPixmap::kFilterLanczos3,	CPUF_SUPPORTS_SSE2,	RowFactoryLanczos3<VDResamplerSeparableTableRowStageSSE2> },
#endif
			// Generic
			{ kVDPixType_8,			false,	nsVDPixmap::kFilterPoint,		0,					RowFactory<VDResamplerRowStageSeparablePoint8> },
			{ kVDPixType_8888,		false,	nsVDPixmap::kFilterPoint,		0,					RowFactory<VDResamplerRowStageSeparablePoint32> },
			{ kVDPixType_8,			true,	nsVDPixmap::kFilterLinear,		0,					RowFactory<VDResamplerRowStageSeparableLinear8> },
			{ kVDPixType_8888,		true,	nsVDPixmap::kFilterLinear,		0,					RowFactory<VDResamplerRowStageSeparableLinear32> },
			{ kVDPixType_8,			false,	nsVDPixmap::kFilterLinear,		0,					RowFactoryLinear<VDResamplerRowStageSeparableTable8> },
			{ kVDPixType_8888,		false,	nsVDPixmap::kFilterLinear,		0,					RowFactoryLinear<VDResamplerRowStageSeparableTable32> },
			{ kVDPixType_32F_LE,	false,	nsVDPixmap::kFilterLinear,		0,					RowFactoryLinear<VDResamplerRowStageSeparableTable32F> },
			{ kVDPixType_32Fx4_LE,	false,	nsVDPixmap::kFilterLinear,		0,					RowFactoryLinear<VDResamplerRowStageSeparableTable32Fx4> },
			{ kVDPixType_8,			false,	nsVDPixmap::kFilterCubic,		0,					RowFactoryCubic<VDResamplerRowStageSeparableTable8> },
			{ kVDPixType_8888,		false,	nsVDPixmap::kFilterCubic,		0,					RowFactoryCubic<VDResamplerRowStageSeparableTable32> },
			{ kVDPixType_32F_LE,	false,	nsVDPixmap::kFilterCubic,		0,					RowFactoryCubic<VDResamplerRowStageSeparableTable32F> },
			{ kVDPixType_32Fx4_LE,	false,	nsVDPixmap::kFilterCubic,		0,					RowFactoryCubic<VDResamplerRowStageSeparableTable32Fx4> },
			{ kVDPixType_8,			false,	nsVDPixmap::kFilterLanczos3,	0,					RowFactoryLanczos3<VDResamplerRowStageSeparableTable8> },
			{ kVDPixType_8888,		false,	nsVDPixmap::kFilterLanczos3,	0,					RowFactoryLanczos3<VDResamplerRowStageSeparableTable32> },
			{ kVDPixType_32F_LE,	false,	nsVDPixmap::kFilterLanczos3,	0,					RowFactoryLanczos3<VDResamplerRowStageSeparableTable32F> },
			{ kVDPixType_32Fx4_LE,	false,	nsVDPixmap::kFilterLanczos3,	0,					RowFactoryLanczos3<VDResamplerRowStageSeparableTable32Fx4> },
		};

		for(int i=0; i<sizeof(kSpanRoutines)/sizeof(kSpanRoutines[0]); ++i) {
			const SpanRoutine& rout = kSpanRoutines[i];

			if (rout.mType != type)
				continue;

			if (rout.mbInterpOnly && x_2fc < 1.0)
				continue;

			if (rout.mFilterMode != filterMode)
				continue;

			if ((rout.mCPUFlags & flags) != rout.mCPUFlags)
				continue;

			mpRowStage = rout.mpClassFactory(x_2fc, filterFactor);
			mpRowStage2 = mpRowStage->AsRowStage2();
			break;
		}
	}

	VDASSERT(mpRowStage);

	mRowFiltW = mpRowStage->GetWindowSize();

	mpSrc->AddWindowRequest(0, 0);

	sint32 fsx1 = (sint32)(offset * 65536.0) - ((mRowFiltW-1) << 15);
	mAxis.Compute(width, fsx1, mSrcWidth, mRowFiltW);
	mWidth = width;

	switch(type) {
		case kVDPixType_8:
			mBytesPerSample = 1;
			break;
		case kVDPixType_8888:
		case kVDPixType_32F_LE:
			mBytesPerSample = 4;
			break;
		case kVDPixType_32Fx4_LE:
			mBytesPerSample = 16;
			break;

		default:
			VDASSERT(false);
	}
}

void VDPixmapGenResampleRow::Start() {
	StartWindow(mWidth * mBytesPerSample);

	uint32 clipSpace = ((mRowFiltW*3*mBytesPerSample + 15) >> 4) << 2;
	mTempSpace.resize(clipSpace);

	if (mpRowStage2)
		mpRowStage2->Init(mAxis, mSrcWidth);
}

void VDPixmapGenResampleRow::Compute(void *dst0, sint32 y) {
	switch(mBytesPerSample) {
		case 1:
			Compute8(dst0, y);
			break;
		case 4:
			Compute32(dst0, y);
			break;
		case 16:
			Compute128(dst0, y);
			break;
	}
}

void VDPixmapGenResampleRow::Compute8(void *dst0, sint32 y) {
	const uint8 *src = (const uint8 *)mpSrc->GetRow(y, mSrcIndex);
	uint8 *dst = (uint8 *)dst0;

	// process pre-copy region
	if (uint32 count = mAxis.dx_precopy) {
		VDMemset8(dst, src[0], count);
		dst += count;
	}

	uint8 *p = (uint8*)mTempSpace.data();
	sint32 u = mAxis.u;
	const sint32 dudx = mAxis.dudx;

	// process dual-clip region
	if (mpRowStage2) {
		uint32 count = mAxis.dx_preclip + mAxis.dx_active + mAxis.dx_postclip + mAxis.dx_dualclip;
		mpRowStage2->Process(dst, src, count);
		dst += count;
	} else if (uint32 count = mAxis.dx_dualclip) {
		VDMemset8(p, src[0], mRowFiltW);
		memcpy(p + mRowFiltW, src+1, (mSrcWidth-2));
		VDMemset8(p + mRowFiltW + (mSrcWidth-2), src[mSrcWidth-1], mRowFiltW);

		mpRowStage->Process(dst, p, count, u + ((mRowFiltW-1)<<16), dudx);
		u += dudx*count;
		dst += count;
	} else {
		// process pre-clip region
		if (uint32 count = mAxis.dx_preclip) {
			VDMemset8(p, src[0], mRowFiltW);
			memcpy(p + mRowFiltW, src+1, (mRowFiltW-1));

			mpRowStage->Process(dst, p, count, u + ((mRowFiltW-1)<<16), dudx);
			u += dudx*count;
			dst += count;
		}

		// process active region
		if (uint32 count = mAxis.dx_active) {
			mpRowStage->Process(dst, src, count, u, dudx);
			u += dudx*count;
			dst += count;
		}

		// process post-clip region
		if (uint32 count = mAxis.dx_postclip) {
			uint32 offset = mSrcWidth + 1 - mRowFiltW;

			memcpy(p, src+offset, (mRowFiltW-1));
			VDMemset8(p + (mRowFiltW-1), src[mSrcWidth-1], mRowFiltW);

			mpRowStage->Process(dst, p, count, u - (offset<<16), dudx);
			dst += count;
		}
	}

	// process post-copy region
	if (uint32 count = mAxis.dx_postcopy) {
		VDMemset8(dst, src[mSrcWidth-1], count);
	}
}

void VDPixmapGenResampleRow::Compute32(void *dst0, sint32 y) {
	const uint32 *src = (const uint32 *)mpSrc->GetRow(y, mSrcIndex);
	uint32 *dst = (uint32 *)dst0;

	// process pre-copy region
	if (uint32 count = mAxis.dx_precopy) {
		VDMemset32(dst, src[0], count);
		dst += count;
	}

	uint32 *p = mTempSpace.data();
	sint32 u = mAxis.u;
	const sint32 dudx = mAxis.dudx;

	// process dual-clip region
	if (uint32 count = mAxis.dx_dualclip) {
		VDMemset32(p, src[0], mRowFiltW);
		memcpy(p + mRowFiltW, src+1, (mSrcWidth-2)*sizeof(uint32));
		VDMemset32(p + mRowFiltW + (mSrcWidth-2), src[mSrcWidth-1], mRowFiltW);

		mpRowStage->Process(dst, p, count, u + ((mRowFiltW-1)<<16), dudx);
		u += dudx*count;
		dst += count;
	} else if (mpRowStage2) {
		mpRowStage2->Process(dst, p, mAxis.dx_preclip + mAxis.dx_active + mAxis.dx_postclip);
	} else {
		// process pre-clip region
		if (uint32 count = mAxis.dx_preclip) {
			VDMemset32(p, src[0], mRowFiltW);
			memcpy(p + mRowFiltW, src+1, (mRowFiltW-1)*sizeof(uint32));

			mpRowStage->Process(dst, p, count, u + ((mRowFiltW-1)<<16), dudx);
			u += dudx*count;
			dst += count;
		}

		// process active region
		if (uint32 count = mAxis.dx_active) {
			mpRowStage->Process(dst, src, count, u, dudx);
			u += dudx*count;
			dst += count;
		}

		// process post-clip region
		if (uint32 count = mAxis.dx_postclip) {
			uint32 offset = mSrcWidth + 1 - mRowFiltW;

			memcpy(p, src+offset, (mRowFiltW-1)*sizeof(uint32));
			VDMemset32(p + (mRowFiltW-1), src[mSrcWidth-1], mRowFiltW);

			mpRowStage->Process(dst, p, count, u - (offset<<16), dudx);
			dst += count;
		}
	}

	// process post-copy region
	if (uint32 count = mAxis.dx_postcopy) {
		VDMemset32(dst, src[mSrcWidth-1], count);
	}
}

void VDPixmapGenResampleRow::Compute128(void *dst0, sint32 y) {
	const uint32 *src = (const uint32 *)mpSrc->GetRow(y, mSrcIndex);
	uint32 *dst = (uint32 *)dst0;

	// process pre-copy region
	if (uint32 count = mAxis.dx_precopy) {
		VDMemset128(dst, src, count);
		dst += 4*count;
	}

	uint32 *p = mTempSpace.data();
	sint32 u = mAxis.u;
	const sint32 dudx = mAxis.dudx;

	// process dual-clip region
	if (uint32 count = mAxis.dx_dualclip) {
		VDMemset128(p, src, mRowFiltW);
		memcpy(p + 4*mRowFiltW, src+1, (mSrcWidth-2)*sizeof(uint32)*4);
		VDMemset128(p + 4*(mRowFiltW + (mSrcWidth-2)), src + 4*(mSrcWidth-1), mRowFiltW);

		mpRowStage->Process(dst, p, count, u + ((mRowFiltW-1)<<16), dudx);
		u += dudx*count;
		dst += count * 4;
	} else if (mpRowStage2) {
		mpRowStage2->Process(dst, p, mAxis.dx_preclip + mAxis.dx_active + mAxis.dx_postclip);
	} else {
		// process pre-clip region
		if (uint32 count = mAxis.dx_preclip) {
			VDMemset128(p, src, mRowFiltW);
			memcpy(p + 4*mRowFiltW, src+1, (mRowFiltW-1)*sizeof(uint32)*4);

			mpRowStage->Process(dst, p, count, u + ((mRowFiltW-1)<<16), dudx);
			u += dudx*count;
			dst += count*4;
		}

		// process active region
		if (uint32 count = mAxis.dx_active) {
			mpRowStage->Process(dst, src, count, u, dudx);
			u += dudx*count;
			dst += count*4;
		}

		// process post-clip region
		if (uint32 count = mAxis.dx_postclip) {
			uint32 offset = mSrcWidth + 1 - mRowFiltW;

			memcpy(p, src+offset*4, (mRowFiltW-1)*sizeof(uint32)*4);
			VDMemset128(p + 4*(mRowFiltW-1), src + 4*(mSrcWidth-1), mRowFiltW);

			mpRowStage->Process(dst, p, count, u - (offset<<16), dudx);
			dst += count*4;
		}
	}

	// process post-copy region
	if (uint32 count = mAxis.dx_postcopy) {
		VDMemset128(dst, src + 4*(mSrcWidth-1), count);
	}
}

///////////////////////////////////////////////////////////////////////////
//
// VDPixmapGenResampleCol
//
///////////////////////////////////////////////////////////////////////////

VDPixmapGenResampleCol::VDPixmapGenResampleCol()
	: mpColStage(NULL)
{
}

VDPixmapGenResampleCol::~VDPixmapGenResampleCol() {
	if (mpColStage)
		delete mpColStage;
}

void VDPixmapGenResampleCol::Init(IVDPixmapGen *src, uint32 srcIndex, uint32 height, float offset, float step, nsVDPixmap::FilterMode filterMode, float filterFactor, bool interpolationOnly) {
	InitSource(src, srcIndex);

	sint32 dvdy = (sint32)(step * 65536.0);

	mAxis.Init(dvdy);

	// construct stages
	double y_2fc = 1.0;
	if (!interpolationOnly && step > 1.0f)
		y_2fc = 1.0 / step;

	struct SpanRoutine {
		uint32 mType;
		bool mbInterpOnly;
		nsVDPixmap::FilterMode mFilterMode;
		uint32 mCPUFlags;
		IVDResamplerSeparableColStage *(*mpClassFactory)(double filterCutoff, float filterFactor);
	};
	
	static const SpanRoutine kSpanRoutines[]={
#if defined _M_IX86
		// X86
		{ kVDPixType_8,			false,	nsVDPixmap::kFilterLinear,		CPUF_SUPPORTS_SSE41,	ColFactoryLinear<VDResamplerSeparableTableColStage8SSE41> },
		{ kVDPixType_8,			false,	nsVDPixmap::kFilterLinear,		CPUF_SUPPORTS_MMX,	ColFactoryLinear<VDResamplerSeparableTableColStage8MMX> },
		{ kVDPixType_8888,		true,	nsVDPixmap::kFilterLinear,		CPUF_SUPPORTS_MMX,	ColFactory<VDResamplerSeparableLinearColStageMMX> },
		{ kVDPixType_8888,		false,	nsVDPixmap::kFilterLinear,		CPUF_SUPPORTS_SSE2,	ColFactoryLinear<VDResamplerSeparableTableColStageSSE2> },
		{ kVDPixType_8888,		false,	nsVDPixmap::kFilterLinear,		CPUF_SUPPORTS_MMX,	ColFactoryLinear<VDResamplerSeparableTableColStageMMX> },
		{ kVDPixType_8,			false,	nsVDPixmap::kFilterCubic,		CPUF_SUPPORTS_SSE41,	ColFactoryCubic<VDResamplerSeparableTableColStage8SSE41> },
		{ kVDPixType_8,			false,	nsVDPixmap::kFilterCubic,		CPUF_SUPPORTS_MMX,	ColFactoryCubic<VDResamplerSeparableTableColStage8MMX> },
		{ kVDPixType_8888,		true,	nsVDPixmap::kFilterCubic,		CPUF_SUPPORTS_SSE2,	ColFactoryCubic2<VDResamplerSeparableCubicColStageSSE2> },
		{ kVDPixType_8888,		true,	nsVDPixmap::kFilterCubic,		CPUF_SUPPORTS_MMX,	ColFactoryCubic2<VDResamplerSeparableCubicColStageMMX> },
		{ kVDPixType_8888,		false,	nsVDPixmap::kFilterCubic,		CPUF_SUPPORTS_SSE2,	ColFactoryCubic<VDResamplerSeparableTableColStageSSE2> },
		{ kVDPixType_8888,		false,	nsVDPixmap::kFilterCubic,		CPUF_SUPPORTS_MMX,	ColFactoryCubic<VDResamplerSeparableTableColStageMMX> },
		{ kVDPixType_8,			false,	nsVDPixmap::kFilterLanczos3,	CPUF_SUPPORTS_SSE41,	ColFactoryLanczos3<VDResamplerSeparableTableColStage8SSE41> },
		{ kVDPixType_8,			false,	nsVDPixmap::kFilterLanczos3,	CPUF_SUPPORTS_MMX,	ColFactoryLanczos3<VDResamplerSeparableTableColStage8MMX> },
		{ kVDPixType_8888,		false,	nsVDPixmap::kFilterLanczos3,	CPUF_SUPPORTS_SSE2,	ColFactoryLanczos3<VDResamplerSeparableTableColStageSSE2> },
		{ kVDPixType_8888,		false,	nsVDPixmap::kFilterLanczos3,	CPUF_SUPPORTS_MMX,	ColFactoryLanczos3<VDResamplerSeparableTableColStageMMX> },
#elif defined _M_AMD64
		// AMD64
		{ kVDPixType_8888,		false,	nsVDPixmap::kFilterLinear,		CPUF_SUPPORTS_SSE2,	ColFactoryLinear<VDResamplerSeparableTableColStageSSE2> },
		{ kVDPixType_8888,		false,	nsVDPixmap::kFilterCubic,		CPUF_SUPPORTS_SSE2,	ColFactoryCubic<VDResamplerSeparableTableColStageSSE2> },
		{ kVDPixType_8888,		false,	nsVDPixmap::kFilterLanczos3,	CPUF_SUPPORTS_SSE2,	ColFactoryLanczos3<VDResamplerSeparableTableColStageSSE2> },
#endif
		// Generic
		{ kVDPixType_8,			true,	nsVDPixmap::kFilterLinear,		0,					ColFactory<VDResamplerColStageSeparableLinear8> },
		{ kVDPixType_8888,		true,	nsVDPixmap::kFilterLinear,		0,					ColFactory<VDResamplerColStageSeparableLinear32> },
		{ kVDPixType_8,			false,	nsVDPixmap::kFilterLinear,		0,					ColFactoryLinear<VDResamplerColStageSeparableTable8> },
		{ kVDPixType_8888,		false,	nsVDPixmap::kFilterLinear,		0,					ColFactoryLinear<VDResamplerColStageSeparableTable32> },
		{ kVDPixType_32F_LE,	false,	nsVDPixmap::kFilterLinear,		0,					ColFactoryLinear<VDResamplerColStageSeparableTable32F> },
		{ kVDPixType_32Fx4_LE,	false,	nsVDPixmap::kFilterLinear,		0,					ColFactoryLinear<VDResamplerColStageSeparableTable32Fx4> },
		{ kVDPixType_8,			false,	nsVDPixmap::kFilterCubic,		0,					ColFactoryCubic<VDResamplerColStageSeparableTable8> },
		{ kVDPixType_8888,		false,	nsVDPixmap::kFilterCubic,		0,					ColFactoryCubic<VDResamplerColStageSeparableTable32> },
		{ kVDPixType_32F_LE,	false,	nsVDPixmap::kFilterCubic,		0,					ColFactoryCubic<VDResamplerColStageSeparableTable32F> },
		{ kVDPixType_32Fx4_LE,	false,	nsVDPixmap::kFilterCubic,		0,					ColFactoryCubic<VDResamplerColStageSeparableTable32Fx4> },
		{ kVDPixType_8,			false,	nsVDPixmap::kFilterLanczos3,	0,					ColFactoryLanczos3<VDResamplerColStageSeparableTable8> },
		{ kVDPixType_8888,		false,	nsVDPixmap::kFilterLanczos3,	0,					ColFactoryLanczos3<VDResamplerColStageSeparableTable32> },
		{ kVDPixType_32F_LE,	false,	nsVDPixmap::kFilterLanczos3,	0,					ColFactoryLanczos3<VDResamplerColStageSeparableTable32F> },
		{ kVDPixType_32Fx4_LE,	false,	nsVDPixmap::kFilterLanczos3,	0,					ColFactoryLanczos3<VDResamplerColStageSeparableTable32Fx4> },
	};

	long flags = CPUGetEnabledExtensions();
	uint32 type = src->GetType(srcIndex) & kVDPixType_Mask;
	for(int i=0; i<sizeof(kSpanRoutines)/sizeof(kSpanRoutines[0]); ++i) {
		const SpanRoutine& rout = kSpanRoutines[i];

		if (rout.mType != type)
			continue;

		if (rout.mbInterpOnly && y_2fc < 1.0)
			continue;

		if (rout.mFilterMode != filterMode)
			continue;

		if ((rout.mCPUFlags & flags) != rout.mCPUFlags)
			continue;

		mpColStage = rout.mpClassFactory(y_2fc, filterFactor);
		break;
	}

	mWinSize = mpColStage ? mpColStage->GetWindowSize() : 1;
	mWindow.resize(mWinSize);

	int delta = (mWinSize + 1) >> 1;
	mpSrc->AddWindowRequest(-delta, delta);

	sint32 fsy1 = (sint32)(offset * 65536.0) - ((mWinSize-1)<<15);
	mAxis.Compute(height, fsy1, mSrcHeight, mWinSize);
	mHeight = height;

	switch(type) {
		case kVDPixType_8:
			mBytesPerSample = 1;
			break;
		case kVDPixType_8888:
		case kVDPixType_32F_LE:
			mBytesPerSample = 4;
			break;
		case kVDPixType_32Fx4_LE:
			mBytesPerSample = 16;
			break;

		default:
			VDASSERT(false);
	}
}

void VDPixmapGenResampleCol::Start() {
	mBytesPerRow = mWidth * mBytesPerSample;
	StartWindow(mBytesPerRow);
}

void VDPixmapGenResampleCol::Compute(void *dst0, sint32 y) {
	const uint32 winsize = mWinSize;
	const uint32 dx = mSrcWidth;

	y -= (sint32)mAxis.dx_precopy;

	if (y < 0) {
		const void *srcrow0 = mpSrc->GetRow(0, mSrcIndex);
		memcpy(dst0, srcrow0, mBytesPerRow);
		return;
	}

	uint32 midrange = mAxis.dx_preclip + mAxis.dx_active + mAxis.dx_postclip + mAxis.dx_dualclip;

	if (y < (sint32)midrange) {
		sint32 v = mAxis.u + mAxis.dudx * y;

		if (mpColStage) {
			for(uint32 i=0; i<winsize; ++i) {
				int sy = (v >> 16) + i;

				if ((unsigned)sy >= (unsigned)mSrcHeight)
					sy = (~sy >> 31) & (mSrcHeight - 1);

				mWindow[i] = mpSrc->GetRow(sy, mSrcIndex);
			}

			mpColStage->Process(dst0, mWindow.data(), dx, v);
		} else
			memcpy(dst0, mpSrc->GetRow(v >> 16, mSrcIndex), mBytesPerRow);
		return;
	}

	const void *p = mpSrc->GetRow(mSrcHeight - 1, mSrcIndex);

	memcpy(dst0, p, mBytesPerRow);
}
