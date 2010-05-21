#ifndef f_VD2_KASUMI_UBERBLIT_FILL_H
#define f_VD2_KASUMI_UBERBLIT_FILL_H

#include "uberblit.h"
#include "uberblit_base.h"

class VDPixmapGenFill8 : public IVDPixmapGen {
public:
	void Init(uint8 fill, uint32 bpr, sint32 width, sint32 height, uint32 type) {
		mRow.resize(bpr, fill);
		mWidth = width;
		mHeight = height;
		mType = type;
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
		return mRow.data();
	}

	void ProcessRow(void *dst, sint32 y) {
		if (!mRow.empty())
			memset(dst, mRow[0], mRow.size());
	}

	uint32 GetType(uint32 index) const {
		return mType;
	}

protected:
	sint32		mWidth;
	sint32		mHeight;
	uint32		mType;

	vdfastvector<uint8> mRow;
};

#endif
