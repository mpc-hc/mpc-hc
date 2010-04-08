/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: comp_compress.h,v 1.19 2008/05/07 05:47:00 asuraparaju Exp $ $Name:  $
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
* Contributor(s): Thomas Davies (Original Author), Scott R Ladd
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


#ifndef _COMP_COMPRESS_H_
#define _COMP_COMPRESS_H_

#include <libdirac_common/arrays.h>
#include <libdirac_common/wavelet_utils.h>
#include <libdirac_common/common.h>
#include <libdirac_byteio/component_byteio.h>

namespace dirac
{
class MEData;

//! Compress a picture component
/*!
    This class compresses one of the three components (Y, U, or V) of a
    picture according to a given set or parameters. CompCompressor is used
    by PictureCompressor.
*/
class CompCompressor
{
public:
    //! Constructor
    /*!
        Create and initialize a component compressor with the given
        characteristics.
        \param  encp    encoding parameters
        \param  fp      picture parameters
    */
    CompCompressor(EncoderParams & encp, const PictureParams& fp);

    //! Compress a picture component
    /*!
        Compress a PicArray containing a picture component (Y, U, or V).
        \param  coeff_data      the component data to be compressed
        \param  bands           Subbands list
        \param  csort           Chroma format
        \param  estimated_bits  the list of estimated number of bits in each subband
        \return Picture-component in Dirac-bytestream format
    */
    ComponentByteIO* Compress(CoeffArray& coeff_data ,
                              SubbandList& bands,
                              CompSort csort,
                              const OneDArray<unsigned int>& estimated_bits);

private:
    //! Copy constructor is private and body-less. This class should not be copied.
    CompCompressor(const CompCompressor& cpy);

    //! Assignment = is private and body-less. This class should not be assigned.
    CompCompressor& operator=(const CompCompressor& rhs);

    //! Set a subband to a constant value
    void SetToVal(CoeffArray& coeff_data, const Subband& node, ValueType val);


private:

    // member variables
    EncoderParams& m_encparams;

    const PictureParams& m_pparams;

    const PictureSort& m_psort;

    const ChromaFormat& m_cformat;

    float m_lambda;

};

} // namespace dirac



#endif
