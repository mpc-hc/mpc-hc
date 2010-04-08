/*
    Copyright (C) 2001-2003 Michael Niedermayer (michaelni@gmx.at)

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

#ifndef NEWPOSTPROCESS_H
#define NEWPOSTPROCESS_H

/**
 * @file postprocess.h
 * @brief
 *     external api for the pp stuff
 */

#ifdef __cplusplus
extern "C" {
#endif

#define PP_QUALITY_MAX 6

#define QP_STORE_T int8_t

#include "postprocFilters.h"
#define FF_CSP_ONLY
#include "ffImgfmt.h"

    /**
     * Postprocessng mode.
     */
    typedef struct PPMode
    {
        int lumMode; 			///< acivates filters for luminance
        int chromMode; 			///< acivates filters for chrominance
        int error; 			///< non zero on error

        int minAllowedY; 		///< for brigtness correction
        int maxAllowedY; 		///< for brihtness correction
        float maxClippedThreshold;      ///< amount of "black" u r willing to loose to get a brightness corrected picture

        int maxTmpNoise[3]; 		///< for Temporal Noise Reducing filter (Maximal sum of abs differences)

        int baseDcDiff;
        int flatnessThreshold;

        int forcedQuant; 		///< quantizer if FORCE_QUANT is used
    } PPMode;

    typedef void pp_context_t;
    typedef PPMode pp_mode_t;

    void  pp_postprocess(uint8_t * src[3], stride_t srcStride[3],
                         uint8_t * dst[3], stride_t dstStride[3],
                         int horizontalSize, int verticalSize,
                         QP_STORE_T *QP_store,  int QP_stride,
                         pp_mode_t *mode, pp_context_t *ppContext, int pict_type);


    pp_context_t *pp_get_context(int width, int height, int flags);
    void pp_free_context(pp_context_t *ppContext);

#define PP_CPU_CAPS_MMX   0x80000000
#define PP_CPU_CAPS_MMX2  0x20000000
#define PP_CPU_CAPS_3DNOW 0x40000000
#define PP_CPU_CAPS_ALTIVEC 0x10000000

#define PP_FORMAT         0x00000008
#define PP_FORMAT_420    (0x00000011|PP_FORMAT)
#define PP_FORMAT_422    (0x00000001|PP_FORMAT)
#define PP_FORMAT_411    (0x00000002|PP_FORMAT)
#define PP_FORMAT_444    (0x00000000|PP_FORMAT)
#define PP_FORMAT_410    (0x00000022|PP_FORMAT)

#define PP_PICT_TYPE_QP2  0x00000010 ///< MPEG2 style QScale

#ifdef __cplusplus
}
#endif

#endif
