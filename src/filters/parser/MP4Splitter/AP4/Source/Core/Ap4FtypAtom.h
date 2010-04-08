/*****************************************************************
|
|    AP4 - ftyp Atoms
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

#ifndef _AP4_FTYP_ATOM_H_
#define _AP4_FTYP_ATOM_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4Atom.h"
#include "Ap4Array.h"

/*----------------------------------------------------------------------
|   class references
+---------------------------------------------------------------------*/
class AP4_ByteStream;

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const AP4_UI32 AP4_FTYP_BRAND_ISOM = AP4_ATOM_TYPE('i', 's', 'o', 'm');

/*----------------------------------------------------------------------
|   AP4_FtypAtom
+---------------------------------------------------------------------*/
class AP4_FtypAtom : public AP4_Atom
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_FtypAtom, AP4_Atom)

    // class methods
    static AP4_FtypAtom* Create(AP4_Size size, AP4_ByteStream& stream)
    {
        return new AP4_FtypAtom(size, stream);
    }

    // methods
    AP4_FtypAtom(AP4_UI32     major_brand,
                 AP4_UI32     minor_version,
                 AP4_UI32*    compatible_brands = NULL,
                 AP4_Cardinal compatible_brand_count = 0);
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);

    // accessors
    AP4_UI32 GetMajorBrand()
    {
        return m_MajorBrand;
    }
    AP4_UI32 GetMinorVersion()
    {
        return m_MinorVersion;
    }
    AP4_Array<AP4_UI32>& GetCompatibleBrands()
    {
        return m_CompatibleBrands;
    }
    bool HasCompatibleBrand(AP4_UI32 brand);

private:
    // methods
    AP4_FtypAtom(AP4_UI32 size, AP4_ByteStream& stream);

    // members
    AP4_UI32            m_MajorBrand;
    AP4_UI32            m_MinorVersion;
    AP4_Array<AP4_UI32> m_CompatibleBrands;
};

#endif // _AP4_FTYP_ATOM_H_
