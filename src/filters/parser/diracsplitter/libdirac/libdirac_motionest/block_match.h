/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: block_match.h 280 2005-01-30 05:11:46Z gabest $ $Name$
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

#ifndef _BLOCK_MATCH_H_
#define _BLOCK_MATCH_H_

#include <libdirac_motionest/me_utils.h>
#include <vector>
//handles the business of finding the best block match

namespace dirac
{

    typedef std::vector< std::vector< MVector > > CandidateList;

    //! Add a new motion vector list of neighbours of a vector to the set of lists
    /*
        Add a new motion vector list to the set of lists consisting of the 
        square neighbourhood [mv.x-xr,mv.x+xr] by 
        [mv.y-yr,mv.y+yr]. Vectors that already occur in previous lists are 
        not added.
    */
    void AddNewVlist( CandidateList& vect_list , const MVector& mv , const int xr , const int yr );

    //! Add a new motion vector list to the set of lists for sub-pixel matching
    /*
        Add a new motion vector list to the set of lists consisting of the 
        vectors of the form (mv.x+m*step,mv.y+n*step) where m lies between 
        -xr and xr and n lies between -yr and yr.  Vectors that already occur 
        in previous lists are not added. 
    */
    void AddNewVlist( CandidateList& vect_list , const MVector& mv , const int xr , const int yr , const int step );

    //! Add a new motion vector list of diagnonal neighbours of a vector to the set of lists 
    /*
        Add a new motion vector list to the set of lists consisting of the
        diagonal neighbourhood of height 2yr+1 pixels and width 2xr+1 centred
        on \param mv.
        Vectors that already occur in previous lists are not added.
    */
    void AddNewVlistD( CandidateList& vect_list , const MVector& mv , const int xr, const int yr);

    //! Add a motion vector to the set of motion vector lists
    /*!
        Add a motion vector to the set of motion vector lists, making sure
        it's not a duplicate.
    */
    void AddVect( CandidateList& vect_list , const MVector& mv , const int list_num);

    //!    Get the (absolute) variation between two motion vectors
    /*!
        Return the variation between two motion vectors, computed as the sum
        of absolute differences of their components.
    */
    ValueType GetVar(const MVector& mv1,const MVector& mv2);

    //!    Get the (absolute) variation between a motion vector and a list of motion vectors
    /*!
        Return the variation between a motion vector and a list of motion
        vectos, computed as the sum of absolute differences between the
        components of the vector and the median vector produced by the list of
        vectors
    */
    ValueType GetVar(const std::vector<MVector>& pred_list,const MVector& mv);


    //! Class to do block matching

    // Subsumes FindBestMatch and FindBestMatchSubpel
    class BlockMatcher
    {
    public:
        //! Constructor
        /*!
        Constructor
            \param ref_data the reference picture component
            \param pic_data    the picture being matched
            \param bparams    the (overlapped) block parameters to be used for the matching
            \param mv_array   the array of vectors we're going to write into
            \param cost_array the array of costs we're going to write into

        */
        BlockMatcher( const PicArray& ref_data , 
                      const PicArray& pic_data , 
                      const OLBParams& bparams ,
                      const MvArray& mv_array ,
                      const TwoDArray< MvCostData >& cost_array);

        //! Find the best matching vector from a list of candidates
        /*!
               Find the best matching vector from a list of candidates.
               \param  xpos  the horizontal location of the block being matched
               \param  ypos  the vertical location of the block being matched
               \param  cand_list  the list of candidate vectors
               \param  mv_prediction  the prediction for the motion vector
               \param  lambda  the Lagrangian parameter    
        */
        void FindBestMatch(int xpos , int ypos,
                           const CandidateList& cand_list,
                           const MVector& mv_prediction,
                           float lambda);

        //! Find the best matching vector from a list of candidates, to sub-pixel accuracy (TBC: merge with FindBestMatch)
        /*!
               Find the best matching vector from a list of candidates.
               \param  xpos  the horizontal location of the block being matched
               \param  ypos  the vertical location of the block being matched
               \param  cand_list  the list of candidate vectors
               \param  mv_prediction  the prediction for the motion vector
               \param  lambda  the Lagrangian parameter    
        */
        void FindBestMatchSubp(int xpos, int ypos,
                                const CandidateList& cand_list,
                                const MVector& mv_prediction,
                                float lambda);

    private:
        // Local copies of the picture and reference
        const PicArray& m_pic_data;
        const PicArray& m_ref_data;
        
        // Local copy of the motion vector array being populated
        const MvArray& m_mv_array;
        
        // Local copy of the costs being determined through the matching
        const TwoDArray< MvCostData >& m_cost_array; 

        // Block difference elements. Will choose between them depending 
        // on whether we're at the edge of the picture
        SimpleBlockDiff m_simplediff;
        BChkBlockDiff m_checkdiff;
        SimpleBlockDiffUp m_simplediffup;
        BChkBlockDiffUp m_checkdiffup;

        // The block parameters we're using
        OLBParams m_bparams;

    };

} // namespace dirac
#endif
