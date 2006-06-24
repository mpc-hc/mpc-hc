/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: me_utils.cpp 280 2005-01-30 05:11:46Z gabest $ $Name$
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
* Contributor(s): Thomas Davies (Original Author), Peter Meerwald (pmeerw@users.sourceforge.net)
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

///////////////////////////////////
//-------------------------------//
//utilities for motion estimation//
//-------------------------------//
///////////////////////////////////

#include <libdirac_motionest/me_utils.h>
#include <libdirac_common/common.h>
using namespace dirac;

#include <algorithm>

void BlockDiffParams::SetBlockLimits( const OLBParams& bparams ,
                                      const PicArray& pic_data , 
                                      const int xbpos , const int ybpos)
{
    const int loc_xp = xbpos * bparams.Xbsep() - bparams.Xoffset();
    const int loc_yp = ybpos * bparams.Ybsep() - bparams.Yoffset();

    m_xp=std::max( loc_xp , 0 );
    m_yp=std::max( loc_yp , 0 );

    m_xl = bparams.Xblen() - m_xp + loc_xp;
    m_yl = bparams.Yblen() - m_yp + loc_yp;

     //constrain block lengths to fall within the picture
    m_xl = ( ( m_xp + m_xl - 1) > pic_data.LastX() ) ? ( pic_data.LastX() + 1 - m_xp ): m_xl;
    m_yl = ( ( m_yp + m_yl - 1) > pic_data.LastY() ) ? ( pic_data.LastY() + 1 - m_yp ) : m_yl;

}

// Block difference class functions

// Constructors ...

BlockDiff::BlockDiff(const PicArray& ref,const PicArray& pic) :
    pic_data( pic ),
    ref_data( ref )
{}

SimpleBlockDiff::SimpleBlockDiff( const PicArray& ref , const PicArray& pic ) :
    BlockDiff( ref , pic )
{}

BChkBlockDiff::BChkBlockDiff( const PicArray& ref , const PicArray& pic ) :
    BlockDiff( ref , pic )
{}    

IntraBlockDiff::IntraBlockDiff( const PicArray& pic ) :
    pic_data( pic )
{}

BiBlockDiff::BiBlockDiff( const PicArray& ref1 , const PicArray& ref2 ,
                          const PicArray& pic) :
    pic_data( pic ),
    ref_data1( ref1 ),
    ref_data2( ref2 )
{}

BiSimpleBlockDiff::BiSimpleBlockDiff( const PicArray& ref1 , const PicArray& ref2 ,
                                      const PicArray& pic) :
    BiBlockDiff(ref1 , ref2 , pic)
{}

BiBChkBlockDiff::BiBChkBlockDiff( const PicArray& ref1 , const PicArray& ref2 ,
                                  const PicArray& pic ) :
    BiBlockDiff(ref1 , ref2 , pic)
{}

BlockDiffUp::BlockDiffUp( const PicArray& ref , const PicArray& pic):
    BlockDiff( ref , pic )
{}

SimpleBlockDiffUp::SimpleBlockDiffUp( const PicArray& ref , const PicArray& pic ) :
    BlockDiffUp( ref , pic )
{}

BChkBlockDiffUp::BChkBlockDiffUp(const PicArray& ref,const PicArray& pic) :
    BlockDiffUp( ref , pic )
{}

BiBlockDiffUp::BiBlockDiffUp( const PicArray& ref1 , const PicArray& ref2 , 
                              const PicArray& pic) :
    BiBlockDiff( ref1 , ref2 , pic )
{}

BiSimpleBlockDiffUp::BiSimpleBlockDiffUp( const PicArray& ref1 , const PicArray& ref2 ,
                                          const PicArray& pic ):
    BiBlockDiffUp( ref1 , ref2 , pic)
{}

BiBChkBlockDiffUp::BiBChkBlockDiffUp( const PicArray& ref1 , const PicArray& ref2 , 
                                      const PicArray& pic ) :
    BiBlockDiffUp( ref1 , ref2 , pic)
{}

