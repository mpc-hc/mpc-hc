/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: me_utils.cpp,v 1.23 2008/10/21 04:55:46 asuraparaju Exp $ $Name:  $
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
* Contributor(s): Thomas Davies (Original Author), 
*                 Peter Meerwald (pmeerw@users.sourceforge.net)
*                 Steve Bearcroft (bearcrsw@users.sourceforge.net)
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

#if defined(HAVE_MMX)
#include <climits>
#endif
#include <libdirac_motionest/me_utils.h>
#include <libdirac_motionest/me_utils_mmx.h>
#include <libdirac_common/common.h>

using namespace dirac;

#include <algorithm>
//#define INTRA_HAAR

void BlockDiffParams::SetBlockLimits( const OLBParams& bparams ,
                                      const PicArray& m_pic_data , 
                                      const int xbpos , const int ybpos)
{
    const int loc_xp = xbpos * bparams.Xbsep() - bparams.Xoffset();
    const int loc_yp = ybpos * bparams.Ybsep() - bparams.Yoffset();

    m_xp=std::max( loc_xp , 0 );
    m_yp=std::max( loc_yp , 0 );

    m_xl = bparams.Xblen() - m_xp + loc_xp;
    m_yl = bparams.Yblen() - m_yp + loc_yp;

     //constrain block lengths to fall within the picture
    m_xl = ( ( m_xp + m_xl - 1) > m_pic_data.LastX() ) ? ( m_pic_data.LastX() + 1 - m_xp ): m_xl;
    m_yl = ( ( m_yp + m_yl - 1) > m_pic_data.LastY() ) ? ( m_pic_data.LastY() + 1 - m_yp ) : m_yl;

    m_xend = m_xp+m_xl;
    m_yend = m_yp+m_yl;

}

// Block difference class functions

// Constructors ...

BlockDiff::BlockDiff(const PicArray& ref,const PicArray& pic) :
    m_pic_data( pic ),
    m_ref_data( ref )
{}

PelBlockDiff::PelBlockDiff( const PicArray& ref , const PicArray& pic ) :
    BlockDiff( ref , pic )
{}

IntraBlockDiff::IntraBlockDiff( const PicArray& pic ) :
    m_pic_data( pic )
{}

BiBlockDiff::BiBlockDiff( const PicArray& ref1 , const PicArray& ref2 ,
                          const PicArray& pic) :
    m_pic_data( pic ),
    m_ref_data1( ref1 ),
    m_ref_data2( ref2 )
{}

BlockDiffUp::BlockDiffUp( const PicArray& ref , const PicArray& pic ):
    BlockDiff( ref , pic )
{}

BlockDiffHalfPel::BlockDiffHalfPel( const PicArray& ref , const PicArray& pic ) :
    BlockDiffUp( ref , pic )
{}

BlockDiffQuarterPel::BlockDiffQuarterPel( const PicArray& ref , const PicArray& pic ) :
    BlockDiffUp( ref , pic )
{}

BlockDiffEighthPel::BlockDiffEighthPel( const PicArray& ref , const PicArray& pic ) :
    BlockDiffUp( ref , pic )
{}

BiBlockHalfPel::BiBlockHalfPel( const PicArray& ref1 , const PicArray& ref2 ,
                                          const PicArray& pic ):
    BiBlockDiff( ref1 , ref2 , pic)
{}

BiBlockQuarterPel::BiBlockQuarterPel( const PicArray& ref1 , const PicArray& ref2 ,
                                          const PicArray& pic ):
    BiBlockDiff( ref1 , ref2 , pic)
{}

BiBlockEighthPel::BiBlockEighthPel( const PicArray& ref1 , const PicArray& ref2 ,
                                          const PicArray& pic ):
    BiBlockDiff( ref1 , ref2 , pic)
{}


// Difference functions ...

float PelBlockDiff::Diff( const BlockDiffParams& dparams, const MVector& mv )
{
    if (dparams.Xl() <= 0 || dparams.Yl() <= 0)
    {
        return 0;
    }

    CalcValueType sum( 0 );

    const ImageCoords ref_start( dparams.Xp()+mv.x , dparams.Yp()+mv.y );
    const ImageCoords ref_stop( dparams.Xend()+mv.x , dparams.Yend()+mv.y );
    
    bool bounds_check( false );

    if ( ref_start.x<0 || 
         ref_stop.x >= m_ref_data.LengthX() ||
         ref_start.y<0 || 
         ref_stop.y >= m_ref_data.LengthY() )
        bounds_check = true;

    if ( !bounds_check )
    {
#if defined(HAVE_MMX)
        return static_cast<float>(simple_block_diff_mmx_4(dparams, mv, m_pic_data, m_ref_data, INT_MAX));
#else
        ValueType diff;    
        for ( int j=dparams.Yp() ; j<dparams.Yp()+dparams.Yl() ; ++j )
        { 
            for(int i=dparams.Xp() ; i< dparams.Xp()+dparams.Xl() ; ++i )
            {
                diff = m_pic_data[j][i]-m_ref_data[j+mv.y][i+mv.x];
                sum += std::abs( diff );
            }// i
        }// j
#endif /* HAVE_MMX */
    }
    else
    {
#if defined (HAVE_MMX)
        return static_cast<float>(bchk_simple_block_diff_mmx_4(dparams, mv, m_pic_data, m_ref_data, INT_MAX));
#else
        ValueType diff;    
        for ( int j=dparams.Yp() ; j < dparams.Yp()+dparams.Yl() ; ++j )
        { 
            for( int i=dparams.Xp() ; i < dparams.Xp()+dparams.Xl() ; ++i )
            {
                diff = m_pic_data[j][i] - m_ref_data[BChk(j+mv.y , m_ref_data.LengthY())][BChk(i+mv.x , m_ref_data.LengthX())];
                sum += std::abs( diff );

            }// i
        }// j

#endif /* HAVE_MMX */
    }
    
    return static_cast<float>( sum );
}

void PelBlockDiff::Diff( const BlockDiffParams& dparams, 
                         const MVector& mv,
                         float& best_sum,
                         MVector& best_mv )
{
    if (dparams.Xl() <= 0 || dparams.Yl() <= 0)
    {
        return;
    }

    CalcValueType sum( 0 );

    const ImageCoords ref_start( dparams.Xp()+mv.x , dparams.Yp()+mv.y );
    const ImageCoords ref_stop( dparams.Xend()+mv.x , dparams.Yend()+mv.y );

    bool bounds_check( false );

    if ( ref_start.x<0 || 
         ref_stop.x >= m_ref_data.LengthX() ||
         ref_start.y<0 || 
         ref_stop.y >= m_ref_data.LengthY() )
        bounds_check = true;

    if ( !bounds_check )
    {
#if defined (HAVE_MMX)
        sum  = simple_block_diff_mmx_4(dparams, mv, m_pic_data, m_ref_data, static_cast<int>(best_sum));
        if (sum < best_sum)
        {
               best_sum = sum;
               best_mv = mv;
        }
        return; 
#else
        ValueType diff;    
        ValueType *pic_curr = &m_pic_data[dparams.Yp()][dparams.Xp()];
        const int pic_next( m_pic_data.LengthX() - dparams.Xl() ); // - go down a row and back along
    

        ValueType *ref_curr = &m_ref_data[ref_start.y][ref_start.x];

        for( int y=dparams.Yl(); y>0; --y, pic_curr+=pic_next, ref_curr+=pic_next )
        {
            for( int x=dparams.Xl(); x>0; --x, ++pic_curr, ++ref_curr )
            {
                diff = (*pic_curr)-(*ref_curr);
                sum += std::abs( diff );
            }// x

            if ( sum>=best_sum )
                return;

        }// y
#endif /* HAVE_MMX */
    }
    else
    {
#if defined (HAVE_MMX)
           sum = (bchk_simple_block_diff_mmx_4(dparams, mv, m_pic_data, m_ref_data, static_cast<int>(best_sum)));
        if (sum < best_sum)
        {
            best_sum = sum;
            best_mv = mv;
        }
        return;
#else
        ValueType diff;    
        for ( int j=dparams.Yp() ; j<dparams.Yend() ; ++j )
        { 
            for( int i=dparams.Xp() ; i<dparams.Xend() ; ++i )
            {
                diff = m_pic_data[j][i] - m_ref_data[BChk(j+mv.y , m_ref_data.LengthY())][BChk(i+mv.x , m_ref_data.LengthX())];
                sum += std::abs( diff );

            }// i

            if ( sum>=best_sum )
                return;

        }// j
#endif /* HAVE_MMX */
    }

    best_sum = sum;
    best_mv = mv;
    
}

