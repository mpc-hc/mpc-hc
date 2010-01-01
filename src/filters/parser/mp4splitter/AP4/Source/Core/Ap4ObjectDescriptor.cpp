/*****************************************************************
|
|    AP4 - Object Descriptor
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
#include "Ap4ObjectDescriptor.h"
#include "Ap4DescriptorFactory.h"
#include "Ap4Utils.h"
#include "Ap4Atom.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_ObjectDescriptor)
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_DescriptorUpdateCommand)

/*----------------------------------------------------------------------
|   AP4_ObjectDescriptor::AP4_ObjectDescriptor
+---------------------------------------------------------------------*/
AP4_ObjectDescriptor::AP4_ObjectDescriptor(AP4_UI08 tag,
                                           AP4_Size header_size,
                                           AP4_Size payload_size) :
    AP4_Descriptor(tag, header_size, payload_size),
    m_UrlFlag(false)
{
}

/*----------------------------------------------------------------------
|   AP4_ObjectDescriptor::AP4_ObjectDescriptor
+---------------------------------------------------------------------*/
AP4_ObjectDescriptor::AP4_ObjectDescriptor(AP4_UI08 tag, AP4_UI16 id) :
    AP4_Descriptor(tag, 3, 2),    
    m_ObjectDescriptorId(id),
    m_UrlFlag(false)
{
}

/*----------------------------------------------------------------------
|   AP4_ObjectDescriptor::AP4_ObjectDescriptor
+---------------------------------------------------------------------*/
AP4_ObjectDescriptor::AP4_ObjectDescriptor(AP4_ByteStream& stream, 
                                           AP4_UI08        tag,
                                           AP4_Size        header_size,
                                           AP4_Size        payload_size) :
    AP4_Descriptor(tag, header_size, payload_size)
{
    AP4_Position start;
    stream.Tell(start);

    // read descriptor fields
    unsigned short bits;
    stream.ReadUI16(bits);
    m_ObjectDescriptorId = (bits>>6);
    m_UrlFlag = ((bits&(1<<5))!=0);
  
    if (m_UrlFlag) {
        unsigned char url_length;
        stream.ReadUI08(url_length);
        char url[256];
        stream.Read(url, url_length);
        url[url_length] = '\0';
        m_Url = url;
    }

    // read other descriptors
    AP4_Position offset;
    stream.Tell(offset);
    AP4_SubStream* substream = new AP4_SubStream(stream, offset, 
                                                 payload_size-AP4_Size(offset-start));
    AP4_Descriptor* descriptor = NULL;
    while (AP4_DescriptorFactory::CreateDescriptorFromStream(*substream, 
                                                             descriptor) 
           == AP4_SUCCESS) {
        m_SubDescriptors.Add(descriptor);
    }
    substream->Release();
}

/*----------------------------------------------------------------------
|   AP4_ObjectDescriptor::~AP4_ObjectDescriptor
+---------------------------------------------------------------------*/
AP4_ObjectDescriptor::~AP4_ObjectDescriptor()
{
    m_SubDescriptors.DeleteReferences();
}

/*----------------------------------------------------------------------
|   AP4_ObjectDescriptor::FindSubDescriptor
+---------------------------------------------------------------------*/
AP4_Descriptor* 
AP4_ObjectDescriptor::FindSubDescriptor(AP4_UI08 tag) const
{
    AP4_Descriptor* descriptor = NULL;
    AP4_Result result = m_SubDescriptors.Find(AP4_DescriptorFinder(tag), descriptor);
    if (AP4_FAILED(result)) return NULL;
    
    return descriptor;
}

