/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: me_subpel.cpp,v 1.20 2008/10/01 01:26:47 asuraparaju Exp $ $Name:  $
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

#include <libdirac_motionest/me_subpel.h>
#include <libdirac_encoder/enc_queue.h>
using namespace dirac;

#include <iostream>

using std::vector;

SubpelRefine::SubpelRefine(const EncoderParams& encp):
    m_encparams(encp),
    m_nshift(4)
{
    //define the relative coordinates of the four neighbours
    m_nshift[0].x = -1;
    m_nshift[0].y = 0;

    m_nshift[1].x = -1;
    m_nshift[1].y = -1;

    m_nshift[2].x = 0;
    m_nshift[2].y = -1;

    m_nshift[3].x = 1;
    m_nshift[3].y = -1;

}

void SubpelRefine::DoSubpel( EncQueue& my_buffer,int pic_num )
{
    m_predparams = &(my_buffer.GetPicture(pic_num).GetMEData().GetPicPredParams() );

    //main loop for the subpel refinement
    int ref1,ref2;

    const PictureSort psort = my_buffer.GetPicture(pic_num).GetPparams().PicSort();

    if (psort.IsInter())
    {
        // Get the references
        const vector<int>& refs = my_buffer.GetPicture(pic_num).GetPparams().Refs();

        int num_refs = refs.size();
        ref1 = refs[0];
        if (num_refs>1)
            ref2 = refs[1];
        else
            ref2 = ref1;

        const PicArray& pic_data = my_buffer.GetPicture(pic_num).DataForME(m_encparams.CombinedME());
        const PicArray& refup1_data = my_buffer.GetPicture(ref1).UpDataForME(m_encparams.CombinedME());
        const PicArray& refup2_data = my_buffer.GetPicture(ref2).UpDataForME(m_encparams.CombinedME());

	MEData& me_data = my_buffer.GetPicture(pic_num).GetMEData();

        // Now match the pictures
        MatchPic( pic_data , refup1_data , me_data ,1 );

        if (ref1 != ref2 )
            MatchPic( pic_data , refup2_data , me_data ,2 );

    }
}

void SubpelRefine::MatchPic(const PicArray& pic_data , const PicArray& refup_data , MEData& me_data ,
                             int ref_id)
{
    // Match a picture against a single reference. Loop over all the blocks
    // doing the matching

    // Initialisation //
    ////////////////////

    // Provide aliases for the appropriate motion vector data components
    MvArray& mv_array = me_data.Vectors( ref_id );
    TwoDArray<MvCostData>& pred_costs = me_data.PredCosts( ref_id );

    // Provide a block matching object to do the work
    BlockMatcher my_bmatch( pic_data , refup_data , m_predparams->LumaBParams(2) ,
                            m_predparams->MVPrecision() , mv_array , pred_costs );

    // Do the work //
    /////////////////

    // Loop over all the blocks, doing the work

    for (int yblock=0 ; yblock<m_predparams->YNumBlocks() ; ++yblock){
        for (int xblock=0 ; xblock<m_predparams->XNumBlocks() ; ++xblock){
            DoBlock(xblock , yblock , my_bmatch , me_data , ref_id );
        }// xblock
    }// yblock

}

void SubpelRefine::DoBlock(const int xblock , const int yblock ,
                           BlockMatcher& my_bmatch, MEData& me_data , const int ref_id )
{
    // For each block, home into the sub-pixel vector

    // Provide aliases for the appropriate motion vector data components
    MvArray& mv_array = me_data.Vectors( ref_id );

    const MVector mv_pred = GetPred( xblock , yblock , mv_array );
    const float loc_lambda = me_data.LambdaMap()[yblock][xblock];

    my_bmatch.RefineMatchSubp( xblock , yblock , mv_pred, loc_lambda );
}

MVector SubpelRefine::GetPred(int xblock,int yblock,const MvArray& mvarray)
{
    MVector mv_pred;
    ImageCoords n_coords;
    vector<MVector> neighbours;

    if (xblock>0 && yblock>0 && xblock<mvarray.LastX())
    {

        for (int i=0 ; i<m_nshift.Length() ; ++i)
        {
            n_coords.x = xblock+m_nshift[i].x;
            n_coords.y = yblock+m_nshift[i].y;
            neighbours.push_back(mvarray[n_coords.y][n_coords.x]);

        }// i
    }
    else
    {
        for (int i=0 ; i<m_nshift.Length(); ++i )
        {
            n_coords.x = xblock+m_nshift[i].x;
            n_coords.y = yblock+m_nshift[i].y;
            if (n_coords.x>=0 && n_coords.y>=0 && n_coords.x<mvarray.LengthX() && n_coords.y<mvarray.LengthY())
                neighbours.push_back(mvarray[n_coords.y][n_coords.x]);
        }// i
    }

    mv_pred = MvMedian(neighbours);

    return mv_pred;
}
