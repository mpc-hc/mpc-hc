/*****************************************************************
|
|    AP4 - ES Descriptors
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
#include "Ap4EsDescriptor.h"
#include "Ap4DescriptorFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|       AP4_EsDescriptor::AP4_EsDescriptor
+---------------------------------------------------------------------*/
AP4_EsDescriptor::AP4_EsDescriptor(AP4_UI16 es_id) :
    AP4_Descriptor(AP4_DESCRIPTOR_TAG_ES, 2, 2+1),
    m_EsId(es_id),
    m_OcrEsId(0),
    m_Flags(0),
    m_StreamPriority(0),
    m_DependsOn(0)
{
}

/*----------------------------------------------------------------------
|       AP4_EsDescriptor::AP4_EsDescriptor
+---------------------------------------------------------------------*/
AP4_EsDescriptor::AP4_EsDescriptor(AP4_ByteStream& stream, 
                                   AP4_Size        header_size,
                                   AP4_Size        payload_size) :
    AP4_Descriptor(AP4_DESCRIPTOR_TAG_ES, header_size, payload_size)
{
    AP4_Offset start;
    stream.Tell(start);

    // read descriptor fields
    stream.ReadUI16(m_EsId);
    unsigned char bits;
    stream.ReadUI08(bits);
    m_Flags = (bits>>5)&7;
    m_StreamPriority = bits&0x1F;
    if (m_Flags & AP4_ES_DESCRIPTOR_FLAG_STREAM_DEPENDENCY) {
        stream.ReadUI16(m_DependsOn);
    }  else {
        m_DependsOn = 0;
    }
    if (m_Flags & AP4_ES_DESCRIPTOR_FLAG_URL) {
        unsigned char url_length;
        stream.ReadUI08(url_length);
        if (url_length) {
            char* url = new char[url_length+1];
            if (url) {
                stream.Read(url, url_length, NULL);
                url[url_length] = '\0';
                m_Url = url;
                delete[] url;
            }
        }
    }
    if (m_Flags & AP4_ES_DESCRIPTOR_FLAG_URL) {
        stream.ReadUI16(m_OcrEsId);
    } else {
        m_OcrEsId = 0;
    }

    // read other descriptors
    AP4_Offset offset;
    stream.Tell(offset);
    AP4_SubStream* substream = new AP4_SubStream(stream, offset, 
                                                 payload_size-(offset-start));
    AP4_Descriptor* descriptor = NULL;
    while (AP4_DescriptorFactory::CreateDescriptorFromStream(*substream, 
                                                             descriptor) 
           == AP4_SUCCESS) {
        m_SubDescriptors.Add(descriptor);
    }
    substream->Release();
}

/*----------------------------------------------------------------------
|       AP4_EsDescriptor::~AP4_EsDescriptor
+---------------------------------------------------------------------*/
AP4_EsDescriptor::~AP4_EsDescriptor()
{
    m_SubDescriptors.DeleteReferences();
}

/*----------------------------------------------------------------------
|       AP4_EsDescriptor::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_EsDescriptor::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    // es id
    result = stream.WriteUI16(m_EsId);
    if (AP4_FAILED(result)) return result;

    // flags and other bits
    AP4_UI08 bits = m_StreamPriority | (AP4_UI08)(m_Flags<<5);
    result = stream.WriteUI08(bits);
    if (AP4_FAILED(result)) return result;
    
    // optional fields
    if (m_Flags & AP4_ES_DESCRIPTOR_FLAG_STREAM_DEPENDENCY) {
        result = stream.WriteUI16(m_DependsOn);
        if (AP4_FAILED(result)) return result;
    }
    if (m_Flags & AP4_ES_DESCRIPTOR_FLAG_URL) {
        result = stream.WriteUI08(m_Url.length());
        if (AP4_FAILED(result)) return result;
        result = stream.WriteString(m_Url.c_str());
        if (AP4_FAILED(result)) return result;
        result = stream.WriteUI08(0);
        if (AP4_FAILED(result)) return result;
    }
    if (m_Flags & AP4_ES_DESCRIPTOR_FLAG_OCR_STREAM) {
        result = stream.WriteUI16(m_OcrEsId);
        if (AP4_FAILED(result)) return result;
    }

    // write the sub descriptors
    m_SubDescriptors.Apply(AP4_DescriptorListWriter(stream));

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_EsDescriptor::Inspect
+---------------------------------------------------------------------*/
AP4_Result
AP4_EsDescriptor::Inspect(AP4_AtomInspector& inspector)
{
    char info[64];
    AP4_StringFormat(info, sizeof(info), "size=%ld+%ld", 
                     GetHeaderSize(),m_PayloadSize);
    inspector.StartElement("#[ES]", info);
    inspector.AddField("es_id", m_EsId);
    inspector.AddField("stream_priority", m_StreamPriority);

    // inspect children
    m_SubDescriptors.Apply(AP4_DescriptorListInspector(inspector));

    inspector.EndElement();

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_EsDescriptor::AddSubDescriptor
+---------------------------------------------------------------------*/
AP4_Result
AP4_EsDescriptor::AddSubDescriptor(AP4_Descriptor* descriptor)
{
    m_SubDescriptors.Add(descriptor);
    m_PayloadSize += descriptor->GetSize();

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_EsDescriptor::GetDecoderConfigDescriptor
+---------------------------------------------------------------------*/
const AP4_DecoderConfigDescriptor*
AP4_EsDescriptor::GetDecoderConfigDescriptor() const
{
    // find the decoder config descriptor
    AP4_Descriptor* descriptor = NULL;
    AP4_Result result = 
        m_SubDescriptors.Find(AP4_DescriptorFinder(AP4_DESCRIPTOR_TAG_DECODER_CONFIG),
                              descriptor);
    
    // return it
    if (AP4_SUCCEEDED(result)) {
        return dynamic_cast<AP4_DecoderConfigDescriptor*>(descriptor);
    } else {
        return NULL;
    }
}
