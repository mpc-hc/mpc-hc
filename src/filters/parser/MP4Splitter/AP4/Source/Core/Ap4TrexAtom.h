/*****************************************************************
|
|    AP4 - trex Atoms 
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

#ifndef _AP4_TREX_ATOM_H_
#define _AP4_TREX_ATOM_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4List.h"
#include "Ap4Atom.h"

/*----------------------------------------------------------------------
|   macros
+---------------------------------------------------------------------*/
#define AP4_TREX_SAMPLE_IS_DIFFERENCE_SAMPLE(_flags) ( (_flags)&

/*----------------------------------------------------------------------
|   AP4_TrexAtom
+---------------------------------------------------------------------*/
class AP4_TrexAtom : public AP4_Atom
{
public:
    AP4_IMPLEMENT_DYNAMIC_CAST_D(AP4_TrexAtom, AP4_Atom)

    // class methods
    static AP4_TrexAtom* Create(AP4_Size size, AP4_ByteStream& stream);

    // methods
    AP4_TrexAtom(AP4_UI32 track_id,
                 AP4_UI32 default_sample_description_index,
                 AP4_UI32 default_sample_duration,
                 AP4_UI32 default_sample_size,
                 AP4_UI32 default_sample_flags);
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);

    // accessors
    AP4_UI32 GetTrackId()                       { return m_TrackId;                       }
    AP4_UI32 GetDefaultSampleDescriptionIndex() { return m_DefaultSampleDescriptionIndex; }
    AP4_UI32 GetDefaultSampleDuration()         { return m_DefaultSampleDuration;         }
    AP4_UI32 GetDefaultSampleSize()             { return m_DefaultSampleSize;             }
    AP4_UI32 GetDefaultSampleFlags()            { return m_DefaultSampleFlags;            }

private:
    // methods
    AP4_TrexAtom(AP4_UI32        size, 
                 AP4_UI32        version,
                 AP4_UI32        flags,
                 AP4_ByteStream& stream); 

    // members
    AP4_UI32 m_TrackId; 
    AP4_UI32 m_DefaultSampleDescriptionIndex; 
    AP4_UI32 m_DefaultSampleDuration; 
    AP4_UI32 m_DefaultSampleSize; 
    AP4_UI32 m_DefaultSampleFlags; 
};

#endif // _AP4_TREX_ATOM_H_
