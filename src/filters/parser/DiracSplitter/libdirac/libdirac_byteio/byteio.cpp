/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: byteio.cpp,v 1.4 2008/03/14 08:17:36 asuraparaju Exp $ $Name:  $
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
* Contributor(s): Andrew Kennedy (Original Author),
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

#include <cmath>
#include <libdirac_byteio/byteio.h>
using namespace dirac;
using namespace std;

ByteIO::ByteIO(bool new_stream):
m_current_byte(0),
m_current_pos(0),
m_num_bytes(0),
m_new_stream(true),
m_bits_left(0)
{
    if(new_stream)
        mp_stream = new stringstream(stringstream::in | stringstream::out |
                                     stringstream::binary);

                                    
}

ByteIO::ByteIO(const ByteIO& stream_data):
m_current_byte(0),
m_current_pos(0),
m_num_bytes(0),
m_new_stream(false),
m_bits_left(0)
{
     mp_stream=stream_data.mp_stream;
}


ByteIO::~ByteIO()
{
    if (m_new_stream)
        delete mp_stream;
}

const string ByteIO::GetBytes() 
{
    return mp_stream->str();
}

int ByteIO::GetSize() const
{
    return m_num_bytes;
}

void ByteIO::SetByteParams(const ByteIO& byte_io)
{
    mp_stream=byte_io.mp_stream;
    m_current_byte=byte_io.m_current_byte;
    m_current_pos=byte_io.m_current_pos;
}

//----------protected---------------------------------------------------------------

void ByteIO::ByteAlignInput()
{
    m_current_pos=0;
    m_current_byte=0;
}

void ByteIO::ByteAlignOutput()
{
    if(m_current_pos!=0)
        OutputCurrentByte();
}

int ByteIO::ReadBit()
{
    if(m_current_pos == CHAR_BIT)
        m_current_pos=0;

    if (m_current_pos == 0)
        m_current_byte = InputUnByte();
#if 1
    // MSB to LSB
    return GetBit(m_current_byte, (CHAR_BIT-1-m_current_pos++));
#else
    // LSB to MSB
    return GetBit(m_current_byte, m_current_pos++);
#endif
}

int ByteIO::ReadBitB()
{
    if (m_bits_left)
    {
        --m_bits_left;
        return ReadBit();
    }
    else
        return 1;
}

bool ByteIO::ReadBool()
{
    return ReadBit();
}

bool ByteIO::ReadBoolB()
{
    return ReadBitB();
}

unsigned int ByteIO::ReadNBits(int count)
{
    unsigned int val = 0;
    for (int i = 0; i < count; ++i)
    {
        val <<= 1;
        val += ReadBit();
    }
    return val;
}

void ByteIO::FlushInputB()
{
    while(m_bits_left)
    {
        ReadBit();
        --m_bits_left;
    }
}

int ByteIO::ReadSint()
{

    int val = ReadUint();
    bool bit;

     //get the sign
    if (val != 0)
    {
        bit = ReadBit();
        if (bit )
            val = -val;
    }
    return val;        
}

int ByteIO::ReadSintB()
{

    int val = ReadUintB();
    bool bit;

     //get the sign
    if (val != 0)
    {
        bit = ReadBitB();
        if (bit )
            val = -val;
    }
    return val;        
}

unsigned int ByteIO::ReadUint()
{
    unsigned int value = 1;
    while (!ReadBit())
    {
        value <<= 1;
        if (ReadBit())
            value +=1; 
    }
    --value;
    return value;
}

unsigned int ByteIO::ReadUintB()
{
    unsigned int value = 1;
    while (!ReadBitB())
    {
        value <<= 1;
        if (ReadBitB())
            value +=1; 
    }
    --value;
    return value;
}

void ByteIO::WriteBit(const bool& bit)
{
    if(bit)
#if 1
        // MSB to LSB
        SetBit(m_current_byte, CHAR_BIT-1-m_current_pos);
#else
        // LSB to MSB
        SetBit(m_current_byte, m_current_pos);
#endif

    if ( m_current_pos == CHAR_BIT-1)
    { 
        // If a whole byte has been written, output to stream
        OutputCurrentByte();
        m_current_byte = 0;
        m_current_pos = 0;
    }    
    else
      // Shift mask to next bit in the output byte
        ++m_current_pos;
}

void ByteIO::WriteNBits(unsigned int val, int count)
{
    do
    {
        WriteBit(val & ( 1 << (count-1)));
        count--;
    }
    while(count > 0);
}

int ByteIO::WriteNBits(unsigned int val)
{
    int nbits = static_cast<int>(log(static_cast<double>(val))/log(2.0)) + 1;
    WriteNBits(val, nbits);
    return nbits;
}

void ByteIO::WriteSint(int val)
{
    unsigned int value = (val >= 0 ? val : -val);
    //output magnitude
    WriteUint(value);

    //do sign
    if (val<0) WriteBit(1);
    else if (val>0) WriteBit(0);
}

void ByteIO::WriteUint(unsigned int value)
{
    unsigned int val = value+1;

    int num_follow_zeroes = 0;

    while (val >= (1U <<num_follow_zeroes))
        ++num_follow_zeroes;
    --num_follow_zeroes;

    for (int i=num_follow_zeroes-1; i>=0; --i)
    {
        WriteBit(BIT_ZERO);
        WriteBit(val&(1<<i));
    }
    WriteBit(BIT_ONE);
}

void ByteIO::RemoveRedundantBytes(const int size)
{
    int prev_pos = mp_stream->tellg();
    string data=mp_stream->str();
    data.erase(0, size);
    mp_stream->str(data);
    m_num_bytes=data.size();
    if(data.size())
        SeekGet(max(prev_pos-size, 0), ios_base::beg);
}
