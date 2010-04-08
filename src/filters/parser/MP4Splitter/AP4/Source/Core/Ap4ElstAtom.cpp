/*****************************************************************
|
|    AP4 - elst Atoms
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
#include "Ap4ElstAtom.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   AP4_ElstAtom::Create
+---------------------------------------------------------------------*/
AP4_ElstAtom*
AP4_ElstAtom::Create(AP4_Size size, AP4_ByteStream& stream)
{
    AP4_UI32 version;
    AP4_UI32 flags;
    if(AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if(version > 1) return NULL;
    return new AP4_ElstAtom(size, version, flags, stream);
}

/*----------------------------------------------------------------------
|   AP4_ElstAtom::AP4_ElstAtom
+---------------------------------------------------------------------*/
AP4_ElstAtom::AP4_ElstAtom(AP4_UI32        size,
                           AP4_UI32        version,
                           AP4_UI32        flags,
                           AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_ELST, size, version, flags)
{
    AP4_UI32 entry_count;
    stream.ReadUI32(entry_count);
    m_Entries.EnsureCapacity(entry_count);
    for(AP4_UI32 i = 0; i < entry_count; i++)
    {
        AP4_UI16 media_rate;
        AP4_UI16 zero;
        if(version == 0)
        {
            AP4_UI32 segment_duration;
            AP4_UI32 media_time;
            stream.ReadUI32(segment_duration);
            stream.ReadUI32(media_time);
            stream.ReadUI16(media_rate);
            stream.ReadUI16(zero);
            m_Entries.Append(AP4_ElstEntry(segment_duration, media_time, media_rate));
        }
        else
        {
            AP4_UI64 segment_duration;
            AP4_UI64 media_time;
            stream.ReadUI64(segment_duration);
            stream.ReadUI64(media_time);
            stream.ReadUI16(media_rate);
            stream.ReadUI16(zero);
            m_Entries.Append(AP4_ElstEntry(segment_duration, media_time, media_rate));
        }
    }
}

/*----------------------------------------------------------------------
|   AP4_ElstAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_ElstAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    result = stream.WriteUI32(m_Entries.ItemCount());
    if(AP4_FAILED(result)) return result;
    for(AP4_Ordinal i = 0; i < m_Entries.ItemCount(); i++)
    {
        if(m_Version == 0)
        {
            result = stream.WriteUI32((AP4_UI32)m_Entries[i].m_SegmentDuration);
            if(AP4_FAILED(result)) return result;
            result = stream.WriteUI32((AP4_UI32)m_Entries[i].m_MediaTime);
            if(AP4_FAILED(result)) return result;
        }
        else
        {
            result = stream.WriteUI64(m_Entries[i].m_SegmentDuration);
            if(AP4_FAILED(result)) return result;
            result = stream.WriteUI64(m_Entries[i].m_MediaTime);
            if(AP4_FAILED(result)) return result;
        }
        result = stream.WriteUI16(m_Entries[i].m_MediaRate);
        if(AP4_FAILED(result)) return result;
        result = stream.WriteUI16(0);
        if(AP4_FAILED(result)) return result;
    }

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_ElstAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_ElstAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("entry count", m_Entries.ItemCount());
    for(AP4_Ordinal i = 0; i < m_Entries.ItemCount(); i++)
    {
        inspector.AddField("entry/segment duration", (AP4_UI32)m_Entries[i].m_SegmentDuration);
        inspector.AddField("entry/media time", (AP4_SI32)m_Entries[i].m_MediaTime);
        inspector.AddField("entry/media rate", (AP4_UI16)m_Entries[i].m_MediaRate);
    }

    return AP4_SUCCESS;
}
