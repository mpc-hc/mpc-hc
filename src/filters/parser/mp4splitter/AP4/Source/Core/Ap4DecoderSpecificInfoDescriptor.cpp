/*****************************************************************
|
|    AP4 - DecoderSpecificInfo Descriptors
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
#include "Ap4DecoderSpecificInfoDescriptor.h"
#include "Ap4DescriptorFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|       AP4_DecoderSpecificInfoDescriptor::AP4_DecoderSpecificInfoDescriptor
+---------------------------------------------------------------------*/
AP4_DecoderSpecificInfoDescriptor::AP4_DecoderSpecificInfoDescriptor(
    const AP4_DataBuffer& data) :
    AP4_Descriptor(AP4_DESCRIPTOR_TAG_DECODER_SPECIFIC_INFO, 
                   MinHeaderSize(data.GetDataSize()), 
                   data.GetDataSize()),
    m_Info(data)
{
}

/*----------------------------------------------------------------------
|       AP4_DecoderSpecificInfoDescriptor::AP4_DecoderSpecificInfoDescriptor
+---------------------------------------------------------------------*/
AP4_DecoderSpecificInfoDescriptor::AP4_DecoderSpecificInfoDescriptor(
    AP4_ByteStream& stream, AP4_Size header_size, AP4_Size payload_size) :
    AP4_Descriptor(AP4_DESCRIPTOR_TAG_DECODER_SPECIFIC_INFO, 
                   header_size, payload_size)
{
    m_Info.SetDataSize(payload_size);
	stream.Read(m_Info.UseData(), payload_size);
}

/*----------------------------------------------------------------------
|       AP4_DecoderSpecificInfoDescriptor::~AP4_DecoderSpecificInfoDescriptor
+---------------------------------------------------------------------*/
AP4_DecoderSpecificInfoDescriptor::~AP4_DecoderSpecificInfoDescriptor()
{
}

/*----------------------------------------------------------------------
|       AP4_DecoderSpecificInfoDescriptor::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_DecoderSpecificInfoDescriptor::WriteFields(AP4_ByteStream& stream)
{
    // write the info buffer
    if (m_PayloadSize && m_Info.GetDataSize()) {
        stream.Write(m_Info.GetData(), m_Info.GetDataSize());
    }

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_DecoderSpecificInfoDescriptor::Inspect
+---------------------------------------------------------------------*/
AP4_Result
AP4_DecoderSpecificInfoDescriptor::Inspect(AP4_AtomInspector& inspector)
{
    char* info = new char[m_Info.GetDataSize()*3+1];
    for (unsigned int i=0; i<m_Info.GetDataSize(); i++) {
		AP4_StringFormat(&info[i*3], 3, "%02x ", m_Info.UseData()[i]);
	}
    info[m_Info.GetDataSize()*3] = '\0';
    inspector.AddField("#[DecoderSpecificInfo]", info);
    delete[] info;

    return AP4_SUCCESS;
}

