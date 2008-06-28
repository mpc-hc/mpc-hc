/*****************************************************************
|
|    AP4 - smhd Atoms 
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
#include "Ap4SmhdAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|       AP4_SmhdAtom::AP4_SmhdAtom
+---------------------------------------------------------------------*/
AP4_SmhdAtom::AP4_SmhdAtom(AP4_UI16 balance) :
    AP4_Atom(AP4_ATOM_TYPE_SMHD, 4+AP4_FULL_ATOM_HEADER_SIZE, true),
    m_Balance(balance)
{
    m_Reserved = 0;
}

/*----------------------------------------------------------------------
|       AP4_SmhdAtom::AP4_SmhdAtom
+---------------------------------------------------------------------*/
AP4_SmhdAtom::AP4_SmhdAtom(AP4_Size size, AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_SMHD, size, true, stream)
{
    stream.ReadUI16(m_Balance);
    stream.ReadUI16(m_Reserved);
}

/*----------------------------------------------------------------------
|       AP4_SmhdAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_SmhdAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    // balance
    result = stream.WriteUI16(m_Balance);
    if (AP4_FAILED(result)) return result;

    // reserved
    result = stream.WriteUI16(m_Reserved);
    if (AP4_FAILED(result)) return result;

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_SmhdAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_SmhdAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("balance", m_Balance);

    return AP4_SUCCESS;
}
