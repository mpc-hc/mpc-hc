/*****************************************************************
|
|    AP4 - Track Objects
|
|    Copyright 2002 Gilles Boccon-Gibod & Julien Boeuf
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
#include "Ap4ByteStream.h"
#include "Ap4HdlrAtom.h"
#include "Ap4MvhdAtom.h"
#include "Ap4Track.h"
#include "Ap4Utils.h"
#include "Ap4Sample.h"
#include "Ap4DataBuffer.h"
#include "Ap4TrakAtom.h"
#include "Ap4MoovAtom.h"
#include "Ap4AtomSampleTable.h"
#include "Ap4SdpAtom.h"
#include "Ap4MdhdAtom.h"

/*----------------------------------------------------------------------
|       AP4_Track::AP4_Track
+---------------------------------------------------------------------*/
AP4_Track::AP4_Track(Type             type,
                     AP4_SampleTable* sample_table,
                     AP4_UI32         track_id, 
                     AP4_UI32         movie_time_scale,
                     AP4_UI32         media_time_scale,
                     AP4_UI64         media_duration,
                     const char*      language,
                     AP4_UI32         width,
                     AP4_UI32         height) :
    m_TrakAtomIsOwned(true),
    m_Type(type),
    m_SampleTable(sample_table),
    m_SampleTableIsOwned(false),
    m_MovieTimeScale(movie_time_scale ? 
                     movie_time_scale : 
                     AP4_TRACK_DEFAULT_MOVIE_TIMESCALE),
    m_MediaTimeScale(media_time_scale)
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

        default:
            hdlr_type = 0;
            hdlr_name = NULL;
            break;
    }

    // compute the track duration in units of the movie time scale
    AP4_UI64 track_duration = AP4_ConvertTime(media_duration,
                                              media_time_scale,
                                              movie_time_scale);

    // create a trak atom
    m_TrakAtom = DNew AP4_TrakAtom(sample_table,
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
|       AP4_Track::AP4_Track
+---------------------------------------------------------------------*/
AP4_Track::AP4_Track(AP4_TrakAtom&   atom, 
                     AP4_ByteStream& sample_stream, 
                     AP4_UI32        movie_time_scale) :
    m_TrakAtom(&atom),
    m_TrakAtomIsOwned(false),
    m_Type(TYPE_UNKNOWN),
    m_SampleTable(NULL),
    m_SampleTableIsOwned(true),
    m_MovieTimeScale(movie_time_scale),
    m_MediaTimeScale(0)
{
    // find the handler type
    AP4_Atom* sub = atom.FindChild("mdia/hdlr");
    if (sub) {
        AP4_HdlrAtom* hdlr = dynamic_cast<AP4_HdlrAtom*>(sub);
        if (hdlr) {
            AP4_Atom::Type type = hdlr->GetHandlerType();
            if (type == AP4_HANDLER_TYPE_SOUN) {
                m_Type = TYPE_AUDIO;
            } else if (type == AP4_HANDLER_TYPE_VIDE) {
                m_Type = TYPE_VIDEO;
            } else if (type == AP4_HANDLER_TYPE_TEXT ||
					   type == AP4_HANDLER_TYPE_SBTL ||
					   type == AP4_HANDLER_TYPE_TX3G) {
                m_Type = TYPE_TEXT;
            } else if (type == AP4_HANDLER_TYPE_SUBP) {
                m_Type = TYPE_SUBP;
            } else if (type == AP4_HANDLER_TYPE_HINT) {
                m_Type = TYPE_HINT;
            }
        }
    }

    // get the media time scale
    sub = atom.FindChild("mdia/mdhd");
    if (sub) {
        AP4_MdhdAtom* mdhd = dynamic_cast<AP4_MdhdAtom*>(sub);
        if (mdhd) {
            m_MediaTimeScale = mdhd->GetTimeScale();
        }
    }

    // create a facade for the stbl atom
    AP4_ContainerAtom* stbl = dynamic_cast<AP4_ContainerAtom*>(
        atom.FindChild("mdia/minf/stbl"));
    if (stbl) {
        m_SampleTable = DNew AP4_AtomSampleTable(stbl, sample_stream);
    }
}

/*----------------------------------------------------------------------
|       AP4_Track::~AP4_Track
+---------------------------------------------------------------------*/
AP4_Track::~AP4_Track()
{
    if (m_TrakAtomIsOwned) delete m_TrakAtom;
    if (m_SampleTableIsOwned) delete m_SampleTable;
}

/*----------------------------------------------------------------------
|       AP4_Track::GetId
+---------------------------------------------------------------------*/
AP4_UI32
AP4_Track::GetId()
{
    return m_TrakAtom->GetId();
}

/*----------------------------------------------------------------------
|       AP4_Track::SetId
+---------------------------------------------------------------------*/
AP4_Result
AP4_Track::SetId(AP4_UI32 id)
{
    m_TrakAtom->SetId(id);

    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|       AP4_Track::GetDuration
+---------------------------------------------------------------------*/
AP4_UI64
AP4_Track::GetDuration()
{
    return m_TrakAtom->GetDuration();
}

/*----------------------------------------------------------------------
|       AP4_Track::GetDurationMs
+---------------------------------------------------------------------*/
AP4_Duration
AP4_Track::GetDurationMs()
{
    AP4_UI64 duration = m_TrakAtom->GetDuration();
    return AP4_DurationMsFromUnits(duration, m_MovieTimeScale);
}

/*----------------------------------------------------------------------
|       AP4_Track::GetSampleCount
+---------------------------------------------------------------------*/
AP4_Cardinal
AP4_Track::GetSampleCount()
{
    // delegate to the sample table
    return m_SampleTable ? m_SampleTable->GetSampleCount() : 0;
}

/*----------------------------------------------------------------------
|       AP4_Track::GetSample
+---------------------------------------------------------------------*/
AP4_Result 
AP4_Track::GetSample(AP4_Ordinal index, AP4_Sample& sample)
{
    // delegate to the sample table
    return m_SampleTable ? m_SampleTable->GetSample(index, sample) : AP4_FAILURE;
}

/*----------------------------------------------------------------------
|       AP4_Track::GetSampleDescription
+---------------------------------------------------------------------*/
AP4_SampleDescription*
AP4_Track::GetSampleDescription(AP4_Ordinal index)
{
    // delegate to the sample table
    return m_SampleTable ? m_SampleTable->GetSampleDescription(index) : NULL;
}

/*----------------------------------------------------------------------
|       AP4_Track::ReadSample
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
|       AP4_Track::GetSampleIndexForTimeStampMs
+---------------------------------------------------------------------*/
AP4_Result  
AP4_Track::GetSampleIndexForTimeStampMs(AP4_TimeStamp ts, AP4_Ordinal& index)
{
    // convert the ts in the timescale of the track's media
    ts = AP4_ConvertTime(ts, 1000, m_MediaTimeScale);

    return m_SampleTable->GetSampleIndexForTimeStamp(ts, index);
}

// mpc-hc custom code start
AP4_Result  
AP4_Track::GetSampleIndexForRefTime(REFERENCE_TIME rt, AP4_Ordinal& index)
{
	AP4_TimeStamp ts = (AP4_TimeStamp)(double(rt) * m_MediaTimeScale / 10000000 + 0.5);

	return m_SampleTable->GetSampleIndexForTimeStamp(ts, index);
}
// mpc-hc custom code end

/*----------------------------------------------------------------------
|       AP4_Track::SetMovieTimeScale
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
|       AP4_Track::GetMediaTimeScale
+---------------------------------------------------------------------*/
AP4_UI32
AP4_Track::GetMediaTimeScale()
{
    return m_MediaTimeScale;
}

// save the implementation for later
#if 0 
/*----------------------------------------------------------------------
|       AP4_HintTrack::SetSdpText
+---------------------------------------------------------------------*/
void
AP4_HintTrack::SetSdpText(const char* text)
{
    // build an sdp atom
    AP4_SdpAtom* sdp = DNew AP4_SdpAtom(text);

    // build the hnti
    AP4_ContainerAtom* hnti = DNew AP4_ContainerAtom(AP4_ATOM_TYPE_HNTI);
    hnti->AddChild(sdp);

    // check if there's already a user data atom
    AP4_ContainerAtom* udta = dynamic_cast<AP4_ContainerAtom*>(m_TrakAtom->FindChild("udta"));
    if (udta == NULL) {
        // otherwise create it
        udta = DNew AP4_ContainerAtom(AP4_ATOM_TYPE_UDTA);
        m_TrakAtom->AddChild(udta);
    }
    udta->AddChild(hnti);
}

#endif

/*----------------------------------------------------------------------
|       AP4_Track::GetTrackName
+---------------------------------------------------------------------*/

AP4_String
AP4_Track::GetTrackName()
{
	AP4_String TrackName;
	if(AP4_HdlrAtom* hdlr = dynamic_cast<AP4_HdlrAtom*>(m_TrakAtom->FindChild("mdia/hdlr")))
		TrackName = hdlr->GetHandlerName();
	return TrackName;
}

/*----------------------------------------------------------------------
|       AP4_Track::GetTrackLanguage
+---------------------------------------------------------------------*/

AP4_String
AP4_Track::GetTrackLanguage()
{
	AP4_String TrackLanguage;
	if(AP4_MdhdAtom* mdhd = dynamic_cast<AP4_MdhdAtom*>(m_TrakAtom->FindChild("mdia/mdhd")))
		TrackLanguage = mdhd->GetLanguage().c_str();
	return TrackLanguage;
}
