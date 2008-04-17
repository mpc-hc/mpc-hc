/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: band_codec.cpp,v 1.40 2007/12/05 01:23:43 asuraparaju Exp $ $Name: Dirac_0_9_1 $
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
* Contributor(s): Thomas Davies (Original Author),
*                 Scott R Ladd,
*                 Steve Bearcroft
*                 Andrew Kennedy
*                 Anuradha Suraparaju
*                 David Schleef
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
#include <libdirac_common/band_codec.h>
#include <libdirac_byteio/subband_byteio.h>
#include <libdirac_common/dirac_exception.h>

using namespace dirac;

//! Constructor for encoding.
BandCodec::BandCodec(SubbandByteIO* subband_byteio,
                     size_t number_of_contexts,
                     const SubbandList & band_list,
                     int band_num,
                     const bool is_intra):
    ArithCodec<CoeffArray>(subband_byteio,number_of_contexts),
    m_is_intra(is_intra),
    m_bnum(band_num),
    m_node(band_list(band_num)),
    m_last_qf_idx(m_node.QIndex())
{
    if (m_node.Parent()!=0) 
        m_pnode=band_list(m_node.Parent());
}


//encoding function
void BandCodec::DoWorkCode(CoeffArray& in_data)
{

    const TwoDArray<CodeBlock>& block_list( m_node.GetCodeBlocks() );

    if (m_node.Parent() != 0)
    {
        m_pxp = m_pnode.Xp();
        m_pyp = m_pnode.Yp();
    }
    else
    {
        m_pxp = 0;
        m_pyp = 0;
    }

    // Now loop over the blocks and code
    bool code_skip = (block_list.LengthX() > 1 || block_list.LengthY() > 1);
    for (int j=block_list.FirstY() ; j<=block_list.LastY() ; ++j)
    {
        CodeBlock *block = block_list[j];
        for (int i=block_list.FirstX() ; i<=block_list.LastX() ; ++i)
        {
            if (code_skip)
                EncodeSymbol(block[i].Skipped() , BLOCK_SKIP_CTX );
            if ( !block[i].Skipped() )
                CodeCoeffBlock( block[i] , in_data );
            else
                ClearBlock (block[i] , in_data);
        }// i
    }// j

}

void BandCodec::CodeCoeffBlock( const CodeBlock& code_block , CoeffArray& in_data )
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
        m_pypos=(( ypos-m_node.Yp() )>>1)+m_pnode.Yp();
        for ( int xpos=xbeg; xpos<xend ;++xpos)
        {
            m_pxpos=(( xpos-m_node.Xp() )>>1)+m_pnode.Xp();

            m_nhood_nonzero = false;
            if (ypos > m_node.Yp())
                m_nhood_nonzero |= bool(in_data[ypos-1][xpos]);
            if (xpos > m_node.Xp())
                m_nhood_nonzero |= bool(in_data[ypos][xpos-1]);
            if (ypos > m_node.Yp() && xpos > m_node.Xp())
                m_nhood_nonzero |= bool(in_data[ypos-1][xpos-1]);

            m_parent_notzero = static_cast<bool> ( in_data[m_pypos][m_pxpos] );

            CodeVal( in_data , xpos , ypos , in_data[ypos][xpos] );

        }// xpos
    }// ypos    

}

/*
Coefficient magnitude value and differential quantiser index magnitude are
coded using interleaved exp-Golomb coding for binarisation. In this scheme, a
value N>=0 is coded by writing N+1 in binary form of a 1 followed by K other
bits: 1bbbbbbb (adding 1 ensures there'll be a leading 1). These K bits ("info
bits") are interleaved with K zeroes ("follow bits") each of which means
"another bit coming", followed by a terminating 1:
 
    0b0b0b ...0b1
 
(Conventional exp-Golomb coding has the K zeroes at the beginning, followed
by the 1 i.e 00...01bb .. b, but interleaving allows the decoder to run a
single loop and avoid counting the number of zeroes, sparing a register.)

All bits are arithmetically coded. The follow bits have separate contexts
based on position, and have different contexts from the info bits. 
*/

