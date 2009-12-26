/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: wavelet_utils_mmx.cpp,v 1.13 2008/08/14 00:51:08 asuraparaju Exp $ $Name:  $
*
* Version: MPL 1.1/GPL 2.0/LGPL 2.1
*
* The contents of this file are subject to the Mozilla Public License
* Version 1.1 (the "License");  you may not use this file except in compliance
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
* Contributor(s): Anuradha Suraparaju (Original Author),
*                 Thomas Davies (wavelet_utils.cpp)
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

/*! 
 * MMX version of wavelet transform routines. Note that these routines
 * assume that wavelet coefficients, of type CoeffType, are in fact
 * shorts. This is set in libdirac_common/common.h. Turning MMX on
 * reduces the supported wavelet depth to a maximum of 4 or 5
 * (depending on the filter) by the use of only 16 bits for coefficients.
 */


#ifdef HAVE_MMX
#include <libdirac_common/wavelet_utils.h>
#include <cstdlib>
#include <mmintrin.h>
using namespace dirac;

static TwoDArray<CoeffType> t_temp_data;

#if 0
//Attempt1
inline void Interleave_mmx( const int xp , 
                    const int yp , 
                    const int xl , 
                    const int yl , 
                    CoeffArray& coeff_data)
{
    const int xl2( xl>>1);
    const int yl2( yl>>1);
    const int yend( yp + yl );

    if (coeff_data.LengthX() > t_temp_data.LengthX() ||
        coeff_data.LengthY() > t_temp_data.LengthY())
    {
        t_temp_data.Resize(coeff_data.LengthY(), coeff_data.LengthX());
    }

    // Make a temporary copy of the subband
    for (int j = yp; j<yend ; j++ )
        memcpy( t_temp_data[j-yp] , coeff_data[j]+xp , xl * sizeof( CoeffType ) );

    int stopx = (xl2>>2)<<2;
    // Re-order to interleave
    for (int j = 0, s=yp; j<yl2 ; j++, s+=2)
    {
        CoeffType *tmp1 = &t_temp_data[j][0];
        CoeffType *out = &coeff_data[s][xp];
        for (int i = 0 , t = xp ; i<xp+stopx ; i+=4 , t+=8)
        {
            __m64 m1 = *(__m64 *)tmp1;
            __m64 m2 = *(__m64 *)(tmp1+xl2);
            *(__m64 *)out = _mm_unpacklo_pi16 (m1, m2);    
            out+=4;
            *(__m64 *)out = _mm_unpackhi_pi16 (m1, m2);    
            out+=4;
            tmp1 += 4;
        }
        for (int i = xp+stopx , r=2*(xp+stopx) ; i<xl2 ; i++ , r += 2)
        {
            *out = *tmp1;
            ++out;
            *out = *(tmp1+xl2);
            ++out;
            ++tmp1;
        }
    }// j 
    
    for (int j = yl2, s=yp+1 ; j<yl ; j++ , s += 2)
    {
        CoeffType *tmp1 = &t_temp_data[j][0];
        //CoeffType *tmp2 = &t_temp_data[j][xl2];
        CoeffType *out = &coeff_data[s][xp];
        for (int i = 0 , t=xp; i<stopx ; i+=4 , t += 8)
        {
            __m64 m1 = *(__m64 *)tmp1;
            __m64 m2 = *(__m64 *)(tmp1+xl2);
            *(__m64 *)out = _mm_unpacklo_pi16 (m1, m2);
            out+=4;
            *(__m64 *)out = _mm_unpackhi_pi16 (m1, m2);    
            out+=4;
            tmp1 += 4;
        }
        for (int i = stopx , r=2*(xp+stopx) ; i<xl2 ; i++ , r += 2)
        {
            *out = *tmp1;
            ++out;
            *out = *(tmp1+xl2);
            ++out;
            ++tmp1;
        }
    }// j 

    _mm_empty();
}
#endif

void VHFilter::ShiftRowLeft(CoeffType *row, int length, int shift)
{
    int xstop = length/4*4;
    CoeffType *shift_row = row;
    for (int i = 0; i < xstop; i+=4, shift_row+=4)
        *(__m64 *)shift_row = _mm_slli_pi16 (*(__m64 *)shift_row, shift);

    // mopup
    for (int i = xstop; i < length; ++i)
        row[i] <<= shift;

    _mm_empty();
}

void VHFilter::ShiftRowRight(CoeffType *row, int length, int shift)
{
    CoeffType *shift_row = row;
    int round_val = 1<<(shift-1);
    __m64 mmx_round = _mm_set_pi16( round_val, round_val, round_val, round_val);

    int xstop = length/4*4;
    for (int i = 0; i < xstop; i+=4, shift_row+=4)
    {
        __m64 tmp = _mm_add_pi16 (*(__m64 *)shift_row, mmx_round);
        *(__m64 *)shift_row = _mm_srai_pi16(tmp, shift);
    }
    // mopup
    for (int i = xstop; i < length; ++i)
        row[i] = ((row[i]+round_val)>>shift);
    _mm_empty();
}

inline void Interleave_mmx( const int xp , 
                    const int yp , 
                    const int xl , 
                    const int yl , 
                    CoeffArray& coeff_data)
{
    const int xl2( xl>>1);
    const int yl2( yl>>1);
    const int yend( yp + yl );

    if (coeff_data.LengthX() > t_temp_data.LengthX() ||
        coeff_data.LengthY() > t_temp_data.LengthY())
    {
        t_temp_data.Resize(coeff_data.LengthY(), coeff_data.LengthX());
    }

    // Make a temporary copy of the subband. We are doing a vertical
    // interleave while copying
    for (int j = yp, s=0; j<yp+yl2 ; j++, s+=2 )
        memcpy( t_temp_data[s] , coeff_data[j]+xp , xl * sizeof( CoeffType ) );
    for (int j = yp+yl2, s=1; j<yend ; j++, s+=2 )
        memcpy( t_temp_data[s] , coeff_data[j]+xp , xl * sizeof( CoeffType ) );

    int stopx = (xl2>>2)<<2;
    // Re-order to horizontally interleave
    for (int j = 0, s=yp; j<yl ; j++, ++s)
    {
        CoeffType *tmp1 = &t_temp_data[j][0];
        CoeffType *out = &coeff_data[s][xp];
        for (int i = 0 , t = xp ; i<xp+stopx ; i+=4 , t+=8)
        {
            __m64 m1 = *(__m64 *)tmp1;
            __m64 m2 = *(__m64 *)(tmp1+xl2);
            *(__m64 *)out = _mm_unpacklo_pi16 (m1, m2);    
            out+=4;
            *(__m64 *)out = _mm_unpackhi_pi16 (m1, m2);    
            out+=4;
            tmp1 += 4;
        }
        for (int i = xp+stopx , r=2*(xp+stopx) ; i<xl2 ; i++ , r += 2)
        {
            *out = *tmp1;
            ++out;
            *out = *(tmp1+xl2);
            ++out;
            ++tmp1;
        }
    }// j 
    
    _mm_empty();
}

