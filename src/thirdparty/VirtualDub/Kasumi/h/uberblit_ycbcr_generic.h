#ifndef f_VD2_KASUMI_UBERBLIT_YCBCR_GENERIC_H
#define f_VD2_KASUMI_UBERBLIT_YCBCR_GENERIC_H

#include "uberblit_ycbcr.h"

struct VDPixmapGenYCbCrBasis {
	float mKr;
	float mKb;
	float mToRGB[2][3];
};

extern const VDPixmapGenYCbCrBasis g_VDPixmapGenYCbCrBasis_601;
extern const VDPixmapGenYCbCrBasis g_VDPixmapGenYCbCrBasis_709;

////////////////////////////////////////////////////////////////////////////

class VDPixmapGenYCbCrToRGB32Generic : public VDPixmapGenYCbCrToRGB32Base {
public:
	VDPixmapGenYCbCrToRGB32Generic(const VDPixmapGenYCbCrBasis& basis, bool studioRGB);

	uint32 GetType(uint32 output) const;

protected:
	virtual void Compute(void *dst0, sint32 y);

	sint32 mCoY;
	sint32 mCoRCr;
	sint32 mCoGCr;
	sint32 mCoGCb;
	sint32 mCoBCb;
	sint32 mBiasR;
	sint32 mBiasG;
	sint32 mBiasB;
};

////////////////////////////////////////////////////////////////////////////

class VDPixmapGenYCbCrToRGB32FGeneric : public VDPixmapGenYCbCrToRGB32FBase {
public:
	VDPixmapGenYCbCrToRGB32FGeneric(const VDPixmapGenYCbCrBasis& basis);

	uint32 GetType(uint32 output) const;

protected:
	void Compute(void *dst0, sint32 y);

	float mCoRCr;
	float mCoGCr;
	float mCoGCb;
	float mCoBCb;
};

////////////////////////////////////////////////////////////////////////////

class VDPixmapGenRGB32ToYCbCrGeneric : public VDPixmapGenRGB32ToYCbCrBase {
public:
	VDPixmapGenRGB32ToYCbCrGeneric(const VDPixmapGenYCbCrBasis& basis, bool studioRGB, uint32 colorSpace);

	uint32 GetType(uint32 output) const;

protected:
	void Compute(void *dst0, sint32 y);

	sint32 mCoYR;
	sint32 mCoYG;
	sint32 mCoYB;
	sint32 mCoCbR;
	sint32 mCoCbG;
	sint32 mCoCbB;
	sint32 mCoCrR;
	sint32 mCoCrG;
	sint32 mCoCrB;
	sint32 mCoYA;
	sint32 mCoCbA;
	sint32 mCoCrA;

	const uint32 mColorSpace;
};

////////////////////////////////////////////////////////////////////////////

class VDPixmapGenRGB32FToYCbCrGeneric : public VDPixmapGenRGB32FToYCbCrBase {
public:
	VDPixmapGenRGB32FToYCbCrGeneric(const VDPixmapGenYCbCrBasis& basis, uint32 colorSpace);

	uint32 GetType(uint32 output) const;

protected:
	void Compute(void *dst0, sint32 y);

	float mCoYR;
	float mCoYG;
	float mCoYB;
	float mCoCb;
	float mCoCr;

	const uint32 mColorSpace;
};

////////////////////////////////////////////////////////////////////////////

class VDPixmapGenYCbCrToYCbCrGeneric : public VDPixmapGenYCbCrToRGBBase {
public:
	VDPixmapGenYCbCrToYCbCrGeneric(const VDPixmapGenYCbCrBasis& dstBasis, bool dstLimitedRange, const VDPixmapGenYCbCrBasis& srcBasis, bool srcLimitedRange, uint32 colorSpace);
	 
	void Start();
	const void *GetRow(sint32 y, uint32 index);
	uint32 GetType(uint32 output) const;

protected:
	void Compute(void *dst0, sint32 ypos);

	sint32 mCoYY;
	sint32 mCoYCb;
	sint32 mCoYCr;
	sint32 mCoYA;
	sint32 mCoCbCb;
	sint32 mCoCbCr;
	sint32 mCoCbA;
	sint32 mCoCrCb;
	sint32 mCoCrCr;
	sint32 mCoCrA;

	const uint32 mColorSpace;
};

////////////////////////////////////////////////////////////////////////////

class VDPixmapGenYCbCrToYCbCrGeneric_32F : public VDPixmapGenYCbCrToRGBBase {
public:
	VDPixmapGenYCbCrToYCbCrGeneric_32F(const VDPixmapGenYCbCrBasis& dstBasis, bool dstLimitedRange, const VDPixmapGenYCbCrBasis& srcBasis, bool srcLimitedRange, uint32 colorSpace);

	void Start();
	const void *GetRow(sint32 y, uint32 index);
	uint32 GetType(uint32 output) const;

protected:
	void Compute(void *dst0, sint32 ypos);

	float mCoYY;
	float mCoYCb;
	float mCoYCr;
	float mCoYA;
	float mCoCbCb;
	float mCoCbCr;
	float mCoCbA;
	float mCoCrCb;
	float mCoCrCr;	
	float mCoCrA;	

	const uint32 mColorSpace;
};

#endif
