/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: band_codec.cpp 280 2005-01-30 05:11:46Z gabest $ $Name$
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
* Contributor(s): Thomas Davies (Original Author), Scott R Ladd
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

#include <libdirac_common/band_codec.h>
using namespace dirac;

//! Constructor for encoding.
BandCodec::BandCodec(BasicOutputManager* bits_out,
                     size_t number_of_contexts,
                     const SubbandList & band_list,
                     int band_num):
    ArithCodec<PicArray>(bits_out,number_of_contexts),
    m_bnum(band_num),
    m_node(band_list(band_num)),
    m_xp(m_node.Xp()),
    m_yp(m_node.Yp()),
    m_xl(m_node.Xl()),
    m_yl(m_node.Yl()),
    m_vol(m_node.Xl()*m_node.Yl()),
    m_reset_coeff_num( std::max( 25 , std::min(m_vol/32,800) ) ),
    m_cut_off_point(m_node.Scale()>>1)
{
    if (m_node.Parent()!=0) 
        m_pnode=band_list(m_node.Parent());
}        

//! Constructor for decoding.
BandCodec::BandCodec(BitInputManager* bits_in,
                     size_t number_of_contexts,
                     const SubbandList& band_list,
                     int band_num):
    ArithCodec<PicArray>(bits_in,number_of_contexts),
    m_bnum(band_num),
    m_node(band_list(band_num)),
    m_xp(m_node.Xp()),
    m_yp(m_node.Yp()),
    m_xl(m_node.Xl()),
    m_yl(m_node.Yl()),
    m_vol(m_node.Xl()*m_node.Yl()),
    m_reset_coeff_num( std::max( 25 , std::min(m_vol/32,800) ) ),
    m_cut_off_point(m_node.Scale()>>1)
{
    if (m_node.Parent()!=0) m_pnode=band_list(m_node.Parent());
}

void BandCodec::InitContexts()
{
    //initialises the contexts. 
    //If _list does not already have values, then they're set to default values. 
    //This way, the constructor can override default initialisation.
    Context tmp_ctx;
    
    for (size_t i=0; i<m_context_list.size(); ++i)
    {
        if (i>=m_context_list.size())
            m_context_list.push_back(tmp_ctx);
        else
        {
            if (m_context_list[i].Weight()==0)
                m_context_list[i].SetCounts(1,1);
        }
    }
}

void BandCodec::ResetAll()
{
    for (unsigned int c = 0; c < m_context_list.size(); ++c)
        if (m_context_list[c].Weight()>16)
            m_context_list[c].HalveCounts();
}

void BandCodec::Resize(const int context_num)
{
    m_context_list[context_num].HalveCounts();
}

void BandCodec::Update( const bool symbol , const int context_num )
{
    m_context_list[context_num].IncrCount(symbol,1);
    
    if ( m_context_list[context_num].Weight() >= 1024 )
        Resize( context_num );
}

int BandCodec::ChooseContext(const PicArray& data) const{ return NZ_BIN5plus_CTX; }

//encoding function
void BandCodec::DoWorkCode(PicArray& in_data)
{

    //main coding function, using binarisation
    if (m_node.Parent()!=0)
    {
        m_pxp=m_pnode.Xp(); m_pyp=m_pnode.Yp();
        m_pxl=m_pnode.Xl(); m_pyl=m_pnode.Yl();
    }
    else
    {
        m_pxp=0; m_pyp=0;
        m_pxl=0; m_pyl=0;
    }
    
    ValueType val;
    m_qf=m_node.Qf(0);
    m_qfinv=(1<<17)/m_qf;
    m_offset=(3*m_qf+4)>>3;    
    m_cut_off_point*=m_qf;

    m_coeff_count=0;

    for (m_ypos=m_yp,m_pypos=m_pyp;m_ypos<m_yp+m_yl;++m_ypos,m_pypos=((m_ypos-m_yp)>>1)+m_pyp)
    {
        for (m_xpos=m_xp,m_pxpos=m_pxp;m_xpos<m_xp+m_xl;++m_xpos,m_pxpos=((m_xpos-m_xp)>>1)+m_pxp)
        {
            if (m_xpos==m_xp)
                m_nhood_sum = (m_ypos!=m_yp) ? std::abs(in_data[m_ypos-1][m_xpos]) : 0;
            else
                m_nhood_sum = (m_ypos!=m_yp) ? 
                (std::abs(in_data[m_ypos-1][m_xpos]) + std::abs(in_data[m_ypos][m_xpos-1])) 
               : std::abs(in_data[m_ypos][m_xpos-1]);
                
            m_parent_notzero = static_cast<bool> ( in_data[m_pypos][m_pxpos] );
            val = in_data[m_ypos][m_xpos];
            in_data[m_ypos][m_xpos]=0;
            CodeVal( in_data , val );

        }//m_xpos
    }//m_ypos    

#if defined(VERBOSE_DEBUG)
    // Show the symbol counts
    cerr<<endl<<"Context counts";
    
    for (int c = 0; c < 16; ++c)
         cerr << endl
             << c
             << ": Zero-"
             << ContextList()[c].get_count0()
             << ", One-"
             << ContextList()[c].get_count1();
#endif
}