void VHFilterDD9_7::Synth(const int xp , 
                                                const int yp , 
                                                const int xl , 
                                                const int yl , 
                                                CoeffArray& coeff_data)
{
    int i, j;
    const int xend( xp+xl );
    const int yend( yp+yl );
    const int ymid = yp+yl/2;

    PredictStepShift<2> predict;
    __m64 pred_round = _mm_set_pi16 (1<<(2-1), 1<<(2-1), 1<<(2-1), 1<<(2-1));
    
    int xstop = xp + ((xl>>2)<<2);

    // First lifting stage
    // Top edge
    CoeffType *in1 =  coeff_data[ymid];
    CoeffType *in2 =  coeff_data[ymid];
    CoeffType *out = coeff_data[yp];
       for ( i = xp ; i < xstop ; i+=4 )
    {
        // tmp = val + val2
        __m64 tmp = _mm_add_pi16 (*(__m64 *)in1, *(__m64 *)in2);
        // unbiased rounding tmp = (tmp + 1<<(shift-1))>>shift
        tmp = _mm_add_pi16(tmp, pred_round);
        tmp = _mm_srai_pi16(tmp, 2);
        // in_val -= tmp;
        *(__m64 *)out = _mm_sub_pi16 (*(__m64*)out, tmp);
        out += 4;
        in1 += 4;
        in2 += 4;
    }

    // Middle bit
       for ( j=1 ; j < yl/2 ; ++j )
    {
        in1 =  coeff_data[ymid+j-1];
        in2 =  coeff_data[ymid+j];
        out = coeff_data[yp+j];
           for ( i = xp ; i < xstop ; i+=4 )
        {
            // tmp = val + val2
            __m64 tmp = _mm_add_pi16 (*(__m64 *)in1, *(__m64 *)in2);
            // unbiased rounding tmp = (tmp + 1<<(shift-1))>>shift
            tmp = _mm_add_pi16(tmp, pred_round);
            tmp = _mm_srai_pi16(tmp, 2);
            // in_val -= tmp;
            *(__m64 *)out = _mm_sub_pi16 (*(__m64*)out, tmp);
            out += 4;
            in1 += 4;
            in2 += 4;
        }
    }

    // Mopup
    if (xstop != xend)
    {
        for ( i = xstop ; i < xend ; i++)
        {
            predict.Filter( coeff_data[yp][i] , coeff_data[ymid][i] , coeff_data[ymid][i] );
        }// i

        for ( j=1 ; j < yl/2 ; ++j )
        {
            for ( i = xstop ; i < xend ; i++)
            {
                predict.Filter( coeff_data[yp+j][i] , coeff_data[ymid+j-1][i] , coeff_data[ymid+j][i] );
            }// i
        }// j
    }
 
    // Second lifting stage
    UpdateStepFourTap< 4 , 9 , -1> update;
    // rounding factor for update step
    __m64 update_round = _mm_set_pi32 (1<<(4-1), 1<<(4-1));
    // top edge
    in1 = coeff_data[yp];
    in2 = coeff_data[yp+1];
    CoeffType *in3 = coeff_data[yp];
    CoeffType *in4 = coeff_data[yp+2];
    out = coeff_data[ymid];
    __m64 tap1 = _mm_set_pi16 (9, 9, 9, 9);
    __m64 tap2 = _mm_set_pi16 (-1, -1, -1, -1);
  
    for ( i = xp ; i < xstop ; i+=4 )
    {
        __m64 val1, val2, val3, val4, tmp1, tmp2;

        val1 = _mm_add_pi16(*(__m64*)in1, *(__m64*)in2);
        tmp1 = _mm_mullo_pi16 (val1, tap1);
        tmp2 = _mm_mulhi_pi16 (val1, tap1);
        val3 = _mm_unpackhi_pi16 (tmp1, tmp2);
        val1 = _mm_unpacklo_pi16 (tmp1, tmp2);

        val2 = _mm_add_pi16(*(__m64*)in3, *(__m64*)in4);
        tmp1 = _mm_mullo_pi16 (val2, tap2);
        tmp2 = _mm_mulhi_pi16 (val2, tap2);
        val4 = _mm_unpackhi_pi16 (tmp1, tmp2);
        val2 = _mm_unpacklo_pi16 (tmp1, tmp2);

        val1 = _mm_add_pi32 (val1, val2);
        val3 = _mm_add_pi32 (val3, val4);
        val1 = _mm_add_pi32 (val1, update_round);
        val3 = _mm_add_pi32 (val3, update_round);
        val1 = _mm_srai_pi32(val1, 4);
        val3 = _mm_srai_pi32(val3, 4);
        val1 = _mm_packs_pi32 (val1, val3);
        
        *(__m64*)out = _mm_add_pi16 (*(__m64*)out,val1);
        in1 +=4;
        in2 +=4;
        in3 +=4;
        in4 +=4;
        out +=4;
    }

       // middle bit
       for ( j=1 ; j < yl/2 - 2 ; ++j)
       {
        in1 = coeff_data[yp+j];
        in2 = coeff_data[yp+j+1];
        in3 = coeff_data[yp+j-1];
        in4 = coeff_data[yp+j+2];
        out = coeff_data[ymid+j];
        for ( i = xp ; i < xstop ; i+=4)
        {
            __m64 val1, val2, val3, val4, tmp1, tmp2;

            val1 = _mm_add_pi16(*(__m64*)in1, *(__m64*)in2);
            tmp1 = _mm_mullo_pi16 (val1, tap1);
            tmp2 = _mm_mulhi_pi16 (val1, tap1);
            val3 = _mm_unpackhi_pi16 (tmp1, tmp2);
            val1 = _mm_unpacklo_pi16 (tmp1, tmp2);

            val2 = _mm_add_pi16(*(__m64*)in3, *(__m64*)in4);
            tmp1 = _mm_mullo_pi16 (val2, tap2);
            tmp2 = _mm_mulhi_pi16 (val2, tap2);
            val4 = _mm_unpackhi_pi16 (tmp1, tmp2);
            val2 = _mm_unpacklo_pi16 (tmp1, tmp2);

            val1 = _mm_add_pi32 (val1, val2);
            val3 = _mm_add_pi32 (val3, val4);
            val1 = _mm_add_pi32 (val1, update_round);
            val3 = _mm_add_pi32 (val3, update_round);
            val1 = _mm_srai_pi32(val1, 4);
            val3 = _mm_srai_pi32(val3, 4);
            val1 = _mm_packs_pi32 (val1, val3);
        
            *(__m64*)out = _mm_add_pi16 (*(__m64*)out,val1);
            in1 +=4;
            in2 +=4;
            in3 +=4;
            in4 +=4;
            out +=4;
           }// i
       }// k
   
       // bottom edge
    in1 = coeff_data[ymid-2];
    in2 = coeff_data[ymid-1];
    in3 = coeff_data[ymid-3];
    in4 = coeff_data[ymid-1];
    out = coeff_data[yend-2];
    for ( i = xp ; i < xstop ; i+=4)
    {
        __m64 val1, val2, val3, val4, tmp1, tmp2;

        val1 = _mm_add_pi16(*(__m64*)in1, *(__m64*)in2);
        tmp1 = _mm_mullo_pi16 (val1, tap1);
        tmp2 = _mm_mulhi_pi16 (val1, tap1);
        val3 = _mm_unpackhi_pi16 (tmp1, tmp2);
        val1 = _mm_unpacklo_pi16 (tmp1, tmp2);

        val2 = _mm_add_pi16(*(__m64*)in3, *(__m64*)in4);
        tmp1 = _mm_mullo_pi16 (val2, tap2);
        tmp2 = _mm_mulhi_pi16 (val2, tap2);
        val4 = _mm_unpackhi_pi16 (tmp1, tmp2);
        val2 = _mm_unpacklo_pi16 (tmp1, tmp2);

        val1 = _mm_add_pi32 (val1, val2);
        val3 = _mm_add_pi32 (val3, val4);
        val1 = _mm_add_pi32 (val1, update_round);
        val3 = _mm_add_pi32 (val3, update_round);
        val1 = _mm_srai_pi32(val1, 4);
        val3 = _mm_srai_pi32(val3, 4);
        val1 = _mm_packs_pi32 (val1, val3);
       
        *(__m64*)out = _mm_add_pi16 (*(__m64*)out,val1);
        in1 +=4;
        in2 +=4;
        in3 +=4;
        in4 +=4;
        out +=4;
    }
        
    in1 = coeff_data[ymid-1];
    in2 = coeff_data[ymid-1];
    in3 = coeff_data[ymid-2];
    in4 = coeff_data[ymid-1];
    out = coeff_data[yend-1];
    for ( i = xp ; i < xstop ; i+=4)
    {
        __m64 val1, val2, val3, val4, tmp1, tmp2;

        val1 = _mm_add_pi16(*(__m64*)in1, *(__m64*)in2);
        tmp1 = _mm_mullo_pi16 (val1, tap1);
        tmp2 = _mm_mulhi_pi16 (val1, tap1);
        val3 = _mm_unpackhi_pi16 (tmp1, tmp2);
        val1 = _mm_unpacklo_pi16 (tmp1, tmp2);

        val2 = _mm_add_pi16(*(__m64*)in3, *(__m64*)in4);
        tmp1 = _mm_mullo_pi16 (val2, tap2);
        tmp2 = _mm_mulhi_pi16 (val2, tap2);
        val4 = _mm_unpackhi_pi16 (tmp1, tmp2);
        val2 = _mm_unpacklo_pi16 (tmp1, tmp2);

        val1 = _mm_add_pi32 (val1, val2);
        val3 = _mm_add_pi32 (val3, val4);
        val1 = _mm_add_pi32 (val1, update_round);
        val3 = _mm_add_pi32 (val3, update_round);
        val1 = _mm_srai_pi32(val1, 4);
        val3 = _mm_srai_pi32(val3, 4);
        val1 = _mm_packs_pi32 (val1, val3);
        
        *(__m64*)out = _mm_add_pi16 (*(__m64*)out,val1);
        in1 +=4;
        in2 +=4;
        in3 +=4;
        in4 +=4;
        out +=4;
    }
        
    if (xstop != xend)
    {
        for ( i = xstop ; i < xend ; i++)
        {
               update.Filter( coeff_data[ymid][i] , coeff_data[yp][i], coeff_data[yp+1][i], coeff_data[yp][i],coeff_data[yp+2][i]);
        }// i

        // middle bit
           for ( j=1 ; j < yl/2 - 2 ; ++j)
        {
            for ( i = xstop ; i < xend ; i++)
            {
                update.Filter( coeff_data[ymid+j][i] , coeff_data[yp+j][i], coeff_data[yp+j+1][i], coeff_data[yp+j-1][i],coeff_data[yp+j+2][i]);
            }// i
        }// k
    
        for ( i = xstop ; i < xend ; i++)
        {
            update.Filter( coeff_data[yend - 2][i] , coeff_data[ymid-2][i], coeff_data[ymid-1][i], coeff_data[ymid-3][i],coeff_data[ymid-1][i]);
            update.Filter( coeff_data[yend - 1][i] , coeff_data[ymid-1][i], coeff_data[ymid-1][i], coeff_data[ymid-2][i],coeff_data[ymid-1][i]);
        }// i
    }


    // Horizontal sythesis
    
    const int xmid = xl/2;
    xstop = xmid %4 ? ((xmid>>2)<<2) + 1 : xmid -3;

    for (j = yp;  j < yend; ++j)
    {
        CoeffType *line_data = &coeff_data[j][xp];                 

        // First lifting stage acts on even samples i.e. the low pass ones
         predict.Filter( line_data[0] , line_data[xmid] , line_data[xmid] );
        for (i=1 ; i < xmid ; ++i)
        {
            predict.Filter( line_data[i] , line_data[xmid+i-1] , line_data[xmid+i] );
        }

        // Second lifting stage
           update.Filter( line_data[xmid] , line_data[0] , line_data[1] , line_data[0] , line_data[2] );

        for (i=1 ; i < xmid - 2; ++i)
        {
            update.Filter( line_data[xmid+i] , line_data[i] , line_data[i+1] , line_data[i-1] , line_data[i+2] );
        }// i 
        update.Filter( line_data[xl-2] , line_data[xmid-2] , line_data[xmid-1] , line_data[xmid-3] , line_data[xmid-1] );
        update.Filter( line_data[xl-1] , line_data[xmid-1] , line_data[xmid-1] , line_data[xmid-2] , line_data[xmid-1] );
        
        // Shift right by one bit to counter the shift in the analysis stage
        ShiftRowRight(line_data, xl, 1);

    }// j
    _mm_empty();
    Interleave_mmx( xp , yp , xl ,yl , coeff_data );
}

