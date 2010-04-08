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

#ifndef _AP4_TFHD_ATOM_H_
#define _AP4_TFHD_ATOM_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Atom.h"

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const AP4_UI32 AP4_TFHD_FLAG_BASE_DATA_OFFSET_PRESENT         = 0x00001;
const AP4_UI32 AP4_TFHD_FLAG_SAMPLE_DESCRIPTION_INDEX_PRESENT = 0x00002;
const AP4_UI32 AP4_TFHD_FLAG_DEFAULT_SAMPLE_DURATION_PRESENT  = 0x00008;
const AP4_UI32 AP4_TFHD_FLAG_DEFAULT_SAMPLE_SIZE_PRESENT      = 0x00010;
const AP4_UI32 AP4_TFHD_FLAG_DEFAULT_SAMPLE_FLAGS_PRESENT     = 0x00020;
const AP4_UI32 AP4_TFHD_FLAG_DURATION_IS_EMPTY                = 0x10000;

/*----------------------------------------------------------------------
|   AP4_TfhdAtom
+---------------------------------------------------------------------*/
class AP4_TfhdAtom : public AP4_Atom
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST(AP4_TfhdAtom)

    // class methods
    static AP4_TfhdAtom* Create(AP4_Size size, AP4_ByteStream& stream);
    static AP4_UI32      ComputeSize(AP4_UI32 flags);

    // methods
    AP4_TfhdAtom(AP4_UI32 flags,
                 AP4_UI32 track_id,
                 AP4_UI64 base_data_offset,
                 AP4_UI32 sample_description_index,
                 AP4_UI32 default_sample_duration,
                 AP4_UI32 default_sample_size,
                 AP4_UI32 default_sample_flags);
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);

    AP4_UI32 GetTrackId()
    {
        return m_TrackId;
    }
    void     SetTrackId(AP4_UI32 track_id)
    {
        m_TrackId = track_id;
    }
    AP4_UI64 GetBaseDataOffset()
    {
        return m_BaseDataOffset;
    }
    void     SetBaseDataOffset(AP4_UI64 offset)
    {
        m_BaseDataOffset = offset;
    }
    AP4_UI32 GetSampleDescriptionIndex()
    {
        return m_SampleDescriptionIndex;
    }
    void     SetSampleDescriptionIndex(AP4_UI32 indx)
    {
        m_SampleDescriptionIndex = indx;
    }
    AP4_UI32 GetDefaultSampleDuration()
    {
        return m_DefaultSampleDuration;
    }
    void     SetDefaultSampleDuration(AP4_UI32 duration)
    {
        m_DefaultSampleDuration = duration;
    }
    AP4_UI32 GetDefaultSampleSize()
    {
        return m_DefaultSampleSize;
    }
    void     SetDefaultSampleSize(AP4_UI32 size)
    {
        m_DefaultSampleSize = size;
    }
    AP4_UI32 GetDefaultSampleFlags()
    {
        return m_DefaultSampleFlags;
    }
    void     SetDefaultSampleFlags(AP4_UI32 flags)
    {
        m_DefaultSampleFlags = flags;
    }

private:
    // methods
    AP4_TfhdAtom(AP4_UI32        size,
                 AP4_UI32        version,
                 AP4_UI32        flags,
                 AP4_ByteStream& stream);

    // members
    AP4_UI32 m_TrackId;
    AP4_UI64 m_BaseDataOffset;
    AP4_UI32 m_SampleDescriptionIndex;
    AP4_UI32 m_DefaultSampleDuration;
    AP4_UI32 m_DefaultSampleSize;
    AP4_UI32 m_DefaultSampleFlags;
};

#endif // _AP4_TFHD_ATOM_H_
