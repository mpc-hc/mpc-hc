/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: accessunit_byteio.cpp,v 1.7 2008/08/14 00:51:08 asuraparaju Exp $ $Name:  $
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
SequenceHeaderByteIO::SequenceHeaderByteIO( SourceParams& src_params, 
                                    EncoderParams& enc_params):                  
ParseUnitByteIO(),
m_parseparams_byteio(*this, m_parse_params, enc_params),
// create default source parameters for comparisions
m_default_src_params(src_params.GetVideoFormat()),
m_src_params(src_params),
m_sourceparams_byteio( m_src_params,
                        m_default_src_params,
                       *this),
m_codec_params(enc_params),
m_codingparams_byteio(m_src_params,
                      m_codec_params,
                      m_default_src_params,
                      *this)
{
}

// Constructor for decoding
SequenceHeaderByteIO::SequenceHeaderByteIO(const ParseUnitByteIO& parseunit_byteio,
                                   ParseParams& parse_params,
                                   SourceParams& src_params,
                                   CodecParams& codec_params) :
ParseUnitByteIO(parseunit_byteio),
m_parseparams_byteio( parseunit_byteio, parse_params),
m_src_params(src_params),
m_sourceparams_byteio( m_src_params,
                        m_default_src_params,
                        parseunit_byteio),
m_codec_params(codec_params),
m_codingparams_byteio( m_src_params,
                        m_codec_params,
                        m_default_src_params,
                        parseunit_byteio)
{
}

SequenceHeaderByteIO::~SequenceHeaderByteIO()
{
}

//-----public------------------------------------------------------
bool SequenceHeaderByteIO::Input() 
{
    //int o=mp_stream->tellg();
    InputParseParams();

    // Inout Video format
    SetByteParams(m_parseparams_byteio);
    VideoFormat vf = IntToVideoFormat(ReadUint());
    if(vf==VIDEO_FORMAT_UNDEFINED)
         DIRAC_THROW_EXCEPTION(
                    ERR_INVALID_VIDEO_FORMAT,
                    "Dirac does not recognise the specified video-format",
                    SEVERITY_ACCESSUNIT_ERROR);

    SourceParams src_params(vf, true);
    m_src_params = src_params;
    
    InputSourceParams();
    
    CodecParams codec_params(vf);
    m_codec_params = codec_params;
    
    InputCodingParams();
    
    return true;
}

void SequenceHeaderByteIO::Output()
{
    OutputParseParams();

    // Output the video format
    SetByteParams(m_parseparams_byteio);
    WriteUint(static_cast<int>(m_src_params.GetVideoFormat()));

    OutputSourceParams();

    OutputCodingParams();
  
}

int SequenceHeaderByteIO::GetSize() const
{
    return ParseUnitByteIO::GetSize()+
           m_parseparams_byteio.GetSize()+
           ByteIO::GetSize() + 
           m_sourceparams_byteio.GetSize()+
           m_codingparams_byteio.GetSize();
}



//-------private-------------------------------------------------------

unsigned char SequenceHeaderByteIO::CalcParseCode() const
{
    unsigned char code = 0;

    // no further mods required

    return code;
}


void SequenceHeaderByteIO::InputSourceParams()
{
     // copy current input params
    m_sourceparams_byteio.SetByteParams(*this);

    m_sourceparams_byteio.Input();
}

void SequenceHeaderByteIO::InputParseParams()
{
    m_parseparams_byteio.Input();
}

void SequenceHeaderByteIO::InputCodingParams()
{
    // copy current input params
    m_codingparams_byteio.SetByteParams(m_sourceparams_byteio);

    m_codingparams_byteio.Input();
}

void SequenceHeaderByteIO::OutputSourceParams()
{
    // copy current output params
    m_sourceparams_byteio.SetByteParams(*this);

    m_sourceparams_byteio.Output();
}

void SequenceHeaderByteIO::OutputParseParams()
{
    m_parseparams_byteio.Output();
}

void SequenceHeaderByteIO::OutputCodingParams()
{
    // copy current output params
    m_codingparams_byteio.SetByteParams(m_sourceparams_byteio);

    m_codingparams_byteio.Output();
}

