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

#ifndef _AP4_IPMP_H_
#define _AP4_IPMP_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4String.h"
#include "Ap4Descriptor.h"

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const AP4_UI08 AP4_DESCRIPTOR_TAG_IPMP_DESCRIPTOR_POINTER = 0x0A;
const AP4_UI08 AP4_DESCRIPTOR_TAG_IPMP_DESCRIPTOR         = 0x0B;

/*----------------------------------------------------------------------
|   AP4_IpmpDescriptorPointer
+---------------------------------------------------------------------*/
class AP4_IpmpDescriptorPointer : public AP4_Descriptor  
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_IpmpDescriptorPointer, AP4_Descriptor)

    // methods
    AP4_IpmpDescriptorPointer(AP4_UI08 descriptor_id);
    AP4_IpmpDescriptorPointer(AP4_ByteStream&     stream, 
                              AP4_Size            header_size, 
                              AP4_Size            payload_size);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    virtual AP4_Result Inspect(AP4_AtomInspector& inspector);

    // accessors
    AP4_UI08 GetDescriptorId()   const { return m_DescriptorId;   }
    AP4_UI16 GetDescriptorIdEx() const { return m_DescriptorIdEx; }
    AP4_UI16 GetEsId()           const { return m_EsId;           }

private:
    // members
    AP4_UI08 m_DescriptorId; 
    AP4_UI16 m_DescriptorIdEx; 
    AP4_UI16 m_EsId; 
};

/*----------------------------------------------------------------------
|   AP4_IpmpDescriptor
+---------------------------------------------------------------------*/
class AP4_IpmpDescriptor : public AP4_Descriptor  
{ 
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_IpmpDescriptor, AP4_Descriptor)

    // methods
    AP4_IpmpDescriptor(AP4_UI08 descriptor_id, AP4_UI16 ipmps_type);
    AP4_IpmpDescriptor(AP4_ByteStream&     stream, 
                       AP4_Size            header_size, 
                       AP4_Size            payload_size);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    virtual AP4_Result Inspect(AP4_AtomInspector& inspector);

    // accessors
    AP4_UI08              GetDescriptorId()     const { return m_DescriptorId;     }
    AP4_UI16              GetIpmpsType()        const { return m_IpmpsType;        }
    AP4_UI16              GetDescriptorIdEx()   const { return m_DescriptorIdEx;   }
    const AP4_UI08*       GetToolId()           const { return m_ToolId;           }
    AP4_UI08              GetControlPointCode() const { return m_ControlPointCode; }
    AP4_UI08              GetSequenceCode()     const { return m_SequenceCode;     }
    const AP4_String&     GetUrl()              const { return m_Url;              }
    const AP4_DataBuffer& GetData()             const { return m_Data;             }
    void                  SetData(const unsigned char* data, AP4_Size data_size);
    
private:
    // members
    AP4_UI08       m_DescriptorId; 
    AP4_UI16       m_IpmpsType;
    AP4_UI16       m_DescriptorIdEx; 
    AP4_UI08       m_ToolId[16];
    AP4_UI08       m_ControlPointCode;
    AP4_UI08       m_SequenceCode;
    AP4_String     m_Url;
    AP4_DataBuffer m_Data;
};

#endif // _AP4_IPMP_H_
