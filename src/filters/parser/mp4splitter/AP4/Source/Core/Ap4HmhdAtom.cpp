/*****************************************************************
|
|    AP4 - hmhd Atoms 
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

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4.h"
#include "Ap4HmhdAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|       AP4_HmhdAtom::AP4_HmhdAtom
+---------------------------------------------------------------------*/
AP4_HmhdAtom::AP4_HmhdAtom(AP4_Size size, AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_HMHD, size, true, stream)
{
    stream.ReadUI16(m_MaxPduSize);
    stream.ReadUI16(m_AvgPduSize);
    stream.ReadUI32(m_MaxBitrate);
    stream.ReadUI32(m_AvgBitrate);
    stream.ReadUI32(m_Reserved);
}

/*----------------------------------------------------------------------
|       AP4_HmhdAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_HmhdAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    // max pdu size
    result = stream.WriteUI16(m_MaxPduSize);
    if (AP4_FAILED(result)) return result;

    // avg pdu size
    result = stream.WriteUI16(m_AvgPduSize);
    if (AP4_FAILED(result)) return result;

    // max bitrate
    result = stream.WriteUI32(m_MaxBitrate);
    if (AP4_FAILED(result)) return result;

    // avg bitrate
    result = stream.WriteUI32(m_AvgBitrate);
    if (AP4_FAILED(result)) return result;

    // reserved
    result = stream.WriteUI32(m_Reserved);
    if (AP4_FAILED(result)) return result;

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_HmhdAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_HmhdAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("max_pdu_size", m_MaxPduSize);
    inspector.AddField("avg_pdu_size", m_AvgPduSize);
    inspector.AddField("max_bitrate",  m_MaxBitrate);
    inspector.AddField("avg_bitrate",  m_AvgBitrate);

    return AP4_SUCCESS;
}
