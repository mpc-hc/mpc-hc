/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: pixel_match.cpp 280 2005-01-30 05:11:46Z gabest $ $Name$
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

#include <libdirac_motionest/pixel_match.h>
#include <libdirac_motionest/block_match.h>
#include <libdirac_common/motion.h>
#include <libdirac_common/frame_buffer.h>
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


void PixelMatcher::DoSearch(const FrameBuffer& my_buffer, int frame_num, MEData& me_data)
{

     //does an initial search using hierarchical matching to get guide vectors    

    // Frame numbers of references
    int ref1,ref2;

    // Use the luminance only for motion estimating
    const PicArray& pic_data = my_buffer.GetComponent( frame_num , Y_COMP );

    const vector<int>& refs = my_buffer.GetFrame( frame_num ).GetFparams().Refs();
    ref1 = refs[0];
    if (refs.size()>1)
        ref2 = refs[1];
    else    
        ref2 = ref1;

    // Obtain C++ references to the reference picture luma components
    const PicArray& ref1_data = my_buffer.GetComponent(ref1 , Y_COMP);
    const PicArray& ref2_data = my_buffer.GetComponent(ref2 , Y_COMP);

    // Determine the frame sort - this affects the motion estimation Lagrangian parameter
    m_fsort = my_buffer.GetFrame(frame_num).GetFparams().FSort();

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
    const OLBParams bparams = m_encparams.LumaBParams(2);

    for (int i=1 ; i<=m_depth;++i)
    {

        xnumblocks = down_data[i]->LengthX()/bparams.Xbsep();
        ynumblocks = down_data[i]->LengthY()/bparams.Ybsep();

        if (( down_data[i]->LengthX() )%bparams.Xbsep() != 0)
            xnumblocks++;

        if (( down_data[i]->LengthY() )%bparams.Ybsep() != 0)
            ynumblocks++;

        me_data_set[i] = new MEData( 0 , 0 , xnumblocks , ynumblocks );
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
                            const MvData& guide_data, int ref_id)
{    

    // Initialisation //
    ////////////////////

    // Set the search ranges according to the level
    if ( m_level == m_depth )
    {
        m_xr = 5;
        m_yr = 5;
    }
    else if ( m_level == m_depth-1 )
    {
        m_xr = 4;
        m_yr = 4;
    }
    else
    {
        m_xr = 2;
        m_yr = 2;
    }

    // Provide aliases for the appropriate motion vector data components
    
    MvArray& mv_array = me_data.Vectors( ref_id );
    const MvArray& guide_array = guide_data.Vectors( ref_id );
    TwoDArray<MvCostData>& pred_costs = me_data.PredCosts( ref_id );

    // Provide a block matching object to do the work
    BlockMatcher my_bmatch( pic_data , ref_data , m_encparams.LumaBParams(2) , mv_array , pred_costs );

    float loc_lambda( 0.0 );

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
    AddNewVlistD( m_cand_list , zero_mv , m_xr , m_yr);

    // Now loop over the blocks and find the best matches. 
    // The loop is unrolled because predictions are different at picture edges.
    // The purpose of the loop is to create appropriate candidate lists, and then 
    // call the DoBlock() function which does the actual work.

    // First do TL corner

    // Set the prediction as the zero vector
    m_mv_prediction = zero_mv;

    // m_lambda is the Lagrangian smoothing parameter set to zero to get us started
    m_lambda = 0.0;
    DoBlock(0, 0 , pred_costs , mv_array, guide_array , my_bmatch);

    // The rest of the first row
    // ( use reduced lambda here )
    m_lambda = loc_lambda / float( m_encparams.YNumBlocks() );
    for ( int xpos=1 ; xpos<mv_array.LengthX() ; ++xpos )
    {
        m_mv_prediction = mv_array[0][xpos-1];
        DoBlock(xpos, 0 , pred_costs , mv_array, guide_array , my_bmatch);
    }// xpos

    // All the remaining rows except the last 
    for ( int ypos=1 ; ypos<mv_array.LengthY() ; ++ypos )
    {

        // The first element of each row
        m_mv_prediction = mv_array[ypos-1][0];
        m_lambda = loc_lambda/float(m_encparams.XNumBlocks());
        DoBlock(0, ypos , pred_costs , mv_array, guide_array , my_bmatch );

         // The middle elementes of each row
        m_lambda = loc_lambda;
        for ( int xpos=1 ; xpos<mv_array.LastX() ; ++xpos )
        {
            m_mv_prediction = MvMedian( mv_array[ypos][xpos-1],
                                        mv_array[ypos-1][xpos],
                                        mv_array[ypos-1][xpos+1]);
            DoBlock(xpos, ypos , pred_costs , mv_array, guide_array , my_bmatch );

        }// xpos

         // The last element in each row
        m_lambda = loc_lambda/float( m_encparams.XNumBlocks() );
        m_mv_prediction = MvMean( mv_array[ypos-1][ mv_array.LastX() ],
                                  mv_array[ypos][ mv_array.LastX()-1 ]);
        DoBlock(mv_array.LastX() , ypos , pred_costs , mv_array, guide_array , my_bmatch );

    }//ypos

}

void PixelMatcher::DoBlock(int xpos, int ypos ,
                           TwoDArray<MvCostData>& pred_costs,
                           MvArray& mv_array,
                           const MvArray& guide_array,
                           BlockMatcher& block_match)
{

    // Find the best match for each block ...

    // Use guide from lower down if one exists
    if ( m_level<m_depth )
        AddNewVlistD( m_cand_list , guide_array[ ypos>>1 ][ xpos>>1 ] * 2 , m_xr , m_yr );

    // use the spatial prediction, also, as a guide
    AddNewVlistD( m_cand_list , m_mv_prediction , m_xr , m_yr );

    // Find the best motion vector //
    /////////////////////////////////

    block_match.FindBestMatch( xpos , ypos , m_cand_list, m_mv_prediction, m_lambda );

    // Reset the lists ready for the next block (don't erase the first sublist as
    // this is a neighbourhood of zero, which we always look at)
    m_cand_list.erase( m_cand_list.begin()+1 , m_cand_list.end() );
}
