/*****************************************************************
|
|    AP4 - tfhd Atoms 
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
#include "Ap4TfhdAtom.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_TfhdAtom)

/*----------------------------------------------------------------------
|   AP4_TfhdAtom::Create
+---------------------------------------------------------------------*/
AP4_TfhdAtom*
AP4_TfhdAtom::Create(AP4_Size size, AP4_ByteStream& stream)
{
    AP4_UI32 version;
    AP4_UI32 flags;
    if (AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if (version > 0) return NULL;
    if (size != ComputeSize(flags)) return NULL;
    return new AP4_TfhdAtom(size, version, flags, stream);
}

/*----------------------------------------------------------------------
|   AP4_TfhdAtom::ComputeSize
+---------------------------------------------------------------------*/
AP4_UI32 
AP4_TfhdAtom::ComputeSize(AP4_UI32 flags)
{
    AP4_UI32 size = AP4_FULL_ATOM_HEADER_SIZE+4;
    if (flags & AP4_TFHD_FLAG_BASE_DATA_OFFSET_PRESENT)         size += 8;
    if (flags & AP4_TFHD_FLAG_SAMPLE_DESCRIPTION_INDEX_PRESENT) size += 4;
    if (flags & AP4_TFHD_FLAG_DEFAULT_SAMPLE_DURATION_PRESENT)  size += 4;
    if (flags & AP4_TFHD_FLAG_DEFAULT_SAMPLE_SIZE_PRESENT)      size += 4;
    if (flags & AP4_TFHD_FLAG_DEFAULT_SAMPLE_FLAGS_PRESENT)     size += 4;
    return size;
}

/*----------------------------------------------------------------------
|   AP4_TfhdAtom::AP4_TfhdAtom
+---------------------------------------------------------------------*/
AP4_TfhdAtom::AP4_TfhdAtom(AP4_UI32 flags, 
                           AP4_UI32 track_id,
                           AP4_UI64 base_data_offset,
                           AP4_UI32 sample_description_index,
                           AP4_UI32 default_sample_duration,
                           AP4_UI32 default_sample_size,
                           AP4_UI32 default_sample_flags) :
    AP4_Atom(AP4_ATOM_TYPE_TFHD, ComputeSize(flags), 0, flags),
    m_TrackId(track_id),
    m_BaseDataOffset(base_data_offset),
    m_SampleDescriptionIndex(sample_description_index),
    m_DefaultSampleDuration(default_sample_duration),
    m_DefaultSampleSize(default_sample_size),
    m_DefaultSampleFlags(default_sample_flags)
{
}

/*----------------------------------------------------------------------
|   AP4_TfhdAtom::AP4_TfhdAtom
+---------------------------------------------------------------------*/
AP4_TfhdAtom::AP4_TfhdAtom(AP4_UI32        size, 
                           AP4_UI32        version,
                           AP4_UI32        flags,
                           AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_TFHD, size, version, flags)
{
    stream.ReadUI32(m_TrackId);
    if (flags & AP4_TFHD_FLAG_BASE_DATA_OFFSET_PRESENT) {
        stream.ReadUI64(m_BaseDataOffset);
    }
    if (flags & AP4_TFHD_FLAG_SAMPLE_DESCRIPTION_INDEX_PRESENT) {
        stream.ReadUI32(m_SampleDescriptionIndex);
    }
    if (flags & AP4_TFHD_FLAG_DEFAULT_SAMPLE_DURATION_PRESENT) {
        stream.ReadUI32(m_DefaultSampleDuration);
    }
    if (flags & AP4_TFHD_FLAG_DEFAULT_SAMPLE_SIZE_PRESENT) {
        stream.ReadUI32(m_DefaultSampleSize);
    }
    if (flags & AP4_TFHD_FLAG_DEFAULT_SAMPLE_FLAGS_PRESENT) {
        stream.ReadUI32(m_DefaultSampleFlags);
    }
}

/*----------------------------------------------------------------------
|   AP4_TfhdAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_TfhdAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;
    
    result = stream.WriteUI32(m_TrackId);
    if (AP4_FAILED(result)) return result;
    if (m_Flags & AP4_TFHD_FLAG_BASE_DATA_OFFSET_PRESENT) {
        result = stream.WriteUI64(m_BaseDataOffset);
        if (AP4_FAILED(result)) return result;
    }
    if (m_Flags & AP4_TFHD_FLAG_SAMPLE_DESCRIPTION_INDEX_PRESENT) {
        result = stream.WriteUI32(m_SampleDescriptionIndex);
        if (AP4_FAILED(result)) return result;
    }
    if (m_Flags & AP4_TFHD_FLAG_DEFAULT_SAMPLE_DURATION_PRESENT) {
        stream.WriteUI32(m_DefaultSampleDuration);
        if (AP4_FAILED(result)) return result;
    }
    if (m_Flags & AP4_TFHD_FLAG_DEFAULT_SAMPLE_SIZE_PRESENT) {
        stream.WriteUI32(m_DefaultSampleSize);
        if (AP4_FAILED(result)) return result;
    }
    if (m_Flags & AP4_TFHD_FLAG_DEFAULT_SAMPLE_FLAGS_PRESENT) {
        stream.WriteUI32(m_DefaultSampleFlags);
        if (AP4_FAILED(result)) return result;
    }
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_TfhdAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_TfhdAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("track ID", m_TrackId);
    if (m_Flags & AP4_TFHD_FLAG_BASE_DATA_OFFSET_PRESENT) {
        inspector.AddField("base data offset", m_BaseDataOffset);
    }
    if (m_Flags & AP4_TFHD_FLAG_SAMPLE_DESCRIPTION_INDEX_PRESENT) {
        inspector.AddField("sample description index", m_SampleDescriptionIndex);
    }
    if (m_Flags & AP4_TFHD_FLAG_DEFAULT_SAMPLE_DURATION_PRESENT) {
        inspector.AddField("default sample duration", m_DefaultSampleDuration);
    }
    if (m_Flags & AP4_TFHD_FLAG_DEFAULT_SAMPLE_SIZE_PRESENT) {
        inspector.AddField("default sample size", m_DefaultSampleSize);
    }
    if (m_Flags & AP4_TFHD_FLAG_DEFAULT_SAMPLE_FLAGS_PRESENT) {
        inspector.AddField("default sample flags", m_DefaultSampleFlags, AP4_AtomInspector::HINT_HEX );
    }

    return AP4_SUCCESS;
}
