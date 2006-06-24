/*****************************************************************
|
|    AP4 - moov Atoms 
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

#ifndef _AP4_MOOV_ATOM_H_
#define _AP4_MOOV_ATOM_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4.h"
#include "Ap4ByteStream.h"
#include "Ap4List.h"
#include "Ap4Atom.h"
#include "Ap4TrakAtom.h"
#include "Ap4ContainerAtom.h"
#include "Ap4AtomFactory.h"

/*----------------------------------------------------------------------
|       AP4_MoovAtom
+---------------------------------------------------------------------*/
class AP4_MoovAtom : public AP4_ContainerAtom
{
public:
    // methods
    AP4_MoovAtom();
    AP4_MoovAtom(AP4_Size         size,
                 AP4_ByteStream&  stream,
                 AP4_AtomFactory& atom_factory);
    AP4_List<AP4_TrakAtom>& GetTrakAtoms() {
        return m_TrakAtoms;
    }
    AP4_UI32 GetTimeScale() {
        return m_TimeScale;
    }
    
    // AP4_AtomParent methods
    void OnChildAdded(AP4_Atom* atom);
    void OnChildRemoved(AP4_Atom* atom);

private:
    // members
    AP4_List<AP4_TrakAtom> m_TrakAtoms;
    AP4_UI32               m_TimeScale;
};

#endif // _AP4_MOOV_ATOM_H_