void VHFilterDD13_7::Synth(const int xp ,
                                           const int yp , 
                                           const int xl ,
                                           const int yl , 
                                           CoeffArray& coeff_data)
{
    int i,j,k;

    const int xend( xp+xl );
    const int yend( yp+yl );

    PredictStepFourTap< 5 , 9 , -1 > predict;
    __m64 pred_round = _mm_set_pi32 (1<<(5-1), 1<<(5-1));
    UpdateStepFourTap< 4 , 9 , -1> update;
    __m64 update_round = _mm_set_pi32 (1<<(4-1), 1<<(4-1));

    // Next, do the vertical synthesis
    int ymid = yp + yl/2;

    int xstop = xp + ((xl>>2)<<2);
    // First lifting stage - odd samples
    // bottom edge
    CoeffType *out = coeff_data[ymid-1];
    CoeffType *in1 = coeff_data[yend-2];
    CoeffType *in2 = coeff_data[yend-1];
    CoeffType *in3 = coeff_data[yend-3];
    CoeffType *in4 = coeff_data[yend-1];

    __m64 tap1 = _mm_set_pi16 (9, 9, 9, 9);
    __m64 tap2 = _mm_set_pi16 (-1, -1, -1, -1);
    for ( i = xp ; i<xstop; i+=4)
    {
        __m64 val1, val2, val3, val4, tmp1, tmp2;

        val1 = _mm_add_pi16(*(__m64*)in1, *(__m64*)in2);
        tmp1 = _mm_mullo_pi16 (val1, tap1);
        tmp2 = _mm_mulhi_pi16 (val1, tap1);
        val3 = _mm_unpackhi_pi16 (tmp1, tmp2);
        val1 = _mm_unpacklo_pi16 (tmp1, tmp2);

        val2 = _mm_add_pi16(*(__m64*)in3, *(__m64*)in4);
        tmp1 = _mm_mullo_pi16 (val2, tap2);
        tmp2 = _mm_mulhi_pi16 (val2, tap2);
        val4 = _mm_unpackhi_pi16 (tmp1, tmp2);
        val2 = _mm_unpacklo_pi16 (tmp1, tmp2);

        val1 = _mm_add_pi32 (val1, val2);
        val3 = _mm_add_pi32 (val3, val4);
        val1 = _mm_add_pi32 (val1, pred_round);
        val3 = _mm_add_pi32 (val3, pred_round);
        val1 = _mm_srai_pi32(val1, 5);
        val3 = _mm_srai_pi32(val3, 5);
        val1 = _mm_packs_pi32 (val1, val3);

        *(__m64*)out = _mm_sub_pi16 (*(__m64*)out,val1);
        in1 +=4;
        in2 +=4;
        in3 +=4;
        in4 +=4;
        out +=4;
    }// i

    // middle bit
    for ( j = 2 ; j < yl/2 -1 ; ++j)
    {
        out = coeff_data[yp+j];
        in1 = coeff_data[ymid+j-1];
        in2 = coeff_data[ymid+j];
        in3 = coeff_data[ymid+j-2];
        in4 = coeff_data[ymid+j+1];
        for ( i = xp ; i<xstop ; i+=4)
        {
            __m64 val1, val2, val3, val4, tmp1, tmp2;

            val1 = _mm_add_pi16(*(__m64*)in1, *(__m64*)in2);
            tmp1 = _mm_mullo_pi16 (val1, tap1);
            tmp2 = _mm_mulhi_pi16 (val1, tap1);
            val3 = _mm_unpackhi_pi16 (tmp1, tmp2);
            val1 = _mm_unpacklo_pi16 (tmp1, tmp2);

            val2 = _mm_add_pi16(*(__m64*)in3, *(__m64*)in4);
            tmp1 = _mm_mullo_pi16 (val2, tap2);
            tmp2 = _mm_mulhi_pi16 (val2, tap2);
            val4 = _mm_unpackhi_pi16 (tmp1, tmp2);
            val2 = _mm_unpacklo_pi16 (tmp1, tmp2);

            val1 = _mm_add_pi32 (val1, val2);
            val3 = _mm_add_pi32 (val3, val4);
            val1 = _mm_add_pi32 (val1, pred_round);
            val3 = _mm_add_pi32 (val3, pred_round);
            val1 = _mm_srai_pi32(val1, 5);
            val3 = _mm_srai_pi32(val3, 5);
            val1 = _mm_packs_pi32 (val1, val3);

            *(__m64*)out = _mm_sub_pi16 (*(__m64*)out,val1);
            in1 +=4;
            in2 +=4;
            in3 +=4;
            in4 +=4;
            out +=4;
        }// i
    }// j

    // top edge - j=xp
    out = coeff_data[yp+1];
    in1 = coeff_data[ymid];
    in2 = coeff_data[ymid+1];
    in3 = coeff_data[ymid+2];
    in4 = coeff_data[ymid];
    for ( i = xp ; i<xstop ; i+=4)
    {
        __m64 val1, val2, val3, val4, tmp1, tmp2;

        val1 = _mm_add_pi16(*(__m64*)in1, *(__m64*)in2);
        tmp1 = _mm_mullo_pi16 (val1, tap1);
        tmp2 = _mm_mulhi_pi16 (val1, tap1);
        val3 = _mm_unpackhi_pi16 (tmp1, tmp2);
        val1 = _mm_unpacklo_pi16 (tmp1, tmp2);

        val2 = _mm_add_pi16(*(__m64*)in3, *(__m64*)in4);
        tmp1 = _mm_mullo_pi16 (val2, tap2);
        tmp2 = _mm_mulhi_pi16 (val2, tap2);
        val4 = _mm_unpackhi_pi16 (tmp1, tmp2);
        val2 = _mm_unpacklo_pi16 (tmp1, tmp2);

        val1 = _mm_add_pi32 (val1, val2);
        val3 = _mm_add_pi32 (val3, val4);
        val1 = _mm_add_pi32 (val1, pred_round);
        val3 = _mm_add_pi32 (val3, pred_round);
        val1 = _mm_srai_pi32(val1, 5);
        val3 = _mm_srai_pi32(val3, 5);
        val1 = _mm_packs_pi32 (val1, val3);

        *(__m64*)out = _mm_sub_pi16 (*(__m64*)out,val1);
        in1 +=4;
        in2 +=4;
        in3 +=4;
        in4 +=4;
        out +=4;
    }

    out = coeff_data[yp];
    in1 = coeff_data[ymid];
    in2 = coeff_data[ymid];
    in3 = coeff_data[ymid+1];
    in4 = coeff_data[ymid];
    for ( i = xp ; i<xstop ; i+=4)
    {
        __m64 val1, val2, val3, val4, tmp1, tmp2;

        val1 = _mm_add_pi16(*(__m64*)in1, *(__m64*)in2);
        tmp1 = _mm_mullo_pi16 (val1, tap1);
        tmp2 = _mm_mulhi_pi16 (val1, tap1);
        val3 = _mm_unpackhi_pi16 (tmp1, tmp2);
        val1 = _mm_unpacklo_pi16 (tmp1, tmp2);

        val2 = _mm_add_pi16(*(__m64*)in3, *(__m64*)in4);
        tmp1 = _mm_mullo_pi16 (val2, tap2);
        tmp2 = _mm_mulhi_pi16 (val2, tap2);
        val4 = _mm_unpackhi_pi16 (tmp1, tmp2);
        val2 = _mm_unpacklo_pi16 (tmp1, tmp2);

        val1 = _mm_add_pi32 (val1, val2);
        val3 = _mm_add_pi32 (val3, val4);
        val1 = _mm_add_pi32 (val1, pred_round);
        val3 = _mm_add_pi32 (val3, pred_round);
        val1 = _mm_srai_pi32(val1, 5);
        val3 = _mm_srai_pi32(val3, 5);
        val1 = _mm_packs_pi32 (val1, val3);

        *(__m64*)out = _mm_sub_pi16 (*(__m64*)out,val1);
        in1 +=4;
        in2 +=4;
        in3 +=4;
        in4 +=4;
        out +=4;
    }

    // Mopup 
    if ( xstop != xend)
    {
        // Mopup bottom edge
        for ( i = xstop ; i<xend ; ++ i)
        {
            predict.Filter( coeff_data[ymid-1][i] , coeff_data[yend-2][i] , coeff_data[yend-1][i] , coeff_data[yend-3][i] , coeff_data[yend-1][i] );
        }// i

        // Mopup middle bit
        for ( k = 2 ; k < yl/2 - 1 ; ++k)
        {
            for ( i = xstop ; i<xend ; ++ i)
            {
                   predict.Filter( coeff_data[yp+k][i] , coeff_data[ymid+k-1][i] , coeff_data[ymid+k][i] , coeff_data[ymid+k-2][i] , coeff_data[ymid+k+1][i] );
            }// i
        }// k

        //Mopup top edge
        for ( i = xstop ; i<xend ; ++ i)
        {
            predict.Filter( coeff_data[yp+1][i] , coeff_data[ymid][i] , coeff_data[ymid+1][i] , coeff_data[ymid+2][i] , coeff_data[ymid][i] );
            predict.Filter( coeff_data[yp][i] , coeff_data[ymid][i] , coeff_data[ymid][i] , coeff_data[ymid+1][i] , coeff_data[ymid][i] );

        }// i

    }

    // Second lifting stage
    // top edge - j=xp
    out = coeff_data[ymid];
    in1 = coeff_data[yp];
    in2 = coeff_data[yp+1];
    in3 = coeff_data[yp];
    in4 = coeff_data[yp+2];
    for ( i = xp ; i<xstop ; i+=4)
    {
        __m64 val1, val2, val3, val4, tmp1, tmp2;

        val1 = _mm_add_pi16(*(__m64*)in1, *(__m64*)in2);
        tmp1 = _mm_mullo_pi16 (val1, tap1);
        tmp2 = _mm_mulhi_pi16 (val1, tap1);
        val3 = _mm_unpackhi_pi16 (tmp1, tmp2);
        val1 = _mm_unpacklo_pi16 (tmp1, tmp2);

        val2 = _mm_add_pi16(*(__m64*)in3, *(__m64*)in4);
        tmp1 = _mm_mullo_pi16 (val2, tap2);
        tmp2 = _mm_mulhi_pi16 (val2, tap2);
        val4 = _mm_unpackhi_pi16 (tmp1, tmp2);
        val2 = _mm_unpacklo_pi16 (tmp1, tmp2);

        val1 = _mm_add_pi32 (val1, val2);
        val3 = _mm_add_pi32 (val3, val4);
        val1 = _mm_add_pi32 (val1, update_round);
        val3 = _mm_add_pi32 (val3, update_round);
        val1 = _mm_srai_pi32(val1, 4);
        val3 = _mm_srai_pi32(val3, 4);
        val1 = _mm_packs_pi32 (val1, val3);

        *(__m64*)out = _mm_add_pi16 (*(__m64*)out,val1);
        in1 +=4;
        in2 +=4;
        in3 +=4;
        in4 +=4;
        out +=4;
    }// i


    // middle bit
    for ( k = 1 ; k < yl/2  - 2  ; ++k)
    {
        out = coeff_data[ymid+k];
        in1 = coeff_data[k];
        in2 = coeff_data[k+1];
        in3 = coeff_data[k-1];
        in4 = coeff_data[k+2];
        for ( i = xp ; i<xstop ; i+=4)
        {
            __m64 val1, val2, val3, val4, tmp1, tmp2;

            val1 = _mm_add_pi16(*(__m64*)in1, *(__m64*)in2);
            tmp1 = _mm_mullo_pi16 (val1, tap1);
            tmp2 = _mm_mulhi_pi16 (val1, tap1);
            val3 = _mm_unpackhi_pi16 (tmp1, tmp2);
            val1 = _mm_unpacklo_pi16 (tmp1, tmp2);

            val2 = _mm_add_pi16(*(__m64*)in3, *(__m64*)in4);
            tmp1 = _mm_mullo_pi16 (val2, tap2);
            tmp2 = _mm_mulhi_pi16 (val2, tap2);
            val4 = _mm_unpackhi_pi16 (tmp1, tmp2);
            val2 = _mm_unpacklo_pi16 (tmp1, tmp2);

            val1 = _mm_add_pi32 (val1, val2);
            val3 = _mm_add_pi32 (val3, val4);
            val1 = _mm_add_pi32 (val1, update_round);
            val3 = _mm_add_pi32 (val3, update_round);
            val1 = _mm_srai_pi32(val1, 4);
            val3 = _mm_srai_pi32(val3, 4);
            val1 = _mm_packs_pi32 (val1, val3);

            *(__m64*)out = _mm_add_pi16 (*(__m64*)out,val1);
            in1 +=4;
            in2 +=4;
            in3 +=4;
            in4 +=4;
            out +=4;
        }// i
    }// k

    // bottom edge
    out = coeff_data[yend-2];
    in1 = coeff_data[ymid-2];
    in2 = coeff_data[ymid-1];
    in3 = coeff_data[ymid-3];
    in4 = coeff_data[ymid-1];
    for ( i = xp ; i<xstop ; i+=4)
    {
        __m64 val1, val2, val3, val4, tmp1, tmp2;

        val1 = _mm_add_pi16(*(__m64*)in1, *(__m64*)in2);
        tmp1 = _mm_mullo_pi16 (val1, tap1);
        tmp2 = _mm_mulhi_pi16 (val1, tap1);
        val3 = _mm_unpackhi_pi16 (tmp1, tmp2);
        val1 = _mm_unpacklo_pi16 (tmp1, tmp2);

        val2 = _mm_add_pi16(*(__m64*)in3, *(__m64*)in4);
        tmp1 = _mm_mullo_pi16 (val2, tap2);
        tmp2 = _mm_mulhi_pi16 (val2, tap2);
        val4 = _mm_unpackhi_pi16 (tmp1, tmp2);
        val2 = _mm_unpacklo_pi16 (tmp1, tmp2);

        val1 = _mm_add_pi32 (val1, val2);
        val3 = _mm_add_pi32 (val3, val4);
        val1 = _mm_add_pi32 (val1, update_round);
        val3 = _mm_add_pi32 (val3, update_round);
        val1 = _mm_srai_pi32(val1, 4);
        val3 = _mm_srai_pi32(val3, 4);
        val1 = _mm_packs_pi32 (val1, val3);

        *(__m64*)out = _mm_add_pi16 (*(__m64*)out,val1);
        in1 +=4;
        in2 +=4;
        in3 +=4;
        in4 +=4;
        out +=4;
    }// i

    out = coeff_data[yend-1];
    in1 = coeff_data[ymid-1];
    in2 = coeff_data[ymid-1];
    in3 = coeff_data[ymid-2];
    in4 = coeff_data[ymid-1];
    for ( i = xp ; i<xstop ; i+=4)
    {
        __m64 val1, val2, val3, val4, tmp1, tmp2;

        val1 = _mm_add_pi16(*(__m64*)in1, *(__m64*)in2);
        tmp1 = _mm_mullo_pi16 (val1, tap1);
        tmp2 = _mm_mulhi_pi16 (val1, tap1);
        val3 = _mm_unpackhi_pi16 (tmp1, tmp2);
        val1 = _mm_unpacklo_pi16 (tmp1, tmp2);

        val2 = _mm_add_pi16(*(__m64*)in3, *(__m64*)in4);
        tmp1 = _mm_mullo_pi16 (val2, tap2);
        tmp2 = _mm_mulhi_pi16 (val2, tap2);
        val4 = _mm_unpackhi_pi16 (tmp1, tmp2);
        val2 = _mm_unpacklo_pi16 (tmp1, tmp2);

        val1 = _mm_add_pi32 (val1, val2);
        val3 = _mm_add_pi32 (val3, val4);
        val1 = _mm_add_pi32 (val1, update_round);
        val3 = _mm_add_pi32 (val3, update_round);
        val1 = _mm_srai_pi32(val1, 4);
        val3 = _mm_srai_pi32(val3, 4);
        val1 = _mm_packs_pi32 (val1, val3);

        *(__m64*)out = _mm_add_pi16 (*(__m64*)out,val1);
        in1 +=4;
        in2 +=4;
        in3 +=4;
        in4 +=4;
        out +=4;
    }// i
    

    // Mopup 
    if ( xstop != xend)
    {
        // bottom edge
        for ( i = xstop ; i<xend ; ++ i)
        {
            update.Filter( coeff_data[yend-1][i] , coeff_data[ymid-1][i] , coeff_data[ymid-1][i] , coeff_data[ymid-2][i] , coeff_data[ymid-1][i] );
            update.Filter( coeff_data[yend-2][i] , coeff_data[ymid-2][i] , coeff_data[ymid-1][i] , coeff_data[ymid-3][i] , coeff_data[ymid-1][i] );

        }// i

        // middle bit
        for ( k = 1 ; k < yl/2  - 2  ; ++k)
        {
            for ( i = xstop ; i<xend ; ++ i)
            {
                update.Filter( coeff_data[ymid+k][i] , coeff_data[k][i] , coeff_data[k+1][i] , coeff_data[k-1][i] , coeff_data[k+2][i] );
            }// i
        }// j

        // top edge - j=xp
        for ( i = xstop ; i<xend ; ++ i)
        {
            update.Filter( coeff_data[ymid][i] , coeff_data[yp][i] , coeff_data[yp+1][i] , coeff_data[yp][i] , coeff_data[yp+2][i] ); 
        }// i
    }

    // Next do the horizontal synthesis

    CoeffType* line_data;
    int xmid = xl/2;

    for (j = yp;  j < yend ; ++j)
    {
        line_data = &coeff_data[j][xp];                 

        // First lifting stage

        predict.Filter( line_data[0] , line_data[xmid] , line_data[xmid] , line_data[xmid+1] , line_data[xmid] );
        predict.Filter( line_data[1] , line_data[xmid] , line_data[xmid+1] , line_data[xmid+2] , line_data[xmid] );

        for (k=2 ; k < xmid-1 ; ++k)
        {
            predict.Filter( line_data[k] , line_data[xmid+k-1] , line_data[xmid+k] , line_data[xmid+k-2] , line_data[xmid+k+1] );

        }// i 
        predict.Filter( line_data[xmid-1] , line_data[xl-2] , line_data[xl-1] , line_data[xl-3] , line_data[xl-1] );

         //second lifting stage 

        update.Filter( line_data[xmid] , line_data[0] , line_data[1] , line_data[0] , line_data[2] ); 
        for (k=1 ; k<xmid-2 ; ++k)
        {
            update.Filter( line_data[xmid+k] , line_data[k] , line_data[k+1] , line_data[k-1] , line_data[k+2] );
        }// i 
        update.Filter( line_data[xl-2] , line_data[xmid-2] , line_data[xmid-1] , line_data[xmid-3] , line_data[xmid-1] );
        update.Filter( line_data[xl-1] , line_data[xmid-1] , line_data[xmid-1] , line_data[xmid-2] , line_data[xmid-1] );

        // Shift right by one bit to counter the shift in the analysis stage
        ShiftRowRight(line_data, xl, 1);

    }// j

    _mm_empty();
    // Interleave subbands 
    Interleave_mmx( xp , yp , xl , yl , coeff_data );  
}

