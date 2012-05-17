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

#ifndef _AP4_MDHD_ATOM_H_
#define _AP4_MDHD_ATOM_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4.h"
#include "Ap4ByteStream.h"
#include "Ap4List.h"
#include "Ap4Atom.h"

/*----------------------------------------------------------------------
|       constants
+---------------------------------------------------------------------*/
const AP4_UI32 AP4_MDHD_DEFAULT_GENERIC_TIMESCALE = 1000;
const AP4_UI32 AP4_MDHD_DEFAULT_VIDEO_TIMESCALE = 90000;

/*----------------------------------------------------------------------
|       AP4_MdhdAtom
+---------------------------------------------------------------------*/
class AP4_MdhdAtom : public AP4_Atom
{
 public:
    // methods
    AP4_MdhdAtom(AP4_UI64    creation_time,
                 AP4_UI64    modification_time,
                 AP4_UI32    time_scale,
                 AP4_UI64    duration,
                 const char* language);
    AP4_MdhdAtom(AP4_Size size, AP4_ByteStream& stream);
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);

    AP4_UI32 GetDurationMs();
    AP4_UI64 GetDuration()  { return m_Duration;  }
    AP4_UI32 GetTimeScale() { return m_TimeScale; }
	AP4_String GetLanguage() { return AP4_String(m_Language, 3); }

 private:
    // members
    AP4_UI64 m_CreationTime;
    AP4_UI64 m_ModificationTime;
    AP4_UI32 m_TimeScale;
    AP4_UI64 m_Duration;
    char     m_Language[3];
};

#endif // _AP4_MDHD_ATOM_H_
