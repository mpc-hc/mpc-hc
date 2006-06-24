/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: frame_decompress.h 280 2005-01-30 05:11:46Z gabest $ $Name$
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



#ifndef _FRAME_DECOMPRESS_H_
#define _FRAME_DECOMPRESS_H_

#include "libdirac_common/frame_buffer.h"
#include "libdirac_common/common.h"

namespace dirac
{
    //! Compress a single image frame
    /*!
        This class decompresses a single frame at a time, using parameters
        supplied at its construction. FrameDecompressor is used by
        SequenceDecompressor.
    */
    class FrameDecompressor{
    public:
        //! Constructor
        /*!
            Creates a FrameDecompressor with specific set of parameters the
            control the decompression process. It decodes motion data before
            decoding each component of the frame.

            \param  decp    decoder parameters
            \param  cf      the chroma format of the frame being decompressed
        */
        FrameDecompressor(DecoderParams& decp, ChromaFormat cf);

        //! Destructor
        /*!
            Releases resources. 
        */
        ~FrameDecompressor();

        //! Decompress the next frame into the buffer
        /*!
            Decompresses the next frame from the stream and place at the end
            of a frame buffer.
            Returns true if able to decode successfully, false otherwise

            \param my_buffer   picture buffer into which the frame is placed
        */
        bool Decompress(FrameBuffer& my_buffer);

        //! Reads the header data
        /*!
            Reads the header data associated with decompressing the frame
            \param my_buffer picture buffer from which frame dimensions are obtained
        */
        bool ReadFrameHeader(const FrameBuffer& my_buffer);

        //! Returns the frame parameters of the current frame being decoded
        const FrameParams& GetFrameParams() const{ return m_fparams; }

    private:
        //! Copy constructor is private and body-less
        /*!
            Copy constructor is private and body-less. This class should not be copied.

        */
        FrameDecompressor(const FrameDecompressor& cpy);

        //! Assignment = is private and body-less
        /*!
            Assignment = is private and body-less. This class should not be
            assigned.
        */
        FrameDecompressor& operator=(const FrameDecompressor& rhs);

        //! Decodes component data    
        void CompDecompress(FrameBuffer& my_buffer,int fnum, CompSort cs);

        //! Reads the header data associated with decompressing the frame
        bool ReadFrameHeader(FrameParams& fparams);

        //Member variables    

        //! Parameters for the decompression, as provided in constructor
        DecoderParams& m_decparams;

        //! Chroma format of the frame being decompressed
        ChromaFormat m_cformat;

        //! An indicator which is true if the frame has been skipped, false otherwise
        bool m_skipped;

        //! An indicator that is true if we use global motion vectors, false otherwise
        bool m_use_global;

        //! An indicator that is true if we use block motion vectors, false otherwise
        bool m_use_block_mv;

        //! Prediction mode to use if we only have global motion vectors
        PredMode m_global_pred_mode;

        //! Current Frame Parameters
        FrameParams m_fparams;

        //! Read header successfully
        bool m_read_header;

    };

} // namespace dirac

#endif
