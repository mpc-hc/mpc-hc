/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: me_utils_mmx.cpp,v 1.8 2007/08/24 16:13:38 asuraparaju Exp $
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
* The Original Code is contributed by Peter Meerwald.
*
* The Initial Developer of the Original Code is Peter Meerwald.
* Portions created by the Initial Developer are Copyright (C) 2004.
* All Rights Reserved.
*
* Contributor(s): Peter Meerwald (Original Author)
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

#include <libdirac_common/dirac_assertions.h>
#include <libdirac_motionest/me_utils_mmx.h>

#if defined HAVE_MMX
using namespace dirac;

namespace dirac
{
    typedef union 
    {
        int  i[2];
        short  h[4];
        __m64 m;
    } u_mmx_val;

    CalcValueType simple_block_diff_mmx_4 ( 
            const BlockDiffParams& dparams, const MVector& mv, 
            const PicArray& pic_data, const PicArray& ref_data,
            CalcValueType i_best_sum)
    {
        u_mmx_val u_sum;

        u_sum.i[0] = u_sum.i[1] = 0;

        ValueType *src = &(pic_data[dparams.Yp()][dparams.Xp()]);
        ValueType *refd = &(ref_data[dparams.Yp()+mv.y][dparams.Xp()+mv.x]);

        int height = dparams.Yl();
        int width = dparams.Xl();
        int stopX = (width>>2)<<2;
        int pic_next = (pic_data.LengthX() - width);
        int ref_next = (ref_data.LengthX() - width);
        CalcValueType mop_sum = 0;
        for (int j = 0; j < height; j++)
        {
            for (int i = 0; i < stopX; i+=4) 
            {
                // pic - ref
                __m64 pic = _mm_sub_pi16 (*(__m64 *)src, *(__m64 *)refd);
                // abs (pic - ref)
                __m64 ref = _mm_srai_pi16(pic, 15);
                pic = _mm_xor_si64(pic, ref);
                pic = _mm_sub_pi16 (pic, ref);
                // sum += abs(pic -ref)
                ref = _mm_xor_si64(ref, ref);
                ref = _mm_unpackhi_pi16(pic, ref);
                pic = _mm_unpacklo_pi16(pic, pic);
                pic = _mm_srai_pi32 (pic, 16);
                pic = _mm_add_pi32 (pic, ref);
                u_sum.m = _mm_add_pi32 (u_sum.m, pic);
                src += 4;
                refd += 4;
            }
            for (int i = stopX; i < width; i++)
            {
                mop_sum += std::abs(*src - *refd);
                src++;
                refd++;
            }
            if ((u_sum.i[0] + u_sum.i[1] + mop_sum) >= i_best_sum)
            {
                _mm_empty();
                return i_best_sum;
            }
            src += pic_next;
            refd += ref_next;
        }
        _mm_empty();

        return  u_sum.i[0] + u_sum.i[1] + mop_sum;
    }


    CalcValueType simple_intra_block_diff_mmx_4 ( 
            const BlockDiffParams& dparams, 
            const PicArray& pic_data, ValueType &dc_val)
    {
        __m64 tmp = _mm_set_pi16(0, 0, 0, 0);
        u_mmx_val u_sum;
        u_sum.i[0] = u_sum.i[1] = 0;

        ValueType *src = &(pic_data[dparams.Yp()][dparams.Xp()]);

        int height = dparams.Yl();
        int width = dparams.Xl();
        int stopX = (width>>2)<<2;
        int pic_next = (pic_data.LengthX() - width);
        CalcValueType mop_sum = 0;
        for (int j = 0; j < height; j++)
        {
            for (int i = 0; i < stopX; i+=4) 
            {
                __m64 pic = *(__m64 *)src;
                // sum += (pic)
                tmp = _mm_xor_si64(tmp, tmp);
                tmp = _mm_unpackhi_pi16(pic, tmp);
                tmp = _mm_slli_pi32 (tmp, 16);
                tmp = _mm_srai_pi32 (tmp, 16);
                pic = _mm_unpacklo_pi16(pic, pic);
                pic = _mm_srai_pi32 (pic, 16);
                pic = _mm_add_pi32 (pic, tmp);
                u_sum.m = _mm_add_pi32 (u_sum.m, pic);
                src += 4;
            }
            // Mop up
            for (int i = stopX; i < width; ++i)
            {
                mop_sum += *src;
                src++;
            }
            src += pic_next;
        }

        CalcValueType int_dc =  (u_sum.i[0] + u_sum.i[1] + mop_sum)/(width*height);

        dc_val = static_cast<ValueType>( int_dc );

        // Now compute the resulting SAD
        __m64 dc = _mm_set_pi16 ( dc_val, dc_val , dc_val , dc_val);
        u_sum.m = _mm_xor_si64(u_sum.m, u_sum.m); // initialise sum to 0
        mop_sum = 0;
        
        src = &(pic_data[dparams.Yp()][dparams.Xp()]);
        for (int j = 0; j < height; ++j)
        {
            for (int i = 0; i < stopX; i+=4)
            {
                __m64 pic = *(__m64 *)src;
                // pic - dc
                pic = _mm_sub_pi16 (pic, dc);
                // abs (pic - dc)
                tmp = _mm_srai_pi16(pic, 15);
                pic = _mm_xor_si64(pic, tmp);
                pic = _mm_sub_pi16 (pic, tmp);
                // sum += abs(pic -dc)
                tmp = _mm_xor_si64(tmp, tmp);
                tmp = _mm_unpackhi_pi16(pic, tmp);
                pic = _mm_unpacklo_pi16(pic, pic);
                pic = _mm_srai_pi32 (pic, 16);
                pic = _mm_add_pi32 (pic, tmp);
                u_sum.m = _mm_add_pi32 (u_sum.m, pic);
                src += 4;
            }
            // Mop up
            for (int i = stopX; i < width; ++i)
            {
                mop_sum += std::abs(*src - dc_val);
                src++;
            }
            src += pic_next;
        }
        CalcValueType intra_cost = u_sum.i[0] + u_sum.i[1] + mop_sum;
        _mm_empty();

        return intra_cost;

    }

