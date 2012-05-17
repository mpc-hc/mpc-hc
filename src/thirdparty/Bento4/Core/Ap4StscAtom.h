/*****************************************************************
|
|    AP4 - stsc Atoms 
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

#ifndef _AP4_STSC_ATOM_H_
#define _AP4_STSC_ATOM_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4.h"
#include "Ap4ByteStream.h"
#include "Ap4Array.h"
#include "Ap4Atom.h"

/*----------------------------------------------------------------------
|       AP4_StscTableEntry
+---------------------------------------------------------------------*/
class AP4_StscTableEntry {
 public:
    AP4_StscTableEntry() : 
        m_FirstChunk(0), 
        m_FirstSample(0),
        m_ChunkCount(0),
        m_SamplesPerChunk(0), 
        m_SampleDescriptionIndex(0) {}
    AP4_StscTableEntry(AP4_Ordinal  first_chunk,
                       AP4_Ordinal  first_sample,
                       AP4_Cardinal samples_per_chunk,
                       AP4_Ordinal  sample_description_index) :
        m_FirstChunk(first_chunk),
        m_FirstSample(first_sample),
        m_ChunkCount(0),
        m_SamplesPerChunk(samples_per_chunk),
        m_SampleDescriptionIndex(sample_description_index) {}
    AP4_StscTableEntry(AP4_Ordinal  first_chunk,
                       AP4_Ordinal  first_sample,
                       AP4_Cardinal chunk_count,
                       AP4_Cardinal samples_per_chunk,
                       AP4_Ordinal  sample_description_index) :
        m_FirstChunk(first_chunk),
        m_FirstSample(first_sample),
        m_ChunkCount(chunk_count),
        m_SamplesPerChunk(samples_per_chunk),
        m_SampleDescriptionIndex(sample_description_index) {}
    AP4_Ordinal  m_FirstChunk;
    AP4_Ordinal  m_FirstSample;  // computed (not in file)
    AP4_Cardinal m_ChunkCount;   // computed (not in file)
    AP4_Cardinal m_SamplesPerChunk;
    AP4_Ordinal  m_SampleDescriptionIndex;
};

/*----------------------------------------------------------------------
|       AP4_StscAtom
+---------------------------------------------------------------------*/
class AP4_StscAtom : public AP4_Atom
{
 public:
    // methods
    AP4_StscAtom();
    AP4_StscAtom(AP4_Size size, AP4_ByteStream& stream);
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result GetChunkForSample(AP4_Ordinal   sample,
                                         AP4_Ordinal&  chunk,
                                         AP4_Ordinal&  skip,
                                         AP4_Ordinal&  sample_description);
    virtual AP4_Result AddEntry(AP4_Cardinal chunk_count,
                                AP4_Cardinal samples_per_chunk,
                                AP4_Ordinal  sample_description_index);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);

	// FIXME
	friend class AP4_AtomSampleTable;

 private:
    // data
    AP4_Array<AP4_StscTableEntry> m_Entries;
    AP4_Ordinal                   m_CachedChunkGroup;
};

#endif // _AP4_STSC_ATOM_H_
