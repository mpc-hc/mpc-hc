#ifndef f_VD2_KASUMI_BITUTILS_H
#define f_VD2_KASUMI_BITUTILS_H

#include <vd2/system/vdtypes.h>

namespace nsVDPixmapBitUtils {
	inline uint32 avg_8888_11(uint32 x, uint32 y) {
		return (x|y) - (((x^y)&0xfefefefe)>>1);
	}

	inline uint32 avg_8888_121(uint32 x, uint32 y, uint32 z) {
		return avg_8888_11(avg_8888_11(x,z), y);
	}

	inline uint32 avg_0808_14641(uint32 a, uint32 b, uint32 c, uint32 d, uint32 e) {
		a &= 0xff00ff;
		b &= 0xff00ff;
		c &= 0xff00ff;
		d &= 0xff00ff;
		e &= 0xff00ff;

		return (((a+e) + 4*(b+d) + 6*c + 0x080008)&0x0ff00ff0)>>4;
	}
};

#endif
