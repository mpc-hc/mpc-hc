/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: displayparams_byteio.cpp,v 1.2 2006/04/20 15:39:30 asuraparaju Exp $ $Name: Dirac_0_7_0 $
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

DisplayParamsByteIO::DisplayParamsByteIO(const SeqParams &seq_params,
                                         SourceParams& src_params,
                                         const SourceParams& default_src_params,
                                         const ByteIO& stream_data):
ByteIO(stream_data),
m_src_params(src_params),
m_default_src_params(default_src_params),
m_seq_params(seq_params)
{
    

}

DisplayParamsByteIO::~DisplayParamsByteIO()
{
}

//-----------------public----------------------------------------------------

void DisplayParamsByteIO::Input()
{
    // Set the defaults for source params.
    SourceParams tmp_src_params = SourceParams(m_seq_params.GetVideoFormat(), true);
    m_src_params = tmp_src_params;

    // input scan format
    InputScanFormat();

    // input frame rate
    InputFrameRate();

    // input aspect ratio 
    InputAspectRatio();

    // input clean area
    InputCleanArea();

    // input signal range
    InputSignalRange();

    // input colour spec
    InputColourSpecification();

    // byte align
    ByteAlignInput();
}

void DisplayParamsByteIO::Output()
{
    // output scan format
    OutputScanFormat();

    // output frame rate
    OutputFrameRate();

    // output aspect ratio 
    OutputAspectRatio();

    // output clean area
    OutputCleanArea();

    // output signal range
    OutputSignalRange();

    // output colour spec
    OutputColourSpecification();

    // byte align
    ByteAlignOutput();
}

//-------------------private-----------------------------------------------

void DisplayParamsByteIO::InputAspectRatio()
{
    bool aspect_ratio_flag = InputBit();
    if(!aspect_ratio_flag)
        return;

    // read index value
    int aspect_ratio_index = InputVarLengthUint();
    AspectRatioType aspect_ratio=IntToAspectRatioType(aspect_ratio_index);
    if(aspect_ratio==ASPECT_RATIO_UNDEFINED)
    DIRAC_THROW_EXCEPTION(
                    ERR_INVALID_ASPECT_RATIO,
                    "Dirac does not recognise the specified aspect-ratio",
                    SEVERITY_ACCESSUNIT_ERROR)

    if(aspect_ratio_index!=ASPECT_RATIO_CUSTOM)
    {
        m_src_params.SetAspectRatio(aspect_ratio);
    }
    else
    {
        // read num/denom
        int numerator = InputVarLengthUint();
        int denominator = InputVarLengthUint();
        m_src_params.SetAspectRatio(numerator, denominator);
    }

}

void DisplayParamsByteIO::InputCleanArea()
{
    bool clean_area_flag = InputBit();
    if(!clean_area_flag)
        return;

    m_src_params.SetCleanWidth( InputVarLengthUint() );
    m_src_params.SetCleanHeight( InputVarLengthUint() );
    m_src_params.SetLeftOffset( InputVarLengthUint() );
    m_src_params.SetTopOffset( InputVarLengthUint() );
}

void DisplayParamsByteIO::InputColourMatrix()
{
    bool colour_matrix_flag = InputBit();
    if(!colour_matrix_flag)
        return;

    // read index value
    int colour_matrix_index = InputVarLengthUint();
    m_src_params.SetColourMatrixIndex(colour_matrix_index);
}

void DisplayParamsByteIO::InputColourPrimaries()
{
    bool colour_primaries_flag = InputBit();
    if(!colour_primaries_flag)
        return;

    // read index value
    int colour_primaries_index = InputVarLengthUint();
    m_src_params.SetColourPrimariesIndex(colour_primaries_index);
}

void DisplayParamsByteIO::InputColourSpecification()
{
    bool colour_spec_flag = InputBit();
    if(!colour_spec_flag)
        return;

    // read index value
    int colour_spec_index = InputVarLengthUint();
    if(colour_spec_index!=0)
    {
        m_src_params.SetColourSpecification( colour_spec_index );
    }
    else 
    {
        InputColourPrimaries();
        InputColourMatrix();
        InputTransferFunction();
    }
}

