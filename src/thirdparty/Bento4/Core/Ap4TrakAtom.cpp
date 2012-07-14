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

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4.h"
#include "Ap4TrakAtom.h"
#include "Ap4MdhdAtom.h"
#include "Ap4VmhdAtom.h"
#include "Ap4SmhdAtom.h"
#include "Ap4HmhdAtom.h"
#include "Ap4NmhdAtom.h"
#include "Ap4DrefAtom.h"
#include "Ap4UrlAtom.h"
#include "Ap4StcoAtom.h"
#include "Ap4Co64Atom.h"
#include "Ap4AtomFactory.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|       AP4_TrakAtom::AP4_TrakAtom
+---------------------------------------------------------------------*/
AP4_TrakAtom::AP4_TrakAtom(AP4_SampleTable* sample_table,
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
                           AP4_UI32         height) :
    AP4_ContainerAtom(AP4_ATOM_TYPE_TRAK)
{
    AP4_Result result;

    // create a tkhd atom
    m_TkhdAtom = new AP4_TkhdAtom(creation_time, 
                                  modification_time, 
                                  track_id,
                                  track_duration, 
                                  volume, 
                                  width, 
                                  height);

    // create an edts

    // create a mdia atom
    AP4_ContainerAtom* mdia = new AP4_ContainerAtom(AP4_ATOM_TYPE_MDIA);

    // create a hdlr atom for the mdia atom
    m_HdlrAtom = new AP4_HdlrAtom(hdlr_type, hdlr_name);

    // create a minf atom 
    AP4_ContainerAtom* minf = new AP4_ContainerAtom(AP4_ATOM_TYPE_MINF);

    // create a media header atom for minf (vmhd, smhd, hmhd or nmhd)
    AP4_Atom* minf_header;
    switch (hdlr_type) {
        case AP4_HANDLER_TYPE_VIDE:
            minf_header = new AP4_VmhdAtom(0, 0, 0, 0);
            break;

        case AP4_HANDLER_TYPE_SOUN:
            minf_header = new AP4_SmhdAtom(0);
            break;

        default:
            minf_header = new AP4_NmhdAtom();
            break;
    }

    // create a dinf atom for minf
    AP4_ContainerAtom* dinf = new AP4_ContainerAtom(AP4_ATOM_TYPE_DINF);

    // create a url atom as a ref for dref
    AP4_Atom* url = new AP4_UrlAtom(); // local ref

    // create a dref atom for dinf
    AP4_DrefAtom* dref = new AP4_DrefAtom(&url, 1);

    // create a stbl atom for minf
    AP4_ContainerAtom* stbl;
    result = sample_table->GenerateStblAtom(stbl);
    if (AP4_FAILED(result)) stbl = NULL;
    
    // populate the dinf atom
    dinf->AddChild(dref);

    // populate the minf atom
    minf->AddChild(minf_header);
    minf->AddChild(dinf);
    if (stbl) minf->AddChild(stbl);

    // create a mdhd atom for the mdia atom
    AP4_MdhdAtom* mdhd = new AP4_MdhdAtom(creation_time,
                                          modification_time,
                                          media_time_scale,
                                          media_duration,
                                          language);

    // populate the mdia atom
    mdia->AddChild(mdhd);
    mdia->AddChild(m_HdlrAtom);
    mdia->AddChild(minf);

    // attach the children
    AddChild(m_TkhdAtom);
    AddChild(mdia);
}

/*----------------------------------------------------------------------
|       AP4_TrakAtom::AP4_TrakAtom
+---------------------------------------------------------------------*/
AP4_TrakAtom::AP4_TrakAtom(AP4_Size         size,
                           AP4_ByteStream&  stream,
                           AP4_AtomFactory& atom_factory) :
    AP4_ContainerAtom(AP4_ATOM_TYPE_TRAK, size, false, stream, atom_factory),
    m_HdlrAtom(NULL),
    m_TkhdAtom(NULL)
{
    AP4_Atom* tkhd = FindChild("tkhd");
    if (tkhd != NULL) {
        m_TkhdAtom = dynamic_cast<AP4_TkhdAtom*>(tkhd);
    } else {
        m_TkhdAtom  = NULL;
    }
}

/*----------------------------------------------------------------------
|       AP4_TrakAtom::GetDuration
+---------------------------------------------------------------------*/
AP4_UI64
AP4_TrakAtom::GetDuration()
{
    if (m_TkhdAtom) {
        return m_TkhdAtom->GetDuration();
    } else {
        return 0;
    }
}

/*----------------------------------------------------------------------
|       AP4_TrakAtom::SetDuration
+---------------------------------------------------------------------*/
AP4_Result
AP4_TrakAtom::SetDuration(AP4_UI64 duration)
{
    if (m_TkhdAtom) {
        return m_TkhdAtom->SetDuration(duration);
    } else {
        return AP4_FAILURE;
    }
}

/*----------------------------------------------------------------------
|       AP4_TrakAtom::AdjustChunkOffsets
+---------------------------------------------------------------------*/
AP4_Result    
AP4_TrakAtom::AdjustChunkOffsets(AP4_Offset offset)
{
    if (AP4_Atom* atom = FindChild("mdia/minf/stbl/co64")) {
        AP4_Co64Atom* co64 = dynamic_cast<AP4_Co64Atom*>(atom);
        co64->AdjustChunkOffsets(offset);
	}

    AP4_Atom* atom = FindChild("mdia/minf/stbl/stco");
    if (atom != NULL) {
        AP4_StcoAtom* stco = dynamic_cast<AP4_StcoAtom*>(atom);
        stco->AdjustChunkOffsets(offset);
        return AP4_SUCCESS;
    } else {
        return AP4_FAILURE;
    }
}