// Difference functions ...

float SimpleBlockDiff::Diff( const BlockDiffParams& dparams, const MVector& mv )
{

    ValueType diff;    

    CalcValueType sum( 0 );

    for (int j=dparams.Yp() ; j != dparams.Yp()+dparams.Yl() ; ++j )
    {
        for(int i=dparams.Xp() ; i!= dparams.Xp()+dparams.Xl() ; ++i )
        {
            diff = pic_data[j][i]-ref_data[j+mv.y][i+mv.x];
            sum += std::abs( diff );
        }// i, k
    }// j, l

    return static_cast<float>( sum );
}

float BChkBlockDiff::Diff( const BlockDiffParams& dparams, const MVector& mv )
{

    const int xmax = ref_data.LengthX();
    const int ymax = ref_data.LengthY();

    ValueType diff;

    CalcValueType sum( 0 );

    for ( int j=dparams.Yp() ; j!=dparams.Yp()+dparams.Yl() ; ++j )
    {
        for( int i=dparams.Xp() ; i!=dparams.Xp()+dparams.Xl() ; ++i )
        {
            diff = pic_data[j][i] - ref_data[BChk(j+mv.y , ymax)][BChk(i+mv.x , xmax)];
            sum += std::abs( diff );

        }// i
    }// j
    
    return static_cast<float>( sum );
}

float IntraBlockDiff::Diff( const BlockDiffParams& dparams , ValueType& dc_val )
{

     //computes the cost if block is predicted by its dc component

    CalcValueType int_dc( 0 );

    for ( int j=dparams.Yp() ; j!=dparams.Yp()+dparams.Yl() ; ++j)
        for(int i=dparams.Xp(); i!=dparams.Xp()+dparams.Xl() ; ++i )
            int_dc += static_cast<int>( pic_data[j][i] );

    int_dc /= ( dparams.Xl() * dparams.Yl() );

    // Just give dc to 8-bit accuracy
    dc_val = static_cast<ValueType>( (int_dc+2)>>2 );

    // Now compute the resulting SAD
    ValueType dc( dc_val<<2 );
    CalcValueType intra_cost( 0 );

    for (int j=dparams.Yp(); j!=dparams.Yp()+dparams.Yl() ; ++j)
        for( int i=dparams.Xp() ; i!=dparams.Xp()+dparams.Xl() ;++i )
            intra_cost += std::abs( pic_data[j][i] - dc );
    
    return static_cast<float>( intra_cost );
}

float BiSimpleBlockDiff::Diff( const BlockDiffParams& dparams, const MVector& mv1,const MVector& mv2){

    CalcValueType sum( 0 );

    ValueType diff;

    for ( int j=dparams.Yp(); j!=dparams.Yp()+dparams.Yl(); ++j )
    {
        for( int i=dparams.Xp() ; i!=dparams.Xp()+dparams.Xl() ; ++i )
        {
            diff = pic_data[j][i]-( ( ref_data1[j+mv1.y][i+mv1.x] + 1 )>>1 );
            diff -= ( ( ref_data2[j+mv2.y][i+mv2.x] + 1 )>>1 );

            sum += std::abs( diff );
        }// i
    }// j

    return static_cast<float>( sum );
}

float BiBChkBlockDiff::Diff( const BlockDiffParams& dparams, const MVector& mv1,const MVector& mv2){

    ValueType diff;
    const int xmax1 = ref_data1.LengthX();
    const int ymax1 = ref_data1.LengthY();

    const int xmax2 = ref_data2.LengthX();
    const int ymax2 = ref_data2.LengthY();

    CalcValueType sum( 0 );

    for ( int j=dparams.Yp() ; j!=dparams.Yp() + dparams.Yl() ; ++j )
    {
        for( int i=dparams.Xp() ; i!=dparams.Xp() + dparams.Xl() ; ++i )
        {
            diff = pic_data[j][i]-( ( ref_data1[BChk(j+mv1.y , ymax1)][BChk(i+mv1.x , xmax1)] + 1 )>>1 );
            diff -= ( ( ref_data2[BChk(j+mv2.y , ymax2)][BChk(i+mv2.x , xmax2)] + 1 )>>1 );

            sum += std::abs( diff );       
        }// i
    }// j

    return static_cast<float>( sum );
}

