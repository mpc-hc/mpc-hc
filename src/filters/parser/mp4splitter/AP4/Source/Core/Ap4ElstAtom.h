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

#ifndef _AP4_ELST_ATOM_H_
#define _AP4_ELST_ATOM_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4Atom.h"
#include "Ap4Array.h"

/*----------------------------------------------------------------------
|   class references
+---------------------------------------------------------------------*/
class AP4_ByteStream;

/*----------------------------------------------------------------------
|   AP4_ElstAtom
+---------------------------------------------------------------------*/
class AP4_ElstEntry {
public:
    AP4_ElstEntry(AP4_UI64 segment_duration = 0, AP4_SI64 media_time = 0, AP4_UI16 media_rate = 1) :
      m_SegmentDuration(segment_duration),
      m_MediaTime(media_time),
      m_MediaRate(media_rate) {}

    AP4_UI64 m_SegmentDuration;
    AP4_SI64 m_MediaTime;
    AP4_UI16 m_MediaRate;
};

class AP4_ElstAtom : public AP4_Atom
{
public:
    // class methods
    static AP4_ElstAtom* Create(AP4_Size size, AP4_ByteStream& stream);

    // methods
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    
private:
    // methods
    AP4_ElstAtom(AP4_UI32        size, 
                 AP4_UI32        version, 
                 AP4_UI32        flags, 
                 AP4_ByteStream& stream);

    // members
    AP4_Array<AP4_ElstEntry> m_Entries;
};

#endif // _AP4_ELST_ATOM_H_
