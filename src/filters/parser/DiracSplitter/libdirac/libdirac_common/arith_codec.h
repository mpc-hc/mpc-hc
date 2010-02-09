/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: arith_codec.h,v 1.40 2007/11/16 04:48:44 asuraparaju Exp $ $Name:  $
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
* Contributor(s):   Richard Felton (Original Author),
                    Thomas Davies,
                    Scott R Ladd,
                    Peter Bleackley,
                    Steve Bearcroft,
                    Anuradha Suraparaju,
                    Tim Borer (major refactor February 2006)
                    Andrew Kennedy
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


#ifndef _ARITH_CODEC_H_
#define _ARITH_CODEC_H_

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
////                                                                 ////
////-----------Abstract binary arithmetic coding class---------------////
////subclass this for coding motion vectors, subband residues etc ...////
////                                                                 ////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

#include <libdirac_common/common.h>
#include <libdirac_byteio/byteio.h>
#include <vector>

namespace dirac
{

    class Context {
    public:

        //! Default Constructor.
        /*!
        Default constructor initialises counts to 1 each of 0 and 1.
        */ 
        inline Context();

        //Class is POD
        //Use built in copy constructor, assignment and destructor.

        //! Returns estimate of probability of 0 (false) scaled to 2**16

        inline unsigned int GetScaledProb0( ) const{ return m_prob0;}

        //! Updates context counts
        inline void Update( bool symbol ) {
          if (symbol) m_prob0 -= lut[m_prob0>>8];
          else m_prob0 += lut[255-(m_prob0>>8)];
        }

    private:

        int m_prob0;
        static const unsigned int lut[256]; //Probability update table
    };
    
    Context::Context(): m_prob0( 0x8000 ) {}

    class ArithCodecBase {

    public:

        //! Constructor
        /*!
            Creates an ArithCodec object to decode input based on a set of
            parameters.
            \param     p_byteio   input/output for encoded bits
            \param    number_of_contexts    the number of contexts used
        */   
        ArithCodecBase(ByteIO* p_byteio, size_t number_of_contexts);

        //! Destructor
        /*!
            Destructor is virtual as this class is abstract.
         */
        virtual ~ArithCodecBase();

    protected:

        //core encode functions
        ////////////////////////////

        //! Initialises the Encoder
        void InitEncoder();

        //! encodes a symbol and writes to output
        void EncodeSymbol(const bool symbol, const int context_num);

        void EncodeUInt(const unsigned int value, const int bin1, const int max_bin);

        void EncodeSInt(const int value, const int bin1, const int max_bin);

        //! flushes the output of the encoder.
        void FlushEncoder();

        int ByteCount() const;     

        // core decode functions
        ////////////////////////////

        //! Initialise the Decoder
        void InitDecoder(int num_bytes);                    

        //! Decodes a symbol given a context number
        bool DecodeSymbol( int context_num );

        unsigned int DecodeUInt(const int bin1, const int max_bin);

        int DecodeSInt(const int bin1, const int max_bin);
        
        //! List of contexts
        std::vector<Context> m_context_list;

    private:
        
        //! private, bodyless copy constructor: class should not be copied
        ArithCodecBase(const ArithCodecBase & cpy);

        //! private, bodyless copy operator=: class should not be assigned
        ArithCodecBase & operator = (const ArithCodecBase & rhs);
             
                     
        // Decode functions
        ////////////////////////////

        //! Read all the data in
        void ReadAllData(int num_bytes);

        //! Read in a bit of data
        inline bool InputBit();

        // Codec data
        ////////////////////////////
 
 unsigned int m_scount;
 
        //! Start of the current code range
        unsigned int m_low_code;
        
        //! Length of the current code range
        unsigned int m_range;

        //! Input/output stream of Dirac-format bytes
        ByteIO *m_byteio;

        // For encoder only

        //! Number of underflow bits
        int m_underflow;

        //! A pointer to the data for reading in
        char* m_decode_data_ptr;

        //! A point to the byte currently being read
        char* m_data_ptr;

        //! The index of the bit of the byte being read
        int m_input_bits_left;

        //! The present input code
        unsigned int m_code;

    };


    inline bool ArithCodecBase::DecodeSymbol( int context_num )
    {
 
        // Determine the next symbol value by placing code within
        // the [low,high] interval.

        // Fetch the statistical context to be used
        Context& ctx  = m_context_list[context_num];

        // Decode as per updated specification
        const unsigned int count = m_code - m_low_code ; 
        const unsigned int range_x_prob = ( m_range* ctx.GetScaledProb0())>>16;
        const bool symbol = ( count >= range_x_prob );

        // Rescale the interval
        if( symbol )    //symbol is 1
        {
            m_low_code += range_x_prob;
            m_range -= range_x_prob;
        }
        else            //symbol is 0, so m_low_code unchanged
        {
            m_range = range_x_prob;
        }

        // Update the statistical context
        ctx.Update( symbol );

        while ( m_range<=0x4000 )
        {
            if( ( (m_low_code+m_range-1)^m_low_code)>=0x8000 )
            {
                // Straddle condition
                // We must have an underflow situation with
                // low = 0x01... and high = 0x10...
                // Flip 2nd bit prior to rescaling
                m_code      ^= 0x4000;
                m_low_code  ^= 0x4000;
            }

            // Double low and range, throw away top bit of low
            m_low_code  <<= 1;
            m_range <<= 1;
            m_low_code   &= 0xFFFF;

            // Shift in another bit of code
            m_code      <<= 1;
            m_code       += InputBit();
            m_code       &= 0xFFFF;
      
        }

        return symbol;
    }