#if 0
//Opts - Attempt1
void VHFilterLEGALL5_3::Synth(const int xp ,
                                          const int yp , 
                                          const int xl , 
                                          const int yl , 
                                          CoeffArray& coeff_data)
{
    int i,j,k;

    const int xend( xp+xl );
    const int yend( yp+yl );

    const PredictStepShift< 2 > predict;
    const UpdateStepShift< 1 > update;

    CoeffType* line_data;

    // Firstly reorder to interleave subbands, so that subsequent calculations 
    // can be in-place
    Interleave_mmx( xp , yp , xl , yl , coeff_data );

    // Next, do the vertical synthesis
    // First lifting stage
    int xstop = (xend>>2)<<2;
    
    // Begin the top edge
    CoeffType *row1, *row2, *row3, *row4;

    row1 = &coeff_data[yp][xp];
    row2 = &coeff_data[yp+1][xp];
    for ( i = xp ; i < xstop ; i+=4)
    {
        __m64 tmp = _mm_add_pi16 (*(__m64 *)row2, *(__m64 *)row2);
        tmp = _mm_srai_pi16(tmp, 2);
        *(__m64 *)row1 = _mm_sub_pi16 (*(__m64*)row1, tmp);

        row1 += 4;
        row2 += 4;
    }
    // Mopup
    for ( i = xstop ; i < xend ; ++i)
    {
        predict.Filter( coeff_data[yp][i] , coeff_data[yp+1][i] , coeff_data[yp+1][i] );
    }// i


    // Next, do the middle bit
    for ( k = yp+2 ; k < yend-2 ; k+=2)
    {
        CoeffType *row1 = &coeff_data[k-2][xp];
        CoeffType *row2 = &coeff_data[k-1][xp];
        CoeffType *row3 = &coeff_data[k][xp];
        CoeffType *row4 = &coeff_data[k+1][xp];

        for ( i = xp ; i < xstop ; i+=4)
        {
            __m64 tmp = _mm_add_pi16 (*(__m64 *)row4, *(__m64 *)row2);
            tmp = _mm_srai_pi16(tmp, 2);
            *(__m64 *)row3 = _mm_sub_pi16 (*(__m64*)row3, tmp);

            tmp = _mm_add_pi16 (*(__m64 *)row1, *(__m64 *)row3);
            tmp = _mm_srai_pi16(tmp, 1);
            *(__m64 *)row2 = _mm_add_pi16 (*(__m64*)row2, tmp);
            row1 += 4;
            row2 += 4;
            row3 += 4;
            row4 += 4;
        }// i

        //Mopup
        for ( i = xstop ; i < xend ; ++i)
        {
            predict.Filter( coeff_data[k][i] , coeff_data[k+1][i] , coeff_data[k-1][i] );
            update.Filter( coeff_data[k-1][i] , coeff_data[k-2][i] , coeff_data[k][i] );
        }// i
    }// j
    
    // Finally with the bottom edge
    row1 = &coeff_data[yend-4][xp];
    row2 = &coeff_data[yend-3][xp];
    row3 = &coeff_data[yend-2][xp];
    row4 = &coeff_data[yend-1][xp];

    for ( i = xp ; i < xstop ; i+=4)
    {
        __m64 tmp = _mm_add_pi16 (*(__m64 *)row2, *(__m64 *)row4);
        tmp = _mm_srai_pi16(tmp, 2);
        *(__m64 *)row3 = _mm_sub_pi16 (*(__m64*)row3, tmp);

        tmp = _mm_add_pi16 (*(__m64 *)row3, *(__m64 *)row1);
        tmp = _mm_srai_pi16(tmp, 1);
        *(__m64 *)row2 = _mm_add_pi16 (*(__m64*)row2, tmp);

        tmp = _mm_add_pi16 (*(__m64 *)row3, *(__m64 *)row3);
        tmp = _mm_srai_pi16(tmp, 1);
        *(__m64 *)row4 = _mm_add_pi16 (*(__m64*)row4, tmp);

        row1 += 4;
        row2 += 4;
        row3 += 4;
        row4 += 4;
    }// i
    // mopup
    for ( i = xstop ; i < xend ; ++i)
    {
        predict.Filter( coeff_data[yend-2][i] , coeff_data[yend-3][i] , coeff_data[yend-1][i] );
        update.Filter( coeff_data[yend-3][i] , coeff_data[yend-2][i] , coeff_data[yend-4][i] );
        update.Filter( coeff_data[yend-1][i] , coeff_data[yend-2][i] , coeff_data[yend-2][i] );
    }// i


    // Next do the horizontal synthesis
    for (j = yp;  j < yend ; ++j)
    {
        // First lifting stage 
        line_data = &coeff_data[j][xp];

        predict.Filter( line_data[0] , line_data[1] , line_data[1] );

        for ( k = 2;  k < xl -2; k+=2)
        { 
            predict.Filter( line_data[k] , line_data[k+1] , line_data[k-1] );
            update.Filter( line_data[k-1] , line_data[k-2] , line_data[k] );
        }// i
        
        predict.Filter( line_data[xl-2] , line_data[xl-3] , line_data[xl-1] ); 
        update.Filter( line_data[xl-3] , line_data[xl-2] , line_data[xl-4] );
        update.Filter( line_data[xl-1] , line_data[xl-2] , line_data[xl-2] );

    }
    _mm_empty();
}
#endif

