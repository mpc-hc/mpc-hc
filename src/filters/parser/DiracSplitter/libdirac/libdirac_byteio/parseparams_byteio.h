/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: parseparams_byteio.h,v 1.4 2008/05/07 05:47:00 asuraparaju Exp $ $Name:  $
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
* Contributor(s): Scott R Ladd (Original Author), Thomas Davies
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
* Definition of class ParseParamsByteIO
*/
#ifndef parseparams_byteio_h
#define parseparams_byteio_h


// DIRAC INCLUDES
#include <libdirac_common/common.h>             // DiracEncoder

//LOCAL INCLUDES
#include <libdirac_byteio/byteio.h>             // Parent class


namespace dirac
{
             
    /**
    * Represents compressed parse-parameter data used in an AccessUnit
    */
    class ParseParamsByteIO : public ByteIO
    {
    public:

        /**
        * Constructor
        *@param stream_data   Destination of data
        *@param parse_params  Parse parameters
        *@param enc_params    Encoder parameters
        */
        ParseParamsByteIO(const ByteIO& stream_data,
                          ParseParams &parse_params,
                          EncoderParams &enc_params);

        /**
        * Constructor
        *@param stream_data Source of data
        *@param parse_params Destination of parse params
        */
        ParseParamsByteIO(const ByteIO& stream_data,
                          ParseParams &parse_params);

        /**
        * Destructor
        */
        ~ParseParamsByteIO();

         /**
        * Reads parse information from Dirac byte-format
        */
        void Input();

        /**
        * Outputs parse information to Dirac byte-format
        */
        void Output();

        /**
        * Get access-unit number
        */
        int GetIdNumber() const;

    protected:
    
    private:
        void CheckVersion();
        void CheckProfile();
        void CheckLevel();

    private:
        /**
        * Reference to parse parameters
        */
        ParseParams& m_parse_params;
    };


} // namespace dirac

#endif
