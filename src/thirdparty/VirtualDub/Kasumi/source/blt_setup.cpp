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