void BandCodec::CodeVal( PicArray& in_data , const ValueType val )
{
    int abs_val( std::abs(val) );
    abs_val *= m_qfinv;
    abs_val >>= 17;

    for ( int bin=1 ; bin<=abs_val ; ++bin )
        EncodeSymbol( 0 , ChooseContext( in_data , bin ) );
    
    EncodeSymbol( 1 , ChooseContext( in_data , abs_val+1 ) );

    if ( abs_val )
    {
        abs_val *= m_qf;
        in_data[m_ypos][m_xpos] = static_cast<ValueType>( abs_val );                
        
        if ( val>0 )
        {
            EncodeSymbol( 1 , ChooseSignContext( in_data ) );
            in_data[m_ypos][m_xpos] += m_offset;
        }
        else
        {
            EncodeSymbol( 0 , ChooseSignContext(in_data) );
            in_data[m_ypos][m_xpos]  = -in_data[m_ypos][m_xpos];
            in_data[m_ypos][m_xpos] -= m_offset;
        }
    }
    
    m_coeff_count++;
    
    if (m_coeff_count > m_reset_coeff_num)
    {
        m_coeff_count=0;
        ResetAll();
    }
}

void BandCodec::DoWorkDecode(PicArray& out_data, int num_bits)
{

    if (m_node.Parent()!=0)
    {
        m_pxp = m_pnode.Xp();
        m_pyp = m_pnode.Yp();
        m_pxl = m_pnode.Xl();
        m_pyl = m_pnode.Yl();
    }
    else
    {
        m_pxp = 0;
        m_pyp = 0;
        m_pxl = 0;
        m_pyl = 0;
    }    

    m_qf = m_node.Qf(0);
    m_offset = (3 * m_qf + 4) >> 3;
    m_cut_off_point *= m_qf;

    //Work
    m_coeff_count=0;
    
    for (m_ypos=m_yp,m_pypos=m_pyp;m_ypos<m_yp+m_yl;++m_ypos,m_pypos=((m_ypos-m_yp)>>1)+m_pyp)
    {        
        for (m_xpos = m_xp, m_pxpos = m_pxp; m_xpos < m_xp+m_xl; ++m_xpos, m_pxpos=((m_xpos-m_xp)>>1)+m_pxp)
        {
            if (m_xpos == m_xp)
                m_nhood_sum=(m_ypos!=m_yp) ? std::abs(out_data[m_ypos-1][m_xpos]): 0;
            else
                m_nhood_sum=(m_ypos!=m_yp) ? 
                (std::abs(out_data[m_ypos-1][m_xpos]) + std::abs(out_data[m_ypos][m_xpos-1])) 
              : std::abs(out_data[m_ypos][m_xpos-1]);
            
            m_parent_notzero = static_cast<bool>( out_data[m_pypos][m_pxpos] );            
            DecodeVal(out_data);            
        }//m_xpos
    }//m_ypos
}

void BandCodec::DecodeVal(PicArray& out_data)
{
    ValueType val = 0;
    bool bit;
    int  bin = 1;
    
    do
    {
        DecodeSymbol(bit,ChooseContext(out_data,bin));
        
        if (!bit)
            val++;
        
        bin++;
    }
    while (!bit);            

    out_data[m_ypos][m_xpos] = val;
    
    if (out_data[m_ypos][m_xpos])
    {
        out_data[m_ypos][m_xpos] *= m_qf;
        out_data[m_ypos][m_xpos] += m_offset;
        DecodeSymbol( bit , ChooseSignContext(out_data) );
    }
    
    if (!bit)
        out_data[m_ypos][m_xpos]=-out_data[m_ypos][m_xpos];

    m_coeff_count++;
    
    if (m_coeff_count>m_reset_coeff_num)
    {
        ResetAll();
        m_coeff_count=0;
    }
}

