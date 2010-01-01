/*****************************************************************
|
|    AP4 - avcC Atoms 
|
|    Copyright 2002-2008 Axiomatic Systems, LLC
|
|
|    This file is part of Bento4/AP4 (MP4 Atom Processing Library).
|
|    Unless you have obtained Bento4 under a difference license,
|    this version of Bento4 is Bento4|GPL.
|    Bento4|GPL is free software; you can redistribute it and/or modify
|    it under the terms of the GNU General Public License as published by
|    the Free Software Foundation; either version 2, or (at your option)
|    any later version.
|
|    Bento4|GPL is distributed in the hope that it will be useful,
|    but WITHOUT ANY WARRANTY; without even the implied warranty of
|    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|    GNU General Public License for more details.
|
|    You should have received a copy of the GNU General Public License
|    along with Bento4|GPL; see the file COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
****************************************************************/

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4AvccAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"
#include "Ap4Types.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_AvccAtom)

/*----------------------------------------------------------------------
|   AP4_AvccAtom::GetProfileName
+---------------------------------------------------------------------*/
const char*
AP4_AvccAtom::GetProfileName(AP4_UI08 profile)
{
    switch (profile) {
        case AP4_AVC_PROFILE_BASELINE: return "Baseline";   
        case AP4_AVC_PROFILE_MAIN:     return "Main";
        case AP4_AVC_PROFILE_EXTENDED: return "Extended";
        case AP4_AVC_PROFILE_HIGH:     return "High";
        case AP4_AVC_PROFILE_HIGH_10:  return "High 10";
        case AP4_AVC_PROFILE_HIGH_422: return "High 4:2:2";
        case AP4_AVC_PROFILE_HIGH_444: return "High 4:4:4";
    }

    return NULL;
}

/*----------------------------------------------------------------------
|   AP4_AvccAtom::AP4_AvccAtom
+---------------------------------------------------------------------*/
AP4_AvccAtom::AP4_AvccAtom() :
    AP4_Atom(AP4_ATOM_TYPE_AVCC, AP4_ATOM_HEADER_SIZE+7),
    m_ConfigurationVersion(0),
    m_Profile(0),
    m_Level(0),
    m_ProfileCompatibility(0),
    m_NaluLengthSize(0)
{
}

/*----------------------------------------------------------------------
|   AP4_AvccAtom::AP4_AvccAtom
+---------------------------------------------------------------------*/
AP4_AvccAtom::AP4_AvccAtom(const AP4_AvccAtom& other) :
    AP4_Atom(AP4_ATOM_TYPE_AVCC, other.m_Size32),
    m_ConfigurationVersion (other.m_ConfigurationVersion),
    m_Profile(other.m_Profile),
    m_Level(other.m_Level),
    m_ProfileCompatibility(other.m_ProfileCompatibility),
    m_NaluLengthSize(other.m_NaluLengthSize),
    m_RawBytes(other.m_RawBytes)
{
    // deep copy of the parameters
    unsigned int i = 0;
    for (i=0; i<other.m_SequenceParameters.ItemCount(); i++) {
        m_SequenceParameters.Append(other.m_SequenceParameters[i]);
    }
    for (i=0; i<other.m_PictureParameters.ItemCount(); i++) {
        m_PictureParameters.Append(other.m_PictureParameters[i]);
    }    
}

/*----------------------------------------------------------------------
|   AP4_AvccAtom::AP4_AvccAtom
+---------------------------------------------------------------------*/
AP4_AvccAtom::AP4_AvccAtom(AP4_UI32 size, AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_AVCC, size)
{
    // make a copy of our configuration bytes
    AP4_Position start_pos;
    stream.Tell(start_pos);
    m_RawBytes.SetDataSize(size);
    stream.Read(m_RawBytes.UseData(), size);
    stream.Seek(start_pos);

    stream.ReadUI08(m_ConfigurationVersion);
    stream.ReadUI08(m_Profile);
    stream.ReadUI08(m_ProfileCompatibility);
    stream.ReadUI08(m_Level);
    AP4_UI08 length_size_minus_one;
    stream.ReadUI08(length_size_minus_one);
    m_NaluLengthSize = 1+(length_size_minus_one&3);
    AP4_UI08 num_seq_params;
    stream.ReadUI08(num_seq_params);
    num_seq_params &= 31;
    m_SequenceParameters.EnsureCapacity(num_seq_params);
    for (unsigned int i=0; i<num_seq_params; i++) {
        m_SequenceParameters.Append(AP4_DataBuffer());
        AP4_UI16 param_length;
        stream.ReadUI16(param_length);
        m_SequenceParameters[i].SetDataSize(param_length);
        stream.Read(m_SequenceParameters[i].UseData(), param_length);
    }
    AP4_UI08 num_pic_params;
    stream.ReadUI08(num_pic_params);
    for (unsigned int i=0; i<num_pic_params; i++) {
        m_PictureParameters.Append(AP4_DataBuffer());
        AP4_UI16 param_length;
        stream.ReadUI16(param_length);
        m_PictureParameters[i].SetDataSize(param_length);
        stream.Read(m_PictureParameters[i].UseData(), param_length);
    }
}


