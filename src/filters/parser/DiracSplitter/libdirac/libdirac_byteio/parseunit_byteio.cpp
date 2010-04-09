/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: parseunit_byteio.cpp,v 1.10 2008/05/02 05:57:19 asuraparaju Exp $ $Name:  $
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
* Contributor(s): Andrew Kennedy (Original Author)
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

#include <libdirac_byteio/parseunit_byteio.h>
#include <libdirac_common/dirac_exception.h>

#include <iomanip>

using namespace dirac;
using namespace std;

// Fixed parse-prefix
const string PU_PREFIX = "BBCD";

// Parse-unit byte sizes
const int PU_NEXT_PARSE_OFFSET_SIZE = 4;
const int PU_PREVIOUS_PARSE_OFFSET_SIZE = 4;
const int PU_PREFIX_SIZE = 4;
const int PU_PARSE_CODE_SIZE = 1;
const int PU_PARSEUNIT_SIZE = PU_NEXT_PARSE_OFFSET_SIZE + PU_PREVIOUS_PARSE_OFFSET_SIZE+
                              PU_PREFIX_SIZE + PU_PARSE_CODE_SIZE;

ParseUnitByteIO::ParseUnitByteIO():
ByteIO(),
m_previous_parse_offset(0),
m_next_parse_offset(0)
{

}

ParseUnitByteIO::ParseUnitByteIO(const ByteIO& byte_io):
ByteIO(byte_io),
m_previous_parse_offset(0),
m_next_parse_offset(0)
{

}

ParseUnitByteIO::ParseUnitByteIO(const ParseUnitByteIO& parseunit_byteio):
ByteIO(parseunit_byteio),
m_previous_parse_offset(parseunit_byteio.m_previous_parse_offset),
m_next_parse_offset(parseunit_byteio.m_next_parse_offset),
m_parse_code(parseunit_byteio.m_parse_code)
{
}

ParseUnitByteIO::~ParseUnitByteIO()
{

}


//--------------public---------------------------------------------------------

void ParseUnitByteIO::CollateByteStats(DiracByteStats& dirac_byte_stats)
{
    dirac_byte_stats.SetByteCount(STAT_TOTAL_BYTE_COUNT,
                                  m_next_parse_offset);
}

bool ParseUnitByteIO::Input()
{

    // input prefix
    if(!SyncToUnitStart())
        return false;

    // input parse code
    m_parse_code = InputUnByte();

    // input parse-offsets
    m_next_parse_offset = ReadUintLit(PU_NEXT_PARSE_OFFSET_SIZE);

    m_previous_parse_offset = ReadUintLit(PU_PREVIOUS_PARSE_OFFSET_SIZE);

    return true;
}

bool ParseUnitByteIO::IsValid()
{
    if (IsEndOfSequence())
        return true;

    // Skip past the end of current parse unit
    SeekGet(m_next_parse_offset-GetSize(), ios_base::cur);

    // check the next series of bytes are the parse-info prefix
    string prefix = InputUnString(PU_PREFIX_SIZE);

    if(prefix==PU_PREFIX)
    {
        unsigned char next_parse_code;

        next_parse_code = InputUnByte();
        // input next unit parse-offsets
        int next_unit_next_parse_offset;
        next_unit_next_parse_offset = ReadUintLit(PU_NEXT_PARSE_OFFSET_SIZE);

        int next_unit_previous_parse_offset;
        next_unit_previous_parse_offset = ReadUintLit(PU_PREVIOUS_PARSE_OFFSET_SIZE);
        if (next_unit_previous_parse_offset == m_next_parse_offset)
        {
            SeekGet(-(m_next_parse_offset-GetSize()+PU_PARSEUNIT_SIZE), ios_base::cur);
            return true;
        }
    }
    SeekGet(-(m_next_parse_offset-GetSize()), ios_base::cur);
    return false;
}