float SimpleBlockDiffUp::Diff( const BlockDiffParams& dparams, const MVector& mv )
{

    //Coordinates in the image being written to
    const ImageCoords StartPos(dparams.Xp(),dparams.Yp());
    const ImageCoords EndPos(StartPos.x+dparams.Xl(),StartPos.y+dparams.Yl());

    //the rounded motion vectors, accurate to 1/2 pel
    //NB: bitshift rounds negative numbers DOWN, as required    
    const MVector roundvec(mv.x>>2,mv.y>>2);

    //remainder, giving 1/8 pel accuracy, needed for linear interp
    const MVector rmdr(mv.x-(roundvec.x<<2),mv.y-(roundvec.y<<2));

    //Set up the start point in the reference image.    
    const ImageCoords RefStart((StartPos.x<<1) + roundvec.x,(StartPos.y<<1) + roundvec.y);


    //weights for doing linear interpolation, calculated from the remainder values

    const ValueType TLweight((4-rmdr.x)*(4-rmdr.y));
    const ValueType TRweight(rmdr.x*(4-rmdr.y));
    const ValueType BLweight((4-rmdr.x)*rmdr.y);
    const ValueType BRweight(rmdr.x*rmdr.y);    

    CalcValueType sum( 0 );

    ValueType temp;    

    for(int c = StartPos.y, uY = RefStart.y; c < EndPos.y; ++c, uY += 2){
        for(int l = StartPos.x, uX = RefStart.x; l < EndPos.x; ++l, uX += 2){
            temp = (
                    TLweight * ref_data[uY][uX] +
                    TRweight * ref_data[uY][uX+1] +
                    BLweight * ref_data[uY+1][uX] +
                    BRweight * ref_data[uY+1][uX+1] +
                    8
                    )>>4;

            sum += std::abs( pic_data[c][l] - temp );
        }//l
    }//c

    return static_cast<float>( sum );    
}

float BChkBlockDiffUp::Diff(  const BlockDiffParams& dparams, const MVector& mv )
{

    //the picture sizes
    const int DoubleXdim=ref_data.LengthX();
    const int DoubleYdim=ref_data.LengthY();

    //Coordinates in the image being written to
    const ImageCoords StartPos(dparams.Xp(),dparams.Yp());
    const ImageCoords EndPos(StartPos.x+dparams.Xl(),StartPos.y+dparams.Yl());

    //the rounded motion vectors, accurate to 1/2 pel
    //NB: bitshift rounds negative numbers DOWN, as required    
    const MVector roundvec(mv.x>>2,mv.y>>2);

    //remainder, giving 1/8 pel accuracy, needed for linear interp
    const MVector rmdr(mv.x-(roundvec.x<<2),mv.y-(roundvec.y<<2));

    //Set up the start point in the reference image.    
    const ImageCoords RefStart((StartPos.x<<1) + roundvec.x,(StartPos.y<<1) + roundvec.y);


    //weights for doing linear interpolation, calculated from the remainder values

    const ValueType    TLweight((4-rmdr.x)*(4-rmdr.y));
    const ValueType    TRweight(rmdr.x*(4-rmdr.y));
    const ValueType    BLweight((4-rmdr.x)*rmdr.y);
    const ValueType    BRweight(rmdr.x*rmdr.y);    

    CalcValueType sum( 0 );

    ValueType temp;

    for(int c = StartPos.y, uY = RefStart.y; c < EndPos.y; ++c, uY += 2)
    {
        for(int l = StartPos.x, uX = RefStart.x; l < EndPos.x; ++l, uX += 2)
        {
            temp = (
                    TLweight * ref_data[BChk(uY,DoubleYdim)][BChk(uX,DoubleXdim)] +
                    TRweight * ref_data[BChk(uY,DoubleYdim)][BChk(uX+1,DoubleXdim)] +
                    BLweight * ref_data[BChk(uY+1,DoubleYdim)][BChk(uX,DoubleXdim)] +
                    BRweight * ref_data[BChk(uY+1,DoubleYdim)][BChk(uX+1,DoubleXdim)] +
                    8
                    )>>4;

            sum += ( std::abs( pic_data[c][l] - temp ) );
        }//l
    }//c    

    return static_cast<float>( sum );

}

