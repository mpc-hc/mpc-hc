#ifndef f_VD2_KASUMI_BLITTER_H
#define f_VD2_KASUMI_BLITTER_H

#include <vd2/system/vectors.h>

struct VDPixmap;
struct VDPixmapLayout;

class IVDPixmapBlitter {
public:
	virtual ~IVDPixmapBlitter() {}
	virtual void Blit(const VDPixmap& dst, const VDPixmap& src) = 0;
	virtual void Blit(const VDPixmap& dst, const vdrect32 *rDst, const VDPixmap& src) = 0;
};

IVDPixmapBlitter *VDPixmapCreateBlitter(const VDPixmap& dst, const VDPixmap& src);
IVDPixmapBlitter *VDPixmapCreateBlitter(const VDPixmapLayout& dst, const VDPixmapLayout& src);

class VDPixmapCachedBlitter {
	VDPixmapCachedBlitter(const VDPixmapCachedBlitter&);
	VDPixmapCachedBlitter& operator=(const VDPixmapCachedBlitter&);
public:
	VDPixmapCachedBlitter();
	~VDPixmapCachedBlitter();

	void Blit(const VDPixmap& dst, const VDPixmap& src);
	void Invalidate();

protected:
	sint32 mSrcWidth;
	sint32 mSrcHeight;
	int mSrcFormat;
	sint32 mDstWidth;
	sint32 mDstHeight;
	int mDstFormat;
	IVDPixmapBlitter *mpCachedBlitter;
};

#endif
