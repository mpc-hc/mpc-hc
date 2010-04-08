/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: downconvert_mmx.cpp,v 1.2 2007/03/19 16:19:00 asuraparaju Exp $ $Name:  $
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

#include <libdirac_motionest/downconvert.h>
using namespace dirac;

#if defined (HAVE_MMX)
#include <mmintrin.h>

typedef union
{
    __m64 m;
    int i[2];
} u_sum;


#define mmx_add(pic1,pic2,tap,zero,sum1,sum2) \
    tmp = _mm_add_pi16 (*(__m64 *)pic1, *(__m64 *)pic2);    \
    m1 = _mm_unpacklo_pi16 ( tmp, zero);    \
    m2 = _mm_unpackhi_pi16 ( tmp, zero);    \
    m1 = _mm_madd_pi16 (m1, tap);    \
    m2 = _mm_madd_pi16 (m2, tap);    \
    *sum1 = _mm_add_pi32 (*sum1, m1);    \
    *sum2 = _mm_add_pi32 (*sum2, m2);    \
 
//General function - does some admin and calls the correct function
void DownConverter::DoDownConvert(const PicArray& old_data, PicArray& new_data)
{
    //Down-convert by a factor of two.
    m_row_buffer = new ValueType[old_data.LengthX()];
    //Variables that will be used by the filter calculations
    int sum;
    int colpos;

    // The area of the picture that will be downconverted
    const int xlen = 2 * new_data.LengthX();
    const int ylen = 2 * new_data.LengthY();


    //There are three y loops to cope with the leading edge, middle
    //and trailing edge of each column.
    colpos = 0;

    static __m64 zero = _mm_set_pi16(0, 0, 0, 0);
    static __m64 tap0 = _mm_set_pi16(0, StageI_I, 0, StageI_I);
    static __m64 tap1 = _mm_set_pi16(0, StageI_II, 0, StageI_II);
    static __m64 tap2 = _mm_set_pi16(0, StageI_III, 0, StageI_III);
    static __m64 tap3 = _mm_set_pi16(0, StageI_IV, 0, StageI_IV);
    static __m64 tap4 = _mm_set_pi16(0, StageI_V, 0, StageI_V);
    static __m64 tap5 = _mm_set_pi16(0, StageI_VI, 0, StageI_VI);
    static __m64 round = _mm_set_pi32(1 << (StageI_Shift - 1), 1 << (StageI_Shift - 1));

    u_sum sum1, sum2;
    __m64 tmp, m1, m2;

    int stopX = (xlen >> 2) << 2;
    for(int y = 0; y < Stage_I_Size * 2 ; y += 2 , colpos++)
    {
        // We are filtering each column but doing it bit by bit.
        // This means our main loop is in the x direction and
        // there is a much greater chance the data we need will
        // be in the cache.

        for(int x = 0 ; x < stopX ; x += 4)
        {
            // In down conversion we interpolate every pixel
            // so there is no copying.
            // Excuse the complicated ternary stuff but it sorts out the edge
            sum1.m = _mm_set_pi32(0, 0);
            sum2.m = _mm_set_pi32(0, 0);

            mmx_add(&old_data[y][x], &old_data[y+1][x], tap0, zero, &sum1.m, &sum2.m);
            mmx_add(&old_data[((y-1)>=0)?(y-1):0][x] , &old_data[y+2][x], tap1, zero, &sum1.m, &sum2.m);
            mmx_add(&old_data[((y-2)>=0)?(y-2):0][x] , &old_data[y+3][x], tap2, zero, &sum1.m, &sum2.m);
            mmx_add(&old_data[((y-3)>=0)?(y-3):0][x] , &old_data[y+4][x], tap3, zero, &sum1.m, &sum2.m);
            mmx_add(&old_data[((y-4)>=0)?(y-4):0][x] , &old_data[y+5][x], tap4, zero, &sum1.m, &sum2.m);
            mmx_add(&old_data[((y-5)>=0)?(y-5):0][x] , &old_data[y+6][x], tap5, zero, &sum1.m, &sum2.m);

            sum1.m = _mm_add_pi32(sum1.m, round);
            sum2.m = _mm_add_pi32(sum2.m, round);
            sum1.m = _mm_srai_pi32(sum1.m, StageI_Shift);
            sum2.m = _mm_srai_pi32(sum2.m, StageI_Shift);
            m_row_buffer[x] = sum1.i[0];
            m_row_buffer[x+1] = sum1.i[1];
            m_row_buffer[x+2] = sum2.i[0];
            m_row_buffer[x+3] = sum2.i[1];
        }// x
        _mm_empty();

        for(int x = stopX ; x < xlen ; x++)
        {
            // In down conversion we interpolate every pixel
            // so there is no copying.
            // Excuse the complicated ternary stuff but it sorts out the edge
            sum = (old_data[y][x] + old_data[y+1][x]) * StageI_I;
            sum += (old_data[((y-1)>=0)?(y-1):0][x] + old_data[y+2][x]) * StageI_II;
            sum += (old_data[((y-2)>=0)?(y-2):0][x] + old_data[y+3][x]) * StageI_III;
            sum += (old_data[((y-3)>=0)?(y-3):0][x] + old_data[y+4][x]) * StageI_IV;
            sum += (old_data[((y-4)>=0)?(y-4):0][x] + old_data[y+5][x]) * StageI_V;
            sum += (old_data[((y-5)>=0)?(y-5):0][x] + old_data[y+6][x]) * StageI_VI;
            sum += 1 << (StageI_Shift - 1); //do rounding right
            m_row_buffer[x] = sum >> StageI_Shift;
        }// x
        //Speaking of which - the row loop.

        RowLoop(colpos, new_data);
    }// y

    // This loop is like the last one but it deals with the center
    // section of the image and so the ternary operations are dropped
    // from the filter section.
    for(int y = Stage_I_Size * 2 ; y < ylen - Stage_I_Size * 2 ; y += 2 , colpos++)
    {
        for(int x = 0 ; x < stopX ; x += 4)
        {
            // In down conversion we interpolate every pixel
            // so there is no copying.
            // Excuse the complicated ternary stuff but it sorts out the edge
            sum1.m = _mm_set_pi32(0, 0);
            sum2.m = _mm_set_pi32(0, 0);

            mmx_add(&old_data[y][x], &old_data[y+1][x], tap0, zero, &sum1.m, &sum2.m);
            mmx_add(&old_data[y-1][x] , &old_data[y+2][x], tap1, zero, &sum1.m, &sum2.m);
            mmx_add(&old_data[y-2][x] , &old_data[y+3][x], tap2, zero, &sum1.m, &sum2.m);
            mmx_add(&old_data[y-3][x] , &old_data[y+4][x], tap3, zero, &sum1.m, &sum2.m);
            mmx_add(&old_data[y-4][x] , &old_data[y+5][x], tap4, zero, &sum1.m, &sum2.m);
            mmx_add(&old_data[y-5][x] , &old_data[y+6][x], tap5, zero, &sum1.m, &sum2.m);

            sum1.m = _mm_add_pi32(sum1.m, round);
            sum2.m = _mm_add_pi32(sum2.m, round);
            sum1.m = _mm_srai_pi32(sum1.m, StageI_Shift);
            sum2.m = _mm_srai_pi32(sum2.m, StageI_Shift);
            m_row_buffer[x] = sum1.i[0];
            m_row_buffer[x+1] = sum1.i[1];
            m_row_buffer[x+2] = sum2.i[0];
            m_row_buffer[x+3] = sum2.i[1];
        }// x
        _mm_empty();

        for(int x = stopX ; x < xlen ; x++)
        {
            sum = (old_data[y][x]   + old_data[y+1][x]) * StageI_I;
            sum += (old_data[y-1][x] + old_data[y+2][x]) * StageI_II;
            sum += (old_data[y-2][x] + old_data[y+3][x]) * StageI_III;
            sum += (old_data[y-3][x] + old_data[y+4][x]) * StageI_IV;
            sum += (old_data[y-4][x] + old_data[y+5][x]) * StageI_V;
            sum += (old_data[y-5][x] + old_data[y+6][x]) * StageI_VI;
            sum += 1 << (StageI_Shift - 1); //do rounding right
            m_row_buffer[x] = sum >> StageI_Shift;
        }// x

        RowLoop(colpos , new_data);
    }// y

    // Another similar loop! - this time we are dealing with
    // the trailing edge so the ternary stuff is back in the
    // filter calcs but in the second parameter.

    for(int y = ylen - (Stage_I_Size * 2) ; y < ylen - 1 ; y += 2 , colpos++)
    {
        for(int x = 0 ; x < stopX ; x += 4)
        {
            // In down conversion we interpolate every pixel
            // so there is no copying.
            // Excuse the complicated ternary stuff but it sorts out the edge
            sum1.m = _mm_set_pi32(0, 0);
            sum2.m = _mm_set_pi32(0, 0);

            mmx_add(&old_data[y][x], &old_data[((y+1)<ylen)?(y+1):(ylen-1)][x], tap0, zero, &sum1.m, &sum2.m);
            mmx_add(&old_data[y-1][x] , &old_data[((y+2)<ylen)?(y+2):(ylen-1)][x], tap1, zero, &sum1.m, &sum2.m);
            mmx_add(&old_data[y-2][x] , &old_data[((y+3)<ylen)?(y+3):(ylen-1)][x], tap2, zero, &sum1.m, &sum2.m);
            mmx_add(&old_data[y-3][x] , &old_data[((y+4)<ylen)?(y+4):(ylen-1)][x], tap3, zero, &sum1.m, &sum2.m);
            mmx_add(&old_data[y-4][x] , &old_data[((y+5)<ylen)?(y+5):(ylen-1)][x], tap4, zero, &sum1.m, &sum2.m);
            mmx_add(&old_data[y-5][x] , &old_data[((y+6)<ylen)?(y+6):(ylen-1)][x], tap5, zero, &sum1.m, &sum2.m);

            sum1.m = _mm_add_pi32(sum1.m, round);
            sum2.m = _mm_add_pi32(sum2.m, round);
            sum1.m = _mm_srai_pi32(sum1.m, StageI_Shift);
            sum2.m = _mm_srai_pi32(sum2.m, StageI_Shift);

            m_row_buffer[x] = sum1.i[0];
            m_row_buffer[x+1] = sum1.i[1];
            m_row_buffer[x+2] = sum2.i[0];
            m_row_buffer[x+3] = sum2.i[1];
        }// x
        _mm_empty();

        for(int x = stopX; x < xlen ; x++)
        {

            sum = (old_data[y][x]   + old_data[((y+1)<ylen)?(y+1):(ylen-1)][x]) * StageI_I;
            sum += (old_data[y-1][x] + old_data[((y+2)<ylen)?(y+2):(ylen-1)][x]) * StageI_II;
            sum += (old_data[y-2][x] + old_data[((y+3)<ylen)?(y+3):(ylen-1)][x]) * StageI_III;
            sum += (old_data[y-3][x] + old_data[((y+4)<ylen)?(y+4):(ylen-1)][x]) * StageI_IV;
            sum += (old_data[y-4][x] + old_data[((y+5)<ylen)?(y+5):(ylen-1)][x]) * StageI_V;
            sum += (old_data[y-5][x] + old_data[((y+6)<ylen)?(y+6):(ylen-1)][x]) * StageI_VI;

            // Do rounding right
            sum += 1 << (StageI_Shift - 1);
            m_row_buffer[x] = sum >> StageI_Shift;

        }// x

        RowLoop(colpos , new_data);

    }//  y

    // Tidy up the data
    delete[] m_row_buffer;

}
#endif
