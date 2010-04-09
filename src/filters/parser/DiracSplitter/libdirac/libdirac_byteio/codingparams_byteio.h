/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: codingparams_byteio.h,v 1.3 2007/12/05 01:42:40 asuraparaju Exp $ $Name:  $
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
* Contributor(s): Anuradha Suraparaju (Original Author)
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
* Definition of class CodingParamsByteIO
*/
#ifndef codingparams_byteio_h
#define codingparams_byteio_h


// DIRAC INCLUDES
#include <libdirac_common/common.h>             // CodecParams, SourceParams

//LOCAL INCLUDES
#include <libdirac_byteio/byteio.h>             // Parent class


namespace dirac
{
             
    /**
    * Represents compressed sequence-parameter data used in an AccessUnit
    */
    class CodingParamsByteIO : public ByteIO
    {
    public:

        /**
        * Constructor
        *@param src_params             Source parameters
        *@param codec_params           Coding parameters
        *@param default_source_params  Default source parameters
        *@param stream_data Source/Destination of data
        */
        CodingParamsByteIO(const SourceParams& src_params,
                           CodecParams& codec_params,
                           const SourceParams& default_source_params,
                           const ByteIO& stream_data);
                      

        /**
        * Destructor
        */
        ~CodingParamsByteIO();

        /**
        * Reads sequence information from Dirac byte-format
        */
        void Input();

        /**
        * Outputs sequence information to Dirac byte-format
        */
        void Output();

    protected:
    

    private:
    
        /**
        * Reads number of bits used to compress input signal 
        */
        void InputVideoDepth();

        /**
        * Reads picture coding mode - 0 - frames, 1 - fields
        */
        void InputPictureCodingMode();

        /**
        * Outputs number of bits used to compress input signal 
        */
        void OutputVideoDepth();

        /**
        * Outputs how input was coded - i.e. frames or fields
        */
        void OutputPictureCodingMode();

        /**
        * Source paramters for intput/output
        */
        const SourceParams&   m_src_params;

        /**
        * Coding paramters for intput/output
        */
        CodecParams&   m_codec_params;

        /**
        * Default source parameters
        */
        const SourceParams& m_default_source_params;

    };


} // namespace dirac

#endif
