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

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4TrakAtom.h"
#include "Ap4MdhdAtom.h"
#include "Ap4TkhdAtom.h"
#include "Ap4HdlrAtom.h"
#include "Ap4VmhdAtom.h"
#include "Ap4SmhdAtom.h"
#include "Ap4HmhdAtom.h"
#include "Ap4NmhdAtom.h"
#include "Ap4DrefAtom.h"
#include "Ap4UrlAtom.h"
#include "Ap4StcoAtom.h"
#include "Ap4Co64Atom.h"
#include "Ap4AtomFactory.h"
#include "Ap4SampleTable.h"
#include "Ap4Utils.h"

/*----------------------------------------------------------------------
|   dynamic cast support
+---------------------------------------------------------------------*/
AP4_DEFINE_DYNAMIC_CAST_ANCHOR(AP4_TrakAtom)

/*----------------------------------------------------------------------
|   AP4_TrakAtom::AP4_TrakAtom
+---------------------------------------------------------------------*/
AP4_TrakAtom::AP4_TrakAtom(AP4_SampleTable* sample_table,
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
    AP4_HdlrAtom* hdlr = new AP4_HdlrAtom(hdlr_type, hdlr_name);

    // create a minf atom
    AP4_ContainerAtom* minf = new AP4_ContainerAtom(AP4_ATOM_TYPE_MINF);

    // create a media header atom for minf (vmhd, smhd, hmhd or nmhd)
    AP4_Atom* minf_header;
    switch(hdlr_type)
    {
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
    if(AP4_FAILED(result)) stbl = NULL;

    // populate the dinf atom
    dinf->AddChild(dref);

    // populate the minf atom
    minf->AddChild(minf_header);
    minf->AddChild(dinf);
    if(stbl) minf->AddChild(stbl);

    // create a mdhd atom for the mdia atom
    m_MdhdAtom = new AP4_MdhdAtom(creation_time,
                                  modification_time,
                                  media_time_scale,
                                  media_duration,
                                  language);

    // populate the mdia atom
    mdia->AddChild(m_MdhdAtom);
    mdia->AddChild(hdlr);
    mdia->AddChild(minf);

    // attach the children
    AddChild(m_TkhdAtom);
    AddChild(mdia);
}

/*----------------------------------------------------------------------
|   AP4_TrakAtom::AP4_TrakAtom
+---------------------------------------------------------------------*/
AP4_TrakAtom::AP4_TrakAtom(AP4_UI32         size,
                           AP4_ByteStream&  stream,
                           AP4_AtomFactory& atom_factory) :
    AP4_ContainerAtom(AP4_ATOM_TYPE_TRAK, size, false, stream, atom_factory)
{
    m_TkhdAtom = AP4_DYNAMIC_CAST(AP4_TkhdAtom, FindChild("tkhd"));
    m_MdhdAtom = AP4_DYNAMIC_CAST(AP4_MdhdAtom, FindChild("mdia/mdhd"));
}

/*----------------------------------------------------------------------
|   AP4_TrakAtom::GetId
+---------------------------------------------------------------------*/
AP4_UI32
AP4_TrakAtom::GetId()
{
    return m_TkhdAtom ? m_TkhdAtom->GetTrackId() : 0;
}

/*----------------------------------------------------------------------
|   AP4_TrakAtom::SetId
+---------------------------------------------------------------------*/
AP4_Result
AP4_TrakAtom::SetId(AP4_UI32 id)
{
    if(m_TkhdAtom)
    {
        m_TkhdAtom->SetTrackId(id);
        return AP4_SUCCESS;
    }
    else
    {
        return AP4_ERROR_INVALID_STATE;
    }
}

/*----------------------------------------------------------------------
|   AP4_TrakAtom::GetMediaTimeScale
+---------------------------------------------------------------------*/
AP4_UI32
AP4_TrakAtom::GetMediaTimeScale()
{
    return m_MdhdAtom ? m_MdhdAtom->GetTimeScale() : 0;
}

/*----------------------------------------------------------------------
|   AP4_TrakAtom::SetMediaTimeScale
+---------------------------------------------------------------------*/
AP4_Result
AP4_TrakAtom::SetMediaTimeScale(AP4_UI32 timescale)
{
    if(m_MdhdAtom)
    {
        m_MdhdAtom->SetTimeScale(timescale);
        return AP4_SUCCESS;
    }
    else
    {
        return AP4_ERROR_INVALID_STATE;
    }
}

/*----------------------------------------------------------------------
|   AP4_TrakAtom::GetDuration
+---------------------------------------------------------------------*/
AP4_UI64
AP4_TrakAtom::GetDuration()
{
    return m_TkhdAtom ? m_TkhdAtom->GetDuration() : 0;
}

/*----------------------------------------------------------------------
|   AP4_TrakAtom::SetDuration
+---------------------------------------------------------------------*/
AP4_Result
AP4_TrakAtom::SetDuration(AP4_UI64 duration)
{
    if(m_TkhdAtom)
    {
        m_TkhdAtom->SetDuration(duration);
        return AP4_SUCCESS;
    }
    else
    {
        return AP4_ERROR_INVALID_STATE;
    }
}

/*----------------------------------------------------------------------
|   AP4_TrakAtom::GetMediaDuration
+---------------------------------------------------------------------*/
AP4_UI64
AP4_TrakAtom::GetMediaDuration()
{
    return m_MdhdAtom ? m_MdhdAtom->GetDuration() : 0;
}

/*----------------------------------------------------------------------
|   AP4_TrakAtom::SetMediaDuration
+---------------------------------------------------------------------*/
AP4_Result
AP4_TrakAtom::SetMediaDuration(AP4_UI32 duration)
{
    if(m_MdhdAtom)
    {
        m_MdhdAtom->SetDuration(duration);
        return AP4_SUCCESS;
    }
    else
    {
        return AP4_ERROR_INVALID_STATE;
    }
}

/*----------------------------------------------------------------------
|   AP4_TrakAtom::GetWidth
+---------------------------------------------------------------------*/
AP4_UI32
AP4_TrakAtom::GetWidth()
{
    return m_TkhdAtom ? m_TkhdAtom->GetWidth() : 0;
}

/*----------------------------------------------------------------------
|   AP4_TrakAtom::SetWidth
+---------------------------------------------------------------------*/
AP4_Result
AP4_TrakAtom::SetWidth(AP4_UI32 width)
{
    if(m_TkhdAtom)
    {
        m_TkhdAtom->SetWidth(width);
        return AP4_SUCCESS;
    }
    else
    {
        return AP4_ERROR_INVALID_STATE;
    }
}

/*----------------------------------------------------------------------
|   AP4_TrakAtom::GetHeight
+---------------------------------------------------------------------*/
AP4_UI32
AP4_TrakAtom::GetHeight()
{
    return m_TkhdAtom ? m_TkhdAtom->GetHeight() : 0;
}

/*----------------------------------------------------------------------
|   AP4_TrakAtom::SetHeight
+---------------------------------------------------------------------*/
AP4_Result
AP4_TrakAtom::SetHeight(AP4_UI32 height)
{
    if(m_TkhdAtom)
    {
        m_TkhdAtom->SetHeight(height);
        return AP4_SUCCESS;
    }
    else
    {
        return AP4_ERROR_INVALID_STATE;
    }
}

/*----------------------------------------------------------------------
|   AP4_TrakAtom::AdjustChunkOffsets
+---------------------------------------------------------------------*/
AP4_Result
AP4_TrakAtom::AdjustChunkOffsets(AP4_SI64 delta)
{
    AP4_Atom* atom;
    if((atom = FindChild("mdia/minf/stbl/stco")))
    {
        AP4_StcoAtom* stco = AP4_DYNAMIC_CAST(AP4_StcoAtom, atom);
        return stco->AdjustChunkOffsets((int)delta);
    }
    else if((atom = FindChild("mdia/minf/stbl/co64")))
    {
        AP4_Co64Atom* co64 = AP4_DYNAMIC_CAST(AP4_Co64Atom, atom);
        return co64->AdjustChunkOffsets(delta);
    }
    else
    {
        return AP4_ERROR_INVALID_STATE;
    }
}

/*----------------------------------------------------------------------
|   AP4_TrakAtom::GetChunkOffsets
+---------------------------------------------------------------------*/
AP4_Result
AP4_TrakAtom::GetChunkOffsets(AP4_Array<AP4_UI64>& chunk_offsets)
{
    AP4_Atom* atom;
    if((atom = FindChild("mdia/minf/stbl/stco")))
    {
        AP4_StcoAtom* stco = AP4_DYNAMIC_CAST(AP4_StcoAtom, atom);
        if(stco == NULL) return AP4_ERROR_INTERNAL;
        AP4_Cardinal    stco_chunk_count   = stco->GetChunkCount();
        const AP4_UI32* stco_chunk_offsets = stco->GetChunkOffsets();
        chunk_offsets.SetItemCount(stco_chunk_count);
        for(unsigned int i = 0; i < stco_chunk_count; i++)
        {
            chunk_offsets[i] = stco_chunk_offsets[i];
        }
        return AP4_SUCCESS;
    }
    else if((atom = FindChild("mdia/minf/stbl/co64")))
    {
        AP4_Co64Atom* co64 = AP4_DYNAMIC_CAST(AP4_Co64Atom, atom);
        if(co64 == NULL) return AP4_ERROR_INTERNAL;
        AP4_Cardinal    co64_chunk_count   = co64->GetChunkCount();
        const AP4_UI64* co64_chunk_offsets = co64->GetChunkOffsets();
        chunk_offsets.SetItemCount(co64_chunk_count);
        for(unsigned int i = 0; i < co64_chunk_count; i++)
        {
            chunk_offsets[i] = co64_chunk_offsets[i];
        }
        return AP4_SUCCESS;
    }
    else
    {
        return AP4_ERROR_INVALID_STATE;
    }
}

/*----------------------------------------------------------------------
|   AP4_TrakAtom::SetChunkOffsets
+---------------------------------------------------------------------*/
AP4_Result
AP4_TrakAtom::SetChunkOffsets(const AP4_Array<AP4_UI64>& chunk_offsets)
{
    AP4_Atom* atom;
    if((atom = FindChild("mdia/minf/stbl/stco")))
    {
        AP4_StcoAtom* stco = AP4_DYNAMIC_CAST(AP4_StcoAtom, atom);
        if(stco == NULL) return AP4_ERROR_INTERNAL;
        AP4_Cardinal stco_chunk_count   = stco->GetChunkCount();
        AP4_UI32*    stco_chunk_offsets = stco->GetChunkOffsets();
        if(stco_chunk_count > chunk_offsets.ItemCount())
        {
            return AP4_ERROR_OUT_OF_RANGE;
        }
        for(unsigned int i = 0; i < stco_chunk_count; i++)
        {
            stco_chunk_offsets[i] = (AP4_UI32)chunk_offsets[i];
        }
        return AP4_SUCCESS;
    }
    else if((atom = FindChild("mdia/minf/stbl/co64")))
    {
        AP4_Co64Atom* co64 = AP4_DYNAMIC_CAST(AP4_Co64Atom, atom);
        if(co64 == NULL) return AP4_ERROR_INTERNAL;
        AP4_Cardinal co64_chunk_count   = co64->GetChunkCount();
        AP4_UI64*    co64_chunk_offsets = co64->GetChunkOffsets();
        if(co64_chunk_count > chunk_offsets.ItemCount())
        {
            return AP4_ERROR_OUT_OF_RANGE;
        }
        for(unsigned int i = 0; i < co64_chunk_count; i++)
        {
            co64_chunk_offsets[i] = chunk_offsets[i];
        }
        return AP4_SUCCESS;
    }
    else
    {
        return AP4_ERROR_INVALID_STATE;
    }
}