/*----------------------------------------------------------------------
|   AP4_AvccAtom::AP4_AvccAtom
+---------------------------------------------------------------------*/
AP4_AvccAtom::AP4_AvccAtom(AP4_UI08 config_version, 
                           AP4_UI08 profile, 
                           AP4_UI08 level, 
                           AP4_UI08 profile_compatibility, 
                           AP4_UI08 length_size, 
                           const AP4_Array<AP4_DataBuffer>& sequence_parameters, 
                           const AP4_Array<AP4_DataBuffer>& picture_parameters) :
    AP4_Atom(AP4_ATOM_TYPE_AVCC, AP4_ATOM_HEADER_SIZE+7),
    m_ConfigurationVersion(config_version),
    m_Profile(profile),
    m_Level(level),
    m_ProfileCompatibility(profile_compatibility),
    m_NaluLengthSize(length_size)
{
    // deep copy of the parameters
    unsigned int i = 0;
    for (i=0; i<sequence_parameters.ItemCount(); i++) {
        m_SequenceParameters.Append(sequence_parameters[i]);
        m_Size32 += 2+sequence_parameters[i].GetDataSize();
    }
    for (i=0; i<picture_parameters.ItemCount(); i++) {
        m_PictureParameters.Append(picture_parameters[i]);
        m_Size32 += 2+picture_parameters[i].GetDataSize();
    }    
}


/*----------------------------------------------------------------------
|   AP4_AvccAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_AvccAtom::WriteFields(AP4_ByteStream& stream)
{
    stream.WriteUI08(m_ConfigurationVersion);
    stream.WriteUI08(m_Profile);
    stream.WriteUI08(m_ProfileCompatibility);
    stream.WriteUI08(m_Level);
    AP4_UI08 length_size_minus_one = 0xFC;
    if (m_NaluLengthSize >= 1 && m_NaluLengthSize <= 4) {
        length_size_minus_one |= m_NaluLengthSize-1;
    }
    stream.WriteUI08(length_size_minus_one);
    m_NaluLengthSize = 1+(length_size_minus_one&3);
    AP4_UI08 num_seq_params = (AP4_UI08)(m_SequenceParameters.ItemCount()&0x31);
    stream.WriteUI08(0xE0 | num_seq_params);
    for (unsigned int i=0; i<num_seq_params; i++) {
        AP4_UI16 param_length = (AP4_UI16)m_SequenceParameters[i].GetDataSize();
        stream.WriteUI16(param_length);
        stream.Write(m_SequenceParameters[i].GetData(), param_length);
    }
    AP4_UI08 num_pic_params = (AP4_UI08)m_PictureParameters.ItemCount();
    stream.WriteUI08(num_pic_params);
    for (unsigned int i=0; i<num_pic_params; i++) {
        AP4_UI16 param_length = (AP4_UI16)m_PictureParameters[i].GetDataSize();
        stream.WriteUI16(param_length);
        stream.Write(m_PictureParameters[i].GetData(), param_length);
    }

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_AvccAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_AvccAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("Configuration Version", m_ConfigurationVersion);
    const char* profile_name = GetProfileName(m_Profile);
    if (profile_name) {
        inspector.AddField("Profile", profile_name);
    } else {
        inspector.AddField("Profile", m_Profile);
    }
    inspector.AddField("Profile Compatibility", m_ProfileCompatibility, AP4_AtomInspector::HINT_HEX);
    inspector.AddField("Level", m_Level);
    inspector.AddField("NALU Length Size", m_NaluLengthSize);
    for (unsigned int i=0; i<m_SequenceParameters.ItemCount(); i++) {
        inspector.AddField("Sequence Parameter", m_SequenceParameters[i].GetData(), m_SequenceParameters[i].GetDataSize());
    }
    for (unsigned int i=0; i<m_SequenceParameters.ItemCount(); i++) {
        inspector.AddField("Picture Parameter", m_PictureParameters[i].GetData(), m_PictureParameters[i].GetDataSize());
    }
    return AP4_SUCCESS;
}
