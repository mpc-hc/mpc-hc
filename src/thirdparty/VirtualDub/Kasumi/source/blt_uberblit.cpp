//	VirtualDub - Video processing and capture application
//	Graphics support library
//	Copyright (C) 1998-2009 Avery Lee
//
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include <stdafx.h>
#include <vd2/system/vdalloc.h>
#include <vd2/Kasumi/pixmap.h>
#include "uberblit.h"

void VDPixmapBlt_UberblitAdapter(const VDPixmap& dst, const VDPixmap& src, vdpixsize w, vdpixsize h) {
	vdautoptr<IVDPixmapBlitter> blitter(VDPixmapCreateBlitter(dst, src));

	if (w > src.w)
		w = src.w;
	if (w > dst.w)
		w = dst.w;
	if (h > src.h)
		h = src.h;
	if (h > dst.h)
		h = dst.h;

	vdrect32 r(0, 0, w, h);
	blitter->Blit(dst, &r, src);
}
