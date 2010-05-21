#ifndef f_VD2_KASUMI_RESAMPLE_KERNELS_H
#define f_VD2_KASUMI_RESAMPLE_KERNELS_H

#include <vd2/system/vdtypes.h>
#include <vd2/Kasumi/pixmap.h>

struct VDResamplerAxis {
	sint32		dx;
	sint32		u;
	sint32		dudx;
	uint32		dx_precopy;
	uint32		dx_preclip;
	uint32		dx_active;
	uint32		dx_postclip;
	uint32		dx_postcopy;
	uint32		dx_dualclip;

	void Init(sint32 dudx);
	void Compute(sint32 count, sint32 u0, sint32 w, sint32 kernel_width);
};


///////////////////////////////////////////////////////////////////////////
//
// filter kernels
//
///////////////////////////////////////////////////////////////////////////

class IVDResamplerFilter {
public:
	virtual ~IVDResamplerFilter() {}

	virtual int GetFilterWidth() const = 0;
	virtual double EvaluateFilter(double offset) const = 0;
	virtual void GenerateFilter(float *dst, double offset) const = 0;
	virtual void GenerateFilterBank(float *dst) const = 0;
};

class VDResamplerLinearFilter : public IVDResamplerFilter {
public:
	VDResamplerLinearFilter(double twofc);

	int GetFilterWidth() const;

	double EvaluateFilter(double offset) const;
	void GenerateFilter(float *dst, double offset) const;
	void GenerateFilterBank(float *dst) const;

protected:
	double		mScale;
	unsigned	mTaps;
};

class VDResamplerCubicFilter : public IVDResamplerFilter {
public:
	VDResamplerCubicFilter(double twofc, double A);

	int GetFilterWidth() const;

	double EvaluateFilter(double offset) const;
	void GenerateFilter(float *dst, double offset) const;
	void GenerateFilterBank(float *dst) const;

protected:
	double		mScale;
	double		mA0;
	double		mA2;
	double		mA3;
	double		mB0;
	double		mB1;
	double		mB2;
	double		mB3;
	unsigned	mTaps;
};

class VDResamplerLanczos3Filter : public IVDResamplerFilter {
public:
	VDResamplerLanczos3Filter(double twofc);

	int GetFilterWidth() const;

	double EvaluateFilter(double offset) const;
	void GenerateFilter(float *dst, double offset) const;
	void GenerateFilterBank(float *dst) const;

protected:
	double		mScale;
	unsigned	mTaps;
};

#endif