bool ParseUnitByteIO::CanSkip()
{
    if(m_next_parse_offset==0 || m_next_parse_offset == GetSize())
        return true;

    // Skip past the end of current parse unit and past the header of the
    // next unit
    SeekGet(m_next_parse_offset-GetSize() + GetSize(), ios_base::cur);
    if(GetReadBytePosition() >= 0)
    {
        SeekGet(-(m_next_parse_offset-GetSize() + GetSize()), ios_base::cur);
           return true; // success
    }

    // end of stream reached
    mp_stream->clear();

    return false;
}

const string ParseUnitByteIO::GetBytes()
{
    stringstream parse_string;
    parse_string << PU_PREFIX;
    parse_string << CalcParseCode();

    //FIXME : Need to do this properly.
    // Write the parse offsets in Big Endian format
    for(int i=PU_NEXT_PARSE_OFFSET_SIZE-1; i >= 0; --i)
    {
        unsigned char cp = (m_next_parse_offset>>(i*8)) & 0xff;
        parse_string << cp;
    }

    for(int i=PU_PREVIOUS_PARSE_OFFSET_SIZE-1; i >= 0; --i)
    {
        unsigned char cp = (m_previous_parse_offset>>(i*8)) & 0xff;
        parse_string << cp;
    }

    return parse_string.str() + ByteIO::GetBytes();
}


int ParseUnitByteIO::GetSize() const
{
    return PU_PARSEUNIT_SIZE;
}

int ParseUnitByteIO::GetNextParseOffset() const
{
    return m_next_parse_offset;
}

int ParseUnitByteIO::GetPreviousParseOffset() const
{
    return m_previous_parse_offset;
}

void ParseUnitByteIO::SetAdjacentParseUnits(ParseUnitByteIO *p_prev_parseunit)
{
    // set next offset
    m_next_parse_offset = CalcNextUnitOffset();

    if(!p_prev_parseunit)
        return;

    // set previous parse offset
    m_previous_parse_offset = p_prev_parseunit->m_next_parse_offset;
}


ParseUnitType ParseUnitByteIO::GetType() const
{
    if(IsSeqHeader())
        return PU_SEQ_HEADER;

    if(IsCoreSyntax())
        return PU_CORE_PICTURE;

    if(IsLowDelay())
        return PU_LOW_DELAY_PICTURE;

    if(IsPicture())
        return PU_PICTURE;

    if(IsEndOfSequence())
        return PU_END_OF_SEQUENCE;

    if(IsAuxiliaryData())
        return PU_AUXILIARY_DATA;

    if(IsPaddingData())
        return PU_PADDING_DATA;

    return PU_UNDEFINED;
}

//------------protected-------------------------------------------------------

int ParseUnitByteIO::CalcNextUnitOffset()
{
    // typically size of current unit
    return GetSize();
}

bool ParseUnitByteIO::SyncToUnitStart()
{
     // locate parse-unit prefix
    string byte_buffer;

    while(CanRead()==true && mp_stream->tellg() >= 0)
    {
        // ensure current buffer length
        if((int)byte_buffer.size() == PU_PREFIX_SIZE)
        {
            byte_buffer.assign(byte_buffer.substr(1,PU_PREFIX_SIZE-1));
        }
        // read next byte
        byte_buffer.push_back(InputUnByte());

        //look to see if we have prefix
        if(byte_buffer==PU_PREFIX)
        {
            // check we can read a parse-unit
            //int prev_pos = mp_stream->tellg();
            mp_stream->seekg (PU_PARSEUNIT_SIZE-PU_PREFIX_SIZE, ios_base::cur);
            int cur_pos = mp_stream->tellg();
            if (cur_pos < 0) // past end of stream
            {
                mp_stream->clear();
                return false;
            }
            mp_stream->seekg(-(PU_PARSEUNIT_SIZE-PU_PREFIX_SIZE), ios_base::cur);
            return true;
        }

    }

    // Clear the eof flag and throw an error.
    mp_stream->clear();
    return false;
}

