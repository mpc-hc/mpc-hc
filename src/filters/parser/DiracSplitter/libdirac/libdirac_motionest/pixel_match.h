/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: pixel_match.h,v 1.11 2008/08/27 00:20:52 asuraparaju Exp $ $Name:  $
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

#ifndef _PIXEL_MATCH_H_
#define _PIXEL_MATCH_H_

/* *************************************************************************
*
* Class for getting motion vectors to pixel-accuracy
*
* The class could be implemented in any number of ways. The approach taken
* has been to do hierarchical matching, which means doing block matching
* on smaller, downcoverted versions of the pictures in order to get a wider
* effective search range. At each level of searching the vectors discovered
* can be used as guides to the next level of searching, and in this way
* large motions can be detected easily. The danger is that the motions of
* small objects can be overlooked.
*
* *************************************************************************/

#include <libdirac_common/common.h>
#include <libdirac_common/motion.h>
#include <libdirac_motionest/block_match.h>
namespace dirac
{
    class EncQueue;
    class MvData;
    class EncoderParams;
    class PicArray;


    class PixelMatcher
    {
    public:

        //! Constructor
        PixelMatcher( const EncoderParams& encp);

        //! Do the actual search
        /* Do the searching.

        \param  my_buffer  the buffer of pictures from which pictures are taken
        \param  pic_num  the number of the picture for which motion is to be estimated
        \param  mv_data    class in which the measured motion vectors are stored, together with costs
        
        */
        void DoSearch( EncQueue& my_buffer, int pic_num ); 

    private:

        // Member variables

        //! Local reference to the encoder params 
        const EncoderParams& m_encparams;

        //! Local reference to the picture pred params 
        const PicturePredParams* m_predparams;

        // the depth of the hierarchical match 
        int m_depth;

        // the level we're at (from 0 to depth)
        int m_level;

        // the search-range sizes for the hierarchical match
        int m_xr, m_yr;
        
        // the search-range sizes for when hierarchical match fails
        int m_big_xr, m_big_yr;
        
        // the temporal distances to the reference pictures
        int m_tdiff[2];

        // the picture sort - I, L1 or L2
        PictureSort m_psort;

        // list of candidate vectors for checking
        CandidateList m_cand_list;

        // Prediction used for each block. This is derived from neighbouring blocks
        // and is used to control the variation in the motion vector field.
        MVector m_mv_prediction;
        
        // The value used in computing block cost means with a simple recursive filter
        double m_rho;
        
        // The mean of the block cost
        double m_cost_mean;
        
        // The mean of the square of the block cost
        double m_cost_mean_sq;
        
    private:

        // Functions

        //! Make down-converted pictures
        void MakePicHierarchy(const PicArray& data, OneDArray< PicArray* >& down_data);

        //! Make a hierarchy of MvData structures
        void MakeMEDataHierarchy(const OneDArray< PicArray*>& down_data,
                                           OneDArray< MEData* >& me_data_set );

        //! Tidy up the allocations made in building the picture hirearchy
        void TidyPics( OneDArray< PicArray*>& down_data );

        //! Tidy up the allocations made in building the MV data hirearchy
        void TidyMEData( OneDArray< MEData*>& me_data_set );

        //! Match the picture data 
        void MatchPic(const PicArray& ref_data , const PicArray& pic_data , MEData& me_data ,
                      const MvData& guide_data, const int ref_id);

        //! Do a given block
        void DoBlock(const int xpos, const int ypos , 
                     const MvArray& guide_array,
                     BlockMatcher& block_match);

    };

} // namespace dirac

#endif
