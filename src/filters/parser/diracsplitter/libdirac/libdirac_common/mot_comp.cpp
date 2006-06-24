/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: mot_comp.cpp 280 2005-01-30 05:11:46Z gabest $ $Name$
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
*                 Thomas Davies,
*                 Steve Bearcroft
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


#include <libdirac_common/mot_comp.h>
#include <libdirac_common/motion.h>
#include <libdirac_common/frame_buffer.h>
using namespace dirac;

using std::vector;

//--public member functions--//
///////////////////////////////


//Constructor
//Initialises the lookup table that is needed for 1/8th pixel accuracy
//motion estimation. Creates the necessary arithmetic objects and
//calls ReConfig to create weighting blocks to fit the values within
//m_cparams.
MotionCompensator::MotionCompensator( const CodecParams &cp , const AddOrSub direction ): 
    m_cparams(cp),
    luma_or_chroma(true),
    m_add_or_sub( direction )
{
    //Configure weighting blocks for the first time
    m_block_weights = NULL;
    m_half_block_weights = NULL;
    ReConfig();
}

//Destructor
MotionCompensator::~MotionCompensator(){

    //Tidy up the pointers
    delete[] m_block_weights;
    delete[] m_half_block_weights;
}

//Called to perform motion compensated addition/subtraction on an entire frame.
void MotionCompensator::CompensateFrame(FrameBuffer& my_buffer,int fnum,const MvData& mv_data)
{

     int ref1_idx,ref2_idx;    
     Frame& my_frame=my_buffer.GetFrame(fnum);
     const FrameSort& fsort=my_frame.GetFparams().FSort();

     m_cformat = my_frame.GetFparams().CFormat();

     if (fsort!=I_frame)
     {//we can motion compensate

         const vector<int>& refs=my_frame.GetFparams().Refs();
         if (refs.size()>0)
         {
             //extract the references
             ref1_idx=refs[0];
             if (refs.size()>1)
                 ref2_idx=refs[1];
             else
                 ref2_idx=refs[0];

             const Frame& ref1frame=my_buffer.GetFrame(ref1_idx);
             const Frame& ref2frame=my_buffer.GetFrame(ref2_idx);

             luma_or_chroma = true;                
             //now do all the components
             CompensateComponent( my_frame , ref1frame , ref2frame , mv_data , Y_COMP);

             if ( m_cformat != Yonly )
             {
                 luma_or_chroma = false;                
                 CompensateComponent( my_frame , ref1frame , ref2frame , mv_data , U_COMP);
                 CompensateComponent( my_frame , ref1frame , ref2frame , mv_data , V_COMP);
             }
         }
     }
}

//--private member functions--//
////////////////////////////////

//Needs to be called if the blocksize changes (and
//on startup). This method creates an array of weighting
//blocks that are used to acheive correctly overlapping
//blocks.
void MotionCompensator::ReConfig()
{
    if (luma_or_chroma)
        m_bparams = m_cparams.LumaBParams(2);
    else
        m_bparams = m_cparams.ChromaBParams(2);

    if(m_block_weights != NULL)
        delete[] m_block_weights;

    if(m_half_block_weights != NULL)
        delete[] m_half_block_weights;

    // Create new weights array.
    m_block_weights = new TwoDArray<CalcValueType>[9];
    m_half_block_weights = new TwoDArray<CalcValueType>[9];
    for(int i = 0; i < 9; i++)
    {
        m_block_weights[i].Resize(  m_bparams.Yblen() , m_bparams.Xblen() );
        m_half_block_weights[i].Resize(  m_bparams.Yblen() , m_bparams.Xblen() );
    }
    // We can create all nine weighting blocks by calculating values
    // for four blocks and mirroring them to generate the others.
    CreateBlock( m_bparams , false , false , m_half_block_weights[0] );
    CreateBlock( m_bparams , false , true , m_half_block_weights[3] );
    CreateBlock( m_bparams , true , false , m_half_block_weights[1] );
    CreateBlock( m_bparams , true , true , m_half_block_weights[4] );

    // Note order of flipping is important.    
    FlipX( m_half_block_weights[3] , m_bparams , m_half_block_weights[5] );
    FlipX( m_half_block_weights[0] , m_bparams , m_half_block_weights[2] );
    FlipY( m_half_block_weights[0] , m_bparams , m_half_block_weights[6] );
    FlipX( m_half_block_weights[6] , m_bparams , m_half_block_weights[8] );
    FlipY( m_half_block_weights[1] , m_bparams , m_half_block_weights[7] );

    for( int k = 0; k < 9; k++)
    {
        for ( int j =m_half_block_weights[k].FirstY(); j <= m_half_block_weights[k].LastY(); j++)
        {
            for ( int i =m_half_block_weights[k].FirstX(); i <= m_half_block_weights[k].LastX(); i++)
            {
                m_block_weights[k][j][i] = m_half_block_weights[k][j][i] << 1;
            }// i
        }// j
    }// k
}

