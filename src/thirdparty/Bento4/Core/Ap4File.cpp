/*****************************************************************
|
|    AP4 - File
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
#include "Ap4File.h"
#include "Ap4Atom.h"
#include "Ap4TrakAtom.h"
#include "Ap4MoovAtom.h"
#include "Ap4MvhdAtom.h"
#include "Ap4AtomFactory.h"
/*----------------------------------------------------------------------
|       AP4_File::AP4_File
+---------------------------------------------------------------------*/
AP4_File::AP4_File(AP4_Movie* movie) :
    m_Movie(movie)
{
}

/*----------------------------------------------------------------------
|       AP4_File::AP4_File
+---------------------------------------------------------------------*/
AP4_File::AP4_File(AP4_ByteStream& stream, AP4_AtomFactory& atom_factory) :
    m_Movie(NULL)
{
    // get all atoms
    AP4_Atom* atom;
    while (AP4_SUCCEEDED(atom_factory.CreateAtomFromStream(stream, atom))) {
        switch (atom->GetType()) {
            case AP4_ATOM_TYPE_MOOV:
                m_Movie = new AP4_Movie(dynamic_cast<AP4_MoovAtom*>(atom),
                                        stream);
                break;
            case AP4_ATOM_TYPE_FTYP:
                //m_Movie = new AP4_Movie(dynamic_cast<AP4_FtypAtom*>(atom),  stream);
                m_FileType = dynamic_cast<AP4_FtypAtom*>(atom);
            default:
                m_OtherAtoms.Add(atom);
        }
    }
}
    
/*----------------------------------------------------------------------
|       AP4_File::~AP4_File
+---------------------------------------------------------------------*/
AP4_File::~AP4_File()
{
    delete m_Movie;
    m_OtherAtoms.DeleteReferences();
}

/*----------------------------------------------------------------------
|       AP4_File::Inspect
+---------------------------------------------------------------------*/
AP4_Result
AP4_File::Inspect(AP4_AtomInspector& inspector)
{
    // dump the moov atom first
    if (m_Movie) m_Movie->Inspect(inspector);

    // dump the other atoms
    m_OtherAtoms.Apply(AP4_AtomListInspector(inspector));

    return AP4_SUCCESS;
}
