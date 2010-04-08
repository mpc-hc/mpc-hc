/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: parseunit_byteio.h,v 1.11 2008/05/02 05:57:19 asuraparaju Exp $ $Name:  $
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
* Definition of class ParseUnitByteIO
*/
#ifndef parseunit_byteio_h
#define parseunit_byteio_h

//SYSTEM INLUDES
#include <map>                  // Byte-map
#include <string>               // stores values

//LOCAL INCLUDES
#include "byteio.h"             // Parent class


namespace dirac
{

/* Types of parse-unit */
typedef enum
{
    PU_SEQ_HEADER = 0,
    PU_PICTURE,
    PU_END_OF_SEQUENCE,
    PU_AUXILIARY_DATA,
    PU_PADDING_DATA,
    PU_CORE_PICTURE,
    PU_LOW_DELAY_PICTURE,
    PU_UNDEFINED
} ParseUnitType;

/**
* Represents a collection of data in a Dirac bytestream that can be parsed as a
* self-contained unit
*/
class ParseUnitByteIO : public ByteIO
{
public:

    /**
    * Constructor
    */
    ParseUnitByteIO();

    /**
            * Constructor
            *@param byte_io Stream parameters
            */
    ParseUnitByteIO(const ByteIO& byte_io);

    /**
    * Constructor
    *@param parseunit_byteio Parse-unit parameters
    */
    ParseUnitByteIO(const ParseUnitByteIO& parseunit_byteio);

    /**
    * Destructor
    */
    ~ParseUnitByteIO();

    /**
            * Gathers byte stats on the parse-unit data
            *@param dirac_byte_stats Stat container
            */
    virtual void CollateByteStats(DiracByteStats& dirac_byte_stats);

    /**
    * Reads from byte-stream to find parse data
    *@return <B>false</B> if not enough data in stream
    */
    bool Input(); // decoding

    /**
    * Accesses validity of a unit by comparing it with an adjacent unit
    */
    bool IsValid();

    /**
    * Can Skip past the entire parse-unit
    *@return <B>false</B> Nothing to skip to
    */
    bool CanSkip();

    /**
    * Gets string containing coded bytes
    */
    virtual const std::string GetBytes();   // encoding

    /**
    * Set next/previous parse-unit values
    *@param p_prev_parseunit Previous parse-unit
    */
    void SetAdjacentParseUnits(ParseUnitByteIO *p_prev_parseunit);  // encoding

    /*
    * Gets number of bytes input/output within unit
    */
    virtual int GetSize() const;

    /**
    * Gets expected number of bytes to start of next parse-unit
    */
    int GetNextParseOffset() const;

    /**
    * Gets number of bytes to start of previous parse-unit
    */
    int GetPreviousParseOffset() const;

    /**
    * Gets parse-unit type
    */
    virtual ParseUnitType GetType() const;

    /**
    * Returns true is parse unit is a Sequence Header
    */
    bool IsSeqHeader() const
    {
        return m_parse_code == 0x00;
    }

    /**
    * Returns true is parse unit is an End of Sequence unit
    */
    bool IsEndOfSequence() const
    {
        return m_parse_code == 0x10;
    }

    /**
    * Returns true is parse unit is Auxiliary Data
    */
    bool IsAuxiliaryData() const
    {
        return (m_parse_code & 0xF8) == 0x20;
    }

    /**
    * Returns true is parse unit is Padding data
    */
    bool IsPaddingData() const
    {
        return m_parse_code == 0x30;
    }

    /**
    * Returns true is parse unit is Picture data
    */
    bool IsPicture() const
    {
        return ((m_parse_code & 0x08) == 0x08);
    }

    /**
    * Returns true is parse unit is Low Delay Sybtax unit
    */
    bool IsLowDelay() const
    {
        return ((m_parse_code & 0x88) == 0x88);
    }

    /**
    * Returns true is parse unit is Core syntax unit
    */
    bool IsCoreSyntax() const
    {
        return ((m_parse_code & 0x88) == 0x08);
    }

    /**
    * Returns true is parse unit uses Arithmetic coding
    */
    bool IsUsingAC() const
    {
        return ((m_parse_code & 0x48) == 0x08);
    }

protected:

    /**
    * Calculates number of bytes to start of next unit
    *@return Number of bytes to next unit
    */
    virtual int CalcNextUnitOffset();

    /**
    * Pure virtual method for calculating parse-code
    *@return Char containing bit-set for parse-unit parameters
    */
    virtual unsigned char CalcParseCode() const
    {
        return 0;   // encoding
    }

    /**
            * Locates start of parse-unit
            *@return <B>false</B> if not enough data
            */
    bool SyncToUnitStart();   // decoding

    /**
    * Get parse code
    */
    unsigned char GetParseCode() const
    {
        return m_parse_code;
    }

private:

    /**
    * Number of bytes to next parse-unit
    */
    int m_previous_parse_offset;

    /**
    * Number of bytes to previous parse-unit
    */
    int m_next_parse_offset;

    /**
    * Parse-type-identifier
    */
    unsigned char m_parse_code;

};


} // namespace dirac

#endif