void MotionCompensator::CompensateComponent(Frame& picframe, const Frame &ref1frame, const Frame& ref2frame,
    const MvData& mv_data,const CompSort cs)
{
    // Set up references to pictures and references
    PicArray& pic_data_out = picframe.Data( cs );

    const PicArray& ref1up = ref1frame.UpData( cs );
    const PicArray& ref2up = ref2frame.UpData( cs );

    // Set up another picture which will contain the MC data, which
    // we'll add or subtract to pic_data_out
    TwoDArray<CalcValueType> pic_data(pic_data_out.LengthY(), pic_data_out.LengthX() , 0);

    // Factors to compensate for subsampling of chroma
    int xscale_factor = 1;
    int yscale_factor = 1;

    if ( cs != Y_COMP )
    {
        if (m_cformat == format420)
        {
            xscale_factor = 2;
            yscale_factor = 2;
        }
        else if (m_cformat == format422)
        {
            xscale_factor = 2;
            yscale_factor = 1;
        }
        else if (m_cformat == format411)
        {
            xscale_factor = 4;
            yscale_factor = 1;
        }

    } 

    // Reference to the relevant DC array
    const TwoDArray<ValueType>& dcarray = mv_data.DC( cs );

    // Set up references to the vectors
    const int num_refs = picframe.GetFparams().Refs().size();
    const MvArray* mv_array1; 
    const MvArray* mv_array2;
    mv_array1 = &mv_data.Vectors(1);
    if (num_refs ==2 )
        mv_array2 = &mv_data.Vectors(2);
    else
        mv_array2 = &mv_data.Vectors(1);

    ReConfig();//set all the weighting blocks up    

    //Blocks are listed left to right, line by line.
    MVector mv1,mv2;
    PredMode block_mode;
    ValueType dc;

    //Coords of the top-left corner of a block
    ImageCoords pos;

    //Loop for each block in the output image.
    //The CompensateBlock function will use the image pointed to by ref1up
    //and add the compensated pixels to the image pointed to by pic_data.
    size_t wgt_idx;

    //Loop over all the block rows

    pos.y = -m_bparams.Yoffset();
    for(int yblock = 0; yblock < m_cparams.YNumBlocks(); ++yblock)
    {
        pos.x = -m_bparams.Xoffset();
        //loop over all the blocks in a row
        for(int xblock = 0 ; xblock < m_cparams.XNumBlocks(); ++xblock)
        {

            //Decide which weights to use.
            if((xblock != 0)&&(xblock < m_cparams.XNumBlocks() - 1))
            {
                if((yblock != 0)&&(yblock < m_cparams.YNumBlocks() - 1))    
                    wgt_idx = 4;
                else if(yblock == 0) 
                    wgt_idx = 1;
                else 
                    wgt_idx= 7;
            }
            else if(xblock == 0)
            {
                if((yblock != 0)&&(yblock < m_cparams.YNumBlocks() - 1))    
                    wgt_idx = 3;
                else if(yblock == 0) 
                    wgt_idx = 0;
                else 
                    wgt_idx = 6;
            }
            else
            {
                if((yblock != 0)&&(yblock < m_cparams.YNumBlocks() - 1))    
                    wgt_idx = 5;
                else if(yblock == 0) 
                    wgt_idx = 2;
                else 
                    wgt_idx = 8;
            }

            block_mode = mv_data.Mode()[yblock][xblock];
            mv1 = (*mv_array1)[yblock][xblock];
            mv1.x /= xscale_factor;
            mv1.y /= yscale_factor;

            mv2 = (*mv_array2)[yblock][xblock];
            mv2.x /= xscale_factor;
            mv2.y /= yscale_factor;


            dc = dcarray[yblock][xblock]<<2;// DC is only given 8 bits, 
                                            // so need to shift to get 10-bit data

            if(block_mode == REF1_ONLY)
            {
                CompensateBlock(pic_data, ref1up, mv1, pos, m_block_weights[wgt_idx]);
            }
            else if (block_mode == REF2_ONLY)
            {                
                CompensateBlock(pic_data, ref2up, mv2, pos, m_block_weights[wgt_idx]);
            }
            else if(block_mode == REF1AND2)
            {
                CompensateBlock(pic_data, ref1up, mv1, pos, m_half_block_weights[wgt_idx]);
                CompensateBlock(pic_data, ref2up, mv2, pos, m_half_block_weights[wgt_idx]);                    
            }
            else
            {//we have a DC block.
                DCBlock(pic_data, dc,pos, m_block_weights[wgt_idx]);
            }

            //Increment the block horizontal position
            pos.x += m_bparams.Xbsep();

        }//xblock

        //Increment the block vertical position
        pos.y += m_bparams.Ybsep();

    }//yblock

    if ( m_add_or_sub == SUBTRACT)
    {
        int x_end_data = std::min(pic_data.LastX(), m_cparams.XNumBlocks()*m_bparams.Xbsep() );
        int y_end_data = std::min(pic_data.LastY(), m_cparams.YNumBlocks()*m_bparams.Ybsep() );

        for ( int i =pic_data.FirstY(); i <= y_end_data; i++)
        {
            for ( int j =pic_data.FirstX(); j <= x_end_data; ++j)
            {
                pic_data_out[i][j] -= static_cast<ValueType>( (pic_data[i][j] + 1024) >> 11 );
            }
 
            // Okay, we've done all the actual blocks. Now if the picture is further padded
            // we need to set the padded values to zero beyond the last block in the row,
            // for all the picture lines in the block row. Need only do this when we're
            // subtracting.

            for (int j=( m_cparams.XNumBlocks()*m_bparams.Xbsep() ); j<pic_data.LengthX() ; ++j )
            {
                pic_data_out[i][j] = 0;
            }
        }
        // Finally, now we've done all the blocks, we must set all padded lines below 
        // the last row equal to 0, if we're subtracting
        for ( int y=m_cparams.YNumBlocks()*m_bparams.Ybsep() ; y<pic_data.LengthY() ; ++y )
        {
            for ( int x=0 ; x<pic_data.LengthX() ; ++x )
            {
                pic_data_out[y][x] = 0;
            }

        }
    }
    else
    {
        for ( int i =pic_data.FirstY(); i <= pic_data.LastY(); i++)
        {
            for ( int j =pic_data.FirstX(); j <= pic_data.LastX(); j++)
            {
                pic_data_out[i][j] += static_cast<ValueType>( (pic_data[i][j] + 1024) >> 11 );
            }
        }
    }
}

