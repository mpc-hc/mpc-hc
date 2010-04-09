/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: rate_control.cpp,v 1.33 2008/12/16 01:21:02 asuraparaju Exp $ $Name:  $
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
* Contributor(s): Myo Tun (Original Author, myo.tun@brunel.ac.uk)
*                 Jonathan Loo (Jonathan.Loo@brunel.ac.uk)
*                 School of Engineering and Design, Brunel University, UK
*                 Thomas Davies (bugfixes to improve stability and a major
                  refactor to introduce a buffer model)
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


//////////////Rate Control Algorithm//////////////////
/*
The algorithm controls the bitrate by adaptively changing the Quality
Factor, QF of each frame before encoding by the
m_fcoder.Compress( , , , ) function in CompressNextPicture() (seq_compress.cpp).
The first sub-group which is I, L1, L2, L2 frames are encoded
by using the initial QF which is set to 7. The corresponding bitrate R is then
calculated. Adaption for the next subgroup is carried out by determining a model
 parameter K, whose relationship to QF and bit rate is:

QF = 10-5/2log(K/R^(-2))

K is determined from the first subgroup data (QF and rate), and used to
calculate a new QF from the target rates for the next subgroup.

The same procedure applies for the  calculation of the QF for the remaining
sub-groups until it reaches to the end  of a GOP.

For the next GOP, the initial QF of the first sub-group is the average value of
the QF of the first sub-group and last sub-group from the previous GOP. The
calculation of the QF of each and every sub-group is carried out in the
CalcNextQualFactor( , ) function at the rate_control.cpp.

The target bitrate is varied according to a decoder buffer model, consisting of
the average bitrate over a GOP plus an adjustment to steer the buffer towards a
target occupancy. A target bit rate corresponding to
 the different sub-groups still neeeds to be calculated from this overall target.

In order to do this, the modified version of test model version 5, TM5 bit
allocation procedure is employed and target bit rates for each sub-group are
calculated by using the allocated bits to each frame types. The modified version
of TM5 bit allocation procedure is implemented in Allocate ( , ) function at the
rate_control.cpp.
*/


#include <math.h>
#include <libdirac_encoder/rate_control.h>
using namespace dirac;

//Default constructor
FrameComplexity::FrameComplexity():
    m_XI(169784),
    m_XL1(36016),
    m_XL2(4824)
{}

//Default constructor
RateController::RateController(int trate, SourceParams& srcp, EncoderParams& encp):
    m_qf (encp.Qf()),
    m_I_qf (encp.Qf()),
    m_I_qf_long_term(encp.Qf()),
    m_target_rate(trate),
    m_buffer_size(5000*trate),// for the moment, set buffer size to 5 seconds
    m_buffer_bits((m_buffer_size*9)/10),// initial occupancy of 90%
    m_encparams(encp),
    m_fcount(encp.L1Sep() ),
    m_intra_only(false),
    m_L2_complexity_sum(0)
{
    SetFrameDistribution();
    CalcTotalBits(srcp);

    if (m_intra_only)
        m_Iframe_bits = m_total_GOP_bits;
    else
    {
        m_Iframe_bits = m_total_GOP_bits/10;
        m_L1frame_bits = (m_Iframe_bits*3)/m_num_L1frame;
        if (m_encparams.L1Sep()>1)
            m_L2frame_bits = ( m_total_GOP_bits - m_Iframe_bits -
                               m_L1frame_bits*m_num_L1frame )/
                         (m_encparams.GOPLength()-1-m_num_L1frame);
        else
            m_L2frame_bits = 0;

    }
}

void RateController::SetFrameDistribution()
{
    m_num_L1frame = m_encparams.NumL1();
    m_num_Iframe = 1;

    if (m_num_L1frame == 0)
    {
        m_num_Iframe = m_encparams.GOPLength();
        m_intra_only = true;
    }

    m_num_L2frame = m_encparams.GOPLength() - m_num_Iframe - m_num_L1frame;
}

