#ifndef f_VD2_KASUMI_UBERBLIT_PAL_H
#define f_VD2_KASUMI_UBERBLIT_PAL_H

#include "uberblit_base.h"
#include "uberblit_input.h"

class VDPixmapGenBase_Pal_To_X8R8G8B8 : public VDPixmapGenWindowBasedOneSourceSimple, public IVDPixmapGenSrc {
public:
	void Start() {
		StartWindow(mWidth * 4);
	}

	void Init(IVDPixmapGen *src, int srcIndex) {
		InitSource(src, srcIndex);
	}

	void SetSource(const void *src, ptrdiff_t pitch, const uint32 *palette) {
		mpPal = palette;
	}

	uint32 GetType(uint32 output) const {
		return (mpSrc->GetType(mSrcIndex) & ~kVDPixType_Mask) | kVDPixType_8888;
	}

protected:
	const uint32 *mpPal;
};

class VDPixmapGen_Pal1_To_X8R8G8B8 : public VDPixmapGenBase_Pal_To_X8R8G8B8 {
protected:
	void Compute(void *dst0, sint32 y) {
		uint32 *dst = (uint32 *)dst0;
		const uint8 *src = (const uint8 *)mpSrc->GetRow(y, mSrcIndex);
		sint32 w = mWidth;
		sint32 h = mHeight;

		const uint32 *pal = mpPal;

		src += (w-1) >> 3;
		dst += (w-1) & ~7;

		int wt = w;

		uint8 v = src[0] >> ((-wt) & 7);
		
		switch(wt & 7) {
			do {
				v = src[0];

		case 0:	dst[7] = pal[v&1];	v >>= 1;
		case 7:	dst[6] = pal[v&1];	v >>= 1;
		case 6:	dst[5] = pal[v&1];	v >>= 1;
		case 5:	dst[4] = pal[v&1];	v >>= 1;
		case 4:	dst[3] = pal[v&1];	v >>= 1;
		case 3:	dst[2] = pal[v&1];	v >>= 1;
		case 2:	dst[1] = pal[v&1];	v >>= 1;
		case 1:	dst[0] = pal[v&1];	v >>= 1;

				dst -= 8;
				--src;
			} while((wt -= 8) > 0);
		}
	}
};

class VDPixmapGen_Pal2_To_X8R8G8B8 : public VDPixmapGenBase_Pal_To_X8R8G8B8 {
protected:
	void Compute(void *dst0, sint32 y) {
		uint32 *dst = (uint32 *)dst0;
		const uint8 *src = (const uint8 *)mpSrc->GetRow(y, mSrcIndex);
		sint32 w = mWidth;
		sint32 h = mHeight;

		const uint32 *pal = mpPal;

		src += (w-1) >> 2;
		dst += (w-1) & ~3;

		int wt = w;

		uint8 v = src[0] >> (((-wt) & 3)*2);
		
		switch(wt & 3) {
			do {
				v = src[0];

		case 0:	dst[3] = pal[v&3];	v >>= 2;
		case 3:	dst[2] = pal[v&3];	v >>= 2;
		case 2:	dst[1] = pal[v&3];	v >>= 2;
		case 1:	dst[0] = pal[v&3];	v >>= 2;

				dst -= 4;
				--src;
			} while((wt -= 4) > 0);
		}
	}
};

class VDPixmapGen_Pal4_To_X8R8G8B8 : public VDPixmapGenBase_Pal_To_X8R8G8B8 {
protected:
	void Compute(void *dst0, sint32 y) {
		uint32 *dst = (uint32 *)dst0;
		const uint8 *src = (const uint8 *)mpSrc->GetRow(y, mSrcIndex);
		sint32 w = mWidth;
		sint32 h = mHeight;

		const uint32 *pal = mpPal;

		src += (w-1) >> 1;
		dst += ((w-1) & ~1);

		int wt = w;

		uint8 v = src[0] >> (((-wt) & 1)*4);
		
		switch(wt & 1) {
			do {
				v = src[0];

		case 0:	dst[1] = pal[v&15];	v >>= 4;
		case 1:	dst[0] = pal[v&15];	v >>= 4;

				dst -= 2;
				--src;
			} while((wt -= 2) > 0);
		}
	}
};

class VDPixmapGen_Pal8_To_X8R8G8B8 : public VDPixmapGenBase_Pal_To_X8R8G8B8 {
protected:
	void Compute(void *dst0, sint32 y) {
		uint32 *dst = (uint32 *)dst0;
		const uint8 *src = (const uint8 *)mpSrc->GetRow(y, mSrcIndex);
		sint32 w = mWidth;
		sint32 h = mHeight;

		const uint32 *pal = mpPal;

		int wt = w;

		do {
			*dst++ = pal[*src++];
		} while(--wt);
	}
};

#endif
