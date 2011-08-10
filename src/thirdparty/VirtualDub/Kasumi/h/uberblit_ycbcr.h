#ifndef f_VD2_KASUMI_UBERBLIT_YCBCR_H
#define f_VD2_KASUMI_UBERBLIT_YCBCR_H

#include <vd2/system/cpuaccel.h>
#include <vd2/system/math.h>
#include <vd2/Kasumi/pixmaputils.h>
#include "uberblit.h"
#include "uberblit_base.h"

class VDPixmapGenYCbCrToRGBBase : public VDPixmapGenWindowBased {
public:
	void Init(IVDPixmapGen *srcCr, uint32 srcindexCr, IVDPixmapGen *srcY, uint32 srcindexY, IVDPixmapGen *srcCb, uint32 srcindexCb) {
		mpSrcY = srcY;
		mSrcIndexY = srcindexY;
		mpSrcCb = srcCb;
		mSrcIndexCb = srcindexCb;
		mpSrcCr = srcCr;
		mSrcIndexCr = srcindexCr;
		mWidth = srcY->GetWidth(srcindexY);
		mHeight = srcY->GetHeight(srcindexY);

		srcY->AddWindowRequest(0, 0);
		srcCb->AddWindowRequest(0, 0);
		srcCr->AddWindowRequest(0, 0);
	}


protected:
	IVDPixmapGen *mpSrcY;
	uint32 mSrcIndexY;
	IVDPixmapGen *mpSrcCb;
	uint32 mSrcIndexCb;
	IVDPixmapGen *mpSrcCr;
	uint32 mSrcIndexCr;
};

class VDPixmapGenYCbCrToRGB32Base : public VDPixmapGenYCbCrToRGBBase {
public:
	void Start() {
		mpSrcY->Start();
		mpSrcCb->Start();
		mpSrcCr->Start();

		StartWindow(mWidth * 4);
	}
};


class VDPixmapGenYCbCrToRGB32FBase : public VDPixmapGenYCbCrToRGBBase {
public:
	void Start() {
		mpSrcY->Start();
		mpSrcCb->Start();
		mpSrcCr->Start();

		StartWindow(mWidth * 16);
	}
};


class VDPixmapGenRGB32ToYCbCrBase : public VDPixmapGenWindowBasedOneSource {
public:
	void Init(IVDPixmapGen *src, uint32 srcindex) {
		InitSource(src, srcindex);
	}

	void Start() {
		StartWindow(mWidth, 3);
	}

	const void *GetRow(sint32 y, uint32 index) {
		return (const uint8 *)VDPixmapGenWindowBasedOneSource::GetRow(y, index) + mWindowPitch * index;
	}
};

class VDPixmapGenRGB32FToYCbCrBase : public VDPixmapGenWindowBasedOneSource {
public:
	void Init(IVDPixmapGen *src, uint32 srcindex) {
		InitSource(src, srcindex);
	}

	void Start() {
		StartWindow(mWidth * sizeof(float), 3);
	}