int BandCodec::ChooseContext(const PicArray& data, const int BinNumber) const
{
    //condition on neighbouring values and parent values
    if (!m_parent_notzero && (m_pxp != 0 || m_pyp != 0))
    {
        if (BinNumber == 1)
        {
            if(m_nhood_sum == 0)
                return Z_BIN1z_CTX;
            else
                return Z_BIN1nz_CTX;
        }
        else if(BinNumber == 2)
            return Z_BIN2_CTX;
        else if(BinNumber == 3)
            return Z_BIN3_CTX;
        else if(BinNumber == 4)
            return Z_BIN4_CTX;
        else
            return Z_BIN5plus_CTX;
    }
    else
    {
        if (BinNumber == 1)
        {
            if(m_nhood_sum == 0)
                return NZ_BIN1z_CTX;
            else if (m_nhood_sum>m_cut_off_point)
                return NZ_BIN1b_CTX;
            else
                return NZ_BIN1a_CTX;
        }
        else if(BinNumber == 2)
            return NZ_BIN2_CTX;
        else if(BinNumber == 3)
            return NZ_BIN3_CTX;
        else if(BinNumber == 4)
            return NZ_BIN4_CTX;
        else
            return NZ_BIN5plus_CTX;
    }
}

int BandCodec::ChooseSignContext(const PicArray& data) const
{    
    if (m_yp == 0 && m_xp != 0)
    {
        //we're in a vertically oriented subband
        if (m_ypos == 0)
            return SIGN0_CTX;
        else
        {
            if (data[m_ypos-1][m_xpos]>0)
                return SIGN_POS_CTX;        
            else if (data[m_ypos-1][m_xpos]<0)
                return SIGN_NEG_CTX;
            else
                return SIGN0_CTX;
        }        
    }
    else if (m_xp == 0 && m_yp != 0)
    {
        //we're in a horizontally oriented subband
        if (m_xpos == 0)
            return SIGN0_CTX;
        else
        {
            if ( data[m_ypos][m_xpos-1] > 0 )
                return SIGN_POS_CTX;                
            else if ( data[m_ypos][m_xpos-1] < 0 )
                return SIGN_NEG_CTX;
            else
                return SIGN0_CTX;
        }
    }
    else
        return SIGN0_CTX;
}

//////////////////////////////////////////////////////////////////////////////////
//Now for special class for LF bands (since we don't want/can't refer to parent)//
//////////////////////////////////////////////////////////////////////////////////

void LFBandCodec::DoWorkCode(PicArray& in_data)
{
    //main coding function, using binarisation
    m_pxp = 0;
    m_pyp = 0;
    m_parent_notzero = false; //set parent to always be zero
    ValueType val;

    m_qf     = m_node.Qf(0);
    m_qfinv  = (1<<17)/m_qf;
    m_offset = (3*m_qf+4)>>3;
    m_cut_off_point*=m_qf;

    m_coeff_count = 0;
    
    for ( m_ypos=m_yp ; m_ypos<m_yp+m_yl ; ++m_ypos )
    {        
        for ( m_xpos=m_xp ; m_xpos<m_xp+m_xl ; ++m_xpos )
        {
            if ( m_xpos == m_xp )
                m_nhood_sum = (m_ypos!=m_yp) ? std::abs(in_data[m_ypos-1][m_xpos]) : 0;
            else
                m_nhood_sum = (m_ypos!=m_yp) ? 
                (std::abs(in_data[m_ypos-1][m_xpos]) + std::abs(in_data[m_ypos][m_xpos-1])) 
               : std::abs(in_data[m_ypos][m_xpos-1]);    
            
            val = in_data[m_ypos][m_xpos];
            in_data[m_ypos][m_xpos] = 0;
            CodeVal(in_data,val);            
        }//m_xpos
    }//m_ypos    
}

void LFBandCodec::DoWorkDecode(PicArray& out_data, int num_bits)
{
    m_pxp = 0;
    m_pyp = 0;
    m_parent_notzero = false;//set parent to always be zero    
    m_qf = m_node.Qf(0);
    m_offset = (3*m_qf+4)>>3;
    m_cut_off_point *= m_qf;

    //Work
    m_coeff_count = 0;
    
    for ( m_ypos=m_yp ; m_ypos<m_yp+m_yl ; ++m_ypos )
    {
        for ( m_xpos=0 ; m_xpos<m_xp+m_xl; ++m_xpos )
        {
            if ( m_xpos == m_xp )
                m_nhood_sum=(m_ypos!=m_yp) ? std::abs(out_data[m_ypos-1][m_xpos]) : 0;
            else
                m_nhood_sum=(m_ypos!=m_yp) ? 
                (std::abs(out_data[m_ypos-1][m_xpos]) + std::abs(out_data[m_ypos][m_xpos-1])) 
               : std::abs(out_data[m_ypos][m_xpos-1]);

            DecodeVal(out_data);            
        }//m_xpos
    }//m_ypos
}

