/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: mot_comp_mmx.cpp,v 1.8 2007/03/19 16:18:59 asuraparaju Exp $ $Name: Dirac_0_8_0 $
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

#if defined(HAVE_MMX)
#include <mmintrin.h>
#include <libdirac_common/mot_comp.h>
#include <libdirac_common/mot_comp_mmx.h>
#include <libdirac_common/motion.h>
#include <libdirac_common/dirac_assertions.h>
using namespace dirac;

inline void check_active_columns(
        int x, int xmax, ValueType act_cols1[4], 
        ValueType act_cols2[4], ValueType *row1, ValueType *row2)
{
    // check if we need any clipping
    if (x >= 0 && (x+3) < xmax) {
        // special case, nothing to do
        memcpy(act_cols1, &row1[x], 4 * sizeof(ValueType));
        memcpy(act_cols2, &row2[x], 4 * sizeof(ValueType));
    }
    else
    {
        act_cols1[0] = row1[BChk(x,xmax)];
        act_cols2[0] = row2[BChk(x,xmax)];
        act_cols1[1] = row1[BChk(x+1,xmax)];
        act_cols2[1] = row2[BChk(x+1,xmax)];
        act_cols1[2] = row1[BChk(x+2,xmax)];
        act_cols2[2] = row2[BChk(x+2,xmax)];
        act_cols1[3] = row1[BChk(x+3,xmax)];
        act_cols2[3] = row2[BChk(x+3,xmax)];
    }
}