inline void BandCodec::CodeVal( CoeffArray& in_data , 
                                const int xpos , 
                                const int ypos , 
                                const CoeffType val )
{
    unsigned int abs_val( std::abs(val) );
    abs_val <<= 2;
    abs_val /= m_qf;

    const int N = abs_val+1;
    int num_follow_zeroes=0;

    while ( N >= (1<<num_follow_zeroes) )
        ++num_follow_zeroes;
    --num_follow_zeroes; 

    for ( int i=num_follow_zeroes-1, c=1; i>=0; --i, ++c )
    {
        EncodeSymbol( 0, ChooseFollowContext( c ) );
        EncodeSymbol( N&(1<<i), ChooseInfoContext() );
    }
    EncodeSymbol( 1, ChooseFollowContext( num_follow_zeroes+1 ) );

    in_data[ypos][xpos] = static_cast<CoeffType>( abs_val );

    if ( abs_val )
    {
        // Must code sign bits and reconstruct
        in_data[ypos][xpos] *= m_qf;
        in_data[ypos][xpos] += m_offset+2;
        in_data[ypos][xpos] >>= 2;

        if ( val>0 )
        {
            EncodeSymbol( 0 , ChooseSignContext( in_data , xpos , ypos ) );
        }
        else
        {
            EncodeSymbol( 1 , ChooseSignContext( in_data , xpos , ypos ) );
            in_data[ypos][xpos]  = -in_data[ypos][xpos];
        }
    }
}

void BandCodec::CodeQIndexOffset( const int offset )
{

    const int abs_val = std::abs( offset );

    int N = abs_val+1;
    int num_follow_zeroes=0;

    while ( N>= (1<<num_follow_zeroes) )
        ++num_follow_zeroes;
    --num_follow_zeroes;

    for ( int i=num_follow_zeroes-1, c=1; i>=0; --i, ++c )
    {
        EncodeSymbol( 0 , Q_OFFSET_FOLLOW_CTX );
        EncodeSymbol( N&(1<<i), Q_OFFSET_INFO_CTX );
    }
    EncodeSymbol( 1 , Q_OFFSET_FOLLOW_CTX );

    if ( abs_val )
    {
        if ( offset>0 )
            EncodeSymbol( 0 , Q_OFFSET_SIGN_CTX );
        else
            EncodeSymbol( 1 , Q_OFFSET_SIGN_CTX );
    }
}

void BandCodec::DoWorkDecode( CoeffArray& out_data )
{
    if (m_node.Parent() != 0)
    {
        m_pxp = m_pnode.Xp();
        m_pyp = m_pnode.Yp();
    }
    else
    {
        m_pxp = 0;
        m_pyp = 0;
    }

    const TwoDArray<CodeBlock>& block_list( m_node.GetCodeBlocks() );

    // Now loop over the blocks and decode
    bool decode_skip= (block_list.LengthX() > 1 || block_list.LengthY() > 1);
    for (int j=block_list.FirstY() ; j<=block_list.LastY() ; ++j)
    {
        CodeBlock *block = block_list[j];
        for (int i=block_list.FirstX() ; i<=block_list.LastX() ; ++i)
        {
            if (decode_skip)
                block[i].SetSkip( DecodeSymbol( BLOCK_SKIP_CTX ) );
            if ( !block[i].Skipped() )
                DecodeCoeffBlock( block[i] , out_data );
            else
                ClearBlock (block[i] , out_data);

        }// i
    }// j

}

void BandCodec::DecodeCoeffBlock( const CodeBlock& code_block , CoeffArray& out_data )
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
    m_pypos=(( ypos-m_node.Yp() )>>1)+m_pnode.Yp();
        CoeffType *p_out_data = out_data[m_pypos];
        CoeffType *c_out_data_1 = NULL;
        if (ypos!=m_node.Yp())
            c_out_data_1 = out_data[ypos-1];
        CoeffType *c_out_data_2 = out_data[ypos];
        for ( int xpos=xbeg; xpos<xend ;++xpos)
        {
        m_pxpos=(( xpos-m_node.Xp() )>>1)+m_pnode.Xp();

            m_nhood_nonzero = false;
            /* c_out_data_1 is the line above the current
             * c_out_data_2 is the current line */ 
            if (ypos > m_node.Yp())
                m_nhood_nonzero |= bool(c_out_data_1[xpos]);
            if (xpos > m_node.Xp())
                m_nhood_nonzero |= bool(c_out_data_2[xpos-1]);
            if (ypos > m_node.Yp() && xpos > m_node.Xp())
                m_nhood_nonzero |= bool(c_out_data_1[xpos-1]);

            m_parent_notzero = ( p_out_data[m_pxpos] != 0 );            
            DecodeVal( out_data , xpos , ypos );

        }// xpos
    }// ypos
}


