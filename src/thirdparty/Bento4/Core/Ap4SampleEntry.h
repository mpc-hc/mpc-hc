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
    AP4_AudioSampleEntry(AP4_Atom::Type    format,
                         AP4_Size          size);
    
    // accessors
    AP4_UI32 GetSampleRate();
//  AP4_UI16 GetSampleSize() { return m_SampleSize; }
    AP4_UI16 GetChannelCount();
// mpc-hc custom code start
	AP4_UI32 GetSamplesPerPacket()
	{
		if (m_QtVersion == 2) {
			return m_QtV2LPCMFramesPerAudioPacket;
		}
		return m_QtV1SamplesPerPacket;
	}
	AP4_UI32 GetBytesPerFrame()
	{
		if (m_QtVersion == 2) {
			return m_QtV2BytesPerAudioPacket;
		}
		return m_QtV1BytesPerFrame;
	}
	AP4_UI16 GetSampleSize()
	{
		if (m_QtVersion == 1 && m_SampleSize == 16) {
			//QuickTime File Format Specification->Sound Sample Description (Version 1)->Bytes per packet
			return m_QtV1BytesPerPacket * 8;
		} else if (m_QtVersion == 2) {
			return m_QtV2BitsPerChannel;
		}
		return m_SampleSize;
	}
	AP4_UI16 GetEndian() 
	{
		/*if (m_QtVersion == 2) {
			if (m_QtV2FormatSpecificFlags & 2) {
				return ENDIAN_BIG;
			} else {
				return ENDIAN_LITTLE;
			}
		}*/
		return m_Endian; 
	}
	AP4_UI32 GetFormatSpecificFlags() { return m_QtV2FormatSpecificFlags; }

#define ENDIAN_NOTSET -1
#define ENDIAN_BIG     0
#define ENDIAN_LITTLE  1

// mpc-hc custom code end

    // methods
    AP4_SampleDescription* ToSampleDescription();

protected:
    // methods
    virtual AP4_Size   GetFieldsSize();
    virtual AP4_Result ReadFields(AP4_ByteStream& stream);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);

    // members
    AP4_UI16 m_QtVersion;       // 0, 1 or 2
    AP4_UI16 m_QtRevision;      // 0
    AP4_UI32 m_QtVendor;        // 0
    AP4_UI16 m_ChannelCount; 
    AP4_UI16 m_SampleSize; 
    AP4_UI16 m_QtCompressionId; // 0 or -2
    AP4_UI16 m_QtPacketSize;    // always 0
    AP4_UI32 m_SampleRate;      // 16.16 fixed point
    // Version == 1
    AP4_UI32 m_QtV1SamplesPerPacket;
    AP4_UI32 m_QtV1BytesPerPacket;
    AP4_UI32 m_QtV1BytesPerFrame;
    AP4_UI32 m_QtV1BytesPerSample;
    // Version == 2
	AP4_UI32 m_QtV2StructSize;
    double   m_QtV2SampleRate64;
    AP4_UI32 m_QtV2ChannelCount;
    AP4_UI32 m_QtV2Reserved;
    AP4_UI32 m_QtV2BitsPerChannel;
    AP4_UI32 m_QtV2FormatSpecificFlags;
    AP4_UI32 m_QtV2BytesPerAudioPacket;
    AP4_UI32 m_QtV2LPCMFramesPerAudioPacket;
    AP4_DataBuffer m_QtV2Extension;
	// from 'enda' atom
	AP4_UI16 m_Endian;

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
    AP4_UI16    GetHorizResolution(){ return m_HorizResolution;  }
    AP4_UI16    GetVertResolution() { return m_VertResolution; }
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

/*----------------------------------------------------------------------
|       AP4_TextSampleEntry
+---------------------------------------------------------------------*/
class AP4_TextSampleEntry : public AP4_SampleEntry
{
public:
    // methods
    AP4_TextSampleEntry(AP4_Size         size,
                           AP4_ByteStream&  stream,
                           AP4_AtomFactory& atom_factory);
    virtual ~AP4_TextSampleEntry();
    
	struct AP4_TextDescription
	{
		AP4_UI32 DisplayFlags;
		AP4_UI32 TextJustification;
		AP4_UI32 BackgroundColor;
		struct {AP4_UI16 Top, Left, Bottom, Right;} TextBox;
		struct {AP4_UI16 StartChar, EndChar, Ascent; struct {AP4_UI16 Id; AP4_UI08 Face, Size; AP4_UI32 Color;} Font;} Style;
		AP4_String DefaultFontName;
	};

	const AP4_TextDescription& GetDescription() const { return m_Description; };

protected:
    // methods
    virtual AP4_Result ReadFields(AP4_ByteStream& stream);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);

    // members
	AP4_TextDescription m_Description;
};

/*----------------------------------------------------------------------
|       AP4_Tx3gSampleEntry
+---------------------------------------------------------------------*/
class AP4_Tx3gSampleEntry : public AP4_SampleEntry
{
public:
    // methods
    AP4_Tx3gSampleEntry(AP4_Size         size,
                           AP4_ByteStream&  stream,
                           AP4_AtomFactory& atom_factory);
    virtual ~AP4_Tx3gSampleEntry();
    
	struct AP4_Tx3gDescription
	{
	    AP4_UI32 DisplayFlags;
		AP4_UI08 HorizontalJustification;
		AP4_UI08 VerticalJustification;
		AP4_UI32 BackgroundColor;
		struct {AP4_UI16 Top, Left, Bottom, Right;} TextBox;
		struct {AP4_UI16 StartChar, EndChar; struct {AP4_UI16 Id; AP4_UI08 Face, Size; AP4_UI32 Color;} Font;} Style;
	};

	const AP4_Tx3gDescription& GetDescription() const { return m_Description; };

	AP4_Result GetFontNameById(AP4_Ordinal Id, AP4_String& Name);

protected:
    // methods
    virtual AP4_Size   GetFieldsSize();
    virtual AP4_Result ReadFields(AP4_ByteStream& stream);
    virtual AP4_Result WriteFields(AP4_ByteStream& stream);
    virtual AP4_Result InspectFields(AP4_AtomInspector& inspector);

    // members
	AP4_Tx3gDescription m_Description;
};

/*----------------------------------------------------------------------
|       AP4_AC3SampleEntry
+---------------------------------------------------------------------*/
class AP4_AC3SampleEntry : public AP4_AudioSampleEntry
{
 public:
    // constructors
    AP4_AC3SampleEntry(AP4_Size         size,
                        AP4_ByteStream&  stream,
                        AP4_AtomFactory& atom_factory);

	AP4_Result ReadFields(AP4_ByteStream& stream);
	AP4_Size   GetFieldsSize();
};

/*----------------------------------------------------------------------
|       AP4_EAC3SampleEntry
+---------------------------------------------------------------------*/
class AP4_EAC3SampleEntry : public AP4_AudioSampleEntry
{
 public:
    // constructors
    AP4_EAC3SampleEntry(AP4_Size         size,
                        AP4_ByteStream&  stream,
                        AP4_AtomFactory& atom_factory);
};

#endif // _AP4_SAMPLE_ENTRY_H_
