/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: dirac_cppparser.cpp,v 1.13 2008/05/02 06:05:04 asuraparaju Exp $ $Name:  $
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
* Contributor(s): Anuradha Suraparaju (Original Author),
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

#include <sstream>
#include <cstdio>
#include <cstring>
#include <libdirac_common/dirac_assertions.h>
#include <libdirac_decoder/dirac_cppparser.h>
#include <libdirac_decoder/seq_decompress.h>
#include <libdirac_common/picture.h>
#include <libdirac_byteio/parseunit_byteio.h>
#include <sstream>
using namespace dirac;


InputStreamBuffer::InputStreamBuffer()
{
    m_chunk_buffer = new char[m_buffer_size];

    setg(m_chunk_buffer,   //beginning of read area
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

    switch(dir)
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
    if(new_pos > egptr() || new_pos < eback())
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
    if(bytes_left < bytes)
    {
        char *temp =  new char [m_buffer_size + bytes];
        memcpy(temp, m_chunk_buffer, m_buffer_size);
        setg(temp, temp + (gptr() - m_chunk_buffer), temp + (egptr() - m_chunk_buffer));
        delete [] m_chunk_buffer;
        m_chunk_buffer = temp;
    }
    //std::cerr << "eback=" << m_chunk_buffer - eback()
    //         << "gptr=" << gptr() -m_chunk_buffer
    //        << "egptr=" << egptr() - m_chunk_buffer << endl;

    memcpy(egptr(), start, bytes);
    setg(m_chunk_buffer, gptr(), egptr() + bytes);

    //std::cerr << "eback=" << m_chunk_buffer - eback()
    //         << "gptr=" << gptr() -m_chunk_buffer
    //        << "egptr=" << egptr() - m_chunk_buffer << endl;
}

void InputStreamBuffer::PurgeProcessedData()
{
    //std::cerr << "eback=" << m_chunk_buffer - eback()
    //         << "gptr=" << gptr() -m_chunk_buffer
    //        << "egptr=" << egptr() - m_chunk_buffer << endl;

    if(gptr() != m_chunk_buffer)
    {
        memmove(m_chunk_buffer, gptr(), egptr() - gptr());
        setg(m_chunk_buffer, m_chunk_buffer, m_chunk_buffer + (egptr() - gptr()));
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
    m_show_pnum(-1),
    m_decomp(0),
    m_verbose(verbose)
{



}

DiracParser::~DiracParser()
{
    delete m_decomp;
}

void DiracParser::SetBuffer(char *start, char *end)
{
    TEST(end > start);
    m_dirac_byte_stream.AddBytes(start, end - start);
}

DecoderState DiracParser::Parse()
{
    while(true)
    {
        ParseUnitByteIO *p_parse_unit = NULL;
        ParseUnitType pu_type = PU_UNDEFINED;

        // look for end-of-sequence flag
        if(m_next_state == STATE_SEQUENCE_END)
        {
            if(!m_decomp)
                return STATE_BUFFER;

            // look to see if all pictures have been processed
            if(m_decomp->Finished())
            {
                // if so....delete
                delete m_decomp;
                m_decomp = NULL;
                m_next_state = STATE_BUFFER;
                return STATE_SEQUENCE_END;
            }
            else
                // otherwise....get remaining pictures from buffer
                pu_type = PU_CORE_PICTURE;
        }

        // get next parse unit from stream
        if(m_next_state != STATE_SEQUENCE_END)
        {
            p_parse_unit = m_dirac_byte_stream.GetNextParseUnit();
            if(p_parse_unit == NULL)
                return STATE_BUFFER;
            pu_type = p_parse_unit->GetType();
        }

        switch(pu_type)
        {
        case PU_SEQ_HEADER:

            if(!m_decomp)
            {
                m_decomp = new SequenceDecompressor(*p_parse_unit, m_verbose);
                m_next_state = STATE_BUFFER;
                return STATE_SEQUENCE;
            }

            m_decomp->NewAccessUnit(*p_parse_unit);
            break;

        case PU_CORE_PICTURE:
        {
            if(!m_decomp)
                continue;

            const Picture *my_picture = m_decomp->DecompressNextPicture(p_parse_unit);
            if(my_picture)
            {
                int picturenum_decoded = my_picture->GetPparams().PictureNum();
                if(picturenum_decoded != m_show_pnum)
                {
                    m_show_pnum = my_picture->GetPparams().PictureNum();
                    if(m_verbose)
                    {
                        std::cout << std::endl;
                        std::cout << "Picture ";
                        std::cout << m_show_pnum << " available";
                    }
                    m_state = STATE_PICTURE_AVAIL;
                    return m_state;
                }
            }
            break;
        }
        case PU_END_OF_SEQUENCE:
            m_next_state = STATE_SEQUENCE_END;
            break;

        case PU_AUXILIARY_DATA:
        case PU_PADDING_DATA:
            if(m_verbose)
                std::cerr << "Ignoring Auxiliary/Padding data" << std::endl;
            // Ignore auxiliary and padding data and continue parsing
            break;
        case PU_LOW_DELAY_PICTURE:
            if(m_verbose)
                std::cerr << "Low delay picture decoding not yet supported" << std::endl;
            return STATE_INVALID;

        default:
            return STATE_INVALID;
        }

    }
}

const SourceParams& DiracParser::GetSourceParams() const
{
    return m_decomp->GetSourceParams();
}

const DecoderParams& DiracParser::GetDecoderParams() const
{
    return m_decomp->GetDecoderParams();
}

const ParseParams& DiracParser::GetParseParams() const
{
    return m_decomp->GetParseParams();
}

const PictureParams* DiracParser::GetNextPictureParams() const
{
    return m_decomp->GetNextPictureParams();
}

const Picture* DiracParser::GetNextPicture() const
{
    return m_decomp->GetNextPicture();
}