void MotionCompensator::CompensateBlock( TwoDArray<CalcValueType> &pic_data , const PicArray &refup_data , const MVector &mv , 
const ImageCoords& pos , const TwoDArray<CalcValueType>& wt_array )
{

    //Coordinates in the image being written to.
    const ImageCoords start_pos( std::max(pos.x,0) , std::max(pos.y,0) );
    const ImageCoords end_pos( std::min( pos.x + m_bparams.Xblen() , pic_data.LengthX() ) , 
                               std::min( pos.y + m_bparams.Yblen() , pic_data.LengthY() ) );

    //The difference between the desired start point
    //pos and the actual start point start_pos.
    const ImageCoords diff( start_pos.x - pos.x , start_pos.y - pos.y );

    //Set up the start point in the reference image by rounding the motion vector
    //NB: bit shift rounds negative values DOWN, as required
    const MVector roundvec( mv.x>>2 , mv.y>>2 );

    //Get the remainder after rounding. NB rmdr values always 0,1,2 or 3
    const MVector rmdr( mv.x - ( roundvec.x<<2 ) , mv.y - ( roundvec.y<<2 ) );

    //Where to start in the upconverted image
    const ImageCoords ref_start( ( start_pos.x<<1 ) + roundvec.x ,( start_pos.y<<1 ) + roundvec.y );

    //weights for doing linear interpolation, calculated from the remainder values
    const ValueType TLweight( (4 - rmdr.x) * (4 - rmdr.y) );
    const ValueType TRweight( rmdr.x * ( 4 - rmdr.y ) );
    const ValueType BLweight( ( 4 - rmdr.x ) * rmdr.y );
    const ValueType BRweight( rmdr.x * rmdr.y );

    //An additional stage to make sure the block to be copied does not fall outside
    //the reference image.
    const int refXlen = refup_data.LengthX();
    const int refYlen = refup_data.LengthY();
    bool do_bounds_checking = false;

    //Check if there are going to be any problems copying the block from
    //the upvconverted reference image.
    if( ref_start.x < 0 ) 
        do_bounds_checking = true;
    else if( ref_start.x + ((end_pos.x - start_pos.x)<<1 ) >= refXlen )
        do_bounds_checking = true;
    if( ref_start.y < 0 ) 
        do_bounds_checking = true;
    else if( ref_start.y + ((end_pos.y - start_pos.y)<<1 ) >= refYlen)
        do_bounds_checking = true;

     if( !do_bounds_checking )
     {
         for(int c = start_pos.y, wY = diff.y, uY = ref_start.y; c < end_pos.y; ++c, ++wY, uY += 2)
         {
             for(int l = start_pos.x, wX = diff.x, uX = ref_start.x; l < end_pos.x; ++l, ++wX, uX += 2)
             {

                 pic_data[c][l] += (( TLweight * refup_data[uY][uX] +
                          TRweight * refup_data[uY][uX+1] +
                          BLweight * refup_data[uY+1][uX] +
                          BRweight * refup_data[uY+1][uX+1] +
                          8
                          ) >> 4) * wt_array[wY][wX];
             }//l
         }//c
     }
     else
     {
         //We're doing bounds checking because we'll fall off the edge of the reference otherwise.

        for(int c = start_pos.y, wY = diff.y, uY = ref_start.y,BuY=BChk(uY,refYlen),BuY1=BChk(uY+1,refYlen);
            c < end_pos.y; ++c, ++wY, uY += 2,BuY=BChk(uY,refYlen),BuY1=BChk(uY+1,refYlen))
        {
            for(int l = start_pos.x, wX = diff.x, uX = ref_start.x,BuX=BChk(uX,refXlen),BuX1=BChk(uX+1,refXlen);
                l < end_pos.x; ++l, ++wX, uX += 2,BuX=BChk(uX,refXlen),BuX1=BChk(uX+1,refXlen))
            {

                pic_data[c][l] += (( TLweight * refup_data[BuY][BuX] +
                         TRweight * refup_data[BuY][BuX1]  +
                         BLweight * refup_data[BuY1][BuX]+
                         BRweight * refup_data[BuY1][BuX1] +
                         8
                         ) >> 4) * wt_array[wY][wX];
            }//l
        }//c

    }

}