ValueType IntraBlockDiff::CalcDC( const BlockDiffParams& dparams ){

    CalcValueType int_dc( 0 );
    if (dparams.Xl() <= 0 || dparams.Yl() <= 0)
    {
        return 0;
    }

    for ( int j=dparams.Yp() ; j<dparams.Yp()+dparams.Yl() ; ++j)
        for(int i=dparams.Xp(); i<dparams.Xp()+dparams.Xl() ; ++i )
            int_dc += static_cast<int>( m_pic_data[j][i] );

    int_dc /= ( dparams.Xl() * dparams.Yl() );

    return static_cast<ValueType>( int_dc );
}

#ifdef INTRA_HAAR
float IntraBlockDiff::Diff( const BlockDiffParams& dparams , ValueType& dc_val )
{
    if (dparams.Xl() <= 0 || dparams.Yl() <= 0)
    {
        dc_val = 0;
        return 0;
    }

    dc_val = CalcDC(dparams);

    // Now compute the resulting SAD
    ValueType dc( dc_val );
    CalcValueType intra_cost( 0 );

    for (int j=dparams.Yp(); j<dparams.Yend() ; j+=2){
        for( int i=dparams.Xp() ; i<dparams.Xend() ;i+=2 ){
            intra_cost += std::abs( m_pic_data[j][i]  
	                          + m_pic_data[j][i+1]
	                          + m_pic_data[j+1][i]
	                          + m_pic_data[j+1][i+1]
				  - 4*dc );
            intra_cost += std::abs( m_pic_data[j][i]  
	                          + m_pic_data[j][i+1]
	                          - m_pic_data[j+1][i]
	                          - m_pic_data[j+1][i+1] );
            intra_cost += std::abs( m_pic_data[j][i]  
	                          - m_pic_data[j][i+1]
	                          + m_pic_data[j+1][i]
	                          - m_pic_data[j+1][i+1] );
            intra_cost += std::abs( m_pic_data[j][i]  
	                          - m_pic_data[j][i+1]
	                          - m_pic_data[j+1][i]
	                          + m_pic_data[j+1][i+1] );
        }
    }
   
    return static_cast<float>( intra_cost );
}

#else

float IntraBlockDiff::Diff( const BlockDiffParams& dparams , ValueType& dc_val )
{
    if (dparams.Xl() <= 0 || dparams.Yl() <= 0)
    {
        dc_val = 0;
        return 0;
    }

     //computes the cost if block is predicted by its dc component
#if defined(HAVE_MMX)
    CalcValueType intra_cost =
            simple_intra_block_diff_mmx_4 (dparams, m_pic_data, dc_val);

#ifdef DIRAC_DEBUG
    CalcValueType int_dc( 0 );
    ValueType non_mmx_dc(0);

    for ( int j=dparams.Yp() ; j<dparams.Yp()+dparams.Yl() ; ++j)
        for(int i=dparams.Xp(); i<dparams.Xp()+dparams.Xl() ; ++i )
            int_dc += static_cast<int>( m_pic_data[j][i] );

    int_dc /= ( dparams.Xl() * dparams.Yl() );

    non_mmx_dc = static_cast<ValueType>( int_dc );

    // Now compute the resulting SAD
    ValueType dc( non_mmx_dc );
    CalcValueType non_mmx_intra_cost( 0 );

    for (int j=dparams.Yp(); j<dparams.Yend() ; ++j)
        for( int i=dparams.Xp() ; i<dparams.Xend() ;++i )
            non_mmx_intra_cost += std::abs( m_pic_data[j][i] - dc );

    if (non_mmx_dc != dc_val || non_mmx_intra_cost != intra_cost)
    {
        std::cerr << "MMX vals: dc=" << dc_val;
        std::cerr << " cost=" << intra_cost << std::endl;
        //print_arr (pic_data, width[i%5], height[i%5]);
        std::cerr << "non-MMX vals: dc=" << non_mmx_dc;
        std::cerr << " cost=" << non_mmx_intra_cost << std::endl;
    }
#endif
    return static_cast<float>( intra_cost );
#else
    CalcValueType int_dc( 0 );

    for ( int j=dparams.Yp() ; j<dparams.Yp()+dparams.Yl() ; ++j)
        for(int i=dparams.Xp(); i<dparams.Xp()+dparams.Xl() ; ++i )
            int_dc += static_cast<int>( m_pic_data[j][i] );

    int_dc /= ( dparams.Xl() * dparams.Yl() );

    dc_val = static_cast<ValueType>( int_dc );

    // Now compute the resulting SAD
    ValueType dc( dc_val );
    CalcValueType intra_cost( 0 );

    for (int j=dparams.Yp(); j<dparams.Yend() ; ++j)
        for( int i=dparams.Xp() ; i<dparams.Xend() ;++i )
            intra_cost += std::abs( m_pic_data[j][i] - dc );
   
    return static_cast<float>( intra_cost );
#endif //HAVE_MMX
}
#endif

float BlockDiffHalfPel::Diff(  const BlockDiffParams& dparams , 
                                      const MVector& mv )
{
    if (dparams.Xl() <= 0 || dparams.Yl() <= 0)
    {
        return 0;
    }
   //Where to start in the upconverted image
    const ImageCoords ref_start( ( dparams.Xp()<<1 ) + mv.x ,( dparams.Yp()<<1 ) + mv.y );
    const ImageCoords ref_stop( ref_start.x+(dparams.Xl()<<1) , ref_start.y+(dparams.Yl()<<1));


    bool bounds_check( false );

    if ( ref_start.x<0 || 
         ref_stop.x >= m_ref_data.LengthX() ||
         ref_start.y<0 || 
         ref_stop.y >= m_ref_data.LengthY() )
        bounds_check = true;

    float sum( 0 );

    if ( !bounds_check )
    {
#if defined (HAVE_MMX)
        MVector rmdr(0,0);
        const ImageCoords start_pos(dparams.Xp(), dparams.Yp());
        const ImageCoords end_pos(dparams.Xp() + dparams.Xl(), dparams.Yp() + dparams.Yl());
        sum = simple_block_diff_up_mmx_4 (m_pic_data, m_ref_data,
                                        start_pos, end_pos,
                                        ref_start, ref_stop,
                                        rmdr,
                                        sum,
                                        static_cast<float>(INT_MAX));
#else
        ValueType* pic_curr = &m_pic_data[dparams.Yp()][dparams.Xp()];
        const int pic_next( m_pic_data.LengthX() - dparams.Xl() );// go down a row and back up
        ValueType *ref_curr = &m_ref_data[ref_start.y][ref_start.x];
        const int ref_next( (m_ref_data.LengthX() - dparams.Xl())*2 );// go down 2 rows and back up

        for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
        {
            for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2 )
            {
                sum += std::abs( *ref_curr - *pic_curr );
            }// x
        }// y
#endif

    }
    else
    {
        // We're doing bounds checking because we'll fall off the edge of the reference otherwise.
        ValueType* pic_curr = &m_pic_data[dparams.Yp()][dparams.Xp()];
        const int pic_next( m_pic_data.LengthX() - dparams.Xl() );// go down a row and back up
        for( int y=dparams.Yl(), ry=ref_start.y, by=BChk(ry,m_ref_data.LengthY()); 
             y>0; 
             --y, pic_curr+=pic_next, ry+=2 , by=BChk(ry,m_ref_data.LengthY()))
        {
             for( int x=dparams.Xl() , rx=ref_start.x , bx=BChk(rx,m_ref_data.LengthX()); 
                  x>0 ; 
                  --x, ++pic_curr, rx+=2 , bx=BChk(rx,m_ref_data.LengthX()))
             {
                 sum += std::abs( m_ref_data[by][bx] -*pic_curr);
             }// x
        }// y

    }

    return sum;

}

