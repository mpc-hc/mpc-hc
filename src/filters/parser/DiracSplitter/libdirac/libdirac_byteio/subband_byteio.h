/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: subband_byteio.h,v 1.1 2006/04/20 10:41:56 asuraparaju Exp $ $Name:  $
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
* Definition of class SubbandByteIO
*/
#ifndef subband_byteio_h
#define subband_byteio_h


//LOCAL INCLUDES
#include <libdirac_byteio/byteio.h>         // Parent class

// DIRAC includes
#include <libdirac_common/wavelet_utils.h>   // Subband class

namespace dirac
{
/**
* Subband Dirac-bytestream input/output
*/
class SubbandByteIO : public ByteIO
{
public:

    /**
    * Constructor
    *@param sub_band Corresponding Subband
    *@param byteio Source/Destination of data
    */
    SubbandByteIO(Subband& sub_band,
                  const ByteIO& byteio);


    /* Constructor
    *@param sub_band Corresponding Subband
    *@param byteio Source/Destination of data
    */
    SubbandByteIO(Subband& sub_band);

    /**
    * Destructor
    */
    ~SubbandByteIO();

    /**
    * Inputs data from Dirac stream-format
    */
    bool Input();


    /**
    * Gets number of bytes in Arith-coded data block
    */
    int GetBandDataLength() const;

    /**
    * Gets subband bytes in Dirac-bytestream format
    */
    const std::string GetBytes();


protected:


private:

    /**
    * Sub-band that is inputed/outputed
    */
    Subband&        m_subband;

    /**
    * Number of bytes in arith-coded data block
    */
    int             m_band_data_length;
};

} // namespace dirac

#endif
