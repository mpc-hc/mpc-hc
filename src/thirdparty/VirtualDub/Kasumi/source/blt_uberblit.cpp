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
