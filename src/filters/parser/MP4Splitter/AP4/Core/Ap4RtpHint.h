/*****************************************************************
|
|    AP4 - RTP Hint Objects
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

#ifndef _AP4_RTP_HINT_H_
#define _AP4_RTP_HINT_H_

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4Types.h"
#include "Ap4List.h"
#include "Ap4DataBuffer.h"
#include "Ap4Interfaces.h"

/*----------------------------------------------------------------------
|   forward declarations
+---------------------------------------------------------------------*/
class AP4_ByteStream;
class AP4_RtpConstructor;
class AP4_RtpPacket;

/*----------------------------------------------------------------------
|   AP4_RtpSampleData
+---------------------------------------------------------------------*/
class AP4_RtpSampleData
{
public:
    // constructors and destructor
    AP4_RtpSampleData(AP4_ByteStream& stream, AP4_UI32 size);
    AP4_RtpSampleData() {}
    virtual ~AP4_RtpSampleData();

    // methods
    virtual AP4_Result          AddPacket(AP4_RtpPacket* packet);
    virtual AP4_Size            GetSize();
    virtual AP4_ByteStream*     ToByteStream();
    
    // accessors
    AP4_List<AP4_RtpPacket>& GetPackets() {
        return m_Packets;
    }
    const AP4_DataBuffer& GetExtraData() const {
        return m_ExtraData;
    }

protected:
    // members
    AP4_List<AP4_RtpPacket>     m_Packets;
    AP4_DataBuffer              m_ExtraData;
};

/*----------------------------------------------------------------------
|   AP4_RtpPacket
+---------------------------------------------------------------------*/
class AP4_RtpPacket : public AP4_Referenceable
{
public:
    // constructor and destructor
    AP4_RtpPacket(AP4_ByteStream& stream);
    AP4_RtpPacket(int      relative_time,
                  bool     p_bit,
                  bool     x_bit,
                  bool     m_bit,
                  AP4_UI08 payload_type,
                  AP4_UI16 sequence_seed,
                  int      time_stamp_offset = 0,
                  bool     bframe_flag = false,
                  bool     repeat_flag = false);
    ~AP4_RtpPacket();

    // methods
    AP4_Result Write(AP4_ByteStream& stream);
    AP4_Result AddConstructor(AP4_RtpConstructor* constructor);
    AP4_Size GetSize();
    AP4_Size GetConstructedDataSize();

    // Referenceable methods
    void AddReference();
    void Release();
    
    // Accessors
    int GetRelativeTime() const { return m_RelativeTime; }
    bool GetPBit() const { return m_PBit; }
    bool GetXBit() const { return m_XBit; }
    bool GetMBit() const { return m_MBit; }
    AP4_UI08 GetPayloadType() const { return m_PayloadType; }
    AP4_UI16 GetSequenceSeed() const { return m_SequenceSeed; }
    int  GetTimeStampOffset() const { return m_TimeStampOffset; }
    bool GetBFrameFlag() const { return m_BFrameFlag; }
    bool GetRepeatFlag() const { return m_RepeatFlag; }
    AP4_List<AP4_RtpConstructor>& GetConstructors() {
        return m_Constructors;
    }

private:
    // members
    AP4_Cardinal                    m_ReferenceCount;                        
    int                             m_RelativeTime;
    bool                            m_PBit;
    bool                            m_XBit;
    bool                            m_MBit;
    AP4_UI08                        m_PayloadType;
    AP4_UI16                        m_SequenceSeed;
    int                             m_TimeStampOffset;
    bool                            m_BFrameFlag;
    bool                            m_RepeatFlag;
    AP4_List<AP4_RtpConstructor>    m_Constructors;
};

/*----------------------------------------------------------------------
|   AP4_RtpContructor
+---------------------------------------------------------------------*/
class AP4_RtpConstructor : public AP4_Referenceable
{
public:
    // types
    typedef AP4_UI08 Type;

    // constructor & destructor
    AP4_RtpConstructor(Type type) : m_ReferenceCount(1), m_Type(type) {}

    // methods
    Type GetType() const { return m_Type; }
    AP4_Result Write(AP4_ByteStream& stream);
    virtual AP4_Size GetConstructedDataSize() = 0;

    // Referenceable methods
    void AddReference();
    void Release();

protected:
    // methods
    virtual ~AP4_RtpConstructor() {}
    virtual AP4_Result DoWrite(AP4_ByteStream& stream) = 0;

    // members
    AP4_Cardinal m_ReferenceCount;
    Type         m_Type;
};

/*----------------------------------------------------------------------
|   constructor size
+---------------------------------------------------------------------*/
const AP4_Size AP4_RTP_CONSTRUCTOR_SIZE = 16;

