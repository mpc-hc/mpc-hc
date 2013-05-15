#include "stdafx.h" // MPC-HC patch
#include "resample_stages_x64.h"

extern "C" long vdasm_resize_table_col_SSE2(uint32 *out, const uint32 *const*in_table, const int *filter, int filter_width, uint32 w);
extern "C" long vdasm_resize_table_row_SSE2(uint32 *out, const uint32 *in, const int *filter, int filter_width, uint32 w, long accum, long frac);

VDResamplerSeparableTableRowStageSSE2::VDResamplerSeparableTableRowStageSSE2(const IVDResamplerFilter& filter)
	: VDResamplerRowStageSeparableTable32(filter)
{
	VDResamplerSwizzleTable(mFilterBank.data(), (uint32)mFilterBank.size() >> 1);
}

void VDResamplerSeparableTableRowStageSSE2::Process(void *dst, const void *src, uint32 w, uint32 u, uint32 dudx) {
	vdasm_resize_table_row_SSE2((uint32 *)dst, (const uint32 *)src, (const int *)mFilterBank.data(), (int)mFilterBank.size() >> 8, w, u, dudx);
}

VDResamplerSeparableTableColStageSSE2::VDResamplerSeparableTableColStageSSE2(const IVDResamplerFilter& filter)
	: VDResamplerColStageSeparableTable32(filter)
{
	VDResamplerSwizzleTable(mFilterBank.data(), (uint32)mFilterBank.size() >> 1);
}

void VDResamplerSeparableTableColStageSSE2::Process(void *dst, const void *const *src, uint32 w, sint32 phase) {
	const unsigned filtSize = (unsigned)mFilterBank.size() >> 8;

	vdasm_resize_table_col_SSE2((uint32*)dst, (const uint32 *const *)src, (const int *)mFilterBank.data() + filtSize*((phase >> 8) & 0xff), filtSize, w);
}
