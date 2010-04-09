/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: bit_manager.h,v 1.16 2008/01/31 11:25:16 tjdwave Exp $ $Name:  $
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
*                 Robert Scott Ladd,
*                 Tim Borer
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

#ifndef _BIT_MANAGER_H_
#define _BIT_MANAGER_H_

#include <libdirac_common/arrays.h>
#include <cstring>
#include <vector>
#include <iostream>

namespace dirac
{
    //!  Prefix for all start codes
    const unsigned int START_CODE_PREFIX = 0x42424344; //BBCD
    const unsigned int START_CODE_PREFIX_BYTE0 = 
                                        (START_CODE_PREFIX >> 24) & 0xFF;
    const unsigned int START_CODE_PREFIX_BYTE1 = 
                                        (START_CODE_PREFIX >> 16) & 0xFF;
    const unsigned int START_CODE_PREFIX_BYTE2 = 
                                        (START_CODE_PREFIX >>  8) & 0xFF;
    const unsigned int START_CODE_PREFIX_BYTE3 = 
                                        START_CODE_PREFIX & 0xFF;

    //! Random Access Point (RAP) Intra Picture start Code
    const unsigned char RAP_START_CODE = 0xD7;
    //! Non-RAP Intra Picture start code
    const unsigned char IFRAME_START_CODE = 0xD6;
    //! L1 Picture start code
    const unsigned char L1FRAME_START_CODE = 0xD4;
    //! L2 Picture start code
    const unsigned char L2FRAME_START_CODE = 0xD5;
    //! Sequence end code
    const unsigned char SEQ_END_CODE = 0xD0;
    //! Not a start code but part of data
    const unsigned char NOT_START_CODE = 0xFF;
    //! Bitstream version
    const unsigned char BITSTREAM_VERSION = 0x05;  //0.5


    ////////////////////////////////////////////////
    //--------------Bit output stuff--------------//
    ////////////////////////////////////////////////

    class UnitOutputManager;
    class FrameOutputManager;
    class SequenceOutputManager;

    //! Class for managing bit- and byte-oriented output.
    /*!
        A class for managing bit- and byte-oriented output. Wraps around 
        an ostream object but stores data in memory until told told to 
        write out in order to support data re-ordering - for example
        writing a header once the subsequent data has been obtained. 
        Implementation to be reviewed in future. TJD 13 April 2004.
    */
    class BasicOutputManager
    {
        // Data cannot be written to file directly, only by other o/p classes
        friend class UnitOutputManager;
        friend class FrameOutputManager;
        friend class SequenceOutputManager;

        public:
            //! Constructor
            /*!
            Constructor requires an ostream object pointer.
            \param out_data the output stream object pointer
            */
            BasicOutputManager(std::ostream* out_data );

            //Copy constructor is default shallow copy

            //Operator= is default shallow=

            //! Destructor
            ~BasicOutputManager(){}

            //! Write a bit out. 
            /*!
            Write a bit out to the internal data cache.
            */ 
            void OutputBit(const bool& bit);

            //! Write a bit out and increment count 
            /*!
            Write a bit out to the internal data cache and increment the 
            count of bits written.
            */
            void OutputBit(const bool& bit,int& count);

            //! Write a byte out.
            /*!
            Write a byte out to the internal data cache.
            */
            void OutputByte(const char& byte);

            //! Write a null-terminated set of bytes out.
            /*!
            Write a null-terminated set of bytes out to the internal data cache.
            */   
            void OutputBytes(char* str_array);

            //! Write a number of bytes out.
            /*!
            Write a number of bytes out to the internal data cache.
            */   
            void OutputBytes(char* str_array,int num);

            //! Return the number of bytes last output to file.
            /*!
            Return the number of bytes last output to file.
            */   
            size_t GetNumBytes() const {return m_num_out_bytes;}

            //! Current size of the internal data cache in bytes.
            /*!
                Current size of the internal data cache in bytes.
            */   
            size_t Size() const;

        private:
            // Number of output bytes written
            size_t m_num_out_bytes;
            std::ostream* m_op_ptr;
            // Buffer used to store output prior to saving to file
            std::vector<char> m_buffer;
            // Char used for temporary storage of op data bits
            char m_current_byte;
            // Used to set individual bit within the current header byte
            int m_output_mask;

            //functions  

            //! Write all data to file.
            /*!
            Dump the internal data cache to the internal ostream object.
            */   
            void WriteToFile();

            //Initialise the output stream.
            void InitOutputStream();

            //Clean out any remaining output bits to the buffer
            void FlushOutput();
            
