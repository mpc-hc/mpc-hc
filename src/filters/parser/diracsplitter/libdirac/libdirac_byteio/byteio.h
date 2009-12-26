/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: byteio.h,v 1.11 2009/01/21 05:18:09 asuraparaju Exp $ $Name:  $
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
* Contributor(s): Andrew Kennedy (Original Author),
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

/**
* Definition of class ByteIO.
*/
#ifndef byteio_h
#define byteio_h

// SYSTEM INCLUDES
#include <iostream>             // IO classes
#include <sstream>              // IO classes
#include <iomanip>              // setw
#include <climits>              // CHAR_BIT

//LOCAL INCLUDEs
#include <libdirac_byteio/dirac_byte_stats.h>   // stores stats

namespace dirac
{
    
    // BIT DEFS
    #define BIT_ZERO 0
    #define BIT_ONE 1

    // most significant bit in a character 
    #define MS_BIT                (1 << (CHAR_BIT - 1))

    /* array index for character containing bit */
    //#define BIT_IN_CHAR(bit)      (1 << (CHAR_BIT-1-bit))
    #define BIT_IN_CHAR(bit)      (1 << bit)
 

   /** 
   * Class ByteIO - top-level class for reading/writing bytes to a stream
   */
   class ByteIO
   {
   public:
       
       /**
       * Default constructor
       *@param new_stream <B>Has Creates & owns data buffer </B>
       */
       ByteIO(bool new_stream=true);

       /**
       * Constructor
       *@param stream_data Copies data buffer details
       */
       ByteIO(const ByteIO& stream_data);

       /**
       * Destructor
       */
       virtual ~ByteIO();

        /**
        * Gathers byte-stream statistics
        *@param dirac_byte_stats Collates byte information
        */
       virtual void CollateByteStats(DiracByteStats& dirac_byte_stats) 
       { dirac_byte_stats.Clear(); }

        /**
        * Get bytes in Dirac-bytestream format
        */
        virtual const std::string GetBytes();

        /**
        * Get position of read stream pointer
        */
        int GetReadBytePosition() const { return mp_stream->tellg();};


        /**
        *Gets size (in bytes)
        */
        virtual int GetSize() const;

        /**
        * Copies stream source/destination info
        *@param byte_io Byte source/destination
        */
        void SetByteParams(const ByteIO& byte_io);

        /**
        * Sync input for byte-alignment
        */
        void ByteAlignOutput();

         /**
        * Ouputs an unsigned integer in interleaved exp Golomb format
        *@param value Integer to be output
        */
        //void OutputVarLengthUint(const unsigned int& value);
        void WriteUint(unsigned int value);

         /**
         * Sets input size in bits. Read is limited by this
         */
         void SetBitsLeft(int left_bits) { m_bits_left = left_bits; }

         /**
         * Sets input size in bits. Read is limited by this
         */
         int BitsLeft(void) { return m_bits_left; }

    protected:

        inline bool CanRead() const { return(!mp_stream->eof()); }

        inline bool GetBit(unsigned char& c, int pos) const { return (c & BIT_IN_CHAR(pos)); }

        inline void SetBit(unsigned char& c, int pos) const { c |= BIT_IN_CHAR(pos); }

        inline void SetBits(unsigned char& c, unsigned char bits) const { c |= bits; }

        /**
        * Sync input for byte-alignment
        */
        void ByteAlignInput();


        /**
        * Reads boolean value
        */
        bool ReadBool();

        /**
        * Reads boolean value - bounded i/o
        */
        bool ReadBoolB();

        /**
        * Reads next bit
        */
        int ReadBit();

        /**
        * Reads next bit - bounded i/o
        */
        int ReadBitB();

        /**
        * Reads next 'count' bits
        *@param count number of bits to be read
        *@return unsigned interger read
        */
        unsigned int ReadNBits(int count);

        /**
        * Reads from stream
        *@param data Start of char buffer
        *@param count Number of bytes to read
        */
        void InputBytes(char* data, int count)
        {
            //int j=mp_stream->tellg();
            mp_stream->read(data, count);

            //int h=mp_stream->tellg();
        }

        /**
        * Flushes the bounde input
        */
        void FlushInputB();

        /**
        * Reads a signed integer in interleaved exp-Golomb format
        *return Signed integer read
        */
        //int InputVarLengthInt();
        int ReadSint();