    /* 
    * NOTE: we are not doing any bounds checks here. This function must
    * be invoked only when the reference images start and stop fall
    * withing bounds
    */
    float simple_block_diff_up_mmx_4(
            const PicArray& pic_data, const PicArray& ref_data, 
            const ImageCoords& start_pos, const ImageCoords& end_pos, 
            const ImageCoords& ref_start, const ImageCoords& ref_stop,
            const MVector& rmdr, float cost_so_far, 
            float best_total_cost_so_far)
    {
        ValueType *pic_curr = &pic_data[start_pos.y][start_pos.x];
        ValueType *ref_curr = &ref_data[ref_start.y][ref_start.x];

        const int width = end_pos.x - start_pos.x;
        int height = end_pos.y - start_pos.y;
        const int ref_stride = ref_data.LengthX();

        // go down a row and back up
        const int pic_next = pic_data.LengthX() - width;
        // go down 2 rows and back up
        const int ref_next = ref_data.LengthX()*2 - width*2;

        REPORTM (ref_start.x>=0 && ref_stop.x < ref_data.LengthX() &&
                ref_start.y>=0 && ref_stop.y < ref_data.LengthY(), 
                "Reference image coordinates within bounds");

        CalcValueType sum = 0;
        CalcValueType mop_sum(0);
        int stopX = (width>>2)<<2;
        __m64 m_sum = _mm_set_pi16(0, 0, 0, 0);
        u_mmx_val u_sum;
        if (rmdr.x == 0 && rmdr.y == 0 )
        {
            //std::cerr << "Inmmx routine rmdr.x = rmdr.y = 0" << std::endl;
#if 1
            for( int y=0; y < height; y++, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                m_sum = _mm_xor_si64 (m_sum, m_sum);
                mop_sum= 0;
                for( int x=0; x < stopX; x+=4, pic_curr+=4, ref_curr+=8 )
                {
                    __m64 pic = *(__m64 *)pic_curr;
                    __m64 ref = _mm_unpacklo_pi16 (*(__m64 *)ref_curr, *(__m64 *)(ref_curr+4));
                    __m64 ref2 = _mm_unpackhi_pi16 (*(__m64 *)ref_curr, *(__m64 *)(ref_curr+4));
                    ref = _mm_unpacklo_pi16 ( ref, ref2);
                    // ref - pic
                    pic = _mm_sub_pi16 (pic, ref);
                    // abs (ref - pic)
                    ref = _mm_srai_pi16(pic, 15);
                    pic = _mm_xor_si64(pic, ref);
                    pic = _mm_sub_pi16 (pic, ref);
                    // sum += abs(ref -pic)
                    /** 
                    * Since we are re-initialising m_sum with every loop
                    * maybe we don't need the following since overflow may
                    * not occur
                    ref = _mm_xor_si64(ref, ref);
                    ref = _mm_unpackhi_pi16(pic, ref);
                    pic = _mm_unpacklo_pi16(pic, pic);
                    pic = _mm_srai_pi32 (pic, 16);
                    pic = _mm_add_pi32 (pic, ref);
                    m_sum = _mm_add_pi32 (m_sum, pic);
                    **/
                    m_sum = _mm_add_pi16 (m_sum, pic);
                }
                // mopup;
                for (int x = stopX; x < width; ++x, ++pic_curr,ref_curr+=2)
                {
                    mop_sum += std::abs (*ref_curr - *pic_curr);
                }
                u_sum.m = m_sum;
                //sum += (u_sum.i[0] + u_sum.i[1] + mop_sum);
                sum += (u_sum.h[0] + u_sum.h[1] + u_sum.h[2] + u_sum.h[3] + mop_sum);
                _mm_empty();
                if ((sum + cost_so_far )>= best_total_cost_so_far)
                {
                    return best_total_cost_so_far;
                }
            }
            _mm_empty();
            return sum + cost_so_far;
#else
            float sum = cost_so_far;
            for( int y=0; y < height; ++y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=0; x < width; ++x, ++pic_curr, ref_curr+=2 )
                {
                    sum += std::abs( *ref_curr - *pic_curr );
                }// x
                
                if ( sum>= best_total_cost_so_far)
                    return best_total_cost_so_far;

            }// y
            return sum;
#endif

        }
        else if( rmdr.y == 0 )
        {
#if 1
            __m64 m_one = _mm_set_pi16(1, 1, 1, 1);
            for( int y=0; y < height; y++, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                m_sum = _mm_xor_si64 (m_sum, m_sum);
                mop_sum= 0;
                for( int x=0; x < stopX; x+=4, pic_curr+=4, ref_curr+=8 )
                {
                    // Load ref
                    __m64 m1 = _mm_unpacklo_pi16 (*(__m64 *)ref_curr, *(__m64 *)(ref_curr+4));
                    __m64 m2 = _mm_unpackhi_pi16 (*(__m64 *)ref_curr, *(__m64 *)(ref_curr+4));
                    // m3 = words 0 2 4 6 of ref_curr
                    __m64 m3 = _mm_unpacklo_pi16 ( m1, m2);
                    // m2 = words 1 3 5 7 of ref_curr
                    m2 = _mm_unpackhi_pi16 ( m1, m2);
                    // (ref_curr[0] + ref_curr[1] + 1)>>1
                    m3 = _mm_add_pi16 (m3, m2);
                    m3 = _mm_add_pi16 (m3, m_one);
                    m3 = _mm_srai_pi16 (m3, 1);
                    // ref - pic
                    m1 = _mm_sub_pi16 (*(__m64 *)pic_curr, m3);
                    // abs (ref - pic)
                    m3 = _mm_srai_pi16(m1, 15);
                    m1 = _mm_xor_si64(m1, m3);
                    m1 = _mm_sub_pi16 (m1, m3);
                    // sum += abs(ref -pic)
                    /** 
                    * Since we are re-initialising m_sum with every loop
                    * maybe we don't need the following since overflow may
                    * not occur
                    ref = _mm_xor_si64(ref, ref);
                    ref = _mm_unpackhi_pi16(pic, ref);
                    pic = _mm_unpacklo_pi16(pic, pic);
                    pic = _mm_srai_pi32 (pic, 16);
                    pic = _mm_add_pi32 (pic, ref);
                    m_sum = _mm_add_pi32 (m_sum, pic);
                    **/
                    m_sum = _mm_add_pi16 (m_sum, m1);
                }
                // mopup;
                for (int x = stopX; x < width; ++x, ++pic_curr,ref_curr+=2)
                {
                    CalcValueType temp = (ref_curr[0] + ref_curr[1]+1)>>1;
                    mop_sum += std::abs (temp - *pic_curr);
                }
                u_sum.m = m_sum;
                //sum += (u_sum.i[0] + u_sum.i[1] + mop_sum);
                sum += (u_sum.h[0] + u_sum.h[1] + u_sum.h[2] + u_sum.h[3] + mop_sum);
                _mm_empty();
                if ((sum + cost_so_far )>= best_total_cost_so_far)
                {
                    return best_total_cost_so_far;
                }
            }
            _mm_empty();
            return sum + cost_so_far;
#else
            //std::cerr << "Inmmx routine rmdr.y == 0" << std::endl;
            CalcValueType sum(0);
            for( int y=0; y < height; ++y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=0; x < width; ++x, ++pic_curr, ref_curr+=2 )
                {
                    CalcValueType temp = (    CalcValueType( ref_curr[0] ) +
                                CalcValueType( ref_curr[1] ) +
                                1
                            ) >> 1;
                    sum += std::abs( temp - *pic_curr );
                }// x
                
                if ( (sum+cost_so_far)>=best_total_cost_so_far)
                    return best_total_cost_so_far;

            }// y
            return sum+cost_so_far;
#endif
        }
        else if( rmdr.x == 0 )
        {
#if 1
            __m64 m_one = _mm_set_pi16(1, 1, 1, 1);
            for( int y=0; y < height; y++, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                m_sum = _mm_xor_si64 (m_sum, m_sum);
                mop_sum= 0;
                for( int x=0; x < stopX; x+=4, pic_curr+=4, ref_curr+=8 )
                {
                    // Load ref
                    __m64 m1 = _mm_unpacklo_pi16 (*(__m64 *)ref_curr, *(__m64 *)(ref_curr+4));
                    __m64 m2 = _mm_unpackhi_pi16 (*(__m64 *)ref_curr, *(__m64 *)(ref_curr+4));
                    // m1 = words 0 2 4 6 of ref_curr
                    m1 = _mm_unpacklo_pi16 ( m1, m2);
                    // m2 = words 0 2 4 6 of ref_curr+ref_stride
                    m2 = _mm_unpacklo_pi16 (*(__m64 *)(ref_curr+ref_stride), *(__m64 *)(ref_curr+ref_stride+4));
                    __m64 m3 = _mm_unpackhi_pi16 (*(__m64 *)(ref_curr+ref_stride), *(__m64 *)(ref_curr+ref_stride+4));
                    m2 = _mm_unpacklo_pi16 (m2, m3);
                    
                    // (ref_curr[0] + ref_curr[ref_stride] + 1)>>1
                    m1 = _mm_add_pi16 (m1, m2);
                    m1 = _mm_add_pi16 (m1, m_one);
                    m1 = _mm_srai_pi16 (m1, 1);
                    // ref - pic
                    m1 = _mm_sub_pi16 (*(__m64 *)pic_curr, m1);
                    // abs (ref - pic)
                    m3 = _mm_srai_pi16(m1, 15);
                    m1 = _mm_xor_si64(m1, m3);
                    m1 = _mm_sub_pi16 (m1, m3);
                    // sum += abs(ref -pic)
                    m_sum = _mm_add_pi16 (m_sum, m1);
                }
                // mopup;
                for (int x = stopX; x < width; ++x, ++pic_curr,ref_curr+=2)
                {
                    CalcValueType temp = (ref_curr[0] + ref_curr[ref_stride]+1)>>1;
                    mop_sum += std::abs (temp - *pic_curr);
                }
                u_sum.m = m_sum;
                //sum += (u_sum.i[0] + u_sum.i[1] + mop_sum);
                sum += (u_sum.h[0] + u_sum.h[1] + u_sum.h[2] + u_sum.h[3] + mop_sum);
                _mm_empty();
                if ((sum + cost_so_far )>= best_total_cost_so_far)
                {
                    return best_total_cost_so_far;
                }
            }
            _mm_empty();
            return sum + cost_so_far;
#else
            CalcValueType sum(0);
            for( int y=0; y < height; ++y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=0; x < width; ++x, ++pic_curr, ref_curr+=2 )
                {
                    CalcValueType temp = (ref_curr[0] + ref_curr[ref_stride]+1)>>1;
                    sum += std::abs (temp - *pic_curr);
                }// x
                
                if ( (sum+cost_so_far)>=best_total_cost_so_far)
                    return best_total_cost_so_far;

            }// y
            return sum+cost_so_far;
