/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: pixel_match.cpp,v 1.20 2008/10/01 01:26:47 asuraparaju Exp $ $Name:  $
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
* Portions created by the Initial Developer are Copm_yright (C) 2004.
* All Rights Reserved.
*
* Contributor(s): Thomas Davies (Original Author),
*                 Tim Borer
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

#include <libdirac_motionest/pixel_match.h>
#include <libdirac_motionest/block_match.h>
#include <libdirac_common/motion.h>
#include <libdirac_encoder/enc_queue.h>
#include <libdirac_motionest/downconvert.h>
#include <libdirac_motionest/me_mode_decn.h>
#include <libdirac_motionest/me_subpel.h>
using namespace dirac;

#include <cmath>
#include <vector>

using std::vector;
using std::log;

PixelMatcher::PixelMatcher( const EncoderParams& encp):
    m_encparams(encp)
{}


void PixelMatcher::DoSearch( EncQueue& my_buffer, int pic_num )
{
    m_predparams = &(my_buffer.GetPicture(pic_num).GetMEData().GetPicPredParams() );

    //does an initial search using hierarchical matching to get guide vectors

    // Picture numbers of references
    int ref1,ref2;

    // Use the luminance only for motion estimating
    const PicArray& pic_data = my_buffer.GetPicture( pic_num ).DataForME(m_encparams.CombinedME());

    const vector<int>& refs = my_buffer.GetPicture( pic_num ).GetPparams().Refs();
    ref1 = refs[0];

    if (refs.size()>1)
        ref2 = refs[1];
    else
        ref2 = ref1;

    // Record temporal distances
    m_tdiff[0] = std::abs( ref1 - pic_num );
    m_tdiff[1] = std::abs( ref2 - pic_num );

    // Obtain C++ references to the reference picture luma components
    const PicArray& ref1_data = my_buffer.GetPicture(ref1).DataForME(m_encparams.CombinedME());
    const PicArray& ref2_data = my_buffer.GetPicture(ref2).DataForME(m_encparams.CombinedME());

    // Determine the picture sort - this affects the motion estimation Lagrangian parameter
    m_psort = my_buffer.GetPicture(pic_num).GetPparams().PicSort();


    if ( m_encparams.FullSearch() == false )
    {
        // Set the number of downconversion levels - not too many or we run out of picture!
        m_depth = ( int) std::min( log(((double) pic_data.LengthX())/12.0)/log(2.0) , 
                                 log(((double) pic_data.LengthY())/12.0)/log(2.0) );

        // These arrays will contain the downconverted picture and MvData hierarchy
        OneDArray<PicArray*> ref1_down( Range( 1 , m_depth ) );
        OneDArray<PicArray*> ref2_down( Range( 1 , m_depth ) );
        OneDArray<PicArray*> pic_down( Range( 1 , m_depth ) );
        OneDArray<MEData*> me_data_set( Range( 1 , m_depth ) );

        // Populate the hierarchies
        MakePicHierarchy( pic_data , pic_down );
        MakePicHierarchy( ref1_data , ref1_down );
        if (ref1 != ref2)
            MakePicHierarchy( ref2_data , ref2_down );

        MakeMEDataHierarchy( pic_down , me_data_set );

         // Now do the work! //
        //////////////////////

        // Start with motion estimating at the very lowest level
        m_level = m_depth;

        MatchPic( *(pic_down[m_depth]) , *(ref1_down[m_depth]) , *(me_data_set[m_depth]) ,
                                     *(me_data_set[m_depth]) , 1 );
        if ( ref1 != ref2 )
            MatchPic( *(pic_down[m_depth]) , *(ref2_down[m_depth]) , *(me_data_set[m_depth]) ,
                                             *(me_data_set[m_depth]) , 2 );

         // Do the intervening levels - here we can have a genuine set of guide vectors
        for ( m_level=m_depth-1 ; m_level>=1 ; --m_level )
        {
            MatchPic( *(pic_down[m_level]) , *(ref1_down[m_level]) , *(me_data_set[m_level]) ,
                                         *(me_data_set[m_level+1]) , 1 );
            if (ref1!=ref2)
                MatchPic( *(pic_down[m_level]) , *(ref2_down[m_level]) , *(me_data_set[m_level]) ,
                                                 *(me_data_set[m_level+1]) , 2 );
        }// level

        // Finally, do the top level, with the pictures themselves
        m_level = 0;
	MEData& me_data = my_buffer.GetPicture(pic_num).GetMEData();
        MatchPic( pic_data , ref1_data, me_data , *(me_data_set[1]) , 1 );
        if ( ref1 != ref2 )
            MatchPic( pic_data , ref2_data , me_data , *(me_data_set[1]) , 2 );

        // Now we're finished, tidy everything up ...
        TidyPics( pic_down );
        TidyPics( ref1_down );
        if (ref1 != ref2)
            TidyPics( ref2_down );
        TidyMEData( me_data_set );
    }
    else
    {
        m_depth = 0;
        m_level = 0;
	MEData& me_data = my_buffer.GetPicture(pic_num).GetMEData();
        MatchPic( pic_data , ref1_data, me_data , me_data , 1 );
        if ( ref1 != ref2 )
            MatchPic( pic_data , ref2_data , me_data , me_data , 2 );
    }

}

