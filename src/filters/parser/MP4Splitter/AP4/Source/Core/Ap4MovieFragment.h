/*****************************************************************
|
|    AP4 - Movie Fragments
|
|    Copyright 2002-2009 Axiomatic Systems, LLC
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

#ifndef _AP4_MOVIE_FRAGMENT_H_
#define _AP4_MOVIE_FRAGMENT_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4MfhdAtom.h"
#include "Ap4List.h"

/*----------------------------------------------------------------------
|   class references
+---------------------------------------------------------------------*/
class AP4_ByteStream;
class AP4_ContainerAtom;
class AP4_FragmentSampleTable;
class AP4_Movie;
class AP4_MoovAtom;

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const AP4_UI32 AP4_FRAG_FLAG_SAMPLE_IS_DIFFERENCE = 0x00010000;

/*----------------------------------------------------------------------
|   AP4_MovieFragment
+---------------------------------------------------------------------*/
class AP4_MovieFragment {
public:
    // this constructor transfers the ownership of the moof atom to the
    // newly constructed object
    AP4_MovieFragment(AP4_ContainerAtom* moof);
    virtual ~AP4_MovieFragment();

    AP4_ContainerAtom* GetMoofAtom() { return m_MoofAtom;}
    AP4_MfhdAtom*      GetMfhdAtom() { return m_MfhdAtom;}
    AP4_UI32           GetSequenceNumber();
    AP4_Result         GetTrackIds(AP4_Array<AP4_UI32>& ids);
    AP4_Result         GetTrafAtom(AP4_UI32 track_id, AP4_ContainerAtom*& traf);
    AP4_Result         CreateSampleTable(AP4_MoovAtom*             moov,
                                         AP4_UI32                  track_id, 
                                         AP4_ByteStream*           sample_stream,
                                         AP4_Position              moof_offset,
                                         AP4_Position              mdat_payload_offset, // hack because MS doesn't implement the spec properly
                                         AP4_FragmentSampleTable*& sample_table);
    AP4_Result         CreateSampleTable(AP4_Movie*                movie,
                                         AP4_UI32                  track_id, 
                                         AP4_ByteStream*           sample_stream,
                                         AP4_Position              moof_offset,
                                         AP4_Position              mdat_payload_offset, // hack because MS doesn't implement the spec properly
                                         AP4_FragmentSampleTable*& sample_table);
    
private:
    // members
    AP4_ContainerAtom*  m_MoofAtom;
    AP4_MfhdAtom*       m_MfhdAtom;
};

#endif // _AP4_MOVIE_FRAGMENT_H_
