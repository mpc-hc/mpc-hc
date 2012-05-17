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

#ifndef _AP4_CTTS_ATOM_H_
#define _AP4_CTTS_ATOM_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4Atom.h"
#include "Ap4Types.h"
#include "Ap4Array.h"

/*----------------------------------------------------------------------
|       AP4_CttsTableEntry
+---------------------------------------------------------------------*/
class AP4_CttsTableEntry {
 public:
    AP4_CttsTableEntry() : 
        m_SampleCount(0), 
        m_SampleOffset(0) {}
    AP4_CttsTableEntry(AP4_Cardinal sample_count,
                       AP4_Offset sample_offset) :
        m_SampleCount(sample_count),
        m_SampleOffset(sample_offset) {}

    AP4_Cardinal m_SampleCount;
    AP4_Offset   m_SampleOffset;
};

/*----------------------------------------------------------------------
|       AP4_CttsAtom
+---------------------------------------------------------------------*/
class AP4_CttsAtom : public AP4_Atom
{
 public:
    // methods
    AP4_CttsAtom(AP4_Size size, AP4_ByteStream& stream);
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    virtual AP4_Result GetCtsOffset(AP4_Ordinal sample, 
                                    AP4_UI32& cts_offset);

 private:
    AP4_Array<AP4_CttsTableEntry> m_Entries;
};

#endif // _AP4_CTTS_ATOM_H_
