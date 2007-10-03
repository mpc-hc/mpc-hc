/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: quant_chooser.cpp,v 1.11 2007/04/11 07:52:06 tjdwave Exp $ $Name: Dirac_0_7_0 $
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
static inline double pow4 (double x)
{
    return x * x * x* x;
}

// Constructor
QuantChooser::QuantChooser(  const PicArray& pic_data, 
                             const float lambda ):
    m_pic_data( pic_data ),
    m_lambda( lambda ),
    m_entropy_correctionfactor( 1.0 )
{}


int QuantChooser::GetBestQuant( Subband& node )
{

    TwoDArray<CodeBlock>& block_list( node.GetCodeBlocks() );
    const int num_blocks( block_list.LengthX() * block_list.LengthY() );

    // The largest value in the block or band
    ValueType max_val;

    // The index of the maximum bit of the largest value
    int max_bit( 0 );

    CodeBlock big_block( node.Xp() , node.Yp() , node.Xp()+node.Xl() , node.Yp()+node.Yl() );
    max_val = BlockAbsMax( big_block );

    if ( max_val>=1 )
        max_bit = int( std::floor( std::log( float( max_val ) )/std::log( 2.0 ) ) );
    else
    {
       // Exit saying 'Skip this subband' if there's no data in it
        node.SetSkip( true );
        return 0;
    }

    // The number of quantisers to be tested
    int num_quants( 4 * max_bit + 5 );

    // Set the array sizes
    m_costs.Resize( num_blocks , num_quants );
    m_count0.Resize(  num_blocks , num_quants );
    m_count1.Resize(  num_blocks );
    m_countPOS.Resize( num_blocks , num_quants );
    m_countNEG.Resize(  num_blocks , num_quants );
    m_error_total.Resize(  num_blocks , num_quants );

    // Total estimated bits for the subband
    double bit_sum( 0.0 );

    // Number of bits for an individual block  
    double block_bit_cost;

    if ( !node.UsingMultiQuants() )
    {
    // We have just one quantiser for the whole band. We still want to collate R-D info by
    // block so that we know which blocks we can skip

        int block_idx;

        // Step 1. Do integral bits first
        m_bottom_idx = 0;
        m_top_idx = num_quants-1;
        m_index_step = 4;

        for (int j=0 ; j<block_list.LengthY() ; ++j )
        {
            for (int i=0 ; i<block_list.LengthX() ; ++i )
            {
                block_idx = j*block_list.LengthX() + i;     

                IntegralErrorCalc( block_list[j][i] , block_idx , 2 , 2);
                LagrangianCalc( block_list[j][i] , block_idx  );
            }// i
        }// j
        SelectBestQuant();


        // Step 2. Do 1/2-bit accuracy next
        m_bottom_idx = std::max( m_min_idx - 2 , 0 );
        m_top_idx = std::min( m_min_idx + 2 , num_quants-1 );
        m_index_step = 2;

        for (int j=0 ; j<block_list.LengthY() ; ++j )
        {
            for (int i=0 ; i<block_list.LengthX() ; ++i )
            {
                block_idx = j*block_list.LengthX() + i;     

                NonIntegralErrorCalc( block_list[j][i] , block_idx  , 2 , 2);
                LagrangianCalc( block_list[j][i] , block_idx  );
            }// i
        }// j
        SelectBestQuant();

        // Step 3. Finally, do 1/4-bit accuracy next
        m_bottom_idx = std::max( m_min_idx - 1 , 0 );
        m_top_idx = std::min( m_min_idx + 1 , num_quants-1 );
        m_index_step = 1;

        for (int j=0 ; j<block_list.LengthY() ; ++j )
        {
            for (int i=0 ; i<block_list.LengthX() ; ++i )
            {
                block_idx = j*block_list.LengthX() + i;     

                NonIntegralErrorCalc( block_list[j][i] , block_idx  , 1 , 2);
                LagrangianCalc( block_list[j][i] , block_idx  );
            }// i
        }// j
        SelectBestQuant();

        for (int j=0 ; j<block_list.LengthY() ; ++j )
        {
            for (int i=0 ; i<block_list.LengthX() ; ++i )
            {
                block_idx = j*block_list.LengthX() + i;     

                block_bit_cost = ( m_costs[block_idx][m_min_idx].ENTROPY 
                           * block_list[j][i].Xl() 
                           * block_list[j][i].Yl() );

                bit_sum += block_bit_cost;

                block_list[j][i].SetQIndex( m_min_idx );

                if ( block_bit_cost >= 1.0 )
                    block_list[j][i].SetSkip( false );
                else
                    // We can skip this block after all
                    block_list[j][i].SetSkip( true );

            }// i
        }// j

        node.SetQIndex( m_min_idx );

        return static_cast<int>( bit_sum );

    }
    else
    {
        int block_idx;

        for (int j=0 ; j<block_list.LengthY() ; ++j )
        {
            for (int i=0 ; i<block_list.LengthX() ; ++i )
            {
                block_idx = j*block_list.LengthX() + i;     
                // Step 1. Do integral bits first
                m_bottom_idx = 4;
                m_top_idx = num_quants-1;
                m_index_step = 4;

                IntegralErrorCalc( block_list[j][i] , block_idx , 2 , 2);
                LagrangianCalc( block_list[j][i] , block_idx  );
                SelectBestQuant( block_idx );

                // Step 2. Do 1/2-bit accuracy next
                m_bottom_idx = std::max( m_min_idx - 2 , 0 );
                m_top_idx = std::min( m_min_idx + 2 , num_quants-1 );
                m_index_step = 2;

                NonIntegralErrorCalc( block_list[j][i] , block_idx  , 2 , 2);
                LagrangianCalc( block_list[j][i] , block_idx  );
                SelectBestQuant( block_idx );

                // Step 3. Finally, do 1/4-bit accuracy next
                m_bottom_idx = std::max( m_min_idx - 1 , 0 );
                m_top_idx = std::min( m_min_idx + 1 , num_quants-1 );
                m_index_step = 1;

                NonIntegralErrorCalc( block_list[j][i] , block_idx  , 1 , 2);
                LagrangianCalc( block_list[j][i] , block_idx  );
                SelectBestQuant( block_idx );

                block_bit_cost = ( m_costs[block_idx][m_min_idx].ENTROPY 
                                 * block_list[j][i].Xl() 
                                 * block_list[j][i].Yl() );
                bit_sum += block_bit_cost;

                block_list[j][i].SetQIndex( m_min_idx );

                if ( block_bit_cost < 1.0 )
                    // We can skip this block after all
                    block_list[j][i].SetSkip( true );
            }// i
        }// j

        // Set the overall quantisation index, used as a predictor for the block indices
        node.SetQIndex( block_list[0][0].QIndex() );

        return static_cast<int>( bit_sum );
    }      

}

