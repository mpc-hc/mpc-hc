/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: frame_compress.h 280 2005-01-30 05:11:46Z gabest $ $Name$
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


#ifndef _FRAME_COMPRESS_H_
#define _FRAME_COMPRESS_H_

#include <libdirac_common/frame_buffer.h>
#include <libdirac_common/common.h>
#include <libdirac_common/motion.h>
namespace dirac
{

    class MvData;

    //! Compress a single image frame
    /*!
        This class compresses a single frame at a time, using parameters
        supplied at its construction. FrameCompressor is used by
        SequenceCompressor.
    */
    class FrameCompressor
    {
    public:
        //! Constructor
        /*!
            Creates a FrameEncoder with specific set of parameters the control
            the compression process. It encodes motion data before encoding
            each component of the frame. 
            \param encp encoder parameters
        */
        FrameCompressor( EncoderParams& encp ); 

        //! Destructor
        ~FrameCompressor( );
        //! Compress a specific frame within a group of pictures (GOP)
        /*!
            Compresses a specified frame within a group of pictures. 
            \param fbuffer picture buffer in which the frame resides
            \param orig_buffer the corresponding picture buffer of uncoded originals
            \param fnum      frame number to compress
        */
        void Compress( FrameBuffer& fbuffer , const FrameBuffer& orig_buffer , int fnum );

        //! Returns true if the frame has been skipped rather than coded normally
        bool IsSkipped(){ return m_skipped; }

        //! Returns true if Motion estimation data is available
        bool IsMEDataAvail() const { return m_medata_avail; }

        //! Returns the motion estimation data
        const MEData* GetMEData() const;

    private:
        //! Copy constructor is private and body-less
        /*!
            Copy constructor is private and body-less. This class should not
            be copied.
        */
        FrameCompressor( const FrameCompressor& cpy );

        //! Assignment = is private and body-less
        /*!
            Assignment = is private and body-less. This class should not be
            assigned.
        */
        FrameCompressor& operator=(const FrameCompressor& rhs);

        //! Write the frame compression header
        void WriteFrameHeader(const FrameParams& fparams);

        //member variables
        // a local copy of the encoder params
        EncoderParams& m_encparams;
     
        // Pointer to the motion vector data
        MEData* m_me_data;

        // True if the frame has been skipped, false otherwise
        bool m_skipped;                

        // True if we use global motion vectors, false otherwise
        bool m_use_global;

        // True if we use block motion vectors, false otherwise
        bool m_use_block_mv;
        
        // Prediction mode to use if we only have global motion vectors
        PredMode m_global_pred_mode;
        
        // True if motion estimation data is available
        bool m_medata_avail;
    };

} // namespace dirac

#endif
