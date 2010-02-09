/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: mv_codec.cpp,v 1.35 2008/10/01 01:26:47 asuraparaju Exp $ $Name:  $
*
* Version: MPL 1.1/GPL 2.0/LGPL 2.1
*
* The contents of this file are subject to the Mozilla Public License
* Version 1.1 (the "License");  you may not use this file except in compliance
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
* Contributor(s): Thomas Davies (Original Author),
*                 Scott R Ladd,
*                 Tim Borer,
*                 Andrew Kennedy,
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

#include <libdirac_common/mv_codec.h>

using namespace dirac;

//public functions//
////////////////////
// Constructor 
SplitModeCodec::SplitModeCodec(ByteIO* p_byteio,
                         size_t number_of_contexts)
  : ArithCodec <MvData> (p_byteio,number_of_contexts)
{}        



void SplitModeCodec::InitContexts() 
{    
}

// Main code function
void SplitModeCodec::DoWorkCode( MvData& in_data )
{
    for (m_sb_yp = 0;  m_sb_yp < in_data.SBSplit().LengthY();  ++m_sb_yp)
    {
        for (m_sb_xp = 0; m_sb_xp < in_data.SBSplit().LengthX(); ++m_sb_xp)
        {
            CodeVal(in_data);
        }//m_sb_xp
    }//m_sb_yp
} 

// Main decode function
void SplitModeCodec::DoWorkDecode( MvData& out_data)
{

    for (m_sb_yp = 0; m_sb_yp < out_data.SBSplit().LengthY(); ++m_sb_yp)
    {
        for (m_sb_xp = 0; m_sb_xp < out_data.SBSplit().LengthX(); ++m_sb_xp)
        {
            DecodeVal( out_data );
        }//m_sb_xp
    }//m_sb_yp
}  

//protected functions//
///////////////////////

void SplitModeCodec::ResetAll()
{
}

//coding functions//
////////////////////

//prediction functions

unsigned int SplitModeCodec::Prediction(const TwoDArray<int> & split_data ) const
{    
    int result = 0;
    
    std::vector < unsigned int >  nbrs;
    
    if (m_sb_xp > 0 && m_sb_yp > 0)
    {
        nbrs.push_back( split_data[m_sb_yp-1][m_sb_xp] ); 
        nbrs.push_back( split_data[m_sb_yp-1][m_sb_xp-1] ); 
        nbrs.push_back( split_data[m_sb_yp][m_sb_xp-1] ); 

        result = GetUMean(nbrs);     
    }
    else if (m_sb_xp > 0 && m_sb_yp == 0)
        result = split_data[m_sb_yp][m_sb_xp-1]; 
    else if (m_sb_xp == 0 && m_sb_yp > 0)
        result =  split_data[m_sb_yp-1][m_sb_xp]; 

    return result; 
}


void SplitModeCodec::CodeVal(const MvData& in_data)
{
    int val = in_data.SBSplit()[m_sb_yp][m_sb_xp] - Prediction( in_data.SBSplit() ); 
    
    if (val < 0) val+=3; //produce prediction mod 3

    EncodeUInt(val, SB_SPLIT_BIN1_CTX, SB_SPLIT_BIN2_CTX);
}

//decoding functions//
//////////////////////



void SplitModeCodec::DecodeVal(MvData& out_data)
{
    out_data.SBSplit()[m_sb_yp][m_sb_xp] =
                             (DecodeUInt(SB_SPLIT_BIN1_CTX, SB_SPLIT_BIN2_CTX) +
                             Prediction(out_data.SBSplit())) % 3;
}

/******************************************************************************/

//public functions//
////////////////////
// Constructor 
PredModeCodec::PredModeCodec(ByteIO* p_byteio,
                         size_t number_of_contexts,
			 int num_refs)
  : ArithCodec <MvData> (p_byteio,number_of_contexts),
    m_num_refs(num_refs)
{}        



void PredModeCodec::InitContexts() 
{    
}

