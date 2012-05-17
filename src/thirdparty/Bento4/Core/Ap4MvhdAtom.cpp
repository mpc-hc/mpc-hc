/*****************************************************************
|
|    AP4 - mvhd Atoms 
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
#include "Ap4MvhdAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|       AP4_MvhdAtom::AP4_MvhdAtom
+---------------------------------------------------------------------*/
AP4_MvhdAtom::AP4_MvhdAtom(AP4_UI64 creation_time,
                           AP4_UI64 modification_time,
                           AP4_UI32 time_scale,
                           AP4_UI64 duration,
                           AP4_UI32 rate,
                           AP4_UI16 volume) :
    AP4_Atom(AP4_ATOM_TYPE_MVHD, 96+AP4_FULL_ATOM_HEADER_SIZE, true),
    m_CreationTime(creation_time),
    m_ModificationTime(modification_time),
    m_TimeScale(time_scale),
    m_Duration(duration),
    m_Rate(rate),
    m_Volume(volume),
    m_NextTrackId(0xFFFFFFFF)
{
    m_Matrix[0] = 0x00010000;
    m_Matrix[1] = 0;
    m_Matrix[2] = 0;
    m_Matrix[3] = 0;
    m_Matrix[4] = 0x00010000;
    m_Matrix[5] = 0;
    m_Matrix[6] = 0;
    m_Matrix[7] = 0;
    m_Matrix[8] = 0x40000000;

    memset(m_Reserved1, 0, sizeof(m_Reserved1));
    memset(m_Reserved2, 0, sizeof(m_Reserved2));
    memset(m_Predefined, 0, sizeof(m_Predefined));
}

/*----------------------------------------------------------------------
|       AP4_MvhdAtom::AP4_MvhdAtom
+---------------------------------------------------------------------*/
AP4_MvhdAtom::AP4_MvhdAtom(AP4_Size size, AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_MVHD, size, true, stream)
{
    if (m_Version == 0) {
        AP4_UI32 tmp = 0;
        stream.ReadUI32(tmp); m_CreationTime = tmp;
        stream.ReadUI32(tmp); m_ModificationTime = tmp;
        stream.ReadUI32(m_TimeScale);
        stream.ReadUI32(tmp); m_Duration = tmp;
	} else if (m_Version == 1) {
        stream.ReadUI64(m_CreationTime);
        stream.ReadUI64(m_ModificationTime);
        stream.ReadUI32(m_TimeScale);
        stream.ReadUI64(m_Duration);
	} else {
		// TODO
    }

    stream.ReadUI32(m_Rate);
    stream.ReadUI16(m_Volume);
    stream.Read(m_Reserved1, sizeof(m_Reserved1));
    stream.Read(m_Reserved2, sizeof(m_Reserved2));
    for (int i=0; i<9; i++) {
        stream.ReadUI32(m_Matrix[i]);
    }
    stream.Read(m_Predefined, sizeof(m_Predefined));
    stream.ReadUI32(m_NextTrackId);
}

/*----------------------------------------------------------------------
|       AP4_MvhdAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_MvhdAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;
    
    if (m_Version == 0) {
        result = stream.WriteUI32((AP4_UI32)m_CreationTime);
        if (AP4_FAILED(result)) return result;
        result = stream.WriteUI32((AP4_UI32)m_ModificationTime);
        if (AP4_FAILED(result)) return result;
        result = stream.WriteUI32(m_TimeScale);
        if (AP4_FAILED(result)) return result;
        result = stream.WriteUI32((AP4_UI32)m_Duration);
        if (AP4_FAILED(result)) return result;
	} else if (m_Version == 1) {
        result = stream.WriteUI64(m_CreationTime);
        if (AP4_FAILED(result)) return result;
        result = stream.WriteUI64(m_ModificationTime);
        if (AP4_FAILED(result)) return result;
        result = stream.WriteUI32(m_TimeScale);
        if (AP4_FAILED(result)) return result;
        result = stream.WriteUI64(m_Duration);
        if (AP4_FAILED(result)) return result;
    } else {
		// TODO
    }

    // rate & volume
    result = stream.WriteUI32(m_Rate);
    if (AP4_FAILED(result)) return result;
    result = stream.WriteUI16(m_Volume);
    if (AP4_FAILED(result)) return result;

    // reserved
    result = stream.Write(m_Reserved1, sizeof(m_Reserved1));
    if (AP4_FAILED(result)) return result;
    result = stream.Write(m_Reserved2, sizeof(m_Reserved2));
    if (AP4_FAILED(result)) return result;

    // matrix
    for (int i=0; i<9; i++) {
        result = stream.WriteUI32(m_Matrix[i]);
        if (AP4_FAILED(result)) return result;
    }

    // pre-defined
    result = stream.Write(m_Predefined, sizeof(m_Predefined));
    if (AP4_FAILED(result)) return result;

    // next track id
    return stream.WriteUI32(m_NextTrackId);
}

/*----------------------------------------------------------------------
|       AP4_MvhdAtom::GetDurationMs
+---------------------------------------------------------------------*/
AP4_Duration
AP4_MvhdAtom::GetDurationMs()
{
    if (m_TimeScale) {
        return AP4_ConvertTime(m_Duration, m_TimeScale, 1000);
    } else {
        return 0;
    }
}

/*----------------------------------------------------------------------
|       AP4_MvhdAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_MvhdAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("timescale", m_TimeScale);
    inspector.AddField("duration", (AP4_UI32)m_Duration);
    inspector.AddField("duration(ms)", (AP4_UI32)GetDurationMs());

    return AP4_SUCCESS;
}
