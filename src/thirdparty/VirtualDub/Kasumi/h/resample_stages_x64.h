#ifndef f_VD2_KASUMI_RESAMPLE_STAGES_X64_H
#define f_VD2_KASUMI_RESAMPLE_STAGES_X64_H

#include "resample_stages_reference.h"

///////////////////////////////////////////////////////////////////////////
//
// resampler stages (SSE2, AMD64)
//
///////////////////////////////////////////////////////////////////////////

class VDResamplerSeparableTableRowStageSSE2 : public VDResamplerRowStageSeparableTable32 {
public:
	VDResamplerSeparableTableRowStageSSE2(const IVDResamplerFilter& filter);

	void Process(void *dst, const void *src, uint32 w, uint32 u, uint32 dudx);
};

class VDResamplerSeparableTableColStageSSE2 : public VDResamplerColStageSeparableTable32 {
public:
	VDResamplerSeparableTableColStageSSE2(const IVDResamplerFilter& filter);

	void Process(void *dst, const void *const *src, uint32 w, sint32 phase);
};

#endif
