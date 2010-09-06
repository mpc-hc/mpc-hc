// File_Mxf - Info for MXF files
// Copyright (C) 2006-2010 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// Compilation conditions
#include "MediaInfo/Setup.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_MXF_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Mxf.h"
#if defined(MEDIAINFO_DVDIF_YES)
    #include "MediaInfo/Multiple/File_DvDif.h"
#endif
#if defined(MEDIAINFO_AVC_YES)
    #include "MediaInfo/Video/File_Avc.h"
#endif
#if defined(MEDIAINFO_MPEG4V_YES)
    #include "MediaInfo/Video/File_Mpeg4v.h"
#endif
#if defined(MEDIAINFO_MPEGV_YES)
    #include "MediaInfo/Video/File_Mpegv.h"
#endif
#if defined(MEDIAINFO_MPEGA_YES)
    #include "MediaInfo/Audio/File_Mpega.h"
#endif
#if defined(MEDIAINFO_JPEG_YES)
    #include "MediaInfo/Image/File_Jpeg.h"
#endif
#include "MediaInfo/File_Unknown.h"
#include "ZenLib/FileName.h"
#include "MediaInfo/MediaInfo_Internal.h"
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
//
//  PartitionPack
//  Primer
//  Preface
//      --> ContentStorage
//              --> Packages --> Package (Material, Source)
//                      --> Tracks --> Track
//                              --> Sequence
//                                      --> StructuralComponents --> StructuralComponent (Timecode, SourceClip)
//                      --> Descriptors --> Descriptor (Multiple, Essence)
//                              --> Descriptors --> Descriptor (Essence)
//              --> EssenceContainerData
//              --> Identifications --> Identification
//
//***************************************************************************

//***************************************************************************
// Constants
//***************************************************************************

#define UUID(NAME, PART1, PART2, PART3, PART4) \
    const int32u NAME##1=0x##PART1; \
    const int32u NAME##2=0x##PART2; \
    const int32u NAME##3=0x##PART3; \
    const int32u NAME##4=0x##PART4; \

namespace Elements
{
    UUID(AES3PCMDescriptor,                                     060E2B34, 02530101, 0D010101, 01014700)
    UUID(CDCIEssenceDescriptor,                                 060E2B34, 02530101, 0D010101, 01012800)
    UUID(OpenIncompleteHeaderPartition,                         060E2B34, 02050101, 0D010201, 01020100)
    UUID(ClosedIncompleteHeaderPartition,                       060E2B34, 02050101, 0D010201, 01020200)
    UUID(OpenCompleteHeaderPartition,                           060E2B34, 02050101, 0D010201, 01020300)
    UUID(ClosedCompleteHeaderPartition,                         060E2B34, 02050101, 0D010201, 01020400)
    UUID(OpenIncompleteBodyPartition,                           060E2B34, 02050101, 0D010201, 01030100)
    UUID(ClosedIncompleteBodyPartition,                         060E2B34, 02050101, 0D010201, 01030200)
    UUID(OpenCompleteBodyPartition,                             060E2B34, 02050101, 0D010201, 01030300)
    UUID(ClosedCompleteBodyPartition,                           060E2B34, 02050101, 0D010201, 01030400)
    UUID(OpenIncompleteFooterPartition,                         060E2B34, 02050101, 0D010201, 01040100)
    UUID(ClosedIncompleteFooterPartition,                       060E2B34, 02050101, 0D010201, 01040200)
    UUID(OpenCompleteFooterPartition,                           060E2B34, 02050101, 0D010201, 01040300)
    UUID(ClosedCompleteFooterPartition,                         060E2B34, 02050101, 0D010201, 01040400)
    UUID(ContentStorage,                                        060E2B34, 02530101, 0D010101, 01011800)
    UUID(DMSegment,                                             060E2B34, 02530101, 0D010101, 01014100)
    UUID(EssenceContainerData,                                  060E2B34, 02530101, 0D010101, 01012300)
    UUID(EventTrack,                                            060E2B34, 02530101, 0D010101, 01013900)
    UUID(GenericSoundEssenceDescriptor,                         060E2B34, 02530101, 0D010101, 01014200)
    UUID(Identification,                                        060E2B34, 02530101, 0D010101, 01013000)
    UUID(IndexTableSegment,                                     060E2B34, 02530101, 0D010201, 01100100)
    UUID(JPEG2000PictureSubDescriptor,                          060E2B34, 02530101, 0D010101, 01015A00)
    UUID(MaterialPackage,                                       060E2B34, 02530101, 0D010101, 01013600)
    UUID(MPEG2VideoDescriptor,                                  060E2B34, 02530101, 0D010101, 01015100)
    UUID(MultipleDescriptor,                                    060E2B34, 02530101, 0D010101, 01014400)
    UUID(NetworkLocator,                                        060E2B34, 02530101, 0D010101, 01013200)
    UUID(Padding,                                               060E2B34, 01010101, 03010210, 01000000)
    UUID(Preface,                                               060E2B34, 02530101, 0D010101, 01012F00)
    UUID(RandomIndexMetadata,                                   060E2B34, 02050101, 0D010201, 01110100)
    UUID(RGBAEssenceDescriptor,                                 060E2B34, 02530101, 0D010101, 01012900)
    UUID(Primer,                                                060E2B34, 02050101, 0D010201, 01050100)
    UUID(Sequence,                                              060E2B34, 02530101, 0D010101, 01010F00)
    UUID(SourceClip,                                            060E2B34, 02530101, 0D010101, 01011100)
    UUID(SourcePackage,                                         060E2B34, 02530101, 0D010101, 01013700)
    UUID(StaticTrack,                                           060E2B34, 02530101, 0D010101, 01013A00)
    UUID(SystemScheme1,                                         060E2B34, 02530101, 0D010301, 14020100) //SMPTE 405M
    UUID(TextLocator,                                           060E2B34, 02530101, 0D010101, 01013300)
    UUID(TimecodeComponent,                                     060E2B34, 02530101, 0D010101, 01011400)
    UUID(Track,                                                 060E2B34, 02530101, 0D010101, 01013B00)
    UUID(WaveAudioDescriptor,                                   060E2B34, 02530101, 0D010101, 01014800)
    UUID(AncPacketsDescriptor,                                  060E2B34, 02530101, 0D010101, 01015C00)

    //OperationalPatterns
    UUID(OP_MultiTrack,                                         060E2B34, 04010101, 0D010201, 01010900)
    UUID(OP_SingleTrack,                                        060E2B34, 04010101, 0D010201, 01010100)
    UUID(OP_SingleItem,                                         060E2B34, 04010101, 0D010201, 01020F00)

    //EssenceContainer
    UUID(EssenceContainer_DV,                                   060E2B34, 04010101, 0D010301, 02025001)
    UUID(EssenceContainer_DV2,                                  060E2B34, 04010101, 0D010301, 02020101)
    UUID(EssenceContainer_DV3,                                  060E2B34, 04010101, 0D010301, 02025002)
    UUID(EssenceContainer_JPEG2000,                             060E2B34, 04010107, 0D010301, 020C0100)
    UUID(EssenceContainer_MPEG2,                                060E2B34, 04010102, 0D010301, 02046001)
    UUID(EssenceContainer_RV24,                                 060E2B34, 04010101, 0D010301, 02050001)
    UUID(EssenceContainer_PCM,                                  060E2B34, 04010101, 0D010301, 02060100)
    UUID(EssenceContainer_PCM2,                                 060E2B34, 04010101, 0D010301, 02060200)
    UUID(EssenceContainer_PCM3,                                 060E2B34, 04010101, 0D010301, 02060300)
    UUID(EssenceContainer_PCM4,                                 060E2B34, 04010101, 0D010301, 02060400)
    UUID(EssenceContainer_Generic,                              060E2B34, 04010103, 0D010301, 027F0100)

    //MPEG2VideoDescriptor
    UUID(MPEG2VideoDescriptor_SingleSequence,                   060E2B34, 01010105, 04010602, 01020000)
    UUID(MPEG2VideoDescriptor_ConstantBFrames,                  060E2B34, 01010105, 04010602, 01030000)
    UUID(MPEG2VideoDescriptor_CodedContentType,                 060E2B34, 01010105, 04010602, 01040000)
    UUID(MPEG2VideoDescriptor_LowDelay,                         060E2B34, 01010105, 04010602, 01050000)
    UUID(MPEG2VideoDescriptor_ClosedGOP,                        060E2B34, 01010105, 04010602, 01060000)
    UUID(MPEG2VideoDescriptor_IdenticalGOP,                     060E2B34, 01010105, 04010602, 01070000)
    UUID(MPEG2VideoDescriptor_MaxGOP,                           060E2B34, 01010105, 04010602, 01080000)
    UUID(MPEG2VideoDescriptor_BPictureCount,                    060E2B34, 01010105, 04010602, 01090000)
    UUID(MPEG2VideoDescriptor_ProfileAndLevel,                  060E2B34, 01010105, 04010602, 010A0000)
    UUID(MPEG2VideoDescriptor_BitRate,                          060E2B34, 01010105, 04010602, 010B0000)

    //EssenceCoding
    UUID(EssenceCoding_RV24,                                    060E2B34, 04010101, 04010201, 7F000000)
    UUID(EssenceCoding_D10Video,                                060E2B34, 04010101, 04010202, 01020101)
    UUID(EssenceCoding_MPEG2Video,                              060E2B34, 04010103, 04010202, 01040300)
    UUID(EssenceCoding_MPEG4Visual,                             060E2B34, 04010103, 04010202, 01200204)
    UUID(EssenceCoding_DV,                                      060E2B34, 04010101, 04010202, 02020200)
    UUID(EssenceCoding_JPEG2000,                                060E2B34, 04010107, 04010202, 03010100)
    UUID(EssenceCoding_AVC,                                     060E2B34, 0401010A, 04010202, 01323102)
    UUID(EssenceCoding_PCM,                                     060E2B34, 04010101, 04020201, 7F000000)
    UUID(EssenceCoding_Alaw,                                    060E2B34, 04010104, 04020202, 03010100)

    //Sequence
    UUID(Sequence_Time,                                         060E2B34, 04010101, 01030201, 01000000)
    UUID(Sequence_Picture,                                      060E2B34, 04010101, 01030202, 01000000)
    UUID(Sequence_Sound,                                        060E2B34, 04010101, 01030202, 02000000)
    UUID(Sequence_Data,                                         060E2B34, 04010101, 01030202, 03000000)

    //Other
    UUID(Filler,                                                060E2B34, 01010102, 03010210, 01000000)
    UUID(TerminatingFiller,                                     060E2B34, 01010102, 03010210, 05000000)
    UUID(XmlDocumentText,                                       060E2B34, 01010105, 03010220, 01000000)

    //SDTI-CP (SMPTE 385M)
    UUID(SDTI_SystemMetadataPack,                               060E2B34, 02050101, 0D010301, 04010100)
    UUID(SDTI_PackageMetadataSet,                               060E2B34, 02430101, 0D010301, 04010200)
    UUID(SDTI_PictureMetadataSet,                               060E2B34, 02430101, 0D010301, 04010300)
    UUID(SDTI_SoundMetadataSet,                                 060E2B34, 02430101, 0D010301, 04010400)
    UUID(SDTI_DataMetadataSet,                                  060E2B34, 02430101, 0D010301, 04010500)
    UUID(SDTI_ControlMetadataSet,                               060E2B34, 02630101, 0D010301, 04010600)

    //Private
    UUID(Omneon_010201010100,                                   060E2B34, 02530105, 0E0B0102, 01010100)
    UUID(Omneon_010201020100,                                   060E2B34, 02530105, 0E0B0102, 01020100)
}

//---------------------------------------------------------------------------
const char* Mxf_Category(int8u Category)
{
    switch(Category)
    {
        case 0x01 : return "Item";
        case 0x02 : return "Group (Set/Pack)";
        case 0x03 : return "Wrapper";
        case 0x04 : return "Value";
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* Mxf_Registry(int8u Category, int8u Registry)
{
    switch(Category)
    {
        case 0x01 : //"Item"
                    switch(Registry)
                    {
                        case 0x01 : return "Metadata dictionary";
                        case 0x02 : return "Essence dictionary";
                        default   : return "";
                    }
        case 0x02 : //"Group (Set/Pack)"
                    switch(Registry)
                    {
                        case 0x05 : return "Contains a fixed number of predefined items with no tag of value indicators";
                        case 0x43 : return "1-byte tag, 2-byte length";
                        case 0x53 : return "The value of this KLV block contains other KLV-items with a two-byte tag and a two-byte length indicator";
                        case 0x63 : return "1-byte tag, 4-byte length";
                        default   : return "";
                    }
        case 0x04 : //"Value"
                    switch(Registry)
                    {
                        case 0x01 : return "Fixed";
                        default   : return "";
                    }
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* Mxf_MPEG2_CodedContentType(int8u CodedContentType)
{
    switch(CodedContentType)
    {
        case 0x01 : return "Progressive";
        case 0x02 : return "Interlaced";
        case 0x03 : return ""; //Mixed
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* Mxf_OperationalPattern(int128u OperationalPattern)
{
    //int32u Code_Compare1=OperationalPattern.hi>>32;
    //int32u Code_Compare2=(int32u)OperationalPattern.hi;
    //int32u Code_Compare3=OperationalPattern.lo>>32;
    int32u Code_Compare4=(int32u)OperationalPattern.lo;
    /*
    #undef ELEMENT
    #define ELEMENT(_ELEMENT, _NAME) \
    else if (Code_Compare1==Elements::_ELEMENT##1 \
     && Code_Compare2==Elements::_ELEMENT##2 \
     && Code_Compare3==Elements::_ELEMENT##3 \
     && Code_Compare4==Elements::_ELEMENT##4) \
        return _NAME; \

    if (0) {}
    ELEMENT   (OP_MultiTrack,                                   "Multi-track")
    ELEMENT   (OP_SingleTrack,                                  "Single-track")
    ELEMENT   (OP_SingleItem,                                   "Single-item")
    else
        return "";
    */

    //Item and Package Complexity
    switch ((int8u)(Code_Compare4>>24))
    {
        case 0x01 : switch ((int8u)(Code_Compare4>>16))
                    {
                        case 0x01 : return "OP-1a";
                        case 0x02 : return "OP-1b";
                        case 0x03 : return "OP-1c";
                        default   : return "";
                    }
        case 0x02 : switch ((int8u)(Code_Compare4>>16))
                    {
                        case 0x01 : return "OP-2a";
                        case 0x02 : return "OP-2b";
                        case 0x03 : return "OP-2c";
                        default   : return "";
                    }
        case 0x03 : switch ((int8u)(Code_Compare4>>16))
                    {
                        case 0x01 : return "OP-3a";
                        case 0x02 : return "OP-3b";
                        case 0x03 : return "OP-3c";
                        default   : return "";
                    }
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* Mxf_EssenceElement(int128u EssenceElement)
{
    if ((EssenceElement.hi&0xFFFFFFFFFFFFFF00LL)!=0x060E2B3401020100LL)
        return "";

    int8u Code5=(int8u)((EssenceElement.lo&0x00000000FF000000LL)>>24);
    int8u Code7=(int8u)((EssenceElement.lo&0x000000000000FF00LL)>> 8);

    switch (Code5)
    {
        case 0x05 : //CP Picture (SMPTE 326M)
                    switch (Code7)
                    {
                        case 0x01 : return "D-10 Video";
                        default   : return "Unknown stream";
                    }
        case 0x06 : //CP Sound (SMPTE 326M)
                    switch (Code7)
                    {
                        case 0x10 : return "D-10 Audio";
                        default   : return "Unknown stream";
                    }
        case 0x07 : //CP Data (SMPTE 326M)
                    switch (Code7)
                    {
                        default   : return "";
                    }
        case 0x15 : //GC Picture
                    switch (Code7)
                    {
                        case 0x01 : return "RGB";
                        case 0x05 : return "MPEG Video";
                        case 0x06 : return "AVC";
                        case 0x07 : return "MPEG-4 Visual";
                        case 0x08 : return "JPEG 2000";
                        default   : return "Unknown stream";
                    }
        case 0x16 : //GC Sound
                    switch (Code7)
                    {
                        case 0x01 : return "PCM"; //BWF
                        case 0x02 : return "PCM"; //BWF
                        case 0x03 : return "PCM"; //DV Audio
                        case 0x04 : return "PCM"; //BWF
                        case 0x05 : return "MPEG Audio";
                        case 0x0A : return "A-law";
                        default   : return "Unknown stream";
                    }
        case 0x17 : //GC Data
                    switch (Code7)
                    {
                        case 0x02 : return "ANC"; //Frame-Wrapped ANC Data Element
                        default   : return "Unknown stream";
                    }
        case 0x18 : //GC Compound
                    switch (Code7)
                    {
                        case 0x01 : return "DV"; //Frame
                        case 0x02 : return "DV"; //Clip
                        default   : return "Unknown stream";
                    }
        default   : return "Unknown stream";
    }
}

//---------------------------------------------------------------------------
const char* Mxf_EssenceContainer(int128u EssenceContainer)
{
    if ((EssenceContainer.hi&0xFFFFFFFFFFFFFF00LL)!=0x060E2B3404010100LL)
        return "";

    int8u Code1=(int8u)((EssenceContainer.lo&0xFF00000000000000LL)>>56);
    int8u Code2=(int8u)((EssenceContainer.lo&0x00FF000000000000LL)>>48);
    int8u Code3=(int8u)((EssenceContainer.lo&0x0000FF0000000000LL)>>40);
    int8u Code4=(int8u)((EssenceContainer.lo&0x000000FF00000000LL)>>32);
    int8u Code5=(int8u)((EssenceContainer.lo&0x00000000FF000000LL)>>24);
    int8u Code6=(int8u)((EssenceContainer.lo&0x0000000000FF0000LL)>>16);
    //int8u Code7=(int8u)((EssenceContainer.lo&0x000000000000FF00LL)>> 8);

    switch (Code1)
    {
        case 0x0D : //Public Use
                    switch (Code2)
                    {
                        case 0x01 : //AAF
                                    switch (Code3)
                                    {
                                        case 0x03 : //Essence Container Application
                                                    switch (Code4)
                                                    {
                                                        case 0x01 : //MXF EC Structure version
                                                                    switch (Code5)
                                                                    {
                                                                        case 0x02 : //Essence container kind
                                                                                    switch (Code6)
                                                                                    {
                                                                                        case 0x02 : return "DV";
                                                                                        case 0x06 : return "PCM";
                                                                                        case 0x04 : return "MPEG ES mappings with Stream ID";
                                                                                        case 0x0A : return "A-law";
                                                                                        default   : return "";
                                                                                    }
                                                                        default   : return "";
                                                                    }
                                                         default   : return "";
                                                    }
                                         default   : return "";
                                    }
                        default   : return "";
                    }
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* Mxf_EssenceContainer_Mapping(int8u Code6, int8u Code7, int8u Code8)
{
    switch (Code6)
    {
        case 0x02 : //DV, SMPTE 383M
                    switch (Code8)
                    {
                        case 0x01 : return "Frame";
                        case 0x02 : return "Clip";
                        default   : return "";
                    }
        case 0x04 : //MPEG ES, SMPTE 381M
        case 0x07 : //MPEG PES, SMPTE 381M
        case 0x08 : //MPEG PS, SMPTE 381M
        case 0x09 : //MPEG TS, SMPTE 381M
                    switch (Code8)
                    {
                        case 0x01 : return "Frame";
                        case 0x02 : return "Clip";
                        case 0x03 : return "Custom: Stripe";
                        case 0x04 : return "Custom: PES";
                        case 0x05 : return "Custom: Fixed Audio Size";
                        case 0x06 : return "Custom: Splice";
                        case 0x07 : return "Custom: Closed GOP";
                        case 0x08 : return "Custom: Slave";
                        case 0x7F : return "Custom";
                        default   : return "";
                    }
        case 0x05 : //Uncompressed pictures, SMPTE 384M
                    switch (Code8)
                    {
                        case 0x01 : return "Frame";
                        case 0x02 : return "Clip";
                        case 0x03 : return "Line";
                        default   : return "";
                    }
        case 0x06 : //AES-PCM, SMPTE 382M
                    switch (Code7)
                    {
                        case 0x01 : return "Frame (BWF)";
                        case 0x02 : return "Clip (BWF)";
                        case 0x03 : return "Frame (AES)";
                        case 0x04 : return "Clip (BWF)";
                        case 0x08 : return "Custom (BWF)";
                        case 0x09 : return "Custom (AES)";
                        default   : return "";
                    }
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* Mxf_EssenceCompression(int128u EssenceCompression)
{
    if ((EssenceCompression.hi&0xFFFFFFFFFFFFFF00LL)!=0x060E2B3404010100LL || (EssenceCompression.lo&0xFF00000000000000LL)!=0x0400000000000000LL)
        return "";

    int8u Code2=(int8u)((EssenceCompression.lo&0x00FF000000000000LL)>>48);
    int8u Code3=(int8u)((EssenceCompression.lo&0x0000FF0000000000LL)>>40);
    int8u Code4=(int8u)((EssenceCompression.lo&0x000000FF00000000LL)>>32);
    int8u Code5=(int8u)((EssenceCompression.lo&0x00000000FF000000LL)>>24);
    int8u Code6=(int8u)((EssenceCompression.lo&0x0000000000FF0000LL)>>16);
    int8u Code7=(int8u)((EssenceCompression.lo&0x000000000000FF00LL)>> 8);

    switch (Code2)
    {
        case 0x01 : //Picture
                    switch (Code3)
                    {
                        case 0x02 : //Coding characteristics
                                    switch (Code4)
                                    {
                                        case 0x02 : //Compressed coding
                                                    switch (Code5)
                                                    {
                                                        case 0x01 : //MPEG Compression
                                                                    switch (Code6)
                                                                    {
                                                                        case 0x01 : return "MPEG-2 Video";
                                                                        case 0x02 : return "MPEG-2 Video";
                                                                        case 0x03 : return "MPEG-2 Video";
                                                                        case 0x04 : return "MPEG-2 Video";
                                                                        case 0x11 : return "MPEG-1 Video";
                                                                        case 0x20 : return "MPEG-4 Visual";
                                                                        default   : return "";
                                                                    }
                                                        case 0x02 : return "DV";
                                                        case 0x03 : //Individual Picture Coding Schemes
                                                                    switch (Code6)
                                                                    {
                                                                        case 0x01 : return "JPEG-2000";
                                                                        default   : return "";
                                                                    }
                                                        default   : return "";
                                                    }
                                         default   : return "";
                                    }
                         default   : return "";
                    }
        case 0x02 : //Sound
                    switch (Code3)
                    {
                        case 0x02 : //Coding characteristics
                                    switch (Code4)
                                    {
                                        case 0x01 : //Uncompressed Sound Coding
                                                    switch (Code5)
                                                    {
                                                        case 0x03 : //Compressed Audio Coding
                                                                    switch (Code6)
                                                                    {
                                                                        case 0x01 : return "MPEG-2"; //D-10
                                                                        case 0x02 : return "DV";
                                                                        case 0x03 : return "D-11?";
                                                                        case 0x04 : return "MPEG Video?"; //ES?
                                                                        case 0x05 : return "Raw";
                                                                        case 0x06 : return "PCM";
                                                                        case 0x07 : return "MPEG Video?"; //PES?
                                                                        case 0x08 : return "MPEG-PS";
                                                                        case 0x09 : return "MPEG-TS";
                                                                        case 0x0A : return "A-law";
                                                                        case 0x0B : return "Encrypted";
                                                                        case 0x0C : return "JPEG-2000";
                                                                        case 0x7F : return "Generic";
                                                                        default   : return "";
                                                                    }
                                                         default   : return "";
                                                    }
                                        case 0x02 : //Compressed coding
                                                    switch (Code5)
                                                    {
                                                        case 0x03 : //Compressed Audio Coding
                                                                    switch (Code6)
                                                                    {
                                                                        case 0x01 : //Compandeded Audio Coding
                                                                                    switch (Code7)
                                                                                    {
                                                                                        case 0x01 : return "A-law";
                                                                                        case 0x10 : return "DV Audio"; //DV 12-bit
                                                                                        default  : return ""; //Unknown
                                                                                    }
                                                                        default   : return "";
                                                                    }
                                                         default   : return "";
                                                    }
                                         default   : return "";
                                    }
                         default   : return "";
                    }
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* Mxf_EssenceCompression_Version(int128u EssenceCompression)
{
    int8u Code2=(int8u)((EssenceCompression.lo&0x00FF000000000000LL)>>48);
    int8u Code3=(int8u)((EssenceCompression.lo&0x0000FF0000000000LL)>>40);
    int8u Code4=(int8u)((EssenceCompression.lo&0x000000FF00000000LL)>>32);
    int8u Code5=(int8u)((EssenceCompression.lo&0x00000000FF000000LL)>>24);
    int8u Code6=(int8u)((EssenceCompression.lo&0x0000000000FF0000LL)>>16);
    //int8u Code7=(int8u)((EssenceCompression.lo&0x000000000000FF00LL)>> 8);

    switch (Code2)
    {
        case 0x01 : //Picture
                    switch (Code3)
                    {
                        case 0x02 : //Coding characteristics
                                    switch (Code4)
                                    {
                                        case 0x02 : //Compressed coding
                                                    switch (Code5)
                                                    {
                                                        case 0x01 : //MPEG Compression
                                                                    switch (Code6)
                                                                    {
                                                                        case 0x01 : return "Version 2";
                                                                        case 0x02 : return "Version 2";
                                                                        case 0x03 : return "Version 2";
                                                                        case 0x04 : return "Version 2";
                                                                        case 0x11 : return "Version 1";
                                                                        default   : return "";
                                                                    }
                                                        default   : return "";
                                                    }
                                         default   : return "";
                                    }
                         default   : return "";
                    }
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* Mxf_Sequence_DataDefinition(int128u DataDefinition)
{
    int32u Code_Compare1=DataDefinition.hi>>32;
    int32u Code_Compare2=(int32u)DataDefinition.hi;
    int32u Code_Compare3=DataDefinition.lo>>32;
    int32u Code_Compare4=(int32u)DataDefinition.lo;
    #undef ELEMENT
    #define ELEMENT(_ELEMENT, _NAME) \
    else if (Code_Compare1==Elements::_ELEMENT##1 \
     && Code_Compare2==Elements::_ELEMENT##2 \
     && Code_Compare3==Elements::_ELEMENT##3 \
     && Code_Compare4==Elements::_ELEMENT##4) \
        return _NAME; \

    if (0) {}
    ELEMENT(Sequence_Time,                                      "Time")
    ELEMENT(Sequence_Picture,                                   "Picture")
    ELEMENT(Sequence_Sound,                                     "Sound")
    ELEMENT(Sequence_Data,                                      "Data")
    else
        return "";
}

//---------------------------------------------------------------------------
extern const char* Mpegv_profile_and_level_indication_profile[];
extern const char* Mpegv_profile_and_level_indication_level[];

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Mxf::File_Mxf()
:File__Analyze()
{
    //Configuration
    ParserName=_T("MXF");
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Mxf;
        StreamIDs_Width[0]=8;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        Demux_Level=2; //Container
    #endif //MEDIAINFO_DEMUX
    MustSynchronize=true;
    DataMustAlwaysBeComplete=false;

    //Temp
    Streams_Count=(size_t)-1;
    OperationalPattern=0;
    Buffer_DataSizeToParse=0;
    Buffer_DataSizeToParse_Complete=(int64u)-1;
    Preface_Current.hi=(int64u)-1;
    Preface_Current.lo=(int64u)-1;
    Track_Number_IsAvailable=false;
    IsParsingEnd=false;
    TimeCode_StartTimecode=(int64u)-1;
    TimeCode_RoundedTimecodeBase=0;
    TimeCode_DropFrame=0;
    SDTI_TimeCode_StartTimecode=(int64u)-1;
    SystemScheme1_TimeCodeArray_StartTimecode=(int64u)-1;
    SystemScheme1_FrameRateFromDescriptor=0;
    #if defined(MEDIAINFO_ANCILLARY_YES)
        Ancillary_InstanceUID=(int128u)-1;
        Ancillary_LinkedTrackID=(int32u)-1;
        Ancillary_TrackNumber=(int32u)-1;
        Ancillary=NULL;
    #endif //defined(MEDIAINFO_ANCILLARY_YES)
}

//---------------------------------------------------------------------------
File_Mxf::~File_Mxf()
{
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mxf::Streams_Finish()
{
    if (!Track_Number_IsAvailable)
    {
        for (tracks::iterator Track=Tracks.begin(); Track!=Tracks.end(); Track++)
        {
            //Searching the corresponding Descriptor
            stream_t StreamKind=Stream_Max;
            for (descriptors::iterator Descriptor=Descriptors.begin(); Descriptor!=Descriptors.end(); Descriptor++)
                if (Descriptor->second.LinkedTrackID==Track->second.TrackID)
                {
                    StreamKind=Descriptor->second.StreamKind;
                    break;
                }
            if (StreamKind!=Stream_Max)
            {
                for (essences::iterator Essence=Essences.begin(); Essence!=Essences.end(); Essence++)
                    if (Essence->second.StreamKind==StreamKind && !Essence->second.Track_Number_IsMappedToTrack)
                    {
                        Track->second.TrackNumber=Essence->first;
                        Essence->second.Track_Number_IsMappedToTrack=true;
                        break;
                    }
            }
        }
    }

    File_Size_Total=File_Size;

    Streams_Finish_Preface(Preface_Current);

    //OperationalPattern
    Fill(Stream_General, 0, General_Format_Profile, Mxf_OperationalPattern(OperationalPattern));

    //File size handling
    if (File_Size_Total!=File_Size)
        Fill(Stream_General, 0, General_FileSize, File_Size_Total, 10, true);

    //Commercial names
    if (Count_Get(Stream_Video)==1)
    {
        Streams_Finish_StreamOnly();
             if (!Retrieve(Stream_Video, 0, Video_Format_Commercial_IfAny).empty())
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, Retrieve(Stream_Video, 0, Video_Format_Commercial_IfAny));
            Fill(Stream_General, 0, General_Format_Commercial, _T("MXF ")+Retrieve(Stream_Video, 0, Video_Format_Commercial_IfAny));
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==_T("DV"))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "DV");
            Fill(Stream_General, 0, General_Format_Commercial, "MXF DV");
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==_T("AVC") && Retrieve(Stream_Video, 0, Video_Format_Settings_GOP)==_T("N=1") && Retrieve(Stream_Video, 0, Video_Colorimetry)==_T("4:2:0") && Retrieve(Stream_Video, 0, Video_BitRate)==_T("56064000"))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "AVC-Intra 50");
            Fill(Stream_General, 0, General_Format_Commercial, "MXF AVC-Intra 50");
            Fill(Stream_Video, 0, Video_Format_Commercial_IfAny, "AVC-Intra 50");
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==_T("AVC") && Retrieve(Stream_Video, 0, Video_Format_Settings_GOP)==_T("N=1") && Retrieve(Stream_Video, 0, Video_Colorimetry)==_T("4:2:2") && Retrieve(Stream_Video, 0, Video_BitRate)==_T("113664000"))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "AVC-Intra 100");
            Fill(Stream_General, 0, General_Format_Commercial, "MXF AVC-Intra 100");
            Fill(Stream_Video, 0, Video_Format_Commercial_IfAny, "AVC-Intra 100");
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==_T("MPEG Video") && Retrieve(Stream_Video, 0, Video_Format_Settings_GOP)==_T("N=1") && Retrieve(Stream_Video, 0, Video_Colorimetry)==_T("4:2:2") && Retrieve(Stream_Video, 0, Video_BitRate)==_T("30000000"))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "XDCAM IMX 30");
            Fill(Stream_Video, 0, Video_Format_Commercial_IfAny, "XDCAM IMX 30");
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==_T("MPEG Video") && Retrieve(Stream_Video, 0, Video_Format_Settings_GOP)==_T("N=1") && Retrieve(Stream_Video, 0, Video_Colorimetry)==_T("4:2:2") && Retrieve(Stream_Video, 0, Video_BitRate)==_T("40000000"))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "XDCAM IMX 40");
            Fill(Stream_Video, 0, Video_Format_Commercial_IfAny, "XDCAM IMX 40");
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==_T("MPEG Video") && Retrieve(Stream_Video, 0, Video_Format_Settings_GOP)==_T("N=1") && Retrieve(Stream_Video, 0, Video_Colorimetry)==_T("4:2:2") && Retrieve(Stream_Video, 0, Video_BitRate)==_T("50000000"))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "XDCAM IMX 50");
            Fill(Stream_Video, 0, Video_Format_Commercial_IfAny, "XDCAM IMX 50");
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==_T("MPEG Video") && Retrieve(Stream_Video, 0, Video_Format_Settings_GOP)!=_T("N=1") && Retrieve(Stream_Video, 0, Video_Colorimetry)==_T("4:2:0") && Retrieve(Stream_Video, 0, Video_BitRate)==_T("18000000"))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "XDCAM HD 18");
            Fill(Stream_Video, 0, Video_Format_Commercial_IfAny, "XDCAM HD 18");
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==_T("MPEG Video") && Retrieve(Stream_Video, 0, Video_Format_Settings_GOP)!=_T("N=1") && Retrieve(Stream_Video, 0, Video_Colorimetry)==_T("4:2:0") && Retrieve(Stream_Video, 0, Video_BitRate)==_T("25000000"))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "XDCAM HD 25");
            Fill(Stream_Video, 0, Video_Format_Commercial_IfAny, "XDCAM HD 25");
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==_T("MPEG Video") && Retrieve(Stream_Video, 0, Video_Format_Settings_GOP)!=_T("N=1") && Retrieve(Stream_Video, 0, Video_Colorimetry)==_T("4:2:0") && Retrieve(Stream_Video, 0, Video_BitRate)==_T("35000000"))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "XDCAM HD 35");
            Fill(Stream_Video, 0, Video_Format_Commercial_IfAny, "XDCAM HD 35");
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==_T("MPEG Video") && Retrieve(Stream_Video, 0, Video_Format_Settings_GOP)!=_T("N=1") && Retrieve(Stream_Video, 0, Video_Colorimetry)==_T("4:2:2") && Retrieve(Stream_Video, 0, Video_BitRate)==_T("50000000"))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "XDCAM HD422");
            Fill(Stream_Video, 0, Video_Format_Commercial_IfAny, "XDCAM HD422");
        }
    }
}

