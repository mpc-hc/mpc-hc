/*****************************************************************
|
|    AP4 - stsd Atoms 
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

#ifndef _AP4_STSD_ATOM_H_
#define _AP4_STSD_ATOM_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4Array.h"
#include "Ap4ContainerAtom.h"

/*----------------------------------------------------------------------
|   class references
+---------------------------------------------------------------------*/
class AP4_SampleTable;
class AP4_SampleDescription;
class AP4_SampleEntry;

/*----------------------------------------------------------------------
|   AP4_StsdAtom
+---------------------------------------------------------------------*/
class AP4_StsdAtom : public AP4_ContainerAtom
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_StsdAtom, AP4_ContainerAtom)

    // class methods
    static AP4_StsdAtom* Create(AP4_Size         size,
                                AP4_ByteStream&  stream,
                                AP4_AtomFactory& atom_factory);

    // methods
    AP4_StsdAtom(AP4_SampleTable* sample_table);
    ~AP4_StsdAtom();
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    virtual AP4_Cardinal           GetSampleDescriptionCount();
    virtual AP4_SampleDescription* GetSampleDescription(AP4_Ordinal index);
    virtual AP4_SampleEntry*       GetSampleEntry(AP4_Ordinal index);

    // AP4_AtomParent methods
    void OnChildChanged(AP4_Atom* child);

private:
    // methods
    AP4_StsdAtom(AP4_UI32         size,
                 AP4_UI32         version,
                 AP4_UI32         flags,
                 AP4_ByteStream&  stream,
                 AP4_AtomFactory& atom_factory);

    // members
    AP4_Array<AP4_SampleDescription*> m_SampleDescriptions;
};

#endif // _AP4_STSD_ATOM_H_