void MotionCompensator_QuarterPixel::CompensateBlock( TwoDArray<CalcValueType> &pic_data , 
                                   const ImageCoords& orig_pic_size , 
                                   const PicArray &refup_data , 
                                   const MVector &mv , 
                                   const ImageCoords& pos , 
                                   const TwoDArray<ValueType>& wt_array )
{
    //Coordinates in the image being written to.
    const ImageCoords start_pos( std::max(pos.x,0) , std::max(pos.y,0) );
    // check that we are doing MC within true pic boundaries
    if (start_pos.x >= orig_pic_size.x || start_pos.y >= orig_pic_size.y)
        return;

    const ImageCoords end_pos( std::min( pos.x + wt_array.LengthX() , orig_pic_size.x ) , 
                               std::min( pos.y + wt_array.LengthY() , orig_pic_size.y ) );

    //The difference between the desired start point
    //pos and the actual start point start_pos.
    const ImageCoords diff( start_pos.x - pos.x , start_pos.y - pos.y );

    // Set up the start point in the reference image by rounding the motion vector
    // to 1/2 pel accuracy.NB: bit shift rounds negative values DOWN, as required
    const MVector roundvec( mv.x>>1 , mv.y>>1 );

    //Get the remainder after rounding. NB rmdr values always 0 or 1
    const MVector rmdr( mv.x & 1 , mv.y & 1 );

    //Where to start in the upconverted image
    const ImageCoords ref_start( ( start_pos.x<<1 ) + roundvec.x ,( start_pos.y<<1 ) + roundvec.y );

    //An additional stage to make sure the block to be copied does not fall outside
    //the reference image.
    const int refXlen = refup_data.LengthX();
    //const int refYlen = refup_data.LengthY();
    
    const int trueRefXlen = orig_pic_size.x << 1;
    const int trueRefYlen = orig_pic_size.y << 1;

    CalcValueType *pic_curr = &pic_data[0][start_pos.x];
    ValueType *wt_curr = &wt_array[diff.y][diff.x];

    const int block_width = end_pos.x - start_pos.x;
    if (block_width <= 0)
    {
        std::cerr << "Block_width=" << block_width << std::endl;
        return;
    }

    const int pic_next( pic_data.LengthX() - block_width ); //go down a row and back to beginning of block line
    const int wt_next( wt_array.LengthX() - block_width ); //go down a row and back to beginning of block line

    bool do_bounds_checking = false;

    //Check if there are going to be any problems copying the block from
    //the upvconverted reference image.
    if( ref_start.x < 0 ) 
        do_bounds_checking = true;
    else if( ref_start.x + ((end_pos.x - start_pos.x)<<1 ) >= trueRefXlen )
        do_bounds_checking = true;
    if( ref_start.y < 0 ) 
        do_bounds_checking = true;
    else if( ref_start.y + ((end_pos.y - start_pos.y)<<1 ) >= trueRefYlen)
        do_bounds_checking = true;

    if( !do_bounds_checking )
    {
        int stopX = (block_width>>2)<<2;
        ValueType *refup_curr = &refup_data[ref_start.y][ref_start.x];
        const int refup_next( ( refXlen - block_width )*2 ); //go down 2 rows and back to beginning of block line
        if( rmdr.x == 0 && rmdr.y == 0 )
        {
            __m64 m1, m2, m3;
            for( int y=end_pos.y-start_pos.y; y > 0; --y, pic_curr+=pic_next, wt_curr+=wt_next, refup_curr+=refup_next )
            {
                int x;
                for( x=0; x < stopX; x+=4, pic_curr+=4, wt_curr+=4, refup_curr+=8 )
                {
                    m1 = _mm_unpacklo_pi16 (*(__m64 *)refup_curr, *(__m64 *)(refup_curr+4));
                    m2 = _mm_unpackhi_pi16 (*(__m64 *)refup_curr, *(__m64 *)(refup_curr+4));
                    m1 = _mm_unpacklo_pi16 (m1, m2);
                    m2 = _mm_mulhi_pi16 (*(__m64 *)wt_curr, m1);
                    m1 = _mm_mullo_pi16 (*(__m64 *)wt_curr, m1);
                    m3 =  _mm_unpacklo_pi16 (m1, m2);
                    m1 =  _mm_unpackhi_pi16 (m1, m2);
                    *(__m64 *)pic_curr = _mm_add_pi32 (*(__m64 *)pic_curr, m3);
                    *(__m64 *)(pic_curr+2) = _mm_add_pi32 (*(__m64 *)(pic_curr+2), m1);
                }
                // Mopup the last value
                for ( x=stopX ; x < block_width; ++x)
                {
                    *pic_curr += CalcValueType( *refup_curr )* *wt_curr;
                    ++pic_curr;
                    ++wt_curr;
                    refup_curr+=2;
                }
            }
            _mm_empty();
        }
        else if( rmdr.y == 0 )
        {
            __m64 round = _mm_set_pi16 (1, 1, 1, 1);
            __m64 m1, m2, m3;

            for( int y=end_pos.y-start_pos.y; y > 0; --y, pic_curr+=pic_next, wt_curr+=wt_next, refup_curr+=refup_next )
            {
                int x;
                for( x=0; x < stopX; x+=4, pic_curr+=4, wt_curr+=4, refup_curr+=8 )
                {
                    m1 = _mm_unpacklo_pi16 (*(__m64 *)refup_curr, *(__m64 *)(refup_curr+4));
                    m3 = _mm_unpackhi_pi16 (*(__m64 *)refup_curr, *(__m64 *)(refup_curr+4));
                    m2 = _mm_unpackhi_pi16 (m1, m3);
                    m1 = _mm_unpacklo_pi16 (m1, m3);

                    m1 = _mm_add_pi16 (m1, m2);
                    
                    m1 = _mm_add_pi16 (m1, round);
                    m1 = _mm_srai_pi16 (m1, 1);

                    m2 = _mm_mulhi_pi16 (*(__m64 *)wt_curr, m1);
                    m1 = _mm_mullo_pi16 (*(__m64 *)wt_curr, m1);
                    
                    m3 =  _mm_unpacklo_pi16 (m1, m2);
                    m1 =  _mm_unpackhi_pi16 (m1, m2);
                    *(__m64 *)pic_curr = _mm_add_pi32 (*(__m64 *)pic_curr, m3);
                    *(__m64 *)(pic_curr+2) = _mm_add_pi32 (*(__m64 *)(pic_curr+2), m1);
                }

                // Mopup the last value
                for ( x=stopX; x < block_width; ++x)
                {
                    *pic_curr += ((    *refup_curr  +
                                       *(refup_curr+1)  + 1
                                  ) >> 1) * CalcValueType(*wt_curr);
                    ++pic_curr;
                    ++wt_curr;
                    refup_curr+=2;
                }
            }
            _mm_empty();
        }
        else if( rmdr.x == 0 )
        {
            __m64 round = _mm_set_pi16 (1, 1, 1, 1);
            __m64 m1, m2, m3;
            for( int y=end_pos.y-start_pos.y; y > 0; --y, pic_curr+=pic_next, wt_curr+=wt_next, refup_curr+=refup_next )
            {
                int x;
                for( x = 0; x < stopX; x+=4, pic_curr+=4, wt_curr+=4, refup_curr+=8 )
                {
                    m1 = _mm_unpacklo_pi16 (*(__m64 *)refup_curr, *(__m64 *)(refup_curr+4));
                    m2 = _mm_unpackhi_pi16 (*(__m64 *)refup_curr, *(__m64 *)(refup_curr+4));
                    m1 = _mm_unpacklo_pi16 (m1, m2);

                    m3 = _mm_unpacklo_pi16 (*(__m64 *)(refup_curr+refXlen), *(__m64 *)(refup_curr+refXlen+4));
                    m2 = _mm_unpackhi_pi16 (*(__m64 *)(refup_curr+refXlen), *(__m64 *)(refup_curr+refXlen+4));
                    m2 = _mm_unpacklo_pi16 (m3, m2);

                    m1 = _mm_add_pi16 (m1, m2);
                    m1 = _mm_add_pi16 (m1, round);
                    m1 = _mm_srai_pi16 (m1, 1);

                    m2 = _mm_mulhi_pi16 (*(__m64 *)wt_curr, m1);
                    m1 = _mm_mullo_pi16 (*(__m64 *)wt_curr, m1);
                    
                    m3 =  _mm_unpacklo_pi16 (m1, m2);
                    m1 =  _mm_unpackhi_pi16 (m1, m2);
                    *(__m64 *)pic_curr = _mm_add_pi32 (*(__m64 *)pic_curr, m3);
                    *(__m64 *)(pic_curr+2) = _mm_add_pi32 (*(__m64 *)(pic_curr+2), m1);
                }
                for ( x=stopX; x < block_width; ++x)
                {
                    *pic_curr += ((    *refup_curr + *(refup_curr+refXlen) +
                                       1
                                   ) >> 1) * CalcValueType(*wt_curr);
                    ++pic_curr;
                    ++wt_curr;
                    refup_curr+=2;
                }
            }
            _mm_empty();
        }
        else
        {
            __m64 round = _mm_set_pi16 (2, 2, 2, 2);
            __m64 m1, m2, m3;
            for( int y=end_pos.y-start_pos.y; y > 0; --y, pic_curr+=pic_next, wt_curr+=wt_next, refup_curr+=refup_next )
            {
                int x;
                for( x = 0; x < stopX; x+=4, pic_curr+=4, wt_curr+=4, refup_curr+=8 )
                {
                    m1 = _mm_add_pi16 (*(__m64 *)refup_curr, *(__m64 *)(refup_curr+refXlen));
                    m2 = _mm_add_pi16 (*(__m64 *)(refup_curr+4), *(__m64 *)(refup_curr+refXlen+4));
                    m3 = _mm_unpacklo_pi16 (m1, m2);
                    m1 = _mm_unpackhi_pi16 (m1, m2);

                    m2 = _mm_unpackhi_pi16 (m3, m1);
                    m1 = _mm_unpacklo_pi16 (m3, m1);

                    m1 = _mm_add_pi16 (m1, m2);
                    m1 = _mm_add_pi16 (m1, round);
                    m1 = _mm_srai_pi16 (m1, 2);
                    
                    m2 = _mm_mulhi_pi16 (*(__m64 *)wt_curr, m1);
                    m1 = _mm_mullo_pi16 (*(__m64 *)wt_curr, m1);
                    
                    m3 =  _mm_unpacklo_pi16 (m1, m2);
                    m1 =  _mm_unpackhi_pi16 (m1, m2);
                    *(__m64 *)pic_curr = _mm_add_pi32 (*(__m64 *)pic_curr, m3);
                    *(__m64 *)(pic_curr+2) = _mm_add_pi32 (*(__m64 *)(pic_curr+2), m1);
                }
                for ( x=stopX; x < block_width; ++x)
                {
                    *pic_curr += ((    *refup_curr  +
                                       *(refup_curr+1)  +
                                       *(refup_curr+refXlen)  +
                                       *(refup_curr+refXlen+1)  +
                                       2
                                   ) >> 2) * CalcValueType(*wt_curr);
                    ++pic_curr;
                    ++wt_curr;
                    refup_curr+=2;
                }
            }
            _mm_empty();
        }
    }
    else
    {
        // We're 2doing bounds checking because we'll fall off the edge of the reference otherwise.

        //weights for doing linear interpolation, calculated from the remainder values
        const ValueType linear_wts[4] = {  (2 - rmdr.x) * (2 - rmdr.y),    //tl
                                           rmdr.x * (2 - rmdr.y),          //tr
                                           (2 - rmdr.x) * rmdr.y,          //bl
                                           rmdr.x * rmdr.y };              //br

        __m64 m_wts1 = _mm_set_pi16 (linear_wts[1], linear_wts[0], linear_wts[1], linear_wts[0]);
        __m64 m_wts2 = _mm_set_pi16 (linear_wts[3], linear_wts[2], linear_wts[3], linear_wts[2]);

        ValueType act_cols1[4], act_cols2[4];
        ValueType *pact_cols1=act_cols1;
        ValueType *pact_cols2=act_cols2;

        int uX, uY, c, l;
        int stopX = (block_width>>1)<<1;
#ifdef WIN32
        stopX = stopX > 2 ? stopX - 2 : 0;
#endif
        __m64 m_two = _mm_set_pi32 (2, 2);
        __m64 m_zero = _mm_set_pi32 (0, 0);
        
       for(c = 0, uY = ref_start.y; c < end_pos.y - start_pos.y; ++c, pic_curr += pic_next, wt_curr += wt_next, uY += 2)
       {
           __m64 m1, m2;
           for(l = 0, uX = ref_start.x; l < stopX; l+=2, pic_curr+=2, wt_curr+=2, uX += 4)
           {
                  check_active_columns(uX, trueRefXlen, act_cols1, act_cols2, refup_data[BChk(uY, trueRefYlen)], refup_data[BChk(uY+1, trueRefYlen)]);

                m1 = _mm_madd_pi16 (*(__m64 *)pact_cols1, m_wts1);
                m2 = _mm_madd_pi16 (*(__m64 *)pact_cols2, m_wts2);
                m1 = _mm_add_pi32 (m1, m2);
                m1 = _mm_add_pi32 (m1, m_two);
                m1 = _mm_srai_pi32 (m1, 2);
                //m2 = _mm_unpackhi_pi32 (m1, m_zero);
                //m1 = _mm_unpacklo_pi32 (m1, m_zero);
                //m1 = _mm_packs_pi32 (m1, m2);
                m2 = _mm_unpacklo_pi16 (*(__m64 *)wt_curr, m_zero);
                m1 = _mm_madd_pi16 (m1, m2);
                *(__m64 *)pic_curr = _mm_add_pi32 (*(__m64 *)pic_curr , m1);
           }//l
           //mopup
           for(l = stopX, uX=ref_start.x + stopX*2; l < block_width; ++l, ++pic_curr, ++wt_curr, uX += 2)
           {
                   //std::cerr << "In mopup : stopX=" << stopX << " block_width=" << block_width<< std::endl;
                  check_active_columns(uX, trueRefXlen, act_cols1, act_cols2, refup_data[BChk(uY, trueRefYlen)], refup_data[BChk(uY+1, trueRefYlen)]);

               *pic_curr += ((     linear_wts[0] * CalcValueType( act_cols1[0] ) +
                                        linear_wts[1] * CalcValueType( act_cols1[1] ) +
                                        linear_wts[2] * CalcValueType( act_cols2[0] )+
                                        linear_wts[3] * CalcValueType( act_cols2[1] ) +
                                        2
                                  ) >> 2) * *wt_curr;
           }//l
       }//c
       _mm_empty();
    }
}