//---------------------------------------------------------------------------
void File_Mxf::Streams_Finish_Preface (int128u PrefaceUID)
{
    prefaces::iterator Preface=Prefaces.find(PrefaceUID);
    if (Preface==Prefaces.end())
        return;

    //ContentStorage
    Streams_Finish_ContentStorage(Preface->second.ContentStorage);

    //Identifications
    for (size_t Pos=0; Pos<Preface->second.Identifications.size(); Pos++)
        Streams_Finish_Identification(Preface->second.Identifications[Pos]);
}

//---------------------------------------------------------------------------
void File_Mxf::Streams_Finish_ContentStorage (int128u ContentStorageUID)
{
    contentstorages::iterator ContentStorage=ContentStorages.find(ContentStorageUID);
    if (ContentStorage==ContentStorages.end())
        return;

    for (size_t Pos=0; Pos<ContentStorage->second.Packages.size(); Pos++)
        Streams_Finish_Package(ContentStorage->second.Packages[Pos]);
}

//---------------------------------------------------------------------------
void File_Mxf::Streams_Finish_Package (int128u PackageUID)
{
    packages::iterator Package=Packages.find(PackageUID);
    if (Package==Packages.end() || !Package->second.IsSourcePackage)
        return;

    for (size_t Pos=0; Pos<Package->second.Tracks.size(); Pos++)
        Streams_Finish_Track(Package->second.Tracks[Pos]);

    Streams_Finish_Descriptor(Package->second.Descriptor, PackageUID);
}

//---------------------------------------------------------------------------
void File_Mxf::Streams_Finish_Track(int128u TrackUID)
{
    tracks::iterator Track=Tracks.find(TrackUID);
    if (Track==Tracks.end() || Track->second.Stream_Finish_Done)
        return;

    Streams_Finish_Essence(Track->second.TrackNumber, TrackUID);

    //Sequence
    Streams_Finish_Component(Track->second.Sequence, Track->second.EditRate);

    //Done
    Track->second.Stream_Finish_Done=true;
}

//---------------------------------------------------------------------------
void File_Mxf::Streams_Finish_Essence(int32u EssenceUID, int128u TrackUID)
{
    essences::iterator Essence=Essences.find(EssenceUID);
    if (Essence==Essences.end() || Essence->second.Stream_Finish_Done)
        return;

    if (Essence->second.Parser==NULL)
        return;

    Finish(Essence->second.Parser);
    StreamKind_Last=Stream_Max;
    if (Essence->second.Parser->Count_Get(Stream_Video))
        Stream_Prepare(Stream_Video);
    else if (Essence->second.Parser->Count_Get(Stream_Audio))
        Stream_Prepare(Stream_Audio);
    if (StreamKind_Last!=Stream_Max)
    {
        for (std::map<std::string, Ztring>::iterator Info=Essence->second.Infos.begin(); Info!=Essence->second.Infos.end(); Info++)
            Fill(StreamKind_Last, StreamPos_Last, Info->first.c_str(), Info->second, true);
        if (TimeCode_RoundedTimecodeBase && TimeCode_StartTimecode!=(int64u)-1)
        {
            Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Delay), TimeCode_StartTimecode*1000/TimeCode_RoundedTimecodeBase, 0, true);
            Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Delay_Source), "Container");
        }
        if (SDTI_TimeCode_StartTimecode!=(int64u)-1)
            Fill(StreamKind_Last, StreamPos_Last, "Delay_SDTI", SDTI_TimeCode_StartTimecode);
        if (SystemScheme1_TimeCodeArray_StartTimecode!=(int64u)-1)
            Fill(StreamKind_Last, StreamPos_Last, "Delay_SystemScheme1", SystemScheme1_TimeCodeArray_StartTimecode);
        Merge(*Essence->second.Parser, StreamKind_Last, 0, StreamPos_Last);
    }

    //Looking for Material package TrackID
    int32u ID=(int32u)-1;
    for (packages::iterator SourcePackage=Packages.begin(); SourcePackage!=Packages.end(); SourcePackage++)
        if (SourcePackage->second.PackageUID.hi.hi) //Looking fo a SourcePackage with PackageUID only
        {
            //Testing if the Track is in this SourcePackage
            for (size_t Tracks_Pos=0; Tracks_Pos<SourcePackage->second.Tracks.size(); Tracks_Pos++)
                if (SourcePackage->second.Tracks[Tracks_Pos]==TrackUID)
                {
                    //We have the the right PackageUID, looking for SourceClip from Sequence from Track from MaterialPackage
                    for (components::iterator SourceClip=Components.begin(); SourceClip!=Components.end(); SourceClip++)
                        if (SourceClip->second.SourcePackageID.lo==SourcePackage->second.PackageUID.lo && SourceClip->second.SourceTrackID==Essence->second.TrackID) //int256u doesn't support yet ==
                        {
                            //We have the right SourceClip, looking for the Sequence from Track from MaterialPackage
                            for (components::iterator Sequence=Components.begin(); Sequence!=Components.end(); Sequence++)
                                for (size_t StructuralComponents_Pos=0; StructuralComponents_Pos<Sequence->second.StructuralComponents.size(); StructuralComponents_Pos++)
                                    if (Sequence->second.StructuralComponents[StructuralComponents_Pos]==SourceClip->first)
                                    {
                                        //We have the right Sequence, looking for Track from MaterialPackage
                                        for (tracks::iterator Track=Tracks.begin(); Track!=Tracks.end(); Track++)
                                            if (Track->second.Sequence==Sequence->first)
                                                ID=Track->second.TrackID;
                                    }
                        }
                }
        }
    if (ID!=(int32u)-1)
        Fill(StreamKind_Last, StreamPos_Last, General_ID, ID);
    else if (Tracks[TrackUID].TrackID!=(int32u)-1)
        Fill(StreamKind_Last, StreamPos_Last, General_ID, Tracks[TrackUID].TrackID);
    else
    {
        Fill(StreamKind_Last, StreamPos_Last, General_ID, Essence->first);
        Fill(StreamKind_Last, StreamPos_Last, General_ID_String, Ztring::ToZtring(Essence->first, 16), true);
    }

    //Interlacement management
    if (StreamKind_Last==Stream_Video && Retrieve(Stream_Video, StreamPos_Last, Video_ScanType)==_T("Interlaced") && Retrieve(Stream_Video, StreamPos_Last, Video_Format)==_T("M-JPEG 2000"))
    {
        //M-JPEG 2000 has no complete frame height, but field height
        int64u Height=Retrieve(Stream_Video, StreamPos_Last, Video_Height).To_int64u();
        Height*=2; //This is per field
        if (Height)
            Fill(Stream_Video, StreamPos_Last, Video_Height, Height, 10, true);

        //M-JPEG 2000 has no complete frame framerate, but field framerate
        float64 FrameRate=Retrieve(Stream_Video, StreamPos_Last, Video_FrameRate).To_float64();
        FrameRate/=2; //This is per field
        if (FrameRate)
            Fill(Stream_Video, StreamPos_Last, Video_FrameRate, FrameRate, 3, true);
    }

    //Special case - DV
    #if defined(MEDIAINFO_DVDIF_YES)
        if (StreamKind_Last==Stream_Video && Retrieve(Stream_Video, StreamPos_Last, Video_Format)==_T("DV"))
        {
            if (Retrieve(Stream_General, 0, General_Recorded_Date).empty())
                Fill(Stream_General, 0, General_Recorded_Date, Essence->second.Parser->Retrieve(Stream_General, 0, General_Recorded_Date));

            //Video and Audio are together
            size_t Audio_Count=Essence->second.Parser->Count_Get(Stream_Audio);
            for (size_t Audio_Pos=0; Audio_Pos<Audio_Count; Audio_Pos++)
            {
                Fill_Flush();
                Stream_Prepare(Stream_Audio);
                size_t Pos=Count_Get(Stream_Audio)-1;
                Essence->second.Parser->Finish();
                if (TimeCode_RoundedTimecodeBase && TimeCode_StartTimecode!=(int64u)-1)
                {
                    Fill(Stream_Audio, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Delay), TimeCode_StartTimecode*1000/TimeCode_RoundedTimecodeBase, 0, true);
                    Fill(Stream_Audio, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Delay_Source), "Container");
                }
                Merge(*Essence->second.Parser, Stream_Audio, Audio_Pos, StreamPos_Last);
                if (Retrieve(Stream_Audio, Pos, Audio_MuxingMode).empty())
                    Fill(Stream_Audio, Pos, Audio_MuxingMode, Retrieve(Stream_Video, Essence->second.StreamPos, Video_Format), true);
                else
                    Fill(Stream_Audio, Pos, Audio_MuxingMode, Retrieve(Stream_Video, Essence->second.StreamPos, Video_Format)+_T(" / ")+Retrieve(Stream_Audio, Pos, Audio_MuxingMode), true);
                Fill(Stream_Audio, Pos, Audio_Duration, Retrieve(Stream_Video, Essence->second.StreamPos, Video_Duration));
                Fill(Stream_Audio, Pos, Audio_StreamSize, "0"); //Included in the DV stream size
                Ztring ID=Retrieve(Stream_Audio, Pos, Audio_ID);
                Fill(Stream_Audio, Pos, Audio_ID, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_ID)+_T("-")+ID, true);
                Fill(Stream_Audio, Pos, Audio_ID_String, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_ID_String)+_T("-")+ID, true);
            }

            StreamKind_Last=Stream_Video;
            StreamPos_Last=Essence->second.StreamPos;
        }
    #endif

    //Special case - MPEG Video + Captions
    if (StreamKind_Last==Stream_Video && Essence->second.Parser && Essence->second.Parser->Count_Get(Stream_Text))
    {
        //Video and Text are together
        File__Analyze* Parser=Essence->second.Parser;
        size_t Parser_Text_Count=Parser->Count_Get(Stream_Text);
        for (size_t Parser_Text_Pos=0; Parser_Text_Pos<Parser_Text_Count; Parser_Text_Pos++)
        {
            size_t StreamPos_Video=StreamPos_Last;
            Fill_Flush();
            Stream_Prepare(Stream_Text);
            Parser->Finish();
            if (TimeCode_RoundedTimecodeBase && TimeCode_StartTimecode!=(int64u)-1)
            {
                Fill(Stream_Text, Parser_Text_Pos, Fill_Parameter(StreamKind_Last, Generic_Delay), TimeCode_StartTimecode*1000/TimeCode_RoundedTimecodeBase, 0, true);
                Fill(Stream_Text, Parser_Text_Pos, Fill_Parameter(StreamKind_Last, Generic_Delay_Source), "Container");
            }
            Merge(*Parser, Stream_Text, Parser_Text_Pos, StreamPos_Last);
            Fill(Stream_Text, StreamPos_Last, Text_Duration, Retrieve(Stream_Video, StreamPos_Video, Video_Duration));
            Ztring ID=Retrieve(Stream_Text, StreamPos_Last, Text_ID);
            if (Retrieve(Stream_Text, StreamPos_Last, Text_MuxingMode).find(_T("Ancillary"))!=string::npos)
            {
                Fill(Stream_Text, StreamPos_Last, Text_ID, Ztring::ToZtring(Ancillary_LinkedTrackID)+_T("-")+ID, true);
                Fill(Stream_Text, StreamPos_Last, Text_ID_String, Ztring::ToZtring(Ancillary_LinkedTrackID)+_T("-")+ID, true);
            }
            else
            {
                Fill(Stream_Text, StreamPos_Last, Text_ID, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_ID)+_T("-")+ID, true);
                Fill(Stream_Text, StreamPos_Last, Text_ID_String, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_ID_String)+_T("-")+ID, true);
            }
        }

        StreamKind_Last=Stream_Video;
        StreamPos_Last=Essence->second.StreamPos;
    }

    //Stream size
    if (StreamKind_Last!=Stream_Max && Count_Get(Stream_Video)+Count_Get(Stream_Audio)==1 && Essence->second.Stream_Size!=(int64u)-1)
        Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_StreamSize), Essence->second.Stream_Size);

    //Done
    Essence->second.Stream_Finish_Done=true;
}

//---------------------------------------------------------------------------
void File_Mxf::Streams_Finish_Descriptor(int128u DescriptorUID, int128u PackageUID)
{
    descriptors::iterator Descriptor=Descriptors.find(DescriptorUID);
    if (Descriptor==Descriptors.end())
        return;

    //Subs
    if (!Descriptor->second.SubDescriptors.empty())
    {
        for (size_t Pos=0; Pos<Descriptor->second.SubDescriptors.size(); Pos++)
            Streams_Finish_Descriptor(Descriptor->second.SubDescriptors[Pos], PackageUID);
        return; //Is not a real descriptor
    }

    StreamKind_Last=Stream_Max;
    StreamPos_Last=(size_t)-1;
    if (Descriptor->second.StreamKind!=Stream_Max)
    {
        StreamKind_Last=Descriptor->second.StreamKind;
        for (size_t Pos=0; Pos<Count_Get(StreamKind_Last); Pos++)
            if (Ztring::ToZtring(Descriptor->second.LinkedTrackID)==Retrieve(StreamKind_Last, Pos, General_ID))
            {
                StreamPos_Last=Pos;
                break;
            }
        if (StreamPos_Last==(size_t)-1)
        {
            if (Descriptors.size()==1)
                StreamPos_Last=0;
            else
            {
                Stream_Prepare(Descriptor->second.StreamKind);
                if (Descriptor->second.LinkedTrackID!=(int32u)-1)
                    Fill(StreamKind_Last, StreamPos_Last, General_ID, Descriptor->second.LinkedTrackID);
                else
                {
                    //Looking for Material package TrackID
                    int32u ID=(int32u)-1;
                    packages::iterator SourcePackage=Packages.find(PackageUID);
                    //We have the the right PackageUID, looking for SourceClip from Sequence from Track from MaterialPackage
                    for (components::iterator SourceClip=Components.begin(); SourceClip!=Components.end(); SourceClip++)
                        if (SourceClip->second.SourcePackageID.lo==SourcePackage->second.PackageUID.lo) //int256u doesn't support yet ==
                        {
                            //We have the right SourceClip, looking for the Sequence from Track from MaterialPackage
                            for (components::iterator Sequence=Components.begin(); Sequence!=Components.end(); Sequence++)
                                for (size_t StructuralComponents_Pos=0; StructuralComponents_Pos<Sequence->second.StructuralComponents.size(); StructuralComponents_Pos++)
                                    if (Sequence->second.StructuralComponents[StructuralComponents_Pos]==SourceClip->first)
                                    {
                                        //We have the right Sequence, looking for Track from MaterialPackage
                                        for (tracks::iterator Track=Tracks.begin(); Track!=Tracks.end(); Track++)
                                        {
                                            int64u A=Sequence->first.hi;
                                            int64u B=Track->first.hi;
                                            if (Track->second.Sequence==Sequence->first)
                                                ID=Track->second.TrackID;
                                        }
                                    }
                        }
                    if (ID!=(int32u)-1)
                        Fill(StreamKind_Last, StreamPos_Last, General_ID, ID, 10, true);
                }
            }
        }
    }

    //Locators
    size_t Before_Count[Stream_Max];
    for (size_t Pos=0; Pos<Stream_Max; Pos++)
        Before_Count[Pos]=(size_t)-1;
    Before_Count[Stream_Video]=Count_Get(Stream_Video);
    Before_Count[Stream_Audio]=Count_Get(Stream_Audio);
    Before_Count[Stream_Text]=Count_Get(Stream_Text);
    for (size_t Locator_Pos=0; Locator_Pos<Descriptor->second.Locators.size(); Locator_Pos++)
    {
        //Locator
        Streams_Finish_Locator(Descriptor->second.Locators[Locator_Pos]);
    }

    if (StreamPos_Last==(size_t)-1 && Essences.size()==1)
    {
        //Only one essence, there is sometimes no LinkedTrackID
        if (Count_Get(Stream_Video)==1)
        {
            StreamKind_Last=Stream_Video;
            StreamPos_Last=0;
        }
        else if (Count_Get(Stream_Audio)==1)
        {
            StreamKind_Last=Stream_Audio;
            StreamPos_Last=0;
        }
    }

    if (StreamKind_Last!=Stream_Max && StreamPos_Last!=(size_t)-1)
    {
        //ID
        if (Descriptor->second.LinkedTrackID!=(int32u)-1 && Retrieve(StreamKind_Last, StreamPos_Last, General_ID).empty())
        {
            for (size_t StreamKind=0; StreamKind<Stream_Max; StreamKind++)
                for (size_t StreamPos=Before_Count[StreamKind]; StreamPos<Count_Get((stream_t)StreamKind); StreamPos++)
                {
                    Ztring ID=Retrieve((stream_t)StreamKind, StreamPos, General_ID);
                    if (ID.empty())
                        Fill((stream_t)StreamKind, StreamPos, General_ID, Descriptor->second.LinkedTrackID);
                    else
                        Fill((stream_t)StreamKind, StreamPos, General_ID, Ztring::ToZtring(Descriptor->second.LinkedTrackID)+ID, true);
                }
        }

        if (Descriptor->second.Width!=(int32u)-1 && Retrieve(Stream_Video, StreamPos_Last, Video_Width).empty())
            Fill(Stream_Video, StreamPos_Last, Video_Width, Descriptor->second.Width, 10, true);
        if (Descriptor->second.Height!=(int32u)-1 && Retrieve(Stream_Video, StreamPos_Last, Video_Height).empty())
            Fill(Stream_Video, StreamPos_Last, Video_Height, Descriptor->second.Height, 10, true);
        for (std::map<std::string, Ztring>::iterator Info=Descriptor->second.Infos.begin(); Info!=Descriptor->second.Infos.end(); Info++)
            if (Retrieve(StreamKind_Last, StreamPos_Last, Info->first.c_str()).empty())
                Fill(StreamKind_Last, StreamPos_Last, Info->first.c_str(), Info->second, true);

        //Bitrate (PCM)
        if (StreamKind_Last==Stream_Audio && Retrieve(Stream_Audio, StreamPos_Last, Audio_BitRate).empty() && Retrieve(Stream_Audio, StreamPos_Last, Audio_Format)==_T("PCM"))
        {
            int64u Channels=Retrieve(Stream_Audio, StreamPos_Last, Audio_Channel_s_).To_int64u();
            int64u SamplingRate=Retrieve(Stream_Audio, StreamPos_Last, Audio_SamplingRate).To_int64u();
            int64u Resolution=Retrieve(Stream_Audio, StreamPos_Last, Audio_Resolution).To_int64u();
            if (Channels && SamplingRate && Resolution)
               Fill(Stream_Audio, StreamPos_Last, Audio_BitRate, Channels*SamplingRate*Resolution);
        }

        //Bitrate (Video)
        if (StreamKind_Last==Stream_Video && Retrieve(Stream_Video, StreamPos_Last, Video_BitRate).empty())
        {
            //Until now, I only found CBR files
            Fill(Stream_Video, StreamPos_Last, Video_BitRate, Retrieve(Stream_Video, StreamPos_Last, Video_BitRate_Nominal));
        }

        //Display Aspect Ratio
        if (StreamKind_Last==Stream_Video && !Descriptor->second.Infos["DisplayAspectRatio"].empty() && Retrieve(Stream_Video, StreamPos_Last, Video_DisplayAspectRatio)!=Descriptor->second.Infos["DisplayAspectRatio"])
        {
            Ztring DAR=Retrieve(Stream_Video, StreamPos_Last, Video_DisplayAspectRatio);
            Clear(Stream_Video, StreamPos_Last, Video_PixelAspectRatio);
            Fill(Stream_Video, StreamPos_Last, Video_DisplayAspectRatio, Descriptor->second.Infos["DisplayAspectRatio"], true);
            Fill(Stream_Video, StreamPos_Last, Video_DisplayAspectRatio_Original, DAR);
        }
    }
}

