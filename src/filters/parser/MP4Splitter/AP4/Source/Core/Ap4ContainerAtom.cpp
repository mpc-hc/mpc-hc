/*****************************************************************
|
|    AP4 - Container Atoms
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

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4Atom.h"
#include "Ap4Utils.h"
#include "Ap4ContainerAtom.h"
#include "Ap4AtomFactory.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_ContainerAtom)

/*----------------------------------------------------------------------
|   AP4_ContainerAtom::AP4_ContainerAtom
+---------------------------------------------------------------------*/
AP4_ContainerAtom*
AP4_ContainerAtom::Create(Type             type,
                          AP4_UI64         size,
                          bool             is_full,
                          bool             force_64,
                          AP4_ByteStream&  stream,
                          AP4_AtomFactory& atom_factory)
{
    if(is_full)
    {
        AP4_UI32 version;
        AP4_UI32 flags;
        if(AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;

        // special case for 'meta' atoms, because Apple sometimes creates them as
        // regular (non-full) atoms. This is bogus, but we can try to detect it
        if(type == AP4_ATOM_TYPE_META)
        {
            AP4_UI32 phantom_size = (version << 24) | flags;
            if(phantom_size >= 8 && size >= 16)
            {
                // version+flags looks like a size. read the next 4 bytes just
                // to be sure it is a hdlr atom
                AP4_UI32 peek;
                if(AP4_FAILED(stream.ReadUI32(peek))) return NULL;
                if(peek == AP4_ATOM_TYPE_HDLR)
                {
                    // rewind the stream by 8 bytes
                    AP4_Position position;
                    stream.Tell(position);
                    stream.Seek(position - 8);

                    // create a non-full container
                    return new AP4_ContainerAtom(type, size, force_64, stream, atom_factory);
                }
            }
        }

        return new AP4_ContainerAtom(type, size, force_64, version, flags, stream, atom_factory);
    }
    else
    {
        return new AP4_ContainerAtom(type, size, force_64, stream, atom_factory);
    }
}

/*----------------------------------------------------------------------
|   AP4_ContainerAtom::AP4_ContainerAtom
+---------------------------------------------------------------------*/
AP4_ContainerAtom::AP4_ContainerAtom(Type type) :
    AP4_Atom(type, AP4_ATOM_HEADER_SIZE)
{
}

/*----------------------------------------------------------------------
|   AP4_ContainerAtom::AP4_ContainerAtom
+---------------------------------------------------------------------*/
AP4_ContainerAtom::AP4_ContainerAtom(Type type, AP4_UI32 version, AP4_UI32 flags) :
    AP4_Atom(type, AP4_FULL_ATOM_HEADER_SIZE, version, flags)
{
}

/*----------------------------------------------------------------------
|   AP4_ContainerAtom::AP4_ContainerAtom
+---------------------------------------------------------------------*/
AP4_ContainerAtom::AP4_ContainerAtom(Type type, AP4_UI64 size, bool force_64) :
    AP4_Atom(type, size, force_64)
{
}

/*----------------------------------------------------------------------
|   AP4_ContainerAtom::AP4_ContainerAtom
+---------------------------------------------------------------------*/
AP4_ContainerAtom::AP4_ContainerAtom(Type     type,
                                     AP4_UI64 size,
                                     bool     force_64,
                                     AP4_UI32 version,
                                     AP4_UI32 flags) :
    AP4_Atom(type, size, force_64, version, flags)
{
}


/*----------------------------------------------------------------------
|   AP4_ContainerAtom::AP4_ContainerAtom
+---------------------------------------------------------------------*/
AP4_ContainerAtom::AP4_ContainerAtom(Type             type,
                                     AP4_UI64         size,
                                     bool             force_64,
                                     AP4_ByteStream&  stream,
                                     AP4_AtomFactory& atom_factory) :
    AP4_Atom(type, size, force_64)
{
    ReadChildren(atom_factory, stream, size - GetHeaderSize());
}

/*----------------------------------------------------------------------
|   AP4_ContainerAtom::AP4_ContainerAtom
+---------------------------------------------------------------------*/
AP4_ContainerAtom::AP4_ContainerAtom(Type             type,
                                     AP4_UI64         size,
                                     bool             force_64,
                                     AP4_UI32         version,
                                     AP4_UI32         flags,
                                     AP4_ByteStream&  stream,
                                     AP4_AtomFactory& atom_factory) :
    AP4_Atom(type, size, force_64, version, flags)
{
    ReadChildren(atom_factory, stream, size - GetHeaderSize());
}

/*----------------------------------------------------------------------
|   AP4_ContainerAtom::Clone
+---------------------------------------------------------------------*/
AP4_Atom*
AP4_ContainerAtom::Clone()
{
    AP4_ContainerAtom* clone;
    if(m_IsFull)
    {
        clone = new AP4_ContainerAtom(m_Type, m_Version, m_Flags);
    }
    else
    {
        clone = new AP4_ContainerAtom(m_Type);
    }

    AP4_List<AP4_Atom>::Item* child_item = m_Children.FirstItem();
    while(child_item)
    {
        AP4_Atom* child_clone = child_item->GetData()->Clone();
        if(child_clone) clone->AddChild(child_clone);
        child_item = child_item->GetNext();
    }

    return clone;
}

/*----------------------------------------------------------------------
|   AP4_ContainerAtom::ReadChildren
+---------------------------------------------------------------------*/
void
AP4_ContainerAtom::ReadChildren(AP4_AtomFactory& atom_factory,
                                AP4_ByteStream&  stream,
                                AP4_UI64         size)
{
    AP4_Atom*     atom;
    AP4_LargeSize bytes_available = size;

    // save and switch the factory's context
    atom_factory.PushContext(m_Type);

    while(AP4_SUCCEEDED(
              atom_factory.CreateAtomFromStream(stream, bytes_available, atom)))
    {
        atom->SetParent(this);
        m_Children.Add(atom);
    }

    // restore the saved context
    atom_factory.PopContext();
}

/*----------------------------------------------------------------------
|   AP4_ContainerAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_ContainerAtom::InspectFields(AP4_AtomInspector& inspector)
{
    return InspectChildren(inspector);
}

/*----------------------------------------------------------------------
|   AP4_ContainerAtom::InspectChildren
+---------------------------------------------------------------------*/
AP4_Result
AP4_ContainerAtom::InspectChildren(AP4_AtomInspector& inspector)
{
    // inspect children
    m_Children.Apply(AP4_AtomListInspector(inspector));

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_ContainerAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_ContainerAtom::WriteFields(AP4_ByteStream& stream)
{
    // write all children
    return m_Children.Apply(AP4_AtomListWriter(stream));
}

/*----------------------------------------------------------------------
|   AP4_ContainerAtom::OnChildChanged
+---------------------------------------------------------------------*/
void
AP4_ContainerAtom::OnChildChanged(AP4_Atom*)
{
    // remcompute our size
    AP4_UI64 size = GetHeaderSize();
    m_Children.Apply(AP4_AtomSizeAdder(size));
    SetSize(size);

    // update our parent
    if(m_Parent) m_Parent->OnChildChanged(this);
}

/*----------------------------------------------------------------------
|   AP4_ContainerAtom::OnChildAdded
+---------------------------------------------------------------------*/
void
AP4_ContainerAtom::OnChildAdded(AP4_Atom* child)
{
    // update our size
    SetSize(GetSize() + child->GetSize());

    // update our parent
    if(m_Parent) m_Parent->OnChildChanged(this);
}

/*----------------------------------------------------------------------
|   AP4_ContainerAtom::OnChildRemoved
+---------------------------------------------------------------------*/
void
AP4_ContainerAtom::OnChildRemoved(AP4_Atom* child)
{
    // update our size
    SetSize(GetSize() - child->GetSize());

    // update our parent
    if(m_Parent) m_Parent->OnChildChanged(this);
}
