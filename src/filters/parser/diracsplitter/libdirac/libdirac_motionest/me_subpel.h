/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: me_subpel.h 280 2005-01-30 05:11:46Z gabest $ $Name$
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

#ifndef _ME_SUBPEL_H_
#define _ME_SUBPEL_H_

#include <libdirac_common/common.h>
#include <libdirac_common/motion.h>
#include <libdirac_motionest/block_match.h>
namespace dirac
{

    class FrameBuffer;
    class MvData;
    class PicArray;
    
    //! The SubpelRefine class takes pixel-accurate motion vectors and refines them to 1/8-pixel accuracy
    /*!
        The SubpelRefine class takes pixel-accurate motion vectors and refines
        them to 1/8-pixel accuracy. It uses references upconverted by a factor
        of 2 in each dimension, with the remaining precision gained by doing
        linear interpolation between values on-the-fly.
     */
    class SubpelRefine
    {
    
    public:
        //! Constructor
        /*!
            The constructor initialises the encoder parameters.
            \param    cp    the parameters used for controlling encoding
         */
        SubpelRefine(const EncoderParams& cp);
    
        //! Destructor
        ~SubpelRefine(){}
    
        //! Does the actual sub-pixel refinement
        /*!
            Does the actual sub-pixel refinement.
            \param    my_buffer    the buffer of pictures being used
            \param    frame_num    the frame number on which motion estimation is being performed
            \param    me_data    the motion vector data, into which the results will be written
         */
        void DoSubpel( const FrameBuffer& my_buffer , int frame_num , MEData& me_data );
    
    private:
        //! Private, body-less copy constructor: this class should not be copied
        SubpelRefine( const SubpelRefine& cpy );
    
        //! Private, body-less assignment=: this class should not be assigned
        SubpelRefine& operator=( const SubpelRefine& rhs );
    
        //! Match a picture from its (upconverted) reference, and record the block mvs
        void MatchPic(const PicArray& pic_data , const PicArray& refup_data , MEData& me_data ,
                                 int ref_id);
    
        //! Match an individual block
        void DoBlock( const int xblock , const int yblock , 
                      BlockMatcher& my_bmatch, MEData& me_data , const int ref_id );
    
        //! Get a prediction for a block MV from the neighbouring blocks
        MVector GetPred( int xblock , int yblock , const MvArray& mvarray );
    
        //member variables
    
        //! A local reference to the encoder params
        const EncoderParams& m_encparams;
    
        //! The list of candidate vectors being tested
        CandidateList m_cand_list;
    
        //! The relative coords of the set of neighbours used to generate MV predictions
        OneDArray<ImageCoords> m_nshift;
    
    
    
    };

} // namespace dirac

#endif
