/*****************************************************************
|
|    AP4 - tfra Atoms 
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
#include "Ap4TfraAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   AP4_TfraAtom::Create
+---------------------------------------------------------------------*/
AP4_TfraAtom* 
AP4_TfraAtom::Create(AP4_Size size, AP4_ByteStream& stream)
{
    AP4_UI32 version = 0;
    AP4_UI32 flags   = 0;
    AP4_Result result = ReadFullHeader(stream, version, flags);
    if (AP4_FAILED(result)) return NULL;
    if (version > 1) return NULL;
    return new AP4_TfraAtom(size, version, flags, stream);
}

/*----------------------------------------------------------------------
|   AP4_TfraAtom::AP4_TfraAtom
+---------------------------------------------------------------------*/
AP4_TfraAtom::AP4_TfraAtom() :
    AP4_Atom(AP4_ATOM_TYPE_TFRA, AP4_FULL_ATOM_HEADER_SIZE+4+4+4)
{
}

/*----------------------------------------------------------------------
|   AP4_TfraAtom::AP4_TfraAtom
+---------------------------------------------------------------------*/
AP4_TfraAtom::AP4_TfraAtom(AP4_UI32        size, 
                           AP4_UI32        version, 
                           AP4_UI32        flags, 
                           AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_TFRA, size, version, flags)
{
    stream.ReadUI32(m_TrackId);
    AP4_UI32 fields = 0;
    stream.ReadUI32(fields);
    m_LengthSizeOfTrafNumber   = (fields>>4)&3;
    m_LengthSizeOfTrunNumber   = (fields>>2)&3;
    m_LengthSizeOfSampleNumber = (fields   )&3;
    AP4_UI32 entry_count = 0;
    stream.ReadUI32(entry_count);
    m_Entries.SetItemCount(entry_count);
    for (unsigned int i=0; i<entry_count; i++) {
        if (version == 1) {
            stream.ReadUI64(m_Entries[i].m_Time);
            stream.ReadUI64(m_Entries[i].m_MoofOffset);
        } else {
            AP4_UI32 time = 0;
            AP4_UI32 moof_offset = 0;
            stream.ReadUI32(time);
            stream.ReadUI32(moof_offset);
            m_Entries[i].m_Time       = time;
            m_Entries[i].m_MoofOffset = moof_offset;
        }
        switch (m_LengthSizeOfTrafNumber) {
            case 0: {
                AP4_UI08 traf_number;
                stream.ReadUI08(traf_number);
                m_Entries[i].m_TrafNumber = traf_number;
                break;
            }
            case 1: {
                AP4_UI16 traf_number;
                stream.ReadUI16(traf_number);
                m_Entries[i].m_TrafNumber = traf_number;
                break;
            }
            case 2: {
                AP4_UI32 traf_number;
                stream.ReadUI24(traf_number);
                m_Entries[i].m_TrafNumber = traf_number;
                break;
            }
            case 3: {
                AP4_UI32 traf_number;
                stream.ReadUI32(traf_number);
                m_Entries[i].m_TrafNumber = traf_number;
                break;
            }
        }
        
        switch (m_LengthSizeOfTrunNumber) {
            case 0: {
                AP4_UI08 trun_number;
                stream.ReadUI08(trun_number);
                m_Entries[i].m_TrunNumber = trun_number;
                break;
            }
            case 1: {
                AP4_UI16 trun_number;
                stream.ReadUI16(trun_number);
                m_Entries[i].m_TrunNumber = trun_number;
                break;
            }
            case 2: {
                AP4_UI32 trun_number;
                stream.ReadUI24(trun_number);
                m_Entries[i].m_TrunNumber = trun_number;
                break;
            }
            case 3: {
                AP4_UI32 trun_number;
                stream.ReadUI32(trun_number);
                m_Entries[i].m_TrunNumber = trun_number;
                break;
            }
        }
        
        switch (m_LengthSizeOfSampleNumber) {
            case 0: {
                AP4_UI08 sample_number;
                stream.ReadUI08(sample_number);
                m_Entries[i].m_SampleNumber = sample_number;
                break;
            }
            case 1: {
                AP4_UI16 sample_number;
                stream.ReadUI16(sample_number);
                m_Entries[i].m_SampleNumber = sample_number;
                break;
            }
            case 2: {
                AP4_UI32 sample_number;
                stream.ReadUI24(sample_number);
                m_Entries[i].m_SampleNumber = sample_number;
                break;
            }
            case 3: {
                AP4_UI32 sample_number;
                stream.ReadUI32(sample_number);
                m_Entries[i].m_SampleNumber = sample_number;
                break;
            }
        }        
    }
}

