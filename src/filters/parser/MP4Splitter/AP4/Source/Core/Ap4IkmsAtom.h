/*****************************************************************
|
|    AP4 - iKMS Atom
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

#ifndef _AP4_IKMS_ATOM_H_
#define _AP4_IKMS_ATOM_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4Atom.h"
#include "Ap4String.h"

/*----------------------------------------------------------------------
|   AP4_IkmsAtom
+---------------------------------------------------------------------*/
class AP4_IkmsAtom : public AP4_Atom
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_IkmsAtom, AP4_Atom)

    // class methods
    static AP4_IkmsAtom* Create(AP4_Size size, AP4_ByteStream& stream);

    // methods
    AP4_IkmsAtom(const char* kms_uri,
                 AP4_UI32    kms_id = 0,
                 AP4_UI32    kms_version = 0);
    virtual AP4_Atom*  Clone();
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);

    // accessors
    const AP4_String& GetKmsUri()     { return m_KmsUri;     }
    AP4_UI32          GetKmsId()      { return m_KmsId;      }
    AP4_UI32          GetKmsVersion() { return m_KmsVersion; }

private:
    // methods
    AP4_IkmsAtom(AP4_UI32        size, 
                 AP4_UI32        version,
                 AP4_UI32        flags,
                 AP4_ByteStream& stream);

    // members
    AP4_String m_KmsUri;
    AP4_UI32   m_KmsId;
    AP4_UI32   m_KmsVersion;
};

#endif // _AP4_IKMS_ATOM_H_
