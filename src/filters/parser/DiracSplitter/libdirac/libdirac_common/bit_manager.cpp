/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: bit_manager.cpp,v 1.10 2008/01/31 11:25:16 tjdwave Exp $ $Name:  $
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
* Contributor(s): Thomas Davies (original author),
*                 Robert Scott Ladd,
*                 Tim Borer
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

#include <libdirac_common/bit_manager.h>
#include <libdirac_common/common.h>
using namespace dirac;

using std::vector;

////////////////
//Output stuff//
////////////////

//Constructor
BasicOutputManager::BasicOutputManager(std::ostream* out_data ):
    m_num_out_bytes(0),
    m_op_ptr(out_data)
{
    InitOutputStream();
}

void BasicOutputManager::InitOutputStream()
{
    // Set byte pointer to start of buffer
    m_current_byte = 0;   
    // Set output mask to MSB of byte 
    m_output_mask = 0x80; 
    // Reset the output buffer
    m_buffer.clear();
}

void BasicOutputManager::OutputSkipInterpretStartPrefixByte()
{
    size_t buf_size = m_buffer.size();
    if (buf_size >=4 && 
        m_buffer[buf_size-1] == (char)START_CODE_PREFIX_BYTE3 &&
        m_buffer[buf_size-2] == (char)START_CODE_PREFIX_BYTE2 && 
        m_buffer[buf_size-3] == (char)START_CODE_PREFIX_BYTE1 &&
        m_buffer[buf_size-4] == (char)START_CODE_PREFIX_BYTE0)
    {
        m_buffer.push_back((char)NOT_START_CODE);
        std::cerr << "Wrote ignore code " << std::endl;
    }
}

void BasicOutputManager::OutputBit(const bool& bit )
{
    m_current_byte |= (bit ? (m_output_mask):0);

    // Shift mask to next bit in the output byte
    m_output_mask >>= 1; 

    if ( m_output_mask == 0 )
    { 
        // If a whole byte has been written, write out
        m_output_mask = 0x80;
        m_buffer.push_back(m_current_byte);
        OutputSkipInterpretStartPrefixByte();
        m_current_byte = 0;
    }    
}

void BasicOutputManager::OutputBit(const bool& bit, int& count)
{
    OutputBit(bit);
    count++;    
}

void BasicOutputManager::OutputByte(const char& byte)
{
    FlushOutput();
    m_buffer.push_back( byte );
    OutputSkipInterpretStartPrefixByte();
}

void BasicOutputManager::OutputBytes( char* str_array )
{
    FlushOutput();
    while ( *str_array != 0 )
    {
        m_buffer.push_back( *str_array );
        str_array++;
    }
}

void BasicOutputManager::OutputBytes(char* str_array,int num)
{
    FlushOutput();
    for ( int i=0 ; i<num ; ++i )
        m_buffer.push_back( str_array[i] );
}


void BasicOutputManager::WriteToFile()
{
    FlushOutput();
    for ( vector<char>::iterator it=m_buffer.begin() ; it!=m_buffer.end() ; ++it )
    {
        m_op_ptr->write( &( *it ) , 1 );        
    }
    m_num_out_bytes = m_buffer.size();
    InitOutputStream();        
}

void BasicOutputManager::FlushOutput()
{
    // Flush the current byte to output buffer and reset
    if ( m_output_mask != 0x80 )
    {
        m_buffer.push_back( m_current_byte );    
        m_current_byte = 0;
        m_output_mask = 0x80;
    }
}

size_t BasicOutputManager::Size() const
{
    if ( m_output_mask==0x80 )
        return m_buffer.size();
    else
        return m_buffer.size()+1;
}

// Unit output - a subband or the MV data, for example //

UnitOutputManager::UnitOutputManager(std::ostream* out_data ):
    m_header(out_data),
    m_data(out_data),
    m_unit_bytes(0),
    m_unit_data_bytes(0),
    m_unit_head_bytes(0)
    {}

