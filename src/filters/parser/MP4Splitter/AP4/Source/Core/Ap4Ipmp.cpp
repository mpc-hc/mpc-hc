/*****************************************************************
|
|    AP4 - IPMP
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
#include "Ap4ByteStream.h"
#include "Ap4Ipmp.h"
#include "Ap4Atom.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_IpmpDescriptor)
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_IpmpDescriptorPointer)

/*----------------------------------------------------------------------
|   AP4_IpmpDescriptorPointer::AP4_IpmpDescriptorPointer
+---------------------------------------------------------------------*/
AP4_IpmpDescriptorPointer::AP4_IpmpDescriptorPointer(AP4_UI08 descriptor_id) :
    AP4_Descriptor(AP4_DESCRIPTOR_TAG_IPMP_DESCRIPTOR_POINTER, 2, 1),
    m_DescriptorId(descriptor_id)
{
}

/*----------------------------------------------------------------------
|   AP4_IpmpDescriptorPointer::AP4_IpmpDescriptorPointer
+---------------------------------------------------------------------*/
AP4_IpmpDescriptorPointer::AP4_IpmpDescriptorPointer(AP4_ByteStream& stream, 
                                                     AP4_Size        header_size,
                                                     AP4_Size        payload_size) :
    AP4_Descriptor(AP4_DESCRIPTOR_TAG_IPMP_DESCRIPTOR_POINTER, header_size, payload_size)
{
    stream.ReadUI08(m_DescriptorId);
    if (m_DescriptorId == 0xFF && payload_size >= 5) {
        stream.ReadUI16(m_DescriptorIdEx);
        stream.ReadUI16(m_EsId);
    }
}