void PixelMatcher::MakePicHierarchy(const PicArray& data ,
                                    OneDArray< PicArray* >& down_data)
{

    DownConverter mydcon;

    // Allocate
    int scale_factor = 1;
    for (int i=1 ; i<=m_depth;++i)
    {
        // Dimensions of pic_down[i] will be shrunk by a factor 2**i
        scale_factor*=2;
        down_data[i] = new PicArray( data.LengthY()/scale_factor , data.LengthX()/scale_factor);
    }

    //do all the downconversions
    if (m_depth>0)
    {
        mydcon.DoDownConvert( data , *(down_data[1]) );

        for (int i=1 ; i<m_depth ; ++i)
            mydcon.DoDownConvert( *(down_data[i]) , *(down_data[i+1]) );

    }
}

void PixelMatcher::MakeMEDataHierarchy(const OneDArray< PicArray*>& down_data,
                                       OneDArray< MEData* >& me_data_set )
{

    int xnumblocks , ynumblocks;
    const OLBParams bparams = m_predparams->LumaBParams(2);

    // We might not have an integral number of Macroblocks and blocks in
    // a picture. So we go start of with the number of macroblocks in the
    // full size picture and calculate the number of in the downsized pics
    // from this.
    xnumblocks = m_predparams->XNumBlocks();
    ynumblocks = m_predparams->YNumBlocks();

    PicturePredParams predparams = *m_predparams;
    predparams.SetXNumSB(0);
    predparams.SetYNumSB(0);
    for (int i=1 ; i<=m_depth;++i)
    {

        xnumblocks = xnumblocks>>1;
        ynumblocks = ynumblocks>>1;

        if (( down_data[i]->LengthX() )%bparams.Xbsep() != 0)
            xnumblocks++;

        if (( down_data[i]->LengthY() )%bparams.Ybsep() != 0)
            ynumblocks++;

	predparams.SetXNumBlocks( xnumblocks );
	predparams.SetYNumBlocks( ynumblocks );

        me_data_set[i] = new MEData( predparams, 2 );
    }// i

}

void PixelMatcher::TidyPics( OneDArray< PicArray*>& down_data )
{
    for (int i=1 ; i <= m_depth ; ++i)
    {
        delete down_data[i];
    }// i

}

void PixelMatcher::TidyMEData( OneDArray< MEData*>& me_data_set )
{
    for (int i=1 ; i <= m_depth ; ++i)
    {
        delete me_data_set[i];
    }// i

}



