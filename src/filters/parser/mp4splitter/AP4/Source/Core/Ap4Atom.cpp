/*****************************************************************
|
|    AP4 - Atoms 
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
#include "Ap4StsdAtom.h"
#include "Ap4Debug.h"

/*----------------------------------------------------------------------
|       AP4_Atom::AP4_Atom
+---------------------------------------------------------------------*/
AP4_Atom::AP4_Atom(Type type, 
                   bool is_full) : 
    m_Type(type),
    m_Size(is_full ? AP4_FULL_ATOM_HEADER_SIZE : AP4_ATOM_HEADER_SIZE),
    m_IsFull(is_full),
    m_Version(0),
    m_Flags(0),
    m_Parent(NULL)
{
}

/*----------------------------------------------------------------------
|       AP4_Atom::AP4_Atom
+---------------------------------------------------------------------*/
AP4_Atom::AP4_Atom(Type     type, 
                   AP4_Size size,
                   bool     is_full) : 
    m_Type(type),
    m_Size(size),
    m_IsFull(is_full),
    m_Version(0),
    m_Flags(0),
    m_Parent(NULL)
{
}

/*----------------------------------------------------------------------
|       AP4_Atom::AP4_Atom
+---------------------------------------------------------------------*/
AP4_Atom::AP4_Atom(Type            type, 
                   AP4_Size        size,
                   bool            is_full,
                   AP4_ByteStream& stream) : 
    m_Type(type),
    m_Size(size),
    m_IsFull(is_full),
    m_Parent(NULL)
{
    // if this is a full atom, read the version and flags
    if (is_full) {
        AP4_UI32 header;
        stream.ReadUI32(header);
        m_Version = (header>>24)&0xFF;
        m_Flags   = (header&0xFFFFFF);
    } else {
        m_Version = 0;
        m_Flags   = 0;
    }
}

/*----------------------------------------------------------------------
|       AP4_Atom::GetHeaderSize
+---------------------------------------------------------------------*/
AP4_Size
AP4_Atom::GetHeaderSize() 
{
    return m_IsFull ? AP4_FULL_ATOM_HEADER_SIZE : AP4_ATOM_HEADER_SIZE;
}