void BlockDiffHalfPel::Diff( const BlockDiffParams& dparams,
                                   const MVector& mv ,
                                   const float mvcost,
                                   const float lambda,
                                   MvCostData& best_costs ,
                                   MVector& best_mv )
{
    if (dparams.Xl() <= 0 || dparams.Yl() <= 0)
    {
        return; 
    }

    //Where to start in the upconverted image
    const ImageCoords ref_start( ( dparams.Xp()<<1 ) + mv.x ,( dparams.Yp()<<1 ) + mv.y );
    const ImageCoords ref_stop( ref_start.x+(dparams.Xl()<<1) , ref_start.y+(dparams.Yl()<<1));


    bool bounds_check( false );

    if ( ref_start.x<0 || 
         ref_stop.x >= m_ref_data.LengthX() ||
         ref_start.y<0 || 
         ref_stop.y >= m_ref_data.LengthY() )
        bounds_check = true;

    const float start_val( mvcost*lambda );
    float sum( start_val );

    if ( !bounds_check )
    {
#if defined (HAVE_MMX)
        
        const ImageCoords start_pos(dparams.Xp(), dparams.Yp());
        const ImageCoords end_pos(dparams.Xp() + dparams.Xl(), dparams.Yp() + dparams.Yl());
        MVector rmdr(0,0);
        sum = simple_block_diff_up_mmx_4 (m_pic_data, m_ref_data,
                                        start_pos, end_pos,
                                        ref_start, ref_stop,
                                        rmdr,
                                        sum,
                                        best_costs.total);
        if ( sum>=best_costs.total )
            return;
#else
           ValueType* pic_curr = &m_pic_data[dparams.Yp()][dparams.Xp()];
           const int pic_next( m_pic_data.LengthX() - dparams.Xl() );// go down a row and back up
        ValueType *ref_curr = &m_ref_data[ref_start.y][ref_start.x];
        const int ref_next( (m_ref_data.LengthX() - dparams.Xl())*2 );// go down 2 rows and back up

        for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
        {
            for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2 )
            {
                sum += std::abs( *ref_curr - *pic_curr );
            }// x

            if ( sum>=best_costs.total )
                return;

        }// y
#endif
    }
    else
    {
        // We're doing bounds checking because we'll fall off the edge of the reference otherwise.
           ValueType* pic_curr = &m_pic_data[dparams.Yp()][dparams.Xp()];
           const int pic_next( m_pic_data.LengthX() - dparams.Xl() );// go down a row and back up
        for( int y=dparams.Yl(), ry=ref_start.y, by=BChk(ry,m_ref_data.LengthY()); 
             y>0; 
             --y, pic_curr+=pic_next, ry+=2 , by=BChk(ry,m_ref_data.LengthY()))
        {
             for( int x=dparams.Xl() , rx=ref_start.x , bx=BChk(rx,m_ref_data.LengthX()); 
                  x>0 ; 
                  --x, ++pic_curr, rx+=2 , bx=BChk(rx,m_ref_data.LengthX()))
             {
                 sum += std::abs( m_ref_data[by][bx] -*pic_curr);
             }// x

             if ( sum>=best_costs.total )
                return;

        }// y

    }

    best_mv = mv;
    best_costs.total = sum;
    best_costs.mvcost = mvcost;
    best_costs.SAD = sum - start_val;
}

float BlockDiffQuarterPel::Diff(  const BlockDiffParams& dparams , const MVector& mv )
{
    if (dparams.Xl() <= 0 || dparams.Yl() <= 0)
    {
        return 0; 
    }
   // Set up the start point in the reference image by rounding the motion vector
    // to 1/2 pel accuracy.NB: bit shift rounds negative values DOWN, as required
    const MVector roundvec( mv.x>>1 , mv.y>>1 );

    //Get the remainder after rounding. NB rmdr values always 0 or 1
    const MVector rmdr( mv.x & 1 , mv.y & 1 );

    //Where to start in the upconverted image
    const ImageCoords ref_start( ( dparams.Xp()<<1 ) + roundvec.x ,( dparams.Yp()<<1 ) + roundvec.y );
    const ImageCoords ref_stop( ref_start.x+(dparams.Xl()<<1) , ref_start.y+(dparams.Yl()<<1));
    bool bounds_check( false );

    if ( ref_start.x<0 || 
         ref_stop.x >= m_ref_data.LengthX() ||
         ref_start.y<0 || 
         ref_stop.y >= m_ref_data.LengthY() )
        bounds_check = true;

    float sum( 0.0f );
       CalcValueType temp;


    if ( !bounds_check )
    {
#if defined (HAVE_MMX)
        const ImageCoords start_pos(dparams.Xp(), dparams.Yp());
        const ImageCoords end_pos(dparams.Xp() + dparams.Xl(), dparams.Yp() + dparams.Yl());
           
        sum = simple_block_diff_up_mmx_4 (m_pic_data, m_ref_data,
                                        start_pos, end_pos,
                                        ref_start, ref_stop,
                                        rmdr,
                                        sum,
                                        static_cast<float>(INT_MAX));

#else
        ValueType* pic_curr = &m_pic_data[dparams.Yp()][dparams.Xp()];
        const int pic_next( m_pic_data.LengthX() - dparams.Xl() );// go down a row and back up
        if( rmdr.x == 0 && rmdr.y == 0 )
        {
            ValueType *ref_curr = &m_ref_data[ref_start.y][ref_start.x];
            const int ref_next( (m_ref_data.LengthX() - dparams.Xl())*2 );// go down 2 rows and back up
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2 )
                {
                    sum += std::abs( *ref_curr - *pic_curr );
                }// x
            }// y
        }
        else if( rmdr.y == 0 )
        {
            ValueType *ref_curr = &m_ref_data[ref_start.y][ref_start.x];
            const int ref_next( (m_ref_data.LengthX() - dparams.Xl())*2 );// go down 2 rows and back up
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2 )
                {
                    temp = (    CalcValueType( ref_curr[0] ) +
                                CalcValueType( ref_curr[1] ) +
                                1
                            ) >> 1;
                    sum += std::abs( temp - *pic_curr );
                }// x
            }// y
        }
        else if( rmdr.x == 0 )
        {
            ValueType *ref_curr = &m_ref_data[ref_start.y][ref_start.x];
            const int ref_next( (m_ref_data.LengthX() - dparams.Xl())*2 );// go down 2 rows and back up
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2 )
                {
                    temp = (    CalcValueType( ref_curr[0] ) +
                                CalcValueType( ref_curr[m_ref_data.LengthX()] ) +
                                1
                            ) >> 1;
                    sum += std::abs( temp - *pic_curr );
                }// x
            }// y
        }
        else
        {
            ValueType *ref_curr = &m_ref_data[ref_start.y][ref_start.x];
            const int ref_next( (m_ref_data.LengthX() - dparams.Xl())*2 );// go down 2 rows and back up
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2 )
                {
                    temp = (    CalcValueType( ref_curr[0] ) +
                                CalcValueType( ref_curr[1] ) +
                                CalcValueType( ref_curr[m_ref_data.LengthX()+0] ) +
                                CalcValueType( ref_curr[m_ref_data.LengthX()+1] ) +
                                2
                            ) >> 2;
                    sum += std::abs( temp - *pic_curr );
                }// x
            }// y
        }
#endif // HAVE_MMX
    }
    else
    {
        // We're doing bounds checking because we'll fall off the edge of the reference otherwise.

       // weights for doing linear interpolation, calculated from the remainder values
        const ValueType linear_wts[4] = {  (2 - rmdr.x) * (2 - rmdr.y),    //tl
                                           rmdr.x * (2 - rmdr.y),          //tr
                                           (2 - rmdr.x) * rmdr.y,          //bl
                                           rmdr.x * rmdr.y };              //br

        const int refXlen( m_ref_data.LengthX() );
        const int refYlen( m_ref_data.LengthY() );

        for(int y = dparams.Yp(), uY = ref_start.y,BuY=BChk(uY,refYlen),BuY1=BChk(uY+1,refYlen);
            y < dparams.Yend(); ++y, uY += 2,BuY=BChk(uY,refYlen),BuY1=BChk(uY+1,refYlen))
        {
            for(int x = dparams.Xp(), uX = ref_start.x,BuX=BChk(uX,refXlen),BuX1=BChk(uX+1,refXlen);
                x < dparams.Xend(); ++x, uX += 2,BuX=BChk(uX,refXlen),BuX1=BChk(uX+1,refXlen))
            {
 
                temp = (     linear_wts[0] * CalcValueType( m_ref_data[BuY][BuX] ) +
                             linear_wts[1] * CalcValueType( m_ref_data[BuY][BuX1] ) +
                             linear_wts[2] * CalcValueType( m_ref_data[BuY1][BuX] )+
                             linear_wts[3] * CalcValueType( m_ref_data[BuY1][BuX1] ) +
                             2
                        ) >> 2;
                sum += std::abs( temp - m_pic_data[y][x] );
            }// x
        }// y

    }

    return sum;

}

