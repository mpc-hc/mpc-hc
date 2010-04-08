/*****************************************************************
|
|    AP4 - tfra Atoms
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
#include "Ap4MfroAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   AP4_MfroAtom::Create
+---------------------------------------------------------------------*/
AP4_MfroAtom*
AP4_MfroAtom::Create(AP4_Size size, AP4_ByteStream& stream)
{
    AP4_UI32 version = 0;
    AP4_UI32 flags   = 0;
    AP4_Result result = ReadFullHeader(stream, version, flags);
    if(AP4_FAILED(result)) return NULL;
    if(version != 0) return NULL;
    return new AP4_MfroAtom(size, version, flags, stream);
}

/*----------------------------------------------------------------------
|   AP4_MfroAtom::AP4_MfroAtom
+---------------------------------------------------------------------*/
AP4_MfroAtom::AP4_MfroAtom(AP4_UI32 mfra_size) :
    AP4_Atom(AP4_ATOM_TYPE_MFRO, AP4_FULL_ATOM_HEADER_SIZE + 4),
    m_MfraSize(mfra_size)
{
}

/*----------------------------------------------------------------------
|   AP4_MfroAtom::AP4_MfroAtom
+---------------------------------------------------------------------*/
AP4_MfroAtom::AP4_MfroAtom(AP4_UI32        size,
                           AP4_UI32        version,
                           AP4_UI32        flags,
                           AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_MFRO, size, version, flags)
{
    stream.ReadUI32(m_MfraSize);
}

/*----------------------------------------------------------------------
|   AP4_MfroAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_MfroAtom::WriteFields(AP4_ByteStream& stream)
{
    return stream.WriteUI32(m_MfraSize);
}

/*----------------------------------------------------------------------
|   AP4_MfroAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_MfroAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("mfra_size", m_MfraSize);

    return AP4_SUCCESS;
}
