/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: mot_comp.h,v 1.18 2008/01/09 10:50:23 asuraparaju Exp $ $Name: Dirac_0_9_1 $
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
* Contributor(s): Richard Felton (Original Author),
*                 Thomas Davies
*                 Anuradha Suraparaju
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

//  Motion Compensation routines.
//  Supports different sizes of blocks as long as the parameters
//     describing them are 'legal'. Blocks overlap the edge of the image
//     being written to but blocks in the reference image are forced to
//     lie completely within the image bounds.

#ifndef _INCLUDED_MOT_COMP
#define _INCLUDED_MOT_COMP

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include <libdirac_common/common.h>
#include <libdirac_common/upconvert.h>
#include <libdirac_common/motion.h>
#include <libdirac_common/frame_buffer.h>

namespace dirac
{
    class FrameBuffer;
    class Frame;

 
    //! Abstract Motion compensator class. 
    /*!
        Motion compensator class, for doing motion compensation with two 
        references and overlapped blocks, using raised-cosine roll-off.
        This is an abstract class. It must be sub-classed and the 
        BlockPixelPred must be defined in the sub-classes.
    */
    class MotionCompensator
    {

    public:
        //! Constructor.
        /*!
            Constructor initialises using codec parameters.
         */
        MotionCompensator( const CodecParams &cp );
        //! Destructor
        virtual ~MotionCompensator();

        //! Convenience function to perform motion compensation on a frame
        /*!
            Static function that motion compensates a frame. It uses the
            MV precision value in the CodecParams to instantiate the 
            appropriate MotionCompensation sub-class.
            \param    cp        Encoder/decoder parameters
            \param    direction whether we're subtracting or adding
            \param    buffer    the FrameBuffer object containing the frame and the reference frames
            \param    fnum    number of frame in the frame buffer to be compensated
    `       \param    mv_data    the motion vector data
         */
        static void CompensateFrame ( const CodecParams &cp, 
                                      const AddOrSub direction , 
                                      FrameBuffer& buffer , 
                                      const int fnum, 
                                      const MvData& mv_data );

        //! Compensate a frame
        /*!
            Perform motion compensated addition/subtraction on a frame using 
            parameters
            \param    direction whether we're subtracting or adding
            \param    fnum    number of frame in the frame buffer to be compensated
            \param    my_buffer    the FrameBuffer object containing the frame and the reference frames
    `       \param    mv_data    the motion vector data
         */
        void CompensateFrame( const AddOrSub direction , 
                              FrameBuffer& my_buffer , 
                              int fnum , 
                              const MvData& mv_data );

    private:
        //private, body-less copy constructor: this class should not be copied
        MotionCompensator( const MotionCompensator& cpy );
        //private, body-less assignment=: this class should not be assigned
        MotionCompensator& operator=( const MotionCompensator& rhs );

        //functions

        //! Motion-compensate a component
        void CompensateComponent( Frame& picframe , 
                                  const Frame& ref1frame , 
                                  const Frame& ref2frame ,
                                  const MvData& mv_data , const CompSort cs);

        //! Recalculate the weight matrix and store other key block related parameters.
        //! DC-compensate an individual block
        void DCBlock( TwoDArray<ValueType> &block_data ,
                      const ValueType dc);
        void ReConfig();

        // Calculates a weighting arrays blocks.
        void CalculateWeights(int xbsep, int ybsep, TwoDArray<ValueType>* wt_array);

        //! Calculates a weighting block.
        /*! 
            Params defines the block parameters so the relevant weighting 
            arrays can be created.  FullX and FullY refer to whether the 
            weight should be adjusted for the edge of an image.  eg. 1D 
            Weighting shapes in x direction
              FullX true        FullX false
                ***           ********
              *     *                  *
             *       *                  *
           *           *                  *
        */
        void CreateBlock(int xbsep, int ybsep, bool FullX, bool FullY, TwoDArray<ValueType>& WeightArray);

        //! Flips the values in an array in the x direction
        void FlipX(const TwoDArray<ValueType>& Original, TwoDArray<ValueType>& Flipped);

        //! Flips the values in an array in the y direction.
        void FlipY(const TwoDArray<ValueType>& Original, TwoDArray<ValueType>& Flipped);

