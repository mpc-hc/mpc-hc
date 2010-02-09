/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: displayparams_byteio.cpp,v 1.10 2008/01/31 11:25:15 tjdwave Exp $ $Name:  $
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

#include <libdirac_byteio/displayparams_byteio.h>
#include <libdirac_common/dirac_exception.h>

using namespace dirac;

SourceParamsByteIO::SourceParamsByteIO( SourceParams& src_params,
                                         const SourceParams& default_src_params,
                                         const ByteIO& stream_data):
ByteIO(stream_data),
m_src_params(src_params),
m_default_src_params(default_src_params)
{
    

}

SourceParamsByteIO::~SourceParamsByteIO()
{
}

//-----------------public----------------------------------------------------

void SourceParamsByteIO::Input()
{
    // input frame dimensions
    InputFrameSize();

    // input chroma sampling format
    InputChromaSamplingFormat();

    // input scan format
    InputScanFormat();

    // input frame rate
    InputFrameRate();

    // input pixel aspect ratio 
    InputPixelAspectRatio();

    // input clean area
    InputCleanArea();

    // input signal range
    InputSignalRange();

    // input colour spec
    InputColourSpecification();
}

void SourceParamsByteIO::Output()
{
    // output frame dimensions
    OutputFrameSize();

    // output chroma sampling format
    OutputChromaSamplingFormat();

    // output scan format
    OutputScanFormat();

    // output frame rate
    OutputFrameRate();

    // output pixel aspect ratio 
    OutputPixelAspectRatio();

    // output clean area
    OutputCleanArea();

    // output signal range
    OutputSignalRange();

    // output colour spec
    OutputColourSpecification();
}

//-------------------private-----------------------------------------------
void SourceParamsByteIO::InputFrameSize()
{
    bool custom_flag = ReadBool();

    if(!custom_flag)
        return;

    // set custom width
    m_src_params.SetXl(ReadUint());

    // set custom height
    m_src_params.SetYl(ReadUint());
}

void SourceParamsByteIO::InputChromaSamplingFormat()
{
    bool chroma_flag = ReadBool();

    if(!chroma_flag)
        return;

    // set chroma
    ChromaFormat chroma_format = IntToChromaFormat(ReadUint());
    if(chroma_format==formatNK)
        DIRAC_THROW_EXCEPTION(
                    ERR_INVALID_CHROMA_FORMAT,
                    "Dirac does not recognise the specified chroma-format",
                    SEVERITY_ACCESSUNIT_ERROR)
    m_src_params.SetCFormat(chroma_format);
}

void SourceParamsByteIO::InputPixelAspectRatio()
{
    bool pixel_aspect_ratio_flag = ReadBool();
    if(!pixel_aspect_ratio_flag)
        return;

    // read index value
    int pixel_aspect_ratio_index = ReadUint();
    PixelAspectRatioType pixel_aspect_ratio=IntToPixelAspectRatioType(pixel_aspect_ratio_index);
    if(pixel_aspect_ratio==PIXEL_ASPECT_RATIO_UNDEFINED)
    DIRAC_THROW_EXCEPTION(
                    ERR_INVALID_PIXEL_ASPECT_RATIO,
                    "Dirac does not recognise the specified pixel_aspect_ratio",
                    SEVERITY_ACCESSUNIT_ERROR)

    if(pixel_aspect_ratio_index!=PIXEL_ASPECT_RATIO_CUSTOM)
    {
        m_src_params.SetPixelAspectRatio(pixel_aspect_ratio);
    }
    else
    {
        // read num/denom
        int numerator = ReadUint();
        int denominator = ReadUint();
        m_src_params.SetPixelAspectRatio(numerator, denominator);
    }

}

void SourceParamsByteIO::InputCleanArea()
{
    bool clean_area_flag = ReadBool();
    if(!clean_area_flag)
        return;

    m_src_params.SetCleanWidth( ReadUint() );
    m_src_params.SetCleanHeight( ReadUint() );
    m_src_params.SetLeftOffset( ReadUint() );
    m_src_params.SetTopOffset( ReadUint() );
}

void SourceParamsByteIO::InputColourMatrix()
{
    bool colour_matrix_flag = ReadBool();
    if(!colour_matrix_flag)
        return;

    // read index value
    int colour_matrix_index = ReadUint();
    m_src_params.SetColourMatrixIndex(colour_matrix_index);
}

