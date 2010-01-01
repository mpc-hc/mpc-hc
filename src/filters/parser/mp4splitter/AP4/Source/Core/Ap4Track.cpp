/*****************************************************************
|
|    AP4 - Track Objects
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
#include "Ap4ByteStream.h"
#include "Ap4HdlrAtom.h"
#include "Ap4MvhdAtom.h"
#include "Ap4Track.h"
#include "Ap4Utils.h"
#include "Ap4Sample.h"
#include "Ap4DataBuffer.h"
#include "Ap4TrakAtom.h"
#include "Ap4TkhdAtom.h"
#include "Ap4MoovAtom.h"
#include "Ap4AtomSampleTable.h"
#include "Ap4SdpAtom.h"
#include "Ap4MdhdAtom.h"
#include "Ap4SyntheticSampleTable.h"

/*----------------------------------------------------------------------
|   AP4_Track::AP4_Track
+---------------------------------------------------------------------*/
AP4_Track::AP4_Track(Type             type,
                     AP4_SampleTable* sample_table,
                     AP4_UI32         track_id, 
                     AP4_UI32         movie_time_scale,
                     AP4_UI64         track_duration,
                     AP4_UI32         media_time_scale,
                     AP4_UI64         media_duration,
                     const char*      language,
                     AP4_UI32         width,
                     AP4_UI32         height) :
    m_TrakAtomIsOwned(true),
    m_Type(type),
    m_SampleTable(sample_table),
    m_SampleTableIsOwned(true),
    m_MovieTimeScale(movie_time_scale ? 
                     movie_time_scale : 
                     AP4_TRACK_DEFAULT_MOVIE_TIMESCALE)
{
    // compute the default volume value
    unsigned int volume = 0;
    if (type == TYPE_AUDIO) volume = 0x100;

    // compute the handler type and name
    AP4_Atom::Type hdlr_type;
    const char* hdlr_name;
    switch (type) {
        case TYPE_AUDIO:
            hdlr_type = AP4_HANDLER_TYPE_SOUN;
            hdlr_name = "Bento4 Sound Handler";
            break;

        case TYPE_VIDEO:
            hdlr_type = AP4_HANDLER_TYPE_VIDE;
            hdlr_name = "Bento4 Video Handler";
            break;

        case TYPE_HINT:
            hdlr_type = AP4_HANDLER_TYPE_HINT;
            hdlr_name = "Bento4 Hint Handler";
            break;

        case TYPE_TEXT:
            hdlr_type = AP4_HANDLER_TYPE_TEXT;
            hdlr_name = "Bento4 Text Handler";
            break;

        default:
            hdlr_type = 0;
            hdlr_name = NULL;
            break;
    }

    // create a trak atom
    m_TrakAtom = new AP4_TrakAtom(sample_table,
                                  hdlr_type, 
                                  hdlr_name,
                                  track_id, 
                                  0, 
                                  0, 
                                  track_duration,
                                  media_time_scale,
                                  media_duration,
                                  volume, 
                                  language,
                                  width, 
                                  height);
}

/*----------------------------------------------------------------------
|   AP4_Track::AP4_Track
+---------------------------------------------------------------------*/
AP4_Track::AP4_Track(AP4_TrakAtom&   atom, 
                     AP4_ByteStream& sample_stream, 
                     AP4_UI32        movie_time_scale) :
    m_TrakAtom(&atom),
    m_TrakAtomIsOwned(false),
    m_Type(TYPE_UNKNOWN),
    m_SampleTable(NULL),
    m_SampleTableIsOwned(true),
    m_MovieTimeScale(movie_time_scale)
{
    // find the handler type
    AP4_Atom* sub = atom.FindChild("mdia/hdlr");
    if (sub) {
        AP4_HdlrAtom* hdlr = AP4_DYNAMIC_CAST(AP4_HdlrAtom, sub);
        if (hdlr) {
            AP4_UI32 type = hdlr->GetHandlerType();
            if (type == AP4_HANDLER_TYPE_SOUN) {
                m_Type = TYPE_AUDIO;
            } else if (type == AP4_HANDLER_TYPE_VIDE) {
                m_Type = TYPE_VIDEO;
            } else if (type == AP4_HANDLER_TYPE_HINT) {
                m_Type = TYPE_HINT;
            } else if (type == AP4_HANDLER_TYPE_ODSM ||
                       type == AP4_HANDLER_TYPE_SDSM) {
                m_Type = TYPE_SYSTEM;
            } else if (type == AP4_HANDLER_TYPE_TEXT ||
                       type == AP4_HANDLER_TYPE_TX3G) {
                m_Type = TYPE_TEXT;
            } else if (type == AP4_HANDLER_TYPE_JPEG) {
                m_Type = TYPE_JPEG;
			// ==> Start patch MPC
			} else if (type == AP4_HANDLER_TYPE_SUBP) {
				m_Type = TYPE_SUBP;
			// <== End patch MPC
			}
        }
    }

    // create a facade for the stbl atom
    AP4_ContainerAtom* stbl = AP4_DYNAMIC_CAST(AP4_ContainerAtom, atom.FindChild("mdia/minf/stbl"));
    if (stbl) {
        m_SampleTable = new AP4_AtomSampleTable(stbl, sample_stream);
    }
}

