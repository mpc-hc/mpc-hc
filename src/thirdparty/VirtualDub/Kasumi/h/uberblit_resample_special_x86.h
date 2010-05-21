#ifndef f_VD2_KASUMI_UBERBLIT_RESAMPLE_SPECIAL_X86_H
#define f_VD2_KASUMI_UBERBLIT_RESAMPLE_SPECIAL_X86_H

#include "uberblit_resample_special.h"

class VDPixmapGenResampleRow_x2_p0_lin_u8_ISSE : public VDPixmapGenResampleRow_x2_p0_lin_u8 {
protected:
	void Compute(void *dst0, sint32 y);
};

class VDPixmapGenResampleRow_x4_p0_lin_u8_MMX : public VDPixmapGenResampleRow_x4_p0_lin_u8 {
protected:
	void Compute(void *dst0, sint32 y);
};

class VDPixmapGenResampleCol_d2_pnqrtr_lin_u8_ISSE: public VDPixmapGenResampleCol_d2_pnqrtr_lin_u8 {
protected:
	void Compute(void *dst0, sint32 y);
};

class VDPixmapGenResampleCol_d4_pn38_lin_u8_ISSE: public VDPixmapGenResampleCol_d4_pn38_lin_u8 {
protected:
	void Compute(void *dst0, sint32 y);
};

#endif