void SourceParamsByteIO::InputColourPrimaries()
{
    bool colour_primaries_flag = ReadBool();
    if(!colour_primaries_flag)
        return;

    // read index value
    int colour_primaries_index = ReadUint();
    m_src_params.SetColourPrimariesIndex(colour_primaries_index);
}

void SourceParamsByteIO::InputColourSpecification()
{
    bool colour_spec_flag = ReadBool();
    if(!colour_spec_flag)
        return;

    // read index value
    int colour_spec_index = ReadUint();
    m_src_params.SetColourSpecification( colour_spec_index );
    if(colour_spec_index==0)
    {
        InputColourPrimaries();
        InputColourMatrix();
        InputTransferFunction();
    }
}

void SourceParamsByteIO::InputFrameRate()
{
    bool fr_flag = ReadBool();
    if(!fr_flag)
        return;

    int frame_rate_index = ReadUint();
    FrameRateType frame_rate=IntToFrameRateType(frame_rate_index);
    if(frame_rate==FRAMERATE_UNDEFINED)
        DIRAC_THROW_EXCEPTION(
                    ERR_INVALID_PICTURE_RATE,
                    "Dirac does not recognise the specified frame-rate",
                    SEVERITY_ACCESSUNIT_ERROR)

    if(frame_rate_index!=FRAMERATE_CUSTOM)
    {
        m_src_params.SetFrameRate(frame_rate);
    }
    else
    {
        // read num/denom
        int numerator = ReadUint();
        int denominator = ReadUint();
        m_src_params.SetFrameRate(numerator, denominator);
    }
}

void SourceParamsByteIO::InputScanFormat()
{
    bool scan_flag = ReadBool();
    if(!scan_flag)
        return;

    unsigned int source_sampling = ReadUint();
    if (source_sampling > 1)
    {
        std::ostringstream errstr;
        errstr << "Source Sampling " << source_sampling
               << " out of range [0-1]";
        DIRAC_THROW_EXCEPTION(
            ERR_UNSUPPORTED_STREAM_DATA,
            errstr.str(),
            SEVERITY_ACCESSUNIT_ERROR);
    }
    m_src_params.SetSourceSampling(source_sampling);
}

void SourceParamsByteIO::InputSignalRange()
{
    bool signal_range_flag = ReadBool();
    if(!signal_range_flag)
        return;

    // read index value
    int signal_range_index = ReadUint();
    SignalRangeType signal_range = IntToSignalRangeType(signal_range_index);
    if(signal_range==SIGNAL_RANGE_UNDEFINED)
        DIRAC_THROW_EXCEPTION(
                    ERR_INVALID_SIGNAL_RANGE,
                    "Dirac does not recognise the specified signal-range",
                    SEVERITY_ACCESSUNIT_ERROR)

    if(signal_range_index!=SIGNAL_RANGE_CUSTOM)
    {
        m_src_params.SetSignalRange(signal_range);
    }
    else
    {
        // read luma values
        m_src_params.SetLumaOffset( ReadUint() );
        m_src_params.SetLumaExcursion( ReadUint() );
        // read chroma values
        m_src_params.SetChromaOffset( ReadUint() );
        m_src_params.SetChromaExcursion( ReadUint() );
    }
}

void SourceParamsByteIO::InputTransferFunction()
{
    bool trans_fun_flag = ReadBool();
    if(!trans_fun_flag)
        return;

    // read index value
    int trans_fun_index = ReadUint();
    m_src_params.SetTransferFunctionIndex(trans_fun_index);
}

void SourceParamsByteIO::OutputFrameSize()
{

    // output 'is custom' dimensions flag
    bool is_custom = (m_src_params.Xl()!=m_default_src_params.Xl() ||
                      m_src_params.Yl()!=m_default_src_params.Yl());

    WriteBit(is_custom);

    if(!is_custom)
        return;

    // set custom X and Y
    WriteUint(m_src_params.Xl());
    WriteUint(m_src_params.Yl());

}

void SourceParamsByteIO::OutputChromaSamplingFormat()
{
    // output 'is default' flag
    bool not_default =  m_src_params.CFormat()!=m_default_src_params.CFormat();
    
    WriteBit(not_default);

    if(!not_default)
        return;

    // output chroma index
    WriteUint(static_cast<int>(m_src_params.CFormat()));
}


