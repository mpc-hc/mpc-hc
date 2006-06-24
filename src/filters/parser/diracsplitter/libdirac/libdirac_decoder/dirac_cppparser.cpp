/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: dirac_cppparser.cpp 280 2005-01-30 05:11:46Z gabest $ $Name$
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

#include <sstream>
#include <cstdio>
#include <cstring>
#include <libdirac_common/dirac_assertions.h> 
#include <libdirac_decoder/dirac_cppparser.h>
#include <libdirac_decoder/seq_decompress.h>
#include <libdirac_common/frame.h> 
using namespace dirac;


InputStreamBuffer::InputStreamBuffer()
{
    m_chunk_buffer = new char[m_buffer_size];
    
    setg (m_chunk_buffer,  //beginning of read area
          m_chunk_buffer,  //read position
          m_chunk_buffer); //end position
}

std::ios::pos_type InputStreamBuffer::Rewind()
{
    return Seek(0, std::ios::beg);
}

std::ios::pos_type InputStreamBuffer::Tell()
{
    return gptr() - eback();
}

std::ios::pos_type InputStreamBuffer::Seek(std::ios::pos_type bytes, std::ios::seekdir dir)
{
    char *new_pos;

    switch (dir)
    {
    case std::ios::beg:
        new_pos  = eback() + bytes;
        break;
    case std::ios::end:
        new_pos  = egptr() + bytes;
        break;
    default:
        new_pos  = gptr() + bytes;
        break;
    }
    if (new_pos > egptr() || new_pos < eback())
        return -1;

    setg(eback(), //start of read 
        new_pos, //current read position
        egptr()); //end of stream positon

    return 0;
}

void InputStreamBuffer::Copy(char *start, int bytes)
{
    //std::cerr << "eback=" << m_chunk_buffer - eback() 
     //         << "gptr=" << gptr() -m_chunk_buffer
      //        << "egptr=" << egptr() - m_chunk_buffer << endl;

    int bytes_left = m_buffer_size - (egptr() - m_chunk_buffer);
    if (bytes_left < bytes)
    {
        char *temp =  new char [m_buffer_size + bytes];
        memcpy (temp, m_chunk_buffer, m_buffer_size);
        setg (temp, temp+(gptr()-m_chunk_buffer), temp + (egptr() - m_chunk_buffer));
        delete [] m_chunk_buffer;
        m_chunk_buffer = temp;
    }
    //std::cerr << "eback=" << m_chunk_buffer - eback() 
     //         << "gptr=" << gptr() -m_chunk_buffer
      //        << "egptr=" << egptr() - m_chunk_buffer << endl;

    memcpy (egptr(), start, bytes);
    setg(m_chunk_buffer, gptr(), egptr()+bytes);

    //std::cerr << "eback=" << m_chunk_buffer - eback() 
     //         << "gptr=" << gptr() -m_chunk_buffer
      //        << "egptr=" << egptr() - m_chunk_buffer << endl;
}

void InputStreamBuffer::PurgeProcessedData()
{
    //std::cerr << "eback=" << m_chunk_buffer - eback() 
     //         << "gptr=" << gptr() -m_chunk_buffer
      //        << "egptr=" << egptr() - m_chunk_buffer << endl;

    if (gptr() != m_chunk_buffer)
    {
        memmove (m_chunk_buffer, gptr(), egptr() - gptr());
        setg(m_chunk_buffer, m_chunk_buffer, m_chunk_buffer+(egptr() - gptr()));
    }
    //std::cerr << "eback=" << m_chunk_buffer - eback() 
     //         << "gptr=" << gptr() -m_chunk_buffer
      //        << "egptr=" << egptr() - m_chunk_buffer << endl;
}

InputStreamBuffer::~InputStreamBuffer()
{
    delete [] m_chunk_buffer;
}


DiracParser::DiracParser(bool verbose) : 
    m_state(STATE_BUFFER), 
    m_next_state(STATE_SEQUENCE), 
    m_show_fnum(-1), 
    m_decomp(0), 
    m_skip(false), 
    m_skip_type(L2_frame), 
    m_verbose(verbose),
    m_found_start(false), 
    m_found_end(false), 
    m_shift (0xffffffff)
{
    m_istr = new std::istream(&m_sbuf);
}

DiracParser::~DiracParser()
{
    delete m_istr;
    delete m_decomp;
}

void DiracParser::SetBuffer (char *start, char *end)
{
    TEST (end > start);
    m_sbuf.Copy(start, end - start);

}