//---------------------------------------------------------------------------
void File_Mxf::Streams_Finish_Locator(int128u LocatorUID)
{
    locators::iterator Locator=Locators.find(LocatorUID);
    if (Locator==Locators.end())
        return;

    Fill(StreamKind_Last, StreamPos_Last, "Source", Locator->second.EssenceLocator);

    //External file name specific
    if (!Locator->second.IsTextLocator && !Locator->second.EssenceLocator.empty())
    {
        //Preparing
        stream_t StreamKind_Last_Essence=StreamKind_Last;
        size_t StreamPos_Last_Essence=StreamPos_Last;

        //Configuring file name
        Ztring AbsoluteName=ZenLib::FileName::Path_Get(File_Name);
        if (!AbsoluteName.empty())
            AbsoluteName+=ZenLib::PathSeparator;
        AbsoluteName+=Locator->second.EssenceLocator;

        MediaInfo_Internal MI;
        MI.Option(_T("File_StopAfterFilled"), _T("1"));
        MI.Option(_T("File_KeepInfo"), _T("1"));

        if (MI.Open(AbsoluteName))
        {
            //Hacks - Before
            Ztring CodecID=Retrieve(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_CodecID));
            Ztring ID=Retrieve(StreamKind_Last, StreamPos_Last, General_ID);
            Clear(StreamKind_Last, StreamPos_Last, General_ID);

            Merge(*MI.Info, StreamKind_Last, 0, StreamPos_Last);
            File_Size_Total+=Ztring(MI.Get(Stream_General, 0, General_FileSize)).To_int64u();

            //Hacks - After
            if (CodecID!=Retrieve(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_CodecID)))
            {
                if (!CodecID.empty())
                    CodecID+=_T(" / ");
                CodecID+=Retrieve(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_CodecID));
                Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_CodecID), CodecID, true);
            }
            if (!Retrieve(StreamKind_Last, StreamPos_Last, General_ID).empty())
                ID+=_T('-')+Retrieve(StreamKind_Last, StreamPos_Last, General_ID);
            Fill(StreamKind_Last, StreamPos_Last, General_ID, ID, true);

            //Special case: MXF in MXF
            if (MI.Info && MI.Info->Get(Stream_General, 0, General_Format)==_T("MXF"))
            {
                Fill(StreamKind_Last, StreamPos_Last, "MuxingMode", "MXF");

                if (MI.Info->Count_Get(Stream_Video)>0)
                {
                    size_t Parser_Audio_Count=MI.Info->Count_Get(Stream_Audio);
                    for (size_t Parser_Audio_Pos=0; Parser_Audio_Pos<Parser_Audio_Count; Parser_Audio_Pos++)
                    {
                        Fill_Flush();
                        Stream_Prepare(Stream_Audio);
                        Merge(*MI.Info, Stream_Audio, Parser_Audio_Pos, StreamPos_Last);
                        if (Retrieve(Stream_Audio, StreamPos_Last, Audio_MuxingMode).empty())
                            Fill(Stream_Audio, StreamPos_Last, Audio_MuxingMode, "MXF");
                        else
                            Fill(Stream_Audio, StreamPos_Last, Audio_MuxingMode, _T("MXF / ")+Retrieve(Stream_Audio, StreamPos_Last, Audio_MuxingMode), true);
                        Fill(Stream_Audio, StreamPos_Last, "Source", Retrieve(Stream_Video, StreamPos_Last_Essence, "Source"));
                        Fill(Stream_Audio, StreamPos_Last, "Source_Info", Retrieve(Stream_Video, StreamPos_Last_Essence, "Source_Info"));
                    }
                    size_t Parser_Text_Count=MI.Info->Count_Get(Stream_Text);
                    for (size_t Parser_Text_Pos=0; Parser_Text_Pos<Parser_Text_Count; Parser_Text_Pos++)
                    {
                        Fill_Flush();
                        Stream_Prepare(Stream_Text);
                        Merge(*MI.Info, Stream_Text, Parser_Text_Pos, StreamPos_Last);
                        if (Retrieve(Stream_Text, StreamPos_Last, Text_MuxingMode).empty())
                            Fill(Stream_Text, StreamPos_Last, Text_MuxingMode, "MXF");
                        else
                            Fill(Stream_Text, StreamPos_Last, Text_MuxingMode, _T("MXF / ")+Retrieve(Stream_Text, StreamPos_Last, Text_MuxingMode), true);
                        Fill(Stream_Text, StreamPos_Last, "Source", Retrieve(Stream_Video, StreamPos_Last_Essence, "Source"));
                        Fill(Stream_Text, StreamPos_Last, "Source_Info", Retrieve(Stream_Video, StreamPos_Last_Essence, "Source_Info"));
                    }
                }
            }
            //Special case: DV with Audio or/and Text in the video stream
            else if (MI.Info && MI.Info->Get(Stream_General, 0, General_Format)==_T("DV") && (MI.Info->Count_Get(Stream_Audio) || MI.Info->Count_Get(Stream_Text)))
            {
                //Video and Audio are together
                size_t Parser_Audio_Count=MI.Info->Count_Get(Stream_Audio);
                for (size_t Parser_Audio_Pos=0; Parser_Audio_Pos<Parser_Audio_Count; Parser_Audio_Pos++)
                {
                    Fill_Flush();
                    Stream_Prepare(Stream_Audio);
                    Merge(*MI.Info, Stream_Audio, Parser_Audio_Pos, StreamPos_Last);
                    if (Retrieve(Stream_Audio, StreamPos_Last, Audio_MuxingMode).empty())
                        Fill(Stream_Audio, StreamPos_Last, Audio_MuxingMode, Retrieve(Stream_Video, StreamPos_Last_Essence, Video_Format), true);
                    else
                        Fill(Stream_Audio, StreamPos_Last, Audio_MuxingMode, Retrieve(Stream_Video, StreamPos_Last_Essence, Video_Format)+_T(" / ")+Retrieve(Stream_Audio, StreamPos_Last, Audio_MuxingMode), true);
                    Fill(Stream_Audio, StreamPos_Last, Audio_Duration, Retrieve(Stream_Video, StreamPos_Last_Essence, Video_Duration), true);
                    Fill(Stream_Audio, StreamPos_Last, "Source", Retrieve(Stream_Video, StreamPos_Last_Essence, "Source"));
                    Fill(Stream_Audio, StreamPos_Last, "Source_Info", Retrieve(Stream_Video, StreamPos_Last_Essence, "Source_Info"));
                    Ztring ID=Retrieve(Stream_Audio, StreamPos_Last, Audio_ID);
                    Fill(Stream_Audio, StreamPos_Last, Audio_ID, Retrieve(Stream_Video, StreamPos_Last_Essence, Video_ID)+_T("-")+ID, true);
                    Fill(Stream_Audio, StreamPos_Last, Audio_ID_String, Retrieve(Stream_Video, StreamPos_Last_Essence, Video_ID_String)+_T("-")+ID, true);
                }

                //Video and Text are together
                size_t Parser_Text_Count=MI.Info->Count_Get(Stream_Text);
                for (size_t Parser_Text_Pos=0; Parser_Text_Pos<Parser_Text_Count; Parser_Text_Pos++)
                {
                    Fill_Flush();
                    Stream_Prepare(Stream_Text);
                    Merge(*MI.Info, Stream_Text, Parser_Text_Pos, StreamPos_Last);
                    if (Retrieve(Stream_Text, StreamPos_Last, Text_MuxingMode).empty())
                        Fill(Stream_Text, StreamPos_Last, Text_MuxingMode, Retrieve(Stream_Video, StreamPos_Last_Essence, Video_Format), true);
                    else
                        Fill(Stream_Text, StreamPos_Last, Text_MuxingMode, Retrieve(Stream_Video, StreamPos_Last_Essence, Video_Format)+_T(" / ")+Retrieve(Stream_Text, StreamPos_Last, Text_MuxingMode), true);
                    Fill(Stream_Text, StreamPos_Last, Text_Duration, Retrieve(Stream_Video, StreamPos_Last_Essence, Video_Duration), true);
                    Fill(Stream_Text, StreamPos_Last, "Source", Retrieve(Stream_Video, StreamPos_Last_Essence, "Source"));
                    Fill(Stream_Text, StreamPos_Last, "Source_Info", Retrieve(Stream_Video, StreamPos_Last_Essence, "Source_Info"));
                    Ztring ID=Retrieve(Stream_Text, StreamPos_Last, Text_ID);
                    Fill(Stream_Text, StreamPos_Last, Text_ID, Retrieve(Stream_Video, StreamPos_Last_Essence, Video_ID)+_T("-")+ID, true);
                    Fill(Stream_Text, StreamPos_Last, Text_ID_String, Retrieve(Stream_Video, StreamPos_Last_Essence, Video_ID_String)+_T("-")+ID, true);
                }
            }

            StreamKind_Last=StreamKind_Last_Essence;
            StreamPos_Last=StreamPos_Last_Essence;
        }
        else
            Fill(StreamKind_Last, StreamPos_Last, "Source_Info", "Missing");
    }
}

//---------------------------------------------------------------------------
void File_Mxf::Streams_Finish_Component(int128u ComponentUID, float32 EditRate)
{
    components::iterator Component=Components.find(ComponentUID);
    if (Component==Components.end())
        return;

    //Duration
    if (EditRate && StreamKind_Last!=Stream_Max && Component->second.Duration!=(int64u)-1)
        Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Duration), Component->second.Duration*1000/EditRate, 0, true);
}

