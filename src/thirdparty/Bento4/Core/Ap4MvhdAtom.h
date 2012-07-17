/*****************************************************************
|
|    AP4 - mvhd Atoms 
|
|    Copyright 2002-2005 Gilles Boccon-Gibod
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

#ifndef _AP4_MVHD_ATOM_H_
#define _AP4_MVHD_ATOM_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4.h"
#include "Ap4ByteStream.h"
#include "Ap4List.h"
#include "Ap4Atom.h"

/*----------------------------------------------------------------------
|       AP4_MvhdAtom
+---------------------------------------------------------------------*/
class AP4_MvhdAtom : public AP4_Atom
{
public:
    // methods
    AP4_MvhdAtom(AP4_UI64 creation_time,
                 AP4_UI64 modification_time,
                 AP4_UI32 time_scale,
                 AP4_UI64 duration,
                 AP4_UI32 rate,
                 AP4_UI16 volume);
    AP4_MvhdAtom(AP4_Size size, AP4_ByteStream& stream); 
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    AP4_UI64           GetDuration() { return m_Duration; }
    void               SetDuration(AP4_UI64 duration) { m_Duration = duration;}
    AP4_Duration       GetDurationMs();
    AP4_UI32           GetTimeScale() { return m_TimeScale; }
    AP4_Result         SetTimeScale(AP4_UI32 time_scale) {
        m_TimeScale = time_scale;
        return AP4_SUCCESS;
    }

private:
    // members
    AP4_UI64 m_CreationTime;
    AP4_UI64 m_ModificationTime;
    AP4_UI32 m_TimeScale;
    AP4_UI64 m_Duration;
    AP4_UI32 m_Rate;
    AP4_UI16 m_Volume;
    AP4_UI08 m_Reserved1[2];
    AP4_UI08 m_Reserved2[8];
    AP4_UI32 m_Matrix[9];
    AP4_UI08 m_Predefined[24];
    AP4_UI32 m_NextTrackId;
};

#endif // _AP4_MVHD_ATOM_H_
