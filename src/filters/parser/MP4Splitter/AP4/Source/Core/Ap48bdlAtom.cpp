/*****************************************************************
|
|    AP4 - 8bdl Atoms
|
|    Copyright 2002-2009 Axiomatic Systems, LLC
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
#include "Ap48bdlAtom.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_8bdlAtom)

/*----------------------------------------------------------------------
|   AP4_8bdlAtom::AP4_8bdlAtom
+---------------------------------------------------------------------*/
AP4_8bdlAtom::AP4_8bdlAtom(AP4_UI32         encoding,
                           AP4_UI32         encoding_version,
                           const AP4_Byte*  data,
                           AP4_Size         data_size) :
    AP4_Atom(AP4_ATOM_TYPE_8BDL, (AP4_UI32)(AP4_ATOM_HEADER_SIZE + 8 + data_size)),
    m_Encoding(encoding),
    m_EncodingVersion(encoding_version),
    m_BundleData(data, data_size)
{
}

/*----------------------------------------------------------------------
|   AP4_8bdlAtom::Create
+---------------------------------------------------------------------*/
AP4_8bdlAtom*
AP4_8bdlAtom::Create(AP4_Size size, AP4_ByteStream& stream)
{
    // make sure we have enough data
    if(size < AP4_ATOM_HEADER_SIZE + 8)
    {
        return NULL;
    }
    else
    {
        return new AP4_8bdlAtom(size, stream);
    }
}

/*----------------------------------------------------------------------
|   AP4_8bdlAtom::AP4_8bdlAtom
+---------------------------------------------------------------------*/
AP4_8bdlAtom::AP4_8bdlAtom(AP4_Size        size,
                           AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_8BDL, (AP4_UI32)(size)),
    m_BundleData(size - AP4_ATOM_HEADER_SIZE - 8)
{
    stream.ReadUI32(m_Encoding);
    stream.ReadUI32(m_EncodingVersion);
    m_BundleData.SetDataSize(m_BundleData.GetBufferSize());
    stream.Read(m_BundleData.UseData(), m_BundleData.GetDataSize());
}

/*----------------------------------------------------------------------
|   AP4_8bdlAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_8bdlAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    // encoding
    result = stream.WriteUI32(m_Encoding);
    if(AP4_FAILED(result)) return result;

    // encoding version
    result = stream.WriteUI32(m_EncodingVersion);
    if(AP4_FAILED(result)) return result;

    // bundle_data
    result = stream.Write(m_BundleData.GetData(), m_BundleData.GetDataSize());
    if(AP4_FAILED(result)) return result;

    return result;
}

/*----------------------------------------------------------------------
|   AP4_8bdlAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_8bdlAtom::InspectFields(AP4_AtomInspector& inspector)
{
    char enc[5];
    AP4_FormatFourChars(enc, m_Encoding);
    inspector.AddField("encoding", enc);
    inspector.AddField("encoding_version", m_EncodingVersion);
    if(m_Encoding == AP4_8BDL_XML_DATA_ENCODING)
    {
        // we, in fact have an xml string
        AP4_String xml((const char*)m_BundleData.GetData(), m_BundleData.GetDataSize());
        inspector.AddField("bundle_data", xml.GetChars());
    }
    else
    {
        inspector.AddField("bundle_data", m_BundleData.GetData(), m_BundleData.GetDataSize());
    }

    return AP4_SUCCESS;
}



