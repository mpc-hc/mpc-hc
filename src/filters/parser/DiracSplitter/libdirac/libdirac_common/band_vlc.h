/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: band_vlc.h,v 1.8 2009/02/09 09:44:56 asuraparaju Exp $ $Name:  $
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
#include <libdirac_common/band_codec.h>


namespace dirac
{

   class SubbandByteIO;
   class ByteIO;

    /*! Abstract VLC entropy codec base class */
    class ArithCodecToVLCAdapter
    {
    public:
        /*! Constructor */
        ArithCodecToVLCAdapter(SubbandByteIO* subband_byteio, size_t number_of_contexts);

        /*! Virtual Destructor */
        virtual ~ArithCodecToVLCAdapter(){}

        /* Compresses the input and returns the number of bits written */
        int Compress (CoeffArray &in_data);

        /* Decompresses the bitstream */
        void Decompress (CoeffArray& out_data, int num_bytes);

        /* Encodes a symbol and writes to the output */
        void EncodeSymbol(bool val, int /*context_num*/)
        {
            m_byteio->WriteBit(val);
        }

        /* Decodes a symbol */
        bool DecodeSymbol(int /*context_num*/)
        {
            return m_byteio->ReadBoolB();
        }

        /*! Purely virtual function that does the actual encoding. Derived classes must define it */
        virtual void DoWorkCode(CoeffArray &in_data) = 0;

        /*! Purely virtual function that does the actual decoding. Derived classes must define it */
        virtual void DoWorkDecode(CoeffArray &out_data) = 0;

    protected:
        /*! Input/output stream for Dirac-format bytes */
        ByteIO *m_byteio;

    private:
        //! Private, bodyless copy constructor: class should not be copied
        ArithCodecToVLCAdapter(const ArithCodecToVLCAdapter& cpy);
        //! Private, bodyless copy operator=: class should not be assigned
        ArithCodecToVLCAdapter& operator=(const ArithCodecToVLCAdapter& rhs);
    };


    //! A general class for coding and decoding wavelet subband data using variable length coding.
    /*!
        A general class for coding and decoding wavelet subband data using variable length coding,
     */
    typedef GenericBandCodec<ArithCodecToVLCAdapter> BandVLC;

    //////////////////////////////////////////////////////////////////////////////////
    //Finally,special class incorporating prediction for the DC band of intra frames//
    //////////////////////////////////////////////////////////////////////////////////

    //! A class specially for coding the DC subband of Intra frames
    /*!
        A class specially for coding the DC subband of Intra frames, using intra-band prediction
        of coefficients.
    */
    class IntraDCBandVLC: public GenericIntraDCBandCodec<ArithCodecToVLCAdapter>
    {
    public:
        //! Constructor
        /*!
            Creates a IntraDCBandVLC object to encode subband data, based on parameters
            \param    subband_byteio input/output for the encoded bits
            \param    band_list    the set of all the subbands
         */
        IntraDCBandVLC(SubbandByteIO* subband_byteio,
                         const SubbandList& band_list);
    private:
        //! Encode a single coefficient using error-feedback DC quantization
        void CodeCoeff(CoeffArray& in_data, const int xpos, const int ypos);

    private:
        //! Private, bodyless copy constructor: class should not be copied
        IntraDCBandVLC (const IntraDCBandVLC& cpy);

        //! Private, bodyless copy operator=: class should not be assigned
        IntraDCBandVLC& operator=(const IntraDCBandVLC& rhs);
    };


}// end namespace dirac
#endif
