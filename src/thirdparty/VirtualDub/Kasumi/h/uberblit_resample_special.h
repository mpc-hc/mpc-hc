#ifndef f_VD2_KASUMI_UBERBLIT_RESAMPLE_SPECIAL_H
#define f_VD2_KASUMI_UBERBLIT_RESAMPLE_SPECIAL_H

#include <vd2/system/vdstl.h>
#include <vd2/system/math.h>
#include "uberblit.h"
#include "uberblit_base.h"

class VDPixmapGenResampleRow_d2_p0_lin_u8 : public VDPixmapGenWindowBasedOneSource {
public:
	void Init(IVDPixmapGen *src, uint32 srcIndex);
	void Start();

protected:
	void Compute(void *dst0, sint32 y);
};

class VDPixmapGenResampleRow_d4_p0_lin_u8 : public VDPixmapGenWindowBasedOneSource {
public:
	void Init(IVDPixmapGen *src, uint32 srcIndex);
	void Start();

protected:
	void Compute(void *dst0, sint32 y);
};

class VDPixmapGenResampleRow_x2_p0_lin_u8 : public VDPixmapGenWindowBasedOneSource {
public:
	void Init(IVDPixmapGen *src, uint32 srcIndex);
	void Start();

protected:
	void Compute(void *dst0, sint32 y);
};

class VDPixmapGenResampleRow_x4_p0_lin_u8 : public VDPixmapGenWindowBasedOneSource {
public:
	void Init(IVDPixmapGen *src, uint32 srcIndex);
	void Start();

protected:
	void Compute(void *dst0, sint32 y);
};

class VDPixmapGenResampleCol_x2_phalf_lin_u8: public VDPixmapGenWindowBasedOneSource {
public:
	void Init(IVDPixmapGen *src, uint32 srcIndex);
	void Start();

protected:
	void Compute(void *dst0, sint32 y);
};

class VDPixmapGenResampleCol_x4_p1half_lin_u8: public VDPixmapGenWindowBasedOneSource {
public:
	void Init(IVDPixmapGen *src, uint32 srcIndex);
	void Start();

protected:
	void Compute(void *dst0, sint32 y);
};

class VDPixmapGenResampleCol_d2_pnqrtr_lin_u8: public VDPixmapGenWindowBasedOneSource {
public:
	void Init(IVDPixmapGen *src, uint32 srcIndex);
	void Start();

protected:
	void Compute(void *dst0, sint32 y);
};

class VDPixmapGenResampleCol_d4_pn38_lin_u8: public VDPixmapGenWindowBasedOneSource {
public:
	void Init(IVDPixmapGen *src, uint32 srcIndex);
	void Start();

protected:
	void Compute(void *dst0, sint32 y);
};

#endif