            //! Write an ignore code
            /*!
            Write a skip interpret start prefix  byte out to the internal data 
            cache.
            */
            void OutputSkipInterpretStartPrefixByte();
    };

    //! A class for handling data output, including headers.
    /*!
    A class for handling data output, including headers and reordering.
    */
    class UnitOutputManager
    {
        // Only the FrameOutputManager can make this class write data to file
        friend class FrameOutputManager;

        public:
            //! Constructor.
            /*!
            Constructor wraps around a pointer to an ostream object, and 
            initialises two BasicOutputManager objects for header and data
            */
            UnitOutputManager(std::ostream* out_data ); 

            //Copy constructor is default shallow copy

            //Operator= is default shallow=

            //! Destructor
            ~UnitOutputManager(){}

            //! Handles the header bits.
            /*!
            A BasicOutputManager object for handling the header bits.
            */
            BasicOutputManager& Header(){return m_header;}

            //! Handles the data bits.
            /*!
            A BasicOutputManager object for handling the data bits.
            */
            BasicOutputManager& Data(){return m_data;}

            //! Returns the total number of bytes written in the last unit coded.
            /*!
            Returns the total number of bytes written in the last unit coded - header + data.
            */
            const size_t GetUnitBytes() const {return m_unit_bytes;}

            //! Returns the total number of header bytes written in the last unit coded. 
            const size_t GetUnitHeaderBytes() const {return m_unit_head_bytes;}

            //! Current size of the internal data cache in bytes.
            /*!
                Current size of the internal data cache in bytes.
            */   
            size_t Size() const;

        private:
            // basic output managers for the header and data
            BasicOutputManager m_header,m_data;
     
            // total number of bytes written in the last unit coded 
            size_t m_unit_bytes;

            // number of data bytes for the last unit coded
            size_t m_unit_data_bytes;

            // number of data bytes for the last unit coded
            size_t m_unit_head_bytes;

            // functions

            //! Writes the bit caches to file.
            /*!
            Writes the header bits to the ostream, followed by the data bits.
            */
            void WriteToFile();
    };

    class FrameOutputManager
    {
    public:

        // Only the SequenceOutputManager can make this class write data to file
        friend class SequenceOutputManager;

        //! Constructor
        /*
            Constructs a class which manages output for an entire picture.
            \param  out_data  pointer to the output stream 
            \param  num_bands  the number of subbands per component
        */
        FrameOutputManager( std::ostream* out_data , const int num_bands=13 ); 

        //! Destructor
        ~FrameOutputManager();

        //! Set the number of bands there will be in a component
        void SetNumBands( const int num_bands );

        //! Get an output manager for a subband
        /*!
            Get an output manager for a subband.
            \param  csort  the component (Y, U or V)
            \param  band_num  the number of the subband
        */
        UnitOutputManager& BandOutput( const int csort , const int band_num );

        //! Get an output manager for a subband
        /*!
            Get an output manager for a subband.
            \param  csort  the component (Y, U or V)
            \param  band_num  the number of the subband
        */
        const UnitOutputManager& BandOutput( const int csort , const int band_num ) const;

        //! Get an output manager for MV data
        /*!
            Get an output manager for MV data
        */
        UnitOutputManager& MVOutput(){ return *m_mv_data; }

        //! Get an output manager for MV data
        /*!
            Get an output manager for MV data
        */
        const UnitOutputManager& MVOutput() const { return *m_mv_data; }

        //! Get an output manager for the picture header
        BasicOutputManager& HeaderOutput(){ return *m_frame_header; }

        //! Return the number of bytes used for each component
        const size_t ComponentBytes( const int comp_num ) const { return m_comp_bytes[comp_num];}

        //! Return the number of header bytes used for each component
        const size_t ComponentHeadBytes( const int comp_num ) const { return m_comp_hdr_bytes[comp_num];}

        //! Return the number of motion vector bytes used
        const size_t MVBytes() const { return m_mv_bytes;}

        //! Return the number of motion vector header bytes used
        const size_t MVHeadBytes() const { return m_mv_hdr_bytes;}

        //! Return the number of bytes used for the whole picture
        const size_t FrameBytes() const { return m_total_bytes;}

        //! Return the number of header bytes used throughout the picture
        const size_t FrameHeadBytes() const { return m_header_bytes;}

        //! Current size of the internal data cache in bytes.
        /*!
            Current size of the internal data cache in bytes.
        */   
            size_t Size() const;

    private:

        // Array of subband outputs, 1 for each component and subband
        TwoDArray< UnitOutputManager* > m_data_array;