#if 0
//Opts Attempt 2
void VHFilterLEGALL5_3::Synth(const int xp ,
                                          const int yp , 
                                          const int xl , 
                                          const int yl , 
                                          PicArray& coeff_data)
{
    int i,j,k;
    const int yend( yp+yl );

    const int xl2 (xl>>1);
    const int ymid (yp + (yl>>1));
    const PredictStepShift< 2 > predict;
    const UpdateStepShift< 1 > update;

    CoeffType* line_data;


    // Next, do the vertical synthesis
    // First lifting stage

    int xstop = (xl>>2)<<2;

    CoeffType *row1, *row2, *row3, *row4;
    // First do the top edge
    row1 = &coeff_data[yp][xp];
    row2 = &coeff_data[ymid][xp];
    for ( i = 0 ; i < xstop ; i+=4)
    {
        __m64 tmp = _mm_add_pi16 (*(__m64 *)row2, *(__m64 *)row2);
        tmp = _mm_srai_pi16(tmp, 2);
        *(__m64 *)row1 = _mm_sub_pi16 (*(__m64*)row1, tmp);
        row1+=4;
        row2+=4;
    }// i

    //mopup
    for ( i = xstop ; i < xl ; ++i)
    {
        predict.Filter( *row1 , *row2 , *row2);
        ++row1;
        ++row2;
    }// i

    // Next, do the middle bit
    for ( k = 1 ; k < ymid-1 ; ++k)
    {
        row1 = &coeff_data[k-1][xp];
        row2 = &coeff_data[k][xp];
        row3 = &coeff_data[ymid+k-1][xp];
        row4 = &coeff_data[ymid+k][xp];

        for ( i = 0 ; i < xstop ; i+=4)
        {
            __m64 tmp = _mm_add_pi16 (*(__m64 *)row3, *(__m64 *)row4);
            tmp = _mm_srai_pi16(tmp, 2);
            *(__m64 *)row2 = _mm_sub_pi16 (*(__m64*)row2, tmp);

            tmp = _mm_add_pi16 (*(__m64 *)row2, *(__m64 *)row1);
            tmp = _mm_srai_pi16(tmp, 1);
            *(__m64 *)row3 = _mm_add_pi16 (*(__m64*)row3, tmp);
            
            row1 += 4;
            row2 += 4;
            row3 += 4;
            row4 += 4;
        }// i

        for ( i = xstop ; i < xl ; ++i)
        {
            predict.Filter( *row2 , *row4 , *row3 );
            update.Filter( *row3 , *row2 , *row1 );
            ++row1;
            ++row2;
            ++row3;
            ++row4;
        }// i
    }// j


    // Finally with the bottom edge
    row1 = &coeff_data[ymid-2][xp];
    row2 = &coeff_data[ymid-1][xp];
    row3 = &coeff_data[yend-2][xp];
    row4 = &coeff_data[yend-1][xp];
    for ( i = xp ; i< xstop ; i+=4)
    {
        __m64 tmp = _mm_add_pi16 (*(__m64 *)row3, *(__m64 *)row4);
        tmp = _mm_srai_pi16(tmp, 2);
        *(__m64 *)row2 = _mm_sub_pi16 (*(__m64*)row2, tmp);

        tmp = _mm_add_pi16 (*(__m64 *)row2, *(__m64 *)row1);
        tmp = _mm_srai_pi16(tmp, 1);
        *(__m64 *)row3 = _mm_add_pi16 (*(__m64*)row3, tmp);
           
        tmp = _mm_add_pi16 (*(__m64 *)row2, *(__m64 *)row2);
        tmp = _mm_srai_pi16(tmp, 1);
        *(__m64 *)row4 = _mm_add_pi16 (*(__m64*)row4, tmp);
           
        row1 += 4;
        row2 += 4;
        row3 += 4;
        row4 += 4;
    }// i
    // mopup
    for ( i = xstop ; i< xl ; ++i)
    {
        predict.Filter( *row2 , *row3 , *row4 );
        update.Filter( *row3 , *row1 , *row2 );
        update.Filter( *row4 , *row2 , *row2 );
        ++row1;
        ++row2;
        ++row3;
        ++row4;
    }// i

    // Next do the horizontal synthesis
    xstop = (((xl2 - 2)>>2)<<2) + 1;
    //xstop = 1;
    for (j = yp;  j < yend ; ++j)
    {
        // First lifting stage 
        line_data = &coeff_data[j][xp];
        predict.Filter( line_data[0] , line_data[xl2] , line_data[xl2] );

        for ( k = 1;  k < xstop; k+=4)
        { 
            //predict.Filter( line_data[k] , line_data[xl2+k] , line_data[xl2+k-1] );
            __m64 m1 = _mm_add_pi16 (*(__m64 *)(line_data+xl2+k), *(__m64 *)(line_data+xl2+k-1));
            m1 = _mm_srai_pi16 (m1, 2);
            *(__m64 *)(line_data+k) = _mm_sub_pi16 (*(__m64 *)(line_data+k), m1);

            //update.Filter( line_data[xl2+k-1] , line_data[k] , line_data[k-1] );
            m1 = _mm_add_pi16 (*(__m64 *)(line_data+k), *(__m64 *)(line_data+k-1));
            m1 = _mm_srai_pi16(m1, 1);
            *(__m64 *)(line_data+xl2+k-1) = _mm_add_pi16 (*(__m64*)(line_data+xl2+k-1), m1);
            
            row1 += 4;
            row2 += 4;
            row3 += 4;
            row4 += 4;
        }// i


        for ( k = xstop;  k < xl2-1; ++k)
        { 
            predict.Filter( line_data[k] , line_data[xl2+k] , line_data[xl2+k-1] );
            update.Filter( line_data[xl2+k-1] , line_data[k] , line_data[k-1] );
        }// i

        predict.Filter( line_data[xl2-1] , line_data[xl-2] , line_data[xl-1] ); 
        update.Filter( line_data[xl-2] , line_data[xl2-2] , line_data[xl2-1] );
        update.Filter( line_data[xl-1] , line_data[xl2-1] , line_data[xl2-1] );
    }
    _mm_empty();
    
    // Finally interleave subbands
    Interleave_mmx( xp , yp , xl , yl , coeff_data );
}
#endif