/*----------------------------------------------------------------------
|       AP4_Atom::WriteHeader
+---------------------------------------------------------------------*/
AP4_Result
AP4_Atom::WriteHeader(AP4_ByteStream& stream)
{
    AP4_Result result;

    // write the size
    result = stream.WriteUI32(m_Size);
    if (AP4_FAILED(result)) return result;

    // write the type
    result = stream.WriteUI32(m_Type);
    if (AP4_FAILED(result)) return result;

    // for full atoms, write version and flags
    if (m_IsFull) {
        result = stream.WriteUI08(m_Version);
        if (AP4_FAILED(result)) return result;
        result = stream.WriteUI24(m_Flags);
        if (AP4_FAILED(result)) return result;
    }

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_Atom::Write
+---------------------------------------------------------------------*/
AP4_Result
AP4_Atom::Write(AP4_ByteStream& stream)
{
    AP4_Result result;

#if defined(AP4_DEBUG)
    AP4_Offset before;
    stream.Tell(before);
#endif

    // write the header
    result = WriteHeader(stream);
    if (AP4_FAILED(result)) return result;

    // write the fields
    result = WriteFields(stream);
    if (AP4_FAILED(result)) return result;

#if defined(AP4_DEBUG)
    AP4_Offset after;
    stream.Tell(after);
    AP4_ASSERT(after-before == m_Size);
#endif

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_Atom::Inspect
+---------------------------------------------------------------------*/
AP4_Result
AP4_Atom::Inspect(AP4_AtomInspector& inspector)
{
    InspectHeader(inspector);
    InspectFields(inspector);
    inspector.EndElement();

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_Atom::InspectHeader
+---------------------------------------------------------------------*/
AP4_Result
AP4_Atom::InspectHeader(AP4_AtomInspector& inspector)
{
    // write atom name
    char name[7];
    name[0] = '[';
    AP4_FormatFourChars(&name[1], m_Type);
    name[5] = ']';
    name[6] = '\0';
    char size[64];
    AP4_StringFormat(size, sizeof(size), "size=%ld+%ld", GetHeaderSize(), 
        m_Size-GetHeaderSize());
    inspector.StartElement(name, size);

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_Atom::Detach
+---------------------------------------------------------------------*/
AP4_Result
AP4_Atom::Detach() 
{
    if (m_Parent) {
        return m_Parent->RemoveChild(this);
    } else {
        return AP4_SUCCESS;
    }
}

/*----------------------------------------------------------------------
|       AP4_UnknownAtom::AP4_UnknownAtom
+---------------------------------------------------------------------*/
AP4_UnknownAtom::AP4_UnknownAtom(Type            type, 
                                 AP4_Size        size,
                                 bool            is_full,
                                 AP4_ByteStream& stream) : 
    AP4_Atom(type, size, is_full, stream),
    m_SourceStream(&stream)
{
    // store source stream offset
    stream.Tell(m_SourceOffset);

    // keep a reference to the source stream
    m_SourceStream->AddReference();
}

/*----------------------------------------------------------------------
|       AP4_UnknownAtom::~AP4_UnknownAtom
+---------------------------------------------------------------------*/
AP4_UnknownAtom::~AP4_UnknownAtom()
{
    // release the source stream reference
    if (m_SourceStream) {
        m_SourceStream->Release();
    }
}

/*----------------------------------------------------------------------
|       AP4_UnknownAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_UnknownAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    // check that we have a source stream
    // and a normal size
    if (m_SourceStream == NULL || m_Size < 8) {
        return AP4_FAILURE;
    }

    // seek into the source at the stored offset
    result = m_SourceStream->Seek(m_SourceOffset);
    if (AP4_FAILED(result)) return result;

    // copy the source stream to the output
    result = m_SourceStream->CopyTo(stream, m_Size-GetHeaderSize());
    if (AP4_FAILED(result)) return result;

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_AtomParent::~AP4_AtomParent
+---------------------------------------------------------------------*/
AP4_AtomParent::~AP4_AtomParent()
{
    m_Children.DeleteReferences();
}

/*----------------------------------------------------------------------
|       AP4_AtomParent::AddChild
+---------------------------------------------------------------------*/
AP4_Result
AP4_AtomParent::AddChild(AP4_Atom* child, int position)
{
    // check that the child does not already have a parent
    if (child->GetParent() != NULL) return AP4_ERROR_INVALID_PARAMETERS;

    // attach the child
    AP4_Result result;
    if (position == -1) {
        // insert at the tail
        result = m_Children.Add(child);
    } else if (position == 0) {
        // insert at the head
        result = m_Children.Insert(NULL, child);
    } else {
        // insert after <n-1>
        AP4_List<AP4_Atom>::Item* insertion_point = m_Children.FirstItem();
        unsigned int count = position;
        while (insertion_point && --count) {
            insertion_point = insertion_point->GetNext();
        }
        if (insertion_point) {
            result = m_Children.Insert(insertion_point, child);
        } else {
            result = AP4_ERROR_OUT_OF_RANGE;
        }
    }
    if (AP4_FAILED(result)) return result;

    // notify the child of its parent
    child->SetParent(this);

    // get a chance to update
    OnChildAdded(child);

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_AtomParent::RemoveChild
+---------------------------------------------------------------------*/
AP4_Result
AP4_AtomParent::RemoveChild(AP4_Atom* child)
{
    // check that this is our child
    if (child->GetParent() != this) return AP4_ERROR_INVALID_PARAMETERS;

    // remove the child
    AP4_Result result = m_Children.Remove(child);
    if (AP4_FAILED(result)) return result;

    // notify that child that it is orphaned
    child->SetParent(NULL);

    // get a chance to update
    OnChildRemoved(child);

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_AtomParent::DeleteChild
+---------------------------------------------------------------------*/
AP4_Result
AP4_AtomParent::DeleteChild(AP4_Atom::Type type)
{
    // find the child
    AP4_Atom* child = GetChild(type);
    if (child == NULL) return AP4_FAILURE;

    // remove the child
    AP4_Result result = RemoveChild(child);
    if (AP4_FAILED(result)) return result;

    // delete the child
    delete child;

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_AtomParent::GetChild
+---------------------------------------------------------------------*/
AP4_Atom*
AP4_AtomParent::GetChild(AP4_Atom::Type type, AP4_Ordinal index /* = 0 */)
{
    AP4_Atom* atom;
    AP4_Result result = m_Children.Find(AP4_AtomFinder(type, index), atom);
    if (AP4_SUCCEEDED(result)) {
        return atom;
    } else { 
        return NULL;
    }
}

/*----------------------------------------------------------------------
|       AP4_AtomParent::FindChild
+---------------------------------------------------------------------*/
AP4_Atom*
AP4_AtomParent::FindChild(const char* path, 
                          bool        auto_create)
{
    // start from here
    AP4_AtomParent* parent = this;

    // walk the path
    while (path[0] && path[1] && path[2] && path[3]) {
        // we have 4 valid chars
        const char* tail;
        int         index = 0;
        if (path[4] == '\0') {
            tail = NULL;
        } else if (path[4] == '/') {
            // separator
            tail = &path[5];
        } else if (path[4] == '[') {
            const char* x = &path[5];
            while (*x >= '0' && *x <= '9') {
                index = 10*index+(*x++ - '0');
            }
            if (x[0] == ']') {
                if (x[1] == '\0') {
                    tail = NULL;
                } else {
                    tail = x+2;
                }
            } else {
                // malformed path
                return NULL;
            }
        } else {
            // malformed path
            return NULL;
        }

        // look for this atom in the current list
        AP4_Atom::Type type = AP4_ATOM_TYPE(path[0], path[1], path[2], path[3]); 
        AP4_Atom* atom = parent->GetChild(type, index);
        if (atom == NULL) {
            // not found
            if (auto_create && (index == 0)) {
                AP4_ContainerAtom* container = dynamic_cast<AP4_ContainerAtom*>(parent);
                if (parent) {
                    atom = new AP4_ContainerAtom(type, false);
                    container->AddChild(atom);
                } else {
                    return NULL;
                }
            } else {
                return NULL;
            }
        }

        if (tail) {
            path = tail;
            // if this atom is an atom parent, recurse
            parent = dynamic_cast<AP4_ContainerAtom*>(atom);
            if (parent == NULL) return NULL;
        } else {
            return atom;
        }
    }

    // not found
    return NULL;
}
