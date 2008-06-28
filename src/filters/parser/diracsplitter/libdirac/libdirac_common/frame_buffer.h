/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: frame_buffer.h,v 1.19 2007/11/16 04:50:08 asuraparaju Exp $ $Name: Dirac_0_9_1 $
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
* Contributor(s): Thomas Davies (Original Author),
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

#ifndef _FRAME_BUFFER_H_
#define _FRAME_BUFFER_H_

#include <vector>
#include <map>
#include <libdirac_common/frame.h>
#include <libdirac_common/common.h>

namespace dirac
{
    //! Holds frames both for reference and to overcome reordering delay
    /*!
        The buffer holds frames in a stack to overcome both reordering due to
        bi-directional prediction and use as references for subsequence motion
        estimation. Frames, and components of frames, can be accessed by their
        frame numbers. GOP parameters can be included in the constructors so
        that frames can be given types (I frame, L1 frame or L2 frame) on
        being pushed onto the stack; alternatively, these parameters can be
        overridden.
    */
    class FrameBuffer{
    public:
        //! Default Constructor
        FrameBuffer();

        //! Constructor
        /*!
            Creates a FrameBuffer using the chroma format. Suitable for
            compressing when there are no L2 frames, or when the temporal
            prediction structure is to be determined on the fly.

            \param   cf    the Chroma format of frames in the buffer
            \param   orig_xlen  the original luma width of frames in the buffer
            \param   orig_ylen  the original luma height of frames in the buffer
            \param   dwt_xlen   the padded luma width of frames in the buffer
            \param   dwt_ylen   the padded luma height of frames in the buffer
            \param   dwt_cxlen  the padded chroma width of frames in the buffer
            \param   dwt_cylen  the padded chroma height of frames in the buffer
            \param   luma_depth the video depth of the luma comp in the buffer
            \param   chroma_depth the video depth of the chroma comp in the buffer
            \param   using_ac   True if using Arithmetic coding to code coefficient data

        */
        FrameBuffer(ChromaFormat cf,
                    const int orig_xlen,
                    const int orig_ylen,
                    const int dwt_xlen,
                    const int dwt_ylen,
                    const int dwt_cxlen,
                    const int dwt_cylen,
                    const unsigned int luma_depth,
                    const unsigned int chroma_depth,
                    bool using_ac);

        //! Constructor
        /*!
            Creates a FrameBuffer using the chroma format, the number of L1
            frames between I frames and the separation in frames between L1
            frames. Suitable for compressing when there is a full GOP structure
            or when the temporal prediction structure is to be determined on
            the fly.

            \param  cf    the Chroma format of frames in the buffer
            \param  numL1    the number of Layer 1 frames before the next I frame. 0 means that there is only one I frame.
            \param  L1sep    the number of Layer 2 frames between Layer 1 frames
            \param  orig_xlen  the original luma width of frames in the buffer
            \param  orig_ylen  the original luma height of frames in the buffer
            \param  dwt_xlen   the padded luma width of frames in the buffer
            \param  dwt_ylen   the padded luma height of frames in the buffer
            \param  dwt_cxlen  the padded chroma width of frames in the buffer
            \param  dwt_cylen  the padded chroma height of frames in the buffer
            \param   luma_depth the video depth of the luma comp in the buffer
            \param   chroma_depth the video depth of the chroma comp in the buffer
            \param   interlace Set true if material is being coded in interlaced mode
            \param   using_ac   True if using Arithmetic coding to code coefficient data
        */
        FrameBuffer(ChromaFormat cf,
                    const int numL1,
                    const int L1sep,
                    const int orig_xlen,
                    const int orig_ylen,
                    const int dwt_xlen,
                    const int dwt_ylen,
                    const int dwt_cxlen,
                    const int dwt_cylen,
                    const unsigned int luma_depth,
                    const unsigned int chroma_depth,
                    bool interlace,
                    bool using_ac);

        //! Copy constructor
        /*!
            Copy constructor. Removes the current contents of the frame buffer
            and copies in the contents of the initialising buffer.
        */
        FrameBuffer(const FrameBuffer& cpy);

        //! Operator=.
        /*!
            Operator=. Assigns all elements of the rhs to the lhs.
        */
        FrameBuffer& operator=(const FrameBuffer& rhs);

        //! Destructor
        ~FrameBuffer();

        //! Get frame with a given frame number (NOT with a given position in the buffer)
        Frame& GetFrame(const unsigned int fnum );

        //! Get frame with a given frame number (NOT with a given position in the buffer)
        const Frame& GetFrame(const unsigned int fnum) const;

        //! Get frame with a given frame number, setting a flag to true if it's there
        Frame& GetFrame(const unsigned int fnum, bool& is_present);

