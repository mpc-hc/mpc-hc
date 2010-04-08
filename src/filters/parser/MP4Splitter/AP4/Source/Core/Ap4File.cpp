/*****************************************************************
|
|    AP4 - File
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
#include "Ap4File.h"
#include "Ap4Atom.h"
#include "Ap4TrakAtom.h"
#include "Ap4MoovAtom.h"
#include "Ap4MvhdAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Movie.h"
#include "Ap4FtypAtom.h"
#include "Ap4MetaData.h"

/*----------------------------------------------------------------------
|   AP4_File::AP4_File
+---------------------------------------------------------------------*/
AP4_File::AP4_File(AP4_Movie* movie) :
    m_Movie(movie),
    m_FileType(NULL),
    m_MetaData(NULL),
    m_MoovIsBeforeMdat(true)
{
}

/*----------------------------------------------------------------------
|   AP4_File::AP4_File
+---------------------------------------------------------------------*/
AP4_File::AP4_File(AP4_ByteStream&  stream,
                   AP4_AtomFactory& atom_factory,
                   bool             moov_only) :
    m_Movie(NULL),
    m_FileType(NULL),
    m_MetaData(NULL),
    m_MoovIsBeforeMdat(true)
{
    // parse top-level atoms
    AP4_Atom*    atom;
    AP4_Position stream_position;
    bool         keep_parsing = true;
    while(keep_parsing &&
          AP4_SUCCEEDED(stream.Tell(stream_position)) &&
          AP4_SUCCEEDED(atom_factory.CreateAtomFromStream(stream, atom)))
    {
        AddChild(atom);
        switch(atom->GetType())
        {
        case AP4_ATOM_TYPE_MOOV:
            m_Movie = new AP4_Movie(AP4_DYNAMIC_CAST(AP4_MoovAtom, atom), stream, false);
            if(moov_only) keep_parsing = false;
            break;

        case AP4_ATOM_TYPE_FTYP:
            m_FileType = AP4_DYNAMIC_CAST(AP4_FtypAtom, atom);
            break;

        case AP4_ATOM_TYPE_MDAT:
            // see if we are before the moov atom
            if(m_Movie == NULL) m_MoovIsBeforeMdat = false;
            break;
        }
    }
}

/*----------------------------------------------------------------------
|   AP4_File::~AP4_File
+---------------------------------------------------------------------*/
AP4_File::~AP4_File()
{
    delete m_Movie;
    delete m_MetaData;
}

/*----------------------------------------------------------------------
|   AP4_File::Inspect
+---------------------------------------------------------------------*/
AP4_Result
AP4_File::Inspect(AP4_AtomInspector& inspector)
{
    // dump the moov atom first
    if(m_Movie) m_Movie->Inspect(inspector);

    // dump the other atoms
    m_Children.Apply(AP4_AtomListInspector(inspector));

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_File::SetFileType
+---------------------------------------------------------------------*/
AP4_Result
AP4_File::SetFileType(AP4_UI32     major_brand,
                      AP4_UI32     minor_version,
                      AP4_UI32*    compatible_brands,
                      AP4_Cardinal compatible_brand_count)
{
    if(m_FileType)
    {
        RemoveChild(m_FileType);
        delete m_FileType;
    }
    m_FileType = new AP4_FtypAtom(major_brand,
                                  minor_version,
                                  compatible_brands,
                                  compatible_brand_count);
    AddChild(m_FileType, 0);

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_File::GetMetaData
+---------------------------------------------------------------------*/
const AP4_MetaData*
AP4_File::GetMetaData()
{
    if(m_MetaData == NULL)
    {
        m_MetaData = new AP4_MetaData(this);
    }

    return m_MetaData;
}

