/*****************************************************************
|
|    AP4 - ES Descriptor 
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

#ifndef _AP4_ES_DESCRIPTOR_H_
#define _AP4_ES_DESCRIPTOR_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4.h"
#include "Ap4ByteStream.h"
#include "Ap4List.h"
#include "Ap4Descriptor.h"
#include "Ap4DecoderConfigDescriptor.h"

/*----------------------------------------------------------------------
|       constants
+---------------------------------------------------------------------*/
const AP4_Descriptor::Tag AP4_DESCRIPTOR_TAG_ES = 0x03;

const int AP4_ES_DESCRIPTOR_FLAG_STREAM_DEPENDENCY = 1;
const int AP4_ES_DESCRIPTOR_FLAG_URL               = 2;
const int AP4_ES_DESCRIPTOR_FLAG_OCR_STREAM        = 4;

/*----------------------------------------------------------------------
|       AP4_EsDescriptor
+---------------------------------------------------------------------*/
class AP4_EsDescriptor : public AP4_Descriptor
{
 public:
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

#endif // _AP4_ES_DESCRIPTOR_H_