	const void *GetRow(sint32 y, uint32 index) {
		return (const uint8 *)VDPixmapGenWindowBasedOneSource::GetRow(y, index) + mWindowPitch * index;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Rec.601 converters
//
//	-->Kr=0.299; Kb=0.114; Z=0; S=255; L = [Kr 1-Kr-Kb Kb]; Y = [219*(L-Z)/S 16]; U = [112*([0 0 1]-L)/((1-Kb)*S) 128]; V =
//	[112*([1 0 0]-L)/((1-Kr)*S) 128]; M = [Y; U; V; 0 0 0 1]; disp(M); disp(inv(M));
//
//	!   0.2567882    0.5041294    0.0979059    16.  !
//	! - 0.1482229  - 0.2909928    0.4392157    128. !
//	!   0.4392157  - 0.3677883  - 0.0714274    128. !
//	!   0.           0.           0.           1.   !
//
//	!   1.1643836  - 5.599D-17    1.5960268  - 222.92157 !
//	!   1.1643836  - 0.3917623  - 0.8129676    135.57529 !
//	!   1.1643836    2.0172321  - 1.110D-16  - 276.83585 !
//	!   0.           0.           0.           1.        !
//
///////////////////////////////////////////////////////////////////////////////////////////////////

class VDPixmapGenYCbCr601ToRGB32 : public VDPixmapGenYCbCrToRGB32Base {
public:
	uint32 GetType(uint32 output) const {
		return (mpSrcY->GetType(mSrcIndexY) & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixType_8888 | kVDPixSpace_BGR;
	}

protected:
	virtual void Compute(void *dst0, sint32 y) {
		uint8 *dst = (uint8 *)dst0;
		const uint8 *srcY = (const uint8 *)mpSrcY->GetRow(y, mSrcIndexY);
		const uint8 *srcCb = (const uint8 *)mpSrcCb->GetRow(y, mSrcIndexCb);
		const uint8 *srcCr = (const uint8 *)mpSrcCr->GetRow(y, mSrcIndexCr);

		VDCPUCleanupExtensions();

		for(sint32 i=0; i<mWidth; ++i) {
			sint32 y = srcY[i];
			sint32 cb = srcCb[i];
			sint32 cr = srcCr[i];

			float yf = (1.164f / 255.0f)*(y - 16);

			dst[0] = VDClampedRoundFixedToUint8Fast(yf + (2.018f / 255.0f) * (cb - 128));
			dst[1] = VDClampedRoundFixedToUint8Fast(yf - (0.813f / 255.0f) * (cr - 128) - (0.391f / 255.0f) * (cb - 128));
			dst[2] = VDClampedRoundFixedToUint8Fast(yf + (1.596f / 255.0f) * (cr - 128));
			dst[3] = 0xff;

			dst += 4;
		}
	}
};

class VDPixmapGenYCbCr601ToRGB32F : public VDPixmapGenYCbCrToRGB32FBase {
public:
	uint32 GetType(uint32 output) const {
		return (mpSrcY->GetType(mSrcIndexY) & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixType_32Fx4_LE | kVDPixSpace_BGR;
	}

protected:
	void Compute(void *dst0, sint32 y) {
		float *dst = (float *)dst0;
		const float *srcY = (const float *)mpSrcY->GetRow(y, mSrcIndexY);
		const float *srcCb = (const float *)mpSrcCb->GetRow(y, mSrcIndexCb);
		const float *srcCr = (const float *)mpSrcCr->GetRow(y, mSrcIndexCr);

		VDCPUCleanupExtensions();

		for(sint32 i=0; i<mWidth; ++i) {
			float y = srcY[i];
			float cb = srcCb[i] - (128.0f / 255.0f);
			float cr = srcCr[i] - (128.0f / 255.0f);

			float yf = 1.164f * (y - 16.0f / 255.0f);

			dst[0] = yf + 1.596f * cr;
			dst[1] = yf - 0.813f * cr - 0.391f * cb;
			dst[2] = yf + 2.018f * cb;
			dst[3] = 1.0f;
			dst += 4;
		}
	}
};

class VDPixmapGenRGB32ToYCbCr601 : public VDPixmapGenRGB32ToYCbCrBase {
public:
	uint32 GetType(uint32 output) const {
		return (mpSrc->GetType(mSrcIndex) & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixType_8 | kVDPixSpace_YCC_601;
	}

protected:
	void Compute(void *dst0, sint32 y) {
		uint8 *dstCr = (uint8 *)dst0;
		uint8 *dstY = dstCr + mWindowPitch;
		uint8 *dstCb = dstY + mWindowPitch;

		const uint8 *srcRGB = (const uint8 *)mpSrc->GetRow(y, mSrcIndex);

		for(sint32 i=0; i<mWidth; ++i) {
			int r = (int)srcRGB[2];
			int g = (int)srcRGB[1];
			int b = (int)srcRGB[0];
			srcRGB += 4;			


			// -2->round(inv([1 0 0 0; 0 1 0 0; 0 0 1 0; -16 -128 -128 1] * [1.1643828 1.1643828 1.1643828 0; 1.5960273 -0.8129688 0 0;
			//  0 -0.3917617 2.0172305 0; 0 0 0 1]) .* 65536)
			// ans  =
			// 
			// !   16829.      28784.    - 9714.       0.     !
			// !   33039.    - 24103.    - 19071.      0.     !
			// !   6416.     - 4681.       28784.      0.     !
			// !   1048576.    8388608.    8388608.    65536. !   

			*dstCr++ = (28784*r - 24103*g -  4681*b + 8388608 + 32768) >> 16;
			*dstY ++ = (16829*r + 33039*g +  6416*b + 1048576 + 32768) >> 16;
			*dstCb++ = (-9714*r - 19071*g + 28784*b + 8388608 + 32768) >> 16;
		}
	}
};

class VDPixmapGenRGB32FToYCbCr601 : public VDPixmapGenRGB32FToYCbCrBase {
public:
	uint32 GetType(uint32 output) const {
		return (mpSrc->GetType(mSrcIndex) & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixType_32F_LE | kVDPixSpace_YCC_601;
	}

protected:
	void Compute(void *dst0, sint32 y) {
		float *dstCb = (float *)dst0;
		float *dstY  = dstCb + mWindowPitch;
		float *dstCr = dstY + mWindowPitch;

		const float *srcRGB = (const float *)mpSrc->GetRow(y, mSrcIndex);

		VDCPUCleanupExtensions();

		for(sint32 i=0; i<mWidth; ++i) {
			float r = srcRGB[2];
			float g = srcRGB[1];
			float b = srcRGB[0];
			srcRGB += 4;			

			*dstCb++ = -0.1482229f*r - 0.2909928f*g + 0.4392157f*b + (128.0f / 255.0f);
			*dstY++  =  0.2567882f*r + 0.5041294f*g + 0.0979059f*b + ( 16.0f / 255.0f);
			*dstCr++ =  0.4392157f*r - 0.3677883f*g - 0.0714274f*b + (128.0f / 255.0f);
		}
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Rec.709 converters
//
//
//	-->Kr=0.2126; Kb=0.0722; Z=0; S=255; L = [Kr 1-Kr-Kb Kb]; Y = [219*(L-Z)/S 16]; U = [112*([0 0 1]-L)/((1-Kb)*S) 128]; V
//	= [112*([1 0 0]-L)/((1-Kr)*S) 128]; M = [Y; U; V; 0 0 0 1]; disp(M); disp(inv(M));
//
//	!   0.1825859    0.6142306    0.0620071    16.  !
//	! - 0.1006437  - 0.3385720    0.4392157    128. !
//	!   0.4392157  - 0.3989422  - 0.0402735    128. !
//	!   0.           0.           0.           1.   !
//
//	!   1.1643836  - 2.932D-17    1.7927411  - 248.10099 !
//	!   1.1643836  - 0.2132486  - 0.5329093    76.87808  !
//	!   1.1643836    2.1124018  - 5.551D-17  - 289.01757 !
//	!   0.           0.           0.           1.        !                     
//
///////////////////////////////////////////////////////////////////////////////////////////////////

class VDPixmapGenYCbCr709ToRGB32 : public VDPixmapGenYCbCrToRGBBase {
public:
	void Start() {
		mpSrcY->Start();
		mpSrcCb->Start();
		mpSrcCr->Start();

		StartWindow(mWidth * 4);
	}

	uint32 GetType(uint32 output) const {
		return (mpSrcY->GetType(mSrcIndexY) & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixType_8888 | kVDPixSpace_BGR;
	}

protected:
	virtual void Compute(void *dst0, sint32 y) {
		uint8 *dst = (uint8 *)dst0;
		const uint8 *srcY = (const uint8 *)mpSrcY->GetRow(y, mSrcIndexY);
		const uint8 *srcCb = (const uint8 *)mpSrcCb->GetRow(y, mSrcIndexCb);
		const uint8 *srcCr = (const uint8 *)mpSrcCr->GetRow(y, mSrcIndexCr);

		VDCPUCleanupExtensions();

		for(sint32 i=0; i<mWidth; ++i) {
			sint32 y = srcY[i];
			sint32 cb = srcCb[i];
			sint32 cr = srcCr[i];

			float yf = (1.164f / 255.0f)*(y - 16);

			dst[0] = VDClampedRoundFixedToUint8Fast(yf + (2.112f / 255.0f) * (cb - 128));
			dst[1] = VDClampedRoundFixedToUint8Fast(yf - (0.533f / 255.0f) * (cr - 128) - (0.213f / 255.0f) * (cb - 128));
			dst[2] = VDClampedRoundFixedToUint8Fast(yf + (1.793f / 255.0f) * (cr - 128));
			dst[3] = 0xff;

			dst += 4;
		}
	}
};

class VDPixmapGenYCbCr709ToRGB32F : public VDPixmapGenYCbCrToRGBBase {
public:
	void Start() {
		mpSrcY->Start();
		mpSrcCb->Start();
		mpSrcCr->Start();

		StartWindow(mWidth * 16);
	}

	uint32 GetType(uint32 output) const {
		return (mpSrcY->GetType(mSrcIndexY) & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixType_32Fx4_LE | kVDPixSpace_BGR;
	}

protected:
	void Compute(void *dst0, sint32 y) {
		float *dst = (float *)dst0;
		const float *srcY = (const float *)mpSrcY->GetRow(y, mSrcIndexY);
		const float *srcCb = (const float *)mpSrcCb->GetRow(y, mSrcIndexCb);
		const float *srcCr = (const float *)mpSrcCr->GetRow(y, mSrcIndexCr);

		VDCPUCleanupExtensions();

		for(sint32 i=0; i<mWidth; ++i) {
			float y = srcY[i];
			float cb = srcCb[i] - (128.0f/255.0f);
			float cr = srcCr[i] - (128.0f/255.0f);

			float yf = 1.164f * (y - 16.0f / 255.0f);

			dst[0] = yf + 1.793f * cr;
			dst[1] = yf - 0.533f * cr - 0.213f * cb;
			dst[2] = yf + 2.112f * cb;
			dst[3] = 1.0f;
			dst += 4;
		}
	}
};

class VDPixmapGenRGB32ToYCbCr709 : public VDPixmapGenWindowBasedOneSource {
public:
	void Init(IVDPixmapGen *src, uint32 srcindex) {
		InitSource(src, srcindex);
	}

	void Start() {
		StartWindow(mWidth, 3);
	}

	const void *GetRow(sint32 y, uint32 index) {
		return (const uint8 *)VDPixmapGenWindowBasedOneSource::GetRow(y, index) + mWindowPitch * index;
	}

	uint32 GetType(uint32 output) const {
		return (mpSrc->GetType(mSrcIndex) & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixType_8 | kVDPixSpace_YCC_709;
	}

protected:
	void Compute(void *dst0, sint32 y) {
		uint8 *dstCr = (uint8 *)dst0;
		uint8 *dstY = dstCr + mWindowPitch;
		uint8 *dstCb = dstY + mWindowPitch;

		const uint8 *srcRGB = (const uint8 *)mpSrc->GetRow(y, mSrcIndex);

		for(sint32 i=0; i<mWidth; ++i) {
			int r = (int)srcRGB[2];
			int g = (int)srcRGB[1];
			int b = (int)srcRGB[0];
			srcRGB += 4;			

			*dstCr++ = (28784*r - 26145*g -  2639*b + 8388608 + 32768) >> 16;
			*dstY ++ = (11966*r + 40254*g +  4064*b + 1048576 + 32768) >> 16;
			*dstCb++ = (-6596*r - 22189*g + 28784*b + 8388608 + 32768) >> 16;
		}
	}
};

class VDPixmapGenRGB32FToYCbCr709 : public VDPixmapGenWindowBasedOneSource {
public:
	void Init(IVDPixmapGen *src, uint32 srcindex) {
		InitSource(src, srcindex);
	}

	void Start() {
		StartWindow(mWidth * sizeof(float), 3);
	}

	const void *GetRow(sint32 y, uint32 index) {
		return (const uint8 *)VDPixmapGenWindowBasedOneSource::GetRow(y, index) + mWindowPitch * index;
	}

	uint32 GetType(uint32 output) const {
		return (mpSrc->GetType(mSrcIndex) & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixType_32F_LE | kVDPixSpace_YCC_709;
	}

protected:
	void Compute(void *dst0, sint32 y) {
		float *dstCr = (float *)dst0;
		float *dstY  = dstCr + mWindowPitch;
		float *dstCb = dstY + mWindowPitch;

		const float *srcRGB = (const float *)mpSrc->GetRow(y, mSrcIndex);

		VDCPUCleanupExtensions();

		for(sint32 i=0; i<mWidth; ++i) {
			float r = srcRGB[2];
			float g = srcRGB[1];
			float b = srcRGB[0];
			srcRGB += 4;			

			*dstCr++ = -0.1006437f*r - 0.3385720f*g + 0.4392157f*b + (128.0f / 255.0f);
			*dstY++  =  0.1825859f*r + 0.6142306f*g + 0.0620071f*b + ( 16.0f / 255.0f);
			*dstCb++ =  0.4392157f*r - 0.3989422f*g - 0.0402735f*b + (128.0f / 255.0f);
		}
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Rec.601 <-> Rec.709 converters
//
//	Rec.601 to Rec.709:
//
//    1.  - 0.1155497  - 0.2079376    41.406386
//    0     1.0186397    0.1146180  - 17.056983
//    0     0.0750494    1.0253271  - 12.848195
//
//	Rec.709 to Rec.601:
//
//    1.    0.0993117    0.1916995  - 37.249435
//    0     0.9898538  - 0.1106525    15.462234
//    0   - 0.0724530    0.9833978    11.399058
//
///////////////////////////////////////////////////////////////////////////////////////////////////

class VDPixmapGenYCbCr601ToYCbCr709 : public VDPixmapGenYCbCrToRGBBase {
public:
	void Start() {
		mpSrcY->Start();
		mpSrcCb->Start();
		mpSrcCr->Start();

		StartWindow(mWidth, 3);
	}

	const void *GetRow(sint32 y, uint32 index) {
		return (const uint8 *)VDPixmapGenYCbCrToRGBBase::GetRow(y, index) + mWindowPitch * index;
	}

	uint32 GetType(uint32 output) const {
		return (mpSrcY->GetType(mSrcIndexY) & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixType_8 | kVDPixSpace_YCC_709;
	}

protected:
	void Compute(void *dst0, sint32 ypos) {
		uint8 *dstCr = (uint8 *)dst0;
		uint8 *dstY  = dstCr + mWindowPitch;
		uint8 *dstCb = dstY + mWindowPitch;

		const uint8 *srcY  = (const uint8 *)mpSrcY->GetRow(ypos, mSrcIndexY);
		const uint8 *srcCb = (const uint8 *)mpSrcCb->GetRow(ypos, mSrcIndexCb);
		const uint8 *srcCr = (const uint8 *)mpSrcCr->GetRow(ypos, mSrcIndexCr);

		for(sint32 i=0; i<mWidth; ++i) {
			sint32 y = srcY[i];
			sint32 cb = srcCb[i];
			sint32 cr = srcCr[i];

			*dstY++  = y + ((-7573*cb - 13627*cr + 2713609 + 32768) >> 16);
			*dstCb++ = (66758*cb + 7512*cr - 1117846 + 32768) >> 16;
			*dstCr++ = (4918*cb + 67196*cr - 842019 + 32768) >> 16;
		}
	}
};

class VDPixmapGenYCbCr709ToYCbCr601 : public VDPixmapGenYCbCrToRGBBase {
public:
	void Start() {
		mpSrcY->Start();
		mpSrcCb->Start();
		mpSrcCr->Start();

		StartWindow(mWidth, 3);
	}

	const void *GetRow(sint32 y, uint32 index) {
		return (const uint8 *)VDPixmapGenYCbCrToRGBBase::GetRow(y, index) + mWindowPitch * index;
	}

	uint32 GetType(uint32 output) const {
		return (mpSrcY->GetType(mSrcIndexY) & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixType_8 | kVDPixSpace_YCC_709;
	}

protected:
	void Compute(void *dst0, sint32 ypos) {
		uint8 *dstCr = (uint8 *)dst0;
		uint8 *dstY  = dstCr + mWindowPitch;
		uint8 *dstCb = dstY + mWindowPitch;

		const uint8 *srcY  = (const uint8 *)mpSrcY->GetRow(ypos, mSrcIndexY);
		const uint8 *srcCb = (const uint8 *)mpSrcCb->GetRow(ypos, mSrcIndexCb);
		const uint8 *srcCr = (const uint8 *)mpSrcCr->GetRow(ypos, mSrcIndexCr);

		for(sint32 i=0; i<mWidth; ++i) {
			sint32 y = srcY[i];
			sint32 cb = srcCb[i];
			sint32 cr = srcCr[i];

			*dstY++  = y + ((6508*cb + 12563*cr - 2441088 + 32768) >> 16);
			*dstCb++ = (64871*cb - 7252*cr + 1013376 + 32768) >> 16;
			*dstCr++ = (-4748*cb + 64448*cr + 747008 + 32768) >> 16;
		}
	}
};

class VDPixmapGenYCbCr601ToYCbCr709_32F : public VDPixmapGenYCbCrToRGBBase {
public:
	void Start() {
		mpSrcY->Start();
		mpSrcCb->Start();
		mpSrcCr->Start();

		StartWindow(mWidth * sizeof(float), 3);
	}

	const void *GetRow(sint32 y, uint32 index) {
		return (const uint8 *)VDPixmapGenYCbCrToRGBBase::GetRow(y, index) + mWindowPitch * index;
	}

	uint32 GetType(uint32 output) const {
		return (mpSrcY->GetType(mSrcIndexY) & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixType_32F_LE | kVDPixSpace_YCC_709;
	}

protected:
	void Compute(void *dst0, sint32 ypos) {
		float *dstCr = (float *)dst0;
		float *dstY  = vdptroffset(dstCr, mWindowPitch);
		float *dstCb = vdptroffset(dstY, mWindowPitch);

		const float *srcY  = (const float *)mpSrcY->GetRow(ypos, mSrcIndexY);
		const float *srcCb = (const float *)mpSrcCb->GetRow(ypos, mSrcIndexCb);
		const float *srcCr = (const float *)mpSrcCr->GetRow(ypos, mSrcIndexCr);

		VDCPUCleanupExtensions();

		for(sint32 i=0; i<mWidth; ++i) {
			float y = srcY[i];
			float cb = srcCb[i];
			float cr = srcCr[i];

			*dstY++  = y - 0.1155497f*cb - 0.2079376f*cr;
			*dstCb++ =     1.0186397f*cb + 0.1146180f*cr;
			*dstCr++ =     0.0750494f*cb + 1.0253271f*cr;
		}
	}
};

class VDPixmapGenYCbCr709ToYCbCr601_32F : public VDPixmapGenYCbCrToRGBBase {
public:
	void Start() {
		mpSrcY->Start();
		mpSrcCb->Start();
		mpSrcCr->Start();

		StartWindow(mWidth * sizeof(float), 3);
	}

	const void *GetRow(sint32 y, uint32 index) {
		return (const uint8 *)VDPixmapGenYCbCrToRGBBase::GetRow(y, index) + mWindowPitch * index;
	}

	uint32 GetType(uint32 output) const {
		return (mpSrcY->GetType(mSrcIndexY) & ~(kVDPixType_Mask | kVDPixSpace_Mask)) | kVDPixType_32F_LE | kVDPixSpace_YCC_709;
	}

protected:
	void Compute(void *dst0, sint32 ypos) {
		float *dstCr = (float *)dst0;
		float *dstY  = vdptroffset(dstCr, mWindowPitch);
		float *dstCb = vdptroffset(dstY, mWindowPitch);

		const float *srcY  = (const float *)mpSrcY->GetRow(ypos, mSrcIndexY);
		const float *srcCb = (const float *)mpSrcCb->GetRow(ypos, mSrcIndexCb);
		const float *srcCr = (const float *)mpSrcCr->GetRow(ypos, mSrcIndexCr);

		VDCPUCleanupExtensions();

		for(sint32 i=0; i<mWidth; ++i) {
			float y = srcY[i];
			float cb = srcCb[i];
			float cr = srcCr[i];

			*dstY++  = y - 0.1155497f*cb - 0.2079376f*cr;
			*dstCb++ =     0.9898538f*cb - 0.1106525f*cr;
			*dstCr++ =   - 0.0724530f*cb + 0.9833978f*cr;
		}
	}
};

#endif