void SourceParamsByteIO::OutputPixelAspectRatio()
{
    if (m_src_params.PixelAspectRatioIndex()!= PIXEL_ASPECT_RATIO_CUSTOM
        && m_src_params.PixelAspectRatioIndex() == m_default_src_params.PixelAspectRatioIndex())
    {
        // default frame rate index
        WriteBit(0);
        return;
    }
    // Non-defaults
       WriteBit(1);

    // Picture rate index
    WriteUint(m_src_params.PixelAspectRatioIndex());
    
    if (!m_src_params.PixelAspectRatioIndex()) // i,e. custom value
    {
        WriteUint(m_src_params.PixelAspectRatio().m_num);
        WriteUint(m_src_params.PixelAspectRatio().m_denom);
    }
}


void SourceParamsByteIO::OutputCleanArea()
{
    if (m_src_params.CleanWidth() != m_default_src_params.CleanWidth() ||
        m_src_params.CleanHeight() != m_default_src_params.CleanHeight() ||
        m_src_params.LeftOffset() != m_default_src_params.LeftOffset() ||
        m_src_params.TopOffset() != m_default_src_params.TopOffset())
    {
        WriteBit(1); // non-default value
        WriteUint(m_src_params.CleanWidth());
        WriteUint(m_src_params.CleanHeight());
        WriteUint(m_src_params.LeftOffset());
        WriteUint(m_src_params.TopOffset());
    }
    else
        WriteBit(0); // default value
}

void SourceParamsByteIO::OutputColourSpecification()
{
    if (m_src_params.ColourSpecificationIndex() &&
        m_src_params.ColourSpecificationIndex() == 
        m_default_src_params.ColourSpecificationIndex())
    {
        // default colour specification
        WriteBit(0);
        return;
    }

    // Non-defaults
       WriteBit(1);
    // Output Colour specification index
    WriteUint(m_src_params.ColourSpecificationIndex());

    if (!m_src_params.ColourSpecificationIndex()) // i,e, custom values
    {
        // Output Colour Primaries
        if (m_src_params.ColourPrimariesIndex() == m_default_src_params.ColourPrimariesIndex())
        {
            // default value
            WriteBit(0);
        }
        else
        {
            WriteBit(1);
            WriteUint(m_src_params.ColourPrimariesIndex());
        }

        // Output Colour Matrix
        if (m_src_params.ColourMatrixIndex() == m_default_src_params.ColourMatrixIndex())
        {
            // default value
            WriteBit(0);
        }
        else
        {
            WriteBit(1);
            WriteUint(m_src_params.ColourMatrixIndex());
        }

        // Output TransferFunction
        if (m_src_params.TransferFunctionIndex() == m_default_src_params.TransferFunctionIndex())
        {
            // default value
            WriteBit(0);
        }
        else
        {
            WriteBit(1);
            WriteUint(m_src_params.TransferFunctionIndex());
        }
    }
}

void SourceParamsByteIO::OutputFrameRate()
{
    if (m_src_params.FrameRateIndex()!=FRAMERATE_CUSTOM
        && m_src_params.FrameRateIndex() == m_default_src_params.FrameRateIndex())
    {
        // default frame rate index
        WriteBit(0);
        return;
    }
    // Non-defaults
       WriteBit(1);

    // Picture rate index
    WriteUint(m_src_params.FrameRateIndex());
    
    if (!m_src_params.FrameRateIndex()) // i,e. custom value
    {
        WriteUint(m_src_params.FrameRate().m_num);
        WriteUint(m_src_params.FrameRate().m_denom);
    }
}

void SourceParamsByteIO::OutputScanFormat()
{
    // output 'is default' flag
    bool not_interlace_default =  m_src_params.SourceSampling()!=m_default_src_params.SourceSampling();
    
    WriteBit(not_interlace_default);

    if(!not_interlace_default)
        return;

    // output interlace value
    WriteUint(m_src_params.SourceSampling());
}


void SourceParamsByteIO::OutputSignalRange()
{
    if (m_src_params.SignalRangeIndex()!=SIGNAL_RANGE_CUSTOM && 
        m_src_params.SignalRangeIndex() == m_default_src_params.SignalRangeIndex())
    {
        // defaults
        WriteBit(0);
        return;
    }
    
    // Non-defaults
       WriteBit(1);
    // Output Signal Range Index
    WriteUint(m_src_params.SignalRangeIndex());

    if (!m_src_params.SignalRangeIndex()) // i.e. custom values
    {
        WriteUint(m_src_params.LumaOffset());
        WriteUint(m_src_params.LumaExcursion());
        WriteUint(m_src_params.ChromaOffset());
        WriteUint(m_src_params.ChromaExcursion());
    }
}