void MotionCompensator_HalfPixel::CompensateBlock( TwoDArray<CalcValueType> &pic_data , 
                                          const ImageCoords& orig_pic_size , 
                                          const PicArray &refup_data , 
                                          const MVector &mv , 
                                          const ImageCoords& pos , 
                                          const TwoDArray<ValueType>& wt_array)
{
    //Coordinates in the image being written to.
    const ImageCoords start_pos( std::max(pos.x,0) , std::max(pos.y,0) );
    const ImageCoords end_pos( std::min( pos.x + wt_array.LengthX() , orig_pic_size.x) , 
                               std::min( pos.y + wt_array.LengthY() , orig_pic_size.y ) );

    //The difference between the desired start point
    //pos and the actual start point start_pos.
    const ImageCoords diff( start_pos.x - pos.x , start_pos.y - pos.y );

    //Where to start in the upconverted image
    const ImageCoords ref_start( ( start_pos.x<<1 ) + mv.x ,( start_pos.y<<1 ) + mv.y );

    //An additional stage to make sure the block to be copied does not fall outside
    //the reference image.
    const int refXlen = refup_data.LengthX();
    //const int refYlen = refup_data.LengthY();
    const int trueRefXlen = orig_pic_size.x << 1;
    const int trueRefYlen = orig_pic_size.y << 1;

    bool do_bounds_checking = false;

    //Check if there are going to be any problems copying the block from
    //the upvconverted reference image.

    if( ref_start.x < 0 ) 
        do_bounds_checking = true;
    else if( ref_start.x + ((end_pos.x - start_pos.x -1 )<<1 ) >= trueRefXlen )
        do_bounds_checking = true;
    if( ref_start.y < 0 ) 
        do_bounds_checking = true;
    else if( ref_start.y + ((end_pos.y - start_pos.y - 1 )<<1 ) >= trueRefYlen)
        do_bounds_checking = true;

    CalcValueType *pic_curr = &pic_data[0][start_pos.x];
    ValueType *wt_curr = &wt_array[diff.y][diff.x];
 
    const int block_width = end_pos.x - start_pos.x;
    if (block_width <= 0)
        return;


    const int pic_next( pic_data.LengthX() - block_width );// go down a row and back up
    const int wt_next( wt_array.LengthX() - block_width ); // go down a row and back up

    if( !do_bounds_checking )
    {  
        ValueType *refup_curr = &refup_data[ref_start.y][ref_start.x];
        const int refup_next( (refXlen - block_width)*2 );// go down 2 rows and back up
        int stopX = (block_width>>2)<<2;
#if 1
        {
            __m64 m1, m2, m3;

            for( int y=end_pos.y-start_pos.y; y > 0; --y, pic_curr+=pic_next, wt_curr+=wt_next, refup_curr+=refup_next )
            {
                int x;
                for( x=0; x < stopX; x+=4, pic_curr+=4, wt_curr+=4, refup_curr+=8 )
                {
                    m1 = _mm_unpacklo_pi16 (*(__m64 *)refup_curr, *(__m64 *)(refup_curr+4));
                    m2 = _mm_unpackhi_pi16 (*(__m64 *)refup_curr, *(__m64 *)(refup_curr+4));
                    m1 = _mm_unpacklo_pi16 (m1, m2);
                    m2 = _mm_mulhi_pi16 (*(__m64 *)wt_curr, m1);
                    m1 = _mm_mullo_pi16 (*(__m64 *)wt_curr, m1);
                    m3 =  _mm_unpacklo_pi16 (m1, m2);
                    m1 =  _mm_unpackhi_pi16 (m1, m2);
                    *(__m64 *)pic_curr = _mm_add_pi32 (*(__m64 *)pic_curr, m3);
                    *(__m64 *)(pic_curr+2) = _mm_add_pi32 (*(__m64 *)(pic_curr+2), m1);
                }
                // Mopup the last value
                for ( x=stopX ; x < block_width; ++x)
                {
                    *pic_curr += CalcValueType( *refup_curr )* *wt_curr;
                    ++pic_curr;
                    ++wt_curr;
                    refup_curr+=2;
                }
            }
            _mm_empty();
        }
#else

        for( int y=end_pos.y-start_pos.y; y > 0; --y, pic_curr+=pic_next, wt_curr+=wt_next, refup_curr+=refup_next )
        {
            for( int x=block_width; x > 0; --x, ++pic_curr, ++wt_curr, refup_curr+=2 )
            {
                *pic_curr += CalcValueType( refup_curr[0] )* *wt_curr;
            }
        }
#endif
    }
    else
    {
        // We're doing bounds checking because we'll fall off the edge of the reference otherwise.
        for( int y=end_pos.y-start_pos.y, ry=ref_start.y, by=BChk(ry,trueRefYlen); 
             y>0; 
             --y, pic_curr+=pic_next, wt_curr+=wt_next , ry+=2 , by=BChk(ry,trueRefYlen))
        {
             for( int x=block_width , rx=ref_start.x , bx=BChk(rx,trueRefXlen); 
                  x>0 ; 
                  --x, ++pic_curr, ++wt_curr, rx+=2 , bx=BChk(rx,trueRefXlen))
             {
                 *pic_curr += CalcValueType( refup_data[by][bx] )* *wt_curr;
             }// x
        }// y
    }
}

