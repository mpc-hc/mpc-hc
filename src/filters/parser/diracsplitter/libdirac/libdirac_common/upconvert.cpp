/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: upconvert.cpp 280 2005-01-30 05:11:46Z gabest $ $Name$
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

//Up-convert by a factor of two.
void UpConverter::DoUpConverter(const PicArray& pic_data, PicArray& up_data)
{

    xOld = pic_data.LengthX();
    yOld = pic_data.LengthY();
    xNew = up_data.LengthX();
    yNew = up_data.LengthY();    //assumes up_data has twice the x and y length of the pic_data. 
                                //TBC: What to do if this is wrong?

    //Variables that will be used by the filter calculations
    int sum;
    int ypos(0);

    //There are three y loops to cope with the leading edge, middle 
    //and trailing edge of each column.

    for(int y = 0 ; y < Stage_I_Size; ++y , ypos += 2)
    {

        //We are filtering each column but doing it bit by bit.
        //This means our main loop is in the x direction and
        //there is a much greater chance the data we need will
        //be in the cache.
        for(int x = 0 , xpos = 0; x < xOld; x++ , xpos+=2 )
        {

            // Copy a Pixel from the original image in each even position
            up_data[ypos][xpos] = pic_data[y][x];

            //Work out the next pixel from filtered values.
            //Excuse the complicated ternary stuff but it sorts out the edge
            sum =  (pic_data[y][x] + pic_data[y+1][x])*StageI_I;
            sum += (pic_data[(y>=1)?(y-1):0][x] + pic_data[y+2][x])*StageI_II;
            sum += (pic_data[(y>=2)?(y-2):0][x] + pic_data[y+3][x])*StageI_III;
            sum += (pic_data[(y>=3)?(y-3):0][x] + pic_data[y+4][x])*StageI_IV;
            sum += (pic_data[(y>=4)?(y-4):0][x] + pic_data[y+5][x])*StageI_V;
            sum += (pic_data[(y>=5)?(y-5):0][x] + pic_data[y+6][x])*StageI_VI;

            up_data[ypos+1][xpos] = sum >> Stage_I_Shift;

        }// x, xpos

        // The row loop.
        RowLoop(up_data, ypos);
    }// y, ypos
    // This loop is like the last one but it deals with the centre
    // section of the image and so the ternary operations are dropped
    // from the filter section.
    for(int y = Stage_I_Size; y < yOld - Stage_I_Size; ++y , ypos += 2)
    {
        for(int x = 0 , xpos=0; x < xOld; x++ , xpos+=2 )
        {

            up_data[ypos][xpos] = pic_data[y][x];

            sum =  (pic_data[y][x]   + pic_data[y+1][x])*StageI_I;
            sum += (pic_data[y-1][x] + pic_data[y+2][x])*StageI_II;
            sum += (pic_data[y-2][x] + pic_data[y+3][x])*StageI_III;
            sum += (pic_data[y-3][x] + pic_data[y+4][x])*StageI_IV;
            sum += (pic_data[y-4][x] + pic_data[y+5][x])*StageI_V;
            sum += (pic_data[y-5][x] + pic_data[y+6][x])*StageI_VI;            

            up_data[ypos+1][xpos] = sum >> Stage_I_Shift;

        }// x,xpos
        RowLoop(up_data, ypos);

    }// y, ypos 
    // Another similar loop! - this time we are dealing with
    // the trailing edge so the ternary stuff is back in the
    // filter calcs but in the second parameter.    
    for(int y = yOld - Stage_I_Size; y < yOld; ++y , ypos+=2)
    {
        for(int x = 0 , xpos=0 ; x < xOld; x++ , xpos+=2)
        {

            up_data[ypos][xpos]=pic_data[y][x];

            sum =  (pic_data[y][x]     + pic_data[((y+1)<yOld)?(y+1):(yOld-1)][x])*StageI_I;
            sum += (pic_data[y - 1][x] + pic_data[((y+2)<yOld)?(y+2):(yOld-1)][x])*StageI_II;
            sum += (pic_data[y - 2][x] + pic_data[((y+3)<yOld)?(y+3):(yOld-1)][x])*StageI_III;
            sum += (pic_data[y - 3][x] + pic_data[((y+4)<yOld)?(y+4):(yOld-1)][x])*StageI_IV;
            sum += (pic_data[y - 4][x] + pic_data[((y+5)<yOld)?(y+5):(yOld-1)][x])*StageI_V;
            sum += (pic_data[y - 5][x] + pic_data[((y+6)<yOld)?(y+6):(yOld-1)][x])*StageI_VI;
            up_data[ypos+1][xpos] = sum >> Stage_I_Shift;

        }//x,xpos
        RowLoop(up_data, ypos);

    }//y,ypos
}