/*
Coefficient magnitude value and differential quantiser index value is coded
using interleaved exp-Golomb coding for binarisation. In this scheme, a value
N>=0 is coded by writing N+1 in binary form of a 1 followed by K other bits:
1bbbbbbb (adding 1 ensures there'll be a leading 1). These K bits ("info bits")
are interleaved with K zeroes ("follow bits") each of which means "another bit
coming", followed by a terminating 1:
 
    0b0b0b ...0b1
 
(Conventional exp-Golomb coding has the K zeroes at the beginning, followed
by the 1 i.e 00...01bb .. b, but interleaving allows the decoder to run a
single loop and avoid counting the number of zeroes, sparing a register.)

All bits are arithmetically coded. The follow bits have separate contexts
based on position, and have different contexts from the info bits. 
*/
inline void BandCodec::DecodeVal( CoeffArray& out_data , const int xpos , const int ypos )
{

    CoeffType& out_pixel = out_data[ypos][xpos];

    out_pixel = 1;
    int bit_count=1;
    
    while ( !DecodeSymbol( ChooseFollowContext( bit_count ) ) )
    {
        out_pixel <<= 1;
        out_pixel |= DecodeSymbol( ChooseInfoContext() );
        bit_count++;
    };
    --out_pixel;

    if ( out_pixel )
    {
        out_pixel *= m_qf;
        out_pixel += m_offset+2;
        out_pixel >>= 2;
     
        if ( DecodeSymbol( ChooseSignContext(out_data, xpos, ypos)) )
            out_pixel = -out_pixel;
    }
}

inline int BandCodec::ChooseFollowContext( const int bin_number ) const 
{
    //condition on neighbouring values and parent values

    if (!m_parent_notzero)
    {
        switch ( bin_number )
        {
            case 1 :
                if(m_nhood_nonzero == false)
                    return Z_FBIN1z_CTX;

                return Z_FBIN1nz_CTX;

            case 2 :
                return Z_FBIN2_CTX;
            case 3 :
                return Z_FBIN3_CTX;
            case 4 :
                return Z_FBIN4_CTX;
            case 5 :
                return Z_FBIN5_CTX;
            default :
                return Z_FBIN6plus_CTX;
        }
    }
    else
    {
        switch ( bin_number )
        {
            case 1 :
                if(m_nhood_nonzero == false)
                    return NZ_FBIN1z_CTX;

                return NZ_FBIN1nz_CTX;

            case 2 :
                return NZ_FBIN2_CTX;
            case 3 :
                return NZ_FBIN3_CTX;
            case 4 :
                return NZ_FBIN4_CTX;
            case 5 :
                return NZ_FBIN5_CTX;
            default :
                return NZ_FBIN6plus_CTX;
        }

    }

}

inline int BandCodec::ChooseInfoContext() const 
{
    return INFO_CTX;
}

inline int BandCodec::ChooseSignContext( const CoeffArray& data , const int xpos , const int ypos ) const
{    
    if ( m_node.Yp()==0 && m_node.Xp()!=0 )
    {
        //we're in a vertically oriented subband
        if (ypos == 0)
            return SIGN0_CTX;
        else
        {
            if (data[ypos-1][xpos]>0)
                return SIGN_POS_CTX;        
            else if (data[ypos-1][xpos]<0)
                return SIGN_NEG_CTX;
            else
                return SIGN0_CTX;
        }        
    }
    else if ( m_node.Xp()==0 && m_node.Yp()!=0 )
    {
        //we're in a horizontally oriented subband
        if (xpos == 0)
            return SIGN0_CTX;
        else
        {
            if ( data[ypos][xpos-1] > 0 )
                return SIGN_POS_CTX;                
            else if ( data[ypos][xpos-1] < 0 )
                return SIGN_NEG_CTX;
            else
                return SIGN0_CTX;
        }
    }
    else
        return SIGN0_CTX;
}