// Main code function
void PredModeCodec::DoWorkCode( MvData& in_data )
{
    int step,max; 
    int split_depth;  

    for (m_sb_yp = 0, m_sb_tlb_y = 0;  m_sb_yp < in_data.SBSplit().LengthY();  ++m_sb_yp, m_sb_tlb_y += 4)
    {
        for (m_sb_xp = 0,m_sb_tlb_x = 0; m_sb_xp < in_data.SBSplit().LengthX(); ++m_sb_xp,m_sb_tlb_x += 4)
        {
            split_depth = in_data.SBSplit()[m_sb_yp][m_sb_xp]; 

            step = 4  >>  (split_depth); 
            max = (1 << split_depth); 
                        
            //now do all the block modes and mvs in the mb            
            for (m_b_yp = m_sb_tlb_y; m_b_yp < m_sb_tlb_y+4; m_b_yp += step)
            {
                for (m_b_xp = m_sb_tlb_x; m_b_xp < m_sb_tlb_x+4; m_b_xp += step)
                {
                    CodeVal(in_data);
                }//m_b_xp
            }//m_b_yp    
            
        }//m_sb_xp
    }//m_sb_yp    
} 

// Main decode function
void PredModeCodec::DoWorkDecode( MvData& out_data)
{
    int step,max; 
    int split_depth; 
    int xstart,ystart;     

    // Then the prediction mode
    for (m_sb_yp = 0,m_sb_tlb_y = 0; m_sb_yp < out_data.SBSplit().LengthY(); ++m_sb_yp,m_sb_tlb_y += 4)
    {
        for (m_sb_xp = 0,m_sb_tlb_x = 0; m_sb_xp < out_data.SBSplit().LengthX(); ++m_sb_xp,m_sb_tlb_x += 4)
        {
            split_depth = out_data.SBSplit()[m_sb_yp][m_sb_xp]; 
            step =  4  >>  (split_depth); 
            max  = (1 << split_depth); 

            //now do all the block mvs in the mb
            for (int j = 0; j < max; ++j)
            {                
                for (int i = 0; i < max; ++i)
                {
                    xstart = m_b_xp = m_sb_tlb_x + i * step; 
                    ystart = m_b_yp = m_sb_tlb_y + j * step;                                             
                    
                    DecodeVal(out_data); 

                    // propagate throughout SB    
                    for (m_b_yp = ystart; m_b_yp < ystart+step; m_b_yp++)
                    {
                        for (m_b_xp = xstart; m_b_xp < xstart+step; m_b_xp++)
                        {                    
                            out_data.Mode()[m_b_yp][m_b_xp] = out_data.Mode()[ystart][xstart]; 
                        }//m_b_xp
                    }//m_b_yp
                }//i                    
            }//j

        }//m_sb_xp
    }//m_sb_yp
}  

//protected functions//
///////////////////////

void PredModeCodec::ResetAll()
{
}

//coding functions//
////////////////////

//prediction functions

unsigned int PredModeCodec::Prediction(const TwoDArray < PredMode > & preddata) const
{
    unsigned int result = (unsigned int)(INTRA);
    unsigned int num_ref1_nbrs( 0 ); 
    unsigned int num_ref2_nbrs( 0 );
    
    if (m_b_xp > 0 && m_b_yp > 0)
    {
        num_ref1_nbrs += ((unsigned int)( preddata[m_b_yp-1][m_b_xp] ) ) & 1; 
        num_ref1_nbrs += ((unsigned int)( preddata[m_b_yp-1][m_b_xp-1] ) ) & 1; 
        num_ref1_nbrs += ((unsigned int)( preddata[m_b_yp][m_b_xp-1] ) ) & 1;

        result = num_ref1_nbrs>>1;

        if ( m_num_refs==2)
        {
            num_ref2_nbrs += ((unsigned int)( preddata[m_b_yp-1][m_b_xp] ) ) & 2; 
            num_ref2_nbrs += ((unsigned int)( preddata[m_b_yp-1][m_b_xp-1] ) ) & 2; 
            num_ref2_nbrs += ((unsigned int)( preddata[m_b_yp][m_b_xp-1] ) ) & 2; 
            num_ref2_nbrs >>= 1;
            result ^= ( (num_ref2_nbrs>>1)<<1 );
        }
    }
    else if (m_b_xp > 0 && m_b_yp == 0)
        result = (unsigned int)( preddata[0][m_b_xp-1] );
    else if (m_b_xp == 0 && m_b_yp > 0)
        result = (unsigned int)( preddata[m_b_yp-1][0] );

    return result;
}