void DisplayParamsByteIO::InputFrameRate()
{
    bool fr_flag = InputBit();
    if(!fr_flag)
        return;

    int frame_rate_index = InputVarLengthUint();
    FrameRateType frame_rate=IntToFrameRateType(frame_rate_index);
    if(frame_rate==FRAMERATE_UNDEFINED)
        DIRAC_THROW_EXCEPTION(
                    ERR_INVALID_FRAME_RATE,
                    "Dirac does not recognise the specified frame-rate",
                    SEVERITY_ACCESSUNIT_ERROR)

    if(frame_rate_index!=FRAMERATE_CUSTOM)
    {
        m_src_params.SetFrameRate(frame_rate);
    }
    else
    {
        // read num/denom
        int numerator = InputVarLengthUint();
        int denominator = InputVarLengthUint();
        m_src_params.SetFrameRate(numerator, denominator);
    }
}

void DisplayParamsByteIO::InputScanFormat()
{
    bool scan_flag = InputBit();
    if(!scan_flag)
        return;

    m_src_params.SetInterlace(InputBit());

    if (!m_src_params.Interlace())
        return;

    // read field dominance flag
    bool field_flag = InputBit();
    if(field_flag)
        m_src_params.SetTopFieldFirst(InputBit());

    // read field interleaving flag
    bool field_interleave_flag = InputBit();
    if(field_interleave_flag)
        m_src_params.SetSequentialFields(InputBit());
}

void DisplayParamsByteIO::InputSignalRange()
{
    bool signal_range_flag = InputBit();
    if(!signal_range_flag)
        return;

    // read index value
    int signal_range_index = InputVarLengthUint();
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
        m_src_params.SetLumaOffset( InputVarLengthUint() );
        m_src_params.SetLumaExcursion( InputVarLengthUint() );
        // read chroma values
        m_src_params.SetChromaOffset( InputVarLengthUint() );
        m_src_params.SetChromaExcursion( InputVarLengthUint() );
    }
}

void DisplayParamsByteIO::InputTransferFunction()
{
    bool trans_fun_flag = InputBit();
    if(!trans_fun_flag)
        return;

    // read index value
    int trans_fun_index = InputVarLengthUint();
    m_src_params.SetTransferFunctionIndex(trans_fun_index);
}

void DisplayParamsByteIO::OutputAspectRatio()
{
    if (m_src_params.AspectRatioIndex()!= ASPECT_RATIO_CUSTOM
        && m_src_params.AspectRatioIndex() == m_default_src_params.AspectRatioIndex())
    {
        // default frame rate index
        OutputBit(0);
        return;
    }
    // Non-defaults
       OutputBit(1);

    // Frame rate index
    OutputVarLengthUint(m_src_params.AspectRatioIndex());
    
    if (!m_src_params.AspectRatioIndex()) // i,e. custom value
    {
        OutputVarLengthUint(m_src_params.AspectRatio().m_num);
        OutputVarLengthUint(m_src_params.AspectRatio().m_denom);
    }
}


void DisplayParamsByteIO::OutputCleanArea()
{
    if (m_src_params.CleanWidth() != m_default_src_params.CleanWidth() ||
        m_src_params.CleanHeight() != m_default_src_params.CleanHeight() ||
        m_src_params.LeftOffset() != m_default_src_params.LeftOffset() ||
        m_src_params.TopOffset() != m_default_src_params.TopOffset())
    {
        OutputBit(1); // non-default value
        OutputVarLengthUint(m_src_params.CleanWidth());
        OutputVarLengthUint(m_src_params.CleanHeight());
        OutputVarLengthUint(m_src_params.LeftOffset());
        OutputVarLengthUint(m_src_params.TopOffset());
    }
    else
        OutputBit(0); // default value
}