void UnitOutputManager::WriteToFile()
{
    m_header.WriteToFile();
    m_data.WriteToFile();
    
    // after writing to file, get the number of unit bytes written
    m_unit_data_bytes = m_data.GetNumBytes();
    m_unit_head_bytes = m_header.GetNumBytes();
    m_unit_bytes = m_unit_data_bytes + m_unit_head_bytes;

}

size_t UnitOutputManager::Size() const
{
    return m_data.Size()+m_header.Size();
}

FrameOutputManager::FrameOutputManager( std::ostream* out_data , int num_bands ) :
    m_data_array( 3 , num_bands ),
    m_comp_bytes( 3 ),
    m_comp_hdr_bytes( 3 ),
    m_out_stream( out_data )
{
    Init( num_bands );
}

FrameOutputManager::~FrameOutputManager()
{
    DeleteAll();
}

void FrameOutputManager::WriteToFile()
{

    // Write out the picture header
    m_frame_header->WriteToFile();
    m_total_bytes = m_frame_header->GetNumBytes();
    m_header_bytes = m_frame_header->GetNumBytes();

    // Write out the motion vector data
    m_mv_data->WriteToFile();

    // after writing to file, get the number of bytes written
    m_mv_hdr_bytes = m_mv_data->GetUnitHeaderBytes();
    m_mv_bytes = m_mv_data->GetUnitBytes();

    m_total_bytes += m_mv_bytes;
    m_header_bytes += m_mv_hdr_bytes;

    // Write out the component data
    for ( int c=0 ; c<3 ; ++c)
    {

        m_comp_hdr_bytes[c] = 0;
        m_comp_bytes[c] = 0;

        for ( int b=m_data_array.LastX() ; b>=0 ; --b)
        {
            m_data_array[c][b]->WriteToFile();
            // after writing to file, get the number of bytes written
            m_comp_hdr_bytes[c] += m_data_array[c][b]->GetUnitHeaderBytes();
            m_comp_bytes[c] += m_data_array[c][b]->GetUnitBytes();
        }// b

    }// c

    for ( int c=0 ; c<m_data_array.LengthY() ; ++c)
    {
        m_total_bytes += m_comp_bytes[c];
        m_header_bytes += m_comp_hdr_bytes[c];
    }
}

UnitOutputManager& FrameOutputManager::BandOutput( const int csort , const int band_num)
{
    return *( m_data_array[csort][band_num-1] );
}

const UnitOutputManager& FrameOutputManager::BandOutput( const int csort , const int band_num) const
{
    return *( m_data_array[csort][band_num-1] );
}

// Picture stuff


void FrameOutputManager::Init( int num_bands )
{
    // Initialise output for the picture header
    m_frame_header = new BasicOutputManager( m_out_stream );

    // Initialise output for the MV data
    m_mv_data = new UnitOutputManager( m_out_stream );

    // Initialise subband outputs
    for ( int c=0 ; c<3 ; ++c)
        for ( int b=0 ; b<num_bands ; ++b)
            m_data_array[c][b] = new UnitOutputManager( m_out_stream );
}

void FrameOutputManager::Reset()
{
    const int num_bands = m_data_array.LengthX();
    DeleteAll();
    Init( num_bands );
}   

void FrameOutputManager::DeleteAll()
{
    // Delete subband outputs
    for ( int c=0 ; c<3 ; ++c)
        for ( int b=0 ; b<m_data_array.LengthX() ; ++b )
            delete m_data_array[c][b];

    // Delete MV data op
    delete m_mv_data;

    // Delete picture header op
    delete m_frame_header;
} 

size_t FrameOutputManager::Size() const
{
    size_t size = 0;

    size += m_frame_header->Size();

    for ( int c=0 ; c<3 ; ++c)
    {
        for ( int b=0 ; b<m_data_array.LengthX() ; ++b )
        {
            size += m_data_array[c][b]->Size();
        }
    }

    size += m_mv_data->Size();

    return size;
}  


