/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: dirac_cppparser.h,v 1.8 2008/05/02 06:05:04 asuraparaju Exp $ $Name:  $
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
* Contributor(s): Anuradha Suraparaju (Original Author)
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



#ifndef DIRAC_CPPPARSER_H
#define DIRAC_CPPPARSER_H

#include <istream>
#include <streambuf>
#include <libdirac_decoder/decoder_types.h> //for DecoderState
#include <libdirac_common/common.h>
#include <libdirac_byteio/dirac_byte_stream.h>

namespace dirac
{
    class SequenceDecompressor;
    class Picture;

    //! Input Stream Buffer Class.
    class InputStreamBuffer : public std::streambuf
    {
    public:
        //! Constructor
        InputStreamBuffer ();

        //! Destructor
        ~InputStreamBuffer();

        //! Rewind buffer to start of data
        std::ios::pos_type Rewind();

        //! Seek to position specified by bytes offset from pos
        /*!
            Seek takes
            \param bytes offset in bytes
            \param pos   the position from which the offset is applied
        */
        std::ios::pos_type Seek(std::ios::pos_type bytes,
                                std::ios::seekdir pos = std::ios::cur);

        //! Return the current read position in the buffer
        std::ios::pos_type Tell();

        //! Copy data into buffer
        /*!
            Copy take
            \param start   memory area start
            \param bytes   number of bytes to copy starting from start
        */
        void Copy(char *start, int bytes);

        //! Delete all processed data from buffer
        void PurgeProcessedData();

    private:

        //! Private body-less copy constructor
        InputStreamBuffer (const InputStreamBuffer& inbuf);

        //! Private body-less assignment operator
        InputStreamBuffer& operator = (const InputStreamBuffer& inbuf);

        //! Buffer size
        static const int m_buffer_size = 1232896;

        //! Buffere
        char *m_chunk_buffer;
    };

    //! Dirac Stream Parser Class
    /*!
        This class is a wrapper around the SequenceDecompressor class. The
        Sequence Decompressor class needs a full picture of data to be available
        to decompress a picture successfully.  So, the DiracParser class uses
        the InputStreamBuffer class to store data until a chunk is available
        to be processed and then invokes the SequenceDecompressor functions to
        process data. A chunk of data can be a start of sequence, a picture or
        end of sequence data.  The istream used to instantiate the
        SequenceDecompressor object is created using an InputStreamBuffer
        object which is manipulated the DiracParser. This ensures that data is
        always available for processing by the  SequenceDecompressor object.
    */
    class DiracParser
    {
    public:
        //! Constructor
        /*!
            Constructor takes
            \param verbose boolean flag. Set to true for verbose output
        */
        DiracParser(bool verbose = false );

        //! Destructor
        ~DiracParser();

        //! Adds bytes to encoder
        /*! SetBuffer takes
            \param start   Start of input buffer
            \param end     End of input buffer
        */
        void SetBuffer (char *start, char *end);

        //! Parse the data in internal buffer
        /*!
            Parses the data in the input buffer. This function returns one
            of the following values
            \n STATE_BUFFER        : Not enough data in internal buffer to process
            \n STATE_SEQUENCE      : Start of sequence detected
            \n STATE_PICTURE_AVAIL : Decoded picture available
            \n STATE_SEQUENCE_END  : End of sequence detected
            \n STATE_INVALID       : Invalid stream. Stop further processing
        */
        DecoderState Parse();

        //! Return the parse parameters of the current sequence
        const ParseParams& GetParseParams() const;

        //! Return the source parameters of the current sequence
        const SourceParams& GetSourceParams() const;

        //! Return the picture parameters of the next picture to be decoded
        const PictureParams* GetNextPictureParams() const;

        //! Return the decoded picture
        const Picture* GetNextPicture() const;

        //! Return the coding parameters of the current sequence
        const DecoderParams& GetDecoderParams() const;

    private:

    private:

        //! private body-less copy constructor
        DiracParser (const DiracParser &dp);
        //! private body-less assignement constructor
        DiracParser& operator = (const DiracParser &dp);
        //! Current state of parser
        DecoderState m_state;
        //! Next state the parser will enter
        DecoderState m_next_state;
        //! picture number of last picture decoded in display order
        int m_show_pnum;
        //! Sequence decompressor object
        SequenceDecompressor *m_decomp;
        //! verbose flag
        bool m_verbose;
        //! Byte Stream Buffer
        DiracByteStream m_dirac_byte_stream;
    };

} // namespace dirac

#endif
