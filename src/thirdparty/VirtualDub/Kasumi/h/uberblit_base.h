#ifndef f_VD2_KASUMI_UBERBLIT_BASE_H
#define f_VD2_KASUMI_UBERBLIT_BASE_H

#include <vd2/system/vdstl.h>
#include "uberblit.h"

class VDPixmapGenWindowBased : public IVDPixmapGen {
public:
	VDPixmapGenWindowBased()
		: mWindowMinDY(0xffff)
		, mWindowMaxDY(-0xffff) {}

	void SetOutputSize(sint32 w, sint32 h) {
		mWidth = w;
		mHeight = h;
	}

	void AddWindowRequest(int minDY, int maxDY) {
		if (mWindowMinDY > minDY)
			mWindowMinDY = minDY;
		if (mWindowMaxDY < maxDY)
			mWindowMaxDY = maxDY;
	}

	void StartWindow(uint32 rowbytes, int outputCount = 1) {
		VDASSERT(mWindowMaxDY >= mWindowMinDY);
		mWindowSize = mWindowMaxDY + 1 - mWindowMinDY;

		mWindowPitch = (rowbytes + 15) & ~15;
		mWindowBuffer.resize(mWindowPitch * mWindowSize * outputCount);
		mWindow.resize(mWindowSize * 2);

		for(sint32 i=0; i<mWindowSize; ++i)
			mWindow[i] = mWindow[i + mWindowSize] = mWindowBuffer.data() + (mWindowPitch * outputCount * i);

		mWindowIndex = 0;
		mWindowLastY = -0x3FFFFFFF;
	}

	sint32 GetWidth(int) const { return mWidth; }
	sint32 GetHeight(int) const { return mHeight; }

	bool IsStateful() const {
		return true;
	}

	const void *GetRow(sint32 y, uint32 index) {
		sint32 tostep = y - mWindowLastY;
		VDASSERT(y >= mWindowLastY - (sint32)mWindowSize + 1);

		if (tostep >= mWindowSize) {
			mWindowLastY = y - 1;
			tostep = 1;
		}

		while(tostep-- > 0) {
			++mWindowLastY;
			Compute(mWindow[mWindowIndex], mWindowLastY);
			if (++mWindowIndex >= mWindowSize)
				mWindowIndex = 0;
		}

		return mWindow[y + mWindowSize - 1 - mWindowLastY + mWindowIndex];
	}

	void ProcessRow(void *dst, sint32 y) {
		Compute(dst, y);
	}

protected:
	virtual void Compute(void *dst0, sint32 y) = 0;

	vdfastvector<uint8> mWindowBuffer;
	vdfastvector<uint8 *> mWindow;
	sint32 mWindowPitch;
	sint32 mWindowIndex;
	sint32 mWindowMinDY;
	sint32 mWindowMaxDY;
	sint32 mWindowSize;
	sint32 mWindowLastY;
	sint32 mWidth;
	sint32 mHeight;
};

class VDPixmapGenWindowBasedOneSource : public VDPixmapGenWindowBased {
public:
	void InitSource(IVDPixmapGen *src, uint32 srcindex) {
		mpSrc = src;
		mSrcIndex = srcindex;
		mSrcWidth = src->GetWidth(srcindex);
		mSrcHeight = src->GetHeight(srcindex);
		mWidth = mSrcWidth;
		mHeight = mSrcHeight;
	}

	void AddWindowRequest(int minDY, int maxDY) {
		VDPixmapGenWindowBased::AddWindowRequest(minDY, maxDY);
		mpSrc->AddWindowRequest(minDY, maxDY);
	}

	void StartWindow(uint32 rowbytes, int outputCount = 1) {
		mpSrc->Start();

		VDPixmapGenWindowBased::StartWindow(rowbytes, outputCount);
	}

	uint32 GetType(uint32 output) const {
		return mpSrc->GetType(mSrcIndex);
	}

protected:
	virtual void Compute(void *dst0, sint32 y) = 0;

	IVDPixmapGen *mpSrc;
	uint32 mSrcIndex;
	sint32 mSrcWidth;
	sint32 mSrcHeight;
};

class VDPixmapGenWindowBasedOneSourceSimple : public VDPixmapGenWindowBasedOneSource {
public:
	void Init(IVDPixmapGen *src, uint32 srcindex) {
		InitSource(src, srcindex);

		src->AddWindowRequest(0, 0);
	}
};

#endif
