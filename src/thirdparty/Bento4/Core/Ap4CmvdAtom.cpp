/*****************************************************************
|
|    AP4 - cmvd Atoms 
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
#include "Ap4CmvdAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4ContainerAtom.h"

/*----------------------------------------------------------------------
|       AP4_CmvdAtom::AP4_CmvdAtom
+---------------------------------------------------------------------*/
AP4_CmvdAtom::AP4_CmvdAtom(AP4_Size         size,
                           AP4_ByteStream&  stream,
                           AP4_AtomFactory& atom_factory) :
    AP4_ContainerAtom(AP4_ATOM_TYPE_CMVD, size, false, stream)
{
	size -= AP4_ATOM_HEADER_SIZE;

    stream.ReadUI32(m_MovieResourceSize);

	size -= 4;

	m_Data.SetDataSize(size);
	stream.Read(m_Data.UseData(), size);

/*
    // read children
    AP4_Size bytes_available = size-AP4_FULL_ATOM_HEADER_SIZE-4;
    while (entry_count--) {
        AP4_Atom* atom; 
        while (AP4_SUCCEEDED(atom_factory.CreateAtomFromStream(stream, 
                                                               bytes_available,
                                                               atom,
															   this))) {
            m_Children.Add(atom);
        }
    }
*/
}