int BandCodec::DecodeQIndexOffset()
{
    int offset = 1;

    while ( !DecodeSymbol( Q_OFFSET_FOLLOW_CTX ) )
    {
        offset <<= 1;
        offset |= DecodeSymbol( Q_OFFSET_INFO_CTX );
    }
    --offset;

    if ( offset )
    {
        if ( DecodeSymbol( Q_OFFSET_SIGN_CTX ) )
            offset = -offset;
    }
    return offset;
}

void BandCodec::SetToVal( const CodeBlock& code_block , CoeffArray& pic_data , const CoeffType val)
{
    for (int j=code_block.Ystart() ; j<code_block.Yend() ; j++)
    {
          for (int i=code_block.Xstart() ; i<code_block.Xend() ; i++)
           {
            pic_data[j][i] = val;
           }// i
    }// j
}

void BandCodec::ClearBlock( const CodeBlock& code_block , CoeffArray& coeff_data)
{
    for (int j=code_block.Ystart() ; j<code_block.Yend() ; j++)
    {
        CoeffType *pic = &coeff_data[j][code_block.Xstart()];
        memset (pic, 0, (code_block.Xend()-code_block.Xstart())*sizeof(CoeffType));
    }// j

}
//////////////////////////////////////////////////////////////////////////////////
//Now for special class for LF bands (since we don't want/can't refer to parent)//
//////////////////////////////////////////////////////////////////////////////////

void LFBandCodec::DoWorkCode(CoeffArray& in_data)
{

    m_pxp = 0;
    m_pyp = 0;

    const TwoDArray<CodeBlock>& block_list( m_node.GetCodeBlocks() );

    // Now loop over the blocks and code
    bool code_skip= (block_list.LengthX() > 1 || block_list.LengthY() > 1);
    for (int j=block_list.FirstY() ; j<=block_list.LastY() ; ++j)
    {
        for (int i=block_list.FirstX() ; i<=block_list.LastX() ; ++i)
        {
            if (code_skip)
                EncodeSymbol(block_list[j][i].Skipped() , BLOCK_SKIP_CTX );
            if ( !block_list[j][i].Skipped() )
                CodeCoeffBlock( block_list[j][i] , in_data );
            else
                ClearBlock (block_list[j][i] , in_data);
        }// i
    }// j
}

void LFBandCodec::CodeCoeffBlock( const CodeBlock& code_block , CoeffArray& in_data )
{
    //main coding function, using binarisation
    const int xbeg = code_block.Xstart();
    const int ybeg = code_block.Ystart();
    const int xend = code_block.Xend();
    const int yend = code_block.Yend();

    m_parent_notzero = false; //set parent to always be zero

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

    for ( int ypos=ybeg ; ypos<yend ; ++ypos )
    {        
        for ( int xpos=xbeg ; xpos<xend ; ++xpos )
        {
            m_nhood_nonzero = false;
            if (ypos > m_node.Yp())
                m_nhood_nonzero |= bool(in_data[ypos-1][xpos]);
            if (xpos > m_node.Xp())
                m_nhood_nonzero |= bool(in_data[ypos][xpos-1]);
            if (ypos > m_node.Yp() && xpos > m_node.Xp())
                m_nhood_nonzero |= bool(in_data[ypos-1][xpos-1]);
            
            CodeVal( in_data , xpos , ypos , in_data[ypos][xpos] );

        }//xpos
    }//ypos    
}


void LFBandCodec::DoWorkDecode(CoeffArray& out_data )
{
    m_pxp = 0;
    m_pyp = 0;

    const TwoDArray<CodeBlock>& block_list( m_node.GetCodeBlocks() );

    // Now loop over the blocks and decode
    bool decode_skip= (block_list.LengthX() > 1 || block_list.LengthY() > 1);
    for (int j=block_list.FirstY() ; j<=block_list.LastY() ; ++j)
    {
        for (int i=block_list.FirstX() ; i<=block_list.LastX() ; ++i)
        {
            if (decode_skip)
                block_list[j][i].SetSkip( DecodeSymbol( BLOCK_SKIP_CTX ) );
            if ( !block_list[j][i].Skipped() )
                DecodeCoeffBlock( block_list[j][i] , out_data );
            else
                ClearBlock (block_list[j][i] , out_data);
        }// i
    }// j

}

