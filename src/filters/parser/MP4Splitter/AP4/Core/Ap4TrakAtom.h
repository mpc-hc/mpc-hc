/*****************************************************************
|
|    AP4 - trak Atoms 
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

#ifndef _AP4_TRAK_ATOM_H_
#define _AP4_TRAK_ATOM_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4ContainerAtom.h"
#include "Ap4Array.h"

/*----------------------------------------------------------------------
|   class references
+---------------------------------------------------------------------*/
class AP4_HdlrAtom;
class AP4_TkhdAtom;
class AP4_MdhdAtom;
class AP4_SampleTable;

/*----------------------------------------------------------------------
|   AP4_TrakAtom
+---------------------------------------------------------------------*/
class AP4_TrakAtom : public AP4_ContainerAtom
{
 public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_TrakAtom, AP4_ContainerAtom)

    // class methods
     static AP4_TrakAtom* Create(AP4_Size         size,
                                 AP4_ByteStream&  stream,
                                 AP4_AtomFactory& atom_factory) {
        return new AP4_TrakAtom(size, stream, atom_factory);
    }

    // methods
    AP4_TrakAtom(AP4_SampleTable* sample_table,
                 AP4_Atom::Type   hdlr_type,
                 const char*      hdlr_name,
                 AP4_UI32         track_id, 
                 AP4_UI32         creation_time,
                 AP4_UI32         modification_time,
                 AP4_UI64         track_duration,
                 AP4_UI32         media_time_scale,
                 AP4_UI64         media_duration,
                 AP4_UI16         volume,
                 const char*      language,
                 AP4_UI32         width,
                 AP4_UI32         heigh);
    AP4_TkhdAtom* GetTkhdAtom() { return m_TkhdAtom; }
    AP4_Result AdjustChunkOffsets(AP4_SI64 delta);
    AP4_Result GetChunkOffsets(AP4_Array<AP4_UI64>& chunk_offsets);
    AP4_Result SetChunkOffsets(const AP4_Array<AP4_UI64>& chunk_offsets);
    AP4_UI32   GetId();
    AP4_Result SetId(AP4_UI32 track_id);
    AP4_UI64   GetDuration();
    AP4_Result SetDuration(AP4_UI64 duration);
    AP4_UI64   GetMediaDuration();
    AP4_Result SetMediaDuration(AP4_UI32 duration);
    AP4_UI32   GetMediaTimeScale();
    AP4_Result SetMediaTimeScale(AP4_UI32 timescale);
    AP4_UI32   GetWidth();
    AP4_Result SetWidth(AP4_UI32 width);
    AP4_UI32   GetHeight();
    AP4_Result SetHeight(AP4_UI32 height);
    
 private:
    // methods
    AP4_TrakAtom(AP4_UI32         size,
                 AP4_ByteStream&  stream,
                 AP4_AtomFactory& atom_factory);

    // members
    AP4_TkhdAtom* m_TkhdAtom;
    AP4_MdhdAtom* m_MdhdAtom;
};

#endif // _AP4_TRAK_ATOM_H_
