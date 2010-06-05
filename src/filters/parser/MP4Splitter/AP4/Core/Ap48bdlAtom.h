/*****************************************************************
|
|    AP4 - 8bdl Atoms 
|
|    Copyright 2002-2009 Axiomatic Systems, LLC
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
 
 #ifndef _AP4_8BDL_ATOM_H_
 #define _AP4_8BDL_ATOM_H_
 
/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4DataBuffer.h"
#include "Ap4Atom.h"
#include "Ap4DynamicCast.h"

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const AP4_UI32 AP4_8BDL_XML_DATA_ENCODING = AP4_ATOM_TYPE('x','m','l',' ');

/*----------------------------------------------------------------------
|   AP4_8bdlAtom
+---------------------------------------------------------------------*/
class AP4_8bdlAtom : public AP4_Atom
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_8bdlAtom, AP4_Atom)

    // virtual constructor
    static AP4_8bdlAtom* Create(AP4_Size size, AP4_ByteStream& stream);
    
    // constructors
    AP4_8bdlAtom(AP4_UI32        encoding,
                 AP4_UI32        encoding_version,
                 const AP4_Byte* data,
                 AP4_Size        data_size);

    // methods
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);

    // accessors
    AP4_UI32              GetEncoding()        { return m_Encoding; }
    AP4_UI32              GetEncodingVersion() { return m_EncodingVersion; }
    const AP4_DataBuffer& GetBundleData()      { return m_BundleData; }

private:
    // methods
    AP4_8bdlAtom(AP4_Size        size,
                 AP4_ByteStream& stream);

    // members
    AP4_UI32       m_Encoding;
    AP4_UI32       m_EncodingVersion;
    AP4_DataBuffer m_BundleData;
};

#endif // _AP4_8BDL_ATOM_H_
