/*****************************************************************
|
|    AP4 - vmhd Atoms
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
#include "Ap4VmhdAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"
#include "Ap4Types.h"

/*----------------------------------------------------------------------
|   AP4_VmhdAtom::Create
+---------------------------------------------------------------------*/
AP4_VmhdAtom*
AP4_VmhdAtom::Create(AP4_Size size, AP4_ByteStream& stream)
{
    AP4_UI32 version;
    AP4_UI32 flags;
    if(AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if(version != 0) return NULL;
    return new AP4_VmhdAtom(size, version, flags, stream);
}

/*----------------------------------------------------------------------
|   AP4_VmhdAtom::AP4_VmhdAtom
+---------------------------------------------------------------------*/
AP4_VmhdAtom::AP4_VmhdAtom(AP4_UI16 graphics_mode, AP4_UI16 r, AP4_UI16 g, AP4_UI16 b) :
    AP4_Atom(AP4_ATOM_TYPE_VMHD, AP4_FULL_ATOM_HEADER_SIZE + 8, 0, 0),
    m_GraphicsMode(graphics_mode)
{
    m_OpColor[0] = r;
    m_OpColor[1] = g;
    m_OpColor[2] = b;
}

/*----------------------------------------------------------------------
|   AP4_VmhdAtom::AP4_VmhdAtom
+---------------------------------------------------------------------*/
AP4_VmhdAtom::AP4_VmhdAtom(AP4_UI32        size,
                           AP4_UI32        version,
                           AP4_UI32        flags,
                           AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_VMHD, size, version, flags)
{
    stream.ReadUI16(m_GraphicsMode);
    stream.Read(m_OpColor, sizeof(m_OpColor));
}

/*----------------------------------------------------------------------
|   AP4_VmhdAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_VmhdAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    // graphics mode
    result = stream.WriteUI16(m_GraphicsMode);
    if(AP4_FAILED(result)) return result;

    // op color
    result = stream.Write(m_OpColor, sizeof(m_OpColor));
    if(AP4_FAILED(result)) return result;

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_VmhdAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_VmhdAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("graphics_mode", m_GraphicsMode);
    char formatted[16];
    AP4_FormatString(formatted, sizeof(formatted), "%04x,%04x,%04x",
                     m_OpColor[0], m_OpColor[1], m_OpColor[2]);
    inspector.AddField("op_color", formatted);

    return AP4_SUCCESS;
}