        /**
        * Reads a signed integer in interleaved exp-Golomb format from bounded input
        *return Signed integer read
        */
        int ReadSintB();

        /**
        * Reads an unsigned integer in interleaved exp Golomb format
        *@return Unsigned Integer read
        */
        //unsigned int InputVarLengthUint();
        unsigned int ReadUint();

        /**
        * Reads an unsigned integer in interleaved exp Golomb format from bounded input
        *@return Unsigned Integer read
        */
        //unsigned int InputVarLengthUint();
        unsigned int ReadUintB();

        /**
        * Reads a fixed length unsigned integer from the stream in big endian
        *@param byte_size Number of bytes in fixed length integer 
        *@return Unsigned Integer read 
        */
        //inline unsigned int InputFixedLengthUint(const int byte_size) { 
        inline unsigned int ReadUintLit(const int byte_size) { 
           unsigned int val=0; 
           for(int i=0; i < byte_size; ++i)
           {
               val <<= 8;
               val += (unsigned char)mp_stream->get();
           }
           m_num_bytes+=byte_size;
           return val;
        } 

        /**
        * Reads a byte from the stream
        */
        inline unsigned char InputUnByte() {m_num_bytes++ ; return mp_stream->get(); }

        /**
        * Reads a series of bytes from a stream
        */
        inline std::string InputUnString(const int count) 
        {
            std::string str;
            for(int index=0; index < count; ++index)
                str.push_back(InputUnByte());
            return str;
        }

        /**
        * Outputs a bit
        *@param bit 1/0 Output
        */
        void WriteBit(const bool& bit);

        /**
        * Outputs an unsigned integer
        *@param val Integer to be output
        *@return number of bits written
        */
        int WriteNBits(unsigned int val);

        /**
        * Outputs an n bit integer
        *@param val   Unsigned Integer to be output
        *@param count number of bits to be written
        */
        void WriteNBits(unsigned int val, int count);



        /**
        * Outputs a series of bytes
        */
        void OutputBytes(const std::string& bytes) {
           int cur_pos = mp_stream->tellg();
          mp_stream->str(mp_stream->str()+bytes);
           m_num_bytes+=bytes.size();
        //   *mp_stream << bytes;
           mp_stream->seekg(std::max(cur_pos,0), std::ios_base::beg);
        }

         /**
        * Outputs current byte contents
        */
        inline void OutputCurrentByte()
        {
            if (m_current_pos)
            {
                *mp_stream << (m_current_byte);
                ++m_num_bytes;
                m_current_pos = 0;
                m_current_byte = 0;
            }
        };

        /**
        * Outputs an integer in Golomb signed integer format
        *@param val Integer to be output
        */ 
       //void OutputVarLengthInt(const int val);
       void WriteSint(int val);

       /**
       * Output unsigned int value in big endian format
       * @param value Integer to be output
       * @param length number of bytes in val to output
       */
       //inline void OutputFixedLengthUint(const unsigned int& value, const int& length)
       inline void WriteUintLit(const unsigned int& value, const int& length)
       {
           for(int i=length-1; i >=0 ; --i)
           {
              unsigned char cp = (value>>(i*8))&0xff; 
               *mp_stream << cp;
           }
           m_num_bytes+=length;
       }
       
       /**
       * Removes portion of byte-stream no longer required
       *@param count Number of bytes to be removed from beginning of stream
       */
       void RemoveRedundantBytes(const int count);

       inline void SeekGet(const int offset, std::ios_base::seekdir dir) 
       {
           mp_stream->seekg(offset, dir);
       }

        /**
       * Input/output steam
       */
       std::stringstream*    mp_stream;


   private:
       
       /**
       * ArithCodec can see internals for getting/setting bits
       */
       friend class ArithCodecBase;

       /**
       * VLC entropy coder can see internals for getting/setting bits
       */
       friend class ArithCodecToVLCAdapter;

       /**
       * Char used for temporary storage of op data bits
       */
       unsigned char m_current_byte;
            
       /**
       * Used to set individual bit within the current header byte
       */
       int m_current_pos;

       /**
       * Number of bytes processed
       */
       int m_num_bytes;
       
       /**
       * stream alloc flag
       */
       bool m_new_stream;
        
       /**
       * num bits left to read
       */
       int m_bits_left;
   protected:

        
   };



} // namespace dirac

#endif
