/*****************************************************************
|
|    AP4 - mehd Atoms
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
#include "Ap4MehdAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_MehdAtom)

/*----------------------------------------------------------------------
|   AP4_MehdAtom::Create
+---------------------------------------------------------------------*/
AP4_MehdAtom*
AP4_MehdAtom::Create(AP4_Size size, AP4_ByteStream& stream)
{
    AP4_UI32 version;
    AP4_UI32 flags;
    if(AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if(version > 1) return NULL;
    return new AP4_MehdAtom(size, version, flags, stream);
}

/*----------------------------------------------------------------------
|   AP4_MehdAtom::AP4_MehdAtom
+---------------------------------------------------------------------*/
AP4_MehdAtom::AP4_MehdAtom(AP4_UI64 duration) :
    AP4_Atom(AP4_ATOM_TYPE_MEHD, AP4_FULL_ATOM_HEADER_SIZE + 4, 0, 0),
    m_Duration(duration)
{
    if(duration > 0xFFFFFFFF)
    {
        m_Version = 1;
        m_Size32 += 4;
    }
}

/*----------------------------------------------------------------------
|   AP4_MehdAtom::AP4_MehdAtom
+---------------------------------------------------------------------*/
AP4_MehdAtom::AP4_MehdAtom(AP4_UI32        size,
                           AP4_UI32        version,
                           AP4_UI32        flags,
                           AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_MEHD, size, version, flags)
{
    if(m_Version == 0)
    {
        AP4_UI32 duration;
        stream.ReadUI32(duration);
        m_Duration = duration;
    }
    else
    {
        stream.ReadUI64(m_Duration);
    }
}

/*----------------------------------------------------------------------
|   AP4_MehdAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_MehdAtom::WriteFields(AP4_ByteStream& stream)
{
    if(m_Version == 0)
    {
        return stream.WriteUI32((AP4_UI32)m_Duration);
    }
    else
    {
        return stream.WriteUI64(m_Duration);
    }
}

/*----------------------------------------------------------------------
|   AP4_MehdAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_MehdAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("duration", m_Duration);

    return AP4_SUCCESS;
}
