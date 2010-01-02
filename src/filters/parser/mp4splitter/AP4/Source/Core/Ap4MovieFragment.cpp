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

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4File.h"
#include "Ap4Atom.h"
#include "Ap4MfhdAtom.h"
#include "Ap4TfhdAtom.h"
#include "Ap4TrexAtom.h"
#include "Ap4MovieFragment.h"
#include "Ap4ContainerAtom.h"
#include "Ap4FragmentSampleTable.h"
#include "Ap4Movie.h"
#include "Ap4Sample.h"

/*----------------------------------------------------------------------
|   AP4_MovieFragment::AP4_MovieFragment
+---------------------------------------------------------------------*/
AP4_MovieFragment::AP4_MovieFragment(AP4_ContainerAtom* moof) :
    m_MoofAtom(moof),
    m_MfhdAtom(NULL)
{
    if (moof) m_MfhdAtom = AP4_DYNAMIC_CAST(AP4_MfhdAtom, moof->GetChild(AP4_ATOM_TYPE_MFHD));
}

/*----------------------------------------------------------------------
|   AP4_MovieFragment::~AP4_MovieFragment
+---------------------------------------------------------------------*/
AP4_MovieFragment::~AP4_MovieFragment()
{
    delete m_MoofAtom;
}

/*----------------------------------------------------------------------
|   AP4_MovieFragment::GetSequenceNumber
+---------------------------------------------------------------------*/
AP4_UI32
AP4_MovieFragment::GetSequenceNumber()
{
    return m_MfhdAtom?m_MfhdAtom->GetSequenceNumber():0;
}

/*----------------------------------------------------------------------
|   AP4_MovieFragment::GetTrackIds
+---------------------------------------------------------------------*/
AP4_Result
AP4_MovieFragment::GetTrackIds(AP4_Array<AP4_UI32>& ids)
{
    ids.Clear();
    ids.EnsureCapacity(m_MoofAtom->GetChildren().ItemCount());
    
    for (AP4_List<AP4_Atom>::Item* item = m_MoofAtom->GetChildren().FirstItem();
                                   item;
                                   item = item->GetNext()) {
        AP4_Atom* atom = item->GetData();
        if (atom->GetType() == AP4_ATOM_TYPE_TRAF) {
            AP4_ContainerAtom* traf = AP4_DYNAMIC_CAST(AP4_ContainerAtom, atom);
            if (traf) {
                AP4_TfhdAtom* tfhd = AP4_DYNAMIC_CAST(AP4_TfhdAtom, traf->GetChild(AP4_ATOM_TYPE_TFHD));
                if (tfhd) ids.Append(tfhd->GetTrackId());
            }
        }
    }
    
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_MovieFragment::GetTrafAtom
+---------------------------------------------------------------------*/
AP4_Result         
AP4_MovieFragment::GetTrafAtom(AP4_UI32 track_id, AP4_ContainerAtom*& traf)
{
    for (AP4_List<AP4_Atom>::Item* item = m_MoofAtom->GetChildren().FirstItem();
                                   item;
                                   item = item->GetNext()) {
        AP4_Atom* atom = item->GetData();
        if (atom->GetType() == AP4_ATOM_TYPE_TRAF) {
            traf = AP4_DYNAMIC_CAST(AP4_ContainerAtom, atom);
            if (traf) {
                AP4_TfhdAtom* tfhd = AP4_DYNAMIC_CAST(AP4_TfhdAtom, traf->GetChild(AP4_ATOM_TYPE_TFHD));
                if (tfhd && tfhd->GetTrackId() == track_id) {
                    return AP4_SUCCESS;
                }
            }
        }
    }
    
    // not found
    traf = NULL;
    return AP4_ERROR_NO_SUCH_ITEM;
}

/*----------------------------------------------------------------------
|   AP4_MovieFragment::CreateSampleTable
+---------------------------------------------------------------------*/
AP4_Result         
AP4_MovieFragment::CreateSampleTable(AP4_MoovAtom*             moov,
                                     AP4_UI32                  track_id, 
                                     AP4_ByteStream*           sample_stream,
                                     AP4_Position              moof_offset,
                                     AP4_Position              mdat_payload_offset,
                                     AP4_FragmentSampleTable*& sample_table)
{
    // default value
    sample_table = NULL;
    
    // find a trex for this track, if any
    AP4_ContainerAtom* mvex = NULL;
    AP4_TrexAtom*      trex = NULL;
    if (moov) {
        mvex = AP4_DYNAMIC_CAST(AP4_ContainerAtom, moov->GetChild(AP4_ATOM_TYPE_MVEX));
    }
    if (mvex) {
        for (AP4_List<AP4_Atom>::Item* item = mvex->GetChildren().FirstItem();
                                       item;
                                       item = item->GetNext()) {
            AP4_Atom* atom = item->GetData();
            if (atom->GetType() == AP4_ATOM_TYPE_TREX) {
                trex = AP4_DYNAMIC_CAST(AP4_TrexAtom, atom);
                if (trex && trex->GetTrackId() == track_id) break;
                trex = NULL;
            }
        }
    }
    AP4_ContainerAtom* traf = NULL;
    if (AP4_SUCCEEDED(GetTrafAtom(track_id, traf))) {
        sample_table = new AP4_FragmentSampleTable(traf, 
                                                   trex, 
                                                   sample_stream,
                                                   moof_offset,
                                                   mdat_payload_offset);
        return AP4_SUCCESS;
    }
    
    return AP4_ERROR_NO_SUCH_ITEM;
}

/*----------------------------------------------------------------------
|   AP4_MovieFragment::CreateSampleTable
+---------------------------------------------------------------------*/
AP4_Result         
AP4_MovieFragment::CreateSampleTable(AP4_Movie*                movie,
                                     AP4_UI32                  track_id, 
                                     AP4_ByteStream*           sample_stream,
                                     AP4_Position              moof_offset,
                                     AP4_Position              mdat_payload_offset,
                                     AP4_FragmentSampleTable*& sample_table)
{
    AP4_MoovAtom* moov = movie?movie->GetMoovAtom():NULL;
    return CreateSampleTable(moov, track_id, sample_stream, moof_offset, mdat_payload_offset, sample_table);
}