void QuantChooser::IntegralErrorCalc( const CodeBlock& code_block , 
                                      const int block_idx , 
                                      const int xratio , 
                                      const int yratio )
{

    ValueType val, quant_val , abs_val;

    CalcValueType error;

    m_count1[block_idx] = ( (code_block.Xl()/xratio)*(code_block.Yl()/yratio) );
    for (int q = m_bottom_idx ; q<=m_top_idx ; q+=4 )
    {
        m_error_total[block_idx][q] = 0.0;
        m_count0[block_idx][q] =0;
        m_countPOS[block_idx][q] = 0;
        m_countNEG[block_idx][q] = 0;
    }

    // Work out the error totals and counts for each quantiser
    for ( int j=code_block.Ystart(); j<code_block.Yend() ; j+=yratio )
    {
        for ( int i=code_block.Xstart(); i<code_block.Xend() ; i+=xratio )
        {
            val = m_pic_data[j][i];
            abs_val = quant_val = abs(val);

            int q = m_bottom_idx;
            for ( ; q<=m_top_idx ; q+=4 )
            {
                // Quantiser is 2^(q/4), so we divide by this
                quant_val >>= (q>>2);    
                if (!quant_val)
                    break;

                m_count0[block_idx][q] += quant_val;
                // Multiply back up so that we can quantise again in the next loop step
                quant_val <<= (q>>2)+2;
                quant_val += dirac_quantiser_lists.InterQuantOffset4( q )+2;
                quant_val >>= 2;
                if (val>0)
                    m_countPOS[block_idx][q]++;
                else
                    m_countNEG[block_idx][q]++;

                error = abs_val-quant_val;

                // Using the fourth power to measure the error
                m_error_total[block_idx][q] +=  pow4( static_cast<double>( error ) );

            }// q
            double derror = pow4 ( static_cast<double>( abs_val ) );
            for (; q <= m_top_idx; q+= 4)
            {
                m_error_total[block_idx][q] += derror;
            }
        }// i
    }// j

}