void UpConverter::RowLoop(PicArray&up_data, const int row_num)
{
    //Calculation variable
    int sum;
    int ypos; 

    //Leading row Edge
    //Note the factor of two difference as we only want to fill in every other
    //line as the others have already been created
    for(int i = 0; i < 2; ++i)
    {
        ypos = row_num + i;

        for(int x = 0; x < (2*Stage_I_Size); x+=2)
        {

            sum =  (up_data[ypos][x]     + up_data[ypos][x+2])*StageI_I;
            sum += (up_data[ypos][(x>=2)?(x-2):0] + up_data[ypos][x+4])*StageI_II;
            sum += (up_data[ypos][(x>=4)?(x-4):0] + up_data[ypos][x+6])*StageI_III;
            sum += (up_data[ypos][(x>=6)?(x-6):0] + up_data[ypos][x+8])*StageI_IV;
            sum += (up_data[ypos][(x>=8)?(x-8):0] + up_data[ypos][x+10])*StageI_V;
            sum += (up_data[ypos][(x>=10)?(x-10):0] + up_data[ypos][x+12])*StageI_VI;

            up_data[ypos][x+1] = sum >> Stage_I_Shift;
        }// x
        //Middle of row
        for(int x = (2*Stage_I_Size); x < xNew - (2*Stage_I_Size); x+=2)
        {
            sum =  (up_data[ypos][x]   + up_data[ypos][x+2])*StageI_I;
            sum += (up_data[ypos][x-2] + up_data[ypos][x+4])*StageI_II;
            sum += (up_data[ypos][x-4] + up_data[ypos][x+6])*StageI_III;
            sum += (up_data[ypos][x-6] + up_data[ypos][x+8])*StageI_IV;
            sum += (up_data[ypos][x-8] + up_data[ypos][x+10])*StageI_V;
            sum += (up_data[ypos][x-10] + up_data[ypos][x+12])*StageI_VI;

            up_data[ypos][x+1] = sum >> Stage_I_Shift;
        }// x
        //Trailing row edge
        for(int x = xNew - (2*Stage_I_Size); x < xNew; x+=2)
        {
            sum =  (up_data[ypos][x]   + up_data[ypos][(((x+2)<xNew)?(x+2):(xNew-2))])*StageI_I;
            sum += (up_data[ypos][x-2] + up_data[ypos][(((x+4)<xNew)?(x+4):(xNew-2))])*StageI_II;
            sum += (up_data[ypos][x-4] + up_data[ypos][(((x+6)<xNew)?(x+6):(xNew-2))])*StageI_III;
            sum += (up_data[ypos][x-6] + up_data[ypos][(((x+8)<xNew)?(x+8):(xNew-2))])*StageI_IV;
            sum += (up_data[ypos][x-8] + up_data[ypos][(((x+10)<xNew)?(x+10):(xNew-2))])*StageI_V;
            sum += (up_data[ypos][x-10] + up_data[ypos][(((x+12)<xNew)?(x+12):(xNew-2))])*StageI_VI;
            up_data[ypos][x+1] = sum >> Stage_I_Shift;
        }// x
    }
}