void MotionCompensator::DCBlock( TwoDArray<CalcValueType> &pic_data ,const ValueType dc , const ImageCoords& pos ,
    const TwoDArray<CalcValueType>& wt_array)
{

    //Coordinates in the image being written to.
    const ImageCoords start_pos( std::max(0 , pos.x) , std::max(0 , pos.y) );
    const ImageCoords end_pos( std::min(pos.x + m_bparams.Xblen() , pic_data.LengthX() ) , 
                               std::min(pos.y + m_bparams.Yblen() , pic_data.LengthY() ) );

    //The difference between the desired start point
    //pos and the actual start point start_pos.
    const ImageCoords diff(start_pos.x - pos.x , start_pos.y - pos.y);

    //Quick process where we can just copy from the double size image.

    for(int c = start_pos.y, wY = diff.y; c < end_pos.y; ++c, ++wY)
    {
        for(int l = start_pos.x, wX = diff.x; l < end_pos.x; ++l, ++wX)
        {
            pic_data[c][l] += dc * wt_array[wY][wX];
        }
    }
}

//Overlapping blocks are acheived by applying a 2D raised cosine shape
//to them. This function facilitates the calculations
float MotionCompensator::RaisedCosine(float t, float B)
{
    if(std::abs(t)>(B+1.0)/2.0) 
        return 0.0f;
    else if(std::abs(t)<(1.0-B)/2.0) 
        return 1.0f;
    else 
        return( 0.5 * ( 1.0 + std::cos( 3.141592654 * ( std::abs(t)-(1.0-B)/2.0 )/B ) ) );
}

//Calculates a weighting block.
//bparams defines the block parameters so the relevant weighting arrays can be created.
//FullX and FullY refer to whether the weight should be adjusted for the edge of an image.
//eg. 1D Weighting shapes in x direction

//  FullX true        FullX false
//     ***           ********
//   *     *                  *
//  *       *                  *
//*           *                  *

