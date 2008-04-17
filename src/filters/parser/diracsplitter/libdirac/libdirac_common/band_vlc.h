/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: band_vlc.h,v 1.2 2007/12/13 14:49:47 tjdwave Exp $ $Name: Dirac_0_9_1 $
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
*                 Steve Bearcroft
*                 Andrew Kennedy
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

#ifndef _BAND_VLC_H
#define _BAND_VLC_H

#include <libdirac_common/wavelet_utils.h>


namespace dirac
{

   class SubbandByteIO;
   class ByteIO;


    //! A general class for coding and decoding wavelet subband data using variable length coding.
    /*!
        A general class for coding and decoding wavelet subband data using variable length coding,
     */
    class BandVLC
    {
    public:

        //! Constructor 
        /*!
            Creates a BandVLC object to encode subband data
            \param    subband_byteio   input/output for the encoded bits
            \param    band_list    the set of all the subbands
            \param    band_num    the number of the subband being coded 
            \param    is_intra    Flag indicating whether the band comes from an intra frame
         */
        BandVLC(SubbandByteIO* subband_byteio,
                  const SubbandList& band_list,
                  int band_num,
                  const bool is_intra);


        void Decompress (CoeffArray& out_data, int num_bytes);

        int Compress (CoeffArray &in_data);

        virtual ~BandVLC(){}

    protected:
        //! Code an individual quantised value and perform inverse-quantisation
        inline void CodeVal( CoeffArray& in_data , const int xpos , const int ypos , const CoeffType val);

        //! Decode an individual quantised value and perform inverse-quantisation
        inline void DecodeVal(CoeffArray& out_data , const int xpos , const int ypos );

        //! Encode the offset for a code block quantiser
        void CodeQIndexOffset( const int offset );

        //! Decode the offset for a code block quantiser
        int DecodeQIndexOffset();

        //! Set a code block area to a given value
        inline void SetToVal( const CodeBlock& code_block , CoeffArray& coeff_data , const CoeffType val);

        //! Set all block values to 0
        inline void ClearBlock( const CodeBlock& code_block , CoeffArray& coeff_data);
    private:
        //functions
        // Overridden from the base class
        virtual void DoWorkCode(CoeffArray& in_data);
        // Ditto
        virtual void DoWorkDecode(CoeffArray& out_data);

        virtual void CodeCoeffBlock(const CodeBlock& code_block , CoeffArray& in_data);
        virtual void DecodeCoeffBlock(const CodeBlock& code_block , CoeffArray& out_data);

        //! Private, bodyless copy constructor: class should not be copied
        BandVLC(const BandVLC& cpy);
        //! Private, bodyless copy operator=: class should not be assigned
        BandVLC& operator=(const BandVLC& rhs);

    protected:
              
        //! Flag indicating whether the band comes from an intra frame
        bool m_is_intra;
    
        //! variables    
        int m_bnum;

        //! the subband being coded
        const Subband m_node;
    
        //! the quantisation index of the last codeblock
        int m_last_qf_idx;
            
        //! quantisation value
        int m_qf;
    
        //! reconstruction point
        CoeffType m_offset;

        //! Input/output stream of Dirac-format bytes
        ByteIO *m_byteio;
    
    };

    //////////////////////////////////////////////////////////////////////////////////
    //Finally,special class incorporating prediction for the DC band of intra frames//
    //////////////////////////////////////////////////////////////////////////////////

    //! A class specially for coding the DC subband of Intra frames 
    /*!
        A class specially for coding the DC subband of Intra frames, using intra-band prediction 
        of coefficients.
    */
    class IntraDCBandVLC: public BandVLC
    {
    public:
        //! Constructor
        /*!
            Creates a IntraDCBandVLC object to encode subband data, based on parameters
            \param    subband_byteio input/output for the encoded bits
            \param    band_list    the set of all the subbands
         */
        IntraDCBandVLC(SubbandByteIO* subband_byteio,
                         const SubbandList& band_list)
          : BandVLC(subband_byteio,band_list,
                      band_list.Length(), true){}

    
    private:
        void DoWorkCode(CoeffArray& in_data);                    //overridden from the base class
        void DoWorkDecode(CoeffArray& out_data); //ditto

        void CodeCoeffBlock(const CodeBlock& code_block , CoeffArray& in_data);
        void DecodeCoeffBlock(const CodeBlock& code_block , CoeffArray& out_data);

        //! Private, bodyless copy constructor: class should not be copied
        IntraDCBandVLC (const IntraDCBandVLC& cpy); 

        //! Private, bodyless copy operator=: class should not be assigned
        IntraDCBandVLC& operator=(const IntraDCBandVLC& rhs);

        //! Prediction of a DC value from its previously coded neighbours
        CoeffType GetPrediction(const CoeffArray& data , const int xpos , const int ypos ) const;
    };


}// end namespace dirac
#endif
