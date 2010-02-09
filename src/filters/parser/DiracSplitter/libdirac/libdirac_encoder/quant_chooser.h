/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: quant_chooser.h,v 1.7 2008/05/27 01:29:54 asuraparaju Exp $ $Name:  $
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


#ifndef _QUANT_CHOOSER_H_
#define _QUANT_CHOOSER_H_

#include <libdirac_common/arrays.h>
#include <libdirac_common/wavelet_utils.h>
#include <libdirac_common/common.h>

namespace dirac
{
    //! Choose a quantiser
    /*!
        This class chooses a quantiser or quantisers for a subband 
    */
    class QuantChooser
    {
    public:

        //! Constructor
        QuantChooser( const CoeffArray& pic_data , const float lambda  );

        //! Finds the best quantisers for the subband, returning the predicted number of bits needed
        int GetBestQuant( Subband& node );

        //! Sets the factor used for correcting the entropy calculation
        void SetEntropyCorrection( const float ecfac ){ m_entropy_correctionfactor = ecfac; }
    private:
        //! Copy constructor is private and body-less. This class should not be copied.
        QuantChooser(const QuantChooser& cpy);

        //! Assignment = is private and body-less. This class should not be assigned.
        QuantChooser& operator=(const QuantChooser& rhs);
 
        //! Calculate errors and entropies for integral-bit quantisers
        void IntegralErrorCalc( Subband& node , const int xratio , const int yratio );

        //! Calculate errors and entropies for non-integral-bit quantisers
        void NonIntegralErrorCalc( Subband& node, const int xratio, const int yratio );

        //! Having got statistics, calculate the Lagrangian costs
        void LagrangianCalc();

        //! Select the best quantisation index on the basis of the Lagrangian calculations
        void SelectBestQuant();

        CoeffType BlockAbsMax( const Subband& node );

        //! Set the skip flag for a codeblock
        void SetSkip( CodeBlock& cblock , const int qidx);

    private:
        //! The perceptual weighting factor of the subband being tested
        float m_subband_wt;

        //! The smallest quantisation index being tested
        int m_bottom_idx;
        //! The largest quantisation index being tested
        int m_top_idx;
        //! The step we use in jumping through the list of quantisers
        int m_index_step;

        //! The index of the quantiser with the lowest cost
        int m_min_idx;

        //! A local reference to the data under consideration
        const CoeffArray& m_coeff_data;

        //!  The lambda value to be used in the Lagrangian calculation
        const float m_lambda;

        //! A value for correcting the crude calculation of the entropy
        float m_entropy_correctionfactor;

        //! An array used to count the number of zeroes
        OneDArray<int> m_count0;
        //! The number of ones (equal to the number of coefficients)
        int m_count1;
        //! An array used to count the number of positive values
        OneDArray<int> m_countPOS;
        //! An array used to count the number of negative values
        OneDArray<int> m_countNEG;    
        //! An array used to collate the sum of the perceptually-weighted errors
        OneDArray<double> m_error_total;
        //! An array used to collate the computed costs
        OneDArray<CostType> m_costs;

    };

} // namespace dirac



#endif