void MotionCompensator::CreateBlock(const OLBParams &bparams, bool FullX, bool FullY, TwoDArray<CalcValueType>& WeightArray)
{
    // Create temporary arrays
    OneDArray<CalcValueType> HWts( WeightArray.LengthX() );
    OneDArray<CalcValueType> VWts( WeightArray.LengthY() );

    // Calculation variables
    float rolloffX = (float(bparams.Xblen()+1)/float(bparams.Xbsep())) - 1;
    float rolloffY = (float(bparams.Yblen()+1)/float(bparams.Ybsep())) - 1;
    float val;

    // Window in the x direction
    for(int x = 0; x < bparams.Xblen(); ++x)
    {
        val = (float(x) - (float(bparams.Xblen()-1)/2.0))/float(bparams.Xbsep());
        HWts[x] = static_cast<CalcValueType>( 32.0 * RaisedCosine(val,rolloffX) );
        HWts[x] = std::max( HWts[x] , 1 );
        HWts[x] = std::min( HWts[x] , 32 );
    }// x

    // Window in the y direction
    for(int y = 0; y < bparams.Yblen(); ++y)
    {
        val = (float(y) - (float(bparams.Yblen()-1)/2.0))/float(bparams.Ybsep());
        VWts[y] = static_cast<CalcValueType>( 32.0 * RaisedCosine(val,rolloffY) );
        VWts[y] = std::max( VWts[y] , 1 );
        VWts[y] = std::min( VWts[y] , 32 );
    }// y

    // Rationalise to avoid rounding errors
    for(int x = HWts.Last(); x > HWts.Last()-bparams.Xoffset(); --x)
    {
        if (HWts[x] + HWts[HWts.Last()-(x-bparams.Xbsep())] > 32)
            HWts[HWts.Last()-(x-bparams.Xbsep())] = 32-HWts[x];
            
        else if (HWts[x] + HWts[HWts.Last()-(x-bparams.Xbsep())] < 32)
            HWts[x] = 32-HWts[HWts.Last()-(x-bparams.Xbsep())];
    }// x 

    // Now reflect or pad, as appropriate
    if (!FullX)
    {
        for( int x = 0; x < (bparams.Xblen()>>1) ; ++x)
            HWts[x] = 32;
    }
    else
    {
        for( int x = 0; x < (bparams.Xblen()>>1); ++x)
            HWts[x] = HWts[HWts.Last()-x];
    }

    // Rationalise to avoid rounding errors
    for(int y = VWts.Last(); y > VWts.Last()-bparams.Yoffset(); --y)
    {
        if (VWts[y] + VWts[VWts.Last()-(y-bparams.Ybsep())] > 32)
            VWts[VWts.Last()-(y-bparams.Ybsep())] = 32-VWts[y];
        else if (VWts[y] + VWts[VWts.Last()-(y-bparams.Ybsep())] < 32)
            VWts[y] = 32-VWts[VWts.Last()-(y-bparams.Ybsep())];
    }// x 

    // Reflect or pad, as appropriate
    if (!FullY)
    {
        for( int y = 0 ; y < (bparams.Yblen()>>1); ++y)
            VWts[y] = 32;
    }
    else
    {
        for( int y = 0 ; y < (bparams.Yblen()>>1); ++y)
            VWts[y] = VWts[VWts.Last()-y];
    }

    for(int y = 0; y < bparams.Yblen(); ++y)
    {
        for(int x = 0; x < bparams.Xblen(); ++x)
        {
            WeightArray[y][x] = VWts[y] * HWts[x];
        }// x
    }// y

}

//Flips the values in an array in the x direction.
void MotionCompensator::FlipX(const TwoDArray<CalcValueType>& Original, const OLBParams &bparams, TwoDArray<CalcValueType>& Flipped)
{
    for(int x = 0; x < bparams.Xblen(); ++x)
    {
        for(int y = 0; y < bparams.Yblen(); ++y)
        {
            Flipped[y][x] = Original[y][(bparams.Xblen()-1) - x];
        }// y
    }// x
}

//Flips the values in an array in the y direction.
void MotionCompensator::FlipY(const TwoDArray<CalcValueType>& Original, const OLBParams &bparams, TwoDArray<CalcValueType>& Flipped)
{
    for(int x = 0; x < bparams.Xblen(); ++x)
    {
        for(int y = 0; y < bparams.Yblen(); ++y)
        {
            Flipped[y][x] = Original[(bparams.Yblen()-1) - y][x];
        }// y
    }// x
}
