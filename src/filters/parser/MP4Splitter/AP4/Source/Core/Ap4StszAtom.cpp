/*****************************************************************
|
|    AP4 - stsz Atoms 
|
|    Copyright 2002 Gilles Boccon-Gibod
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
#include "Ap4StszAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"
/*----------------------------------------------------------------------
|       AP4_StszAtom::AP4_StszAtom
+---------------------------------------------------------------------*/
AP4_StszAtom::AP4_StszAtom() :
    AP4_Atom(AP4_ATOM_TYPE_STSZ, AP4_FULL_ATOM_HEADER_SIZE+8, true),
    m_SampleSize(0),
    m_SampleCount(0)
{
}

/*----------------------------------------------------------------------
|       AP4_StszAtom::AP4_StszAtom
+---------------------------------------------------------------------*/
AP4_StszAtom::AP4_StszAtom(AP4_Size size, AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_STSZ, size, true, stream)
{
    stream.ReadUI32(m_SampleSize);
    stream.ReadUI32(m_SampleCount);

    unsigned long sample_count = m_SampleCount;
    if (m_SampleSize == 0) { // means that the samples have different sizes
        while (sample_count--) {
            AP4_UI32 entry_size;
            if (stream.ReadUI32(entry_size) == AP4_SUCCESS) {
                m_Entries.Append(entry_size);
            }
        }
    }
}

/*----------------------------------------------------------------------
|       AP4_StszAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_StszAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    // sample size
    result = stream.WriteUI32(m_SampleSize);
    if (AP4_FAILED(result)) return result;

    // sample count
    result = stream.WriteUI32(m_SampleCount);
    if (AP4_FAILED(result)) return result;

    // entries if needed (the samples have different sizes)
    if (m_SampleSize == 0) {
        for (AP4_UI32 i=0; i<m_SampleCount; i++) {
            result = stream.WriteUI32(m_Entries[i]);
            if (AP4_FAILED(result)) return result;
        }
    }

    return result;
}

/*----------------------------------------------------------------------
|       AP4_StszAtom::GetSampleCount
+---------------------------------------------------------------------*/
AP4_UI32
AP4_StszAtom::GetSampleCount()
{
    return m_SampleCount;
}

/*----------------------------------------------------------------------
|       AP4_StszAtom::GetSampleSize
+---------------------------------------------------------------------*/
AP4_Result
AP4_StszAtom::GetSampleSize(AP4_Ordinal sample_start, 
							AP4_Ordinal sample_end,
							AP4_Size&   sample_size)
{
	sample_size = 0;

    if(sample_start > m_SampleCount || sample_end > m_SampleCount)
	{
		return AP4_ERROR_OUT_OF_RANGE;
	}

	if(m_SampleSize != 0)
	{
		sample_size = m_SampleSize * (sample_end - sample_start);
	}
	else
	{
		// compute the additional offset inside the chunk
		for (unsigned int i = sample_start; i < sample_end; i++) {
			AP4_Size size;
			AP4_Result result = GetSampleSize(i, size); 
			if (AP4_FAILED(result)) return result;
			sample_size += size;
		}
	}

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_StszAtom::GetSampleSize
+---------------------------------------------------------------------*/
AP4_Result
AP4_StszAtom::GetSampleSize(AP4_Ordinal sample, AP4_Size& sample_size)
{
    // check the sample index
    if (sample > m_SampleCount) {
        sample_size = 0;
        return AP4_ERROR_OUT_OF_RANGE;
    } else {
        // find the size
        if (m_SampleSize != 0) { // constant size
            sample_size = m_SampleSize;
        } else {
            sample_size = m_Entries[sample - 1];
        }
        return AP4_SUCCESS;
    }
}

/*----------------------------------------------------------------------
|       AP4_StszAtom::SetSampleSize
+---------------------------------------------------------------------*/
AP4_Result
AP4_StszAtom::SetSampleSize(AP4_Ordinal sample, AP4_Size sample_size)
{
    // check the sample index
    if (sample > m_SampleCount) {
        return AP4_ERROR_OUT_OF_RANGE;
    } else {
        m_Entries[sample - 1] = sample_size;
        return AP4_SUCCESS;
    }
}

/*----------------------------------------------------------------------
|       AP4_StszAtom::AddEntry
+---------------------------------------------------------------------*/
AP4_Result 
AP4_StszAtom::AddEntry(AP4_UI32 size)
{
    m_Entries.Append(size);
    m_SampleCount++;
    m_Size += 4;

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_StszAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_StszAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("sample_size", m_SampleSize);
    inspector.AddField("sample_count", m_Entries.ItemCount());

    return AP4_SUCCESS;
}
