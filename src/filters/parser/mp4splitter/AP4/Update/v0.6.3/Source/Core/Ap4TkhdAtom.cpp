/*****************************************************************
|
|    AP4 - tkhd Atoms 
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
#include "Ap4TkhdAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|       AP4_TkhdAtom::AP4_TkhdAtom
+---------------------------------------------------------------------*/
AP4_TkhdAtom::AP4_TkhdAtom(AP4_UI64 creation_time,
                           AP4_UI64 modification_time,
                           AP4_UI32 track_id,
                           AP4_UI64 duration,
                           AP4_UI16 volume,
                           AP4_UI32 width,
                           AP4_UI32 height) :
    AP4_Atom(AP4_ATOM_TYPE_TKHD, 80+AP4_FULL_ATOM_HEADER_SIZE, true),
    m_CreationTime(creation_time),
    m_ModificationTime(modification_time),
    m_TrackId(track_id),
    m_Reserved1(0),
    m_Duration(duration),
    m_Layer(0),
    m_AlternateGroup(0),
    m_Volume(volume),
    m_Reserved3(0),
    m_Width(width),
    m_Height(height)
{
    m_Flags = AP4_TKHD_FLAG_DEFAULTS;

    m_Matrix[0] = 0x00010000;
    m_Matrix[1] = 0;
    m_Matrix[2] = 0;
    m_Matrix[3] = 0;
    m_Matrix[4] = 0x00010000;
    m_Matrix[5] = 0;
    m_Matrix[6] = 0;
    m_Matrix[7] = 0;
    m_Matrix[8] = 0x40000000;

    m_Reserved2[0] = 0;
    m_Reserved2[1] = 0;
}

/*----------------------------------------------------------------------
|       AP4_TkhdAtom::AP4_TkhdAtom
+---------------------------------------------------------------------*/
AP4_TkhdAtom::AP4_TkhdAtom(AP4_Size size, AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_TKHD, size, true, stream)
{
    if (m_Version == 0) {
		AP4_UI32 tmp = 0;
        stream.ReadUI32(tmp); m_CreationTime = tmp;
        stream.ReadUI32(tmp); m_ModificationTime = tmp;
        stream.ReadUI32(m_TrackId);
        stream.ReadUI32(m_Reserved1);
        stream.ReadUI32(tmp); m_Duration = tmp;
	} else if (m_Version == 1) {
        stream.ReadUI64(m_CreationTime);
        stream.ReadUI64(m_ModificationTime);
        stream.ReadUI32(m_TrackId);
        stream.ReadUI32(m_Reserved1);
        stream.ReadUI64(m_Duration);
    } else {
        // TODO
    }

    stream.Read((void*)m_Reserved2, 8, NULL);
    stream.ReadUI16(m_Layer);
    stream.ReadUI16(m_AlternateGroup);
    stream.ReadUI16(m_Volume);
    stream.ReadUI16(m_Reserved3);
    for (int i=0; i<9; i++) {
        stream.ReadUI32(m_Matrix[i]);
    }
    stream.ReadUI32(m_Width);
    stream.ReadUI32(m_Height);
}

/*----------------------------------------------------------------------
|       AP4_TkhdAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_TkhdAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    // creation/modification time, track id, reserved1 & duration
    if (m_Version == 0) {
        result = stream.WriteUI32((AP4_UI32)m_CreationTime);
        if (AP4_FAILED(result)) return result;
        result = stream.WriteUI32((AP4_UI32)m_ModificationTime);
        if (AP4_FAILED(result)) return result;
        result = stream.WriteUI32(m_TrackId);
        if (AP4_FAILED(result)) return result;
        result = stream.WriteUI32(m_Reserved1);
        if (AP4_FAILED(result)) return result;
        result = stream.WriteUI32((AP4_UI32)m_Duration);
        if (AP4_FAILED(result)) return result;
    } else if (m_Version == 1) {
        result = stream.WriteUI64(m_CreationTime);
        if (AP4_FAILED(result)) return result;
        result = stream.WriteUI64(m_ModificationTime);
        if (AP4_FAILED(result)) return result;
        result = stream.WriteUI32(m_TrackId);
        if (AP4_FAILED(result)) return result;
        result = stream.WriteUI32(m_Reserved1);
        if (AP4_FAILED(result)) return result;
        result = stream.WriteUI64(m_Duration);
        if (AP4_FAILED(result)) return result;
    } else {
		// TODO
    }

    // reserved2
    result = stream.Write(m_Reserved2, sizeof(m_Reserved2));
    if (AP4_FAILED(result)) return result;

    // layer, alternate group & volume
    result = stream.WriteUI16(m_Layer);
    if (AP4_FAILED(result)) return result;
    result = stream.WriteUI16(m_AlternateGroup);
    if (AP4_FAILED(result)) return result;
    result = stream.WriteUI16(m_Volume);
    if (AP4_FAILED(result)) return result;

    // reserved3
    result = stream.WriteUI16(m_Reserved3);

    // matrix
    for (int i=0; i<9; i++) {
        result = stream.WriteUI32(m_Matrix[i]);
        if (AP4_FAILED(result)) return result;
    }

    // width & height
    result = stream.WriteUI32(m_Width);
    if (AP4_FAILED(result)) return result;
    result = stream.WriteUI32(m_Height);
    if (AP4_FAILED(result)) return result;

    return result;
}

/*----------------------------------------------------------------------
|       AP4_TkhdAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_TkhdAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("enabled", ((m_Flags & AP4_TKHD_FLAG_TRACK_ENABLED) ? 1 : 0), AP4_AtomInspector::HINT_BOOLEAN);
    inspector.AddField("id", m_TrackId);
    inspector.AddField("duration", (AP4_UI32)m_Duration);

    return AP4_SUCCESS;
}
