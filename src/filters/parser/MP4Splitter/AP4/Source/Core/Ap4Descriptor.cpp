/*****************************************************************
|
|    AP4 - Descriptors
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
#include "Ap4Descriptor.h"
#include "Ap4Utils.h"
#include "Ap4ByteStream.h"
#include "Ap4Atom.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_Descriptor)

/*----------------------------------------------------------------------
|   AP4_Descriptor::Inspect
+---------------------------------------------------------------------*/
AP4_Result
AP4_Descriptor::Inspect(AP4_AtomInspector& inspector)
{
    char name[20];
    AP4_FormatString(name, sizeof(name), "[Descriptor:%02x]", m_ClassId);
    char info[64];
    AP4_FormatString(info, sizeof(info), "size=%d+%d",
                     (int)GetHeaderSize(),
                     (int)m_PayloadSize);
    inspector.StartElement(name, info);
    inspector.EndElement();

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_UnknownDescriptor::AP4_UnknownDescriptor
+---------------------------------------------------------------------*/
AP4_UnknownDescriptor::AP4_UnknownDescriptor(AP4_ByteStream& stream,
                                             AP4_UI08        tag,
                                             AP4_Size        header_size,
                                             AP4_Size        payload_size) :
    AP4_Descriptor(tag, header_size, payload_size)
{
    m_Data.SetDataSize(payload_size);
    stream.Read(m_Data.UseData(), payload_size);    
}

/*----------------------------------------------------------------------
|   AP4_UnknownDescriptor::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_UnknownDescriptor::WriteFields(AP4_ByteStream& stream)
{
    // write the payload
    stream.Write(m_Data.GetData(), m_Data.GetDataSize());

    return AP4_SUCCESS;
}
