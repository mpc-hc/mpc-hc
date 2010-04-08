/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: accessunit_byteio.h,v 1.7 2008/08/14 00:51:08 asuraparaju Exp $ $Name:  $
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
* Definition of class SequenceHeaderByteIO
*/
#ifndef accessunit_byteio_h
#define accessunit_byteio_h

//LOCAL INCLUDES
#include <libdirac_byteio/parseunit_byteio.h>             // Parent class
#include <libdirac_byteio/parseparams_byteio.h>           // ParseParamsByteIO class
#include <libdirac_byteio/displayparams_byteio.h>         // SourceParamsByteIO class
#include <libdirac_byteio/codingparams_byteio.h>         // CodingParamsByteIO class

namespace dirac
{
/**
* A random access point within a Dirac bytestream
*/
class SequenceHeaderByteIO : public ParseUnitByteIO
{
public:

    /**
    * Constructor (encoding)
    *@param src_params Source parameters for current AccessUnit
    *@param enc_params Encoder parameters for current AccessUnit
    */
    SequenceHeaderByteIO(SourceParams& src_params,
                         EncoderParams& enc_params);

    /**
    * Constructor (decoding)
    *@param parseunit_byteio Source of data
    *@param parse_params     Destination of parse paramters data
    *@param src_params       Destination of source paramters data
    *@param codec_params     Destination of coding paramters data
    */
    SequenceHeaderByteIO(const ParseUnitByteIO& parseunit_byteio,
                         ParseParams& parse_params,
                         SourceParams& src_params,
                         CodecParams& codec_params);

    /**
    * Destructor
    */
    ~SequenceHeaderByteIO();

    /**
    * Parses data in Dirac-stream format (decoding)
    */
    bool Input();

    /**
    * Writes access-unit info to Dirac stream-format (encoding)
    */
    void Output();

    /*
    * Gets size of access-unit (in bytes)
    */
    int GetSize() const;

    /**
    * Gets parse-unit type
    */
    ParseUnitType GetType() const
    {
        return PU_SEQ_HEADER;
    }

private:

    /**
    * Calculates parse-code based on access-unit parameters (encoding)
    *@return Char bit-set
    */
    unsigned char CalcParseCode() const;

    /**
            * Parse source attributes from bytestream-compatible input (decoding)
            */
    void InputSourceParams();

    /**
    * Parse parse attributes from bytestream-compatible input (decoding)
    */
    void InputParseParams();

    /**
    * Parse Coding attributes from bytestream-compatible input (decoding)
    */
    void InputCodingParams();

    /**
    * Output source attributes for bytestream-compatible output (encoding)
    */
    void OutputSourceParams();

    /**
    * Output parse attributes for bytestream-compatible output (encoding)
    */
    void OutputParseParams();

    /**
    * Output coding attributes for bytestream-compatible output (encoding)
    */
    void OutputCodingParams();

    /**
    * Current parse parameters
    */
    ParseParams m_parse_params;


    /**
    * Parse-params byte input/output
    */
    ParseParamsByteIO m_parseparams_byteio;

    /**
    * Default source parameters
    */
    SourceParams m_default_src_params;

    /**
    * Current source parameters
    */
    SourceParams& m_src_params;

    /**
    * Source-params byte input/output
    */
    SourceParamsByteIO m_sourceparams_byteio;

    /**
    * Current codec parameters
    */
    CodecParams& m_codec_params;

    /**
    * Coding-params byte input/output
    */
    CodingParamsByteIO m_codingparams_byteio;
};

} // namespace dirac

#endif
