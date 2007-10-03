/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: seqparams_byteio.cpp,v 1.3 2006/06/05 14:51:23 asuraparaju Exp $ $Name: Dirac_0_7_0 $
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

#include <libdirac_byteio/seqparams_byteio.h>
#include <libdirac_common/common.h>
#include <libdirac_common/dirac_exception.h>

using namespace dirac;

SeqParamsByteIO::SeqParamsByteIO(SeqParams& seq_params,
                                 const SeqParams& default_seq_params,
                                 const ByteIO& stream_data):
ByteIO(stream_data),
m_seq_params(seq_params),
m_default_seq_params(default_seq_params)
{
    

}

SeqParamsByteIO::~SeqParamsByteIO()
{
}

//---------public---------------------------------------------------

void SeqParamsByteIO::Input()
{
    // input video-format & set all parameters to default
    VideoFormat video_format = IntToVideoFormat(InputVarLengthUint());
    if(video_format==VIDEO_FORMAT_UNDEFINED)
         DIRAC_THROW_EXCEPTION(
                    ERR_INVALID_VIDEO_FORMAT,
                    "Dirac does not recognise the specified video-format",
                    SEVERITY_ACCESSUNIT_ERROR)

    SeqParams seq_params(video_format, true);
    m_seq_params = seq_params;

    // input image size
    InputImageSize();

    // input chroma
    InputChromaFormat();

    // input video-depth
    InputVideoDepth();
}


void SeqParamsByteIO::Output()
{
    // output video format
    OutputVarLengthUint(static_cast<int>(m_seq_params.GetVideoFormat()));

    // output image size
    OutputImageSize();

    // output chroma 
    OutputChromaFormat();

    // output video depth
    OutputVideoDepth();

}

//-------------private---------------------------------------------------------------

void SeqParamsByteIO::InputChromaFormat()
{
    bool chroma_flag = InputBit();

    if(!chroma_flag)
        return;

    // set chroma
    ChromaFormat chroma_format = IntToChromaFormat(InputVarLengthUint());
    if(chroma_format==formatNK)
        DIRAC_THROW_EXCEPTION(
                    ERR_INVALID_CHROMA_FORMAT,
                    "Dirac does not recognise the specified chroma-format",
                    SEVERITY_ACCESSUNIT_ERROR)
    m_seq_params.SetCFormat(chroma_format);
}

void SeqParamsByteIO::InputImageSize()
{
    bool custom_flag = InputBit();

    if(!custom_flag)
        return;

    // set custom width
    m_seq_params.SetXl(InputVarLengthUint());

    // set custom height
    m_seq_params.SetYl(InputVarLengthUint());
}

void SeqParamsByteIO::InputVideoDepth()
{
    bool depth_flag = InputBit();

    if(depth_flag)
        //NOTE: FIXME - remove when we support multiple video-depths
        DIRAC_THROW_EXCEPTION(
                    ERR_INVALID_VIDEO_DEPTH,
                    "Dirac does not support video-depth other than 8 at the moment",
                    SEVERITY_ACCESSUNIT_ERROR)

    if (depth_flag)
    {
        int video_depth = InputVarLengthUint();
        // set video depth
        m_seq_params.SetVideoDepth(video_depth);
    }
}

void SeqParamsByteIO::OutputChromaFormat()
{
    // output 'is default' flag
    bool not_default =  m_seq_params.CFormat()!=m_default_seq_params.CFormat();
    
    OutputBit(not_default);

    if(!not_default)
        return;

    // output chroma index
    OutputVarLengthUint(static_cast<int>(m_seq_params.CFormat()));
}

void SeqParamsByteIO::OutputImageSize()
{

    // output 'is custom' dimensions flag
    bool is_custom = (m_seq_params.Xl()!=m_default_seq_params.Xl() ||
                      m_seq_params.Yl()!=m_default_seq_params.Yl());

    OutputBit(is_custom);

    if(!is_custom)
        return;

    // set custom X and Y
    OutputVarLengthUint(m_seq_params.Xl());
    OutputVarLengthUint(m_seq_params.Yl());

}

void SeqParamsByteIO::OutputVideoDepth()
{
     // output 'is default' flag
    bool not_default =  m_seq_params.GetVideoDepth()!=m_default_seq_params.GetVideoDepth();
    
    OutputBit(not_default);

    if(!not_default)
        return;

    // output video depth
    OutputVarLengthUint(static_cast<int>(m_seq_params.GetVideoDepth()));
}

