/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: pic_io.h 280 2005-01-30 05:11:46Z gabest $ $Name$
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
*                 Scott Robert Ladd,
*                 Stuart Cunningham,
*                 Tim Borer,
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

#ifndef _PIC_IO_H_
#define _PIC_IO_H_

#include <iostream>
#include <fstream>
#include <streambuf>

#include <libdirac_common/common.h>
#include <libdirac_common/frame.h>

namespace dirac
{
    //////////////////////////////////////////
    //--------------------------------------//
    //-                                    -//
    //-Uncompressed picture file IO wrapper-//
    //-                                    -//
    //--------------------------------------//
    //////////////////////////////////////////

    // Stream classes for writing/reading frames of uncompressed/decoded data
    // to stream. Streams currently supported are Memory based streams and
    // File based streams. These classes need further restructuring. 
    // Anu - 19-11-2004

    // Subclass these to provide functionality for different file formats and 
    // for streaming.


    //! Class for outputting pictures


    /*!
        Abstract base class for classes that output frames to stream
    */
    class StreamPicOutput
    {
        public:

            //! Default Constructor
            StreamPicOutput();
            //! Constructor
            /*!
                Constructor, takes
                \param sp the sequence parameters
             */  
            StreamPicOutput( const SeqParams& sp);

            //! virtual Destructor
            virtual ~StreamPicOutput();

            //! Write the next frame to the output
            virtual bool WriteNextFrame(const Frame& myframe);

            //! Get the sequence parameters 
            SeqParams& GetSeqParams() {return m_sparams;}

        protected:

            //! Sequence parameters
            SeqParams m_sparams;
            //! Output stream
            std::ostream* m_op_pic_ptr;

            //! Write a component to file
            virtual bool WriteComponent(const PicArray& pic_data, 
                                        const CompSort& cs);
    };

    /*!
        Outputs pictures to a memory buffer
    */
    class MemoryStreamOutput : public StreamPicOutput
    {
        public:
            //! Default Constructor
            MemoryStreamOutput();

            //! Destructor
            ~MemoryStreamOutput();

            //! Set sequence parameters
            void SetSequenceParams ( SeqParams &sparams)
            { m_sparams = sparams; }

            //! Set the memory buffer to write the data to
            void SetMembufReference (unsigned char *buf, int buf_size);

            //! Returns true if we're at the end of the input, false otherwise
            bool End() const ;

        protected:
            //! Body-less copy constructor
            MemoryStreamOutput(const MemoryStreamOutput&);
            //! Body-less assignment operator
            MemoryStreamOutput & operator =(const MemoryStreamOutput&);

        protected:

            //! local memory buffer
            class OutputMemoryBuffer : public std::streambuf
            {
            public:
                //! Memory buffer constructor
                OutputMemoryBuffer () : 
                m_op_buf(0), 
                m_op_buf_size(0), 
                m_op_idx(0)
                {}

                //! Set the buffer variables
                /*! Set the memory buffer variables
                    \param   buffer      buffer to write data to
                    \param   buffer_size size of output buffer
                */
                void SetMembufReference (unsigned char *buffer, int buffer_size)
                {
                    m_op_buf = buffer;
                    m_op_buf_size = buffer_size;
                    m_op_idx = 0;
                }

            protected:
                //! Memory buffer to write data to
                unsigned char *m_op_buf;
                //! Memory buffer size
                int m_op_buf_size;
                //! Index of first available byte in buffer
                int m_op_idx;

                //! Write Overflow method to write one char at a time 
                virtual int overflow (int c)
                {
                    if ( c != EOF)
                    {
                        if (m_op_idx == m_op_buf_size)
                            return EOF;

                        m_op_buf[m_op_idx] = (char)c;
                        m_op_idx++;
                    }
                    return c;
                }

                //! xsputn method to write one multiple chars at a time to buffer
                virtual std::streamsize xsputn (const char *s, 
                                            std::streamsize num)
                {
                    std::streamsize bytes_left = m_op_buf_size - m_op_idx;
                    std::streamsize bytes_written = bytes_left > num 
                                                        ? num : bytes_left;
                    memcpy (&m_op_buf[m_op_idx], (unsigned char *)s, 
                            bytes_written);
                    m_op_idx += bytes_written;
                    return bytes_written;
                }

            private:
                //! Body-less copy constructor
                OutputMemoryBuffer(const OutputMemoryBuffer&);
                //! Body-less assignment operator
                OutputMemoryBuffer& operator =(const OutputMemoryBuffer&);
            };

            //! Output stream Memory buffer
            OutputMemoryBuffer m_membuf;
    };

    /*!
        Outputs pictures to a file
    */
    class FileStreamOutput : public StreamPicOutput
    {
        public:

            //! Constructor
            /*!
                Constructor, takes
                \param output_name the name of the output file
                \param sp the sequence parameters
                \param write_header_only optionally write only the header
             */  
            FileStreamOutput (const char* output_name,
              const SeqParams& sp,
              bool write_header_only = false);

            //! Destructor
            virtual ~FileStreamOutput ();

            //! Write the picture sequence header
            virtual bool WritePicHeader();

        protected:

            //! Header output stream
            std::ofstream* m_op_head_ptr;

            //! Open picture's header file for output
            virtual bool OpenHeader(const char* output_name);

            //! Open picture's YUV data file for output
            virtual bool OpenYUV(const char* output_name);
    };

    //! Picture input class
    /*!
        Abstract Class for reading picture data from a stream.
     */

    class StreamPicInput
    {
        public:

            //! Default Constructor
            StreamPicInput();
            //! Constructor
            /*!
                Constructor, takes
                \param ip_pic_ptr input stream to read from
                \param sparams    Sequence parameters
             */
            StreamPicInput(std::istream *ip_pic_ptr, const SeqParams& sparams);

            //! Destructor
            virtual ~StreamPicInput();

            //! Skip n frames of input
            virtual void Skip( const int n) = 0;

            //! Set padding values to take into account block and transform sizes
            void SetPadding(const int xpd, const int ypd);

            //! Read the next frame from the file
            virtual bool ReadNextFrame(Frame& myframe);

            //! Get the sequence parameters (got from the picture header)
            const SeqParams& GetSeqParams() const {return m_sparams;}

            //! Returns true if we're at the end of the input, false otherwise
            bool End() const ;

        protected:

            //! Sequence parameters
            SeqParams m_sparams;

            //! Input stream
            std::istream* m_ip_pic_ptr;

            //!padding values
            int m_xpad,m_ypad;

            //! Read a component from the file
            virtual bool ReadComponent(PicArray& pic_data,const CompSort& cs);
    };

    /*!
        Class for reading picture data from memory
     */
    class MemoryStreamInput : public StreamPicInput
    {
        public:
            //! Default constructor
            MemoryStreamInput();

            //! Destructor
            ~MemoryStreamInput();

            //! Set the seqence parameters
            void SetSequenceParams ( SeqParams &sparams)
            { m_sparams = sparams; }

            //! Set Memory buffer
            /*! Set the input memory buffer variables
                \param    buf      Input Buffer to read data from
                \param    buf_size Input buffer size
            */
            void SetMembufReference (unsigned char *buf, int buf_size);

            //! Returns true if we're at the end of the input, false otherwise
            bool End() const ;

            //! Skip n frame of input. Unimplemented to this class
            virtual void Skip( const int n);

        protected:
            //! Body-less copy constructor
            MemoryStreamInput(const MemoryStreamInput&);
            //! Body-less assignment operator
            MemoryStreamInput & operator =(const MemoryStreamInput&);

        protected:
            //! Class that defines the Input Stream Memory Buffer
            class InputMemoryBuffer : public std::streambuf
            {
            public:
                //! Constructor
                InputMemoryBuffer() : m_buffer(0), m_buffer_size(0)
                {
                    setg ((char *)m_buffer, (char *)m_buffer, (char *)m_buffer);
                }

                //! Destructor
                ~InputMemoryBuffer(){}

                //! Set Input Memory buffer variables
                /*! Initialises the input memory buffer vars
                    \param   buffer       Input memory buffer
                    \param   buffer_size  Input memory buffer size
                */
                void SetMembufReference (unsigned char *buffer, int buffer_size)
                {
                    m_buffer = buffer;
                    m_buffer_size = buffer_size;

                    setg ((char *)m_buffer, (char *)m_buffer, 
                                (char *)(m_buffer + buffer_size));
                }

            private:
                //! Body-less copy constructor
                InputMemoryBuffer (const InputMemoryBuffer& inbuf);
                //! Body-less assignment operator
                InputMemoryBuffer& operator = (const InputMemoryBuffer& inbuf);

                //! Input memory buffer
                unsigned char *m_buffer;
                //! Input memory buffer size
                int m_buffer_size;
            };

            //! Input stream buffer
            InputMemoryBuffer m_membuf;
    };

    //! Picture input class
    /*!
        Class for reading picture data from a file.
     */
    class FileStreamInput : public StreamPicInput
    {
        public:

            //! Constructor
            /*!
                Constructor, takes
                \param input_name the name of the input picture file
             */
            FileStreamInput (const char* input_name);

            //! Destructor
            virtual ~FileStreamInput ();

            //! Read the picture header
            virtual bool ReadPicHeader();

            //! Skip n frames of input
            virtual void Skip( const int n);


        protected:
            //! input header stream
            std::ifstream* m_ip_head_ptr;
    };

} // namespace dirac

#endif