void RateController::CalcTotalBits(const SourceParams& sourceparams)
{
    const Rational& frame_rate = sourceparams.FrameRate();
    double f_rate = double(frame_rate.m_num)/double(frame_rate.m_denom);
    int GOP_length = m_encparams.GOPLength();

    m_GOP_duration = GOP_length/f_rate;
    m_total_GOP_bits = (long int)(m_GOP_duration*1000.0)*m_target_rate; //Unit in bits

    m_GOP_target = m_total_GOP_bits;

    m_picture_bits = m_total_GOP_bits/GOP_length;

    if (m_encparams.Verbose())
    {
        std::cout<<"\nRate Control Encoding with target bit rate = ";
        std::cout<<m_target_rate<<" kbps"<< std::endl;

        std::cout<<"GOP Length = "<<GOP_length<< std::endl;

        std::cout<<"Frame Rate = "<<f_rate<< std::endl;

        std::cout<<"GOP Duration = "<<m_GOP_duration<< std::endl;

        std::cout<<"Total Allocated Num. of bits for each GOP = ";
        std::cout<<m_total_GOP_bits<<" ("<<m_picture_bits<<" per frame)";
        std::cout<<std::endl;
    }
}

void RateController::Report()
{

    std::cout<<std::endl;
    std::cout<<std::endl<<"GOP target is "<<m_GOP_target;
    std::cout<<std::endl<<"Allocated frame bits by type: ";
    std::cout<<"I frames - "<<m_Iframe_bits;
    std::cout<<"; L1/P frames - "<<m_L1frame_bits;
    std::cout<<"; L2/B frames - "<<m_L2frame_bits;
    std::cout<<std::endl;
}


double RateController::TargetSubgroupRate()
{
    long int bits = (m_encparams.L1Sep()-1)*m_L2frame_bits+
               m_L1frame_bits;
    return (double)(bits)/(1000.0*m_GOP_duration);

}

