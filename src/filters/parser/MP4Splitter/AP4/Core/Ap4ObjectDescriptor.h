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

#ifndef _AP4_OBJECT_DESCRIPTOR_H_
#define _AP4_OBJECT_DESCRIPTOR_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4List.h"
#include "Ap4String.h"
#include "Ap4Descriptor.h"
#include "Ap4Command.h"

/*----------------------------------------------------------------------
|   class references
+---------------------------------------------------------------------*/
class AP4_ByteStream;

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const AP4_UI08 AP4_DESCRIPTOR_TAG_OD      = 0x01;
const AP4_UI08 AP4_DESCRIPTOR_TAG_IOD     = 0x02;
const AP4_UI08 AP4_DESCRIPTOR_TAG_MP4_OD  = 0x11;
const AP4_UI08 AP4_DESCRIPTOR_TAG_MP4_IOD = 0x10;

/*----------------------------------------------------------------------
|   AP4_ObjectDescriptor
+---------------------------------------------------------------------*/
class AP4_ObjectDescriptor : public AP4_Descriptor
{
 public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_ObjectDescriptor, AP4_Descriptor)

    // methods
    AP4_ObjectDescriptor(AP4_ByteStream& stream, 
                         AP4_UI08        tag,
                         AP4_Size        header_size, 
                         AP4_Size        payload_size);
    AP4_ObjectDescriptor(AP4_UI08 tag, AP4_UI16 id);
    virtual ~AP4_ObjectDescriptor();
    
    /**
     * Add a sub-descriptor. 
     * Ownership of the sub-descriptor object is transfered.
     */ 
    virtual AP4_Result AddSubDescriptor(AP4_Descriptor* descriptor);
    
    virtual AP4_Descriptor* FindSubDescriptor(AP4_UI08 tag) const;
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    virtual AP4_Result Inspect(AP4_AtomInspector& inspector);

    // accessors
    AP4_UI16 GetObjectDescriptorId() const { return m_ObjectDescriptorId; }
    bool     GetUrlFlag()            const { return m_UrlFlag; }
    const AP4_String& GetUrl()       const { return m_Url;}

 protected:
    // constructor
    AP4_ObjectDescriptor(AP4_UI08 tag, AP4_Size header_size, AP4_Size payload_size);
    
    // members
    AP4_UI16                         m_ObjectDescriptorId;
    bool                             m_UrlFlag;
    AP4_String                       m_Url;
    mutable AP4_List<AP4_Descriptor> m_SubDescriptors;
};

/*----------------------------------------------------------------------
|   AP4_InitialObjectDescriptor
+---------------------------------------------------------------------*/
class AP4_InitialObjectDescriptor : public AP4_ObjectDescriptor
{
 public:
    // methods
    AP4_InitialObjectDescriptor(AP4_ByteStream& stream, 
                                AP4_UI08        tag,
                                AP4_Size        header_size, 
                                AP4_Size        payload_size);
    AP4_InitialObjectDescriptor(AP4_UI08    tag, // should be AP4_DESCRIPTOR_TAG_IOD or AP4_DESCRIPTOR_TAG_MP4_IOD
                                AP4_UI16    object_descriptor_id,
                                bool        include_inline_profile_level,
                                AP4_UI08    od_profile_level_indication,
                                AP4_UI08    scene_profile_level_indication,
                                AP4_UI08    audio_profile_level_indication,
                                AP4_UI08    visual_profile_level_indication,
                                AP4_UI08    graphics_profile_level_indication);

    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    virtual AP4_Result Inspect(AP4_AtomInspector& inspector);
    
    // accessors
    bool     GetIncludeProfileLevelFlag()        const { return m_IncludeInlineProfileLevelFlag; }
    AP4_UI08 GetOdProfileLevelIndication()       const { return m_OdProfileLevelIndication; }
    AP4_UI08 GetSceneProfileLevelIndication()    const { return m_SceneProfileLevelIndication; }
    AP4_UI08 GetAudioProfileLevelIndication()    const { return m_AudioProfileLevelIndication; }
    AP4_UI08 GetVisualProfileLevelIndication()   const { return m_VisualProfileLevelIndication; }
    AP4_UI08 GetGraphicsProfileLevelIndication() const { return m_GraphicsProfileLevelIndication; }

 private:
    // members
    bool     m_IncludeInlineProfileLevelFlag;
    AP4_UI08 m_OdProfileLevelIndication; 
    AP4_UI08 m_SceneProfileLevelIndication; 
    AP4_UI08 m_AudioProfileLevelIndication; 
    AP4_UI08 m_VisualProfileLevelIndication; 
    AP4_UI08 m_GraphicsProfileLevelIndication; 
};

/*----------------------------------------------------------------------
|   AP4_DescriptorUpdateCommand
+---------------------------------------------------------------------*/
/**
 * This class is used for ObjectDescriptorUpdateCommand and
 * IPMP_DescriptorUpdateCommand
 */
class AP4_DescriptorUpdateCommand : public AP4_Command
{
 public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_DescriptorUpdateCommand, AP4_Command)

    // methods
    AP4_DescriptorUpdateCommand(AP4_UI08 tag);
    AP4_DescriptorUpdateCommand(AP4_ByteStream& stream, 
                                AP4_UI08        tag,
                                AP4_Size        header_size, 
                                AP4_Size        payload_size);
    virtual ~AP4_DescriptorUpdateCommand();
    virtual AP4_Result AddDescriptor(AP4_Descriptor* descriptor);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    virtual AP4_Result Inspect(AP4_AtomInspector& inspector);

    // accessors
    const AP4_List<AP4_Descriptor>& GetDescriptors() { return m_Descriptors; }
    
 protected:
    // members
    mutable AP4_List<AP4_Descriptor> m_Descriptors;
};

#endif // _AP4_OBJECT_DESCRIPTOR_H_
