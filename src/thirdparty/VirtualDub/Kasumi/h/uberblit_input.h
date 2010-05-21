#ifndef f_VD2_KASUMI_UBERBLIT_INPUT_H
#define f_VD2_KASUMI_UBERBLIT_INPUT_H

#include "uberblit.h"
#include "uberblit_base.h"

class IVDPixmapGenSrc {
public:
	virtual void SetSource(const void *src, ptrdiff_t pitch, const uint32 *palette) = 0;
};

class VDPixmapGenSrc : public IVDPixmapGen, public IVDPixmapGenSrc {
public:
	void Init(sint32 width, sint32 height, uint32 type, uint32 bpr) {
		mWidth = width;
		mHeight = height;
		mType = type;
		mBpr = bpr;
	}

	void SetSource(const void *src, ptrdiff_t pitch, const uint32 *palette) {
		mpSrc = src;
		mPitch = pitch;
	}

	void AddWindowRequest(int minY, int maxY) {
	}

	void Start() {
	}

	sint32 GetWidth(int) const {
		return mWidth;
	}

	sint32 GetHeight(int) const {
		return mHeight;
	}

	bool IsStateful() const {
		return false;
	}

	const void *GetRow(sint32 y, uint32 output) {
		if (y < 0)
			y = 0;
		else if (y >= mHeight)
			y = mHeight - 1;
		return vdptroffset(mpSrc, mPitch*y);
	}

	void ProcessRow(void *dst, sint32 y) {
		memcpy(dst, GetRow(y, 0), mBpr);
	}

	uint32 GetType(uint32 index) const {
		return mType;
	}

protected:
	const void *mpSrc;
	ptrdiff_t	mPitch;
	size_t		mBpr;
	sint32		mWidth;
	sint32		mHeight;
	uint32		mType;
};

#endif
