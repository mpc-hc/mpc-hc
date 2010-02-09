/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: dirac_byte_stream.cpp,v 1.8 2008/08/14 00:51:08 asuraparaju Exp $ $Name:  $
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

#include <libdirac_byteio/dirac_byte_stream.h>
#include <libdirac_byteio/endofsequence_byteio.h>
#include <libdirac_common/dirac_exception.h>

using namespace dirac;
using namespace std;

DiracByteStream::DiracByteStream():
ByteIO(),
mp_prev_parse_unit(NULL)
{
}

DiracByteStream::~DiracByteStream()
{
    delete mp_prev_parse_unit;
}

//---------------decoding----------------------------------------------------

void DiracByteStream::AddBytes(char* start,
                               int count)
{
    // add to input stream
    string str(start, count);
    ByteIO::OutputBytes(str);

}

DiracByteStats DiracByteStream::GetLastUnitStats()
{
    DiracByteStats dirac_byte_stats;

    if(m_parse_unit_list.empty())
        return dirac_byte_stats;

    ParseUnitByteIO* p_parse_unit = m_parse_unit_list.back().second;
    p_parse_unit->CollateByteStats(dirac_byte_stats);

    return dirac_byte_stats;
}

void DiracByteStream::Reset(ParseUnitByteIO* p_curr_unit, int pos)
{
    delete p_curr_unit;
    SeekGet(pos, ios_base::beg);
}

ParseUnitByteIO* DiracByteStream::GetNextParseUnit()
{
    if(GetSize()==0)
        return NULL;

    int pos=0;
    if(mp_prev_parse_unit)
    {
        // remove the unwanted bytes associated with the previous parse-unit
        int prev_offset = mp_prev_parse_unit->GetNextParseOffset();
        RemoveRedundantBytes(prev_offset ? prev_offset : mp_prev_parse_unit->GetSize());
        delete mp_prev_parse_unit;
        mp_prev_parse_unit=NULL;
        if(!GetSize())
            return NULL;
    }

    ParseUnitByteIO* p_curr_unit=NULL;

    while(true)
    {
        pos  = GetReadBytePosition();

        p_curr_unit = new ParseUnitByteIO(*this);
        if (!p_curr_unit->Input())
        {
            Reset(p_curr_unit, pos);
            return NULL;
        }

        // skip past current unit
        if(!p_curr_unit->CanSkip())
        {
            Reset(p_curr_unit, pos);
            return NULL;
        }

        if (p_curr_unit->IsEndOfSequence())
        {
            break;
        }

        // look to see if next unit validates the current one
        if(!p_curr_unit->IsValid())
        {
            // delete the unit - it's invalid
            delete p_curr_unit;
            // remove unwanted portion of bytes
            RemoveRedundantBytes(pos);
            // look for next potential parse-unit
            continue;
        }
        break;
    } // while

    // Remove all redundant bytes that are not part of a parse unit
    int remove_size = std::max (0, GetReadBytePosition()-p_curr_unit->GetSize());
    if (remove_size)
    {
       //std::cerr << "Size="<<GetSize() << " Un-useful bytes=" << remove_size << std::endl;
        RemoveRedundantBytes(remove_size);
    }

     mp_prev_parse_unit=p_curr_unit;
     return p_curr_unit;
}

DiracByteStats DiracByteStream::GetSequenceStats() const
{
    return m_sequence_stats;
}

//---------------encoding-----------------------------------------------------

void DiracByteStream::AddSequenceHeader(SequenceHeaderByteIO *p_seqheader_byteio)
{
    // set previous parse-unit details
    ParseUnitByteIO *mp_previous_parse_unit=mp_prev_parse_unit;

    if(!m_parse_unit_list.empty())
        mp_previous_parse_unit = m_parse_unit_list.back().second;

    // set adjacent parse-unit
    p_seqheader_byteio->SetAdjacentParseUnits(mp_previous_parse_unit);

    // push onto to pending list
    m_parse_unit_list.push(std::make_pair (PU_SEQ_HEADER, p_seqheader_byteio) );

    // set previous parse-unit
    mp_previous_parse_unit = p_seqheader_byteio;

    // save stats
    p_seqheader_byteio->CollateByteStats(m_sequence_stats);
}

void DiracByteStream::AddPicture(PictureByteIO *p_frame_byteio)
{
    // set previous parse-unit details
    ParseUnitByteIO *mp_previous_parse_unit=mp_prev_parse_unit;

    if(!m_parse_unit_list.empty())
        mp_previous_parse_unit = m_parse_unit_list.back().second;

    // set adjacent parse-unit
    p_frame_byteio->SetAdjacentParseUnits(mp_previous_parse_unit);

     // push onto to pending list
    m_parse_unit_list.push(std::make_pair(PU_PICTURE, p_frame_byteio ) );

   // set previous parse-unit
    mp_previous_parse_unit = p_frame_byteio;

     // save stats
    p_frame_byteio->CollateByteStats(m_sequence_stats);
}

void DiracByteStream::Clear()
{
       while(!m_parse_unit_list.empty())
    {
        ParseUnitByteIO* p_parse_unit=m_parse_unit_list.front().second;
        m_parse_unit_list.pop();
        if(m_parse_unit_list.empty())
        {
            delete mp_prev_parse_unit;
            mp_prev_parse_unit=p_parse_unit;
        }
        else
            delete p_parse_unit;
    }
}

DiracByteStats DiracByteStream::EndSequence()
{
    // create
    EndOfSequenceByteIO *p_endofsequence_byteio = new EndOfSequenceByteIO(*this);

     // set previous parse-unit details
    ParseUnitByteIO *mp_previous_parse_unit=mp_prev_parse_unit;

    if(!m_parse_unit_list.empty())
        mp_previous_parse_unit = m_parse_unit_list.back().second;

    // set adjacent parse-unit
    p_endofsequence_byteio->SetAdjacentParseUnits(mp_previous_parse_unit);

     // push onto to pending list
    m_parse_unit_list.push(std::make_pair(PU_END_OF_SEQUENCE, p_endofsequence_byteio) );

    p_endofsequence_byteio->CollateByteStats(m_sequence_stats);

    // clear stats
    DiracByteStats seq_stats(m_sequence_stats);
    m_sequence_stats.Clear();

    // return seq stats
    return seq_stats;
}

const string DiracByteStream::GetBytes()
{
    // take copy
    ParseUnitList parse_list = m_parse_unit_list;
    mp_stream->str("");

    while(!parse_list.empty())
    {
        *mp_stream << parse_list.front().second->GetBytes();
        parse_list.pop();
    }

    return mp_stream->str();
}

bool DiracByteStream::IsUnitAvailable() const
{
    return !m_parse_unit_list.empty();
}
