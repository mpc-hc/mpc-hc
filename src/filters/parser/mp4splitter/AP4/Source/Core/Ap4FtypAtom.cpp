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

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4FtypAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_FtypAtom)

/*----------------------------------------------------------------------
|   AP4_FtypAtom::AP4_FtypAtom
+---------------------------------------------------------------------*/
AP4_FtypAtom::AP4_FtypAtom(AP4_UI32 size, AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_FTYP, size)
{
    stream.ReadUI32(m_MajorBrand);
    stream.ReadUI32(m_MinorVersion);
    size -= 16;
    while (size) {
        AP4_UI32 compatible_brand;
        stream.ReadUI32(compatible_brand);
        m_CompatibleBrands.Append(compatible_brand);
        size -= 4;
    }
}

/*----------------------------------------------------------------------
|   AP4_FtypAtom::AP4_FtypAtom
+---------------------------------------------------------------------*/
AP4_FtypAtom::AP4_FtypAtom(AP4_UI32     major_brand,
                           AP4_UI32     minor_version,
                           AP4_UI32*    compatible_brands,
                           AP4_Cardinal compatible_brand_count) :
    AP4_Atom(AP4_ATOM_TYPE_FTYP, AP4_ATOM_HEADER_SIZE+8+4*compatible_brand_count),
    m_MajorBrand(major_brand),
    m_MinorVersion(minor_version),
    m_CompatibleBrands(compatible_brands, compatible_brand_count)
{
}

/*----------------------------------------------------------------------
|   AP4_FtypAtom::HasCompatibleBrand
+---------------------------------------------------------------------*/
bool
AP4_FtypAtom::HasCompatibleBrand(AP4_UI32 brand)
{
    for (unsigned int i=0; i<m_CompatibleBrands.ItemCount(); i++) {
        if (m_CompatibleBrands[i] == brand) return true;
    }

    return false;
}

/*----------------------------------------------------------------------
|   AP4_FtypAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_FtypAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;
   
    // major brand
    result = stream.WriteUI32(m_MajorBrand);
    if (AP4_FAILED(result)) return result;

    // minor version
    result = stream.WriteUI32(m_MinorVersion);
    if (AP4_FAILED(result)) return result;

    // compatible brands
    AP4_Cardinal compat_brand_count = m_CompatibleBrands.ItemCount();
    for (AP4_Ordinal i=0; i<compat_brand_count; i++) {
        result = stream.WriteUI32(m_CompatibleBrands[i]);
        if (AP4_FAILED(result)) return result;
    }

    return result;
}

/*----------------------------------------------------------------------
|   AP4_FtypAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_FtypAtom::InspectFields(AP4_AtomInspector& inspector)
{
    char name[5];
    AP4_FormatFourChars(name, m_MajorBrand);
    inspector.AddField("major_brand", name);
    inspector.AddField("minor_version", m_MinorVersion, AP4_AtomInspector::HINT_HEX);

    // compatible brands
    for (unsigned int i=0; i<m_CompatibleBrands.ItemCount(); i++) {
        AP4_UI32 cb = m_CompatibleBrands[i];
        AP4_FormatFourChars(name, cb);
        inspector.AddField("compatible_brand", name);
    }

    return AP4_SUCCESS;
}
