/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: parseunit_byteio.cpp,v 1.6 2007/09/03 11:31:42 asuraparaju Exp $ $Name: Dirac_0_8_0 $
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
    m_next_parse_offset = InputFixedLengthUint(PU_NEXT_PARSE_OFFSET_SIZE);

    m_previous_parse_offset = InputFixedLengthUint(PU_PREVIOUS_PARSE_OFFSET_SIZE);

    return true;
}

bool ParseUnitByteIO::IsValid(const ParseUnitByteIO& next_unit)
{
    // move back by size of next-unit
    SeekGet(-(next_unit.GetPreviousParseOffset()+next_unit.GetSize()), ios_base::cur);

    // check the next series of bytes are the parse-info prefix
    string prefix = InputUnString(PU_PREFIX_SIZE);

    bool is_valid = (prefix==PU_PREFIX);

    if(is_valid)
    {
        // move to start beginning of data in this unit
        SeekGet(GetSize()-PU_PREFIX_SIZE, ios_base::cur);
        return true;
    }

    // not a valid unit - move to start of entire unit
    SeekGet(-PU_PREFIX_SIZE, ios_base::cur);

    return false;
}

bool ParseUnitByteIO::Skip()
{
    if(m_next_parse_offset==0)
        return false;

    //int curr_pos = GetReadBytePosition();
    SeekGet(m_next_parse_offset-GetSize(), ios_base::cur);
    if(GetReadBytePosition() >= 0)
        return true; // success

    // end of stream reached
    mp_stream->clear();
    
   DIRAC_THROW_EXCEPTION(
                    ERR_END_OF_STREAM,
                    "End of stream",
                    SEVERITY_NO_ERROR)

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
    if(IsAU())
        return PU_ACCESS_UNIT;
    
    if(IsLowDelay())
        return PU_LOW_DELAY_FRAME;

    if(IsPicture())
        return PU_FRAME;

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
    
    while(CanRead()==true)
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
              //  mp_stream->seekg(prev_pos-PU_PREFIX_SIZE, ios_base::beg);
                cur_pos = mp_stream->tellg();

                DIRAC_THROW_EXCEPTION(ERR_END_OF_STREAM,
                                      "End of stream",
                                      SEVERITY_NO_ERROR)
            }
            mp_stream->seekg(-(PU_PARSEUNIT_SIZE-PU_PREFIX_SIZE), ios_base::cur);
            return true;
        }
        
    }

    // Clear the eof flag and throw an error.
    mp_stream->clear();
    DIRAC_THROW_EXCEPTION(ERR_END_OF_STREAM,
                          "End of stream",
                          SEVERITY_NO_ERROR);
    return false;
}

