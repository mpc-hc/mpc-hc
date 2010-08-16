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

#ifndef _AP4_FILE_H_
#define _AP4_FILE_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4.h"
#include "Ap4Atom.h"
#include "Ap4Track.h"
#include "Ap4List.h"
#include "Ap4Movie.h"
#include "Ap4ByteStream.h"
#include "Ap4AtomFactory.h"
#include "Ap4FtypAtom.h"

/*----------------------------------------------------------------------
|       AP4_File
+---------------------------------------------------------------------*/
class AP4_File {
public:
    // constructors and destructor
    AP4_File(AP4_Movie* movie);
    AP4_File(AP4_ByteStream& stream, 
             AP4_AtomFactory& atom_factory = AP4_AtomFactory::DefaultFactory);
    virtual ~AP4_File();

    // methods
    AP4_List<AP4_Atom>& GetOtherAtoms() { return m_OtherAtoms;}
    AP4_Movie*          GetMovie() { return m_Movie; }
    AP4_FtypAtom*       GetFileType()   { return m_FileType;  }
    virtual AP4_Result  Inspect(AP4_AtomInspector& inspector);

private:
    // members
    AP4_Movie*         m_Movie;
    AP4_FtypAtom*      m_FileType;
    AP4_List<AP4_Atom> m_OtherAtoms;
};

#endif // _AP4_FILE_H_