//Attempt 3

inline void VHFilterLEGALL5_3::HorizSynth (int xp, int xl, int ystart, int yend, CoeffArray &coeff_data)
{
    static const PredictStepShift< 2 > predict;
    static const UpdateStepShift< 1 > update;
    int j, k;
    // Next do the horizontal synthesis
    for (j = ystart;  j <= yend ; ++j)
    {
        // First lifting stage 
        CoeffType *line_data = &coeff_data[j][xp];

        predict.Filter( line_data[0] , line_data[1] , line_data[1] );

        for ( k = 2;  k < xl -2; k+=2)
        { 
            predict.Filter( line_data[k] , line_data[k+1] , line_data[k-1] );
            update.Filter( line_data[k-1] , line_data[k-2] , line_data[k] );
        }// i
        
        predict.Filter( line_data[xl-2] , line_data[xl-3] , line_data[xl-1] ); 
        update.Filter( line_data[xl-3] , line_data[xl-2] , line_data[xl-4] );
        update.Filter( line_data[xl-1] , line_data[xl-2] , line_data[xl-2] );
        // Shift right by one bit to counter the shift in the analysis stage
        ShiftRowRight(line_data, xl, 1);
    }
}

void VHFilterLEGALL5_3::Synth(const int xp ,
                                          const int yp , 
                                          const int xl , 
                                          const int yl , 
                                          CoeffArray &coeff_data)
{
    int i, k;

    const int xend( xp+xl );
    const int yend( yp+yl );

    const PredictStepShift< 2 > predict;
    __m64 pred_round = _mm_set_pi16 (1<<(2-1), 1<<(2-1), 1<<(2-1), 1<<(2-1));
    const UpdateStepShift< 1 > update;
    __m64 update_round = _mm_set_pi16 (1, 1, 1, 1);

    int horiz_start = 0;
    int horiz_end = 0;

    // Firstly reorder to interleave subbands, so that subsequent calculations 
    // can be in-place
    Interleave_mmx( xp , yp , xl , yl , coeff_data );

    // Next, do the vertical synthesis
    // First lifting stage
    int xstop = (xend>>2)<<2;
    
    // Begin the top edge
    CoeffType *row1, *row2, *row3, *row4;

    row1 = &coeff_data[yp][xp];
    row2 = &coeff_data[yp+1][xp];
    for ( i = xp ; i < xstop ; i+=4)
    {
        __m64 tmp = _mm_add_pi16 (*(__m64 *)row2, *(__m64 *)row2);
        tmp = _mm_add_pi16 (tmp, pred_round);
        tmp = _mm_srai_pi16(tmp, 2);
        *(__m64 *)row1 = _mm_sub_pi16 (*(__m64*)row1, tmp);

        row1 += 4;
        row2 += 4;
    }
    // Mopup
    for ( i = xstop ; i < xend ; ++i)
    {
        predict.Filter( *row1, *row2, *row2 );
        ++row1;
        ++row2;
    }// i


    // Next, do the middle bit
    for ( k = yp+2 ; k < yend-2 ; k+=2)
    {
        CoeffType *row1 = &coeff_data[k-2][xp];
        CoeffType *row2 = &coeff_data[k-1][xp];
        CoeffType *row3 = &coeff_data[k][xp];
        CoeffType *row4 = &coeff_data[k+1][xp];

        for ( i = xp ; i < xstop ; i+=4)
        {
            __m64 tmp = _mm_add_pi16 (*(__m64 *)row4, *(__m64 *)row2);
            tmp = _mm_add_pi16 (tmp, pred_round);
            tmp = _mm_srai_pi16(tmp, 2);
            *(__m64 *)row3 = _mm_sub_pi16 (*(__m64*)row3, tmp);

            tmp = _mm_add_pi16 (*(__m64 *)row1, *(__m64 *)row3);
            tmp = _mm_add_pi16 (tmp, update_round);
            tmp = _mm_srai_pi16(tmp, 1);
            *(__m64 *)row2 = _mm_add_pi16 (*(__m64*)row2, tmp);
            row1 += 4;
            row2 += 4;
            row3 += 4;
            row4 += 4;
        }// i

        //Mopup
        for ( i = xstop ; i < xend ; ++i)
        {
            predict.Filter( *row3, *row2, *row4 );
            update.Filter( *row2, *row1, *row3 );
            ++row1;
            ++row2;
            ++row3;
            ++row4;
        }// i
        horiz_end = k - 2;
        // Do the horizontal synthesis
        HorizSynth (xp, xl, horiz_start, horiz_end, coeff_data);
        horiz_start = horiz_end + 1;
    }// j
    
    // Finally with the bottom edge
    row1 = &coeff_data[yend-4][xp];
    row2 = &coeff_data[yend-3][xp];
    row3 = &coeff_data[yend-2][xp];
    row4 = &coeff_data[yend-1][xp];

    for ( i = xp ; i < xstop ; i+=4)
    {
        __m64 tmp = _mm_add_pi16 (*(__m64 *)row2, *(__m64 *)row4);
        tmp = _mm_add_pi16 (tmp, pred_round);
        tmp = _mm_srai_pi16(tmp, 2);
        *(__m64 *)row3 = _mm_sub_pi16 (*(__m64*)row3, tmp);

        tmp = _mm_add_pi16 (*(__m64 *)row3, *(__m64 *)row1);
           tmp = _mm_add_pi16 (tmp, update_round);
        tmp = _mm_srai_pi16(tmp, 1);
        *(__m64 *)row2 = _mm_add_pi16 (*(__m64*)row2, tmp);

        tmp = _mm_add_pi16 (*(__m64 *)row3, *(__m64 *)row3);
        tmp = _mm_add_pi16 (tmp, update_round);
        tmp = _mm_srai_pi16(tmp, 1);
        *(__m64 *)row4 = _mm_add_pi16 (*(__m64*)row4, tmp);

        row1 += 4;
        row2 += 4;
        row3 += 4;
        row4 += 4;
    }// i
    // mopup
    for ( i = xstop ; i < xend ; ++i)
    {
        predict.Filter( *row3, *row2, *row4 );
        update.Filter( *row2, *row1, *row3 );
        update.Filter( *row4, *row3, *row3 );
        ++row1;
        ++row2;
        ++row3;
        ++row4;
    }// i

    _mm_empty();
    // Last lines of horizontal synthesis
    HorizSynth (xp, xl, horiz_start, yend-1, coeff_data);
}


