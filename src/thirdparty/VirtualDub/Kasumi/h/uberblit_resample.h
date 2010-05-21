#ifndef f_VD2_KASUMI_UBERBLIT_RESAMPLE_H
#define f_VD2_KASUMI_UBERBLIT_RESAMPLE_H

#include <vd2/system/vdstl.h>
#include <vd2/system/math.h>
#include "uberblit.h"
#include "uberblit_base.h"
#include <vd2/Kasumi/resample_kernels.h>

class IVDResamplerSeparableRowStage;
class IVDResamplerSeparableRowStage2;
class IVDResamplerSeparableColStage;

namespace nsVDPixmap {
	enum FilterMode {
		kFilterPoint,
		kFilterLinear,
		kFilterCubic,
		kFilterLanczos3,
		kFilterCount
	};
}

class VDPixmapGenResampleRow : public VDPixmapGenWindowBasedOneSource {
public:
	VDPixmapGenResampleRow();
	~VDPixmapGenResampleRow();

	void Init(IVDPixmapGen *src, uint32 srcIndex, uint32 width, float offset, float step, nsVDPixmap::FilterMode filterMode, float filterFactor, bool interpolationOnly);

	void Start();

	uint32 GetType(uint32 output) const {
		return mpSrc->GetType(mSrcIndex);
	}

protected:
	void Compute(void *dst0, sint32 y);
	void Compute8(void *dst0, sint32 y);
	void Compute32(void *dst0, sint32 y);
	void Compute128(void *dst0, sint32 y);

	IVDResamplerSeparableRowStage *mpRowStage;
	IVDResamplerSeparableRowStage2 *mpRowStage2;

	uint32				mRowFiltW;
	uint32				mBytesPerSample;

	VDResamplerAxis		mAxis;

	vdblock<void *>	mWindow;
	void				**mpAllocWindow;
	vdblock<uint32, vdaligned_alloc<uint32> >		mTempSpace;
};

class VDPixmapGenResampleCol : public VDPixmapGenWindowBasedOneSource {
public:
	VDPixmapGenResampleCol();
	~VDPixmapGenResampleCol();

	void Init(IVDPixmapGen *src, uint32 srcIndex, uint32 height, float offset, float step, nsVDPixmap::FilterMode filterMode, float filterFactor, bool interpolationOnly);

	void Start();

	uint32 GetType(uint32 output) const {
		return mpSrc->GetType(mSrcIndex);
	}

protected:
	void Compute(void *dst0, sint32 y);

	IVDResamplerSeparableColStage *mpColStage;

	uint32				mWinSize;
	uint32				mBytesPerSample;
	uint32				mBytesPerRow;

	VDResamplerAxis		mAxis;

	vdblock<const void *>	mWindow;
};

#endif