/*----------------------------------------------------------------------
|   AP4_TfraAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_TfraAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    result = stream.WriteUI32(m_TrackId);
    if (AP4_FAILED(result)) return result;
    AP4_UI32 fields = (m_LengthSizeOfTrafNumber<<4)|
                      (m_LengthSizeOfTrunNumber<<2)|
                      (m_LengthSizeOfSampleNumber );
    result = stream.WriteUI32(fields);
    if (AP4_FAILED(result)) return result;
    result = stream.WriteUI32(m_Entries.ItemCount());
    for (unsigned int i=0; i<m_Entries.ItemCount(); i++) {
        if (m_Version == 1) {
            result = stream.WriteUI64(m_Entries[i].m_Time);
            if (AP4_FAILED(result)) return result;
            result = stream.WriteUI64(m_Entries[i].m_MoofOffset);
            if (AP4_FAILED(result)) return result;
        } else {
            result = stream.WriteUI32((AP4_UI32)m_Entries[i].m_Time);
            if (AP4_FAILED(result)) return result;
            result = stream.WriteUI32((AP4_UI32)m_Entries[i].m_MoofOffset);
            if (AP4_FAILED(result)) return result;
        }

        switch (m_LengthSizeOfTrafNumber) {
            case 0: 
                result = stream.WriteUI08((AP4_UI08)m_Entries[i].m_TrafNumber);
                if (AP4_FAILED(result)) return result;
                break;

            case 1: 
                result = stream.WriteUI16((AP4_UI16)m_Entries[i].m_TrafNumber);
                if (AP4_FAILED(result)) return result;
                break;

            case 2: 
                result = stream.WriteUI24(m_Entries[i].m_TrafNumber);
                if (AP4_FAILED(result)) return result;
                break;

            case 3:
                result = stream.WriteUI32(m_Entries[i].m_TrafNumber);
                if (AP4_FAILED(result)) return result;
                break;
        }
        
        switch (m_LengthSizeOfTrunNumber) {
            case 0: 
                result = stream.WriteUI08((AP4_UI08)m_Entries[i].m_TrunNumber);
                if (AP4_FAILED(result)) return result;
                break;

            case 1: 
                result = stream.WriteUI16((AP4_UI16)m_Entries[i].m_TrunNumber);
                if (AP4_FAILED(result)) return result;
                break;

            case 2: 
                result = stream.WriteUI24(m_Entries[i].m_TrunNumber);
                if (AP4_FAILED(result)) return result;
                break;

            case 3:
                result = stream.WriteUI32(m_Entries[i].m_TrunNumber);
                if (AP4_FAILED(result)) return result;
                break;
        }

        switch (m_LengthSizeOfSampleNumber) {
            case 0: 
                result = stream.WriteUI08((AP4_UI08)m_Entries[i].m_SampleNumber);
                if (AP4_FAILED(result)) return result;
                break;

            case 1: 
                result = stream.WriteUI16((AP4_UI16)m_Entries[i].m_SampleNumber);
                if (AP4_FAILED(result)) return result;
                break;

            case 2: 
                result = stream.WriteUI24(m_Entries[i].m_SampleNumber);
                if (AP4_FAILED(result)) return result;
                break;

            case 3:
                result = stream.WriteUI32(m_Entries[i].m_SampleNumber);
                if (AP4_FAILED(result)) return result;
                break;
        }
    }
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_TfraAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_TfraAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("track_ID", m_TrackId);
    inspector.AddField("length_size_of_traf_num",   m_LengthSizeOfTrafNumber);
    inspector.AddField("length_size_of_trun_num",   m_LengthSizeOfTrunNumber);
    inspector.AddField("length_size_of_sample_num", m_LengthSizeOfSampleNumber);
    inspector.AddField("entry count", m_Entries.ItemCount());
    if (inspector.GetVerbosity() >= 1) {
        for (unsigned int i=0; i<m_Entries.ItemCount(); i++) {
            char name[16];
            char value[256];
            AP4_FormatString(name, sizeof(name), "[%04d]", i);
            AP4_FormatString(value, sizeof(value), 
                             "time=%lld, moof_offset=%lld, traf_number=%d, trun_number=%d, sample_number=%d",
                             m_Entries[i].m_Time,
                             m_Entries[i].m_MoofOffset,
                             m_Entries[i].m_TrafNumber,
                             m_Entries[i].m_TrunNumber,
                             m_Entries[i].m_SampleNumber);
            inspector.AddField(name, value);
        }
    }
    
    return AP4_SUCCESS;
}