void PixelMatcher::MatchPic(const PicArray& pic_data , const PicArray& ref_data , MEData& me_data ,
                            const MvData& guide_data, const int ref_id)
{

    // Initialisation //
    ////////////////////

    m_big_xr = std::min( m_tdiff[ref_id-1], 3 )*m_encparams.XRangeME();
    m_big_yr = std::min( m_tdiff[ref_id-1], 3 )*m_encparams.YRangeME();

    // Set the search ranges according to the level
    if ( m_encparams.FullSearch() == false )
    {
        m_cost_mean = 0.0;
        m_cost_mean_sq = 0.0;

        m_xr = std::min( m_level+1, 5);
        m_yr = std::min( m_level+1, 5);
    }
    else
    {
        m_xr = m_big_xr;
        m_yr = m_big_yr;
    }

    // Provide aliases for the appropriate motion vector data components

    MvArray& mv_array = me_data.Vectors( ref_id );
    const MvArray& guide_array = guide_data.Vectors( ref_id );
    TwoDArray<MvCostData>& pred_costs = me_data.PredCosts( ref_id );

    // Initialise the arrays
    for (int y=0; y<mv_array.LengthY(); ++y)
    {
        for (int x=0; x<mv_array.LengthX(); ++x)
        {
            mv_array[y][x].x = 0;
            mv_array[y][x].y = 0;
            pred_costs[y][x].total = 10000000.0f;
        }// x
    }// y

    // Provide a block matching object to do the work
    BlockMatcher my_bmatch( pic_data , ref_data ,
                            m_predparams->LumaBParams(2) , m_predparams->MVPrecision() ,
                            mv_array , pred_costs );

    // Do the work - loop over all the blocks, finding the best match //
    ////////////////////////////////////////////////////////////////////

    /*
    The idea is for each block construct a list of candidate vectors,which will
    be tested. This list is actually a list of lists, implemented as a C++
    vector of C++ vectors. This is so that FindBestMatch can shorten the
    search process by looking at the beginning of each sublist and
    discarding that sub-list if it's too far off.
    */

    // Make a zero-based list that is always used
    m_cand_list.clear();
    MVector zero_mv( 0 , 0 );

    AddNewVlist( m_cand_list , zero_mv , m_xr , m_yr);

    // Now loop over the blocks and find the best matches.
    // The loop is unrolled because predictions are different at picture edges.
    // The purpose of the loop is to create appropriate candidate lists, and then
    // call the DoBlock() function which does the actual work.

    // First do TL corner

    // Set the prediction as the zero vector
    m_mv_prediction = zero_mv;

    DoBlock(0, 0 , guide_array , my_bmatch);

    // The rest of the first row
    for ( int xpos=1 ; xpos<mv_array.LengthX() ; ++xpos )
    {
        m_mv_prediction = mv_array[0][xpos-1];
        DoBlock(xpos, 0 , guide_array , my_bmatch);
    }// xpos

    // All the remaining rows except the last
    for ( int ypos=1 ; ypos<mv_array.LengthY() ; ++ypos )
    {

        // The first element of each row
        m_mv_prediction = mv_array[ypos-1][0];
        DoBlock(0, ypos , guide_array , my_bmatch );

         // The middle elements of each row
        for ( int xpos=1 ; xpos<mv_array.LastX() ; ++xpos )
        {
            m_mv_prediction = MvMedian( mv_array[ypos][xpos-1],
                                        mv_array[ypos-1][xpos],
                                        mv_array[ypos-1][xpos+1]);
            DoBlock(xpos, ypos , guide_array , my_bmatch );

        }// xpos

         // The last element in each row
        m_mv_prediction = MvMean( mv_array[ypos-1][ mv_array.LastX() ],
                                  mv_array[ypos][ mv_array.LastX()-1 ]);
        DoBlock(mv_array.LastX() , ypos , guide_array , my_bmatch );
    }//ypos

}

void PixelMatcher::DoBlock(const int xpos, const int ypos ,
                           const MvArray& guide_array,
                           BlockMatcher& block_match)
{
    // Find the best match for each block ...

    // Use guide from lower down if one exists
    if ( m_level<m_depth )
    {
        int xdown = BChk(xpos>>1, guide_array.LengthX());
        int ydown = BChk(ypos>>1, guide_array.LengthY());
        AddNewVlist( m_cand_list , guide_array[ydown][xdown] * 2 , m_xr , m_yr );

    }

    // use the spatial prediction, also, as a guide
    if (m_encparams.FullSearch()==false )
        AddNewVlist( m_cand_list , m_mv_prediction , m_xr , m_yr );
    else
        AddNewVlist( m_cand_list , m_mv_prediction , 1 , 1);

    // Find the best motion vector //
    /////////////////////////////////

    block_match.FindBestMatchPel( xpos , ypos , m_cand_list, m_mv_prediction, 0 );

    // Reset the lists ready for the next block (don't erase the first sublist as
    // this is a neighbourhood of zero, which we always look at)
    m_cand_list.erase( m_cand_list.begin()+1 , m_cand_list.end() );
}
