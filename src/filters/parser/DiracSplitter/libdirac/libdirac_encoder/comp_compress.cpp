/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: comp_compress.cpp,v 1.49 2009/01/21 05:18:09 asuraparaju Exp $ $Name:  $
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
#include <libdirac_common/band_codec.h>
#include <libdirac_common/band_vlc.h>
#include <libdirac_common/motion.h>

using namespace dirac;

#include <ctime>
#include <vector>
#include <iostream>

CompCompressor::CompCompressor( EncoderParams& encp,const PictureParams& pp)
: m_encparams(encp),
  m_pparams(pp),
  m_psort( m_pparams.PicSort() ),
  m_cformat( m_pparams.CFormat() )
{}

ComponentByteIO* CompCompressor::Compress( CoeffArray& coeff_data ,
                                           SubbandList& bands,
                                           CompSort csort,
                                           const OneDArray<unsigned int>& estimated_bits)
{
    // Need to transform, select quantisers for each band,
    // and then compress each component in turn

    unsigned int num_band_bytes( 0 );

    // create byte output
    ComponentByteIO *p_component_byteio = new ComponentByteIO(csort);

    // Loop over all the bands (from DC to HF) quantising and coding them
    for (int b=bands.Length() ; b>=1 ; --b )
    {

        // create subband byte io
        SubbandByteIO subband_byteio(bands(b));

        if ( !bands(b).Skipped() )
        {   // If not skipped ...
            if (m_pparams.UsingAC())
            {
                // A pointer to an object  for coding the subband data
                BandCodec* bcoder;


                 // Pick the right codec according to the picture type and subband
                if (b >= bands.Length()-3)
                {
                    if ( m_psort.IsIntra() && b == bands.Length() )
                        bcoder=new IntraDCBandCodec(&subband_byteio,
                                                TOTAL_COEFF_CTXS , bands );
                    else
                        bcoder=new LFBandCodec(&subband_byteio ,TOTAL_COEFF_CTXS,
                                           bands , b, m_psort.IsIntra());
                }
                else
                    bcoder=new BandCodec(&subband_byteio , TOTAL_COEFF_CTXS ,
                                         bands , b, m_psort.IsIntra() );

                num_band_bytes = bcoder->Compress(coeff_data);


                delete bcoder;
            }
            else
            {
                // A pointer to an object  for coding the subband data
                BandVLC* bcoder;

                   if ( m_psort.IsIntra() && b == bands.Length() )
                       bcoder=new IntraDCBandVLC(&subband_byteio, bands );
                else
                    bcoder=new BandVLC(&subband_byteio , 0, bands , b,
                                       m_psort.IsIntra() );

                num_band_bytes = bcoder->Compress(coeff_data);

                delete bcoder;
            }
             // Update the entropy correction factors
             m_encparams.EntropyFactors().Update(b , m_pparams , csort ,
                                            estimated_bits[b] , 8*num_band_bytes);
        }
        else
        {   // ... skipped
            SetToVal( coeff_data , bands(b) , 0 );
        }

            // output sub-band data
            p_component_byteio->AddSubband(&subband_byteio);


    }//b

    return p_component_byteio;
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