void QuantChooser::NonIntegralErrorCalc( const CodeBlock& code_block , const int block_idx , const int xratio , const int yratio )
{

    ValueType val, abs_val;

    CalcValueType quant_val;
    CalcValueType error;

    m_count1[block_idx] = ( (code_block.Xl()/xratio)*(code_block.Yl()/yratio) );
    for (int q = m_bottom_idx ; q<=m_top_idx ; q+=m_index_step )
    {
        m_error_total[block_idx][q] = 0.0;
        m_count0[block_idx][q] =0;
        m_countPOS[block_idx][q] = 0;
        m_countNEG[block_idx][q] = 0;
    }

    // Work out the error totals and counts for each quantiser
    for ( int j=code_block.Ystart(); j<code_block.Yend() ; j+=yratio )
    {
        for ( int i=code_block.Xstart(); i<code_block.Xend() ; i+=xratio )
        {

            val = m_pic_data[j][i];
            abs_val = abs( val );

            int q=m_bottom_idx;
            for ( ; q<=m_top_idx ; q+=m_index_step )
            {
                 // Since the quantiser isn't a power of 2 we have to divide each time
                 quant_val = static_cast<CalcValueType>( abs_val );
                 quant_val <<= 2;
                 quant_val /= dirac_quantiser_lists.QuantFactor4( q );

                 if ( !quant_val )
                     break;

                 m_count0[block_idx][q] += quant_val;
                 quant_val *= dirac_quantiser_lists.QuantFactor4( q );
                 quant_val += dirac_quantiser_lists.InterQuantOffset4( q )+2;
                 quant_val >>= 2;

                 if ( val>0 )
                     m_countPOS[block_idx][q]++;
                 else
                     m_countNEG[block_idx][q]++;

                 error = abs_val-quant_val;
                 m_error_total[block_idx][q] += pow4( error );
             }// q
             double derror = pow4( abs_val );
             for ( ; q <= m_top_idx; q += m_index_step)
                 m_error_total[block_idx][q] += derror;
        }// i
    }// j

}