DecoderState DiracParser::Parse()
{

    while(true)
    {
        m_state = SeekChunk();
        switch (m_state)
        {
        case STATE_BUFFER:
                return m_state;

        case STATE_SEQUENCE:
            if (m_next_state == m_state)
            {
                if (m_decomp)
                    delete m_decomp;

                m_decomp = new SequenceDecompressor (m_istr, m_verbose);
                if (m_decomp->GetSeqParams().BitstreamVersion() != BITSTREAM_VERSION)
                {
                    std::ostringstream errstr;
                    errstr << "Input Bitstream version " << m_decomp->GetSeqParams().BitstreamVersion() << " supported";
                    REPORTM(false, errstr.str().c_str());
                    return STATE_INVALID;
                }
                InitStateVars();
                return m_state;
            }
            else
                m_state = STATE_BUFFER;
            
            break;
    
        case STATE_PICTURE_START:
            if (m_next_state == m_state)
            {
                m_decomp->ReadNextFrameHeader();
                m_next_state = STATE_PICTURE_DECODE;
                m_sbuf.PurgeProcessedData();
                return m_state;
            }
            else
            {
                m_state = STATE_BUFFER;
            }
            break;

        case STATE_PICTURE_DECODE:
        {
            Frame &my_frame = m_decomp->DecompressNextFrame(m_skip);
            if (m_skip)
            {
                // Go pass start code so that we skip frame
                m_sbuf.Seek(5);
            }
            else
            {
                int framenum_decoded = my_frame.GetFparams().FrameNum();
                if (framenum_decoded != m_show_fnum)
                {
                    m_show_fnum = my_frame.GetFparams().FrameNum();
                    if (m_verbose)
                    {
                        std::cerr << "Frame " << m_show_fnum << " available" << std::endl;
                    }
                    m_state = STATE_PICTURE_AVAIL;
                }
            }
            InitStateVars();
            if (m_state == STATE_PICTURE_AVAIL)
                return m_state;

            break;
        }
        case STATE_SEQUENCE_END:
        {
            //push last frame in sequence out
            m_sbuf.Seek(5);
            Frame &my_frame = m_decomp->DecompressNextFrame(m_skip);
            if (!m_skip)
            {
                if (my_frame.GetFparams().FrameNum() != m_show_fnum)
                {
                    m_show_fnum = my_frame.GetFparams().FrameNum();
                    if (m_verbose)
                    {
                        std::cerr << "Frame " << m_show_fnum << " available" << std::endl;
                    }
                    m_state = STATE_PICTURE_AVAIL;
                    m_next_state = STATE_SEQUENCE_END;
                }
                else
                {
                    InitStateVars();
                }
            }
            else
            {
                InitStateVars();
            }
            return m_state;
                
            break;
        }
        default:
            return STATE_INVALID;
        }
    }
    return m_state;
}

const SeqParams& DiracParser::GetSeqParams() const
{
    return m_decomp->GetSeqParams();
}

const FrameParams& DiracParser::GetNextFrameParams() const
{
    return m_decomp->GetNextFrameParams();
}

const Frame& DiracParser::GetNextFrame() const
{
    return m_decomp->GetNextFrame();
}

const Frame& DiracParser::GetLastFrame() const
{
    return  m_decomp->DecompressNextFrame();
}

void DiracParser::SetSkip(bool skip)
{
    const FrameParams& fparams = m_decomp->GetNextFrameParams();
    // FIXME: need to change this logic once bitstream is finalised. so that
    // we skip to next RAP when an L1 frame is skipped
    if (skip == false)
    {
        if (m_skip_type == L2_frame)
            m_skip = false;

        else if (m_skip_type == L1_frame || m_skip_type == I_frame)
        {
            if (fparams.FSort() == L2_frame || fparams.FSort() == L1_frame)
                m_skip = true;
            else
            {
                m_skip_type = L2_frame;
                m_skip = false;
            }
        }
    }
    else
    {
        m_skip = true;
        if (m_skip_type != fparams.FSort())
        {
            switch (fparams.FSort())
            {
            case L2_frame:
                break;

            case L1_frame:
                if (m_skip_type != I_frame)
                    m_skip_type = L1_frame;
                break;
            case I_frame:
                m_skip_type = I_frame;
                break;

            default:
                dirac_ASSERTM(false, "Frame type must be I or L1 or L2");
                break;
            }
        }
    }
}


DecoderState DiracParser::SeekChunk()
{
    char byte;
    if (!m_found_start)
    {
        while (m_sbuf.sgetn(&byte, 1))
        {
            //Find start of next chunk to be processed
            if (m_shift == START_CODE_PREFIX)
            {
                switch ((unsigned char)byte)
                {
                case NOT_START_CODE:
                    m_shift = 0xffffffff;
                    continue;

                case RAP_START_CODE:
                    m_next_state = STATE_SEQUENCE;
                    break;

                case IFRAME_START_CODE:
                case L1FRAME_START_CODE:
                case L2FRAME_START_CODE:
                    m_next_state = STATE_PICTURE_START;
                    break;

                case SEQ_END_CODE:
                    m_next_state = STATE_SEQUENCE_END;
                    break;
                default:
                    dirac_ASSERTM (false, "Should never have reached here!!!");
                    break;
                }
                m_found_start = true;
                m_sbuf.Seek(-5);
                m_sbuf.PurgeProcessedData();
                m_sbuf.Seek(5);
                m_shift = 0xffffffff;
                break;
            }
            m_shift = (m_shift << 8) | byte;
        }

        if (!m_found_start)
        {
            m_next_state =  STATE_BUFFER;
        }
    }

    if (m_found_start && !m_found_end && m_next_state != STATE_SEQUENCE_END)
    {
        while (m_sbuf.sgetn(&byte, 1))
        {
            //Find start of next chunk to be processed
            if (m_shift == START_CODE_PREFIX)
            {
                switch ((unsigned char)byte)
                {
                case NOT_START_CODE:
                    m_shift = 0xffffffff;
                    continue;

                case RAP_START_CODE:
                    break;

                case IFRAME_START_CODE:
                case L1FRAME_START_CODE:
                case L2FRAME_START_CODE:
                    break;

                case SEQ_END_CODE:
                    break;

                default:
                    dirac_ASSERTM (false, "Should never have reached here!!!");
                    break;
                }
                m_found_end = true;
                break;

            }
            m_shift = (m_shift << 8) | byte;
        } 

        if (!m_found_end)
        {
            if (m_next_state != STATE_SEQUENCE_END)
                return STATE_BUFFER;
        }
    }

    if (m_found_start && m_found_end)
    {
        m_sbuf.Rewind();
        m_shift = 0xffffffff;
    }
    return m_next_state;
}

void DiracParser::InitStateVars()
{
    m_shift = 0xffffffff;
    m_found_start = false;
    m_found_end = false;
}