/*----------------------------------------------------------------------
|   AP4_Track::~AP4_Track
+---------------------------------------------------------------------*/
AP4_Track::~AP4_Track()
{
    if (m_TrakAtomIsOwned) delete m_TrakAtom;
    if (m_SampleTableIsOwned) delete m_SampleTable;
}

/*----------------------------------------------------------------------
|   AP4_Track::Clone
+---------------------------------------------------------------------*/
AP4_Track* 
AP4_Track::Clone(AP4_Result* result)
{
    AP4_SyntheticSampleTable* sample_table = new AP4_SyntheticSampleTable();
    
    // default return value
    if (result) *result = AP4_SUCCESS;
    
    // add clones of the sample descriptions to the new sample table
    for (unsigned int i=0; ;i++) {
        AP4_SampleDescription* sample_description = GetSampleDescription(i);
        if (sample_description == NULL) break;
        sample_table->AddSampleDescription(sample_description->Clone());
    }

    AP4_Sample  sample;
    AP4_Ordinal index = 0;
    while (AP4_SUCCEEDED(GetSample(index, sample))) {
        AP4_ByteStream* data_stream;
        data_stream = sample.GetDataStream();
        sample_table->AddSample(*data_stream,
                                sample.GetOffset(),
                                sample.GetSize(),
                                sample.GetDuration(),
                                sample.GetDescriptionIndex(),
                                sample.GetDts(),
                                sample.GetCtsDelta(),
                                sample.IsSync());
        AP4_RELEASE(data_stream); // release our ref, the table has kept its own ref.
        index++;
    }    
    
    // create the cloned track
    AP4_Track* clone = new AP4_Track(GetType(),
                                     sample_table,
                                     GetId(),
                                     GetMovieTimeScale(),
                                     GetDuration(),
                                     GetMediaTimeScale(),
                                     GetMediaDuration(),
                                     GetTrackLanguage().GetChars(),
                                     GetWidth(),
                                     GetHeight());
                                     
    return clone;
}