//////////////////////////////////////////////////////////////////////////////////
//Finally,special class incorporating prediction for the DC band of intra frames//
//////////////////////////////////////////////////////////////////////////////////

void IntraDCBandCodec::DoWorkCode(PicArray& in_data)
{
    //main coding function, using binarisation
    m_pxp = 0;
    m_pyp = 0;

    //set parent to always be zero
    m_parent_notzero = false;
    ValueType val;
    
    //residues after prediction, quantisation and inverse quant
    PicArray pred_res(m_yl , m_xl);
    ValueType prediction;

    m_qf     = m_node.Qf(0);
    m_qfinv  = (1<<17) / m_qf;
    m_offset = (3*m_qf+4) >> 3;
    m_cut_off_point *= m_qf;

    m_coeff_count=0;
    
    for (m_ypos=m_yp; m_ypos < m_yp + m_yl; ++m_ypos)
    {
        for (m_xpos = m_xp; m_xpos < m_xp + m_xl; ++m_xpos)
        {
             if (m_xpos == m_xp)
                m_nhood_sum = (m_ypos!=m_yp) ? std::abs(pred_res[m_ypos-1][m_xpos]) : 0;
             else
                 m_nhood_sum = (m_ypos!=m_yp) ? 
                               (std::abs(pred_res[m_ypos-1][m_xpos]) + std::abs(pred_res[m_ypos][m_xpos-1])) 
                              : std::abs(pred_res[m_ypos][m_xpos-1]);
          
            prediction = GetPrediction(in_data);            
            val = in_data[m_ypos][m_xpos]-prediction;
            in_data[m_ypos][m_xpos] = 0;
            CodeVal(in_data,val);            
            pred_res[m_ypos][m_xpos] = in_data[m_ypos][m_xpos];
            in_data[m_ypos][m_xpos] += prediction;
        }//m_xpos

#if defined(VERBOSE_DEBUG)
         cerr << endl
             << "Val at "
             << m_ypos
             << " "
             << m_xl-1
             << " : "
             << in_data[m_ypos][m_xl-1]
             << endl 
             << "Contexts at "
             << m_ypos 
             << " " 
             << m_xl-1 
             << " : ";
             
         for (int c=0;c<18;++c)
             cerr << endl
                 << c 
                 << ": Zero "
                 << ContextList()[c].get_count0()
                 << ", One "
                 << ContextList()[c].get_count1();    
#endif
            
    }//m_ypos    
}

void IntraDCBandCodec::DoWorkDecode(PicArray& out_data, int num_bits)
{
    m_pxp = 0;
    m_pyp = 0;
    m_parent_notzero = false; //set parent to always be zero

    //residues after prediction, quantisation and inverse quant
    PicArray pred_res(m_yl , m_xl); 

    m_qf = m_node.Qf(0);
    m_offset = (3*m_qf+4)>>3;
    m_cut_off_point *= m_qf;

    //Work
    m_coeff_count=0;
    
    for (m_ypos=m_yp;m_ypos<m_yp+m_yl;++m_ypos)
    {
        for (m_xpos=0;m_xpos<m_xp+m_xl;++m_xpos)
        {
             if (m_xpos==m_xp)
                 m_nhood_sum=(m_ypos!=m_yp) ? std::abs(pred_res[m_ypos - 1][m_xpos]) : 0;
             else
                 m_nhood_sum=(m_ypos!=m_yp) ? 
                             (std::abs(pred_res[m_ypos - 1][m_xpos]) + std::abs(pred_res[m_ypos][m_xpos - 1]))
                            : std::abs(pred_res[m_ypos][m_xpos-1]);
          
            DecodeVal(out_data);
             pred_res[m_ypos][m_xpos]=out_data[m_ypos][m_xpos];
            out_data[m_ypos][m_xpos]+=GetPrediction(out_data);
        }//m_xpos
    }//m_ypos
}

ValueType IntraDCBandCodec::GetPrediction(const PicArray& data) const
{
    if (m_ypos!=0)
    {
        if (m_xpos!=0)
            return (data[m_ypos][m_xpos - 1] + data[m_ypos - 1][m_xpos - 1] + data[m_ypos - 1][m_xpos]) / 3;
        else
            return data[m_ypos - 1][0];
    }
    else
    {
        if(m_xpos!=0)
            return data[0][m_xpos - 1];
        else
            return 2692; // TODO: What does this mean? Literal constants like this are dangerous!
    }
}