void QuantChooser::LagrangianCalc(const CodeBlock& code_block , const int block_idx )
{
    const double vol( static_cast<double>( m_count1[block_idx] ) );

    // probabilities
    double p0,p1;

    double sign_entropy;

    // Do Lagrangian costs calculation        
    for ( int q=m_bottom_idx ; q<=m_top_idx ; q += m_index_step )
    {

        m_costs[block_idx][q].Error = m_error_total[block_idx][q]/vol;
        m_costs[block_idx][q].Error = std::sqrt( m_costs[block_idx][q].Error )/( code_block.Wt()*code_block.Wt() );

        // Calculate probabilities and entropy
        p0 = double( m_count0[block_idx][q] )/ double( m_count0[block_idx][q]+m_count1[block_idx] );
        p1 = 1.0 - p0;

        if ( p0 != 0.0 && p1 != 0.0)
            m_costs[block_idx][q].ENTROPY = -( p0*std::log(p0)+p1*std::log(p1) ) / std::log(2.0);
        else
            m_costs[block_idx][q].ENTROPY = 0.0;

        // We want the entropy *per symbol*, not per bit ...            
        m_costs[block_idx][q].ENTROPY *= double(m_count0[block_idx][q]+m_count1[block_idx]);
        m_costs[block_idx][q].ENTROPY /= vol;

        // Now add in the sign entropy
        if ( m_countPOS[block_idx][q] + m_countNEG[block_idx][q] != 0 )
        {
            p0 = double( m_countNEG[block_idx][q] )/double( m_countPOS[block_idx][q]+m_countNEG[block_idx][q] );
            p1 = 1.0-p0;
            if ( p0 != 0.0 && p1 != 0.0)
                sign_entropy = -( (p0*std::log(p0)+p1*std::log(p1) ) / std::log(2.0));
            else
                sign_entropy = 0.0;
        }
        else
            sign_entropy = 0.0;    

        // We want the entropy *per symbol*, not per bit ...
        sign_entropy *= double( m_countNEG[block_idx][q] + m_countPOS[block_idx][q] );
        sign_entropy /= vol;    

        m_costs[block_idx][q].ENTROPY += sign_entropy;

        // Sort out correction factors
        m_costs[block_idx][q].ENTROPY *= m_entropy_correctionfactor;
        m_costs[block_idx][q].TOTAL = m_costs[block_idx][q].Error+m_lambda*m_costs[block_idx][q].ENTROPY;

    }// q
}

void QuantChooser::SelectBestQuant( const int block_idx )
{
    // Selects the best quantiser to use for a code block

    m_min_idx = m_bottom_idx;
    for ( int q=m_bottom_idx + m_index_step; q<=m_top_idx ; q +=m_index_step )
    {
        if ( m_costs[block_idx][q].TOTAL < m_costs[block_idx][m_min_idx].TOTAL)
            m_min_idx = q;
    }// q
         
}

void QuantChooser::SelectBestQuant()
{
    // Selects the best quantiser to use for all the code blocks together    

    m_min_idx = m_bottom_idx;

    OneDArray<double> total_costs( m_costs.LengthX() );
    double vol;
/*
    for ( int q=m_bottom_idx; q<=m_top_idx ; q +=m_index_step )
    {
        total_costs[q] = 0.0;
        vol = 0.0;
        for (int block_idx=0 ; block_idx<m_costs.LengthY() ; ++block_idx)
        {
            total_costs[q] += m_costs[block_idx][q].TOTAL * m_count1[block_idx];
            vol += m_count1[block_idx];
        }

        total_costs[q] /= vol;

        if ( total_costs[q] < total_costs[m_min_idx] )
            m_min_idx = q;
    }// q
*/


    double entropy_total, error_total;

    for ( int q=m_bottom_idx; q<=m_top_idx ; q +=m_index_step )
    {
        entropy_total = 0.0;
        error_total = 0.0;
        vol = 0.0;
        for (int block_idx=0 ; block_idx<m_costs.LengthY() ; ++block_idx)
        {
            entropy_total += m_costs[block_idx][q].ENTROPY * m_count1[block_idx];
            error_total += m_costs[block_idx][q].Error * m_count1[block_idx];
            vol += m_count1[block_idx];
        }

        entropy_total /= vol;
        error_total /= vol;

        total_costs[q] = error_total + m_lambda*entropy_total;

        if ( total_costs[q] < total_costs[m_min_idx] )
            m_min_idx = q;

     }
}

ValueType QuantChooser::BlockAbsMax( const CodeBlock& code_block )
{
    ValueType val( 0 );

    for (int j=code_block.Ystart() ; j<code_block.Yend(); ++j)
    {
        for (int i=code_block.Xstart() ; i<code_block.Xend(); ++i)
        {    
            val = std::max( val , m_pic_data[j][i] );    
        }// i
    }// j

    return val;
}
