/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: upconvert.cpp,v 1.10 2007/06/10 14:00:41 asuraparaju Exp $ $Name: Dirac_0_8_0 $
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
* Contributor(s): Richard Felton (Original Author), Thomas Davies
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

#include <libdirac_common/upconvert.h>
using namespace dirac;

#include <iostream>

#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define CLIP(x,min,max) MAX(MIN(x,max),min)

UpConverter::UpConverter (int min_val, int max_val, int orig_xlen, int orig_ylen) :
    m_min_val(min_val),
    m_max_val(max_val),
    m_orig_xl(orig_xlen),
    m_orig_yl(orig_ylen)
{}

#ifndef HAVE_MMX
//MMX version defined in upconver_mmx.cpp
//Up-convert by a factor of two.
void UpConverter::DoUpConverter(const PicArray& pic_data, PicArray& up_data)
{

    m_width_old = std::min (pic_data.LengthX(), m_orig_xl);
    m_height_old = std::min (pic_data.LengthY(), m_orig_yl);
    m_width_new = std::min(2*m_width_old, up_data.LengthX());
    m_height_new = std::min(2*m_height_old, up_data.LengthY());

    //Variables that will be used by the filter calculations
    int sum;
    int ypos(0);

    //There are three y loops to cope with the leading edge, middle 
    //and trailing edge of each column.

    for(int y = 0 ; y < m_filter_size; ++y , ypos += 2)
    {

        //We are filtering each column but doing it bit by bit.
        //This means our main loop is in the x direction and
        //there is a much greater chance the data we need will
        //be in the cache.
        for(int x = 0 , xpos = 0; x < m_width_old; x++ , xpos+=2 )
        {
            // Copy a Pixel from the original image in each even position
            up_data[ypos][xpos] = pic_data[y][x];

            //Work out the next pixel from filtered values.
            //Excuse the complicated ternary stuff but it sorts out the edge
            sum  = 1 << (m_filter_shift-1);
            sum += (pic_data[y][x]              + pic_data[y+1][x]) * m_tap0;
            sum += (pic_data[(y>=1)?(y-1):0][x] + pic_data[y+2][x]) * m_tap1;
            sum += (pic_data[(y>=2)?(y-2):0][x] + pic_data[y+3][x]) * m_tap2;
            sum += (pic_data[(y>=3)?(y-3):0][x] + pic_data[y+4][x]) * m_tap3;
            sum += (pic_data[(y>=4)?(y-4):0][x] + pic_data[y+5][x]) * m_tap4;

            sum >>= m_filter_shift;
            up_data[ypos+1][xpos] = CLIP(sum, m_min_val, m_max_val);
        }// x, xpos

        // The row loop.
        RowLoop( up_data , ypos );
    }// y, ypos
    // This loop is like the last one but it deals with the centre
    // section of the image and so the ternary operations are dropped
    // from the filter section.
    for(int y = m_filter_size; y < m_height_old - m_filter_size; ++y , ypos += 2)
    {
        for(int x = 0 , xpos=0; x < m_width_old; x++ , xpos+=2 )
        {
            up_data[ypos][xpos] = pic_data[y][x];

            sum  = 1 << (m_filter_shift-1);
            sum += (pic_data[y][x]   + pic_data[y+1][x]) * m_tap0;
            sum += (pic_data[y-1][x] + pic_data[y+2][x]) * m_tap1;
            sum += (pic_data[y-2][x] + pic_data[y+3][x]) * m_tap2;
            sum += (pic_data[y-3][x] + pic_data[y+4][x]) * m_tap3;
            sum += (pic_data[y-4][x] + pic_data[y+5][x]) * m_tap4;

            sum >>= m_filter_shift;
            up_data[ypos+1][xpos] = CLIP(sum, m_min_val, m_max_val);
        }// x,xpos
        RowLoop( up_data , ypos );

    }// y, ypos 
    // Another similar loop! - this time we are dealing with
    // the trailing edge so the ternary stuff is back in the
    // filter calcs but in the second parameter.    
    for(int y = m_height_old - m_filter_size; y < m_height_old; ++y , ypos+=2)
    {
        for(int x = 0 , xpos=0 ; x < m_width_old; x++ , xpos+=2)
        {
            up_data[ypos][xpos]=pic_data[y][x];

            sum  = 1 << (m_filter_shift-1);
            sum += (pic_data[y][x]     + pic_data[((y+1)<m_height_old)?(y+1):(m_height_old-1)][x]) * m_tap0;
            sum += (pic_data[y - 1][x] + pic_data[((y+2)<m_height_old)?(y+2):(m_height_old-1)][x]) * m_tap1;
            sum += (pic_data[y - 2][x] + pic_data[((y+3)<m_height_old)?(y+3):(m_height_old-1)][x]) * m_tap2;
            sum += (pic_data[y - 3][x] + pic_data[((y+4)<m_height_old)?(y+4):(m_height_old-1)][x]) * m_tap3;
            sum += (pic_data[y - 4][x] + pic_data[((y+5)<m_height_old)?(y+5):(m_height_old-1)][x]) * m_tap4;

            sum >>= m_filter_shift;
            up_data[ypos+1][xpos] = CLIP(sum, m_min_val, m_max_val);
        }//x,xpos
        RowLoop( up_data , ypos );

    }//y,ypos
}
#endif