#endif
        }
        else
        {
#if 1
            __m64 m_two = _mm_set_pi32(2, 2);
            __m64 m_one = _mm_set_pi16(1, 1, 1, 1);
            // processing four pic_data values at a time
            for( int y=0; y < height; y++, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                m_sum = _mm_xor_si64 (m_sum, m_sum);
                mop_sum= 0;
                for( int x=0; x < stopX; x+=4, pic_curr+=4, ref_curr+=8 )
                {
                    // Load ref
                    // m1 = words 0 1 2 3 of line 0 ref_curr
                    __m64 m1 = *(__m64 *)ref_curr;
                    // m1 = words 0 1 2 3 of line 1 of ref_curr
                    __m64 m2 = *(__m64 *)(ref_curr+ref_stride);
                    // (ref_curr[0] + ref_curr[1] + 
                    // ref_curr[ref_stride] + ref_curr[ref_stride+1] + 2) >>2
                    m1 = _mm_add_pi16 (m1, m2);
                    m1 = _mm_madd_pi16 (m1, m_one);
                    m1 = _mm_add_pi32 (m1, m_two);
                    m1 = _mm_srai_pi32 (m1, 2);


                    // m2 = words 4 5 6 7 of line 0 ref_curr
                    __m64 m3 = *(__m64 *)(ref_curr+4);
                    // m1 = words 4 5 6 7 of line 1 of ref_curr
                    m2 = *(__m64 *)(ref_curr+4+ref_stride);
                    // (ref_curr[0] + ref_curr[1] + 
                    // ref_curr[ref_stride] + ref_curr[ref_stride+1] + 2) >>2
                    m3 = _mm_add_pi16 (m3, m2);
                    m3 = _mm_madd_pi16 (m3, m_one);
                    m3 = _mm_add_pi32 (m3, m_two);
                    m3 = _mm_srai_pi32 (m3, 2);

                    m1 = _mm_packs_pi32 (m1, m3);

                    // load first four values pic_data
                    m2 = *(__m64 *)pic_curr;
                    
                    // ref - pic
                    m1 = _mm_sub_pi16 (m1, m2);
                    // abs (ref - pic)
                    m2 = _mm_srai_pi16(m1, 15);
                    m1 = _mm_xor_si64(m1, m2);
                    m1 = _mm_sub_pi16(m1, m2);
                    // sum += abs(ref -pic)
                    m_sum = _mm_add_pi16 (m_sum, m1);
                }
                // mopup;
                for (int x = stopX; x < width; ++x, ++pic_curr,ref_curr+=2)
                {
                    CalcValueType temp = (ref_curr[0] + ref_curr[1] +
                        ref_curr[ref_stride] + ref_curr[ref_stride+1]+2)>>2;
                    mop_sum += std::abs (temp - *pic_curr);
                }
                u_sum.m = m_sum;
                sum += (u_sum.h[0] + u_sum.h[1] + u_sum.h[2] + u_sum.h[3] + mop_sum);
                _mm_empty();
                if ((sum + cost_so_far )>= best_total_cost_so_far)
                {
                    return best_total_cost_so_far;
                }
            }
            _mm_empty();
            return sum + cost_so_far;
#else
            //std::cerr << "Inmmx routine rmdr.y == 0" << std::endl;
            CalcValueType sum(0);
            for( int y=0; y < height; ++y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=0; x < width; ++x, ++pic_curr, ref_curr+=2 )
                {
                    CalcValueType temp = ( CalcValueType( ref_curr[0] ) +
                                          CalcValueType( ref_curr[1] ) +
                                          CalcValueType( ref_curr[ref_stride] ) +
                                          CalcValueType( ref_curr[ref_stride+1] ) +
                                       2
                            ) >> 2;
                    sum += std::abs( temp - *pic_curr );
                }// x
                
                if ( (sum+cost_so_far)>=best_total_cost_so_far)
                    return best_total_cost_so_far;

            }// y
            return sum+cost_so_far;
