/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: pic_io.h,v 1.19 2008/06/19 10:17:17 tjdwave Exp $ $Name:  $
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
#include <libdirac_common/picture.h>

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
            //! Constructor
            /*!
                Constructor, takes
                \param op_ptr the output stream object
                \param sp     the source parameters
             */
            StreamPicOutput( std::ostream* op_ptr, const SourceParams& sp);

            //! virtual Destructor
            virtual ~StreamPicOutput();

            //! Write a picture to the next frame to be output
            virtual bool WriteToNextFrame(const Picture& myframe) = 0;

            //! Get the source parameters
            SourceParams& GetSourceParams() {return m_sparams;}

        protected:
            //! Source parameters
            SourceParams m_sparams;
            //! Output stream
            std::ostream* m_op_pic_ptr;

            //! Body-less default Constructor
            StreamPicOutput();
        private:

    };

    class StreamFrameOutput : public StreamPicOutput
    {
        public:

            /*!
                Constructor, takes
                \param op_ptr the output stream object
                \param sp the source parameters
             */
            StreamFrameOutput( std::ostream *op_ptr, const SourceParams& sp);

            //! virtual Destructor
            virtual ~StreamFrameOutput();

            //! Write the next frame to the output
            bool WriteToNextFrame(const Picture& myframe);

        protected:
            //! Write a frame component to file
            bool WriteFrameComponent(const PicArray& pic_data,
                                     const CompSort& cs);
        private:
            //! Body-less Default Constructor
            StreamFrameOutput();
    };

    class StreamFieldOutput : public StreamPicOutput
    {
        public:
            //! Constructor
            /*!
                Constructor, takes
                \param op_ptr the output stream object
                \param sp the source parameters
             */
            StreamFieldOutput( std::ostream *op_ptr, const SourceParams& sp);

            //! virtual Destructor
            virtual ~StreamFieldOutput();

            //! Write a field to the next frame to be output
            bool WriteToNextFrame(const Picture& myfield);

        protected:
            //! Write a field component to file
            bool WriteFieldComponent(const PicArray& pic_data,
                                     int field_num,
                                     const CompSort& cs);

        private:
            //! Body-less Default Constructor
            StreamFieldOutput();
            unsigned char *m_frame_store;
    };

    /*!
        Outputs pictures to a memory buffer
    */
    class MemoryStreamOutput
    {
        public:
            //! Constructor
            MemoryStreamOutput(SourceParams &sparams, bool interlace);

            //! Destructor
            ~MemoryStreamOutput();

            //! Get source parameters
            SourceParams& GetSourceParams()
            { return m_op_pic_str->GetSourceParams();}

            StreamPicOutput *GetStream() { return m_op_pic_str; }
            //! Set the memory buffer to write the data to
            void SetMembufReference (unsigned char *buf, int buf_size);

        protected:
            //! Body-less default Constructor
            MemoryStreamOutput();
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

        private:
            //! Output stream Memory buffer
            OutputMemoryBuffer m_membuf;
            //! Physical Output stream
            std::ostream* m_op_pic_ptr;
            //! Pic output Stream
            StreamPicOutput *m_op_pic_str;
    };

    /*!
        Outputs pictures to a file
    */
    class FileStreamOutput
    {
        public:

            //! Constructor
            /*!
                Constructor, takes
                \param output_name the name of the output file
                \param sp the source parameters
                \param interlace the output is interlaced
             */
            FileStreamOutput (const char* output_name,
              const SourceParams& sp, bool interlace);

            //! Destructor
            virtual ~FileStreamOutput ();

            StreamPicOutput *GetStream() { return m_op_pic_str; }
        private:
            //! Physical Output stream
            std::ostream* m_op_pic_ptr;
            //! Pic output Stream
            StreamPicOutput *m_op_pic_str;
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
                \param sparams    Source parameters
             */
            StreamPicInput(std::istream *ip_pic_ptr, const SourceParams& sparams);

            //! Destructor
            virtual ~StreamPicInput();

            //! Skip n frames of input
            virtual void Skip( const int n)= 0;

            //! Read the next picture frame/field from the file
            virtual bool ReadNextPicture(Picture& mypic) = 0;

            //! Get the source parameters
            SourceParams& GetSourceParams() const {return m_sparams;}

            //! Returns true if we're at the end of the input, false otherwise
            bool End() const ;

        protected:

            //! Source parameters
            mutable SourceParams m_sparams;

            //! Input stream
            std::istream* m_ip_pic_ptr;

    };

    class StreamFrameInput : public StreamPicInput
    {
        public:

            //! Default Constructor
            StreamFrameInput();
            //! Constructor
            /*!
                Constructor, takes
                \param ip_pic_ptr input stream to read from
                \param sparams    Source parameters
             */
            StreamFrameInput(std::istream *ip_pic_ptr, const SourceParams& sparams);

            //! Destructor
            virtual ~StreamFrameInput();

            //! Skip n frames of input
            virtual void Skip( const int n);

            //! Read the next frame from the file
            virtual bool ReadNextPicture(Picture& myframe);

        private:

            //! Read a Frame component from the file
            bool ReadFrameComponent(PicArray& pic_data,const CompSort& cs);

    };

    class StreamFieldInput : public StreamPicInput
    {
        public:

            //! Default Constructor
            StreamFieldInput();
            //! Constructor
            /*!
                Constructor, takes
                \param ip_pic_ptr input stream to read from
                \param sparams    Source parameters
             */
            StreamFieldInput(std::istream *ip_pic_ptr, const SourceParams& sparams);

            //! Destructor
            virtual ~StreamFieldInput();

            //! Skip n frames of input
            virtual void Skip( const int n);

            //! Read the next field from the file
            virtual bool ReadNextPicture(Picture& myfield);

            //! Read the next frame from the file
            bool ReadNextFrame(Picture& field1, Picture& field2);

        protected:
            //! Read both Field components from the file
            bool ReadFieldComponent(PicArray& pic_data1,
                                    PicArray& pic_data2,
                                    const CompSort& cs);

            //! Read  one Field component from the file
            bool ReadFieldComponent(bool is_field1, PicArray& pic_data,
                                    const CompSort& cs);
    };
    /*!
        Class for reading picture data from memory
     */
    class MemoryStreamInput
    {
        public:
            //! Constructor
            /*! Create a MemoryStreamInput object
                \param  sparams   Source parameters
                \param  field_input Treat input as fields, not frames
            */
            MemoryStreamInput(SourceParams& sparams, bool field_input);

            //! Destructor
            ~MemoryStreamInput();

            SourceParams& GetSourceParams ( )
            { return m_inp_str->GetSourceParams(); }

            //! Set Memory buffer
            /*! Set the input memory buffer variables
                \param    buf      Input Buffer to read data from
                \param    buf_size Input buffer size
            */
            void SetMembufReference (unsigned char *buf, int buf_size);

            //! Return the input stream
            StreamPicInput *GetStream() { return m_inp_str; }
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

        private:
            //! Input stream buffer
            InputMemoryBuffer m_membuf;

            //! Input Stream Object
            StreamPicInput *m_inp_str;

            //! Input stream
            std::istream* m_ip_pic_ptr;
    };

    //! Picture input class
    /*!
        Class for reading picture data from a file.
     */
    class FileStreamInput
    {
        public:

            //! Constructor
            /*!
                Constructor, takes
                \param input_name the name of the input picture file
                \param sparams    the source parameters
                \param interlace  input is treated as interlaced
             */
            FileStreamInput (const char* input_name, const SourceParams &sparams, bool interlace);

            //! Destructor
            virtual ~FileStreamInput ();

            SourceParams& GetSourceParams ( )
            { return m_inp_str->GetSourceParams(); }

            //! Return the input stream
            StreamPicInput *GetStream() { return m_inp_str; }

        private:
            StreamPicInput *m_inp_str;

            //! Input stream
            std::istream* m_ip_pic_ptr;

    };

} // namespace dirac

#endif
