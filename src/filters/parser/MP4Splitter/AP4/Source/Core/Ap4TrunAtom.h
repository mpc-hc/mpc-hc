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

#ifndef _AP4_TRUN_ATOM_H_
#define _AP4_TRUN_ATOM_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Atom.h"
#include "Ap4Array.h"

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const AP4_UI32 AP4_TRUN_FLAG_DATA_OFFSET_PRESENT                    = 0x0001;
const AP4_UI32 AP4_TRUN_FLAG_FIRST_SAMPLE_FLAGS_PRESENT             = 0x0004;
const AP4_UI32 AP4_TRUN_FLAG_SAMPLE_DURATION_PRESENT                = 0x0100;
const AP4_UI32 AP4_TRUN_FLAG_SAMPLE_SIZE_PRESENT                    = 0x0200;
const AP4_UI32 AP4_TRUN_FLAG_SAMPLE_FLAGS_PRESENT                   = 0x0400;
const AP4_UI32 AP4_TRUN_FLAG_SAMPLE_COMPOSITION_TIME_OFFSET_PRESENT = 0x0800;

/*----------------------------------------------------------------------
|   AP4_TrunAtom
+---------------------------------------------------------------------*/
class AP4_TrunAtom : public AP4_Atom
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST(AP4_TrunAtom)

    // types
    struct Entry {
        Entry() : sample_duration(0), sample_size(0), sample_flags(0), sample_composition_time_offset(0) {}
        AP4_UI32 sample_duration;
        AP4_UI32 sample_size;
        AP4_UI32 sample_flags;
        AP4_UI32 sample_composition_time_offset;
    };
    
    // class methods
    static AP4_TrunAtom* Create(AP4_Size size, AP4_ByteStream& stream);
    static unsigned int  ComputeOptionalFieldsCount(AP4_UI32 flags);
    static unsigned int  ComputeRecordFieldsCount(AP4_UI32 flags);

    // methods
    AP4_TrunAtom(AP4_UI32 flags, 
                 AP4_SI32 data_offset,
                 AP4_UI32 first_sample_flags);
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);

    // accessors
    AP4_SI32                GetDataOffset()       { return m_DataOffset;       }
    AP4_UI32                GetFirstSampleFlags() { return m_FirstSampleFlags; }
    const AP4_Array<Entry>& GetEntries()          { return m_Entries;          }
    
private:
    // methods
    AP4_TrunAtom(AP4_UI32        size, 
                 AP4_UI32        version,
                 AP4_UI32        flags,
                 AP4_ByteStream& stream);

    // members
    AP4_SI32         m_DataOffset;
    AP4_UI32         m_FirstSampleFlags;
    AP4_Array<Entry> m_Entries;
};

#endif // _AP4_TRUN_ATOM_H_
