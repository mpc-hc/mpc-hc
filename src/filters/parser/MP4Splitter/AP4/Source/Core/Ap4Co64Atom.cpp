/*****************************************************************
|
|    AP4 - co64 Atoms
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
#include "Ap4Co64Atom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_Co64Atom)

/*----------------------------------------------------------------------
|   AP4_Co64Atom::Create
+---------------------------------------------------------------------*/
AP4_Co64Atom*
AP4_Co64Atom::Create(AP4_Size size, AP4_ByteStream& stream)
{
    AP4_UI32 version;
    AP4_UI32 flags;
    if(AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if(version != 0) return NULL;
    return new AP4_Co64Atom(size, version, flags, stream);
}

/*----------------------------------------------------------------------
|   AP4_Co64Atom::AP4_Co64Atom
+---------------------------------------------------------------------*/
AP4_Co64Atom::AP4_Co64Atom(AP4_UI64* entries, AP4_UI32 entry_count) :
    AP4_Atom(AP4_ATOM_TYPE_CO64,
             AP4_FULL_ATOM_HEADER_SIZE + 4 + entry_count * 8,
             0, 0),
    m_Entries(new AP4_UI64[entry_count]),
    m_EntryCount(entry_count)
{
    AP4_CopyMemory(m_Entries, entries, m_EntryCount * 8);
}

/*----------------------------------------------------------------------
|   AP4_Co64Atom::AP4_Co64Atom
+---------------------------------------------------------------------*/
AP4_Co64Atom::AP4_Co64Atom(AP4_UI32        size,
                           AP4_UI32        version,
                           AP4_UI32        flags,
                           AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_CO64, size, version, flags)
{
    stream.ReadUI32(m_EntryCount);
    if(m_EntryCount > (size - AP4_FULL_ATOM_HEADER_SIZE - 4) / 8)
    {
        m_EntryCount = (size - AP4_FULL_ATOM_HEADER_SIZE - 4) / 8;
    }
    m_Entries = new AP4_UI64[m_EntryCount];
    for(AP4_Ordinal i = 0; i < m_EntryCount; i++)
    {
        stream.ReadUI64(m_Entries[i]);
    }
}

/*----------------------------------------------------------------------
|   AP4_Co64Atom::~AP4_Co64Atom
+---------------------------------------------------------------------*/
AP4_Co64Atom::~AP4_Co64Atom()
{
    delete[] m_Entries;
}

/*----------------------------------------------------------------------
|   AP4_Co64Atom::GetChunkOffset
+---------------------------------------------------------------------*/
AP4_Result
AP4_Co64Atom::GetChunkOffset(AP4_Ordinal chunk, AP4_UI64& chunk_offset)
{
    // check the bounds
    if(chunk > m_EntryCount || chunk == 0)
    {
        return AP4_ERROR_OUT_OF_RANGE;
    }

    // get the chunk offset
    chunk_offset = m_Entries[chunk - 1]; // m_Entries is zero index based

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_Co64Atom::SetChunkOffset
+---------------------------------------------------------------------*/
AP4_Result
AP4_Co64Atom::SetChunkOffset(AP4_Ordinal chunk, AP4_UI64 chunk_offset)
{
    // check the bounds
    if(chunk > m_EntryCount || chunk == 0)
    {
        return AP4_ERROR_OUT_OF_RANGE;
    }

    // get the chunk offset
    m_Entries[chunk - 1] = chunk_offset; // m_Entries is zero index based

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_Co64Atom::AdjustChunkOffsets
+---------------------------------------------------------------------*/
AP4_Result
AP4_Co64Atom::AdjustChunkOffsets(AP4_SI64 delta)
{
    for(AP4_Ordinal i = 0; i < m_EntryCount; i++)
    {
        m_Entries[i] += delta;
    }

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_Co64Atom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_Co64Atom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    // entry count
    result = stream.WriteUI32(m_EntryCount);
    if(AP4_FAILED(result)) return result;

    // entries
    for(AP4_Ordinal i = 0; i < m_EntryCount; i++)
    {
        result = stream.WriteUI64(m_Entries[i]);
        if(AP4_FAILED(result)) return result;
    }

    return result;
}

/*----------------------------------------------------------------------
|   AP4_Co64Atom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_Co64Atom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("entry_count", m_EntryCount);
    if(inspector.GetVerbosity() >= 1)
    {
        char header[32];
        for(AP4_Ordinal i = 0; i < m_EntryCount; i++)
        {
            AP4_FormatString(header, sizeof(header), "entry %8d", i);
            inspector.AddField(header, m_Entries[i]);
        }
    }

    return AP4_SUCCESS;
}
