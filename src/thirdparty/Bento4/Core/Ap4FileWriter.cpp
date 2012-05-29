/*****************************************************************
|
|    AP4 - File Writer
|
|    Copyright 2002-2005 Gilles Boccon-Gibod
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
#include "Ap4MoovAtom.h"
#include "Ap4FileWriter.h"

/*----------------------------------------------------------------------
|       AP4_FileWriter::AP4_FileWriter
+---------------------------------------------------------------------*/
AP4_FileWriter::AP4_FileWriter(AP4_File& file) : m_File(file)
{
}

/*----------------------------------------------------------------------
|       AP4_FileWriter::~AP4_FileWriter
+---------------------------------------------------------------------*/
AP4_FileWriter::~AP4_FileWriter()
{
}

/*----------------------------------------------------------------------
|       AP4_FileWriter::Write
+---------------------------------------------------------------------*/
AP4_Result
AP4_FileWriter::Write(AP4_ByteStream& stream)
{
    //AP4_Result result;

    // get the movie object
    AP4_Movie* movie = m_File.GetMovie();
    if (movie == NULL) return AP4_SUCCESS;

    // compute the final offset of the sample data in mdat
    AP4_Offset data_offset = movie->GetMoovAtom()->GetSize()+AP4_ATOM_HEADER_SIZE;

    // adjust the tracks
    AP4_List<AP4_Track>::Item* track_item = movie->GetTracks().FirstItem();
    while (track_item) {
        AP4_Track* track = track_item->GetData();
        track->GetTrakAtom()->AdjustChunkOffsets(data_offset);
        track_item = track_item->GetNext();
    }

    // write the moov atom
    movie->GetMoovAtom()->Write(stream);

    // create an mdat
    stream.WriteUI32(0);
    stream.Write("mdat", 4);
    AP4_Track* track = movie->GetTracks().FirstItem()->GetData();
    AP4_Cardinal sample_count = track->GetSampleCount();
    for (AP4_Ordinal i=0; i<sample_count; i++) {
        AP4_Sample sample;
        AP4_DataBuffer data;
        track->ReadSample(i, sample, data);
        stream.Write(data.GetData(), data.GetDataSize());
    }

    // TODO: update the mdat size

    return AP4_SUCCESS;
}
