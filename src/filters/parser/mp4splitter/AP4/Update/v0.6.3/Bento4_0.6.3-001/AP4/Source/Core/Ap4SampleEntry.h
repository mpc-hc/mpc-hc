/*****************************************************************
|
|    AP4 - sample entries
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

#ifndef _AP4_SAMPLE_ENTRY_H_
#define _AP4_SAMPLE_ENTRY_H_

/*----------------------------------------------------------------------
|       includes
+---------------------------------------------------------------------*/
#include "Ap4.h"
#include "Ap4ByteStream.h"
#include "Ap4List.h"
#include "Ap4Atom.h"
#include "Ap4EsdsAtom.h"
#include "Ap4AtomFactory.h"
#include "Ap4ContainerAtom.h"

/*----------------------------------------------------------------------
|       class references
+---------------------------------------------------------------------*/
class AP4_SampleDescription;

/*----------------------------------------------------------------------
|       AP4_SampleEntry
+---------------------------------------------------------------------*/
class AP4_SampleEntry : public AP4_ContainerAtom
{
 public: 
    // methods
    AP4_SampleEntry(AP4_Atom::Type format, AP4_UI16 data_ref_index = 1);
    AP4_SampleEntry(AP4_Atom::Type   format, 
                    AP4_Size         size,
                    AP4_ByteStream&  stream,
                    AP4_AtomFactory& atom_factory);
    AP4_UI16           GetDataReferenceIndex() { return m_DataReferenceIndex; }
    virtual AP4_Result Write(AP4_ByteStream& stream);
    virtual AP4_Result Inspect(AP4_AtomInspector& inspector);
    virtual AP4_SampleDescription* ToSampleDescription();

    // AP4_AtomParent methods
    virtual void OnChildChanged(AP4_Atom* child);

 protected:
    // constructor
    AP4_SampleEntry(AP4_Atom::Type format, AP4_Size size);

    // methods
    virtual AP4_Size   GetFieldsSize();
    virtual AP4_Result ReadFields(AP4_ByteStream& stream);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);

    // members
    AP4_UI08 m_Reserved1[6];         // = 0
    AP4_UI16 m_DataReferenceIndex;
};

/*----------------------------------------------------------------------
|       AP4_MpegSampleEntry
+---------------------------------------------------------------------*/
class AP4_MpegSampleEntry : public AP4_SampleEntry
{
protected:
    // constructor
    AP4_MpegSampleEntry(AP4_Atom::Type format);
    AP4_MpegSampleEntry(AP4_Atom::Type format, AP4_Size size);
    AP4_MpegSampleEntry(AP4_Atom::Type    format, 
                        AP4_EsDescriptor* descriptor);
    AP4_MpegSampleEntry(AP4_Atom::Type   format,
                        AP4_Size         size,
                        AP4_ByteStream&  stream,
                        AP4_AtomFactory& atom_factory);

    // methods
    const AP4_DecoderConfigDescriptor* GetDecoderConfigDescriptor();
};

/*----------------------------------------------------------------------
|       AP4_Mp4sSampleEntry
+---------------------------------------------------------------------*/
class AP4_Mp4sSampleEntry : public AP4_MpegSampleEntry
{
 public:
    // constructors
    AP4_Mp4sSampleEntry(AP4_Size         size,
                        AP4_ByteStream&  stream,
                        AP4_AtomFactory& atom_factory);
    AP4_Mp4sSampleEntry(AP4_EsDescriptor* descriptor);

    // methods
    AP4_SampleDescription* ToSampleDescription();
};

/*----------------------------------------------------------------------
|       AP4_AudioSampleEntry
+---------------------------------------------------------------------*/
class AP4_AudioSampleEntry : public AP4_MpegSampleEntry
{
 public:
    // methods
    AP4_AudioSampleEntry(AP4_Atom::Type    format,
                         AP4_EsDescriptor* descriptor,
                         AP4_UI32          sample_rate,
                         AP4_UI16          sample_size,
                         AP4_UI16          channel_count);
    AP4_AudioSampleEntry(AP4_Atom::Type    format,
                         AP4_Size          size,
                         AP4_ByteStream&   stream,
                         AP4_AtomFactory&  atom_factory);
    
    // accessors
    AP4_UI32 GetSampleRate()   { return m_SampleRate>>16; }
    AP4_UI16 GetSampleSize()   { return m_SampleSize;     }
    AP4_UI16 GetChannelCount() { return m_ChannelCount;   }

    // methods
    AP4_SampleDescription* ToSampleDescription();

protected:
    // methods
    virtual AP4_Size   GetFieldsSize();
    virtual AP4_Result ReadFields(AP4_ByteStream& stream);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);

    // members
    AP4_UI08 m_Reserved2[8]; // = 0
    AP4_UI32 m_SampleRate;           
    AP4_UI16 m_ChannelCount; // = 2
    AP4_UI16 m_SampleSize;   // = 16
    AP4_UI16 m_Predefined1;  // = 0
    AP4_UI16 m_Reserved3;    // = 0
};