void PredModeCodec::CodeVal(const MvData& in_data)
{
    // Xor with the prediction so we predict whether REF1 is used or REF2 is
    // used, separately
    unsigned int residue = in_data.Mode()[m_b_yp][m_b_xp] ^
                                Prediction( in_data.Mode() );

    // Code REF1 part of the prediction residue (ie the first bit)
    EncodeSymbol( residue & 1 , PMODE_BIT0_CTX );

    // Code REF2 part of the prediction residue (ie the second bit)
    if (m_num_refs==2)
    {
        EncodeSymbol( residue & 2 , PMODE_BIT1_CTX );
    }

}

//decoding functions//
//////////////////////

void PredModeCodec::DecodeVal( MvData& out_data )
{
    // Xor with the prediction so we predict whether REF1 is used or REF2 is
    // used, separately
    unsigned int residue;

    // Decode REF1 part of the prediction residue (ie the first bit)
    bool bit;
    bit = DecodeSymbol( PMODE_BIT0_CTX );
    residue = (unsigned int) bit;

    // Decode REF2 part of the prediction residue (ie the second bit)
    if (m_num_refs==2)
    {
        bit = DecodeSymbol( PMODE_BIT1_CTX );
        residue |= ( (unsigned int) bit ) << 1;
    }

    out_data.Mode()[m_b_yp][m_b_xp] =
        PredMode( Prediction( out_data.Mode() ) ^ residue );
}

/******************************************************************************/


//public functions//
////////////////////
// Constructor
VectorElementCodec::VectorElementCodec(ByteIO* p_byteio,
                                       int ref_id,
                                       MvElement horvert,
                                       size_t number_of_contexts)
  : ArithCodec <MvData> (p_byteio,number_of_contexts),
  m_ref(ref_id),
  m_hv(horvert)
{}



void VectorElementCodec::InitContexts()
{}

// Main code function
void VectorElementCodec::DoWorkCode( MvData& in_data )
{
    int step,max;
    int split_depth;

    for (m_sb_yp = 0, m_sb_tlb_y = 0;  m_sb_yp < in_data.SBSplit().LengthY();  ++m_sb_yp, m_sb_tlb_y += 4)
    {
        for (m_sb_xp = 0,m_sb_tlb_x = 0; m_sb_xp < in_data.SBSplit().LengthX(); ++m_sb_xp,m_sb_tlb_x += 4)
        {
            split_depth = in_data.SBSplit()[m_sb_yp][m_sb_xp];

            step = 4  >>  (split_depth);
            max = (1 << split_depth);
                        
            //now do all the block modes and mvs in the mb            
            for (m_b_yp = m_sb_tlb_y; m_b_yp < m_sb_tlb_y+4; m_b_yp += step)
            {
                for (m_b_xp = m_sb_tlb_x; m_b_xp < m_sb_tlb_x+4; m_b_xp += step)
                {
                    if ( in_data.Mode()[m_b_yp][m_b_xp] & m_ref )
                    {
                        CodeVal(in_data);
                    } 
                }//m_b_xp
            }//m_b_yp    
                            
        }//m_sb_xp
    }//m_sb_yp
} 

// Main decode function
void VectorElementCodec::DoWorkDecode( MvData& out_data)
{
    int step,max; 
    int split_depth; 
    int xstart,ystart;     

    for (m_sb_yp = 0,m_sb_tlb_y = 0; m_sb_yp < out_data.SBSplit().LengthY(); ++m_sb_yp,m_sb_tlb_y += 4)
    {
        for (m_sb_xp = 0,m_sb_tlb_x = 0; m_sb_xp < out_data.SBSplit().LengthX(); ++m_sb_xp,m_sb_tlb_x += 4)
        {
            split_depth = out_data.SBSplit()[m_sb_yp][m_sb_xp]; 
            step =  4  >>  (split_depth); 
            max  = (1 << split_depth); 

            //now do all the block mvs in the mb
            for (int j = 0; j < max; ++j)
            {                
                for (int i = 0; i < max; ++i)
                {
                    xstart = m_b_xp = m_sb_tlb_x + i * step; 
                    ystart = m_b_yp = m_sb_tlb_y + j * step;                                             
                    
                    if (out_data.Mode()[m_b_yp][m_b_xp] & m_ref)
                    {
                        DecodeVal( out_data ); 
                    }
                                        
                      // propagate throughout SB    
                     for (m_b_yp = ystart; m_b_yp < ystart+step; m_b_yp++)
                    {
                        for (m_b_xp = xstart; m_b_xp < xstart+step; m_b_xp++)
                        {                    
                            out_data.Vectors(m_ref)[m_b_yp][m_b_xp][m_hv] = 
                                  out_data.Vectors(m_ref)[ystart][xstart][m_hv]; 

                        }//m_b_xp
                    }//m_b_yp
                }//i                    
            }//j

        }//m_sb_xp
    }//m_sb_yp
}  