void DisplayParamsByteIO::OutputColourSpecification()
{
    if (m_src_params.ColourSpecificationIndex() &&
        m_src_params.ColourSpecificationIndex() == 
        m_default_src_params.ColourSpecificationIndex())
    {
        // default colour specification
        OutputBit(0);
        return;
    }

    // Non-defaults
       OutputBit(1);
    // Output Colour specification index
    OutputVarLengthUint(m_src_params.ColourSpecificationIndex());

    if (!m_src_params.ColourSpecificationIndex()) // i,e, custom values
    {
        // Output Colour Primaries
        if (m_src_params.ColourPrimariesIndex() == m_default_src_params.ColourPrimariesIndex())
        {
            // default value
            OutputBit(0);
        }
        else
        {
            OutputBit(1);
            OutputVarLengthUint(m_src_params.ColourPrimariesIndex());
        }

        // Output Colour Matrix
        if (m_src_params.ColourMatrixIndex() == m_default_src_params.ColourMatrixIndex())
        {
            // default value
            OutputBit(0);
        }
        else
        {
            OutputBit(1);
            OutputVarLengthUint(m_src_params.ColourMatrixIndex());
        }

        // Output TransferFunction
        if (m_src_params.TransferFunctionIndex() == m_default_src_params.TransferFunctionIndex())
        {
            // default value
            OutputBit(0);
        }
        else
        {
            OutputBit(1);
            OutputVarLengthUint(m_src_params.TransferFunctionIndex());
        }
    }
}

void DisplayParamsByteIO::OutputFrameRate()
{
    if (m_src_params.FrameRateIndex()!=FRAMERATE_CUSTOM
        && m_src_params.FrameRateIndex() == m_default_src_params.FrameRateIndex())
    {
        // default frame rate index
        OutputBit(0);
        return;
    }
    // Non-defaults
       OutputBit(1);

    // Frame rate index
    OutputVarLengthUint(m_src_params.FrameRateIndex());
    
    if (!m_src_params.FrameRateIndex()) // i,e. custom value
    {
        OutputVarLengthUint(m_src_params.FrameRate().m_num);
        OutputVarLengthUint(m_src_params.FrameRate().m_denom);
    }
}

void DisplayParamsByteIO::OutputScanFormat()
{
    // output 'is default' flag
    bool not_interlace_default =  m_src_params.Interlace()!=m_default_src_params.Interlace();
    
    OutputBit(not_interlace_default);

    if(!not_interlace_default)
        return;

    // output interlace value
    OutputBit(m_src_params.Interlace());

    if(!m_src_params.Interlace())
        return;

    // output field dominance flag
    bool not_field_dom_default =  m_src_params.TopFieldFirst()!=m_default_src_params.TopFieldFirst();
    
    OutputBit(not_field_dom_default);

    if(not_field_dom_default)
    {
        // output top field value
        OutputBit(m_src_params.TopFieldFirst());
    }
    
    // output field interleaving flag
    bool not_field_seq_default =  m_src_params.SequentialFields()!=m_default_src_params.SequentialFields();
    
    OutputBit(not_field_seq_default);

    if(not_field_seq_default)
    {
        // output sequential fields value
        OutputBit(m_src_params.SequentialFields());
    }
}


void DisplayParamsByteIO::OutputSignalRange()
{
    if (m_src_params.SignalRangeIndex()!=SIGNAL_RANGE_CUSTOM && 
        m_src_params.SignalRangeIndex() == m_default_src_params.SignalRangeIndex())
    {
        // defaults
        OutputBit(0);
        return;
    }
    
    // Non-defaults
       OutputBit(1);
    // Output Signal Range Index
    OutputVarLengthUint(m_src_params.SignalRangeIndex());

    if (!m_src_params.SignalRangeIndex()) // i.e. custom values
    {
        OutputVarLengthUint(m_src_params.LumaOffset());
        OutputVarLengthUint(m_src_params.LumaExcursion());
        OutputVarLengthUint(m_src_params.ChromaOffset());
        OutputVarLengthUint(m_src_params.ChromaExcursion());
    }
}