namespace dirac
{
    void CompensateComponentAddAndShift_mmx (int start_y, int end_y, 
                                           int weight_bits,
                                           const ImageCoords& orig_pic_size,
                                           TwoDArray<CalcValueType> &comp_data, 
                                           PicArray &pic_data_out)
    {
        if (start_y >= end_y)
            return;
        const int round_val = 1<<(weight_bits-1);
        int stopX = pic_data_out.FirstX() + ((orig_pic_size.x>>2)<<2);
        int x_end_truepic_data = pic_data_out.FirstX() + orig_pic_size.x;
        int x_end_data = pic_data_out.FirstX() + pic_data_out.LengthX();
        __m64 max_val = _mm_set_pi32 (round_val, round_val);
           CalcValueType *pic_row = &comp_data[0][comp_data.FirstX()];
           ValueType *out_row = &pic_data_out[start_y][pic_data_out.FirstX()];
        for ( int i = start_y; i < end_y; i++)
        {
            for ( int j =  pic_data_out.FirstX(); j < stopX; j+=4)
            {
                __m64 in1 = _mm_add_pi32 (*(__m64 *)pic_row, max_val);
                in1 = _mm_srai_pi32 (in1, weight_bits);
                __m64 in2 = _mm_add_pi32 (*(__m64 *)(pic_row+2), max_val);
                in2 = _mm_srai_pi32 (in2, weight_bits);
                in1 = _mm_packs_pi32 (in1, in2);
                __m64 *out = (__m64 *)out_row;
                *out = _mm_add_pi16 (in1, *out);
                pic_row += 4;
                out_row += 4;
            }
            for ( int j =stopX; j < x_end_truepic_data; j++)
            {
                *out_row += static_cast<ValueType>( (*pic_row + round_val) >> weight_bits ); 
                ++out_row;
                ++pic_row;
            }
            // Now pad past the true picture with the last true pic val in
            // current row
            ValueType last_true_val = *(out_row - 1);
            for ( int j = x_end_truepic_data; j < x_end_data; ++j)
            {
                *out_row = last_true_val;
                ++out_row;
                ++pic_row;
            }
         }
        _mm_empty();
   }
}
#endif
