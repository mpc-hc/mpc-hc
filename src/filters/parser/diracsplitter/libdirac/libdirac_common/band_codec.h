/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: band_codec.h,v 1.26 2007/07/26 12:46:35 tjdwave Exp $ $Name: Dirac_0_9_1 $
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

#ifndef _BAND_CODEC_H_
#define _BAND_CODEC_H_

#include <libdirac_common/arith_codec.h>
#include <libdirac_common/wavelet_utils.h>


namespace dirac
{

   class SubbandByteIO;

    //Subclasses the arithmetic codec to produce a coding/decoding tool for subbands


    //! A general class for coding and decoding wavelet subband data.
    /*!
        A general class for coding and decoding wavelet subband data, deriving from the abstract ArithCodec class.
     */
    class BandCodec: public ArithCodec<CoeffArray>
    {
    public:

        //! Constructor 
        /*!
            Creates a BandCodec object to encode subband data
            \param    subband_byteio   input/output for the encoded bits
            \param    number_of_contexts the number of contexts used in the encoding process
            \param    band_list    the set of all the subbands
            \param    band_num    the number of the subband being coded 
            \param    is_intra    Flag indicating whether the band comes from an intra frame
         */
        BandCodec(SubbandByteIO* subband_byteio,
                  size_t number_of_contexts,
                  const SubbandList& band_list,
                  int band_num,
                  const bool is_intra);

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

        //! A function for choosing the context for "follow bits"
        inline int ChooseFollowContext( const int bin_number ) const;

        //! A function for choosing the context for "information bits"
        inline int ChooseInfoContext() const;

        //! A function for choosing the context for sign bits
        inline int ChooseSignContext(const CoeffArray& data , const int xpos , const int ypos ) const;

        //! Private, bodyless copy constructor: class should not be copied
        BandCodec(const BandCodec& cpy);
        //! Private, bodyless copy operator=: class should not be assigned
        BandCodec& operator=(const BandCodec& rhs);

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

        //! True if neighbours non-zero
        bool m_nhood_nonzero;
    
        //! the parent subband
        Subband m_pnode;
    
        //! coords of the parent subband
        int m_pxp, m_pyp, m_pxl, m_pyl;
    
        //! position of the parent coefficient
        int m_pxpos, m_pypos;
    
        //! True if the parent of a coeff is not zero
        bool m_parent_notzero;
    
    };

    //! A class specially for coding the LF subbands 
    /*!
        A class specially for coding the LF subbands, where we don't want to/can't refer to the 
        parent subband.
    */
    class LFBandCodec: public BandCodec
    {
    public:
        //! Constructor
        /*!
            Creates a LFBandCodec object to encode subband data.
            \param    subband_byteio input/output for the encoded bits
            \param    number_of_contexts the number of contexts used in the encoding process
            \param    band_list    the set of all the subbands
            \param    band_num    the number of the subband being coded 
            \param    is_intra    Flag indicating whether the band comes from an intra frame
         */        
        LFBandCodec(SubbandByteIO* subband_byteio,
                    size_t number_of_contexts,
                    const SubbandList& band_list,
                    int band_num,
                    const bool is_intra)
                    : BandCodec(subband_byteio,number_of_contexts,band_list,
                                band_num,is_intra){}

    private:
        // Overridden from the base class
        void DoWorkCode(CoeffArray& in_data);
        // Ditto
        void DoWorkDecode(CoeffArray& out_data);

        void CodeCoeffBlock(const CodeBlock& code_block , CoeffArray& in_data);
        void DecodeCoeffBlock(const CodeBlock& code_block , CoeffArray& out_data);

        //! Private, bodyless copy constructor: class should not be copied
        LFBandCodec(const LFBandCodec& cpy);
        //! Private, bodyless copy operator=: class should not be assigned
        LFBandCodec& operator=(const LFBandCodec& rhs);

    };


    //////////////////////////////////////////////////////////////////////////////////
    //Finally,special class incorporating prediction for the DC band of intra frames//
    //////////////////////////////////////////////////////////////////////////////////

    //! A class specially for coding the DC subband of Intra frames 
    /*!
        A class specially for coding the DC subband of Intra frames, using intra-band prediction 
        of coefficients.
    */
    class IntraDCBandCodec: public BandCodec
    {
    public:
        //! Constructor
        /*!
            Creates a IntraDCBandCodec object to encode subband data, based on parameters
            \param    subband_byteio input/output for the encoded bits
            \param    number_of_contexts the number of contexts used in the encoding process
            \param    band_list    the set of all the subbands
         */
        IntraDCBandCodec(SubbandByteIO* subband_byteio,
                         size_t number_of_contexts,
                         const SubbandList& band_list)
          : BandCodec(subband_byteio,number_of_contexts,band_list,
                      band_list.Length(), true){}

    
    private:
        void DoWorkCode(CoeffArray& in_data);                    //overridden from the base class
        void DoWorkDecode(CoeffArray& out_data); //ditto

        void CodeCoeffBlock(const CodeBlock& code_block , CoeffArray& in_data);
        void DecodeCoeffBlock(const CodeBlock& code_block , CoeffArray& out_data);

        //! Private, bodyless copy constructor: class should not be copied
        IntraDCBandCodec(const IntraDCBandCodec& cpy); 

        //! Private, bodyless copy operator=: class should not be assigned
        IntraDCBandCodec& operator=(const IntraDCBandCodec& rhs);

        //! Prediction of a DC value from its previously coded neighbours
        CoeffType GetPrediction(const CoeffArray& data , const int xpos , const int ypos ) const;

    private:
        CoeffArray m_dc_pred_res;
    };


}// end namespace dirac
#endif
