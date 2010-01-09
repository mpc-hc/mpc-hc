/*
    Copyright (C) 2001-2002 Michael Niedermayer (michaelni@gmx.at)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "postprocFilters.h"

//use if u want a faster postprocessing code
//cant differentiate between chroma & luma filters (both on or both off)
//obviosly the -pp option at the commandline has no effect except turning the here selected
//filters on
//#define COMPILE_TIME_MODE 0x77

#if 1
static inline int CLIP(int a){
	if(a&256) return ((a)>>31)^(-1);
	else      return a;
}
//#define CLIP(a) (((a)&256) ? ((a)>>31)^(-1) : (a))
#elif 0
#define CLIP(a) clip_tab[a]
#else
#define CLIP(a) (a)
#endif
/**
 * Postprocessng filter.
 */
struct PPFilter{
	char *shortName;
	char *longName;
	int chromDefault; 	///< is chrominance filtering on by default if this filter is manually activated
	int minLumQuality; 	///< minimum quality to turn luminance filtering on
	int minChromQuality;	///< minimum quality to turn chrominance filtering on
	int mask; 		///< Bitmask to turn this filter on
};

/**
 * postprocess context.
 */
typedef struct PPContext{
	uint8_t *tempBlocks; ///<used for the horizontal code

	/**
	 * luma histogram.
	 * we need 64bit here otherwise we'll going to have a problem
	 * after watching a black picture for 5 hours
	 */
	uint64_t *yHistogram;

	uint64_t __attribute__((aligned(8))) packedYOffset;
	uint64_t __attribute__((aligned(8))) packedYScale;

	/** Temporal noise reducing buffers */
	uint8_t *tempBlured[3];
	int32_t *tempBluredPast[3];

	/** Temporary buffers for handling the last row(s) */
	uint8_t *tempDst;
	uint8_t *tempSrc;

	uint8_t *deintTemp;

	uint64_t __attribute__((aligned(8))) pQPb;
	uint64_t __attribute__((aligned(8))) pQPb2;

	uint64_t __attribute__((aligned(8))) mmxDcOffset[64];
	uint64_t __attribute__((aligned(8))) mmxDcThreshold[64];

	QP_STORE_T *stdQPTable;       ///< used to fix MPEG2 style qscale
	QP_STORE_T *nonBQPTable;
	QP_STORE_T *forcedQPTable;

	int QP;
	int nonBQP;

	int frameNum;

	int cpuCaps;

	int qpStride; ///<size of qp buffers (needed to realloc them if needed)
	stride_t stride; ///<size of some buffers (needed to realloc them if needed)

	int hChromaSubSample;
	int vChromaSubSample;

	PPMode ppMode;
} PPContext;






