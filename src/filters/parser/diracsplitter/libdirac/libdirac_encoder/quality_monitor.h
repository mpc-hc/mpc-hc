/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: quality_monitor.h 280 2005-01-30 05:11:46Z gabest $ $Name$
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
* Contributor(s): Thomas Davies (Original Author)
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

#ifndef _QUALITY_MONITOR_H_
#define _QUALITY_MONITOR_H_

#include <libdirac_common/common.h>
#include <libdirac_common/frame.h>
#include <libdirac_common/wavelet_utils.h>
namespace dirac
{

    //! Class to monitor the quality of pictures and adjust coding parameters appropriately
    class QualityMonitor
    {
    public:

        //! Constructor. Sets up initial Lagrangian values
        /*
           Constructor sets up initial Lagrangian values.
        */
        QualityMonitor(EncoderParams& ep,
                       const SeqParams& sparams );

        ////////////////////////////////////////////////////////////
        //                                                        //
        //    Assumes default copy constructor,  assignment =     //
        //                 and destructor                         //
        ////////////////////////////////////////////////////////////

        //! Update the quality factors, returning true if we need to recode
        /*!
            Update the quality factors, returning true if we need to recode
            \param ld_frame the locally-decoded frame
            \param orig_frame the original frame
            \param count the number of times we've tried to code this frame before
        */
        bool UpdateModel(const Frame& ld_frame, const Frame& orig_frame ,const int count);

        //! Reset the quality factors (say if there's been a cut)
        void ResetAll();

    private:
        //functions

        //! Use the model parameters to calculate the resulting Lagrangian parameters
        void CalcNewLambdas(const FrameSort fsort, const double slope, const double offset);

        //! Calculate the quality of coded wrt original picture
        double QualityVal( const PicArray& coded_data , 
                                           const PicArray& orig_data ,
                                           double cpd , 
                                           const FrameSort fsort );

        //member variables//
        ////////////////////

        //! A reference to the encoder parameters
        EncoderParams& m_encparams;

        //! The chroma format
        const ChromaFormat m_cformat;

        //! The true picture width, minus padding
        const int m_true_xl;

        //! The true picture height, minus padding
        const int m_true_yl;

        // target quality values for each frame type
        OneDArray<double> m_target_quality;

        // weighted PSNR values for last of each frame type
        OneDArray<double> m_last_quality;

        /* Default Model parameters for quality wrt to log10(lambda)
           Model is : 
                quality = offset + slope * log10( lambda )
            for each lambda parameter type.
            Default parameters will be used if it's not possible to measure
            them, and updated using measured data
        */
        OneDArray<double> m_slope;
        OneDArray<double> m_offset;

        //! Lagrangian parameters for the last I, L1 and L2 frames
        OneDArray<double> m_last_lambda;

        //! The Lagrangian ME parameters
        double m_L1_me_lambda, m_L2_me_lambda;

        //! The ratio of Lagrangian ME parameters to frame motion estimation parameters
        double m_me_ratio;

    };

} // namespace dirac

#endif
