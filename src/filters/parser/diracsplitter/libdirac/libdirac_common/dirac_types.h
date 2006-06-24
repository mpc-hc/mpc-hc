/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: dirac_types.h 280 2005-01-30 05:11:46Z gabest $ $Name$
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
* Some basic enumeration types used by end user encoder and decoder ...//
*/
typedef ChromaFormat dirac_chroma_t;
typedef FrameSort dirac_frame_type_t;

typedef struct
{
    int numerator;
    int denominator;
} dirac_rational_t;

typedef dirac_rational_t dirac_frame_rate_t;

/*! Structure that holds the sequence parameters */
typedef struct
{
    /*! numper of pixels per line */
    int width;
    /*! number of lines per frame */
    int height;
    /*! chroma type */
    dirac_chroma_t chroma;
    /*! numper of pixels of chroma per line */
    int chroma_width;
    /*! number of lines of chroma per frame */
    int chroma_height;
    /*! frame rate */
    dirac_frame_rate_t frame_rate;
    /*! interlace flag: 0 - progressive; 1 - interlaced */
    int interlace;
    /*! top field comes first : 0 - false; 1 - true */
    int topfieldfirst;
} dirac_seqparams_t;

/*! Structure that holds the frame parameters */
typedef struct
{
    /*! frame type */
    dirac_frame_type_t ftype;
    /*! frame number in decoded order */
    int fnum;
} dirac_frameparams_t;


/*! Structure that holds the frame buffers into which data is written */
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
