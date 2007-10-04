/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: accessunit_byteio.cpp,v 1.3 2007/09/03 11:31:42 asuraparaju Exp $ $Name: Dirac_0_8_0 $
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

#include <libdirac_common/dirac_exception.h>
#include <libdirac_byteio/accessunit_byteio.h>

using namespace dirac;

// Constructor for encoding
AccessUnitByteIO::AccessUnitByteIO( SourceParams& src_params, CodecParams& codec_params):                  
ParseUnitByteIO(),
m_parseparams_byteio(*this),
// create default source parameters for comparisions
m_default_src_params(src_params.GetVideoFormat()),
m_src_params(src_params),
m_displayparams_byteio( m_src_params,
                        m_default_src_params,
                       *this),
m_codec_params(codec_params),
m_codingparams_byteio(m_src_params,
                      m_codec_params,
                      m_default_src_params,
                      *this)
{
}

// Constructor for decoding
AccessUnitByteIO::AccessUnitByteIO(const ParseUnitByteIO& parseunit_byteio,
                                   ParseParams& parse_params,
                                   SourceParams& src_params,
                                   CodecParams& codec_params) :
ParseUnitByteIO(parseunit_byteio),
m_parseparams_byteio( parseunit_byteio, parse_params),
m_src_params(src_params),
m_displayparams_byteio( m_src_params,
                        m_default_src_params,
                        parseunit_byteio),
m_codec_params(codec_params),
m_codingparams_byteio( m_src_params,
                        m_codec_params,
                        m_default_src_params,
                        parseunit_byteio)
{
}

AccessUnitByteIO::~AccessUnitByteIO()
{
}

//-----public------------------------------------------------------
bool AccessUnitByteIO::Input() 
{
    //int o=mp_stream->tellg();
    InputParseParams();

    // Inout Video format
    SetByteParams(m_parseparams_byteio);
    VideoFormat vf = IntToVideoFormat(InputVarLengthUint());
    if(vf==VIDEO_FORMAT_UNDEFINED)
         DIRAC_THROW_EXCEPTION(
                    ERR_INVALID_VIDEO_FORMAT,
                    "Dirac does not recognise the specified video-format",
                    SEVERITY_ACCESSUNIT_ERROR);

    SourceParams src_params(vf, true);
    m_src_params = src_params;
    
    InputDisplayParams();
    
    CodecParams codec_params(vf);
    m_codec_params = codec_params;
    
    InputCodingParams();
    
    return true;
}

void AccessUnitByteIO::Output()
{
    OutputParseParams();

    // Output the video format
    SetByteParams(m_parseparams_byteio);
    OutputVarLengthUint(static_cast<int>(m_src_params.GetVideoFormat()));

    OutputDisplayParams();

    OutputCodingParams();
  
}

int AccessUnitByteIO::GetSize() const
{
    return ParseUnitByteIO::GetSize()+
           m_parseparams_byteio.GetSize()+
           ByteIO::GetSize() + 
           m_displayparams_byteio.GetSize()+
           m_codingparams_byteio.GetSize();
}



//-------private-------------------------------------------------------

unsigned char AccessUnitByteIO::CalcParseCode() const
{
    unsigned char code = 0;

    // no further mods required

    return code;
}


void AccessUnitByteIO::InputDisplayParams()
{
     // copy current input params
    m_displayparams_byteio.SetByteParams(*this);

    m_displayparams_byteio.Input();
}

void AccessUnitByteIO::InputParseParams()
{
    m_parseparams_byteio.Input();
}

void AccessUnitByteIO::InputCodingParams()
{
    // copy current input params
    m_codingparams_byteio.SetByteParams(m_displayparams_byteio);

    m_codingparams_byteio.Input();
}

void AccessUnitByteIO::OutputDisplayParams()
{
    // copy current output params
    m_displayparams_byteio.SetByteParams(*this);

    m_displayparams_byteio.Output();
}

void AccessUnitByteIO::OutputParseParams()
{
    m_parseparams_byteio.Output();
}

void AccessUnitByteIO::OutputCodingParams()
{
    // copy current output params
    m_codingparams_byteio.SetByteParams(m_displayparams_byteio);

    m_codingparams_byteio.Output();
}

