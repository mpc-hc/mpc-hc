/*****************************************************************
|
|    AP4 - iods Atom
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
#include "Ap4IodsAtom.h"
#include "Ap4DescriptorFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_IodsAtom)

/*----------------------------------------------------------------------
|   AP4_IodsAtom::Create
+---------------------------------------------------------------------*/
AP4_IodsAtom*
AP4_IodsAtom::Create(AP4_Size size, AP4_ByteStream& stream)
{
    AP4_UI32 version;
    AP4_UI32 flags;
    if (AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if (version != 0) return NULL;
    return new AP4_IodsAtom(size, version, flags, stream);
}

/*----------------------------------------------------------------------
|   AP4_IodsAtom::AP4_IodsAtom
+---------------------------------------------------------------------*/
AP4_IodsAtom::AP4_IodsAtom(AP4_ObjectDescriptor* descriptor) :
    AP4_Atom(AP4_ATOM_TYPE_IODS, AP4_FULL_ATOM_HEADER_SIZE, 0, 0),
    m_ObjectDescriptor(descriptor)
{
    if (m_ObjectDescriptor) m_Size32 += m_ObjectDescriptor->GetSize();
}

/*----------------------------------------------------------------------
|   AP4_IodsAtom::AP4_IodsAtom
+---------------------------------------------------------------------*/
AP4_IodsAtom::AP4_IodsAtom(AP4_UI32        size, 
                           AP4_UI32        version,
                           AP4_UI32        flags,
                           AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_IODS, size, version, flags)
{
    // read the descriptor
    AP4_Descriptor* descriptor = NULL;
    if (AP4_DescriptorFactory::CreateDescriptorFromStream(stream, descriptor) == AP4_SUCCESS) {
        m_ObjectDescriptor = AP4_DYNAMIC_CAST(AP4_ObjectDescriptor, descriptor);
        if (m_ObjectDescriptor == NULL) delete descriptor;
    } else {
        m_ObjectDescriptor = NULL;
    }
}

/*----------------------------------------------------------------------
|   AP4_IodsAtom::~AP4_IodsAtom
+---------------------------------------------------------------------*/
AP4_IodsAtom::~AP4_IodsAtom()
{
    delete m_ObjectDescriptor;
}

/*----------------------------------------------------------------------
|   AP4_IodsAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_IodsAtom::WriteFields(AP4_ByteStream& stream)
{
    // write the es descriptor
    if (m_ObjectDescriptor) return m_ObjectDescriptor->Write(stream);

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_IodsAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_IodsAtom::InspectFields(AP4_AtomInspector& inspector)
{
    // inspect descriptor
    if (m_ObjectDescriptor) {
        m_ObjectDescriptor->Inspect(inspector);
    }

    return AP4_SUCCESS;
}