void BlockDiffQuarterPel::Diff( const BlockDiffParams& dparams,
                                   const MVector& mv ,
                                   const float mvcost,
                                   const float lambda,
                                   MvCostData& best_costs ,
                                   MVector& best_mv)
{
    if (dparams.Xl() <= 0 || dparams.Yl() <= 0)
    {
        return; 
    }

    // Set up the start point in the reference image by rounding the motion vector
    // to 1/2 pel accuracy.NB: bit shift rounds negative values DOWN, as required
    const MVector roundvec( mv.x>>1 , mv.y>>1 );

    //Get the remainder after rounding. NB rmdr values always 0 or 1
    const MVector rmdr( mv.x & 1 , mv.y & 1 );

    //Where to start in the upconverted image
    const ImageCoords ref_start( ( dparams.Xp()<<1 ) + roundvec.x ,( dparams.Yp()<<1 ) + roundvec.y );
    const ImageCoords ref_stop( ref_start.x+(dparams.Xl()<<1) , ref_start.y+(dparams.Yl()<<1));

    bool bounds_check( false );

    if ( ref_start.x<0 || 
         ref_stop.x >= m_ref_data.LengthX() ||
         ref_start.y<0 || 
         ref_stop.y >= m_ref_data.LengthY() )
        bounds_check = true;

    const float start_val( mvcost*lambda );
    float sum( start_val );

    CalcValueType temp;

    if ( !bounds_check )
    {
#if defined (HAVE_MMX)
        const ImageCoords start_pos(dparams.Xp(), dparams.Yp());
        const ImageCoords end_pos(dparams.Xp() + dparams.Xl(), dparams.Yp() + dparams.Yl());
        
        sum = simple_block_diff_up_mmx_4 (m_pic_data, m_ref_data,
                                        start_pos, end_pos,
                                        ref_start, ref_stop,
                                        rmdr,
                                        sum,
                                        best_costs.total);

        if ( sum>=best_costs.total )
            return;
#else
        ValueType* pic_curr = &m_pic_data[dparams.Yp()][dparams.Xp()];
        const int pic_next( m_pic_data.LengthX() - dparams.Xl() );// go down a row and back up
        ValueType *ref_curr = &m_ref_data[ref_start.y][ref_start.x];
        const int ref_next( (m_ref_data.LengthX() - dparams.Xl())*2 );// go down 2 rows and back up

        if( rmdr.x == 0 && rmdr.y == 0 )
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2 )
                {
                    sum += std::abs( *ref_curr - *pic_curr );
                }// x
                
                if ( sum>=best_costs.total )
                    return;
            }// y
        }
        else if( rmdr.y == 0 )
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2 )
                {
                    temp = (    CalcValueType( ref_curr[0] ) +
                                CalcValueType( ref_curr[1] ) +
                                1
                            ) >> 1;
                    sum += std::abs( temp - *pic_curr );
                }// x
                
                if ( sum>=best_costs.total )
                    return;
            }// y
        }
        else if( rmdr.x == 0 )
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2 )
                {
                    temp = (    CalcValueType( ref_curr[0] ) +
                                CalcValueType( ref_curr[m_ref_data.LengthX()] ) +
                                1
                            ) >> 1;
                    sum += std::abs( temp - *pic_curr );
                }// x
                
                if ( sum>=best_costs.total )
                    return;

            }// y
        }
        else
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2 )
                {
                    temp = (    CalcValueType( ref_curr[0] ) +
                                CalcValueType( ref_curr[1] ) +
                                CalcValueType( ref_curr[m_ref_data.LengthX()+0] ) +
                                CalcValueType( ref_curr[m_ref_data.LengthX()+1] ) +
                                2
                            ) >> 2;
                    sum += std::abs( temp - *pic_curr );
                }// x
                
                if ( sum>=best_costs.total )
                    return;

            }// y

        }
#endif // HAVE_MMX
    }
    else
    {
        // We're doing bounds checking because we'll fall off the edge of the reference otherwise.

       // weights for doing linear interpolation, calculated from the remainder values
        const ValueType linear_wts[4] = {  (2 - rmdr.x) * (2 - rmdr.y),    //tl
                                           rmdr.x * (2 - rmdr.y),          //tr
                                           (2 - rmdr.x) * rmdr.y,          //bl
                                           rmdr.x * rmdr.y };              //br

        const int refXlen( m_ref_data.LengthX() );
        const int refYlen( m_ref_data.LengthY() );

        for(int y = dparams.Yp(), uY = ref_start.y,BuY=BChk(uY,refYlen),BuY1=BChk(uY+1,refYlen);
            y < dparams.Yend(); ++y, uY += 2,BuY=BChk(uY,refYlen),BuY1=BChk(uY+1,refYlen))
        {
            for(int x = dparams.Xp(), uX = ref_start.x,BuX=BChk(uX,refXlen),BuX1=BChk(uX+1,refXlen);
                x < dparams.Xend(); ++x, uX += 2,BuX=BChk(uX,refXlen),BuX1=BChk(uX+1,refXlen))
            {
 
                temp = (     linear_wts[0] * CalcValueType( m_ref_data[BuY][BuX] ) +
                             linear_wts[1] * CalcValueType( m_ref_data[BuY][BuX1] ) +
                             linear_wts[2] * CalcValueType( m_ref_data[BuY1][BuX] )+
                             linear_wts[3] * CalcValueType( m_ref_data[BuY1][BuX1] ) +
                             2
                        ) >> 2;
                sum += std::abs( temp - m_pic_data[y][x] );
            }// x
                
            if ( sum>=best_costs.total )
                return;

        }// y

    }

    // Since we've got here, we must have beaten the best cost to date

    best_mv = mv;
    best_costs.total = sum;
    best_costs.mvcost = mvcost;
    best_costs.SAD = sum - start_val;
}

