/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: downconvert.cpp,v 1.10 2007/03/19 16:19:00 asuraparaju Exp $ $Name:  $
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
* Contributor(s): Richard Felton (Original Author), 
*                 Thomas Davies
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

DownConverter::DownConverter()
{}


#if !defined(HAVE_MMX)
//General function - does some admin and calls the correct function
//NOTE: The mmx version of this function is defined in downconvert_mmx.cpp
//Ensusre that changes made in this function are reflected in the mmx version
//as well.
void DownConverter::DoDownConvert(const PicArray& old_data, PicArray& new_data)
{
    //Down-convert by a factor of two.
    m_row_buffer= new ValueType[old_data.LengthX()];    
    //Variables that will be used by the filter calculations
    int sum;
    int colpos;

    // The area of the picture that will be downconverted
    const int xlen = 2*new_data.LengthX();    
    const int ylen = 2*new_data.LengthY();    


    //There are three y loops to cope with the leading edge, middle 
    //and trailing edge of each column.
    colpos=0;
    for( int y=0; y<Stage_I_Size*2 ; y+=2 , colpos++ )
    {
        // We are filtering each column but doing it bit by bit.
        // This means our main loop is in the x direction and
        // there is a much greater chance the data we need will
        // be in the cache.

        for( int x=0 ; x<xlen ; x++ )
        {            
            // In down conversion we interpolate every pixel
            // so there is no copying.
            // Excuse the complicated ternary stuff but it sorts out the edge
            sum =  (old_data[y][x] + old_data[y+1][x])*StageI_I;
            sum += (old_data[((y-1)>=0)?(y-1):0][x] + old_data[y+2][x])*StageI_II;
            sum += (old_data[((y-2)>=0)?(y-2):0][x] + old_data[y+3][x])*StageI_III;
            sum += (old_data[((y-3)>=0)?(y-3):0][x] + old_data[y+4][x])*StageI_IV;
            sum += (old_data[((y-4)>=0)?(y-4):0][x] + old_data[y+5][x])*StageI_V;
            sum += (old_data[((y-5)>=0)?(y-5):0][x] + old_data[y+6][x])*StageI_VI;
            sum += 1<<(StageI_Shift-1);//do rounding right
            m_row_buffer[x] = sum >> StageI_Shift;
        }// x
        //Speaking of which - the row loop.

        RowLoop(colpos,new_data);
    }// y 

    // This loop is like the last one but it deals with the center
    // section of the image and so the ternary operations are dropped
    // from the filter section.
    for( int y=Stage_I_Size*2 ; y<ylen-Stage_I_Size*2 ; y+=2 , colpos++ )
    {
        for( int x=0 ; x<xlen ; x++ )
        {

            sum =  (old_data[y][x]   + old_data[y+1][x])*StageI_I;
            sum += (old_data[y-1][x] + old_data[y+2][x])*StageI_II;
            sum += (old_data[y-2][x] + old_data[y+3][x])*StageI_III;
            sum += (old_data[y-3][x] + old_data[y+4][x])*StageI_IV;
            sum += (old_data[y-4][x] + old_data[y+5][x])*StageI_V;
            sum += (old_data[y-5][x] + old_data[y+6][x])*StageI_VI;
            sum += 1<<(StageI_Shift-1);//do rounding right
            m_row_buffer[x] = sum >> StageI_Shift;
        }// x

        RowLoop( colpos , new_data );
    }// y

    // Another similar loop! - this time we are dealing with
    // the trailing edge so the ternary stuff is back in the
    // filter calcs but in the second parameter.

    for( int y=ylen-(Stage_I_Size*2) ; y<ylen-1 ; y+=2 , colpos++ )
    {
        for( int x=0; x<xlen ; x++ )
        {

            sum =  (old_data[y][x]   + old_data[((y+1)<ylen)?(y+1):(ylen-1)][x])*StageI_I;
            sum += (old_data[y-1][x] + old_data[((y+2)<ylen)?(y+2):(ylen-1)][x])*StageI_II;
            sum += (old_data[y-2][x] + old_data[((y+3)<ylen)?(y+3):(ylen-1)][x])*StageI_III;
            sum += (old_data[y-3][x] + old_data[((y+4)<ylen)?(y+4):(ylen-1)][x])*StageI_IV;
            sum += (old_data[y-4][x] + old_data[((y+5)<ylen)?(y+5):(ylen-1)][x])*StageI_V;
            sum += (old_data[y-5][x] + old_data[((y+6)<ylen)?(y+6):(ylen-1)][x])*StageI_VI;

            // Do rounding right
            sum += 1<<(StageI_Shift-1);
            m_row_buffer[x] = sum >> StageI_Shift;

        }// x

        RowLoop( colpos , new_data );

    }//  y

    // Tidy up the data
    delete[] m_row_buffer;

}
#endif


