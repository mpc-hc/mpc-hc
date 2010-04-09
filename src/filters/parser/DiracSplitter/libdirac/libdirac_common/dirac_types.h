/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: dirac_types.h,v 1.12 2008/11/18 23:25:54 asuraparaju Exp $ $Name:  $
*
* Version: MPL 1.1/GPL 2.0/LGPL 2.1
*
* The contents of this file are subject to the Mozilla Public License
* Version 1.1 (the "License"); you may not use this file except in compliance
* with the License. You may obtain a copy of the License at
* http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License for
* the specific language governing rights and limitations under the License.
*
* The Original Code is BBC Research and Development code.
*
* The Initial Developer of the Original Code is the British Broadcasting
* Corporation.
* Portions created by the Initial Developer are Copyright (C) 2004.
* All Rights Reserved.
*
* Contributor(s): Anuradha Suraparaju (Original Author)
*                 Andrew Kennedy
*
* Alternatively, the contents of this file may be used under the terms of
* the GNU General Public License Version 2 (the "GPL"), or the GNU Lesser
* Public License Version 2.1 (the "LGPL"), in which case the provisions of
* the GPL or the LGPL are applicable instead of those above. If you wish to
* allow use of your version of this file only under the terms of the either
* the GPL or LGPL and not to allow others to use your version of this file
* under the MPL, indicate your decision by deleting the provisions above
* and replace them with the notice and other provisions required by the GPL
* or LGPL. If you do not delete the provisions above, a recipient may use
* your version of this file under the terms of any one of the MPL, the GPL
* or the LGPL.
* ***** END LICENSE BLOCK ***** */

#ifndef _DIRAC_TYPES_H
#define _DIRAC_TYPES_H

#include <libdirac_common/common_types.h>

/*! This file contains common enumerated types used throughout 
    the end user interfaces to the encoder and decoder
*/

#ifdef __cplusplus
extern "C" {
#endif

#if defined(WIN32) && defined(_WINDLL)
#define DllExport   __declspec( dllexport )
#else
#define DllExport
#endif

/*
*   Major version corresponds to major version of the software.
*   Minor version corresponds to minor version of the software. Bump
*   this up by one whenever there are  major feature changes to the software.
*   Patch version corresponds to changes in the API. It should be
*   bumped up by 1 for every committed change to the API
*/
#define DIRAC_RESEARCH_MAJOR_VERSION 1   /* 0..255 */
#define DIRAC_RESEARCH_MINOR_VERSION 0   /* 0..255 */
#define DIRAC_RESEARCH_PATCH_VERSION 2   /* 0..255 */

#define DIRAC_RESEARCH_VERSION(X, Y, Z)      \
    (((X)<<16) + ((Y)<<8) + (Z))

#define DIRAC_RESEARCH_CURVERSION                        \
    DIRAC_RESEARCH_VERSION(DIRAC_RESEARCH_MAJOR_VERSION, \
    DIRAC_RESEARCH_MINOR_VERSION,                        \
    DIRAC_RESEARCH_PATCH_VERSION)

#define DIRAC_RESEARCH_VERSION_ATLEAST(X, Y, Z) \
    (DIRAC_RESEARCH_CURVERSION >= DIRAC_RESEARCH_VERSION(X, Y, Z))

/*
* Some basic enumeration types used by end user encoder and decoder ...//
*/
typedef ChromaFormat dirac_chroma_t;
typedef PictureType dirac_picture_type_t;
typedef ReferenceType dirac_reference_type_t;
typedef WltFilter dirac_wlt_filter_t;

typedef struct
{
    int numerator;
    int denominator;
} dirac_rational_t;

typedef dirac_rational_t dirac_frame_rate_t;
typedef dirac_rational_t dirac_pix_asr_t;

/*! Structure that holds the parase parameters */
typedef struct
{
    //! Major version
    unsigned int major_ver;
    //! Minor version
    unsigned int minor_ver;
    //! Profile
    unsigned int profile;
    //! level
    unsigned int level;
} dirac_parseparams_t;

typedef struct
{
    unsigned int width;
    unsigned int height;
    unsigned int left_offset;
    unsigned int top_offset;
} dirac_clean_area_t;

typedef struct
{
    unsigned int luma_offset;
    unsigned int luma_excursion;
    unsigned int chroma_offset;
    unsigned int chroma_excursion;
} dirac_signal_range_t;

typedef struct
{
    float kr;
    float kb;
} dirac_col_matrix_t;

typedef ColourPrimaries dirac_col_primaries_t;
typedef TransferFunction dirac_transfer_func_t;

typedef struct
{
    dirac_col_primaries_t col_primary;
    dirac_col_matrix_t col_matrix;
    dirac_transfer_func_t trans_func;
} dirac_colour_spec_t;

/*! Structure that holds the source parameters */
typedef struct
{
    /*! numper of pixels per line */
    unsigned int width;
    /*! number of lines per frame */
    unsigned int height;
    /*! chroma type */
    dirac_chroma_t chroma;
    /*! numper of pixels of chroma per line */
    unsigned int chroma_width;
    /*! number of lines of chroma per frame */
    unsigned int chroma_height;
    /*! source sampling field: 0 - progressive; 1 - interlaced */
    unsigned int source_sampling;
    /*! top field comes first : 0 - false; 1 - true. Set by Dirac library. */
    int topfieldfirst;
    /*! frame rate */
    dirac_frame_rate_t frame_rate;
    /*! pixel aspect ratio */
    dirac_pix_asr_t pix_asr;
    /* clean area*/
    dirac_clean_area_t clean_area;
    /* signal range*/
    dirac_signal_range_t signal_range;
    /* colour specification*/
    dirac_colour_spec_t colour_spec;

} dirac_sourceparams_t;

/*! Structure that holds the picture parameters */
typedef struct
{
    /*! picture type */
    dirac_picture_type_t ptype;
    /*! reference type */
    dirac_reference_type_t rtype;
    /*! picture number in decoded order */
    int pnum;
} dirac_picparams_t;


/*! Structure that holds the frame buffers into which data is written 
(NB we have frame-oriented IO even though we code pictures)*/
typedef struct
{
    /*! buffers to hold the luma and chroma data */
    unsigned char  *buf[3];
    /*! user data */
    void  *id;
} dirac_framebuf_t;

#ifdef __cplusplus
}
#endif

#endif 