    inline unsigned int ArithCodecBase::DecodeUInt(const int bin1, const int max_bin) {
        const int info_ctx = (max_bin+1);
        int bin = bin1;
        unsigned int value = 1;
        while (!DecodeSymbol(bin)) {
            value <<= 1;
            if (DecodeSymbol(info_ctx)) value+=1;
            if (bin<max_bin) bin+=1;
        }
        value -= 1;
        return value;
    }

    inline int ArithCodecBase::DecodeSInt(const int bin1, const int max_bin) {
        int value = 0;
        const int magnitude = DecodeUInt(bin1, max_bin);
        if (magnitude!=0) {
            if (DecodeSymbol(max_bin+2)) value=-magnitude;
            else value=magnitude;
        }
        return value;
    }

    inline void ArithCodecBase::EncodeSymbol(const bool symbol, const int context_num)
    {

        // Adjust high and low (rescale interval) based on the symbol we are encoding

        Context& ctx = m_context_list[context_num];

        const unsigned int range_x_prob = ( m_range* ctx.GetScaledProb0())>>16;

        if ( symbol )    //symbol is 1
        {
            m_low_code += range_x_prob;
            m_range -= range_x_prob;  
        }
        else             // symbol is 0, so m_low_code unchanged
        {
            m_range = range_x_prob;
        }
        
        // Update the statistical context
        ctx.Update( symbol );

        while ( m_range <= 0x4000 )
        { 
            if ( ( (m_low_code+m_range-1)^m_low_code)>=0x8000 )
            {    
                // Straddle condition
                // We must have an underflow situation with
                // low = 0x01... and high = 0x10...

                m_low_code  ^= 0x4000;
                m_underflow++;

            }
            else
            {
                // Bits agree - output them
                m_byteio->WriteBit( m_low_code & 0x8000);
                for (; m_underflow > 0; m_underflow-- )
                    m_byteio->WriteBit(~m_low_code & 0x8000);
            }

            // Double low value and range
            m_low_code  <<= 1;
            m_range <<= 1;

            // keep low to 16 bits - throw out top bit
            m_low_code   &= 0xFFFF;

         }
    }

    inline void ArithCodecBase::EncodeUInt(const unsigned int the_int,
                                           const int bin1, const int max_bin) {
        const int value = (the_int+1);
        const int info_ctx = (max_bin+1);
        int bin = bin1;
        int top_bit = 1;
        {
            int max_value = 1;
            while (value>max_value) {
                top_bit <<= 1;
                max_value <<= 1;
                max_value += 1;
            }
        }
        bool stop = (top_bit==1);
        EncodeSymbol(stop, bin);
        while (!stop) {
            top_bit >>= 1;
            EncodeSymbol( (value&top_bit), info_ctx);
            if ( bin < max_bin) bin+=1;
            stop = (top_bit==1);
            EncodeSymbol(stop, bin);
        }
    }

    inline void ArithCodecBase::EncodeSInt(const int value,
                                           const int bin1, const int max_bin) {
        EncodeUInt(std::abs(value), bin1, max_bin);
        if (value != 0) {
            EncodeSymbol( (value < 0), max_bin+2 );
        }
    }


    //! Abstract binary arithmetic coding class
    /*!
        This is an abtract binary arithmetic encoding class, used as the base
        for concrete classes that encode motion vectors and subband residues.
        \param        T        a container (most probably, or array) type
    */

    template<class T> //T is container/array type
    class ArithCodec
        : public ArithCodecBase
    {
    public:

        //! Constructor for encoding
        /*!
            Creates an ArithCodec object to decode input based on a set of
            parameters.
            \param    p_byteio    input/output for encoded bits
            \param    number_of_contexts    the number of contexts used
        */    
        ArithCodec(ByteIO* p_byteio, size_t number_of_contexts);

      
        //! Destructor
        /*!
            Destructor is virtual as this class is abstract.
         */ 
        virtual ~ArithCodec() {}

        //! Compresses the input and returns the number of bits written. 
        /*!
            Compress takes a type T object (a container or array) and 
            compresses it using the abstract function DoWorkCode() which
            is overridden in subclasses. It returns the number of 
            bits written. 
            \param    in_data    the input to be compressed. Non-const, 
            since the compression may be lossy.
         */
        int Compress(T & in_data);
    
        //! Decompresses the bitstream and writes into the output.
        /*!
            Decompresses the  bitstream, up to the number of bytes 
            specified and writes into the output subclasses.
            \param    out_data the output into which the decompressed data
            is written.
            \param    num_bytes    the number of bytes to be read from the 
            bitstream.
         */ 
        void Decompress(T & out_data, const int num_bytes);

    protected:

        //virtual encode-only functions
        /////////////////////////////// 

        //! Does the work of actually coding the data 
        virtual void DoWorkCode(T & in_data) = 0; 

        //! virtual decode-only functions    
        /////////////////////////////// 
        virtual void DoWorkDecode(T & out_data)=0;
   };

    //Implementation - core functions
    /////////////////////////////////

    template<class T>
    ArithCodec<T>::ArithCodec(ByteIO* p_byteio, size_t number_of_contexts):
        ArithCodecBase(p_byteio, number_of_contexts) {}



    template<class T>
    int ArithCodec<T>::Compress(T &in_data)
    {
        InitEncoder();                
        DoWorkCode(in_data);
        FlushEncoder();
        return ByteCount();
    }

    template<class T>
    void ArithCodec<T>::Decompress( T &out_data, const int num_bytes )
    {
        InitDecoder(num_bytes);
        DoWorkDecode( out_data );
    }

   inline bool ArithCodecBase::InputBit()
    {
        if (m_input_bits_left == 0)
        {
            m_data_ptr++;
            m_input_bits_left = 8;
        }
        m_input_bits_left--;
        // MSB to LSB
        return bool( ( (*m_data_ptr) >> m_input_bits_left ) & 1 );
    }

}// namespace dirac
#endif