/*----------------------------------------------------------------------
|   constructor types
+---------------------------------------------------------------------*/
const AP4_RtpConstructor::Type AP4_RTP_CONSTRUCTOR_TYPE_NOOP        = 0;
const AP4_RtpConstructor::Type AP4_RTP_CONSTRUCTOR_TYPE_IMMEDIATE   = 1;
const AP4_RtpConstructor::Type AP4_RTP_CONSTRUCTOR_TYPE_SAMPLE      = 2;
const AP4_RtpConstructor::Type AP4_RTP_CONSTRUCTOR_TYPE_SAMPLE_DESC = 3;

/*----------------------------------------------------------------------
|   AP4_NoopRtpConstructor
+---------------------------------------------------------------------*/
class AP4_NoopRtpConstructor : public AP4_RtpConstructor
{
public:
    // constructor
    AP4_NoopRtpConstructor(AP4_ByteStream& stream);
    AP4_NoopRtpConstructor() : AP4_RtpConstructor(AP4_RTP_CONSTRUCTOR_TYPE_NOOP) {}

    // methods
    virtual AP4_Size GetConstructedDataSize() { return 0; }

protected:
    // methods
    virtual AP4_Result DoWrite(AP4_ByteStream& stream);
};

/*----------------------------------------------------------------------
|   AP4_ImmediateRtpConstructor
+---------------------------------------------------------------------*/
class AP4_ImmediateRtpConstructor : public AP4_RtpConstructor
{
public:
    // constructor
    AP4_ImmediateRtpConstructor(AP4_ByteStream& stream);
    AP4_ImmediateRtpConstructor(const AP4_DataBuffer& data);
    
    // accessors
    const AP4_DataBuffer& GetData() const { return m_Data; }

    // methods
    virtual AP4_Size GetConstructedDataSize() { return m_Data.GetDataSize(); }

protected:
    // methods
    virtual AP4_Result DoWrite(AP4_ByteStream& stream);

    // members
    AP4_DataBuffer m_Data;
};

/*----------------------------------------------------------------------
|   AP4_SampleRtpConstructor
+---------------------------------------------------------------------*/
class AP4_SampleRtpConstructor : public AP4_RtpConstructor
{
public:
    // constructor
    AP4_SampleRtpConstructor(AP4_ByteStream& stream);
    AP4_SampleRtpConstructor(AP4_UI08 track_ref_index,
                             AP4_UI16 length,
                             AP4_UI32 sample_num,
                             AP4_UI32 sample_offset);
    
    // accessors
    AP4_UI08 GetTrackRefIndex() const { return m_TrackRefIndex; }
    AP4_UI16 GetLength() const { return m_Length; }
    AP4_UI32 GetSampleNum() const { return m_SampleNum; }
    AP4_UI32 GetSampleOffset() const { return m_SampleOffset; }

    // methods
    virtual AP4_Size GetConstructedDataSize() { return m_Length; }

protected:
    // methods
    virtual AP4_Result DoWrite(AP4_ByteStream& stream);

    // members
    AP4_UI08    m_TrackRefIndex;
    AP4_UI16    m_Length;
    AP4_UI32    m_SampleNum;
    AP4_UI32    m_SampleOffset;
};

/*----------------------------------------------------------------------
|   AP4_SampleDescRtpConstructor
+---------------------------------------------------------------------*/
class AP4_SampleDescRtpConstructor : public AP4_RtpConstructor
{
public:
    // constructor
    AP4_SampleDescRtpConstructor(AP4_ByteStream& stream);
    AP4_SampleDescRtpConstructor(AP4_UI08 track_ref_index,
                                 AP4_UI16 length,
                                 AP4_UI32 sample_desc_index,
                                 AP4_UI32 sample_desc_offset);

    // accessors
    AP4_UI08 GetTrackRefIndex() const { return m_TrackRefIndex; }
    AP4_UI16 GetLength() const { return m_Length; }
    AP4_UI32 GetSampleDescIndex() const { return m_SampleDescIndex; }
    AP4_UI32 GetSampleDescOffset() const { return m_SampleDescOffset; }

    // methods
    virtual AP4_Size GetConstructedDataSize() { return m_Length; }
        
protected:
    // methods
    virtual AP4_Result DoWrite(AP4_ByteStream& stream);

    // members
    AP4_UI08    m_TrackRefIndex;
    AP4_UI16    m_Length;
    AP4_UI32    m_SampleDescIndex;
    AP4_UI32    m_SampleDescOffset;
};

/*----------------------------------------------------------------------
|   AP4_RtpConstructorFactory
+---------------------------------------------------------------------*/
class AP4_RtpConstructorFactory 
{
public:
    static AP4_Result CreateConstructorFromStream(AP4_ByteStream&      stream,
                                                  AP4_RtpConstructor*& constructor);
};

#endif // _AP4_RTP_HINT_H_
