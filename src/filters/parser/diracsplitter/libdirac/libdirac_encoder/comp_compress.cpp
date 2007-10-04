/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: comp_compress.cpp,v 1.40 2007/09/26 12:18:43 asuraparaju Exp $ $Name: Dirac_0_8_0 $
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
* Contributor(s): Thomas Davies (Original Author),
*                 Scott R Ladd,
*                 Anuradha Suraparaju
*                 Andrew Kennedy
*                 Tim Borer
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


//Compression of an individual component,
//after motion compensation if appropriate
//////////////////////////////////////////

#include <libdirac_encoder/comp_compress.h>
#include <libdirac_encoder/quant_chooser.h>
#include <libdirac_common/band_codec.h>
#include <libdirac_common/motion.h>

using namespace dirac;

#include <ctime>
#include <vector>
#include <iostream>

CompCompressor::CompCompressor( EncoderParams& encp,const FrameParams& fp)
: m_encparams(encp),
  m_fparams(fp),
  m_fsort( m_fparams.FSort() ),
  m_cformat( m_fparams.CFormat() )
{}

ComponentByteIO* CompCompressor::Compress( PicArray& pic_data ,
                                           const bool is_a_cut ,
                                           const double intra_ratio ,
                                           MEData* me_data )
{
    //need to transform, select quantisers for each band, and then compress each component in turn
    m_csort=pic_data.CSort();
    const int depth=m_encparams.TransformDepth();
    unsigned int num_band_bytes( 0 );
    m_me_data = me_data;

    // A pointer to an object  for coding the subband data
    BandCodec* bcoder;

    Subband node;

    //set up Lagrangian params
    SetCompLambda( intra_ratio, is_a_cut );

    WaveletTransform wtransform( depth , m_encparams.TransformFilter() );

    CoeffArray coeff_data( pic_data.LengthY(), pic_data.LengthX() );
    wtransform.Transform( FORWARD , pic_data, coeff_data );

    // Choose all the quantisers //
    ///////////////////////////////
    SubbandList& bands = wtransform.BandList();

    // Set up the code blocks
    SetupCodeBlocks( bands );

    wtransform.SetBandWeights( m_encparams.CPD() , m_fparams.FSort() , m_fparams.CFormat(), m_csort, m_encparams.Interlace());

    OneDArray<unsigned int> estimated_bits( Range( 1 , bands.Length() ) );

    SelectQuantisers( coeff_data , bands , estimated_bits , m_encparams.GetCodeBlockMode() );

    // create byte output
    ComponentByteIO *p_component_byteio = new ComponentByteIO(m_csort);

    // Loop over all the bands (from DC to HF) quantising and coding them
    for (int b=bands.Length() ; b>=1 ; --b )
    {
        // create subband byte io
        SubbandByteIO subband_byteio(bands(b));

        if ( !bands(b).Skipped() )
        {   // If not skipped ...

             // Pick the right codec according to the frame type and subband
            if (b >= bands.Length()-3)
            {
                if ( m_fsort.IsIntra() && b == bands.Length() )
                    bcoder=new IntraDCBandCodec(&subband_byteio,
                                                TOTAL_COEFF_CTXS , bands );
                else
                    bcoder=new LFBandCodec(&subband_byteio ,TOTAL_COEFF_CTXS,
                                           bands , b, m_fsort.IsIntra());
            }
            else
                bcoder=new BandCodec(&subband_byteio , TOTAL_COEFF_CTXS ,
                                     bands , b, m_fsort.IsIntra() );

            num_band_bytes = bcoder->Compress(coeff_data);

             // Update the entropy correction factors
            m_encparams.EntropyFactors().Update(b , m_fsort , m_csort , estimated_bits[b] , 8*num_band_bytes);

            delete bcoder;
        }
        else
        {   // ... skipped
            SetToVal( coeff_data , bands(b) , 0 );
        }

            // output sub-band data
            p_component_byteio->AddSubband(&subband_byteio);


    }//b

    if ( m_fsort.IsIntra() || m_fsort.IsRef() || m_encparams.LocalDecode() )
    {
        // Transform back into the picture domain
        wtransform.Transform( BACKWARD , pic_data, coeff_data );
    }

    return p_component_byteio;
}

void CompCompressor::SetCompLambda( const double intra_ratio, const bool is_a_cut )
{
    if ( m_fsort.IsIntra() )
    {
        m_lambda= m_encparams.ILambda();
        if ( is_a_cut )
        {
            // The intra frame is inserted so we can lower the quality
            m_lambda *= 8;

        }
    }
    else
    {
        double log_intra_lambda = std::log10( m_encparams.ILambda() );
        double log_frame_lambda;

        if (m_fparams.IsBFrame() )
            log_frame_lambda= std::log10( m_encparams.L2Lambda() );
        else
            log_frame_lambda= std::log10( m_encparams.L1Lambda() );


        m_lambda= std::pow(10.0, ( (intra_ratio*log_intra_lambda+
                           (100.0-intra_ratio)*log_frame_lambda )/100.0) );
    }


    if (m_csort == U_COMP)
        m_lambda*= m_encparams.UFactor();
    if (m_csort == V_COMP)
        m_lambda*= m_encparams.VFactor();
}

