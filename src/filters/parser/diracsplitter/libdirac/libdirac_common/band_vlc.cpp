/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: band_vlc.cpp,v 1.2 2007/12/05 01:23:43 asuraparaju Exp $ $Name: Dirac_0_9_1 $
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
* Portions created by the Initial Developer are Copyright (c) 2004.
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

// System includes
#include <sstream>

// Dirac includes
#include <libdirac_common/band_vlc.h>
#include <libdirac_byteio/subband_byteio.h>
#include <libdirac_common/dirac_exception.h>

using namespace dirac;

// Constructor for encoding
BandVLC::BandVLC( SubbandByteIO* subband_byteio,
                  const SubbandList& band_list,
                  int band_num,
                  const bool is_intra) :
    m_is_intra(is_intra),
    m_bnum(band_num),
    m_node(band_list(band_num)),
    m_last_qf_idx(m_node.QIndex()),
    m_byteio(subband_byteio)
{
}

// encoding functions
int BandVLC::Compress(CoeffArray &in_data)
{
    DoWorkCode(in_data);
    return m_byteio->GetSize();
}

void BandVLC::DoWorkCode(CoeffArray &in_data)
{
    const TwoDArray<CodeBlock>& block_list( m_node.GetCodeBlocks() );
    // coeff blocks can be skipped only if SpatialPartitioning is
    // enabled i.e. more than one code-block per subband
    bool code_skip = (block_list.LengthX() > 1 || block_list.LengthY() > 1);

    for (int j=block_list.FirstY() ; j<=block_list.LastY() ; ++j)
    {
        CodeBlock *block = block_list[j];
        for (int i=block_list.FirstX() ; i<=block_list.LastX() ; ++i)
        {
            if (code_skip)
                m_byteio->WriteBit(block[i].Skipped()); // encode skip flag
            if ( !block[i].Skipped() )
            {
                CodeCoeffBlock( block[i] , in_data );
            }
            else
            {
                ClearBlock (block[i] , in_data);
            }
        }// i
    }// j
}

void BandVLC::CodeCoeffBlock(const CodeBlock& code_block , CoeffArray& in_data)
{
    //main coding function, using binarisation

    const int xbeg = code_block.Xstart();
    const int ybeg = code_block.Ystart();
    const int xend = code_block.Xend();
    const int yend = code_block.Yend();
 
    const int qf_idx = code_block.QIndex();

    if ( m_node.UsingMultiQuants() )
    {
          CodeQIndexOffset( qf_idx - m_last_qf_idx);
          m_last_qf_idx = qf_idx;
    }

    m_qf = dirac_quantiser_lists.QuantFactor4( qf_idx );
    if (m_is_intra)
        m_offset =  dirac_quantiser_lists.IntraQuantOffset4( qf_idx );
    else
        m_offset =  dirac_quantiser_lists.InterQuantOffset4( qf_idx );

    for ( int ypos=ybeg; ypos<yend ;++ypos)
    {
        for ( int xpos=xbeg; xpos<xend ;++xpos)
        {
            CodeVal( in_data , xpos , ypos , in_data[ypos][xpos] );
        }// xpos
    }// ypos    

}

void BandVLC::CodeQIndexOffset( const int offset )
{
    m_byteio->WriteSint(offset);
}

inline void BandVLC::CodeVal( CoeffArray& in_data , 
                                const int xpos , 
                                const int ypos , 
                                const CoeffType val )
{
    int qval( std::abs(val) );
    qval <<= 2;
    qval /= m_qf;
    in_data[ypos][xpos] = static_cast<CoeffType>( qval );

    // Code the quantised value into the bitstream
    if (val < 0)
        qval = -qval;
    m_byteio->WriteSint(qval);

    if (qval)
    {
        // Reconstruct
        in_data[ypos][xpos] *= m_qf;
        in_data[ypos][xpos] += m_offset+2;
        in_data[ypos][xpos] >>= 2;
        if (val < 0)
            in_data[ypos][xpos]  = -in_data[ypos][xpos];
            
    }


}