float BiSimpleBlockDiffUp::Diff( const BlockDiffParams& dparams, const MVector& mv1, const MVector& mv2){

    //the start and end points in the current frame
    const ImageCoords StartPos(dparams.Xp(),dparams.Yp());//Coordinates in the current image
    const ImageCoords EndPos(StartPos.x+dparams.Xl(),StartPos.y+dparams.Yl());    

    //the motion vectors rounded to 1/2 pel accuracy
    const MVector roundvec1(mv1.x>>2,mv1.y>>2);
    const MVector roundvec2(mv2.x>>2,mv2.y>>2);

    //the remainders giving 1/8 pel accuracy    
    const MVector rmdr1(mv1.x-(roundvec1.x<<2),mv1.y-(roundvec1.y<<2));
    const MVector rmdr2(mv2.x-(roundvec2.x<<2),mv2.y-(roundvec2.y<<2));

    //the starting points of the reference blocks in the reference images, to 1/2 pel accuracy
    const ImageCoords RefStart1((StartPos.x<<1) + roundvec1.x,(StartPos.y<<1) + roundvec1.y);
    const ImageCoords RefStart2((StartPos.x<<1) + roundvec2.x,(StartPos.y<<1) + roundvec2.y);

    //weights for doing linear interpolation, calculated from the remainder values
    const ValueType    TLweight1((4-rmdr1.x)*(4-rmdr1.y));
    const ValueType    TRweight1(rmdr1.x*(4-rmdr1.y));
    const ValueType    BLweight1((4-rmdr1.x)*rmdr1.y);
    const ValueType    BRweight1(rmdr1.x*rmdr1.y);        

    const ValueType    TLweight2((4-rmdr2.x)*(4-rmdr2.y));
    const ValueType    TRweight2(rmdr2.x*(4-rmdr2.y));
    const ValueType    BLweight2((4-rmdr2.x)*rmdr2.y);
    const ValueType    BRweight2(rmdr2.x*rmdr2.y);        

    CalcValueType temp;

    CalcValueType sum( 0 );

    for(int c = StartPos.y, uY1 = RefStart1.y,uY2=RefStart2.y; c < EndPos.y; ++c, uY1 += 2,uY2 += 2){
        for(int l = StartPos.x, uX1 = RefStart1.x,uX2=RefStart2.x; l < EndPos.x; ++l, uX1 += 2, uX2 += 2){
            temp = (
                    TLweight1 * ref_data1[uY1][uX1] +
                    TRweight1 * ref_data1[uY1][uX1+1] +
                    BLweight1 * ref_data1[uY1+1][uX1] +
                    BRweight1 * ref_data1[uY1+1][uX1+1] +
                    16
                    )>>5;

            temp += (
                    TLweight2 * ref_data2[uY2][uX2] +
                    TRweight2 * ref_data2[uY2][uX2+1] +
                    BLweight2 * ref_data2[uY2+1][uX2] +
                    BRweight2 * ref_data2[uY2+1][uX2+1] +
                    16
                    )>>5;

            sum += std::abs( pic_data[c][l] - temp );
        }//l
    }//c    

    return static_cast<float>( sum );   
}