        //! Get frame with a given frame number, setting a flag to true if it's there
        const Frame& GetFrame(const unsigned int fnum, bool& is_present) const;

        //! Return true if frame with the particular frame number is available else return false
        bool IsFrameAvail(const unsigned int fnum) const;

        //! Get component with a given component sort and frame number (NOT with a given position in the buffer)
        PicArray& GetComponent(const unsigned int frame_num, CompSort c);

        //! Get component with a given component sort and frame number (NOT with a given position in the buffer)
        const PicArray& GetComponent(const unsigned int frame_num, CompSort c) const;

        //! Get upconverted component with a given component sort and frame number (NOT with a given position in the buffer)
        PicArray& GetUpComponent(const unsigned int frame_num, CompSort c);

        //! Get upconverted component with a given component sort and frame number (NOT with a given position in the buffer)
        const PicArray& GetUpComponent(const unsigned int frame_num, CompSort c) const;

        //! Returns a list of member frames
        std::vector<int> Members() const;

        //! Put a new frame into the top of the buffer
        /*!
            Put a new frame into the top of the buffer. Frame parameters
            associated with the frame will be the built-in parameters for the
            buffer.

            \param    frame_num    the number of the frame being inserted
        */
        void PushFrame(const unsigned int frame_num);

        //! Put a new frame into the top of the buffer
        /*!
            Put a new frame into the top of the buffer. Frame parameters
            associated with the frame will be as given by the frame parameter
            object.
        */
        void PushFrame(const FrameParams& fp);

        //! Put a copy of a new frame into the top of the buffer
        /*!
            Put a copy of a new frame into the top of the buffer.
        */
        void PushFrame( const Frame& frame );

        //! Sets the reference frame number that will be cleaned
        /*!
            Indicate which frame which has been output and which is no longer
            required for reference. Expiry times are set in each frame's
            frame parameters.
            \param show_fnum             frame number in display order that can be output
            \param current_coded_fnum    frame number in display order of frame currently being coded
        */
        void SetRetiredFrameNum(const int show_fnum, const int current_coded_fnum);

        //! Delete all expired frames
        /*!
            Delete frames which have been output and which are no longer
            required for reference. Expiry times are set in each frame's
            frame parameters.
            \param show_fnum             frame number in display order that can be output
            \param current_coded_fnum    frame number in display order of frame currently being coded
        */
        void CleanAll(const int show_fnum, const int current_coded_fnum);

        //! Delete retired reference frames and expired non-ref frames
        /*!
            Delete frames which have been output and retired reference frames.
            Expiry times are set in each frame's frame parameters.
            \param show_fnum             frame number in display order that can be output
            \param current_coded_fnum    frame number in display order of frame currently being coded
        */
        void CleanRetired(const int show_fnum, const int current_coded_fnum);

        //! Delete frame
        /*!
            Delete frame.
            \param fnum             frame number in display order to be deleted from frame buffer
        */
        void Clean(int fnum);

        //! Return the default frame parameters
        const FrameParams& GetFParams() const{return m_fparams;}

        //! Returnthe default frame parameters
        FrameParams& GetFParams() { return m_fparams; }

        //! Set the frame parameters based on the frame number in display order and internal GOP parameters
        void SetFrameParams(const unsigned int fnum);

    private:
        //! Remove a frame with a given frame number from the buffer
        /*!
            Remove a frame with a given frame number (in display order) from
            the buffer. Searches through the buffer and removes frame(s) with
            that number.
        */
        void Remove(const unsigned int fnum);

        //! Set the frame parameters for a progressive frame based on the frame number in display order and internal GOP parameters
        void SetProgressiveFrameParams(const unsigned int fnum);

        //! Set the frame parameters for an interlaced frame based on the frame number in display order and internal GOP parameters
        void SetInterlacedFrameParams(const unsigned int fnum);

    private:

        //! the count of the number of reference frames in the buffer
        int m_ref_count;

        //! the buffer storing all the values
        std::vector<Frame*> m_frame_data;

        //! the flags that specifies if the frame is currently in use or not
        std::vector<bool> m_frame_in_use;

        //!the map from frame numbers to position in the buffer
        std::map<unsigned int,unsigned int> m_fnum_map;

        //! The frame parameters to use as a default if none are supplied with the frame
        FrameParams m_fparams;

        //! The number of L1 frames before next I frame
        unsigned int m_num_L1;

        //! The distance, in frames, between L1 frames
        unsigned int m_L1_sep;

        //! The length of the group of pictures (GOP)
        unsigned int m_gop_len;

        //! Interlaced coding
        bool m_interlace;

        //! Arithmetic coding flag to code coefficients
        bool m_using_ac;




    };

} // namespace dirac

#endif
