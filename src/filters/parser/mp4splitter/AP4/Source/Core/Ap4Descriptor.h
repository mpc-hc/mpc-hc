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

#ifndef _AP4_DESCRIPTOR_H_
#define _AP4_DESCRIPTOR_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Expandable.h"
#include "Ap4List.h"
#include "Ap4DynamicCast.h"

/*----------------------------------------------------------------------
|   class references
+---------------------------------------------------------------------*/
class AP4_ByteStream;
class AP4_AtomInspector;

/*----------------------------------------------------------------------
|   AP4_Descriptor
+---------------------------------------------------------------------*/
class AP4_Descriptor : public AP4_Expandable
{
 public:
    AP4_IMPLEMENT_DYNAMIC_CAST(AP4_Descriptor)
    
    // constructor
    AP4_Descriptor(AP4_UI08 tag, AP4_Size header_size, AP4_Size payload_size) :
        AP4_Expandable(tag, CLASS_ID_SIZE_08, header_size, payload_size) {}

    // AP4_Exandable methods
    virtual AP4_Result Inspect(AP4_AtomInspector& inspector);

    // methods
    AP4_UI08 GetTag() { return (AP4_UI08)m_ClassId; }
};

/*----------------------------------------------------------------------
|   AP4_UnknownDescriptor
+---------------------------------------------------------------------*/
class AP4_UnknownDescriptor : public AP4_Descriptor
{
public:
    // contrusctor
    AP4_UnknownDescriptor(AP4_ByteStream& stream, 
                          AP4_UI08        tag,
                          AP4_Size        header_size,
                          AP4_Size        payload_size);
                          
    // AP4_Expandable methods
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    
private:
    // members
    AP4_DataBuffer m_Data;
};

/*----------------------------------------------------------------------
|   AP4_DescriptorFinder
+---------------------------------------------------------------------*/
class AP4_DescriptorFinder : public AP4_List<AP4_Descriptor>::Item::Finder
{
 public:
    AP4_DescriptorFinder(AP4_UI08 tag) : m_Tag(tag) {}
    AP4_Result Test(AP4_Descriptor* descriptor) const {
        return descriptor->GetTag() == m_Tag ? AP4_SUCCESS : AP4_FAILURE;
    }
    
 private:
    AP4_UI08 m_Tag;
};

/*----------------------------------------------------------------------
|   AP4_DescriptorListWriter
+---------------------------------------------------------------------*/
class AP4_DescriptorListWriter : public AP4_List<AP4_Descriptor>::Item::Operator
{
public:
    AP4_DescriptorListWriter(AP4_ByteStream& stream) :
      m_Stream(stream) {}
    AP4_Result Action(AP4_Descriptor* descriptor) const {
        return descriptor->Write(m_Stream);
    }

private:
    AP4_ByteStream& m_Stream;
};

/*----------------------------------------------------------------------
|   AP4_DescriptorListInspector
+---------------------------------------------------------------------*/
class AP4_DescriptorListInspector : public AP4_List<AP4_Descriptor>::Item::Operator
{
 public:
    AP4_DescriptorListInspector(AP4_AtomInspector& inspector) :
        m_Inspector(inspector) {}
    AP4_Result Action(AP4_Descriptor* descriptor) const {
        descriptor->Inspect(m_Inspector);
        return AP4_SUCCESS;
    }

 private:
    AP4_AtomInspector& m_Inspector;
};

#endif // _AP4_DESCRIPTOR_H_