//protected functions//
///////////////////////

void VectorElementCodec::ResetAll()
{
}

//coding functions//
////////////////////

//prediction functions

int VectorElementCodec::Prediction(const MvArray& mvarray,
                                  const TwoDArray < PredMode > & preddata) const
{
    std::vector <int>  nbrs; 
    PredMode pmode;     
    int result( 0 ); 
    
    if (m_b_xp > 0 && m_b_yp > 0)
    {
        pmode = preddata[m_b_yp-1][m_b_xp]; 
        if (pmode & m_ref) 
            nbrs.push_back(mvarray[m_b_yp-1][m_b_xp][m_hv]); 
        
        pmode = preddata[m_b_yp-1][m_b_xp-1]; 
        if (pmode & m_ref)
            nbrs.push_back(mvarray[m_b_yp-1][m_b_xp-1][m_hv]); 
        
        pmode = preddata[m_b_yp][m_b_xp-1]; 
        if (pmode & m_ref)
            nbrs.push_back(mvarray[m_b_yp][m_b_xp-1][m_hv]);
        
        if (nbrs.size() > 0)
            result = Median(nbrs); 
    }
    else if (m_b_xp > 0 && m_b_yp == 0)
    {
        pmode = preddata[0][m_b_xp-1]; 
        if (pmode & m_ref)
            result = mvarray[0][m_b_xp-1][m_hv]; 
    }
    else if (m_b_xp == 0 && m_b_yp > 0)
    {
        pmode = preddata[m_b_yp-1][0]; 
        if (pmode & m_ref)
            result = mvarray[m_b_yp-1][0][m_hv];
    }
    return result; 
}

void VectorElementCodec::CodeVal(const MvData& in_data )
{
    const MvArray& mv_array = in_data.Vectors(m_ref);
    const int pred = Prediction( mv_array , in_data.Mode() ); 
    const int val = mv_array[m_b_yp][m_b_xp][m_hv] - pred;

    EncodeSInt(val, MV_FBIN1_CTX, MV_FBIN5plus_CTX);
}

//decoding functions//
//////////////////////


void VectorElementCodec::DecodeVal( MvData& out_data )
{
    MvArray& mv_array = out_data.Vectors(m_ref);    
    int pred = Prediction( mv_array , out_data.Mode() );
    mv_array[m_b_yp][m_b_xp][m_hv] = pred + 
                                     DecodeSInt(MV_FBIN1_CTX, MV_FBIN5plus_CTX);

}

/******************************************************************************/
//public functions//
////////////////////
// Constructor 
DCCodec::DCCodec(ByteIO* p_byteio,
                         const CompSort csort,
                         size_t number_of_contexts):
ArithCodec <MvData> (p_byteio,number_of_contexts),
m_csort( csort )
{}        



void DCCodec::InitContexts() 
{    
}

// Main code function
void DCCodec::DoWorkCode( MvData& in_data )
{
    int step,max; 
    int split_depth;  
    
    for (m_sb_yp = 0, m_sb_tlb_y = 0;  m_sb_yp < in_data.SBSplit().LengthY();  ++m_sb_yp, m_sb_tlb_y += 4)
    {
        for (m_sb_xp = 0,m_sb_tlb_x = 0; m_sb_xp < in_data.SBSplit().LengthX(); ++m_sb_xp,m_sb_tlb_x += 4)
        {
            split_depth = in_data.SBSplit()[m_sb_yp][m_sb_xp]; 

            step = 4  >>  (split_depth); 
            max = (1 << split_depth); 
                        
            //now do all the block modes and mvs in the mb            
            for (m_b_yp = m_sb_tlb_y; m_b_yp < m_sb_tlb_y+4; m_b_yp += step)
            {
                for (m_b_xp = m_sb_tlb_x; m_b_xp < m_sb_tlb_x+4; m_b_xp += step)
                {
                    if(in_data.Mode()[m_b_yp][m_b_xp] == INTRA)
                    {
                        CodeVal(in_data);
                    }
                }//m_b_xp
            }//m_b_yp    
                            
        }//m_sb_xp
    }//m_sb_yp
} 

