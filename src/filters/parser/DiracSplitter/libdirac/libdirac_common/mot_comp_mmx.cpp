/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: mot_comp_mmx.cpp,v 1.9 2008/01/09 10:50:23 asuraparaju Exp $ $Name:  $
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
    if(x >= 0 && (x + 3) < xmax)
    {
        // special case, nothing to do
        memcpy(act_cols1, &row1[x], 4 * sizeof(ValueType));
        memcpy(act_cols2, &row2[x], 4 * sizeof(ValueType));
    }
    else
    {
        act_cols1[0] = row1[BChk(x, xmax)];
        act_cols2[0] = row2[BChk(x, xmax)];
        act_cols1[1] = row1[BChk(x+1, xmax)];
        act_cols2[1] = row2[BChk(x+1, xmax)];
        act_cols1[2] = row1[BChk(x+2, xmax)];
        act_cols2[2] = row2[BChk(x+2, xmax)];
        act_cols1[3] = row1[BChk(x+3, xmax)];
        act_cols2[3] = row2[BChk(x+3, xmax)];
    }
}

void MotionCompensator_QuarterPixel::BlockPixelPred(
    TwoDArray<ValueType> &block_data ,
    const ImageCoords& pos ,
    const ImageCoords& orig_pic_size ,
    const PicArray &refup_data ,
    const MVector &mv)
{
    // Set up the start point in the reference image by rounding the motion vector
    // to 1/2 pel accuracy.NB: bit shift rounds negative values DOWN, as required
    const MVector roundvec(mv.x >> 1 , mv.y >> 1);

    //Get the remainder after rounding. NB rmdr values always 0 or 1
    const MVector rmdr(mv.x & 1 , mv.y & 1);

    //Where to start in the upconverted image
    const ImageCoords start_pos(std::max(pos.x, 0) , std::max(pos.y, 0));
    // check that we are doing MC within true pic boundaries
    if(start_pos.x >= orig_pic_size.x || start_pos.y >= orig_pic_size.y)
        return;
    const ImageCoords ref_start((start_pos.x << 1) + roundvec.x , (start_pos.y << 1) + roundvec.y);

    //An additional stage to make sure the block to be copied does not fall outside
    //the reference image.
    const int refXlen = refup_data.LengthX();
    const int trueRefXlen = (orig_pic_size.x << 1) - 1;
    const int trueRefYlen = (orig_pic_size.y << 1) - 1;

    ValueType *block_curr = &block_data[0][0];

    bool do_bounds_checking = false;
    //Check if there are going to be any problems copying the block from
    //the upvconverted reference image.
    if(ref_start.x < 0)
        do_bounds_checking = true;
    else if(ref_start.x + (block_data.LengthX() << 1) >= trueRefXlen)
        do_bounds_checking = true;
    if(ref_start.y < 0)
        do_bounds_checking = true;
    else if(ref_start.y + (block_data.LengthY() << 1) >= trueRefYlen)
        do_bounds_checking = true;

    if(!do_bounds_checking)
    {
        int stopX = (block_data.LengthX() >> 2) << 2;
        ValueType *refup_curr = &refup_data[ref_start.y][ref_start.x];
        const int refup_next((refXlen - block_data.LengthX()) * 2);   //go down 2 rows and back to beginning of block line
        if(rmdr.x == 0 && rmdr.y == 0)
        {
            __m64 m1, m2;
            for(int y = 0; y < block_data.LengthY(); ++y, refup_curr += refup_next)
            {
                int x;
                for(x = 0; x < stopX; x += 4, block_curr += 4, refup_curr += 8)
                {
                    m1 = _mm_unpacklo_pi16(*(__m64 *)refup_curr, *(__m64 *)(refup_curr + 4));
                    m2 = _mm_unpackhi_pi16(*(__m64 *)refup_curr, *(__m64 *)(refup_curr + 4));
                    // *block_curr = refup_curr[0]
                    *(__m64 *)block_curr = _mm_unpacklo_pi16(m1, m2);
                }
                // Mopup the last value
                for(x = stopX ; x < block_data.LengthX(); ++x)
                {
                    *block_curr = *refup_curr;
                    ++block_curr;
                    refup_curr += 2;
                }
            }
            _mm_empty();
        }
        else if(rmdr.y == 0)
        {
            __m64 round = _mm_set_pi16(1, 1, 1, 1);
            __m64 m1, m2, m3;

            for(int y = 0; y < block_data.LengthY(); ++y, refup_curr += refup_next)
            {
                int x;
                for(x = 0; x < stopX; x += 4, block_curr += 4, refup_curr += 8)
                {
                    m1 = _mm_unpacklo_pi16(*(__m64 *)refup_curr, *(__m64 *)(refup_curr + 4));
                    m3 = _mm_unpackhi_pi16(*(__m64 *)refup_curr, *(__m64 *)(refup_curr + 4));
                    m2 = _mm_unpackhi_pi16(m1, m3);
                    m1 = _mm_unpacklo_pi16(m1, m3);

                    // (refup_curr[0] + refup_curr[1] + 1)>>1
                    m1 = _mm_add_pi16(m1, m2);
                    m1 = _mm_add_pi16(m1, round);
                    *(__m64 *)block_curr = _mm_srai_pi16(m1, 1);
                }

                // Mopup the last value
                for(x = stopX; x < block_data.LengthX(); ++x)
                {
                    *block_curr = ((*refup_curr  +
                                    *(refup_curr + 1)  + 1
                                   ) >> 1);
                    ++block_curr;
                    refup_curr += 2;
                }
            }
            _mm_empty();
        }
        else if(rmdr.x == 0)
        {
            __m64 round = _mm_set_pi16(1, 1, 1, 1);
            __m64 m1, m2, m3;
            for(int y = 0; y < block_data.LengthY(); ++y, refup_curr += refup_next)
            {
                int x;
                for(x = 0; x < stopX; x += 4, block_curr += 4, refup_curr += 8)
                {
                    m1 = _mm_unpacklo_pi16(*(__m64 *)refup_curr, *(__m64 *)(refup_curr + 4));
                    m2 = _mm_unpackhi_pi16(*(__m64 *)refup_curr, *(__m64 *)(refup_curr + 4));
                    // m1 now contains r00 r02 r04 r06
                    m1 = _mm_unpacklo_pi16(m1, m2);

                    m3 = _mm_unpacklo_pi16(*(__m64 *)(refup_curr + refXlen), *(__m64 *)(refup_curr + refXlen + 4));
                    m2 = _mm_unpackhi_pi16(*(__m64 *)(refup_curr + refXlen), *(__m64 *)(refup_curr + refXlen + 4));
                    // m1 now contains r10 r12 r14 r16
                    m2 = _mm_unpacklo_pi16(m3, m2);

                    // (refup_curr[0] + (refup_curr+refXlen)[0] + 1)>>1
                    m1 = _mm_add_pi16(m1, m2);
                    m1 = _mm_add_pi16(m1, round);
                    *(__m64 *)block_curr = _mm_srai_pi16(m1, 1);
                }
                for(x = stopX; x < block_data.LengthX(); ++x)
                {
                    *block_curr = ((*refup_curr + *(refup_curr + refXlen) +
                                    1
                                   ) >> 1);
                    ++block_curr;
                    refup_curr += 2;
                }
            }
            _mm_empty();
        }
        else
        {
            __m64 round = _mm_set_pi16(2, 2, 2, 2);
            __m64 m1, m2, m3;
            for(int y = 0; y < block_data.LengthY(); ++y, refup_curr += refup_next)
            {
                int x;
                for(x = 0; x < stopX; x += 4, block_curr += 4, refup_curr += 8)
                {
                    m1 = _mm_add_pi16(*(__m64 *)refup_curr, *(__m64 *)(refup_curr + refXlen));
                    m2 = _mm_add_pi16(*(__m64 *)(refup_curr + 4), *(__m64 *)(refup_curr + refXlen + 4));
                    m3 = _mm_unpacklo_pi16(m1, m2);
                    m1 = _mm_unpackhi_pi16(m1, m2);

                    m2 = _mm_unpackhi_pi16(m3, m1);
                    m1 = _mm_unpacklo_pi16(m3, m1);

                    m1 = _mm_add_pi16(m1, m2);
                    m1 = _mm_add_pi16(m1, round);
                    *(__m64 *)block_curr = _mm_srai_pi16(m1, 2);
                }
                for(x = stopX; x < block_data.LengthX(); ++x)
                {
                    *block_curr = ((*refup_curr  +
                                    *(refup_curr + 1)  +
                                    *(refup_curr + refXlen)  +
                                    *(refup_curr + refXlen + 1)  +
                                    2
                                   ) >> 2);
                    ++block_curr;
                    refup_curr += 2;
                }
            }
            _mm_empty();
        }
    }
    else
    {
        // We're 2doing bounds checking because we'll fall off the edge of the reference otherwise.

        //weights for doing linear interpolation, calculated from the remainder values
        const ValueType linear_wts[4] = {  (2 - rmdr.x) *(2 - rmdr.y),     //tl
                                           rmdr.x *(2 - rmdr.y),           //tr
                                           (2 - rmdr.x) * rmdr.y,          //bl
                                           rmdr.x * rmdr.y
                                        };              //br

        ValueType act_cols1[4], act_cols2[4];
        int uX, uY, c, l;
        for(c = 0, uY = ref_start.y; c < block_data.LengthY(); ++c, uY += 2)
        {
            for(l = 0, uX = ref_start.x; l < block_data.LengthX(); ++l, ++block_curr, uX += 2)
            {
                check_active_columns(uX, trueRefXlen, act_cols1, act_cols2, refup_data[BChk(uY, trueRefYlen)], refup_data[BChk(uY+1, trueRefYlen)]);

                *block_curr = ((linear_wts[0] * act_cols1[0] +
                                linear_wts[1] * act_cols1[1] +
                                linear_wts[2] * act_cols2[0] +
                                linear_wts[3] * act_cols2[1] +
                                2
                               ) >> 2);
            }//l
        }//c
    }
}