void LFBandCodec::DecodeCoeffBlock( const CodeBlock& code_block , CoeffArray& out_data )
{

    const int xbeg = code_block.Xstart();
    const int ybeg = code_block.Ystart();
    const int xend = code_block.Xend();
    const int yend = code_block.Yend();

    m_parent_notzero = false;//set parent to always be zero    

    int qf_idx = m_node.QIndex();

    if ( m_node.UsingMultiQuants() )
    {
        qf_idx += m_last_qf_idx+DecodeQIndexOffset(); 
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
    
    for ( int ypos=ybeg ; ypos<yend ; ++ypos )
    {
        for ( int xpos=xbeg ; xpos<xend; ++xpos )
        {
            m_nhood_nonzero = false;
            if (ypos > m_node.Yp())
                m_nhood_nonzero |= bool(out_data[ypos-1][xpos]);
            if (xpos > m_node.Xp())
                m_nhood_nonzero |= bool(out_data[ypos][xpos-1]);
            if (ypos > m_node.Yp() && xpos > m_node.Xp())
                m_nhood_nonzero |= bool(out_data[ypos-1][xpos-1]);

            DecodeVal( out_data , xpos , ypos );

        }// xpos
    }// ypos
}


//////////////////////////////////////////////////////////////////////////////////
//Finally,special class incorporating prediction for the DC band of intra frames//
//////////////////////////////////////////////////////////////////////////////////

void IntraDCBandCodec::DoWorkCode(CoeffArray& in_data)
{

    m_pxp = 0;
    m_pyp = 0;

    // Residues after prediction, quantisation and inverse quantisation
    m_dc_pred_res.Resize( m_node.Yl() , m_node.Xl() );

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

void IntraDCBandCodec::CodeCoeffBlock( const CodeBlock& code_block , CoeffArray& in_data)
{
    // Main coding function, using binarisation
    const int xbeg = code_block.Xstart();
    const int ybeg = code_block.Ystart();
    const int xend = code_block.Xend();
    const int yend = code_block.Yend();

    //set parent to always be zero
    m_parent_notzero = false;
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
            m_nhood_nonzero = false;
            if (ypos > m_node.Yp())
                m_nhood_nonzero |= bool(m_dc_pred_res[ypos-1][xpos]);
            if (xpos > m_node.Xp())
                m_nhood_nonzero |= bool(m_dc_pred_res[ypos][xpos-1]);
            if (ypos > m_node.Yp() && xpos > m_node.Xp())
                m_nhood_nonzero |= bool(m_dc_pred_res[ypos-1][xpos-1]);
          
            prediction = GetPrediction( in_data , xpos , ypos );            
            val = in_data[ypos][xpos] - prediction;
            CodeVal( in_data , xpos , ypos , val );            
            m_dc_pred_res[ypos][xpos] = in_data[ypos][xpos];
            in_data[ypos][xpos] += prediction;
        }//xpos            
    }//ypos    
}


void IntraDCBandCodec::DoWorkDecode(CoeffArray& out_data)
{

    m_pxp = 0;
    m_pyp = 0;

    m_dc_pred_res.Resize( m_node.Yl() , m_node.Xl() );

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

void IntraDCBandCodec::DecodeCoeffBlock( const CodeBlock& code_block , CoeffArray& out_data)
{
    const int xbeg = code_block.Xstart();
    const int ybeg = code_block.Ystart();
    const int xend = code_block.Xend();
    const int yend = code_block.Yend();

    m_parent_notzero = false; //set parent to always be zero

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
            m_nhood_nonzero = false;
            if (ypos > m_node.Yp())
                m_nhood_nonzero |= bool(m_dc_pred_res[ypos-1][xpos]);
            if (xpos > m_node.Xp())
                m_nhood_nonzero |= bool(m_dc_pred_res[ypos][xpos-1]);
            if (ypos > m_node.Yp() && xpos > m_node.Xp())
                m_nhood_nonzero |= bool(m_dc_pred_res[ypos-1][xpos-1]);

             DecodeVal( out_data , xpos , ypos );
             m_dc_pred_res[ypos][xpos] = out_data[ypos][xpos];
             out_data[ypos][xpos] += GetPrediction( out_data , xpos , ypos );

        }//xpos
    }//ypos
}


CoeffType IntraDCBandCodec::GetPrediction( const CoeffArray& data , const int xpos , const int ypos ) const
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