float BlockDiffEighthPel::Diff(  const BlockDiffParams& dparams , const MVector& mv )
{
    if (dparams.Xl() <= 0 || dparams.Yl() <= 0)
    {
        return 0; 
    }
   //Set up the start point in the reference image by rounding the motion vector
    //NB: bit shift rounds negative values DOWN, as required
    const MVector roundvec( mv.x>>2 , mv.y>>2 );

    //Get the remainder after rounding. NB rmdr values always 0,1,2 or 3
    const MVector rmdr( mv.x & 3 , mv.y & 3 );

    //Where to start in the upconverted image
    const ImageCoords ref_start( ( dparams.Xp()<<1 ) + roundvec.x ,( dparams.Yp()<<1 ) + roundvec.y );
    const ImageCoords ref_stop( ref_start.x+(dparams.Xl()<<1) , ref_start.y+(dparams.Yl()<<1));

    ValueType* pic_curr = &m_pic_data[dparams.Yp()][dparams.Xp()];
    const int pic_next( m_pic_data.LengthX() - dparams.Xl() );// go down a row and back up

    //weights for doing linear interpolation, calculated from the remainder values
    const ValueType linear_wts[4] = {  (4 - rmdr.x) * (4 - rmdr.y),    //tl
                                       rmdr.x * (4 - rmdr.y),          //tr
                                       (4 - rmdr.x) * rmdr.y,          //bl
                                       rmdr.x * rmdr.y };              //br

    bool bounds_check( false );

    if ( ref_start.x<0 || 
         ref_stop.x >= m_ref_data.LengthX() ||
         ref_start.y<0 || 
         ref_stop.y >= m_ref_data.LengthY() )
        bounds_check = true;

    float sum( 0.0f );

    CalcValueType temp;

    if ( !bounds_check )
    {
        ValueType *ref_curr = &m_ref_data[ref_start.y][ref_start.x];
        const int ref_next( (m_ref_data.LengthX() - dparams.Xl())*2 );// go down 2 rows and back up

        if( rmdr.x == 0 && rmdr.y == 0 )
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2 )
                {
                    sum += CalcValueType( std::abs( ref_curr[0] - *pic_curr ) );
                }// x
            }// y
        }
        else if( rmdr.y == 0 )
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2 )
                {
                    temp = ((    linear_wts[0] * CalcValueType( ref_curr[0] ) +
                                 linear_wts[1] * CalcValueType( ref_curr[1] ) +
                                 8
                            ) >> 4);
                    sum += std::abs( temp - *pic_curr );
                }// x
            }// y
        }
        else if( rmdr.x == 0 )
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2 )
                {
                    temp = ((    linear_wts[0] * CalcValueType( ref_curr[0] ) +
                                 linear_wts[2] * CalcValueType( ref_curr[m_ref_data.LengthX()+0] ) +
                                       8
                                   ) >> 4);
                    sum += std::abs( temp - *pic_curr );
                }// x
            }// y
        }
        else
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2 )
                {
                    temp = ((    linear_wts[0] * CalcValueType( ref_curr[0] ) +
                                 linear_wts[1] * CalcValueType( ref_curr[1] ) +
                                 linear_wts[2] * CalcValueType( ref_curr[m_ref_data.LengthX()+0] ) +
                                 linear_wts[3] * CalcValueType( ref_curr[m_ref_data.LengthX()+1] ) +
                                 8
                            ) >> 4);
                    sum += std::abs( temp - *pic_curr );
                }// x
            }// y
        }
    }
    else
    {
        // We're doing bounds checking because we'll fall off the edge of the reference otherwise.
       const int refXlen( m_ref_data.LengthX() );
       const int refYlen( m_ref_data.LengthY() );

       for(int y = dparams.Yp(), uY = ref_start.y,BuY=BChk(uY,refYlen),BuY1=BChk(uY+1,refYlen);
            y < dparams.Yend(); ++y, uY += 2,BuY=BChk(uY,refYlen),BuY1=BChk(uY+1,refYlen))
        {
            for(int x = dparams.Xp(), uX = ref_start.x,BuX=BChk(uX,refXlen),BuX1=BChk(uX+1,refXlen);
                x < dparams.Xend(); ++x, uX += 2,BuX=BChk(uX,refXlen),BuX1=BChk(uX+1,refXlen))
            {
    
                temp = ( linear_wts[0] * CalcValueType( m_ref_data[BuY][BuX] ) +
                         linear_wts[1] * CalcValueType( m_ref_data[BuY][BuX1] ) +
                         linear_wts[2] * CalcValueType( m_ref_data[BuY1][BuX] )+
                         linear_wts[3] * CalcValueType( m_ref_data[BuY1][BuX1] ) +
                         8
                        ) >> 4;
                sum += std::abs( temp - m_pic_data[y][x] );
            }// x
        }// y

    }

    return sum;
}

void BlockDiffEighthPel::Diff( const BlockDiffParams& dparams,
                                   const MVector& mv ,
                                   const float mvcost,
                                   const float lambda,
                                   MvCostData& best_costs ,
                                   MVector& best_mv)
{
    if (dparams.Xl() <= 0 || dparams.Yl() <= 0)
    {
        return; 
    }
    //Set up the start point in the reference image by rounding the motion vector
    //NB: bit shift rounds negative values DOWN, as required
    const MVector roundvec( mv.x>>2 , mv.y>>2 );

    //Get the remainder after rounding. NB rmdr values always 0,1,2 or 3
    const MVector rmdr( mv.x & 3 , mv.y & 3 );

    //Where to start in the upconverted image
    const ImageCoords ref_start( ( dparams.Xp()<<1 ) + roundvec.x ,( dparams.Yp()<<1 ) + roundvec.y );
    const ImageCoords ref_stop( ref_start.x+(dparams.Xl()<<1) , ref_start.y+(dparams.Yl()<<1));

    ValueType* pic_curr = &m_pic_data[dparams.Yp()][dparams.Xp()];
    const int pic_next( m_pic_data.LengthX() - dparams.Xl() );// go down a row and back up

    //weights for doing linear interpolation, calculated from the remainder values
    const ValueType linear_wts[4] = {  (4 - rmdr.x) * (4 - rmdr.y),    //tl
                                       rmdr.x * (4 - rmdr.y),          //tr
                                       (4 - rmdr.x) * rmdr.y,          //bl
                                       rmdr.x * rmdr.y };              //br

    bool bounds_check( false );

    if ( ref_start.x<0 || 
         ref_stop.x >= m_ref_data.LengthX() ||
         ref_start.y<0 || 
         ref_stop.y >= m_ref_data.LengthY() )
        bounds_check = true;

    const float start_val( mvcost*lambda );
    float sum( start_val );

    CalcValueType temp;

    if ( !bounds_check )
    {
        ValueType *ref_curr = &m_ref_data[ref_start.y][ref_start.x];
        const int ref_next( (m_ref_data.LengthX() - dparams.Xl())*2 );// go down 2 rows and back up

        if( rmdr.x == 0 && rmdr.y == 0 )
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2 )
                {
                    sum += CalcValueType( std::abs( ref_curr[0] - *pic_curr ) );
                }// x
                
                if ( sum>=best_costs.total )
                    return;

            }// y
        }
        else if( rmdr.y == 0 )
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2 )
                {
                    temp = ((    linear_wts[0] * CalcValueType( ref_curr[0] ) +
                                 linear_wts[1] * CalcValueType( ref_curr[1] ) +
                                 8
                            ) >> 4);
                    sum += std::abs( temp - *pic_curr );
                }// x
                
                if ( sum>=best_costs.total )
                    return;

            }// y
        }
        else if( rmdr.x == 0 )
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2 )
                {
                    temp = ((    linear_wts[0] * CalcValueType( ref_curr[0] ) +
                                 linear_wts[2] * CalcValueType( ref_curr[m_ref_data.LengthX()+0] ) +
                                       8
                                   ) >> 4);
                    sum += std::abs( temp - *pic_curr );
                }// x
                
                if ( sum>=best_costs.total )
                    return;

            }// y
        }
        else
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2 )
                {
                    temp = ((    linear_wts[0] * CalcValueType( ref_curr[0] ) +
                                 linear_wts[1] * CalcValueType( ref_curr[1] ) +
                                 linear_wts[2] * CalcValueType( ref_curr[m_ref_data.LengthX()+0] ) +
                                 linear_wts[3] * CalcValueType( ref_curr[m_ref_data.LengthX()+1] ) +
                                 8
                            ) >> 4);
                    sum += std::abs( temp - *pic_curr );
                }// x
                
                if ( sum>=best_costs.total )
                    return;

            }// y
        }
    }
    else
    {
        // We're doing bounds checking because we'll fall off the edge of the reference otherwise.
       const int refXlen( m_ref_data.LengthX() );
       const int refYlen( m_ref_data.LengthY() );

       for(int y = dparams.Yp(), uY = ref_start.y,BuY=BChk(uY,refYlen),BuY1=BChk(uY+1,refYlen);
            y < dparams.Yend(); ++y, uY += 2,BuY=BChk(uY,refYlen),BuY1=BChk(uY+1,refYlen))
        {
            for(int x = dparams.Xp(), uX = ref_start.x,BuX=BChk(uX,refXlen),BuX1=BChk(uX+1,refXlen);
                x < dparams.Xend(); ++x, uX += 2,BuX=BChk(uX,refXlen),BuX1=BChk(uX+1,refXlen))
            {
    
                temp = ( linear_wts[0] * CalcValueType( m_ref_data[BuY][BuX] ) +
                         linear_wts[1] * CalcValueType( m_ref_data[BuY][BuX1] ) +
                         linear_wts[2] * CalcValueType( m_ref_data[BuY1][BuX] )+
                         linear_wts[3] * CalcValueType( m_ref_data[BuY1][BuX1] ) +
                         8
                        ) >> 4;
                sum += std::abs( temp - m_pic_data[y][x] );
            }// x
                
            if ( sum>=best_costs.total )
                return;

        }// y

    }

    // If we've got here we must have done better than the best costs so far
    best_mv = mv;
    best_costs.total = sum;
    best_costs.mvcost = mvcost;
    best_costs.SAD = sum - start_val;
}