#endif
        }
    return cost_so_far;
    }

    /* 
    * NOTE: we are not doing any bounds checks here. This function must
    * be invoked only when the reference images start and stop fall
    * withing bounds
    */
    void simple_biblock_diff_pic_mmx_4(
            const PicArray& pic_data, const PicArray& ref_data, 
            TwoDArray<ValueType>& diff,
            const ImageCoords& start_pos, const ImageCoords& end_pos, 
            const ImageCoords& ref_start, const ImageCoords& ref_stop,
            const MVector& rmdr)
    {
        ValueType *pic_curr = &pic_data[start_pos.y][start_pos.x];
        ValueType *ref_curr = &ref_data[ref_start.y][ref_start.x];
        ValueType *diff_curr = &diff[0][0];

        const int width = end_pos.x - start_pos.x;
        int height = end_pos.y - start_pos.y;
        const int ref_stride = ref_data.LengthX();

        // go down a row and back up
        const int pic_next = pic_data.LengthX() - width;
        // go down 2 rows and back up
        const int ref_next = ref_data.LengthX()*2 - width*2;

        REPORTM (ref_start.x>=0 && ref_stop.x < ref_data.LengthX() &&
                ref_start.y>=0 && ref_stop.y < ref_data.LengthY(), 
                "Reference image coordinates withing bounds");

        int stopX = (width>>2)<<2;
        if (rmdr.x == 0 && rmdr.y == 0 )
        {
            //std::cerr << "Inmmx routine rmdr.x = rmdr.y = 0" << std::endl;
#if 1
            for( int y=0; y < height; y++, pic_curr+=pic_next, ref_curr+=ref_next)
            {
                for( int x=0; x < stopX; x+=4, pic_curr+=4, ref_curr+=8, diff_curr += 4 )
                {
                    __m64 pic = *(__m64 *)pic_curr;
                    // pic << 1
                    pic = _mm_slli_pi16(pic, 1);
                    // load ref
                    __m64 ref = _mm_unpacklo_pi16 (*(__m64 *)ref_curr, *(__m64 *)(ref_curr+4));
                    __m64 ref2 = _mm_unpackhi_pi16 (*(__m64 *)ref_curr, *(__m64 *)(ref_curr+4));
                    ref = _mm_unpacklo_pi16 ( ref, ref2);
                    // pic<<1 - ref
                    *(__m64 *)diff_curr = _mm_sub_pi16 (pic, ref);
                }
                // mopup;
                for (int x = stopX; x < width; ++x, ++pic_curr, ++diff_curr, ref_curr+=2)
                {
                    *diff_curr = ((*pic_curr)<<1) - *ref_curr;
                }
            }
#else
            for( int y=0; y < height; ++y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=0; x < width; ++x, ++pic_curr, ++diff_curr, ref_curr+=2 )
                {
                    *diff_curr = ((*pic_curr)<<1) - *ref_curr;
                }// x
            }// y
#endif
        }
        else if( rmdr.y == 0 )
        {
#if 1
            __m64 m_one = _mm_set_pi16(1, 1, 1, 1);
            for( int y=0; y < height; y++, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=0; x < stopX; x+=4, pic_curr+=4, diff_curr += 4, ref_curr+=8 )
                {
                    // Load ref
                    __m64 m1 = _mm_unpacklo_pi16 (*(__m64 *)ref_curr, *(__m64 *)(ref_curr+4));
                    __m64 m2 = _mm_unpackhi_pi16 (*(__m64 *)ref_curr, *(__m64 *)(ref_curr+4));
                    // m3 = words 0 2 4 6 of ref_curr
                    __m64 m3 = _mm_unpacklo_pi16 ( m1, m2);
                    // m2 = words 1 3 5 7 of ref_curr
                    m2 = _mm_unpackhi_pi16 ( m1, m2);
                    // (ref_curr[0] + ref_curr[1] + 1)>>1
                    m3 = _mm_add_pi16 (m3, m2);
                    m3 = _mm_add_pi16 (m3, m_one);
                    m3 = _mm_srai_pi16 (m3, 1);
                    // pic << 1
                    m1 = _mm_slli_pi16(*(__m64 *)pic_curr, 1);
                    // diff = pic - ref
                    *(__m64 *)diff_curr = _mm_sub_pi16 (m1, m3);
                }
                // mopup;
                for (int x = stopX; x < width; ++x, ++pic_curr, ++diff_curr, ref_curr+=2)
                {
                    CalcValueType temp = (ref_curr[0] + ref_curr[1]+1)>>1;
                    *diff_curr = ((*pic_curr)<<1) - temp;
                }
            }