// The loop over the columns is the same every time so lends itself to isolation
// as an individual function.
void DownConverter::RowLoop( const int colpos , PicArray& new_data)
{

     //Calculation variables
    int sum;
    const int xlen = 2*new_data.LengthX();
    int linepos=0;    

     // Leading Column Edge
     // Similar loops to the x case in ByHalf_opto, for explanation look there.
     // Note the factor of two difference as we only want to fill in every other
     // line as the others have already been created by the line loops.

    for( int x=0; x<(2*Stage_I_Size) ; x+=2 , linepos++ )
    {
        sum =  (m_row_buffer[((x)>=0)?(x):0]     + m_row_buffer[x+1])*StageI_I;
        sum += (m_row_buffer[((x-1)>=0)?(x-1):0] + m_row_buffer[x+2])*StageI_II;
        sum += (m_row_buffer[((x-2)>=0)?(x-2):0] + m_row_buffer[x+3])*StageI_III;
        sum += (m_row_buffer[((x-3)>=0)?(x-3):0] + m_row_buffer[x+4])*StageI_IV;
        sum += (m_row_buffer[((x-4)>=0)?(x-4):0] + m_row_buffer[x+5])*StageI_V;
        sum += (m_row_buffer[((x-5)>=0)?(x-5):0] + m_row_buffer[x+6])*StageI_VI;
        sum += 1<<(StageI_Shift-1);//do rounding right

        new_data[colpos][linepos] = sum >> StageI_Shift;

    }
     //Middle of column
    for( int x=(2*Stage_I_Size) ; x<xlen-(2*Stage_I_Size) ; x+=2 , linepos++) 
    {
        sum =  (m_row_buffer[x]   + m_row_buffer[x+1])*StageI_I;
        sum += (m_row_buffer[x-1] + m_row_buffer[x+2])*StageI_II;
        sum += (m_row_buffer[x-2] + m_row_buffer[x+3])*StageI_III;
        sum += (m_row_buffer[x-3] + m_row_buffer[x+4])*StageI_IV;
        sum += (m_row_buffer[x-4] + m_row_buffer[x+5])*StageI_V;
        sum += (m_row_buffer[x-5] + m_row_buffer[x+6])*StageI_VI;
        sum += 1<<(StageI_Shift-1);//do rounding right

        new_data[colpos][linepos] = sum >> StageI_Shift;

    }
     //Trailing column edge
    for( int x=xlen-(2*Stage_I_Size) ; x< xlen-1 ; x+=2 , linepos++ )
    {
        sum =  (m_row_buffer[x]   + m_row_buffer[((x+1)<xlen)?(x+1):(xlen-1)])*StageI_I;
        sum += (m_row_buffer[x-1] + m_row_buffer[((x+2)<xlen)?(x+2):(xlen-1)])*StageI_II;
        sum += (m_row_buffer[x-2] + m_row_buffer[((x+3)<xlen)?(x+3):(xlen-1)])*StageI_III;
        sum += (m_row_buffer[x-3] + m_row_buffer[((x+4)<xlen)?(x+4):(xlen-1)])*StageI_IV;
        sum += (m_row_buffer[x-4] + m_row_buffer[((x+5)<xlen)?(x+5):(xlen-1)])*StageI_V;
        sum += (m_row_buffer[x-5] + m_row_buffer[((x+6)<xlen)?(x+6):(xlen-1)])*StageI_VI;
        sum += 1<<(StageI_Shift-1);//do rounding right

        new_data[colpos][linepos] = sum >> StageI_Shift;

    }

}
