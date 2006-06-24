/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: motion_estimate.cpp 280 2005-01-30 05:11:46Z gabest $ $Name$
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
* Contributor(s): Thomas Davies (Original Author)
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


#include <libdirac_common/frame_buffer.h>
#include <libdirac_motionest/motion_estimate.h>
#include <libdirac_motionest/pixel_match.h>
#include <libdirac_motionest/me_subpel.h>
#include <libdirac_motionest/me_mode_decn.h>
using namespace dirac;

#include <cmath>
#include <vector>

MotionEstimator::MotionEstimator( const EncoderParams& encp ):
    m_encparams( encp )
{}

bool MotionEstimator::DoME(const FrameBuffer& my_buffer, int frame_num, MEData& me_data)
{

    const FrameParams& fparams = my_buffer.GetFrame(frame_num).GetFparams();

   // Step 1. 
   //Initial search gives vectors for each reference accurate to 1 pixel

    PixelMatcher pix_match( m_encparams );
    pix_match.DoSearch( my_buffer , frame_num , me_data);

    // Step 2. 
    // Pixel accurate vectors are then refined to 1/8 of a pixel

    SubpelRefine pelrefine( m_encparams );
    pelrefine.DoSubpel( my_buffer , frame_num , me_data );


    // Step3.
    // We now have to decide how each macroblock should be split 
    // and which references should be used, and so on.

    ModeDecider my_mode_dec( m_encparams );
    my_mode_dec.DoModeDecn( my_buffer , frame_num , me_data );

    // Finally, although not strictly part of motion estimation,
    // we have to assign DC values for chroma components for
    // blocks we're decided are intra.

    if (fparams.CFormat() != Yonly)
        SetChromaDC( my_buffer , frame_num , me_data );

    return IsACut( me_data );

}

ValueType MotionEstimator::GetChromaBlockDC(const PicArray& pic_data,
                                            int xunit , int yunit , int split)
{
    BlockDiffParams dparams;
    dparams.SetBlockLimits( m_encparams.ChromaBParams( split ) , 
                            pic_data, xunit , yunit);

    ValueType dc;

    IntraBlockDiff intradiff( pic_data );

    intradiff.Diff( dparams , dc );

    return dc;
}

void MotionEstimator::SetChromaDC( const PicArray& pic_data , MvData& mv_data , CompSort csort )
{

    // Lower limit of block coords in MB
    int xtl,ytl;
    // Upper limit of block coords in MB
    int xbr,ybr;

    // Ditto, for subMBs    
    int xsubMBtl,ysubMBtl;
    int xsubMBbr,ysubMBbr;

    TwoDArray<ValueType>& dcarray = mv_data.DC( csort );

    ValueType dc = 0;

    // Coords of the prediction units (at appropriate level)
    int xunit, yunit;

    // The delimiters of the blocks contained in the prediction unit
    int xstart, ystart;
    int xend, yend;

    int level;

    for ( int ymb=0 ; ymb<mv_data.MBSplit().LengthY() ; ++ymb )
    {
        for ( int xmb=0 ; xmb<mv_data.MBSplit().LengthX() ; ++xmb )
        {

            level = mv_data.MBSplit()[ymb][xmb];

            xtl = xmb<<2;
            ytl = ymb<<2;            
            xbr = xtl+4;
            ybr = ytl+4;

            xsubMBtl = xmb<<1;
            ysubMBtl = ymb<<1;
            xsubMBbr = xsubMBtl+2;
            ysubMBbr = ysubMBtl+2;


            for (int j = 0 ; j<(1<<level) ;++j)
            {
                 for (int i = 0 ; i<(1<<level) ;++i)
                 {
                     xunit = ( xmb<<level ) + i;
                     yunit = ( ymb<<level ) + j;

                     xstart = xunit<<( 2-level );
                     ystart = yunit<<( 2-level );

                     xend = xstart + ( 1<<( 2-level ) );
                     yend = ystart + ( 1<<( 2-level ) );

                     if ( mv_data.Mode()[ystart][xstart] == INTRA )
                         // Get the DC value for the unit
                         dc = GetChromaBlockDC( pic_data , xunit , yunit , level );

                     // Copy it into the corresponding blocks
                     for ( int q=ystart ; q< yend ; ++q )
                         for ( int p=xstart ; p< xend ; ++p )
                             dcarray[q][p] = dc;

                 }// i
             }// j

        }// xmb
    }// ymb
}

void MotionEstimator::SetChromaDC( const FrameBuffer& my_buffer , int frame_num , MvData& mv_data)
{

    SetChromaDC( my_buffer.GetComponent( frame_num , U_COMP) , mv_data , U_COMP );
    SetChromaDC( my_buffer.GetComponent( frame_num , V_COMP) , mv_data , V_COMP );

}

bool MotionEstimator::IsACut( const MEData& me_data ) const
{
    // Count the number of intra blocks
    const TwoDArray<PredMode>& modes = me_data.Mode();

    int count_intra = 0;
    for ( int j=0 ; j<modes.LengthY() ; ++j )
    {
        for ( int i=0 ; i<modes.LengthX() ; ++i )
        {
            if ( modes[j][i] == INTRA )
                count_intra++;
        }
    }// j
    
    double intra_percent = 100.0*static_cast<double>( count_intra ) / 
                           static_cast<double>( modes.LengthX() * modes.LengthY() );

    if ( m_encparams.Verbose() )
        std::cerr<<std::endl<<intra_percent<<"% of blocks are intra   ";

    // Check the size of SAD errors across reference 1    
    const TwoDArray<MvCostData>& pcosts = me_data.PredCosts( 1 );

    // averege SAD across all relevant blocks
    long double sad_average = 0.0;
    // average SAD in a given block
    long double block_average; 
    // the block parameters
    const OLBParams& bparams = m_encparams.LumaBParams( 2 ); 
    //the count of the relevant blocks
    int block_count = 0;

    for ( int j=0 ; j<pcosts.LengthY() ; ++j )
    {
        for ( int i=0 ; i<pcosts.LengthX() ; ++i )
        {

            if ( modes[j][i] == REF1_ONLY || modes[j][i] == REF1AND2 )
            {
                block_average = pcosts[j][i].SAD /
                                static_cast<long double>( bparams.Xblen() * bparams.Yblen() * 4 );
                sad_average += block_average;
                block_count++;
            }

        }// i
    }// j

    if ( block_count != 0)
        sad_average /= static_cast<long double>( block_count );
   
    if ( (sad_average > 30.0) || (intra_percent > 50.0) )
        return true;
    else
        return false;
  
}
