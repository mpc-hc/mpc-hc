/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: mvdata_byteio.cpp,v 1.13 2008/08/27 00:17:10 asuraparaju Exp $ $Name:  $
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
*                 Andrew Kennedy
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

#include <libdirac_byteio/mvdata_byteio.h>
#include <libdirac_common/video_format_defaults.h>
#include <libdirac_common/dirac_exception.h>

using namespace dirac;

MvDataByteIO::MvDataByteIO(PictureParams& pparams,
                                 PicturePredParams& picpredparams):
ByteIO(),
m_pparams(pparams),
m_picpredparams(picpredparams),
m_splitmode_data(),
m_predmode_data(),
m_mv1hblock_data(),
m_mv1vblock_data(),
m_mv2hblock_data(),
m_mv2vblock_data(),
m_ydcblock_data(),
m_udcblock_data(),
m_vdcblock_data()
{}

MvDataByteIO::MvDataByteIO(ByteIO &byte_io, PictureParams& pparams,
                                 PicturePredParams& picpredparams):
ByteIO(byte_io),
m_pparams(pparams),
m_picpredparams(picpredparams),
m_splitmode_data(byte_io),
m_predmode_data(byte_io),
m_mv1hblock_data(byte_io),
m_mv1vblock_data(byte_io),
m_mv2hblock_data(byte_io),
m_mv2vblock_data(byte_io),
m_ydcblock_data(byte_io),
m_udcblock_data(byte_io),
m_vdcblock_data(byte_io)
{}

MvDataByteIO::~MvDataByteIO()
{}

void MvDataByteIO::CollateByteStats(DiracByteStats& dirac_byte_stats)
{
    // set number of MV bytes
    dirac_byte_stats.SetByteCount(STAT_MV_BYTE_COUNT,
                                  GetSize());
}

int MvDataByteIO::GetSize() const
{
    if (m_pparams.NumRefs()==2)
        return ByteIO::GetSize() +
               m_splitmode_data.GetSize()+
               m_predmode_data.GetSize()+
               m_mv1hblock_data.GetSize()+
               m_mv1vblock_data.GetSize()+
               m_mv2hblock_data.GetSize()+
               m_mv2vblock_data.GetSize()+
               m_ydcblock_data.GetSize()+
               m_udcblock_data.GetSize()+
               m_vdcblock_data.GetSize();
    else
        return ByteIO::GetSize() +
               m_splitmode_data.GetSize()+
               m_predmode_data.GetSize()+
               m_mv1hblock_data.GetSize()+
               m_mv1vblock_data.GetSize()+
               m_ydcblock_data.GetSize()+
               m_udcblock_data.GetSize()+
               m_vdcblock_data.GetSize();
}

const std::string MvDataByteIO::GetBytes()
{
    //Output header and block data
    if (m_pparams.NumRefs()==2 )
        return ByteIO::GetBytes() +
               m_splitmode_data.GetBytes()+
               m_predmode_data.GetBytes()+
               m_mv1hblock_data.GetBytes()+
               m_mv1vblock_data.GetBytes()+
               m_mv2hblock_data.GetBytes()+
               m_mv2vblock_data.GetBytes()+
               m_ydcblock_data.GetBytes()+
               m_udcblock_data.GetBytes()+
               m_vdcblock_data.GetBytes();
    else
        return ByteIO::GetBytes() +
               m_splitmode_data.GetBytes()+
               m_predmode_data.GetBytes()+
               m_mv1hblock_data.GetBytes()+
               m_mv1vblock_data.GetBytes()+
               m_ydcblock_data.GetBytes()+
               m_udcblock_data.GetBytes()+
               m_vdcblock_data.GetBytes();
}

void MvDataByteIO::Input()
{
    // Byte Alignment
    ByteAlignInput();

    // Input Block Params
    InputBlockParams();

    // Input Motion Vector Precision
    InputMVPrecision();

    // Input chroma
    InputGlobalMotionParams();

    // Input Picture Prediction mode
    InputFramePredictionMode();

    // Input picture weights
    InputPictureWeights();

    // Byte Alignment
    ByteAlignInput();
}

void MvDataByteIO::Output()
{
    // Output Block Params
    OutputBlockParams();

    // Output Motion Vector Precision
    OutputMVPrecision();

    // output global motion
    OutputGlobalMotionParams();

    // output picture prediction mode
    OutputFramePredictionMode();

    // output picture weights
    OutputPictureWeights();

    // Byte Align
    ByteAlignOutput();
}

//-------------private---------------------------------------------------------------

