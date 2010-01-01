/*****************************************************************
|
|    AP4 - grpi Atom
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

#ifndef _AP4_GRPI_ATOM_H_
#define _AP4_GRPI_ATOM_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4Atom.h"
#include "Ap4String.h"
#include "Ap4DataBuffer.h"

/*----------------------------------------------------------------------
|   AP4_GrpiAtom
+---------------------------------------------------------------------*/
class AP4_GrpiAtom : public AP4_Atom
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_GrpiAtom, AP4_Atom)

    // class methods
    static AP4_GrpiAtom* Create(AP4_Size        size, 
                                AP4_ByteStream& stream);

    // constructor
    AP4_GrpiAtom(AP4_UI08        key_encryption_method, 
                 const char*     group_id,
                 const AP4_UI08* group_key,
                 AP4_Size        group_key_length);
                 
    // methods
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    virtual AP4_Atom*  Clone();

    // accessors
    AP4_UI08              GetKeyEncryptionMethod() const { return m_KeyEncryptionMethod; } 
    void                  SetKeyEncryptionMethod(AP4_UI08 encryption_method) { m_KeyEncryptionMethod = encryption_method; }
    const AP4_String&     GetGroupId()  const { return m_GroupId; }
    const AP4_DataBuffer& GetGroupKey() const { return m_GroupKey;  }
    
private:
    // methods
    AP4_GrpiAtom(AP4_UI32         size, 
                 AP4_UI32         version,
                 AP4_UI32         flags,
                 AP4_ByteStream&  stream);

    // members
    AP4_UI08       m_KeyEncryptionMethod; 
    AP4_String     m_GroupId;
    AP4_DataBuffer m_GroupKey;
};

#endif // _AP4_GRPI_ATOM_H_
