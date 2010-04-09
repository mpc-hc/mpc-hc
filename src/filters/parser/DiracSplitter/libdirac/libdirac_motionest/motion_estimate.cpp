/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: motion_estimate.cpp,v 1.23 2008/10/01 01:26:47 asuraparaju Exp $ $Name:  $
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


#include <libdirac_encoder/enc_queue.h>
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

void MotionEstimator::DoME( EncQueue& my_buffer, int pic_num )
{
    MEData& me_data = my_buffer.GetPicture( pic_num ).GetMEData();

    const PictureParams& pparams = my_buffer.GetPicture(pic_num).GetPparams();

   // Step 1. 
   //Initial search gives vectors for each reference accurate to 1 pixel

    PixelMatcher pix_match( m_encparams );
    pix_match.DoSearch( my_buffer , pic_num );

    float lambda;
    // Get the references
    const std::vector<int>& refs = my_buffer.GetPicture(pic_num).GetPparams().Refs();

    const int num_refs = refs.size();
    if ( pparams.IsBPicture())
        lambda = m_encparams.L2MELambda();
    else
        lambda = m_encparams.L1MELambda();

    // Set up the lambda to be used
    me_data.SetLambdaMap( num_refs , lambda );

    MVPrecisionType orig_prec = m_encparams.GetPicPredParams().MVPrecision();

    // Step 2. 
    // Pixel accurate vectors are then refined to sub-pixel accuracy

    if (orig_prec != MV_PRECISION_PIXEL)
    {
        SubpelRefine pelrefine( m_encparams );
        pelrefine.DoSubpel( my_buffer , pic_num );
    }
    else
    {
        // FIXME: HACK HACK
        // Mutiplying the motion vectors by 2 and setting MV precision to
        // HALF_PIXEL to implement pixel accurate motion estimate
        MvArray &mv_arr1 = me_data.Vectors(1);
        for (int j = 0; j < mv_arr1.LengthY(); ++j)
        {
            for (int i = 0; i < mv_arr1.LengthX(); ++i)
                mv_arr1[j][i] = mv_arr1[j][i] << 1;
        }
        if (num_refs > 1)
        {
            MvArray &mv_arr2 = me_data.Vectors(2);
            for (int j = 0; j < mv_arr2.LengthY(); ++j)
            {
                for (int i = 0; i < mv_arr2.LengthX(); ++i)
                    mv_arr2[j][i] = mv_arr2[j][i] << 1;
            }
        }
        m_encparams.GetPicPredParams().SetMVPrecision(MV_PRECISION_HALF_PIXEL);
    }

    // Step3.
    // We now have to decide how each superblock should be split 
    // and which references should be used, and so on.

    ModeDecider my_mode_dec( m_encparams );
    my_mode_dec.DoModeDecn( my_buffer , pic_num );
    
    if (orig_prec ==  MV_PRECISION_PIXEL)
    {
        // FIXME: HACK HACK
        // Divide the motion vectors by 2 to convert back to pixel
        // accurate motion vectors and reset MV precision to
        // PIXEL accuracy 
        MvArray &mv_arr1 = me_data.Vectors(1);
        for (int j = 0; j < mv_arr1.LengthY(); ++j)
        {
            for (int i = 0; i < mv_arr1.LengthX(); ++i)
                mv_arr1[j][i] = mv_arr1[j][i] >> 1;
        }
        if (num_refs > 1)
        {
            MvArray &mv_arr2 = me_data.Vectors(2);
            for (int j = 0; j < mv_arr2.LengthY(); ++j)
            {
                for (int i = 0; i < mv_arr2.LengthX(); ++i)
                    mv_arr2[j][i] = mv_arr2[j][i]>>1;
            }
        }
        m_encparams.GetPicPredParams().SetMVPrecision(MV_PRECISION_PIXEL);
    }

    // Finally, although not strictly part of motion estimation,
    // we have to assign DC values for chroma components for
    // blocks we're decided are intra.

    SetChromaDC( my_buffer , pic_num );

//return false;
}

ValueType MotionEstimator::GetChromaBlockDC(const PicArray& pic_data,
                                            int xunit , int yunit , int split)
{
    BlockDiffParams dparams;
    dparams.SetBlockLimits( m_encparams.GetPicPredParams().ChromaBParams( split ) , 
                            pic_data, xunit , yunit);

    ValueType dc;

    IntraBlockDiff intradiff( pic_data );

    intradiff.Diff( dparams , dc );

    return dc;
}

void MotionEstimator::SetChromaDC( const PicArray& pic_data , MEData& me_data , CompSort csort )
{

    // Lower limit of block coords in SB
    int xtl,ytl;
    // Upper limit of block coords in SB
    int xbr,ybr;

    // Ditto, for subSBs    
    int xsubSBtl,ysubSBtl;
    int xsubSBbr,ysubSBbr;

    TwoDArray<ValueType>& dcarray = me_data.DC( csort );

    ValueType dc = 0;

    // Coords of the prediction units (at appropriate level)
    int xunit, yunit;

    // The delimiters of the blocks contained in the prediction unit
    int xstart, ystart;
    int xend, yend;

    int level;

    for ( int ysb=0 ; ysb<me_data.SBSplit().LengthY() ; ++ysb )
    {
        for ( int xsb=0 ; xsb<me_data.SBSplit().LengthX() ; ++xsb )
        {

            level = me_data.SBSplit()[ysb][xsb];

            xtl = xsb<<2;
            ytl = ysb<<2;            
            xbr = xtl+4;
            ybr = ytl+4;

            xsubSBtl = xsb<<1;
            ysubSBtl = ysb<<1;
            xsubSBbr = xsubSBtl+2;
            ysubSBbr = ysubSBtl+2;


            for (int j = 0 ; j<(1<<level) ;++j)
            {
                 for (int i = 0 ; i<(1<<level) ;++i)
                 {
                     xunit = ( xsb<<level ) + i;
                     yunit = ( ysb<<level ) + j;

                     xstart = xunit<<( 2-level );
                     ystart = yunit<<( 2-level );

                     xend = xstart + ( 1<<( 2-level ) );
                     yend = ystart + ( 1<<( 2-level ) );

                     if ( me_data.Mode()[ystart][xstart] == INTRA )
                         // Get the DC value for the unit
                         dc = GetChromaBlockDC( pic_data , xunit , yunit , level );

                     // Copy it into the corresponding blocks
                     for ( int q=ystart ; q< yend ; ++q )
                         for ( int p=xstart ; p< xend ; ++p )
                             dcarray[q][p] = dc;

                 }// i
             }// j

        }// xsb
    }// ysb
}

void MotionEstimator::SetChromaDC( EncQueue& my_buffer , int pic_num )
{
    MEData& me_data = my_buffer.GetPicture(pic_num).GetMEData();  
    SetChromaDC( my_buffer.GetPicture( pic_num ).OrigData(U_COMP) , me_data , U_COMP );
    SetChromaDC( my_buffer.GetPicture( pic_num ).OrigData(V_COMP) , me_data , V_COMP );

}


