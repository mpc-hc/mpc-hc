/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: video_format_defaults.h,v 1.7 2008/10/01 00:26:20 asuraparaju Exp $ $Name:  $
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
* Contributor(s): Andrew Kennedy (Original Author),
*                 Anuradha Suraparaju
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

/**
* Returns structures containing default parameter values for different video-formats
*/
#ifndef video_format_defaults_h
#define video_format_defaults_h

//LOCAL INCLUDES
#include <libdirac_common/common.h>         // SeqParams

namespace dirac
{
/**
* Sets default codec parameters - common to encoder and decoder
*@param cparams   Codec Params objects for setting defaults
*@param ptype     Picture type i,e, INTRA or INTER
*@param num_refs  Number of reference frames
*/
void SetDefaultCodecParameters(CodecParams &cparams, PictureType ptype, unsigned int num_refs);

/**
* Sets default encoder parameters
*@param encparams Params objects for setting defaults
*/
void SetDefaultEncoderParameters(EncoderParams& encparams);

/**
* Sets default Source parameters
*@param vf      Video Format
*@param sparams Params object for setting defaults
*/
void SetDefaultSourceParameters(const VideoFormat &vf, SourceParams& sparams);

/**
* Sets default block parameters
*@param bparams Params object for setting defaults
*@param video_format Video format
*/
void SetDefaultBlockParameters(OLBParams& bparams,
                               const VideoFormat& video_format);

/**
* Sets default block parameters
*@param bparams Params object for setting defaults
*@param pidx Index into Block Parameters table
*/
void SetDefaultBlockParameters(OLBParams& bparams,
                               int pidx);
/**
* Returns index of  block parameters in Defaults table
*@param bparams Params object for getting index
*/
unsigned int BlockParametersIndex(const OLBParams& bparams);

/**
* Sets the default Transform filter depending on picture type
*@param ptype    Picture type i.e. intra or inter
*@param video_format The video format
*@param wf       WltFilter object for getting the default wavelet filter
*/
void SetDefaultTransformFilter(const PictureType ptype, const VideoFormat video_format,
                               WltFilter &wf);
} // namespace dirac

#endif
