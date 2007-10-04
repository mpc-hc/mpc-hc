/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: seq_decompress.h,v 1.10 2007/09/03 11:31:43 asuraparaju Exp $ $Name: Dirac_0_8_0 $
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


#ifndef _SEQ_DECOMPRESS_H_
#define _SEQ_DECOMPRESS_H_

///////////////////////////////////////////
//---------------------------------------//
//Class to manage decompressing sequences//
//---------------------------------------//
///////////////////////////////////////////

#include "libdirac_common/common.h"
#include "libdirac_byteio/parseunit_byteio.h"
#include <iostream>

namespace dirac
{
    class FrameBuffer;
    class Frame;
    class FrameDecompressor;

    //! Decompresses a sequence of frames from a stream.
    /*!
        This class decompresses a sequence of frames, frame by frame.
    */
    class SequenceDecompressor{
    public:

        //! Constructor
        /*!
            Initializes the decompressor with an input stream and level of
            output detail.
            \param  parseunit   First access-unit of new sequence
            \param  verbosity   when true, increases the amount of information displayed during decompression
         */
        SequenceDecompressor(ParseUnitByteIO& parseunit, bool verbosity);

        //! Destructor
        /*!
            Closes files and releases resources. 
        */
        ~SequenceDecompressor();

        //! Marks beginning of a new AccessUnit
        /*!
            \param parseunit_byteio AccessUnit info in Dirac-stream format
        */
        void NewAccessUnit(ParseUnitByteIO& parseunit_byteio);


        //! Decompress the next frame in sequence
        /*!
            This function decodes the next frame in coding order and returns
            the next frame in display order. In general these will differ, and
            because of re-ordering there is a delay which needs to be imposed.
            This creates problems at the start and at the end of the sequence
            which must be dealt with. At the start we just keep outputting
            frame 0. At the end you will need to loop for longer to get all
            the frames out. It's up to the calling function to do something
            with the decoded frames as they come out -- write them to screen
            or to file, as required.

            \param p_parseunit_byteio Frame information in Dirac-stream format
            \param  skip skip decoding next frame
            \return      reference to the next locally decoded frame available for display
        */
        Frame& DecompressNextFrame(ParseUnitByteIO* p_parseunit_byteio,
                                   bool skip = false);

        //! Get the next frame available for display
        Frame& GetNextFrame();

        //! Get the next frame parameters
        const FrameParams& GetNextFrameParams() const;
        //! Determine if decompression is complete.
        /*!
            Indicates whether or not the last frame in the sequence has been
            decompressed.
            \return     true if last frame has been compressed; false if not
        */
        bool Finished(); 
        //! Interrogates for parse parameters.
        /*!
            Returns the parse parameters used for this decompression run.

            \return parse parameters.
         */
        ParseParams & GetParseParams() { return m_parse_params; }


        //! Interrogates for source parameters.
        /*!
            Returns the source parameters used for this decompression run.

            \return source parameters.
         */
        SourceParams & GetSourceParams() { return m_srcparams; }


        //! Interrogates for coding parameters.
        /*!
            Returns the decoder parameters used for this decompression run.

            \return decoder parameters.
         */
        DecoderParams & GetDecoderParams() { return m_decparams; }
    private:
        //! Copy constructor is private and body-less
        /*!
            Copy constructor is private and body-less. This class should not
            be copied.

        */
        SequenceDecompressor(const SequenceDecompressor& cpy);

        //! Assignment = is private and body-less
        /*!
            Assignment = is private and body-less. This class should not be
            assigned.

        */
        SequenceDecompressor& operator=(const SequenceDecompressor& rhs);


        //Member variables

        //! Completion flag, returned via the Finished method
        bool m_all_done;
        //! Parameters for the decompression, as provided in constructor
        DecoderParams m_decparams;
        //! The parse parameters obtained from the stream header
        ParseParams m_parse_params;
        //! The source parameters obtained from the stream header
        SourceParams m_srcparams;
        //! A picture buffer used for local storage of frames whilst pending re-ordering or being used for reference.
        FrameBuffer* m_fbuffer;   
        //! Number of the frame in coded order which is to be decoded
        int m_current_code_fnum;        
        //! A delay so that we don't display what we haven't decoded
        int m_delay;                    
        //! Index, in display order, of the last frame read
        int m_last_frame_read;
        //! Index, in display order of the frame to be displayed next - computed from delay and current_code_fnum
        int m_show_fnum;
        //! Frame decompressor object
        FrameDecompressor *m_fdecoder;
        //! Highest frame-num processed - for tracking end-of-sequence
        int m_highest_fnum;
    };

} // namespace dirac

#endif
