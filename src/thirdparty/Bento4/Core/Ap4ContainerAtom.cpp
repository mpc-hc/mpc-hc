/*****************************************************************
|
|    AP4 - Container Atoms
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
#include "Ap4Atom.h"
#include "Ap4Utils.h"
#include "Ap4ContainerAtom.h"
#include "Ap4AtomFactory.h"

/*----------------------------------------------------------------------
|       AP4_ContainerAtom::AP4_ContainerAtom
+---------------------------------------------------------------------*/
AP4_ContainerAtom::AP4_ContainerAtom(Type type, bool is_full) :
    AP4_Atom(type, is_full)
{
}

/*----------------------------------------------------------------------
|       AP4_ContainerAtom::AP4_ContainerAtom
+---------------------------------------------------------------------*/
AP4_ContainerAtom::AP4_ContainerAtom(Type type, AP4_Size size, bool is_full) :
    AP4_Atom(type, size, is_full)
{
}

/*----------------------------------------------------------------------
|       AP4_ContainerAtom::AP4_ContainerAtom
+---------------------------------------------------------------------*/
AP4_ContainerAtom::AP4_ContainerAtom(Type             type, 
                                     AP4_Size         size,
                                     bool             is_full,
                                     AP4_ByteStream&  stream,
                                     AP4_AtomFactory& atom_factory) :
    AP4_Atom(type, size, is_full, stream)
{
    ReadChildren(atom_factory, stream, size-GetHeaderSize());
}

/*----------------------------------------------------------------------
|       AP4_ContainerAtom::AP4_ContainerAtom
+---------------------------------------------------------------------*/
AP4_ContainerAtom::AP4_ContainerAtom(Type             type, 
                                     AP4_Size         size,
                                     bool             is_full,
                                     AP4_ByteStream&  stream) :
    AP4_Atom(type, size, is_full, stream)
{
}

/*----------------------------------------------------------------------
|       AP4_ContainerAtom::ReadChildren
+---------------------------------------------------------------------*/
void
AP4_ContainerAtom::ReadChildren(AP4_AtomFactory& atom_factory,
                                AP4_ByteStream&  stream, 
                                AP4_Size         size)
{
    AP4_Atom* atom;
    AP4_Size  bytes_available = size;
    while (AP4_SUCCEEDED(
        atom_factory.CreateAtomFromStream(stream, bytes_available, atom, this))) {
        atom->SetParent(this);
        m_Children.Add(atom);
    }
}

/*----------------------------------------------------------------------
|       AP4_ContainerAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_ContainerAtom::InspectFields(AP4_AtomInspector& inspector)
{
    return InspectChildren(inspector);
}

/*----------------------------------------------------------------------
|       AP4_ContainerAtom::InspectChildren
+---------------------------------------------------------------------*/
AP4_Result
AP4_ContainerAtom::InspectChildren(AP4_AtomInspector& inspector)
{
    // inspect children
    m_Children.Apply(AP4_AtomListInspector(inspector));

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_ContainerAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_ContainerAtom::WriteFields(AP4_ByteStream& stream)
{
    // write all children
    return m_Children.Apply(AP4_AtomListWriter(stream));
}

/*----------------------------------------------------------------------
|       AP4_ContainerAtom::OnChildChanged
+---------------------------------------------------------------------*/
void
AP4_ContainerAtom::OnChildChanged(AP4_Atom*)
{
    // remcompute our size
    m_Size = GetHeaderSize();
    m_Children.Apply(AP4_AtomSizeAdder(m_Size));

    // update our parent
    if (m_Parent) m_Parent->OnChildChanged(this);
}

/*----------------------------------------------------------------------
|       AP4_ContainerAtom::OnChildAdded
+---------------------------------------------------------------------*/
void
AP4_ContainerAtom::OnChildAdded(AP4_Atom* child)
{
    // update our size
    m_Size += child->GetSize();

    // update our parent
    if (m_Parent) m_Parent->OnChildChanged(this);
}

/*----------------------------------------------------------------------
|       AP4_ContainerAtom::OnChildRemoved
+---------------------------------------------------------------------*/
void
AP4_ContainerAtom::OnChildRemoved(AP4_Atom* child)
{
    // update our size
    m_Size -= child->GetSize();

    // update our parent
    if (m_Parent) m_Parent->OnChildChanged(this);
}
