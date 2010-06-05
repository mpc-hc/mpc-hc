/*****************************************************************
|
|    AP4 - Commands 
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

#ifndef _AP4_COMMAND_H_
#define _AP4_COMMAND_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Expandable.h"
#include "Ap4DynamicCast.h"

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const AP4_UI08 AP4_COMMAND_TAG_OBJECT_DESCRIPTOR_UPDATE  = 0x01;
const AP4_UI08 AP4_COMMAND_TAG_OBJECT_DESCRIPTOR_REMOVE  = 0x02;
const AP4_UI08 AP4_COMMAND_TAG_ES_DESCRIPTOR_UPDATE      = 0x03;
const AP4_UI08 AP4_COMMAND_TAG_ES_DESCRIPTOR_REMOVE      = 0x04;
const AP4_UI08 AP4_COMMAND_TAG_IPMP_DESCRIPTOR_UPDATE    = 0x05;
const AP4_UI08 AP4_COMMAND_TAG_IPMP_DESCRIPTOR_REMOVE    = 0x06;
const AP4_UI08 AP4_COMMAND_TAG_ES_DESCRIPTOR_REMOVE_REF  = 0x07;
const AP4_UI08 AP4_COMMAND_TAG_OBJECT_DESCRIPTOR_EXECUTE = 0x08;

/*----------------------------------------------------------------------
|   AP4_Command
+---------------------------------------------------------------------*/
class AP4_Command : public AP4_Expandable
{
 public:
     AP4_IMPLEMENT_DYNAMIC_CAST(AP4_Command)

    // constructor
    AP4_Command(AP4_UI08 tag, AP4_Size header_size, AP4_Size payload_size) :
        AP4_Expandable(tag, CLASS_ID_SIZE_08, header_size, payload_size) {}

    // AP4_Exandable methods
    virtual AP4_Result Inspect(AP4_AtomInspector& inspector);

    // methods
    AP4_UI08 GetTag() { return (AP4_UI08)m_ClassId; }
};

/*----------------------------------------------------------------------
|   AP4_UnknownCommand
+---------------------------------------------------------------------*/
class AP4_UnknownCommand : public AP4_Command
{
public:
    // contrusctor
    AP4_UnknownCommand(AP4_ByteStream& stream, 
                       AP4_UI08        tag,
                       AP4_Size        header_size,
                       AP4_Size        payload_size);
                          
    // AP4_Expandable methods
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);

private:
    // members
    AP4_DataBuffer m_Data;
};

#endif // _AP4_COMMAND_H_
