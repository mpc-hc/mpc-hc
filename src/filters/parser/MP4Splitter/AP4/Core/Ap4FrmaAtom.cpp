/*****************************************************************
|
|    AP4 - frma Atoms 
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
#include "Ap4FrmaAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|       AP4_FrmaAtom::AP4_FrmaAtom
+---------------------------------------------------------------------*/
AP4_FrmaAtom::AP4_FrmaAtom(AP4_UI32 original_format) :
    AP4_Atom(AP4_ATOM_TYPE_FRMA, AP4_ATOM_HEADER_SIZE+4, false),
    m_OriginalFormat(original_format)
{
}

/*----------------------------------------------------------------------
|       AP4_FrmaAtom::AP4_FrmaAtom
+---------------------------------------------------------------------*/
AP4_FrmaAtom::AP4_FrmaAtom(AP4_Size size, AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_FRMA, size, false, stream)
{
    stream.ReadUI32(m_OriginalFormat);
}

/*----------------------------------------------------------------------
|       AP4_FrmaAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_FrmaAtom::WriteFields(AP4_ByteStream& stream)
{
    // write the original format
    return stream.WriteUI32(m_OriginalFormat);
}

/*----------------------------------------------------------------------
|       AP4_FrmaAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_FrmaAtom::InspectFields(AP4_AtomInspector& inspector)
{
    char format[5];
    AP4_FormatFourChars(format, m_OriginalFormat);
    inspector.AddField("original_format", format);

    return AP4_SUCCESS;
}