/*----------------------------------------------------------------------
|   AP4_IpmpDescriptorPointer::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_IpmpDescriptorPointer::WriteFields(AP4_ByteStream& stream)
{
    stream.WriteUI08(m_DescriptorId);
    if (m_DescriptorId == 0xFF) {
        stream.WriteUI16(m_DescriptorIdEx);
        stream.WriteUI16(m_EsId);
    }
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_IpmpDescriptorPointer::Inspect
+---------------------------------------------------------------------*/
AP4_Result
AP4_IpmpDescriptorPointer::Inspect(AP4_AtomInspector& inspector)
{
    char info[64];
    AP4_FormatString(info, sizeof(info), "size=%ld+%ld", 
                     GetHeaderSize(),m_PayloadSize);
    inspector.StartElement("[IPMP_DescriptorPointer]", info);
    inspector.AddField("IPMP_DescriptorID", m_DescriptorId);
    if (m_DescriptorId == 0xFF) {
        inspector.AddField("IPMP_DescriptorIDEx", m_DescriptorIdEx);
        inspector.AddField("IPMP_ES_ID",          m_EsId);
    }

    inspector.EndElement();

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_IpmpDescriptor::AP4_IpmpDescriptor
+---------------------------------------------------------------------*/
AP4_IpmpDescriptor::AP4_IpmpDescriptor(AP4_UI08 descriptor_id, AP4_UI16 ipmps_type) :
    AP4_Descriptor(AP4_DESCRIPTOR_TAG_IPMP_DESCRIPTOR, 2, 3),
    m_DescriptorId(descriptor_id),
    m_IpmpsType(ipmps_type),
    m_DescriptorIdEx(0),
    m_ControlPointCode(0),
    m_SequenceCode(0)
{
    AP4_SetMemory(m_ToolId, 0, sizeof(m_ToolId));
}

/*----------------------------------------------------------------------
|   AP4_IpmpDescriptor::AP4_IpmpDescriptor
+---------------------------------------------------------------------*/
AP4_IpmpDescriptor::AP4_IpmpDescriptor(AP4_ByteStream& stream, 
                                       AP4_Size        header_size,
                                       AP4_Size        payload_size) :
    AP4_Descriptor(AP4_DESCRIPTOR_TAG_IPMP_DESCRIPTOR, header_size, payload_size),
    m_DescriptorIdEx(0),
    m_ControlPointCode(0),
    m_SequenceCode(0)
{
    stream.ReadUI08(m_DescriptorId);
    stream.ReadUI16(m_IpmpsType);
    AP4_SetMemory(m_ToolId, 0, sizeof(m_ToolId));
    if (m_DescriptorId == 0xFF && m_IpmpsType == 0xFFFF) {
        AP4_Size fields_size = 3+3;
        stream.ReadUI16(m_DescriptorIdEx);
        stream.Read(m_ToolId, 16);
        stream.ReadUI08(m_ControlPointCode);
        if (m_ControlPointCode > 0) {
            stream.ReadUI08(m_SequenceCode);
            ++fields_size;
        }
        if (fields_size < payload_size) {
            m_Data.SetDataSize(payload_size-fields_size);
            stream.Read(m_Data.UseData(), payload_size-fields_size);
        }
    } else if (m_IpmpsType == 0) {
        if (payload_size > 3) {
            char* buffer = new char[1+payload_size-3];
            buffer[payload_size-3] = '\0';
            stream.Read(buffer, payload_size-3);
            m_Url.Assign(buffer, payload_size-3);
            delete[] buffer;
        }
    } else {
        if (payload_size > 3) {
            m_Data.SetDataSize(payload_size-3);
            stream.Read(m_Data.UseData(), payload_size-3);
        }
    }
}

/*----------------------------------------------------------------------
|   AP4_IpmpDescriptor::SetData
+---------------------------------------------------------------------*/
void
AP4_IpmpDescriptor::SetData(const unsigned char* data, AP4_Size data_size)
{
    m_Data.SetData(data, data_size);
    m_PayloadSize += data_size;
    m_HeaderSize = MinHeaderSize(m_PayloadSize);
}

/*----------------------------------------------------------------------
|   AP4_IpmpDescriptor::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_IpmpDescriptor::WriteFields(AP4_ByteStream& stream)
{
    stream.WriteUI08(m_DescriptorId);
    stream.WriteUI16(m_IpmpsType);
    if (m_DescriptorId == 0xFF && m_IpmpsType == 0xFFFF) {
        stream.WriteUI16(m_DescriptorIdEx);
        stream.Write(m_ToolId, 16);
        stream.WriteUI08(m_ControlPointCode);
        if (m_ControlPointCode > 0) {
            stream.WriteUI08(m_SequenceCode);
        }
        if (m_Data.GetDataSize()) {
            stream.Write(m_Data.GetData(), m_Data.GetDataSize());
        }
    } else if (m_IpmpsType == 0) {
        stream.Write(m_Url.GetChars(), m_Url.GetLength()+1);
    } else {
        stream.Write(m_Data.GetData(), m_Data.GetDataSize());
    }
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_IpmpDescriptor::Inspect
+---------------------------------------------------------------------*/
AP4_Result
AP4_IpmpDescriptor::Inspect(AP4_AtomInspector& inspector)
{
    char info[64];
    AP4_FormatString(info, sizeof(info), "size=%ld+%ld", 
                     GetHeaderSize(),m_PayloadSize);
    inspector.StartElement("[IPMP_Descriptor]", info);
    inspector.AddField("IPMP_DescriptorID", m_DescriptorId);
    inspector.AddField("IPMPS_Type", m_IpmpsType, AP4_AtomInspector::HINT_HEX);
    if (m_DescriptorId == 0xFF && m_IpmpsType == 0xFFFF) {
        inspector.AddField("IPMP_DescriptorIDEx", m_DescriptorIdEx);
        inspector.AddField("IPMP_ToolID", (const unsigned char*)(&m_ToolId[0]), 16, AP4_AtomInspector::HINT_HEX);
        inspector.AddField("controlPointCode", m_ControlPointCode);
        if (m_ControlPointCode > 0) {
            inspector.AddField("sequenceCode", m_SequenceCode);
        }
    } else if (m_IpmpsType == 0) {
        inspector.AddField("URL", m_Url.GetChars());
    } else {
        inspector.AddField("data size", m_Data.GetDataSize());
    }

    inspector.EndElement();

    return AP4_SUCCESS;
}