void MotionCompensator_HalfPixel::BlockPixelPred(
    TwoDArray<ValueType> &block_data ,
    const ImageCoords& pos ,
    const ImageCoords& orig_pic_size ,
    const PicArray &refup_data ,
    const MVector &mv)
{
    //Where to start in the upconverted image
    const ImageCoords start_pos(std::max(pos.x, 0) , std::max(pos.y, 0));
    const ImageCoords ref_start((start_pos.x << 1) + mv.x , (start_pos.y << 1) + mv.y);

    //An additional stage to make sure the block to be copied does not fall outside
    //the reference image.
    const int refXlen = refup_data.LengthX();
    //const int refYlen = refup_data.LengthY();
    const int trueRefXlen = (orig_pic_size.x << 1) - 1;
    const int trueRefYlen = (orig_pic_size.y << 1) - 1;

    bool do_bounds_checking = false;

    //Check if there are going to be any problems copying the block from
    //the upvconverted reference image.

    if(ref_start.x < 0)
        do_bounds_checking = true;
    else if(ref_start.x + ((block_data.LengthX() - 1) << 1) >= trueRefXlen)
        do_bounds_checking = true;
    if(ref_start.y < 0)
        do_bounds_checking = true;
    else if(ref_start.y + ((block_data.LengthY() - 1) << 1) >= trueRefYlen)
        do_bounds_checking = true;

    ValueType *block_curr = &block_data[0][0];

    if(!do_bounds_checking)
    {
        ValueType *refup_curr = &refup_data[ref_start.y][ref_start.x];
        const int refup_next((refXlen - block_data.LengthX()) * 2);// go down 2 rows and back up
#if 1
        int stopX = (block_data.LengthX() >> 2) << 2;
        {
            __m64 m1, m2;

            for(int y = 0; y < block_data.LengthY(); ++y, refup_curr += refup_next)
            {
                int x;
                for(x = 0; x < stopX; x += 4, block_curr += 4, refup_curr += 8)
                {
                    m1 = _mm_unpacklo_pi16(*(__m64 *)refup_curr, *(__m64 *)(refup_curr + 4));
                    m2 = _mm_unpackhi_pi16(*(__m64 *)refup_curr, *(__m64 *)(refup_curr + 4));
                    *(__m64 *)block_curr  = _mm_unpacklo_pi16(m1, m2);
                }
                // Mopup the last value
                for(x = stopX ; x < block_data.LengthX(); ++x)
                {
                    *block_curr = *refup_curr;
                    ++block_curr;
                    refup_curr += 2;
                }
            }
            _mm_empty();
        }
#else

        for(int y = 0; y < block_data.LengthY(); ++y, refup_curr += refup_next)
        {
            for(int x = 0; x < block_data.LengthX(); ++x, ++block_curr, refup_curr += 2)
            {
                *block_curr =  refup_curr[0];
            }
        }
#endif
    }
    else
    {
        // We're doing bounds checking because we'll fall off the edge of the reference otherwise.
        for(int y = 0, ry = ref_start.y, by = BChk(ry, trueRefYlen);
            y < block_data.LengthY();
            ++y, ry += 2, by = BChk(ry, trueRefYlen))
        {
            for(int x = 0 , rx = ref_start.x , bx = BChk(rx, trueRefXlen);
                x < block_data.LengthX() ;
                ++x, ++block_curr, rx += 2 , bx = BChk(rx, trueRefXlen))
            {
                *block_curr = refup_data[by][bx];
            }// x
        }// y
    }
}

