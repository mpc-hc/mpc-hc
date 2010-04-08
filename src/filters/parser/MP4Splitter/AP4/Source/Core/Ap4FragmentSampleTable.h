/*****************************************************************
|
|    AP4 - Fragment Based Sample Table
|
|    Copyright 2002-2009 Axiomatic Systems, LLC
|
|
|    This atom is part of AP4 (MP4 Audio Processing Library).
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
|    along with Bento4|GPL; see the atom COPYING.  If not, write to the
|    Free Software Foundation, 59 Temple Place - Suite 330, Boston, MA
|    02111-1307, USA.
|
 ****************************************************************/

#ifndef _AP4_FRAGMENT_SAMPLE_TABLE_H_
#define _AP4_FRAGMENT_SAMPLE_TABLE_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4SampleTable.h"
#include "Ap4Array.h"

/*----------------------------------------------------------------------
|   forward declarations
+---------------------------------------------------------------------*/
class AP4_Atom;
class AP4_ByteStream;
class AP4_TrunAtom;
class AP4_TrexAtom;
class AP4_TfhdAtom;

/*----------------------------------------------------------------------
|   AP4_FragmentSampleTable
+---------------------------------------------------------------------*/
class AP4_FragmentSampleTable : public AP4_SampleTable
{
public:
    // methods
    AP4_FragmentSampleTable(AP4_ContainerAtom* traf,
                            AP4_TrexAtom*      trex,
                            AP4_ByteStream*    sample_stream,
                            AP4_Position       moof_offset,
                            AP4_Position       mdat_payload_offset, // hack because MS doesn't implement the spec correctly
                            AP4_UI64           dts_origin = 0);
    virtual ~AP4_FragmentSampleTable();

    // AP4_SampleTable methods
    virtual AP4_Result   GetSample(AP4_Ordinal sample_index, AP4_Sample& sample);
    virtual AP4_Cardinal GetSampleCount();
    virtual AP4_SampleDescription* GetSampleDescription(AP4_Ordinal sd_index);
    virtual AP4_Cardinal GetSampleDescriptionCount();
    virtual AP4_Result   GetSampleChunkPosition(AP4_Ordinal  sample_index,
            AP4_Ordinal& chunk_index,
            AP4_Ordinal& position_in_chunk);
    virtual AP4_Result   GetSampleIndexForTimeStamp(AP4_UI64 ts, AP4_Ordinal& sample_index);
    virtual AP4_Ordinal  GetNearestSyncSampleIndex(AP4_Ordinal index, bool before = true);

    // methods
    AP4_UI64 GetDuration()
    {
        return m_Duration;
    }

private:
    // members
    AP4_TrunAtom*         m_TrunAtom;
    AP4_Array<AP4_Sample> m_Samples;
    AP4_UI64              m_Duration;

    // methods
    AP4_Result AddTrun(AP4_TrunAtom*   trun,
                       AP4_TfhdAtom*   tfhd,
                       AP4_TrexAtom*   trex,
                       AP4_ByteStream* sample_stream,
                       AP4_Position    moof_offset,
                       AP4_Position&   payload_offset,
                       AP4_UI64&       dts_origin);

};

#endif // _AP4_FRAGMENT_SAMPLE_TABLE_H_
