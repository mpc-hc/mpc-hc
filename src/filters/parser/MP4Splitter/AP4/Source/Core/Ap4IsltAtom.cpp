/*****************************************************************
|
|    AP4 - iSLT Atom 
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
#include "Ap4Utils.h"
#include "Ap4IsltAtom.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_IsltAtom)

/*----------------------------------------------------------------------
|   AP4_IsltAtom::AP4_IsltAtom
+---------------------------------------------------------------------*/
AP4_IsltAtom::AP4_IsltAtom(const AP4_UI08* salt) :
    AP4_Atom(AP4_ATOM_TYPE_ISLT, AP4_ATOM_HEADER_SIZE+8)
{
    for (unsigned int i=0; i<8; i++) {
        m_Salt[i] = salt[i];
    }
}

/*----------------------------------------------------------------------
|   AP4_IsltAtom::AP4_IsltAtom
+---------------------------------------------------------------------*/
AP4_IsltAtom::AP4_IsltAtom(AP4_UI32 size, AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_ISLT, size)
{
    stream.Read((void*)m_Salt, 8);
}

/*----------------------------------------------------------------------
|   AP4_IsltAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_IsltAtom::WriteFields(AP4_ByteStream& stream)
{
    return stream.Write((const void*)m_Salt, 8);
}

/*----------------------------------------------------------------------
|   AP4_IsltAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_IsltAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("salt", m_Salt, 8);

    return AP4_SUCCESS;
}
