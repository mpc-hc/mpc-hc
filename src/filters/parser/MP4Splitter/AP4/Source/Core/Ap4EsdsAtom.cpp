/*****************************************************************
|
|    AP4 - esds Atoms
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
#include "Ap4EsdsAtom.h"
#include "Ap4DescriptorFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_EsdsAtom)

/*----------------------------------------------------------------------
|   AP4_EsdsAtom::Create
+---------------------------------------------------------------------*/
AP4_EsdsAtom*
AP4_EsdsAtom::Create(AP4_Size size, AP4_ByteStream& stream)
{
    AP4_UI32 version;
    AP4_UI32 flags;
    if(AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if(version != 0) return NULL;
    return new AP4_EsdsAtom(size, version, flags, stream);
}

/*----------------------------------------------------------------------
|   AP4_EsdsAtom::AP4_EsdsAtom
+---------------------------------------------------------------------*/
AP4_EsdsAtom::AP4_EsdsAtom(AP4_EsDescriptor* descriptor) :
    AP4_Atom(AP4_ATOM_TYPE_ESDS, AP4_FULL_ATOM_HEADER_SIZE, 0, 0),
    m_EsDescriptor(descriptor)
{
    if(m_EsDescriptor) m_Size32 += m_EsDescriptor->GetSize();
}

/*----------------------------------------------------------------------
|   AP4_EsdsAtom::AP4_EsdsAtom
+---------------------------------------------------------------------*/
AP4_EsdsAtom::AP4_EsdsAtom(AP4_UI32        size,
                           AP4_UI32        version,
                           AP4_UI32        flags,
                           AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_ESDS, size, version, flags)
{
    // read descriptor
    AP4_Descriptor* descriptor = NULL;
    if(AP4_DescriptorFactory::CreateDescriptorFromStream(stream, descriptor)
       == AP4_SUCCESS)
    {
        m_EsDescriptor = AP4_DYNAMIC_CAST(AP4_EsDescriptor, descriptor);
    }
    else
    {
        m_EsDescriptor = NULL;
    }
}

/*----------------------------------------------------------------------
|   AP4_EsdsAtom::~AP4_EsdsAtom
+---------------------------------------------------------------------*/
AP4_EsdsAtom::~AP4_EsdsAtom()
{
    delete m_EsDescriptor;
}

/*----------------------------------------------------------------------
|   AP4_EsdsAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_EsdsAtom::WriteFields(AP4_ByteStream& stream)
{
    // write the es descriptor
    if(m_EsDescriptor) return m_EsDescriptor->Write(stream);

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_EsdsAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_EsdsAtom::InspectFields(AP4_AtomInspector& inspector)
{
    // inspect descriptor
    if(m_EsDescriptor)
    {
        m_EsDescriptor->Inspect(inspector);
    }

    return AP4_SUCCESS;
}
