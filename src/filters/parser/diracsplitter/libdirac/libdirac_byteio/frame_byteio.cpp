/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: frame_byteio.cpp,v 1.3 2006/06/05 14:51:05 asuraparaju Exp $ $Name: Dirac_0_7_0 $
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
* Contributor(s): Andrew Kennedy (Original Author),
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

#include "frame_byteio.h"

using namespace dirac;
using namespace std;

const unsigned int PP_FRAME_NUM_SIZE = 4;

const int CODE_ONE_REF_BIT =  0;
const int CODE_TWO_REF_BIT = 1;
const int CODE_REF_FRAME_BIT = 2;
const int CODE_PUTYPE_1_BIT = 3;
const int CODE_PUTYPE_2_BIT = 4;

// maximum number of refs allowed
const unsigned int MAX_NUM_REFS = 2;

// determines frame-type
#define PARSE_CODE_FRAME_TYPE_BITS      (0x03)
#define PARSE_CODE_INTER_FRAME_BITS     (0x14)
#define PARSE_CODE_INTRA_FRAME_BITS     (0x10)   
// determines number of reference frames
#define NUM_REF_FRAMES(byte)         (byte&0x03)
#define IS_INTER_FRAME(byte)         ((NUM_REF_FRAMES(byte))>0)
#define IS_INTRA_FRAME(byte)         ((NUM_REF_FRAMES(byte))==0)
// determines reference type
#define IS_REFERENCE_FRAME(byte)     ((byte&0x04)==0x04)
#define IS_NON_REFERENCE_FRAME(byte) ((byte&0x04)==0x00)




FrameByteIO::FrameByteIO(FrameParams& frame_params,
                         int frame_num,
                         int au_fnum):
ParseUnitByteIO(au_fnum),
m_frame_params(frame_params),
m_frame_num(frame_num),
m_mv_data(0),
m_transform_data(0)
{
   
}

FrameByteIO::FrameByteIO(FrameParams& frame_params,
                         const ParseUnitByteIO& parseunit_byteio,
                         int au_fnum):
ParseUnitByteIO(au_fnum, parseunit_byteio),
m_frame_params(frame_params),
m_frame_num(0),
m_mv_data(0),
m_transform_data(0)
{
   
}


FrameByteIO::~FrameByteIO()
{
    //delete block data
    if (m_mv_data)
    {
        delete m_mv_data;
        m_mv_data = 0;
    }
    if (m_transform_data)
    {
        delete m_transform_data;
        m_transform_data = 0;
    }
}

void FrameByteIO::CollateByteStats(DiracByteStats& dirac_byte_stats)
{
    if(m_mv_data)
        m_mv_data->CollateByteStats(dirac_byte_stats);
    if(m_transform_data)
        m_transform_data->CollateByteStats(dirac_byte_stats);

    ParseUnitByteIO::CollateByteStats(dirac_byte_stats);
}

bool FrameByteIO::Input()
{
    // set frame type
    SetFrameType();
    SetReferenceType();

    // input frame number
    m_frame_num = InputFixedLengthUint(PP_FRAME_NUM_SIZE);
    m_frame_params.SetFrameNum(m_frame_num);

    // input reference frame numbers
    InputReferenceFrames();
    
    // input retired frames
    int val = InputVarLengthUint();
    // input retired frames
    std::vector<int>& retd_list = m_frame_params.RetiredFrames();
    retd_list.resize(val);
    if (val)
    {
        for (size_t i = 0; i < retd_list.size(); ++i)
        {
            int offset = InputVarLengthInt();
            retd_list[i] = m_frame_num + offset;
        }
    }

    // byte align
    ByteAlignInput();

    return true;
}

const string FrameByteIO::GetBytes() 
{
    // Write mv data
    if(m_frame_params.FSort().IsInter() && m_mv_data)
    {
        OutputBytes(m_mv_data->GetBytes());
    }

    // Write transform header
    if (m_transform_data)
    {
        OutputBytes(m_transform_data->GetBytes());
    }
    return ParseUnitByteIO::GetBytes();
}

int FrameByteIO::GetSize() const
{
    int size = 0;
    if (m_mv_data)
        size += m_mv_data->GetSize();
    if (m_transform_data)
        size += m_transform_data->GetSize();

    //std::cerr << "Frame Header Size=" << ByteIO::GetSize();
    //std::cerr << "Data Size=" << size << std::endl;

    return size+ParseUnitByteIO::GetSize()+ByteIO::GetSize();
}

void FrameByteIO::Output()
{
    // output frame number
    OutputFixedLengthUint(m_frame_num, PP_FRAME_NUM_SIZE);

    if(m_frame_params.GetFrameType()==INTER_FRAME)
    {
        // output reference frame numbers
        const std::vector<int>& refs = m_frame_params.Refs();
        for(size_t i=0; i < refs.size() && i < MAX_NUM_REFS; ++i)
            OutputVarLengthInt(refs[i] - m_frame_num);
    }

    // output retired frames
    const std::vector<int>& retd_list = m_frame_params.RetiredFrames();
    OutputVarLengthUint(retd_list.size());
    for (size_t i = 0; i < retd_list.size(); ++i)
    {
        OutputVarLengthInt(retd_list[i] - m_frame_num);
    }

    // byte align output
    ByteAlignOutput();

}


//-------------private-------------------------------------------------------
 
unsigned char FrameByteIO::CalcParseCode() const
{
    unsigned char code = 0;

    int num_refs = m_frame_params.Refs().size();

    if(m_frame_params.GetFrameType()==INTER_FRAME)
    {
        // set number of refs
        if(num_refs == 1)
            SetBit(code, CODE_ONE_REF_BIT);
        if(num_refs > 1)
            SetBit(code, CODE_TWO_REF_BIT);
    }
    // set ref type
    if(m_frame_params.GetReferenceType()==REFERENCE_FRAME)
        SetBit(code, CODE_REF_FRAME_BIT);

    // Set parse unit type
        SetBit(code, CODE_PUTYPE_1_BIT);

    return code;

    
}

void FrameByteIO::InputReferenceFrames() 
{
    // get number of frames referred to
   int ref_count = NUM_REF_FRAMES(GetParseCode());

   // get the number of these frames
    vector<int>& refs = m_frame_params.Refs();
    refs.resize(ref_count);
    for(int i=0; i < ref_count; ++i)
        refs[i]=m_frame_num+InputVarLengthInt();
}

void FrameByteIO::SetFrameType()
{
    if(IS_INTRA_FRAME(GetParseCode()))
        m_frame_params.SetFrameType(INTRA_FRAME);
    else if(IS_INTER_FRAME(GetParseCode()))
        m_frame_params.SetFrameType(INTER_FRAME);

}

void FrameByteIO::SetReferenceType()
{
    if(IS_REFERENCE_FRAME(GetParseCode()))
        m_frame_params.SetReferenceType(REFERENCE_FRAME);
    else if(IS_NON_REFERENCE_FRAME(GetParseCode()))
        m_frame_params.SetReferenceType(NON_REFERENCE_FRAME);

}