        virtual void CompensateBlock( TwoDArray<ValueType>& pic_data , 
                              const ImageCoords& pos , 
                              const ImageCoords &orig_pic_size,
                              PredMode block_mode,
                              ValueType dc,
                              const PicArray& ref1up_data , 
                              const MVector& mv1 ,
                              const PicArray& ref2up_data , 
                              const MVector& mv2 ,
                              const TwoDArray<ValueType>& Weights );
        //! Predict pixels in a block. Pure virtual. SubClasses need to define it
        virtual void BlockPixelPred( TwoDArray<ValueType>& block_data , 
                              const ImageCoords& pos,
                              const ImageCoords &orig_pic_size,
                              const PicArray& refup_data , 
                              const MVector& mv) = 0;
     
        // Adjust the block value based on reference weights
        /*
        * Adjust the block value based on reference weights of each
        * reference picture.
        * val1_block - Block predicted from a single reference picture
        * val2_block - Block predicted from second reference picture
        *              mode is REF1AND2
        * block_mode  - Block prediction mode.
        *
        * On return, val1_block will contain the weight reference weight 
        * adjusted block values
        */
        void AdjustBlockByRefWeights (TwoDArray<ValueType>& val1_block, 
                                      TwoDArray<ValueType>& val2_block, 
                                      PredMode block_mode);
     
        // Adjust the block value based spatial weighting matrix
        /*
        * Adjust the block value based on spatial weighting matrix
        * val_block - Predicted block
        * pos       - position of top lef corner of block in frame
        * wt_array  - spatial weighting matrix
        *
        * On return, val_block will contain the spatial weight adjusted block 
        * values
        */
        void AdjustBlockBySpatialWeights (TwoDArray<ValueType>& val_block,
                                          const ImageCoords &pos,
                                          const TwoDArray<ValueType> &wt_array);
    protected:
        //variables    

        //! The codec parameters
        CodecParams m_cparams;

        //! The chroma format
        ChromaFormat m_cformat;
        bool luma_or_chroma;    //true if we're doing luma, false if we're coding chroma  
        
        // A marker saying whether we're doing MC addition or subtraction
        AddOrSub m_add_or_sub;                    

        // Block information
        OLBParams m_bparams;
        // Arrays of  block weights
        TwoDArray<ValueType>* m_block_weights;
        // Arrays of super block weights
        TwoDArray<ValueType>* m_macro_block_weights;
        // Arrays of  sub super block weights
        TwoDArray<ValueType>* m_sub_block_weights;
    };

    //! Pixel precision Motion compensator class. 
    class MotionCompensator_Pixel : public MotionCompensator
    {

    public:
        //! Constructor.
        /*!
            Constructor initialises using codec parameters.
         */
        MotionCompensator_Pixel (const CodecParams &cp);

    private:
        //! Motion-compensate a block. 
        virtual void BlockPixelPred( TwoDArray<ValueType>& block_data , 
                              const ImageCoords& pos,
                              const ImageCoords &orig_pic_size,
                              const PicArray& refup_data , 
                              const MVector& mv);
    };

    //! Half Pixel precision Motion compensator class. 
    class MotionCompensator_HalfPixel : public MotionCompensator
    {
    public:
        //! Constructor.
        /*!
            Constructor initialises using codec parameters.
         */
        MotionCompensator_HalfPixel (const CodecParams &cp);
    private:
        //! Motion-compensate a block. 
        virtual void BlockPixelPred( TwoDArray<ValueType>& block_data , 
                              const ImageCoords& pos,
                              const ImageCoords &orig_pic_size,
                              const PicArray& refup_data , 
                              const MVector& mv);
    };

    //! Quarter Pixel precision Motion compensator class. 
    class MotionCompensator_QuarterPixel : public MotionCompensator
    {
    public:
        //! Constructor.
        /*!
            Constructor initialises using codec parameters.
         */
        MotionCompensator_QuarterPixel (const CodecParams &cp);
    private:
        //! Motion-compensate a block. 
        virtual void BlockPixelPred( TwoDArray<ValueType>& block_data , 
                              const ImageCoords& pos,
                              const ImageCoords &orig_pic_size,
                              const PicArray& refup_data , 
                              const MVector& mv);
    };

    //! Eighth Pixel precision Motion compensator class. 
    class MotionCompensator_EighthPixel : public MotionCompensator
    {
    public:
        //! Constructor.
        /*!
            Constructor initialises using codec parameters.
         */
        MotionCompensator_EighthPixel (const CodecParams &cp);
    private:
        //! Motion-compensate a block. 
        virtual void BlockPixelPred( TwoDArray<ValueType>& block_data , 
                              const ImageCoords& pos,
                              const ImageCoords &orig_pic_size,
                              const PicArray& refup_data , 
                              const MVector& mv);
    };


} // namespace dirac

#endif
