//	VirtualDub - Video processing and capture application
//	Graphics support library
//	Copyright (C) 1998-2008 Avery Lee
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

#ifndef f_VD2_KASUMI_TRIBLT_H
#define f_VD2_KASUMI_TRIBLT_H

#include <vd2/system/vdstl.h>
#include <vd2/Kasumi/pixmaputils.h>
#include <vector>

struct VDTriBltVertex {
	float x, y, z, u, v;
};

struct VDTriColorVertex {
	float x, y, z, r, g, b, a;
};

enum VDTriBltFilterMode {
	kTriBltFilterPoint,
	kTriBltFilterBilinear,
	kTriBltFilterTrilinear,
	kTriBltFilterBicubicMipLinear,
	kTriBltFilterCount
};

bool VDPixmapTriFill(VDPixmap& dst, uint32 c,
					const VDTriBltVertex *pVertices, int nVertices,
					const int *pIndices, const int nIndices,
					const float pTransform[16] = NULL);

bool VDPixmapTriFill(VDPixmap& dst,
					const VDTriColorVertex *pVertices, int nVertices,
					const int *pIndices, const int nIndices,
					const float pTransform[16] = NULL,
					const float *chroma_yoffset = NULL);

bool VDPixmapTriBlt(VDPixmap& dst, const VDPixmap *const *pSources, int nMipmaps,
					const VDTriBltVertex *pVertices, int nVertices,
					const int *pIndices, const int nIndices,
					VDTriBltFilterMode filterMode,
					float mipMapLODBias,
					const float pTransform[16] = NULL);

class VDPixmapTextureMipmapChain {
public:
	VDPixmapTextureMipmapChain(const VDPixmap& src, bool wrap=false, bool cubic = false, int maxlevels = 16);

	const VDPixmap *const *Mips() const { return mMipMaps.data(); }
	int Levels() const { return mMipMaps.size(); }

protected:
	std::vector<VDPixmapBuffer>		mBuffers;
	vdfastvector<const VDPixmap *>	mMipMaps;
};

#endif
