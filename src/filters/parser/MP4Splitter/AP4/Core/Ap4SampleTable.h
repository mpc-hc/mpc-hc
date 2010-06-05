/*****************************************************************
|
|    AP4 - Sample Table Interface
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

#ifndef _AP4_SAMPLE_TABLE_H_
#define _AP4_SAMPLE_TABLE_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4DynamicCast.h"

/*----------------------------------------------------------------------
|   class references
+---------------------------------------------------------------------*/
class AP4_Sample;
class AP4_ContainerAtom;
class AP4_SampleDescription;

/*----------------------------------------------------------------------
|   AP4_SampleTable
+---------------------------------------------------------------------*/
class AP4_SampleTable {
public:
    AP4_IMPLEMENT_DYNAMIC_CAST(AP4_SampleTable)

    // constructors and destructor
    virtual ~AP4_SampleTable() {};

    // methods
    virtual AP4_Result   GenerateStblAtom(AP4_ContainerAtom*& stbl);
    virtual AP4_Result   GetSample(AP4_Ordinal sample_index, AP4_Sample& sample) = 0;
    virtual AP4_Cardinal GetSampleCount() = 0;
    virtual AP4_Result   GetSampleChunkPosition(AP4_Ordinal  sample_index, 
                                                AP4_Ordinal& chunk_index,
                                                AP4_Ordinal& position_in_chunk) = 0;
    virtual AP4_Cardinal GetSampleDescriptionCount() = 0;
    virtual AP4_SampleDescription* GetSampleDescription(AP4_Ordinal index) = 0;
    virtual AP4_Result   GetSampleIndexForTimeStamp(AP4_UI64     ts,
                                                    AP4_Ordinal& index) = 0;
    virtual AP4_Ordinal  GetNearestSyncSampleIndex(AP4_Ordinal index, bool before=true) = 0;
};

#endif // _AP4_SAMPLE_TABLE_H_
