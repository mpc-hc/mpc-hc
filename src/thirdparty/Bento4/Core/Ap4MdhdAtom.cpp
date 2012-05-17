/*****************************************************************
|
|    AP4 - mdhd Atoms 
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
#include "Ap4MdhdAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|       AP4_MdhdAtom::AP4_MdhdAtom
+---------------------------------------------------------------------*/
AP4_MdhdAtom::AP4_MdhdAtom(AP4_UI64    creation_time,
                           AP4_UI64    modification_time,
                           AP4_UI32    time_scale,
                           AP4_UI64    duration,
                           const char* language) :
    AP4_Atom(AP4_ATOM_TYPE_MDHD, 20+AP4_FULL_ATOM_HEADER_SIZE, true),
    m_CreationTime(creation_time),
    m_ModificationTime(modification_time),
    m_TimeScale(time_scale),
    m_Duration(duration)
{
    m_Language[0] = language[0];
    m_Language[1] = language[1];
    m_Language[2] = language[2];
}

/*----------------------------------------------------------------------
|       AP4_MdhdAtom::AP4_MdhdAtom
+---------------------------------------------------------------------*/
AP4_MdhdAtom::AP4_MdhdAtom(AP4_Size size, AP4_ByteStream& stream) :
    AP4_Atom(AP4_ATOM_TYPE_MDHD, size, true, stream),
    m_CreationTime(0),
    m_ModificationTime(0),
    m_TimeScale(0),
    m_Duration(0)
{
    m_Language[0] = 0;
    m_Language[1] = 0;
    m_Language[2] = 0;

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
    
    unsigned char lang[2];
    stream.Read(lang, 2, NULL);
    char l0 = ((lang[0]>>2)&0x1F);
    char l1 = (((lang[0]&0x3)<<3) | ((lang[1]>>5)&0x7));
    char l2 = ((lang[1]&0x1F));
    if (l0) {
        m_Language[0] = l0+0x60;
    }
    if (l1) {
        m_Language[1] = l1+0x60;
    }
    if (l2) {
        m_Language[2] = l2+0x60;
    }
}

/*----------------------------------------------------------------------
|       AP4_MdhdAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_MdhdAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result;

    if (m_Version == 0) {
        // we only deal with version 0 for the moment
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

    // write the language
    AP4_UI08 l0 = (m_Language[0]==0)?0:(m_Language[0]-0x60);
    AP4_UI08 l1 = (m_Language[1]==0)?0:(m_Language[1]-0x60);
    AP4_UI08 l2 = (m_Language[2]==0)?0:(m_Language[2]-0x60);
    result = stream.WriteUI08(l0<<2 | l1>>3);
    if (AP4_FAILED(result)) return result;
    result = stream.WriteUI08(l1<<5 | l2);
    if (AP4_FAILED(result)) return result;

    // pre-defined
    return stream.WriteUI16(0);
}

/*----------------------------------------------------------------------
|       AP4_MdhdAtom::GetDurationMs
+---------------------------------------------------------------------*/
AP4_UI32
AP4_MdhdAtom::GetDurationMs()
{
    return AP4_DurationMsFromUnits(m_Duration, m_TimeScale);
}

/*----------------------------------------------------------------------
|       AP4_MdhdAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_MdhdAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("timescale", m_TimeScale);
    inspector.AddField("duration", (AP4_UI32)m_Duration); // TODO
    inspector.AddField("duration(ms)", GetDurationMs());
    char language[4];
    AP4_StringFormat(language, sizeof(language), 
        "%c%c%c", 
        m_Language[0] ? m_Language[0]:'-',
        m_Language[1] ? m_Language[1]:'-',
        m_Language[2] ? m_Language[2]:'-');
    inspector.AddField("language", (const char*)language);

    return AP4_SUCCESS;
}