float BiBChkBlockDiffUp::Diff( const BlockDiffParams& dparams, const MVector& mv1, const MVector& mv2)
{

    //as above, but with bounds checking
    const int xmax1 = ref_data1.LengthX(); 
    const int ymax1 = ref_data1.LengthY();
    const int xmax2 = ref_data2.LengthX(); 
    const int ymax2 = ref_data2.LengthY();    

    //the start and end points in the current frame
    const ImageCoords StartPos(dparams.Xp(),dparams.Yp());//Coordinates in the current image
    const ImageCoords EndPos(StartPos.x+dparams.Xl(),StartPos.y+dparams.Yl());    

    //the motion vectors rounded to 1/2 pel accuracy
    const MVector roundvec1(mv1.x>>2,mv1.y>>2);
    const MVector roundvec2(mv2.x>>2,mv2.y>>2);

    //the remainders giving 1/8 pel accuracy    
    const MVector rmdr1(mv1.x-(roundvec1.x<<2),mv1.y-(roundvec1.y<<2));
    const MVector rmdr2(mv2.x-(roundvec2.x<<2),mv2.y-(roundvec2.y<<2));

    //the starting points of the reference blocks in the reference images, to 1/2 pel accuracy
    const ImageCoords RefStart1((StartPos.x<<1) + roundvec1.x,(StartPos.y<<1) + roundvec1.y);
    const ImageCoords RefStart2((StartPos.x<<1) + roundvec2.x,(StartPos.y<<1) + roundvec2.y);

    //weights for doing linear interpolation, calculated from the remainder values
    const ValueType TLweight1((4-rmdr1.x)*(4-rmdr1.y));
    const ValueType TRweight1(rmdr1.x*(4-rmdr1.y));
    const ValueType BLweight1((4-rmdr1.x)*rmdr1.y);
    const ValueType BRweight1(rmdr1.x*rmdr1.y);        

    const ValueType TLweight2((4-rmdr2.x)*(4-rmdr2.y));
    const ValueType TRweight2(rmdr2.x*(4-rmdr2.y));
    const ValueType BLweight2((4-rmdr2.x)*rmdr2.y);
    const ValueType BRweight2(rmdr2.x*rmdr2.y);        

    CalcValueType temp;

    CalcValueType sum( 0 );

    for(int c = StartPos.y, uY1 = RefStart1.y,uY2=RefStart2.y; c < EndPos.y; ++c, uY1 += 2,uY2 += 2)
    {
        for(int l = StartPos.x, uX1 = RefStart1.x,uX2=RefStart2.x; l < EndPos.x; ++l, uX1 += 2, uX2 += 2)
        {
            temp = (
                    TLweight1 * ref_data1[BChk(uY1,ymax1)][BChk(uX1,xmax1)] +
                    TRweight1 * ref_data1[BChk(uY1,ymax1)][BChk(uX1+1,xmax1)] +
                    BLweight1 * ref_data1[BChk(uY1+1,ymax1)][BChk(uX1,xmax1)] +
                    BRweight1 * ref_data1[BChk(uY1+1,ymax1)][BChk(uX1+1,xmax1)] +
                    16)>>5;

            temp += (
                    TLweight2 * ref_data2[BChk(uY2,ymax2)][BChk(uX2,xmax2)] +
                    TRweight2 * ref_data2[BChk(uY2,ymax2)][BChk(uX2+1,xmax2)] +
                    BLweight2 * ref_data2[BChk(uY2+1,ymax2)][BChk(uX2,xmax2)] +
                    BRweight2 * ref_data2[BChk(uY2+1,ymax2)][BChk(uX2+1,xmax2)]+
                    16)>>5;

            sum += std::abs( pic_data[c][l] - temp );
        }//l
    }//c    

    return static_cast<float>( sum );
}