/*----------------------------------------------------------------------
|   AP4_ObjectDescriptor::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_ObjectDescriptor::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    // id and flag
    unsigned short bits = (m_ObjectDescriptorId<<6)|(m_UrlFlag?(1<<5):0)|0x1F;
    result = stream.WriteUI16(bits);
    if (AP4_FAILED(result)) return result;

    // optional url
    if (m_UrlFlag) {
        stream.WriteUI08((AP4_UI08)m_Url.GetLength());
        stream.Write(m_Url.GetChars(), m_Url.GetLength());
    }
    
    // write the sub descriptors
    m_SubDescriptors.Apply(AP4_DescriptorListWriter(stream));

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_ObjectDescriptor::Inspect
+---------------------------------------------------------------------*/
AP4_Result
AP4_ObjectDescriptor::Inspect(AP4_AtomInspector& inspector)
{
    char info[64];
    AP4_FormatString(info, sizeof(info), "size=%ld+%ld", 
                     GetHeaderSize(),m_PayloadSize);
    inspector.StartElement("[ObjectDescriptor]", info);
    inspector.AddField("id", m_ObjectDescriptorId);
    if (m_UrlFlag) {
        inspector.AddField("url", m_Url.GetChars());
    }
    
    // inspect children
    m_SubDescriptors.Apply(AP4_DescriptorListInspector(inspector));

    inspector.EndElement();

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_ObjectDescriptor::AddSubDescriptor
+---------------------------------------------------------------------*/
AP4_Result
AP4_ObjectDescriptor::AddSubDescriptor(AP4_Descriptor* descriptor)
{
    m_SubDescriptors.Add(descriptor);
    m_PayloadSize += descriptor->GetSize();

    // check that the header is still large enough to encode the payload
    // length
    unsigned int min_header_size = MinHeaderSize(m_PayloadSize);
    if (min_header_size > m_HeaderSize) m_HeaderSize = min_header_size;
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_InitialObjectDescriptor::AP4_InitialObjectDescriptor
+---------------------------------------------------------------------*/
AP4_InitialObjectDescriptor::AP4_InitialObjectDescriptor(
    AP4_UI08    tag, // should be AP4_DESCRIPTOR_TAG_IOD or AP4_DESCRIPTOR_TAG_MP4_IOD
    AP4_UI16    object_descriptor_id,
    bool        include_inline_profile_level,
    AP4_UI08    od_profile_level_indication,
    AP4_UI08    scene_profile_level_indication,
    AP4_UI08    audio_profile_level_indication,
    AP4_UI08    visual_profile_level_indication,
    AP4_UI08    graphics_profile_level_indication) :
    AP4_ObjectDescriptor(tag, object_descriptor_id),
    m_IncludeInlineProfileLevelFlag(include_inline_profile_level),
    m_OdProfileLevelIndication(od_profile_level_indication),
    m_SceneProfileLevelIndication(scene_profile_level_indication),
    m_AudioProfileLevelIndication(audio_profile_level_indication),
    m_VisualProfileLevelIndication(visual_profile_level_indication),
    m_GraphicsProfileLevelIndication(graphics_profile_level_indication)
{
    m_PayloadSize = 7;
}

/*----------------------------------------------------------------------
|   AP4_InitialObjectDescriptor::AP4_InitialObjectDescriptor
+---------------------------------------------------------------------*/
AP4_InitialObjectDescriptor::AP4_InitialObjectDescriptor(AP4_ByteStream& stream, 
                                                         AP4_UI08        tag,
                                                         AP4_Size        header_size,
                                                         AP4_Size        payload_size) :
    AP4_ObjectDescriptor(tag, header_size, payload_size),
    m_OdProfileLevelIndication(0),
    m_SceneProfileLevelIndication(0),
    m_AudioProfileLevelIndication(0),
    m_VisualProfileLevelIndication(0),
    m_GraphicsProfileLevelIndication(0)
{
    AP4_Position start;
    stream.Tell(start);

    // read descriptor fields
    unsigned short bits;
    stream.ReadUI16(bits);
    m_ObjectDescriptorId = (bits>>6);
    m_UrlFlag = ((bits&(1<<5))!=0);
    m_IncludeInlineProfileLevelFlag = ((bits&(1<<4))!=0);
    
    if (m_UrlFlag) {
        unsigned char url_length;
        stream.ReadUI08(url_length);
        char url[256];
        stream.Read(url, url_length);
        url[url_length] = '\0';
        m_Url = url;
    } else {
        stream.ReadUI08(m_OdProfileLevelIndication); 
        stream.ReadUI08(m_SceneProfileLevelIndication); 
        stream.ReadUI08(m_AudioProfileLevelIndication); 
        stream.ReadUI08(m_VisualProfileLevelIndication); 
        stream.ReadUI08(m_GraphicsProfileLevelIndication); 
    }
    
    // read other descriptors
    AP4_Position offset;
    stream.Tell(offset);
    AP4_SubStream* substream = new AP4_SubStream(stream, offset, 
                                                 payload_size-AP4_Size(offset-start));
    AP4_Descriptor* descriptor = NULL;
    while (AP4_DescriptorFactory::CreateDescriptorFromStream(*substream, 
                                                             descriptor) 
           == AP4_SUCCESS) {
        m_SubDescriptors.Add(descriptor);
    }
    substream->Release();
}

/*----------------------------------------------------------------------
|   AP4_InitialObjectDescriptor::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_InitialObjectDescriptor::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    // id and flags
    unsigned short bits = (m_ObjectDescriptorId<<6)                 | 
                          (m_UrlFlag?(1<<5):0)                      |
                          (m_IncludeInlineProfileLevelFlag?(1<<4):0)|
                          0xF;
    result = stream.WriteUI16(bits);
    if (AP4_FAILED(result)) return result;

    // optional url
    if (m_UrlFlag) {
        stream.WriteUI08((AP4_UI08)m_Url.GetLength());
        stream.Write(m_Url.GetChars(), m_Url.GetLength());
    } else {
        stream.WriteUI08(m_OdProfileLevelIndication); 
        stream.WriteUI08(m_SceneProfileLevelIndication); 
        stream.WriteUI08(m_AudioProfileLevelIndication); 
        stream.WriteUI08(m_VisualProfileLevelIndication); 
        stream.WriteUI08(m_GraphicsProfileLevelIndication); 
    }
    
    // write the sub descriptors
    m_SubDescriptors.Apply(AP4_DescriptorListWriter(stream));

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_InitialObjectDescriptor::Inspect
+---------------------------------------------------------------------*/
AP4_Result
AP4_InitialObjectDescriptor::Inspect(AP4_AtomInspector& inspector)
{
    char info[64];
    AP4_FormatString(info, sizeof(info), "size=%ld+%ld", 
                     GetHeaderSize(),m_PayloadSize);
    inspector.StartElement("[InitialObjectDescriptor]", info);
    inspector.AddField("id", m_ObjectDescriptorId);
    if (m_UrlFlag) {
        inspector.AddField("url", m_Url.GetChars());
    } else {
        inspector.AddField("include inline profile level flag", 
                           m_IncludeInlineProfileLevelFlag, 
                           AP4_AtomInspector::HINT_BOOLEAN);
        inspector.AddField("OD profile level", m_OdProfileLevelIndication, AP4_AtomInspector::HINT_HEX); 
        inspector.AddField("scene profile level", m_SceneProfileLevelIndication, AP4_AtomInspector::HINT_HEX); 
        inspector.AddField("audio profile level", m_AudioProfileLevelIndication, AP4_AtomInspector::HINT_HEX); 
        inspector.AddField("visual profile level", m_VisualProfileLevelIndication, AP4_AtomInspector::HINT_HEX); 
        inspector.AddField("graphics profile level", m_GraphicsProfileLevelIndication, AP4_AtomInspector::HINT_HEX); 
    }
    
    // inspect children
    m_SubDescriptors.Apply(AP4_DescriptorListInspector(inspector));

    inspector.EndElement();

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_DescriptorUpdateCommand::AP4_DescriptorUpdateCommand
+---------------------------------------------------------------------*/
AP4_DescriptorUpdateCommand::AP4_DescriptorUpdateCommand(AP4_UI08 tag) :
    AP4_Command(tag, 2, 0)
{
}

/*----------------------------------------------------------------------
|   AP4_DescriptorUpdateCommand::AP4_DescriptorUpdateCommand
+---------------------------------------------------------------------*/
AP4_DescriptorUpdateCommand::AP4_DescriptorUpdateCommand(
    AP4_ByteStream& stream, 
    AP4_UI08        tag,
    AP4_Size        header_size,
    AP4_Size        payload_size) :
    AP4_Command(tag, header_size, payload_size)
{
    // read the descriptors
    AP4_Position offset;
    stream.Tell(offset);
    AP4_SubStream* substream = new AP4_SubStream(stream, offset, 
                                                 payload_size);
    AP4_Descriptor* descriptor = NULL;
    while (AP4_DescriptorFactory::CreateDescriptorFromStream(*substream, descriptor) == AP4_SUCCESS) {
        m_Descriptors.Add(descriptor);
    }
    substream->Release();
}

/*----------------------------------------------------------------------
|   AP4_DescriptorUpdateCommand::~AP4_DescriptorUpdateCommand
+---------------------------------------------------------------------*/
AP4_DescriptorUpdateCommand::~AP4_DescriptorUpdateCommand()
{
    m_Descriptors.DeleteReferences();
}

/*----------------------------------------------------------------------
|   AP4_DescriptorUpdateCommand::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_DescriptorUpdateCommand::WriteFields(AP4_ByteStream& stream)
{
    // write the descriptors
    m_Descriptors.Apply(AP4_DescriptorListWriter(stream));

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_DescriptorUpdateCommand::Inspect
+---------------------------------------------------------------------*/
AP4_Result
AP4_DescriptorUpdateCommand::Inspect(AP4_AtomInspector& inspector)
{
    char info[64];
    AP4_FormatString(info, sizeof(info), "size=%ld+%ld", 
                     GetHeaderSize(),m_PayloadSize);
    switch (GetTag()) {
        case AP4_COMMAND_TAG_OBJECT_DESCRIPTOR_UPDATE:
            inspector.StartElement("[ObjectDescriptorUpdate]", info);
            break;

        case AP4_COMMAND_TAG_IPMP_DESCRIPTOR_UPDATE:
            inspector.StartElement("[IPMP_DescriptorUpdate]", info);
            break;

        default:
            inspector.StartElement("[DescriptorUpdate]", info);
            break;
    }
    
    // inspect children
    m_Descriptors.Apply(AP4_DescriptorListInspector(inspector));

    inspector.EndElement();

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_DescriptorUpdateCommand::AddDescriptor
+---------------------------------------------------------------------*/
AP4_Result
AP4_DescriptorUpdateCommand::AddDescriptor(AP4_Descriptor* descriptor)
{
    m_Descriptors.Add(descriptor);
    m_PayloadSize += descriptor->GetSize();

    // check that the header is still large enough to encode the payload
    // length
    unsigned int min_header_size = MinHeaderSize(m_PayloadSize);
    if (min_header_size > m_HeaderSize) m_HeaderSize = min_header_size;

    return AP4_SUCCESS;
}

