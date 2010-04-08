/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: endofsequence_byteio.h,v 1.3 2008/01/31 11:25:15 tjdwave Exp $ $Name:  $
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
* Definition of class EndOfSequenceByteIO
*/
#ifndef endofsequence_byteio_h
#define endofsequence_byteio_h


//LOCAL INCLUDES
#include <libdirac_byteio/parseunit_byteio.h>      // Parent class

// DIRAC includes
#include <libdirac_common/common.h>                // PictureType etc

namespace dirac
{
/**
* Signals the end of a sequence in a Dirac-formatted bitstream. Current Accessunit parameters
* are no longer valid for subsequent frames
*/
class EndOfSequenceByteIO : public ParseUnitByteIO
{
public:

    /**
    * Constructor
    *@param stream_data Stream parameters
    */
    EndOfSequenceByteIO(const ByteIO& stream_data);

    /**
    * Destructor
    */
    ~EndOfSequenceByteIO();


    /**
    * Gets parse-unit type
    */
    ParseUnitType GetType() const
    {
        return PU_END_OF_SEQUENCE;
    }

    /**
    * Gathers byte stats on the end of sequence data
    *@param dirac_byte_stats Stat container
    */
    void CollateByteStats(DiracByteStats& dirac_byte_stats);
protected:

    /**
    * Calculates number of bytes to start of next unit
    *@return Zero(0) End of sequence does not specify a 'next'unit
    */
    int CalcNextUnitOffset()
    {
        return 0;
    }

private:

    /**
    * Calculates parse-code based on picture parameters
    *@return Char bit-set
    */
    unsigned char CalcParseCode() const;


};

} // namespace dirac

#endif
