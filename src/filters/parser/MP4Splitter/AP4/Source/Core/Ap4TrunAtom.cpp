/*****************************************************************
|
|    AP4 - trun Atoms 
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
#include "Ap4TrunAtom.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_TrunAtom)

/*----------------------------------------------------------------------
|   AP4_TrunAtom::Create
+---------------------------------------------------------------------*/
AP4_TrunAtom*
AP4_TrunAtom::Create(AP4_Size size, AP4_ByteStream& stream)
{
    AP4_UI32 version;
    AP4_UI32 flags;
    if (AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if (version > 0) return NULL;
    return new AP4_TrunAtom(size, version, flags, stream);
}

/*----------------------------------------------------------------------
|   AP4_TrunAtom::ComputeRecordFieldsCount
+---------------------------------------------------------------------*/
unsigned int 
AP4_TrunAtom::ComputeRecordFieldsCount(AP4_UI32 flags)
{
    // count the number of bits set to 1 in the second byte of the flags
    unsigned int count = 0;
    for (unsigned int i=0; i<8; i++) {
        if (flags & (1<<(i+8))) ++count;
    }
    return count;
}

/*----------------------------------------------------------------------
|   AP4_TrunAtom::ComputeOptionalFieldsCount
+---------------------------------------------------------------------*/
unsigned int 
AP4_TrunAtom::ComputeOptionalFieldsCount(AP4_UI32 flags)
{
    // count the number of bits set to 1 in the LSB of the flags
    unsigned int count = 0;
    for (unsigned int i=0; i<8; i++) {
        if (flags & (1<<i)) ++count;
    }
    return count;
}

/*----------------------------------------------------------------------
|   AP4_TrunAtom::AP4_TrunAtom
+---------------------------------------------------------------------*/
AP4_TrunAtom::AP4_TrunAtom(AP4_UI32 flags, 
                           AP4_SI32 data_offset,
                           AP4_UI32 first_sample_flags) :
    AP4_Atom(AP4_ATOM_TYPE_TRUN, AP4_FULL_ATOM_HEADER_SIZE+4, 0, flags),
    m_DataOffset(data_offset),
    m_FirstSampleFlags(first_sample_flags)
{
    m_Size32 += 4*ComputeOptionalFieldsCount(flags);
}

/*----------------------------------------------------------------------
|   AP4_TrunAtom::AP4_TrunAtom
+---------------------------------------------------------------------*/
AP4_TrunAtom::AP4_TrunAtom(AP4_UI32        size, 
                           AP4_UI32        version,
                           AP4_UI32        flags,
                           AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_TRUN, size, version, flags)
{
    AP4_UI32 sample_count = 0;
    stream.ReadUI32(sample_count);

    // read optional fields
    int optional_fields_count = (int)ComputeOptionalFieldsCount(flags);
    if (flags & AP4_TRUN_FLAG_DATA_OFFSET_PRESENT) {
        AP4_UI32 offset = 0;
        stream.ReadUI32(offset);
        m_DataOffset = (AP4_SI32)offset;
        --optional_fields_count;
    }
    if (flags & AP4_TRUN_FLAG_FIRST_SAMPLE_FLAGS_PRESENT) {
        stream.ReadUI32(m_FirstSampleFlags);
        --optional_fields_count;
    }
    
    // discard unknown optional fields 
    for (int i=0; i<optional_fields_count; i++) {
        AP4_UI32 discard;
        stream.ReadUI32(discard);
    }
    
    int record_fields_count = (int)ComputeRecordFieldsCount(flags);
    m_Entries.SetItemCount(sample_count);
    for (unsigned int i=0; i<sample_count; i++) {
        if (flags & AP4_TRUN_FLAG_SAMPLE_DURATION_PRESENT) {
            stream.ReadUI32(m_Entries[i].sample_duration);
            --record_fields_count;
        }
        if (flags & AP4_TRUN_FLAG_SAMPLE_SIZE_PRESENT) {
            stream.ReadUI32(m_Entries[i].sample_size);
            --record_fields_count;
        }
        if (flags & AP4_TRUN_FLAG_SAMPLE_FLAGS_PRESENT) {
            stream.ReadUI32(m_Entries[i].sample_flags);
            --record_fields_count;
        }
        if (flags & AP4_TRUN_FLAG_SAMPLE_COMPOSITION_TIME_OFFSET_PRESENT) {
            stream.ReadUI32(m_Entries[i].sample_composition_time_offset);
            --record_fields_count;
        }
    
        // skip unknown fields 
        for (int j=0;j<record_fields_count; j++) {
            AP4_UI32 discard;
            stream.ReadUI32(discard);
        }
    }
}

/*----------------------------------------------------------------------
|   AP4_TrunAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_TrunAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;
    
    result = stream.WriteUI32(m_Entries.ItemCount());
    if (AP4_FAILED(result)) return result;
    if (m_Flags & AP4_TRUN_FLAG_DATA_OFFSET_PRESENT) {
        result = stream.WriteUI32((AP4_UI32)m_DataOffset);
        if (AP4_FAILED(result)) return result;
    }
    if (m_Flags & AP4_TRUN_FLAG_FIRST_SAMPLE_FLAGS_PRESENT) {
        result = stream.WriteUI32(m_FirstSampleFlags);
        if (AP4_FAILED(result)) return result;
    }
    AP4_UI32 sample_count = m_Entries.ItemCount();
    for (unsigned int i=0; i<sample_count; i++) {
        if (m_Flags & AP4_TRUN_FLAG_SAMPLE_DURATION_PRESENT) {
            result = stream.WriteUI32(m_Entries[i].sample_duration);
            if (AP4_FAILED(result)) return result;
        }
        if (m_Flags & AP4_TRUN_FLAG_SAMPLE_SIZE_PRESENT) {
            result = stream.WriteUI32(m_Entries[i].sample_size);
            if (AP4_FAILED(result)) return result;
        }
        if (m_Flags & AP4_TRUN_FLAG_SAMPLE_FLAGS_PRESENT) {
            result = stream.WriteUI32(m_Entries[i].sample_flags);
            if (AP4_FAILED(result)) return result;
        }
        if (m_Flags & AP4_TRUN_FLAG_SAMPLE_COMPOSITION_TIME_OFFSET_PRESENT) {
            stream.WriteUI32(m_Entries[i].sample_composition_time_offset);
            if (AP4_FAILED(result)) return result;
        }
    }
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_TrunAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_TrunAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("sample count", m_Entries.ItemCount());
    if (m_Flags & AP4_TRUN_FLAG_DATA_OFFSET_PRESENT) {
        inspector.AddField("data offset", m_DataOffset);
    }
    if (m_Flags & AP4_TRUN_FLAG_FIRST_SAMPLE_FLAGS_PRESENT) {
        inspector.AddField("first sample flags", m_FirstSampleFlags, AP4_AtomInspector::HINT_HEX);
    }
    if (inspector.GetVerbosity() >= 1) {
    AP4_UI32 sample_count = m_Entries.ItemCount();
        for (unsigned int i=0; i<sample_count; i++) {
            char header[32];
            AP4_FormatString(header, sizeof(header), "entry %04d", i);
            char v0[32];
            char v1[32];
            char v2[32];
            char v3[64];
            const char* s0 = "";
            const char* s1 = "";
            const char* s2 = "";
            const char* s3 = "";
            if (m_Flags & AP4_TRUN_FLAG_SAMPLE_DURATION_PRESENT) {
                AP4_FormatString(v0, sizeof(v0), "sample duration:%d", m_Entries[i].sample_duration);
                s0 = v0;
            }
            if (m_Flags & AP4_TRUN_FLAG_SAMPLE_SIZE_PRESENT) {
                AP4_FormatString(v1, sizeof(v1), "sample size:%d", m_Entries[i].sample_size);
                s1 = v1;
            }
            if (m_Flags & AP4_TRUN_FLAG_SAMPLE_FLAGS_PRESENT) {
                AP4_FormatString(v2, sizeof(v2), "sample flags:%x", m_Entries[i].sample_flags);
                s2 = v2;
            }
            if (m_Flags & AP4_TRUN_FLAG_SAMPLE_COMPOSITION_TIME_OFFSET_PRESENT) {
                AP4_FormatString(v3, sizeof(v3), "sample composition time offset:%d", m_Entries[i].sample_composition_time_offset);
                s3 = v3;
            }
            char value[128];
            AP4_FormatString(value, sizeof(value), "%s %s %s %s", s0, s1, s2, s3);
            inspector.AddField(header, value);
        }
    }
    
    return AP4_SUCCESS;
}