// Main decode function
void DCCodec::DoWorkDecode( MvData& out_data)
{
    int step,max; 
    int split_depth; 
    int xstart,ystart;     

    for (m_sb_yp = 0,m_sb_tlb_y = 0; m_sb_yp < out_data.SBSplit().LengthY(); ++m_sb_yp,m_sb_tlb_y += 4)
    {
        for (m_sb_xp = 0,m_sb_tlb_x = 0; m_sb_xp < out_data.SBSplit().LengthX(); ++m_sb_xp,m_sb_tlb_x += 4)
        {
             //start with split mode
            split_depth = out_data.SBSplit()[m_sb_yp][m_sb_xp]; 
            step =  4  >>  (split_depth); 
            max  = (1 << split_depth); 

            //now do all the block mvs in the mb
            for (int j = 0; j < max; ++j)
            {                
                for (int i = 0; i < max; ++i)
                {
                    xstart = m_b_xp = m_sb_tlb_x + i * step; 
                    ystart = m_b_yp = m_sb_tlb_y + j * step;                                             
                    
                    if(out_data.Mode()[m_b_yp][m_b_xp] == INTRA)
                    {
                        DecodeVal( out_data ); 
                    }
                    
                      // propagate throughout SB    
                     for (m_b_yp = ystart; m_b_yp < ystart+step; m_b_yp++)
                    {
                        for (m_b_xp = xstart; m_b_xp < xstart+step; m_b_xp++)
                        {                    
                            out_data.DC( m_csort )[m_b_yp][m_b_xp] = out_data.DC( m_csort )[ystart][xstart]; 
                        }//m_b_xp
                    }//m_b_yp
                }//i                    
            }//j

        }//m_sb_xp
    }//m_sb_yp

}  

//protected functions//
///////////////////////

void DCCodec::ResetAll()
{
}

//coding functions//
////////////////////

//prediction functions

ValueType DCCodec::Prediction(const TwoDArray < ValueType > & dcdata,
                                    const TwoDArray < PredMode > & preddata) const
{
    std::vector < int >  nbrs; 
    PredMode pmode;
    ValueType result = 0; 
    
    if (m_b_xp > 0 && m_b_yp > 0)
    {
        pmode = preddata[m_b_yp-1][m_b_xp]; 
        if (pmode == INTRA) 
            nbrs.push_back( (int) dcdata[m_b_yp-1][m_b_xp] ); 
        
        pmode = preddata[m_b_yp-1][m_b_xp-1]; 
        if (pmode == INTRA)
            nbrs.push_back((int)dcdata[m_b_yp-1][m_b_xp-1] ); 
        
        pmode = preddata[m_b_yp][m_b_xp-1]; 
        if (pmode == INTRA)        
            nbrs.push_back( (int) dcdata[m_b_yp][m_b_xp-1] ); 
        
        if (nbrs.size() > 0)
            result = ValueType(GetSMean(nbrs));     
    }
    else if (m_b_xp > 0 && m_b_yp == 0)
    {
        pmode = preddata[0][m_b_xp-1]; 
        if (pmode == INTRA)
            result = dcdata[0][m_b_xp-1]; 
    }
    else if (m_b_xp == 0 && m_b_yp > 0)
    {
        pmode = preddata[m_b_yp-1][0]; 
        if (pmode == INTRA)
            result = dcdata[m_b_yp-1][0]; 
    }
    return result;
}

void DCCodec::CodeVal(const MvData& in_data)
{    
    const int val = in_data.DC( m_csort )[m_b_yp][m_b_xp] -
                             Prediction( in_data.DC(m_csort) , in_data.Mode() );
    EncodeSInt(val, DC_FBIN1_CTX, DC_FBIN2plus_CTX);
}

//decoding functions//
//////////////////////

void DCCodec::DecodeVal( MvData& out_data )
{
    out_data.DC( m_csort )[m_b_yp][m_b_xp] = DecodeSInt(DC_FBIN1_CTX, DC_FBIN2plus_CTX) +
                            Prediction(out_data.DC( m_csort ), out_data.Mode());
}
