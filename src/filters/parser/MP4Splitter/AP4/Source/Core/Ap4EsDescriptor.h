/*****************************************************************
|
|    AP4 - ES Descriptor 
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

#ifndef _AP4_ES_DESCRIPTOR_H_
#define _AP4_ES_DESCRIPTOR_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4List.h"
#include "Ap4String.h"
#include "Ap4Descriptor.h"
#include "Ap4DecoderConfigDescriptor.h"

/*----------------------------------------------------------------------
|   class references
+---------------------------------------------------------------------*/
class AP4_ByteStream;

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const AP4_UI08 AP4_DESCRIPTOR_TAG_ES        = 0x03;
const AP4_UI08 AP4_DESCRIPTOR_TAG_ES_ID_INC = 0x0E;
const AP4_UI08 AP4_DESCRIPTOR_TAG_ES_ID_REF = 0x0F;

const int AP4_ES_DESCRIPTOR_FLAG_STREAM_DEPENDENCY = 1;
const int AP4_ES_DESCRIPTOR_FLAG_URL               = 2;
const int AP4_ES_DESCRIPTOR_FLAG_OCR_STREAM        = 4;

/*----------------------------------------------------------------------
|   AP4_EsDescriptor
+---------------------------------------------------------------------*/
class AP4_EsDescriptor : public AP4_Descriptor
{
 public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_EsDescriptor, AP4_Descriptor)

    // methods
    AP4_EsDescriptor(AP4_UI16 es_id);
    AP4_EsDescriptor(AP4_ByteStream& stream, 
                     AP4_Size        header_size, 
                     AP4_Size        payload_size);
    ~AP4_EsDescriptor();
    virtual AP4_Result AddSubDescriptor(AP4_Descriptor* descriptor);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    virtual AP4_Result Inspect(AP4_AtomInspector& inspector);
    virtual const AP4_DecoderConfigDescriptor* GetDecoderConfigDescriptor() const;    

 private:
    // members
    unsigned short                   m_EsId;
    unsigned short                   m_OcrEsId;
    AP4_Flags                        m_Flags;
    unsigned char                    m_StreamPriority;
    unsigned short                   m_DependsOn;
    AP4_String                       m_Url;
    mutable AP4_List<AP4_Descriptor> m_SubDescriptors;
};

/*----------------------------------------------------------------------
|   AP4_EsIdIncDescriptor
+---------------------------------------------------------------------*/
class AP4_EsIdIncDescriptor : public AP4_Descriptor
{
 public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_EsIdIncDescriptor, AP4_Descriptor)

    // methods
    AP4_EsIdIncDescriptor(AP4_UI32 track_id);
    AP4_EsIdIncDescriptor(AP4_ByteStream& stream, 
                          AP4_Size        header_size, 
                          AP4_Size        payload_size);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    virtual AP4_Result Inspect(AP4_AtomInspector& inspector);

    // accessors
    AP4_UI32 GetTrackId() const { return m_TrackId; }
    
 private:
    // members
    AP4_UI32 m_TrackId;
};

/*----------------------------------------------------------------------
|   AP4_EsIdRefDescriptor
+---------------------------------------------------------------------*/
class AP4_EsIdRefDescriptor : public AP4_Descriptor
{
 public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_EsIdRefDescriptor, AP4_Descriptor)

    // methods
    AP4_EsIdRefDescriptor(AP4_UI16 ref_index);
    AP4_EsIdRefDescriptor(AP4_ByteStream& stream, 
                          AP4_Size        header_size, 
                          AP4_Size        payload_size);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    virtual AP4_Result Inspect(AP4_AtomInspector& inspector);

    // accessors
    AP4_UI16 GetRefIndex() const { return m_RefIndex; }
    
 private:
    // members
    AP4_UI16 m_RefIndex;
};

#endif // _AP4_ES_DESCRIPTOR_H_
