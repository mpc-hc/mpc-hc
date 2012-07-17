/*****************************************************************
|
|    AP4 - trak Atoms 
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

#ifndef _AP4_TRAK_ATOM_H_
#define _AP4_TRAK_ATOM_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4.h"
#include "Ap4ByteStream.h"
#include "Ap4List.h"
#include "Ap4Atom.h"
#include "Ap4HdlrAtom.h"
#include "Ap4TkhdAtom.h"
#include "Ap4ContainerAtom.h"
#include "Ap4SampleTable.h"

/*----------------------------------------------------------------------
|       AP4_TrakAtom
+---------------------------------------------------------------------*/
class AP4_TrakAtom : public AP4_ContainerAtom
{
 public:
    // methods
     AP4_TrakAtom(AP4_SampleTable* sample_table,
                  AP4_Atom::Type   hdlr_type,
                  const char*      hdlr_name,
                  AP4_UI32         track_id, 
                  AP4_UI64         creation_time,
                  AP4_UI64         modification_time,
                  AP4_UI64         track_duration,
                  AP4_UI32         media_time_scale,
                  AP4_UI64         media_duration,
                  AP4_UI16         volume,
                  const char*      language,
                  AP4_UI32         width,
                  AP4_UI32         heigh);
    AP4_TrakAtom(AP4_Size         size,
                 AP4_ByteStream&  stream,
                 AP4_AtomFactory& atom_factory);
    AP4_Result AdjustChunkOffsets(AP4_Offset offset);
    AP4_UI32   GetId() { 
        return m_TkhdAtom->GetTrackId(); 
    }
    AP4_Result SetId(AP4_UI32 track_id) {
        return m_TkhdAtom->SetTrackId(track_id);
    }
    AP4_UI64   GetDuration();
    AP4_Result SetDuration(AP4_UI64 duration);
    AP4_TkhdAtom* GetTkhdAtom() { return m_TkhdAtom; }
    AP4_HdlrAtom* GetHdlrAtom() { return m_HdlrAtom; }

 private:
    // members
    AP4_HdlrAtom* m_HdlrAtom;
    AP4_TkhdAtom* m_TkhdAtom;
};

#endif // _AP4_TRAK_ATOM_H_