// decoding functions
void BandVLC::Decompress(CoeffArray &out_data, int num_bytes)
{
    m_byteio->SetBitsLeft(num_bytes * 8);
    DoWorkDecode(out_data);
    m_byteio->FlushInputB();
}

void BandVLC::DoWorkDecode(CoeffArray& out_data)
{
    const TwoDArray<CodeBlock>& block_list( m_node.GetCodeBlocks() );

    // coeff blocks can be skipped only if SpatialPartitioning is
    // enabled i.e. more than one code-block per subband
    bool decode_skip= (block_list.LengthX() > 1 || block_list.LengthY() > 1);
    // Now loop over the blocks and decode
    for (int j=block_list.FirstY() ; j<=block_list.LastY() ; ++j)
    {
        CodeBlock *block = block_list[j];
        for (int i=block_list.FirstX() ; i<=block_list.LastX() ; ++i)
        {
            if (decode_skip)
                block[i].SetSkip( m_byteio->ReadBoolB() );
            if ( !block[i].Skipped() )
            {
                DecodeCoeffBlock( block[i] , out_data );
            }
            else
            {
                ClearBlock (block[i] , out_data);
            }

        }// i
    }// j
}

void BandVLC::DecodeCoeffBlock(const CodeBlock& code_block , CoeffArray& out_data)
{
    const int xbeg = code_block.Xstart();
    const int ybeg = code_block.Ystart();
    const int xend = code_block.Xend();
    const int yend = code_block.Yend();

    int qf_idx = m_node.QIndex();

    if ( m_node.UsingMultiQuants() )
    {
        qf_idx = m_last_qf_idx+DecodeQIndexOffset(); 
        m_last_qf_idx = qf_idx;
    }

    if (qf_idx > (int)dirac_quantiser_lists.MaxQIndex())
    {
        std::ostringstream errstr;
        errstr << "Quantiser index out of range [0.."  
               << (int)dirac_quantiser_lists.MaxQIndex() << "]";
        DIRAC_THROW_EXCEPTION(
            ERR_UNSUPPORTED_STREAM_DATA,
            errstr.str(),
            SEVERITY_FRAME_ERROR);
    }

    m_qf = dirac_quantiser_lists.QuantFactor4( qf_idx );
    
    if (m_is_intra)
        m_offset =  dirac_quantiser_lists.IntraQuantOffset4( qf_idx );
    else
        m_offset =  dirac_quantiser_lists.InterQuantOffset4( qf_idx );

    //Work
    
    for ( int ypos=ybeg; ypos<yend ;++ypos)
    {
        for ( int xpos=xbeg; xpos<xend ;++xpos)
        {
            DecodeVal( out_data , xpos , ypos );
        }// xpos
    }// ypos
}

int BandVLC::DecodeQIndexOffset( )
{
    return m_byteio->ReadSintB();
}

inline void BandVLC::DecodeVal( CoeffArray& out_data , const int xpos , const int ypos )
{
    // Read quantised coefs
    int quant_coeff = m_byteio->ReadSintB();
    CoeffType& out_pixel = out_data[ypos][xpos];

    // Reconstruct
    out_pixel = std::abs(quant_coeff);
    if ( out_pixel )
    {
        out_pixel *= m_qf;
        out_pixel += m_offset+2;
        out_pixel >>= 2;
     
        if ( quant_coeff < 0)
            out_pixel = -out_pixel;
    }
}

// general purpose functions
void BandVLC::ClearBlock( const CodeBlock& code_block , CoeffArray& coeff_data)
{
    for (int j=code_block.Ystart() ; j<code_block.Yend() ; j++)
    {
        CoeffType *pic = &coeff_data[j][code_block.Xstart()];
        memset (pic, 0, (code_block.Xend()-code_block.Xstart())*sizeof(CoeffType));
    }// j

}

void BandVLC::SetToVal( const CodeBlock& code_block , CoeffArray& pic_data , const CoeffType val)
{
    for (int j=code_block.Ystart() ; j<code_block.Yend() ; j++)
    {
          for (int i=code_block.Xstart() ; i<code_block.Xend() ; i++)
           {
            pic_data[j][i] = val;
           }// i
    }// j
}

