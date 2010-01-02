/*****************************************************************
|
|    AP4 - Sample Objects
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

#ifndef _AP4_SAMPLE_H_
#define _AP4_SAMPLE_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"

/*----------------------------------------------------------------------
|   class references
+---------------------------------------------------------------------*/
class AP4_ByteStream;
class AP4_DataBuffer;

/*----------------------------------------------------------------------
|   AP4_Sample DO NOT DERIVE FROM THIS CLASS
+---------------------------------------------------------------------*/
class AP4_Sample 
{
public:
    /**
     * Default constructor
     */
    AP4_Sample();

    /**
     * Copy constructor
     */
    AP4_Sample(const AP4_Sample& other);
    
    /**
     * Construct an AP4_Sample referencing a data stream
     *
     * @param data_stream The byte stream that contains the sample data. 
     * The sample object added to the track will keep a reference to that byte 
     * stream
     * @param offset Position of the first byte of sample data within the stream
     * @param size Size in bytes of the sample data
     * @param description_index Index of the sample description that applies to 
     * this sample
     * @param dts Decoding timestamp of the sample
     * @param cts_delta Difference between the CTS (composition/display timestamp) and the 
     * DTS (decoding timestamp), in the timescale of the media.
     * @param sync_flag Boolean flag indicating whether this is a sync sample
     * or not
     */
    AP4_Sample(AP4_ByteStream& data_stream,
               AP4_Position    offset,
               AP4_Size        size,
               AP4_UI32        duration,
               AP4_Ordinal     description_index,
               AP4_UI64        dts,
               AP4_UI32        cts_delta,
               bool            sync_flag);
               
    ~AP4_Sample(); // not virtual on purpose: do not derive from it

    // operators
    AP4_Sample&     operator=(const AP4_Sample& other);

    // methods
    AP4_Result      ReadData(AP4_DataBuffer& data);
    AP4_Result      ReadData(AP4_DataBuffer& data, 
                             AP4_Size        size, 
                             AP4_Size        offset = 0);

    // sample properties accessors
    AP4_ByteStream* GetDataStream();
    void            SetDataStream(AP4_ByteStream& stream);
    AP4_Position    GetOffset() const { return m_Offset; }
    void            SetOffset(AP4_Position offset) { m_Offset = offset; }
    AP4_Size        GetSize() { return m_Size; }
    void            SetSize(AP4_Size size) { m_Size = size; }
    AP4_Ordinal     GetDescriptionIndex() const { return m_DescriptionIndex; }
    void            SetDescriptionIndex(AP4_Ordinal index) { m_DescriptionIndex = index; }
    
    /**
     * Get the DTS (Decoding Time Stamp) of the sample in the timescale of the media
     */
    AP4_UI64        GetDts() const { return m_Dts; }

    /**
     * Set the DTS (Decoding Time Stamp) of the sample in the timescale of the media
     */
    void            SetDts(AP4_UI64 dts) { m_Dts = dts; }

    /**
     * Get the CTS (Composition Time Stamp) of the sample in the timescale of the media
     */
    AP4_UI64        GetCts() const { return m_Dts+m_CtsDelta; }

    /**
     * Set the CTS (Composition Time Stamp) of the sample in the timescale of the media
     */
    void            SetCts(AP4_UI64 cts) { m_CtsDelta = (cts > m_Dts) ? (AP4_UI32)(cts-m_Dts) : 0;  }

    /**
     * Get the CTS Delta (difference between the CTS (Composition Time Stamp) and DTS (Decoding Time Stamp)
     * of the sample in the timescale of the media.
     */
    AP4_UI32        GetCtsDelta() const { return m_CtsDelta; }

    /**
     * Set the CTS Delta (difference between the CTS (Composition Time Stamp) and DTS (Decoding Time Stamp)
     * of the sample in the timescale of the media.
     */
    void            SetCtsDelta(AP4_UI32 delta) { m_CtsDelta = (AP4_SI32)delta;  }

    /**
     * Get the duration of the sample in the timescale of the media
     */
    AP4_UI32        GetDuration() const { return m_Duration; }

    /**
     * Set the duration of the sample in the timescale of the media
     */
    void            SetDuration(AP4_UI32 duration) { m_Duration = duration; }

    /**
     * Return whether the sample is a sync (random-access point) sample or not.
     */
    bool            IsSync() const { return m_IsSync; }

    /**
     * Set whether the sample is a sync (random-access point) sample or not.
     */
    void            SetSync(bool is_sync) { m_IsSync = is_sync; }

protected:
    AP4_ByteStream* m_DataStream;
    AP4_Position    m_Offset;
    AP4_Size        m_Size;
    AP4_UI32        m_Duration;
    AP4_Ordinal     m_DescriptionIndex;
    AP4_UI64        m_Dts;
    AP4_SI32        m_CtsDelta; // make this a signed value, because quicktime can use negative offsets
    bool            m_IsSync;
};

#endif // _AP4_SAMPLE_H_
