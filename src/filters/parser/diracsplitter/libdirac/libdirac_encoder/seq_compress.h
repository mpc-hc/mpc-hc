/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: seq_compress.h,v 1.16 2007/03/21 11:05:43 tjdwave Exp $ $Name: Dirac_0_7_0 $
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
*                 Scott R Ladd,
*                 Anuradha Suraparaju
*                 Andrew Kennedy
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

#ifndef _SEQ_COMPRESS_H_
#define _SEQ_COMPRESS_H_

/////////////////////////////////////////
//-------------------------------------//
//Class to manage compressing sequences//
//-------------------------------------//
/////////////////////////////////////////

#include <libdirac_byteio/dirac_byte_stream.h>
#include <libdirac_common/common.h>
#include <libdirac_common/frame_buffer.h>
#include <libdirac_common/pic_io.h>
#include <libdirac_common/dirac_assertions.h>
#include <libdirac_encoder/quality_monitor.h>
#include <libdirac_encoder/frame_compress.h>
#include <libdirac_encoder/rate_control.h>

#include <fstream>

namespace dirac
{
              
    //! Compresses a sequence of frames from a stream.
    /*!
        This class compresses a sequence of frames, frame by frame. It
        currently uses GOP parameters set in the encoder parameters in order
        to define the temporal prediction structure. A version to incorporate
        non-GOP structures is TBC.
    */
    class SequenceCompressor{
    public:
        //! Constructor
        /*!
            Creates a sequence compressor, and prepares to begin compressing
            with the first frame.Sets up frame padding in the picture input if
            necesary
            \param      pin     an input stream containing a sequence of frames
            \param      srcp    parameters for the input source
            \param      encp    parameters for the encoding process
            \param      dirac_byte_stream Output destination for compressed data
        */
        SequenceCompressor(StreamPicInput* pin, 
                           SourceParams& srcp,
                           EncoderParams& encp,
                           DiracByteStream& dirac_byte_stream);

        //! Destructor
        /*!
            Destructor. Must delete IO objects created by constructor.
        */
        ~SequenceCompressor();

        //! Load data
        /*!
            Load one frame of data into the Sequence Compressor. Sets
            m_all_done to true if no more data is available to be loaded.
            \return             true - if frame load succeeded.
                                false - otherwise
        */
        bool LoadNextFrame();

        //! Compress the next frame in sequence
        /*!
            This function codes the next frame in coding order and returns the
            next frame in display order. In general these will differ, and
            because of re-ordering there is a delay which needs to be imposed.
            This creates problems at the start and at the end of the sequence
            which must be dealt with. At the start we just keep outputting
            frame 0. At the end you will need to loop for longer to get all
            the frames out. It's up to the calling function to do something
            with the decoded frames as they come out -- write them to screen
            or to file, for example.  .
            If coding is fast enough the compressed version could be watched
            real-time (with suitable buffering in the calling function to
            account for encode-time variations).

            NOTE: LoadNextFrame must be called atleast once before invoking this
            method.

            \return           reference to the next locally decoded frame available for display
        */
        Frame &CompressNextFrame();

        //! Return a pointer to the most recent frame encoded
        const Frame *GetFrameEncoded();

        //! Return Motion estimation info related to the most recent frame encoded
        const MEData *GetMEData();

        DiracByteStats EndSequence();

        //! Determine if compression is complete.
        /*!
            Indicates whether or not the last frame in the sequence has been
            compressed.
            \return     true if last frame has been compressed; false if not
        */
        bool Finished(){return m_all_done;}


    private:
        //! Copy constructor is private and body-less
        /*!
            Copy constructor is private and body-less. This class should not
            be copied.
        */
        SequenceCompressor(const SequenceCompressor& cpy);

        //! Assignment = is private and body-less
        /*!
            Assignment = is private and body-less. This class should not be
            assigned..
        */
        SequenceCompressor& operator=(const SequenceCompressor& rhs);

        //! Denoises an input frame
        void Denoise( Frame& frame );

        //! Denoises a component
        void DenoiseComponent( PicArray& pic_data );

        ValueType Median( const ValueType* val_list, const int length);

        /**
        * Writes sequence data to byte stream
        *@param p_accessunit_byteio Output destination
        */
        //SequenceCompressor& operator<<(AccessUnitByteIO *p_accessunit_byteio);

        //! Uses the GOP parameters to convert frame numbers in coded order to display order.
        /*!
             Uses the GOP parameters to convert frame numbers in coded order
             to display order
            \param  fnum  the frame number in coded order
        */
        int CodedToDisplay(const int fnum);

        //! Make a report to screen on the coding results for the whole sequence
        void MakeSequenceReport(); 

        //! Make a report to screen on the coding results for a single frame
        void MakeFrameReport();

        //! Completion flag, returned via the Finished method.
        bool m_all_done;

        //! Flag indicating whether we've just finished.
        /*!
            Flag which is false if we've been all-done for more than one
            frame, true otherwise (so that we can take actions on finishing
            once only).
        */
        bool m_just_finished;

        //! The parameters of the input source
        SourceParams& m_srcparams;

        //! The parameters used for encoding.
        EncoderParams& m_encparams;

        //! Pointer pointing at the picture input.
        StreamPicInput* m_pic_in;

        //! A picture buffer used for local storage of frames whilst pending re-ordering or being used for reference.
        FrameBuffer* m_fbuffer;

        //! A picture buffer of original frames
        FrameBuffer* m_origbuffer;

        //state variables for CompressNextFrame

        //! The number of the current frame to be coded, in display order
        int m_current_display_fnum;

        //! The number of the current frame to be coded, in coded order
        int m_current_code_fnum;

        //! Current AccessUnit frame-number
        int m_current_accessunit_fnum;

        //! The number of the frame which should be output for concurrent display or storage
        int m_show_fnum;

        //! The index, in display order, of the last frame read
        int m_last_frame_read;        

        //! A delay so that we don't display what we haven't coded
        int m_delay;

        //! A class for monitoring the quality of pictures and adjusting parameters appropriately
        QualityMonitor m_qmonitor;
        
        //! A class for monitoring and controlling bit rate
        RateController* m_ratecontrol;

        //! A class to hold the frame compressor object
        FrameCompressor m_fcoder;

        //! Output destination for compressed data in bitstream format
        DiracByteStream& m_dirac_byte_stream;
        
    };

} // namespace dirac

#endif
