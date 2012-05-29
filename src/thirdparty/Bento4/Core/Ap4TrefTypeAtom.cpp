/*****************************************************************
|
|    AP4 - tref type Atoms 
|
|    Copyright 2002-2005 Gilles Boccon-Gibod & Julien Boeuf
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
#include "Ap4TrefTypeAtom.h"

/*----------------------------------------------------------------------
|       AP4_TrefTypeAtom::AP4_TrefTypeAtom
+---------------------------------------------------------------------*/
AP4_TrefTypeAtom::AP4_TrefTypeAtom(AP4_Atom::Type type, 
                                   AP4_Size size, 
                                   AP4_ByteStream& stream) :
    AP4_Atom(type, size, false, stream)
{
    AP4_Size data_size = size - 8; // size and atom type
    
    // read the track ids
    while (data_size != 0) {
        AP4_UI32 track_id;
        stream.ReadUI32(track_id);
        m_TrackIds.Append(track_id);
        data_size -= 4;
    }
}

/*----------------------------------------------------------------------
|       AP4_TrefTypeAtom::WriteFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_TrefTypeAtom::WriteFields(AP4_ByteStream& stream)
{
    AP4_Result result = AP4_SUCCESS;

    AP4_Size track_id_count = m_TrackIds.ItemCount();
    for (AP4_Ordinal i=0; i<track_id_count; i++) {
        result = stream.WriteUI32(m_TrackIds[i]);
        if (AP4_FAILED(result)) return result;
    }

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_TrefTypeAtom::InspectFields
+---------------------------------------------------------------------*/
AP4_Result
AP4_TrefTypeAtom::InspectFields(AP4_AtomInspector& inspector)
{
    inspector.AddField("track_id_count", m_TrackIds.ItemCount());
    for (AP4_Ordinal i=0; i<m_TrackIds.ItemCount(); i++) {
    	inspector.AddField("track id ", m_TrackIds[i]);
    }
    return AP4_SUCCESS;
}