void UpConverter::RowLoop(PicArray&up_data, const int row_num )
{
    const int dble_size( m_filter_size<<1 );

    //Calculation variable
    int sum;
    int ypos; 

    //Leading row Edge
    //Note the factor of two difference as we only want to fill in every other
    //line as the others have already been created
    for(int j = 0; j < 2; ++j)
    {
        ypos = row_num + j;

        for(int x = 0; x < dble_size ; x+=2)
        {
            sum  = 1 << (m_filter_shift-1);
            sum += (up_data[ypos][x]              + up_data[ypos][x+2]) * m_tap0;
            sum += (up_data[ypos][(x>=2)?(x-2):0] + up_data[ypos][x+4]) * m_tap1;
            sum += (up_data[ypos][(x>=4)?(x-4):0] + up_data[ypos][x+6]) * m_tap2;
            sum += (up_data[ypos][(x>=6)?(x-6):0] + up_data[ypos][x+8]) * m_tap3;
            sum += (up_data[ypos][(x>=8)?(x-8):0] + up_data[ypos][x+10]) * m_tap4;

            sum >>= m_filter_shift;
            up_data[ypos][x+1] = CLIP(sum, m_min_val, m_max_val);
        }// x
        //Middle of row
        for(int x = dble_size; x<m_width_new-dble_size ; x+=2 )
        {
            sum  = 1 << (m_filter_shift-1);
            sum += (up_data[ypos][x]   + up_data[ypos][x+2]) * m_tap0;
            sum += (up_data[ypos][x-2] + up_data[ypos][x+4]) * m_tap1;
            sum += (up_data[ypos][x-4] + up_data[ypos][x+6]) * m_tap2;
            sum += (up_data[ypos][x-6] + up_data[ypos][x+8]) * m_tap3;
            sum += (up_data[ypos][x-8] + up_data[ypos][x+10])* m_tap4;

            sum >>= m_filter_shift;
            up_data[ypos][x+1] = CLIP(sum, m_min_val, m_max_val);
        }// x
        //Trailing row edge
        for(int x = m_width_new - dble_size ; x<m_width_new ; x+=2)
        {
            sum  = 1 << (m_filter_shift-1);
            sum += (up_data[ypos][x]   + up_data[ypos][(((x+2)<m_width_new)?(x+2):(m_width_new-2))]) * m_tap0;
            sum += (up_data[ypos][x-2] + up_data[ypos][(((x+4)<m_width_new)?(x+4):(m_width_new-2))]) * m_tap1;
            sum += (up_data[ypos][x-4] + up_data[ypos][(((x+6)<m_width_new)?(x+6):(m_width_new-2))]) * m_tap2;
            sum += (up_data[ypos][x-6] + up_data[ypos][(((x+8)<m_width_new)?(x+8):(m_width_new-2))]) * m_tap3;
            sum += (up_data[ypos][x-8] + up_data[ypos][(((x+10)<m_width_new)?(x+10):(m_width_new-2))]) * m_tap4;

            sum >>= m_filter_shift;
            up_data[ypos][x+1] = CLIP(sum, m_min_val, m_max_val);
        }// x
    }
}
