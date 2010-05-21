/*****************************************************************
|
|    AP4 - UUID Atoms 
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
/**
* @file 
* @brief UUID Atoms
*/

#ifndef _AP4_UUID_ATOM_H_
#define _AP4_UUID_ATOM_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Atom.h"
#include "Ap4ByteStream.h"

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const AP4_UI32 AP4_UUID_ATOM_HEADER_SIZE      = AP4_ATOM_HEADER_SIZE+16;
const AP4_UI32 AP4_FULL_UUID_ATOM_HEADER_SIZE = AP4_FULL_ATOM_HEADER_SIZE+16;

/*----------------------------------------------------------------------
|   AP4_UuidAtom
+---------------------------------------------------------------------*/
/**
 * Base class for uuid atoms.
 */
class AP4_UuidAtom : public AP4_Atom {
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_UuidAtom, AP4_Atom)

    // constructor and destructor
    virtual ~AP4_UuidAtom() {};

    // methods
    virtual AP4_Size   GetHeaderSize() const;
    virtual AP4_Result WriteHeader(AP4_ByteStream& stream);
    virtual AP4_Result InspectHeader(AP4_AtomInspector& inspector);

    // accessors
    const AP4_UI08* GetUuid() { return m_Uuid; }
    
protected:
    // members
    AP4_UuidAtom(AP4_UI64 size, const AP4_UI08* uuid);
    AP4_UuidAtom(AP4_UI64 size, const AP4_UI08* uuid, AP4_UI32 version, AP4_UI32 flags);
    AP4_UuidAtom(AP4_UI64 size, bool is_full, AP4_ByteStream& stream);
    AP4_UI08 m_Uuid[16];
};

/*----------------------------------------------------------------------
|   AP4_UnknownUuidAtom
+---------------------------------------------------------------------*/
/**
 * Unknown uuid atoms.
 */
class AP4_UnknownUuidAtom : public AP4_UuidAtom {
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_UnknownUuidAtom, AP4_UuidAtom)

    // constructors
    AP4_UnknownUuidAtom(AP4_UI64 size, AP4_ByteStream& stream);
    AP4_UnknownUuidAtom(AP4_UI64 size, const AP4_UI08* uuid, AP4_ByteStream& stream);

    // methods
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);

protected:
    // members
    AP4_DataBuffer m_Data;
};

#endif // _AP4_UUID_ATOM_H_
