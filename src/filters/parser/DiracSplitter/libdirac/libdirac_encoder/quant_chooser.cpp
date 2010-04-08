/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: quant_chooser.cpp,v 1.20 2009/01/21 05:22:05 asuraparaju Exp $ $Name:  $
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


//Compression of an individual component,
//after motion compensation if appropriate
//////////////////////////////////////////

#include <libdirac_encoder/quant_chooser.h>

using namespace dirac;

// Custom 4th power, to speed things up
static inline double pow4(double x)
{
    return x * x * x * x;
}

// Constructor
QuantChooser::QuantChooser(const CoeffArray& coeff_data,
                           const float lambda):
    m_coeff_data(coeff_data),
    m_lambda(lambda),
    m_entropy_correctionfactor(1.0)
{}


int QuantChooser::GetBestQuant(Subband& node)
{
    // NB : quantiser selection only supports a single quantiser per subband
    // Setting MultiQuants=true and using this function will get the same
    // quantiser for each codeblock

    m_subband_wt = node.Wt();

    // The largest value in the block or band
    CoeffType max_val;

    // The index of the maximum bit of the largest value
    int max_bit(0);

    max_val = BlockAbsMax(node);

    if(max_val >= 1)
        max_bit = int(std::floor(std::log(float(max_val)) / std::log(2.0)));
    else
    {
        // Exit saying 'Skip this subband' if there's no data in it
        node.SetSkip(true);
        return 0;
    }

    // The number of quantisers to be tested
    int num_quants(4 * max_bit + 5);

    // Set the array sizes
    m_costs.Resize(num_quants);
    m_count0.Resize(num_quants);
    m_count1 = node.Xl() * node.Yl();
    m_countPOS.Resize(num_quants);
    m_countNEG.Resize(num_quants);
    m_error_total.Resize(num_quants);

    // Total estimated bits for the subband
    double bit_sum(0.0);

    // Step 1. Do integral bits first
    m_bottom_idx = 0;
    m_top_idx = num_quants - 1;
    m_index_step = 4;

    IntegralErrorCalc(node, 2 , 2);
    LagrangianCalc();
    SelectBestQuant();

    // Step 2. Do 1/2-bit accuracy next
    m_bottom_idx = std::max(m_min_idx - 2 , 0);
    m_top_idx = std::min(m_min_idx + 2 , num_quants - 1);
    m_index_step = 2;

    NonIntegralErrorCalc(node, 2 , 2);
    LagrangianCalc();
    SelectBestQuant();

    // Step 3. Finally, do 1/4-bit accuracy next
    m_bottom_idx = std::max(m_min_idx - 1 , 0);
    m_top_idx = std::min(m_min_idx + 1 , num_quants - 1);
    m_index_step = 1;

    NonIntegralErrorCalc(node, 1 , 2);
    LagrangianCalc();
    SelectBestQuant();

    bit_sum = m_costs[m_min_idx].ENTROPY * node.Xl() * node.Yl();

    node.SetQuantIndex(m_min_idx);

    TwoDArray<CodeBlock>& block_list(node.GetCodeBlocks());

    // Set the codeblock quantisers
    for(int j = 0 ; j < block_list.LengthY() ; ++j)
        for(int i = 0 ; i < block_list.LengthX() ; ++i)
            block_list[j][i].SetQuantIndex(m_min_idx);

    // Set the codeblock skip flags
    for(int j = 0 ; j < block_list.LengthY() ; ++j)
        for(int i = 0 ; i < block_list.LengthX() ; ++i)
            SetSkip(block_list[j][i], m_min_idx);


    return static_cast<int>(bit_sum);

}

void QuantChooser::IntegralErrorCalc(Subband& node, const int xratio , const int yratio)
{

    CoeffType val, quant_val , abs_val;

    CalcValueType error;

    m_count1 = ((node.Xl() / xratio) * (node.Yl() / yratio));
    for(int q = m_bottom_idx ; q <= m_top_idx ; q += 4)
    {
        m_error_total[q] = 0.0;
        m_count0[q] = 0;
        m_countPOS[q] = 0;
        m_countNEG[q] = 0;
    }

    // Work out the error totals and counts for each quantiser
    for(int j = node.Yp(); j < node.Yp() + node.Yl() ; j += yratio)
    {
        for(int i = node.Xp(); i < node.Xp() + node.Xl() ; i += xratio)
        {
            val = m_coeff_data[j][i];
            abs_val = quant_val = abs(val);

            int q = m_bottom_idx;
            for(; q <= m_top_idx ; q += 4)
            {
                // Quantiser is 2^(q/4), so we divide by this
                quant_val >>= (q >> 2);
                if(!quant_val)
                    break;

                m_count0[q] += quant_val;
                // Multiply back up so that we can quantise again in the next loop step
                quant_val <<= (q >> 2) + 2;
                quant_val += dirac_quantiser_lists.InterQuantOffset4(q) + 2;
                quant_val >>= 2;
                if(val > 0)
                    m_countPOS[q]++;
                else
                    m_countNEG[q]++;

                error = abs_val - quant_val;

                // Using the fourth power to measure the error
                m_error_total[q] +=  pow4(static_cast<double>(error));

            }// q
            double derror = pow4(static_cast<double>(abs_val));
            for(; q <= m_top_idx; q += 4)
            {
                m_error_total[q] += derror;
            }
        }// i
    }// j

}

