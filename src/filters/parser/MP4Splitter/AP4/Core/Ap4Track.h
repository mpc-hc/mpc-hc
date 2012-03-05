/*****************************************************************
|
|    AP4 - Track Objects
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

#ifndef _AP4_TRAK_H_
#define _AP4_TRAK_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4.h"
#include "Ap4Array.h"
#include "Ap4SampleDescription.h"
#include "Ap4SampleTable.h"

/*----------------------------------------------------------------------
|       forward declarations
+---------------------------------------------------------------------*/
class AP4_StblAtom;
class AP4_ByteStream;
class AP4_Sample;
class AP4_DataBuffer;
class AP4_TrakAtom;
class AP4_MoovAtom;

/*----------------------------------------------------------------------
|       constants
+---------------------------------------------------------------------*/
const AP4_UI32 AP4_TRACK_DEFAULT_MOVIE_TIMESCALE = 1000;

/*----------------------------------------------------------------------
|       AP4_Track
+---------------------------------------------------------------------*/
class AP4_Track {
 public:
    // types
    typedef enum {
        TYPE_UNKNOWN,
        TYPE_AUDIO,
        TYPE_VIDEO,
        TYPE_TEXT,
        TYPE_SUBP,
        TYPE_HINT
    } Type;

    // methods
    AP4_Track(Type             type,
              AP4_SampleTable* sample_table,
              AP4_UI32         track_id, 
              AP4_UI32         movie_time_scale, // 0 = use default
              AP4_UI32         media_time_scale,
              AP4_UI64         media_duration,
              const char*      language,
              AP4_UI32         width,
              AP4_UI32         height);
    AP4_Track(AP4_TrakAtom&   atom, 
              AP4_ByteStream& sample_stream,
              AP4_UI32        movie_time_scale);
    virtual ~AP4_Track();
    AP4_Track::Type GetType() { return m_Type; }
    AP4_UI64     GetDuration();
    AP4_Duration GetDurationMs();
    AP4_Cardinal GetSampleCount();
	AP4_Result   GetSample(AP4_Ordinal index, AP4_Sample& sample);
	AP4_Result   ReadSample(AP4_Ordinal     index, 
                            AP4_Sample&     sample,
                            AP4_DataBuffer& data);
    AP4_Result   GetSampleIndexForTimeStampMs(AP4_TimeStamp ts, 
                                              AP4_Ordinal& index);
// mpc-hc custom code start
	AP4_Result   GetSampleIndexForRefTime(REFERENCE_TIME rt, AP4_Ordinal& index);
// mpc-hc custom code end
    AP4_SampleDescription* GetSampleDescription(AP4_Ordinal index);
    AP4_UI32      GetId();
    AP4_Result    SetId(AP4_UI32 track_id);
    AP4_TrakAtom* GetTrakAtom() { return m_TrakAtom; }
    AP4_Result    SetMovieTimeScale(AP4_UI32 time_scale);
    AP4_UI32      GetMediaTimeScale();

	AP4_String    GetTrackName();
	AP4_String    GetTrackLanguage();

 protected:
    // members
    AP4_TrakAtom*    m_TrakAtom;
    bool             m_TrakAtomIsOwned;
    Type             m_Type;
    AP4_SampleTable* m_SampleTable;
    bool             m_SampleTableIsOwned;
    AP4_UI32         m_MovieTimeScale;
    AP4_UI32         m_MediaTimeScale;
};

#endif // _AP4_TRAK_H_
