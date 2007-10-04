/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: comp_decompress.cpp,v 1.27 2007/07/26 12:46:35 tjdwave Exp $ $Name: Dirac_0_8_0 $
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
*                 Anuradha Suraparaju,
*                 Andrew Kennedy,
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


#include <libdirac_decoder/comp_decompress.h>
#include <libdirac_common/wavelet_utils.h>
#include <libdirac_common/band_codec.h>
using namespace dirac;

#include <vector>

#include <ctime>

using std::vector;

//Constructor
CompDecompressor::CompDecompressor( DecoderParams& decp, const FrameParams& fp)
:
    m_decparams(decp),
    m_fparams(fp)
{}


void CompDecompressor::Decompress(ComponentByteIO* p_component_byteio,
                                  PicArray& pic_data )
{
    const FrameSort& fsort=m_fparams.FSort();
    const int depth( m_decparams.TransformDepth() );

    // A pointer to the object(s) we'll be using for coding the bands
    BandCodec* bdecoder;
    
    // The array holding the coefficients
    CoeffArray coeff_data( pic_data.LengthY(), pic_data.LengthX() );

    WaveletTransform wtransform( depth , m_decparams.TransformFilter() );
    SubbandList& bands=wtransform.BandList();

    // Initialise all the subbands
    bands.Init(depth , coeff_data.LengthX() , coeff_data.LengthY());

    // Set up the code blocks
    SetupCodeBlocks( bands );

    for ( int b=bands.Length() ; b>=1 ; --b )
    {
        // Multiple quantiser are used only if
        // a. The global code_block_mode is QUANT_MULTIPLE
        //              and
        // b. More than one code block is present in the subband.
        bands(b).SetUsingMultiQuants( 
                           m_decparams.SpatialPartition() &&
                           m_decparams.GetCodeBlockMode() == QUANT_MULTIPLE &&
                           (bands(b).GetCodeBlocks().LengthX() > 1 ||
                           bands(b).GetCodeBlocks().LengthY() > 1)
                                );

        // Read the header data first
        SubbandByteIO subband_byteio(bands(b), *p_component_byteio);
        subband_byteio.Input();
        //std::cout << "Subband Num=" << b << "Arithdata size=" << subband_byteio.GetBandDataLength() << std::endl;

        if ( !bands(b).Skipped() )
        {
            if ( b>=bands.Length()-3)
            {
                if ( fsort.IsIntra() && b==bands.Length() )
                    bdecoder=new IntraDCBandCodec(&subband_byteio, 
                                                   TOTAL_COEFF_CTXS ,bands);
                else
                    bdecoder=new LFBandCodec(&subband_byteio , TOTAL_COEFF_CTXS,
                                             bands , b, fsort.IsIntra());
            }
            else
                bdecoder=new BandCodec( &subband_byteio , TOTAL_COEFF_CTXS ,
                                        bands , b, fsort.IsIntra());

            bdecoder->Decompress(coeff_data , subband_byteio.GetBandDataLength());
            delete bdecoder;
        }
        else
        {
#if 0
            if ( b==bands.Length() && fsort.IsIntra() )
                SetToVal( coeff_data , bands(b) , wtransform.GetMeanDCVal() );
            else
                SetToVal( coeff_data , bands(b) , 0 );
#else
            SetToVal( coeff_data , bands(b) , 0 );
#endif
        }
    }
    
    wtransform.Transform(BACKWARD,pic_data, coeff_data);
}

void CompDecompressor::SetupCodeBlocks( SubbandList& bands )
{
    int xregions;
    int yregions;

    for (int band_num = 1; band_num<=bands.Length() ; ++band_num)
    {
        if (m_decparams.SpatialPartition())
        {
            int level = m_decparams.TransformDepth() - (band_num-1)/3;
            const CodeBlocks &cb = m_decparams.GetCodeBlocks(level);
            xregions = cb.HorizontalCodeBlocks();
            yregions = cb.VerticalCodeBlocks();
        }
        else
        {
               xregions = 1;
               yregions = 1;
        }

        bands( band_num ).SetNumBlocks( yregions ,xregions );

    }// band_num
}

void CompDecompressor::SetToVal( CoeffArray& coeff_data , 
                                 const Subband& node , 
                                 CoeffType val )
{

    for (int j=node.Yp() ; j<node.Yp()+node.Yl() ; ++j)
        for (int i=node.Xp() ; i<node.Xp()+node.Xl() ; ++i)
            coeff_data[j][i]=val;

}
