#ifndef f_VD2_KASUMI_PIXMAP_H
#define f_VD2_KASUMI_PIXMAP_H

#ifdef _MSC_VER
	#pragma once
#endif

#include <vd2/system/vdtypes.h>

namespace nsVDPixmap {
	enum VDPixmapFormat {
		kPixFormat_Null,
		kPixFormat_Pal1,
		kPixFormat_Pal2,
		kPixFormat_Pal4,
		kPixFormat_Pal8,
		kPixFormat_XRGB1555,
		kPixFormat_RGB565,
		kPixFormat_RGB888,
		kPixFormat_XRGB8888,
		kPixFormat_Y8,
		kPixFormat_YUV422_UYVY,
		kPixFormat_YUV422_YUYV,
		kPixFormat_YUV444_XVYU,				// The reason for the strange VYU ordering is to make it easier to convert to UYVY/YUY2.
		kPixFormat_YUV444_Planar,
		kPixFormat_YUV422_Planar,
		kPixFormat_YUV420_Planar,
		kPixFormat_YUV411_Planar,
		kPixFormat_YUV410_Planar,
		kPixFormat_YUV422_Planar_Centered,	// MPEG-1/MJPEG chroma alignment
		kPixFormat_YUV420_Planar_Centered,	// MPEG-1/MJPEG chroma alignment
		kPixFormat_YUV422_Planar_16F,
		kPixFormat_YUV422_V210,
		kPixFormat_YUV422_UYVY_709,			// Also known as HDYC.
		kPixFormat_YUV420_NV12,
		kPixFormat_Y8_FR,
		kPixFormat_YUV422_YUYV_709,
		kPixFormat_YUV444_Planar_709,
		kPixFormat_YUV422_Planar_709,
		kPixFormat_YUV420_Planar_709,
		kPixFormat_YUV411_Planar_709,
		kPixFormat_YUV410_Planar_709,
		kPixFormat_YUV422_UYVY_FR,
		kPixFormat_YUV422_YUYV_FR,
		kPixFormat_YUV444_Planar_FR,
		kPixFormat_YUV422_Planar_FR,
		kPixFormat_YUV420_Planar_FR,
		kPixFormat_YUV411_Planar_FR,
		kPixFormat_YUV410_Planar_FR,
		kPixFormat_YUV422_UYVY_709_FR,
		kPixFormat_YUV422_YUYV_709_FR,
		kPixFormat_YUV444_Planar_709_FR,
		kPixFormat_YUV422_Planar_709_FR,
		kPixFormat_YUV420_Planar_709_FR,
		kPixFormat_YUV411_Planar_709_FR,
		kPixFormat_YUV410_Planar_709_FR,
		kPixFormat_YUV420i_Planar,
		kPixFormat_YUV420i_Planar_FR,
		kPixFormat_YUV420i_Planar_709,
		kPixFormat_YUV420i_Planar_709_FR,
		kPixFormat_YUV420it_Planar,
		kPixFormat_YUV420it_Planar_FR,
		kPixFormat_YUV420it_Planar_709,
		kPixFormat_YUV420it_Planar_709_FR,
		kPixFormat_YUV420ib_Planar,
		kPixFormat_YUV420ib_Planar_FR,
		kPixFormat_YUV420ib_Planar_709,
		kPixFormat_YUV420ib_Planar_709_FR,
		kPixFormat_Max_Standard
	};
}

typedef sint32		vdpixpos;
typedef sint32		vdpixsize;
typedef ptrdiff_t	vdpixoffset;

struct VDPixmap {
	void			*data;
	const uint32	*palette;
	vdpixsize		w;
	vdpixsize		h;
	vdpixoffset		pitch;
	sint32			format;

	// Auxiliary planes are always byte-per-pixel.

	void			*data2;		// Cb (U) for YCbCr
	vdpixoffset		pitch2;
	void			*data3;		// Cr (V) for YCbCr
	vdpixoffset		pitch3;
};

struct VDPixmapLayout {
	ptrdiff_t		data;
	const uint32	*palette;
	vdpixsize		w;
	vdpixsize		h;
	vdpixoffset		pitch;
	sint32			format;

	// Auxiliary planes are always byte-per-pixel.

	ptrdiff_t		data2;		// Cb (U) for YCbCr
	vdpixoffset		pitch2;
	ptrdiff_t		data3;		// Cr (V) for YCbCr
	vdpixoffset		pitch3;
};

#endif
