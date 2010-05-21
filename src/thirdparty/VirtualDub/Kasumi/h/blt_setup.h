#ifndef f_VD2_KASUMI_BLT_SETUP_H
#define f_VD2_KASUMI_BLT_SETUP_H

#include <vd2/Kasumi/pixmap.h>
#include <vd2/Kasumi/pixmaputils.h>

typedef void (*VDPixmapPalettedBlitterFn)(void *dst, ptrdiff_t dstpitch, const void *src, ptrdiff_t srcpitch, vdpixsize w, vdpixsize h, const void *pal);
typedef void (*VDPixmapChunkyBlitterFn)(void *dst, ptrdiff_t dstpitch, const void *src, ptrdiff_t srcpitch, vdpixsize w, vdpixsize h);

void VDPixmapBltDirectPalettedConversion(const VDPixmap& dst, const VDPixmap& src, vdpixsize w, vdpixsize h, VDPixmapPalettedBlitterFn pBlitter);

template<VDPixmapPalettedBlitterFn palettedBlitter>
void VDPixmapBlitterPalettedAdapter(const VDPixmap& dst, const VDPixmap& src, vdpixsize w, vdpixsize h)
{
	if (dst.format == nsVDPixmap::kPixFormat_XRGB8888)
		palettedBlitter(dst.data, dst.pitch, src.data, src.pitch, w, h, src.palette);
	else
		VDPixmapBltDirectPalettedConversion(dst, src, w, h, palettedBlitter);
}

template<VDPixmapChunkyBlitterFn chunkyBlitter>
void VDPixmapBlitterChunkyAdapter(const VDPixmap& dst, const VDPixmap& src, vdpixsize w, vdpixsize h)
{
	chunkyBlitter(dst.data, dst.pitch, src.data, src.pitch, w, h);
}

struct VDPixmapFormatSubset {
public:
	VDPixmapFormatSubset() : mFormatCount(0) {}

	VDPixmapFormatSubset& operator=(int format) {
		mFormatCount = 0;
		mFormats[mFormatCount++] = format;
		return *this;
	}

	VDPixmapFormatSubset& operator,(int format) {
		VDASSERT(mFormatCount < nsVDPixmap::kPixFormat_Max_Standard);
		mFormats[mFormatCount++] = format;
		return *this;
	}

	int mFormatCount;
	int mFormats[nsVDPixmap::kPixFormat_Max_Standard];
};

class VDPixmapBlitterTable {
public:
	void Clear();
	void AddBlitter(int srcFormat, int dstFormat, VDPixmapBlitterFn blitter);
	void AddBlitter(const VDPixmapFormatSubset& srcFormats, VDPixmapFormatSubset& dstFormats, VDPixmapBlitterFn blitter);

	VDPixmapBlitterFn mTable[nsVDPixmap::kPixFormat_Max_Standard][nsVDPixmap::kPixFormat_Max_Standard];
};

inline void VDPixmapBlitterTable::AddBlitter(int srcFormat, int dstFormat, VDPixmapBlitterFn blitter) {
	mTable[srcFormat][dstFormat] = blitter;
}



#endif
