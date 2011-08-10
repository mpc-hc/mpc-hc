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
#include "blt_setup.h"

void VDPixmapBlitterTable::Clear() {
	memset(mTable, 0, sizeof mTable);
}

void VDPixmapBlitterTable::AddBlitter(const VDPixmapFormatSubset& srcFormats, VDPixmapFormatSubset& dstFormats, VDPixmapBlitterFn blitter) {
	for(int i=0; i<srcFormats.mFormatCount; ++i) {
		int srcFormat = srcFormats.mFormats[i];
		for(int j=0; j<dstFormats.mFormatCount; ++j) {
			int dstFormat = dstFormats.mFormats[j];

			if (srcFormat != dstFormat)
				mTable[srcFormat][dstFormat] = blitter;
		}
	}
}