void QuantChooser::NonIntegralErrorCalc(Subband& node , const int xratio , const int yratio)
{

    CoeffType val, abs_val;

    CalcValueType quant_val;
    CalcValueType error;

    m_count1 = ((node.Xl() / xratio) * (node.Yl() / yratio));
    for(int q = m_bottom_idx ; q <= m_top_idx ; q += m_index_step)
    {
        m_error_total[q] = 0.0;
        m_count0[q] = 0;
        m_countPOS[q] = 0;
        m_countNEG[q] = 0;
    }

    // Work out the error totals and counts for each quantiser
    for(int j = node.Yp(); j < node.Yp() + node.Yl() ; j += yratio)
    {
        for(int i = node.Xp(); i < node.Xp() + node.Xl() ; i += xratio)
        {

            val = m_coeff_data[j][i];
            abs_val = abs(val);

            int q = m_bottom_idx;
            for(; q <= m_top_idx ; q += m_index_step)
            {
                // Since the quantiser isn't a power of 2 we have to divide each time
                quant_val = static_cast<CalcValueType>(abs_val);
                quant_val <<= 2;
                quant_val /= dirac_quantiser_lists.QuantFactor4(q);

                if(!quant_val)
                    break;

                m_count0[q] += quant_val;
                quant_val *= dirac_quantiser_lists.QuantFactor4(q);
                quant_val += dirac_quantiser_lists.InterQuantOffset4(q) + 2;
                quant_val >>= 2;

                if(val > 0)
                    m_countPOS[q]++;
                else
                    m_countNEG[q]++;

                error = abs_val - quant_val;
                m_error_total[q] += pow4(error);
            }// q
            double derror = pow4(abs_val);
            for(; q <= m_top_idx; q += m_index_step)
                m_error_total[q] += derror;
        }// i
    }// j

}


void QuantChooser::LagrangianCalc()
{

    // probabilities
    double p0, p1;

    double sign_entropy;

    // Do Lagrangian costs calculation
    for(int q = m_bottom_idx ; q <= m_top_idx ; q += m_index_step)
    {

        m_costs[q].Error = m_error_total[q] / double(m_count1);
        m_costs[q].Error = std::sqrt(m_costs[q].Error) / (m_subband_wt * m_subband_wt);

        // Calculate probabilities and entropy
        p0 = double(m_count0[q]) / double(m_count0[q] + m_count1);
        p1 = 1.0 - p0;

        if(p0 != 0.0 && p1 != 0.0)
            m_costs[q].ENTROPY = -(p0 * std::log(p0) + p1 * std::log(p1)) / std::log(2.0);
        else
            m_costs[q].ENTROPY = 0.0;

        // We want the entropy *per symbol*, not per bit ...
        m_costs[q].ENTROPY *= double(m_count0[q] + m_count1);
        m_costs[q].ENTROPY /= double(m_count1);

        // Now add in the sign entropy
        if(m_countPOS[q] + m_countNEG[q] != 0)
        {
            p0 = double(m_countNEG[q]) / double(m_countPOS[q] + m_countNEG[q]);
            p1 = 1.0 - p0;
            if(p0 != 0.0 && p1 != 0.0)
                sign_entropy = -((p0 * std::log(p0) + p1 * std::log(p1)) / std::log(2.0));
            else
                sign_entropy = 0.0;
        }
        else
            sign_entropy = 0.0;

        // We want the entropy *per symbol*, not per bit ...
        sign_entropy *= double(m_countNEG[q] + m_countPOS[q]);
        sign_entropy /= double(m_count1);

        m_costs[q].ENTROPY += sign_entropy;

        // Sort out correction factors
        m_costs[q].ENTROPY *= m_entropy_correctionfactor;
        m_costs[q].TOTAL = m_costs[q].Error + m_lambda * m_costs[q].ENTROPY;

    }// q
}

void QuantChooser::SelectBestQuant()
{
    // Selects the best quantiser to use for a subband block

    m_min_idx = m_bottom_idx;
    for(int q = m_bottom_idx + m_index_step; q <= m_top_idx ; q += m_index_step)
    {
        if(m_costs[q].TOTAL < m_costs[m_min_idx].TOTAL)
            m_min_idx = q;
    }// q

}

void QuantChooser::SetSkip(CodeBlock& cblock , const int qidx)
{
    const int u_threshold = dirac_quantiser_lists.QuantFactor4(qidx);

    // Sets the skip flag for a codeblock
    bool can_skip = true;
    for(int j = cblock.Ystart(); j < cblock.Yend(); ++j)
    {
        for(int i = cblock.Xstart(); i < cblock.Xend(); ++i)
        {
            if((std::abs(m_coeff_data[j][i]) << 2) >= u_threshold)
                can_skip = false;
        }
    }
    cblock.SetSkip(can_skip);
}

CoeffType QuantChooser::BlockAbsMax(const Subband& node)
{
    int val(0);

    for(int j = node.Yp() ; j < node.Yp() + node.Yl(); ++j)
    {
        for(int i = node.Xp() ; i < node.Xp() + node.Xl(); ++i)
        {
            val = std::max(val , std::abs(m_coeff_data[j][i]));
        }// i
    }// j

    return val;
}
