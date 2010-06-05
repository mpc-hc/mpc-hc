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

#ifndef _AP4_TRAK_H_
#define _AP4_TRAK_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4Array.h"

// ==> Start patch MPC
#include "Ap4String.h"
// <== End patch MPC

/*----------------------------------------------------------------------
|   forward declarations
+---------------------------------------------------------------------*/
class AP4_StblAtom;
class AP4_ByteStream;
class AP4_Sample;
class AP4_DataBuffer;
class AP4_TrakAtom;
class AP4_MoovAtom;
class AP4_SampleDescription;
class AP4_SampleTable;

/*----------------------------------------------------------------------
|   constants
+---------------------------------------------------------------------*/
const AP4_UI32 AP4_TRACK_DEFAULT_MOVIE_TIMESCALE = 1000;

const AP4_UI32 AP4_TRACK_FLAG_ENABLED    = 0x0001;
const AP4_UI32 AP4_TRACK_FLAG_IN_MOVIE   = 0x0002;
const AP4_UI32 AP4_TRACK_FLAG_IN_PREVIEW = 0x0004;

/*----------------------------------------------------------------------
|   AP4_Track
+---------------------------------------------------------------------*/
class AP4_Track {
 public:
    // types
    typedef enum {
        TYPE_UNKNOWN = 0,
        TYPE_AUDIO   = 1,
        TYPE_VIDEO   = 2,
        TYPE_SYSTEM  = 3,
        TYPE_HINT    = 4,
        TYPE_TEXT    = 5,
        TYPE_JPEG    = 6,
        TYPE_RTP     = 7,
		// ==> Start patch MPC
		TYPE_SUBP	 = 8
		// <== End patch MPC
    } Type;

    // methods
    AP4_Track(Type             type,
              AP4_SampleTable* sample_table,     // ownership is transfered to the AP4_Track object
              AP4_UI32         track_id, 
              AP4_UI32         movie_time_scale, // 0 = use default
              AP4_UI64         track_duration,   // in the movie timescale
              AP4_UI32         media_time_scale,
              AP4_UI64         media_duration,   // in the media timescale
              const char*      language,
              AP4_UI32         width,            // in 16.16 fixed point
              AP4_UI32         height);          // in 16.16 fixed point
    AP4_Track(AP4_TrakAtom&   atom, 
              AP4_ByteStream& sample_stream,
              AP4_UI32        movie_time_scale);
    virtual ~AP4_Track();
    
    /** 
     * Clone a track. This is useful if you want to create a track from 
     * a non-synthetic track (parsed from a file for example) and 
     * write it out
     */
    AP4_Track* Clone(AP4_Result* result = NULL);
    
    AP4_UI32     GetFlags();
    AP4_Result   SetFlags(AP4_UI32 flags);
    AP4_Track::Type GetType() { return m_Type; }
    AP4_UI32     GetHandlerType();
    AP4_UI64     GetDuration();   // in the timescale of the movie
    AP4_UI32     GetDurationMs(); // in milliseconds
    AP4_UI32     GetWidth();      // in 16.16 fixed point
    AP4_UI32     GetHeight();     // in 16.16 fixed point
    AP4_Cardinal GetSampleCount();
    AP4_Result   GetSample(AP4_Ordinal index, AP4_Sample& sample);
    AP4_Result   ReadSample(AP4_Ordinal     index, 
                            AP4_Sample&     sample,
                            AP4_DataBuffer& data);
    AP4_Result   GetSampleIndexForTimeStampMs(AP4_UI32     ts_ms, 
                                              AP4_Ordinal& index);
    AP4_Ordinal  GetNearestSyncSampleIndex(AP4_Ordinal index, bool before=true);
    AP4_SampleDescription* GetSampleDescription(AP4_Ordinal index);
    AP4_SampleTable*       GetSampleTable() { return m_SampleTable; }
    AP4_UI32      GetId();
    AP4_Result    SetId(AP4_UI32 track_id);
    AP4_TrakAtom* GetTrakAtom() { return m_TrakAtom; }
    AP4_Result    SetMovieTimeScale(AP4_UI32 time_scale);
    AP4_UI32      GetMovieTimeScale() { return m_MovieTimeScale; }
    AP4_UI32      GetMediaTimeScale();
    AP4_UI64      GetMediaDuration(); // in the timescale of the media
    const AP4_String GetTrackName();
    const AP4_String GetTrackLanguage();
    AP4_Result    Attach(AP4_MoovAtom* moov);

 protected:
    // members
    AP4_TrakAtom*    m_TrakAtom;
    bool             m_TrakAtomIsOwned;
    Type             m_Type;
    AP4_SampleTable* m_SampleTable;
    bool             m_SampleTableIsOwned;
    AP4_UI32         m_MovieTimeScale;
};

#endif // _AP4_TRAK_H_
