/*****************************************************************
|
|    AP4 - ohdr Atom
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

#ifndef _AP4_OHDR_ATOM_H_
#define _AP4_OHDR_ATOM_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4Atom.h"
#include "Ap4ContainerAtom.h"
#include "Ap4String.h"
#include "Ap4OmaDcf.h"

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const AP4_UI08 AP4_OMA_DCF_ENCRYPTION_METHOD_NULL    = 0;
const AP4_UI08 AP4_OMA_DCF_ENCRYPTION_METHOD_AES_CBC = 1;
const AP4_UI08 AP4_OMA_DCF_ENCRYPTION_METHOD_AES_CTR = 2;

const AP4_UI08 AP4_OMA_DCF_PADDING_SCHEME_NONE     = 0;
const AP4_UI08 AP4_OMA_DCF_PADDING_SCHEME_RFC_2630 = 1;

/*----------------------------------------------------------------------
|   AP4_OhdrAtom
+---------------------------------------------------------------------*/
class AP4_OhdrAtom : public AP4_ContainerAtom, public AP4_OmaDrmInfo
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D2(AP4_OhdrAtom, AP4_ContainerAtom, AP4_OmaDrmInfo)

    // class methods
    static AP4_OhdrAtom* Create(AP4_Size         size, 
                                AP4_ByteStream&  stream, 
                                AP4_AtomFactory& atom_factory);

    // constructor
    AP4_OhdrAtom(AP4_UI08        encryption_method, 
                 AP4_UI08        padding_scheme,
                 AP4_UI64        plaintext_length,
                 const char*     content_id,
                 const char*     rights_issuer_url,
                 const AP4_Byte* textual_headers,
                 AP4_Size        textual_headers_size);

    // methods
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    virtual AP4_Atom*  Clone();

    // accessors
    AP4_UI08          GetEncryptionMethod()   const { return m_EncryptionMethod; } 
    void              SetEncryptionMethod(AP4_UI08 encryption_method) { m_EncryptionMethod = encryption_method; }
    AP4_UI08          GetPaddingScheme()      const { return m_PaddingScheme;    }
    void              SetPaddingScheme(AP4_UI08 padding_scheme) { m_PaddingScheme = padding_scheme; }
    AP4_UI64          GetPlaintextLength()    const { return m_PlaintextLength;  }
    
    // AP4_OmaDrmInfo implementation
    const AP4_String& GetContentId()          const { return m_ContentId;        }
    const AP4_String& GetRightsIssuerUrl()    const { return m_RightsIssuerUrl;  }
    const AP4_DataBuffer& GetTextualHeaders() const { return m_TextualHeaders;   }

private:
    // methods
    AP4_OhdrAtom(AP4_UI32         size, 
                 AP4_UI32         version,
                 AP4_UI32         flags,
                 AP4_ByteStream&  stream,
                 AP4_AtomFactory& atom_factory);

    // members
    AP4_UI08       m_EncryptionMethod; 
    AP4_UI08       m_PaddingScheme;
    AP4_UI64       m_PlaintextLength;
    AP4_String     m_ContentId;
    AP4_String     m_RightsIssuerUrl;
    AP4_DataBuffer m_TextualHeaders;
};

#endif // _AP4_OHDR_ATOM_H_