double RateController::ProjectedSubgroupRate()
{
    int bits = (m_encparams.L1Sep()-1)*m_frame_complexity.L2Complexity()+
               m_frame_complexity.L1Complexity();

    return (double)(bits)/(1000.0*m_GOP_duration);
}
void RateController::CalcNextQualFactor(const PictureParams& pparams, int num_bits)
{

    // Decrement the subgroup frame counter. This is zero just after the last
    // L2 frame before the next L1 frame i.e. before the start of an L1L2L2
    // subgroup
    m_fcount--;
    UpdateBuffer( num_bits );

    // filter tap for adjusting the QF
    double target_ratio = 0.9;

    int field_factor = m_encparams.FieldCoding() ? 2 : 1;

    double top_size = (1.0 - target_ratio)-0.5;
    double bottom_size = target_ratio-0.1;
    double actual_ratio = double(m_buffer_bits)/double(m_buffer_size);
    double tap;

    if ((pparams.PictureNum()/field_factor)<=3*m_encparams.L1Sep() )
        tap = 1.0;
    else{
        if (actual_ratio>target_ratio)
            tap = (actual_ratio-target_ratio)/top_size;
        else
            tap = (target_ratio-actual_ratio)/bottom_size;

        tap = std::min( 1.0, std::max(tap, 0.25 ));
    }


    if (!m_intra_only)
    {
        bool emergency_realloc = false;
        int target;

        // First, do normal coding

        if ( pparams.PicSort().IsIntra() == true )
        {
            target = m_Iframe_bits;

            if (num_bits < target/2 )
                emergency_realloc = true;

            // Update the statistics
            m_frame_complexity.SetIComplexity( num_bits );

            // We've just coded an intra frame so we need to set qf for
            // the next group of L2(B) frames
            m_qf = std::max(tap*m_qf+(1.0-tap)*m_encparams.Qf(), m_encparams.Qf()-1.0);
            m_encparams.SetQf( m_qf );

            if (pparams.PictureNum()/field_factor==0)
            {
                // We've just coded the very first frame, which is a special
                // case as the two L2 frames which normally follow are missing
                m_fcount = m_encparams.L1Sep();
            }

        }

        //Update complexities
        if ( (pparams.PictureNum()/field_factor) % m_encparams.L1Sep() !=0 )
        {
            // Scheduled B/L2 picture

            target = m_L2frame_bits;

            if (num_bits < target/2 ){
                emergency_realloc = true;
            }

            m_L2_complexity_sum += num_bits;
        }
        else if ( pparams.PicSort().IsIntra() == false )
        {
            // Scheduled P/L1 picture (if inserted I picture, don't change the complexity)

            target = m_L1frame_bits;

            if (num_bits < target/2 || num_bits > target*3 ){
                emergency_realloc = true;
            }

            m_frame_complexity.SetL1Complexity(num_bits);
        }

        if ( m_fcount==0 || emergency_realloc==true)
        {
            if (emergency_realloc==true && m_encparams.Verbose()==true )
                std::cout<<std::endl<<"Major mis-prediction of frame bit rate: re-allocating";


            /* We recompute allocations for the next subgroup */

            if ( m_encparams.L1Sep()>1 && m_fcount<m_encparams.L1Sep()-1)
            {
                m_frame_complexity.SetL2Complexity(m_L2_complexity_sum/
                                                  (m_encparams.L1Sep()-1-m_fcount));
            }
            Allocate( (pparams.PictureNum()/field_factor) );

            /* We work out what this means for the quality factor and set it*/

            // Get the target number of bits for the coming subgroup
            // calculated from allocations
            double trate = TargetSubgroupRate();

            // Get the projected rate for the coming subgroup, calculated
            // from measured values (complexities)
            double prate = ProjectedSubgroupRate();

            if (m_encparams.Verbose()==true )
            {
                std::cout<<std::endl<<"Target subgroup rate = "<<trate;
                std::cout<<", projected subgroup rate = "<<prate;
            }
            // Determine K value
            double K = std::pow(prate, 2)*std::pow(10.0, ((double)2/5*(10-m_qf)))/16;

            // Determine a new QF
            double new_qf = 10 - (double)5/2*log10(16*K/std::pow(trate, 2));
            if ( ( std::abs(m_qf-new_qf)<0.25 && new_qf > 4.0 ) || new_qf>8.0)
                m_qf = new_qf;
            else
                m_qf = tap*new_qf+(1.0-tap)*m_qf;

            m_qf = ReviewQualityFactor( m_qf , num_bits );

            if ( m_qf<8.0 ){
                if (prate<2*trate)
                    m_qf = std::max(m_qf,m_encparams.Qf()-1.0);
                else
                    m_qf = std::max(m_qf,m_encparams.Qf()-2.0);
            }

            m_encparams.SetQf(m_qf);


            /* Resetting */

            // Reset the frame counter
            if (m_fcount==0)
                m_fcount = m_encparams.L1Sep();

            // Reset the count of L2 bits
            m_L2_complexity_sum = 0;
        }

    }
    else
    {
        // We're doing intra-only coding


        // Target rate
        double trate = double(m_total_GOP_bits)/(1000.0*m_num_Iframe);

        // Projected rate with current QF
        double prate = double(num_bits)/1000.0;


        // Determine K value
        double K = std::pow(prate, 2)*std::pow(10.0, ((double)2/5*(10-m_qf)))/16;

        // Determine a new QF
        double new_qf = 10 - (double)5/2*log10(16*K/std::pow(trate, 2));

        // Adjust the QF to meet the target

        double abs_delta = std::abs( new_qf - m_qf );
        if ( abs_delta>0.01)
        {
            // Rate of convergence to new QF
            double r;

            /* Use an S-shaped curve to compute r
               Where the qf difference is less than 1/2, r decreases to zero
               exponentially, so for small differences in QF we jump straight
               to the target value. For large differences in QF, r converges
               exponentially to 0.75, so we converge to the target value at
               a fixed rate.

               Overall behaviour is to converge steadily for 2 or 3 frames until
               close and then lock to the correct value. This avoids very rapid
               changes in quality.

               Actual parameters may be adjusted later. Some applications may
               require instant lock.
            */
            double lg_diff = std::log( abs_delta/2.0 );
            if ( lg_diff< 0.0 )
                r = 0.5*std::exp(-lg_diff*lg_diff/2.0);
            else
                r = 1.0-0.5*std::exp(-lg_diff*lg_diff/2.0);

            r *= 0.75;

            m_qf = r*m_qf + (1.0-r)*new_qf;
            m_qf = ReviewQualityFactor( m_qf , num_bits );

            m_encparams.SetQf(m_qf);
        }


    }

}


