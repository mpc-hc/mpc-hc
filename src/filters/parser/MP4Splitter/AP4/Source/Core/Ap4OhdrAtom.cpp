/*****************************************************************
|
|    AP4 - ohdr Atoms
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
#include "Ap4Utils.h"
#include "Ap4OhdrAtom.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_OhdrAtom)

/*----------------------------------------------------------------------
|   AP4_OhdrAtom::Create
+---------------------------------------------------------------------*/
AP4_OhdrAtom*
AP4_OhdrAtom::Create(AP4_Size         size,
                     AP4_ByteStream&  stream,
                     AP4_AtomFactory& atom_factory)
{
    AP4_UI32 version;
    AP4_UI32 flags;
    if(AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if(version != 0) return NULL;
    return new AP4_OhdrAtom(size, version, flags, stream, atom_factory);
}

/*----------------------------------------------------------------------
|   AP4_OhdrAtom::AP4_OhdrAtom
+---------------------------------------------------------------------*/
AP4_OhdrAtom::AP4_OhdrAtom(AP4_UI08        encryption_method,
                           AP4_UI08        padding_scheme,
                           AP4_UI64        plaintext_length,
                           const char*     content_id,
                           const char*     rights_issuer_url,
                           const AP4_Byte* textual_headers,
                           AP4_Size        textual_headers_size) :
    AP4_ContainerAtom(AP4_ATOM_TYPE_OHDR, (AP4_UI32)0, (AP4_UI32)0),
    m_EncryptionMethod(encryption_method),
    m_PaddingScheme(padding_scheme),
    m_PlaintextLength(plaintext_length),
    m_ContentId(content_id),
    m_RightsIssuerUrl(rights_issuer_url),
    m_TextualHeaders(textual_headers, textual_headers_size)
{
    m_Size32 += 1 + 1 + 8 + 2 + 2 + 2 + m_ContentId.GetLength() + m_RightsIssuerUrl.GetLength() + textual_headers_size;
}

/*----------------------------------------------------------------------
|   AP4_OhdrAtom::AP4_OhdrAtom
+---------------------------------------------------------------------*/
AP4_OhdrAtom::AP4_OhdrAtom(AP4_UI32         size,
                           AP4_UI32         version,
                           AP4_UI32         flags,
                           AP4_ByteStream&  stream,
                           AP4_AtomFactory& atom_factory) :
    AP4_ContainerAtom(AP4_ATOM_TYPE_OHDR, size, false, version, flags)
{
    // encryption method
    stream.ReadUI08(m_EncryptionMethod);

    // padding scheme
    stream.ReadUI08(m_PaddingScheme);

    // plaintext length
    stream.ReadUI64(m_PlaintextLength);

    // string lengths
    AP4_UI16 content_id_length;
    AP4_UI16 rights_issuer_url_length;
    AP4_UI16 textual_headers_length;
    stream.ReadUI16(content_id_length);
    stream.ReadUI16(rights_issuer_url_length);
    stream.ReadUI16(textual_headers_length);

    // content id
    char* buffer = new char[content_id_length];
    stream.Read(buffer, content_id_length);
    m_ContentId.Assign(buffer, content_id_length);
    delete[] buffer;

    // rights issuer url
    buffer = new char[rights_issuer_url_length];
    stream.Read(buffer, rights_issuer_url_length);
    m_RightsIssuerUrl.Assign(buffer, rights_issuer_url_length);
    delete[] buffer;

    // textual headers
    buffer = new char[textual_headers_length];
    stream.Read(buffer, textual_headers_length);
    m_TextualHeaders.SetData((AP4_Byte*)buffer, textual_headers_length);
    delete[] buffer;

    // read the children
    AP4_Size bytes_used = AP4_FULL_ATOM_HEADER_SIZE + 1 + 1 + 8 + 2 + 2 + 2 + content_id_length + rights_issuer_url_length + textual_headers_length;
    if(bytes_used <= size)
    {
        ReadChildren(atom_factory, stream, size - bytes_used);
    }
}

/*----------------------------------------------------------------------
|   AP4_OhdrAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_OhdrAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_CHECK(stream.WriteUI08(m_EncryptionMethod));
    AP4_CHECK(stream.WriteUI08(m_PaddingScheme));
    AP4_CHECK(stream.WriteUI64(m_PlaintextLength));
    AP4_CHECK(stream.WriteUI16((AP4_UI16)m_ContentId.GetLength()));
    AP4_CHECK(stream.WriteUI16((AP4_UI16)m_RightsIssuerUrl.GetLength()));
    AP4_CHECK(stream.WriteUI16((AP4_UI16)m_TextualHeaders.GetDataSize()));
    AP4_CHECK(stream.Write(m_ContentId.GetChars(), m_ContentId.GetLength()));
    AP4_CHECK(stream.Write(m_RightsIssuerUrl.GetChars(), m_RightsIssuerUrl.GetLength()));
    AP4_CHECK(stream.Write(m_TextualHeaders.GetData(), m_TextualHeaders.GetDataSize()));

    // write the children
    return m_Children.Apply(AP4_AtomListWriter(stream));
}

/*----------------------------------------------------------------------
|   AP4_OhdrAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_OhdrAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("encryption_method", m_EncryptionMethod);
    inspector.AddField("padding_scheme",    m_PaddingScheme);
    inspector.AddField("plaintext_length", (AP4_UI32)m_PlaintextLength);
    inspector.AddField("content_id",        m_ContentId.GetChars());
    inspector.AddField("rights_issuer_url", m_RightsIssuerUrl.GetChars());

    {
        AP4_DataBuffer output_buffer;
        AP4_Result     result;

        result = output_buffer.Reserve(1 + m_TextualHeaders.GetDataSize());
        if(AP4_FAILED(result))
        {
            inspector.AddField("textual_headers",
                               m_TextualHeaders.UseData(),
                               m_TextualHeaders.GetDataSize(),
                               AP4_AtomInspector::HINT_HEX);
        }
        else
        {
            AP4_Size       data_len    = m_TextualHeaders.GetDataSize();
            AP4_Byte*      textual_headers_string;
            AP4_Byte*      curr;

            output_buffer.SetData((const AP4_Byte*)m_TextualHeaders.GetData(), m_TextualHeaders.GetDataSize());
            curr = textual_headers_string = output_buffer.UseData();
            textual_headers_string[m_TextualHeaders.GetDataSize()] = '\0';
            while(curr < textual_headers_string + data_len)
            {
                if('\0' == *curr)
                {
                    *curr = '\n';
                }
                curr++;
            }
            inspector.AddField("textual_headers", (const char*) textual_headers_string);
        }
    }

    return InspectChildren(inspector);
}

/*----------------------------------------------------------------------
|   AP4_OhdrAtom::Clone
+---------------------------------------------------------------------*/
AP4_Atom*
AP4_OhdrAtom::Clone()
{
    AP4_OhdrAtom* clone;
    clone = new AP4_OhdrAtom(m_EncryptionMethod,
                             m_PaddingScheme,
                             m_PlaintextLength,
                             m_ContentId.GetChars(),
                             m_RightsIssuerUrl.GetChars(),
                             m_TextualHeaders.GetData(),
                             m_TextualHeaders.GetDataSize());

    AP4_List<AP4_Atom>::Item* child_item = m_Children.FirstItem();
    while(child_item)
    {
        AP4_Atom* child_clone = child_item->GetData()->Clone();
        if(child_clone) clone->AddChild(child_clone);
        child_item = child_item->GetNext();
    }

    return clone;
}
