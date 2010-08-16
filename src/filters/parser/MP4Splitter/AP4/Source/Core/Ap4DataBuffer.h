/*****************************************************************
|
|    AP4 - Data Buffer Objects
|
|    Copyright 2002 Gilles Boccon-Gibod & Julien Boeuf
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

#ifndef _AP4_DATA_BUFFER_H_
#define _AP4_DATA_BUFFER_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4.h"

/*----------------------------------------------------------------------
|       AP4_DataBuffer
+---------------------------------------------------------------------*/
class AP4_DataBuffer 
{
 public:
    // constructors & destructor
    AP4_DataBuffer();              
    AP4_DataBuffer(AP4_Size size);
    AP4_DataBuffer(const AP4_DataBuffer& other);
    virtual ~AP4_DataBuffer();

    // data buffer handling methods
    virtual AP4_Result SetBuffer(AP4_Byte* buffer, AP4_Size buffer_size);
    virtual AP4_Result SetBufferSize(AP4_Size buffer_size);
    virtual AP4_Size   GetBufferSize() const { return m_BufferSize; }

    // data handling methods
    virtual const AP4_Byte* GetData() const { return m_Buffer; }
    virtual AP4_Byte*       UseData() { return m_Buffer; };
    virtual AP4_Size        GetDataSize() const { return m_DataSize; }
    virtual AP4_Result      SetDataSize(AP4_Size size);
    virtual AP4_Result      SetData(AP4_Byte* data, AP4_Size data_size);

 protected:
    // members
    bool      m_BufferIsLocal;
    AP4_Byte* m_Buffer;
    AP4_Size  m_BufferSize;
    AP4_Size  m_DataSize;

    // methods
    AP4_Result ReallocateBuffer(AP4_Size size);
};
   
#endif // _AP4_DATA_BUFFER_H_