//---------------------------------------------------------------------------
void File_Mxf::Streams_Finish_Identification (int128u IdentificationUID)
{
    identifications::iterator Identification=Identifications.find(IdentificationUID);
    if (Identification==Identifications.end())
        return;

    if (!Identification->second.ProductName.empty())
    {
        Ztring Encoded_Library_Name;
        if (!Identification->second.CompanyName.empty())
        {
            Encoded_Library_Name+=Identification->second.CompanyName;
            Encoded_Library_Name+=_T(' ');
        }
        Encoded_Library_Name+=Identification->second.ProductName;
        Ztring Encoded_Library_Version;
        if (!Identification->second.ProductVersion.empty())
        {
            Encoded_Library_Version=Identification->second.ProductVersion;
        }
        else if (!Identification->second.VersionString.empty())
        {
            Encoded_Library_Version=Identification->second.VersionString;
        }
        Ztring Encoded_Application=Encoded_Library_Name;
        if (!Encoded_Library_Version.empty())
        {
            Encoded_Application+=_T(' ');
            Encoded_Application+=Encoded_Library_Version;
        }
        Fill(Stream_General, 0, General_Encoded_Application, Encoded_Application, true);
        Fill(Stream_General, 0, General_Encoded_Library_Name, Encoded_Library_Name, true);
        Fill(Stream_General, 0, General_Encoded_Library_Version, Encoded_Library_Version, true);
    }

    for (std::map<std::string, Ztring>::iterator Info=Identification->second.Infos.begin(); Info!=Identification->second.Infos.end(); Info++)
        Fill(Stream_General, 0, Info->first.c_str(), Info->second, true);
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mxf::Read_Buffer_Continue()
{
    if (Buffer_DataSizeToParse)
    {
        if (Buffer_Size<=Buffer_DataSizeToParse)
        {
            Element_Size=Buffer_Size; //All the buffer is used
            Buffer_DataSizeToParse-=Buffer_Size;
        }
        else
        {
            Element_Size=Buffer_DataSizeToParse;
            Buffer_DataSizeToParse=0;
        }

        Element_Begin();
        Data_Parse();
        Element_Offset=Element_Size;
        Element_End();
    }
    else
    {
        if (IsParsingEnd && File_Offset+Buffer_Offset+4<File_Size)
        {
            if (Buffer_Offset+16+4<Buffer_Size)
                return; //Wait for more data
            if (CC4(Buffer+Buffer_Offset)!=0x060E2B34)
                Finish(); //No footer
        }

        if (MediaInfoLib::Config.ParseSpeed_Get()<1.0 && File_Offset+Buffer_Offset+4==File_Size)
        {
            int32u Length;
            Get_B4 (Length,                                         "Length (Ramdom Index)");
            if (Length>=16+4 && Length<File_Size/2)
                GoToFromEnd(Length); //For random access table
        }
    }
}

//---------------------------------------------------------------------------
void File_Mxf::Read_Buffer_Unsynched()
{
    Buffer_DataSizeToParse=0;
}

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Mxf::FileHeader_Begin()
{
    return true;
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Mxf::Synchronize()
{
    //Synchronizing
    while (Buffer_Offset+4<=Buffer_Size
        && CC4(Buffer+Buffer_Offset)!=0x060E2B34)
        Buffer_Offset++;

    //Parsing last bytes if needed
    if (Buffer_Offset+4>Buffer_Size)
    {
        if (Buffer_Offset+4==Buffer_Size && CC4(Buffer+Buffer_Offset)!=0x060E2B34)
            Buffer_Offset++;
        if (Buffer_Offset+3==Buffer_Size && CC3(Buffer+Buffer_Offset)!=0x060E2B)
            Buffer_Offset++;
        if (Buffer_Offset+2==Buffer_Size && CC2(Buffer+Buffer_Offset)!=0x060E)
            Buffer_Offset++;
        if (Buffer_Offset+1==Buffer_Size && CC1(Buffer+Buffer_Offset)!=0x06)
            Buffer_Offset++;
        return false;
    }

    if (!Status[IsAccepted])
    {
        Accept();

        Fill(Stream_General, 0, General_Format, "MXF");
    }

    //Synched is OK
    return true;
}

//---------------------------------------------------------------------------
bool File_Mxf::Synched_Test()
{
    //Trailing 0x00
    while(Buffer_Offset+1<=Buffer_Size && Buffer[Buffer_Offset]==0x00)
        Buffer_Offset++;

    //Must have enough buffer for having header
    if (Buffer_Offset+16>Buffer_Size)
        return false;

    //Quick test of synchro
    if (CC4(Buffer+Buffer_Offset)!=0x060E2B34)
    {
        Synched=false;
        Trusted++;
    }

    //Quick search
    if (!Synched && !Synchronize())
        return false;

    //We continue
    return true;
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mxf::Header_Parse()
{
    //Parsing
    int8u Length;
    Get_UL(Code,                                                "Code", NULL);
    Get_B1(Length,                                              "Length");
    int64u Length_Final;
    if (Length<0x80)
    {
        Length_Final=Length;
    }
    else
    {
        Length&=0x7F;
        switch (Length)
        {
            case 1 :
                    {
                    int8u  Length1;
                    Get_B1(Length1,                             "Length");
                    Length_Final=Length1;
                    }
                    break;
            case 2 :
                    {
                    int16u Length2;
                    Get_B2(Length2,                             "Length");
                    Length_Final=Length2;
                    }
                    break;
            case 3 :
                    {
                    int32u Length3;
                    Get_B3(Length3,                             "Length");
                    Length_Final=Length3;
                    }
                    break;
            case 4 :
                    {
                    int32u Length4;
                    Get_B4(Length4,                             "Length");
                    Length_Final=Length4;
                    }
                    break;
            case 5 :
                    {
                    int64u Length5;
                    Get_B5(Length5,                             "Length");
                    Length_Final=Length5;
                    }
                    break;
            case 6 :
                    {
                    int64u Length6;
                    Get_B6(Length6,                             "Length");
                    Length_Final=Length6;
                    }
                    break;
            case 7 :
                    {
                    int64u Length7;
                    Get_B7(Length7,                             "Length");
                    Length_Final=Length7;
                    }
                    break;
            case 8 :
                    {
                    int64u Length8;
                    Get_B8(Length8,                             "Length");
                    Length_Final=Length8;
                    }
                    break;
            default: Length_Final=0; //Problem
        }
    }

    //Filling
    if (Buffer_Offset+Length_Final>(size_t)-1 || Buffer_Offset+(size_t)Length_Final>Buffer_Size)
    {
        int32u Code_Compare1=Code.hi>>32;
        int32u Code_Compare2=(int32u)Code.hi;
        int32u Code_Compare3=Code.lo>>32;
        if (Code_Compare1==0x060E2B34
         && Code_Compare2==0x01020101
         && Code_Compare3==0x0D010301)
        {
            Buffer_DataSizeToParse_Complete=Length_Final;
            Length_Final=Buffer_Size-(Buffer_Offset+Element_Offset);
            Buffer_DataSizeToParse=Buffer_DataSizeToParse_Complete-Length_Final;
        }
        else
        {
            Element_WaitForMoreData();
            return;
        }
    }
    Header_Fill_Code(0, Ztring::ToZtring(Code.hi, 16)+Ztring::ToZtring(Code.lo, 16));
    Header_Fill_Size(Element_Offset+Length_Final);
}

//---------------------------------------------------------------------------
void File_Mxf::Data_Parse()
{
    //Clearing
    InstanceUID=0;

    //Parsing
    int32u Code_Compare1=Code.hi>>32;
    int32u Code_Compare2=(int32u)Code.hi;
    int32u Code_Compare3=Code.lo>>32;
    int32u Code_Compare4=(int32u)Code.lo;
    #undef ELEMENT
    #define ELEMENT(_ELEMENT, _NAME) \
    else if (Code_Compare1==Elements::_ELEMENT##1 \
     && Code_Compare2==Elements::_ELEMENT##2 \
     && Code_Compare3==Elements::_ELEMENT##3 \
     && Code_Compare4==Elements::_ELEMENT##4) \
    { \
        if (!Element_IsComplete_Get()) \
        { \
            Element_WaitForMoreData(); \
            return; \
        } \
        Element_Name(_NAME); \
        switch (Code_Compare2>>24) \
        { \
            case 0x01 : _ELEMENT(); break; \
            case 0x02 : switch ((int8u)(Code_Compare2>>16)) \
                        { \
                            case 0x05 : _ELEMENT(); break; \
                            case 0x43 : _ELEMENT(); break; \
                            case 0x53 : \
                                        while(Element_Offset<Element_Size) \
                                        { \
                                            Element_Begin(); \
                                            Element_Begin("Header"); \
                                                Get_B2 (Code2,                                  "Code"); \
                                                Get_B2 (Length2,                                "Length"); \
                                            Element_End(); \
                                            Element_Name(Ztring().From_CC2(Code2)); \
                                            \
                                            int64u End=Element_Offset+Length2; \
                                            _ELEMENT(); \
                                            if (Element_Offset<End) \
                                                Skip_XX(End-Element_Offset,                     "Unknown"); \
                                            \
                                            Element_End(4+Length2); \
                                        } \
                                        break; \
                            case 0x63 : _ELEMENT(); break; \
                            default   : Skip_XX(Element_Size,                                   "Unknown"); \
                        } \
        } \
    } \


    if (0) {}
    ELEMENT(AES3PCMDescriptor,                                  "AES3 Descriptor")
    ELEMENT(CDCIEssenceDescriptor,                              "CDCI Essence Descriptor")
    ELEMENT(OpenIncompleteHeaderPartition,                      "Open and Incomplete Header Partition Pack")
    ELEMENT(ClosedIncompleteHeaderPartition,                    "Closed and Iomplete Header Partition Pack")
    ELEMENT(OpenCompleteHeaderPartition,                        "Open and Complete Header Partition Pack")
    ELEMENT(ClosedCompleteHeaderPartition,                      "Closed and Complete Header Partition Pack")
    ELEMENT(OpenIncompleteBodyPartition,                        "Open and Incomplete Body Partition Pack")
    ELEMENT(ClosedIncompleteBodyPartition,                      "Closed and Iomplete Body Partition Pack")
    ELEMENT(OpenCompleteBodyPartition,                          "Open and Complete Body Partition Pack")
    ELEMENT(ClosedCompleteBodyPartition,                        "Closed and Complete Body Partition Pack")
    ELEMENT(OpenIncompleteFooterPartition,                      "Open and Incomplete Footer Partition Pack")
    ELEMENT(ClosedIncompleteFooterPartition,                    "Closed and Iomplete Footer Partition Pack")
    ELEMENT(OpenCompleteFooterPartition,                        "Open and Complete Footer Partition Pack")
    ELEMENT(ClosedCompleteFooterPartition,                      "Closed and Complete Footer Partition Pack")
    ELEMENT(ContentStorage,                                     "Content Storage")
    ELEMENT(DMSegment,                                          "Descriptive Metadata Segment")
    ELEMENT(EssenceContainerData,                               "Essence Container Data")
    ELEMENT(EventTrack,                                         "Event track")
    ELEMENT(GenericSoundEssenceDescriptor,                      "Generic Sound Essence Descriptor")
    ELEMENT(Identification,                                     "Identification")
    ELEMENT(IndexTableSegment,                                  "Index Table (Segment)")
    ELEMENT(JPEG2000PictureSubDescriptor,                       "JPEG 2000 Picture Sub Descriptor")
    ELEMENT(MaterialPackage,                                    "Material Package")
    ELEMENT(MultipleDescriptor,                                 "Multiple Descriptor")
    ELEMENT(MPEG2VideoDescriptor,                               "MPEG-2 Video Descriptor")
    ELEMENT(NetworkLocator,                                     "Network Locator")
    ELEMENT(OpenCompleteBodyPartition,                          "OpenCompleteBodyPartition")
    ELEMENT(Preface,                                            "Preface")
    ELEMENT(Padding,                                            "Padding")
    ELEMENT(Primer,                                             "Primer")
    ELEMENT(RGBAEssenceDescriptor,                              "RGBA Essence Descriptor")
    ELEMENT(RandomIndexMetadata,                                "Random Index Metadata")
    ELEMENT(Sequence,                                           "Sequence")
    ELEMENT(SourceClip,                                         "Source Clip")
    ELEMENT(SourcePackage,                                      "Source Package")
    ELEMENT(StaticTrack,                                        "Static Track")
    ELEMENT(SystemScheme1,                                      "System Scheme 1") //SMPTE 405M
    ELEMENT(TextLocator,                                        "Text Locator")
    ELEMENT(TimecodeComponent,                                  "Timecode Component")
    ELEMENT(Track,                                              "Track")
    ELEMENT(WaveAudioDescriptor,                                "Wave Audio Descriptor")
    ELEMENT(AncPacketsDescriptor,                               "ANC Packets Descriptor")
    ELEMENT(Filler,                                             "Filler")
    ELEMENT(TerminatingFiller,                                  "Terminating Filler")
    ELEMENT(XmlDocumentText,                                    "XML Document Text")
    ELEMENT(SDTI_SystemMetadataPack,                            "SDTI System Metadata Pack")
    ELEMENT(Omneon_010201010100,                                "Omneon (010201010100)")
    ELEMENT(Omneon_010201020100,                                "Omneon (010201020100)")
    else if (Code_Compare1==0x060E2B34
          && ((Code_Compare2)&0xFFBFFFFF)==0x02030101 //0x43 or 0x63
          && Code_Compare3==0x0D010301
          && ((Code_Compare4)&0xFFFF0000)==0x04010000)
    {
        Code_Compare4&=0xFFFFFF00; //Remove MetaData Block Count
        if (0) {}
        ELEMENT(SDTI_PackageMetadataSet,                        "SDTI Package Metadata Set")
        ELEMENT(SDTI_PictureMetadataSet,                        "SDTI Picture Metadata Set")
        ELEMENT(SDTI_SoundMetadataSet,                          "SDTI Sound Metadata Set")
        ELEMENT(SDTI_DataMetadataSet,                           "SDTI Data Metadata Set")
        ELEMENT(SDTI_ControlMetadataSet,                        "SDTI Control Metadata Set")
    }
    else if (Code_Compare1==0x060E2B34
          && Code_Compare2==0x01020101
          && Code_Compare3==0x0D010301)
    {
        //Element_Name(Mxf_EssenceElement(Code));
        Element_Name(Mxf_EssenceElement(Code));

        #if MEDIAINFO_DEMUX
            if (!Essences[Code_Compare4].TrackID_WasLookedFor)
            {
                for (tracks::iterator Track=Tracks.begin(); Track!=Tracks.end(); Track++)
                    if (Track->second.TrackNumber==Code_Compare4)
                        Essences[Code_Compare4].TrackID=Track->second.TrackID;
                Essences[Code_Compare4].TrackID_WasLookedFor=true;
            }
            if (Essences[Code_Compare4].TrackID!=(int32u)-1)
                Element_Code=Essences[Code_Compare4].TrackID;
            else
                Element_Code=Code.lo;
        #endif //MEDIAINFO_DEMUX

        if (Essences[Code_Compare4].Parser==NULL)
        {
            switch (Code_Compare4&0xFF00FF00)
            {
                case 0x05000100 : //D-10 Video, SMPTE 386M
                case 0x15000500 : //MPEG Video
                                    Essences[Code_Compare4].StreamKind=Stream_Video;
                                    Essences[Code_Compare4].StreamPos=Code_Compare4&0x000000FF;
                                    #if defined(MEDIAINFO_MPEGV_YES)
                                        Essences[Code_Compare4].Parser=new File_Mpegv();
                                        ((File_Mpegv*)Essences[Code_Compare4].Parser)->Ancillary=&Ancillary;
                                    #else
                                        Essences[Code_Compare4].Parser=new File_Unknown();
                                        Open_Buffer_Init(Essences[Code_Compare4].Parser);
                                        Essences[Code_Compare4].Parser->Stream_Prepare(Stream_Video);
                                        Essences[Code_Compare4].Parser->Fill(Stream_Video, 0, Video_Format, "MPEG Video");
                                        if (Streams_Count>0)
                                            Streams_Count--;
                                    #endif
                                    break;
                case 0x15000100 : //RV24
                                    Essences[Code_Compare4].StreamKind=Stream_Video;
                                    Essences[Code_Compare4].StreamPos=Code_Compare4&0x000000FF;
                                    Essences[Code_Compare4].Parser=new File_Unknown();
                                    Open_Buffer_Init(Essences[Code_Compare4].Parser);
                                    Essences[Code_Compare4].Parser->Stream_Prepare(Stream_Video);
                                    Essences[Code_Compare4].Parser->Fill(Stream_Video, 0, Video_Format, "RV24");
                                    if (Streams_Count>0)
                                        Streams_Count--;
                                    break;
                case 0x15000600 : //AVC
                                    Essences[Code_Compare4].StreamKind=Stream_Video;
                                    Essences[Code_Compare4].StreamPos=Code_Compare4&0x000000FF;
                                    #if defined(MEDIAINFO_AVC_YES)
                                        Essences[Code_Compare4].Parser=new File_Avc();
                                    #else
                                        Essences[Code_Compare4].Parser=new File_Unknown();
                                        Open_Buffer_Init(Essences[Code_Compare4].Parser);
                                        Essences[Code_Compare4].Parser->Stream_Prepare(Stream_Video);
                                        Essences[Code_Compare4].Parser->Fill(Stream_Video, 0, Video_Format, "AVC");
                                        if (Streams_Count>0)
                                            Streams_Count--;
                                    #endif
                                    break;
                case 0x15000700 : //MPEG-4 Visual
                                    Essences[Code_Compare4].StreamKind=Stream_Video;
                                    Essences[Code_Compare4].StreamPos=Code_Compare4&0x000000FF;
                                    #if defined(MEDIAINFO_MPEG4V_YES)
                                        Essences[Code_Compare4].Parser=new File_Mpeg4v();
                                    #else
                                        Essences[Code_Compare4].Parser=new File_Unknown();
                                        Open_Buffer_Init(Essences[Code_Compare4].Parser);
                                        Essences[Code_Compare4].Parser->Stream_Prepare(Stream_Video);
                                        Essences[Code_Compare4].Parser->Fill(Stream_Video, 0, Video_Format, "MPEG-4 Visual");
                                        if (Streams_Count>0)
                                            Streams_Count--;
                                    #endif
                                    break;
                case 0x15000800 : //M-JPEG 2000
                                    Essences[Code_Compare4].StreamKind=Stream_Video;
                                    Essences[Code_Compare4].StreamPos=Code_Compare4&0x000000FF;
                                    #if defined(MEDIAINFO_JPEG_YES)
                                        Essences[Code_Compare4].Parser=new File_Jpeg();
                                        ((File_Jpeg*)Essences[Code_Compare4].Parser)->StreamKind=Stream_Video;
                                    #else
                                        Essences[Code_Compare4].Parser=new File_Unknown();
                                        Open_Buffer_Init(Essences[Code_Compare4].Parser);
                                        Essences[Code_Compare4].Parser->Stream_Prepare(Stream_Video);
                                        Essences[Code_Compare4].Parser->Fill(Stream_Video, 0, Video_Format, "M-JPEG 2000");
                                        if (Streams_Count>0)
                                            Streams_Count--;
                                    #endif
                                    break;
                case 0x06001000 : //D-10 Audio, SMPTE 386M
                case 0x16000100 : //BWF (PCM)
                case 0x16000200 : //BWF (PCM)
                case 0x16000300 : //DV Audio (PCM)
                case 0x16000400 : //P2 Audio (PCM)
                                    Essences[Code_Compare4].StreamKind=Stream_Audio;
                                    Essences[Code_Compare4].StreamPos=Code_Compare4&0x000000FF;
                                    Essences[Code_Compare4].Parser=new File_Unknown();
                                    Open_Buffer_Init(Essences[Code_Compare4].Parser);
                                    Essences[Code_Compare4].Parser->Stream_Prepare(Stream_Audio);
                                    Essences[Code_Compare4].Parser->Fill(Stream_Audio, 0, Audio_Format, "PCM");
                                    if (Streams_Count>0)
                                        Streams_Count--;
                                    break;
                case 0x16000500 : //MPEG Audio
                                    Essences[Code_Compare4].StreamKind=Stream_Audio;
                                    Essences[Code_Compare4].StreamPos=Code_Compare4&0x000000FF;
                                    #if defined(MEDIAINFO_MPEGA_YES)
                                        Essences[Code_Compare4].Parser=new File_Mpega();
                                    #else
                                        Essences[Code_Compare4].Parser=new File_Unknown();
                                        Open_Buffer_Init(Essences[Code_Compare4].Parser);
                                        Essences[Code_Compare4].Parser->Stream_Prepare(Stream_Audio);
                                        Essences[Code_Compare4].Parser->Fill(Stream_Audio, 0, Audio_Format, "MPEG Audio");
                                        if (Streams_Count>0)
                                            Streams_Count--;
                                    #endif
                                    break;
                case 0x16000A00 : //A-law
                                    Essences[Code_Compare4].StreamKind=Stream_Audio;
                                    Essences[Code_Compare4].StreamPos=Code_Compare4&0x000000FF;
                                    Essences[Code_Compare4].Parser=new File_Unknown();
                                    Open_Buffer_Init(Essences[Code_Compare4].Parser);
                                    Essences[Code_Compare4].Parser->Stream_Prepare(Stream_Audio);
                                    Essences[Code_Compare4].Parser->Fill(Stream_Audio, 0, Audio_Format, "A-law");
                                    if (Streams_Count>0)
                                        Streams_Count--;
                                    break;
                case 0x17000200 : //Ancillary
                                    Essences[Code_Compare4].Parser=new File_Ancillary();
                                    Open_Buffer_Init(Essences[Code_Compare4].Parser);
                                    Ancillary=(File_Ancillary*)Essences[Code_Compare4].Parser;
                                    Ancillary_TrackNumber=Code_Compare4;
                                    break;
                case 0x17000800 : //Line Wrapped Data Element, SMPTE 384M
                case 0x17000900 : //Line Wrapped VANC Data Element, SMPTE 384M
                case 0x17000A00 : //Line Wrapped HANC Data Element, SMPTE 384M
                                    break;
                case 0x18000100 : //DV
                case 0x18000200 : //DV
                                    Essences[Code_Compare4].StreamKind=Stream_Video;
                                    Essences[Code_Compare4].StreamPos=Code_Compare4&0x000000FF;
                                    #if defined(MEDIAINFO_DVDIF_YES)
                                        Essences[Code_Compare4].Parser=new File_DvDif();
                                        //Bitrate is precise
                                        if (Element_Size==144000)
                                            Essences[Code_Compare4].Infos["BitRate"].From_Number( 28800000);
                                        if (Element_Size==288000)
                                            Essences[Code_Compare4].Infos["BitRate"].From_Number( 57600000);
                                        if (Element_Size==576000)
                                            Essences[Code_Compare4].Infos["BitRate"].From_Number(115200000);
                                    #else
                                        Essences[Code_Compare4].Parser=new File_Unknown();
                                        Open_Buffer_Init(Essences[Code_Compare4].Parser);
                                        Essences[Code_Compare4].Parser->Stream_Prepare(Stream_Video);
                                        Essences[Code_Compare4].Parser->Fill(Stream_Video, 0, Video_Format, "DV");
                                        if (Streams_Count>0)
                                            Streams_Count--;
                                    #endif
                                    break;
                default         :   Essences[Code_Compare4].Parser=new File__Analyze();
            }
            Open_Buffer_Init(Essences[Code_Compare4].Parser);

            //Stream size is sometime easy to find
            if ((Buffer_DataSizeToParse_Complete==(int64u)-1?Element_Size:Buffer_DataSizeToParse_Complete)>=File_Size*0.98) //let imagine: if element size is 98% of file size, this is the only one element in the file
                Essences[Code_Compare4].Stream_Size=(Buffer_DataSizeToParse_Complete==(int64u)-1?Element_Size:Buffer_DataSizeToParse_Complete);
        }

        //Demux
        Demux(Buffer+Buffer_Offset, (size_t)Element_Size, ContentType_MainStream);

        if (!Essences[Code_Compare4].Parser->Status[IsFinished])
        {
            if (Code_Compare4==0x17010201)
            {
                //Ancillary with
                int16u Count;
                Get_B2 (Count,                                  "Number of ANC packets");
                for (int16u Pos=0; Pos<Count; Pos++)
                {
                    Element_Begin("ANC packet");
                    int16u Size;
                    Skip_B2(                                    "Line Number");
                    Skip_B1(                                    "Wrapping Type");
                    Skip_B1(                                    "Payload Sample Coding");
                    Get_B2 (Size,                               "Payload Sample Count");
                    Skip_B4(                                    "Size?");
                    Skip_B4(                                    "Count?");
                    Open_Buffer_Continue(Essences[Code_Compare4].Parser, Buffer+Buffer_Offset+(size_t)(Element_Offset), Size);
                    Element_End();
                }
            }
            else
            {
                //Parsing
                Open_Buffer_Continue(Essences[Code_Compare4].Parser);

                //Disabling this Streams
                if (!Essences[Code_Compare4].IsFilled && (Essences[Code_Compare4].Parser->Status[IsFinished] || Essences[Code_Compare4].Parser->Status[IsFilled]))
                {
                    if (Streams_Count>0)
                        Streams_Count--;
                    Essences[Code_Compare4].IsFilled=true;
                    if (!Essences[Code_Compare4].Parser->Status[IsFinished] && MediaInfoLib::Config.ParseSpeed_Get()<1.0)
                        Essences[Code_Compare4].Parser->Finish();
                }
            }
        }
        else
            Skip_XX(Element_Size,                               "Data");
    }
    else
        Skip_XX(Element_Size,                                   "Unknown");

    if (!IsParsingEnd && MediaInfoLib::Config.ParseSpeed_Get()<1.0
     && (File_Offset>=0x4000000 //TODO: 64 MB by default (security), should be changed
      || (Streams_Count==0 && !Descriptors.empty())))
    {
        GoToFromEnd(4); //For random access table
        IsParsingEnd=true;
    }
}

//***************************************************************************
// Elements
//***************************************************************************

#undef ELEMENT
#define ELEMENT(_CODE, _CALL, _NAME) \
    case 0x##_CODE :   Element_Name(_NAME); _CALL(); break; \

#define ELEMENT_UUID(_ELEMENT, _NAME) \
else if (Code_Compare1==Elements::_ELEMENT##1 \
      && Code_Compare2==Elements::_ELEMENT##2 \
      && Code_Compare3==Elements::_ELEMENT##3 \
      && Code_Compare4==Elements::_ELEMENT##4) \
{ \
    Element_Name(_NAME); \
    _ELEMENT(); \
}

//---------------------------------------------------------------------------
void File_Mxf::AES3PCMDescriptor()
{
    switch(Code2)
    {
        ELEMENT(3D08, AES3PCMDescriptor_AuxBitsMode,            "Use of Auxiliary Bits")
        ELEMENT(3D0D, AES3PCMDescriptor_Emphasis,               "Emphasis")
        ELEMENT(3D0F, AES3PCMDescriptor_BlockStartOffset,       "Position of first Z preamble in essence stream")
        ELEMENT(3D10, AES3PCMDescriptor_ChannelStatusMode,      "Enumerated mode of carriage of channel status data")
        ELEMENT(3D11, AES3PCMDescriptor_FixedChannelStatusData, "Fixed data pattern for channel status data")
        ELEMENT(3D12, AES3PCMDescriptor_UserDataMode,           "Mode of carriage of user data")
        ELEMENT(3D13, AES3PCMDescriptor_FixedUserData,          "Fixed data pattern for user data")
        default: WaveAudioDescriptor();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::CDCIEssenceDescriptor()
{
    switch(Code2)
    {
        ELEMENT(3301, CDCIEssenceDescriptor_ComponentDepth,     "Active bits per sample")
        ELEMENT(3302, CDCIEssenceDescriptor_HorizontalSubsampling, "Horizontal colour subsampling")
        ELEMENT(3303, CDCIEssenceDescriptor_ColorSiting,        "Color siting")
        ELEMENT(3304, CDCIEssenceDescriptor_BlackRefLevel,      "Black refernece level")
        ELEMENT(3305, CDCIEssenceDescriptor_WhiteReflevel,      "White reference level")
        ELEMENT(3306, CDCIEssenceDescriptor_ColorRange,         "Color range")
        ELEMENT(3307, CDCIEssenceDescriptor_PaddingBits,        "Bits to round up each pixel to stored size")
        ELEMENT(3308, CDCIEssenceDescriptor_VerticalSubsampling,"Vertical colour subsampling")
        ELEMENT(3309, CDCIEssenceDescriptor_AlphaSampleDepth,   "Bits per alpha sample")
        ELEMENT(330B, CDCIEssenceDescriptor_ReversedByteOrder,  "Luma followed by Chroma")
        default: GenericPictureEssenceDescriptor();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::OpenIncompleteHeaderPartition()
{
    //Parsing
    PartitionMetadata();
}

//---------------------------------------------------------------------------
void File_Mxf::ClosedIncompleteHeaderPartition()
{
    //Parsing
    PartitionMetadata();
}

//---------------------------------------------------------------------------
void File_Mxf::OpenCompleteHeaderPartition()
{
    //Parsing
    PartitionMetadata();
}

//---------------------------------------------------------------------------
void File_Mxf::ClosedCompleteHeaderPartition()
{
    //Parsing
    PartitionMetadata();
}

//---------------------------------------------------------------------------
void File_Mxf::OpenIncompleteBodyPartition()
{
    //Parsing
    PartitionMetadata();
}

//---------------------------------------------------------------------------
void File_Mxf::ClosedIncompleteBodyPartition()
{
    //Parsing
    PartitionMetadata();
}

//---------------------------------------------------------------------------
void File_Mxf::OpenCompleteBodyPartition()
{
    //Parsing
    PartitionMetadata();
}

//---------------------------------------------------------------------------
void File_Mxf::ClosedCompleteBodyPartition()
{
    //Parsing
    PartitionMetadata();
}

//---------------------------------------------------------------------------
void File_Mxf::OpenIncompleteFooterPartition()
{
    //Parsing
    PartitionMetadata();
}

//---------------------------------------------------------------------------
void File_Mxf::ClosedIncompleteFooterPartition()
{
    //Parsing
    PartitionMetadata();
}

//---------------------------------------------------------------------------
void File_Mxf::OpenCompleteFooterPartition()
{
    //Parsing
    PartitionMetadata();
}

//---------------------------------------------------------------------------
void File_Mxf::ClosedCompleteFooterPartition()
{
    //Parsing
    PartitionMetadata();
}

//---------------------------------------------------------------------------
void File_Mxf::ContentStorage()
{
    switch(Code2)
    {
        ELEMENT(1901, ContentStorage_Packages,                  "Packages")
        ELEMENT(1902, ContentStorage_EssenceContainerData,      "EssenceContainerData")
        default: GenerationInterchangeObject();
    }

    if (Code2==0x3C0A && InstanceUID==Prefaces[Preface_Current].ContentStorage) //InstanceIUD
    {
        Element_Level--;
        Element_Info("Valid from Preface");
        Element_Level++;
    }
}

//---------------------------------------------------------------------------
void File_Mxf::DMSegment()
{
    switch(Code2)
    {
        ELEMENT(6101, DMSegment_DMFramework,                    "DM Framework")
        default: StructuralComponent();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::EssenceContainerData()
{
    switch(Code2)
    {
        ELEMENT(2701, EssenceContainerData_LinkedPackageUID,    "LinkedPackageUID")
        ELEMENT(3F06, EssenceContainerData_IndexSID,            "IndexSID")
        ELEMENT(3F07, EssenceContainerData_BodySID,             "BodySID")
        default: GenerationInterchangeObject();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::EventTrack()
{
    switch(Code2)
    {
        ELEMENT(4901, EventTrack_EventEditRate,                 "Edit Rate of Event Track")
        ELEMENT(4902, EventTrack_EventOrigin,                   "Offset used to resolved timeline references to this event track")
        default: GenericTrack();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::FileDescriptor()
{
    switch(Code2)
    {
        ELEMENT(3001, FileDescriptor_SampleRate,                "SampleRate")
        ELEMENT(3002, FileDescriptor_ContainerDuration,         "ContainerDuration")
        ELEMENT(3004, FileDescriptor_EssenceContainer,          "EssenceContainer")
        ELEMENT(3005, FileDescriptor_Codec,                     "Codec")
        ELEMENT(3006, FileDescriptor_LinkedTrackID,             "LinkedTrackID")
        default: GenericDescriptor();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::Identification()
{
    switch(Code2)
    {
        ELEMENT(3C01, Identification_CompanyName,               "CompanyName")
        ELEMENT(3C02, Identification_ProductName,               "ProductName")
        ELEMENT(3C03, Identification_ProductVersion,            "ProductVersion")
        ELEMENT(3C04, Identification_VersionString,             "VersionString")
        ELEMENT(3C05, Identification_ProductUID,                "ProductUID")
        ELEMENT(3C06, Identification_ModificationDate ,         "ModificationDate ")
        ELEMENT(3C07, Identification_ToolkitVersion,            "ToolkitVersion")
        ELEMENT(3C08, Identification_Platform,                  "Platform")
        ELEMENT(3C09, Identification_ThisGenerationUID,         "ThisGenerationUID")
        default: InterchangeObject();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::IndexTableSegment()
{
    switch(Code2)
    {
        ELEMENT(3F05, IndexTableSegment_EditUnitByteCount,      "Edit Unit Byte Count")
        ELEMENT(3F06, IndexTableSegment_IndexSID,               "IndexSID")
        ELEMENT(3F07, IndexTableSegment_BodySID,                "BodySID")
        ELEMENT(3F08, IndexTableSegment_SliceCount,             "Slice Count")
        ELEMENT(3F09, IndexTableSegment_DeltaEntryArray,        "Delta Entry Array")
        ELEMENT(3F0A, IndexTableSegment_IndexEntryArray,        "Index Entry Array")
        ELEMENT(3F0B, IndexTableSegment_IndexEditRate,          "Index Edit Rate")
        ELEMENT(3F0C, IndexTableSegment_IndexStartPosition,     "Index Start Position")
        ELEMENT(3F0D, IndexTableSegment_IndexDuration,          "Index Duration")
        ELEMENT(3F0E, IndexTableSegment_PosTableCount,          "PosTableCount")
        default: InterchangeObject();
    }

    if (Code2==0x3C0A) //InstanceIUD
    {
        IndexTable_NSL=0;
        IndexTable_NPE=0;
    }
}

//---------------------------------------------------------------------------
void File_Mxf::GenericDescriptor()
{
    switch(Code2)
    {
        ELEMENT(2F01, GenericDescriptor_Locators,               "Locators")
        default: GenerationInterchangeObject();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::JPEG2000PictureSubDescriptor()
{
    switch(Code2)
    {
        ELEMENT(8001, JPEG2000PictureSubDescriptor_Rsiz,        "Decoder capabilities")
        ELEMENT(8002, JPEG2000PictureSubDescriptor_Xsiz,        "Width")
        ELEMENT(8003, JPEG2000PictureSubDescriptor_Ysiz,        "Height")
        ELEMENT(8004, JPEG2000PictureSubDescriptor_XOsiz,       "Horizontal offset")
        ELEMENT(8005, JPEG2000PictureSubDescriptor_YOsiz,       "Vertical offset ")
        ELEMENT(8006, JPEG2000PictureSubDescriptor_XTsiz,       "Width of one reference tile ")
        ELEMENT(8007, JPEG2000PictureSubDescriptor_YTsiz,       "Height of one reference tile ")
        ELEMENT(8008, JPEG2000PictureSubDescriptor_XTOsiz,      "Horizontal offset of the first tile")
        ELEMENT(8009, JPEG2000PictureSubDescriptor_YTOsiz,      "Vertical offset of the first tile")
        ELEMENT(800A, JPEG2000PictureSubDescriptor_Csiz,        "Number of components in the picture")
        ELEMENT(800B, JPEG2000PictureSubDescriptor_PictureComponentSizing, "Picture components")
        default: GenerationInterchangeObject();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::GenerationInterchangeObject()
{
    //Parsing
    switch(Code2)
    {
        ELEMENT(0102, GenerationInterchangeObject_GenerationUID, "GenerationUID")
        default: InterchangeObject();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::GenericPackage()
{
    switch(Code2)
    {
        ELEMENT(4401, GenericPackage_PackageUID,                "PackageUID")
        ELEMENT(4402, GenericPackage_Name,                      "Name")
        ELEMENT(4403, GenericPackage_Tracks,                    "Tracks")
        ELEMENT(4404, GenericPackage_PackageModifiedDate,       "PackageModifiedDate")
        ELEMENT(4405, GenericPackage_PackageCreationDate,       "PackageCreationDate")
        default: GenerationInterchangeObject();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::GenericPictureEssenceDescriptor()
{
    switch(Code2)
    {
        ELEMENT(3201, GenericPictureEssenceDescriptor_PictureEssenceCoding, "Identifier of the Picture Compression Scheme")
        ELEMENT(3202, GenericPictureEssenceDescriptor_StoredHeight, "Vertical Field Size")
        ELEMENT(3203, GenericPictureEssenceDescriptor_StoredWidth, "Horizontal Size")
        ELEMENT(3204, GenericPictureEssenceDescriptor_SampledHeight, "Sampled height supplied to codec")
        ELEMENT(3205, GenericPictureEssenceDescriptor_SampledWidth, "Sampled width supplied to codec")
        ELEMENT(3206, GenericPictureEssenceDescriptor_SampledXOffset, "Offset from sampled to stored width")
        ELEMENT(3207, GenericPictureEssenceDescriptor_SampledYOffset, "Offset from sampled to stored")
        ELEMENT(3208, GenericPictureEssenceDescriptor_DisplayHeight, "Displayed Height placed in Production Aperture")
        ELEMENT(3209, GenericPictureEssenceDescriptor_DisplayWidth, "Displayed Width placed in Production Aperture")
        ELEMENT(320A, GenericPictureEssenceDescriptor_DisplayXOffset,"Horizontal offset from the of the picture as displayed")
        ELEMENT(320B, GenericPictureEssenceDescriptor_DisplayYOffset,"Vertical offset of the picture as displayed")
        ELEMENT(320C, GenericPictureEssenceDescriptor_FrameLayout, "Interlace or Progressive layout")
        ELEMENT(320D, GenericPictureEssenceDescriptor_VideoLineMap, "First active line in each field")
        ELEMENT(320E, GenericPictureEssenceDescriptor_AspectRatio, "Aspect ratio")
        ELEMENT(320F, GenericPictureEssenceDescriptor_AlphaTransparency, "Is Alpha Inverted")
        ELEMENT(3210, GenericPictureEssenceDescriptor_Gamma,    "Gamma")
        ELEMENT(3211, GenericPictureEssenceDescriptor_ImageAlignmentOffset, "Byte Boundary alignment required for Low Level Essence Storage")
        ELEMENT(3212, GenericPictureEssenceDescriptor_FieldDominance,"Number of the field which is considered temporally to come first")
        ELEMENT(3213, GenericPictureEssenceDescriptor_ImageStartOffset, "Unused bytes before start of stored data")
        ELEMENT(3214, GenericPictureEssenceDescriptor_ImageEndOffset,"Unused bytes before start of stored data")
        ELEMENT(3215, GenericPictureEssenceDescriptor_SignalStandard, "Underlying signal standard")
        ELEMENT(3216, GenericPictureEssenceDescriptor_StoredF2Offset, "Topness Adjustment for stored picture")
        ELEMENT(3217, GenericPictureEssenceDescriptor_DisplayF2Offset, "Topness Adjustment for Displayed Picture")
        ELEMENT(3218, GenericPictureEssenceDescriptor_ActiveFormatDescriptor, "Specifies the intended framing of the content within the displayed image")
        default: FileDescriptor();
    }

    if (Code2==0x3C0A) //InstanceUID
    {
        Descriptors[InstanceUID].StreamKind=Stream_Video;
        if (Streams_Count==(size_t)-1)
            Streams_Count=0;
        Streams_Count++;
    }
}

//---------------------------------------------------------------------------
void File_Mxf::GenericSoundEssenceDescriptor()
{
    //Parsing
    switch(Code2)
    {
        ELEMENT(3D01, GenericSoundEssenceDescriptor_QuantizationBits, "QuantizationBits")
        ELEMENT(3D02, GenericSoundEssenceDescriptor_Locked ,    "Locked ")
        ELEMENT(3D03, GenericSoundEssenceDescriptor_AudioSamplingRate, "AudioSamplingRate")
        ELEMENT(3D04, GenericSoundEssenceDescriptor_AudioRefLevel, "AudioRefLevel")
        ELEMENT(3D05, GenericSoundEssenceDescriptor_ElectroSpatialFormulation, "ElectroSpatialFormulation")
        ELEMENT(3D06, GenericSoundEssenceDescriptor_SoundEssenceCompression, "SoundEssenceCompression")
        ELEMENT(3D07, GenericSoundEssenceDescriptor_ChannelCount, "ChannelCount")
        ELEMENT(3D0C, GenericSoundEssenceDescriptor_DialNorm, "DialNorm")
        default: FileDescriptor();
    }

    if (Code2==0x3C0A) //InstanceUID
    {
        Descriptors[InstanceUID].StreamKind=Stream_Audio;
        if (Streams_Count==(size_t)-1)
            Streams_Count=0;
        Streams_Count++;
    }
}

//---------------------------------------------------------------------------
void File_Mxf::GenericTrack()
{
    //Parsing
    switch(Code2)
    {
        ELEMENT(4801, GenericTrack_TrackID,                     "TrackID")
        ELEMENT(4802, GenericTrack_TrackName,                   "TrackName")
        ELEMENT(4803, GenericTrack_Sequence,                    "Sequence")
        ELEMENT(4804, GenericTrack_TrackNumber,                 "TrackNumber")
        default: GenerationInterchangeObject();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::InterchangeObject()
{
    //Parsing
    switch(Code2)
    {
        ELEMENT(3C0A, InterchangeObject_InstanceUID,            "InstanceUID")
        default: ;
    }
}

//---------------------------------------------------------------------------
void File_Mxf::MaterialPackage()
{
    GenericPackage();

    if (Code2==0x3C0A)
    {
        if (InstanceUID==Prefaces[Preface_Current].PrimaryPackage) //InstanceIUD
        {
            Element_Level--;
            Element_Info("Primary package");
            Element_Level++;
        }
        for (contentstorages::iterator ContentStorage=ContentStorages.begin(); ContentStorage!=ContentStorages.end(); ContentStorage++)
        {
            for (size_t Pos=0; Pos<ContentStorage->second.Packages.size(); Pos++)
                if (InstanceUID==ContentStorage->second.Packages[Pos])
                {
                    Element_Level--;
                    Element_Info("Valid from Content storage");
                    Element_Level++;
                }
        }
    }
}

//---------------------------------------------------------------------------
void File_Mxf::MPEG2VideoDescriptor()
{
    std::map<int16u, int128u>::iterator Primer_Value=Primer_Values.find(Code2);
    if (Primer_Value==Primer_Values.end()) //if not a standard code or unknown user defined code
    {
        CDCIEssenceDescriptor();
        return;
    }

    int32u Code_Compare1=Primer_Value->second.hi>>32;
    int32u Code_Compare2=(int32u)Primer_Value->second.hi;
    int32u Code_Compare3=Primer_Value->second.lo>>32;
    int32u Code_Compare4=(int32u)Primer_Value->second.lo;
    if(0);
    ELEMENT_UUID(MPEG2VideoDescriptor_SingleSequence,           "Single sequence")
    ELEMENT_UUID(MPEG2VideoDescriptor_ConstantBFrames,          "Number of B frames always constant")
    ELEMENT_UUID(MPEG2VideoDescriptor_CodedContentType,         "Coded content type")
    ELEMENT_UUID(MPEG2VideoDescriptor_LowDelay,                 "Low delay")
    ELEMENT_UUID(MPEG2VideoDescriptor_ClosedGOP,                "Closed GOP")
    ELEMENT_UUID(MPEG2VideoDescriptor_IdenticalGOP,             "Identical GOP")
    ELEMENT_UUID(MPEG2VideoDescriptor_MaxGOP,                   "Maximum occurring spacing between I frames")
    ELEMENT_UUID(MPEG2VideoDescriptor_BPictureCount,            "Maximum number of B pictures between P or I frames")
    ELEMENT_UUID(MPEG2VideoDescriptor_ProfileAndLevel,          "Profile and level")
    ELEMENT_UUID(MPEG2VideoDescriptor_BitRate,                  "Maximum bit rate")
}

//---------------------------------------------------------------------------
void File_Mxf::MultipleDescriptor()
{
    switch(Code2)
    {
        ELEMENT(3F01, MultipleDescriptor_SubDescriptorUIDs,     "SubDescriptorUIDs")
        default: FileDescriptor();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::NetworkLocator()
{
    switch(Code2)
    {
        ELEMENT(4001, NetworkLocator_URLString,                 "A URL indicating where the essence may be found.")
        default: GenerationInterchangeObject();
    }

    if (Code2==0x3C0A)
    {
        for (descriptors::iterator Descriptor=Descriptors.begin(); Descriptor!=Descriptors.end(); Descriptor++)
        {
            for (size_t Pos=0; Pos<Descriptor->second.Locators.size(); Pos++)
                if (InstanceUID==Descriptor->second.Locators[Pos])
                {
                    Element_Level--;
                    Element_Info("Valid from Descriptor");
                    Element_Level++;
                }
        }
    }
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void File_Mxf::Padding()
{
    //Parsing
    Skip_XX(Element_Size,                                       "Padding");
}

//---------------------------------------------------------------------------
void File_Mxf::Preface()
{
    switch(Code2)
    {
        ELEMENT(3B02, Preface_LastModifiedDate,                 "LastModifiedDate")
        ELEMENT(3B03, Preface_ContentStorage,                   "ContentStorage")
        ELEMENT(3B05, Preface_Version,                          "Version")
        ELEMENT(3B06, Preface_Identifications,                  "Identifications")
        ELEMENT(3B07, Preface_ObjectModelVersion,               "ObjectModelVersion")
        ELEMENT(3B08, Preface_PrimaryPackage,                   "PrimaryPackage")
        ELEMENT(3B09, Preface_OperationalPattern,               "OperationalPattern")
        ELEMENT(3B0A, Preface_EssenceContainers,                "EssenceContainers")
        ELEMENT(3B0B, Preface_DMSchemes,                        "DMSchemes")
        default: GenerationInterchangeObject();
    }

    if (Code2==0x3C0A) //InstanceIUD
    {
        Preface_Current=InstanceUID;
    }
}

//---------------------------------------------------------------------------
void File_Mxf::Primer()
{
    //Parsing
    //Vector
    int32u Count, Length;
    Get_B4 (Count,                                              "Count");
    Get_B4 (Length,                                             "Length");
    for (int32u Pos=0; Pos<Count; Pos++)
    {
        Element_Begin("LocalTagEntryBatch", Length);
        int16u LocalTag;
        int128u UID;
        Get_B2 (LocalTag,                                       "LocalTag"); Element_Info(Ztring().From_CC2(LocalTag));
        Get_UL (UID,                                            "UID", NULL); Element_Info(Ztring().From_UUID(UID));
        Element_End();

        FILLING_BEGIN();
            if (LocalTag>=0x8000) //user defined
                Primer_Values[LocalTag]=UID;
        FILLING_END();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::RGBAEssenceDescriptor()
{
    switch(Code2)
    {
        ELEMENT(3401, RGBAEssenceDescriptor_PixelLayout,        "Pixel Layout")
        ELEMENT(3403, RGBAEssenceDescriptor_Palette,            "Palette")
        ELEMENT(3404, RGBAEssenceDescriptor_PaletteLayout,      "Palette Layout")
        ELEMENT(3405, RGBAEssenceDescriptor_ScanningDirection,  "Enumerated Scanning Direction")
        ELEMENT(3406, RGBAEssenceDescriptor_ComponentMaxRef,    "Maximum value for RGB components")
        ELEMENT(3407, RGBAEssenceDescriptor_ComponentMinRef,    "Minimum value for RGB components")
        ELEMENT(3408, RGBAEssenceDescriptor_AlphaMaxRef,        "Maximum value for alpha component")
        ELEMENT(3409, RGBAEssenceDescriptor_AlphaMinRef,        "Minimum value for alpha component")
        default: GenericPictureEssenceDescriptor();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::RandomIndexMetadata()
{
    //Keep trace
    bool AlreadyParsed=!RandomIndexMetadatas.empty();

    //Parsing
    while (Element_Offset+4<Element_Size)
    {
        Element_Begin("PartitionArray", 12);
        randomindexmetadata RandomIndexMetadata;
        Get_B4 (RandomIndexMetadata.BodySID,                    "BodySID"); Element_Info(RandomIndexMetadata.BodySID);
        Get_B8 (RandomIndexMetadata.ByteOffset,                 "ByteOffset"); Element_Info(Ztring::ToZtring(RandomIndexMetadata.ByteOffset, 16));
        Element_End();

        FILLING_BEGIN();
            if (!AlreadyParsed)
                RandomIndexMetadatas.push_back(RandomIndexMetadata);
        FILLING_END();
    }
    Skip_B4(                                                    "Length");

    FILLING_BEGIN();
        if (MediaInfoLib::Config.ParseSpeed_Get()<1.0 && !AlreadyParsed && RandomIndexMetadatas.size()>=2 && RandomIndexMetadatas[RandomIndexMetadatas.size()-1].BodySID==0 && RandomIndexMetadatas[RandomIndexMetadatas.size()-2].BodySID!=0)
        {
            Data_GoTo(RandomIndexMetadatas[RandomIndexMetadatas.size()-1].ByteOffset, "MXF");
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
void File_Mxf::Sequence()
{
    switch(Code2)
    {
        ELEMENT(1001, Sequence_StructuralComponents,            "StructuralComponents")
        default: StructuralComponent();
    }

    if (Code2==0x3C0A)
    {
        for (std::map<int128u, track>::iterator Track=Tracks.begin(); Track!=Tracks.end(); Track++)
        {
            if (InstanceUID==Track->second.Sequence)
            {
                Element_Level--;
                Element_Info("Valid from track");
                Element_Level++;
            }
        }
    }
}

//---------------------------------------------------------------------------
void File_Mxf::SourceClip()
{
    switch(Code2)
    {
        ELEMENT(1101, SourceClip_SourcePackageID,               "SourcePackageID")
        ELEMENT(1102, SourceClip_SourceTrackID,                 "SourceTrackID")
        ELEMENT(1201, SourceClip_StartPosition,                 "StartPosition")
        default: StructuralComponent();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::SourcePackage()
{
    switch(Code2)
    {
        //SourcePackage
        ELEMENT(4701, SourcePackage_Descriptor,                 "Descriptor")
        default: GenericPackage();
                 Packages[InstanceUID].IsSourcePackage=true;
    }
}

//---------------------------------------------------------------------------
void File_Mxf::StaticTrack()
{
    GenericTrack();
}

//---------------------------------------------------------------------------
//SMPTE 405M
void File_Mxf::SystemScheme1()
{
    switch(Code2)
    {
        ELEMENT(0102, SystemScheme1_TimeCodeArray,              "TimeCode Array")
        default: InterchangeObject();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::StructuralComponent()
{
    switch(Code2)
    {
        ELEMENT(0201, StructuralComponent_DataDefinition,       "DataDefinition")
        ELEMENT(0202, StructuralComponent_Duration,             "Duration")
        default: GenerationInterchangeObject();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::TextLocator()
{
    switch(Code2)
    {
        ELEMENT(4101, TextLocator_LocatorName,                  "Human-readable locator text string for manual location of essence")
        default: GenerationInterchangeObject();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::TimecodeComponent()
{
    switch(Code2)
    {
        ELEMENT(1501, TimecodeComponent_StartTimecode,          "StartTimecode")
        ELEMENT(1502, TimecodeComponent_RoundedTimecodeBase,    "RoundedTimecodeBase")
        ELEMENT(1503, TimecodeComponent_DropFrame,              "DropFrame")
        default: StructuralComponent();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::WaveAudioDescriptor()
{
    switch(Code2)
    {
        ELEMENT(3D09, WaveAudioDescriptor_AvgBps,               "Average Bytes per second")
        ELEMENT(3D0A, WaveAudioDescriptor_BlockAlign,           "Sample Block alignment")
        ELEMENT(3D0B, WaveAudioDescriptor_SequenceOffset,       "Frame number of first essence")
        ELEMENT(3D29, WaveAudioDescriptor_PeakEnvelopeVersion,  "Peak envelope version information")
        ELEMENT(3D2A, WaveAudioDescriptor_PeakEnvelopeFormat,   "Format of a peak point")
        ELEMENT(3D2B, WaveAudioDescriptor_PointsPerPeakValue,   "Number of peak points per peak value")
        ELEMENT(3D2C, WaveAudioDescriptor_PeakEnvelopeBlockSize,"Number of audio samples used to generate each peak frame")
        ELEMENT(3D2D, WaveAudioDescriptor_PeakChannels,         "Number of peak channels")
        ELEMENT(3D2E, WaveAudioDescriptor_PeakFrames,           "Number of peak frames")
        ELEMENT(3D2F, WaveAudioDescriptor_PeakOfPeaksPosition,  "Offset to the first audio sample whose absolute value is the maximum value of the entire audio file")
        ELEMENT(3D30, WaveAudioDescriptor_PeakEnvelopeTimestamp,"Time stamp of the creation of the peak data")
        ELEMENT(3D31, WaveAudioDescriptor_PeakEnvelopeData ,    "Peak envelope data")
        ELEMENT(3D32, WaveAudioDescriptor_ChannelAssignment,    "Channel assignment in use")
        default: GenericSoundEssenceDescriptor();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::AncPacketsDescriptor()
{
    Ancillary_InstanceUID=InstanceUID;
    switch(Code2)
    {
        default: FileDescriptor();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::Filler()
{
    Skip_XX(Element_Size,                                       "Junk");
}

//---------------------------------------------------------------------------
void File_Mxf::TerminatingFiller()
{
    Skip_XX(Element_Size,                                       "Junk");
}

//---------------------------------------------------------------------------
void File_Mxf::XmlDocumentText()
{
    Skip_XX(Element_Size,                                       "XML data");
}

//---------------------------------------------------------------------------
void File_Mxf::SDTI_SystemMetadataPack() //SMPTE 385M + 326M
{
    //Parsing
    int8u SMB, CPR_Rate, Format;
    bool SMB_UL_Present, SMB_CreationTimeStamp, SMB_UserTimeStamp, CPR_DropFrame;
    Get_B1 (SMB,                                                "System Metadata Bitmap");
        Skip_Flags(SMB, 7,                                      "FEC Active");
        Get_Flags (SMB, 6, SMB_UL_Present,                      "SMPTE Label");
        Get_Flags (SMB, 5, SMB_CreationTimeStamp,               "Creation Date/Time");
        Get_Flags (SMB, 4, SMB_UserTimeStamp,                   "User Date/Time");
        Skip_Flags(SMB, 3,                                      "Picture item");
        Skip_Flags(SMB, 2,                                      "Sound item");
        Skip_Flags(SMB, 1,                                      "Data item");
        Skip_Flags(SMB, 0,                                      "Control item");
    BS_Begin();
    Element_Begin("Content Package Rate");
    Skip_S1(2,                                                  "Reserved");
    Get_S1 (5, CPR_Rate,                                        "Package Rate"); //See SMPTE 326M
    Get_SB (   CPR_DropFrame,                                   "1.001 Flag");
    Element_End();
    Element_Begin("Content Package Type");
    Skip_S1(3,                                                  "Stream Status");
    Skip_SB(                                                    "Sub-package flag");
    Skip_SB(                                                    "Transfer Mode");
    Skip_S1(3,                                                  "Timing Mode");
    Element_End();
    BS_End();
    Skip_B2(                                                    "channel handle");
    Skip_B2(                                                    "continuity count");

    //Some computing
    float64 FrameRate;
    switch (CPR_Rate) //See SMPTE 326M
    {
        case 0x01 : FrameRate=24; break;
        case 0x02 : FrameRate=25; break;
        case 0x03 : FrameRate=30; break;
        case 0x04 : FrameRate=48; break;
        case 0x05 : FrameRate=50; break;
        case 0x06 : FrameRate=60; break;
        case 0x07 : FrameRate=72; break;
        case 0x08 : FrameRate=75; break;
        case 0x09 : FrameRate=90; break;
        case 0x0A : FrameRate=96; break;
        case 0x0B : FrameRate=100; break;
        case 0x0C : FrameRate=120; break;
        default   : FrameRate=0; break;    
    }
    if (CPR_DropFrame)
    {
        FrameRate*=1000;
        FrameRate/=1001;
    }

    //Parsing
    if (SMB_UL_Present)
        Skip_UL(                                                "SMPTE Universal label");
    if (SMB_CreationTimeStamp)
    {
        Get_B1 (Format,                                         "Format"); //0x81=timecode, 0x82=date-timecode
        Skip_B8(                                                "Time stamp");
        Skip_B8(                                                "Zero");
    }
    else
        Skip_XX(17,                                             "Junk");
    if (SMB_UserTimeStamp)
    {
        Get_B1 (Format,                                         "Format"); //0x81=timecode, 0x82=date-timecode, SMPTE 331M
        Element_Begin("TimeCode");
        int8u Frames_Units, Frames_Tens, Seconds_Units, Seconds_Tens, Minutes_Units, Minutes_Tens, Hours_Units, Hours_Tens;
        bool  DropFrame;
        BS_Begin();

        Skip_SB(                                                "CF - Color fame");
        Get_SB (   DropFrame,                                   "DP - Drop frame");
        Get_S1 (2, Frames_Tens,                                 "Frames (Tens)");
        Get_S1 (4, Frames_Units,                                "Frames (Units)");

        Skip_SB(                                                "FP - Field Phase / BGF0");
        Get_S1 (3, Seconds_Tens,                                "Seconds (Tens)");
        Get_S1 (4, Seconds_Units,                               "Seconds (Units)");

        Skip_SB(                                                "BGF0 / BGF2");
        Get_S1 (3, Minutes_Tens,                                "Minutes (Tens)");
        Get_S1 (4, Minutes_Units,                               "Minutes (Units)");

        Skip_SB(                                                "BGF2 / Field Phase");
        Skip_SB(                                                "BGF1");
        Get_S1 (2, Hours_Tens,                                  "Hours (Tens)");
        Get_S1 (4, Hours_Units,                                 "Hours (Units)");

        Skip_S1(4,                                              "BG2");
        Skip_S1(4,                                              "BG1");

        Skip_S1(4,                                              "BG4");
        Skip_S1(4,                                              "BG3");

        Skip_S1(4,                                              "BG6");
        Skip_S1(4,                                              "BG5");

        Skip_S1(4,                                              "BG8");
        Skip_S1(4,                                              "BG7");

        BS_End();

        int64u TimeCode=(int64u)(Hours_Tens     *10*60*60*1000
                               + Hours_Units       *60*60*1000
                               + Minutes_Tens      *10*60*1000
                               + Minutes_Units        *60*1000
                               + Seconds_Tens         *10*1000
                               + Seconds_Units           *1000
                               + FrameRate?float64_int32s((Frames_Tens*10+Frames_Units)*1000/FrameRate):0);

        Element_Info(Ztring().Duration_From_Milliseconds(TimeCode));

        Element_End();

        Skip_B8(                                            "Zero");

        //TimeCode
        if (SDTI_TimeCode_StartTimecode==(int64u)-1)
            SDTI_TimeCode_StartTimecode=TimeCode;
    }
    else
        Skip_XX(17,                                             "Junk");
}

//---------------------------------------------------------------------------
void File_Mxf::SDTI_PackageMetadataSet()
{
    while (Element_Offset<Element_Size)
    {
        //Parsing
        int16u Length;
        int8u Tag;
        Get_B1 (Tag,                                            "Tag");
        Get_B2 (Length,                                         "Length");
        switch (Tag)
        {
            case 0x83 : //UMID
                        {
                            Skip_XX(64,                         "UMID?");
                        }
                        break;
            case 0x88 : //KLV Metadata
                        {
                            Skip_XX(Length,                     "KLV Metadata");
                        }
                        break;
            default   : Skip_XX(Length,                         "Unknown");
        }
    }

}

//---------------------------------------------------------------------------
void File_Mxf::SDTI_PictureMetadataSet()
{
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mxf::SDTI_SoundMetadataSet()
{
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mxf::SDTI_DataMetadataSet()
{
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mxf::SDTI_ControlMetadataSet()
{
    Skip_XX(Element_Size,                                       "Data");
}

//---------------------------------------------------------------------------
void File_Mxf::Omneon_010201010100()
{
    //Parsing
    switch(Code2)
    {
        ELEMENT(8001, Omneon_010201010100_8001,                 "Omneon (80.01)")
        ELEMENT(8003, Omneon_010201010100_8003,                 "Omneon (80.03)")
        default: GenerationInterchangeObject();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::Omneon_010201020100()
{
    //Parsing
    switch(Code2)
    {
        ELEMENT(8002, Omneon_010201020100_8002,                 "Omneon (80.02)")
        ELEMENT(8003, Omneon_010201020100_8003,                 "Omneon (80.03)")
        ELEMENT(8004, Omneon_010201020100_8004,                 "Omneon (80.04)")
        ELEMENT(8005, Omneon_010201020100_8005,                 "Omneon (80.05)")
        ELEMENT(8006, Omneon_010201020100_8006,                 "Omneon (80.06)")
        default: GenerationInterchangeObject();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::Track()
{
    //Parsing
    switch(Code2)
    {
        ELEMENT(4B01, Track_EditRate,                           "EditRate")
        ELEMENT(4B02, Track_Origin,                             "Origin")
        default: GenericTrack();
    }

    if (Code2==0x3C0A)
    {
        for (packages::iterator Package=Packages.begin(); Package!=Packages.end(); Package++)
        {
            if (Package->first==Prefaces[Preface_Current].PrimaryPackage) //InstanceIUD
            {
                Element_Level--;
                Element_Info("Primary package");
                Element_Level++;
            }
            for (size_t Pos=0; Pos<Package->second.Tracks.size(); Pos++)
                if (InstanceUID==Package->second.Tracks[Pos])
                {
                    Element_Level--;
                    Element_Info("Valid from Package");
                    Element_Level++;
                }
        }
    }
}

//***************************************************************************
// Base
//***************************************************************************

//---------------------------------------------------------------------------
// 0x3D08
void File_Mxf::AES3PCMDescriptor_AuxBitsMode()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3D0D
void File_Mxf::AES3PCMDescriptor_Emphasis()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3D0F
void File_Mxf::AES3PCMDescriptor_BlockStartOffset()
{
    //Parsing
    Info_B2(Data,                                               "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3D10
void File_Mxf::AES3PCMDescriptor_ChannelStatusMode()
{
    //Parsing
    Skip_XX(Length2,                                            "Batch");
}

//---------------------------------------------------------------------------
// 0x3D11
void File_Mxf::AES3PCMDescriptor_FixedChannelStatusData()
{
    //Parsing
    Skip_XX(Length2,                                           "Data");
}

//---------------------------------------------------------------------------
// 0x3D12
void File_Mxf::AES3PCMDescriptor_UserDataMode()
{
    //Parsing
    Skip_XX(Length2,                                           "Data");
}

//---------------------------------------------------------------------------
// 0x3D13
void File_Mxf::AES3PCMDescriptor_FixedUserData()
{
    //Parsing
    Skip_XX(Length2,                                           "Data");
}

//---------------------------------------------------------------------------
// 0x3301
void File_Mxf::CDCIEssenceDescriptor_ComponentDepth()
{
    //Parsing
    int32u Data;
    Get_B4 (Data,                                                "Data"); Element_Info(Data);

    FILLING_BEGIN();
        if (Data)
            Descriptors[InstanceUID].Infos["Resolution"].From_Number(Data);
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3302
void File_Mxf::CDCIEssenceDescriptor_HorizontalSubsampling()
{
    //Parsing
    Info_B4(Data,                                               "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3303
void File_Mxf::CDCIEssenceDescriptor_ColorSiting()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3304
void File_Mxf::CDCIEssenceDescriptor_BlackRefLevel()
{
    //Parsing
    Info_B4(Data,                                               "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3305
void File_Mxf::CDCIEssenceDescriptor_WhiteReflevel()
{
    //Parsing
    Info_B4(Data,                                               "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3306
void File_Mxf::CDCIEssenceDescriptor_ColorRange()
{
    //Parsing
    Info_B4(Data,                                               "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3307
void File_Mxf::CDCIEssenceDescriptor_PaddingBits()
{
    //Parsing
    Info_B2(Data,                                               "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3308
void File_Mxf::CDCIEssenceDescriptor_VerticalSubsampling()
{
    //Parsing
    Info_B4(Data,                                               "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3309
void File_Mxf::CDCIEssenceDescriptor_AlphaSampleDepth()
{
    //Parsing
    Info_B4(Data,                                               "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x330B
void File_Mxf::CDCIEssenceDescriptor_ReversedByteOrder()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x1901
void File_Mxf::ContentStorage_Packages()
{
    //Parsing
    //Vector
    int32u Count, Length;
    Get_B4 (Count,                                              "Count");
    Get_B4 (Length,                                             "Length");
    for (int32u Pos=0; Pos<Count; Pos++)
    {
        int128u Data;
        Get_UUID(Data,                                          "Package");

        FILLING_BEGIN();
            if (Data==Prefaces[Preface_Current].PrimaryPackage)
                Element_Info("Primary package");
            ContentStorages[InstanceUID].Packages.push_back(Data);
        FILLING_END();
    }
}

//---------------------------------------------------------------------------
// 0x1902
void File_Mxf::ContentStorage_EssenceContainerData()
{
    //Parsing
    //Vector
    int32u Count, Length;
    Get_B4 (Count,                                              "Count");
    Get_B4 (Length,                                             "Length");
    for (int32u Pos=0; Pos<Count; Pos++)
    {
        Skip_UUID(                                              "EssenceContainer");
    }
}

//---------------------------------------------------------------------------
// 0x6101
void File_Mxf::DMSegment_DMFramework()
{
    //Parsing
    Info_UUID(Data,                                             "DM Framework"); Element_Info(Ztring().From_UUID(Data));
}

//---------------------------------------------------------------------------
// 0x2701
void File_Mxf::EssenceContainerData_LinkedPackageUID()
{
    //Parsing
    Skip_UMID();
}

//---------------------------------------------------------------------------
// 0x3F06
void File_Mxf::EssenceContainerData_IndexSID()
{
    //Parsing
    Info_B4(Data,                                               "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3F07
void File_Mxf::EssenceContainerData_BodySID()
{
    //Parsing
    Info_B4(Data,                                               "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x4901
void File_Mxf::EventTrack_EventEditRate()
{
    //Parsing
    Info_Rational();
}

//---------------------------------------------------------------------------
// 0x4902
void File_Mxf::EventTrack_EventOrigin()
{
    //Parsing
    Info_B8(Data,                                               "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3001
void File_Mxf::FileDescriptor_SampleRate()
{
    //Parsing
    Get_Rational(Descriptors[InstanceUID].SampleRate); Element_Info(Descriptors[InstanceUID].SampleRate);

    FILLING_BEGIN();
        switch (Descriptors[InstanceUID].StreamKind)
        {
            case Stream_Video   : Descriptors[InstanceUID].Infos["FrameRate"]=Ztring().From_Number(Descriptors[InstanceUID].SampleRate, 3); break;
            default             : ;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3002
void File_Mxf::FileDescriptor_ContainerDuration()
{
    //Parsing
    int64u Data;
    Get_B8 (Data,                                               "Data"); Element_Info(Data);

    FILLING_BEGIN();
        float32 SampleRate=Descriptors[InstanceUID].SampleRate;
        if (SampleRate && Data!=0xFFFFFFFFFFFFFFFFLL)
            Descriptors[InstanceUID].Infos["Duration"].From_Number(Data/SampleRate*1000);
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3004
void File_Mxf::FileDescriptor_EssenceContainer()
{
    //Parsing
    Info_UL(EssenceContainer,                                   "EssenceContainer", Mxf_EssenceContainer); Element_Info(Mxf_EssenceContainer(EssenceContainer));

    FILLING_BEGIN();
        int8u Code6=(int8u)((EssenceContainer.lo&0x0000000000FF0000LL)>>16);
        int8u Code7=(int8u)((EssenceContainer.lo&0x000000000000FF00LL)>> 8);
        int8u Code8=(int8u)((EssenceContainer.lo&0x00000000000000FFLL)    );

        Descriptors[InstanceUID].Infos["Format_Settings_Wrapping"].From_UTF8(Mxf_EssenceContainer_Mapping(Code6, Code7, Code8));
    FILLING_END();

}

//---------------------------------------------------------------------------
// 0x3005
void File_Mxf::FileDescriptor_Codec()
{
    //Parsing
    Skip_UL(                                                    "UUID");
}

//---------------------------------------------------------------------------
// 0x3006
void File_Mxf::FileDescriptor_LinkedTrackID()
{
    //Parsing
    int32u Data;
    Get_B4 (Data,                                               "Data"); Element_Info(Data);

    FILLING_BEGIN();
        if (Descriptors[InstanceUID].LinkedTrackID==(int32u)-1)
            Descriptors[InstanceUID].LinkedTrackID=Data;
        if (InstanceUID==Ancillary_InstanceUID)
            Ancillary_LinkedTrackID=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3C0A
void File_Mxf::InterchangeObject_InstanceUID()
{
    //Parsing
    Get_UUID(InstanceUID,                                       "UUID"); Element_Info(Ztring().From_UUID(InstanceUID));

    FILLING_BEGIN();
        //Putting the right UID for already parsed items
        prefaces::iterator Preface=Prefaces.find(0);
        if (Preface!=Prefaces.end())
        {
            Prefaces[InstanceUID]=Preface->second;
            Prefaces.erase(Preface);
        }
        identifications::iterator Identification=Identifications.find(0);
        if (Identification!=Identifications.end())
        {
            Identifications[InstanceUID]=Identification->second;
            Identifications.erase(Identification);
        }
        contentstorages::iterator ContentStorage=ContentStorages.find(0);
        if (ContentStorage!=ContentStorages.end())
        {
            ContentStorages[InstanceUID]=ContentStorage->second;
            ContentStorages.erase(ContentStorage);
        }
        packages::iterator Package=Packages.find(0);
        if (Package!=Packages.end())
        {
            Packages[InstanceUID]=Package->second;
            Packages.erase(Package);
        }
        tracks::iterator Track=Tracks.find(0);
        if (Track!=Tracks.end())
        {
            Tracks[InstanceUID]=Track->second;
            Tracks.erase(Track);
        }
        descriptors::iterator Descriptor=Descriptors.find(0);
        if (Descriptor!=Descriptors.end())
        {
            Descriptors[InstanceUID]=Descriptor->second;
            Descriptors.erase(Descriptor);
        }
        locators::iterator Locator=Locators.find(0);
        if (Locator!=Locators.end())
        {
            Locators[InstanceUID]=Locator->second;
            Locators.erase(Locator);
        }
        components::iterator Component=Components.find(0);
        if (Component!=Components.end())
        {
            Components[InstanceUID]=Component->second;
            Components.erase(Component);
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x0102
void File_Mxf::GenerationInterchangeObject_GenerationUID()
{
    //Parsing
    Skip_UUID(                                                  "UUID");
}

//---------------------------------------------------------------------------
// 0x2F01
void File_Mxf::GenericDescriptor_Locators()
{
    //Parsing
    //Vector
    int32u Count, Length;
    Get_B4 (Count,                                              "Count");
    Get_B4 (Length,                                             "Length");
    for (int32u Pos=0; Pos<Count; Pos++)
    {
        Element_Begin("Locator", Length);
        int128u UUID;
        Get_UUID(UUID,                                          "UUID");

        FILLING_BEGIN();
            Descriptors[InstanceUID].Locators.push_back(UUID);
        FILLING_END();

        Element_End();
    }
}

//---------------------------------------------------------------------------
// 0x4401
void File_Mxf::GenericPackage_PackageUID()
{
    //Parsing
    int256u Data;
    Get_UMID (Data,                                             "PackageUID");

    FILLING_BEGIN();
        Packages[InstanceUID].PackageUID=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x4402
void File_Mxf::GenericPackage_Name()
{
    //Parsing
    Info_UTF16B(Length2, Data,                                  "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x4403
void File_Mxf::GenericPackage_Tracks()
{
    //Parsing
    //Vector
    int32u Count, Length;
    Get_B4 (Count,                                              "Count");
    Get_B4 (Length,                                             "Length");
    for (int32u Pos=0; Pos<Count; Pos++)
    {
        int128u Data;
        Get_UUID(Data,                                          "Track");

        FILLING_BEGIN();
            Packages[InstanceUID].Tracks.push_back(Data);
        FILLING_END();
    }
}

//---------------------------------------------------------------------------
// 0x4404
void File_Mxf::GenericPackage_PackageModifiedDate()
{
    //Parsing
    Info_Timestamp();
}

//---------------------------------------------------------------------------
// 0x4405
void File_Mxf::GenericPackage_PackageCreationDate()
{
    //Parsing
    Info_Timestamp();
}

//---------------------------------------------------------------------------
// 0x3201
void File_Mxf::GenericPictureEssenceDescriptor_PictureEssenceCoding()
{
    //Parsing
    int128u Data;
    Get_UL(Data,                                                "Data", Mxf_EssenceCompression); Element_Info(Mxf_EssenceCompression(Data));

    FILLING_BEGIN();
        Descriptors[InstanceUID].Infos["Format"]=Mxf_EssenceCompression(Data);
        Descriptors[InstanceUID].Infos["Format_Version"]=Mxf_EssenceCompression_Version(Data);
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3202
void File_Mxf::GenericPictureEssenceDescriptor_StoredHeight()
{
    //Parsing
    int32u Data;
    Get_B4 (Data,                                                "Data"); Element_Info(Data);

    FILLING_BEGIN();
        if (Descriptors[InstanceUID].Infos["ScanType"]==_T("Interlaced"))
            Data*=2; //This is per field
        if (Descriptors[InstanceUID].Height==(int32u)-1)
            Descriptors[InstanceUID].Height=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3203
void File_Mxf::GenericPictureEssenceDescriptor_StoredWidth()
{
    //Parsing
    int32u Data;
    Get_B4 (Data,                                                "Data"); Element_Info(Data);

    FILLING_BEGIN();
        if (Descriptors[InstanceUID].Width==(int32u)-1)
            Descriptors[InstanceUID].Width=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3204
void File_Mxf::GenericPictureEssenceDescriptor_SampledHeight()
{
    //Parsing
    int32u Data;
    Get_B4 (Data,                                                "Data"); Element_Info(Data);

    FILLING_BEGIN();
        if (Descriptors[InstanceUID].Infos["ScanType"]==_T("Interlaced"))
            Data*=2; //This is per field
        Descriptors[InstanceUID].Height=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3205
void File_Mxf::GenericPictureEssenceDescriptor_SampledWidth()
{
    //Parsing
    int32u Data;
    Get_B4 (Data,                                                "Data"); Element_Info(Data);

    FILLING_BEGIN();
        Descriptors[InstanceUID].Width=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3206
void File_Mxf::GenericPictureEssenceDescriptor_SampledXOffset()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3207
void File_Mxf::GenericPictureEssenceDescriptor_SampledYOffset()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3208
void File_Mxf::GenericPictureEssenceDescriptor_DisplayHeight()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3209
void File_Mxf::GenericPictureEssenceDescriptor_DisplayWidth()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x320A
void File_Mxf::GenericPictureEssenceDescriptor_DisplayXOffset()
{
    //Parsing
    Info_B4(Data,                                               "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x320B
void File_Mxf::GenericPictureEssenceDescriptor_DisplayYOffset()
{
    //Parsing
    Info_B4(Data,                                               "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x320C
void File_Mxf::GenericPictureEssenceDescriptor_FrameLayout()
{
    //Parsing
    int8u Data;
    Get_B1 (Data,                                               "Data"); Element_Info(Data);

    FILLING_BEGIN();
        Descriptors[InstanceUID].Infos["ScanType"]=Data?"Interlaced":"PPF";
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x320D
void File_Mxf::GenericPictureEssenceDescriptor_VideoLineMap()
{
    //Parsing
    int32u Count, Length;
    Get_B4 (Count,                                              "Count");
    Get_B4 (Length,                                             "Length");
    for (int32u Pos=0; Pos<Count; Pos++)
    {
        Skip_B4(                                                "VideoLineMapEntry");
    }
}

//---------------------------------------------------------------------------
// 0x320E
void File_Mxf::GenericPictureEssenceDescriptor_AspectRatio()
{
    //Parsing
    float64 Data;
    Get_Rational(Data);

    FILLING_BEGIN();
        if (Data)
            Descriptors[InstanceUID].Infos["DisplayAspectRatio"].From_Number(Data, 3);
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x320F
void File_Mxf::GenericPictureEssenceDescriptor_AlphaTransparency()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3210
void File_Mxf::GenericPictureEssenceDescriptor_Gamma()
{
    //Parsing
    Skip_UL(                                                    "Data");
}

//---------------------------------------------------------------------------
// 0x3211
void File_Mxf::GenericPictureEssenceDescriptor_ImageAlignmentOffset()
{
    //Parsing
    Info_B4(Data,                                               "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3212
void File_Mxf::GenericPictureEssenceDescriptor_FieldDominance()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3213
void File_Mxf::GenericPictureEssenceDescriptor_ImageStartOffset()
{
    //Parsing
    Info_B4(Data,                                               "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3214
void File_Mxf::GenericPictureEssenceDescriptor_ImageEndOffset()
{
    //Parsing
    Info_B4(Data,                                               "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3215
void File_Mxf::GenericPictureEssenceDescriptor_SignalStandard()
{
    //Parsing
    Info_B1(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3216
void File_Mxf::GenericPictureEssenceDescriptor_StoredF2Offset()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3217
void File_Mxf::GenericPictureEssenceDescriptor_DisplayF2Offset()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3218
void File_Mxf::GenericPictureEssenceDescriptor_ActiveFormatDescriptor()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3D01
void File_Mxf::GenericSoundEssenceDescriptor_QuantizationBits()
{
    //Parsing
    int32u Data;
    Get_B4 (Data,                                               "Data"); Element_Info(Data);

    FILLING_BEGIN();
        if (Data)
            Descriptors[InstanceUID].Infos["Resolution"].From_Number(Data);
    FILLING_END();

}

//---------------------------------------------------------------------------
// 0x3D02
void File_Mxf::GenericSoundEssenceDescriptor_Locked()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info(Data?"Yes":"No");
}

//---------------------------------------------------------------------------
// 0x3D03
void File_Mxf::GenericSoundEssenceDescriptor_AudioSamplingRate()
{
    //Parsing
    float64 Data;
    Get_Rational(Data); Element_Info(Data);

    FILLING_BEGIN();
        Descriptors[InstanceUID].Infos["SamplingRate"].From_Number(Data, 0);
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3D04
void File_Mxf::GenericSoundEssenceDescriptor_AudioRefLevel()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info(Data, " dB");
}

//---------------------------------------------------------------------------
// 0x3D05
void File_Mxf::GenericSoundEssenceDescriptor_ElectroSpatialFormulation()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info(Data); //Enum
}

//---------------------------------------------------------------------------
// 0x3D06
void File_Mxf::GenericSoundEssenceDescriptor_SoundEssenceCompression()
{
    //Parsing
    int128u Data;
    Get_UL(Data,                                                "Data", Mxf_EssenceCompression); Element_Info(Mxf_EssenceCompression(Data));

    FILLING_BEGIN();
        Descriptors[InstanceUID].Infos["Format"]=Mxf_EssenceCompression(Data);
        Descriptors[InstanceUID].Infos["Format_Version"]=Mxf_EssenceCompression_Version(Data);
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3D07
void File_Mxf::GenericSoundEssenceDescriptor_ChannelCount()
{
    //Parsing
    int32u Data;
    Get_B4 (Data,                                               "Data"); Element_Info(Data);

    FILLING_BEGIN();
        Descriptors[InstanceUID].Infos["Channel(s)"].From_Number(Data);
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3D0C
void File_Mxf::GenericSoundEssenceDescriptor_DialNorm()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info(Data, " dB");
}

//---------------------------------------------------------------------------
// 0x4801
void File_Mxf::GenericTrack_TrackID()
{
    //Parsing
    int32u Data;
    Get_B4 (Data,                                                "Data"); Element_Info(Data);

    FILLING_BEGIN();
        if (Tracks[InstanceUID].TrackID==(int32u)-1)
            Tracks[InstanceUID].TrackID=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x4802
void File_Mxf::GenericTrack_TrackName()
{
    //Parsing
    Info_UTF16B(Length2, Data,                                  "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x4803
void File_Mxf::GenericTrack_Sequence()
{
    //Parsing
    int128u Data;
    Get_UUID(Data,                                              "Data"); Element_Info(Ztring::ToZtring(Data, 16));

    FILLING_BEGIN();
        Tracks[InstanceUID].Sequence=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x4804
void File_Mxf::GenericTrack_TrackNumber()
{
    //Parsing
    int32u Data;
    Get_B4 (Data,                                                "Data"); Element_Info(Ztring::ToZtring(Data, 16));

    FILLING_BEGIN();
        Tracks[InstanceUID].TrackNumber=Data;
        Track_Number_IsAvailable=true;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3C01
void File_Mxf::Identification_CompanyName()
{
    //Parsing
    Ztring Data;
    Get_UTF16B(Length2, Data,                                  "Data"); Element_Info(Data);

    FILLING_BEGIN();
        Identifications[InstanceUID].CompanyName=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3C02
void File_Mxf::Identification_ProductName()
{
    //Parsing
    Ztring Data;
    Get_UTF16B(Length2, Data,                                  "Data"); Element_Info(Data);

    FILLING_BEGIN();
        Identifications[InstanceUID].ProductName=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3C03
void File_Mxf::Identification_ProductVersion()
{
    //Parsing
    int16u Major, Minor, Patch, Build, Release;
    Get_B2 (Major,                                              "Major");
    Get_B2 (Minor,                                              "Minor");
    Get_B2 (Patch,                                              "Patch");
    Get_B2 (Build,                                              "Build");
    Get_B2 (Release,                                            "Release");
    Ztring Version=Ztring::ToZtring(Major)+_T('.')
                  +Ztring::ToZtring(Minor)+_T('.')
                  +Ztring::ToZtring(Patch)+_T('.')
                  +Ztring::ToZtring(Build)+_T('.')
                  +Ztring::ToZtring(Release)      ;
    Element_Info(Version);

    FILLING_BEGIN();
        Identifications[InstanceUID].ProductVersion=Version;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3C04
void File_Mxf::Identification_VersionString()
{
    //Parsing
    Ztring Data;
    Get_UTF16B(Length2, Data,                                  "Data"); Element_Info(Data);

    FILLING_BEGIN();
        Identifications[InstanceUID].VersionString=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3C05
void File_Mxf::Identification_ProductUID()
{
    //Parsing
    Skip_UUID(                                                  "UUID");
}

//---------------------------------------------------------------------------
// 0x3C06
void File_Mxf::Identification_ModificationDate()
{
    //Parsing
    Info_Timestamp();
}

//---------------------------------------------------------------------------
// 0x3C07
void File_Mxf::Identification_ToolkitVersion()
{
    //Parsing
    //Parsing
    Info_B2(Major,                                              "Major");
    Info_B2(Minor,                                              "Minor");
    Info_B2(Patch,                                              "Patch");
    Info_B2(Build,                                              "Build");
    Info_B2(Release,                                            "Release");
    Element_Info(Ztring::ToZtring(Major)+_T('.')
                +Ztring::ToZtring(Minor)+_T('.')
                +Ztring::ToZtring(Patch)+_T('.')
                +Ztring::ToZtring(Build)+_T('.')
                +Ztring::ToZtring(Release)      );
}

//---------------------------------------------------------------------------
// 0x3C08
void File_Mxf::Identification_Platform()
{
    //Parsing
    Info_UTF16B(Length2, Data,                                  "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3C09
void File_Mxf::Identification_ThisGenerationUID()
{
    //Parsing
    Skip_UUID(                                                  "UUID");
}

//---------------------------------------------------------------------------
// 0x3F05
void File_Mxf::IndexTableSegment_EditUnitByteCount()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3F06
void File_Mxf::IndexTableSegment_IndexSID()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3F07
void File_Mxf::IndexTableSegment_BodySID()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3F08
void File_Mxf::IndexTableSegment_SliceCount()
{
    //Parsing
    int8u Data;
    Get_B1(Data,                                                "Data"); Element_Info(Data);

    FILLING_BEGIN();
        IndexTable_NSL=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3F09
void File_Mxf::IndexTableSegment_DeltaEntryArray()
{
    //Parsing
    int32u NDE, Length;
    Get_B4(NDE,                                                 "NDE");
    Get_B4(Length,                                              "Length");
    for (int32u Pos=0; Pos<NDE; Pos++)
    {
        Element_Begin("Delta Entry");
        Skip_B1(                                                "PosTableIndex");    
        Skip_B1(                                                "Slice");    
        Skip_B4(                                                "Element Delta");
        Element_End();    
    }
}

//---------------------------------------------------------------------------
// 0x3F0A
void File_Mxf::IndexTableSegment_IndexEntryArray()
{
    //Parsing
    int32u NIE, Length;
    Get_B4(NIE,                                                 "NIE");
    Get_B4(Length,                                              "Length");
    for (int32u Pos=0; Pos<NIE; Pos++)
    {
        Element_Begin("Index Entry");
        Skip_B1(                                                "Temporal Offset");    
        Skip_B1(                                                "Key-Frame Offset");    
        Skip_B1(                                                "Flags");    
        Skip_B8(                                                "Stream Offset");    
        for (int32u NSL_Pos=0; NSL_Pos<IndexTable_NSL; NSL_Pos++)
            Skip_B4(                                            "SliceOffset");    
        for (int32u NPE_Pos=0; NPE_Pos<IndexTable_NPE; NPE_Pos++)
            Skip_B4(                                            "PosTable");    
        Element_End();    
    }
}

//---------------------------------------------------------------------------
// 0x3F0B
void File_Mxf::IndexTableSegment_IndexEditRate()
{
    //Parsing
    Info_Rational();
}

//---------------------------------------------------------------------------
// 0x3F0C
void File_Mxf::IndexTableSegment_IndexStartPosition()
{
    //Parsing
    Info_B8(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3F0D
void File_Mxf::IndexTableSegment_IndexDuration()
{
    //Parsing
    Info_B8(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3F0E
void File_Mxf::IndexTableSegment_PosTableCount()
{
    //Parsing
    int8u Data;
    Get_B1(Data,                                                "Data"); Element_Info(Data);

    FILLING_BEGIN();
        IndexTable_NPE=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x8001
void File_Mxf::JPEG2000PictureSubDescriptor_Rsiz()
{
    //Parsing
    Info_B2(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x8002
void File_Mxf::JPEG2000PictureSubDescriptor_Xsiz()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x8003
void File_Mxf::JPEG2000PictureSubDescriptor_Ysiz()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x8004
void File_Mxf::JPEG2000PictureSubDescriptor_XOsiz()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x8005
void File_Mxf::JPEG2000PictureSubDescriptor_YOsiz()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x8006
void File_Mxf::JPEG2000PictureSubDescriptor_XTsiz()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x8007
void File_Mxf::JPEG2000PictureSubDescriptor_YTsiz()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x8008
void File_Mxf::JPEG2000PictureSubDescriptor_XTOsiz()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x8009
void File_Mxf::JPEG2000PictureSubDescriptor_YTOsiz()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x800A
void File_Mxf::JPEG2000PictureSubDescriptor_Csiz()
{
    //Parsing
    Info_B2(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x800B
void File_Mxf::JPEG2000PictureSubDescriptor_PictureComponentSizing()
{
    //Parsing
    int32u Count, Length;
    Get_B4 (Count,                                              "Count");
    Get_B4 (Length,                                             "Length");
    for (int32u Pos=0; Pos<Count; Pos++)
    {
        Element_Begin("PictureComponentSize", Length);
        Info_B1(Ssiz,                                           "Component sample precision"); Element_Info(Ssiz);
        Info_B1(XRsiz,                                          "Horizontal separation of a sample"); Element_Info(XRsiz);
        Info_B1(YRsiz,                                          "Vertical separation of a sample"); Element_Info(YRsiz);
        Element_End();
    }
}

//---------------------------------------------------------------------------
// 0x3B02
void File_Mxf::Preface_LastModifiedDate()
{
    //Parsing
    Ztring Value;
    Get_Timestamp(Value); Element_Info(Value);

    FILLING_BEGIN();
        Fill(Stream_General, 0, General_Encoded_Date, Value, true);
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x
void File_Mxf::MPEG2VideoDescriptor_SingleSequence()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info(Data?"Yes":"No");
}

//---------------------------------------------------------------------------
// 0x
void File_Mxf::MPEG2VideoDescriptor_ConstantBFrames()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info(Data?"Yes":"No");
}

//---------------------------------------------------------------------------
// 0x
void File_Mxf::MPEG2VideoDescriptor_CodedContentType()
{
    //Parsing
    int8u Data;
    Get_B1 (Data,                                               "Data"); Element_Info(Mxf_MPEG2_CodedContentType(Data));

    FILLING_BEGIN();
        Descriptors[InstanceUID].Infos["ScanType"]=Mxf_MPEG2_CodedContentType(Data);
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x
void File_Mxf::MPEG2VideoDescriptor_LowDelay()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info(Data?"Yes":"No");
}

//---------------------------------------------------------------------------
// 0x
void File_Mxf::MPEG2VideoDescriptor_ClosedGOP()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info(Data?"Yes":"No");
}

//---------------------------------------------------------------------------
// 0x
void File_Mxf::MPEG2VideoDescriptor_IdenticalGOP()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info(Data?"Yes":"No");
}

//---------------------------------------------------------------------------
// 0x
void File_Mxf::MPEG2VideoDescriptor_MaxGOP()
{
    //Parsing
    Info_B2(Data,                                               "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x
void File_Mxf::MPEG2VideoDescriptor_BPictureCount()
{
    //Parsing
    Info_B2(Data,                                               "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x
void File_Mxf::MPEG2VideoDescriptor_ProfileAndLevel()
{
    //Parsing
    int8u profile_and_level_indication_profile, profile_and_level_indication_level;
    BS_Begin();
    Skip_SB(                                                    "profile_and_level_indication_escape");
    Get_S1 ( 3, profile_and_level_indication_profile,           "profile_and_level_indication_profile"); Param_Info(Mpegv_profile_and_level_indication_profile[profile_and_level_indication_profile]);
    Get_S1 ( 4, profile_and_level_indication_level,             "profile_and_level_indication_level"); Param_Info(Mpegv_profile_and_level_indication_level[profile_and_level_indication_level]);
    BS_End();

    FILLING_BEGIN();
        if (profile_and_level_indication_profile && profile_and_level_indication_level)
            Descriptors[InstanceUID].Infos["Format_Profile"]=Ztring().From_Local(Mpegv_profile_and_level_indication_profile[profile_and_level_indication_profile])+_T("@")+Ztring().From_Local(Mpegv_profile_and_level_indication_level[profile_and_level_indication_level]);
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x
void File_Mxf::MPEG2VideoDescriptor_BitRate()
{
    //Parsing
    int32u Data;
    Get_B4 (Data,                                               "Data"); Element_Info(Data);

    FILLING_BEGIN();
        Descriptors[InstanceUID].Infos["BitRate"].From_Number(Data);
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x4001
void File_Mxf::NetworkLocator_URLString()
{
    //Parsing
    Ztring Data;
    Get_UTF16B(Length2, Data,                                   "Essence Locator"); Element_Info(Data);

    FILLING_BEGIN();
        Locators[InstanceUID].EssenceLocator=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3F01
void File_Mxf::MultipleDescriptor_SubDescriptorUIDs()
{
    //Parsing
    //Vector
    int32u Count, Length;
    Get_B4 (Count,                                              "Count");
    Get_B4 (Length,                                             "Length");
    for (int32u Pos=0; Pos<Count; Pos++)
    {
        //Parsing
        int128u Data;
        Get_UUID(Data,                                          "UUID");

        FILLING_BEGIN();
            Descriptors[InstanceUID].SubDescriptors.push_back(Data);
        FILLING_END();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::PartitionMetadata()
{
    //Parsing
    Skip_B2(                                                    "MajorVersion");
    Skip_B2(                                                    "MinorVersion");
    Skip_B4(                                                    "KAGSize");
    Skip_B8(                                                    "ThisPartition");
    Skip_B8(                                                    "PreviousPartition");
    Skip_B8(                                                    "FooterPartition");
    Skip_B8(                                                    "HeaderByteCount");
    Skip_B8(                                                    "IndexByteCount");
    Skip_B4(                                                    "IndexSID");
    Skip_B8(                                                    "BodyOffset");
    Skip_B4(                                                    "BodySID");
    Get_UL (OperationalPattern,                                 "OperationalPattern", Mxf_OperationalPattern);

    Element_Begin("EssenceContainers"); //Vector
        int32u Count, Length;
        Get_B4 (Count,                                          "Count");
        Get_B4 (Length,                                         "Length");
        for (int32u Pos=0; Pos<Count; Pos++)
        {
            Info_UL(EssenceContainer,                           "EssenceContainer", Mxf_EssenceContainer);
        }
    Element_End();
}

//---------------------------------------------------------------------------
// 0x3B03
void File_Mxf::Preface_ContentStorage()
{
    //Parsing
    int128u Data;
    Get_UUID(Data,                                              "Data"); Element_Info(Ztring().From_UUID(Data));

    FILLING_BEGIN();
        Prefaces[Preface_Current].ContentStorage=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3B05
void File_Mxf::Preface_Version()
{
    //Parsing
    Info_B1(Major,                                              "Major"); //1
    Info_B1(Minor,                                              "Minor"); //2
    Element_Info(Ztring::ToZtring(Major)+_T('.')+Ztring::ToZtring(Minor));
}

//---------------------------------------------------------------------------
// 0x3B06
void File_Mxf::Preface_Identifications()
{
    //Parsing
    //Vector
    int32u Count, Length;
    Get_B4 (Count,                                              "Count");
    Get_B4 (Length,                                             "Length");
    for (int32u Pos=0; Pos<Count; Pos++)
    {
        Element_Begin("Identification", Length);
        int128u Data;
        Get_UUID(Data,                                          "UUID"); Element_Info(Ztring().From_UUID(Data));
        Element_End();

        FILLING_BEGIN();
            Prefaces[Preface_Current].Identifications.push_back(Data);
        FILLING_END();
    }
}

//---------------------------------------------------------------------------
// 0x3B07
void File_Mxf::Preface_ObjectModelVersion()
{
    //Parsing
    Skip_B4(                                                    "Data");
}

//---------------------------------------------------------------------------
// 0x3B08
void File_Mxf::Preface_PrimaryPackage()
{
    //Parsing
    int128u Data;
    Get_UUID(Data,                                              "Data");

    FILLING_BEGIN();
        Prefaces[Preface_Current].PrimaryPackage=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3B09
void File_Mxf::Preface_OperationalPattern()
{
    //Parsing
    Get_UL (OperationalPattern,                                 "UUID", Mxf_OperationalPattern); Element_Info(Mxf_OperationalPattern(OperationalPattern));
}

//---------------------------------------------------------------------------
// 0x3B0A
void File_Mxf::Preface_EssenceContainers()
{
    //Parsing
    //Vector
    int32u Count, Length;
    Get_B4 (Count,                                              "Count");
    Get_B4 (Length,                                             "Length");
    for (int32u Pos=0; Pos<Count; Pos++)
    {
        Info_UL(EssenceContainer,                               "EssenceContainer", Mxf_EssenceContainer);
    }
}

//---------------------------------------------------------------------------
// 0x3B0B
void File_Mxf::Preface_DMSchemes()
{
    //Parsing
    //Vector
    int32u Count, Length;
    Get_B4 (Count,                                              "Count");
    Get_B4 (Length,                                             "Length");
    for (int32u Pos=0; Pos<Count; Pos++)
        if (Length==16)
        {
            Info_UL(Data,                                       "DMScheme", NULL); Element_Info(Ztring().From_UUID(Data));
        }
        else
            Skip_XX(Length,                                     "DMScheme");
}

//---------------------------------------------------------------------------
// 0x3401
void File_Mxf::RGBAEssenceDescriptor_PixelLayout()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3403
void File_Mxf::RGBAEssenceDescriptor_Palette()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3404
void File_Mxf::RGBAEssenceDescriptor_PaletteLayout()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3405
void File_Mxf::RGBAEssenceDescriptor_ScanningDirection()
{
    //Parsing
    Info_B1(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3406
void File_Mxf::RGBAEssenceDescriptor_ComponentMaxRef()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3407
void File_Mxf::RGBAEssenceDescriptor_ComponentMinRef()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3408
void File_Mxf::RGBAEssenceDescriptor_AlphaMaxRef()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3409
void File_Mxf::RGBAEssenceDescriptor_AlphaMinRef()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x1001
void File_Mxf::Sequence_StructuralComponents()
{
    //Parsing
    //Vector
    int32u Count, Length;
    Get_B4 (Count,                                              "Count");
    Get_B4 (Length,                                             "Length");
    for (int32u Pos=0; Pos<Count; Pos++)
    {
        int128u Data;
        Get_UUID (Data,                                         "StructuralComponent");

        FILLING_BEGIN();
            Components[InstanceUID].StructuralComponents.push_back(Data);
        FILLING_END();
    }
}

//---------------------------------------------------------------------------
// 0x1101
void File_Mxf::SourceClip_SourcePackageID()
{
    //Parsing
    int256u Data;
    Get_UMID(Data,                                              "SourcePackageID");

    FILLING_BEGIN();
        Components[InstanceUID].SourcePackageID=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x1102
void File_Mxf::SourceClip_SourceTrackID()
{
    //Parsing
    Info_B4(Data,                                                "SourceTrackID"); Element_Info(Data);

    FILLING_BEGIN();
        if (Components[InstanceUID].SourceTrackID==(int32u)-1)
            Components[InstanceUID].SourceTrackID=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x1201
void File_Mxf::SourceClip_StartPosition()
{
    //Parsing
    Info_B8(Data,                                               "StartPosition"); Element_Info(Data); //units of edit rate
}

//---------------------------------------------------------------------------
// 0x4701
void File_Mxf::SourcePackage_Descriptor()
{
    //Parsing
    int128u Data;
    Get_UUID(Data,                                              "Data"); Element_Info(Ztring().From_UUID(Data));

    FILLING_BEGIN();
        Packages[InstanceUID].Descriptor=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x0201
void File_Mxf::StructuralComponent_DataDefinition()
{
    //Parsing
    Info_UL(Data,                                               "Data", Mxf_Sequence_DataDefinition); Element_Info(Mxf_Sequence_DataDefinition(Data));
}

//---------------------------------------------------------------------------
// 0x0202
void File_Mxf::StructuralComponent_Duration()
{
    //Parsing
    int64u Data;
    Get_B8 (Data,                                               "Data"); Element_Info(Data); //units of edit rate

    FILLING_BEGIN();
        if (Data!=0xFFFFFFFFFFFFFFFFLL)
            Components[InstanceUID].Duration=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x0102
void File_Mxf::SystemScheme1_TimeCodeArray()
{
    //Parsing
    //Vector
    int32u Count, Length;
    Get_B4 (Count,                                              "Count");
    Get_B4 (Length,                                             "Length");
    for (int32u Pos=0; Pos<Count; Pos++)
    {
        Element_Begin("TimeCode");
        int8u Frames_Units, Frames_Tens, Seconds_Units, Seconds_Tens, Minutes_Units, Minutes_Tens, Hours_Units, Hours_Tens;
        bool  DropFrame;
        BS_Begin();

        Skip_SB(                                                "CF - Color fame");
        Get_SB (   DropFrame,                                   "DP - Drop frame");
        Get_S1 (2, Frames_Tens,                                 "Frames (Tens)");
        Get_S1 (4, Frames_Units,                                "Frames (Units)");

        Skip_SB(                                                "FP - Field Phase / BGF0");
        Get_S1 (3, Seconds_Tens,                                "Seconds (Tens)");
        Get_S1 (4, Seconds_Units,                               "Seconds (Units)");

        Skip_SB(                                                "BGF0 / BGF2");
        Get_S1 (3, Minutes_Tens,                                "Minutes (Tens)");
        Get_S1 (4, Minutes_Units,                               "Minutes (Units)");

        Skip_SB(                                                "BGF2 / Field Phase");
        Skip_SB(                                                "BGF1");
        Get_S1 (2, Hours_Tens,                                  "Hours (Tens)");
        Get_S1 (4, Hours_Units,                                 "Hours (Units)");

        Skip_S1(4,                                              "BG2");
        Skip_S1(4,                                              "BG1");

        Skip_S1(4,                                              "BG4");
        Skip_S1(4,                                              "BG3");

        Skip_S1(4,                                              "BG6");
        Skip_S1(4,                                              "BG5");

        Skip_S1(4,                                              "BG8");
        Skip_S1(4,                                              "BG7");

        BS_End();

        int64u TimeCode=(int64u)(Hours_Tens     *10*60*60*1000
                               + Hours_Units       *60*60*1000
                               + Minutes_Tens      *10*60*1000
                               + Minutes_Units        *60*1000
                               + Seconds_Tens         *10*1000
                               + Seconds_Units           *1000
                               + (SystemScheme1_FrameRateFromDescriptor?float64_int32s((Frames_Tens*10+Frames_Units)*1000/SystemScheme1_FrameRateFromDescriptor):0));

        Element_Info(Ztring().Duration_From_Milliseconds(TimeCode));

        Element_End();

        //TimeCode
        if (SystemScheme1_TimeCodeArray_StartTimecode==(int64u)-1)
            SystemScheme1_TimeCodeArray_StartTimecode=TimeCode;
    }
}

//---------------------------------------------------------------------------
// 0x4101
void File_Mxf::TextLocator_LocatorName()
{
    //Parsing
    Info_UTF16B(Length2, Data,                                  "Data"); Element_Info(Data);

    FILLING_BEGIN();
        Locators[InstanceUID].EssenceLocator=Data;
        Locators[InstanceUID].IsTextLocator=true;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x1501
void File_Mxf::TimecodeComponent_StartTimecode()
{
    //Parsing
    int64u Data;
    Get_B8 (Data,                                                "Data"); Element_Info(Data);

    FILLING_BEGIN();
        if (Data!=0xFFFFFFFFFFFFFFFFLL && TimeCode_StartTimecode==(int64u)-1) //Using first TimeCode
            TimeCode_StartTimecode=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x1502
void File_Mxf::TimecodeComponent_RoundedTimecodeBase()
{
    //Parsing
    int16u Data;
    Get_B2 (Data,                                                "Data"); Element_Info(Data);

    FILLING_BEGIN();
        if (Data!=0xFFFFFFFFFFFFFFFFLL)
            TimeCode_RoundedTimecodeBase=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x1503
void File_Mxf::TimecodeComponent_DropFrame()
{
    //Parsing
    int8u Data;
    Get_B1 (Data,                                                "Data"); Element_Info(Data);

    FILLING_BEGIN();
        if (Data!=0xFFFFFFFFFFFFFFFFLL)
            TimeCode_DropFrame=Data?true:false;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x4B01
void File_Mxf::Track_EditRate()
{
    //Parsing
    float64 Data;
    Get_Rational(Data); Element_Info(Data);

    FILLING_BEGIN();
        Tracks[InstanceUID].EditRate=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x4B02
void File_Mxf::Track_Origin()
{
    //Parsing
    Info_B8(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3D09
void File_Mxf::WaveAudioDescriptor_AvgBps()
{
    //Parsing
    int32u Data;
    Get_B4 (Data,                                               "Data"); Element_Info(Data);

    FILLING_BEGIN();
        Descriptors[InstanceUID].Infos["BitRate"].From_Number(Data*8);
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3D0A
void File_Mxf::WaveAudioDescriptor_BlockAlign()
{
    //Parsing
    Info_B2(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3D0B
void File_Mxf::WaveAudioDescriptor_SequenceOffset()
{
    //Parsing
    Info_B1(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3D29
void File_Mxf::WaveAudioDescriptor_PeakEnvelopeVersion()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3D2A
void File_Mxf::WaveAudioDescriptor_PeakEnvelopeFormat()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3D2B
void File_Mxf::WaveAudioDescriptor_PointsPerPeakValue()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3D2C
void File_Mxf::WaveAudioDescriptor_PeakEnvelopeBlockSize()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3D2D
void File_Mxf::WaveAudioDescriptor_PeakChannels()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3D2E
void File_Mxf::WaveAudioDescriptor_PeakFrames()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3D2F
void File_Mxf::WaveAudioDescriptor_PeakOfPeaksPosition()
{
    //Parsing
    Info_B8(Data,                                                "Data"); Element_Info(Data);
}

//---------------------------------------------------------------------------
// 0x3D30
void File_Mxf::WaveAudioDescriptor_PeakEnvelopeTimestamp()
{
    //Parsing
    Info_Timestamp();
}

//---------------------------------------------------------------------------
// 0x3D31
void File_Mxf::WaveAudioDescriptor_PeakEnvelopeData()
{
    //Parsing
    Skip_XX(Length2,                                            "Data");
}

//---------------------------------------------------------------------------
// 0x3D32
void File_Mxf::WaveAudioDescriptor_ChannelAssignment()
{
    //Parsing
    Skip_B16(                                                   "Label");
}

//---------------------------------------------------------------------------
// 0x8001
void File_Mxf::Omneon_010201010100_8001()
{
    //Parsing
    //Vector
    int32u Count, Length;
    Get_B4 (Count,                                              "Count");
    Get_B4 (Length,                                             "Length");
    for (int32u Pos=0; Pos<Count; Pos++)
    {
        Skip_UUID(                                              "UUID Omneon_010201020100");
    }
}

//---------------------------------------------------------------------------
// 0x8001
void File_Mxf::Omneon_010201010100_8003()
{
    //Parsing
    //Vector
    int32u Count, Length;
    Get_B4 (Count,                                              "Count");
    Get_B4 (Length,                                             "Length");
    for (int32u Pos=0; Pos<Count; Pos++)
    {
        Skip_UUID(                                              "UUID Omneon_010201020100");
    }
}

//---------------------------------------------------------------------------
// 0x8003
void File_Mxf::Omneon_010201020100_8002()
{
    //Parsing
    Skip_UTF16B(Length2,                                        "Content");
}

//---------------------------------------------------------------------------
// 0x8003
void File_Mxf::Omneon_010201020100_8003()
{
    //Parsing
    Skip_UTF16B(Length2,                                        "Content");
}

//---------------------------------------------------------------------------
// 0x8004
void File_Mxf::Omneon_010201020100_8004()
{
    //Parsing
    Skip_XX(Length2,                                            "Unknown");
}

//---------------------------------------------------------------------------
// 0x8005
void File_Mxf::Omneon_010201020100_8005()
{
    //Parsing
    Skip_UTF16B(Length2,                                        "Content");
}

//---------------------------------------------------------------------------
// 0x8006
void File_Mxf::Omneon_010201020100_8006()
{
    //Parsing
    Skip_Local(Length2,                                         "Content");
}

//***************************************************************************
// Basic types
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mxf::Get_Rational(float64 &Value)
{
    //Parsing
    int32u N, D;
    Get_B4 (N,                                                  "Numerator");
    Get_B4 (D,                                                  "Denominator");
    if (D)
        Value=((float32)N)/D;
    else
        Value=0; //Error
}

//---------------------------------------------------------------------------
void File_Mxf::Skip_Rational()
{
    //Parsing
    Skip_B4(                                                    "Numerator");
    Skip_B4(                                                    "Denominator");
}

//---------------------------------------------------------------------------
void File_Mxf::Info_Rational()
{
    //Parsing
    Info_B4(N,                                                  "Numerator");
    Info_B4(D,                                                  "Denominator");
    if (D)
        Element_Info(((float32)N)/D);
}

//---------------------------------------------------------------------------
void File_Mxf::Get_UL(int128u &Value, const char* Name, const char* (*Param) (int128u))
{
    #ifdef MEDIAINFO_MINIMIZE_SIZE
        Skip_UUID();
    #else
    //Parsing
    Element_Begin(Name);
    int64u Value_hi, Value_lo;
    Peek_B8(Value_hi);
    Skip_B1(                                                    "Start (0x06)");
    Skip_B1(                                                    "Length of the remaining key (0x0E)");
    Skip_B1(                                                    "ISO, ORG (0x2B)");
    Skip_B1(                                                    "SMPTE (0x34)");
    Info_B1(Category,                                           "Category"); Param_Info(Mxf_Category(Category));
    Info_B1(Registry,                                           "Registry"); Param_Info(Mxf_Registry(Category, Registry));
    Info_B1(Structure,                                          "Structure");
    Skip_B1(                                                    "Version");
    Peek_B8(Value_lo);
    switch (Category)
    {
        case 0x01 : //Elements
                    {
                    switch (Registry)
                    {
                        default   :
                                    {
                                    switch (Structure)
                                    {
                                        case 0x01 : //Standard
                                                    Param_Info("Essence element"); //SMPTE 379M
                                                    Info_UL_01xx01_Essences();
                                                    break;
                                        default   :
                                                    Skip_B8(    "Unknown");
                                    }
                                    }
                    }
                    }
                    break;
        case 0x02 :
                    {
                    switch (Registry)
                    {
                        default   :
                                    {
                                    switch (Structure)
                                    {
                                        case 0x01 :
                                                    Info_UL_02xx01();
                                                    break;
                                        default   :
                                                    Skip_B8(    "Unknown");
                                    }
                                    }
                    }
                    }
                    break;
        case 0x04 : //Labels
                    {
                    switch (Registry)
                    {
                        case 0x01 :
                                    {
                                    Param_Info("Labels");
                                    switch (Structure)
                                    {
                                        case 0x01 :
                                                    Info_UL_040101_Labels();
                                                    break;
                                        default   :
                                                    Skip_B8(   "Unknown");
                                    }
                                    }
                                    break;
                        default   :
                                    Skip_B8(                    "Unknown");
                    }
                    }
                    break;
        default   :
                    Skip_B8(                                    "Unknown");
    }


    Value.hi=Value_hi;
    Value.lo=Value_lo;
    if (Param)
        Element_Info(Param(Value));
    Element_End();
    #endif
}

//---------------------------------------------------------------------------
void File_Mxf::Info_UL_01xx01_Essences()
{
    Info_B1(Code1,                                              "Item Designator");
    switch (Code1)
    {
        case 0x03 :
            {
            Param_Info("Interpretive");
            Info_B1(Code2,                                      "Code (2)");
            switch (Code2)
            {
                case 0x01 :
                    {
                    Param_Info("Fundamental");
                    Info_B1(Code3,                              "Code (3)");
                    switch (Code3)
                    {
                        case 0x02 :
                            {
                            Param_Info("Data Interpretations and Definitions");
                            Info_B1(Code4,                      "Code (4)");
                            switch (Code4)
                            {
                                case 0x20 :
                                    {
                                    Param_Info("XML Constructs and Interpretations");
                                    Info_B1(Code5,              "Code (5)");
                                    switch (Code5)
                                    {
                                        case 0x01 :
                                            {
                                            Param_Info("XML Document Text");
                                            Info_B1(Code6,      "Unknown");
                                            Info_B1(Code7,      "Unknown");
                                            Info_B1(Code8,      "Unknown");
                                            }
                                            break;
                                       default   :
                                            Skip_B3(            "Unknown");
                                    }
                                    }
                                    break;
                                default   :
                                    Skip_B4(                    "Unknown");
                            }
                            }
                            break;
                        default   :
                            Skip_B5(                            "Unknown");
                    }
                    }
                    break;
                default   :
                    Skip_B6(                                    "Unknown");
            }
            }
            break;
        case 0x0D :
            {
            Param_Info("User Organisation Registered For Public Use");
            Info_B1(Code2,                                      "Organization");
            switch (Code2)
            {
                case 0x01 :
                    {
                    Param_Info("AAF");
                    Info_B1(Code3,                              "Application");
                    switch (Code3)
                    {
                        case 0x03 :
                            {
                            Param_Info("MXF Generic Container Keys");
                            Info_B1(Code4,                      "Structure Version");
                            switch (Code4)
                            {
                                case 0x01 :
                                    {
                                    Param_Info("MXF-GC Version 1");
                                    Info_B1(Code5,              "Item Type Identifier");
                                    switch (Code5)
                                    {
                                        case 0x05 : Param_Info("CP Picture (SMPTE 326M)"); break;
                                        case 0x06 : Param_Info("CP Sound (SMPTE 326M)"); break;
                                        case 0x07 : Param_Info("CP Data (SMPTE 326M)"); break;
                                        case 0x15 : Param_Info("GC Picture"); break;
                                        case 0x16 : Param_Info("GC Sound"); break;
                                        case 0x17 : Param_Info("GC Data"); break;
                                        case 0x18 : Param_Info("GC Compound"); break;
                                        default   : ;
                                    }
                                    Info_B1(Code6,              "Essence Element Count");
                                    Info_B1(Code7,              "Essence Element Type");
                                    Info_B1(Code8,              "Essence Element Number");
                                    }
                                    break;
                                default   :
                                    Skip_B4(                    "Unknown");
                            }
                            }
                            break;
                        default   :
                            Skip_B5(                            "Unknown");
                    }
                    }
                    break;
                default   :
                    Skip_B6(                                    "Unknown");
            }
            }
            break;
        default   :
            Skip_B7(                                            "Unknown");
    }
}

//---------------------------------------------------------------------------
void File_Mxf::Info_UL_02xx01()
{
    Info_B1(Code1,                                              "Item Designator");
    switch (Code1)
    {
        case 0x0D :
            {
            Param_Info("User Organisation Registered For Public Use");
            Info_B1(Code2,                                      "Organization");
            switch (Code2)
            {
                case 0x01 :
                    {
                    Param_Info("AAF");
                    Info_B1(Code3,                              "Application");
                    switch (Code3)
                    {
                        case 0x01 :
                            {
                            Param_Info("Structural Metadata Sets");
                            Info_B1(Code4,                      "Structure Version");
                            switch (Code4)
                            {
                                case 0x01 :
                                    {
                                    Param_Info("Version 1");
                                    Info_B1(Code5,              "Structure Kind");
                                    switch (Code5)
                                    {
                                        case 0x01 :
                                            {
                                            Param_Info("MXF / AAF Association compatible sets & packs");
                                            Info_B1(Code6,      "Set Kind (1)"); //See table 14
                                            Info_B1(Code7,      "Set Kind (2)"); //See table 14
                                            Info_B1(Code8,      "Reserved");
                                            }
                                            break;
                                       default   :
                                            Skip_B3(            "Unknown");
                                    }
                                    }
                                    break;
                                default   :
                                    Skip_B4(                    "Unknown");
                            }
                            }
                            break;
                        case 0x02 :
                            {
                            Param_Info("MXF File Structure");
                            Info_B1(Code4,                      "Structure Version");
                            switch (Code4)
                            {
                                case 0x01 :
                                    {
                                    Param_Info("Version 1");
                                    Info_B1(Code5,              "Structure Kind");
                                    switch (Code5)
                                    {
                                        case 0x01 :
                                            {
                                            Param_Info("MXF File Structure sets & packs");
                                            Info_B1(Code6,      "Set / Pack Kind");
                                            switch (Code6)
                                            {
                                                case 0x02 :
                                                    {
                                                    Param_Info("Header Partition");
                                                    Info_B1(Code7, "Partition Status");
                                                    Info_B1(Code8, "Reserved");
                                                    }
                                                    break;
                                                case 0x03 :
                                                    {
                                                    Param_Info("Body Partition");
                                                    Info_B1(Code7, "Partition Status");
                                                    Info_B1(Code8, "Reserved");
                                                    }
                                                    break;
                                                case 0x04 :
                                                    {
                                                    Param_Info("Footer Partition");
                                                    Info_B1(Code7, "Partition Status");
                                                    Info_B1(Code8, "Reserved");
                                                    }
                                                    break;
                                                case 0x05 :
                                                    {
                                                    Param_Info("Primer");
                                                    Info_B1(Code7, "Version of the Primer Pack");
                                                    Info_B1(Code8, "Reserved");
                                                    }
                                                    break;
                                                case 0x10 :
                                                    Param_Info("Index Table Segment");
                                                    Skip_B1(    "Version");
                                                    Skip_B1(    "Reserved");
                                                    break;
                                                case 0x11 :
                                                    Param_Info("Random Index Pack");
                                                    Skip_B1(    "Version");
                                                    Skip_B1(    "Reserved");
                                                    break;
                                                default   :
                                                    Skip_B3(    "Unknown");
                                            }
                                            }
                                            break;
                                       default   :
                                            Skip_B3(            "Unknown");
                                    }
                                    }
                                    break;
                                default   :
                                    Skip_B4(                    "Unknown");
                            }
                            }
                            break;
                        case 0x03 :
                            {
                            Param_Info("MXF Generic Container Keys");
                            Info_B1(Code4,                      "Structure Version");
                            switch (Code4)
                            {
                                case 0x01 :
                                    {
                                    Param_Info("MXF-GC Version 1");
                                    Info_B1(Code5,              "Item Type Identifier");
                                    switch (Code5)
                                    {
                                        case 0x04 :
                                            {
                                            Param_Info("CP-Compatible System Item"); //SMPTE 379M
                                            Info_B1(Code6,      "System Scheme Identifier");
                                            switch (Code6)
                                            {
                                                case 0x02 :
                                                    {
                                                    Param_Info("SDTI-CP, version 1"); //SMPTE 385M
                                                    Info_B1(Code7, "Metadata or Control Element Identifier");
                                                    switch (Code7)
                                                    {
                                                        case 0x01 :
                                                            {
                                                            Param_Info("System Metadata Pack");
                                                            Info_B1(Code8, "Reserved");
                                                            }
                                                            break;
                                                        case 0x02 :
                                                            {
                                                            Param_Info("Package metadata set");
                                                            Info_B1(Code8, "Metadata Block Count");
                                                            }
                                                            break;
                                                        case 0x03 :
                                                            {
                                                            Param_Info("Picture metadata set");
                                                            Info_B1(Code8, "Metadata Block Count");
                                                            }
                                                            break;
                                                        case 0x04 :
                                                            {
                                                            Param_Info("Sound metadata set");
                                                            Info_B1(Code8, "Metadata Block Count");
                                                            }
                                                            break;
                                                        case 0x05 :
                                                            {
                                                            Param_Info("Data metadata set");
                                                            Info_B1(Code8, "Metadata Block Count");
                                                            }
                                                            break;
                                                        case 0x06 :
                                                            {
                                                            Param_Info("Control data set");
                                                            Info_B1(Code8, "Metadata Block Count");
                                                            }
                                                            break;
                                                        default   :
                                                            Info_B1(Code8, "Metadata Block Count");
                                                    }
                                                    }
                                                    break;
                                                default   :
                                                    Info_B1(Code7,      "Metadata or Control Element Identifier");
                                                    Info_B1(Code8,      "Reserved");
                                            }
                                            }
                                            break;
                                        case 0x14 :
                                            {
                                            Param_Info("GC-Compatible System Item"); //SMPTE 379M
                                            Info_B1(Code6,      "System Scheme Identifier");
                                            switch (Code6)
                                            {
                                                case 0x02 :
                                                    {
                                                    Param_Info("GC System Scheme 1"); //SMPTE 394M
                                                    Info_B1(Code7, "Metadata or Control Element Identifier");
                                                    switch (Code7)
                                                    {
                                                        case 0x01 :
                                                            Param_Info("First Element");
                                                            break;
                                                        case 0x02 :
                                                            Param_Info("Subsequent Element");
                                                            break;
                                                        case 0x03 :
                                                            Param_Info("Picture Item Descriptor");
                                                            break;
                                                        case 0x04 :
                                                            Param_Info("Sound Item Descriptor");
                                                            break;
                                                        case 0x05 :
                                                            Param_Info("Data Item Descriptor");
                                                            break;
                                                        case 0x06 :
                                                            Param_Info("Control Item Descriptor");
                                                            break;
                                                        case 0x07 :
                                                            Param_Info("Compound Item Descriptor");
                                                            break;
                                                        default   : if (Code7>=0x10 && Code7<=0x7F) Param_Info("Pack coded System Elements (SMPTE 336M)");
                                                    }
                                                    Info_B1(Code8, "Element Number");
                                                    }
                                                    break;
                                                default   :
                                                    Info_B1(Code7,      "Metadata or Control Element Identifier");
                                                    Info_B1(Code8,      "Unknown");
                                            }
                                            }
                                            break;
                                       default   :
                                            Skip_B3(            "Unknown");
                                    }
                                    }
                                    break;
                                default   :
                                    Skip_B4(                    "Unknown");
                            }
                            }
                            break;
                        case 0x04 :
                            {
                            Param_Info("MXF / AAF Descriptive Metadata sets");
                            Info_B1(Code4,                      "Structure Version");
                            switch (Code4)
                            {
                                case 0x01 :
                                    {
                                    Param_Info("Version 1");
                                    Info_B1(Code5,              "Structure / Scheme Kind");
                                    Info_B1(Code6,              "Reserved");
                                    Info_B1(Code7,              "Reserved");
                                    Info_B1(Code8,              "Reserved");
                                    }
                                    break;
                                default   :
                                    Skip_B4(                    "Unknown");
                            }
                            }
                            break;
                        default   :
                            Skip_B5(                            "Unknown");
                    }
                    }
                    break;
                default   :
                    Skip_B6(                                    "Unknown");
            }
            }
            break;
        case 0x0E :
            {
            Param_Info("User Organisation Registered For Private Use");
            Skip_B7(                                            "Private");
            break;
            }
        default   :
            Skip_B7(                                            "Unknown");
    }
}

//---------------------------------------------------------------------------
void File_Mxf::Info_UL_040101_Labels()
{
    Info_B1(Code1,                                              "Item Designator");
    switch (Code1)
    {
        case 0x01 :
            {
            Param_Info("Interpretive");
            Info_B1(Code2,                                      "Code (2)");
            switch (Code2)
            {
                case 0x03 :
                    {
                    Info_B1(Code3,                              "Code (3)");
                    switch (Code3)
                    {
                        case 0x02 :
                            Skip_B5(                            "Track identifier");
                            break;
                        default   :
                            Skip_B5(                            "Unknown");
                    }
                    }
                    break;
                default   :
                    Skip_B6(                                    "Unknown");
            }
            }
            break;
        case 0x04 :
            {
            Param_Info("Parametric");
            Info_B1(Code2,                                      "Code (2)");
            switch (Code2)
            {
                case 0x01 :
                    {
                    Param_Info("Picture essence");
                    Info_B1(Code3,                              "Code (3)");
                    switch (Code3)
                    {
                        case 0x01 :
                            Param_Info("Fundamental Picture Characteristics");
                            Skip_B5(                            "Picture coding or compression");
                            break;
                        case 0x02 :
                            Param_Info("Picture Coding Characteristics");
                            Info_B1(Code4,                      "Code (4)");
                            switch (Code4)
                            {
                                case 0x01 :
                                    Param_Info("Uncompressed Picture Coding");
                                    Skip_B1(                    "Item Type Identifier"); //if 0x14: SMPTE 384M Uncompressed picture Line wrapped
                                    Skip_B1(                    "System Scheme Identifier"); //SMPTE 384M
                                    Skip_B1(                    "System Element Identifier"); //SMPTE 384M
                                    Skip_B1(                    "Reserved");
                                    break;
                                case 0x02 :
                                    {
                                    Param_Info("Compressed Picture Coding");
                                    Info_B1(Code5,              "Code (5)");
                                    switch (Code5)
                                    {
                                        case 0x01 :
                                            {
                                            Param_Info("MPEG Compression");
                                            Info_B1(Code6,      "Code (6)");
                                            switch (Code6)
                                            {
                                                case 0x01 :
                                                    Param_Info("MPEG-2 MP@ML");
                                                    Skip_B2(            "Unknown");
                                                    break;
                                                case 0x02 :
                                                    Param_Info("MPEG-2 422P@ML");
                                                    Skip_B2(            "Unknown");
                                                    break;
                                                case 0x03 :
                                                    Param_Info("MPEG-2 MP@HL");
                                                    Skip_B2(            "Unknown");
                                                    break;
                                                case 0x04 :
                                                    Param_Info("MPEG-2 422P@HL");
                                                    Skip_B2(            "Unknown");
                                                    break;
                                                case 0x10 :
                                                    Param_Info("MPEG-1");
                                                    Skip_B2(            "Unknown");
                                                    break;
                                                case 0x20 :
                                                    Param_Info("MPEG-4 Visual");
                                                    Skip_B2(            "Unknown");
                                                    break;
                                                default   :
                                                    Skip_B2(            "Unknown");
                                            }
                                            }
                                            break;
                                        case 0x02 :
                                            {
                                            Param_Info("DV Video Compression");
                                            Info_B1(Code6,      "Code (6)");
                                            switch (Code6)
                                            {
                                                case 0x02 :
                                                    Param_Info("DV-Based Compression");
                                                    Info_B1(Code7,      "DV type (SMPTE 383)");
                                                    Info_B1(Code8,      "Mapping Kind"); Param_Info(Mxf_EssenceContainer_Mapping(Code6, Code7, Code8));
                                                    break;
                                                default   :
                                                    Skip_B2(            "Unknown");
                                            }
                                            }
                                            break;
                                        default   :
                                            Skip_B3(            "Unknown");
                                    }
                                    }
                                    break;
                                default   :
                                    Skip_B4(                    "Unknown");
                            }
                            break;
                        default   :
                            Skip_B5(                            "Unknown");
                    }
                    }
                    break;
                case 0x02 :
                    {
                    Param_Info("Sound essence");
                    Info_B1(Code3,                              "Code (3)");
                    switch (Code3)
                    {
                        case 0x01 :
                            Skip_B5(                            "Sound coding or compression");
                            break;
                        case 0x02 :
                            {
                            Param_Info("Sound Coding Characteristics");
                            Info_B1(Code4,                      "Code (4)");
                            switch (Code4)
                            {
                                case 0x02 :
                                    {
                                    Param_Info("Compressed Sound Coding");
                                    Info_B1(Code5,              "Code (5)");
                                    switch (Code5)
                                    {
                                        case 0x03 :
                                            {
                                            Param_Info("Compressed Audio Coding");
                                            Info_B1(Code6,      "Code (6)");
                                            switch (Code6)
                                            {
                                                case 0x01 :
                                                    Param_Info("Compandeded Audio Coding");
                                                    Info_B1(Code7, "Code (7)");
                                                    switch (Code7)
                                                    {
                                                        case 0x01 :
                                                            Param_Info("A-law Coded Audio (default)");
                                                            Skip_B1("Unknown");
                                                            break;
                                                        case 0x02 :
                                                            Param_Info("DV Compressed Audio");
                                                            Skip_B1("Unknown");
                                                            break;
                                                        default   :
                                                            Skip_B2("Unknown");
                                                    }
                                                    break;
                                                default   :
                                                    Skip_B2(    "Unknown");
                                            }
                                            }
                                            break;
                                        default   :
                                            Skip_B3(            "Unknown");
                                    }
                                    }
                                    break;
                                default   :
                                    Skip_B4(                    "Unknown");
                            }
                            }
                            break;
                        default   :
                            Skip_B5(                            "Unknown");
                    }
                    }
                    break;
                default   :
                    Skip_B6(                                    "Unknown");
            }
            }
            break;
        case 0x0D :
            {
            Param_Info("User Organisation Registered For Public Use");
            Info_B1(Code2,                                      "Organization");
            switch (Code2)
            {
                case 0x01 :
                    {
                    Param_Info("AAF");
                    Info_B1(Code3,                              "Application");
                    switch (Code3)
                    {
                        case 0x02 :
                            {
                            Param_Info("Operational Patterns");
                            Info_B1(Code4,                      "Version");
                            switch (Code4)
                            {
                                case 0x01 :
                                    {
                                    Param_Info("Version 1");
                                    Info_B1(Code5,              "Item Complexity");
                                    Info_B1(Code6,              "Package Complexity");
                                    Info_B1(Code7,              "Qualifier");
                                        Skip_Flags(Code7, 3,    "uni/multi-track");
                                        Skip_Flags(Code7, 2,    "stream/non-stream file");
                                        Skip_Flags(Code7, 1,    "internal/external essence");
                                    Info_B1(Code8,              "Reserved");
                                    }
                                    break;
                                case 0x02 :
                                    {
                                    Info_B1(Code5,              "Code (5)");
                                    switch (Code5)
                                    {
                                        case 0x01 :
                                            Skip_B3(            "Essence container kind");
                                            break;
                                        default   :
                                            Skip_B3(            "Unknown");
                                    }
                                    }
                                    break;
                                default   :
                                    Skip_B4(                    "Unknown");
                            }
                            }
                            break;
                        case 0x03 :
                            {
                            Param_Info("Essence Container Application");
                            Info_B1(Code4,                      "Structure Version");
                            switch (Code4)
                            {
                                case 0x01 :
                                    {
                                    Param_Info("MXF EC Structure Version 1");
                                    Info_B1(Code5,              "Essence container Kind");
                                    switch (Code5)
                                    {
                                        case 0x01 :
                                            Param_Info("Deprecated Essence Container Kind");
                                            Skip_B3(            "Unknown");
                                            break;
                                        case 0x02 :
                                            {
                                            Param_Info("Essence Container Kind");
                                            Info_B1(Code6,      "Code (6)");
                                            switch (Code6)
                                            {
                                                case 0x01 : //SMPTE 386M
                                                    Param_Info("Type D-10 Mapping");
                                                    Skip_B1(            "MPEG Constraints"); //SMPTE 356M
                                                    Skip_B1(            "Template Extension");
                                                    break;
                                                case 0x02 :
                                                    Param_Info("DV Mappings");
                                                    Skip_B1(            "Mapping Kind");
                                                    Skip_B1(            "Locally defined");
                                                    break;
                                                case 0x03 :
                                                    Param_Info("Type D-11 Mapping");
                                                    Skip_B1(            "Mapping Kind");
                                                    Skip_B1(            "Locally defined");
                                                    break;
                                                case 0x04 :
                                                    {
                                                    Param_Info("MPEG ES mappings");
                                                    Info_B1(Code7,      "ISO13818-1 stream_id bits 6..0"); Param_Info(Ztring::ToZtring(0x80+Code7, 16));
                                                    Info_B1(Code8,      "Mapping Kind"); Param_Info(Mxf_EssenceContainer_Mapping(Code6, Code7, Code8));
                                                    }
                                                    break;
                                                case 0x05 : //SMPTE 384M
                                                    Param_Info("Uncompressed Pictures");
                                                    Info_B1(Code7,      "Number of lines / field rate combination"); //SMPTE 384M
                                                    Info_B1(Code8,      "Mapping Kind"); Param_Info(Mxf_EssenceContainer_Mapping(Code6, Code7, Code8));
                                                    break;
                                                case 0x06 :
                                                    {
                                                    Param_Info("AES-BWF");
                                                    Info_B1(Code7,      "Mapping Kind"); Param_Info(Mxf_EssenceContainer_Mapping(Code6, Code7, 0x7F));
                                                    Skip_B1(            "Locally defined");
                                                    }
                                                    break;
                                                case 0x07 :
                                                    {
                                                    Param_Info("MPEG PES mappings");
                                                    Info_B1(Code7,      "ISO13818-1 stream_id bits 6..0"); Param_Info(Ztring::ToZtring(0x80+Code7, 16));
                                                    Info_B1(Code8,      "Mapping Kind"); Param_Info(Mxf_EssenceContainer_Mapping(Code6, Code7, Code8));
                                                    }
                                                    break;
                                                case 0x08 :
                                                    {
                                                    Param_Info("MPEG PS mappings");
                                                    Info_B1(Code7,      "ISO13818-1 stream_id bits 6..0"); Param_Info(Ztring::ToZtring(0x80+Code7, 16));
                                                    Info_B1(Code8,      "Mapping Kind"); Param_Info(Mxf_EssenceContainer_Mapping(Code6, Code7, Code8));
                                                    }
                                                    break;
                                                case 0x09 :
                                                    {
                                                    Param_Info("MPEG TS mappings");
                                                    Info_B1(Code7,      "ISO13818-1 stream_id bits 6..0"); Param_Info(Ztring::ToZtring(0x80+Code7, 16));
                                                    Info_B1(Code8,      "Mapping Kind"); Param_Info(Mxf_EssenceContainer_Mapping(Code6, Code7, Code8));
                                                    }
                                                    break;
                                                case 0x0A :
                                                    Param_Info("A-law Sound Element Mapping");
                                                    Skip_B1(            "Mapping Kind");
                                                    Skip_B1(            "Locally defined");
                                                    break;
                                                case 0x0B :
                                                    Param_Info("Encrypted Generic Container");
                                                    Skip_B1(            "Mapping Kind");
                                                    Skip_B1(            "Locally defined");
                                                    break;
                                                case 0x0C :
                                                    Param_Info("JPEG 2000 Picture Mapping");
                                                    Skip_B1(            "Mapping Kind");
                                                    Skip_B1(            "Locally defined");
                                                    break;
                                                case 0x7F :
                                                    Param_Info("Generic Essence Container Wrapping");
                                                    Skip_B1(            "Mapping Kind");
                                                    Skip_B1(            "Locally defined");
                                                    break;
                                                default   :
                                                    Skip_B1(            "Mapping Kind");
                                                    Skip_B1(            "Locally defined");
                                            }
                                            }
                                            break;
                                        default   :
                                            Skip_B1(            "Essence container Kind");
                                            Skip_B1(            "Mapping Kind");
                                            Skip_B1(            "Locally defined");
                                    }
                                    }
                                    break;
                                default   :
                                    Skip_B4(                    "Unknown");
                            }
                            }
                            break;
                        case 0x04 :
                            {
                            Param_Info("MXF / AAF compatible Descriptive Metadata Labels");
                            Info_B1(Code4,                      "Label Version");
                            switch (Code4)
                            {
                                case 0x01 :
                                    {
                                    Param_Info("Version 1");
                                    Info_B1(Code5,              "Scheme Kind");
                                    Info_B1(Code6,              "Reserved");
                                    Info_B1(Code7,              "Reserved");
                                    Info_B1(Code8,              "Reserved");
                                    }
                                    break;
                                default   :
                                    Skip_B4(                    "Unknown");
                            }
                            }
                            break;
                        default   :
                            Skip_B5(                            "Unknown");
                    }
                    }
                    break;
                case 0x02 :
                    {
                    Param_Info("EBU/UER");
                    Skip_B6(                                    "Unknown");
                    }
                    break;
                case 0x03 :
                    {
                    Param_Info("Pro-MPEG Forum");
                    Skip_B6(                                    "Unknown");
                    }
                    break;
                case 0x04 :
                    {
                    Param_Info("BBC");
                    Skip_B6(                                    "Unknown");
                    }
                    break;
                case 0x05 :
                    {
                    Param_Info("IRT");
                    Skip_B6(                                    "Unknown");
                    }
                    break;
                case 0x06 :
                    {
                    Param_Info("ARIB");
                    Skip_B6(                                    "Unknown");
                    }
                    break;
                default   :
                    Skip_B6(                                    "Unknown");
            }
            }
            break;
        case 0x0E :
            {
            Param_Info("User Organisation Registered For Private Use");
            Skip_B7(                                            "Private");
            break;
            }
        default   :
            Skip_B7(                                            "Unknown");
    }
}

//---------------------------------------------------------------------------
void File_Mxf::Skip_UL(const char* Name)
{
    #ifdef MEDIAINFO_MINIMIZE_SIZE
        Skip_UUID();
    #else
        int128u Value;
        Get_UL(Value, Name, NULL);
    #endif
}

//---------------------------------------------------------------------------
void File_Mxf::Get_UMID(int256u &Value, const char* Name)
{
    Element_Name(Name);

    //Parsing
    Get_UUID (Value.hi,                                         "Fixed");
    Get_UUID (Value.lo,                                         "UUID"); Element_Info(Ztring().From_UUID(Value.lo));
}

//---------------------------------------------------------------------------
void File_Mxf::Skip_UMID()
{
    //Parsing
    Skip_UUID(                                                  "Fixed");
    Info_UUID(Data,                                             "UUID"); Element_Info(Ztring().From_UUID(Data));
}

//---------------------------------------------------------------------------
void File_Mxf::Get_Timestamp(Ztring &Value)
{
    //Parsing
    Info_B2(Year,                                               "Year");
    Info_B1(Month,                                              "Month");
    Info_B1(Day,                                                "Day");
    Info_B1(Hours,                                              "Hours");
    Info_B1(Minutes,                                            "Minutes");
    Info_B1(Seconds,                                            "Seconds");
    Info_B1(Milliseconds,                                       "Milliseconds/4"); Param_Info(Milliseconds*4, " ms");
    Value.From_Number(Year);
    Value+=_T('-');
    Ztring Temp;
    Temp.From_Number(Month);
    if (Temp.size()<2)
        Temp.insert(Temp.begin(), _T('0'));
    Value+=Temp;
    Value+=_T('-');
    Temp.From_Number(Day);
    if (Temp.size()<2)
        Temp.insert(Temp.begin(), _T('0'));
    Value+=Temp;
    Value+=_T(' ');
    Temp.From_Number(Hours);
    if (Temp.size()<2)
        Temp.insert(Temp.begin(), _T('0'));
    Value+=Temp;
    Value+=_T(':');
    Temp.From_Number(Minutes);
    if (Temp.size()<2)
        Temp.insert(Temp.begin(), _T('0'));
    Value+=Temp;
    Value+=_T(':');
    Temp.From_Number(Seconds);
    if (Temp.size()<2)
        Temp.insert(Temp.begin(), _T('0'));
    Value+=Temp;
    Value+=_T('.');
    Temp.From_Number(Milliseconds*4);
    if (Temp.size()<2)
        Temp.insert(Temp.begin(), _T('0'));
    if (Temp.size()<2)
        Temp.insert(Temp.begin(), _T('0'));
    Value+=Temp;
}

//---------------------------------------------------------------------------
void File_Mxf::Skip_Timestamp()
{
    //Parsing
    Skip_B2(                                                    "Year");
    Skip_B1(                                                    "Month");
    Skip_B1(                                                    "Day");
    Skip_B1(                                                    "Hours");
    Skip_B1(                                                    "Minutes");
    Skip_B1(                                                    "Seconds");
    Info_B1(Milliseconds,                                       "Milliseconds/4"); Param_Info(Milliseconds*4, " ms");
}

//---------------------------------------------------------------------------
void File_Mxf::Info_Timestamp()
{
    //Parsing
    Info_B2(Year,                                               "Year");
    Info_B1(Month,                                              "Month");
    Info_B1(Day,                                                "Day");
    Info_B1(Hours,                                              "Hours");
    Info_B1(Minutes,                                            "Minutes");
    Info_B1(Seconds,                                            "Seconds");
    Info_B1(Milliseconds,                                       "Milliseconds/4"); Param_Info(Milliseconds*4, " ms");
    Element_Info(Ztring::ToZtring(Year          )+_T('-')+
                 Ztring::ToZtring(Month         )+_T('-')+
                 Ztring::ToZtring(Day           )+_T(' ')+
                 Ztring::ToZtring(Hours         )+_T(':')+
                 Ztring::ToZtring(Minutes       )+_T(':')+
                 Ztring::ToZtring(Seconds       )+_T('.')+
                 Ztring::ToZtring(Milliseconds*4)         );
}

} //NameSpace

#endif //MEDIAINFO_MXF_*

