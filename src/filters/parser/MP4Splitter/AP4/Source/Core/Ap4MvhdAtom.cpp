/*****************************************************************
|
|    AP4 - mvhd Atoms
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
#include "Ap4MvhdAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_MvhdAtom)

/*----------------------------------------------------------------------
|   AP4_MvhdAtom::Create
+---------------------------------------------------------------------*/
AP4_MvhdAtom*
AP4_MvhdAtom::Create(AP4_Size size, AP4_ByteStream& stream)
{
    AP4_UI32 version;
    AP4_UI32 flags;
    if(AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if(version > 1) return NULL;
    return new AP4_MvhdAtom(size, version, flags, stream);
}

/*----------------------------------------------------------------------
|   AP4_MvhdAtom::AP4_MvhdAtom
+---------------------------------------------------------------------*/
AP4_MvhdAtom::AP4_MvhdAtom(AP4_UI32 creation_time,
                           AP4_UI32 modification_time,
                           AP4_UI32 time_scale,
                           AP4_UI32 duration,
                           AP4_UI32 rate,
                           AP4_UI16 volume) :
    AP4_Atom(AP4_ATOM_TYPE_MVHD, AP4_FULL_ATOM_HEADER_SIZE + 96, 0, 0),
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

    AP4_SetMemory(m_Reserved1, 0, sizeof(m_Reserved1));
    AP4_SetMemory(m_Reserved2, 0, sizeof(m_Reserved2));
    AP4_SetMemory(m_Predefined, 0, sizeof(m_Predefined));
}

/*----------------------------------------------------------------------
|   AP4_MvhdAtom::AP4_MvhdAtom
+---------------------------------------------------------------------*/
AP4_MvhdAtom::AP4_MvhdAtom(AP4_UI32        size,
                           AP4_UI32        version,
                           AP4_UI32        flags,
                           AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_MVHD, size, version, flags)
{
    if(m_Version == 0)
    {
        AP4_UI32 creation_time;
        stream.ReadUI32(creation_time);
        m_CreationTime = creation_time;
        AP4_UI32 modification_time;
        stream.ReadUI32(modification_time);
        m_ModificationTime = modification_time;
        stream.ReadUI32(m_TimeScale);
        AP4_UI32 duration;
        stream.ReadUI32(duration);
        m_Duration = duration;
    }
    else
    {
        stream.ReadUI64(m_CreationTime);
        stream.ReadUI64(m_ModificationTime);
        stream.ReadUI32(m_TimeScale);
        stream.ReadUI64(m_Duration);
    }

    stream.ReadUI32(m_Rate);
    stream.ReadUI16(m_Volume);
    stream.Read(m_Reserved1, sizeof(m_Reserved1));
    stream.Read(m_Reserved2, sizeof(m_Reserved2));
    for(int i = 0; i < 9; i++)
    {
        stream.ReadUI32(m_Matrix[i]);
    }
    stream.Read(m_Predefined, sizeof(m_Predefined));
    stream.ReadUI32(m_NextTrackId);
}

/*----------------------------------------------------------------------
|   AP4_MvhdAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_MvhdAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    if(m_Version == 0)
    {
        result = stream.WriteUI32((AP4_UI32)m_CreationTime);
        if(AP4_FAILED(result)) return result;
        result = stream.WriteUI32((AP4_UI32)m_ModificationTime);
        if(AP4_FAILED(result)) return result;
        result = stream.WriteUI32(m_TimeScale);
        if(AP4_FAILED(result)) return result;
        result = stream.WriteUI32((AP4_UI32)m_Duration);
    }
    else
    {
        result = stream.WriteUI64(m_CreationTime);
        if(AP4_FAILED(result)) return result;
        result = stream.WriteUI64(m_ModificationTime);
        if(AP4_FAILED(result)) return result;
        result = stream.WriteUI32(m_TimeScale);
        if(AP4_FAILED(result)) return result;
        result = stream.WriteUI64(m_Duration);
        if(AP4_FAILED(result)) return result;
    }

    // rate & volume
    result = stream.WriteUI32(m_Rate);
    if(AP4_FAILED(result)) return result;
    result = stream.WriteUI16(m_Volume);
    if(AP4_FAILED(result)) return result;

    // reserved
    result = stream.Write(m_Reserved1, sizeof(m_Reserved1));
    if(AP4_FAILED(result)) return result;
    result = stream.Write(m_Reserved2, sizeof(m_Reserved2));
    if(AP4_FAILED(result)) return result;

    // matrix
    for(int i = 0; i < 9; i++)
    {
        result = stream.WriteUI32(m_Matrix[i]);
        if(AP4_FAILED(result)) return result;
    }

    // pre-defined
    result = stream.Write(m_Predefined, sizeof(m_Predefined));
    if(AP4_FAILED(result)) return result;

    // next track id
    return stream.WriteUI32(m_NextTrackId);
}

/*----------------------------------------------------------------------
|   AP4_MvhdAtom::GetDurationMs
+---------------------------------------------------------------------*/
AP4_UI32
AP4_MvhdAtom::GetDurationMs()
{
    if(m_TimeScale)
    {
        return (AP4_UI32)AP4_ConvertTime(m_Duration, m_TimeScale, 1000);
    }
    else
    {
        return 0;
    }
}

/*----------------------------------------------------------------------
|   AP4_MvhdAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_MvhdAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("timescale", m_TimeScale);
    inspector.AddField("duration", m_Duration);
    inspector.AddField("duration(ms)", GetDurationMs());

    return AP4_SUCCESS;
}