// Sequence stuff //

SequenceOutputManager::SequenceOutputManager( std::ostream* out_data ):
    m_frame_op_mgr( out_data ),
    m_seq_header( out_data ),
    m_seq_end( out_data ),
    m_comp_bytes( 3 ),
    m_comp_hdr_bytes( 3 ),
    m_mv_hdr_bytes(0),
    m_mv_bytes(0),
    m_total_bytes(0),
    m_header_bytes(0),
    m_trailer_bytes(0)

{
    for (int c=0 ; c<3 ; ++c )
    {
        m_comp_hdr_bytes[c] = 0;
        m_comp_bytes[c] = 0;
    }
}

void SequenceOutputManager::WriteFrameData()
{
    m_frame_op_mgr.WriteToFile();
    
    // Keep up with count of component bytes
    for (int c=0 ; c<m_comp_hdr_bytes.Length(); ++c)
    {
        m_comp_hdr_bytes[c] += m_frame_op_mgr.ComponentHeadBytes( c );
        m_comp_bytes[c] += m_frame_op_mgr.ComponentBytes( c );
    }// c

    // Keep up with count of MV bytes
    m_mv_hdr_bytes += m_frame_op_mgr.MVHeadBytes();
    m_mv_bytes += m_frame_op_mgr.MVBytes();

    // Keep up with overall totals
    m_header_bytes += m_frame_op_mgr.FrameHeadBytes();
    m_total_bytes += m_frame_op_mgr.FrameBytes();

}

void SequenceOutputManager::WriteSeqHeaderToFile()
{
    m_seq_header.WriteToFile();
    m_header_bytes += m_seq_header.GetNumBytes();
    m_total_bytes += m_seq_header.GetNumBytes();
}

void SequenceOutputManager::WriteSeqTrailerToFile()
{
    m_seq_end.WriteToFile();
    m_trailer_bytes += m_seq_end.GetNumBytes();
    m_total_bytes += m_seq_end.GetNumBytes();
}

////////////////
//Input stuff//
////////////////

//Constructor
BitInputManager::BitInputManager(std::istream* in_data ):
    m_ip_ptr(in_data)
{
    InitInputStream();
}


void BitInputManager::InitInputStream()
{
    m_shift = 0xffffffff;
    m_input_bits_left = 0;
}

bool BitInputManager::InputBit()
{
    //assumes mode errors will be caught by iostream class    

    if (m_input_bits_left == 0)
    {
        m_ip_ptr->read(&m_current_byte,1);
        m_input_bits_left = 8;
        if (m_shift == START_CODE_PREFIX && (unsigned char)m_current_byte == NOT_START_CODE)
        {
            std::cerr << "Ignoring byte " << std::endl;
            m_ip_ptr->read(&m_current_byte,1);
            m_shift = 0xffffffff;
        }
        m_shift = (m_shift << 8) | m_current_byte;
    }

    m_input_bits_left--;

    return bool( ( m_current_byte >> m_input_bits_left ) & 1 );

}

bool BitInputManager::InputBit(int& count)
{
    count++;
    return InputBit();
}

bool BitInputManager::InputBit(int& count, const int max_count)
{
    if ( count<max_count )
    {
        count++;
        return InputBit();
    }
    else
        return false;
}

char BitInputManager::InputByte()
{
    // Forget about what's in the current byte    
    FlushInput();

    char byte;
    m_ip_ptr->read(&byte,1);

    return byte;    
}

void BitInputManager::InputBytes(char* cptr, int num)
{
    // Forget about what's in the current byte    
    FlushInput();

    m_ip_ptr->read(cptr,num);    
}

void BitInputManager::FlushInput()
{
    m_input_bits_left = 0;    
}

bool BitInputManager::End() const 
{
    return m_ip_ptr->eof();    
}