#else
            //std::cerr << "Inmmx routine rmdr.y == 0" << std::endl;
            for( int y=0; y < height; ++y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=0; x < width; ++x, ++pic_curr, ++diff_curr, ref_curr+=2 )
                {
                    CalcValueType temp = (    CalcValueType( ref_curr[0] ) +
                                CalcValueType( ref_curr[1] ) +
                                1
                            ) >> 1;
                    *diff_curr = ((*pic_curr)<<1) - temp;
                }// x
                
            }// y
#endif
        }
        else if( rmdr.x == 0 )
        {
#if 1
            __m64 m_one = _mm_set_pi16(1, 1, 1, 1);
            for( int y=0; y < height; y++, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=0; x < stopX; x+=4, pic_curr+=4, diff_curr +=4, ref_curr+=8 )
                {
                    // Load ref
                    __m64 m1 = _mm_unpacklo_pi16 (*(__m64 *)ref_curr, *(__m64 *)(ref_curr+4));
                    __m64 m2 = _mm_unpackhi_pi16 (*(__m64 *)ref_curr, *(__m64 *)(ref_curr+4));
                    // m1 = words 0 2 4 6 of ref_curr
                    m1 = _mm_unpacklo_pi16 ( m1, m2);
                    // m2 = words 0 2 4 6 of ref_curr+ref_stride
                    m2 = _mm_unpacklo_pi16 (*(__m64 *)(ref_curr+ref_stride), *(__m64 *)(ref_curr+ref_stride+4));
                    __m64 m3 = _mm_unpackhi_pi16 (*(__m64 *)(ref_curr+ref_stride), *(__m64 *)(ref_curr+ref_stride+4));
                    m2 = _mm_unpacklo_pi16 (m2, m3);
                    
                    // (ref_curr[0] + ref_curr[ref_stride] + 1)>>1
                    m1 = _mm_add_pi16 (m1, m2);
                    m1 = _mm_add_pi16 (m1, m_one);
                    m1 = _mm_srai_pi16 (m1, 1);
                    // pic << 1
                    m2 = _mm_slli_pi16 (*(__m64 *)pic_curr, 1);
                    // diff = pic<<1 - ref)
                    *(__m64 *)diff_curr = _mm_sub_pi16(m2, m1 );
                }
                // mopup;
                for (int x = stopX; x < width; ++x, ++pic_curr, ++diff_curr, ref_curr+=2)
                {
                    CalcValueType temp = (ref_curr[0] + ref_curr[ref_stride]+1)>>1;
                    *diff_curr = ((*pic_curr)<<1) - temp;
                }
            }
#else
            //std::cerr << "Inmmx routine rmdr.y == 0" << std::endl;
            for( int y=0; y < height; ++y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=0; x < width; ++x, ++pic_curr, ++diff_curr, ref_curr+=2 )
                {
                    CalcValueType temp = (    CalcValueType( ref_curr[0] ) +
                                CalcValueType( ref_curr[1] ) +
                                1
                            ) >> 1;
                    *diff_curr = ((*pic_curr)<<1) - temp;
                }// x
            }// y
#endif
        }
        else
        {
#if 1
            __m64 m_two = _mm_set_pi32(2, 2);
            __m64 m_one = _mm_set_pi16(1, 1, 1, 1);
            // processing four pic_data values at a time
            for( int y=0; y < height; y++, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=0; x < stopX; x+=4, pic_curr+=4, diff_curr+=4, ref_curr+=8 )
                {
                    // Load ref
                    // m1 = words 0 1 2 3 of line 0 ref_curr
                    __m64 m1 = *(__m64 *)ref_curr;
                    // m1 = words 0 1 2 3 of line 1 of ref_curr
                    __m64 m2 = *(__m64 *)(ref_curr+ref_stride);
                    // (ref_curr[0] + ref_curr[1] + 
                    // ref_curr[ref_stride] + ref_curr[ref_stride+1] + 2) >>2
                    m1 = _mm_add_pi16 (m1, m2);
                    m1 = _mm_madd_pi16 (m1, m_one);
                    m1 = _mm_add_pi32 (m1, m_two);
                    m1 = _mm_srai_pi32 (m1, 2);

                    // m2 = words 4 5 6 7 of line 0 ref_curr
                    __m64 m3 = *(__m64 *)(ref_curr+4);
                    // m1 = words 4 5 6 7 of line 1 of ref_curr
                    m2 = *(__m64 *)(ref_curr+4+ref_stride);
                    // (ref_curr[0] + ref_curr[1] + 
                    // ref_curr[ref_stride] + ref_curr[ref_stride+1] + 2) >>2
                    m3 = _mm_add_pi16 (m3, m2);
                    m3 = _mm_madd_pi16 (m3, m_one);
                    m3 = _mm_add_pi32 (m3, m_two);
                    m3 = _mm_srai_pi32 (m3, 2);

                    m1 = _mm_packs_pi32 (m1, m3);

                    // load first four values pic_data and <<1
                    m2 = _mm_slli_pi16 (*(__m64 *)pic_curr, 1);
                    
                    // pic<<1 - ref
                    *(__m64 *)diff_curr = _mm_sub_pi16 (m2, m1);
                }
                // mopup;
                for (int x = stopX; x < width; ++x, ++pic_curr,++diff_curr, ref_curr+=2)
                {
                    CalcValueType temp = (ref_curr[0] + ref_curr[1] +
                        ref_curr[ref_stride] + ref_curr[ref_stride+1]+2)>>2;
                    *diff_curr = ((*pic_curr)<<1) - temp;
                }
            }