void DeInterleave_mmx( const int xp , 
                 const int yp , 
                 const int xl , 
                 const int yl , 
                 CoeffArray &coeff_data)
{
    const int xl2( xl>>1);
    const int yl2( yl>>1);
    const int yend( yp + yl );

    if (coeff_data.LengthX() > t_temp_data.LengthX() ||
        coeff_data.LengthY() > t_temp_data.LengthY())
    {
        t_temp_data.Resize(coeff_data.LengthY(), coeff_data.LengthX());
    }

    // Make a temporary copy of the subband
    for (int j = yp; j<yend ; j++ )
        memcpy( t_temp_data[j-yp] , coeff_data[j]+xp , xl * sizeof( CoeffType ) );

    int stopx = (xl2>>2)<<2;
    
    for (int j = yp, s=0; j<(yp+yl2) ; j++, s+=2)
    {
        CoeffType *tmp1 = &t_temp_data[s][0];
        CoeffType *out1 = &coeff_data[j][xp];
        CoeffType *out2 = &coeff_data[j][xl2];
        int r = xp;
        for (int i = 0; i<xp+stopx ; i+=4 , r+=8)
        {
            __m64 m1 = _mm_unpacklo_pi16 (*(__m64 *)tmp1, *(__m64 *)(tmp1+4));
            __m64 m2 = _mm_unpackhi_pi16 (*(__m64 *)tmp1, *(__m64 *)(tmp1+4));
            *(__m64 *)out1 = _mm_unpacklo_pi16 (m1, m2);
            *(__m64 *)out2 = _mm_unpackhi_pi16 (m1, m2);
            out1 += 4;
            out2 += 4;
            tmp1+=8;
        }
        //mopup
        for (int i = xp+stopx; i < xp+xl2; ++i, r+=2)
        {
            coeff_data[j][i] = t_temp_data[s][r];
            coeff_data[j][i+xl2] = t_temp_data[s][r+1];
        }
    }// j 

    for (int j = yl2, s=1; j< yend ; j++, s+=2)
    {
        CoeffType *tmp1 = &t_temp_data[s][0];
        CoeffType *out1 = &coeff_data[j][xp];
        CoeffType *out2 = &coeff_data[j][xl2];
        int r = xp;
        for (int i = 0; i<xp+stopx ; i+=4 , r+=8)
        {
            __m64 m1 = _mm_unpacklo_pi16 (*(__m64 *)tmp1, *(__m64 *)(tmp1+4));
            __m64 m2 = _mm_unpackhi_pi16 (*(__m64 *)tmp1, *(__m64 *)(tmp1+4));
            *(__m64 *)out1 = _mm_unpacklo_pi16 (m1, m2);
            *(__m64 *)out2 = _mm_unpackhi_pi16 (m1, m2);
            out1 += 4;
            out2 += 4;
            tmp1+=8;
        }
        //mopup
        for (int i = xp+stopx; i < xp+xl2; ++i, r+=2)
        {
            coeff_data[j][i] = t_temp_data[s][r];
            coeff_data[j][i+xl2] = t_temp_data[s][r+1];
        }
    }// j 
    _mm_empty();
}

