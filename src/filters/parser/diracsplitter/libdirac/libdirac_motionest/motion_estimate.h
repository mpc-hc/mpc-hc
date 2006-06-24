/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: motion_estimate.h 280 2005-01-30 05:11:46Z gabest $ $Name$
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


#ifndef _MOTION_ESTIMATE_H_
#define _MOTION_ESTIMATE_H_

#include <libdirac_common/motion.h>
namespace dirac
{

    class FrameBuffer;


    //! Class to handle the whole motion estimation process. 
    /*!
     
     Class to handle the whole motion estimation process, which works in 
     three stages. 

     First a pixel-accurate estimate is formed by looking at the current 
     frame data and the data from the reference frame(s). Motion vectors
     are found for every block.

     Second, these pixel-accurate motion vectors are refined to sub-pixel
     accuracy. This means some sort of upconversion needs to be applied to
     the reference. This can be done by actually upconverting the reference
     to create a bigger picture or by doing some interpolation of values
     on the fly.

     Third, mode decisions have to be made. This means choosing which (if
     any) reference to use for each block, and whether to use the same 
     motion vectors for groups of blocks together. A 2x2 group of blocks is
     called a sub-MB and a 4x4 group of blocks is a MB (Macroblock). All 
     the MV data is organised by MB.
    */
    class MotionEstimator{
    public:
        //! Constructor
        MotionEstimator( const EncoderParams& encp );
        //! Destructor
        ~MotionEstimator(){}

        //! Do the motion estimation
        bool DoME(const FrameBuffer& my_buffer , int frame_num , MEData& me_data);

    private:
        //! Copy constructor: private, body-less - class should not be copied
        MotionEstimator( const MotionEstimator& cpy );

        //! Assignment= : //private, body-less - class should not be assigned
        MotionEstimator& operator=( const MotionEstimator& rhs );

        //! Go through all the intra blocks and extract the chroma dc values to be coded
        void SetChromaDC(const FrameBuffer& my_buffer, int frame_num, MvData& mv_data);

        //! Called by previous fn for each component
        void SetChromaDC(const PicArray& pic_data, MvData& mv_data,CompSort csort);        

        //! Called by previous fn for each block
        ValueType GetChromaBlockDC(const PicArray& pic_data, int xloc,int yloc,int split);

        //! Analyses the ME data and returns true if a cut is detected, false otherwise
        bool IsACut( const MEData& ) const;

        // Member variables

        //! A local reference to the encoder parameters
        const EncoderParams& m_encparams;
    };

} // namespace dirac

#endif