/*----------------------------------------------------------------------
|   AP4_Track::Attach
+---------------------------------------------------------------------*/
AP4_Result
AP4_Track::Attach(AP4_MoovAtom* moov)
{
    if (!m_TrakAtomIsOwned) return AP4_ERROR_INTERNAL;
    moov->AddChild(m_TrakAtom);
    m_TrakAtomIsOwned = false;

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_Track::GetFlags
+---------------------------------------------------------------------*/
AP4_UI32
AP4_Track::GetFlags()
{
    if (m_TrakAtom) {
        AP4_TkhdAtom* tkhd = AP4_DYNAMIC_CAST(AP4_TkhdAtom, m_TrakAtom->FindChild("tkhd"));
        if (tkhd) {
            return tkhd->GetFlags();
        }
    }
    return 0;
}

/*----------------------------------------------------------------------
|   AP4_Track::SetFlags
+---------------------------------------------------------------------*/
AP4_Result
AP4_Track::SetFlags(AP4_UI32 flags)
{
    if (m_TrakAtom) {
        AP4_TkhdAtom* tkhd = AP4_DYNAMIC_CAST(AP4_TkhdAtom, m_TrakAtom->FindChild("tkhd"));
        if (tkhd) {
            tkhd->SetFlags(flags);
            return AP4_SUCCESS;
        }
    }
    return AP4_ERROR_INVALID_STATE;
}

/*----------------------------------------------------------------------
|   AP4_Track::GetHandlerType
+---------------------------------------------------------------------*/
AP4_UI32
AP4_Track::GetHandlerType()
{
    if (m_TrakAtom) {
        AP4_HdlrAtom* hdlr = AP4_DYNAMIC_CAST(AP4_HdlrAtom, m_TrakAtom->FindChild("mdia/hdlr"));
        if (hdlr) {
            return hdlr->GetHandlerType();
        }
    }
    return 0;
}

/*----------------------------------------------------------------------
|   AP4_Track::GetId
+---------------------------------------------------------------------*/
AP4_UI32
AP4_Track::GetId()
{
    return m_TrakAtom->GetId();
}

/*----------------------------------------------------------------------
|   AP4_Track::GetWidth
+---------------------------------------------------------------------*/
AP4_UI32
AP4_Track::GetWidth()
{
    return m_TrakAtom->GetWidth();
}

/*----------------------------------------------------------------------
|   AP4_Track::GetHeight
+---------------------------------------------------------------------*/
AP4_UI32
AP4_Track::GetHeight()
{
    return m_TrakAtom->GetHeight();
}

/*----------------------------------------------------------------------
|   AP4_Track::SetId
+---------------------------------------------------------------------*/
AP4_Result
AP4_Track::SetId(AP4_UI32 id)
{
    m_TrakAtom->SetId(id);

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_Track::GetDuration
+---------------------------------------------------------------------*/
AP4_UI64
AP4_Track::GetDuration()
{
    return m_TrakAtom->GetDuration();
}

/*----------------------------------------------------------------------
|   AP4_Track::GetDurationMs
+---------------------------------------------------------------------*/
AP4_UI32
AP4_Track::GetDurationMs()
{
    AP4_UI64 duration = m_TrakAtom->GetDuration();
    return AP4_DurationMsFromUnits(duration, m_MovieTimeScale);
}

/*----------------------------------------------------------------------
|   AP4_Track::GetSampleCount
+---------------------------------------------------------------------*/
AP4_Cardinal
AP4_Track::GetSampleCount()
{
    // delegate to the sample table
    return m_SampleTable ? m_SampleTable->GetSampleCount() : 0;
}

/*----------------------------------------------------------------------
|   AP4_Track::GetSample
+---------------------------------------------------------------------*/
AP4_Result 
AP4_Track::GetSample(AP4_Ordinal index, AP4_Sample& sample)
{
    // delegate to the sample table
    return m_SampleTable ? m_SampleTable->GetSample(index, sample) : AP4_FAILURE;
}

/*----------------------------------------------------------------------
|   AP4_Track::GetSampleDescription
+---------------------------------------------------------------------*/
AP4_SampleDescription*
AP4_Track::GetSampleDescription(AP4_Ordinal index)
{
    // delegate to the sample table
    return m_SampleTable ? m_SampleTable->GetSampleDescription(index) : NULL;
}

/*----------------------------------------------------------------------
|   AP4_Track::ReadSample
+---------------------------------------------------------------------*/
AP4_Result   
AP4_Track::ReadSample(AP4_Ordinal     index, 
                      AP4_Sample&     sample,
                      AP4_DataBuffer& data)
{
    AP4_Result result;

    // get the sample
    result = GetSample(index, sample);
    if (AP4_FAILED(result)) return result;

    // read the data
    return sample.ReadData(data);
}

/*----------------------------------------------------------------------
|   AP4_Track::GetSampleIndexForTimeStampMs
+---------------------------------------------------------------------*/
AP4_Result  
AP4_Track::GetSampleIndexForTimeStampMs(AP4_UI32 ts_ms, AP4_Ordinal& index)
{
    // convert the ts in the timescale of the track's media
    AP4_UI64 ts = AP4_ConvertTime(ts_ms, 1000, GetMediaTimeScale());

    return m_SampleTable->GetSampleIndexForTimeStamp(ts, index);
}

/*----------------------------------------------------------------------
|   AP4_Track::GetNearestSyncSampleIndex
+---------------------------------------------------------------------*/
AP4_Ordinal  
AP4_Track::GetNearestSyncSampleIndex(AP4_Ordinal index, bool before /* = true */)
{
    if (m_SampleTable == NULL) return index;
    return m_SampleTable->GetNearestSyncSampleIndex(index, before);
}

/*----------------------------------------------------------------------
|   AP4_Track::SetMovieTimeScale
+---------------------------------------------------------------------*/
AP4_Result
AP4_Track::SetMovieTimeScale(AP4_UI32 time_scale)
{
    // check that we can convert
    if (m_MovieTimeScale == 0) return AP4_FAILURE;

    // convert from one time scale to the other
    m_TrakAtom->SetDuration(AP4_ConvertTime(m_TrakAtom->GetDuration(), 
                                            m_MovieTimeScale,
                                            time_scale));
    
    // keep the new movie timescale
    m_MovieTimeScale = time_scale;

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   AP4_Track::GetMediaTimeScale
+---------------------------------------------------------------------*/
AP4_UI32
AP4_Track::GetMediaTimeScale()
{
    return m_TrakAtom?m_TrakAtom->GetMediaTimeScale():0;
}

/*----------------------------------------------------------------------
|   AP4_Track::GetMediaDuration
+---------------------------------------------------------------------*/
AP4_UI64
AP4_Track::GetMediaDuration()
{
    return m_TrakAtom?m_TrakAtom->GetMediaDuration():0;
}

/*----------------------------------------------------------------------
|   AP4_Track::GetTrackName
+---------------------------------------------------------------------*/
const AP4_String
AP4_Track::GetTrackName()
{
    if (AP4_HdlrAtom* hdlr = AP4_DYNAMIC_CAST(AP4_HdlrAtom, m_TrakAtom->FindChild("mdia/hdlr"))) {
        return hdlr->GetHandlerName();
    }
    return NULL;
}

/*----------------------------------------------------------------------
|   AP4_Track::GetTrackLanguage
+---------------------------------------------------------------------*/
const AP4_String
AP4_Track::GetTrackLanguage()
{
    if (AP4_MdhdAtom* mdhd = AP4_DYNAMIC_CAST(AP4_MdhdAtom, m_TrakAtom->FindChild("mdia/mdhd"))) {
        return mdhd->GetLanguage();
    }
    return NULL;
}
