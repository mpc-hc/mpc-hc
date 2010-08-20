/*****************************************************************
|
|    AP4 - stss Atoms 
|
|    Copyright 2003 Gilles Boccon-Gibod & Julien Boeuf
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

#ifndef _AP4_STSS_ATOM_H_
#define _AP4_STSS_ATOM_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4.h"
#include "Ap4ByteStream.h"
#include "Ap4Array.h"
#include "Ap4Atom.h"

/*----------------------------------------------------------------------
|       AP4_StssAtom
+---------------------------------------------------------------------*/
class AP4_StssAtom : public AP4_Atom
{
 public:
    // methods
    AP4_StssAtom(AP4_Size size, AP4_ByteStream& stream);
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual bool       IsSampleSync(AP4_Ordinal sample);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);

    AP4_Array<AP4_UI32> m_Entries; // FIXME
 private:
};

#endif // _AP4_STSS_ATOM_H_
