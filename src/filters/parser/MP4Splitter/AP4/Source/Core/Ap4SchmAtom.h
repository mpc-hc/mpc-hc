/*****************************************************************
|
|    AP4 - schm Atoms 
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

#ifndef _AP4_SCHM_ATOM_H_
#define _AP4_SCHM_ATOM_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4String.h"
#include "Ap4Atom.h"
#include "Ap4Array.h"

/*----------------------------------------------------------------------
|   class references
+---------------------------------------------------------------------*/
class AP4_ByteStream;

/*----------------------------------------------------------------------
|   AP4_SchmAtom
+---------------------------------------------------------------------*/
class AP4_SchmAtom : public AP4_Atom
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_SchmAtom, AP4_Atom)

    // class methods
    static AP4_SchmAtom* Create(AP4_Size                   size, 
                                AP4_Array<AP4_Atom::Type>* context,
                                AP4_ByteStream&            stream);

    // constructors
    AP4_SchmAtom(AP4_UI32    scheme_type,
                 AP4_UI32    scheme_version,
                 const char* scheme_uri = NULL,
                 bool        short_form = false);

    // methods
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);

    // accessors
    AP4_UI32    GetSchemeType()    { return m_SchemeType;    }
    AP4_UI32    GetSchemeVersion() { return m_SchemeVersion; }
    AP4_String& GetSchemeUri()     { return m_SchemeUri;     }

private:
    // methods
    AP4_SchmAtom(AP4_UI32        size, 
                 AP4_UI32        version,
                 AP4_UI32        flags,
                 bool            short_form,
                 AP4_ByteStream& stream);

    // members
    bool       m_AtomHasShortForm; // for versions of this where the version
                                   // field is only 16 bits
    AP4_UI32   m_SchemeType;
    AP4_UI32   m_SchemeVersion;
    AP4_String m_SchemeUri;
};

#endif // _AP4_SCHM_ATOM_H_
