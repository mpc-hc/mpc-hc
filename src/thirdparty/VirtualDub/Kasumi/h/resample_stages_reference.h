#ifndef f_VD2_KASUMI_RESAMPLE_STAGES_REFERENCE_H
#define f_VD2_KASUMI_RESAMPLE_STAGES_REFERENCE_H

#include <vd2/system/vdstl.h>
#include "resample_stages.h"

///////////////////////////////////////////////////////////////////////////
//
// resampler stages (portable)
//
///////////////////////////////////////////////////////////////////////////

class VDResamplerRowStageSeparablePoint8 : public IVDResamplerSeparableRowStage {
public:
	int GetWindowSize() const;
	void Process(void *dst0, const void *src0, uint32 w, uint32 u, uint32 dudx);
};

class VDResamplerRowStageSeparablePoint16 : public IVDResamplerSeparableRowStage {
public:
	int GetWindowSize() const;
	void Process(void *dst0, const void *src0, uint32 w, uint32 u, uint32 dudx);
};

class VDResamplerRowStageSeparablePoint32 : public IVDResamplerSeparableRowStage {
public:
	int GetWindowSize() const;
	void Process(void *dst0, const void *src0, uint32 w, uint32 u, uint32 dudx);
};

class VDResamplerRowStageSeparableLinear8 : public IVDResamplerSeparableRowStage {
public:
	int GetWindowSize() const;
	virtual void Process(void *dst0, const void *src0, uint32 w, uint32 u, uint32 dudx);
};

class VDResamplerRowStageSeparableLinear8_phaseZeroStepHalf : public VDResamplerRowStageSeparableLinear8 {
public:
	void Process(void *dst0, const void *src0, uint32 w, uint32 u, uint32 dudx);
};

class VDResamplerRowStageSeparableLinear32 : public IVDResamplerSeparableRowStage {
public:
	int GetWindowSize() const;
	void Process(void *dst0, const void *src0, uint32 w, uint32 u, uint32 dudx);
};

class VDResamplerColStageSeparableLinear8 : public IVDResamplerSeparableColStage {
public:
	int GetWindowSize() const;
	void Process(void *dst0, const void *const *srcarray, uint32 w, sint32 phase);
};

class VDResamplerColStageSeparableLinear32 : public IVDResamplerSeparableColStage {
public:
	int GetWindowSize() const;
	void Process(void *dst0, const void *const *srcarray, uint32 w, sint32 phase);
};

class VDResamplerRowStageSeparableTable8 : public IVDResamplerSeparableRowStage {
public:
	VDResamplerRowStageSeparableTable8(const IVDResamplerFilter& filter);

	int GetWindowSize() const;

	void Process(void *dst0, const void *src0, uint32 w, uint32 u, uint32 dudx);

protected:
	vdblock<sint32, vdaligned_alloc<sint32> >	mFilterBank;
};

class VDResamplerRowStageSeparableTable32 : public IVDResamplerSeparableRowStage {
public:
	VDResamplerRowStageSeparableTable32(const IVDResamplerFilter& filter);

	int GetWindowSize() const;

	void Process(void *dst0, const void *src0, uint32 w, uint32 u, uint32 dudx);

protected:
	vdblock<sint32, vdaligned_alloc<sint32> >	mFilterBank;
};

class VDResamplerRowStageSeparableTable32F : public IVDResamplerSeparableRowStage {
public:
	VDResamplerRowStageSeparableTable32F(const IVDResamplerFilter& filter);

	int GetWindowSize() const;

	void Process(void *dst0, const void *src0, uint32 w, uint32 u, uint32 dudx);

protected:
	vdblock<float, vdaligned_alloc<float> >	mFilterBank;
};

class VDResamplerRowStageSeparableTable32Fx4 : public IVDResamplerSeparableRowStage {
public:
	VDResamplerRowStageSeparableTable32Fx4(const IVDResamplerFilter& filter);

	int GetWindowSize() const;

	void Process(void *dst0, const void *src0, uint32 w, uint32 u, uint32 dudx);

protected:
	vdblock<float, vdaligned_alloc<float> >	mFilterBank;
};

class VDResamplerColStageSeparableTable8 : public IVDResamplerSeparableColStage {
public:
	VDResamplerColStageSeparableTable8(const IVDResamplerFilter& filter);

	int GetWindowSize() const;

	void Process(void *dst0, const void *const *src0, uint32 w, sint32 phase);

protected:
	vdblock<sint32, vdaligned_alloc<sint32> >	mFilterBank;
};

class VDResamplerColStageSeparableTable32 : public IVDResamplerSeparableColStage {
public:
	VDResamplerColStageSeparableTable32(const IVDResamplerFilter& filter);

	int GetWindowSize() const;

	void Process(void *dst0, const void *const *src0, uint32 w, sint32 phase);

protected:
	vdblock<sint32, vdaligned_alloc<sint32> >	mFilterBank;
};

class VDResamplerColStageSeparableTable32F : public IVDResamplerSeparableColStage {
public:
	VDResamplerColStageSeparableTable32F(const IVDResamplerFilter& filter);

	int GetWindowSize() const;

	void Process(void *dst0, const void *const *src0, uint32 w, sint32 phase);

protected:
	vdblock<float, vdaligned_alloc<float> >	mFilterBank;
};

class VDResamplerColStageSeparableTable32Fx4 : public IVDResamplerSeparableColStage {
public:
	VDResamplerColStageSeparableTable32Fx4(const IVDResamplerFilter& filter);

	int GetWindowSize() const;

	void Process(void *dst0, const void *const *src0, uint32 w, sint32 phase);

protected:
	vdblock<float, vdaligned_alloc<float> >	mFilterBank;
};

#endif