void MotionCompensator::AdjustBlockBySpatialWeights(
    TwoDArray<ValueType>& val_block,
    const ImageCoords &pos,
    const TwoDArray<ValueType> &wt_array)
{
    ImageCoords start_pos(std::max(0, pos.x), std::max(0, pos.y));
    ImageCoords wt_start(start_pos.x - pos.x, start_pos.y - pos.y);

    ValueType *val_curr = &val_block[0][0];
    ValueType *wt_curr = &wt_array[wt_start.y][wt_start.x];

    // go down at row and back to beginning of weights line
    const int wt_next = wt_array.LengthX() - val_block.LengthX();

    const int stopX = (val_block.LengthX() >> 2) << 2;

    for(int j = 0; j < val_block.LengthY(); ++j, wt_curr += wt_next)
    {
        for(int i =  0; i < stopX; i += 4, val_curr += 4, wt_curr += 4)
        {
            /*
            * NOTE: Using only the low 16 bits of the result of multiplication
            * by weights because the result is supposed to fit in 16 bit
            * words. For some weights could result in overflow and errors
            */
            __m64 *out = (__m64 *)val_curr;
            *out = _mm_mullo_pi16(*(__m64 *)val_curr, *(__m64 *)wt_curr);
        }
        for(int i = stopX; i < val_block.LengthX(); ++i, ++val_curr, ++wt_curr)
        {
            *val_curr = *val_curr * *wt_curr;
        }
    }
    _mm_empty();
}

