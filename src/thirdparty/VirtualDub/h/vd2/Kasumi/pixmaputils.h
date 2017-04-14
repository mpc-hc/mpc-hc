#ifndef f_VD2_KASUMI_PIXMAPUTILS_H
#define f_VD2_KASUMI_PIXMAPUTILS_H

#ifdef _MSC_VER
	#pragma once
#endif

#include <vd2/Kasumi/pixmap.h>

struct VDPixmapFormatInfo {
	const char *name;		// debugging name
	bool qchunky;			// quantums are chunky (not 1x1 pixels)
	int qw, qh;				// width, height of a quantum
	int	qwbits, qhbits;		// width and height of a quantum as shifts
	int qsize;				// size of a pixel in bytes
	int auxbufs;			// number of auxiliary buffers (0 for chunky formats, usually 2 for planar)
	int	auxwbits, auxhbits;	// subsampling factors for auxiliary buffers in shifts
	int auxsize;			// size of an aux sample in bytes
	int palsize;			// entries in palette
	int subformats;			// number of subformats for this format
};

extern const VDPixmapFormatInfo g_vdPixmapFormats[];

inline const VDPixmapFormatInfo& VDPixmapGetInfo(sint32 format) {
	VDASSERT((uint32)format < nsVDPixmap::kPixFormat_Max_Standard);
	return g_vdPixmapFormats[(uint32)format < nsVDPixmap::kPixFormat_Max_Standard ? format : 0];
}

#ifdef _DEBUG
	bool VDAssertValidPixmap(const VDPixmap& px);
#else
	inline bool VDAssertValidPixmap(const VDPixmap& px) { return true; }
#endif

inline VDPixmap VDPixmapFromLayout(const VDPixmapLayout& layout, void *p) {
	VDPixmap px;

	px.data		= (char *)p + layout.data;
	px.data2	= (char *)p + layout.data2;
	px.data3	= (char *)p + layout.data3;
	px.format	= layout.format;
	px.w		= layout.w;
	px.h		= layout.h;
	px.palette	= layout.palette;
	px.pitch	= layout.pitch;
	px.pitch2	= layout.pitch2;
	px.pitch3	= layout.pitch3;

	return px;
}

inline VDPixmapLayout VDPixmapToLayoutFromBase(const VDPixmap& px, void *p) {
	VDPixmapLayout layout;
	layout.data		= (char *)px.data - (char *)p;
	layout.data2	= (char *)px.data2 - (char *)p;
	layout.data3	= (char *)px.data3 - (char *)p;
	layout.format	= px.format;
	layout.w		= px.w;
	layout.h		= px.h;
	layout.palette	= px.palette;
	layout.pitch	= px.pitch;
	layout.pitch2	= px.pitch2;
	layout.pitch3	= px.pitch3;
	return layout;
}

inline VDPixmapLayout VDPixmapToLayout(const VDPixmap& px, void *&p) {
	VDPixmapLayout layout;
	p = px.data;
	layout.data		= 0;
	layout.data2	= (char *)px.data2 - (char *)px.data;
	layout.data3	= (char *)px.data3 - (char *)px.data;
	layout.format	= px.format;
	layout.w		= px.w;
	layout.h		= px.h;
	layout.palette	= px.palette;
	layout.pitch	= px.pitch;
	layout.pitch2	= px.pitch2;
	layout.pitch3	= px.pitch3;
	return layout;
}

uint32 VDPixmapCreateLinearLayout(VDPixmapLayout& layout, int format, vdpixsize w, vdpixsize h, int alignment);

VDPixmap VDPixmapOffset(const VDPixmap& src, vdpixpos x, vdpixpos y);
VDPixmapLayout VDPixmapLayoutOffset(const VDPixmapLayout& src, vdpixpos x, vdpixpos y);

void VDPixmapFlipV(VDPixmap& layout);
void VDPixmapLayoutFlipV(VDPixmapLayout& layout);

uint32 VDPixmapLayoutGetMinSize(const VDPixmapLayout& layout);

VDPixmap VDPixmapExtractField(const VDPixmap& src, bool field2);

#ifndef VDPTRSTEP_DECLARED
	template<class T>
	inline void vdptrstep(T *&p, ptrdiff_t offset) {
		p = (T *)((char *)p + offset);
	}
#endif
#ifndef VDPTROFFSET_DECLARED
	template<class T>
	inline T *vdptroffset(T *p, ptrdiff_t offset) {
		return (T *)((char *)p + offset);
	}
#endif
#ifndef VDPTRDIFFABS_DECLARED
	inline ptrdiff_t vdptrdiffabs(ptrdiff_t x) {
		return x<0 ? -x : x;
	}
#endif


typedef void (*VDPixmapBlitterFn)(const VDPixmap& dst, const VDPixmap& src, vdpixsize w, vdpixsize h);
typedef VDPixmapBlitterFn (*tpVDPixBltTable)[nsVDPixmap::kPixFormat_Max_Standard];

tpVDPixBltTable VDGetPixBltTableReference();
tpVDPixBltTable VDGetPixBltTableX86Scalar();
tpVDPixBltTable VDGetPixBltTableX86MMX();



class VDPixmapBuffer : public VDPixmap {
public:
	VDPixmapBuffer() : mpBuffer(NULL), mLinearSize(0) { data = NULL; format = 0; }
	explicit VDPixmapBuffer(const VDPixmap& src);
	VDPixmapBuffer(const VDPixmapBuffer& src);
	VDPixmapBuffer(sint32 w, sint32 h, int format) : mpBuffer(NULL), mLinearSize(0) {
		init(w, h, format);
	}
	explicit VDPixmapBuffer(const VDPixmapLayout& layout);

	~VDPixmapBuffer();

	void clear() {
		if (mpBuffer)		// to reduce debug checks
			delete[] mpBuffer;
		mpBuffer = NULL;
		mLinearSize = 0;
		format = nsVDPixmap::kPixFormat_Null;
	}

#ifdef _DEBUG
	void *base() { return mpBuffer + (-(int)(uintptr)mpBuffer & 15) + 16; }
	const void *base() const { return mpBuffer + (-(int)(uintptr)mpBuffer & 15) + 16; }
	size_t size() const { return mLinearSize - 28; }

	void validate();
#else
	void *base() { return mpBuffer + (-(int)(uintptr)mpBuffer & 15); }
	const void *base() const { return mpBuffer + (-(int)(uintptr)mpBuffer & 15); }
	size_t size() const { return mLinearSize; }

	void validate() {}
#endif

	void init(sint32 w, sint32 h, int format);
	void init(const VDPixmapLayout&, uint32 additionalPadding = 0);

	void assign(const VDPixmap& src);

	void swap(VDPixmapBuffer&);

protected:
	char *mpBuffer;
	size_t	mLinearSize;
};


#endif
