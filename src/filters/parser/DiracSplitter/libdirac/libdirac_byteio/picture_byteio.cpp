/* ***** BEGIN LICENSE BLOCK *****
*
* $Id: picture_byteio.cpp,v 1.3 2008/08/14 01:58:39 asuraparaju Exp $ $Name:  $
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

#include "picture_byteio.h"
#include <libdirac_common/dirac_exception.h>

using namespace dirac;
using namespace std;

const unsigned int PP_PICTURE_NUM_SIZE = 4;

const int CODE_ONE_REF_BIT =  0;
const int CODE_TWO_REF_BIT = 1;
const int CODE_REF_PICTURE_BIT = 2;
const int CODE_PUTYPE_1_BIT = 3;
const int CODE_VLC_ENTROPY_CODING_BIT = 6;

// maximum number of refs allowed
const unsigned int MAX_NUM_REFS = 2;

PictureByteIO::PictureByteIO(PictureParams& frame_params,
                         int frame_num) :
ParseUnitByteIO(),
m_frame_params(frame_params),
m_frame_num(frame_num),
m_mv_data(0),
m_transform_data(0)
{
   
}

PictureByteIO::PictureByteIO(PictureParams& frame_params,
                         const ParseUnitByteIO& parseunit_byteio ):
ParseUnitByteIO(parseunit_byteio),
m_frame_params(frame_params),
m_frame_num(0),
m_mv_data(0),
m_transform_data(0)
{
   
}


PictureByteIO::~PictureByteIO()
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

void PictureByteIO::CollateByteStats(DiracByteStats& dirac_byte_stats)
{
    if(m_mv_data)
        m_mv_data->CollateByteStats(dirac_byte_stats);
    if(m_transform_data)
        m_transform_data->CollateByteStats(dirac_byte_stats);

    ParseUnitByteIO::CollateByteStats(dirac_byte_stats);
}

bool PictureByteIO::Input()
{
    // set picture type
    SetPictureType();
    SetReferenceType();
    SetEntropyCodingFlag();

    // Use of VLC for entropy coding is supported for
    // intra frames only
    if (m_frame_params.GetPictureType() == INTER_PICTURE && 
        m_frame_params.UsingAC() == false)
    {
        DIRAC_THROW_EXCEPTION(
                    ERR_UNSUPPORTED_STREAM_DATA,
                    "VLC codes for entropy coding of coefficient data supported for Intra frames only",
                    SEVERITY_PICTURE_ERROR);
    }

    // input picture number
    m_frame_num = ReadUintLit(PP_PICTURE_NUM_SIZE);
    m_frame_params.SetPictureNum(m_frame_num);

    // input reference Picture numbers
    InputReferencePictures();
    
    // input retired Picture numbers list
    m_frame_params.SetRetiredPictureNum(-1);
    if (IsRef())
        InputRetiredPicture();

    // byte align
    ByteAlignInput();

    return true;
}

const string PictureByteIO::GetBytes() 
{
    // Write mv data
    if(m_frame_params.PicSort().IsInter() && m_mv_data)
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

int PictureByteIO::GetSize() const
{
    int size = 0;
    if (m_mv_data)
        size += m_mv_data->GetSize();
    if (m_transform_data)
        size += m_transform_data->GetSize();

    //std::cerr << "Picture Header Size=" << ByteIO::GetSize();
    //std::cerr << "Data Size=" << size << std::endl;

    return size+ParseUnitByteIO::GetSize()+ByteIO::GetSize();
}

void PictureByteIO::Output()
{
    // output picture number
    WriteUintLit(m_frame_num, PP_PICTURE_NUM_SIZE);

    if(m_frame_params.GetPictureType()==INTER_PICTURE)
    {
        // output reference picture numbers
        const std::vector<int>& refs = m_frame_params.Refs();
        for(size_t i=0; i < refs.size() && i < MAX_NUM_REFS; ++i)
            WriteSint(refs[i] - m_frame_num);
    }

    // output retired picture
    ASSERTM (m_frame_params.GetReferenceType() == REFERENCE_PICTURE || m_frame_params.RetiredPictureNum() == -1, "Only Reference frames can retire frames");
    if(m_frame_params.GetReferenceType() == REFERENCE_PICTURE)
    {
        if (m_frame_params.RetiredPictureNum() == -1)
            WriteSint(0);
        else
        {
            WriteSint (m_frame_params.RetiredPictureNum() - m_frame_num);
        }
    }
    // byte align output
    ByteAlignOutput();

}


//-------------private-------------------------------------------------------
 
unsigned char PictureByteIO::CalcParseCode() const
{
    unsigned char code = 0;

    int num_refs = m_frame_params.Refs().size();

    if(m_frame_params.GetPictureType()==INTER_PICTURE)
    {
        // set number of refs
        if(num_refs == 1)
            SetBit(code, CODE_ONE_REF_BIT);
        if(num_refs > 1)
            SetBit(code, CODE_TWO_REF_BIT);
    }
    // set ref type
    if(m_frame_params.GetReferenceType()==REFERENCE_PICTURE)
        SetBit(code, CODE_REF_PICTURE_BIT);

    // Set parse unit type
    SetBit(code, CODE_PUTYPE_1_BIT);

    // Set Entropy Coding type
    if (!m_frame_params.UsingAC())
    {
        SetBit(code, CODE_VLC_ENTROPY_CODING_BIT);
    }
    return code;

    
}

void PictureByteIO::InputReferencePictures() 
{
    // get number of frames referred to
   int ref_count = NumRefs();

   // get the number of these frames
    vector<int>& refs = m_frame_params.Refs();
    refs.resize(ref_count);
    for(int i=0; i < ref_count; ++i)
        refs[i]=m_frame_num+ReadSint();
}
    
void PictureByteIO::InputRetiredPicture() 
{
    TESTM(IsRef(), "Retired Picture offset only set for Reference Frames");

    // input retired picture offset
    int offset = ReadSint();
    // input retired frames
    if (offset)
    {
        m_frame_params.SetRetiredPictureNum(m_frame_num + offset);
    }
}

void PictureByteIO::SetPictureType()
{
    if(IsIntra())
        m_frame_params.SetPictureType(INTRA_PICTURE);
    else if(IsInter())
        m_frame_params.SetPictureType(INTER_PICTURE);

}

void PictureByteIO::SetReferenceType()
{
    if(IsRef())
        m_frame_params.SetReferenceType(REFERENCE_PICTURE);
    else if(IsNonRef())
        m_frame_params.SetReferenceType(NON_REFERENCE_PICTURE);

}

void PictureByteIO::SetEntropyCodingFlag()
{
    m_frame_params.SetUsingAC(IsUsingAC());
}