#else
            //std::cerr << "Inmmx routine rmdr.y == 0" << std::endl;
            CalcValueType sum(0);
            for( int y=0; y < height; ++y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=0; x < width; ++x, ++pic_curr, ++diff_curr, ref_curr+=2 )
                {
                    CalcValueType temp = ( CalcValueType( ref_curr[0] ) +
                                          CalcValueType( ref_curr[1] ) +
                                          CalcValueType( ref_curr[ref_stride] ) +
                                          CalcValueType( ref_curr[ref_stride+1] ) +
                                       2
                            ) >> 2;
                    *diff_curr = ((*pic_curr)<<1) - temp;
                }// x
                
            }// y
#endif
        }
    _mm_empty();
    return;
    }

    /* 
    * NOTE: we are not doing any bounds checks here. This function must
    * be invoked only when the reference images start and stop fall
    * withing bounds
    */
    CalcValueType simple_biblock_diff_up_mmx_4(
            const TwoDArray<ValueType>& diff_data, const PicArray& ref_data, 
            const ImageCoords& ref_start, const ImageCoords& ref_stop,
            const MVector& rmdr)
    {
        ValueType *diff_curr = &diff_data[0][0];
        ValueType *ref_curr = &ref_data[ref_start.y][ref_start.x];

        const int width = diff_data.LengthX();
        int height = diff_data.LengthY();
        const int ref_stride = ref_data.LengthX();

        // go down 2 rows and back up
        const int ref_next = ref_data.LengthX()*2 - width*2;

        REPORTM (ref_start.x>=0 && ref_stop.x < ref_data.LengthX() &&
                ref_start.y>=0 && ref_stop.y < ref_data.LengthY(), 
                "Reference image coordinates withing bounds");

        CalcValueType mop_sum(0);
        int stopX = (width>>2)<<2;
        __m64 m_sum = _mm_set_pi16(0, 0, 0, 0);
        u_mmx_val u_sum;
        if (rmdr.x == 0 && rmdr.y == 0 )
        {
            //std::cerr << "Inmmx routine rmdr.x = rmdr.y = 0" << std::endl;
#if 1
            for( int y=0; y < height; y++, ref_curr+=ref_next )
            {
                for( int x=0; x < stopX; x+=4, diff_curr+=4, ref_curr+=8 )
                {
                    u_mmx_val diff = *(u_mmx_val *)diff_curr;
                    __m64 ref = _mm_unpacklo_pi16 (*(__m64 *)ref_curr, *(__m64 *)(ref_curr+4));
                    __m64 ref2 = _mm_unpackhi_pi16 (*(__m64 *)ref_curr, *(__m64 *)(ref_curr+4));
                    ref = _mm_unpacklo_pi16 ( ref, ref2);
                    // diff - ref
                    diff.m = _mm_sub_pi16 (diff.m, ref);
                    // (diff - ref)>>1
                    diff.m = _mm_srai_pi16 (diff.m, 1);
                    // abs (diff - ref)
                    ref = _mm_srai_pi16(diff.m, 15);
                    diff.m = _mm_xor_si64(diff.m, ref);
                    diff.m = _mm_sub_pi16 (diff.m, ref);
                    // sum += abs(ref -pic)
                    ref = _mm_xor_si64(ref, ref);
                    ref = _mm_unpackhi_pi16(diff.m, ref);
                    diff.m = _mm_unpacklo_pi16(diff.m, diff.m);
                    diff.m = _mm_srai_pi32 (diff.m, 16);
                    diff.m = _mm_add_pi32 (diff.m, ref);
                    m_sum = _mm_add_pi32 (m_sum, diff.m);
                }
                // mopup;
                for (int x = stopX; x < width; ++x, ++diff_curr,ref_curr+=2)
                {
                    mop_sum += std::abs ((*diff_curr - *ref_curr)>>1);
                }
            }
            u_sum.m = m_sum;
            _mm_empty();
            return u_sum.i[0] + u_sum.i[1] + mop_sum;
#else
            CalcValueType sum(0);
            for( int y=0; y < height; ++y, ref_curr+=ref_next )
            {
                for( int x=0; x < width; ++x, ++diff_curr, ref_curr+=2 )
                {
                    sum += std::abs( (*diff_curr - *ref_curr)>>1 );
                }// x
                
            }// y
            return sum;
#endif

        }
        else if( rmdr.y == 0 )
        {
#if 1
            __m64 m_one = _mm_set_pi16(1, 1, 1, 1);
            for( int y=0; y < height; y++, ref_curr+=ref_next )
            {
                for( int x=0; x < stopX; x+=4, diff_curr+=4, ref_curr+=8 )
                {
                    // Load ref
                    __m64 m1 = _mm_unpacklo_pi16 (((u_mmx_val *)ref_curr)->m, ((u_mmx_val *)(ref_curr+4))->m);
                    __m64 m2 = _mm_unpackhi_pi16 (*(__m64 *)ref_curr, *(__m64 *)(ref_curr+4));
                    // m3 = words 0 2 4 6 of ref_curr
                    __m64 m3 = _mm_unpacklo_pi16 ( m1, m2);
                    // m2 = words 1 3 5 7 of ref_curr
                    m2 = _mm_unpackhi_pi16 ( m1, m2);
                    // (ref_curr[0] + ref_curr[1] + 1)>>1
                    m3 = _mm_add_pi16 (m3, m2);
                    m3 = _mm_add_pi16 (m3, m_one);
                    m3 = _mm_srai_pi16 (m3, 1);
                    // diff - pic
                    m1 = _mm_sub_pi16 (*(__m64 *)diff_curr, m3);
                    // (diff - pic)>>1
                    m1 = _mm_srai_pi16 (m1, 1);
                    // abs (diff-ref)>>1
                    m3 = _mm_srai_pi16(m1, 15);
                    m1 = _mm_xor_si64(m1, m3);
                    m1 = _mm_sub_pi16 (m1, m3);
                    // sum += abs(diff-ref)>>1
                    m2 = _mm_xor_si64(m2, m2);
                    m2 = _mm_unpackhi_pi16(m1, m2);
                    m1 = _mm_unpacklo_pi16(m1, m1);
                    m1 = _mm_srai_pi32 (m1, 16);
                    m1 = _mm_add_pi32 (m1, m2);
                    m_sum = _mm_add_pi32 (m_sum, m1);
                }
                // mopup;
                for (int x = stopX; x < width; ++x, ++diff_curr,ref_curr+=2)
                {
                    CalcValueType temp = (ref_curr[0] + ref_curr[1]+1)>>1;
                    mop_sum += std::abs ((*diff_curr - temp)>>1);
                }
            }
            u_sum.m = m_sum;
            _mm_empty();
            return (u_sum.i[0] + u_sum.i[1] + mop_sum);
#else
            //std::cerr << "Inmmx routine rmdr.y == 0" << std::endl;
            CalcValueType sum(0);
            for( int y=0; y < height; ++y, ref_curr+=ref_next )
            {
                for( int x=0; x < width; ++x, ++diff_curr, ref_curr+=2 )
                {
                    CalcValueType temp = (    CalcValueType( ref_curr[0] ) +
                                CalcValueType( ref_curr[1] ) +
                                1
                            ) >> 1;
                    sum += std::abs( (*diff_curr - temp)>>1 );
                }// x
            }// y
            return sum;
#endif
        }
        else if( rmdr.x == 0 )
        {
#if 1
            __m64 m_one = _mm_set_pi16(1, 1, 1, 1);
            for( int y=0; y < height; y++, ref_curr+=ref_next )
            {
                for( int x=0; x < stopX; x+=4, diff_curr+=4, ref_curr+=8 )
                {
                    // Load ref
                    __m64 m1 = _mm_unpacklo_pi16 (*(__m64 *)ref_curr, *(__m64 *)(ref_curr+4));
                    __m64 m2 = _mm_unpackhi_pi16 (*(__m64 *)ref_curr, *(__m64 *)(ref_curr+4));
                    // m1 = words 0 2 4 6 of ref_curr
                    m1 = _mm_unpacklo_pi16 ( m1, m2);
                    // m2 = words 0 2 4 6 of ref_curr+ref_stride
                    m2 = _mm_unpacklo_pi16 (*(__m64 *)(ref_curr+ref_stride), *(__m64 *)(ref_curr+ref_stride+4));
                    __m64 m3 = _mm_unpackhi_pi16 (*(__m64 *)(ref_curr+ref_stride), *(__m64 *)(ref_curr+ref_stride+4));
                    m2 = _mm_unpacklo_pi16 (m2, m3);
                    
                    // (ref_curr[0] + ref_curr[ref_stride] + 1)>>1
                    m1 = _mm_add_pi16 (m1, m2);
                    m1 = _mm_add_pi16 (m1, m_one);
                    m1 = _mm_srai_pi16 (m1, 1);
                    // diff - ref
                    m1 = _mm_sub_pi16 (*(__m64 *)diff_curr, m1);
                    // (diff - ref)>>1
                    m1 = _mm_srai_pi16 (m1, 1);
                    // abs ((diff - pic)>>1)
                    m3 = _mm_srai_pi16(m1, 15);
                    m1 = _mm_xor_si64(m1, m3);
                    m1 = _mm_sub_pi16 (m1, m3);
                    // sum += abs(ref -pic)
                    m2 = _mm_xor_si64(m2, m2);
                    m2 = _mm_unpackhi_pi16(m1, m2);
                    m1 = _mm_unpacklo_pi16(m1, m1);
                    m1 = _mm_srai_pi32 (m1, 16);
                    m1 = _mm_add_pi32 (m1, m2);
                    m_sum = _mm_add_pi32 (m_sum, m1);
                }
                // mopup;
                for (int x = stopX; x < width; ++x, ++diff_curr,ref_curr+=2)
                {
                    CalcValueType temp = (ref_curr[0] + ref_curr[ref_stride]+1)>>1;
                    mop_sum += std::abs ( (*diff_curr - temp)>>1 );
                }
            }
            u_sum.m = m_sum;
            _mm_empty();
            return (u_sum.i[0] + u_sum.i[1] + mop_sum);
#else
            CalcValueType sum(0);
            for( int y=0; y < height; ++y, ref_curr+=ref_next )
            {
                for( int x=0; x < width; ++x, ++diff_curr, ref_curr+=2 )
                {
                    CalcValueType temp = (ref_curr[0] + ref_curr[ref_stride]+1)>>1;
                    sum += std::abs ( (*diff_curr - temp)>>1 );
                }// x
            }// y
            return sum;
#endif
        }
        else
        {
#if 1
            __m64 m_two = _mm_set_pi32(2, 2);
            __m64 m_one = _mm_set_pi16(1, 1, 1, 1);
            // processing four pic_data values at a time
            for( int y=0; y < height; y++, ref_curr+=ref_next )
            {
                for( int x=0; x < stopX; x+=4, diff_curr+=4, ref_curr+=8 )
                {
                    // Load ref
                    // m1 = words 0 1 2 3 of line 0 ref_curr
                    __m64 m1 = *(__m64 *)ref_curr;
                    // m1 = words 0 1 2 3 of line 1 of ref_curr
                    __m64 m2 = *(__m64 *)(ref_curr+ref_stride);
                    // (ref_curr[0] + ref_curr[1] + 
                    // ref_curr[ref_stride] + ref_curr[ref_stride+1] + 2) >>2
                    m1 = _mm_add_pi16 (m1, m2);
                    m1 = _mm_madd_pi16 (m1, m_one);
                    m1 = _mm_add_pi32 (m1, m_two);
                    m1 = _mm_srai_pi32 (m1, 2);

                    // m2 = words 4 5 6 7 of line 0 ref_curr
                    __m64 m3 = *(__m64 *)(ref_curr+4);
                    // m1 = words 4 5 6 7 of line 1 of ref_curr
                    m2 = *(__m64 *)(ref_curr+4+ref_stride);
                    // (ref_curr[0] + ref_curr[1] + 
                    // ref_curr[ref_stride] + ref_curr[ref_stride+1] + 2) >>2
                    m3 = _mm_add_pi16 (m3, m2);
                    m3 = _mm_madd_pi16 (m3, m_one);
                    m3 = _mm_add_pi32 (m3, m_two);
                    m3 = _mm_srai_pi32 (m3, 2);
                    m1 = _mm_packs_pi32 (m1, m3);

                    // load first four values pic_data
                    m2 = *(__m64 *)diff_curr;
                    
                    // diff - ref
                    m1 = _mm_sub_pi16 (m2, m1);
                    // (diff - ref)>>1
                    m1 = _mm_srai_pi16 (m1, 1);
                    // abs (diff - ref)>>1
                    m2 = _mm_srai_pi16(m1, 15);
                    m1 = _mm_xor_si64(m1, m2);
                    m1 = _mm_sub_pi16(m1, m2);
                    // sum += abs(ref -pic)>>1
                    m1 = _mm_madd_pi16(m1, m_one);
                    m_sum = _mm_add_pi32 (m_sum, m1);
                }
                // mopup;
                for (int x = stopX; x < width; ++x, ++diff_curr,ref_curr+=2)
                {
                    CalcValueType temp = (ref_curr[0] + ref_curr[1] +
                        ref_curr[ref_stride] + ref_curr[ref_stride+1]+2)>>2;
                    mop_sum += std::abs ( (*diff_curr - temp)>>1 );
                }
            }
            u_sum.m = m_sum;
            _mm_empty();
            return (u_sum.i[0] + u_sum.i[1] + mop_sum);
#else
            CalcValueType sum(0);
            for( int y=0; y < height; ++y, ref_curr+=ref_next )
            {
                for( int x=0; x < width; ++x, ++diff_curr, ref_curr+=2 )
                {
                    CalcValueType temp = ( CalcValueType( ref_curr[0] ) +
                                          CalcValueType( ref_curr[1] ) +
                                          CalcValueType( ref_curr[ref_stride] ) +
                                          CalcValueType( ref_curr[ref_stride+1] ) +
                                       2
                            ) >> 2;
                    sum += std::abs( (*diff_curr - temp)>>1 );
                }// x
            }// y
            return sum;
#endif
        }
    return 0;
    }

    inline void check_active_columns(
            int x, int xmax, ValueType act_cols1[4],ValueType *row1)
    {
        // check if we need any clipping
        if (x >= 0 && (x+3) < xmax) {
            // special case, nothing to do
            memcpy(act_cols1, &row1[x], 4 * sizeof(ValueType));
        }
        else if (x < 0)
        {
            act_cols1[0] = row1[0];
            //act_cols1[1] = (x + 1) < 0 ? row1[0] : row1[x+1];
            //act_cols1[2] = (x + 2) < 0 ? row1[0] : row1[x+2];
            //act_cols1[3] = (x + 3) < 0 ? row1[0] : row1[x+3];
            for (int i = 1; i < 4; ++i)
            {
                act_cols1[i] = (x + i) < 0 ? row1[0] : row1[x+i];
            } 
        }
        else
        {
            for (int i = 0; i < 3; ++i)
            {
                act_cols1[i] = (x + i) < xmax ? row1[x+i] : row1[xmax-1];
            } 
            act_cols1[3] =  row1[xmax-1];
        }
    }
    
    CalcValueType bchk_simple_block_diff_mmx_4 ( 
            const BlockDiffParams& dparams, const MVector& mv, 
            const PicArray& pic_data, const PicArray& ref_data,
            CalcValueType i_best_sum)
    {
        u_mmx_val u_sum;
        u_mmx_val u_ref;
        u_sum.i[0] = u_sum.i[1]= 0;
    
        ValueType *src = &(pic_data[dparams.Yp()][dparams.Xp()]);
        ImageCoords ref_start(dparams.Xp()+mv.x, dparams.Yp()+mv.y);

        int height = dparams.Yl();
        int width = dparams.Xl();
        int stopX = (width>>2)<<2;
        int pic_next = (pic_data.LengthX() - width);
        CalcValueType mop_sum = 0;
        for (int j = 0; j < height; j++)
        {
            for (int i = 0; i < stopX; i+=4) 
            {
                check_active_columns(ref_start.x+i, ref_data.LengthX(), u_ref.h, ref_data[BChk(ref_start.y+j, ref_data.LengthY())]);
                // pic - ref
                __m64 pic = _mm_sub_pi16 (*(__m64 *)src, u_ref.m);
                // abs (pic - ref)
                u_ref.m = _mm_srai_pi16(pic, 15);
                pic = _mm_xor_si64(pic, u_ref.m);
                pic = _mm_sub_pi16 (pic, u_ref.m);
                // sum += abs(pic -ref)
                u_ref.m = _mm_xor_si64(u_ref.m, u_ref.m);
                u_ref.m = _mm_unpackhi_pi16(pic, u_ref.m);
                pic = _mm_unpacklo_pi16(pic, pic);
                pic = _mm_srai_pi32 (pic, 16);
                pic = _mm_add_pi32 (pic, u_ref.m);
                u_sum.m = _mm_add_pi32 (u_sum.m, pic);
                src += 4;
            }
            for (int i = stopX; i < width; i++)
            {
                mop_sum += std::abs(*src - 
                    ref_data[BChk(j+ref_start.y , ref_data.LengthY())][BChk(i+ref_start.x , ref_data.LengthX())]);
                src++;
            }
            if ((u_sum.i[0] + u_sum.i[1] + mop_sum) >= i_best_sum)
            {
                _mm_empty();
                return i_best_sum;
            }
            src += pic_next;
        }
        _mm_empty();

        return  u_sum.i[0] + u_sum.i[1] + mop_sum;
    }
}
#endif
