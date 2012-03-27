/*****************************************************************
|
|    AP4 - ctts Atoms 
|
|    Copyright 2003 Gilles Boccon-Gibod & Julien Boeuf
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
|       includes
+---------------------------------------------------------------------*/
#include "Ap4.h"
#include "Ap4CttsAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|       AP4_CttsAtom::AP4_CttsAtom
+---------------------------------------------------------------------*/
AP4_CttsAtom::AP4_CttsAtom(AP4_Size size, AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_CTTS, size, true, stream)
{
    AP4_UI32 entry_count;
    stream.ReadUI32(entry_count);
    while (entry_count--) {
        AP4_UI32 sample_count;
        AP4_UI32 sample_offset;
        if (stream.ReadUI32(sample_count)  == AP4_SUCCESS &&
            stream.ReadUI32(sample_offset) == AP4_SUCCESS) {
            m_Entries.Append(AP4_CttsTableEntry(sample_count,
                                                sample_offset));
        }
    }
}

/*----------------------------------------------------------------------
|       AP4_CttsAtom::GetCtsOffset
+---------------------------------------------------------------------*/
AP4_Result
AP4_CttsAtom::GetCtsOffset(AP4_Ordinal sample, AP4_UI32& cts_offset)
{
    AP4_Ordinal current_sample = 0;

    for (unsigned int i = 0; i < m_Entries.ItemCount(); i++) {
        AP4_CttsTableEntry& entry = m_Entries[i];

        current_sample += entry.m_SampleCount;        
        // check if we have the right entry 
        if (current_sample >= sample) {
            cts_offset = entry.m_SampleOffset;
            return AP4_SUCCESS;
        }
    }

    // sample is greater than the number of samples
    return AP4_FAILURE;
}

/*----------------------------------------------------------------------
|       AP4_CttsAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_CttsAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    // write the entry count
    AP4_Cardinal entry_count = m_Entries.ItemCount();
    result = stream.WriteUI32(entry_count);
    if (AP4_FAILED(result)) return result;

    // write the entries
    for (AP4_Ordinal i=0; i<entry_count; i++) {
        // sample count
        result = stream.WriteUI32(m_Entries[i].m_SampleCount);
        if (AP4_FAILED(result)) return result;

        // time offset
        result = stream.WriteUI32(m_Entries[i].m_SampleOffset);
        if (AP4_FAILED(result)) return result;
    }

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_CttsAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_CttsAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("entry_count", m_Entries.ItemCount());

    return AP4_SUCCESS;
}
