#ifndef f_VD2_PLUGIN_VDVIDEOUTIL_H
#define f_VD2_PLUGIN_VDVIDEOUTIL_H

template<class T>
T *vd_ptroffset(T *p, ptrdiff_t diff) {
	return (T *)((char *)p + diff);
}

template<class T>
class vd_row_iter {
public:
	vd_row_iter() {}
	vd_row_iter(T *p, ptrdiff_t pitch) : mp(p), mpitch(pitch) {}
	vd_row_iter(T *p, ptrdiff_t pitch, int y) : mp(vd_ptroffset(p, pitch*y)), mpitch(pitch) {}

	vd_row_iter(const VFBitmap& bm, int x = 0, int y = 0) : mp(vd_ptroffset((T*)bm.data, bm.pitch*(bm.h - 1 - y))+x), mpitch(-bm.pitch) {}

	operator T*() const { return mp; }
	T& operator[](int x) const { return mp[x]; }

	void mulstep(int x) {
		mpitch *= x;
	}

	const vd_row_iter<T>& operator+=(int y) {
		mp = vd_ptroffset(mp, mpitch * y);
		return *this;
	}

	const vd_row_iter<T>& operator-=(int y) {
		mp = vd_ptroffset(mp, -(mpitch * y));
		return *this;
	}

	const vd_row_iter<T>& operator++() {
		mp = vd_ptroffset(mp, mpitch);
		return *this;
	}

	const vd_row_iter<T> operator++(int) {
		const vd_row_iter<T> temp(*this);
		mp = vd_ptroffset(mp, mpitch);
		return temp;
	}

	const vd_row_iter<T>& operator--() {
		mp = vd_ptroffset(mp, -mpitch);
		return *this;
	}

	const vd_row_iter<T> operator--(int) {
		const vd_row_iter<T> temp(*this);
		mp = vd_ptroffset(mp, -mpitch);
		return temp;
	}

protected:
	T *mp;
	ptrdiff_t mpitch;
};

typedef vd_row_iter<uint32> vd_pixrow_iter;

inline uint32 vd_pixavg_down(uint32 x, uint32 y) {
	return (x&y) + (((x^y)&0xfefefefe)>>1);
}

inline uint32 vd_pixavg_up(uint32 x, uint32 y) {
	return (x|y) - (((x^y)&0xfefefefe)>>1);
}

inline void vd_pixunpack(uint32 px, int& r, int& g, int& b) {
	r = (px>>16)&255;
	g = (px>> 8)&255;
	b = (px    )&255;
}

inline uint32 vd_pixpack(int r, int g, int b) {
	if ((unsigned)r >= 256) r = ~(r>>31) & 255;
	if ((unsigned)g >= 256) g = ~(g>>31) & 255;
	if ((unsigned)b >= 256) b = ~(b>>31) & 255;

	return (r<<16) + (g<<8) + b;
}

inline uint32 vd_pixpackfast(int r, int g, int b) {
	return (r<<16) + (g<<8) + b;
}

struct vd_transform_pixmap_helper {
	vd_transform_pixmap_helper(const VFBitmap& dst)
		: p((uint32 *)dst.data)
		, pitch(dst.pitch)
		, w(dst.w)
		, h(dst.h) {}

	operator bool() const { return false; }

	uint32 *p;
	const ptrdiff_t pitch;
	const int w, h;
};

#define vd_transform_pixmap_blt(dst, src)		\
	if(vd_transform_pixmap_helper dstinfo = dst);else					\
	if(vd_transform_pixmap_helper srcinfo = src);else					\
	for(int y = 0, h = dstinfo.h, w = dstinfo.w; y < h; ++y, dstinfo.p=vd_ptroffset(dstinfo.p, dstinfo.pitch), srcinfo.p=vd_ptroffset(srcinfo.p, srcinfo.pitch))	\
	for(int x = 0; x < dstinfo.w; ++x)			\
	switch(unsigned& out = dstinfo.p[x]) case 0: default:		\
	switch(const unsigned& in = srcinfo.p[x]) case 0: default:

#define vd_transform_pixmap_inplace(dst)		\
	if(vd_transform_pixmap_helper dstinfo = dst);else					\
	for(int y = 0, h = dstinfo.h, w = dstinfo.w; y < h; ++y, dstinfo.p=vd_ptroffset(dstinfo.p, dstinfo.pitch))	\
	for(int x = 0; x < dstinfo.w; ++x)			\
	switch(unsigned& px = dstinfo.p[x]) case 0: default:		\

#define vd_maketable256_16(x) formula((x+0)),formula((x+1)),formula((x+2)),formula((x+3)),formula((x+4)),formula((x+5)),formula((x+6)),formula((x+7)),formula((x+8)),formula((x+9)),formula((x+10)),formula((x+11)),formula((x+12)),formula((x+13)),formula((x+14)),formula((x+15))
#define vd_maketable256 vd_maketable256_16(0x00),vd_maketable256_16(0x10),vd_maketable256_16(0x20),vd_maketable256_16(0x30),vd_maketable256_16(0x40),vd_maketable256_16(0x50),vd_maketable256_16(0x60),vd_maketable256_16(0x70),vd_maketable256_16(0x80),vd_maketable256_16(0x90),vd_maketable256_16(0xA0),vd_maketable256_16(0xB0),vd_maketable256_16(0xC0),vd_maketable256_16(0xD0),vd_maketable256_16(0xE0),vd_maketable256_16(0xF0),

#endif