float BiBlockHalfPel::Diff(  const BlockDiffParams& dparams , 
                             const MVector& mv1 ,
                             const MVector& mv2 )
{
    if (dparams.Xl() <= 0 || dparams.Yl() <= 0)
    {
        return 0; 
    }
    // First create a difference array, and subtract the reference 1 data into it
    TwoDArray<ValueType> diff_array( dparams.Yl() , dparams.Xl() );

    //Where to start in the upconverted images
    const ImageCoords ref_start1( ( dparams.Xp()<<1 ) + mv1.x ,( dparams.Yp()<<1 ) + mv1.y );
    const ImageCoords ref_stop1( ref_start1.x+(dparams.Xl()<<1) , ref_start1.y+(dparams.Yl()<<1));

    const ImageCoords ref_start2( ( dparams.Xp()<<1 ) + mv2.x ,( dparams.Yp()<<1 ) + mv2.y );
    const ImageCoords ref_stop2( ref_start2.x+(dparams.Xl()<<1) , ref_start2.y+(dparams.Yl()<<1));

    ValueType* pic_curr = &m_pic_data[dparams.Yp()][dparams.Xp()];
    const int pic_next( m_pic_data.LengthX() - dparams.Xl() );// go down a row and back up

    ValueType* diff_curr = &diff_array[0][0];

    bool bounds_check( false );

    if ( ref_start1.x<0 || 
         ref_stop1.x >= m_ref_data1.LengthX() ||
         ref_start1.y<0 || 
         ref_stop1.y >= m_ref_data1.LengthY() )
        bounds_check = true;

    if ( !bounds_check )
    {
        ValueType *ref_curr = &m_ref_data1[ref_start1.y][ref_start1.x];
        const int ref_next( (m_ref_data1.LengthX() - dparams.Xl())*2 );// go down 2 rows and back up

        for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next)
        {
            for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2, ++diff_curr )
            {
                *diff_curr = ( (*pic_curr)<<1 ) - *ref_curr;

            }// x
        }// y

    }
    else
    {
        // We're doing bounds checking because we'll fall off the edge of the reference otherwise.
        for( int y=dparams.Yl(), ry=ref_start1.y, by=BChk(ry,m_ref_data1.LengthY()); 
             y>0; 
             --y, pic_curr+=pic_next, ry+=2 , by=BChk(ry,m_ref_data1.LengthY()))
        {
             for( int x=dparams.Xl() , rx=ref_start1.x , bx=BChk(rx,m_ref_data1.LengthX()); 
                  x>0 ; 
                  --x, ++pic_curr, rx+=2 , ++diff_curr, bx=BChk(rx,m_ref_data1.LengthX()))
             {
                 *diff_curr = ( (*pic_curr)<<1 ) - m_ref_data1[by][bx];
             }// x
        }// y

    }

    // Now do the other reference

    bounds_check = false;

    if ( ref_start2.x<0 || 
         ref_stop2.x >= m_ref_data2.LengthX() ||
         ref_start2.y<0 || 
         ref_stop2.y >= m_ref_data2.LengthY() )
        bounds_check = true;

    float sum( 0 );

    diff_curr = &diff_array[0][0];
    ValueType temp;

    if ( !bounds_check )
    {
        ValueType *ref_curr = &m_ref_data2[ref_start2.y][ref_start2.x];
        const int ref_next( (m_ref_data2.LengthX() - dparams.Xl())*2 );// go down 2 rows and back up

        for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next)
        {
            for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2, ++diff_curr )
            {
                temp = (*diff_curr - *ref_curr )>>1;
                sum += std::abs( temp );

            }// x
        }// y

    }
    else
    {
        // We're doing bounds checking because we'll fall off the edge of the reference otherwise.
        for( int y=dparams.Yl(), ry=ref_start2.y, by=BChk(ry,m_ref_data2.LengthY()); 
             y>0; 
             --y, pic_curr+=pic_next, ry+=2 , by=BChk(ry,m_ref_data2.LengthY()))
        {
             for( int x=dparams.Xl() , rx=ref_start2.x , bx=BChk(rx,m_ref_data2.LengthX()); 
                  x>0 ; 
                  --x, ++pic_curr, rx+=2 , ++diff_curr, bx=BChk(rx,m_ref_data2.LengthX()))
             {
                temp = (*diff_curr - m_ref_data2[by][bx] )>>1;
                sum += std::abs( temp );
             }// x
        }// y

    }

    return sum;

}

float BiBlockQuarterPel::Diff(  const BlockDiffParams& dparams , 
                             const MVector& mv1 ,
                             const MVector& mv2 )
{
    if (dparams.Xl() <= 0 || dparams.Yl() <= 0)
    {
        return 0; 
    }
    // First create a difference array, and subtract the reference 1 data into it
    TwoDArray<ValueType> diff_array( dparams.Yl() , dparams.Xl() );

   // Set up the start point in the reference images by rounding the motion vectors
    // to 1/2 pel accuracy.NB: bit shift rounds negative values DOWN, as required
    const MVector roundvec1 ( mv1.x>>1 , mv1.y>>1 );
    const MVector roundvec2 ( mv2.x>>1 , mv2.y>>1 );

   //Get the remainders after rounding. NB rmdr values always 0 or 1
    const MVector rmdr1( mv1.x & 1 , mv1.y & 1 );
    const MVector rmdr2( mv2.x & 1 , mv2.y & 1 );

    //Where to start in the upconverted images
    const ImageCoords ref_start1( ( dparams.Xp()<<1 ) + roundvec1.x ,( dparams.Yp()<<1 ) + roundvec1.y );
    const ImageCoords ref_stop1( ref_start1.x+(dparams.Xl()<<1) , ref_start1.y+(dparams.Yl()<<1));

    const ImageCoords ref_start2( ( dparams.Xp()<<1 ) + roundvec2.x ,( dparams.Yp()<<1 ) + roundvec2.y );
    const ImageCoords ref_stop2( ref_start2.x+(dparams.Xl()<<1) , ref_start2.y+(dparams.Yl()<<1));

    ValueType* pic_curr = &m_pic_data[dparams.Yp()][dparams.Xp()];
    const int pic_next( m_pic_data.LengthX() - dparams.Xl() );// go down a row and back up

    ValueType* diff_curr = &diff_array[0][0];

    bool bounds_check( false );

    if ( ref_start1.x<0 || 
         ref_stop1.x >= m_ref_data1.LengthX() ||
         ref_start1.y<0 || 
         ref_stop1.y >= m_ref_data1.LengthY() )
        bounds_check = true;

    ValueType temp;

    if ( !bounds_check )
    {
#if defined (HAVE_MMX)
        const ImageCoords start_pos(dparams.Xp(), dparams.Yp());
        const ImageCoords end_pos(dparams.Xp() + dparams.Xl(), dparams.Yp() + dparams.Yl());
        
        simple_biblock_diff_pic_mmx_4 (m_pic_data, m_ref_data1, diff_array,
                                    start_pos, end_pos,
                                    ref_start1, ref_stop1,
                                    rmdr1);
#else
        ValueType *ref_curr = &m_ref_data1[ref_start1.y][ref_start1.x];
        const int ref_next( (m_ref_data1.LengthX() - dparams.Xl())*2 );// go down 2 rows and back up



        if( rmdr1.x == 0 && rmdr1.y == 0 )
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2, ++diff_curr )
                {
                    *diff_curr = ( (*pic_curr)<<1 ) - *ref_curr;
                }// x
            }// y
        }
        else if( rmdr1.y == 0 )
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2, ++diff_curr )
                {
                    temp = (    CalcValueType( ref_curr[0] ) +
                                CalcValueType( ref_curr[1] ) +
                                1
                            ) >> 1;

                    *diff_curr = ( (*pic_curr)<<1 ) - temp;
                }// x
            }// y
        }
        else if( rmdr1.x == 0 )
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2, ++diff_curr )
                {
                    temp = (    CalcValueType( ref_curr[0] ) +
                                CalcValueType( ref_curr[m_ref_data1.LengthX()] ) +
                                1
                            ) >> 1;
                  *diff_curr = ( (*pic_curr)<<1 ) - temp;
                }// x
            }// y
        }
        else
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2, ++diff_curr )
                {
                    temp = (    CalcValueType( ref_curr[0] ) +
                                CalcValueType( ref_curr[1] ) +
                                CalcValueType( ref_curr[m_ref_data1.LengthX()+0] ) +
                                CalcValueType( ref_curr[m_ref_data1.LengthX()+1] ) +
                                2
                            ) >> 2;
                  *diff_curr = ( (*pic_curr)<<1 ) - temp;
                }// x
            }// y
        }