void RateController::UpdateBuffer( const long int num_bits )
{
    m_buffer_bits -= num_bits;
    m_buffer_bits += m_picture_bits;

    if (m_encparams.Verbose())
    {
        std::cout<<std::endl<<"Buffer occupancy = "<<((double)m_buffer_bits*100.0)
                                                     /((double)m_buffer_size)<<"%";
    }

    if (m_buffer_bits<0)
    {
        if (m_encparams.Verbose())
        {
        std::cout<<std::endl<<"WARNING: decoder buffer is out of bits - bit rate is too high";
        }
//        m_buffer_bits = 0;
    }

    if (m_buffer_bits>m_buffer_size)
    {
        if (m_encparams.Verbose())
        {
        std::cout<<std::endl<<"WARNING: decoder buffer has overflowed  - bit rate is too low.  Assuming bit-stuffing.";
        }
        m_buffer_bits = m_buffer_size;
    }


}

void RateController::Allocate (const int fnum)
{
    const int XI = m_frame_complexity.IComplexity();
    const int XL1 = m_frame_complexity.L1Complexity();
    const int XL2 = m_frame_complexity.L2Complexity();

    double buffer_occ = ( (double)m_buffer_bits)/((double)m_buffer_size);

    if ( !m_intra_only)
    {
        double correction;
        if (buffer_occ<0.9 && ( (fnum+1) % 4*m_encparams.L1Sep())==0 )
        {
            // If we're undershooting buffer target, correct slowly
            correction = std::min( 0.25, 0.25*(0.9 - buffer_occ )/0.9 );
            m_GOP_target = ( long int)(double(m_total_GOP_bits)*( 1.0-correction) );
        }
        else if (buffer_occ>0.9 && ((fnum+1) % m_encparams.L1Sep())==0)
        {
            // If we're overshooting buffer target, correct quickly
            correction = std::min( 0.5, 0.5*( buffer_occ - 0.9 )/0.9 );
            m_GOP_target = ( long int)(double(m_total_GOP_bits)*( 1.0+correction) );
        }
    }


    const long int min_bits = m_total_GOP_bits/(100*m_encparams.GOPLength());

    // Allocate intra bits
    m_Iframe_bits = (long int) (m_GOP_target
                  / (m_num_Iframe
                    +(double)(m_num_L1frame*XL1)/XI
                    +(double)(m_num_L2frame*XL2)/XI));

    m_Iframe_bits = std::max( min_bits, m_Iframe_bits );

    // Allocate L1 bits
    m_L1frame_bits = (long int) (m_GOP_target
                   / (m_num_L1frame
                     +(double)(m_num_Iframe*XI)/XL1
                     +(double)(m_num_L2frame*XL2)/XL1));

    m_L1frame_bits = std::max( min_bits, m_L1frame_bits );

    // Allocate L2 bits
    m_L2frame_bits = (long int) (m_GOP_target
                   / (m_num_L2frame
                     +(double)(m_num_Iframe*XI)/XL2
                     +(double)(m_num_L1frame*XL1)/XL2));

    m_L2frame_bits = std::max( min_bits, m_L2frame_bits );
}



float RateController::ReviewQualityFactor( const float qfac, const long int num_bits )
{
    if (num_bits>m_total_GOP_bits/2)
    {
        // The frame is too big, so reset to a smaller quality factor
        return ClipQualityFactor(qfac-2);
    }
    else
    {
        // Keep the quality factor in a sensible range
        return ClipQualityFactor( qfac );
    }
}

float RateController::ClipQualityFactor( const float qfac )
{
    // Keep the quality factor in a sensible range
    if ( !m_intra_only )
        return std::min( std::max(qfac, 0.0f), 16.0f);
    else
        return std::max(qfac, 0.0f);
}

void RateController::CalcNextIntraQualFactor()
{
    m_I_qf = (m_I_qf + m_qf)/2.0;
    m_I_qf = ClipQualityFactor( m_I_qf );
    m_encparams.SetQf(m_I_qf);

    const double ff = 0.95;
    m_I_qf_long_term *= ff;
    m_I_qf_long_term += ( 1.0 - ff )*m_I_qf;
}

void RateController::SetCutPictureQualFactor()
{
    m_qf = std::min( m_qf , m_I_qf_long_term );
    m_encparams.SetQf( m_qf );
}
