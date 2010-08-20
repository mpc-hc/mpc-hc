/*****************************************************************
|
|    AP4 - Synthetic Sample Table
|
|    Copyright 2003-2005 Gilles Boccon-Gibod & Julien Boeuf
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

#ifndef _AP4_SYNTHETIC_SAMPLE_TABLE_H_
#define _AP4_SYNTHETIC_SAMPLE_TABLE_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4.h"
#include "Ap4Atom.h"
#include "Ap4Array.h"
#include "Ap4SampleTable.h"

/*----------------------------------------------------------------------
|       forward declarations
+---------------------------------------------------------------------*/
class AP4_ByteStream;

/*----------------------------------------------------------------------
|       AP4_SyntheticSampleTable
+---------------------------------------------------------------------*/
class AP4_SyntheticSampleTable : public AP4_SampleTable
{
 public:
    // methods
             AP4_SyntheticSampleTable();
    virtual ~AP4_SyntheticSampleTable();

    // AP4_SampleTable methods
    virtual AP4_Result GetSample(AP4_Ordinal index, AP4_Sample& sample);
    virtual AP4_Cardinal GetSampleCount();
    virtual AP4_Cardinal GetSampleDescriptionCount();
    virtual AP4_SampleDescription* GetSampleDescription(AP4_Ordinal index);
    virtual AP4_Result GetSampleIndexForTimeStamp(AP4_TimeStamp ts,
                                                  AP4_Ordinal& index);

    // methods
    virtual AP4_Result AddSampleDescription(AP4_SampleDescription* description);
    virtual AP4_Result AddSample(AP4_ByteStream& data_stream,
                                 AP4_Offset      offset,
                                 AP4_Size        size,
                                 AP4_Ordinal     description_index,
                                 AP4_TimeStamp   cts = 0,
                                 AP4_TimeStamp   dts = 0,
                                 bool            sync = false);

private:
    // members
    AP4_Array<AP4_Sample>           m_Samples;
    AP4_List<AP4_SampleDescription> m_SampleDescriptions;
};

#endif // _AP4_SYNTHETIC_SAMPLE_TABLE_H_