#endif
    }
    else
    {
        const ValueType linear_wts[4] = {  (2 - rmdr1.x) * (2 - rmdr1.y),    //tl
                                           rmdr1.x * (2 - rmdr1.y),          //tr
                                           (2 - rmdr1.x) * rmdr1.y,          //bl
                                           rmdr1.x * rmdr1.y };              //br

        // We're doing bounds checking because we'll fall off the edge of the reference otherwise.
        for( int y=dparams.Yl(), ry=ref_start1.y, by=BChk(ry,m_ref_data1.LengthY()), by1=BChk(ry+1,m_ref_data1.LengthY()); 
             y>0; 
             --y, pic_curr+=pic_next, ry+=2 , by=BChk(ry,m_ref_data1.LengthY()), by1=BChk(ry+1,m_ref_data1.LengthY()) )
        {
             for( int x=dparams.Xl() , rx=ref_start1.x , bx=BChk(rx,m_ref_data1.LengthX()), bx1=BChk(rx+1,m_ref_data1.LengthX()); 
                  x>0 ; 
                  --x, ++pic_curr, rx+=2 , ++diff_curr, bx=BChk(rx,m_ref_data1.LengthX()), bx1=BChk(rx+1,m_ref_data1.LengthX()))
             {
                temp = (     linear_wts[0] * CalcValueType( m_ref_data1[by][bx] ) +
                             linear_wts[1] * CalcValueType( m_ref_data1[by][bx1] ) +
                             linear_wts[2] * CalcValueType( m_ref_data1[by1][bx] )+
                             linear_wts[3] * CalcValueType( m_ref_data1[by1][bx1] ) +
                             2
                        ) >> 2;
                 *diff_curr = ( (*pic_curr)<<1 ) - temp;
             }// x
        }// y
    }

    // Now do the other reference

    bounds_check = false;

    if ( ref_start2.x<0 || 
         ref_stop2.x >= m_ref_data2.LengthX() ||
         ref_start2.y<0 || 
         ref_stop2.y >= m_ref_data2.LengthY() )
        bounds_check = true;

    float sum( 0 );

    diff_curr = &diff_array[0][0];

    if ( !bounds_check )
    {


#if defined (HAVE_MMX)
            sum = static_cast<float>( simple_biblock_diff_up_mmx_4 (diff_array, 
                                                m_ref_data2,
                                                ref_start2, ref_stop2,
                                                rmdr2) );
#else
        ValueType *ref_curr = &m_ref_data2[ref_start2.y][ref_start2.x];
        const int ref_next( (m_ref_data2.LengthX() - dparams.Xl())*2 );// go down 2 rows and back up
        if( rmdr2.x == 0 && rmdr2.y == 0 )
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2, ++diff_curr )
                {
                    sum += std::abs( (*diff_curr - *ref_curr)>>1 );
                }// x
            }// y
        }
        else if( rmdr2.y == 0 )
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2, ++diff_curr )
                {
                    temp = (    CalcValueType( ref_curr[0] ) +
                                CalcValueType( ref_curr[1] ) +
                                1
                            ) >> 1;

                    sum += std::abs( (*diff_curr - temp)>>1 );
                }// x
            }// y
        }
        else if( rmdr2.x == 0 )
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2, ++diff_curr )
                {
                    temp = (    CalcValueType( ref_curr[0] ) +
                                CalcValueType( ref_curr[m_ref_data2.LengthX()] ) +
                                1
                            ) >> 1;
                    sum += std::abs( (*diff_curr - temp)>>1 );
                }// x
            }// y
        }
        else
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2, ++diff_curr )
                {
                    temp = (    CalcValueType( ref_curr[0] ) +
                                CalcValueType( ref_curr[1] ) +
                                CalcValueType( ref_curr[m_ref_data2.LengthX()+0] ) +
                                CalcValueType( ref_curr[m_ref_data2.LengthX()+1] ) +
                                2
                            ) >> 2;
                    sum += std::abs( (*diff_curr - temp)>>1 );
                }// x
            }// y
        }
#endif
    }
    else
    {
        const ValueType linear_wts[4] = {  (2 - rmdr2.x) * (2 - rmdr2.y),    //tl
                                           rmdr2.x * (2 - rmdr2.y),          //tr
                                           (2 - rmdr2.x) * rmdr2.y,          //bl
                                           rmdr2.x * rmdr2.y };              //br

        // We're doing bounds checking because we'll fall off the edge of the reference otherwise.
        for( int y=dparams.Yl(), ry=ref_start2.y, by=BChk(ry,m_ref_data2.LengthY()),by1=BChk(ry+1,m_ref_data2.LengthY()); 
             y>0; 
             --y, pic_curr+=pic_next, ry+=2 , by=BChk(ry,m_ref_data2.LengthY()),by1=BChk(ry+1,m_ref_data2.LengthY()))
        {
             for( int x=dparams.Xl() , rx=ref_start2.x , bx=BChk(rx,m_ref_data2.LengthX()), bx1=BChk(rx+1,m_ref_data2.LengthX()); 
                  x>0 ; 
                  --x, ++pic_curr, rx+=2 , ++diff_curr, bx=BChk(rx,m_ref_data2.LengthX()), bx1=BChk(rx+1,m_ref_data2.LengthX()))
             {
                temp = (     linear_wts[0] * CalcValueType( m_ref_data2[by][bx] ) +
                             linear_wts[1] * CalcValueType( m_ref_data2[by][bx1] ) +
                             linear_wts[2] * CalcValueType( m_ref_data2[by1][bx] )+
                             linear_wts[3] * CalcValueType( m_ref_data2[by1][bx1] ) +
                             2
                        ) >> 2;
                sum += std::abs( (*diff_curr - temp)>>1 );
             }// x
        }// y
    }

    return sum;

}