/*----------------------------------------------------------------------
|       AP4_Mp4aSampleEntry
+---------------------------------------------------------------------*/
class AP4_Mp4aSampleEntry : public AP4_AudioSampleEntry
{
 public:
    // constructors
    AP4_Mp4aSampleEntry(AP4_Size         size,
                        AP4_ByteStream&  stream,
                        AP4_AtomFactory& atom_factory);
    AP4_Mp4aSampleEntry(AP4_UI32          sample_rate, 
                        AP4_UI16          sample_size,
                        AP4_UI16          channel_count,
                        AP4_EsDescriptor* descriptor);
};

/*----------------------------------------------------------------------
|       AP4_VisualSampleEntry
+---------------------------------------------------------------------*/
class AP4_VisualSampleEntry : public AP4_MpegSampleEntry
{
 public:
    // methods
    AP4_VisualSampleEntry(AP4_Atom::Type    format, 
                          AP4_EsDescriptor* descriptor,
                          AP4_UI16          width,
                          AP4_UI16          height,
                          AP4_UI16          depth,
                          const char*       compressor_name);
    AP4_VisualSampleEntry(AP4_Atom::Type   format,
                          AP4_Size         size,
                          AP4_ByteStream&  stream,
                          AP4_AtomFactory& atom_factory);

    // accessors
    AP4_UI16    GetWidth()          { return m_Width;  }
    AP4_UI16    GetHeight()         { return m_Height; }
    AP4_UI16    GetDepth()          { return m_Depth;  }
    const char* GetCompressorName() { return m_CompressorName.c_str(); }

    // methods
    AP4_SampleDescription* ToSampleDescription();

protected:
    // methods
    virtual AP4_Size   GetFieldsSize();
    virtual AP4_Result ReadFields(AP4_ByteStream& stream);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);

    //members
    AP4_UI16   m_Predefined1;     // = 0
	AP4_UI16   m_Reserved2;       // = 0
	AP4_UI08   m_Predefined2[12]; // = 0
	AP4_UI16   m_Width;
	AP4_UI16   m_Height;
	AP4_UI32   m_HorizResolution; // = 0x00480000 (72 dpi)
	AP4_UI32   m_VertResolution;  // = 0x00480000 (72 dpi)
	AP4_UI32   m_Reserved3;       // = 0
	AP4_UI16   m_FrameCount;      // = 1
	AP4_String m_CompressorName;       
	AP4_UI16   m_Depth;           // = 0x0018
	AP4_UI16   m_Predefined3;     // = 0xFFFF
};

/*----------------------------------------------------------------------
|       AP4_Mp4vSampleEntry
+---------------------------------------------------------------------*/
class AP4_Mp4vSampleEntry : public AP4_VisualSampleEntry
{
 public:
    // constructors
    AP4_Mp4vSampleEntry(AP4_Size         size,
                        AP4_ByteStream&  stream,
                        AP4_AtomFactory& atom_factory);
    AP4_Mp4vSampleEntry(AP4_UI16          width,
                        AP4_UI16          height,
                        AP4_UI16          depth,
                        const char*       compressor_name,
                        AP4_EsDescriptor* descriptor);
};

/*----------------------------------------------------------------------
|       AP4_Avc1SampleEntry
+---------------------------------------------------------------------*/
class AP4_Avc1SampleEntry : public AP4_VisualSampleEntry
{
public:
    // constructors
    AP4_Avc1SampleEntry(AP4_Size         size,
                        AP4_ByteStream&  stream,
                        AP4_AtomFactory& atom_factory);
    AP4_Avc1SampleEntry(AP4_UI16          width,
                        AP4_UI16          height,
                        AP4_UI16          depth,
                        const char*       compressor_name,
                        AP4_EsDescriptor* descriptor);
};

/*----------------------------------------------------------------------
|       AP4_RtpHintSampleEntry
+---------------------------------------------------------------------*/
class AP4_RtpHintSampleEntry : public AP4_SampleEntry
{
public:
    // methods
    AP4_RtpHintSampleEntry(AP4_UI16 hint_track_version,
                           AP4_UI16 highest_compatible_version,
                           AP4_UI32 max_packet_size,
                           AP4_UI32 timescale);
    AP4_RtpHintSampleEntry(AP4_Size         size,
                           AP4_ByteStream&  stream,
                           AP4_AtomFactory& atom_factory);
    virtual ~AP4_RtpHintSampleEntry();
    
protected:
    // methods
    virtual AP4_Size   GetFieldsSize();
    virtual AP4_Result ReadFields(AP4_ByteStream& stream);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);

    // members
    AP4_UI16 m_HintTrackVersion;
    AP4_UI16 m_HighestCompatibleVersion;
    AP4_UI32 m_MaxPacketSize;
};

#endif // _AP4_SAMPLE_ENTRY_H_