void CompCompressor::SetupCodeBlocks( SubbandList& bands )
{
    int xregions;
    int yregions;

    // The maximum number of regions horizontally and vertically


    for (int band_num = 1; band_num<=bands.Length() ; ++band_num)
    {
        if (m_encparams.SpatialPartition())
        {
            int level = m_encparams.TransformDepth() - (band_num-1)/3;
            const CodeBlocks &cb = m_encparams.GetCodeBlocks(level);
            xregions = cb.HorizontalCodeBlocks();
            yregions = cb.VerticalCodeBlocks();
        }
        else
        {
               xregions = 1;
               yregions = 1;
        }

        bands( band_num ).SetNumBlocks( yregions , xregions );
    }// band_num
}

void CompCompressor::SelectQuantisers( CoeffArray& coeff_data ,
                                       SubbandList& bands ,
                                       OneDArray<unsigned int>& est_bits,
                                       const CodeBlockMode cb_mode )
{

   // Set up the multiquantiser mode
    for ( int b=bands.Length() ; b>=1 ; --b )
    {
        // Set multiquants flag in the subband only if
        // a. Global m_cb_mode flag is set to QUANT_MULTIPLE in encparams
        //           and
        // b. Current subband has more than one block
        if (
            cb_mode == QUANT_MULTIPLE &&
            (bands(b).GetCodeBlocks().LengthX() > 1  ||
            bands(b).GetCodeBlocks().LengthY() > 1)
           )
            bands(b).SetUsingMultiQuants( true );
        else
            bands(b).SetUsingMultiQuants( false );
    }// b

    // Select all the quantizers
    if ( !m_encparams.Lossless() )
    {
        // Set the DC band quantiser to be 1
        bands( bands.Length() ).SetQIndex( 0 );
        bands( bands.Length() ).SetSkip( false );
        bands( bands.Length() ).SetUsingMultiQuants( false );
        est_bits[ bands.Length()] = 0;
        TwoDArray<CodeBlock>& blocks = bands( bands.Length() ).GetCodeBlocks();
        for (int j=0; j<blocks.LengthY(); ++j)
            for (int i=0 ; i<blocks.LengthX(); ++i )
                blocks[j][i].SetQIndex( 0 );

        // Now do the rest of the bands.
        for ( int b=bands.Length()-1 ; b>=1 ; --b )
        {            est_bits[b] = SelectMultiQuants( coeff_data , bands , b );
        }// b
    }
    else
    {
        for ( int b=bands.Length() ; b>=1 ; --b )
        {
            bands(b).SetQIndex( 0 );
            TwoDArray<CodeBlock>& blocks = bands(b).GetCodeBlocks();
            for (int j=0; j<blocks.LengthY() ;++j)
            {
                for (int i=0; i<blocks.LengthX() ;++i)
                {
                    blocks[j][i].SetQIndex( 0 );
                }// i
            }// j
        }// b
    }
}

int CompCompressor::SelectMultiQuants( CoeffArray& coeff_data , SubbandList& bands , const int band_num )
{
    Subband& node( bands( band_num ) );

    // Now select the quantisers //
    ///////////////////////////////

    QuantChooser qchooser( coeff_data , m_lambda );

    // For the DC band in I frames, remove the average
    if ( band_num == bands.Length() && m_fsort.IsIntra() )
        AddSubAverage( coeff_data , node.Xl() , node.Yl() , SUBTRACT);

    // The total estimated bits for the subband
    int band_bits( 0 );
    qchooser.SetEntropyCorrection( m_encparams.EntropyFactors().Factor( band_num , m_fsort , m_csort ) );
    band_bits = qchooser.GetBestQuant( node );

    // Put the DC band average back in if necessary
    if ( band_num == bands.Length() && m_fsort.IsIntra() )
        AddSubAverage( coeff_data , node.Xl() , node.Yl() , ADD);

    if ( band_bits == 0 )
        node.SetSkip( true );
    else
        node.SetSkip( false );

    return band_bits;
}



void CompCompressor::SetToVal(CoeffArray& coeff_data,const Subband& node,ValueType val)
{

    for (int j=node.Yp() ; j<node.Yp() + node.Yl() ; ++j)
    {
        for (int i=node.Xp(); i<node.Xp() + node.Xl() ; ++i)
        {
            coeff_data[j][i] = val;
        }// i
    }// j

}


void CompCompressor::AddSubAverage( CoeffArray& coeff_data ,
                                    int xl ,
                                    int yl ,
                                    AddOrSub dirn)
{

    ValueType last_val=0;
    ValueType last_val2;

    if ( dirn == SUBTRACT )
    {
        for ( int j=0 ; j<yl ; j++)
            {
            for ( int i=0 ; i<xl ; i++)
                {
                last_val2 = coeff_data[j][i];
                coeff_data[j][i] -= last_val;
                last_val = last_val2;
            }// i
        }// j
    }
    else
    {
        for ( int j=0 ; j<yl ; j++)
        {
            for ( int i=0 ; i<xl; i++ )
            {
                coeff_data[j][i] += last_val;
                last_val = coeff_data[j][i];
            }// i
        }// j

    }
}
