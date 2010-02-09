/*****************************************************************
|
|    AP4 - mdhd Atoms 
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
#include "Ap4MdhdAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_MdhdAtom)

/*----------------------------------------------------------------------
|   AP4_MdhdAtom::Create
+---------------------------------------------------------------------*/
AP4_MdhdAtom*
AP4_MdhdAtom::Create(AP4_Size size, AP4_ByteStream& stream)
{
    AP4_UI32 version;
    AP4_UI32 flags;
    if (AP4_FAILED(AP4_Atom::ReadFullHeader(stream, version, flags))) return NULL;
    if (version > 1) return NULL;
    return new AP4_MdhdAtom(size, version, flags, stream);
}

/*----------------------------------------------------------------------
|   AP4_MdhdAtom::AP4_MdhdAtom
+---------------------------------------------------------------------*/
AP4_MdhdAtom::AP4_MdhdAtom(AP4_UI32    creation_time,
                           AP4_UI32    modification_time,
                           AP4_UI32    time_scale,
                           AP4_UI64    duration,
                           const char* language) :
    AP4_Atom(AP4_ATOM_TYPE_MDHD, AP4_FULL_ATOM_HEADER_SIZE+20, 0, 0),
    m_CreationTime(creation_time),
    m_ModificationTime(modification_time),
    m_TimeScale(time_scale),
    m_Duration(duration)
{
    m_Language.Assign(language, 3);
}

/*----------------------------------------------------------------------
|   AP4_MdhdAtom::AP4_MdhdAtom
+---------------------------------------------------------------------*/
AP4_MdhdAtom::AP4_MdhdAtom(AP4_UI32        size, 
                           AP4_UI32        version,
                           AP4_UI32        flags,
                           AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_MDHD, size, version, flags)
{
    if (m_Version == 0) {
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
    } else {
        stream.ReadUI64(m_CreationTime);
        stream.ReadUI64(m_ModificationTime);
        stream.ReadUI32(m_TimeScale);
        stream.ReadUI64(m_Duration);
    }
    
    unsigned char lang[2];
    stream.Read(lang, 2);
    char l0 = ((lang[0]>>2)&0x1F);
    char l1 = (((lang[0]&0x3)<<3) | ((lang[1]>>5)&0x7));
    char l2 = ((lang[1]&0x1F));
    if (l0 && l1 && l2) {
        char lang_str[3] = {l0+0x60, l1+0x60, l2+0x60};
        m_Language.Assign(lang_str, 3);
    } else {
        m_Language.Assign("```", 3);
    }
}

/*----------------------------------------------------------------------
|   AP4_MdhdAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_MdhdAtom::WriteFields(AP4_ByteStream& stream)
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
    } else {
        result = stream.WriteUI64(m_CreationTime);
        if (AP4_FAILED(result)) return result;
        result = stream.WriteUI64(m_ModificationTime);
        if (AP4_FAILED(result)) return result;
        result = stream.WriteUI32(m_TimeScale);
        if (AP4_FAILED(result)) return result;
        result = stream.WriteUI64(m_Duration);
        if (AP4_FAILED(result)) return result;
    }

    // write the language
    AP4_UI08 l0 = m_Language[0]-0x60;
    AP4_UI08 l1 = m_Language[1]-0x60;
    AP4_UI08 l2 = m_Language[2]-0x60;
    result = stream.WriteUI08(l0<<2 | l1>>3);
    if (AP4_FAILED(result)) return result;
    result = stream.WriteUI08(l1<<5 | l2);
    if (AP4_FAILED(result)) return result;

    // pre-defined
    return stream.WriteUI16(0);
}

/*----------------------------------------------------------------------
|   AP4_MdhdAtom::GetDurationMs
+---------------------------------------------------------------------*/
AP4_UI32
AP4_MdhdAtom::GetDurationMs()
{
    return AP4_DurationMsFromUnits(m_Duration, m_TimeScale);
}

/*----------------------------------------------------------------------
|   AP4_MdhdAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_MdhdAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("timescale", m_TimeScale);
    inspector.AddField("duration", m_Duration);
    inspector.AddField("duration(ms)", GetDurationMs());
    inspector.AddField("language", m_Language.GetChars());

    return AP4_SUCCESS;
}