namespace dirac
{
void CompensateComponentAddAndShift_mmx(int start_y, int end_y,
                                        int weight_bits,
                                        const ImageCoords& orig_pic_size,
                                        TwoDArray<ValueType> &comp_data,
                                        PicArray &pic_data_out)
{
    if(start_y >= end_y)
        return;
    const int round_val = 1 << (weight_bits - 1);
    int stopX = pic_data_out.FirstX() + ((orig_pic_size.x >> 2) << 2);
    int x_end_truepic_data = pic_data_out.FirstX() + orig_pic_size.x;
    int x_end_data = pic_data_out.FirstX() + pic_data_out.LengthX();
    __m64 mround_val = _mm_set_pi16(round_val, round_val, round_val, round_val);
    ValueType *pic_row = &comp_data[0][comp_data.FirstX()];
    ValueType *out_row = &pic_data_out[start_y][pic_data_out.FirstX()];
    for(int i = start_y; i < end_y; i++)
    {
        for(int j =  pic_data_out.FirstX(); j < stopX; j += 4)
        {
            __m64 in1 = _mm_add_pi16(*(__m64 *)pic_row, mround_val);
            in1 = _mm_srai_pi16(in1, weight_bits);
            __m64 *out = (__m64 *)out_row;
            *out = _mm_add_pi16(in1, *out);
            pic_row += 4;
            out_row += 4;
        }
        for(int j = stopX; j < x_end_truepic_data; j++)
        {
            *out_row += static_cast<ValueType>((*pic_row + round_val) >> weight_bits);
            ++out_row;
            ++pic_row;
        }
        // Now pad past the true picture with the last true pic val in
        // current row
        ValueType last_true_val = *(out_row - 1);
        for(int j = x_end_truepic_data; j < x_end_data; ++j)
        {
            *out_row = last_true_val;
            ++out_row;
            ++pic_row;
        }
    }
    _mm_empty();
}

void AddMCBlock_mmx(const ImageCoords& start_pos,
                    TwoDArray<ValueType> &comp_strip,
                    TwoDArray<ValueType>& block_data)
{
    const int stopX = (block_data.LengthX() >> 2) << 2;

    const int comp_next = comp_strip.LengthX() - block_data.LengthX();
    ValueType *comp_curr = &comp_strip[start_pos.y][start_pos.x];
    ValueType *block_curr = &block_data[0][0];

    for(int j = 0; j < block_data.LengthY(); ++j, comp_curr += comp_next)
    {
        for(int i = 0; i < stopX; i += 4, comp_curr += 4, block_curr += 4)
        {
            __m64 *out = (__m64 *)comp_curr;
            // mc_tmp[y][x] += val
            *out = _mm_add_pi16(*(__m64 *)comp_curr, *(__m64 *)block_curr);
        }
        for(int i = stopX; i < block_data.LengthX(); ++i, ++comp_curr, ++block_curr)
        {
            *comp_curr += *block_curr;
        }
    }
    _mm_empty();
}
}
#endif