//////////////////////////////////////////////////////////////////////////////////
//Finally,special class incorporating prediction for the DC band of intra frames//
//////////////////////////////////////////////////////////////////////////////////

void IntraDCBandVLC::DoWorkCode(CoeffArray& in_data)
{
    const TwoDArray<CodeBlock>& block_list( m_node.GetCodeBlocks() );

    // Now loop over the blocks and code. Note that DC blocks can't be skipped
    for (int j=block_list.FirstY() ; j<=block_list.LastY() ; ++j)
    {
        for (int i=block_list.FirstX() ; i<=block_list.LastX() ; ++i)
        {
            CodeCoeffBlock( block_list[j][i] , in_data );
        }// i
    }// j
}

void IntraDCBandVLC::CodeCoeffBlock( const CodeBlock& code_block , CoeffArray& in_data)
{
    // Main coding function, using binarisation
    const int xbeg = code_block.Xstart();
    const int ybeg = code_block.Ystart();
    const int xend = code_block.Xend();
    const int yend = code_block.Yend();

    CoeffType val;
    
    CoeffType prediction;

    const int qf_idx = code_block.QIndex();

    if ( m_node.UsingMultiQuants() )
    {
          CodeQIndexOffset( qf_idx - m_last_qf_idx);
          m_last_qf_idx = qf_idx;
    }

    m_qf = dirac_quantiser_lists.QuantFactor4( qf_idx );

    m_offset =  dirac_quantiser_lists.IntraQuantOffset4( qf_idx );

    for ( int ypos=ybeg ; ypos < yend; ++ypos )
    {
        for (int xpos = xbeg ; xpos < xend; ++xpos )
        {
            prediction = GetPrediction( in_data , xpos , ypos );            
            val = in_data[ypos][xpos] - prediction;
            CodeVal( in_data , xpos , ypos , val );            
            in_data[ypos][xpos] += prediction;
        }//xpos            
    }//ypos    
}


void IntraDCBandVLC::DoWorkDecode(CoeffArray& out_data)
{
    const TwoDArray<CodeBlock>& block_list( m_node.GetCodeBlocks() );

    // Now loop over the blocks and decode
    for (int j=block_list.FirstY() ; j<=block_list.LastY() ; ++j)
    {
        for (int i=block_list.FirstX() ; i<=block_list.LastX() ; ++i)
        {
            DecodeCoeffBlock( block_list[j][i] , out_data );
        }// i
    }// j
}

void IntraDCBandVLC::DecodeCoeffBlock( const CodeBlock& code_block , CoeffArray& out_data)
{
    const int xbeg = code_block.Xstart();
    const int ybeg = code_block.Ystart();
    const int xend = code_block.Xend();
    const int yend = code_block.Yend();

    int qf_idx = m_node.QIndex();

    if ( m_node.UsingMultiQuants() )
    {
        qf_idx = DecodeQIndexOffset()+m_last_qf_idx;
        m_last_qf_idx = qf_idx;
    }

    m_qf = dirac_quantiser_lists.QuantFactor4( qf_idx );
    
    m_offset =  dirac_quantiser_lists.IntraQuantOffset4( qf_idx );

    //Work
    
    for ( int ypos=ybeg ; ypos<yend ; ++ypos)
    {
        for ( int xpos=xbeg ; xpos<xend ; ++xpos)
        {
            DecodeVal( out_data , xpos , ypos );
            out_data[ypos][xpos] += GetPrediction( out_data , xpos , ypos );
        }//xpos
    }//ypos
}


CoeffType IntraDCBandVLC::GetPrediction( const CoeffArray& data , const int xpos , const int ypos ) const
{
    /* NB, 4.5.3 integer division
     * numbers are rounded down towards -ve infinity, differing from
     * C's convention that rounds towards 0
     */

    if (ypos!=0)
    {
        if (xpos!=0)
        {
            int sum = data[ypos][xpos-1] + data[ypos-1][xpos-1] + data[ypos-1][xpos] + 3/2;
            if (sum<0)
                return (sum-2)/3;
            else
                return sum/3;
        }
        else
            return data[ypos - 1][0];
    }
    else
    {
        if(xpos!=0)
            return data[0][xpos - 1];
        else
            return 0;
    }
}
