/*****************************************************************
|
|    AP4 - Expandable base class 
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

#ifndef _AP4_EXPANDABLE_H_
#define _AP4_EXPANDABLE_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4DataBuffer.h"

/*----------------------------------------------------------------------
|   class references
+---------------------------------------------------------------------*/
class AP4_ByteStream;
class AP4_AtomInspector;

/*----------------------------------------------------------------------
|   AP4_Expandable
+---------------------------------------------------------------------*/
class AP4_Expandable
{
 public:
    // types
    enum ClassIdSize {
        CLASS_ID_SIZE_08
    };
     
    // class methods
    static AP4_Size MinHeaderSize(AP4_Size payload_size);

    // methods
    AP4_Expandable(AP4_UI32    class_id, 
                   ClassIdSize class_id_size, 
                   AP4_Size    header_size, 
                   AP4_Size    payload_size);
    virtual ~AP4_Expandable() {}
    AP4_UI32           GetClassId()    { return m_ClassId;                  }
    AP4_Size           GetSize()       { return m_PayloadSize+m_HeaderSize; }
    AP4_Size           GetHeaderSize() { return m_HeaderSize;               }
    virtual AP4_Result Write(AP4_ByteStream& stream);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream) = 0;
    virtual AP4_Result Inspect(AP4_AtomInspector& inspector);

 protected:
    // members
    AP4_UI32    m_ClassId;
    ClassIdSize m_ClassIdSize;
    AP4_Size    m_HeaderSize;
    AP4_Size    m_PayloadSize;
};

/*----------------------------------------------------------------------
|   AP4_UnknownExpandable
+---------------------------------------------------------------------*/
class AP4_UnknownExpandable : public AP4_Expandable 
{
public:
    // methods
    AP4_UnknownExpandable(AP4_ByteStream& stream, 
                          AP4_UI32        class_id,
                          ClassIdSize     class_id_size,
                          AP4_Size        header_size,
                          AP4_Size        payload_size);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    
private:
    // members
    AP4_DataBuffer m_Data;
};

#endif // _AP4_EXPANDABLE_H_
