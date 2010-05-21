#ifndef f_VD2_KASUMI_RESAMPLE_STAGES_X86
#define f_VD2_KASUMI_RESAMPLE_STAGES_X86

#include "resample_stages_reference.h"

///////////////////////////////////////////////////////////////////////////
//
// resampler stages (scalar, x86)
//
///////////////////////////////////////////////////////////////////////////

class VDResamplerSeparablePointRowStageX86 : public IVDResamplerSeparableRowStage {
public:
	int GetWindowSize() const;
	void Process(void *dst, const void *src, uint32 w, uint32 u, uint32 dudx);
};

///////////////////////////////////////////////////////////////////////////
//
// resampler stages (MMX, x86)
//
///////////////////////////////////////////////////////////////////////////

class VDResamplerSeparablePointRowStageMMX : public IVDResamplerSeparableRowStage {
public:
	int GetWindowSize() const;
	void Process(void *dst, const void *src, uint32 w, uint32 u, uint32 dudx);
};

class VDResamplerSeparableLinearRowStageMMX : public IVDResamplerSeparableRowStage {
public:
	int GetWindowSize() const;
	void Process(void *dst0, const void *src0, uint32 w, uint32 u, uint32 dudx);
};

class VDResamplerSeparableLinearColStageMMX : public IVDResamplerSeparableColStage {
public:
	int GetWindowSize() const;
	void Process(void *dst0, const void *const *srcarray, uint32 w, sint32 phase);
};

class VDResamplerSeparableCubicRowStageMMX : public IVDResamplerSeparableRowStage {
public:
	VDResamplerSeparableCubicRowStageMMX(double A);

	int GetWindowSize() const;
	void Process(void *dst0, const void *src0, uint32 w, uint32 u, uint32 dudx);

protected:
	vdblock<sint32, vdaligned_alloc<sint32> > mFilterBank;
};

class VDResamplerSeparableCubicColStageMMX : public IVDResamplerSeparableColStage {
public:
	VDResamplerSeparableCubicColStageMMX(double A);

	int GetWindowSize() const;
	void Process(void *dst0, const void *const *srcarray, uint32 w, sint32 phase);

protected:
	vdblock<sint32, vdaligned_alloc<sint32> > mFilterBank;
};

class VDResamplerSeparableTableRowStage8MMX : public VDResamplerRowStageSeparableTable32, public IVDResamplerSeparableRowStage2 {
public:
	VDResamplerSeparableTableRowStage8MMX(const IVDResamplerFilter& filter);

	IVDResamplerSeparableRowStage2 *AsRowStage2() { return this; } 

	void Init(const VDResamplerAxis& axis, uint32 srcw);
	void Process(void *dst, const void *src, uint32 w);
	void Process(void *dst, const void *src, uint32 w, uint32 u, uint32 dudx);

protected:
	void RedoRowFilters(const VDResamplerAxis& axis, uint32 w, uint32 srcw);

	int		mAlignedKernelWidth;
	int		mAlignedKernelSize;
	ptrdiff_t	mRowKernelSize;
	uint32	mLastSrcWidth;
	uint32	mLastDstWidth;
	sint32	mLastU;
	sint32	mLastDUDX;

	bool	mbQuadOptimizationEnabled[4];
	int		mKernelSizeByOffset[4];
	ptrdiff_t	mTailOffset[4];

	vdfastvector<sint16, vdaligned_alloc<sint16> > mRowKernels;
};

class VDResamplerSeparableTableRowStageMMX : public VDResamplerRowStageSeparableTable32 {
public:
	VDResamplerSeparableTableRowStageMMX(const IVDResamplerFilter& filter);

	void Process(void *dst, const void *src, uint32 w, uint32 u, uint32 dudx);
};

class VDResamplerSeparableTableColStage8MMX : public VDResamplerColStageSeparableTable8 {
public:
	VDResamplerSeparableTableColStage8MMX(const IVDResamplerFilter& filter);

	void Process(void *dst, const void *const *src, uint32 w, sint32 phase);
};

class VDResamplerSeparableTableColStageMMX : public VDResamplerColStageSeparableTable32 {
public:
	VDResamplerSeparableTableColStageMMX(const IVDResamplerFilter& filter);

	void Process(void *dst, const void *const *src, uint32 w, sint32 phase);
};

///////////////////////////////////////////////////////////////////////////
//
// resampler stages (ISSE, x86)
//
///////////////////////////////////////////////////////////////////////////

class VDResamplerRowStageSeparableLinear8_phaseZeroStepHalf_ISSE : public VDResamplerRowStageSeparableLinear8 {
public:
	void Process(void *dst0, const void *src0, uint32 w, uint32 u, uint32 dudx);
};


///////////////////////////////////////////////////////////////////////////
//
// resampler stages (SSE2, x86)
//
///////////////////////////////////////////////////////////////////////////

class VDResamplerSeparableCubicColStageSSE2 : public VDResamplerSeparableCubicColStageMMX {
public:
	VDResamplerSeparableCubicColStageSSE2(double A);

	void Process(void *dst0, const void *const *srcarray, uint32 w, sint32 phase);
};

class VDResamplerSeparableTableRowStageSSE2 : public VDResamplerSeparableTableRowStageMMX {
public:
	VDResamplerSeparableTableRowStageSSE2(const IVDResamplerFilter& filter);

	void Process(void *dst, const void *src, uint32 w, uint32 u, uint32 dudx);
};

class VDResamplerSeparableTableColStageSSE2 : public VDResamplerSeparableTableColStageMMX {
public:
	VDResamplerSeparableTableColStageSSE2(const IVDResamplerFilter& filter);

	void Process(void *dst, const void *const *src, uint32 w, sint32 phase);
};

///////////////////////////////////////////////////////////////////////////
//
// resampler stages (SSE4.1, x86)
//
///////////////////////////////////////////////////////////////////////////

class VDResamplerSeparableTableRowStage8SSE41 : public VDResamplerRowStageSeparableTable32, public IVDResamplerSeparableRowStage2 {
public:
	VDResamplerSeparableTableRowStage8SSE41(const IVDResamplerFilter& filter);

	IVDResamplerSeparableRowStage2 *AsRowStage2() { return this; } 

	void Init(const VDResamplerAxis& axis, uint32 srcw);
	void Process(void *dst, const void *src, uint32 w);
	void Process(void *dst, const void *src, uint32 w, uint32 u, uint32 dudx);

protected:
	void RedoRowFilters(const VDResamplerAxis& axis, uint32 w, uint32 srcw);

	int		mAlignedKernelWidth;
	int		mAlignedKernelSize;
	ptrdiff_t	mRowKernelSize;
	uint32	mLastSrcWidth;
	uint32	mLastDstWidth;
	sint32	mLastU;
	sint32	mLastDUDX;

	bool	mbQuadOptimizationEnabled[8];
	int		mKernelSizeByOffset[8];
	ptrdiff_t	mTailOffset[8];

	vdfastvector<sint16, vdaligned_alloc<sint16> > mRowKernels;
};

class VDResamplerSeparableTableColStage8SSE41 : public VDResamplerColStageSeparableTable8 {
public:
	VDResamplerSeparableTableColStage8SSE41(const IVDResamplerFilter& filter);

	void Process(void *dst, const void *const *src, uint32 w, sint32 phase);
};

#endif