void MvDataByteIO::OutputBlockParams()
{
    const OLBParams& olb_params = m_picpredparams.LumaBParams(2);

    // output custom block params flag
    unsigned int pidx = BlockParametersIndex(olb_params);
    WriteUint(pidx);
    if (pidx == 0) // custom block params
    {
        // output Xblen
        WriteUint(olb_params.Xblen());
        // output Yblen
        WriteUint(olb_params.Yblen());
        // output Xbsep
        WriteUint(olb_params.Xbsep());
        // output Ybsep
        WriteUint(olb_params.Ybsep());
    }
}

void MvDataByteIO::InputBlockParams()
{
    OLBParams olb_params;

    unsigned int p_idx = ReadUint();
    if (p_idx == 0)
    {
        // Input Xblen
        olb_params.SetXblen(ReadUint());
        // Input Yblen
        olb_params.SetYblen(ReadUint());
        // Input Xbsep
        olb_params.SetXbsep(ReadUint());
        // Input Ybsep
        olb_params.SetYbsep(ReadUint());
    }
    else
        SetDefaultBlockParameters (olb_params, p_idx);

    m_picpredparams.SetLumaBlockParams(olb_params);
}

void MvDataByteIO::OutputMVPrecision()
{
    // Output Motion vector precision
    WriteUint(m_picpredparams.MVPrecision());
}

void MvDataByteIO::InputMVPrecision()
{
    // Input Motion vector precision
    MVPrecisionType mv_prec = IntToMVPrecisionType(ReadUint());
    if(mv_prec==MV_PRECISION_UNDEFINED)
        DIRAC_THROW_EXCEPTION(
                ERR_INVALID_MOTION_VECTOR_PRECISION,
                "Dirac does not recognise the specified MV precision",
                SEVERITY_PICTURE_ERROR)

    m_picpredparams.SetMVPrecision(mv_prec);
}

void MvDataByteIO::OutputGlobalMotionParams()
{
    // Always setting global motion to false
    // NOTE: FIXME - output actual global motion params in future
    // Using Global motion flag
    WriteBit(false);
}

void MvDataByteIO::InputGlobalMotionParams()
{
    // Always setting global motion to false
    // Using Global motion flag
    if (ReadBool())
    {
        m_picpredparams.SetUsingGlobalMotion(true);

        // NOTE: FIXME - input actual global motion params in future
        DIRAC_THROW_EXCEPTION(
                    ERR_UNSUPPORTED_STREAM_DATA,
                    "Cannot handle global motion parameters",
                    SEVERITY_PICTURE_ERROR)
    }
    else
        m_picpredparams.SetUsingGlobalMotion(false);
}

void MvDataByteIO::OutputFramePredictionMode()
{
    //  TODO: Output default picture prediction mode index until other
    //  modes are supported.
    WriteUint(0);
}

void MvDataByteIO::InputFramePredictionMode()
{
    // TODO - need to process this field when alternative prediction modes
    // become available.
    unsigned int frame_pred_mode = ReadUint();
    if (frame_pred_mode != 0)
    {
        DIRAC_THROW_EXCEPTION(
            ERR_UNSUPPORTED_STREAM_DATA,
            "Non-default Picture Prediction Mode not supported",
            SEVERITY_PICTURE_ERROR);
    }
}


void MvDataByteIO::OutputPictureWeights()
{
    // Output default weights flags
    if (m_picpredparams.PictureWeightsBits() != 1 ||
        m_picpredparams.Ref1Weight() !=  1 ||
        (m_pparams.Refs().size() > 1 && m_picpredparams.Ref2Weight() != 1) )
    {
         WriteBit(true);
        // Output weight precision bits
        WriteUint(m_picpredparams.PictureWeightsBits());
        // Output Ref1 weight
        WriteSint(m_picpredparams.Ref1Weight());
        if (m_pparams.Refs().size() > 1)
        {
            // Output Ref1 weight
            WriteSint(m_picpredparams.Ref2Weight());
        }
    }
    else
    {
           WriteBit(false);
    }
}

void MvDataByteIO::InputPictureWeights()
{
    if (ReadBool())
    {
        m_picpredparams.SetPictureWeightsPrecision(ReadUint());
        m_picpredparams.SetRef1Weight(ReadSint());
        if (m_pparams.Refs().size() > 1)
            m_picpredparams.SetRef2Weight(ReadSint());
        else
            m_picpredparams.SetRef2Weight(0);
    }
    else
    {
        m_picpredparams.SetPictureWeightsPrecision(1);
        m_picpredparams.SetRef1Weight(1);
        m_picpredparams.SetRef2Weight(1);
    }
}
