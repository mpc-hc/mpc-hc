/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: me_mode_decn.h,v 1.18 2008/10/01 01:26:47 asuraparaju Exp $ $Name:  $
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

#ifndef _ME_MODE_DECN_H_
#define _ME_MODE_DECN_H_

#include <libdirac_common/motion.h>
#include <libdirac_motionest/block_match.h>

namespace dirac
{
class EncQueue;

//! Decides between superblock and block prediction modes.
/*!
    Loops over all the superblocks and decides on the best modes. A
    superblock is a square of 16 blocks. There are three possible
    splitting levels:
        level 0 means the superblock is considered as a single block;
        level 1 means the superblock is considered as 4 larger blocks,
        termed sub-superblocks;
       level 0 means the superblock is split right down to blocks.

    In deciding which modes
    to adopt, the ModeDecider object calculates costs for all
    permutations, doing motion estimation for the level 1 and level 0
    modes as these have not been calculated before.
    The process of decision for each is as follows. For each SB, we loop
    over the levels, and call DoLevelDecn. DoLevelDecn does motion
    estimation if it's necessary. Then it assumes that we don't have a
    common block mode and calls DoUnitDecn which finds the best mode for
    each unit in the SB at that level, individually. When we've got a
    best cost for that level we go up to the next one.
 */
class ModeDecider
{

public:
    //! Constructor
    /*!
        The constructor creates arrays for handling the motion vector data
        at splitting levels 0 and 1, as motion
        estimation must be performed for these levels.
     */
    ModeDecider(const EncoderParams& encp);

    //! Destructor
    /*!
        The destructor destroys the classes created in the constructor
     */
    ~ModeDecider();

    //! Does the actual mode decision
    /*!
        Does the mode decision
        \param    my_buffer    the buffer of all the relevant frames
        \param    pic_num    the picture number for which motion estimation is being done
     */
    void DoModeDecn(EncQueue& my_buffer , int pic_num);

private:
    ModeDecider(const ModeDecider& cpy);  //private, body-less copy constructor: this class should not be copied
    ModeDecider& operator=(const ModeDecider& rhs);  //private, body-less assignment=: this class should not be assigned

    //functions
    void DoSBDecn();    //called by do_mode_decn for each SB

    //! Make a mode decision given a particular level of decomposition
    void DoLevelDecn(int level);

    //! Decide on a mode for a given prediction unit (block, sub-SB or SB)
    float DoUnitDecn(const int xpos , const int ypos , const int level);

    //! Do motion estimation for a prediction unit at a given level
    void DoME(const int xpos , const int ypos , const int level);

    //! Return a measure of the cost of coding a given mode
    float ModeCost(const int xindex , const int yindex);

    //! Get a prediction for the dc value of a block
    ValueType GetDCPred(int xblock , int yblock);

    //! Get a measure of DC value variance
    float GetDCVar(const ValueType dc_val , const ValueType dc_pred);

    //! Go through all the intra blocks and extract the chroma dc values to be coded
    void SetDC(EncQueue& my_buffer, int pic_num);

    //! Called by previous fn for each component
    void SetDC(const PicArray& pic_data, MEData& me_data, CompSort cs);

    //! Called by previous fn for each block
    ValueType GetBlockDC(const PicArray& pic_data, int xloc, int yloc, int split, CompSort cs);


    // Member data
    PictureSort m_psort;

    //! A local reference to the encoder parameters
    const EncoderParams& m_encparams;

    //! A local pointer to the picture prediction params
    const PicturePredParams* m_predparams;

    //! The Lagrangian parameter for motion estimation
    float m_lambda;

    //! Correction factor for comparing SAD costs for different SB splittings
    OneDArray<float> m_level_factor;


    //! Correction factor for comparing mode costs for different SB splittings
    OneDArray<float> m_mode_factor;

    //! Motion vector data for each level of splitting
    OneDArray< MEData* > m_me_data_set;

    const PicArray* m_pic_data;
    const PicArray* m_ref1_updata;
    const PicArray* m_ref2_updata;
    int num_refs;

    IntraBlockDiff* m_intradiff;
    BiBlockDiff* m_bicheckdiff;

    //position variables, used in all the mode decisions
    int m_xsb_loc, m_ysb_loc;   //coords of the current SB

};

} // namespace dirac

#endif
