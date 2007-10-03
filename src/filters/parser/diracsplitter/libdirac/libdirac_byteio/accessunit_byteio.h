/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: accessunit_byteio.h,v 1.1 2006/04/20 10:41:56 asuraparaju Exp $ $Name: Dirac_0_7_0 $
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
* Contributor(s): Andrew Kennedy(Original Author)
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
* Definition of class AccessUnitByteIO
*/
#ifndef accessunit_byteio_h
#define accessunit_byteio_h


//LOCAL INCLUDES
#include <libdirac_byteio/parseunit_byteio.h>             // Parent class
#include <libdirac_byteio/displayparams_byteio.h>         // DisplayParamsByteIO class
#include <libdirac_byteio/parseparams_byteio.h>           // ParseParamsByteIO class
#include <libdirac_byteio/seqparams_byteio.h>             // SeqParamsByteIO class


namespace dirac
{
    /**
    * A random access point within a Dirac bytestream 
    */
    class AccessUnitByteIO : public ParseUnitByteIO
    {
    public:

        /**
        * Constructor (encoding)
        *@param accessunit_fnum Current AccessUnit frame-number
        *@param seq_params Sequence parameters for current AccessUnit
        *@param src_params Source parameters for current AccessUnit
        */
        AccessUnitByteIO(int& accessunit_fnum,
                         SeqParams& seq_params,
                         SourceParams& src_params);

        /**
        * Constructor (decoding)
        *@param parseunit_byteio Source of data
        *@param seq_params       Destination of sequence paramters data 
        *@param src_params       Destination of source paramters data 
        *@param parse_params     Destination of source paramters data 
        */
        AccessUnitByteIO(const ParseUnitByteIO& parseunit_byteio,
                         SeqParams& seq_params,
                         SourceParams& src_params,
                         ParseParams& parse_params);

       /**
       * Destructor
       */
        ~AccessUnitByteIO();

        /**
        * Parses data in Dirac-stream format (decoding)
        */
        bool Input();

        /**
        * Writes access-unit info to Dirac stream-format (encoding)
        */
        void Output();
     
        /**
        * Get access-unit number
        */
        int GetIdNumber() const;

        /* 
        * Gets size of access-unit (in bytes)
        */
        int GetSize() const;

        /**
        * Gets parse-unit type
        */
        ParseUnitType GetType() const { return PU_ACCESS_UNIT;}

    private:

        /**
        * Calculates parse-code based on access-unit parameters (encoding)
        *@return Char bit-set 
        */
        unsigned char CalcParseCode() const;

         /**
        * Parse display attributes from bytestream-compatible input (decoding)
        */
        void InputDisplayParams();
        
        /**
        * Parse parse attributes from bytestream-compatible input (decoding)
        */
        void InputParseParams();

        /**
        * Parse sequence attributes from bytestream-compatible input (decoding)
        */
        void InputSequenceParams();

        /**
        * Output display attributes for bytestream-compatible output (encoding)
        */
        void OutputDisplayParams();
        
        /**
        * Output parse attributes for bytestream-compatible output (encoding)
        */
        void OutputParseParams();

        /**
        * Output sequence attributes for bytestream-compatible output (encoding)
        */
        void OutputSequenceParams();
    
        /**
        * Parse-params byte input/output
        */
        ParseParamsByteIO m_parseparams_byteio;
   
        /**
        * Default sequence parameters
        */
        SeqParams m_default_seq_params;

        /**
        * Default source parameters
        */
        SourceParams m_default_src_params;
   
        /**
        * Sequence-params byte input/output
        */
        SeqParamsByteIO m_seqparams_byteio;

        /**
        * Display-params byte input/output
        */
        DisplayParamsByteIO m_displayparams_byteio;
   };

} // namespace dirac

#endif
