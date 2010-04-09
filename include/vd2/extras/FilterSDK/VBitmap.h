//	VirtualDub - Video processing and capture application
//	Copyright (C) 1998-2002 Avery Lee
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
//
//
//	FILTER EXEMPTION:
//
//	As a special exemption to the GPL in order to permit creation of
//	filters that work with multiple programs as well as VirtualDub,
//	compiling with this header file shall not be considered creation
//	of a derived work; that is, the act of compiling with this header
//	file does not require your source code or the resulting module
//	to be released in source code form or under a GPL-compatible
//	license according to parts (2) and (3) of the GPL.  A filter built
//	using this header file may thus be licensed or dual-licensed so
//	that it may be used with VirtualDub as well as an alternative
//	product whose license is incompatible with the GPL.
//
//	Nothing in this exemption shall be construed as applying to
//	VirtualDub itself -- that is, this exemption does not give you
//	permission to use parts of VirtualDub's source besides this
//	header file, or to dynamically link with VirtualDub as part
//	of the filter load process, in a fashion not permitted by the
//	GPL.

#ifndef f_VIRTUALDUB_VBITMAP_H
#define f_VIRTUALDUB_VBITMAP_H

#include <windows.h>

typedef unsigned long	Pixel;
typedef unsigned long	Pixel32;
typedef unsigned char	Pixel8;
typedef long			PixCoord;
typedef	long			PixDim;
typedef	long			PixOffset;

#ifdef VDEXT_VIDEO_FILTER
#define NOVTABLE __declspec(novtable)
#else
#define NOVTABLE
#endif

class NOVTABLE VBitmap {
public:
	Pixel *			data;
	Pixel *			palette;
	int				depth;
	PixCoord		w, h;
	PixOffset		pitch;
	PixOffset		modulo;
	PixOffset		size;
	PixOffset		offset;

	Pixel *Address(PixCoord x, PixCoord y) const {
		return Addressi(x, h-y-1);
	}

	Pixel *Addressi(PixCoord x, PixCoord y) const {
		return (Pixel *)((char *)data + y*pitch + x*(depth>>3));
	}

	Pixel *Address16(PixCoord x, PixCoord y) const {
		return Address16i(x, h-y-1);
	}

	Pixel *Address16i(PixCoord x, PixCoord y) const {
		return (Pixel *)((char *)data + y*pitch + x*2);
	}

	Pixel *Address32(PixCoord x, PixCoord y) const {
		return Address32i(x, h-y-1);
	}

	Pixel *Address32i(PixCoord x, PixCoord y) const {
		return (Pixel *)((char *)data + y*pitch + x*sizeof(Pixel));
	}

	PixOffset PitchAlign4() {
		return ((w * depth + 31)/32)*4;
	}

	PixOffset PitchAlign8() {
		return ((w * depth + 63)/64)*8;
	}

	PixOffset Modulo() {
		return pitch - (w*depth+7)/8;
	}

	PixOffset Size() {
		return pitch*h;
	}

	//////

	VBitmap() throw() {
#ifdef VDEXT_VIDEO_FILTER
		init();
#endif
	}
	VBitmap(void *data, PixDim w, PixDim h, int depth) throw();
	VBitmap(void *data, BITMAPINFOHEADER *) throw();

#ifdef VDEXT_VIDEO_FILTER
	void init() throw() { *(void **)this = g_vtbls.pvtblVBitmap; }
#endif

	virtual VBitmap& init(void *data, PixDim w, PixDim h, int depth) throw();
	virtual VBitmap& init(void *data, BITMAPINFOHEADER *) throw();

	virtual void MakeBitmapHeader(BITMAPINFOHEADER *bih) const throw();

	virtual void AlignTo4() throw();
	virtual void AlignTo8() throw();

	virtual void BitBlt(PixCoord x2, PixCoord y2, const VBitmap *src, PixCoord x1, PixCoord y1, PixDim dx, PixDim dy) const throw();
	virtual void BitBltDither(PixCoord x2, PixCoord y2, const VBitmap *src, PixDim x1, PixDim y1, PixDim dx, PixDim dy, bool to565) const throw();
	virtual void BitBlt565(PixCoord x2, PixCoord y2, const VBitmap *src, PixDim x1, PixDim y1, PixDim dx, PixDim dy) const throw();

	virtual bool BitBltXlat1(PixCoord x2, PixCoord y2, const VBitmap *src, PixCoord x1, PixCoord y1, PixDim dx, PixDim dy, const Pixel8 *tbl) const throw();
	virtual bool BitBltXlat3(PixCoord x2, PixCoord y2, const VBitmap *src, PixCoord x1, PixCoord y1, PixDim dx, PixDim dy, const Pixel32 *tbl) const throw();

	virtual bool StretchBltNearestFast(PixCoord x1, PixCoord y1, PixDim dx, PixDim dy, const VBitmap *src, double x2, double y2, double dx1, double dy1) const throw();

	virtual bool StretchBltBilinearFast(PixCoord x1, PixCoord y1, PixDim dx, PixDim dy, const VBitmap *src, double x2, double y2, double dx1, double dy1) const throw();

	virtual bool RectFill(PixCoord x1, PixCoord y1, PixDim dx, PixDim dy, Pixel32 c) const throw();

	enum {
		HISTO_LUMA,
		HISTO_GRAY,
		HISTO_RED,
		HISTO_GREEN,
		HISTO_BLUE,
	};

	virtual bool Histogram(PixCoord x, PixCoord y, PixCoord dx, PixCoord dy, long *pHisto, int iHistoType) const throw();

	//// NEW AS OF VIRTUALDUB V1.2B

	virtual bool BitBltFromYUY2(PixCoord x2, PixCoord y2, const VBitmap *src, PixCoord x1, PixCoord y1, PixDim dx, PixDim dy) const throw();
	virtual bool BitBltFromI420(PixCoord x2, PixCoord y2, const VBitmap *src, PixCoord x1, PixCoord y1, PixDim dx, PixDim dy) const throw();

	//// NEW AS OF VIRTUALDUB V1.4C

	virtual void MakeBitmapHeaderNoPadding(BITMAPINFOHEADER *bih) const throw();

	///////////

	bool BitBltFromYUY2Fullscale(PixCoord x2, PixCoord y2, const VBitmap *src, PixCoord x1, PixCoord y1, PixDim dx, PixDim dy) const throw();

private:
	bool dualrectclip(PixCoord& x2, PixCoord& y2, const VBitmap *src, PixCoord& x1, PixCoord& y1, PixDim& dx, PixDim& dy) const throw();
};

#undef NOVTABLE

#endif
