#ifndef f_VD2_KASUMI_PIXMAPOPS_H
#define f_VD2_KASUMI_PIXMAPOPS_H

#ifdef _MSC_VER
	#pragma once
#endif

#include <vd2/Kasumi/pixmap.h>

bool VDPixmapIsBltPossible(int dst_format, int src_format);
bool VDPixmapBlt(const VDPixmap& dst, const VDPixmap& src);
bool VDPixmapBlt(const VDPixmap& dst, vdpixpos x1, vdpixpos y1, const VDPixmap& src, vdpixpos x2, vdpixpos y2, vdpixsize w, vdpixsize h);
bool VDPixmapStretchBltNearest(const VDPixmap& dst, const VDPixmap& src);
bool VDPixmapStretchBltNearest(const VDPixmap& dst, sint32 x1, sint32 y1, sint32 x2, sint32 y2, const VDPixmap& src, sint32 u1, sint32 v1, sint32 u2, sint32 v2);
bool VDPixmapStretchBltBilinear(const VDPixmap& dst, const VDPixmap& src);
bool VDPixmapStretchBltBilinear(const VDPixmap& dst, sint32 x1, sint32 y1, sint32 x2, sint32 y2, const VDPixmap& src, sint32 u1, sint32 v1, sint32 u2, sint32 v2);

bool VDPixmapBltAlphaConst(const VDPixmap& dst, const VDPixmap& src, float alpha);

#endif
