/*****************************************************************
|
|    AP4 - nmhd Atoms 
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
#include "Ap4NmhdAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|       AP4_NmhdAtom::AP4_NmhdAtom
+---------------------------------------------------------------------*/
AP4_NmhdAtom::AP4_NmhdAtom() :
    AP4_Atom(AP4_ATOM_TYPE_NMHD, AP4_FULL_ATOM_HEADER_SIZE, true)
{
}

/*----------------------------------------------------------------------
|       AP4_NmhdAtom::AP4_NmhdAtom
+---------------------------------------------------------------------*/
AP4_NmhdAtom::AP4_NmhdAtom(AP4_Size size, AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_NMHD, size, true, stream)
{
}

/*----------------------------------------------------------------------
|       AP4_NmhdAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_NmhdAtom::WriteFields(AP4_ByteStream& stream)
{
    // not implemented yet
    return AP4_SUCCESS;
}