void VHFilterLEGALL5_3::Split(const int xp , 
                                          const int yp , 
                                          const int xl , 
                                          const int yl , 
                                          CoeffArray& coeff_data)
{
    //version based on integer-like types
    //using edge-extension rather than reflection

    const int xend=xp+xl;
    const int yend=yp+yl;
    const int xl2 = xl>>1;
    const int yl2 = yl>>1;

    CoeffType* line_data; 

    // Positional variables
    int i,j,k; 
  
    // Objects to do lifting stages 
    // (in revese order and type from synthesis)
    const PredictStepShift< 1 > predict;
    __m64 pred_round = _mm_set_pi16 (1, 1, 1, 1);
    const UpdateStepShift< 2 > update;
    __m64 update_round = _mm_set_pi16 (1<<(2-1), 1<<(2-1), 1<<(2-1), 1<<(2-1));

    // Lastly, have to reorder so that subbands are no longer interleaved
    DeInterleave_mmx( xp , yp , xl , yl , coeff_data );
     //first do horizontal 

    for (j = yp;  j < yend; ++j)
    {
        // First lifting stage
        line_data = &coeff_data[j][xp];
        // Shift left by one bit to give us more accuracy
        ShiftRowLeft(line_data, xl, 1);

        predict.Filter( line_data[xp+xl2] , line_data[1] , line_data[0] );
        update.Filter( line_data[0] , line_data[xp+xl2] , line_data[xp+xl2] );

        for (k = 1; k < xp+xl2-1; k+=1)
        {
            predict.Filter( line_data[xp+xl2+k] , line_data[k+1] , line_data[k] );
            update.Filter(  line_data[k] , line_data[xp+xl2+k-1] , line_data[xp+xl2+k] );
        }// i
        
        predict.Filter( line_data[xl-1] , line_data[xp+xl2-1] , line_data[xp+xl2-1] );
        update.Filter( line_data[xp+xl2-1] , line_data[xl-2] , line_data[xl-1] );

    }// j

    // next do vertical

    // First lifting stage

    // top edge - j=xp
    int stopX = (xl>>2)<<2;
    CoeffType *in_val = &coeff_data[yp+yl2][xp];
    CoeffType *val1 = &coeff_data[1][xp];
    CoeffType *val2 = &coeff_data[0][xp];
    for ( i = xp ; i<(xp+stopX); i+=4)
    {
        //predict.Filter( coeff_data[yp+yl2][i] , coeff_data[1][i] , coeff_data[0][i] );
        __m64 m1 = _mm_add_pi16 (*(__m64 *)val1, *(__m64 *)val2);
        m1 = _mm_add_pi16 (m1, pred_round);
        m1 = _mm_srai_pi16(m1, 1);
        *(__m64 *)in_val = _mm_sub_pi16 (*(__m64 *)in_val, m1);

        //update.Filter( coeff_data[0][i] , coeff_data[yp+yl2][i] , coeff_data[yp+yl2][i] );
        m1 = _mm_add_pi16 (*(__m64 *)in_val, *(__m64 *)in_val);
        m1 = _mm_add_pi16 (m1, update_round);
        m1 = _mm_srai_pi16(m1, 2);
        *(__m64 *)val2 = _mm_add_pi16 (*(__m64 *)val2, m1);
        in_val += 4;
        val1 += 4;
        val2 += 4;
    }// i
    // mopup
    for ( i = xp+stopX ; i<xend ; ++ i)
    {
        predict.Filter( coeff_data[yp+yl2][i] , coeff_data[1][i] , coeff_data[0][i] );
        update.Filter( coeff_data[0][i] , coeff_data[yp+yl2][i] , coeff_data[yp+yl2][i] );
    }// i

    // middle bit
    for (k = 1 ; k<yp+yl2-1 ; k+=1)
    {
        CoeffType *in_val = &coeff_data[yp+yl2+k][xp];
        CoeffType *in_val2 = &coeff_data[yp+yl2+k-1][xp];
        CoeffType *val1 = &coeff_data[k+1][xp];
        CoeffType *val2 = &coeff_data[k][xp];
        for ( i = xp ; i<xp+stopX ; i+=4)
        {
            //predict.Filter( coeff_data[yp+yl2+k][i] , coeff_data[k+1][i] , coeff_data[k][i] );
            __m64 m1 = _mm_add_pi16 (*(__m64 *)val1, *(__m64 *)val2);
            m1 = _mm_add_pi16 (m1, pred_round);
            m1 = _mm_srai_pi16(m1, 1);
            *(__m64 *)in_val = _mm_sub_pi16 (*(__m64 *)in_val, m1);
            
            //update.Filter( coeff_data[k][i] , coeff_data[yp+yl2+k-1][i] , coeff_data[yp+yl2+k][i] );
            m1 = _mm_add_pi16 (*(__m64 *)in_val, *(__m64 *)in_val2);
            m1 = _mm_add_pi16 (m1, update_round);
            m1 = _mm_srai_pi16(m1, 2);
            *(__m64 *)val2 = _mm_add_pi16 (*(__m64 *)val2, m1);

            in_val += 4;
            in_val2 += 4;
            val1 += 4;
            val2 += 4;
        }// i

        //mopup
        for ( i = xp+stopX ; i<xend ; ++ i)
        {
            predict.Filter( coeff_data[yp+yl2+k][i] , coeff_data[k+1][i] , coeff_data[k][i] );
            update.Filter( coeff_data[k][i] , coeff_data[yp+yl2+k-1][i] , coeff_data[yp+yl2+k][i] );
        }// i
    }// j

    in_val = &coeff_data[yend-1][xp];
    val2 = &coeff_data[yp+yl2-1][xp];
    CoeffType *in_val2 = &coeff_data[yend-2][xp];
    // bottom edge
    for ( i = xp ; i<xp+stopX ; i+=4)
    {
        //predict.Filter( coeff_data[yend-1][i] , coeff_data[yp+yl2-1][i] , coeff_data[yp+yl2-1][i] );
        __m64 m1 = _mm_add_pi16 (*(__m64 *)val2, *(__m64 *)val2);
        m1 = _mm_add_pi16 (m1, pred_round);
        m1 = _mm_srai_pi16(m1, 1);
        *(__m64 *)in_val = _mm_sub_pi16 (*(__m64 *)in_val, m1);
       
       //update.Filter( coeff_data[yp+yl2-1][i] , coeff_data[yend-2][i] , coeff_data[yend-1][i] );
        m1 = _mm_add_pi16 (*(__m64 *)in_val2, *(__m64 *)in_val);
        m1 = _mm_add_pi16 (m1, update_round);
        m1 = _mm_srai_pi16(m1, 2);
        *(__m64 *)val2 = _mm_add_pi16 (*(__m64 *)val2, m1);

        in_val += 4;
        in_val2 += 4;
        val2 += 4;
    }// i
    // mopup
    for ( i = xp+stopX ; i<xend ; ++ i)
    {
        predict.Filter( coeff_data[yend-1][i] , coeff_data[yp+yl2-1][i] , coeff_data[yp+yl2-1][i] );
        update.Filter( coeff_data[yp+yl2-1][i] , coeff_data[yend-2][i] , coeff_data[yend-1][i] );
    }// i
    _mm_empty();
}
#endif
