/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: picture_compress.h,v 1.7 2008/08/14 02:30:50 asuraparaju Exp $ $Name:  $
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
* Contributor(s): Thomas Davies (Original Author),
*                 Scott R Ladd,
*                 Anuradha Suraparaju
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


#ifndef _PICTURE_COMPRESS_H_
#define _PICTURE_COMPRESS_H_

#include <libdirac_encoder/enc_queue.h>
#include <libdirac_common/common.h>
#include <libdirac_common/motion.h>
#include <libdirac_byteio/picture_byteio.h>

namespace dirac
{

class MvData;

//! Compress a single image picture
/*!
    This class compresses a single picture at a time, using parameters
    supplied at its construction. PictureCompressor is used by
    SequenceCompressor.
*/
class PictureCompressor
{
public:
    //! Constructor
    /*!
        Creates a FrameEncoder with specific set of parameters the control
        the compression process. It encodes motion data before encoding
        each component of the picture.
        \param encp encoder parameters
    */
    PictureCompressor(EncoderParams& encp);

    //! Destructor
    ~PictureCompressor();

    //! Do pixel accurate motion estimate
    void PixelME(EncQueue& my_buffer , int pnum);

    //! Calculate the complexity of a picture
    void CalcComplexity(EncQueue& my_buffer, int pnum , const OLBParams& olbparams);
    void CalcComplexity2(EncQueue& my_buffer, int pnum);

    //! Normalise picture complexity with respect to others in the queue
    void NormaliseComplexity(EncQueue& my_buffer, int pnum);

    //! Do subpixel accurate motion vector refinement
    void SubPixelME(EncQueue& my_buffer , int pnum);

    //! Do mode decision based on sub-pel vectors
    void ModeDecisionME(EncQueue& my_buffer, int pnum);

    //! Detect cuts in the current picture
    void IntraModeAnalyse(EncQueue& my_buffer, int pnum);

    //! Does motion compensation on picture pnum (forward or backward)
    void MotionCompensate(EncQueue& my_buffer, int pnum, AddOrSub dirn);

    //! Prefilter if required
    void Prefilter(EncQueue& my_buffer, int pnum);

    //! Do the DWT on a given picture
    void DoDWT(EncQueue& my_buffer , int pnum, Direction dirn);

    //! Compress a specific picture within a group of pictures (GOP)
    /*!
        Compresses a specified picture within a group of pictures.
        \param my_pbuffer  picture buffer in which the reference frames resides
        \param pnum        picture number to compress
        \param pic_byteio  compressed picture in Dirac bytestream format
    */
    void CodeResidue(EncQueue& my_pbuffer , int pnum , PictureByteIO* pic_byteio);

    //! Compresses the motion vector data
    void CodeMVData(EncQueue& my_buffer, int pnum, PictureByteIO* pic_byteio);

    //! Returns true if the picture has been skipped rather than coded normally
    bool IsSkipped()
    {
        return m_skipped;
    }

    //! Returns true if Motion estimation data is available
    bool IsMEDataAvail() const
    {
        return m_medata_avail;
    }

    //! Returns the motion estimation data
    const MEData* GetMEData() const;

private:
    //! Copy constructor is private and body-less
    /*!
        Copy constructor is private and body-less. This class should not
        be copied.
    */
    PictureCompressor(const PictureCompressor& cpy);

    //! Assignment = is private and body-less
    /*!
        Assignment = is private and body-less. This class should not be
        assigned.
    */
    PictureCompressor& operator=(const PictureCompressor& rhs);

    //! Initialise the coefficient data array for holding wavelet coefficients
    void InitCoeffData(CoeffArray& coeff_data, const int xl, const int yl);

    //! Returns the value lambda according to picture and component type
    float GetCompLambda(const EncPicture& my_picture,
                        const CompSort csort);

    void SelectQuantisers(CoeffArray& coeff_data ,
                          SubbandList& bands ,
                          const float lambda,
                          OneDArray<unsigned int>& est_counts,
                          const CodeBlockMode cb_mode,
                          const PictureParams& pp,
                          const CompSort csort);

    int SelectMultiQuants(CoeffArray& coeff_data ,
                          SubbandList& bands ,
                          const int band_num,
                          const float lambda,
                          const PictureParams& pp,
                          const CompSort csort);

    void SetupCodeBlocks(SubbandList& bands);


    void AddSubAverage(CoeffArray& coeff_data, int xl, int yl, AddOrSub dirn);

private:

    //member variables
    // a local copy of the encoder params
    EncoderParams& m_encparams;

    // True if the picture has been skipped, false otherwise
    bool m_skipped;

    // True if we use global motion vectors, false otherwise
    bool m_use_global;

    // True if we use block motion vectors, false otherwise
    bool m_use_block_mv;

    // Prediction mode to use if we only have global motion vectors
    PredMode m_global_pred_mode;

    // A pointer to the current picture motion vector data
    MEData* m_me_data;

    // True if motion estimation data is available
    bool m_medata_avail;

    // True if we have detected a cut
    bool m_is_a_cut;

    // The original MV precision type
    MVPrecisionType m_orig_prec;

};

} // namespace dirac

#endif