        // Motion vector output
        UnitOutputManager* m_mv_data;

        // Picture header output
        BasicOutputManager* m_frame_header;

        // The total number of picture bytes
        size_t m_total_bytes;

        // The total number of header bytes
        size_t m_header_bytes;

        // The total number of MV header bytes
        size_t m_mv_hdr_bytes; 

        // The total number of MV bytes
        size_t m_mv_bytes; 

        // The total number of bytes in each component
        OneDArray< size_t > m_comp_bytes;

        // The total number of header bytes in each component
        OneDArray< size_t > m_comp_hdr_bytes;

        // A copy of a pointer to the output stream
        std::ostream* m_out_stream;

        // Functions

        //! Initialise the band data
        void Init( const int num_bands );

        //! Reset all the data
        void Reset();

        //! Delete all the data
        void DeleteAll();

        //! Write all the picture data to file
        void WriteToFile();
    };

    class SequenceOutputManager
    {
    public:
        //! Constructor
        SequenceOutputManager( std::ostream* out_data );

        //! Return a reference to the output for a single picture
        FrameOutputManager& FrameOutput(){ return m_frame_op_mgr; }

        //! Return a reference to the output for the sequence header
        BasicOutputManager& HeaderOutput(){ return m_seq_header; }

        //! Return a reference to the output for the sequence trailer
        BasicOutputManager& TrailerOutput(){ return m_seq_end; }

        //! Reset the picture data without outputting
        void ResetFrame(){ m_frame_op_mgr.Reset(); }

        //! Write the sequence header
        void WriteSeqHeaderToFile();

        //! Write all the picture data to file
        void WriteFrameData();

        //! Write the sequence trailer
        void WriteSeqTrailerToFile();

        //! Return the total number of bytes used for the sequence
        const size_t SequenceBytes() { return m_total_bytes; } 

        //! Return the total number of header bytes used throughout the sequence
        const size_t SequenceHeadBytes() { return m_header_bytes; } 

        //! Return the total number bytes used for MVs
        const size_t MVBytes() { return m_mv_bytes; }

        //! Return the total number bytes used for a component
        const size_t ComponentBytes( const int comp_num ) { return m_comp_bytes[comp_num]; }

        //! Reset the picture data
        void ResetFrameData();


    private:

        // The picture output manager
        FrameOutputManager m_frame_op_mgr;

        // Output manager for the sequence header
        BasicOutputManager m_seq_header;

        // Output manager for the sequence end
        BasicOutputManager m_seq_end;

        // The total number of bytes in each component
        OneDArray< size_t > m_comp_bytes;

        // The total number of header bits in each component
        OneDArray< size_t > m_comp_hdr_bytes;

        // The number of MV header bytes
        size_t m_mv_hdr_bytes;

        // The total number of MV bytes
        size_t m_mv_bytes;

        // The total number of bytes written so far
        size_t m_total_bytes;

        // The total number of header bytes written so far
        size_t m_header_bytes;

        // The total number of trailer bytes written so far
        size_t m_trailer_bytes;
    };

    ///////////////////////////////////////////////
    //--------------Bit input stuff--------------//
    ///////////////////////////////////////////////

    //! A class for managing bit-wise and byte-wise input. 
    class BitInputManager
    {

        public:
            //! Constructor. 
            /*!
            Constructor. Wraps around an istream object.
            */
            BitInputManager(std::istream* in_data );

            //Copy constructor is default shallow copy

            //Operator= is default shallow=

            //! Destructor
            ~BitInputManager(){}

            //input functions
            //! Obtain the next bit.
            bool InputBit();

            //! Obtain the next bit, incrementing count. 
            bool InputBit(int& count);

            //! Obtain the next bit, incrementing count, if count<max_count; else return 0 (false).
            bool InputBit(int& count, const int max_count);

            //! Obtain the next byte. 
            char InputByte();

            //! Obtain a number of bytes. 
            void InputBytes(char* cptr,int num);

            //! Move onto the next byte. Needed if a data unit is not an exact number of bytes.
            void FlushInput();

            //! Returns true if we're at the end of the input, false otherwise
            bool End() const ;

        private:

            std::istream* m_ip_ptr;
            // Char used for temporary storage of ip bits
            char m_current_byte;     
            // The number of bits left withint the current input byte being decoded
            int m_input_bits_left;

            //used to check if start code is detected
            unsigned int m_shift;
            //functions 
            // Initialise the input stream
            void InitInputStream(); 
    };

} // namespace dirac
#endif