float BiBlockEighthPel::Diff(  const BlockDiffParams& dparams , 
                             const MVector& mv1 ,
                             const MVector& mv2 )
{
    if (dparams.Xl() <= 0 || dparams.Yl() <= 0)
    {
        return 0; 
    }

    // First create a difference array, and subtract the reference 1 data into it
    TwoDArray<ValueType> diff_array( dparams.Yl() , dparams.Xl() );

   // Set up the start point in the reference images by rounding the motion vectors
    // to 1/2 pel accuracy.NB: bit shift rounds negative values DOWN, as required
    const MVector roundvec1 ( mv1.x>>2 , mv1.y>>2 );
    const MVector roundvec2 ( mv2.x>>2 , mv2.y>>2 );

   //Get the remainders after rounding. NB rmdr values always 0-3
    const MVector rmdr1( mv1.x & 3 , mv1.y & 3 );
    const MVector rmdr2( mv2.x & 3 , mv2.y & 3 );

    //weights for doing linear interpolation, calculated from the remainder values
    const ValueType linear_wts1[4] = {  (4 - rmdr1.x) * (4 - rmdr1.y),    //tl
                                       rmdr1.x * (4 - rmdr1.y),          //tr
                                       (4 - rmdr1.x) * rmdr1.y,          //bl
                                       rmdr1.x * rmdr1.y };              //br
    const ValueType linear_wts2[4] = {  (4 - rmdr2.x) * (4 - rmdr2.y),    //tl
                                       rmdr2.x * (4 - rmdr2.y),          //tr
                                       (4 - rmdr2.x) * rmdr2.y,          //bl
                                       rmdr2.x * rmdr2.y };              //br

    //Where to start in the upconverted images
    const ImageCoords ref_start1( ( dparams.Xp()<<1 ) + roundvec1.x ,( dparams.Yp()<<1 ) + roundvec1.y );
    const ImageCoords ref_stop1( ref_start1.x+(dparams.Xl()<<1) , ref_start1.y+(dparams.Yl()<<1));

    const ImageCoords ref_start2( ( dparams.Xp()<<1 ) + roundvec2.x ,( dparams.Yp()<<1 ) + roundvec2.y );
    const ImageCoords ref_stop2( ref_start2.x+(dparams.Xl()<<1) , ref_start2.y+(dparams.Yl()<<1));

    ValueType* pic_curr = &m_pic_data[dparams.Yp()][dparams.Xp()];
    const int pic_next( m_pic_data.LengthX() - dparams.Xl() );// go down a row and back up

    ValueType* diff_curr = &diff_array[0][0];

    bool bounds_check( false );

    if ( ref_start1.x<0 || 
         ref_stop1.x >= m_ref_data1.LengthX() ||
         ref_start1.y<0 || 
         ref_stop1.y >= m_ref_data1.LengthY() )
        bounds_check = true;

    ValueType temp;

    if ( !bounds_check )
    {
        ValueType *ref_curr = &m_ref_data1[ref_start1.y][ref_start1.x];
        const int ref_next( (m_ref_data1.LengthX() - dparams.Xl())*2 );// go down 2 rows and back up

        if( rmdr1.x == 0 && rmdr1.y == 0 )
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2, ++diff_curr )
                {
                    *diff_curr = ( (*pic_curr)<<1 ) - *ref_curr;
                }// x
            }// y
        }
        else if( rmdr1.y == 0 )
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2, ++diff_curr )
                {
                    temp = ((    linear_wts1[0] * CalcValueType( ref_curr[0] ) +
                                 linear_wts1[1] * CalcValueType( ref_curr[1] ) +
                                 8
                            ) >> 4);

                    *diff_curr = ( (*pic_curr)<<1 ) - temp;
                }// x
            }// y
        }
        else if( rmdr1.x == 0 )
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2, ++diff_curr )
                {
                    temp = ((    linear_wts1[0] * CalcValueType( ref_curr[0] ) +
                                 linear_wts1[2] * CalcValueType( ref_curr[m_ref_data1.LengthX()+0] ) +
                                       8
                                   ) >> 4);

                    *diff_curr = ( (*pic_curr)<<1 ) - temp;
                }// x
            }// y
        }
        else
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2, ++diff_curr )
                {
                    temp = ((    linear_wts1[0] * CalcValueType( ref_curr[0] ) +
                                 linear_wts1[1] * CalcValueType( ref_curr[1] ) +
                                 linear_wts1[2] * CalcValueType( ref_curr[m_ref_data1.LengthX()+0] ) +
                                 linear_wts1[3] * CalcValueType( ref_curr[m_ref_data1.LengthX()+1] ) +
                                 8
                            ) >> 4);
                  *diff_curr = ( (*pic_curr)<<1 ) - temp;
                }// x
            }// y
        }


    }
    else
    {
        // We're doing bounds checking because we'll fall off the edge of the reference otherwise.
        for( int y=dparams.Yl(), ry=ref_start1.y, by=BChk(ry,m_ref_data1.LengthY()), by1=BChk(ry+1,m_ref_data1.LengthY()); 
             y>0; 
             --y, pic_curr+=pic_next, ry+=2 , by=BChk(ry,m_ref_data1.LengthY()), by1=BChk(ry+1,m_ref_data1.LengthY()) )
        {
             for( int x=dparams.Xl() , rx=ref_start1.x , bx=BChk(rx,m_ref_data1.LengthX()), bx1=BChk(rx+1,m_ref_data1.LengthX()); 
                  x>0 ; 
                  --x, ++pic_curr, rx+=2 , ++diff_curr, bx=BChk(rx,m_ref_data1.LengthX()), bx1=BChk(rx+1,m_ref_data1.LengthX()))
             {
                temp = (     linear_wts1[0] * CalcValueType( m_ref_data1[by][bx] ) +
                             linear_wts1[1] * CalcValueType( m_ref_data1[by][bx1] ) +
                             linear_wts1[2] * CalcValueType( m_ref_data1[by1][bx] )+
                             linear_wts1[3] * CalcValueType( m_ref_data1[by1][bx1] ) +
                             8
                        ) >> 4;
                 *diff_curr = ( (*pic_curr)<<1 ) - temp;
             }// x
        }// y
    }

    // Now do the other reference

    bounds_check = false;

    if ( ref_start2.x<0 || 
         ref_stop2.x >= m_ref_data2.LengthX() ||
         ref_start2.y<0 || 
         ref_stop2.y >= m_ref_data2.LengthY() )
        bounds_check = true;

    float sum( 0 );

    diff_curr = &diff_array[0][0];

    if ( !bounds_check )
    {
        ValueType *ref_curr = &m_ref_data2[ref_start2.y][ref_start2.x];
        const int ref_next( (m_ref_data2.LengthX() - dparams.Xl())*2 );// go down 2 rows and back up

        if( rmdr2.x == 0 && rmdr2.y == 0 )
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2, ++diff_curr )
                {
                    sum += std::abs( (*diff_curr - *ref_curr)>>1 );
                }// x
            }// y
        }
        else if( rmdr2.y == 0 )
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2, ++diff_curr )
                {
                    temp = ((    linear_wts2[0] * CalcValueType( ref_curr[0] ) +
                                 linear_wts2[1] * CalcValueType( ref_curr[1] ) +
                                 8
                            ) >> 4);

                    sum += std::abs( (*diff_curr - temp)>>1 );
                }// x
            }// y
        }
        else if( rmdr2.x == 0 )
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2, ++diff_curr )
                {
                    temp = ((    linear_wts2[0] * CalcValueType( ref_curr[0] ) +
                                 linear_wts2[2] * CalcValueType( ref_curr[m_ref_data2.LengthX()+0] ) +
                                       8
                                   ) >> 4);

                    sum += std::abs( (*diff_curr - temp)>>1 );
                }// x
            }// y
        }
        else
        {
            for( int y=dparams.Yl(); y > 0; --y, pic_curr+=pic_next, ref_curr+=ref_next )
            {
                for( int x=dparams.Xl(); x > 0; --x, ++pic_curr, ref_curr+=2, ++diff_curr )
                {
                    temp = ((    linear_wts2[0] * CalcValueType( ref_curr[0] ) +
                                 linear_wts2[1] * CalcValueType( ref_curr[1] ) +
                                 linear_wts2[2] * CalcValueType( ref_curr[m_ref_data2.LengthX()+0] ) +
                                 linear_wts2[3] * CalcValueType( ref_curr[m_ref_data2.LengthX()+1] ) +
                                 8
                            ) >> 4);
                    sum += std::abs( (*diff_curr - temp)>>1 );
                }// x
            }// y
        }

    }
    else
    {
        // We're doing bounds checking because we'll fall off the edge of the reference otherwise.
        for( int y=dparams.Yl(), ry=ref_start1.y, by=BChk(ry,m_ref_data2.LengthY()),by1=BChk(ry+1,m_ref_data2.LengthY()); 
             y>0; 
             --y, pic_curr+=pic_next, ry+=2 , by=BChk(ry,m_ref_data2.LengthY()),by1=BChk(ry+1,m_ref_data2.LengthY()))
        {
             for( int x=dparams.Xl() , rx=ref_start1.x , bx=BChk(rx,m_ref_data2.LengthX()), bx1=BChk(rx+1,m_ref_data2.LengthX()); 
                  x>0 ; 
                  --x, ++pic_curr, rx+=2 , ++diff_curr, bx=BChk(rx,m_ref_data2.LengthX()), bx1=BChk(rx+1,m_ref_data2.LengthX()))
             {
                temp = (     linear_wts2[0] * CalcValueType( m_ref_data2[by][bx] ) +
                             linear_wts2[1] * CalcValueType( m_ref_data2[by][bx1] ) +
                             linear_wts2[2] * CalcValueType( m_ref_data2[by1][bx] )+
                             linear_wts2[3] * CalcValueType( m_ref_data2[by1][bx1] ) +
                             8
                        ) >> 4;
                sum += std::abs( (*diff_curr - temp)>>1 );
             }// x
        }// y
    }

    return sum;

}
