/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: dirac_byte_stream.h,v 1.4 2008/08/14 00:51:08 asuraparaju Exp $ $Name:  $
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
* Contributor(s): Andrew Kennedy (Original Author)
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
* Definition of class DiracByteStream
*/
#ifndef dirac_byte_stream_h
#define dirac_byte_stream_h

//SYSTEM INCLUDES
#include <queue>

//LOCAL INCLUDES
#include "byteio.h"             // Parent class
#include "accessunit_byteio.h"  // manages parse-unit types
#include "picture_byteio.h"       // manages parse-unit types

namespace dirac
{

    /**
    * Represents a series of bytes in the Dirac bytestream specfication format.
    * These bytes are grouped into more managable parse units by this class.
    */
    class DiracByteStream : public ByteIO
    {
        public:

        /**
        * Constructor
        */
        DiracByteStream();

        /**
        * Destructor
        */
        ~DiracByteStream();

        /**
        * Adds Dirac-formatted bytes to internal-byte-stream for processing
        *@param start Start of char list
        *@param count Number of chars
        */
        void AddBytes(char* start, int count);

        /**
        * Gets the statistics of the most recent parse-unit to be processed
        *@return Byte-statistics
        */
        DiracByteStats GetLastUnitStats();

        /**
        * Gets the next parse-unit in the current byte-stream
        */
        ParseUnitByteIO* GetNextParseUnit();


        /**
        * Gets stats for current sequence
        */
        DiracByteStats GetSequenceStats() const;

        /**
        * Adds a random access point to the current Dirac byte stream
        *@param p_seqheader_byteio Sequence header data. 
        */
        void AddSequenceHeader(SequenceHeaderByteIO *p_seqheader_byteio);

        /**
        * Adds a picture to the current Dirac byte stream
        *@param p_frame_byteio Picture stream. This class is now responsible for deleting.
        */
        void AddPicture(PictureByteIO *p_frame_byteio);

        /**
        * Clear parse-units
        */
        void Clear();

        /**
        * Insert end-of-sequence data
        *@return Sequence stats
        */
        DiracByteStats EndSequence();

        /**
        * Gets a pointer to all current output bytes
        */
        const std::string GetBytes();

        /**
        * Any info pending?
        */
        bool IsUnitAvailable() const;

        private:

        void Reset(ParseUnitByteIO* p_curr_unit, int pos);

        private:

        /**
        * Parse-units in Dirac stream
        */
        typedef std::queue< std::pair <ParseUnitType, ParseUnitByteIO*> > ParseUnitList;
        ParseUnitList       m_parse_unit_list;

        /**
        * Last unit to be processed
        * Required for specifying the previous parse-unit
        */
        ParseUnitByteIO* mp_prev_parse_unit;

        /**
        * Stats for current sequence
        */
        DiracByteStats      m_sequence_stats;

    };

} // namespace dirac

#endif
