/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: codingparams_byteio.cpp,v 1.9 2008/04/29 08:51:52 tjdwave Exp $ $Name:  $
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

#include <libdirac_byteio/codingparams_byteio.h>
#include <libdirac_common/common.h>
#include <libdirac_common/dirac_exception.h>

using namespace dirac;

CodingParamsByteIO::CodingParamsByteIO(const SourceParams& src_params,
                                       CodecParams& codec_params,
                                       const SourceParams& default_source_params,
                                       const ByteIO& stream_data):
    ByteIO(stream_data),
    m_src_params(src_params),
    m_codec_params(codec_params),
    m_default_source_params(default_source_params)
{

}

CodingParamsByteIO::~CodingParamsByteIO()
{
}

//---------public---------------------------------------------------

void CodingParamsByteIO::Input()
{
    // input picture coding mode
    InputPictureCodingMode();

    m_codec_params.SetTopFieldFirst(m_src_params.TopFieldFirst());

    // Set the dimensions to frame dimensions
    m_codec_params.SetXl(m_src_params.Xl());
    m_codec_params.SetYl(m_src_params.Yl());

    m_codec_params.SetChromaXl(m_src_params.ChromaWidth());
    m_codec_params.SetChromaYl(m_src_params.ChromaHeight());

    // If source was coded as fields, halve the vertical dimensions
    // to set them to field dimensions
    if(m_codec_params.FieldCoding())
    {
        m_codec_params.SetYl(m_codec_params.Yl() >> 1);
        m_codec_params.SetChromaYl(m_codec_params.ChromaYl() >> 1);
    }

    unsigned int luma_depth = static_cast<unsigned int>
                              (
                                  std::log((double)m_src_params.LumaExcursion()) / std::log(2.0) + 1
                              );
    m_codec_params.SetLumaDepth(luma_depth);

    unsigned int chroma_depth = static_cast<unsigned int>
                                (
                                    std::log((double)m_src_params.ChromaExcursion()) / std::log(2.0) + 1
                                );
    m_codec_params.SetChromaDepth(chroma_depth);

    // byte align
    ByteAlignInput();
}


void CodingParamsByteIO::Output()
{
    // output picture coding mode flag
    OutputPictureCodingMode();

    // byte align
    ByteAlignOutput();
}

//-------------private---------------------------------------------------------------

void CodingParamsByteIO::InputPictureCodingMode()
{
    unsigned int coding_mode = ReadUint();
    if(coding_mode > 1)
    {
        std::ostringstream errstr;
        errstr << "Picture coding mode " << coding_mode
               << " out of range [0-1]";
        DIRAC_THROW_EXCEPTION(
            ERR_UNSUPPORTED_STREAM_DATA,
            errstr.str(),
            SEVERITY_ACCESSUNIT_ERROR);
    }
    m_codec_params.SetPictureCodingMode(coding_mode);
}


void CodingParamsByteIO::OutputPictureCodingMode()
{
    WriteUint(m_codec_params.FieldCoding() ? 1 : 0);
}


