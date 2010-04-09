/*****************************************************************
|
|    AP4 - Sample Objects
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
#include "Ap4Sample.h"
#include "Ap4Utils.h"
#include "Ap4DataBuffer.h"
#include "Ap4Interfaces.h"
#include "Ap4ByteStream.h"
#include "Ap4Atom.h"

/*----------------------------------------------------------------------
|   AP4_Sample::AP4_Sample
+---------------------------------------------------------------------*/
AP4_Sample::AP4_Sample() :
    m_DataStream(NULL),
    m_Offset(0),
    m_Size(0),
    m_Duration(0),
    m_DescriptionIndex(0),
    m_Dts(0),
    m_CtsDelta(0),
    m_IsSync(true)
{
}

/*----------------------------------------------------------------------
|   AP4_Sample::AP4_Sample
+---------------------------------------------------------------------*/
AP4_Sample::AP4_Sample(AP4_ByteStream& data_stream,
                       AP4_Position    offset,
                       AP4_Size        size,
                       AP4_UI32        duration,
                       AP4_Ordinal     description_index,
                       AP4_UI64        dts,
                       AP4_UI32        cts_delta,
                       bool            is_sync) :
    m_Offset(offset),
    m_Size(size),
    m_Duration(duration),
    m_DescriptionIndex(description_index),
    m_Dts(dts),
    m_CtsDelta(cts_delta),
    m_IsSync(is_sync)
{
    m_DataStream = &data_stream;
    AP4_ADD_REFERENCE(m_DataStream);
}

/*----------------------------------------------------------------------
|   AP4_Sample::AP4_Sample
+---------------------------------------------------------------------*/
AP4_Sample::AP4_Sample(const AP4_Sample& other) :
    m_DataStream(other.m_DataStream),
    m_Offset(other.m_Offset),
    m_Size(other.m_Size),
    m_Duration(other.m_Duration),
    m_DescriptionIndex(other.m_DescriptionIndex),
    m_Dts(other.m_Dts),
    m_CtsDelta(other.m_CtsDelta),
    m_IsSync(other.m_IsSync)
{
    AP4_ADD_REFERENCE(m_DataStream);
}

/*----------------------------------------------------------------------
|   AP4_Sample::~AP4_Sample
+---------------------------------------------------------------------*/
AP4_Sample::~AP4_Sample()
{
    AP4_RELEASE(m_DataStream);
}

/*----------------------------------------------------------------------
|   AP4_Sample::operator=
+---------------------------------------------------------------------*/
AP4_Sample&
AP4_Sample::operator=(const AP4_Sample& other)
{
    AP4_ADD_REFERENCE(other.m_DataStream);
    AP4_RELEASE(m_DataStream);
    m_DataStream = other.m_DataStream;

    m_Offset           = other.m_Offset;
    m_Size             = other.m_Size;
    m_Duration         = other.m_Duration;
    m_DescriptionIndex = other.m_DescriptionIndex;
    m_Dts              = other.m_Dts;
    m_CtsDelta         = other.m_CtsDelta;
    m_IsSync           = other.m_IsSync;

    return *this;
}
/*----------------------------------------------------------------------
|   AP4_Sample::ReadData
+---------------------------------------------------------------------*/
AP4_Result
AP4_Sample::ReadData(AP4_DataBuffer& data)
{
    return ReadData(data, m_Size);
}


/*----------------------------------------------------------------------
|   AP4_Sample::ReadData
+---------------------------------------------------------------------*/
AP4_Result
AP4_Sample::ReadData(AP4_DataBuffer& data, AP4_Size size, AP4_Size offset)
{
    // check that we have a stream
    if (m_DataStream == NULL) return AP4_FAILURE;

    // shortcut
    if (size == 0) return AP4_SUCCESS;

    // check the size
    if (m_Size < size+offset) return AP4_FAILURE;

    // set the buffer size
    AP4_Result result = data.SetDataSize(size);
    if (AP4_FAILED(result)) return result;

    // get the data from the stream
    result = m_DataStream->Seek(m_Offset+offset);
    if (AP4_FAILED(result)) return result;
    return m_DataStream->Read(data.UseData(), size);
}

/*----------------------------------------------------------------------
|   AP4_Sample::GetDataStream
+---------------------------------------------------------------------*/
AP4_ByteStream*
AP4_Sample::GetDataStream()
{
    AP4_ADD_REFERENCE(m_DataStream);
    return m_DataStream;
}

/*----------------------------------------------------------------------
|   AP4_Sample::SetDataStream
+---------------------------------------------------------------------*/
void
AP4_Sample::SetDataStream(AP4_ByteStream& stream)
{
    AP4_RELEASE(m_DataStream);
    m_DataStream = &stream;
    AP4_ADD_REFERENCE(m_DataStream);
}
