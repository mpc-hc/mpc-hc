/*  Copyright (c) MediaArea.net SARL. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license that can
 *  be found in the License.html file in the root of the source tree.
 */

//---------------------------------------------------------------------------
// Pre-compilation
#include "MediaInfo/PreComp.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Setup.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#if defined(MEDIAINFO_MXF_YES)
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Multiple/File_Mxf.h"
#if defined(MEDIAINFO_DVDIF_YES)
    #include "MediaInfo/Multiple/File_DvDif.h"
#endif
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
#if defined(MEDIAINFO_VC3_YES)
    #include "MediaInfo/Video/File_Vc3.h"
#endif
#if defined(MEDIAINFO_AAC_YES)
    #include "MediaInfo/Audio/File_Aac.h"
#endif
#if defined(MEDIAINFO_AC3_YES)
    #include "MediaInfo/Audio/File_Ac3.h"
#endif
#if defined(MEDIAINFO_SMPTEST0337_YES)
    #include "MediaInfo/Audio/File_ChannelGrouping.h"
#endif
#if defined(MEDIAINFO_MPEGA_YES)
    #include "MediaInfo/Audio/File_Mpega.h"
#endif
#if defined(MEDIAINFO_PCM_YES)
    #include "MediaInfo/Audio/File_Pcm.h"
#endif
#if defined(MEDIAINFO_SMPTEST0331_YES)
    #include "MediaInfo/Audio/File_SmpteSt0331.h"
#endif
#if defined(MEDIAINFO_SMPTEST0337_YES)
    #include "MediaInfo/Audio/File_SmpteSt0337.h"
#endif
#if defined(MEDIAINFO_JPEG_YES)
    #include "MediaInfo/Image/File_Jpeg.h"
#endif
#include "MediaInfo/TimeCode.h"
#include "MediaInfo/File_Unknown.h"
#include "ZenLib/File.h"
#include "ZenLib/FileName.h"
#include "ZenLib/Dir.h"
#include "MediaInfo/MediaInfo_Internal.h"
#include "MediaInfo/Multiple/File__ReferenceFilesHelper.h"
#include "ZenLib/Format/Http/Http_Utils.h"
#include <cfloat>
#if MEDIAINFO_SEEK
    #include <algorithm>
#endif //MEDIAINFO_SEEK
using namespace std;
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
    //Item - Elements - Interpretive - Fundamental - Data Interpretations and Definitions - Name-Value Construct Interpretations
    UUID(Ansi_01,                                               060E2B34, 01010105, 0301020A, 01000000)
    UUID(UTF16_01,                                              060E2B34, 01010105, 0301020A, 01010000)
    UUID(Ansi_02,                                               060E2B34, 01010105, 0301020A, 02000000)
    UUID(UTF16_02,                                              060E2B34, 01010105, 0301020A, 02010000)

    //Item - Elements - Interpretive - Fundamental - Data Interpretations and Definitions - KLV Interpretations
    UUID(Filler01,                                              060E2B34, 01010101, 03010210, 01000000)
    UUID(Filler02,                                              060E2B34, 01010102, 03010210, 01000000)
    UUID(TerminatingFiller,                                     060E2B34, 01010102, 03010210, 05000000)

    //Item - Elements - Interpretive - Fundamental - Data Interpretations and Definitions - XML Constructs and Interpretations
    UUID(XmlDocumentText,                                       060E2B34, 01010105, 03010220, 01000000)

    //Item - Elements - Interpretive - Fundamental - Data Interpretations and Definitions - ?
    UUID(SubDescriptors,                                        060E2B34, 01010109, 06010104, 06100000) // SMPTE ST 0429-10

    //Item - Elements - User organization registred for public use - AAF Association - Generic Container - Version 1
    UUID(GenericContainer_Aaf,                                  060E2B34, 01020101, 0D010301, 00000000)

    //Item - Elements - User organization registred for private use - Avid - Generic Container - Version 1
    UUID(GenericContainer_Avid,                                 060E2B34, 01020101, 0E040301, 00000000)

    //Item - Elements - User organization registred for private use - Sony - Generic Container - Version 6
    UUID(GenericContainer_Sony,                                 060E2B34, 01020101, 0E067F03, 00000000)

    //Item - Elements - Interpretive -
    UUID(DMScheme1_PrimaryExtendedSpokenLanguage,               060E2B34, 01010107, 03010102, 03110000)
    UUID(DMScheme1_SecondaryExtendedSpokenLanguage,             060E2B34, 01010107, 03010102, 03110000)
    UUID(DMScheme1_OriginalExtendedSpokenLanguage,              060E2B34, 01010107, 03010102, 03110000)

    //Item - Elements - Parametric - Video and Image Essence Characteristics - Digital Video and Image Compression Parameters - MPEG Coding Parameters - MPEG-2 Coding Parameters
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

    //Groups - Elements - User organization registred for public use - AAF Association - AAF Attributes - AAF Information Attributes - Version 1 - Enumerated Attributes
    UUID(Sequence,                                              060E2B34, 02530101, 0D010101, 01010F00)
    UUID(SourceClip,                                            060E2B34, 02530101, 0D010101, 01011100)
    UUID(TimecodeComponent,                                     060E2B34, 02530101, 0D010101, 01011400)
    UUID(ContentStorage,                                        060E2B34, 02530101, 0D010101, 01011800)
    UUID(EssenceContainerData,                                  060E2B34, 02530101, 0D010101, 01012300)
    UUID(GenericPictureEssenceDescriptor,                       060E2B34, 02530101, 0D010101, 01012700)
    UUID(CDCIEssenceDescriptor,                                 060E2B34, 02530101, 0D010101, 01012800)
    UUID(RGBAEssenceDescriptor,                                 060E2B34, 02530101, 0D010101, 01012900)
    UUID(Preface,                                               060E2B34, 02530101, 0D010101, 01012F00)
    UUID(Identification,                                        060E2B34, 02530101, 0D010101, 01013000)
    UUID(NetworkLocator,                                        060E2B34, 02530101, 0D010101, 01013200)
    UUID(TextLocator,                                           060E2B34, 02530101, 0D010101, 01013300)
    UUID(StereoscopicPictureSubDescriptor,                      060E2B34, 0253010C, 0D010101, 01016300) // SMPTE ST 0429-10
    UUID(MaterialPackage,                                       060E2B34, 02530101, 0D010101, 01013600)
    UUID(SourcePackage,                                         060E2B34, 02530101, 0D010101, 01013700)
    UUID(EventTrack,                                            060E2B34, 02530101, 0D010101, 01013900)
    UUID(StaticTrack,                                           060E2B34, 02530101, 0D010101, 01013A00)
    UUID(Track,                                                 060E2B34, 02530101, 0D010101, 01013B00)
    UUID(DMSegment,                                             060E2B34, 02530101, 0D010101, 01014100)
    UUID(GenericSoundEssenceDescriptor,                         060E2B34, 02530101, 0D010101, 01014200)
    UUID(GenericDataEssenceDescriptor,                          060E2B34, 02530101, 0D010101, 01014300)
    UUID(MultipleDescriptor,                                    060E2B34, 02530101, 0D010101, 01014400)
    UUID(AES3PCMDescriptor,                                     060E2B34, 02530101, 0D010101, 01014700)
    UUID(WaveAudioDescriptor,                                   060E2B34, 02530101, 0D010101, 01014800)
    UUID(MPEG2VideoDescriptor,                                  060E2B34, 02530101, 0D010101, 01015100)
    UUID(JPEG2000PictureSubDescriptor,                          060E2B34, 02530101, 0D010101, 01015A00)
    UUID(VbiPacketsDescriptor,                                  060E2B34, 02530101, 0D010101, 01015B00)
    UUID(AncPacketsDescriptor,                                  060E2B34, 02530101, 0D010101, 01015C00)

    //Groups - Elements - User organization registred for public use - AAF Association - ? - Version 1 - ?
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

    //Groups - Elements - User organization registred for public use - AAF Association - ?  - Version 1 - ?
    UUID(Primer,                                                060E2B34, 02050101, 0D010201, 01050100)

    //Groups - Elements - User organization registred for public use - AAF Association - ? - Version 1 - ?
    UUID(IndexTableSegment,                                     060E2B34, 02530101, 0D010201, 01100100)

    //Groups - Elements - User organization registred for public use - AAF Association - ? - Version 1 - ?
    UUID(RandomIndexMetadata,                                   060E2B34, 02050101, 0D010201, 01110100)

    //Groups - Elements - User organization registred for public use - AAF Association - ? - Version 1 - ? (SDTI-CP (SMPTE 385M))
    UUID(SDTI_SystemMetadataPack,                               060E2B34, 02050101, 0D010301, 04010100)
    UUID(SDTI_PackageMetadataSet,                               060E2B34, 02430101, 0D010301, 04010200)
    UUID(SDTI_PictureMetadataSet,                               060E2B34, 02430101, 0D010301, 04010300)
    UUID(SDTI_SoundMetadataSet,                                 060E2B34, 02430101, 0D010301, 04010400)
    UUID(SDTI_DataMetadataSet,                                  060E2B34, 02430101, 0D010301, 04010500)
    UUID(SDTI_ControlMetadataSet,                               060E2B34, 02630101, 0D010301, 04010600)

    //Groups - Elements - User organization registred for public use - AAF Association - ? - Version 1 - ? (SystemScheme (SMPTE 405M))
    UUID(SystemScheme1,                                         060E2B34, 02530101, 0D010301, 14020100)

    //Groups - Elements - User organization registred for public use - AAF Association - Descriptive Metadata Scheme - Version 1 (SystemScheme (SMPTE 380M))
    UUID(DMScheme1,                                             060E2B34, 02530101, 0D010401, 01010100)

    //Groups - Elements - User organization registred for private use - Omneon Video Networks
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
                        case 0x01 : return "Metadata";
                        case 0x02 : return "Essence";
                        default   : return "";
                    }
        case 0x02 : //"Group (Set/Pack)"
                    switch(Registry)
                    {
                        case 0x05 : return "Predefined items";
                        case 0x43 : return "1-byte tag, 2-byte length";
                        case 0x53 : return "2-byte tag, 2-byte length";
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
const char* Mxf_Structure(int8u Category, int8u Registry, int8u Structure)
{
    switch(Category)
    {
        case 0x01 : //"Item"
                    switch(Registry)
                    {
                        case 0x02 : //Essence
                                    switch(Structure)
                                    {
                                        case 0x01 : return "Standard";
                                        default   : return "";
                                    }
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
const char* Mxf_OperationalPattern(const int128u OperationalPattern)
{
    //Item and Package Complexity
    int32u Code_Compare4=(int32u)OperationalPattern.lo;
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
        case 0x10 : return "OP-Atom";
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* Mxf_EssenceElement(const int128u EssenceElement)
{
    if ((EssenceElement.hi&0xFFFFFFFFFFFFFF00LL)!=0x060E2B3401020100LL)
        return "";

    int8u Code1=(int8u)((EssenceElement.lo&0xFF00000000000000LL)>>56);
    int8u Code2=(int8u)((EssenceElement.lo&0x00FF000000000000LL)>>48);
    int8u Code5=(int8u)((EssenceElement.lo&0x00000000FF000000LL)>>24);
    int8u Code7=(int8u)((EssenceElement.lo&0x000000000000FF00LL)>> 8);
    
    switch (Code1)
    {
        case 0x0E : //Private
                    switch (Code2)
                    {
                        case 0x06 : //Sony
                                    case 0x15 : //GC Picture
                                                switch (Code5)
                                                {
                                                    case 0x15 : return "Sony private picture stream";
                                                    default   : return "Sony private stream";
                                                }
                        default   : return "Unknown private stream";
                    }
        default   : ;
    }

    switch (Code5)
    {
        case 0x05 : //CP Picture (SMPTE 386M)
                    switch (Code7)
                    {
                        case 0x01 : return "D-10 Video";
                        default   : return "Unknown stream";
                    }
        case 0x06 : //CP Sound (SMPTE 386M)
                    switch (Code7)
                    {
                        case 0x10 : return "D-10 Audio";
                        default   : return "Unknown stream";
                    }
        case 0x07 : //CP Data (SMPTE 386M)
                    return "";
        case 0x14 : //MXF in MXF?
                    switch (Code7)
                    {
                        case 0x01 : return "MXF in MXF?";
                        default   : return "Unknown stream";
                    }
        case 0x15 : //GC Picture
                    switch (Code7)
                    {
                        case 0x01 : return "RGB";
                        case 0x05 : return "MPEG stream (Frame)";
                        case 0x06 : return "MPEG stream (Clip)";
                        case 0x07 : return "MPEG stream (Custom)";
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
                        case 0x05 : return "MPEG Audio / AC-3";
                        case 0x0A : return "A-law";
                        default   : return "Unknown stream";
                    }
        case 0x17 : //GC Data
                    switch (Code7)
                    {
                        case 0x01 : return "VBI"; //Frame-Wrapped VBI Data Element
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
const char* Mxf_EssenceContainer(const int128u EssenceContainer)
{
    if ((EssenceContainer.hi&0xFFFFFFFFFFFFFF00LL)!=0x060E2B3404010100LL)
        return "";

    int8u Code1=(int8u)((EssenceContainer.lo&0xFF00000000000000LL)>>56);
    int8u Code2=(int8u)((EssenceContainer.lo&0x00FF000000000000LL)>>48);
    int8u Code3=(int8u)((EssenceContainer.lo&0x0000FF0000000000LL)>>40);
    int8u Code4=(int8u)((EssenceContainer.lo&0x000000FF00000000LL)>>32);
    int8u Code5=(int8u)((EssenceContainer.lo&0x00000000FF000000LL)>>24);
    int8u Code6=(int8u)((EssenceContainer.lo&0x0000000000FF0000LL)>>16);
    int8u Code7=(int8u)((EssenceContainer.lo&0x000000000000FF00LL)>> 8);

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
                                                                                        case 0x01 : return "D-10"; // Video and Audio
                                                                                        case 0x02 : return "DV";
                                                                                        case 0x05 : return "Uncompressed pictures";
                                                                                        case 0x06 : return "PCM";
                                                                                        case 0x04 : return "MPEG ES mappings with Stream ID";
                                                                                        case 0x0A : return "A-law";
                                                                                        case 0x0C : return "JPEG 2000";
                                                                                        case 0x10 : return "AVC";
                                                                                        case 0x11 : return "VC-3";
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
        case 0x0E : //Private Use
                    switch (Code2)
                    {
                        case 0x04 : //Avid
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
                                                                                        case 0x06 : return "VC-3";
                                                                                        default   : return "";
                                                                                    }
                                                                        default   : return "";
                                                                    }
                                                         default   : return "";
                                                    }
                                         default   : return "";
                                    }
                        case 0x06 : //Sony
                                    switch (Code3)
                                    {
                                        case 0x0D :
                                                    switch (Code4)
                                                    {
                                                        case 0x03 :
                                                                    switch (Code5)
                                                                    {
                                                                        case 0x02 :
                                                                                    switch (Code6)
                                                                                    {
                                                                                        case 0x01 :
                                                                                                    switch (Code7)
                                                                                                    {
                                                                                                        case 0x01 : return "Sony RAW?";
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
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* Mxf_EssenceContainer_Mapping(int8u Code6, int8u Code7, int8u Code8)
{
    switch (Code6)
    {
        case 0x01 : //D-10, SMPTE 386M
                    return "Frame (D-10)";
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
        case 0x10 : //AVC
        case 0x15 : //YUV
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
                        case 0x04 : return "Clip (AES)";
                        case 0x08 : return "Custom (BWF)";
                        case 0x09 : return "Custom (AES)";
                        default   : return "";
                    }
        case 0x0A : //A-Law
                    switch (Code7)
                    {
                        case 0x01 : return "Frame";
                        case 0x02 : return "Clip";
                        case 0x03 : return "?";
                        case 0x07 : return "Custom";
                        default   : return "";
                    }
        case 0x0C : //JPEG 2000
                    switch (Code7)
                    {
                        case 0x01 : return "Frame";
                        case 0x02 : return "Clip";
                        default   : return "";
                    }
        case 0x11 : //VC-3, SMPTE 2019-4
                    switch (Code7)
                    {
                        case 0x01 : return "Frame";
                        case 0x02 : return "Clip";
                        default   : return "";
                    }
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* Mxf_EssenceCompression(const int128u EssenceCompression)
{
    if ((EssenceCompression.hi&0xFFFFFFFFFFFFFF00LL)!=0x060E2B3404010100LL || !((EssenceCompression.lo&0xFF00000000000000LL)==0x0400000000000000LL || (EssenceCompression.lo&0xFF00000000000000LL)==0x0E00000000000000LL))
        return "";

    int8u Code1=(int8u)((EssenceCompression.lo&0xFF00000000000000LL)>>56);
    int8u Code2=(int8u)((EssenceCompression.lo&0x00FF000000000000LL)>>48);
    int8u Code3=(int8u)((EssenceCompression.lo&0x0000FF0000000000LL)>>40);
    int8u Code4=(int8u)((EssenceCompression.lo&0x000000FF00000000LL)>>32);
    int8u Code5=(int8u)((EssenceCompression.lo&0x00000000FF000000LL)>>24);
    int8u Code6=(int8u)((EssenceCompression.lo&0x0000000000FF0000LL)>>16);
    int8u Code7=(int8u)((EssenceCompression.lo&0x000000000000FF00LL)>> 8);

    switch (Code1)
    {
        case 0x04 : //
                    switch (Code2)
                    {
                        case 0x01 : //Picture
                                    switch (Code3)
                                    {
                                        case 0x02 : //Coding characteristics
                                                    switch (Code4)
                                                    {
                                                        case 0x01 : //Uncompressed coding
                                                                    switch (Code5)
                                                                    {
                                                                        case 0x01 : //Uncompressed picture coding
                                                                                    return "YUV";
                                                                        default   : return "";
                                                                    }
                                                        case 0x02 : //Compressed coding
                                                                    switch (Code5)
                                                                    {
                                                                        case 0x01 : //MPEG Compression
                                                                                    switch (Code6)
                                                                                    {
                                                                                        case 0x00 : return "MPEG Video";
                                                                                        case 0x01 : return "MPEG Video"; //Version 2
                                                                                        case 0x02 : return "MPEG Video"; //Version 2
                                                                                        case 0x03 : return "MPEG Video"; //Version 2
                                                                                        case 0x04 : return "MPEG Video"; //Version 2
                                                                                        case 0x11 : return "MPEG Video"; //Version 1
                                                                                        case 0x20 : return "MPEG-4 Visual";
                                                                                        case 0x30 :
                                                                                        case 0x31 :
                                                                                        case 0x32 :
                                                                                        case 0x33 :
                                                                                        case 0x34 :
                                                                                        case 0x35 :
                                                                                        case 0x36 :
                                                                                        case 0x37 :
                                                                                        case 0x38 :
                                                                                        case 0x39 :
                                                                                        case 0x3A :
                                                                                        case 0x3B :
                                                                                        case 0x3C :
                                                                                        case 0x3D :
                                                                                        case 0x3E :
                                                                                        case 0x3F : return "AVC";
                                                                                        default   : return "";
                                                                                    }
                                                                        case 0x02 : return "DV";
                                                                        case 0x03 : //Individual Picture Coding Schemes
                                                                                    switch (Code6)
                                                                                    {
                                                                                        case 0x01 : return "JPEG 2000";
                                                                                        default   : return "";
                                                                                    }
                                                                        case 0x71 : return "VC-3";
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
                                                                        case 0x00 : return "PCM";
                                                                        case 0x01 : return "PCM";
                                                                        case 0x7E : return "PCM"; //AIFF
                                                                        case 0x7F : return "PCM"; // TODO: Undefined
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
                                                                                                        default   : return ""; //Unknown
                                                                                                    }
                                                                                        case 0x02 : //SMPTE 338M Audio Coding
                                                                                                    switch (Code7)
                                                                                                    {
                                                                                                        case 0x01 : return "AC-3";
                                                                                                        case 0x04 : return "MPEG-1 Audio Layer 1";
                                                                                                        case 0x05 : return "MPEG-1 Audio Layer 2 or 3";
                                                                                                        case 0x06 : return "MPEG-2 Audio Layer 1";
                                                                                                        case 0x1C : return "Dolby E";
                                                                                                        default   : return ""; //Unknown
                                                                                                    }
                                                                                        case 0x03 : //MPEG-2 Coding (not defined in SMPTE 338M)
                                                                                                    switch (Code7)
                                                                                                    {
                                                                                                        case 0x01 : return "AAC version 2";
                                                                                                        default   : return ""; //Unknown
                                                                                                    }
                                                                                        case 0x04 : //MPEG-4 Audio Coding
                                                                                                    switch (Code7)
                                                                                                    {
                                                                                                        case 0x01 : return "MPEG-4 Speech Profile";
                                                                                                        case 0x02 : return "MPEG-4 Synthesis Profile";
                                                                                                        case 0x03 : return "MPEG-4 Scalable Profile";
                                                                                                        case 0x04 : return "MPEG-4 Main Profile";
                                                                                                        case 0x05 : return "MPEG-4 High Quality Audio Profile";
                                                                                                        case 0x06 : return "MPEG-4 Low Delay Audio Profile";
                                                                                                        case 0x07 : return "MPEG-4 Natural Audio Profile";
                                                                                                        case 0x08 : return "MPEG-4 Mobile Audio Internetworking Profile";
                                                                                                        default   : return ""; //Unknown
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
        case 0x0E : //Private Use
                    switch (Code2)
                    {
                        case 0x04 : //Avid
                                    switch (Code3)
                                    {
                                        case 0x02 : //Essence Compression ?
                                                    switch (Code4)
                                                    {
                                                        case 0x01 : //?
                                                                    switch (Code5)
                                                                    {
                                                                        case 0x02 : //?
                                                                                    switch (Code6)
                                                                                    {
                                                                                        case 0x04 : return "VC-3";
                                                                                        default   : return "";
                                                                                    }
                                                                        default   : return "";
                                                                    }
                                                         default   : return "";
                                                    }
                                         default   : return "";
                                    }
                        case 0x06 : //Sony
                                    switch (Code3)
                                    {
                                        case 0x04 :
                                                    switch (Code4)
                                                    {
                                                        case 0x01 :
                                                                    switch (Code5)
                                                                    {
                                                                        case 0x02 :
                                                                                    switch (Code6)
                                                                                    {
                                                                                        case 0x04 :
                                                                                                    switch (Code7)
                                                                                                    {
                                                                                                        case 0x02 : return "Sony RAW SQ";
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
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* Mxf_EssenceCompression_Version(const int128u EssenceCompression)
{
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
        case 0x02 : //Sound
                    switch (Code3)
                    {
                        case 0x02 : //Coding characteristics
                                    switch (Code4)
                                    {
                                        case 0x02 : //Compressed coding
                                                    switch (Code5)
                                                    {
                                                        case 0x03 : //Compressed Audio Coding
                                                                    switch (Code6)
                                                                    {
                                                                        case 0x02 : //SMPTE 338M Audio Coding
                                                                                    switch (Code7)
                                                                                    {
                                                                                        case 0x04 : return "Version 1"; //Layer 1
                                                                                        case 0x05 : return "Version 1"; //Layer 2 or 3
                                                                                        case 0x06 : return "Version 2"; //Layer 1
                                                                                        default   : return ""; //Unknown
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
const char* Mxf_Sequence_DataDefinition(const int128u DataDefinition)
{
    int8u Code4=(int8u)((DataDefinition.lo&0x000000FF00000000LL)>>32);
    int8u Code5=(int8u)((DataDefinition.lo&0x00000000FF000000LL)>>24);

    switch (Code4)
    {
        case 0x01 : return "Time";
        case 0x02 :
                    switch (Code5)
                    {
                        case 0x01 : return "Picture";
                        case 0x02 : return "Sound";
                        case 0x03 : return "Data";
                        default   : return "";
                    }
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* Mxf_FrameLayout(int8u FrameLayout)
{
    switch (FrameLayout)
    {
        case 0x00 : return "Full frame";
        case 0x01 : return "Separated fields";
        case 0x02 : return "Single field";
        case 0x03 : return "Mixed fields";
        case 0x04 : return "Segmented frame";
        default   : return "";
    }
}

//---------------------------------------------------------------------------
const char* Mxf_FrameLayout_ScanType(int8u FrameLayout)
{
    switch (FrameLayout)
    {
        case 0x01 :
        case 0x04 :
        case 0xFF : //Seen in one file
                    return "Interlaced";
        default   :
                    return "Progressive";
    }
}

//---------------------------------------------------------------------------
int8u Mxf_FrameLayout_Multiplier(int8u FrameLayout)
{
    switch (FrameLayout)
    {
        case 0x01 :
        case 0x04 :
        case 0xFF : //Seen in one file
                    return 2;
        default   :
                    return 1;
    }
}

//---------------------------------------------------------------------------
extern const char* Mpegv_profile_and_level_indication_profile[];
extern const char* Mpegv_profile_and_level_indication_level[];

//---------------------------------------------------------------------------
extern const char* AfdBarData_active_format[];
extern const char* AfdBarData_active_format_4_3[];
extern const char* AfdBarData_active_format_16_9[];

//***************************************************************************
// Constructor/Destructor
//***************************************************************************

//---------------------------------------------------------------------------
File_Mxf::File_Mxf()
:File__Analyze()
{
    //Configuration
    ParserName=__T("MXF");
    #if MEDIAINFO_EVENTS
        ParserIDs[0]=MediaInfo_Parser_Mxf;
        StreamIDs_Width[0]=8;
    #endif //MEDIAINFO_EVENTS
    #if MEDIAINFO_DEMUX
        Demux_Level=2; //Container
    #endif //MEDIAINFO_DEMUX
    MustSynchronize=true;
    DataMustAlwaysBeComplete=false;
    Buffer_TotalBytes_Fill_Max=(int64u)-1; //Disabling this feature for this format, this is done in the parser
    FrameInfo.DTS=0;
    Frame_Count_NotParsedIncluded=0;
    #if MEDIAINFO_DEMUX
        Demux_EventWasSent_Accept_Specific=true;
    #endif //MEDIAINFO_DEMUX

    //Temp
    RandomIndexMetadatas_AlreadyParsed=false;
    Streams_Count=(size_t)-1;
    OperationalPattern=0;
    Buffer_Begin=(int64u)-1;
    Buffer_End=0;
    Buffer_End_Unlimited=false;
    Buffer_Header_Size=0;
    Preface_Current.hi=0;
    Preface_Current.lo=0;
    IsParsingMiddle_MaxOffset=(int64u)-1;
    Track_Number_IsAvailable=false;
    IsParsingEnd=false;
    IsCheckingRandomAccessTable=false;
    IsCheckingFooterPartitionAddress=false;
    FooterPartitionAddress_Jumped=false;
    PartitionPack_Parsed=false;
    IdIsAlwaysSame_Offset=0;
    PartitionMetadata_PreviousPartition=(int64u)-1;
    PartitionMetadata_FooterPartition=(int64u)-1;
    TimeCode_StartTimecode=(int64u)-1;
    TimeCode_RoundedTimecodeBase=0;
    TimeCode_DropFrame=false;
    DTS_Delay=0;
    StreamPos_StartAtOne=true;
    SDTI_TimeCode_StartTimecode_ms=(int64u)-1;
    SDTI_SizePerFrame=0;
    SDTI_IsPresent=false;
    SDTI_IsInIndexStreamOffset=true;
    SystemScheme1_TimeCodeArray_StartTimecode_ms=(int64u)-1;
    SystemScheme1_FrameRateFromDescriptor=0;
    Essences_FirstEssence_Parsed=false;
    StereoscopicPictureSubDescriptor_IsPresent=false;
    Essences_UsedForFrameCount=(int32u)-1;
    #if MEDIAINFO_ADVANCED
        Footer_Position=(int64u)-1;
    #endif //MEDIAINFO_ADVANCED
    ReferenceFiles=NULL;
    #if MEDIAINFO_NEXTPACKET
        ReferenceFiles_IsParsing=false;
    #endif //MEDIAINFO_NEXTPACKET
    #if defined(MEDIAINFO_ANCILLARY_YES)
        Ancillary=NULL;
        Ancillary_IsBinded=false;
    #endif //defined(MEDIAINFO_ANCILLARY_YES)

    #if MEDIAINFO_DEMUX
        Demux_HeaderParsed=false;
    #endif //MEDIAINFO_DEMUX

    Partitions_Pos=0;
    Partitions_IsCalculatingHeaderByteCount=false;
    Partitions_IsCalculatingSdtiByteCount=false;
    Partitions_IsFooter=false;

    #if MEDIAINFO_SEEK
        IndexTables_Pos=0;
        Clip_Header_Size=0;
        Clip_Begin=(int64u)-1;
        Clip_End=0;
        OverallBitrate_IsCbrForSure=0;
        Duration_Detected=false;
    #endif //MEDIAINFO_SEEK
}

//---------------------------------------------------------------------------
File_Mxf::~File_Mxf()
{
    delete ReferenceFiles;
    if (!Ancillary_IsBinded)
        delete Ancillary;
}

//***************************************************************************
// Streams management
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mxf::Streams_Accept()
{
    //Configuration
    Buffer_MaximumSize=64*1024*1024; //Some big frames are possible (e.g YUV 4:2:2 10 bits 1080p, 4K)
}

//---------------------------------------------------------------------------
void File_Mxf::Streams_Fill()
{
    for (essences::iterator Essence=Essences.begin(); Essence!=Essences.end(); ++Essence)
        for (parsers::iterator Parser=Essence->second.Parsers.begin(); Parser!=Essence->second.Parsers.end(); ++Parser)
            Fill(*Parser);
}

//---------------------------------------------------------------------------
void File_Mxf::Streams_Finish()
{
    #if MEDIAINFO_NEXTPACKET
        //Locators only
        if (ReferenceFiles_IsParsing)
        {
            ReferenceFiles->ParseReferences();
            #if MEDIAINFO_DEMUX
                if (Config->Demux_EventWasSent)
                    return;
            #endif //MEDIAINFO_DEMUX

            Streams_Finish_CommercialNames();
            return;
        }
    #endif //MEDIAINFO_NEXTPACKET

    //Per stream
    for (essences::iterator Essence=Essences.begin(); Essence!=Essences.end(); ++Essence)
    {
        if (Essence->second.Parsers.size()!=1 && Essence->second.StreamKind==Stream_Audio) // Last parser is PCM, impossible to detect with another method if there is only one block
        {
            for (size_t Pos=0; Pos<Essence->second.Parsers.size()-1; Pos++)
                delete Essence->second.Parsers[Pos];
            Essence->second.Parsers.erase(Essence->second.Parsers.begin(), Essence->second.Parsers.begin()+Essence->second.Parsers.size()-1);
            Essence->second.Parsers[0]->Accept();
            Essence->second.Parsers[0]->Fill();
        }
        for (parsers::iterator Parser=Essence->second.Parsers.begin(); Parser!=Essence->second.Parsers.end(); ++Parser)
        {
            if (!(*Parser)->Status[IsFinished])
            {
                if (Config->ParseSpeed>=1)
                {
                    int64u File_Size_Temp=File_Size;
                    File_Size=File_Offset+Buffer_Offset+Element_Offset;
                    Open_Buffer_Continue(*Parser, Buffer, 0);
                    File_Size=File_Size_Temp;
                }
                Finish(*Parser);
                #if MEDIAINFO_DEMUX
                    if (Config->Demux_EventWasSent)
                        return;
                #endif //MEDIAINFO_DEMUX
            }
        }
    }

    if (!Track_Number_IsAvailable)
    {
        if (Tracks.empty())
        {
            for (essences::iterator Essence=Essences.begin(); Essence!=Essences.end(); ++Essence)
                for (parsers::iterator Parser=Essence->second.Parsers.begin(); Parser!=Essence->second.Parsers.end(); ++Parser)
                    Merge(*(*Parser));
        }
        else
            for (tracks::iterator Track=Tracks.begin(); Track!=Tracks.end(); ++Track)
            {
                //Searching the corresponding Descriptor
                stream_t StreamKind=Stream_Max;
                for (descriptors::iterator Descriptor=Descriptors.begin(); Descriptor!=Descriptors.end(); ++Descriptor)
                    if (Descriptor->second.LinkedTrackID==Track->second.TrackID)
                    {
                        StreamKind=Descriptor->second.StreamKind;
                        break;
                    }
                if (StreamKind!=Stream_Max)
                {
                    for (essences::iterator Essence=Essences.begin(); Essence!=Essences.end(); ++Essence)
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
    StreamKind_Last=Stream_Max;
    StreamPos_Last=(size_t)-1;

    Streams_Finish_Preface(Preface_Current);

    //OperationalPattern
    Fill(Stream_General, 0, General_Format_Profile, Mxf_OperationalPattern(OperationalPattern));

    //Time codes
    if (SDTI_TimeCode_StartTimecode_ms!=(int64u)-1)
    {
        bool IsDuplicate=false;
        for (size_t Pos2=0; Pos2<Count_Get(Stream_Other); Pos2++)
            if (Retrieve(Stream_Other, Pos2, "TimeCode_Source")==__T("SDTI"))
                IsDuplicate=true;
        if (!IsDuplicate)
        {
            Fill_Flush();
            Stream_Prepare(Stream_Other);
            Fill(Stream_Other, StreamPos_Last, Other_Type, "Time code");
            Fill(Stream_Other, StreamPos_Last, Other_Format, "SMPTE TC");
            Fill(Stream_Other, StreamPos_Last, Other_MuxingMode, "SDTI");
            Fill(Stream_Other, StreamPos_Last, Other_TimeCode_FirstFrame, SDTI_TimeCode_StartTimecode.c_str());
        }
    }
    if (SystemScheme1_TimeCodeArray_StartTimecode_ms!=(int64u)-1)
    {
        bool IsDuplicate=false;
        for (size_t Pos2=0; Pos2<Count_Get(Stream_Other); Pos2++)
            if (Retrieve(Stream_Other, Pos2, "TimeCode_Source")==__T("System scheme 1"))
                IsDuplicate=true;
        if (!IsDuplicate)
        {
            Fill_Flush();
            Stream_Prepare(Stream_Other);
            Fill(Stream_Other, StreamPos_Last, Other_Type, "Time code");
            Fill(Stream_Other, StreamPos_Last, Other_Format, "SMPTE TC");
            Fill(Stream_Other, StreamPos_Last, Other_MuxingMode, "System scheme 1");
            Fill(Stream_Other, StreamPos_Last, Other_TimeCode_FirstFrame, SystemScheme1_TimeCodeArray_StartTimecode.c_str());
        }
    }

    //Parsing locators
    Locators_Test();
    #if MEDIAINFO_NEXTPACKET
        if (Config->NextPacket_Get() && ReferenceFiles && !ReferenceFiles->References.empty())
        {
            ReferenceFiles_IsParsing=true;
            return;
        }
    #endif //MEDIAINFO_NEXTPACKET

    //Sizes
    #if MEDIAINFO_ADVANCED
        if (Footer_Position!=(int64u)-1)
            Fill(Stream_General, 0, General_FooterSize, File_Size-Footer_Position);
    #endif //MEDIAINFO_ADVANCED

    //Commercial names
    Streams_Finish_CommercialNames();

    //Handling separate streams
    for (size_t StreamKind=Stream_General+1; StreamKind<Stream_Max; StreamKind++)
        for (size_t StreamPos=0; StreamPos<Count_Get((stream_t)StreamKind); StreamPos++)
            if (Retrieve((stream_t)StreamKind, StreamPos, Fill_Parameter((stream_t)StreamKind, Generic_StreamSize_Encoded)).empty() && !Retrieve((stream_t)StreamKind, StreamPos, Fill_Parameter((stream_t)StreamKind, Generic_BitRate_Encoded)).empty() && !Retrieve((stream_t)StreamKind, StreamPos, Fill_Parameter((stream_t)StreamKind, Generic_Duration)).empty())
            {
                float64 BitRate_Encoded=Retrieve((stream_t)StreamKind, StreamPos, Fill_Parameter((stream_t)StreamKind, Generic_BitRate_Encoded)).To_float64();
                float64 Duration=Retrieve((stream_t)StreamKind, StreamPos, Fill_Parameter((stream_t)StreamKind, Generic_Duration)).To_float64();
                if (Duration)
                    Fill((stream_t)StreamKind, StreamPos, Fill_Parameter((stream_t)StreamKind, Generic_StreamSize_Encoded), BitRate_Encoded/8*(Duration/1000), 0);
            }

    //File size in case of partial file analysis
    if (Config->File_IgnoreFramesBefore || Config->File_IgnoreFramesAfter!=(int64u)-1)
    {
        int64u FrameCount_FromComponent=(int64u)-1;
        for (components::iterator Component=Components.begin(); Component!=Components.end(); ++Component)
            if (FrameCount_FromComponent>Component->second.Duration)
                FrameCount_FromComponent=Component->second.Duration;
        float64 EditRate_FromTrack=DBL_MAX;
        for (tracks::iterator Track=Tracks.begin(); Track!=Tracks.end(); ++Track)
            if (EditRate_FromTrack>Track->second.EditRate)
                EditRate_FromTrack=Track->second.EditRate;
        if (FrameCount_FromComponent!=(int64u)-1 && FrameCount_FromComponent && EditRate_FromTrack!=DBL_MAX && EditRate_FromTrack)
        {
            int64u FrameCount=FrameCount_FromComponent;
            int64u File_IgnoreFramesBefore=Config->File_IgnoreFramesBefore;
            if (File_IgnoreFramesBefore && Config->File_IgnoreFramesRate && (EditRate_FromTrack<Config->File_IgnoreFramesRate*0.9 || EditRate_FromTrack>Config->File_IgnoreFramesRate*1.1)) //In case of problem or EditRate being sampling rate
                File_IgnoreFramesBefore=float64_int64s(((float64)File_IgnoreFramesBefore)/Config->File_IgnoreFramesRate*EditRate_FromTrack);
            int64u File_IgnoreFramesAfter=Config->File_IgnoreFramesAfter;
            if (File_IgnoreFramesAfter!=(int64u)-1 && Config->File_IgnoreFramesRate && (EditRate_FromTrack<Config->File_IgnoreFramesRate*0.9 || EditRate_FromTrack>Config->File_IgnoreFramesRate*1.1)) //In case of problem or EditRate being sampling rate
                File_IgnoreFramesAfter=float64_int64s(((float64)File_IgnoreFramesAfter)/Config->File_IgnoreFramesRate*EditRate_FromTrack);
            if (File_IgnoreFramesAfter<FrameCount)
                FrameCount=File_IgnoreFramesAfter;
            if (FrameCount<File_IgnoreFramesBefore)
                FrameCount=File_IgnoreFramesBefore;
            FrameCount-=File_IgnoreFramesBefore;

            float64 File_Size_Temp=(float64)File_Size;
            File_Size_Temp/=FrameCount_FromComponent;
            File_Size_Temp*=FrameCount;
            Fill(Stream_General, 0, General_FileSize, File_Size_Temp, 0, true);
        }
    }
}

//---------------------------------------------------------------------------
void File_Mxf::Streams_Finish_Preface (const int128u PrefaceUID)
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
void File_Mxf::Streams_Finish_ContentStorage (const int128u ContentStorageUID)
{
    contentstorages::iterator ContentStorage=ContentStorages.find(ContentStorageUID);
    if (ContentStorage==ContentStorages.end())
        return;

    for (size_t Pos=0; Pos<ContentStorage->second.Packages.size(); Pos++)
        Streams_Finish_Package(ContentStorage->second.Packages[Pos]);
}

//---------------------------------------------------------------------------
void File_Mxf::Streams_Finish_Package (const int128u PackageUID)
{
    packages::iterator Package=Packages.find(PackageUID);
    if (Package==Packages.end() || !Package->second.IsSourcePackage)
        return;

    for (size_t Pos=0; Pos<Package->second.Tracks.size(); Pos++)
        Streams_Finish_Track(Package->second.Tracks[Pos]);

    Streams_Finish_Descriptor(Package->second.Descriptor, PackageUID);
}

//---------------------------------------------------------------------------
void File_Mxf::Streams_Finish_Track(const int128u TrackUID)
{
    tracks::iterator Track=Tracks.find(TrackUID);
    if (Track==Tracks.end() || Track->second.Stream_Finish_Done)
        return;

    StreamKind_Last=Stream_Max;
    StreamPos_Last=(size_t)-1;

    Streams_Finish_Essence(Track->second.TrackNumber, TrackUID);

    //Sequence
    Streams_Finish_Component(Track->second.Sequence, Track->second.EditRate_Real?Track->second.EditRate_Real:Track->second.EditRate, Track->second.TrackID, Track->second.Origin);

    //Done
    Track->second.Stream_Finish_Done=true;
}

//---------------------------------------------------------------------------
void File_Mxf::Streams_Finish_Essence(int32u EssenceUID, int128u TrackUID)
{
    essences::iterator Essence=Essences.find(EssenceUID);
    if (Essence==Essences.end() || Essence->second.Stream_Finish_Done)
        return;

    if (Essence->second.Parsers.size()!=1)
        return;
    parsers::iterator Parser=Essence->second.Parsers.begin();

    //Descriptive Metadata
    std::vector<int128u> DMScheme1s_List;
    int32u TrackID=(int32u)-1;
    tracks::iterator Track=Tracks.find(TrackUID);
    if (Track!=Tracks.end())
        TrackID=Track->second.TrackID;

    for (dmsegments::iterator DMSegment=DMSegments.begin(); DMSegment!=DMSegments.end(); ++DMSegment)
        for (size_t Pos=0; Pos<DMSegment->second.TrackIDs.size(); Pos++)
            if (DMSegment->second.TrackIDs[Pos]==TrackID)
                DMScheme1s_List.push_back(DMSegment->second.Framework);

    if (Config->ParseSpeed<1.0 && !(*Parser)->Status[IsFinished])
    {
        Fill(*Parser);
        (*Parser)->Open_Buffer_Unsynch();
    }
    Finish(*Parser);
    StreamKind_Last=Stream_Max;
    if ((*Parser)->Count_Get(Stream_Video))
        Stream_Prepare(Stream_Video);
    else if ((*Parser)->Count_Get(Stream_Audio))
        Stream_Prepare(Stream_Audio);
    else if ((*Parser)->Count_Get(Stream_Text))
        Stream_Prepare(Stream_Text);
    else if ((*Parser)->Count_Get(Stream_Other))
        Stream_Prepare(Stream_Other);
    else if (Essence->second.StreamKind!=Stream_Max)
        Stream_Prepare(Essence->second.StreamKind);
    else
    {
        for (descriptors::iterator Descriptor=Descriptors.begin(); Descriptor!=Descriptors.end(); ++Descriptor)
            if (Descriptor->second.LinkedTrackID==Essence->second.TrackID)
            {
                if (Descriptor->second.StreamKind!=Stream_Max)
                {
                    Stream_Prepare(Descriptor->second.StreamKind);
                    Descriptor->second.StreamPos=StreamPos_Last;
                }
                break;
            }
        if (StreamKind_Last==Stream_Max)
            return; //Not found
    }

    for (descriptors::iterator Descriptor=Descriptors.begin(); Descriptor!=Descriptors.end(); ++Descriptor)
        if (Descriptor->second.LinkedTrackID==Essence->second.TrackID)
        {
            if (Descriptor->second.StreamKind!=Stream_Max)
                Descriptor->second.StreamPos=StreamPos_Last;
            break;
        }

    for (std::map<std::string, Ztring>::iterator Info=Essence->second.Infos.begin(); Info!=Essence->second.Infos.end(); ++Info)
        Fill(StreamKind_Last, StreamPos_Last, Info->first.c_str(), Info->second, true);
    if (TimeCode_RoundedTimecodeBase && TimeCode_StartTimecode!=(int64u)-1)
    {
        float64 TimeCode_StartTimecode_Temp=((float64)(TimeCode_StartTimecode+Config->File_IgnoreFramesBefore))/TimeCode_RoundedTimecodeBase;
        if (TimeCode_DropFrame)
        {
            TimeCode_StartTimecode_Temp*=1001;
            TimeCode_StartTimecode_Temp/=1000;
        }
        Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Delay), TimeCode_StartTimecode_Temp*1000, 0, true);
        Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Delay_Source), "Container");
        Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Delay_DropFrame), TimeCode_DropFrame?"Yes":"No");

        //TimeCode TC(TimeCode_StartTimecode, TimeCode_RoundedTimecodeBase, TimeCode_DropFrame);
        //Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_TimeCode_FirstFrame), TC.ToString().c_str());
        //Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_TimeCode_Source), "Time code track (stripped)");
    }
    if (SDTI_TimeCode_StartTimecode_ms!=(int64u)-1)
    {
        Fill(StreamKind_Last, StreamPos_Last, "Delay_SDTI", SDTI_TimeCode_StartTimecode_ms);
        if (StreamKind_Last!=Stream_Max)
            (*Stream_More)[StreamKind_Last][StreamPos_Last](Ztring().From_Local("Delay_SDTI"), Info_Options)=__T("N NT");

        //Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_TimeCode_FirstFrame), SDTI_TimeCode_StartTimecode.c_str());
        //Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_TimeCode_Source), "SDTI");
    }
    if (SystemScheme1_TimeCodeArray_StartTimecode_ms!=(int64u)-1)
    {
        Fill(StreamKind_Last, StreamPos_Last, "Delay_SystemScheme1", SystemScheme1_TimeCodeArray_StartTimecode_ms);
        if (StreamKind_Last!=Stream_Max)
            (*Stream_More)[StreamKind_Last][StreamPos_Last](Ztring().From_Local("Delay_SystemScheme1"), Info_Options)=__T("N NT");

        //Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_TimeCode_FirstFrame), SystemScheme1_TimeCodeArray_StartTimecode.c_str());
        //Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_TimeCode_Source), "System scheme 1");
    }

    //Special case - Multiple sub-streams in a stream
    if (((*Parser)->Retrieve(Stream_General, 0, General_Format)==__T("ChannelGrouping") || (*Parser)->Count_Get(StreamKind_Last)>1) && (*Parser)->Count_Get(Stream_Audio))
    {
        //Before
        if (StreamKind_Last==Stream_Audio)
        {
            Clear(Stream_Audio, StreamPos_Last, Audio_Format_Settings_Sign);
        }
        ZtringList StreamSave; StreamSave.Write((*File__Analyze::Stream)[StreamKind_Last][StreamPos_Last].Read());
        ZtringListList StreamMoreSave; StreamMoreSave.Write((*Stream_More)[StreamKind_Last][StreamPos_Last].Read());

        //Erasing former streams data
        stream_t NewKind=StreamKind_Last;
        size_t NewPos1;
        Ztring ID;
        if ((*Parser)->Retrieve(Stream_General, 0, General_Format)==__T("ChannelGrouping"))
        {
            //Searching second stream
            size_t StreamPos_Difference=Essence->second.StreamPos-Essence->second.StreamPos_Initial;
            essences::iterator Essence1=Essence;
            --Essence1;
            Essence->second.StreamPos=Essence1->second.StreamPos;
            for (descriptors::iterator Descriptor=Descriptors.begin(); Descriptor!=Descriptors.end(); ++Descriptor)
            {
                if (Descriptor->second.LinkedTrackID==Essence1->second.TrackID)
                    Descriptor->second.StreamPos=Essence1->second.StreamPos;
                if (Descriptor->second.LinkedTrackID==Essence->second.TrackID)
                    Descriptor->second.StreamPos=Essence->second.StreamPos;
            }

            //Removing the 2 corresponding streams
            NewPos1=(Essence->second.StreamPos_Initial/2)*2+StreamPos_Difference;
            size_t NewPos2=NewPos1+1;
            ID=Ztring::ToZtring(Essence1->second.TrackID)+__T(" / ")+Ztring::ToZtring(Essence->second.TrackID);

            Stream_Erase(NewKind, NewPos2);
            Stream_Erase(NewKind, NewPos1);
        }
        else
        {
            NewPos1=StreamPos_Last;
            ID=Ztring::ToZtring(Essence->second.TrackID);
            Stream_Erase(NewKind, NewPos1);
        }

        //After
        for (size_t StreamPos=0; StreamPos<(*Parser)->Count_Get(NewKind); StreamPos++)
        {
            Stream_Prepare(NewKind, NewPos1+StreamPos);
            Merge(*(*Parser), StreamKind_Last, StreamPos, StreamPos_Last);
            Ztring Parser_ID=Retrieve(StreamKind_Last, StreamPos_Last, General_ID);
            Fill(StreamKind_Last, StreamPos_Last, General_ID, ID+(Parser_ID.empty()?Ztring():(__T("-")+Parser_ID)), true);
            for (size_t Pos=0; Pos<StreamSave.size(); Pos++)
            {
                if (Pos==Fill_Parameter(StreamKind_Last, Generic_BitRate) && (*Parser)->Count_Get(NewKind)>1 && (!StreamSave[Pos].empty() || StreamPos))
                    Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_BitRate_Encoded), StreamPos?0:(StreamSave[Pos].To_int64u()*2));
                else if (Pos==Fill_Parameter(StreamKind_Last, Generic_StreamSize) && (*Parser)->Count_Get(NewKind)>1 && (!StreamSave[Pos].empty() || StreamPos))
                    Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_StreamSize_Encoded), StreamPos?0:(StreamSave[Pos].To_int64u()*2));
                else if (Retrieve(StreamKind_Last, StreamPos_Last, Pos).empty())
                    Fill(StreamKind_Last, StreamPos_Last, Pos, StreamSave[Pos]);
            }
            for (size_t Pos=0; Pos<StreamMoreSave.size(); Pos++)
            {
                Fill(StreamKind_Last, StreamPos_Last, StreamMoreSave(Pos, 0).To_Local().c_str(), StreamMoreSave(Pos, 1));
                if (StreamMoreSave(Pos, Info_Name)==__T("Delay_SDTI"))
                    (*Stream_More)[StreamKind_Last][StreamPos_Last](Ztring().From_Local("Delay_SDTI"), Info_Options)=__T("N NT");
                if (StreamMoreSave(Pos, Info_Name)==__T("Delay_SystemScheme1"))
                    (*Stream_More)[StreamKind_Last][StreamPos_Last](Ztring().From_Local("Delay_SystemScheme1"), Info_Options)=__T("N NT");
            }

            for (size_t Pos=0; Pos<DMScheme1s_List.size(); Pos++)
            {
                dmscheme1s::iterator DMScheme1=DMScheme1s.find(DMScheme1s_List[Pos]);
                if (DMScheme1!=DMScheme1s.end())
                {
                    Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Language), DMScheme1->second.PrimaryExtendedSpokenLanguage, true);
                }
            }
        }

        //Positioning other streams
        for (essences::iterator Essence_Temp=Essence; Essence_Temp!=Essences.end(); ++Essence_Temp)
            if (*(Essence_Temp->second.Parsers.begin()) && (*(Essence_Temp->second.Parsers.begin()))->Count_Get(Stream_Audio))
            {
                Essence_Temp->second.StreamPos-=2; //ChannelGrouping
                Essence_Temp->second.StreamPos+=(*(Essence_Temp->second.Parsers.begin()))->Count_Get(Stream_Audio);
            }
    }
    else //Normal
    {
        //From descriptor
        if ((*Parser)->Retrieve(Stream_Audio, StreamPos_Last, Audio_Format)==__T("PCM")) // MXF handles channel count only with PCM, not with compressed data
            for (descriptors::iterator Descriptor=Descriptors.begin(); Descriptor!=Descriptors.end(); ++Descriptor)
                if (Descriptor->second.LinkedTrackID==Essence->second.TrackID && Descriptor->second.StreamKind==Stream_Audio && StreamKind_Last==Stream_Audio && Descriptor->second.ChannelCount!=(int32u)-1)
                {
                    Fill(Stream_Audio, StreamPos_Last, Audio_Channel_s_, Descriptor->second.ChannelCount);
                    break;
                }

        Merge(*(*Parser), StreamKind_Last, 0, StreamPos_Last);

        for (size_t Pos=0; Pos<DMScheme1s_List.size(); Pos++)
        {
            dmscheme1s::iterator DMScheme1=DMScheme1s.find(DMScheme1s_List[Pos]);
            if (DMScheme1!=DMScheme1s.end())
            {
                Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Language), DMScheme1->second.PrimaryExtendedSpokenLanguage, true);
            }
        }

        for (size_t StreamPos=1; StreamPos<(*Parser)->Count_Get(StreamKind_Last); StreamPos++) //If more than 1 stream, TODO: better way to do this
        {
            Stream_Prepare(StreamKind_Last);
            Merge(*(*Parser), StreamKind_Last, StreamPos, StreamPos_Last);
        }
    }

    if (Retrieve(StreamKind_Last, StreamPos_Last, General_ID).empty() || StreamKind_Last==Stream_Text || StreamKind_Last==Stream_Other) //TODO: better way to do detect subID
    {
        //Looking for Material package TrackID
        int32u TrackID=(int32u)-1;
        for (packages::iterator SourcePackage=Packages.begin(); SourcePackage!=Packages.end(); ++SourcePackage)
            if (SourcePackage->second.PackageUID.hi.hi) //Looking fo a SourcePackage with PackageUID only
            {
                //Testing if the Track is in this SourcePackage
                for (size_t Tracks_Pos=0; Tracks_Pos<SourcePackage->second.Tracks.size(); Tracks_Pos++)
                    if (SourcePackage->second.Tracks[Tracks_Pos]==TrackUID)
                    {
                        tracks::iterator Track=Tracks.find(SourcePackage->second.Tracks[Tracks_Pos]);
                        if (Track!=Tracks.end())
                            TrackID=Track->second.TrackID;
                    }
            }

        Ztring ID;
        Ztring ID_String;
        if (TrackID!=(int32u)-1)
            ID=Ztring::ToZtring(TrackID);
        else if (Tracks[TrackUID].TrackID!=(int32u)-1)
            ID=Ztring::ToZtring(Tracks[TrackUID].TrackID);
        else
        {
            ID=Ztring::ToZtring(Essence->first);
            ID_String=Ztring::ToZtring(Essence->first, 16);
        }
        if (!ID.empty())
        {
            for (size_t StreamPos=StreamPos_Last-((*Parser)->Count_Get(StreamKind_Last)?((*Parser)->Count_Get(StreamKind_Last)-1):0); StreamPos<=StreamPos_Last; StreamPos++) //If more than 1 stream
            {
                Ztring ID_Temp(ID);
                if (!Retrieve(StreamKind_Last, StreamPos, General_ID).empty())
                {
                    ID_Temp+=__T("-");
                    ID_Temp+=Retrieve(StreamKind_Last, StreamPos, General_ID);
                }
                Fill(StreamKind_Last, StreamPos, General_ID, ID_Temp, true);
                if (!ID_String.empty())
                    Fill(StreamKind_Last, StreamPos, General_ID_String, ID_String, true);
            }
        }
        if (!Tracks[TrackUID].TrackName.empty())
        {
            for (size_t StreamPos=StreamPos_Last-((*Parser)->Count_Get(StreamKind_Last)?((*Parser)->Count_Get(StreamKind_Last)-1):0); StreamPos<=StreamPos_Last; StreamPos++) //If more than 1 stream
                Fill(StreamKind_Last, StreamPos, "Title", Tracks[TrackUID].TrackName);
        }
    }

    //Special case - DV
    #if defined(MEDIAINFO_DVDIF_YES)
        if (StreamKind_Last==Stream_Video && Retrieve(Stream_Video, StreamPos_Last, Video_Format)==__T("DV"))
        {
            if (Retrieve(Stream_General, 0, General_Recorded_Date).empty())
                Fill(Stream_General, 0, General_Recorded_Date, (*Parser)->Retrieve(Stream_General, 0, General_Recorded_Date));

            //Video and Audio are together
            size_t Audio_Count=(*Parser)->Count_Get(Stream_Audio);
            for (size_t Audio_Pos=0; Audio_Pos<Audio_Count; Audio_Pos++)
            {
                Fill_Flush();
                Stream_Prepare(Stream_Audio);
                size_t Pos=Count_Get(Stream_Audio)-1;
                (*Parser)->Finish();
                if (TimeCode_RoundedTimecodeBase && TimeCode_StartTimecode!=(int64u)-1)
                {
                    float64 TimeCode_StartTimecode_Temp=((float64)(TimeCode_StartTimecode+Config->File_IgnoreFramesBefore))/TimeCode_RoundedTimecodeBase;
                    if (TimeCode_DropFrame)
                    {
                        TimeCode_StartTimecode_Temp*=1001;
                        TimeCode_StartTimecode_Temp/=1000;
                    }
                    Fill(Stream_Audio, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Delay), TimeCode_StartTimecode_Temp*1000, 0, true);
                    Fill(Stream_Audio, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Delay_Source), "Container");
                }
                Merge(*(*Parser), Stream_Audio, Audio_Pos, StreamPos_Last);
                if (Retrieve(Stream_Audio, Pos, Audio_MuxingMode).empty())
                    Fill(Stream_Audio, Pos, Audio_MuxingMode, Retrieve(Stream_Video, Essence->second.StreamPos-(StreamPos_StartAtOne?1:0), Video_Format), true);
                else
                    Fill(Stream_Audio, Pos, Audio_MuxingMode, Retrieve(Stream_Video, Essence->second.StreamPos-(StreamPos_StartAtOne?1:0), Video_Format)+__T(" / ")+Retrieve(Stream_Audio, Pos, Audio_MuxingMode), true);
                Fill(Stream_Audio, Pos, Audio_Duration, Retrieve(Stream_Video, Essence->second.StreamPos-(StreamPos_StartAtOne?1:0), Video_Duration));
                Fill(Stream_Audio, Pos, Audio_StreamSize_Encoded, 0); //Included in the DV stream size
                Ztring ID=Retrieve(Stream_Audio, Pos, Audio_ID);
                Fill(Stream_Audio, Pos, Audio_ID, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_ID)+__T("-")+ID, true);
                Fill(Stream_Audio, Pos, Audio_ID_String, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_ID_String)+__T("-")+ID, true);
                Fill(Stream_Audio, Pos, Audio_Title, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_Title), true);
            }

            StreamKind_Last=Stream_Video;
            StreamPos_Last=Essence->second.StreamPos-(StreamPos_StartAtOne?1:0);
        }
    #endif

    //Special case - MPEG Video + Captions
    if (StreamKind_Last==Stream_Video && (*Parser)->Count_Get(Stream_Text))
    {
        //Video and Text are together
        size_t Parser_Text_Count=(*Parser)->Count_Get(Stream_Text);
        for (size_t Parser_Text_Pos=0; Parser_Text_Pos<Parser_Text_Count; Parser_Text_Pos++)
        {
            size_t StreamPos_Video=StreamPos_Last;
            Fill_Flush();
            Stream_Prepare(Stream_Text);
            (*Parser)->Finish();
            if (TimeCode_RoundedTimecodeBase && TimeCode_StartTimecode!=(int64u)-1)
            {
                float64 TimeCode_StartTimecode_Temp=((float64)(TimeCode_StartTimecode+Config->File_IgnoreFramesBefore))/TimeCode_RoundedTimecodeBase;
                if (TimeCode_DropFrame)
                {
                    TimeCode_StartTimecode_Temp*=1001;
                    TimeCode_StartTimecode_Temp/=1000;
                }
                Fill(Stream_Text, Parser_Text_Pos, Fill_Parameter(StreamKind_Last, Generic_Delay), TimeCode_StartTimecode_Temp*1000, 0, true);
                Fill(Stream_Text, Parser_Text_Pos, Fill_Parameter(StreamKind_Last, Generic_Delay_Source), "Container");
            }
            Merge(*(*Parser), Stream_Text, Parser_Text_Pos, StreamPos_Last);
            Fill(Stream_Text, StreamPos_Last, Text_Duration, Retrieve(Stream_Video, StreamPos_Video, Video_Duration));
            Ztring ID=Retrieve(Stream_Text, StreamPos_Last, Text_ID);
            if (Retrieve(Stream_Text, StreamPos_Last, Text_MuxingMode).find(__T("Ancillary"))!=string::npos)
            {
                for (descriptors::iterator Descriptor=Descriptors.begin(); Descriptor!=Descriptors.end(); ++Descriptor)
                    if (Descriptor->second.Type==descriptor::Type_AncPackets)
                    {
                        Fill(Stream_Text, StreamPos_Last, Text_ID, Ztring::ToZtring(Descriptor->second.LinkedTrackID)+__T("-")+ID, true);
                        Fill(Stream_Text, StreamPos_Last, Text_ID_String, Ztring::ToZtring(Descriptor->second.LinkedTrackID)+__T("-")+ID, true);
                        Fill(Stream_Text, StreamPos_Last, Text_Title, Tracks[TrackUID].TrackName, true);
                        break;
                    }
            }
            else
            {
                Fill(Stream_Text, StreamPos_Last, Text_ID, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_ID)+__T("-")+ID, true);
                Fill(Stream_Text, StreamPos_Last, Text_ID_String, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_ID_String)+__T("-")+ID, true);
                Fill(Stream_Text, StreamPos_Last, Text_Title, Retrieve(Stream_Video, Count_Get(Stream_Video)-1, Video_Title), true);
            }

            for (size_t Pos=0; Pos<DMScheme1s_List.size(); Pos++)
            {
                dmscheme1s::iterator DMScheme1=DMScheme1s.find(DMScheme1s_List[Pos]);
                if (DMScheme1!=DMScheme1s.end())
                {
                    Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Language), DMScheme1->second.PrimaryExtendedSpokenLanguage, true);
                }
            }
        }

        StreamKind_Last=Stream_Video;
        StreamPos_Last=Essence->second.StreamPos-(StreamPos_StartAtOne?1:0);
    }

    //Stream size
    if (StreamKind_Last!=Stream_Max && Count_Get(Stream_Video)+Count_Get(Stream_Audio)==1 && Essence->second.Stream_Size!=(int64u)-1)
    {
        //TODO: Stream_Size is present only if there is one stream, so it works in most cases. We should find a better way.
        int64u Stream_Size=Essence->second.Stream_Size;
        if (Config->File_IgnoreFramesBefore || Config->File_IgnoreFramesAfter!=(int64u)-1)
        {
            int64u FrameCount_FromComponent=(int64u)-1;
            for (components::iterator Component=Components.begin(); Component!=Components.end(); ++Component)
                if (FrameCount_FromComponent>Component->second.Duration)
                    FrameCount_FromComponent=Component->second.Duration;
            float64 EditRate_FromTrack=DBL_MAX;
            for (tracks::iterator Track=Tracks.begin(); Track!=Tracks.end(); ++Track)
                if (EditRate_FromTrack>Track->second.EditRate)
                    EditRate_FromTrack=Track->second.EditRate;
            if (FrameCount_FromComponent!=(int64u)-1 && FrameCount_FromComponent && EditRate_FromTrack!=DBL_MAX && EditRate_FromTrack)
            {
                int64u FrameCount=FrameCount_FromComponent;
                int64u File_IgnoreFramesBefore=Config->File_IgnoreFramesBefore;
                if (File_IgnoreFramesBefore && Config->File_IgnoreFramesRate && (EditRate_FromTrack<Config->File_IgnoreFramesRate*0.9 || EditRate_FromTrack>Config->File_IgnoreFramesRate*1.1)) //In case of problem or EditRate being sampling rate
                    File_IgnoreFramesBefore=float64_int64s(((float64)File_IgnoreFramesBefore)/Config->File_IgnoreFramesRate*EditRate_FromTrack);
                int64u File_IgnoreFramesAfter=Config->File_IgnoreFramesAfter;
                if (File_IgnoreFramesAfter!=(int64u)-1 && Config->File_IgnoreFramesRate && (EditRate_FromTrack<Config->File_IgnoreFramesRate*0.9 || EditRate_FromTrack>Config->File_IgnoreFramesRate*1.1)) //In case of problem or EditRate being sampling rate
                    File_IgnoreFramesAfter=float64_int64s(((float64)File_IgnoreFramesAfter)/Config->File_IgnoreFramesRate*EditRate_FromTrack);
                if (File_IgnoreFramesAfter<FrameCount)
                    FrameCount=File_IgnoreFramesAfter;
                if (FrameCount<File_IgnoreFramesBefore)
                    FrameCount=File_IgnoreFramesBefore;
                FrameCount-=File_IgnoreFramesBefore;

                float64 Stream_Size_Temp=(float64)Stream_Size;
                Stream_Size_Temp/=FrameCount_FromComponent;
                Stream_Size_Temp*=FrameCount;
                Stream_Size=float64_int64s(Stream_Size_Temp);
            }
        }

        Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_StreamSize), Stream_Size);
    }

    //Done
    Essence->second.Stream_Finish_Done=true;
}

//---------------------------------------------------------------------------
void File_Mxf::Streams_Finish_Descriptor(const int128u DescriptorUID, const int128u PackageUID)
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

    StreamKind_Last=Descriptor->second.StreamKind;
    StreamPos_Last=Descriptor->second.StreamPos;
    if (StreamPos_Last==(size_t)-1)
    {
        for (size_t Pos=0; Pos<Count_Get(StreamKind_Last); Pos++)
        {
            Ztring ID=Retrieve(StreamKind_Last, Pos, General_ID);
            size_t ID_Dash_Pos=ID.find(__T('-'));
            if (ID_Dash_Pos!=string::npos)
                ID.resize(ID_Dash_Pos);
            if (Ztring::ToZtring(Descriptor->second.LinkedTrackID)==ID)
            {
                StreamPos_Last=Pos;
                break;
            }
        }
    }
    if (StreamPos_Last==(size_t)-1)
    {
        if (Descriptors.size()==1)
            StreamPos_Last=0;
        else if (Descriptor->second.LinkedTrackID!=(int32u)-1)
        {
            //Workaround for a specific file with same ID
            if (!Locators.empty())
                for (descriptors::iterator Descriptor1=Descriptors.begin(); Descriptor1!=Descriptor; ++Descriptor1)
                    if (Descriptor1->second.LinkedTrackID==Descriptor->second.LinkedTrackID)
                    {
                        IdIsAlwaysSame_Offset++;
                        break;
                    }

            Stream_Prepare(Descriptor->second.StreamKind);
            Fill(StreamKind_Last, StreamPos_Last, General_ID, Descriptor->second.LinkedTrackID+IdIsAlwaysSame_Offset);
        }
        else
        {
            //Looking for Material package TrackID
            packages::iterator SourcePackage=Packages.find(PackageUID);
            //We have the the right PackageUID, looking for SourceClip from Sequence from Track from MaterialPackage
            for (components::iterator SourceClip=Components.begin(); SourceClip!=Components.end(); ++SourceClip)
                if (SourceClip->second.SourcePackageID.lo==SourcePackage->second.PackageUID.lo) //int256u doesn't support yet ==
                {
                    //We have the right SourceClip, looking for the Sequence from Track from MaterialPackage
                    for (components::iterator Sequence=Components.begin(); Sequence!=Components.end(); ++Sequence)
                        for (size_t StructuralComponents_Pos=0; StructuralComponents_Pos<Sequence->second.StructuralComponents.size(); StructuralComponents_Pos++)
                            if (Sequence->second.StructuralComponents[StructuralComponents_Pos]==SourceClip->first)
                            {
                                //We have the right Sequence, looking for Track from MaterialPackage
                                for (tracks::iterator Track=Tracks.begin(); Track!=Tracks.end(); ++Track)
                                {
                                    if (Track->second.Sequence==Sequence->first)
                                    {
                                        Ztring ID=Ztring::ToZtring(Track->second.TrackID);
                                        StreamKind_Last=Stream_Max;
                                        StreamPos_Last=(size_t)-1;
                                        for (size_t StreamKind=Stream_General+1; StreamKind<Stream_Max; StreamKind++)
                                            for (size_t StreamPos=0; StreamPos<Count_Get((stream_t)StreamKind); StreamPos++)
                                                if (ID==Retrieve((stream_t)StreamKind, StreamPos, General_ID))
                                                {
                                                    StreamKind_Last=(stream_t)StreamKind;
                                                    StreamPos_Last=(stream_t)StreamPos;
                                                }
                                        if (StreamPos_Last==(size_t)-1 && !Descriptor->second.Locators.empty()) //TODO: 1 file has a TimeCode stream linked to a video stream, and it is displayed if Locator test is removed. Why? AS02 files streams are not filled if I remove completely this block, why?
                                        {
                                            if (Descriptor->second.StreamKind!=Stream_Max)
                                                Stream_Prepare(Descriptor->second.StreamKind);
                                            if (Track->second.TrackID!=(int32u)-1)
                                            {
                                                if (Descriptor->second.LinkedTrackID==(int32u)-1)
                                                    Descriptor->second.LinkedTrackID=Track->second.TrackID;
                                                if (Descriptor->second.StreamKind!=Stream_Max)
                                                {
                                                    Fill(StreamKind_Last, StreamPos_Last, General_ID, ID);
                                                    Fill(StreamKind_Last, StreamPos_Last, "Title", Track->second.TrackName);
                                                }
                                            }
                                        }
                                    }
                                }
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
        Streams_Finish_Locator(DescriptorUID, Descriptor->second.Locators[Locator_Pos]);
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
        //Handling buggy files
        if (Descriptor->second.ScanType==__T("Interlaced") && Descriptor->second.Height==1152 && Descriptor->second.Height_Display==1152 && Descriptor->second.Width==720) //Height value is height of the frame instead of the field
            Descriptor->second.Height_Display/=2;

        //ID
        if (Descriptor->second.LinkedTrackID!=(int32u)-1 && Retrieve(StreamKind_Last, StreamPos_Last, General_ID).empty())
        {
            for (size_t StreamKind=0; StreamKind<Stream_Max; StreamKind++)
                for (size_t StreamPos=Before_Count[StreamKind]; StreamPos<Count_Get((stream_t)StreamKind); StreamPos++)
                {
                    Ztring ID=Retrieve((stream_t)StreamKind, StreamPos, General_ID);
                    if (ID.empty() || Config->File_ID_OnlyRoot_Get())
                        Fill((stream_t)StreamKind, StreamPos, General_ID, Descriptor->second.LinkedTrackID, 10, true);
                    else
                        Fill((stream_t)StreamKind, StreamPos, General_ID, Ztring::ToZtring(Descriptor->second.LinkedTrackID)+ID, true);
                }
        }

        if (Descriptor->second.Width!=(int32u)-1 && Retrieve(Stream_Video, StreamPos_Last, Video_Width).empty())
            Fill(Stream_Video, StreamPos_Last, Video_Width, Descriptor->second.Width, 10, true);
        if (Descriptor->second.Width_Display!=(int32u)-1 && Descriptor->second.Width_Display!=Retrieve(Stream_Video, StreamPos_Last, Video_Width).To_int32u())
        {
            Fill(Stream_Video, StreamPos_Last, Video_Width_Original, Retrieve(Stream_Video, StreamPos_Last, Video_Width), true);
            if (Retrieve(Stream_Video, StreamPos_Last, Video_PixelAspectRatio_Original).empty())
                Fill(Stream_Video, StreamPos_Last, Video_PixelAspectRatio_Original, Retrieve(Stream_Video, StreamPos_Last, Video_PixelAspectRatio), true);
            Clear(Stream_Video, StreamPos_Last, Video_PixelAspectRatio);
            Fill(Stream_Video, StreamPos_Last, Video_Width, Descriptor->second.Width_Display, 10, true);
            if (Descriptor->second.Width_Display_Offset!=(int32u)-1)
                Fill(Stream_Video, StreamPos_Last, Video_Width_Offset, Descriptor->second.Width_Display_Offset, 10, true);
        }
        if (Descriptor->second.Height!=(int32u)-1 && Retrieve(Stream_Video, StreamPos_Last, Video_Height).empty())
            Fill(Stream_Video, StreamPos_Last, Video_Height, Descriptor->second.Height, 10, true);
        if (Descriptor->second.Height_Display!=(int32u)-1 && Descriptor->second.Height_Display!=Retrieve(Stream_Video, StreamPos_Last, Video_Height).To_int32u())
        {
            Fill(Stream_Video, StreamPos_Last, Video_Height_Original, Retrieve(Stream_Video, StreamPos_Last, Video_Height), true);
            if (Retrieve(Stream_Video, StreamPos_Last, Video_PixelAspectRatio_Original).empty())
                Fill(Stream_Video, StreamPos_Last, Video_PixelAspectRatio_Original, Retrieve(Stream_Video, StreamPos_Last, Video_PixelAspectRatio), true);
            Clear(Stream_Video, StreamPos_Last, Video_PixelAspectRatio);
            Fill(Stream_Video, StreamPos_Last, Video_Height, Descriptor->second.Height_Display, 10, true);
            if (Descriptor->second.Height_Display_Offset!=(int32u)-1)
                Fill(Stream_Video, StreamPos_Last, Video_Height_Offset, Descriptor->second.Height_Display_Offset, 10, true);
        }

        //Info
        const Ztring &ID=Retrieve(StreamKind_Last, StreamPos_Last, General_ID);
        size_t ID_Dash_Pos=ID.find(__T('-'));
        size_t StreamWithSameID=1;
        if (ID_Dash_Pos!=(size_t)-1)
        {
            Ztring RealID=ID.substr(0, ID_Dash_Pos+1);
            while (StreamPos_Last+StreamWithSameID<Count_Get(StreamKind_Last) && Retrieve(StreamKind_Last, StreamPos_Last+StreamWithSameID, General_ID).find(RealID)==0)
                StreamWithSameID++;
        }
        if (Descriptor->second.SampleRate && StreamKind_Last==Stream_Video)
        {
            float64 SampleRate=Descriptor->second.SampleRate;
            if (StereoscopicPictureSubDescriptor_IsPresent)
            {
                SampleRate/=2;
                Fill(Stream_Video, StreamPos_Last, Video_MultiView_Count, 2, 10, true);
            }
            for (essences::iterator Essence=Essences.begin(); Essence!=Essences.end(); ++Essence)
                if (Essence->second.StreamKind==Stream_Video && Essence->second.StreamPos-(StreamPos_StartAtOne?1:0)==StreamPos_Last)
                {
                    if (Essence->second.Field_Count_InThisBlock_1 && !Essence->second.Field_Count_InThisBlock_2)
                        SampleRate/=2;
                    break;
                }
            Ztring SampleRate_Container; SampleRate_Container.From_Number(SampleRate); //TODO: fill frame rate before the merge with raw stream information
            const Ztring &SampleRate_RawStream=Retrieve(Stream_Video, StreamPos_Last, Video_FrameRate);
            if (!SampleRate_RawStream.empty() && SampleRate_Container!=SampleRate_RawStream)
                Fill(Stream_Video, StreamPos_Last, Video_FrameRate_Original, SampleRate_RawStream);
            Fill(Stream_Video, StreamPos_Last, Video_FrameRate, SampleRate, 3, true);
        }
        for (std::map<std::string, Ztring>::iterator Info=Descriptor->second.Infos.begin(); Info!=Descriptor->second.Infos.end(); ++Info)
            if (Retrieve(StreamKind_Last, StreamPos_Last, Info->first.c_str()).empty())
            {
                //Special case
                if (Info->first=="BitRate" && Retrieve(StreamKind_Last, StreamPos_Last, General_ID).find(__T(" / "))!=string::npos)
                {
                    if (Retrieve(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_BitRate)).empty() || Retrieve(StreamKind_Last, StreamPos_Last, General_ID).find(__T("-"))!=string::npos)
                        Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_BitRate_Encoded), Info->second.To_int64u()*2, 10, true);
                    else
                        Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_BitRate), Info->second.To_int64u()*2, 10, true);
                }
                else
                {
                    for (size_t Pos=0; Pos<StreamWithSameID; Pos++)
                        Fill(StreamKind_Last, StreamPos_Last+Pos, Info->first.c_str(), Info->second, true);
                }
            }
        Ztring Format, CodecID;
        if (Descriptor->second.EssenceContainer.hi!=(int64u)-1)
        {
            CodecID.From_Number(Descriptor->second.EssenceContainer.lo, 16);
            if (CodecID.size()<16)
                CodecID.insert(0, 16-CodecID.size(), __T('0'));
            Format.From_Local(Mxf_EssenceContainer(Descriptor->second.EssenceContainer));
        }
        if (Descriptor->second.EssenceCompression.hi!=(int64u)-1)
        {
            if (!CodecID.empty())
                CodecID+=__T('-');
            Ztring EssenceCompression;
            EssenceCompression.From_Number(Descriptor->second.EssenceCompression.lo, 16);
            if (EssenceCompression.size()<16)
                EssenceCompression.insert(0, 16-EssenceCompression.size(), __T('0'));
            CodecID+=EssenceCompression;
            Ztring Format_FromCompression; Format_FromCompression.From_Local(Mxf_EssenceCompression(Descriptor->second.EssenceCompression));
            if (!Format_FromCompression.empty())
                Format=Format_FromCompression; //EssenceCompression has priority
        }
        if (!CodecID.empty())
            for (size_t Pos=0; Pos<StreamWithSameID; Pos++)
                Fill(StreamKind_Last, StreamPos_Last+Pos, Fill_Parameter(StreamKind_Last, Generic_CodecID), CodecID, true);
        if (!Format.empty())
            for (size_t Pos=0; Pos<StreamWithSameID; Pos++)
                if (Retrieve(StreamKind_Last, StreamPos_Last+Pos, Fill_Parameter(StreamKind_Last, Generic_Format)).empty())
                    Fill(StreamKind_Last, StreamPos_Last+Pos, Fill_Parameter(StreamKind_Last, Generic_Format), Format);

        //Bitrate (PCM)
        if (StreamKind_Last==Stream_Audio && Retrieve(Stream_Audio, StreamPos_Last, Audio_BitRate).empty() && Retrieve(Stream_Audio, StreamPos_Last, Audio_Format)==__T("PCM") && Retrieve(Stream_Audio, StreamPos_Last, Audio_Format_Settings_Wrapping).find(__T("D-10"))!=string::npos)
        {
            int64u SamplingRate=Retrieve(Stream_Audio, StreamPos_Last, Audio_SamplingRate).To_int64u();
            if (SamplingRate)
               Fill(Stream_Audio, StreamPos_Last, Audio_BitRate, 8*SamplingRate*32);
        }
        if (StreamKind_Last==Stream_Audio && Retrieve(Stream_Audio, StreamPos_Last, Audio_BitRate).empty() && Retrieve(Stream_Audio, StreamPos_Last, Audio_Format)==__T("PCM"))
        {
            int64u Channels=Retrieve(Stream_Audio, StreamPos_Last, Audio_Channel_s_).To_int64u();
            int64u SamplingRate=Retrieve(Stream_Audio, StreamPos_Last, Audio_SamplingRate).To_int64u();
            int64u Resolution=Retrieve(Stream_Audio, StreamPos_Last, Audio_BitDepth).To_int64u();
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
            float32 Width =Retrieve(Stream_Video, StreamPos_Last, Video_Width             ).To_float32();
            float32 Height=Retrieve(Stream_Video, StreamPos_Last, Video_Height            ).To_float32();
            float32 DAR_F =DAR.To_float32();
            if (Width && Height && DAR_F)
            {
                float32 PAR   =1/(Width/Height/DAR_F);
                if (PAR>(float32)12/(float32)11*0.99 && PAR<(float32)12/(float32)11*1.01)
                    PAR=(float32)12/(float32)11;
                if (PAR>(float32)10/(float32)11*0.99 && PAR<(float32)10/(float32)11*1.01)
                    PAR=(float32)10/(float32)11;
                if (PAR>(float32)16/(float32)11*0.99 && PAR<(float32)16/(float32)11*1.01)
                    PAR=(float32)16/(float32)11;
                if (PAR>(float32)40/(float32)33*0.99 && PAR<(float32)40/(float32)33*1.01)
                    PAR=(float32)40/(float32)33;
                if (PAR>(float32)24/(float32)11*0.99 && PAR<(float32)24/(float32)11*1.01)
                    PAR=(float32)24/(float32)11;
                if (PAR>(float32)20/(float32)11*0.99 && PAR<(float32)20/(float32)11*1.01)
                    PAR=(float32)20/(float32)11;
                if (PAR>(float32)32/(float32)11*0.99 && PAR<(float32)32/(float32)11*1.01)
                    PAR=(float32)32/(float32)11;
                if (PAR>(float32)80/(float32)33*0.99 && PAR<(float32)80/(float32)33*1.01)
                    PAR=(float32)80/(float32)33;
                if (PAR>(float32)18/(float32)11*0.99 && PAR<(float32)18/(float32)11*1.01)
                    PAR=(float32)18/(float32)11;
                if (PAR>(float32)15/(float32)11*0.99 && PAR<(float32)15/(float32)11*1.01)
                    PAR=(float32)15/(float32)11;
                if (PAR>(float32)64/(float32)33*0.99 && PAR<(float32)64/(float32)33*1.01)
                    PAR=(float32)64/(float32)33;
                if (PAR>(float32)160/(float32)99*0.99 && PAR<(float32)160/(float32)99*1.01)
                    PAR=(float32)160/(float32)99;
                if (PAR>(float32)4/(float32)3*0.99 && PAR<(float32)4/(float32)3*1.01)
                    PAR=(float32)4/(float32)3;
                if (PAR>(float32)3/(float32)2*0.99 && PAR<(float32)3/(float32)2*1.01)
                    PAR=(float32)3/(float32)2;
                if (PAR>(float32)2/(float32)1*0.99 && PAR<(float32)2/(float32)1*1.01)
                    PAR=(float32)2;
                if (PAR>(float32)59/(float32)54*0.99 && PAR<(float32)59/(float32)54*1.01)
                    PAR=(float32)59/(float32)54;
            }
        }

        //ActiveFormatDescriptor
        if (StreamKind_Last==Stream_Video && Descriptor->second.ActiveFormat!=(int8u)-1 && Retrieve(Stream_Video, StreamPos_Last, Video_ActiveFormatDescription).empty())
        {
            Fill(Stream_Video, 0, Video_ActiveFormatDescription, Descriptor->second.ActiveFormat);
            if (Descriptor->second.ActiveFormat<16)
            {
                float32 DAR=Retrieve(Stream_Video, StreamPos_Last, Video_DisplayAspectRatio).To_float32();
                if (DAR>(float32)4/(float32)3*0.99 && DAR<(float32)4/(float32)3*1.01)
                    Fill(Stream_Video, 0, Video_ActiveFormatDescription_String, AfdBarData_active_format_4_3[Descriptor->second.ActiveFormat]);
                if (DAR>(float32)16/(float32)9*0.99 && DAR<(float32)16/(float32)9*1.01)
                    Fill(Stream_Video, 0, Video_ActiveFormatDescription_String, AfdBarData_active_format_16_9[Descriptor->second.ActiveFormat]);
            }
            if (Retrieve(Stream_Video, 0, Video_ActiveFormatDescription_String).empty())
                Fill(Stream_Video, 0, Video_ActiveFormatDescription_String, Descriptor->second.ActiveFormat);
        }

        //ScanType / ScanOrder
        if (StreamKind_Last==Stream_Video && Retrieve(Stream_Video, StreamPos_Last, Video_ScanType_Original).empty())
        {
            //ScanType
            if (!Descriptor->second.ScanType.empty() && (Descriptor->second.ScanType!=Retrieve(Stream_Video, StreamPos_Last, Video_ScanType) && !(Descriptor->second.ScanType==__T("Interlaced") && Retrieve(Stream_Video, StreamPos_Last, Video_ScanType)==__T("MBAFF"))))
            {
                Fill(Stream_Video, StreamPos_Last, Video_ScanType_Original, Retrieve(Stream_Video, StreamPos_Last, Video_ScanType));
                Fill(Stream_Video, StreamPos_Last, Video_ScanType, Descriptor->second.ScanType, true);
            }

            //ScanOrder
            Ztring ScanOrder_Temp;
            if ((Descriptor->second.FieldDominance==1 && Descriptor->second.FieldTopness==1) || (Descriptor->second.FieldDominance!=1 && Descriptor->second.FieldTopness==2))
                ScanOrder_Temp.From_UTF8("TFF");
            if ((Descriptor->second.FieldDominance==1 && Descriptor->second.FieldTopness==2) || (Descriptor->second.FieldDominance!=1 && Descriptor->second.FieldTopness==1))
                    ScanOrder_Temp.From_UTF8("BFF");
            if ((!ScanOrder_Temp.empty() && ScanOrder_Temp!=Retrieve(Stream_Video, StreamPos_Last, Video_ScanOrder)) || !Retrieve(Stream_Video, StreamPos_Last, Video_ScanType_Original).empty())
            {
                Fill(Stream_Video, StreamPos_Last, Video_ScanOrder_Original, Retrieve(Stream_Video, StreamPos_Last, Video_ScanOrder), true);
                if (ScanOrder_Temp.empty())
                {
                    Clear(Stream_Video, StreamPos_Last, Video_ScanOrder);
                    Clear(Stream_Video, StreamPos_Last, Video_ScanOrder_String);
                }
                else
                    Fill(Stream_Video, StreamPos_Last, Video_ScanOrder, ScanOrder_Temp, true);
            }
        }
    }

    //Fallback on partition data if classic methods failed
    if (StreamKind_Last!=Stream_Max && StreamPos_Last!=(size_t)-1 && Retrieve(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Format)).empty() && Descriptors.size()==1 && Count_Get(StreamKind_Last)==1)
        Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Format), Mxf_EssenceContainer(EssenceContainer_FromPartitionMetadata));
}

//---------------------------------------------------------------------------
void File_Mxf::Streams_Finish_Locator(const int128u DescriptorUID, const int128u LocatorUID)
{
    descriptors::iterator Descriptor=Descriptors.find(DescriptorUID);
    if (Descriptor==Descriptors.end())
        return;

    locators::iterator Locator=Locators.find(LocatorUID);
    if (Locator==Locators.end())
        return;

    //External file name specific
    if (!Locator->second.IsTextLocator && !Locator->second.EssenceLocator.empty())
    {
        //Preparing
        Locator->second.StreamKind=StreamKind_Last;
        Locator->second.StreamPos=StreamPos_Last;
        Locator->second.LinkedTrackID=Descriptor->second.LinkedTrackID;
    }
}

//---------------------------------------------------------------------------
void File_Mxf::Streams_Finish_CommercialNames ()
{
    //Commercial names
    if (Count_Get(Stream_Video)==1)
    {
        Streams_Finish_StreamOnly();
             if (!Retrieve(Stream_Video, 0, Video_Format_Commercial_IfAny).empty())
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, Retrieve(Stream_Video, 0, Video_Format_Commercial_IfAny));
            Fill(Stream_General, 0, General_Format_Commercial, __T("MXF ")+Retrieve(Stream_Video, 0, Video_Format_Commercial_IfAny));
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==__T("DV"))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "DV");
            Fill(Stream_General, 0, General_Format_Commercial, "MXF DV");
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==__T("AVC") && Retrieve(Stream_Video, 0, Video_Format_Settings_GOP)==__T("N=1") && Retrieve(Stream_Video, 0, Video_Colorimetry)==__T("4:2:0") && Retrieve(Stream_Video, 0, Video_BitRate)==__T("56064000"))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "AVC-Intra 50");
            Fill(Stream_General, 0, General_Format_Commercial, "MXF AVC-Intra 50");
            Fill(Stream_Video, 0, Video_Format_Commercial_IfAny, "AVC-Intra 50");
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==__T("AVC") && Retrieve(Stream_Video, 0, Video_Format_Settings_GOP)==__T("N=1") && Retrieve(Stream_Video, 0, Video_Colorimetry)==__T("4:2:2") && Retrieve(Stream_Video, 0, Video_BitRate)==__T("113664000"))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "AVC-Intra 100");
            Fill(Stream_General, 0, General_Format_Commercial, "MXF AVC-Intra 100");
            Fill(Stream_Video, 0, Video_Format_Commercial_IfAny, "AVC-Intra 100");
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==__T("MPEG Video") && Retrieve(Stream_Video, 0, Video_Format_Settings_GOP)==__T("N=1") && Retrieve(Stream_Video, 0, Video_Colorimetry)==__T("4:2:2") && (Retrieve(Stream_Video, 0, Video_BitRate)==__T("30000000") || Retrieve(Stream_Video, 0, Video_BitRate_Nominal)==__T("30000000") || Retrieve(Stream_Video, 0, Video_BitRate_Maximum)==__T("30000000")))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "IMX 30");
            Fill(Stream_Video, 0, Video_Format_Commercial_IfAny, "IMX 30");
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==__T("MPEG Video") && Retrieve(Stream_Video, 0, Video_Format_Settings_GOP)==__T("N=1") && Retrieve(Stream_Video, 0, Video_Colorimetry)==__T("4:2:2") && (Retrieve(Stream_Video, 0, Video_BitRate)==__T("40000000") || Retrieve(Stream_Video, 0, Video_BitRate_Nominal)==__T("40000000") || Retrieve(Stream_Video, 0, Video_BitRate_Maximum)==__T("40000000")))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "IMX 40");
            Fill(Stream_Video, 0, Video_Format_Commercial_IfAny, "IMX 40");
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==__T("MPEG Video") && Retrieve(Stream_Video, 0, Video_Format_Settings_GOP)==__T("N=1") && Retrieve(Stream_Video, 0, Video_Colorimetry)==__T("4:2:2") && (Retrieve(Stream_Video, 0, Video_BitRate)==__T("50000000") || Retrieve(Stream_Video, 0, Video_BitRate_Nominal)==__T("50000000") || Retrieve(Stream_Video, 0, Video_BitRate_Maximum)==__T("50000000")))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "IMX 50");
            Fill(Stream_Video, 0, Video_Format_Commercial_IfAny, "IMX 50");
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==__T("MPEG Video") && !Retrieve(Stream_Video, 0, Video_Format_Settings_GOP).empty() && Retrieve(Stream_Video, 0, Video_Format_Settings_GOP)!=__T("N=1") && Retrieve(Stream_Video, 0, Video_Colorimetry)==__T("4:2:0") && (Retrieve(Stream_Video, 0, Video_BitRate)==__T("18000000") || Retrieve(Stream_Video, 0, Video_BitRate_Nominal)==__T("18000000") || Retrieve(Stream_Video, 0, Video_BitRate_Maximum)==__T("18000000")))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "XDCAM HD 18");
            Fill(Stream_Video, 0, Video_Format_Commercial_IfAny, "XDCAM HD 18");
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==__T("MPEG Video") && !Retrieve(Stream_Video, 0, Video_Format_Settings_GOP).empty() && Retrieve(Stream_Video, 0, Video_Format_Settings_GOP)!=__T("N=1") && Retrieve(Stream_Video, 0, Video_Colorimetry)==__T("4:2:0") && (Retrieve(Stream_Video, 0, Video_BitRate)==__T("25000000") || Retrieve(Stream_Video, 0, Video_BitRate_Nominal)==__T("25000000") || Retrieve(Stream_Video, 0, Video_BitRate_Maximum)==__T("25000000")))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "XDCAM HD 25");
            Fill(Stream_Video, 0, Video_Format_Commercial_IfAny, "XDCAM HD 25");
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==__T("MPEG Video") && !Retrieve(Stream_Video, 0, Video_Format_Settings_GOP).empty() && Retrieve(Stream_Video, 0, Video_Format_Settings_GOP)!=__T("N=1") && Retrieve(Stream_Video, 0, Video_Colorimetry)==__T("4:2:0") && (Retrieve(Stream_Video, 0, Video_BitRate)==__T("35000000") || Retrieve(Stream_Video, 0, Video_BitRate_Nominal)==__T("35000000") || Retrieve(Stream_Video, 0, Video_BitRate_Maximum)==__T("35000000")))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "XDCAM HD 35");
            Fill(Stream_Video, 0, Video_Format_Commercial_IfAny, "XDCAM HD 35");
        }
        else if (Retrieve(Stream_Video, 0, Video_Format)==__T("MPEG Video") && !Retrieve(Stream_Video, 0, Video_Format_Settings_GOP).empty() && Retrieve(Stream_Video, 0, Video_Format_Settings_GOP)!=__T("N=1") && Retrieve(Stream_Video, 0, Video_Colorimetry)==__T("4:2:2") && (Retrieve(Stream_Video, 0, Video_BitRate)==__T("50000000") || Retrieve(Stream_Video, 0, Video_BitRate_Nominal)==__T("50000000") || Retrieve(Stream_Video, 0, Video_BitRate_Maximum)==__T("50000000")))
        {
            Fill(Stream_General, 0, General_Format_Commercial_IfAny, "XDCAM HD422");
            Fill(Stream_Video, 0, Video_Format_Commercial_IfAny, "XDCAM HD422");
        }
    }
}

//---------------------------------------------------------------------------
void File_Mxf::Streams_Finish_Component(const int128u ComponentUID, float64 EditRate, int32u TrackID, int64u Origin)
{
    components::iterator Component=Components.find(ComponentUID);
    if (Component==Components.end())
        return;

    //Duration
    if (EditRate && StreamKind_Last!=Stream_Max && Component->second.Duration!=(int64u)-1)
    {
        int64u FrameCount=Component->second.Duration;
        if (StreamKind_Last==Stream_Video || Config->File_IgnoreFramesRate)
        {
            int64u File_IgnoreFramesBefore=Config->File_IgnoreFramesBefore;
            if (File_IgnoreFramesBefore && Config->File_IgnoreFramesRate && (EditRate<Config->File_IgnoreFramesRate*0.9 || EditRate>Config->File_IgnoreFramesRate*1.1)) //In case of problem or EditRate being sampling rate
                File_IgnoreFramesBefore=float64_int64s(((float64)File_IgnoreFramesBefore)/Config->File_IgnoreFramesRate*EditRate);
            int64u File_IgnoreFramesAfter=Config->File_IgnoreFramesAfter;
            if (File_IgnoreFramesAfter!=(int64u)-1 && Config->File_IgnoreFramesRate && (EditRate<Config->File_IgnoreFramesRate*0.9 || EditRate>Config->File_IgnoreFramesRate*1.1)) //In case of problem or EditRate being sampling rate
                File_IgnoreFramesAfter=float64_int64s(((float64)File_IgnoreFramesAfter)/Config->File_IgnoreFramesRate*EditRate);
            if (File_IgnoreFramesAfter<FrameCount)
                FrameCount=File_IgnoreFramesAfter;
            if (FrameCount<File_IgnoreFramesBefore)
                FrameCount=File_IgnoreFramesBefore;
            FrameCount-=File_IgnoreFramesBefore;
        }
        Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_Duration), FrameCount*1000/EditRate, 0, true);
        size_t ID_SubStreamInfo_Pos=Retrieve(StreamKind_Last, StreamPos_Last, General_ID).find(__T("-"));
        if (ID_SubStreamInfo_Pos!=string::npos)
        {
            Ztring ID=Retrieve(StreamKind_Last, StreamPos_Last, General_ID);
            ID.resize(ID_SubStreamInfo_Pos+1);
            size_t StreamPos_Last_Temp=StreamPos_Last;
            while (StreamPos_Last_Temp)
            {
                StreamPos_Last_Temp--;
                if (Retrieve(StreamKind_Last, StreamPos_Last_Temp, General_ID).find(ID)!=0)
                    break;
                Fill(StreamKind_Last, StreamPos_Last_Temp, Fill_Parameter(StreamKind_Last, Generic_Duration), FrameCount*1000/EditRate, 0, true);
            }
        }

        // Hack, TODO: find a correct method for detecting fiel/frame differene
        if (StreamKind_Last==Stream_Video)
            for (essences::iterator Essence=Essences.begin(); Essence!=Essences.end(); ++Essence)
                if (Essence->second.StreamKind==Stream_Video && Essence->second.StreamPos-(StreamPos_StartAtOne?1:0)==StreamPos_Last)
                {
                    if (Essence->second.Field_Count_InThisBlock_1 && !Essence->second.Field_Count_InThisBlock_2)
                        FrameCount/=2;
                    break;
                }

        if (Retrieve(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_FrameCount)).empty())
            Fill(StreamKind_Last, StreamPos_Last, Fill_Parameter(StreamKind_Last, Generic_FrameCount), FrameCount);
    }

    //For the sequence, searching Structural componenents
    for (size_t Pos=0; Pos<Component->second.StructuralComponents.size(); Pos++)
    {
        components::iterator Component2=Components.find(Component->second.StructuralComponents[Pos]);
        if (Component2!=Components.end() && Component2->second.TimeCode_StartTimecode!=(int64u)-1 && !Config->File_IsReferenced_Get())
        {
            bool IsDuplicate=false;
            for (size_t Pos2=0; Pos2<Count_Get(Stream_Other); Pos2++)
                if (Ztring::ToZtring(TrackID)==Retrieve(Stream_Other, Pos2, "ID"))
                    IsDuplicate=true;
            if (!IsDuplicate)
            {
                TimeCode TC(Component2->second.TimeCode_StartTimecode-Origin+Config->File_IgnoreFramesBefore, (int8u)Component2->second.TimeCode_RoundedTimecodeBase, Component2->second.TimeCode_DropFrame);
                Stream_Prepare(Stream_Other);
                Fill(Stream_Other, StreamPos_Last, Other_ID, TrackID);
                Fill(Stream_Other, StreamPos_Last, Other_Type, "Time code");
                Fill(Stream_Other, StreamPos_Last, Other_Format, "MXF TC");
                //Fill(Stream_Other, StreamPos_Last, Other_MuxingMode, "Time code track");
                Fill(Stream_Other, StreamPos_Last, Other_TimeCode_FirstFrame, TC.ToString().c_str());
                Fill(Stream_Other, StreamPos_Last, Other_TimeCode_Settings, "Striped");
            }
        }
    }
}

//---------------------------------------------------------------------------
void File_Mxf::Streams_Finish_Identification (const int128u IdentificationUID)
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
            Encoded_Library_Name+=__T(' ');
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
            Encoded_Application+=__T(' ');
            Encoded_Application+=Encoded_Library_Version;
        }
        Fill(Stream_General, 0, General_Encoded_Application, Encoded_Application, true);
        Fill(Stream_General, 0, General_Encoded_Library_Name, Encoded_Library_Name, true);
        Fill(Stream_General, 0, General_Encoded_Library_Version, Encoded_Library_Version, true);
    }

    for (std::map<std::string, Ztring>::iterator Info=Identification->second.Infos.begin(); Info!=Identification->second.Infos.end(); ++Info)
        Fill(Stream_General, 0, Info->first.c_str(), Info->second, true);
}

//***************************************************************************
// Buffer - Global
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mxf::Read_Buffer_Init()
{
    EssenceContainer_FromPartitionMetadata=0;
    #if MEDIAINFO_DEMUX
         Demux_UnpacketizeContainer=Config->Demux_Unpacketize_Get();
         Demux_Rate=Config->Demux_Rate_Get();
    #endif //MEDIAINFO_DEMUX
}

//---------------------------------------------------------------------------
void File_Mxf::Read_Buffer_Continue()
{
    #if MEDIAINFO_DEMUX
        if (Demux_CurrentParser)
        {
            if (Frame_Count_NotParsedIncluded!=(int64u)-1)
                Frame_Count_NotParsedIncluded--;
            Open_Buffer_Continue(Demux_CurrentParser, Buffer+Buffer_Offset, 0, false);
            if (Frame_Count_NotParsedIncluded!=(int64u)-1)
                Frame_Count_NotParsedIncluded++;
            if (Config->Demux_EventWasSent)
                return;
            switch (Demux_CurrentParser->Field_Count_InThisBlock)
            {
                case 1 : Demux_CurrentEssence->second.Field_Count_InThisBlock_1++; break;
                case 2 : Demux_CurrentEssence->second.Field_Count_InThisBlock_2++; break;
                default: ;
            }
            if (Demux_CurrentParser->Buffer_Size)
                Demux_CurrentParser=NULL; //No more need of it
        }
    #endif //MEDIAINFO_DEMUX

    if (!IsSub)
    {
        if (Config->ParseSpeed>=1.0)
        {
            bool Buffer_End_IsUpdated=false;
            if (Config->File_IsGrowing && !Config->File_IsNotGrowingAnymore)
            {
                File F;
                F.Open(File_Name);
                std::vector<int8u> SearchingPartitionPack(65536);
                size_t SearchingPartitionPack_Size=F.Read(&SearchingPartitionPack[0], SearchingPartitionPack.size());
                for (size_t Pos=0; Pos+16<SearchingPartitionPack_Size; Pos++)
                    if (SearchingPartitionPack[Pos   ]==0x06
                     && SearchingPartitionPack[Pos+ 1]==0x0E
                     && SearchingPartitionPack[Pos+ 2]==0x2B
                     && SearchingPartitionPack[Pos+ 3]==0x34
                     && SearchingPartitionPack[Pos+ 4]==0x02
                     && SearchingPartitionPack[Pos+ 5]==0x05
                     && SearchingPartitionPack[Pos+ 6]==0x01
                     && SearchingPartitionPack[Pos+ 7]==0x01
                     && SearchingPartitionPack[Pos+ 8]==0x0D
                     && SearchingPartitionPack[Pos+ 9]==0x01
                     && SearchingPartitionPack[Pos+10]==0x02
                     && SearchingPartitionPack[Pos+11]==0x01
                     && SearchingPartitionPack[Pos+12]==0x01
                     && SearchingPartitionPack[Pos+13]==0x02) //Header Partition Pack
                    {
                        switch (SearchingPartitionPack[Pos+14])
                        {
                            case 0x02 :
                            case 0x04 :
                                        {
                                        //Filling duration
                                        F.Close();
                                        Config->File_IsNotGrowingAnymore=true;
                                        MediaInfo_Internal MI;
                                        Ztring ParseSpeed_Save=MI.Option(__T("ParseSpeed_Get"), __T(""));
                                        Ztring Demux_Save=MI.Option(__T("Demux_Get"), __T(""));
                                        MI.Option(__T("ParseSpeed"), __T("0"));
                                        MI.Option(__T("Demux"), Ztring());
                                        size_t MiOpenResult=MI.Open(File_Name);
                                        MI.Option(__T("ParseSpeed"), ParseSpeed_Save); //This is a global value, need to reset it. TODO: local value
                                        MI.Option(__T("Demux"), Demux_Save); //This is a global value, need to reset it. TODO: local value
                                        if (MiOpenResult)
                                        {
                                            Fill(Stream_General, 0, General_Format_Settings, MI.Get(Stream_General, 0, General_Format_Settings), true);
                                            Fill(Stream_General, 0, General_Duration, MI.Get(Stream_General, 0, General_Duration), true);
                                            Fill(Stream_General, 0, General_FileSize, MI.Get(Stream_General, 0, General_FileSize), true);
                                            Fill(Stream_General, 0, General_StreamSize, MI.Get(Stream_General, 0, General_StreamSize), true);
                                            if (Buffer_End_Unlimited)
                                            {
                                                Buffer_End=MI.Get(Stream_General, 0, General_FileSize).To_int64u()-MI.Get(Stream_General, 0, General_FooterSize).To_int64u();
                                                Buffer_End_IsUpdated=true;
                                            }
                                            if (!Config->File_IsReferenced_Get() && ReferenceFiles && Retrieve(Stream_General, 0, General_StreamSize).To_int64u())
                                            {
                                                //Playlist file size is not correctly modified
                                                Config->File_Size-=File_Size;
                                                File_Size=Retrieve(Stream_General, 0, General_StreamSize).To_int64u();
                                                Config->File_Size+=File_Size;
                                            }
                                        }
                                        }
                                        break;
                            default   : ;
                        }
                    }

                if (Buffer_End && Buffer_End_Unlimited && !Buffer_End_IsUpdated)
                    Buffer_End=Config->File_Size; //Updating Clip end in case the
            }

            Config->State_Set(((float)Buffer_TotalBytes)/Config->File_Size);
        }
    }

    if ((IsCheckingRandomAccessTable || IsCheckingFooterPartitionAddress) && File_Offset+Buffer_Offset+16<File_Size)
    {
        if (Buffer_Offset+16>Buffer_Size)
        {
            Element_WaitForMoreData();
            return;
        }
        if (CC4(Buffer+Buffer_Offset)!=0x060E2B34 || CC3(Buffer+Buffer_Offset+4)!=0x020501 || CC3(Buffer+Buffer_Offset+8)!=0x0D0102 || CC1(Buffer+Buffer_Offset+12)!=0x01)
        {
            if (IsCheckingRandomAccessTable || (IsCheckingFooterPartitionAddress && FooterPartitionAddress_Jumped))
                TryToFinish(); //No footer
            else if (IsCheckingFooterPartitionAddress)
            {
                IsParsingEnd=true;
                GoToFromEnd(4); //For random access table
                FooterPartitionAddress_Jumped=true;
                Open_Buffer_Unsynch();
            }
        }
        IsCheckingRandomAccessTable=false;
        IsCheckingFooterPartitionAddress=false;
    }

    if (Config->ParseSpeed<1.0 && File_Offset+Buffer_Offset+4==File_Size)
    {
        int32u Length;
        Get_B4 (Length,                                         "Length (Random Index)");
        if (Length>=16+4 && Length<File_Size/2)
        {
            GoToFromEnd(Length); //For random access table
            Open_Buffer_Unsynch();
        }
    }
}

//---------------------------------------------------------------------------
void File_Mxf::Read_Buffer_AfterParsing()
{
    if (File_GoTo==(int64u)-1 && File_Offset+Buffer_Offset>=IsParsingMiddle_MaxOffset)
    {
        Fill();
        Open_Buffer_Unsynch();
        Finish();
        return;
    }

    if (File_Offset+Buffer_Size>=File_Size)
    {
        if (Partitions_IsCalculatingHeaderByteCount)
        {
            Partitions_IsCalculatingHeaderByteCount=false;
            if (Partitions_Pos<Partitions.size())
                Partitions[Partitions_Pos].PartitionPackByteCount=File_Offset+Buffer_Offset-Partitions[Partitions_Pos].StreamOffset;
        }

        if (IsParsingEnd)
        {
            if (PartitionMetadata_PreviousPartition && RandomIndexMetadatas.empty() && !RandomIndexMetadatas_AlreadyParsed)
            {
                Partitions_Pos=0;
                while (Partitions_Pos<Partitions.size() && Partitions[Partitions_Pos].StreamOffset!=PartitionMetadata_PreviousPartition)
                    Partitions_Pos++;
                if (Partitions_Pos==Partitions.size())
                {
                    GoTo(PartitionMetadata_PreviousPartition);
                    Open_Buffer_Unsynch();
                    return;
                }
            }
        }

        //Checking if we want to seek again
        if (File_GoTo==(int64u)-1)
            TryToFinish();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::Read_Buffer_Unsynched()
{
    //Adapting DataSizeToParse
    if (Buffer_End)
    {
        if (File_GoTo>=Buffer_End //Too much late
        || File_GoTo<=Buffer_Begin) //Too much early
        {
            Buffer_Begin=(int64u)-1;
            Buffer_End=0;
            Buffer_End_Unlimited=false;
            Buffer_Header_Size=0;
            MustSynchronize=true;
            Synched=false;
            UnSynched_IsNotJunk=true;
        }
        else
            Synched=true; //Always in clip data
    }

    FrameInfo=frame_info();
    FrameInfo.DTS=float64_int64s(DTS_Delay*1000000000);
    Frame_Count_NotParsedIncluded=(int64u)-1;
    #if MEDIAINFO_DEMUX || MEDIAINFO_SEEK
        if (!Tracks.empty() && Tracks.begin()->second.EditRate) //TODO: use the corresponding track instead of the first one
            FrameInfo.DUR=float64_int64s(1000000000/Tracks.begin()->second.EditRate);
        else if (!IndexTables.empty() && IndexTables[0].IndexEditRate)
            FrameInfo.DUR=float64_int64s(1000000000/IndexTables[0].IndexEditRate);

        //Calculating the byte count not included in seek information (partition, index...)
        int64u FutureFileOffset=File_GoTo==(int64u)-1?(File_Offset+Buffer_Offset):File_GoTo;
        int64u StreamOffset_Offset=0;
        Partitions_Pos=0;
        while (Partitions_Pos<Partitions.size() && Partitions[Partitions_Pos].StreamOffset<=FutureFileOffset)
        {
            StreamOffset_Offset+=Partitions[Partitions_Pos].PartitionPackByteCount+Partitions[Partitions_Pos].HeaderByteCount+Partitions[Partitions_Pos].IndexByteCount;
            Partitions_Pos++;
        }

        if (Descriptors.size()==1 && Descriptors.begin()->second.ByteRate!=(int32u)-1 && Descriptors.begin()->second.SampleRate)
        {
            float64 BytePerFrame=Descriptors.begin()->second.ByteRate/Descriptors.begin()->second.SampleRate;
            float64 Frame_Count_NotParsedIncluded_Precise;
            if (FutureFileOffset>(StreamOffset_Offset+Buffer_Header_Size))
                Frame_Count_NotParsedIncluded_Precise=(FutureFileOffset-(StreamOffset_Offset+Buffer_Header_Size))/BytePerFrame; //In case of audio at frame rate not an integer
            else
                Frame_Count_NotParsedIncluded_Precise=0;
            Frame_Count_NotParsedIncluded=float64_int64s(Frame_Count_NotParsedIncluded_Precise);
            FrameInfo.DTS=float64_int64s(DTS_Delay*1000000000+((float64)Frame_Count_NotParsedIncluded_Precise)*1000000000/Descriptors.begin()->second.SampleRate);
            FrameInfo.PTS=FrameInfo.DTS;
            if (!Tracks.empty() && Tracks.begin()->second.EditRate) //TODO: use the corresponding track instead of the first one
                FrameInfo.DUR=float64_int64s(1000000000/Tracks.begin()->second.EditRate);
            else if (!IndexTables.empty() && IndexTables[0].IndexEditRate)
                FrameInfo.DUR=float64_int64s(1000000000/IndexTables[0].IndexEditRate);
            else
                FrameInfo.DUR=float64_int64s(1000000000/Descriptors.begin()->second.SampleRate);
            Demux_random_access=true;
        }
        else if (!IndexTables.empty() && IndexTables[0].EditUnitByteCount)
        {
            int64u Position=0;
            Frame_Count_NotParsedIncluded=0;
            for (size_t Pos=0; Pos<IndexTables.size(); Pos++)
            {
                if (IndexTables[0].IndexDuration && FutureFileOffset>=((Buffer_End?Buffer_Begin:(StreamOffset_Offset+Buffer_Header_Size))+Position)+IndexTables[Pos].IndexDuration*IndexTables[Pos].EditUnitByteCount) //Considering IndexDuration=0 as unlimited
                {
                    Position+=SDTI_SizePerFrame+IndexTables[Pos].EditUnitByteCount*IndexTables[Pos].IndexDuration;
                    Frame_Count_NotParsedIncluded+=IndexTables[Pos].IndexDuration;
                }
                else
                {
                    int64u FramesToAdd;
                    if (FutureFileOffset>((Buffer_End?Buffer_Begin:(StreamOffset_Offset+Buffer_Header_Size))+Position))
                        FramesToAdd=(FutureFileOffset-((Buffer_End?Buffer_Begin:(StreamOffset_Offset+Buffer_Header_Size))+Position))/IndexTables[Pos].EditUnitByteCount;
                    else
                        FramesToAdd=0;
                    Position+=(SDTI_SizePerFrame+IndexTables[Pos].EditUnitByteCount)*FramesToAdd;
                    if (IndexTables[Pos].IndexEditRate)
                    {
                        if (Descriptors.size()==1 && Descriptors.begin()->second.SampleRate!=IndexTables[Pos].IndexEditRate)
                        {
                            float64 Frame_Count_NotParsedIncluded_Precise=((float64)FramesToAdd)/IndexTables[Pos].IndexEditRate*Descriptors.begin()->second.SampleRate;
                            Frame_Count_NotParsedIncluded+=float64_int64s(((float64)FramesToAdd)/IndexTables[Pos].IndexEditRate*Descriptors.begin()->second.SampleRate);
                            FrameInfo.PTS=FrameInfo.DTS=float64_int64s(DTS_Delay*1000000000+((float64)Frame_Count_NotParsedIncluded_Precise)*1000000000/Descriptors.begin()->second.SampleRate);
                        }
                        else
                        {
                            Frame_Count_NotParsedIncluded+=FramesToAdd;
                            FrameInfo.PTS=FrameInfo.DTS=float64_int64s(DTS_Delay*1000000000+((float64)Frame_Count_NotParsedIncluded)*1000000000/IndexTables[Pos].IndexEditRate);
                        }
                    }
                    else
                        FrameInfo.PTS=FrameInfo.DTS=(int64u)-1;
                    Demux_random_access=true;

                    break;
                }
            }
        }
        else if (!IndexTables.empty() && !IndexTables[0].Entries.empty())
        {
            int64u StreamOffset;
            if (StreamOffset_Offset<FutureFileOffset)
                StreamOffset=FutureFileOffset-StreamOffset_Offset;
            else
                StreamOffset=0;
            for (size_t Pos=0; Pos<IndexTables.size(); Pos++)
            {
                //Searching the right index
                if (!IndexTables[Pos].Entries.empty() && StreamOffset>=IndexTables[Pos].Entries[0].StreamOffset+(IndexTables[Pos].IndexStartPosition)*SDTI_SizePerFrame && (Pos+1>=IndexTables.size() || IndexTables[Pos+1].Entries.empty() || StreamOffset<IndexTables[Pos+1].Entries[0].StreamOffset+(IndexTables[Pos+1].IndexStartPosition)*SDTI_SizePerFrame))
                {
                    //Searching the frame pos
                    for (size_t EntryPos=0; EntryPos<IndexTables[Pos].Entries.size(); EntryPos++)
                    {
                        //Testing coherency
                        int64u Entry0_StreamOffset=0; //For coherency checking
                        int64u Entry_StreamOffset=IndexTables[Pos].Entries[EntryPos].StreamOffset+(IndexTables[Pos].IndexStartPosition+EntryPos)*SDTI_SizePerFrame;
                        int64u Entry1_StreamOffset=File_Size; //For coherency checking
                        if (EntryPos==0 && Pos && IndexTables[Pos-1].Entries.empty())
                            Entry0_StreamOffset=IndexTables[Pos-1].Entries[IndexTables[Pos-1].Entries.size()-1].StreamOffset+(IndexTables[Pos].IndexStartPosition+EntryPos-1)*SDTI_SizePerFrame;
                        else if (EntryPos)
                            Entry0_StreamOffset=IndexTables[Pos].Entries[EntryPos-1].StreamOffset+(IndexTables[Pos].IndexStartPosition+EntryPos-1)*SDTI_SizePerFrame;
                        if (EntryPos+1<IndexTables[Pos].Entries.size())
                            Entry1_StreamOffset=IndexTables[Pos].Entries[EntryPos+1].StreamOffset+(IndexTables[Pos].IndexStartPosition+EntryPos+1)*SDTI_SizePerFrame;
                        else if (Pos+1<IndexTables.size() && !IndexTables[Pos+1].Entries.empty())
                            Entry1_StreamOffset=IndexTables[Pos+1].Entries[0].StreamOffset+(IndexTables[Pos].IndexStartPosition+EntryPos+1)*SDTI_SizePerFrame;

                        if (Entry0_StreamOffset>Entry_StreamOffset || Entry_StreamOffset>Entry1_StreamOffset)
                            break; //Problem

                        if (StreamOffset>=Entry_StreamOffset && StreamOffset<Entry1_StreamOffset)
                        {
                            //Special case: we are not sure the last index is the last frame, doing nothing
                            if (Pos+1==IndexTables.size() && EntryPos+1==IndexTables[Pos].Entries.size())
                                break;

                            Frame_Count_NotParsedIncluded=IndexTables[Pos].IndexStartPosition+EntryPos;
                            if (IndexTables[Pos].IndexEditRate)
                                FrameInfo.DTS=float64_int64s(DTS_Delay*1000000000+((float64)Frame_Count_NotParsedIncluded)/IndexTables[Pos].IndexEditRate*1000000000);
                            Demux_random_access=IndexTables[Pos].Entries[EntryPos].Type?false:true;
                            break;
                        }
                    }
                }
            }
        }
        else if (OverallBitrate_IsCbrForSure)
        {
            int64u Begin=Partitions[0].StreamOffset+Partitions[0].PartitionPackByteCount+Partitions[0].HeaderByteCount+Partitions[0].IndexByteCount;
            Frame_Count_NotParsedIncluded=(FutureFileOffset-Begin)/OverallBitrate_IsCbrForSure;
            if (!Descriptors.empty() && Descriptors.begin()->second.SampleRate)
                FrameInfo.PTS=FrameInfo.DTS=float64_int64s(DTS_Delay*1000000000+((float64)Frame_Count_NotParsedIncluded)*1000000000/Descriptors.begin()->second.SampleRate);
        }
        else if (Frame_Count_NotParsedIncluded==0)
        {
            FrameInfo.DTS=float64_int64s(DTS_Delay*1000000000);
        }

    #endif //if MEDIAINFO_DEMUX || MEDIAINFO_SEEK

    if (!Tracks.empty() && Tracks.begin()->second.EditRate) //TODO: use the corresponding track instead of the first one
        FrameInfo.DUR=float64_int64s(1000000000/Tracks.begin()->second.EditRate);
    #if MEDIAINFO_DEMUX || MEDIAINFO_SEEK
        else if (!IndexTables.empty() && IndexTables[0].IndexEditRate)
            FrameInfo.DUR=float64_int64s(1000000000/IndexTables[0].IndexEditRate);
    #endif //if MEDIAINFO_DEMUX || MEDIAINFO_SEEK

    for (essences::iterator Essence=Essences.begin(); Essence!=Essences.end(); ++Essence)
        for (parsers::iterator Parser=Essence->second.Parsers.begin(); Parser!=Essence->second.Parsers.end(); ++Parser)
        {
            (*Parser)->Open_Buffer_Unsynch();
            Essence->second.FrameInfo=FrameInfo;
            Essence->second.Frame_Count_NotParsedIncluded=Frame_Count_NotParsedIncluded;
        }

    Partitions_Pos=0;
    if (Partitions_IsCalculatingHeaderByteCount)
    {
        Partitions.erase(Partitions.end()-1);
        Partitions_IsCalculatingHeaderByteCount=false;
    }
    if (Partitions_IsCalculatingSdtiByteCount)
        Partitions_IsCalculatingSdtiByteCount=false;
    Essences_FirstEssence_Parsed=false;

    #if MEDIAINFO_SEEK
        IndexTables_Pos=0;
    #endif //MEDIAINFO_SEEK
}

//---------------------------------------------------------------------------
#if MEDIAINFO_DEMUX || MEDIAINFO_SEEK
bool File_Mxf::DetectDuration ()
{
    if (Duration_Detected)
        return false;

    MediaInfo_Internal MI;
    MI.Option(__T("File_IsDetectingDuration"), __T("1"));
    MI.Option(__T("File_KeepInfo"), __T("1"));
    Ztring ParseSpeed_Save=MI.Option(__T("ParseSpeed_Get"), __T(""));
    Ztring Demux_Save=MI.Option(__T("Demux_Get"), __T(""));
    MI.Option(__T("ParseSpeed"), __T("0"));
    MI.Option(__T("Demux"), Ztring());
    size_t MiOpenResult=MI.Open(File_Name);
    MI.Option(__T("ParseSpeed"), ParseSpeed_Save); //This is a global value, need to reset it. TODO: local value
    MI.Option(__T("Demux"), Demux_Save); //This is a global value, need to reset it. TODO: local value
    if (!MiOpenResult || MI.Get(Stream_General, 0, General_Format)!=__T("MXF"))
        return false;
    Partitions=((File_Mxf*)MI.Info)->Partitions;
    std::sort(Partitions.begin(), Partitions.end());
    IndexTables=((File_Mxf*)MI.Info)->IndexTables;
    std::sort(IndexTables.begin(), IndexTables.end());
    SDTI_SizePerFrame=((File_Mxf*)MI.Info)->SDTI_SizePerFrame;
    Clip_Begin=((File_Mxf*)MI.Info)->Clip_Begin;
    Clip_End=((File_Mxf*)MI.Info)->Clip_End;
    Clip_Header_Size=((File_Mxf*)MI.Info)->Clip_Header_Size;
    Clip_Code=((File_Mxf*)MI.Info)->Clip_Code;
    Tracks=((File_Mxf*)MI.Info)->Tracks; //In one file (*-009.mxf), the TrackNumber is known only at the end of the file (Open and incomplete header/footer)
    for (tracks::iterator Track=Tracks.begin(); Track!=Tracks.end(); ++Track)
        Track->second.Stream_Finish_Done=false; //Reseting the value, it is not done in this instance
    if (MI.Get(Stream_General, 0, General_OverallBitRate_Mode)==__T("CBR") && Partitions.size()==2 && Partitions[0].FooterPartition==Partitions[1].StreamOffset && !Descriptors.empty())
    {
        //Searching duration
        int64u Duration=0;
        for (descriptors::iterator Descriptor=Descriptors.begin(); Descriptor!=Descriptors.end(); ++Descriptor)
            if (Descriptor->second.Duration!=(int64u)-1 && Descriptor->second.Duration)
            {
                if (Duration && Duration!=Descriptor->second.Duration)
                {
                    Duration=0;
                    break; //Not supported
                }
                Duration=Descriptor->second.Duration;
            }

        //Computing the count of bytes per frame
        if (Duration)
        {
            int64u Begin=Partitions[0].StreamOffset+Partitions[0].PartitionPackByteCount+Partitions[0].HeaderByteCount+Partitions[0].IndexByteCount;
            float64 BytesPerFrameF=((float64)(Partitions[0].FooterPartition-Begin)/Duration);
            OverallBitrate_IsCbrForSure=float64_int64s(BytesPerFrameF);
            if (OverallBitrate_IsCbrForSure!=BytesPerFrameF) //Testing integrity of the computing
                OverallBitrate_IsCbrForSure=0;
        }
    }
    Duration_Detected=true;

    return true;
}
#endif //MEDIAINFO_DEMUX || MEDIAINFO_SEEK

#if MEDIAINFO_SEEK
size_t File_Mxf::Read_Buffer_Seek (size_t Method, int64u Value, int64u ID)
{
    if (ReferenceFiles)
        return ReferenceFiles->Read_Buffer_Seek(Method, Value, ID);

    //Init
    if (!Duration_Detected)
    {
        if (!DetectDuration())
            return 0;
    }

    //Config - TODO: merge with the one in Data_Parse()
    if (!Essences_FirstEssence_Parsed)
    {
        if (Descriptors.size()==1 && Descriptors.begin()->second.StreamKind==Stream_Audio)
        {
            //Configuring bitrate is not available in descriptor
            if (Descriptors.begin()->second.ByteRate==(int32u)-1 && Descriptors.begin()->second.Infos.find("SamplingRate")!=Descriptors.begin()->second.Infos.end())
            {
                int32u SamplingRate=Descriptors.begin()->second.Infos["SamplingRate"].To_int32u();

                if (Descriptors.begin()->second.BlockAlign!=(int16u)-1)
                    Descriptors.begin()->second.ByteRate=SamplingRate*Descriptors.begin()->second.BlockAlign;
                else if (Descriptors.begin()->second.QuantizationBits!=(int8u)-1)
                    Descriptors.begin()->second.ByteRate=SamplingRate*Descriptors.begin()->second.QuantizationBits/8;
            }

            //Configuring EditRate if needed (e.g. audio at 48000 Hz)
            if (Demux_Rate) //From elsewhere
            {
                Descriptors.begin()->second.SampleRate=Demux_Rate;
            }
            else if (Descriptors.begin()->second.SampleRate>1000)
            {
                float64 EditRate_FromTrack=DBL_MAX;
                for (tracks::iterator Track=Tracks.begin(); Track!=Tracks.end(); ++Track)
                    if (EditRate_FromTrack>Track->second.EditRate)
                        EditRate_FromTrack=Track->second.EditRate;
                if (EditRate_FromTrack>1000)
                    Descriptors.begin()->second.SampleRate=24; //Default value
                else
                    Descriptors.begin()->second.SampleRate=EditRate_FromTrack;
                for (tracks::iterator Track=Tracks.begin(); Track!=Tracks.end(); ++Track)
                    if (Track->second.EditRate>EditRate_FromTrack)
                    {
                        Track->second.EditRate_Real=Track->second.EditRate;
                        Track->second.EditRate=EditRate_FromTrack;
                    }
            }
        }

        Essences_FirstEssence_Parsed=true;
    }
    //Parsing
    switch (Method)
    {
        case 0  :
                    {
                    if (Config->File_IgnoreFramesBefore && Config->File_IgnoreFramesRate)
                    {
                        Read_Buffer_Seek(3, 0, (int64u)-1);
                        if (File_GoTo!=(int64u)-1)
                            Value+=File_GoTo;
                    }
                        
                    //Calculating the byte count not included in seek information (partition, index...)
                    Partitions_Pos=0;
                    while (Partitions_Pos<Partitions.size() && Partitions[Partitions_Pos].StreamOffset<Value)
                        Partitions_Pos++;
                    if (Partitions_Pos && (Partitions_Pos==Partitions.size() || Partitions[Partitions_Pos].StreamOffset!=Value))
                        Partitions_Pos--; //This is the previous item
                    if (Partitions_Pos>=Partitions.size())
                    {
                        GoTo(0);
                        Open_Buffer_Unsynch();
                        return 1;
                    }
                    int64u StreamOffset_Offset=Partitions[Partitions_Pos].StreamOffset-Partitions[Partitions_Pos].BodyOffset+Partitions[Partitions_Pos].PartitionPackByteCount+Partitions[Partitions_Pos].HeaderByteCount+Partitions[Partitions_Pos].IndexByteCount;

                    //If in header
                    if ((Clip_Begin!=(int64u)-1 && Value<Clip_Begin) || Value<StreamOffset_Offset)
                    {
                        GoTo(StreamOffset_Offset);
                        Open_Buffer_Unsynch();
                        return 1;
                    }

                    if (Buffer_End
                     && Descriptors.size()==1 && Descriptors.begin()->second.ByteRate!=(int32u)-1 && Descriptors.begin()->second.BlockAlign && Descriptors.begin()->second.BlockAlign!=(int16u)-1  && Descriptors.begin()->second.SampleRate)
                    {
                        if (Value>StreamOffset_Offset)
                        {
                            float64 BytesPerFrame=Descriptors.begin()->second.ByteRate/Descriptors.begin()->second.SampleRate;
                            int64u FrameCount=(int64u)((Value-Buffer_Begin)/BytesPerFrame);
                            int64u SizeBlockAligned=float64_int64s(FrameCount*BytesPerFrame);
                            SizeBlockAligned/=Descriptors.begin()->second.BlockAlign;
                            SizeBlockAligned*=Descriptors.begin()->second.BlockAlign;

                            GoTo(Buffer_Begin+SizeBlockAligned);
                            Open_Buffer_Unsynch();
                            return 1;
                        }
                    }
                    else if (Buffer_End
                     && !IndexTables.empty() && IndexTables[0].EditUnitByteCount)
                    {
                        int64u Stream_Offset=0;
                        for (size_t Pos=0; Pos<IndexTables.size(); Pos++)
                        {
                            if (IndexTables[Pos].IndexDuration==0 || Value<StreamOffset_Offset+Stream_Offset+IndexTables[Pos].IndexDuration*IndexTables[Pos].EditUnitByteCount)
                            {
                                int64u FrameToAdd=(Value-(StreamOffset_Offset+Stream_Offset))/IndexTables[Pos].EditUnitByteCount;
                                Stream_Offset+=FrameToAdd*IndexTables[Pos].EditUnitByteCount;

                                GoTo(Buffer_Begin+Stream_Offset);
                                Open_Buffer_Unsynch();
                                return 1;
                            }
                            else
                                Stream_Offset+=IndexTables[Pos].IndexDuration*IndexTables[Pos].EditUnitByteCount;
                        }
                        return 2; //Invalid value
                    }

                    GoTo(Value);
                    Open_Buffer_Unsynch();
                    return 1;
                    }
        case 1  :
                    return Read_Buffer_Seek(0, File_Size*Value/10000, ID);
        case 2  :   //Timestamp
                    {
                        if (Config->File_IgnoreFramesBefore && Config->File_IgnoreFramesRate)
                            Value+=float64_int64s(((float64)Config->File_IgnoreFramesBefore)/Config->File_IgnoreFramesRate*1000000000);

                        //We transform TimeStamp to a frame number
                        descriptors::iterator Descriptor;
                        for (Descriptor=Descriptors.begin(); Descriptor!=Descriptors.end(); ++Descriptor)
                            if (Descriptors.begin()->second.SampleRate)
                                break;
                        if (Descriptor==Descriptors.end())
                            return (size_t)-1; //Not supported

                        if (TimeCode_StartTimecode!=(int64u)-1)
                        {
                            int64u Delay=float64_int64s(DTS_Delay*1000000000);
                            if (Value<Delay)
                                return 2; //Invalid value
                            Value-=Delay;
                        }
                        Value=float64_int64s(((float64)Value)/1000000000*Descriptor->second.SampleRate);
                        }
                    //No break;
        case 3  :   //FrameNumber
                    Value+=Config->File_IgnoreFramesBefore;

                    if (Descriptors.size()==1 && Descriptors.begin()->second.ByteRate!=(int32u)-1 && Descriptors.begin()->second.BlockAlign && Descriptors.begin()->second.BlockAlign!=(int16u)-1  && Descriptors.begin()->second.SampleRate)
                    {
                        float64 BytesPerFrame=Descriptors.begin()->second.ByteRate/Descriptors.begin()->second.SampleRate;
                        int64u StreamOffset=(int64u)(Value*BytesPerFrame);
                        StreamOffset/=Descriptors.begin()->second.BlockAlign;
                        StreamOffset*=Descriptors.begin()->second.BlockAlign;

                        //Calculating the byte count not included in seek information (partition, index...)
                        int64u StreamOffset_Offset=0;
                        Partitions_Pos=0;
                        while (Partitions_Pos<Partitions.size() && Partitions[Partitions_Pos].StreamOffset<=StreamOffset_Offset+StreamOffset+Value*SDTI_SizePerFrame)
                        {
                            StreamOffset_Offset+=Partitions[Partitions_Pos].PartitionPackByteCount+Partitions[Partitions_Pos].HeaderByteCount+Partitions[Partitions_Pos].IndexByteCount;
                            Partitions_Pos++;
                        }

                        if (Clip_Begin!=(int64u)-1)
                        {
                            Buffer_Begin=Clip_Begin;
                            Buffer_End=Clip_End;
                            Buffer_Header_Size=Clip_Header_Size;
                            Code=Clip_Code;
                            MustSynchronize=false;
                            #if MEDIAINFO_DEMUX
                                if (Buffer_End && Demux_UnpacketizeContainer && Essences.size()==1 && Essences.begin()->second.Parsers.size()==1 && (*(Essences.begin()->second.Parsers.begin()))->Demux_UnpacketizeContainer)
                                {
                                    (*(Essences.begin()->second.Parsers.begin()))->Demux_Level=2; //Container
                                    (*(Essences.begin()->second.Parsers.begin()))->Demux_UnpacketizeContainer=true;
                                }
                            #endif //MEDIAINFO_DEMUX
                        }

                        GoTo(StreamOffset_Offset+Buffer_Header_Size+StreamOffset+Value*SDTI_SizePerFrame);
                        Open_Buffer_Unsynch();
                        return 1;
                    }
                    else if (!IndexTables.empty() && IndexTables[0].EditUnitByteCount)
                    {
                        if (Descriptors.size()==1 && Descriptors.begin()->second.SampleRate!=IndexTables[0].IndexEditRate)
                        {
                            float64 ValueF=(float64)Value;
                            ValueF/=Descriptors.begin()->second.SampleRate;
                            ValueF*=IndexTables[0].IndexEditRate;
                            Value=float64_int64s(ValueF);
                        }

                        if (IndexTables[IndexTables.size()-1].IndexDuration && IndexTables[IndexTables.size()-1].IndexStartPosition!=(int64u)-1 && Value>=IndexTables[IndexTables.size()-1].IndexStartPosition+IndexTables[IndexTables.size()-1].IndexDuration) //Considering IndexDuration=0 as unlimited
                            return 2; //Invalid value

                        int64u StreamOffset=0;
                        for (size_t Pos=0; Pos<IndexTables.size(); Pos++)
                        {
                            if (IndexTables[Pos].IndexDuration && Value>IndexTables[Pos].IndexStartPosition+IndexTables[Pos].IndexDuration) //Considering IndexDuration=0 as unlimited
                                StreamOffset+=IndexTables[Pos].EditUnitByteCount*IndexTables[Pos].IndexDuration;
                            else
                            {
                                StreamOffset+=IndexTables[Pos].EditUnitByteCount*(Value-IndexTables[Pos].IndexStartPosition);
                                break;
                            }
                        }

                        //Calculating the byte count not included in seek information (partition, index...)
                        int64u StreamOffset_Offset=0;
                        Partitions_Pos=0;
                        while (Partitions_Pos<Partitions.size() && Partitions[Partitions_Pos].StreamOffset<=StreamOffset_Offset+StreamOffset+Value*SDTI_SizePerFrame)
                        {
                            StreamOffset_Offset+=Partitions[Partitions_Pos].PartitionPackByteCount+Partitions[Partitions_Pos].HeaderByteCount+Partitions[Partitions_Pos].IndexByteCount;
                            Partitions_Pos++;
                        }

                        if (Clip_Begin!=(int64u)-1)
                        {
                            Buffer_Begin=Clip_Begin;
                            Buffer_End=Clip_End;
                            Buffer_Header_Size=Clip_Header_Size;
                            Code=Clip_Code;
                            MustSynchronize=false;
                            #if MEDIAINFO_DEMUX
                                if (Buffer_End && Demux_UnpacketizeContainer && Essences.size()==1 && Essences.begin()->second.Parsers.size()==1 && (*(Essences.begin()->second.Parsers.begin()))->Demux_UnpacketizeContainer)
                                {
                                    (*(Essences.begin()->second.Parsers.begin()))->Demux_Level=2; //Container
                                    (*(Essences.begin()->second.Parsers.begin()))->Demux_UnpacketizeContainer=true;
                                }
                            #endif //MEDIAINFO_DEMUX
                        }

                        GoTo(StreamOffset_Offset+Buffer_Header_Size+StreamOffset+Value*SDTI_SizePerFrame);
                        Open_Buffer_Unsynch();
                        return 1;
                    }
                    else if (!IndexTables.empty() && !IndexTables[0].Entries.empty())
                    {
                        for (size_t Pos=0; Pos<IndexTables.size(); Pos++)
                        {
                            if (Value>=IndexTables[Pos].IndexStartPosition && Value<IndexTables[Pos].IndexStartPosition+IndexTables[Pos].IndexDuration)
                            {
                                while (Value>=IndexTables[Pos].IndexStartPosition && IndexTables[Pos].Entries[(size_t)(Value-IndexTables[Pos].IndexStartPosition)].Type)
                                {
                                    Value--;
                                    if (Value<IndexTables[Pos].IndexStartPosition)
                                    {
                                        if (Pos==0)
                                            break; //There is a problem
                                        Pos--; //In previous index
                                    }
                                }

                                int64u StreamOffset=IndexTables[Pos].Entries[(size_t)(Value-IndexTables[Pos].IndexStartPosition)].StreamOffset;

                                //Calculating the byte count not included in seek information (partition, index...)
                                int64u StreamOffset_Offset=0;
                                Partitions_Pos=0;
                                while (Partitions_Pos<Partitions.size() && Partitions[Partitions_Pos].StreamOffset<=StreamOffset_Offset+StreamOffset+Value*SDTI_SizePerFrame)
                                {
                                    StreamOffset_Offset+=Partitions[Partitions_Pos].PartitionPackByteCount+Partitions[Partitions_Pos].HeaderByteCount+Partitions[Partitions_Pos].IndexByteCount;
                                    Partitions_Pos++;
                                }

                                if (Clip_Begin!=(int64u)-1)
                                {
                                    Buffer_Begin=Clip_Begin;
                                    Buffer_End=Clip_End;
                                    Buffer_Header_Size=Clip_Header_Size;
                                    Code=Clip_Code;
                                    MustSynchronize=false;
                                    #if MEDIAINFO_DEMUX
                                        if (Buffer_End && Demux_UnpacketizeContainer && Essences.size()==1 && Essences.begin()->second.Parsers.size()==1 && (*(Essences.begin()->second.Parsers.begin()))->Demux_UnpacketizeContainer)
                                        {
                                            (*(Essences.begin()->second.Parsers.begin()))->Demux_Level=2; //Container
                                            (*(Essences.begin()->second.Parsers.begin()))->Demux_UnpacketizeContainer=true;
                                        }
                                    #endif //MEDIAINFO_DEMUX
                                }

                                GoTo(StreamOffset_Offset+Buffer_Header_Size+StreamOffset+Value*SDTI_SizePerFrame);
                                Open_Buffer_Unsynch();
                                return 1;
                            }
                        }
                        return 2; //Invalid value
                    }
                    else if (OverallBitrate_IsCbrForSure)
                    {
                        int64u Begin=Partitions[0].StreamOffset+Partitions[0].PartitionPackByteCount+Partitions[0].HeaderByteCount+Partitions[0].IndexByteCount;
                        GoTo(Begin+Value*OverallBitrate_IsCbrForSure);
                        Open_Buffer_Unsynch();
                        return 1;
                    }
                    else
                        return (size_t)-1; //Not supported
        default :   return (size_t)-1; //Not supported
    }
}
#endif //MEDIAINFO_SEEK

//***************************************************************************
// Buffer - File header
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Mxf::FileHeader_Begin()
{
    //Element_Size
    if (Buffer_Size<0x18)
        return false; //Must wait for more data

    //AAF has some MXF start codes
    if (Buffer[ 0x0]==0xD0
     && Buffer[ 0x1]==0xCF
     && Buffer[ 0x2]==0x11
     && Buffer[ 0x3]==0xE0
     && Buffer[ 0x4]==0xA1
     && Buffer[ 0x5]==0xB1
     && Buffer[ 0x6]==0x1A
     && Buffer[ 0x7]==0xE1
     && Buffer[ 0x8]==0x41
     && Buffer[ 0x9]==0x41
     && Buffer[ 0xA]==0x46
     && Buffer[ 0xB]==0x42
     && Buffer[ 0xC]==0x0D
     && Buffer[ 0xD]==0x00
     && Buffer[ 0xE]==0x4F
     && Buffer[ 0xF]==0x4D
     && Buffer[0x10]==0x06
     && Buffer[0x11]==0x0E
     && Buffer[0x12]==0x2B
     && Buffer[0x13]==0x34
     && Buffer[0x14]==0x01
     && Buffer[0x15]==0x01
     && Buffer[0x16]==0x01
     && Buffer[0x17]==0xFF)
    {
        Reject("Mxf");
        return false;
    }

    //DCA uses buffer interface without filename
    if (File_Name.empty())
        File_Name=Config->File_FileName_Get();

    return true;
}

//***************************************************************************
// Buffer - Synchro
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Mxf::Synchronize()
{
    //Synchronizing
    while (Buffer_Offset+4<=Buffer_Size && (Buffer[Buffer_Offset  ]!=0x06
                                         || Buffer[Buffer_Offset+1]!=0x0E
                                         || Buffer[Buffer_Offset+2]!=0x2B
                                         || Buffer[Buffer_Offset+3]!=0x34))
    {
        Buffer_Offset++;
        while (Buffer_Offset<Buffer_Size && Buffer[Buffer_Offset]!=0x06)
            Buffer_Offset++;
    }


    while (Buffer_Offset+4<=Buffer_Size
        && CC4(Buffer+Buffer_Offset)!=0x060E2B34)
        Buffer_Offset++;

    //Parsing last bytes if needed
    if (Buffer_Offset+4>Buffer_Size)
    {
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

        File_Buffer_Size_Hint_Pointer=Config->File_Buffer_Size_Hint_Pointer_Get();
    }

    //Synched is OK
    return true;
}

//---------------------------------------------------------------------------
bool File_Mxf::Synched_Test()
{
    //Must have enough buffer for having header
    if (Buffer_Offset+16>Buffer_Size)
        return false;

    //Quick test of synchro
    if (CC4(Buffer+Buffer_Offset)!=0x060E2B34)
        Synched=false;

    //Trace config
    #if MEDIAINFO_TRACE
        if (Synched)
        {
            int64u Compare=CC8(Buffer+Buffer_Offset+ 4);
            if (Compare==0x010201010D010301LL //Raw stream
             || (Compare==0x0101010203010210LL && CC1(Buffer+Buffer_Offset+12)==0x01) //Filler
             || (Compare==0x020501010D010301LL && CC3(Buffer+Buffer_Offset+12)==0x040101) //SDTI Package Metadata Pack
             || (Compare==0x024301010D010301LL && CC3(Buffer+Buffer_Offset+12)==0x040102) //SDTI Package Metadata Set
             || (Compare==0x025301010D010301LL && CC3(Buffer+Buffer_Offset+12)==0x140201)) //System Scheme 1
            {
                Trace_Layers_Update(8); //Stream
            }
            else
            {
                Trace_Layers_Update(0); //Container1
            }
        }
    #endif //MEDIAINFO_TRACE

    //We continue
    return true;
}

//***************************************************************************
// Buffer - Per element
//***************************************************************************

//---------------------------------------------------------------------------
bool File_Mxf::Header_Begin()
{
    while (Buffer_End)
    {
        #if MEDIAINFO_DEMUX
            if (Demux_UnpacketizeContainer && Descriptors.size()==1 && Descriptors.begin()->second.ByteRate!=(int32u)-1 && Descriptors.begin()->second.BlockAlign && Descriptors.begin()->second.BlockAlign!=(int16u)-1  && Descriptors.begin()->second.SampleRate)
            {
                float64 BytesPerFrame=((float64)Descriptors.begin()->second.ByteRate)/Descriptors.begin()->second.SampleRate;
                int64u FramesAlreadyParsed=float64_int64s(((float64)(File_Offset+Buffer_Offset-Buffer_Begin))/BytesPerFrame);
                Element_Size=float64_int64s(Descriptors.begin()->second.ByteRate/Descriptors.begin()->second.SampleRate*(FramesAlreadyParsed+1));
                Element_Size/=Descriptors.begin()->second.BlockAlign;
                Element_Size*=Descriptors.begin()->second.BlockAlign;
                Element_Size-=File_Offset+Buffer_Offset-Buffer_Begin;
                if (Config->File_IsGrowing && Element_Size && File_Offset+Buffer_Offset+Element_Size>Buffer_End)
                    return false; //Waiting for more data
                while (Element_Size && File_Offset+Buffer_Offset+Element_Size>Buffer_End)
                    Element_Size-=Descriptors.begin()->second.BlockAlign;
                if (Element_Size==0)
                    Element_Size=Buffer_End-(File_Offset+Buffer_Offset);
                if (Buffer_Offset+Element_Size>Buffer_Size)
                    return false;
            }
            else if (Demux_UnpacketizeContainer && !IndexTables.empty() && IndexTables[0].EditUnitByteCount)
            {
                //Calculating the byte count not included in seek information (partition, index...)
                int64u StreamOffset_Offset;
                if (!Partitions.empty())
                {
                    while (Partitions_Pos<Partitions.size() && Partitions[Partitions_Pos].StreamOffset<File_Offset+Buffer_Offset-Header_Size)
                        Partitions_Pos++;
                    if (Partitions_Pos && (Partitions_Pos==Partitions.size() || Partitions[Partitions_Pos].StreamOffset!=File_Offset+Buffer_Offset-Header_Size))
                        Partitions_Pos--; //This is the previous item
                    StreamOffset_Offset=Partitions[Partitions_Pos].StreamOffset-Partitions[Partitions_Pos].BodyOffset+Partitions[Partitions_Pos].PartitionPackByteCount+Partitions[Partitions_Pos].HeaderByteCount+Partitions[Partitions_Pos].IndexByteCount;
                }
                else
                    StreamOffset_Offset=0;

                int64u Position=0;
                for (size_t Pos=0; Pos<IndexTables.size(); Pos++)
                {
                    if (IndexTables[Pos].IndexDuration && File_Offset+Buffer_Offset>=StreamOffset_Offset+Buffer_Header_Size+Position+IndexTables[Pos].IndexDuration*IndexTables[Pos].EditUnitByteCount) //Considering IndexDuration==0 as unlimited
                        Position+=IndexTables[Pos].EditUnitByteCount*IndexTables[Pos].IndexDuration;
                    else
                    {
                        Element_Size=IndexTables[Pos].EditUnitByteCount;
                        if (File_Offset+Buffer_Offset+Element_Size>Buffer_End)
                        {
                            Element_Size=Buffer_End-(File_Offset+Buffer_Offset);
                            break; //There is a problem
                        }

                        if (Buffer_Offset+Element_Size>Buffer_Size)
                        {
                            //Hints
                            if (File_Buffer_Size_Hint_Pointer)
                            {
                                size_t Buffer_Size_Target=(size_t)(Buffer_Offset+Element_Size-Buffer_Size+24); //+24 for next packet header
                                if (Buffer_Size_Target<128*1024)
                                    Buffer_Size_Target=128*1024;
                                //if ((*File_Buffer_Size_Hint_Pointer)<Buffer_Size_Target)
                                    (*File_Buffer_Size_Hint_Pointer)=Buffer_Size_Target;
                            }

                            return false;
                        }
                        break;
                    }
                }

                if (Buffer_Offset+(size_t)Element_Size>Buffer_Size)
                    Element_Size=Buffer_Size-Buffer_Offset; //There is a problem
            }
            else if (Demux_UnpacketizeContainer && !IndexTables.empty() && !IndexTables[0].Entries.empty())
            {
                //Calculating the byte count not included in seek information (partition, index...)
                int64u StreamOffset_Offset;
                if (!Partitions.empty())
                {
                    while (Partitions_Pos<Partitions.size() && Partitions[Partitions_Pos].StreamOffset<File_Offset+Buffer_Offset-Header_Size)
                        Partitions_Pos++;
                    if (Partitions_Pos && (Partitions_Pos==Partitions.size() || Partitions[Partitions_Pos].StreamOffset!=File_Offset+Buffer_Offset-Header_Size))
                        Partitions_Pos--; //This is the previous item
                    StreamOffset_Offset=Partitions[Partitions_Pos].StreamOffset-Partitions[Partitions_Pos].BodyOffset+Partitions[Partitions_Pos].PartitionPackByteCount+Partitions[Partitions_Pos].HeaderByteCount+Partitions[Partitions_Pos].IndexByteCount;
                }
                else
                    StreamOffset_Offset=0;

                int64u StreamOffset=File_Offset+Buffer_Offset-StreamOffset_Offset;
                for (size_t Pos=0; Pos<IndexTables.size(); Pos++)
                {
                    //Searching the right index
                    if (!IndexTables[Pos].Entries.empty() && StreamOffset>=IndexTables[Pos].Entries[0].StreamOffset+(IndexTables[Pos].IndexStartPosition)*SDTI_SizePerFrame && (Pos+1>=IndexTables.size() || StreamOffset<IndexTables[Pos+1].Entries[0].StreamOffset+(IndexTables[Pos+1].IndexStartPosition)*SDTI_SizePerFrame))
                    {
                        //Searching the frame pos
                        for (size_t EntryPos=0; EntryPos<IndexTables[Pos].Entries.size(); EntryPos++)
                        {
                            //Testing coherency
                            int64u Entry0_StreamOffset=0; //For coherency checking
                            int64u Entry_StreamOffset=IndexTables[Pos].Entries[EntryPos].StreamOffset+(IndexTables[Pos].IndexStartPosition+EntryPos)*SDTI_SizePerFrame;
                            int64u Entry1_StreamOffset=File_Size; //For coherency checking
                            if (EntryPos==0 && Pos && IndexTables[Pos-1].Entries.empty())
                                Entry0_StreamOffset=IndexTables[Pos-1].Entries[IndexTables[Pos-1].Entries.size()-1].StreamOffset+(IndexTables[Pos].IndexStartPosition+EntryPos-1)*SDTI_SizePerFrame;
                            else if (EntryPos)
                                Entry0_StreamOffset=IndexTables[Pos].Entries[EntryPos-1].StreamOffset+(IndexTables[Pos].IndexStartPosition+EntryPos-1)*SDTI_SizePerFrame;
                            if (EntryPos+1<IndexTables[Pos].Entries.size())
                                Entry1_StreamOffset=IndexTables[Pos].Entries[EntryPos+1].StreamOffset+(IndexTables[Pos].IndexStartPosition+EntryPos+1)*SDTI_SizePerFrame;
                            else if (Pos+1<IndexTables.size() && !IndexTables[Pos+1].Entries.empty())
                                Entry1_StreamOffset=IndexTables[Pos+1].Entries[0].StreamOffset+(IndexTables[Pos].IndexStartPosition+EntryPos+1)*SDTI_SizePerFrame;

                            if (Entry0_StreamOffset>Entry_StreamOffset || Entry_StreamOffset>Entry1_StreamOffset)
                                break; //Problem

                            if (StreamOffset>=Entry_StreamOffset && StreamOffset<Entry1_StreamOffset)
                            {
                                Element_Size=StreamOffset_Offset+Buffer_Header_Size+Entry1_StreamOffset-(File_Offset+Buffer_Offset);
                                if (File_Offset+Buffer_Offset+Element_Size>Buffer_End)
                                {
                                    Element_Size=Buffer_End-(File_Offset+Buffer_Offset);
                                    break; //There is a problem
                                }

                                if (Buffer_Offset+Element_Size>Buffer_Size)
                                {
                                    //Hints
                                    if (File_Buffer_Size_Hint_Pointer)
                                    {
                                        size_t Buffer_Size_Target=(size_t)(Buffer_Offset+Element_Size-Buffer_Size+24); //+24 for next packet header
                                        if (Buffer_Size_Target<128*1024)
                                            Buffer_Size_Target=128*1024;
                                        //if ((*File_Buffer_Size_Hint_Pointer)<Buffer_Size_Target)
                                            (*File_Buffer_Size_Hint_Pointer)=Buffer_Size_Target;
                                    }

                                    return false;
                                }
                                break;
                            }
                        }
                    }
                }
            }
            else
        #endif //MEDIAINFO_DEMUX
            if (File_Offset+Buffer_Size<=Buffer_End)
                Element_Size=Buffer_Size-Buffer_Offset; //All the buffer is used
            else
                Element_Size=Buffer_End-(File_Offset+Buffer_Offset);

        Element_Begin0();
        Data_Parse();
        Buffer_Offset+=(size_t)Element_Size;
        Element_Size-=Element_Offset;
        Element_Offset=0;
        Element_End0();

        if (Buffer_End && File_Offset+Buffer_Offset+Element_Size>=Buffer_End)
        {
            Buffer_Begin=(int64u)-1;
            Buffer_End=0;
            Buffer_End_Unlimited=false;
            Buffer_Header_Size=0;
            MustSynchronize=true;
        }

        if (Buffer_Offset>=Buffer_Size)
            return false;

        #if MEDIAINFO_DEMUX
            if (Config->Demux_EventWasSent)
                return false;
        #endif //MEDIAINFO_DEMUX
    }

    return true;
}

//---------------------------------------------------------------------------
void File_Mxf::Header_Parse()
{
    //Parsing
    int64u Length;
    Get_UL(Code,                                                "Code", NULL);
    Get_BER(Length,                                             "Length");
    if (Element_IsWaitingForMoreData())
        return;

    if (Length==0
     && ((int32u)Code.hi)==Elements::GenericContainer_Aaf2
     && (((int32u)(Code.lo>>32))==Elements::GenericContainer_Aaf3 || ((int32u)(Code.lo>>32))==Elements::GenericContainer_Avid3)
     && Retrieve(Stream_General, 0, General_Format_Settings).find(__T(" / Incomplete"))!=string::npos
     )
    {
        if (Buffer_Offset+Element_Offset+4>Buffer_Size)
        {
            Element_WaitForMoreData();
            return;
        }

        if (BigEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset)!=0x060E2B34)
        {
            Buffer_End_Unlimited=true;
            Length=File_Size-(File_Offset+Buffer_Offset+Element_Offset);
        }
    }

    if (Config->File_IsGrowing && File_Offset+Buffer_Offset+Element_Offset+Length>File_Size)
    {
        Element_WaitForMoreData();
        return;
    }

    if (Length==0 && Essences.empty() && Retrieve(Stream_General, 0, General_Format_Settings).find(__T(" / Incomplete"))!=string::npos)
    {
        if (Buffer_Offset+Element_Offset+4>Buffer_Size)
        {
            Element_WaitForMoreData();
            return;
        }

        if (BigEndian2int32u(Buffer+Buffer_Offset+(size_t)Element_Offset)!=0x060E2B34)
        {
            Buffer_End_Unlimited=true;
            Length=File_Size-(File_Offset+Buffer_Offset+Element_Offset);
        }
    }

    if (Config->File_IsGrowing && File_Offset+Buffer_Offset+Element_Offset+Length>File_Size)
    {
        Element_WaitForMoreData();
        return;
    }

    //Filling
    int32u Code_Compare1=Code.hi>>32;
    int32u Code_Compare2=(int32u)Code.hi;
    int32u Code_Compare3=Code.lo>>32;
    int32u Code_Compare4=(int32u)Code.lo;
    if (Partitions_IsCalculatingHeaderByteCount)
    {
        if (!(Code_Compare1==Elements::Filler011
           && (Code_Compare2&0xFFFFFF00)==(Elements::Filler012&0xFFFFFF00)
           && Code_Compare3==Elements::Filler013))
        {
            Partitions_IsCalculatingHeaderByteCount=false;
            if (Partitions_Pos<Partitions.size())
                Partitions[Partitions_Pos].PartitionPackByteCount=File_Offset+Buffer_Offset-Partitions[Partitions_Pos].StreamOffset;
        }
    }
    if (Partitions_IsCalculatingSdtiByteCount)
    {
        if (!((Code_Compare1==Elements::SDTI_SystemMetadataPack1
            && (Code_Compare2&0xFF00FFFF)==(Elements::SDTI_SystemMetadataPack2&0xFF00FFFF) //Independent of Category
            && Code_Compare3==Elements::SDTI_SystemMetadataPack3
            && (Code_Compare4&0xFFFF0000)==(Elements::SDTI_SystemMetadataPack4&0xFFFF0000))
          || ((Code_Compare1==Elements::Filler011
            && (Code_Compare2&0xFFFFFF00)==(Elements::Filler012&0xFFFFFF00)
            && Code_Compare3==Elements::Filler013))))
        {
            if (Partitions_Pos<Partitions.size() && !SDTI_IsInIndexStreamOffset)
                SDTI_SizePerFrame=File_Offset+Buffer_Offset-(Partitions[Partitions_Pos].StreamOffset+Partitions[Partitions_Pos].PartitionPackByteCount+Partitions[Partitions_Pos].HeaderByteCount);
            Partitions_IsCalculatingSdtiByteCount=false;
        }
    }

    #if MEDIAINFO_NEXTPACKET && MEDIAINFO_DEMUX
        if (!Demux_HeaderParsed && !Partitions.empty() && Partitions[Partitions.size()-1].StreamOffset+Partitions[Partitions.size()-1].PartitionPackByteCount+Partitions[Partitions.size()-1].HeaderByteCount+Partitions[Partitions.size()-1].IndexByteCount==File_Offset+Buffer_Offset)
        {
            Demux_HeaderParsed=true;

            //Testing locators
            Locators_CleanUp();

            if (Config->File_IgnoreFramesBefore && !Config->File_IsDetectingDuration_Get())
                Open_Buffer_Seek(3, 0, (int64u)-1); //Forcing seek to Config->File_IgnoreFramesBefore
            if (Config->NextPacket_Get() && Config->Event_CallBackFunction_IsSet())
            {
                if (Locators.empty())
                {
                    Config->Demux_EventWasSent=true; //First set is to indicate the user that header is parsed
                    return;
                }
            }
        }
    #endif //MEDIAINFO_NEXTPACKET && MEDIAINFO_DEMUX

    #if MEDIAINFO_DEMUX || MEDIAINFO_SEEK
        if (IsParsingEnd && PartitionPack_Parsed && !Partitions.empty() && Partitions[Partitions.size()-1].StreamOffset+Partitions[Partitions.size()-1].PartitionPackByteCount+Partitions[Partitions.size()-1].HeaderByteCount+Partitions[Partitions.size()-1].IndexByteCount==File_Offset+Buffer_Offset
         && !(Code_Compare1==Elements::RandomIndexMetadata1 && Code_Compare2==Elements::RandomIndexMetadata2 && Code_Compare3==Elements::RandomIndexMetadata3 && Code_Compare4==Elements::RandomIndexMetadata4))
        {
            if (RandomIndexMetadatas.empty())
            {
                if (!RandomIndexMetadatas_AlreadyParsed)
                {
                    Partitions_Pos=0;
                    while (Partitions_Pos<Partitions.size() && Partitions[Partitions_Pos].StreamOffset!=PartitionMetadata_PreviousPartition)
                        Partitions_Pos++;
                    if (Partitions_Pos==Partitions.size())
                    {
                        GoTo(PartitionMetadata_PreviousPartition);
                        Open_Buffer_Unsynch();
                    }
                    else
                        TryToFinish();
                }
            }
            else
            {
                GoTo(RandomIndexMetadatas[0].ByteOffset);
                RandomIndexMetadatas.erase(RandomIndexMetadatas.begin());
                PartitionPack_Parsed=false;
                Open_Buffer_Unsynch();
            }
            return;
        }
    #endif //MEDIAINFO_DEMUX || MEDIAINFO_SEEK

    if (Buffer_Offset+Element_Offset+Length>(size_t)-1 || Buffer_Offset+(size_t)(Element_Offset+Length)>Buffer_Size) //Not complete
    {
        if (Length>File_Size/2) //Divided by 2 for testing if this is a big chunk = Clip based and not frames.
        {
            //Calculating the byte count not included in seek information (partition, index...)
            int64u StreamOffset_Offset;
            if (!Partitions.empty())
            {
                while (Partitions_Pos<Partitions.size() && Partitions[Partitions_Pos].StreamOffset<File_Offset+Buffer_Offset-Header_Size)
                    Partitions_Pos++;
                if (Partitions_Pos && (Partitions_Pos==Partitions.size() || Partitions[Partitions_Pos].StreamOffset!=File_Offset+Buffer_Offset-Header_Size))
                    Partitions_Pos--; //This is the previous item
                StreamOffset_Offset=Partitions[Partitions_Pos].StreamOffset-Partitions[Partitions_Pos].BodyOffset+Partitions[Partitions_Pos].PartitionPackByteCount+Partitions[Partitions_Pos].HeaderByteCount+Partitions[Partitions_Pos].IndexByteCount;
            }
            else
                StreamOffset_Offset=0;

            if (StreamOffset_Offset<=File_Offset+Buffer_Offset
             && !Partitions_IsFooter
             && !(Code_Compare1==Elements::OpenIncompleteHeaderPartition1   //Skipping any kind of Partition
               && Code_Compare2==Elements::OpenIncompleteHeaderPartition2
               && Code_Compare3==Elements::OpenIncompleteHeaderPartition3)
             && !(Code_Compare1==Elements::IndexTableSegment1               //Skipping any kind of IndexTableSegment
               && Code_Compare2==Elements::IndexTableSegment2
               && Code_Compare3==Elements::IndexTableSegment3))
            {
                Buffer_Begin=File_Offset+Buffer_Offset+Element_Offset;
                Buffer_End=Buffer_Begin+Length;
                Buffer_Header_Size=Element_Offset;
                MustSynchronize=false;
                Length=0;
                #if MEDIAINFO_DEMUX || MEDIAINFO_SEEK
                Clip_Begin=Buffer_Begin;
                Clip_End=Buffer_End;
                Clip_Header_Size=Buffer_Header_Size;
                Clip_Code=Code;
                #endif //MEDIAINFO_DEMUX || MEDIAINFO_SEEK
            }
        }

        if (Buffer_Begin==(int64u)-1)
        {
            if (Length<=File_Size/2) //Divided by 2 for testing if this is a big chunk = Clip based and not frames.))
            {
                if (File_Buffer_Size_Hint_Pointer)
                {
                    int64u Buffer_Size_Target=(size_t)(Buffer_Offset+Element_Offset+Length-Buffer_Size+24); //+24 for next packet header

                    if (Buffer_Size_Target<128*1024)
                        Buffer_Size_Target=128*1024;
                    //if ((*File_Buffer_Size_Hint_Pointer)<Buffer_Size_Target)
                        (*File_Buffer_Size_Hint_Pointer)=(size_t)Buffer_Size_Target;
                }


                Element_WaitForMoreData();
                return;
            }
        }
    }

    #if MEDIAINFO_TRACE
        Header_Fill_Code(0, Ztring::ToZtring(Code.hi, 16)+Ztring::ToZtring(Code.lo, 16));
    #else //MEDIAINFO_TRACE
        Header_Fill_Code(0);
    #endif //MEDIAINFO_TRACE
    Header_Fill_Size(Element_Offset+Length);
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
            if (Buffer_End==0) \
            { \
                Element_WaitForMoreData(); \
                return; \
            } \
            Skip_XX(Element_Size, "Data"); \
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
                                            Element_Begin0(); \
                                            Element_Begin1("Header"); \
                                                Get_B2 (Code2,                                  "Code"); \
                                                Get_B2 (Length2,                                "Length"); \
                                            Element_End0(); \
                                            Element_Name(Ztring().From_CC2(Code2)); \
                                            \
                                            int64u End=Element_Offset+Length2; \
                                            _ELEMENT(); \
                                            if (Element_Offset<End) \
                                                Skip_XX(End-Element_Offset,                     "Unknown"); \
                                            \
                                            Element_End0(); \
                                        } \
                                        break; \
                            case 0x63 : _ELEMENT(); break; \
                            default   : Skip_XX(Element_Size,                                   "Unknown"); \
                        } \
        } \
    } \

    //Parsing
    if (0) {}
    ELEMENT(Filler01,                                           "Filler")
    ELEMENT(Filler02,                                           "Padding")
    ELEMENT(TerminatingFiller,                                  "Terminating Filler")
    ELEMENT(XmlDocumentText,                                    "XML Document Text")
    ELEMENT(SubDescriptors,                                     "Sub Descriptors")
    ELEMENT(Sequence,                                           "Sequence")
    ELEMENT(SourceClip,                                         "Source Clip")
    ELEMENT(TimecodeComponent,                                  "Timecode Component")
    ELEMENT(ContentStorage,                                     "Content Storage")
    ELEMENT(EssenceContainerData,                               "Essence Container Data")
    ELEMENT(GenericPictureEssenceDescriptor,                    "Generic Picture Essence Descriptor")
    ELEMENT(CDCIEssenceDescriptor,                              "CDCI Essence Descriptor")
    ELEMENT(RGBAEssenceDescriptor,                              "RGBA Essence Descriptor")
    ELEMENT(Preface,                                            "Preface")
    ELEMENT(Identification,                                     "Identification")
    ELEMENT(NetworkLocator,                                     "Network Locator")
    ELEMENT(TextLocator,                                        "Text Locator")
    ELEMENT(StereoscopicPictureSubDescriptor,                   "Stereoscopic Picture Sub Descriptor")
    ELEMENT(MaterialPackage,                                    "Material Package")
    ELEMENT(SourcePackage,                                      "Source Package")
    ELEMENT(EventTrack,                                         "Event track")
    ELEMENT(StaticTrack,                                        "Static Track")
    ELEMENT(Track,                                              "Track")
    ELEMENT(DMSegment,                                          "Descriptive Metadata Segment")
    ELEMENT(GenericSoundEssenceDescriptor,                      "Generic Sound Essence Descriptor")
    ELEMENT(GenericDataEssenceDescriptor,                       "Generic Data Essence Descriptor")
    ELEMENT(MultipleDescriptor,                                 "Multiple Descriptor")
    ELEMENT(AES3PCMDescriptor,                                  "AES3 Descriptor")
    ELEMENT(WaveAudioDescriptor,                                "Wave Audio Descriptor")
    ELEMENT(MPEG2VideoDescriptor,                               "MPEG-2 Video Descriptor")
    ELEMENT(JPEG2000PictureSubDescriptor,                       "JPEG 2000 Picture Sub Descriptor")
    ELEMENT(VbiPacketsDescriptor,                               "VBI Descriptor")
    ELEMENT(AncPacketsDescriptor,                               "ANC Packets Descriptor")
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
    ELEMENT(Primer,                                             "Primer")
    ELEMENT(IndexTableSegment,                                  "Index Table (Segment)")
    ELEMENT(RandomIndexMetadata,                                "Random Index Metadata")
    ELEMENT(SDTI_SystemMetadataPack,                            "SDTI System Metadata Pack")
    else if (Code_Compare1==Elements::SDTI_SystemMetadataPack1
          && ((Code_Compare2)&0xFF00FFFF)==(Elements::SDTI_SystemMetadataPack2&0xFF00FFFF)
          && Code_Compare3==Elements::SDTI_SystemMetadataPack3
          && ((Code_Compare4)&0xFFFF0000)==(Elements::SDTI_SystemMetadataPack4&0xFFFF0000))
    {
        Code_Compare4&=0xFFFFFF00; //Remove MetaData Block Count
        if (0) {}
        ELEMENT(SDTI_PackageMetadataSet,                        "SDTI Package Metadata Set")
        ELEMENT(SDTI_PictureMetadataSet,                        "SDTI Picture Metadata Set")
        ELEMENT(SDTI_SoundMetadataSet,                          "SDTI Sound Metadata Set")
        ELEMENT(SDTI_DataMetadataSet,                           "SDTI Data Metadata Set")
        ELEMENT(SDTI_ControlMetadataSet,                        "SDTI Control Metadata Set")
    }
    ELEMENT(SystemScheme1,                                      "System Scheme 1") //SMPTE 405M
    ELEMENT(DMScheme1,                                          "Descriptive Metadata Scheme 1") //SMPTE 380M
    ELEMENT(Omneon_010201010100,                                "Omneon (010201010100)")
    ELEMENT(Omneon_010201020100,                                "Omneon (010201020100)")
    else if (Code_Compare1==Elements::GenericContainer_Aaf1
          && ((Code_Compare2)&0xFFFFFF00)==(Elements::GenericContainer_Aaf2&0xFFFFFF00)
          && (Code_Compare3==Elements::GenericContainer_Aaf3
           || Code_Compare3==Elements::GenericContainer_Avid3
           || Code_Compare3==Elements::GenericContainer_Sony3))
    {
        Element_Name(Mxf_EssenceElement(Code));

        //Config
        #if MEDIAINFO_DEMUX || MEDIAINFO_SEEK
        if (!Essences_FirstEssence_Parsed)
        {
            if (Descriptors.size()==1 && Descriptors.begin()->second.StreamKind==Stream_Audio)
            {
                //Configuring bitrate is not available in descriptor
                if (Descriptors.begin()->second.ByteRate==(int32u)-1 && Descriptors.begin()->second.Infos.find("SamplingRate")!=Descriptors.begin()->second.Infos.end())
                {
                    int32u SamplingRate=Descriptors.begin()->second.Infos["SamplingRate"].To_int32u();

                    if (Descriptors.begin()->second.BlockAlign!=(int16u)-1)
                        Descriptors.begin()->second.ByteRate=SamplingRate*Descriptors.begin()->second.BlockAlign;
                    else if (Descriptors.begin()->second.QuantizationBits!=(int8u)-1)
                        Descriptors.begin()->second.ByteRate=SamplingRate*Descriptors.begin()->second.QuantizationBits/8;
                }

                //Configuring EditRate if needed (e.g. audio at 48000 Hz)
                if (Demux_Rate) //From elsewhere
                {
                    Descriptors.begin()->second.SampleRate=Demux_Rate;
                }
                else if (Descriptors.begin()->second.SampleRate>1000)
                {
                    float64 EditRate_FromTrack=DBL_MAX;
                    for (tracks::iterator Track=Tracks.begin(); Track!=Tracks.end(); ++Track)
                        if (EditRate_FromTrack>Track->second.EditRate)
                            EditRate_FromTrack=Track->second.EditRate;
                    if (EditRate_FromTrack>1000)
                        Descriptors.begin()->second.SampleRate=24; //Default value
                    else
                        Descriptors.begin()->second.SampleRate=EditRate_FromTrack;
                    for (tracks::iterator Track=Tracks.begin(); Track!=Tracks.end(); ++Track)
                        if (Track->second.EditRate>EditRate_FromTrack)
                        {
                            Track->second.EditRate_Real=Track->second.EditRate;
                            Track->second.EditRate=EditRate_FromTrack;
                        }
                }
            }

            Essences_FirstEssence_Parsed=true;
        }
        #endif //MEDIAINFO_DEMUX || MEDIAINFO_SEEK

        if (IsParsingEnd)
        {
            //We have the necessary for indexes, jumping to next index
            Skip_XX(Element_Size,                               "Data");
            if (RandomIndexMetadatas.empty())
            {
                if (!RandomIndexMetadatas_AlreadyParsed)
                {
                    Partitions_Pos=0;
                    while (Partitions_Pos<Partitions.size() && Partitions[Partitions_Pos].StreamOffset!=PartitionMetadata_PreviousPartition)
                        Partitions_Pos++;
                    if (Partitions_Pos==Partitions.size())
                    {
                        GoTo(PartitionMetadata_PreviousPartition);
                        Open_Buffer_Unsynch();
                    }
                    else
                        TryToFinish();
                }
            }
            else
            {
                GoTo(RandomIndexMetadatas[0].ByteOffset);
                RandomIndexMetadatas.erase(RandomIndexMetadatas.begin());
                Open_Buffer_Unsynch();
            }
            return;
        }

        essences::iterator Essence=Essences.find(Code_Compare4);
        if (Essence==Essences.end())
            Essence=Essences.insert(make_pair(Code_Compare4,essence())).first;

        if (Essence->second.Parsers.empty())
        {
            //Format_Settings_Wrapping
            if (Descriptors.size()==1 && (Descriptors.begin()->second.Infos.find("Format_Settings_Wrapping")==Descriptors.begin()->second.Infos.end() || Descriptors.begin()->second.Infos["Format_Settings_Wrapping"].empty()) && (Buffer_End?(Buffer_End-Buffer_Begin):Element_Size)>File_Size/2) //Divided by 2 for testing if this is a big chunk = Clip based and not frames.
                Descriptors.begin()->second.Infos["Format_Settings_Wrapping"]=__T("Clip"); //By default, not sure about it, should be from descriptor

            //Searching the corresponding Track (for TrackID)
            if (!Essence->second.TrackID_WasLookedFor)
            {
                for (tracks::iterator Track=Tracks.begin(); Track!=Tracks.end(); ++Track)
                    if (Track->second.TrackNumber==Code_Compare4)
                        Essence->second.TrackID=Track->second.TrackID;
                #if MEDIAINFO_DEMUX || MEDIAINFO_SEEK
                    if (Essence->second.TrackID==(int32u)-1 && !Duration_Detected && !Config->File_IsDetectingDuration_Get())
                    {
                        DetectDuration(); //In one file (*-009.mxf), the TrackNumber is known only at the end of the file (Open and incomplete header/footer)
                        for (tracks::iterator Track=Tracks.begin(); Track!=Tracks.end(); ++Track)
                            if (Track->second.TrackNumber==Code_Compare4)
                                Essence->second.TrackID=Track->second.TrackID;
                    }
                #endif //MEDIAINFO_DEMUX || MEDIAINFO_SEEK
                Essence->second.TrackID_WasLookedFor=true;
            }

            //Searching the corresponding Descriptor
            for (descriptors::iterator Descriptor=Descriptors.begin(); Descriptor!=Descriptors.end(); ++Descriptor)
                if (Descriptors.size()==1 || (Descriptor->second.LinkedTrackID==Essence->second.TrackID && Descriptor->second.LinkedTrackID!=(int32u)-1))
                {
                    Essence->second.StreamPos_Initial=Essence->second.StreamPos=Code_Compare4&0x000000FF;
                    if ((Code_Compare4&0x000000FF)==0x00000000)
                        StreamPos_StartAtOne=false;

                    if (Descriptor->second.StreamKind==Stream_Audio && Descriptor->second.Infos.find("Format_Settings_Endianness")==Descriptor->second.Infos.end())
                    {
                        Ztring Format;
                        Format.From_Local(Mxf_EssenceCompression(Descriptor->second.EssenceCompression));
                        if (Format.empty())
                            Format.From_Local(Mxf_EssenceContainer(Descriptor->second.EssenceContainer));
                        if (Format.find(__T("PCM"))==0)
                            Descriptor->second.Infos["Format_Settings_Endianness"]=__T("Little");
                    }

                    ChooseParser(Essence, Descriptor); //Searching by the descriptor
                    if (Essence->second.Parsers.empty())
                        ChooseParser__FromEssence(Essence, Descriptor); //Searching by the track identifier

                    #ifdef MEDIAINFO_VC3_YES
                        if (Ztring().From_Local(Mxf_EssenceContainer(Descriptor->second.EssenceContainer))==__T("VC-3"))
                            ((File_Vc3*)(*(Essence->second.Parsers.begin())))->FrameRate=Descriptor->second.SampleRate;
                    #endif //MEDIAINFO_VC3_YES
                    break;
                }

            //Searching by the track identifier
            if (Essence->second.Parsers.empty())
                ChooseParser__FromEssence(Essence, Descriptors.end());

            //Check of Essence used as a reference for frame count
            if (Essences_UsedForFrameCount==(int32u)-1)
                Essences_UsedForFrameCount=Essence->first;
            else if ((Essence->second.StreamKind==Stream_Audio && Essences[Essences_UsedForFrameCount].StreamKind>Stream_Audio)
                  || (Essence->second.StreamKind==Stream_Video && Essences[Essences_UsedForFrameCount].StreamKind>Stream_Video))
                    Essences_UsedForFrameCount=Essence->first;

            //Demux
            #if MEDIAINFO_DEMUX
                //Configuration
                if (!IsSub) //Updating for MXF only if MXF is not embedded in another container
                {
                    Essence->second.Frame_Count_NotParsedIncluded=Frame_Count_NotParsedIncluded;
                    if (Essence->second.Frame_Count_NotParsedIncluded!=(int64u)-1 && Essence->second.Frame_Count_NotParsedIncluded)
                        Essence->second.Frame_Count_NotParsedIncluded--; //Info is from the first essence parsed, and 1 frame is already parsed
                    Essence->second.FrameInfo.DTS=FrameInfo.DTS;
                    if (Essence->second.FrameInfo.DTS!=(int64u)-1 && FrameInfo.DUR!=(int64u)-1)
                        Essence->second.FrameInfo.DTS-=FrameInfo.DUR; //Info is from the first essence parsed, and 1 frame is already parsed
                    if (!Tracks.empty() && Tracks.begin()->second.EditRate) //TODO: use the corresponding track instead of the first one
                        Essence->second.FrameInfo.DUR=float64_int64s(1000000000/Tracks.begin()->second.EditRate);
                    else if (!IndexTables.empty() && IndexTables[0].IndexEditRate)
                        Essence->second.FrameInfo.DUR=float64_int64s(1000000000/IndexTables[0].IndexEditRate);
                    #if MEDIAINFO_DEMUX
                        if (Buffer_End && Demux_UnpacketizeContainer && Essences.size()==1 && !Essences.begin()->second.Parsers.empty() && !(*(Essences.begin()->second.Parsers.begin()))->Demux_UnpacketizeContainer)
                            for (parsers::iterator Parser=Essence->second.Parsers.begin(); Parser!=Essence->second.Parsers.end(); ++Parser)
                            {
                                (*Parser)->Demux_Level=2; //Container
                                (*Parser)->Demux_UnpacketizeContainer=true;
                            }
                    #endif //MEDIAINFO_DEMUX
                }
                if (Essence->second.TrackID!=(int32u)-1)
                    Element_Code=Essence->second.TrackID;
                else
                    Element_Code=Code.lo;
            #endif //MEDIAINFO_DEMUX

            if (Essence->second.Parsers.empty())
            {
                if (Streams_Count>0)
                    Streams_Count--;
            }
            else
            {
                Element_Code=Essence->second.TrackID;
                for (parsers::iterator Parser=Essence->second.Parsers.begin(); Parser!=Essence->second.Parsers.end(); ++Parser)
                {
                    Open_Buffer_Init(*Parser);
                    if ((*Parser)->Status[IsFinished])
                        if (Streams_Count>0)
                            Streams_Count--;
                }
            }

            //Stream size is sometime easy to find
            if ((Buffer_End?(Buffer_End-Buffer_Begin):Element_TotalSize_Get())>=File_Size*0.98) //let imagine: if element size is 98% of file size, this is the only one element in the file
            {
                Essence->second.Stream_Size=Buffer_End?(Buffer_End-Buffer_Begin):Element_TotalSize_Get();
            }

            //Compute stream bit rate if there is only one stream
            int64u Stream_Size;
            if (Essence->second.Stream_Size!=(int64u)-1)
                Stream_Size=Essence->second.Stream_Size;
            else
                Stream_Size=File_Size; //TODO: find a way to remove header/footer correctly
            if (Stream_Size!=(int64u)-1)
            {
                if (Descriptors.size()==1 && Descriptors.begin()->second.ByteRate!=(int32u)-1)
                    for (parsers::iterator Parser=Essence->second.Parsers.begin(); Parser!=Essence->second.Parsers.end(); ++Parser)
                        (*Parser)->Stream_BitRateFromContainer=Descriptors.begin()->second.ByteRate*8;
                else if (Descriptors.size()==1 && Descriptors.begin()->second.Infos["Duration"].To_float64())
                    for (parsers::iterator Parser=Essences.begin()->second.Parsers.begin(); Parser!=Essences.begin()->second.Parsers.end(); ++Parser)
                        (*Parser)->Stream_BitRateFromContainer=((float64)Stream_Size)*8/(Descriptors.begin()->second.Infos["Duration"].To_float64()/1000);
            }
        }

        //Frame info is specific to the container, and it is not updated
        frame_info FrameInfo_Temp=FrameInfo;
        int64u Frame_Count_NotParsedIncluded_Temp=Frame_Count_NotParsedIncluded;
        if (!IsSub) //Updating for MXF only if MXF is not embedded in another container
        {
            FrameInfo=frame_info();
            Frame_Count_NotParsedIncluded=(int64u)-1;
        }

        //Demux
        #if MEDIAINFO_DEMUX
            if (Essence->second.TrackID!=(int32u)-1)
                Element_Code=Essence->second.TrackID;
            else
                Element_Code=Code.lo;
            Demux_Level=(!Essence->second.Parsers.empty() && ((*(Essence->second.Parsers.begin()))->Demux_UnpacketizeContainer || (*(Essence->second.Parsers.begin()))->Demux_Level==2))?4:2; //Intermediate (D-10 Audio) / Container
            if (!IsSub) //Updating for MXF only if MXF is not embedded in another container
            {
                FrameInfo=Essence->second.FrameInfo;
                Frame_Count_NotParsedIncluded=Essence->second.Frame_Count_NotParsedIncluded;
            }
            Demux_random_access=true;
            Demux(Buffer+Buffer_Offset, (size_t)Element_Size, ContentType_MainStream);
        #endif //MEDIAINFO_DEMUX

        if (!Essence->second.Parsers.empty() && !(*(Essence->second.Parsers.begin()))->Status[IsFinished])
        {
            if ((Code_Compare4&0xFF00FF00)==0x17000100 || (Code_Compare4&0xFF00FF00)==0x17000200)
            {
                if (Element_Size)
                {
                    parsers::iterator Parser=Essence->second.Parsers.begin();

                    //Ancillary with
                    int16u Count;
                    Get_B2 (Count,                                  "Number of Lines");
                    if (Count*14>Element_Size)
                    {
                        (*Parser)->Finish();
                        Skip_XX(Element_Size-2,                     "Unknown");
                        Count=0;
                    }
                    for (int16u Pos=0; Pos<Count; Pos++)
                    {
                        Element_Begin1("Packet");
                        int32u Size2, Count2;
                        int16u LineNumber, Size;
                        Get_B2 (LineNumber,                         "Line Number");
                        Skip_B1(                                    "Wrapping Type");
                        Skip_B1(                                    "Payload Sample Coding");
                        Get_B2 (Size,                               "Payload Sample Count");
                        Get_B4 (Size2,                              "Size?");
                        Get_B4 (Count2,                             "Count?");

                        if (Essence->second.Frame_Count_NotParsedIncluded!=(int64u)-1)
                            (*Parser)->Frame_Count_NotParsedIncluded=Essence->second.Frame_Count_NotParsedIncluded;
                        if (Essence->second.FrameInfo.DTS!=(int64u)-1)
                            (*Parser)->FrameInfo.DTS=Essence->second.FrameInfo.DTS;
                        if (Essence->second.FrameInfo.PTS!=(int64u)-1)
                            (*Parser)->FrameInfo.PTS=Essence->second.FrameInfo.PTS;
                        if (Essence->second.FrameInfo.DUR!=(int64u)-1)
                            (*Parser)->FrameInfo.DUR=Essence->second.FrameInfo.DUR;
                        if ((*Parser)->ParserName==__T("Ancillary"))
                            ((File_Ancillary*)(*Parser))->LineNumber=LineNumber;
                        if ((*Parser)->ParserName==__T("Ancillary") && (((File_Ancillary*)(*Parser))->FrameRate==0 || ((File_Ancillary*)(*Parser))->AspectRatio==0))
                        {
                            //Configuring with video info
                            for (descriptors::iterator Descriptor=Descriptors.begin(); Descriptor!=Descriptors.end(); ++Descriptor)
                                if (Descriptor->second.StreamKind==Stream_Video)
                                {
                                    ((File_Ancillary*)(*Parser))->HasBFrames=Descriptor->second.HasBFrames;
                                    ((File_Ancillary*)(*Parser))->AspectRatio=Descriptor->second.DisplayAspectRatio;
                                    ((File_Ancillary*)(*Parser))->FrameRate=Descriptor->second.SampleRate;
                                    break;
                                }
                        }
                        if (Element_Offset+Size>Element_Size)
                            Size=(int16u)(Element_Size-Element_Offset);
                        Open_Buffer_Continue((*Parser), Buffer+Buffer_Offset+(size_t)(Element_Offset), Size);
                        if ((Code_Compare4&0xFF00FF00)==0x17000100 && LineNumber==21 && (*Parser)->Count_Get(Stream_Text)==0)
                        {
                            (*Parser)->Accept();
                            (*Parser)->Stream_Prepare(Stream_Text);
                            (*Parser)->Fill(Stream_Text, StreamPos_Last, Text_Format, "EIA-608");
                            (*Parser)->Fill(Stream_Text, StreamPos_Last, Text_MuxingMode, "VBI / Line 21");
                        }
                        Element_Offset+=Size;
                        if (Size<Size2*Count2)
                            Skip_XX(Size2*Count2-Size,              "Padding");
                        Element_End0();
                    }
                }
            }
            else
            {
                for (size_t Pos=0; Pos<Essence->second.Parsers.size(); Pos++)
                {
                    //Parsing
                    if (Essence->second.Frame_Count_NotParsedIncluded!=(int64u)-1)
                        Essence->second.Parsers[Pos]->Frame_Count_NotParsedIncluded=Essence->second.Frame_Count_NotParsedIncluded;
                    if (Essence->second.FrameInfo.DTS!=(int64u)-1)
                        Essence->second.Parsers[Pos]->FrameInfo.DTS=Essence->second.FrameInfo.DTS;
                    if (Essence->second.FrameInfo.PTS!=(int64u)-1)
                        Essence->second.Parsers[Pos]->FrameInfo.PTS=Essence->second.FrameInfo.PTS;
                    if (Essence->second.FrameInfo.DUR!=(int64u)-1)
                        Essence->second.Parsers[Pos]->FrameInfo.DUR=Essence->second.FrameInfo.DUR;
                    Open_Buffer_Continue(Essence->second.Parsers[Pos], Buffer+Buffer_Offset, (size_t)Element_Size);
                    #if MEDIAINFO_DEMUX
                        if (Demux_Level==4 && Config->Demux_EventWasSent && Essence->second.StreamKind==Stream_Video && Essence->second.Parsers[Pos]->ParserIDs[StreamIDs_Size]==MediaInfo_Parser_Jpeg) // Only File_Jpeg. TODO: limit to File_Jpeg instead of video streams
                        {
                            Demux_CurrentParser=Essence->second.Parsers[Pos];
                            Demux_CurrentEssence=Essence;
                        }
                    #endif //MEDIAINFO_DEMUX
                    switch (Essence->second.Parsers[Pos]->Field_Count_InThisBlock)
                    {
                        case 1 : Essence->second.Field_Count_InThisBlock_1++; break;
                        case 2 : Essence->second.Field_Count_InThisBlock_2++; break;
                        default: ;
                    }

                    //Multiple parsers
                    if (Essence->second.Parsers.size()>1)
                    {
                        if (!Essence->second.Parsers[Pos]->Status[IsAccepted] && Essence->second.Parsers[Pos]->Status[IsFinished])
                        {
                            delete *(Essence->second.Parsers.begin()+Pos);
                            Essence->second.Parsers.erase(Essence->second.Parsers.begin()+Pos);
                            Pos--;
                        }
                        else if (Essence->second.Parsers.size()>1 && Essence->second.Parsers[Pos]->Status[IsAccepted])
                        {
                            File__Analyze* Parser=Essence->second.Parsers[Pos];
                            for (size_t Pos2=0; Pos2<Essence->second.Parsers.size(); Pos2++)
                            {
                                if (Pos2!=Pos)
                                    delete *(Essence->second.Parsers.begin()+Pos2);
                            }
                            Essence->second.Parsers.clear();
                            Essence->second.Parsers.push_back(Parser);
                        }
                    }
                }

                Element_Offset=Element_Size;
            }

            if (Essence->second.Parsers.size()==1 && Essence->second.Parsers[0]->Status[IsAccepted] && Essence->second.Frame_Count_NotParsedIncluded==(int64u)-1)
            {
                Essence->second.Frame_Count_NotParsedIncluded=Essence->second.Parsers[0]->Frame_Count_NotParsedIncluded;
                Essence->second.FrameInfo.DTS=Essence->second.Parsers[0]->FrameInfo.DTS;
                Essence->second.FrameInfo.PTS=Essence->second.Parsers[0]->FrameInfo.PTS;
                Essence->second.FrameInfo.DUR=Essence->second.Parsers[0]->FrameInfo.DUR;
            }
            else if (Buffer_End)
            {
                Essence->second.Frame_Count_NotParsedIncluded=(int64u)-1;
                Essence->second.FrameInfo=frame_info();
            }
            else
            {
                if (Essence->second.Frame_Count_NotParsedIncluded!=(int64u)-1)
                    Essence->second.Frame_Count_NotParsedIncluded++;
                if (Essence->second.FrameInfo.DTS!=(int64u)-1 && Essence->second.FrameInfo.DUR!=(int64u)-1)
                    Essence->second.FrameInfo.DTS+=Essence->second.FrameInfo.DUR;
                if (Essence->second.FrameInfo.PTS!=(int64u)-1 && Essence->second.FrameInfo.DUR!=(int64u)-1)
                    Essence->second.FrameInfo.PTS+=Essence->second.FrameInfo.DUR;
            }

            //Disabling this Streams
            if (!Essence->second.IsFilled && Essence->second.Parsers.size()==1 && Essence->second.Parsers[0]->Status[IsFilled])
            {
                if (Streams_Count>0)
                    Streams_Count--;
                Essence->second.IsFilled=true;
                if (Config->ParseSpeed<1.0 && IsSub)
                {
                    Fill();
                    Open_Buffer_Unsynch();
                    Finish();
                }
            }
        }
        else
            Skip_XX(Element_Size,                               "Data");

        //Frame info is specific to the container, and it is not updated
        if (Essence->first==Essences_UsedForFrameCount)
        {
            FrameInfo=Essence->second.FrameInfo;
            Frame_Count_NotParsedIncluded=Essence->second.Frame_Count_NotParsedIncluded;
        }
        else
        {
            FrameInfo=FrameInfo_Temp;
            Frame_Count_NotParsedIncluded=Frame_Count_NotParsedIncluded_Temp;
        }

        //Ignore tail
        if (Config->ParseSpeed>=1.0 && Frame_Count_NotParsedIncluded!=(int64u)-1 && Config->File_IgnoreFramesAfter!=(int64u)-1 && Frame_Count_NotParsedIncluded>=Config->File_IgnoreFramesAfter)
        {
            if (PartitionMetadata_FooterPartition!=(int64u)-1)
                GoTo(PartitionMetadata_FooterPartition);
            else
                GoToFromEnd(0);
        }
    }
    else
        Skip_XX(Element_Size,                                   "Unknown");

    if (Buffer_End && File_Offset+Buffer_Offset+Element_Size>=Buffer_End)
    {
        Buffer_Begin=(int64u)-1;
        Buffer_End=0;
        Buffer_End_Unlimited=false;
        Buffer_Header_Size=0;
        MustSynchronize=true;
    }

    if ((!IsParsingEnd && IsParsingMiddle_MaxOffset==(int64u)-1 && MediaInfoLib::Config.ParseSpeed_Get()<1.0)
     && ((!IsSub && File_Offset>=0x4000000) //TODO: 64 MB by default (security), should be changed
      || (Streams_Count==0 && !Descriptors.empty())))
    {
        Fill();

        IsParsingEnd=true;
        if (PartitionMetadata_FooterPartition!=(int64u)-1)
        {
            GoTo(PartitionMetadata_FooterPartition);
            IsCheckingFooterPartitionAddress=true;
        }
        else
        {
            GoToFromEnd(4); //For random access table
            FooterPartitionAddress_Jumped=true;
        }
        Open_Buffer_Unsynch();
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
    Descriptors[InstanceUID].IsAes3Descriptor=true;

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

    if (Descriptors[InstanceUID].Infos["ColorSpace"].empty())
        Descriptors[InstanceUID].Infos["ColorSpace"]="YUV";
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

    #if MEDIAINFO_NEXTPACKET && MEDIAINFO_DEMUX
        if (!Demux_HeaderParsed)
        {
            Demux_HeaderParsed=true;

            //Testing locators
            Locators_CleanUp();

            if (Config->File_IgnoreFramesBefore && !Config->File_IsDetectingDuration_Get())
                Open_Buffer_Seek(3, 0, (int64u)-1); //Forcing seek to Config->File_IgnoreFramesBefore
            if (Config->NextPacket_Get() && Config->Event_CallBackFunction_IsSet())
            {
                if (Locators.empty())
                {
                    Config->Demux_EventWasSent=true; //First set is to indicate the user that header is parsed
                    return;
                }
            }
        }
    #endif //MEDIAINFO_NEXTPACKET && MEDIAINFO_DEMUX
}

//---------------------------------------------------------------------------
void File_Mxf::ClosedIncompleteBodyPartition()
{
    //Parsing
    PartitionMetadata();

    #if MEDIAINFO_NEXTPACKET && MEDIAINFO_DEMUX
        if (!Demux_HeaderParsed)
        {
            Demux_HeaderParsed=true;

            //Testing locators
            Locators_CleanUp();

            if (Config->File_IgnoreFramesBefore && !Config->File_IsDetectingDuration_Get())
                Open_Buffer_Seek(3, 0, (int64u)-1); //Forcing seek to Config->File_IgnoreFramesBefore
            if (Config->NextPacket_Get() && Config->Event_CallBackFunction_IsSet())
            {
                if (Locators.empty())
                {
                    Config->Demux_EventWasSent=true; //First set is to indicate the user that header is parsed
                    return;
                }
            }
        }
    #endif //MEDIAINFO_NEXTPACKET && MEDIAINFO_DEMUX
}

//---------------------------------------------------------------------------
void File_Mxf::OpenCompleteBodyPartition()
{
    //Parsing
    PartitionMetadata();

    #if MEDIAINFO_NEXTPACKET && MEDIAINFO_DEMUX
        if (!Demux_HeaderParsed)
        {
            Demux_HeaderParsed=true;

            //Testing locators
            Locators_CleanUp();

            if (Config->File_IgnoreFramesBefore && !Config->File_IsDetectingDuration_Get())
                Open_Buffer_Seek(3, 0, (int64u)-1); //Forcing seek to Config->File_IgnoreFramesBefore
            if (Config->NextPacket_Get() && Config->Event_CallBackFunction_IsSet())
            {
                if (Locators.empty())
                {
                    Config->Demux_EventWasSent=true; //First set is to indicate the user that header is parsed
                    return;
                }
            }
        }
    #endif //MEDIAINFO_NEXTPACKET && MEDIAINFO_DEMUX
}

//---------------------------------------------------------------------------
void File_Mxf::ClosedCompleteBodyPartition()
{
    //Parsing
    PartitionMetadata();

    #if MEDIAINFO_NEXTPACKET && MEDIAINFO_DEMUX
        if (!Demux_HeaderParsed)
        {
            Demux_HeaderParsed=true;

            //Testing locators
            Locators_CleanUp();

            if (Config->File_IgnoreFramesBefore && !Config->File_IsDetectingDuration_Get())
                Open_Buffer_Seek(3, 0, (int64u)-1); //Forcing seek to Config->File_IgnoreFramesBefore
            if (Config->NextPacket_Get() && Config->Event_CallBackFunction_IsSet())
            {
                if (Locators.empty())
                {
                    Config->Demux_EventWasSent=true; //First set is to indicate the user that header is parsed
                    return;
                }
            }
        }
    #endif //MEDIAINFO_NEXTPACKET && MEDIAINFO_DEMUX
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
        Element_Info1("Valid from Preface");
        Element_Level++;
    }
}

//---------------------------------------------------------------------------
void File_Mxf::DMSegment()
{
    switch(Code2)
    {
        ELEMENT(6101, DMSegment_DMFramework,                    "DM Framework")
        ELEMENT(6102, DMSegment_TrackIDs,                       "Track IDs")
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
    if (Element_Offset==4)
    {
        #if MEDIAINFO_DEMUX || MEDIAINFO_SEEK
            //Testing if already parsed
            for (size_t Pos=0; Pos<IndexTables.size(); Pos++)
                if (File_Offset+Buffer_Offset-Header_Size==IndexTables[Pos].StreamOffset)
                {
                    Element_Offset=Element_Size;
                    return;
                }

            IndexTables.push_back(indextable());
            IndexTables[IndexTables.size()-1].StreamOffset=File_Offset+Buffer_Offset-Header_Size;
        #endif //MEDIAINFO_DEMUX || MEDIAINFO_SEEK
    }

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
        ELEMENT(8002, IndexTableSegment_8002,                   "8002?")
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
        ELEMENT(3207, GenericPictureEssenceDescriptor_SampledYOffset, "Offset from sampled to stored height")
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

    if (Descriptors[InstanceUID].StreamKind==Stream_Max)
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

    if (Descriptors[InstanceUID].StreamKind==Stream_Max)
    {
        Descriptors[InstanceUID].StreamKind=Stream_Audio;
        if (Streams_Count==(size_t)-1)
            Streams_Count=0;
        Streams_Count++;
    }
}

//---------------------------------------------------------------------------
void File_Mxf::GenericDataEssenceDescriptor()
{
    //Parsing
    switch(Code2)
    {
        ELEMENT(3E01, GenericDataEssenceDescriptor_DataEssenceCoding, "DataEssenceCoding")
        default: FileDescriptor();
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
            Element_Info1("Primary package");
            Element_Level++;
        }
        for (contentstorages::iterator ContentStorage=ContentStorages.begin(); ContentStorage!=ContentStorages.end(); ++ContentStorage)
        {
            for (size_t Pos=0; Pos<ContentStorage->second.Packages.size(); Pos++)
                if (InstanceUID==ContentStorage->second.Packages[Pos])
                {
                    Element_Level--;
                    Element_Info1("Valid from Content storage");
                    Element_Level++;
                }
        }
    }
}

//---------------------------------------------------------------------------
void File_Mxf::MPEG2VideoDescriptor()
{
    Descriptors[InstanceUID].HasMPEG2VideoDescriptor=true;

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
        for (descriptors::iterator Descriptor=Descriptors.begin(); Descriptor!=Descriptors.end(); ++Descriptor)
        {
            for (size_t Pos=0; Pos<Descriptor->second.Locators.size(); Pos++)
                if (InstanceUID==Descriptor->second.Locators[Pos])
                {
                    Element_Level--;
                    Element_Info1("Valid from Descriptor");
                    Element_Level++;
                }
        }
    }
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
        Element_Begin1("LocalTagEntryBatch");
        int16u LocalTag;
        int128u UID;
        Get_B2 (LocalTag,                                       "LocalTag"); Element_Info1(Ztring().From_CC2(LocalTag));
        Get_UL (UID,                                            "UID", NULL); Element_Info1(Ztring().From_UUID(UID));
        Element_End0();

        FILLING_BEGIN();
            if (LocalTag>=0x8000) //user defined
                Primer_Values[LocalTag]=UID;
        FILLING_END();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::RGBAEssenceDescriptor()
{
    if (Code2>0x8000)
    {
        std::map<int16u, int128u>::iterator Primer_Value=Primer_Values.find(Code2);
        if (Primer_Value==Primer_Values.end()) //if not a standard code or unknown user defined code
        {
            GenericPictureEssenceDescriptor();
            return;
        }

        int32u Code_Compare1=Primer_Value->second.hi>>32;
        int32u Code_Compare2=(int32u)Primer_Value->second.hi;
        int32u Code_Compare3=Primer_Value->second.lo>>32;
        int32u Code_Compare4=(int32u)Primer_Value->second.lo;
        if(0);
        ELEMENT_UUID(SubDescriptors,                                "Sub Descriptors")

        return;
    }

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

    if (Descriptors[InstanceUID].Infos["ColorSpace"].empty())
        Descriptors[InstanceUID].Infos["ColorSpace"]="RGB";
}

//---------------------------------------------------------------------------
void File_Mxf::RandomIndexMetadata()
{
    //Parsing
    while (Element_Offset+4<Element_Size)
    {
        Element_Begin1("PartitionArray");
        randomindexmetadata RandomIndexMetadata;
        Get_B4 (RandomIndexMetadata.BodySID,                    "BodySID"); Element_Info1(RandomIndexMetadata.BodySID);
        Get_B8 (RandomIndexMetadata.ByteOffset,                 "ByteOffset"); Element_Info1(Ztring::ToZtring(RandomIndexMetadata.ByteOffset, 16));
        Element_End0();

        FILLING_BEGIN();
            if (!RandomIndexMetadatas_AlreadyParsed && PartitionPack_AlreadyParsed.find(RandomIndexMetadata.ByteOffset)==PartitionPack_AlreadyParsed.end())
                RandomIndexMetadatas.push_back(RandomIndexMetadata);
        FILLING_END();
    }
    Skip_B4(                                                    "Length");

    FILLING_BEGIN();
        if (MediaInfoLib::Config.ParseSpeed_Get()<1.0 && !RandomIndexMetadatas_AlreadyParsed && !RandomIndexMetadatas.empty())
        {
            IsParsingEnd=true;
            IsCheckingRandomAccessTable=true;
            GoTo(RandomIndexMetadatas[0].ByteOffset);
            RandomIndexMetadatas.erase(RandomIndexMetadatas.begin());
            Open_Buffer_Unsynch();
        }
        RandomIndexMetadatas_AlreadyParsed=true;
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
        for (std::map<int128u, track>::iterator Track=Tracks.begin(); Track!=Tracks.end(); ++Track)
        {
            if (InstanceUID==Track->second.Sequence)
            {
                Element_Level--;
                Element_Info1("Valid from track");
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
//SMPTE 380M
void File_Mxf::DMScheme1()
{
    std::map<int16u, int128u>::iterator Primer_Value=Primer_Values.find(Code2);
    if (Primer_Value==Primer_Values.end()) //if not a standard code or unknown user defined code
    {
        InterchangeObject();
        return;
    }

    int32u Code_Compare1=Primer_Value->second.hi>>32;
    int32u Code_Compare2=(int32u)Primer_Value->second.hi;
    int32u Code_Compare3=Primer_Value->second.lo>>32;
    int32u Code_Compare4=(int32u)Primer_Value->second.lo;
    if(0);
    ELEMENT_UUID(DMScheme1_PrimaryExtendedSpokenLanguage,       "Primary Extended Spoken Language")
    ELEMENT_UUID(DMScheme1_SecondaryExtendedSpokenLanguage,     "Secondary Extended Spoken Language")
    ELEMENT_UUID(DMScheme1_OriginalExtendedSpokenLanguage,      "Original Extended Spoken Language")
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
void File_Mxf::StereoscopicPictureSubDescriptor()
{
    StereoscopicPictureSubDescriptor_IsPresent=true;

    //switch(Code2)
    //{
    //    default:
                    GenerationInterchangeObject();
    //}
}

//---------------------------------------------------------------------------
void File_Mxf::TimecodeComponent()
{
    if (Element_Offset==4)
    {
        TimeCode_StartTimecode=(int64u)-1;
        TimeCode_RoundedTimecodeBase=0;
        TimeCode_DropFrame=false;
        DTS_Delay=0;
        FrameInfo.DTS=0;
    }

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
void File_Mxf::VbiPacketsDescriptor()
{
    //switch(Code2)
    //{
    //    default:
                GenericDataEssenceDescriptor();
    //}

    if (Descriptors[InstanceUID].Type==descriptor::Type_Unknown)
    {
        Descriptors[InstanceUID].Type=descriptor::Type_AncPackets;
        if (Streams_Count==(size_t)-1)
            Streams_Count=0;
        Streams_Count++;
    }
}

//---------------------------------------------------------------------------
void File_Mxf::AncPacketsDescriptor()
{
    //switch(Code2)
    //{
    //    default:
                GenericDataEssenceDescriptor();
    //}

    if (Descriptors[InstanceUID].Type==descriptor::Type_Unknown)
    {
        Descriptors[InstanceUID].Type=descriptor::Type_AncPackets;
        if (Streams_Count==(size_t)-1)
            Streams_Count=0;
        Streams_Count++;
    }
}

//---------------------------------------------------------------------------
void File_Mxf::Filler()
{
    Skip_XX(Element_Size,                                       "Junk");

    Buffer_PaddingBytes+=Element_Size;
}

//---------------------------------------------------------------------------
void File_Mxf::TerminatingFiller()
{
    Skip_XX(Element_Size,                                       "Junk");

    Buffer_PaddingBytes+=Element_Size;
}

//---------------------------------------------------------------------------
void File_Mxf::XmlDocumentText()
{
    Skip_XX(Element_Size,                                       "XML data");
}

//---------------------------------------------------------------------------
void File_Mxf::SubDescriptors()
{
    //Parsing
    //Vector
    int32u Count, Length;
    Get_B4 (Count,                                              "Count");
    Get_B4 (Length,                                             "Length");
    for (int32u Pos=0; Pos<Count; Pos++)
    {
        int128u Data;
        Get_UUID(Data,                                          "Sub Descriptor");
    }
}

//---------------------------------------------------------------------------
void File_Mxf::SDTI_SystemMetadataPack() //SMPTE 385M + 326M
{
    //Info for SDTI in Index StreamOffset
    if (!SDTI_IsPresent)
    {
        if (!Partitions.empty() && File_Offset+Buffer_Offset<Partitions[Partitions_Pos].StreamOffset+Partitions[Partitions_Pos].BodyOffset)
            SDTI_IsInIndexStreamOffset=false;
        SDTI_IsPresent=true;
    }

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
    Element_Begin1("Content Package Rate");
    Skip_S1(2,                                                  "Reserved");
    Get_S1 (5, CPR_Rate,                                        "Package Rate"); //See SMPTE 326M
    Get_SB (   CPR_DropFrame,                                   "1.001 Flag");
    Element_End0();
    Element_Begin1("Content Package Type");
    Skip_S1(3,                                                  "Stream Status");
    Skip_SB(                                                    "Sub-package flag");
    Skip_SB(                                                    "Transfer Mode");
    Skip_S1(3,                                                  "Timing Mode");
    Element_End0();
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
        Element_Begin1("TimeCode");
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

        int64u TimeCode_ms=(int64u)(Hours_Tens     *10*60*60*1000
                               + Hours_Units       *60*60*1000
                               + Minutes_Tens      *10*60*1000
                               + Minutes_Units        *60*1000
                               + Seconds_Tens         *10*1000
                               + Seconds_Units           *1000
                               + (FrameRate?float64_int32s((Frames_Tens*10+Frames_Units)*1000/FrameRate):0));

        Element_Info1(Ztring().Duration_From_Milliseconds(TimeCode_ms));

        Element_End0();

        Skip_B8(                                            "Zero");

        //TimeCode
        if (SDTI_TimeCode_StartTimecode_ms==(int64u)-1)
        {
            SDTI_TimeCode_StartTimecode_ms=TimeCode_ms;

            SDTI_TimeCode_StartTimecode+=('0'+Hours_Tens);
            SDTI_TimeCode_StartTimecode+=('0'+Hours_Units);
            SDTI_TimeCode_StartTimecode+=':';
            SDTI_TimeCode_StartTimecode+=('0'+Minutes_Tens);
            SDTI_TimeCode_StartTimecode+=('0'+Minutes_Units);
            SDTI_TimeCode_StartTimecode+=':';
            SDTI_TimeCode_StartTimecode+=('0'+Seconds_Tens);
            SDTI_TimeCode_StartTimecode+=('0'+Seconds_Units);
            SDTI_TimeCode_StartTimecode+=DropFrame?';':':';
            SDTI_TimeCode_StartTimecode+=('0'+Frames_Tens);
            SDTI_TimeCode_StartTimecode+=('0'+Frames_Units);
        }
    }
    else
        Skip_XX(17,                                             "Junk");

    //Filling
    if (SDTI_SizePerFrame==0)
        Partitions_IsCalculatingSdtiByteCount=true;
}

//---------------------------------------------------------------------------
void File_Mxf::SDTI_PackageMetadataSet()
{
    while (Element_Offset<Element_Size)
    {
        //Parsing
        Element_Begin1("Item");
        int128u Tag;
        int16u Length;
        int8u Type;
        Get_B1 (Type,                                            "Type");
        Get_B2 (Length,                                         "Length");
        int64u End=Element_Offset+Length;
        Get_UL (Tag,                                            "Tag", NULL); //TODO: check 10-byte UL with out_imx.mxf
        switch (Type)
        {
            case 0x83 : //UMID
                        {
                            Skip_UMID(                          );
                            if (Element_Offset<End)
                                Skip_UL  (                      "Zeroes");
                        }
                        break;
            case 0x88 : //KLV Metadata
                        {
                            while (Element_Offset<End)
                            {
                                int64u Length;
                                Get_BER(Length,                 "Length");
                                switch ((Tag.lo>>16)&0xFF)
                                {
                                    case 0x00 : Skip_Local(Length,"Data"); break;
                                    case 0x01 : Skip_UTF16L(Length,"Data"); break;
                                    default   : Skip_XX(Length, "Data");
                                }
                            }
                        }
                        break;
            default   : Skip_XX(Length,                         "Unknown");
        }
        Element_End0();
    }

    //Filling
    if (SDTI_SizePerFrame==0)
        Partitions_IsCalculatingSdtiByteCount=true;
}

//---------------------------------------------------------------------------
void File_Mxf::SDTI_PictureMetadataSet()
{
    Skip_XX(Element_Size,                                       "Data");

    //Filling
    if (SDTI_SizePerFrame==0)
        Partitions_IsCalculatingSdtiByteCount=true;
}

//---------------------------------------------------------------------------
void File_Mxf::SDTI_SoundMetadataSet()
{
    Skip_XX(Element_Size,                                       "Data");

    //Filling
    if (SDTI_SizePerFrame==0)
        Partitions_IsCalculatingSdtiByteCount=true;
}

//---------------------------------------------------------------------------
void File_Mxf::SDTI_DataMetadataSet()
{
    Skip_XX(Element_Size,                                       "Data");

    //Filling
    if (SDTI_SizePerFrame==0)
        Partitions_IsCalculatingSdtiByteCount=true;
}

//---------------------------------------------------------------------------
void File_Mxf::SDTI_ControlMetadataSet()
{
    Skip_XX(Element_Size,                                       "Data");

    //Filling
    if (SDTI_SizePerFrame==0)
        Partitions_IsCalculatingSdtiByteCount=true;
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
        for (packages::iterator Package=Packages.begin(); Package!=Packages.end(); ++Package)
        {
            if (Package->first==Prefaces[Preface_Current].PrimaryPackage) //InstanceIUD
            {
                Element_Level--;
                Element_Info1("Primary package");
                Element_Level++;
            }
            for (size_t Pos=0; Pos<Package->second.Tracks.size(); Pos++)
                if (InstanceUID==Package->second.Tracks[Pos])
                {
                    Element_Level--;
                    Element_Info1("Valid from Package");
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
    Info_B1(Data,                                               "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3D0D
void File_Mxf::AES3PCMDescriptor_Emphasis()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3D0F
void File_Mxf::AES3PCMDescriptor_BlockStartOffset()
{
    //Parsing
    Info_B2(Data,                                               "Data"); Element_Info1(Data);
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
    Get_B4 (Data,                                                "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        if (!Partitions_IsFooter || Descriptors[InstanceUID].Infos["BitDepth"].empty())
        {
            if (Data)
                Descriptors[InstanceUID].Infos["BitDepth"].From_Number(Data);
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3302
void File_Mxf::CDCIEssenceDescriptor_HorizontalSubsampling()
{
    //Parsing
    int32u Data;
    Get_B4 (Data,                                                "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        Descriptors[InstanceUID].SubSampling_Horizontal=Data;
        Subsampling_Compute(Descriptors.find(InstanceUID));
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3303
void File_Mxf::CDCIEssenceDescriptor_ColorSiting()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3304
void File_Mxf::CDCIEssenceDescriptor_BlackRefLevel()
{
    //Parsing
    Info_B4(Data,                                               "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3305
void File_Mxf::CDCIEssenceDescriptor_WhiteReflevel()
{
    //Parsing
    Info_B4(Data,                                               "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3306
void File_Mxf::CDCIEssenceDescriptor_ColorRange()
{
    //Parsing
    Info_B4(Data,                                               "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3307
void File_Mxf::CDCIEssenceDescriptor_PaddingBits()
{
    //Parsing
    Info_B2(Data,                                               "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3308
void File_Mxf::CDCIEssenceDescriptor_VerticalSubsampling()
{
    //Parsing
    int32u Data;
    Get_B4 (Data,                                                "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        Descriptors[InstanceUID].SubSampling_Vertical=Data;
        Subsampling_Compute(Descriptors.find(InstanceUID));
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3309
void File_Mxf::CDCIEssenceDescriptor_AlphaSampleDepth()
{
    //Parsing
    Info_B4(Data,                                               "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x330B
void File_Mxf::CDCIEssenceDescriptor_ReversedByteOrder()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info1(Data);
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
            Element_Info1C((Data==Prefaces[Preface_Current].PrimaryPackage), "Primary package");
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
    int128u Data;
    Get_UUID(Data,                                             "DM Framework"); Element_Info1(Ztring().From_UUID(Data));

    FILLING_BEGIN();
        DMSegments[InstanceUID].Framework=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x6101
void File_Mxf::DMSegment_TrackIDs()
{
    //Parsing
    //Vector
    int32u Count, Length;
    Get_B4 (Count,                                              "Count");
    Get_B4 (Length,                                             "Length");
    for (int32u Pos=0; Pos<Count; Pos++)
    {
        int32u Data;
        Get_B4 (Data,                                           "Track ID");

        FILLING_BEGIN();
            DMSegments[InstanceUID].TrackIDs.push_back(Data);
        FILLING_END();
    }
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
    Info_B4(Data,                                               "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3F07
void File_Mxf::EssenceContainerData_BodySID()
{
    //Parsing
    Info_B4(Data,                                               "Data"); Element_Info1(Data);
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
    Info_B8(Data,                                               "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3001
void File_Mxf::FileDescriptor_SampleRate()
{
    //Parsing
    Get_Rational(Descriptors[InstanceUID].SampleRate); Element_Info1(Descriptors[InstanceUID].SampleRate);

    FILLING_BEGIN();
        if (Descriptors[InstanceUID].SampleRate && Descriptors[InstanceUID].Duration!=(int64u)-1)
            Descriptors[InstanceUID].Infos["Duration"].From_Number(Descriptors[InstanceUID].Duration/Descriptors[InstanceUID].SampleRate*1000, 0);
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3002
void File_Mxf::FileDescriptor_ContainerDuration()
{
    //Parsing
    int64u Data;
    Get_B8 (Data,                                               "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        if (Data)
        {
            Descriptors[InstanceUID].Duration=Data;
            if (Descriptors[InstanceUID].SampleRate && Descriptors[InstanceUID].Duration!=(int64u)-1)
                Descriptors[InstanceUID].Infos["Duration"].From_Number(Descriptors[InstanceUID].Duration/Descriptors[InstanceUID].SampleRate*1000, 0);
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3004
void File_Mxf::FileDescriptor_EssenceContainer()
{
    //Parsing
    int128u EssenceContainer;
    Get_UL (EssenceContainer,                                   "EssenceContainer", Mxf_EssenceContainer); Element_Info1(Mxf_EssenceContainer(EssenceContainer));

    FILLING_BEGIN();
        int8u Code6=(int8u)((EssenceContainer.lo&0x0000000000FF0000LL)>>16);
        int8u Code7=(int8u)((EssenceContainer.lo&0x000000000000FF00LL)>> 8);
        int8u Code8=(int8u)((EssenceContainer.lo&0x00000000000000FFLL)    );

        Descriptors[InstanceUID].EssenceContainer=EssenceContainer;
        Descriptors[InstanceUID].Infos["Format_Settings_Wrapping"].From_UTF8(Mxf_EssenceContainer_Mapping(Code6, Code7, Code8));

        if (!DataMustAlwaysBeComplete && Descriptors[InstanceUID].Infos["Format_Settings_Wrapping"].find(__T("Frame"))!=string::npos)
            DataMustAlwaysBeComplete=true;
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
    Get_B4 (Data,                                               "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        if (Descriptors[InstanceUID].LinkedTrackID==(int32u)-1)
            Descriptors[InstanceUID].LinkedTrackID=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3C0A
void File_Mxf::InterchangeObject_InstanceUID()
{
    //Parsing
    Get_UUID(InstanceUID,                                       "UUID"); Element_Info1(Ztring().From_UUID(InstanceUID));

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
            descriptors::iterator Descriptor_Previous=Descriptors.find(InstanceUID);
            if (Descriptor_Previous!=Descriptors.end())
            {
                //Merging
                Descriptor->second.Infos.insert(Descriptor_Previous->second.Infos.begin(), Descriptor_Previous->second.Infos.end()); //TODO: better implementation
            }
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
            Components[InstanceUID].Update(Component->second);
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
    Descriptors[InstanceUID].Locators.clear();

    //Parsing
    //Vector
    int32u Count, Length;
    Get_B4 (Count,                                              "Count");
    Get_B4 (Length,                                             "Length");
    for (int32u Pos=0; Pos<Count; Pos++)
    {
        Element_Begin1("Locator");
        int128u UUID;
        Get_UUID(UUID,                                          "UUID");

        FILLING_BEGIN();
            Descriptors[InstanceUID].Locators.push_back(UUID);
        FILLING_END();

        Element_End0();
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
    Info_UTF16B(Length2, Data,                                  "Data"); Element_Info1(Data);
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
    Get_UL(Data,                                                "Data", Mxf_EssenceCompression); Element_Info1(Mxf_EssenceCompression(Data));

    FILLING_BEGIN();
        Descriptors[InstanceUID].EssenceCompression=Data;
        Descriptors[InstanceUID].StreamKind=Stream_Video;
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
    Get_B4 (Data,                                                "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        if (!Partitions_IsFooter || Descriptors[InstanceUID].Height==(int32u)-1)
        {
            if (Descriptors[InstanceUID].ScanType==__T("Interlaced"))
                Data*=2; //This is per field
            if (Descriptors[InstanceUID].Height==(int32u)-1)
                Descriptors[InstanceUID].Height=Data;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3203
void File_Mxf::GenericPictureEssenceDescriptor_StoredWidth()
{
    //Parsing
    int32u Data;
    Get_B4 (Data,                                                "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        if (!Partitions_IsFooter || Descriptors[InstanceUID].Width==(int32u)-1)
        {
            if (Descriptors[InstanceUID].Width==(int32u)-1)
                Descriptors[InstanceUID].Width=Data;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3204
void File_Mxf::GenericPictureEssenceDescriptor_SampledHeight()
{
    //Parsing
    int32u Data;
    Get_B4 (Data,                                                "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        if (!Partitions_IsFooter || Descriptors[InstanceUID].Height==(int32u)-1)
        {
            if (Descriptors[InstanceUID].ScanType==__T("Interlaced"))
                Data*=2; //This is per field
            Descriptors[InstanceUID].Height=Data;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3205
void File_Mxf::GenericPictureEssenceDescriptor_SampledWidth()
{
    //Parsing
    int32u Data;
    Get_B4 (Data,                                                "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        Descriptors[InstanceUID].Width=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3206
void File_Mxf::GenericPictureEssenceDescriptor_SampledXOffset()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3207
void File_Mxf::GenericPictureEssenceDescriptor_SampledYOffset()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3208
void File_Mxf::GenericPictureEssenceDescriptor_DisplayHeight()
{
    //Parsing
    int32u Data;
    Get_B4 (Data,                                                "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        if (!Partitions_IsFooter || Descriptors[InstanceUID].Height_Display!=(int32u)-1)
        {
            if (Descriptors[InstanceUID].ScanType==__T("Interlaced"))
                Data*=2; //This is per field
            Descriptors[InstanceUID].Height_Display=Data;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3209
void File_Mxf::GenericPictureEssenceDescriptor_DisplayWidth()
{
    //Parsing
    int32u Data;
    Get_B4 (Data,                                                "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        Descriptors[InstanceUID].Width_Display=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x320A
void File_Mxf::GenericPictureEssenceDescriptor_DisplayXOffset()
{
    //Parsing
    int32u Data;
    Get_B4 (Data,                                               "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        Descriptors[InstanceUID].Width_Display_Offset=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x320B
void File_Mxf::GenericPictureEssenceDescriptor_DisplayYOffset()
{
    //Parsing
    int32u Data;
    Get_B4 (Data,                                               "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        if (!Partitions_IsFooter || Descriptors[InstanceUID].Height_Display_Offset==(int32u)-1)
        {
            if (Descriptors[InstanceUID].ScanType==__T("Interlaced"))
                Data*=2; //This is per field
            Descriptors[InstanceUID].Height_Display_Offset=Data;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x320C
void File_Mxf::GenericPictureEssenceDescriptor_FrameLayout()
{
    //Parsing
    int8u Data;
    Get_B1 (Data,                                               "Data"); Element_Info1(Data); Param_Info1(Mxf_FrameLayout(Data)); Element_Info1(Mxf_FrameLayout(Data));

    FILLING_BEGIN();
        if (!Partitions_IsFooter || Descriptors[InstanceUID].ScanType.empty())
        {
            if (Descriptors[InstanceUID].ScanType.empty())
            {
                if (Descriptors[InstanceUID].Height!=(int32u)-1) Descriptors[InstanceUID].Height*=Mxf_FrameLayout_Multiplier(Data);
                if (Descriptors[InstanceUID].Height_Display!=(int32u)-1) Descriptors[InstanceUID].Height_Display*=Mxf_FrameLayout_Multiplier(Data);
                if (Descriptors[InstanceUID].Height_Display_Offset!=(int32u)-1) Descriptors[InstanceUID].Height_Display_Offset*=Mxf_FrameLayout_Multiplier(Data);
            }
            Descriptors[InstanceUID].ScanType.From_UTF8(Mxf_FrameLayout_ScanType(Data));
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x320D
void File_Mxf::GenericPictureEssenceDescriptor_VideoLineMap()
{
    int64u VideoLineMapEntries_Total=0;
    bool   VideoLineMapEntry_IsZero=false;

    //Parsing
    int32u Count, Length;
    Get_B4 (Count,                                              "Count");
    Get_B4 (Length,                                             "Length");
    for (int32u Pos=0; Pos<Count; Pos++)
    {
        int32u VideoLineMapEntry;
        Get_B4 (VideoLineMapEntry,                              "VideoLineMapEntry");

        if (VideoLineMapEntry)
            VideoLineMapEntries_Total+=VideoLineMapEntry;
        else
            VideoLineMapEntry_IsZero=true;
    }

    FILLING_BEGIN();
        // Cryptic formula:
        //    odd odd field 2 upper
        //    odd even field 1 upper
        //    even odd field 1 upper
        //    even even field 2 upper
        if (Count==2 && !VideoLineMapEntry_IsZero)
            Descriptors[InstanceUID].FieldTopness=(VideoLineMapEntries_Total%2)?1:2;
    FILLING_END();
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
        {
            Descriptors[InstanceUID].DisplayAspectRatio=Data;
            Descriptors[InstanceUID].Infos["DisplayAspectRatio"].From_Number(Data, 3);
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x320F
void File_Mxf::GenericPictureEssenceDescriptor_AlphaTransparency()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info1(Data);
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
    Info_B4(Data,                                               "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3212
void File_Mxf::GenericPictureEssenceDescriptor_FieldDominance()
{
    //Parsing
    int8u Data;
    Get_B1 (Data,                                               "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        Descriptors[InstanceUID].FieldDominance=Data;
    FILLING_END();
    //Parsing
}

//---------------------------------------------------------------------------
// 0x3213
void File_Mxf::GenericPictureEssenceDescriptor_ImageStartOffset()
{
    //Parsing
    Info_B4(Data,                                               "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3214
void File_Mxf::GenericPictureEssenceDescriptor_ImageEndOffset()
{
    //Parsing
    Info_B4(Data,                                               "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3215
void File_Mxf::GenericPictureEssenceDescriptor_SignalStandard()
{
    //Parsing
    Info_B1(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3216
void File_Mxf::GenericPictureEssenceDescriptor_StoredF2Offset()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3217
void File_Mxf::GenericPictureEssenceDescriptor_DisplayF2Offset()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3218
void File_Mxf::GenericPictureEssenceDescriptor_ActiveFormatDescriptor()
{
    //Parsing
    int8u Data;
    Get_B1 (Data,                                                "Data"); Element_Info1C((Data<16), AfdBarData_active_format[Data]);

    FILLING_BEGIN();
        Descriptors[InstanceUID].ActiveFormat=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3D01
void File_Mxf::GenericSoundEssenceDescriptor_QuantizationBits()
{
    //Parsing
    int32u Data;
    Get_B4 (Data,                                               "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        if (Data)
        {
            Descriptors[InstanceUID].Infos["BitDepth"].From_Number(Data);
            Descriptors[InstanceUID].QuantizationBits=Data;
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3D02
void File_Mxf::GenericSoundEssenceDescriptor_Locked()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info1(Data?"Yes":"No");
}

//---------------------------------------------------------------------------
// 0x3D03
void File_Mxf::GenericSoundEssenceDescriptor_AudioSamplingRate()
{
    //Parsing
    float64 Data;
    Get_Rational(Data); Element_Info1(Data);

    FILLING_BEGIN();
        Descriptors[InstanceUID].Infos["SamplingRate"].From_Number(Data, 0);
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3D04
void File_Mxf::GenericSoundEssenceDescriptor_AudioRefLevel()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info2(Data, " dB");
}

//---------------------------------------------------------------------------
// 0x3D05
void File_Mxf::GenericSoundEssenceDescriptor_ElectroSpatialFormulation()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info1(Data); //Enum
}

//---------------------------------------------------------------------------
// 0x3D06
void File_Mxf::GenericSoundEssenceDescriptor_SoundEssenceCompression()
{
    //Parsing
    int128u Data;
    Get_UL(Data,                                                "Data", Mxf_EssenceCompression); Element_Info1(Mxf_EssenceCompression(Data));

    FILLING_BEGIN();
        Descriptors[InstanceUID].EssenceCompression=Data;
        Descriptors[InstanceUID].StreamKind=Stream_Audio;
        Descriptors[InstanceUID].Infos["Format"]=Mxf_EssenceCompression(Data);
        Descriptors[InstanceUID].Infos["Format_Version"]=Mxf_EssenceCompression_Version(Data);
        if ((Data.lo&0xFFFFFFFFFF000000LL)==0x040202017e000000LL)
            Descriptors[InstanceUID].Infos["Format_Settings_Endianness"]=__T("Big");
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3D07
void File_Mxf::GenericSoundEssenceDescriptor_ChannelCount()
{
    //Parsing
    int32u Data;
    Get_B4 (Data,                                               "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        Descriptors[InstanceUID].ChannelCount=Data;
        Descriptors[InstanceUID].Infos["Channel(s)"].From_Number(Data);
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3D0C
void File_Mxf::GenericSoundEssenceDescriptor_DialNorm()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info2(Data, " dB");
}

//---------------------------------------------------------------------------
// 0x3E01
void File_Mxf::GenericDataEssenceDescriptor_DataEssenceCoding()
{
    //Parsing
    Skip_UL(                                                    "UUID");
}

//---------------------------------------------------------------------------
// 0x4801
void File_Mxf::GenericTrack_TrackID()
{
    //Parsing
    int32u Data;
    Get_B4 (Data,                                                "Data"); Element_Info1(Data);

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
    Ztring Data;
    Get_UTF16B (Length2, Data,                                  "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        Tracks[InstanceUID].TrackName=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x4803
void File_Mxf::GenericTrack_Sequence()
{
    //Parsing
    int128u Data;
    Get_UUID(Data,                                              "Data"); Element_Info1(Ztring::ToZtring(Data, 16));

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
    Get_B4 (Data,                                                "Data"); Element_Info1(Ztring::ToZtring(Data, 16));

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
    Get_UTF16B(Length2, Data,                                  "Data"); Element_Info1(Data);

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
    Get_UTF16B(Length2, Data,                                  "Data"); Element_Info1(Data);

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
    Ztring Version=Ztring::ToZtring(Major)+__T('.')
                  +Ztring::ToZtring(Minor)+__T('.')
                  +Ztring::ToZtring(Patch)+__T('.')
                  +Ztring::ToZtring(Build)+__T('.')
                  +Ztring::ToZtring(Release)      ;
    Element_Info1(Version);

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
    Get_UTF16B(Length2, Data,                                  "Data"); Element_Info1(Data);

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
    Element_Info1(Ztring::ToZtring(Major)+__T('.')
                +Ztring::ToZtring(Minor)+__T('.')
                +Ztring::ToZtring(Patch)+__T('.')
                +Ztring::ToZtring(Build)+__T('.')
                +Ztring::ToZtring(Release)      );
}

//---------------------------------------------------------------------------
// 0x3C08
void File_Mxf::Identification_Platform()
{
    //Parsing
    Info_UTF16B(Length2, Data,                                  "Data"); Element_Info1(Data);
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
    int32u Data;
    Get_B4(Data,                                                "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        #if MEDIAINFO_SEEK
            IndexTables[IndexTables.size()-1].EditUnitByteCount=Data;
        #endif //MEDIAINFO_SEEK
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3F06
void File_Mxf::IndexTableSegment_IndexSID()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3F07
void File_Mxf::IndexTableSegment_BodySID()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3F08
void File_Mxf::IndexTableSegment_SliceCount()
{
    //Parsing
    int8u Data;
    Get_B1(Data,                                                "Data"); Element_Info1(Data);

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
        Element_Begin1("Delta Entry");
        Skip_B1(                                                "PosTableIndex");
        Skip_B1(                                                "Slice");
        Skip_B4(                                                "Element Delta");
        Element_End0();
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
        #if MEDIAINFO_SEEK
            indextable::entry Entry;
            int64u Stream_Offset;
            bool   forward_rediction_flag, backward_prediction_flag;
        #endif //MEDIAINFO_SEEK
        int8u Flags;
        Element_Begin1("Index Entry");
        Skip_B1(                                                "Temporal Offset");
        Skip_B1(                                                "Key-Frame Offset");
        Get_B1 (Flags,                                          "Flags");
            Skip_Flags(Flags, 7,                                "Random Access");
            Skip_Flags(Flags, 6,                                "Sequence Header");
            #if MEDIAINFO_SEEK
                Get_Flags (Flags, 5, forward_rediction_flag,    "forward prediction flag");
                Get_Flags (Flags, 4, backward_prediction_flag,  "backward prediction flag");
            #else //MEDIAINFO_SEEK
                Skip_Flags(Flags, 5,                            "forward prediction flag");
                Skip_Flags(Flags, 4,                            "backward prediction flag");
            #endif //MEDIAINFO_SEEK
        #if MEDIAINFO_SEEK
            Get_B8 (Stream_Offset,                              "Stream Offset");
            Entry.StreamOffset=Stream_Offset;
            Entry.Type=(forward_rediction_flag?1:0)*2+(backward_prediction_flag?1:0);
            IndexTables[IndexTables.size()-1].Entries.push_back(Entry);
        #else //MEDIAINFO_SEEK
            Skip_B8(                                            "Stream Offset");
        #endif //MEDIAINFO_SEEK
        for (int32u NSL_Pos=0; NSL_Pos<IndexTable_NSL; NSL_Pos++)
            Skip_B4(                                            "SliceOffset");
        for (int32u NPE_Pos=0; NPE_Pos<IndexTable_NPE; NPE_Pos++)
            Skip_B4(                                            "PosTable");
        Element_End0();
    }
}

//---------------------------------------------------------------------------
// 0x3F0B
void File_Mxf::IndexTableSegment_IndexEditRate()
{
    //Parsing
    float64 Data;
    Get_Rational(Data);

    FILLING_BEGIN();
        #if MEDIAINFO_SEEK
            IndexTables[IndexTables.size()-1].IndexEditRate=Data;
        #endif //MEDIAINFO_SEEK
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3F0C
void File_Mxf::IndexTableSegment_IndexStartPosition()
{
    //Parsing
    int64u Data;
    Get_B8 (Data,                                                "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        #if MEDIAINFO_SEEK
            IndexTables[IndexTables.size()-1].IndexStartPosition=Data;

            //Integrity test (in one file, I have 2 indexes with IndexStartPosition=0, the first  one is weird (only one frame), or the same index is repeated
            //Integrity test (in one file, I have 2 indexes with IndexStartPosition=0, the second one is weird (only one frame), or the same index is repeated
            for (size_t Pos=0; Pos<IndexTables.size()-1; Pos++)
                if (IndexTables[Pos].IndexStartPosition==Data)
                {
                    if (IndexTables[Pos].IndexDuration==1 && Pos!=IndexTables.size()-1)
                        IndexTables.erase(IndexTables.begin()+Pos);
                    else
                    {
                        IndexTables.erase(IndexTables.begin()+IndexTables.size()-1);
                        Element_Offset=Element_Size;
                    }

                    return;
                }
        #endif //MEDIAINFO_SEEK
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3F0D
void File_Mxf::IndexTableSegment_IndexDuration()
{
    //Parsing
    int64u Data;
    Get_B8 (Data,                                                "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        #if MEDIAINFO_SEEK
            IndexTables[IndexTables.size()-1].IndexDuration=Data;
        #endif //MEDIAINFO_SEEK
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3F0E
void File_Mxf::IndexTableSegment_PosTableCount()
{
    //Parsing
    int8u Data;
    Get_B1(Data,                                                "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        IndexTable_NPE=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x8002
void File_Mxf::IndexTableSegment_8002()
{
    //Parsing
    Info_B8(Data,                                               "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x8001
void File_Mxf::JPEG2000PictureSubDescriptor_Rsiz()
{
    //Parsing
    Info_B2(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x8002
void File_Mxf::JPEG2000PictureSubDescriptor_Xsiz()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x8003
void File_Mxf::JPEG2000PictureSubDescriptor_Ysiz()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x8004
void File_Mxf::JPEG2000PictureSubDescriptor_XOsiz()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x8005
void File_Mxf::JPEG2000PictureSubDescriptor_YOsiz()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x8006
void File_Mxf::JPEG2000PictureSubDescriptor_XTsiz()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x8007
void File_Mxf::JPEG2000PictureSubDescriptor_YTsiz()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x8008
void File_Mxf::JPEG2000PictureSubDescriptor_XTOsiz()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x8009
void File_Mxf::JPEG2000PictureSubDescriptor_YTOsiz()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x800A
void File_Mxf::JPEG2000PictureSubDescriptor_Csiz()
{
    //Parsing
    Info_B2(Data,                                                "Data"); Element_Info1(Data);
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
        Element_Begin1("PictureComponentSize");
        Info_B1(Ssiz,                                           "Component sample precision"); Element_Info1(Ssiz);
        Info_B1(XRsiz,                                          "Horizontal separation of a sample"); Element_Info1(XRsiz);
        Info_B1(YRsiz,                                          "Vertical separation of a sample"); Element_Info1(YRsiz);
        Element_End0();
    }
}

//---------------------------------------------------------------------------
// 0x3B02
void File_Mxf::Preface_LastModifiedDate()
{
    //Parsing
    Ztring Value;
    Get_Timestamp(Value); Element_Info1(Value);

    FILLING_BEGIN();
        Fill(Stream_General, 0, General_Encoded_Date, Value, true);
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x
void File_Mxf::DMScheme1_PrimaryExtendedSpokenLanguage()
{
    //Parsing
    Ztring Data;
    Get_Local (Length2, Data,                                   "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        DMScheme1s[InstanceUID].PrimaryExtendedSpokenLanguage=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x
void File_Mxf::DMScheme1_SecondaryExtendedSpokenLanguage()
{
    //Parsing
    Info_Local(Length2, Data,                                   "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x
void File_Mxf::DMScheme1_OriginalExtendedSpokenLanguage()
{
    //Parsing
    Info_Local(Length2, Data,                                   "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x
void File_Mxf::MPEG2VideoDescriptor_SingleSequence()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info1(Data?"Yes":"No");
}

//---------------------------------------------------------------------------
// 0x
void File_Mxf::MPEG2VideoDescriptor_ConstantBFrames()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info1(Data?"Yes":"No");
}

//---------------------------------------------------------------------------
// 0x
void File_Mxf::MPEG2VideoDescriptor_CodedContentType()
{
    //Parsing
    int8u Data;
    Get_B1 (Data,                                               "Data"); Element_Info1(Mxf_MPEG2_CodedContentType(Data));

    FILLING_BEGIN();
        if (!Partitions_IsFooter || Descriptors[InstanceUID].ScanType.empty())
        {
            if (Data==2 && Descriptors[InstanceUID].ScanType.empty())
            {
                if (Descriptors[InstanceUID].Height!=(int32u)-1) Descriptors[InstanceUID].Height*=2;
                if (Descriptors[InstanceUID].Height_Display!=(int32u)-1) Descriptors[InstanceUID].Height_Display*=2;
                if (Descriptors[InstanceUID].Height_Display_Offset!=(int32u)-1) Descriptors[InstanceUID].Height_Display_Offset*=2;
            }
            Descriptors[InstanceUID].ScanType.From_UTF8(Mxf_MPEG2_CodedContentType(Data));
        }
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x
void File_Mxf::MPEG2VideoDescriptor_LowDelay()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info1(Data?"Yes":"No");
}

//---------------------------------------------------------------------------
// 0x
void File_Mxf::MPEG2VideoDescriptor_ClosedGOP()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info1(Data?"Yes":"No");
}

//---------------------------------------------------------------------------
// 0x
void File_Mxf::MPEG2VideoDescriptor_IdenticalGOP()
{
    //Parsing
    Info_B1(Data,                                               "Data"); Element_Info1(Data?"Yes":"No");
}

//---------------------------------------------------------------------------
// 0x
void File_Mxf::MPEG2VideoDescriptor_MaxGOP()
{
    //Parsing
    Info_B2(Data,                                               "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x
void File_Mxf::MPEG2VideoDescriptor_BPictureCount()
{
    //Parsing
    int16u Data;
    Get_B2 (Data,                                               "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        Descriptors[InstanceUID].HasBFrames=Data?true:false;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x
void File_Mxf::MPEG2VideoDescriptor_ProfileAndLevel()
{
    //Parsing
    int8u profile_and_level_indication_profile, profile_and_level_indication_level;
    BS_Begin();
    Skip_SB(                                                    "profile_and_level_indication_escape");
    Get_S1 ( 3, profile_and_level_indication_profile,           "profile_and_level_indication_profile"); Param_Info1(Mpegv_profile_and_level_indication_profile[profile_and_level_indication_profile]);
    Get_S1 ( 4, profile_and_level_indication_level,             "profile_and_level_indication_level"); Param_Info1(Mpegv_profile_and_level_indication_level[profile_and_level_indication_level]);
    BS_End();

    FILLING_BEGIN();
        if (profile_and_level_indication_profile && profile_and_level_indication_level)
            Descriptors[InstanceUID].Infos["Format_Profile"]=Ztring().From_Local(Mpegv_profile_and_level_indication_profile[profile_and_level_indication_profile])+__T("@")+Ztring().From_Local(Mpegv_profile_and_level_indication_level[profile_and_level_indication_level]);
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x
void File_Mxf::MPEG2VideoDescriptor_BitRate()
{
    //Parsing
    int32u Data;
    Get_B4 (Data,                                               "Data"); Element_Info1(Data);

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
    Get_UTF16B(Length2, Data,                                   "Essence Locator"); Element_Info1(Data);

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
            Descriptors[Data].Infos["StreamOrder"].From_Number(Pos);
        FILLING_END();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::PartitionMetadata()
{
    //Parsing
    int64u PreviousPartition, FooterPartition, HeaderByteCount, IndexByteCount, BodyOffset;
    int32u IndexSID;
    int32u KAGSize;
    Skip_B2(                                                    "MajorVersion");
    Skip_B2(                                                    "MinorVersion");
    Get_B4 (KAGSize,                                            "KAGSize");
    Skip_B8(                                                    "ThisPartition");
    Get_B8 (PreviousPartition,                                  "PreviousPartition");
    Get_B8 (FooterPartition,                                    "FooterPartition");
    Get_B8 (HeaderByteCount,                                    "HeaderByteCount");
    Get_B8 (IndexByteCount,                                     "IndexByteCount");
    Get_B4 (IndexSID,                                           "IndexSID");
    Get_B8 (BodyOffset,                                         "BodyOffset");
    Skip_B4(                                                    "BodySID");
    Get_UL (OperationalPattern,                                 "OperationalPattern", Mxf_OperationalPattern);

    Element_Begin1("EssenceContainers"); //Vector
        int32u Count, Length;
        Get_B4 (Count,                                          "Count");
        Get_B4 (Length,                                         "Length");
        for (int32u Pos=0; Pos<Count; Pos++)
        {
            int128u EssenceContainer;
            Get_UL (EssenceContainer,                           "EssenceContainer", Mxf_EssenceContainer);
            if (Count==1)
                EssenceContainer_FromPartitionMetadata=EssenceContainer;
        }
    Element_End0();

    PartitionPack_Parsed=true;
    Partitions_IsFooter=(Code.lo&0x00FF0000)==0x00040000;
    if (PreviousPartition!=File_Offset+Buffer_Offset-Header_Size)
        PartitionMetadata_PreviousPartition=PreviousPartition;
    if (FooterPartition)
        PartitionMetadata_FooterPartition=FooterPartition;
    bool AlreadyParsed=false;
    for (size_t Pos=0; Pos<Partitions.size(); Pos++)
        if (Partitions[Pos].StreamOffset==File_Offset+Buffer_Offset-Header_Size)
            AlreadyParsed=true;
    if (!AlreadyParsed)
    {
        partition Partition;
        Partition.StreamOffset=File_Offset+Buffer_Offset-Header_Size;
        Partition.FooterPartition=FooterPartition;
        Partition.HeaderByteCount=HeaderByteCount;
        Partition.IndexByteCount=IndexByteCount;
        Partition.BodyOffset=BodyOffset;
        Partitions_Pos=0;
        while (Partitions_Pos<Partitions.size() && Partitions[Partitions_Pos].StreamOffset<Partition.StreamOffset)
            Partitions_Pos++;
        Partitions.insert(Partitions.begin()+Partitions_Pos, Partition);
        Partitions_IsCalculatingHeaderByteCount=true;
    }

    if ((Code.lo&0xFF0000)==0x020000) //If Header Partition Pack
        switch ((Code.lo>>8)&0xFF)
        {
            case 0x01 : Fill(Stream_General, 0, General_Format_Settings, "Open / Incomplete"  , Unlimited, true, true);
                        if (Config->ParseSpeed>=1.0)
                        {
                            Config->File_IsGrowing=true;
                            #if MEDIAINFO_MD5
                                delete MD5; MD5=NULL;
                            #endif //MEDIAINFO_MD5
                        }
                        break;
            case 0x02 : Fill(Stream_General, 0, General_Format_Settings, "Closed / Incomplete", Unlimited, true, true);
                        break;
            case 0x03 : Fill(Stream_General, 0, General_Format_Settings, "Open / Complete"    , Unlimited, true, true);
                        if (Config->ParseSpeed>=1.0)
                        {
                            Config->File_IsGrowing=true;
                            #if MEDIAINFO_MD5
                                delete MD5; MD5=NULL;
                            #endif //MEDIAINFO_MD5
                        }
                        break;
            case 0x04 : Fill(Stream_General, 0, General_Format_Settings, "Closed / Complete"  , Unlimited, true, true);
                        break;
            default   : ;
        }

    if ((Code.lo&0xFF0000)==0x040000) //If Footer Partition Pack
    {
        switch ((Code.lo>>8)&0xFF)
        {
            case 0x02 :
            case 0x04 :
                        Config->File_IsGrowing=false;
                        break;
            default   : ;
        }

        #if MEDIAINFO_ADVANCED
            if (Footer_Position==(int64u)-1)
                Footer_Position=File_Offset+Buffer_Offset-Header_Size;
        #endif //MEDIAINFO_ADVANCED
    }

    PartitionPack_AlreadyParsed.insert(File_Offset+Buffer_Offset-Header_Size);
}

//---------------------------------------------------------------------------
// 0x3B03
void File_Mxf::Preface_ContentStorage()
{
    //Parsing
    int128u Data;
    Get_UUID(Data,                                              "Data"); Element_Info1(Ztring().From_UUID(Data));

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
    Element_Info1(Ztring::ToZtring(Major)+__T('.')+Ztring::ToZtring(Minor));
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
        Element_Begin1("Identification");
        int128u Data;
        Get_UUID(Data,                                          "UUID"); Element_Info1(Ztring().From_UUID(Data));
        Element_End0();

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
    Get_UL (OperationalPattern,                                 "UUID", Mxf_OperationalPattern); Element_Info1(Mxf_OperationalPattern(OperationalPattern));
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
            Info_UL(Data,                                       "DMScheme", NULL); Element_Info1(Ztring().From_UUID(Data));
        }
        else
            Skip_XX(Length,                                     "DMScheme");
}

//---------------------------------------------------------------------------
// 0x3401
void File_Mxf::RGBAEssenceDescriptor_PixelLayout()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3403
void File_Mxf::RGBAEssenceDescriptor_Palette()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3404
void File_Mxf::RGBAEssenceDescriptor_PaletteLayout()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3405
void File_Mxf::RGBAEssenceDescriptor_ScanningDirection()
{
    //Parsing
    Info_B1(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3406
void File_Mxf::RGBAEssenceDescriptor_ComponentMaxRef()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3407
void File_Mxf::RGBAEssenceDescriptor_ComponentMinRef()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3408
void File_Mxf::RGBAEssenceDescriptor_AlphaMaxRef()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3409
void File_Mxf::RGBAEssenceDescriptor_AlphaMinRef()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
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
    int32u Data;
    Get_B4 (Data,                                                "SourceTrackID"); Element_Info1(Data);

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
    Info_B8(Data,                                               "StartPosition"); Element_Info1(Data); //units of edit rate
}

//---------------------------------------------------------------------------
// 0x4701
void File_Mxf::SourcePackage_Descriptor()
{
    //Parsing
    int128u Data;
    Get_UUID(Data,                                              "Data"); Element_Info1(Ztring().From_UUID(Data));

    FILLING_BEGIN();
        Packages[InstanceUID].Descriptor=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x0201
void File_Mxf::StructuralComponent_DataDefinition()
{
    //Parsing
    Info_UL(Data,                                               "Data", Mxf_Sequence_DataDefinition); Element_Info1(Mxf_Sequence_DataDefinition(Data));
}

//---------------------------------------------------------------------------
// 0x0202
void File_Mxf::StructuralComponent_Duration()
{
    //Parsing
    int64u Data;
    Get_B8 (Data,                                               "Data"); Element_Info1(Data); //units of edit rate

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
        Element_Begin1("TimeCode");
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
                               + (SystemScheme1_FrameRateFromDescriptor?float64_int32s((Frames_Tens*10+Frames_Units)*1000/(float64)SystemScheme1_FrameRateFromDescriptor):0));

        Element_Info1(Ztring().Duration_From_Milliseconds(TimeCode));

        Element_End0();

        //TimeCode
        if (SystemScheme1_TimeCodeArray_StartTimecode_ms==(int64u)-1 && !IsParsingEnd && IsParsingMiddle_MaxOffset==(int64u)-1)
        {
            SystemScheme1_TimeCodeArray_StartTimecode_ms=TimeCode;

            SystemScheme1_TimeCodeArray_StartTimecode+=('0'+Hours_Tens);
            SystemScheme1_TimeCodeArray_StartTimecode+=('0'+Hours_Units);
            SystemScheme1_TimeCodeArray_StartTimecode+=':';
            SystemScheme1_TimeCodeArray_StartTimecode+=('0'+Minutes_Tens);
            SystemScheme1_TimeCodeArray_StartTimecode+=('0'+Minutes_Units);
            SystemScheme1_TimeCodeArray_StartTimecode+=':';
            SystemScheme1_TimeCodeArray_StartTimecode+=('0'+Seconds_Tens);
            SystemScheme1_TimeCodeArray_StartTimecode+=('0'+Seconds_Units);
            SystemScheme1_TimeCodeArray_StartTimecode+=DropFrame?';':':';
            SystemScheme1_TimeCodeArray_StartTimecode+=('0'+Frames_Tens);
            SystemScheme1_TimeCodeArray_StartTimecode+=('0'+Frames_Units);
        }
    }
}

//---------------------------------------------------------------------------
// 0x4101
void File_Mxf::TextLocator_LocatorName()
{
    //Parsing
    Ztring Data;
    Get_UTF16B (Length2, Data,                                  "Data"); Element_Info1(Data);

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
    Get_B8 (Data,                                                "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        if (Data!=(int64u)-1)
        {
            TimeCode_StartTimecode=Data;
            if (TimeCode_RoundedTimecodeBase)
            {
                DTS_Delay=((float64)TimeCode_StartTimecode)/TimeCode_RoundedTimecodeBase;
                if (TimeCode_DropFrame)
                {
                    DTS_Delay*=1001;
                    DTS_Delay/=1000;
                }
                FrameInfo.DTS=float64_int64s(DTS_Delay*1000000000);
                #if MEDIAINFO_DEMUX
                    Config->Demux_Offset_DTS_FromStream=FrameInfo.DTS;
                #endif //MEDIAINFO_DEMUX
            }
        }

        Components[InstanceUID].TimeCode_StartTimecode=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x1502
void File_Mxf::TimecodeComponent_RoundedTimecodeBase()
{
    //Parsing
    int16u Data;
    Get_B2 (Data,                                                "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        if (Data && Data!=(int16u)-1)
        {
            TimeCode_RoundedTimecodeBase=Data;
            if (TimeCode_StartTimecode!=(int64u)-1)
            {
                DTS_Delay=((float64)TimeCode_StartTimecode)/TimeCode_RoundedTimecodeBase;
                if (TimeCode_DropFrame)
                {
                    DTS_Delay*=1001;
                    DTS_Delay/=1000;
                }
                FrameInfo.DTS=float64_int64s(DTS_Delay*1000000000);
                #if MEDIAINFO_DEMUX
                    Config->Demux_Offset_DTS_FromStream=FrameInfo.DTS;
                #endif //MEDIAINFO_DEMUX
            }
        }

        Components[InstanceUID].TimeCode_RoundedTimecodeBase=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x1503
void File_Mxf::TimecodeComponent_DropFrame()
{
    //Parsing
    int8u Data;
    Get_B1 (Data,                                                "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        if (Data!=(int8u)-1 && Data)
        {
            TimeCode_DropFrame=true;
            if (DTS_Delay)
            {
                DTS_Delay*=1001;
                DTS_Delay/=1000;
            }
            FrameInfo.DTS=float64_int64s(DTS_Delay*1000000000);
            #if MEDIAINFO_DEMUX
                Config->Demux_Offset_DTS_FromStream=FrameInfo.DTS;
            #endif //MEDIAINFO_DEMUX
        }

        Components[InstanceUID].TimeCode_DropFrame=Data?true:false;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x4B01
void File_Mxf::Track_EditRate()
{
    //Parsing
    float64 Data;
    Get_Rational(Data); Element_Info1(Data);

    FILLING_BEGIN();
        Tracks[InstanceUID].EditRate=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x4B02
void File_Mxf::Track_Origin()
{
    //Parsing
    int64u Data;
    Get_B8 (Data,                                                "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        if (Data!=(int64u)-1)
            Tracks[InstanceUID].Origin=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3D09
void File_Mxf::WaveAudioDescriptor_AvgBps()
{
    //Parsing
    int32u Data;
    Get_B4 (Data,                                               "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        Descriptors[InstanceUID].Infos["BitRate"].From_Number(Data*8);
        Descriptors[InstanceUID].ByteRate=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3D0A
void File_Mxf::WaveAudioDescriptor_BlockAlign()
{
    //Parsing
    int16u Data;
    Get_B2 (Data,                                               "Data"); Element_Info1(Data);

    FILLING_BEGIN();
        Descriptors[InstanceUID].BlockAlign=Data;
    FILLING_END();
}

//---------------------------------------------------------------------------
// 0x3D0B
void File_Mxf::WaveAudioDescriptor_SequenceOffset()
{
    //Parsing
    Info_B1(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3D29
void File_Mxf::WaveAudioDescriptor_PeakEnvelopeVersion()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3D2A
void File_Mxf::WaveAudioDescriptor_PeakEnvelopeFormat()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3D2B
void File_Mxf::WaveAudioDescriptor_PointsPerPeakValue()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3D2C
void File_Mxf::WaveAudioDescriptor_PeakEnvelopeBlockSize()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3D2D
void File_Mxf::WaveAudioDescriptor_PeakChannels()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3D2E
void File_Mxf::WaveAudioDescriptor_PeakFrames()
{
    //Parsing
    Info_B4(Data,                                                "Data"); Element_Info1(Data);
}

//---------------------------------------------------------------------------
// 0x3D2F
void File_Mxf::WaveAudioDescriptor_PeakOfPeaksPosition()
{
    //Parsing
    Info_B8(Data,                                                "Data"); Element_Info1(Data);
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
    Element_Info1C(D, ((float32)N)/D);
}

//---------------------------------------------------------------------------
void File_Mxf::Get_UL(int128u &Value, const char* Name, const char* (*Param) (int128u))
{
    #ifdef MEDIAINFO_MINIMIZE_SIZE
        Skip_UUID();
    #else
    //Parsing
    Element_Begin1(Name);
    int64u Value_hi, Value_lo;
    int8u Category, Registry, Structure;
    Peek_B8(Value_hi);
    Skip_B1(                                                    "Start (0x06)");
    Skip_B1(                                                    "Length of the remaining key (0x0E)");
    Skip_B1(                                                    "ISO, ORG (0x2B)");
    Skip_B1(                                                    "SMPTE (0x34)");
    Get_B1 (Category,                                           "Category"); Param_Info1(Mxf_Category(Category));
    Get_B1 (Registry,                                           "Registry"); Param_Info1(Mxf_Registry(Category, Registry));
    Get_B1 (Structure,                                          "Structure"); Param_Info1(Mxf_Structure(Category, Registry, Structure));
    Skip_B1(                                                    "Version");
    Peek_B8(Value_lo);
    switch (Category)
    {
        case 0x01 : //Item
                    {
                    //switch (Registry)
                    //{
                    //    default   :
                                    {
                                    switch (Structure)
                                    {
                                        case 0x01 : //Standard
                                                    Param_Info1("Essence element"); //SMPTE 379M
                                                    Info_UL_01xx01_Items();
                                                    break;
                                        default   :
                                                    Skip_B8(    "Unknown");
                                    }
                                    }
                    //}
                    }
                    break;
        case 0x02 : //Group
                    {
                    //switch (Registry)
                    //{
                    //    default   :
                                    {
                                    switch (Structure)
                                    {
                                        case 0x01 :
                                                    Info_UL_02xx01_Groups();
                                                    break;
                                        default   :
                                                    Skip_B8(    "Unknown");
                                    }
                                    }
                    //}
                    }
                    break;
        case 0x04 : //Value
                    {
                    switch (Registry)
                    {
                        case 0x01 :
                                    {
                                    Param_Info1("Labels");
                                    switch (Structure)
                                    {
                                        case 0x01 :
                                                    Info_UL_040101_Values();
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
    Element_Info1C((Param), Param(Value));
    Element_End0();
    #endif
}

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File_Mxf::Info_UL_01xx01_Items()
{
    Info_B1(Code1,                                              "Item Designator");
    switch (Code1)
    {
        case 0x01 :
            {
            Param_Info1("Identifiers and locators");
            Info_B1(Code2,                                      "Code (2)");
            switch (Code2)
            {
                case 0x01 :
                    {
                    Param_Info1("GUID");
                    Info_B1(Code3,                              "Code (3)");
                    switch (Code3)
                    {
                        case 0x0D :
                            {
                            Param_Info1("UMID Mixed");
                            Info_B1(Code4,                      "Code (4)");
                            Info_B1(Code5,                      "Code (5)");
                            Info_B1(Code6,                      "Code (6)");
                            Info_B1(Code7,                      "Code (7)");
                            Info_B1(Code8,                      "Code (8)");
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
        case 0x03 :
            {
            Param_Info1("Interpretive");
            Info_B1(Code2,                                      "Code (2)");
            switch (Code2)
            {
                case 0x01 :
                    {
                    Param_Info1("Fundamental");
                    Info_B1(Code3,                              "Code (3)");
                    switch (Code3)
                    {
                        case 0x02 :
                            {
                            Param_Info1("Data Interpretations and Definitions");
                            Info_B1(Code4,                      "Code (4)");
                            switch (Code4)
                            {
                                case 0x0A :
                                    {
                                    Param_Info1("Name-Value Construct Interpretations");
                                    Info_B1(Code5,              "Code (5)");
                                    switch (Code5)
                                    {
                                        case 0x01 :
                                        case 0x02 :
                                            {
                                            Param_Info1("");
                                            Info_B1(Code6,              "Code (6)");
                                            switch (Code6)
                                            {
                                                case 0x00 :
                                                    {
                                                    Param_Info1("ANSI");
                                                    Info_B1(Code7,      "Reserved");
                                                    Info_B1(Code8,      "Reserved");
                                                    }
                                                    break;
                                                case 0x01 :
                                                    {
                                                    Param_Info1("UTF-16");
                                                    Info_B1(Code7,      "Reserved");
                                                    Info_B1(Code8,      "Reserved");
                                                    }
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
                                case 0x10 :
                                    {
                                    Param_Info1("KLV Interpretations");
                                    Info_B1(Code5,              "Code (5)");
                                    switch (Code5)
                                    {
                                        case 0x01 :
                                            {
                                            Param_Info1("Filler");
                                            Info_B1(Code6,      "Reserved");
                                            Info_B1(Code7,      "Reserved");
                                            Info_B1(Code8,      "Reserved");
                                            }
                                            break;
                                        case 0x05 :
                                            {
                                            Param_Info1("Terminating Filler");
                                            Info_B1(Code6,      "Reserved");
                                            Info_B1(Code7,      "Reserved");
                                            Info_B1(Code8,      "Reserved");
                                            }
                                            break;
                                       default   :
                                            Skip_B3(            "Unknown");
                                    }
                                    }
                                    break;
                                case 0x20 :
                                    {
                                    Param_Info1("XML Constructs and Interpretations");
                                    Info_B1(Code5,              "Code (5)");
                                    switch (Code5)
                                    {
                                        case 0x01 :
                                            {
                                            Param_Info1("XML Document Text");
                                            Info_B1(Code6,      "Reserved");
                                            Info_B1(Code7,      "Reserved");
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
            Param_Info1("User Organisation Registered For Public Use");
            Info_B1(Code2,                                      "Organization");
            switch (Code2)
            {
                case 0x01 :
                    {
                    Param_Info1("AAF");
                    Info_B1(Code3,                              "Application");
                    switch (Code3)
                    {
                        case 0x03 :
                            {
                            Param_Info1("MXF Generic Container Keys");
                            Info_B1(Code4,                      "Structure Version");
                            switch (Code4)
                            {
                                case 0x01 :
                                    {
                                    Param_Info1("Version 1");
                                    Info_B1(Code5,              "Item Type Identifier");
                                    switch (Code5)
                                    {
                                        case 0x05 : Param_Info1("CP Picture (SMPTE 386M)"); break;
                                        case 0x06 : Param_Info1("CP Sound (SMPTE 386M)"); break;
                                        case 0x07 : Param_Info1("CP Data (SMPTE 386M)"); break;
                                        case 0x14 : Param_Info1("MXF in MXF? (To confirm)"); break;
                                        case 0x15 : Param_Info1("GC Picture"); break;
                                        case 0x16 : Param_Info1("GC Sound"); break;
                                        case 0x17 : Param_Info1("GC Data"); break;
                                        case 0x18 : Param_Info1("GC Compound"); break;
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
        case 0x0E :
            {
            Param_Info1("User Organisation Registered For Private Use");
            Info_B1(Code2,                                      "Organization");
            switch (Code2)
            {
                case 0x04 :
                    {
                    Param_Info1("Avid");
                    Info_B1(Code3,                              "Application");
                    switch (Code3)
                    {
                        case 0x03 :
                            {
                            Param_Info1("Container Keys");
                            Info_B1(Code4,                      "Structure Version");
                            switch (Code4)
                            {
                                case 0x01 :
                                    {
                                    Param_Info1("Version 1");
                                    Info_B1(Code5,              "Item Type Identifier");
                                    switch (Code5)
                                    {
                                        case 0x15 : Param_Info1("Picture"); break;
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
                case 0x06 :
                    {
                    Param_Info1("Sony");
                    Info_B1(Code3,                              "Application");
                    switch (Code3)
                    {
                        case 0x7F :
                            {
                            Param_Info1("?");
                            Info_B1(Code4,                      "?");
                            switch (Code4)
                            {
                                case 0x03 :
                                    {
                                    Param_Info1("?");
                                    Info_B1(Code5,              "?");
                                    switch (Code5)
                                    {
                                        case 0x15 : Param_Info1("Picture"); break;
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
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File_Mxf::Info_UL_02xx01_Groups()
{
    Info_B1(Code1,                                              "Item Designator");
    switch (Code1)
    {
        case 0x0D :
            {
            Param_Info1("User Organisation Registered For Public Use");
            Info_B1(Code2,                                      "Organization");
            switch (Code2)
            {
                case 0x01 :
                    {
                    Param_Info1("AAF");
                    Info_B1(Code3,                              "Application");
                    switch (Code3)
                    {
                        case 0x01 :
                            {
                            Param_Info1("Structural Metadata Sets");
                            Info_B1(Code4,                      "Structure Version");
                            switch (Code4)
                            {
                                case 0x01 :
                                    {
                                    Param_Info1("Version 1");
                                    Info_B1(Code5,              "Structure Kind");
                                    switch (Code5)
                                    {
                                        case 0x01 :
                                            {
                                            Param_Info1("MXF / AAF Association compatible sets & packs");
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
                            Param_Info1("MXF File Structure");
                            Info_B1(Code4,                      "Structure Version");
                            switch (Code4)
                            {
                                case 0x01 :
                                    {
                                    Param_Info1("Version 1");
                                    Info_B1(Code5,              "Structure Kind");
                                    switch (Code5)
                                    {
                                        case 0x01 :
                                            {
                                            Param_Info1("MXF File Structure sets & packs");
                                            Info_B1(Code6,      "Set / Pack Kind");
                                            switch (Code6)
                                            {
                                                case 0x02 :
                                                    {
                                                    Param_Info1("Header Partition");
                                                    Info_B1(Code7, "Partition Status");
                                                    Info_B1(Code8, "Reserved");
                                                    }
                                                    break;
                                                case 0x03 :
                                                    {
                                                    Param_Info1("Body Partition");
                                                    Info_B1(Code7, "Partition Status");
                                                    Info_B1(Code8, "Reserved");
                                                    }
                                                    break;
                                                case 0x04 :
                                                    {
                                                    Param_Info1("Footer Partition");
                                                    Info_B1(Code7, "Partition Status");
                                                    Info_B1(Code8, "Reserved");
                                                    }
                                                    break;
                                                case 0x05 :
                                                    {
                                                    Param_Info1("Primer");
                                                    Info_B1(Code7, "Version of the Primer Pack");
                                                    Info_B1(Code8, "Reserved");
                                                    }
                                                    break;
                                                case 0x10 :
                                                    Param_Info1("Index Table Segment");
                                                    Skip_B1(    "Version");
                                                    Skip_B1(    "Reserved");
                                                    break;
                                                case 0x11 :
                                                    Param_Info1("Random Index Pack");
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
                            Param_Info1("MXF Generic Container Keys");
                            Info_B1(Code4,                      "Structure Version");
                            switch (Code4)
                            {
                                case 0x01 :
                                    {
                                    Param_Info1("MXF-GC Version 1");
                                    Info_B1(Code5,              "Item Type Identifier");
                                    switch (Code5)
                                    {
                                        case 0x04 :
                                            {
                                            Param_Info1("CP-Compatible System Item"); //SMPTE 379M
                                            Info_B1(Code6,      "System Scheme Identifier");
                                            switch (Code6)
                                            {
                                                case 0x02 :
                                                    {
                                                    Param_Info1("SDTI-CP, version 1"); //SMPTE 385M
                                                    Info_B1(Code7, "Metadata or Control Element Identifier");
                                                    switch (Code7)
                                                    {
                                                        case 0x01 :
                                                            {
                                                            Param_Info1("System Metadata Pack");
                                                            Info_B1(Code8, "Reserved");
                                                            }
                                                            break;
                                                        case 0x02 :
                                                            {
                                                            Param_Info1("Package metadata set");
                                                            Info_B1(Code8, "Metadata Block Count");
                                                            }
                                                            break;
                                                        case 0x03 :
                                                            {
                                                            Param_Info1("Picture metadata set");
                                                            Info_B1(Code8, "Metadata Block Count");
                                                            }
                                                            break;
                                                        case 0x04 :
                                                            {
                                                            Param_Info1("Sound metadata set");
                                                            Info_B1(Code8, "Metadata Block Count");
                                                            }
                                                            break;
                                                        case 0x05 :
                                                            {
                                                            Param_Info1("Data metadata set");
                                                            Info_B1(Code8, "Metadata Block Count");
                                                            }
                                                            break;
                                                        case 0x06 :
                                                            {
                                                            Param_Info1("Control data set");
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
                                            Param_Info1("GC-Compatible System Item"); //SMPTE 379M
                                            Info_B1(Code6,      "System Scheme Identifier");
                                            switch (Code6)
                                            {
                                                case 0x02 :
                                                    {
                                                    Param_Info1("GC System Scheme 1"); //SMPTE 394M
                                                    Info_B1(Code7, "Metadata or Control Element Identifier");
                                                    switch (Code7)
                                                    {
                                                        case 0x01 :
                                                            Param_Info1("First Element");
                                                            break;
                                                        case 0x02 :
                                                            Param_Info1("Subsequent Element");
                                                            break;
                                                        case 0x03 :
                                                            Param_Info1("Picture Item Descriptor");
                                                            break;
                                                        case 0x04 :
                                                            Param_Info1("Sound Item Descriptor");
                                                            break;
                                                        case 0x05 :
                                                            Param_Info1("Data Item Descriptor");
                                                            break;
                                                        case 0x06 :
                                                            Param_Info1("Control Item Descriptor");
                                                            break;
                                                        case 0x07 :
                                                            Param_Info1("Compound Item Descriptor");
                                                            break;
                                                        default   : if (Code7>=0x10 && Code7<=0x7F) Param_Info1("Pack coded System Elements (SMPTE 336M)");
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
                            Param_Info1("MXF / AAF Descriptive Metadata sets");
                            Info_B1(Code4,                      "Structure Version");
                            switch (Code4)
                            {
                                case 0x01 :
                                    {
                                    Param_Info1("Version 1");
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
            Param_Info1("User Organisation Registered For Private Use");
            Skip_B7(                                            "Private");
            break;
            }
        default   :
            Skip_B7(                                            "Unknown");
    }
}
#endif //MEDIAINFO_TRACE

//---------------------------------------------------------------------------
#if MEDIAINFO_TRACE
void File_Mxf::Info_UL_040101_Values()
{
    Info_B1(Code1,                                              "Item Designator");
    switch (Code1)
    {
        case 0x01 :
            {
            Param_Info1("Interpretive");
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
            Param_Info1("Parametric");
            Info_B1(Code2,                                      "Code (2)");
            switch (Code2)
            {
                case 0x01 :
                    {
                    Param_Info1("Picture essence");
                    Info_B1(Code3,                              "Code (3)");
                    switch (Code3)
                    {
                        case 0x01 :
                            Param_Info1("Fundamental Picture Characteristics");
                            Skip_B5(                            "Picture coding or compression");
                            break;
                        case 0x02 :
                            Param_Info1("Picture Coding Characteristics");
                            Info_B1(Code4,                      "Code (4)");
                            switch (Code4)
                            {
                                case 0x01 :
                                    Param_Info1("Uncompressed Picture Coding");
                                    Skip_B1(                    "Item Type Identifier"); //if 0x14: SMPTE 384M Uncompressed picture Line wrapped
                                    Skip_B1(                    "System Scheme Identifier"); //SMPTE 384M
                                    Skip_B1(                    "System Element Identifier"); //SMPTE 384M
                                    Skip_B1(                    "Reserved");
                                    break;
                                case 0x02 :
                                    {
                                    Param_Info1("Compressed Picture Coding");
                                    Info_B1(Code5,              "Code (5)");
                                    switch (Code5)
                                    {
                                        case 0x01 :
                                            {
                                            Param_Info1("MPEG Compression");
                                            Info_B1(Code6,      "Code (6)");
                                            switch (Code6)
                                            {
                                                case 0x01 :
                                                    Param_Info1("MPEG-2 MP@ML");
                                                    Skip_B2(    "Unknown");
                                                    break;
                                                case 0x02 :
                                                    Param_Info1("MPEG-2 422P@ML");
                                                    Skip_B2(    "Unknown");
                                                    break;
                                                case 0x03 :
                                                    Param_Info1("MPEG-2 MP@HL");
                                                    Skip_B2(    "Unknown");
                                                    break;
                                                case 0x04 :
                                                    Param_Info1("MPEG-2 422P@HL");
                                                    Skip_B2(    "Unknown");
                                                    break;
                                                case 0x10 :
                                                    Param_Info1("MPEG-1");
                                                    Skip_B2(    "Unknown");
                                                    break;
                                                case 0x20 :
                                                    Param_Info1("MPEG-4 Visual");
                                                    Skip_B2(    "Unknown");
                                                    break;
                                                case 0x32 :
                                                    Param_Info1("AVC");
                                                    Skip_B2(    "Unknown");
                                                    break;
                                                default   :
                                                    Skip_B2(    "Unknown");
                                            }
                                            }
                                            break;
                                        case 0x02 :
                                            {
                                            Param_Info1("DV Video Compression");
                                            Info_B1(Code6,      "Code (6)");
                                            switch (Code6)
                                            {
                                                case 0x02 :
                                                    Param_Info1("DV-Based Compression");
                                                    Info_B1(Code7, "DV type (SMPTE 383)");
                                                    Info_B1(Code8, "Mapping Kind"); Param_Info1(Mxf_EssenceContainer_Mapping(Code6, Code7, Code8));
                                                    break;
                                                default   :
                                                    Skip_B2(    "Unknown");
                                            }
                                            }
                                            break;
                                        case 0x03 :
                                            {
                                            Param_Info1("Individual Picture Coding Schemes");
                                            Info_B1(Code6,      "Code (6)");
                                            switch (Code6)
                                            {
                                                case 0x01 :
                                                    Param_Info1("JPEG 2000");
                                                    Skip_B1(    "Unused");
                                                    Skip_B1(    "Unused");
                                                    break;
                                                default   :
                                                    Skip_B2(    "Unknown");
                                            }
                                            }
                                            break;
                                        case 0x71 :
                                            {
                                            Param_Info1("VC-3");
                                            Skip_B1(            "Variant");
                                            Skip_B1(            "Unused");
                                            Skip_B1(            "Unused");
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
                    Param_Info1("Sound essence");
                    Info_B1(Code3,                              "Code (3)");
                    switch (Code3)
                    {
                        case 0x01 :
                            Skip_B5(                            "Sound coding or compression");
                            break;
                        case 0x02 :
                            {
                            Param_Info1("Sound Coding Characteristics");
                            Info_B1(Code4,                      "Code (4)");
                            switch (Code4)
                            {
                                case 0x01 :
                                    {
                                    Param_Info1("Uncompressed Sound Coding");
                                    Info_B1(Code5,              "Code (5)");
                                    switch (Code5)
                                    {
                                        case 0x7E :
                                            {
                                            Param_Info1("PCM (AIFF)");
                                            Skip_B3(            "Reserved");
                                            }
                                            break;
                                        case 0x7F :
                                            {
                                            Param_Info1("Undefined");
                                            Skip_B3(            "Reserved");
                                            }
                                            break;
                                        default   :
                                            Skip_B3(            "Unknown");
                                    }
                                    }
                                    break;
                                case 0x02 :
                                    {
                                    Param_Info1("Compressed Sound Coding");
                                    Info_B1(Code5,              "Code (5)");
                                    switch (Code5)
                                    {
                                        case 0x03 :
                                            {
                                            Param_Info1("Compressed Audio Coding");
                                            Info_B1(Code6,      "Code (6)");
                                            switch (Code6)
                                            {
                                                case 0x01 :
                                                    {
                                                    Param_Info1("Compandeded Audio Coding");
                                                    Info_B1(Code7, "Code (7)");
                                                    switch (Code7)
                                                    {
                                                        case 0x01 :
                                                            Param_Info1("A-law Coded Audio (default)");
                                                            Skip_B1("Unknown");
                                                            break;
                                                        case 0x02 :
                                                            Param_Info1("DV Compressed Audio");
                                                            Skip_B1("Unknown");
                                                            break;
                                                        default   :
                                                            Skip_B2("Unknown");
                                                    }
                                                    }
                                                    break;
                                                case 0x02 :
                                                    {
                                                    Param_Info1("SMPTE 338M Audio Coding");
                                                    Info_B1(Code7, "Code (7)");
                                                    switch (Code7)
                                                    {
                                                        case 0x01 :
                                                            Param_Info1("AC-3");
                                                            Skip_B1("Unknown");
                                                            break;
                                                        case 0x04 :
                                                            Param_Info1("MPEG-1 Audio Layer 1");
                                                            Skip_B1("Unknown");
                                                            break;
                                                        case 0x05 :
                                                            Param_Info1("MPEG-1 Audio Layer 2");
                                                            Skip_B1("Unknown");
                                                            break;
                                                        case 0x06 :
                                                            Param_Info1("MPEG-2 Audio Layer 1");
                                                            Skip_B1("Unknown");
                                                            break;
                                                        case 0x1C :
                                                            Param_Info1("Dolby E");
                                                            Skip_B1("Unknown");
                                                            break;
                                                        default   :
                                                            Skip_B2("Unknown");
                                                    }
                                                    }
                                                    break;
                                                case 0x03 :
                                                    {
                                                    Param_Info1("MPEG-2 Coding (not defined in SMPTE 338M)");
                                                    Info_B1(Code7, "Code (7)");
                                                    switch (Code7)
                                                    {
                                                        case 0x01 :
                                                            Param_Info1("AAC version 2");
                                                            Skip_B1("Unknown");
                                                            break;
                                                        default   :
                                                            Skip_B2("Unknown");
                                                    }
                                                    }
                                                    break;
                                                case 0x04 :
                                                    {
                                                    Param_Info1("MPEG-4 Audio Coding");
                                                    Info_B1(Code7, "Code (7)");
                                                    switch (Code7)
                                                    {
                                                        case 0x01 :
                                                            Param_Info1("MPEG-4 Speech Profile");
                                                            Skip_B1("Unknown");
                                                            break;
                                                        case 0x02 :
                                                            Param_Info1("MPEG-4 Synthesis Profile");
                                                            Skip_B1("Unknown");
                                                            break;
                                                        case 0x03 :
                                                            Param_Info1("MPEG-4 Scalable Profile");
                                                            Skip_B1("Unknown");
                                                            break;
                                                        case 0x04 :
                                                            Param_Info1("MPEG-4 Main Profile");
                                                            Skip_B1("Unknown");
                                                            break;
                                                        case 0x05 :
                                                            Param_Info1("MPEG-4 High Quality Audio Profile");
                                                            Skip_B1("Unknown");
                                                            break;
                                                        case 0x06 :
                                                            Param_Info1("MPEG-4 Low Delay Audio Profile");
                                                            Skip_B1("Unknown");
                                                            break;
                                                        case 0x07 :
                                                            Param_Info1("MPEG-4 Natural Audio Profile");
                                                            Skip_B1("Unknown");
                                                            break;
                                                        case 0x08 :
                                                            Param_Info1("MPEG-4 Mobile Audio Internetworking Profile");
                                                            Skip_B1("Unknown");
                                                            break;
                                                        default   :
                                                            Skip_B2("Unknown");
                                                    }
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
            Param_Info1("User Organisation Registered For Public Use");
            Info_B1(Code2,                                      "Organization");
            switch (Code2)
            {
                case 0x01 :
                    {
                    Param_Info1("AAF");
                    Info_B1(Code3,                              "Application");
                    switch (Code3)
                    {
                        case 0x02 :
                            {
                            Param_Info1("Operational Patterns");
                            Info_B1(Code4,                      "Version");
                            switch (Code4)
                            {
                                case 0x01 :
                                    {
                                    Param_Info1("Version 1");
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
                            Param_Info1("Essence Container Application");
                            Info_B1(Code4,                      "Structure Version");
                            switch (Code4)
                            {
                                case 0x01 :
                                    {
                                    Param_Info1("MXF EC Structure Version 1");
                                    Info_B1(Code5,              "Essence container Kind");
                                    switch (Code5)
                                    {
                                        case 0x01 :
                                            Param_Info1("Deprecated Essence Container Kind");
                                            Skip_B3(            "Unknown");
                                            break;
                                        case 0x02 :
                                            {
                                            Param_Info1("Essence Container Kind");
                                            Info_B1(Code6,      "Code (6)");
                                            switch (Code6)
                                            {
                                                case 0x01 : //SMPTE 386M
                                                    {
                                                    Param_Info1("Type D-10 Mapping");
                                                    Skip_B1(            "MPEG Constraints"); //SMPTE 356M
                                                    Skip_B1(            "Template Extension");
                                                    }
                                                    break;
                                                case 0x02 :
                                                    {
                                                    Param_Info1("DV Mappings");
                                                    Skip_B1(            "Mapping Kind");
                                                    Skip_B1(            "Locally defined");
                                                    }
                                                    break;
                                                case 0x03 :
                                                    {
                                                    Param_Info1("Type D-11 Mapping");
                                                    Skip_B1(            "Mapping Kind");
                                                    Skip_B1(            "Locally defined");
                                                    }
                                                    break;
                                                case 0x04 :
                                                    {
                                                    Param_Info1("MPEG ES mappings");
                                                    Info_B1(Code7,      "ISO13818-1 stream_id bits 6..0"); Param_Info1(Ztring::ToZtring(0x80+Code7, 16));
                                                    Info_B1(Code8,      "Mapping Kind"); Param_Info1(Mxf_EssenceContainer_Mapping(Code6, Code7, Code8));
                                                    }
                                                    break;
                                                case 0x05 : //SMPTE 384M
                                                    {
                                                    Param_Info1("Uncompressed Pictures");
                                                    Info_B1(Code7,      "Number of lines / field rate combination"); //SMPTE 384M
                                                    Info_B1(Code8,      "Mapping Kind"); Param_Info1(Mxf_EssenceContainer_Mapping(Code6, Code7, Code8));
                                                    }
                                                    break;
                                                case 0x06 :
                                                    {
                                                    Param_Info1("AES-BWF");
                                                    Info_B1(Code7,      "Mapping Kind"); Param_Info1(Mxf_EssenceContainer_Mapping(Code6, Code7, 0x00));
                                                    Skip_B1(            "Locally defined");
                                                    }
                                                    break;
                                                case 0x07 :
                                                    {
                                                    Param_Info1("MPEG PES mappings");
                                                    Info_B1(Code7,      "ISO13818-1 stream_id bits 6..0"); Param_Info1(Ztring::ToZtring(0x80+Code7, 16));
                                                    Info_B1(Code8,      "Mapping Kind"); Param_Info1(Mxf_EssenceContainer_Mapping(Code6, Code7, Code8));
                                                    }
                                                    break;
                                                case 0x08 :
                                                    {
                                                    Param_Info1("MPEG PS mappings");
                                                    Info_B1(Code7,      "ISO13818-1 stream_id bits 6..0"); Param_Info1(Ztring::ToZtring(0x80+Code7, 16));
                                                    Info_B1(Code8,      "Mapping Kind"); Param_Info1(Mxf_EssenceContainer_Mapping(Code6, Code7, Code8));
                                                    }
                                                    break;
                                                case 0x09 :
                                                    {
                                                    Param_Info1("MPEG TS mappings");
                                                    Info_B1(Code7,      "ISO13818-1 stream_id bits 6..0"); Param_Info1(Ztring::ToZtring(0x80+Code7, 16));
                                                    Info_B1(Code8,      "Mapping Kind"); Param_Info1(Mxf_EssenceContainer_Mapping(Code6, Code7, Code8));
                                                    }
                                                    break;
                                                case 0x0A :
                                                    {
                                                    Param_Info1("A-law Sound Element Mapping");
                                                    Info_B1(Code7,      "Mapping Kind"); Param_Info1(Mxf_EssenceContainer_Mapping(Code6, Code7, 0xFF));
                                                    Skip_B1(            "Locally defined");
                                                    }
                                                    break;
                                                case 0x0B :
                                                    {
                                                    Param_Info1("Encrypted Generic Container");
                                                    Skip_B1(            "Mapping Kind");
                                                    Skip_B1(            "Locally defined");
                                                    }
                                                    break;
                                                case 0x0C :
                                                    {
                                                    Param_Info1("JPEG 2000 Picture Mapping");
                                                    Skip_B1(            "Mapping Kind");
                                                    Skip_B1(            "Locally defined");
                                                    }
                                                    break;
                                                case 0x11 :
                                                    {
                                                    Param_Info1("VC-3 Picture Element");
                                                    Info_B1(Code7,      "Content Kind"); Param_Info1(Mxf_EssenceContainer_Mapping(Code6, Code7, 0xFF));
                                                    Skip_B1(            "Reserved");
                                                    }
                                                    break;
                                                case 0x16 :
                                                    {
                                                    Param_Info1("AVC Picture Element");
                                                    Skip_B1(            "Unknown");
                                                    Skip_B1(            "Unknown");
                                                    }
                                                    break;
                                                case 0x7F :
                                                    {
                                                    Param_Info1("Generic Essence Container Wrapping");
                                                    Skip_B1(            "Mapping Kind");
                                                    Skip_B1(            "Locally defined");
                                                    }
                                                    break;
                                                default   :
                                                    {
                                                    Skip_B1(            "Mapping Kind");
                                                    Skip_B1(            "Locally defined");
                                                    }
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
                            Param_Info1("MXF / AAF compatible Descriptive Metadata Labels");
                            Info_B1(Code4,                      "Label Version");
                            switch (Code4)
                            {
                                case 0x01 :
                                    {
                                    Param_Info1("Version 1");
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
                    Param_Info1("EBU/UER");
                    Skip_B6(                                    "Unknown");
                    }
                    break;
                case 0x03 :
                    {
                    Param_Info1("Pro-MPEG Forum");
                    Skip_B6(                                    "Unknown");
                    }
                    break;
                case 0x04 :
                    {
                    Param_Info1("BBC");
                    Skip_B6(                                    "Unknown");
                    }
                    break;
                case 0x05 :
                    {
                    Param_Info1("IRT");
                    Skip_B6(                                    "Unknown");
                    }
                    break;
                case 0x06 :
                    {
                    Param_Info1("ARIB");
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
            Param_Info1("User Organisation Registered For Private Use");
            Info_B1(Code2,                                      "Code (2)");
            switch (Code2)
            {
                case 0x04 :
                    {
                    Param_Info1("Avid");
                    Info_B1(Code3,                              "Code (3)");
                    switch (Code3)
                    {
                        case 0x02 :
                            {
                            Param_Info1("Essence Compression?");
                            Info_B1(Code4,                      "?");
                            switch (Code4)
                            {
                                case 0x01 :
                                    {
                                    Param_Info1("?");
                                    Info_B1(Code5,              "?");
                                    switch (Code5)
                                    {
                                        case 0x02 :
                                            {
                                            Param_Info1("?");
                                            Info_B1(Code6,      "Code (6)");
                                            switch (Code6)
                                            {
                                                case 0x04 :
                                                    Param_Info1("VC-3");
                                                    Skip_B2(    "Unknown");
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
                        case 0x03 :
                            {
                            Param_Info1("Essence Container Application");
                            Info_B1(Code4,                      "Structure Version");
                            switch (Code4)
                            {
                                case 0x01 :
                                    {
                                    Param_Info1("MXF EC Structure Version 1");
                                    Info_B1(Code5,              "Essence container Kind");
                                    switch (Code5)
                                    {
                                        case 0x02 :
                                            {
                                            Param_Info1("Essence Container Kind");
                                            Info_B1(Code6,      "Code (6)");
                                            switch (Code6)
                                            {
                                                case 0x06 :
                                                    Param_Info1("VC-3");
                                                    Skip_B2(    "Unknown");
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
                case 0x06 :
                    {
                    Param_Info1("Sony");
                    Info_B1(Code3,                              "Code (3)");
                    switch (Code3)
                    {
                        case 0x04 :
                            {
                            Param_Info1("Essence Compression?");
                            Info_B1(Code4,                      "?");
                            switch (Code4)
                            {
                                case 0x01 :
                                    {
                                    Param_Info1("?");
                                    Info_B1(Code5,              "?");
                                    switch (Code5)
                                    {
                                        case 0x02 :
                                            {
                                            Param_Info1("?");
                                            Info_B1(Code6,      "Code (6)");
                                            switch (Code6)
                                            {
                                                case 0x04 :
                                                    {
                                                    Param_Info1("?");
                                                    Info_B1(Code7,      "Code (7)");
                                                    switch (Code7)
                                                    {
                                                        case 0x02 :
                                                            Param_Info1("?");
                                                            Info_B1(Code8,      "Code (8)");
                                                            switch (Code8)
                                                            {
                                                                case 0x01 :
                                                                    Param_Info1("RAW SQ");
                                                                    break;
                                                                default   :
                                                                    ;
                                                            }
                                                            break;
                                                        default   :
                                                            Skip_B1(    "Unknown");
                                                    }
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
                        case 0x0D :
                            {
                            Param_Info1("Essence Container?");
                            Info_B1(Code4,                      "?");
                            switch (Code4)
                            {
                                case 0x03 :
                                    {
                                    Param_Info1("?");
                                    Info_B1(Code5,              "?");
                                    switch (Code5)
                                    {
                                        case 0x02 :
                                            {
                                            Param_Info1("?");
                                            Info_B1(Code6,      "Code (6)");
                                            switch (Code6)
                                            {
                                                case 0x01 :
                                                    {
                                                    Param_Info1("?");
                                                    Info_B1(Code7,      "Code (7)");
                                                    switch (Code7)
                                                    {
                                                        case 0x01 :
                                                            Param_Info1("RAW?");
                                                            Skip_B1(    "Unknown");
                                                            break;
                                                        default   :
                                                            Skip_B1(    "Unknown");
                                                    }
                                                    }
                                                    break;
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
                                Skip_B5(                        "Private");
                        }
                        }
                        break;
                default   :
                    Skip_B6(                                    "Private");
            }
            }
            break;
        default   :
            Skip_B7(                                            "Unknown");
    }
}
#endif //MEDIAINFO_TRACE

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
    Get_UUID (Value.lo,                                         "UUID"); Element_Info1(Ztring().From_UUID(Value.lo));
}

//---------------------------------------------------------------------------
void File_Mxf::Skip_UMID()
{
    //Parsing
    Skip_UUID(                                                  "Fixed");
    Info_UUID(Data,                                             "UUID"); Element_Info1(Ztring().From_UUID(Data));
}

//---------------------------------------------------------------------------
void File_Mxf::Get_Timestamp(Ztring &Value)
{
    //Parsing
    int16u  Year;
    int8u   Month, Day, Hours, Minutes, Seconds, Milliseconds;
    Get_B2 (Year,                                               "Year");
    Get_B1 (Month,                                              "Month");
    Get_B1 (Day,                                                "Day");
    Get_B1 (Hours,                                              "Hours");
    Get_B1 (Minutes,                                            "Minutes");
    Get_B1 (Seconds,                                            "Seconds");
    Get_B1 (Milliseconds,                                       "Milliseconds/4"); Param_Info2(Milliseconds*4, " ms");
    Value.From_Number(Year);
    Value+=__T('-');
    Ztring Temp;
    Temp.From_Number(Month);
    if (Temp.size()<2)
        Temp.insert(0, 1, __T('0'));
    Value+=Temp;
    Value+=__T('-');
    Temp.From_Number(Day);
    if (Temp.size()<2)
        Temp.insert(0, 1, __T('0'));
    Value+=Temp;
    Value+=__T(' ');
    Temp.From_Number(Hours);
    if (Temp.size()<2)
        Temp.insert(0, 1, __T('0'));
    Value+=Temp;
    Value+=__T(':');
    Temp.From_Number(Minutes);
    if (Temp.size()<2)
        Temp.insert(0, 1, __T('0'));
    Value+=Temp;
    Value+=__T(':');
    Temp.From_Number(Seconds);
    if (Temp.size()<2)
        Temp.insert(0, 1, __T('0'));
    Value+=Temp;
    Value+=__T('.');
    Temp.From_Number(Milliseconds*4);
    if (Temp.size()<3)
        Temp.insert(0, 3-Temp.size(), __T('0'));
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
    Info_B1(Milliseconds,                                       "Milliseconds/4"); Param_Info2(Milliseconds*4, " ms");
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
    Info_B1(Milliseconds,                                       "Milliseconds/4"); Param_Info2(Milliseconds*4, " ms");
    Element_Info1(Ztring::ToZtring(Year          )+__T('-')+
                 Ztring::ToZtring(Month         )+__T('-')+
                 Ztring::ToZtring(Day           )+__T(' ')+
                 Ztring::ToZtring(Hours         )+__T(':')+
                 Ztring::ToZtring(Minutes       )+__T(':')+
                 Ztring::ToZtring(Seconds       )+__T('.')+
                 Ztring::ToZtring(Milliseconds*4)         );
}

//---------------------------------------------------------------------------
void File_Mxf::Get_BER(int64u &Value, const char* Name)
{
    int8u Length;
    Get_B1(Length,                                              Name);
    if (Length<0x80)
    {
        Value=Length; //1-byte
        return;
    }

    Length&=0x7F;
    switch (Length)
    {
        case 1 :
                {
                int8u  Length1;
                Get_B1(Length1,                                 Name);
                Value=Length1;
                break;
                }
        case 2 :
                {
                int16u Length2;
                Get_B2(Length2,                                 Name);
                Value=Length2;
                break;
                }
        case 3 :
                {
                int32u Length3;
                Get_B3(Length3,                                 Name);
                Value=Length3;
                break;
                }
        case 4 :
                {
                int32u Length4;
                Get_B4(Length4,                                 Name);
                Value=Length4;
                break;
                }
        case 5 :
                {
                int64u Length5;
                Get_B5(Length5,                                 Name);
                Value=Length5;
                break;
                }
        case 6 :
                {
                int64u Length6;
                Get_B6(Length6,                                 Name);
                Value=Length6;
                break;
                }
        case 7 :
                {
                int64u Length7;
                Get_B7(Length7,                                 Name);
                Value=Length7;
                break;
                }
        case 8 :
                {
                int64u Length8;
                Get_B8(Length8,                                 Name);
                Value=Length8;
                break;
                }
        default:Value=(int64u)-1; //Problem
    }
}

//***************************************************************************
// Parsers
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mxf::ChooseParser(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    if ((Descriptor->second.EssenceCompression.hi&0xFFFFFFFFFFFFFF00LL)!=0x060E2B3404010100LL || (Descriptor->second.EssenceCompression.lo&0xFF00000000000000LL)!=0x0400000000000000LL)
        return ChooseParser__FromEssenceContainer (Essence, Descriptor);

    int8u Code2=(int8u)((Descriptor->second.EssenceCompression.lo&0x00FF000000000000LL)>>48);
    int8u Code3=(int8u)((Descriptor->second.EssenceCompression.lo&0x0000FF0000000000LL)>>40);
    int8u Code4=(int8u)((Descriptor->second.EssenceCompression.lo&0x000000FF00000000LL)>>32);
    int8u Code5=(int8u)((Descriptor->second.EssenceCompression.lo&0x00000000FF000000LL)>>24);
    int8u Code6=(int8u)((Descriptor->second.EssenceCompression.lo&0x0000000000FF0000LL)>>16);
    int8u Code7=(int8u)((Descriptor->second.EssenceCompression.lo&0x000000000000FF00LL)>> 8);

    switch (Code2)
    {
        case 0x01 : //Picture
                    switch (Code3)
                    {
                        case 0x02 : //Coding characteristics
                                    switch (Code4)
                                    {
                                        case 0x01 : //Uncompressed Picture Coding
                                                    switch (Code5)
                                                    {
                                                        case 0x01 : return ChooseParser_Raw(Essence, Descriptor);
                                                        case 0x7F : return ChooseParser_RV24(Essence, Descriptor);
                                                        default   : return;
                                                    }
                                        case 0x02 : //Compressed coding
                                                    switch (Code5)
                                                    {
                                                        case 0x01 : //MPEG Compression
                                                                    switch (Code6)
                                                                    {
                                                                        case 0x01 :
                                                                        case 0x02 :
                                                                        case 0x03 :
                                                                        case 0x04 :
                                                                        case 0x11 : return ChooseParser_Mpegv(Essence, Descriptor);
                                                                        case 0x20 : return ChooseParser_Mpeg4v(Essence, Descriptor);
                                                                        case 0x30 :
                                                                        case 0x31 :
                                                                        case 0x32 :
                                                                        case 0x33 :
                                                                        case 0x34 :
                                                                        case 0x35 :
                                                                        case 0x36 :
                                                                        case 0x37 :
                                                                        case 0x38 :
                                                                        case 0x39 :
                                                                        case 0x3A :
                                                                        case 0x3B :
                                                                        case 0x3C :
                                                                        case 0x3D :
                                                                        case 0x3E :
                                                                        case 0x3F : return ChooseParser_Avc(Essence, Descriptor);
                                                                        default   : return;
                                                                    }
                                                        case 0x02 : return ChooseParser_DV(Essence, Descriptor);
                                                        case 0x03 : //Individual Picture Coding Schemes
                                                                    switch (Code6)
                                                                    {
                                                                        case 0x01 : return ChooseParser_Jpeg2000(Essence, Descriptor);
                                                                        default   : return;
                                                                    }
                                                        case 0x71 : return ChooseParser_Vc3(Essence, Descriptor);
                                                        default   : return;
                                                    }
                                         default   : return;
                                    }
                         default   : return;
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
                                                        case 0x01 :
                                                        case 0x7F : if (Descriptor->second.ChannelCount==1) //PCM, but one file is found with Dolby E in it
                                                                        ChooseParser_ChannelGrouping(Essence, Descriptor);
                                                                    if (Descriptor->second.ChannelCount==2) //PCM, but one file is found with Dolby E in it
                                                                        ChooseParser_SmpteSt0337(Essence, Descriptor);
                                                        default   : return ChooseParser_Pcm(Essence, Descriptor);
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
                                                                                        case 0x01 : if ((Descriptor->second.EssenceContainer.lo&0xFFFF0000)==0x02060000) //Test coherency between container and compression
                                                                                                        return ChooseParser_Pcm(Essence, Descriptor); //Compression is A-Law but Container is PCM, not logic, prioritizing PCM
                                                                                                     else
                                                                                                        return ChooseParser_Alaw(Essence, Descriptor);
                                                                                        case 0x10 : return ChooseParser_Pcm(Essence, Descriptor); //DV 12-bit
                                                                                        default   : return;
                                                                                    }
                                                                        case 0x02 : //SMPTE 338M Audio Coding
                                                                                    switch (Code7)
                                                                                    {
                                                                                        case 0x01 : if (Descriptor->second.IsAes3Descriptor)
                                                                                                        return ChooseParser_SmpteSt0337(Essence, Descriptor);
                                                                                                    else
                                                                                                        return ChooseParser_Ac3(Essence, Descriptor);
                                                                                        case 0x04 :
                                                                                        case 0x05 :
                                                                                        case 0x06 : if (Descriptor->second.IsAes3Descriptor)
                                                                                                        return ChooseParser_SmpteSt0337(Essence, Descriptor);
                                                                                                    else
                                                                                                        return ChooseParser_Mpega(Essence, Descriptor);
                                                                                        case 0x1C : if (Descriptor->second.ChannelCount==1)
                                                                                                        return ChooseParser_ChannelGrouping(Essence, Descriptor); //Dolby E (in 2 mono streams)
                                                                                                    else
                                                                                                        return ChooseParser_SmpteSt0337(Essence, Descriptor); //Dolby E (in 1 stereo streams)
                                                                                        default   : return;
                                                                                    }
                                                                        case 0x03 : //MPEG-2 Coding (not defined in SMPTE 338M)
                                                                                    switch (Code7)
                                                                                    {
                                                                                        case 0x01 : return ChooseParser_Aac(Essence, Descriptor);
                                                                                        default   : return;
                                                                                    }
                                                                        case 0x04 : //MPEG-4 Audio Coding
                                                                                    switch (Code7)
                                                                                    {
                                                                                        case 0x01 :
                                                                                        case 0x02 :
                                                                                        case 0x03 :
                                                                                        case 0x04 :
                                                                                        case 0x05 :
                                                                                        case 0x06 :
                                                                                        case 0x07 :
                                                                                        case 0x08 : return ChooseParser_Aac(Essence, Descriptor);
                                                                                        default   : return;
                                                                                    }
                                                                        default   : return;
                                                                    }
                                                         default   : return;
                                                    }
                                         default   : return;
                                    }
                         default   : return;
                    }
        default   : return;
    }
}

//---------------------------------------------------------------------------
void File_Mxf::ChooseParser__FromEssenceContainer(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    int8u Code1=(int8u)((Descriptor->second.EssenceContainer.lo&0xFF00000000000000LL)>>56);
    int8u Code2=(int8u)((Descriptor->second.EssenceContainer.lo&0x00FF000000000000LL)>>48);
    int8u Code3=(int8u)((Descriptor->second.EssenceContainer.lo&0x0000FF0000000000LL)>>40);
    int8u Code4=(int8u)((Descriptor->second.EssenceContainer.lo&0x000000FF00000000LL)>>32);
    int8u Code5=(int8u)((Descriptor->second.EssenceContainer.lo&0x00000000FF000000LL)>>24);
    int8u Code6=(int8u)((Descriptor->second.EssenceContainer.lo&0x0000000000FF0000LL)>>16);
    //int8u Code7=(int8u)((Descriptor->second.EssenceContainer.lo&0x000000000000FF00LL)>> 8);

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
                                                                                        case 0x01 : switch(Descriptor->second.StreamKind)
                                                                                                    {
                                                                                                        case Stream_Video : return ChooseParser_Mpegv(Essence, Descriptor);
                                                                                                        case Stream_Audio : return ChooseParser_SmpteSt0331(Essence, Descriptor);
                                                                                                        default           : return;
                                                                                                    }
                                                                                        case 0x02 : return; //DV
                                                                                        case 0x05 : return ChooseParser_Raw(Essence, Descriptor);
                                                                                        case 0x06 : if (Descriptor->second.ChannelCount==1) //PCM, but one file is found with Dolby E in it
                                                                                                        ChooseParser_ChannelGrouping(Essence, Descriptor);
                                                                                                    if (Descriptor->second.ChannelCount==2) //PCM, but one file is found with Dolby E in it
                                                                                                        ChooseParser_SmpteSt0337(Essence, Descriptor);
                                                                                                    return ChooseParser_Pcm(Essence, Descriptor);
                                                                                        case 0x04 : return; //MPEG ES mappings with Stream ID
                                                                                        case 0x0A : return ChooseParser_Alaw(Essence, Descriptor);
                                                                                        case 0x0C : return ChooseParser_Jpeg2000(Essence, Descriptor);
                                                                                        case 0x10 : return ChooseParser_Avc(Essence, Descriptor);
                                                                                        case 0x11 : return ChooseParser_Vc3(Essence, Descriptor);
                                                                                        default   : return;
                                                                                    }
                                                                        default   : return;
                                                                    }
                                                         default   : return;
                                                    }
                                         default   : return;
                                    }
                        default   : return;
                    }
        case 0x0E : //Private Use
                    switch (Code2)
                    {
                        case 0x04 : //Avid
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
                                                                                        case 0x06 : return ChooseParser_Vc3(Essence, Descriptor);
                                                                                        default   : return;
                                                                                    }
                                                                        default   : return;
                                                                    }
                                                         default   : return;
                                                    }
                                         default   : return;
                                    }
                        default   : return;
                    }
        default   : return;
    }
}

//---------------------------------------------------------------------------
void File_Mxf::ChooseParser__FromEssence(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    int32u Code_Compare3=Code.lo>>32;

    switch (Code_Compare3)
    {
        case Elements::GenericContainer_Aaf3        : return ChooseParser__Aaf(Essence, Descriptor);
        case Elements::GenericContainer_Avid3       : return ChooseParser__Avid(Essence, Descriptor);
        case Elements::GenericContainer_Sony3       : return ChooseParser__Sony(Essence, Descriptor);
        default                                     : return;
    }
}

//---------------------------------------------------------------------------
void File_Mxf::ChooseParser__Aaf(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    int32u Code_Compare4=(int32u)Code.lo;
    int8u  Code_Compare4_1=Code_Compare4>>24;

    switch (Code_Compare4_1)
    {
        case 0x05 : //CP Picture
                    ChooseParser__Aaf_CP_Picture(Essence, Descriptor);
                    break;
        case 0x06 : //CP Sound
                    ChooseParser__Aaf_CP_Sound(Essence, Descriptor);
                    break;
        case 0x07 : //CP Data
                    ChooseParser__Aaf_CP_Data(Essence, Descriptor);
                    break;
        case 0x14 : //MXF in MXF?
                    ChooseParser__Aaf_14(Essence, Descriptor);
                    break;
        case 0x15 : //CP Picture
                    ChooseParser__Aaf_GC_Picture(Essence, Descriptor);
                    break;
        case 0x16 : //CP Sound
                    ChooseParser__Aaf_GC_Sound(Essence, Descriptor);
                    break;
        case 0x17 : //CP Data
                    ChooseParser__Aaf_GC_Data(Essence, Descriptor);
                    break;
        case 0x18 : //CP Compound
                    ChooseParser__Aaf_GC_Compound(Essence, Descriptor);
                    break;
        default   : //Unknown
                    ;
    }
}

//---------------------------------------------------------------------------
void File_Mxf::ChooseParser__Avid(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    int32u Code_Compare4=(int32u)Code.lo;
    int8u  Code_Compare4_1=Code_Compare4>>24;

    switch (Code_Compare4_1)
    {
        case 0x15 : //CP Picture
                    ChooseParser__Avid_Picture(Essence, Descriptor);
                    break;
        default   : //Unknown
                    ;
    }
}

//---------------------------------------------------------------------------
// 0x05, SMPTE 386M
void File_Mxf::ChooseParser__Aaf_CP_Picture(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    int32u Code_Compare4=(int32u)Code.lo;
    int8u  Code_Compare4_3=(int8u)(Code_Compare4>>8);

    Essences[Code_Compare4].StreamKind=Stream_Video;
    Essences[Code_Compare4].StreamPos=Code_Compare4&0x000000FF;

    switch (Code_Compare4_3)
    {
        case 0x01 : //D-10 Video, SMPTE 386M
                    ChooseParser_Mpegv(Essence, Descriptor);
                    break;
        default   : //Unknown
                    ;
    }
}

//---------------------------------------------------------------------------
void File_Mxf::ChooseParser__Sony(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    int32u Code_Compare4=(int32u)Code.lo;
    int8u  Code_Compare4_1=Code_Compare4>>24;

    switch (Code_Compare4_1)
    {
        case 0x15 : //CP Picture
                    ChooseParser__Sony_Picture(Essence, Descriptor);
                    break;
        default   : //Unknown
                    ;
    }
}

//---------------------------------------------------------------------------
// 0x06, SMPTE 386M
void File_Mxf::ChooseParser__Aaf_CP_Sound(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    int32u Code_Compare4=(int32u)Code.lo;
    int8u  Code_Compare4_3=(int8u)(Code_Compare4>>8);

    Essences[Code_Compare4].StreamKind=Stream_Audio;
    Essences[Code_Compare4].StreamPos=Code_Compare4&0x000000FF;

    switch (Code_Compare4_3)
    {
        case 0x10 : //D-10 Audio, SMPTE 386M
                    ChooseParser_SmpteSt0331(Essence, Descriptor);
                    break;
        default   : //Unknown
                    ;
    }
}

//---------------------------------------------------------------------------
// 0x07, SMPTE 386M
void File_Mxf::ChooseParser__Aaf_CP_Data(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
}

//---------------------------------------------------------------------------
// 0x14
void File_Mxf::ChooseParser__Aaf_14(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    int32u Code_Compare4=(int32u)Code.lo;
    int8u  Code_Compare4_3=(int8u)(Code_Compare4>>8);

    switch (Code_Compare4_3)
    {
        case 0x01 : //MXF in MXF?
                    Essence->second.Parsers.push_back(new File_Mxf());
                    break;
        default   : ;
    }
}

//---------------------------------------------------------------------------
// 0x15
void File_Mxf::ChooseParser__Aaf_GC_Picture(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    int32u Code_Compare4=(int32u)Code.lo;
    int8u  Code_Compare4_3=(int8u)(Code_Compare4>>8);

    Essences[Code_Compare4].StreamKind=Stream_Video;
    Essences[Code_Compare4].StreamPos=Code_Compare4&0x000000FF;

    switch (Code_Compare4_3)
    {
        case 0x01 : //RV24
                    ChooseParser_RV24(Essence, Descriptor);
                    break;
        case 0x02 : //Raw video
                    ChooseParser_Raw(Essence, Descriptor);
                    break;
        case 0x05 : //SMPTE 381M, Frame wrapped
                    ChooseParser_Mpegv(Essence, Descriptor); //Trying...
                    Essences[Code_Compare4].Infos["Format_Settings_Wrapping"]=__T("Frame");
                    DataMustAlwaysBeComplete=true;
                    break;
        case 0x06 : //SMPTE 381M, Clip wrapped
                    ChooseParser_Mpegv(Essence, Descriptor); //Trying...
                    Essences[Code_Compare4].Infos["Format_Settings_Wrapping"]=__T("Clip");
                    break;
        case 0x07 : //SMPTE 381M, Custom wrapped
                    ChooseParser_Mpegv(Essence, Descriptor); //Trying...
                    Essences[Code_Compare4].Infos["Format_Settings_Wrapping"]=__T("Custom");
                    break;
        case 0x08 : //JPEG 2000
                    ChooseParser_Jpeg2000(Essence, Descriptor);
                    break;
        case 0x0D : //VC-3
                    ChooseParser_Vc3(Essence, Descriptor);
                    break;
        default   : //Unknown
                    ;
    }
}

//---------------------------------------------------------------------------
// 0x16
void File_Mxf::ChooseParser__Aaf_GC_Sound(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    int32u Code_Compare4=(int32u)Code.lo;
    int8u  Code_Compare4_3=(int8u)(Code_Compare4>>8);

    Essences[Code_Compare4].StreamKind=Stream_Audio;
    Essences[Code_Compare4].StreamPos=Code_Compare4&0x000000FF;

    switch (Code_Compare4_3)
    {
        case 0x01 : //BWF (PCM)
        case 0x02 : //BWF (PCM)
        case 0x03 : //DV Audio (PCM)
        case 0x04 : //P2 Audio (PCM)
                    ChooseParser_Pcm(Essence, Descriptor);
                    break;
        case 0x05 : //MPEG Audio
                    ChooseParser_Mpega(Essence, Descriptor);
                    break;
        case 0x08 : //A-law, Frame wrapped
                    ChooseParser_Alaw(Essence, Descriptor);
                    Essences[Code_Compare4].Infos["Format_Settings_Wrapping"]=__T("Frame");
                    DataMustAlwaysBeComplete=true;
                    break;
        case 0x09 : //A-law, Clip wrapped
                    ChooseParser_Alaw(Essence, Descriptor);
                    Essences[Code_Compare4].Infos["Format_Settings_Wrapping"]=__T("Clip");
                    break;
        case 0x0A : //A-law, Custom wrapped
                    ChooseParser_Alaw(Essence, Descriptor);
                    Essences[Code_Compare4].Infos["Format_Settings_Wrapping"]=__T("Custom");
                    break;
        default   : //Unknown
                    ;
    }
}

//---------------------------------------------------------------------------
// 0x17
void File_Mxf::ChooseParser__Aaf_GC_Data(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    int32u Code_Compare4=(int32u)Code.lo;
    int8u  Code_Compare4_3=(int8u)(Code_Compare4>>8);

    switch (Code_Compare4_3)
    {
        case 0x01 : //VBI, SMPTE ST 436
                    Essence->second.Parsers.push_back(new File__Analyze());
                    break;
        case 0x02 : //Ancillary
                    if (!Ancillary)
                        Ancillary=new File_Ancillary();
                    Essence->second.Parsers.push_back(Ancillary);
                    Ancillary_IsBinded=true;
                    break;
        case 0x08 : //Line Wrapped Data Element, SMPTE 384M
        case 0x09 : //Line Wrapped VANC Data Element, SMPTE 384M
        case 0x0A : //Line Wrapped HANC Data Element, SMPTE 384M
                    break;
        default   : //Unknown
                    ;
    }
}

//---------------------------------------------------------------------------
// 0x18
void File_Mxf::ChooseParser__Aaf_GC_Compound(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    int32u Code_Compare4=(int32u)Code.lo;
    int8u  Code_Compare4_3=(int8u)(Code_Compare4>>8);

    Essences[Code_Compare4].StreamKind=Stream_Video; //Default to video, audio will be merge later
    Essences[Code_Compare4].StreamPos=Code_Compare4&0x000000FF;

    switch (Code_Compare4_3)
    {
        case 0x01 : //DV
        case 0x02 : //DV
                    ChooseParser_DV(Essence, Descriptor);
                    break;
        default   : //Unknown
                    ;
    }
}

//---------------------------------------------------------------------------
// 0x15
void File_Mxf::ChooseParser__Avid_Picture(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    int32u Code_Compare4=(int32u)Code.lo;
    int8u  Code_Compare4_3=(int8u)(Code_Compare4>>8);

    Essences[Code_Compare4].StreamKind=Stream_Video;
    Essences[Code_Compare4].StreamPos=Code_Compare4&0x000000FF;

    switch (Code_Compare4_3)
    {
        case 0x05 : //VC-1, Frame wrapped
                    ChooseParser_Vc3(Essence, Descriptor);
                    Essences[Code_Compare4].Infos["Format_Settings_Wrapping"]=__T("Frame");
                    DataMustAlwaysBeComplete=true;
                    break;
        case 0x06 : //VC-1, Clip wrapped
                    ChooseParser_Vc3(Essence, Descriptor);
                    Essences[Code_Compare4].Infos["Format_Settings_Wrapping"]=__T("Clip");
                    break;
        case 0x07 : //VC-1, Custom wrapped
                    ChooseParser_Vc3(Essence, Descriptor);
                    Essences[Code_Compare4].Infos["Format_Settings_Wrapping"]=__T("Custom");
                    break;
        default   : //Unknown
                    ;
    }
}

//---------------------------------------------------------------------------
// 0x15
void File_Mxf::ChooseParser__Sony_Picture(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    int32u Code_Compare4=(int32u)Code.lo;

    Essences[Code_Compare4].StreamKind=Stream_Video;
    Essences[Code_Compare4].StreamPos=Code_Compare4&0x000000FF;
}

//---------------------------------------------------------------------------
void File_Mxf::ChooseParser_Avc(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    Essence->second.StreamKind=Stream_Video;

    //Filling
    #if defined(MEDIAINFO_AVC_YES)
        File_Avc* Parser=new File_Avc;
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Video);
        Parser->Fill(Stream_Video, 0, Video_Format, "AVC");
    #endif
    Essence->second.Parsers.push_back(Parser);
}

//---------------------------------------------------------------------------
void File_Mxf::ChooseParser_DV(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    Essence->second.StreamKind=Stream_Video;

    //Filling
    #if defined(MEDIAINFO_DVDIF_YES)
        File_DvDif* Parser=new File_DvDif;
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Video);
        Parser->Fill(Stream_Audio, 0, Audio_Format, "DV");
    #endif
    Essence->second.Parsers.push_back(Parser);
}

//---------------------------------------------------------------------------
void File_Mxf::ChooseParser_Mpeg4v(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    Essence->second.StreamKind=Stream_Video;

    //Filling
    #if defined(MEDIAINFO_MPEG4V_YES)
        File_Mpeg4v* Parser=new File_Mpeg4v;
        Parser->OnlyVOP();
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Video);
        Parser->Fill(Stream_Video, 0, Video_Format, "MPEG-4 Visual");
    #endif
    Essence->second.Parsers.push_back(Parser);
}

//---------------------------------------------------------------------------
void File_Mxf::ChooseParser_Mpegv(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    Essence->second.StreamKind=Stream_Video;

    //Filling
    #if defined(MEDIAINFO_MPEGV_YES)
        File_Mpegv* Parser=new File_Mpegv();
        Parser->Ancillary=&Ancillary;
        #if MEDIAINFO_ADVANCED
            Parser->InitDataNotRepeated_Optional=true;
        #endif // MEDIAINFO_ADVANCED
        #if MEDIAINFO_DEMUX
            if (Demux_UnpacketizeContainer)
            {
                Parser->Demux_Level=2; //Container
                Parser->Demux_UnpacketizeContainer=true;
            }
        #endif //MEDIAINFO_DEMUX
    #else
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Video);
        Parser->Fill(Stream_Video, 0, Video_Format, "MPEG Video");
    #endif
    Essence->second.Parsers.push_back(Parser);
}

//---------------------------------------------------------------------------
void File_Mxf::ChooseParser_Raw(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    Essence->second.StreamKind=Stream_Video;

    //Filling
    File__Analyze* Parser=new File_Unknown();
    Open_Buffer_Init(Parser);
    Parser->Stream_Prepare(Stream_Video);
    Parser->Fill(Stream_Video, 0, Video_Format, "YUV");
    Essence->second.Parsers.push_back(Parser);
}

//---------------------------------------------------------------------------
void File_Mxf::ChooseParser_RV24(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    Essence->second.StreamKind=Stream_Video;

    //Filling
    File__Analyze* Parser=new File_Unknown();
    Open_Buffer_Init(Parser);
    Parser->Stream_Prepare(Stream_Video);
    Parser->Fill(Stream_Video, 0, Video_Format, "RV24");
    Essence->second.Parsers.push_back(Parser);
}

//---------------------------------------------------------------------------
void File_Mxf::ChooseParser_Vc3(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    Essence->second.StreamKind=Stream_Video;

    //Filling
    #if defined(MEDIAINFO_VC3_YES)
        File_Vc3* Parser=new File_Vc3;
        if (Descriptor!=Descriptors.end())
            Parser->FrameRate=Descriptor->second.SampleRate;
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Video);
        Parser->Fill(Stream_Video, 0, Video_Format, "VC-3");
    #endif
    Essence->second.Parsers.push_back(Parser);
}

//---------------------------------------------------------------------------
void File_Mxf::ChooseParser_Aac(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    Essence->second.StreamKind=Stream_Audio;

    //Filling
    #if defined(MEDIAINFO_AAC_YES)
        File_Aac* Parser=new File_Aac;
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Audio);
        Parser->Fill(Stream_Audio, 0, Audio_Format, "AAC");
    #endif
    Essence->second.Parsers.push_back(Parser);
}

//---------------------------------------------------------------------------
void File_Mxf::ChooseParser_Ac3(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    Essence->second.StreamKind=Stream_Audio;

    //Filling
    #if defined(MEDIAINFO_AC3_YES)
        File_Ac3* Parser=new File_Ac3;
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Audio);
        Parser->Fill(Stream_Audio, 0, Audio_Format, "AC-3");
    #endif
    Essence->second.Parsers.push_back(Parser);
}

//---------------------------------------------------------------------------
void File_Mxf::ChooseParser_Alaw(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    Essence->second.StreamKind=Stream_Audio;

    //Filling
    File__Analyze* Parser=new File_Unknown();
    Open_Buffer_Init(Parser);
    Parser->Stream_Prepare(Stream_Audio);
    Parser->Fill(Stream_Audio, 0, Audio_Format, "Alaw");
    Essence->second.Parsers.push_back(Parser);
}


//---------------------------------------------------------------------------
void File_Mxf::ChooseParser_ChannelGrouping(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    Essence->second.StreamKind=Stream_Audio;

    //Creating the parser
    if (!((Essence->second.StreamPos-(StreamPos_StartAtOne?1:0))%2 && Essences[Essence->first-1].Parsers.size()<=1))
    {
        File_ChannelGrouping* Parser;
        if ((Essence->second.StreamPos-(StreamPos_StartAtOne?1:0))%2) //If the first half-stream was already rejected, don't try this one
        {
            essences::iterator FirstChannel=Essences.find(Essence->first-1);
            if (FirstChannel==Essences.end() || !FirstChannel->second.IsChannelGrouping)
                return ChooseParser_Pcm(Essence, Descriptor); //Not a channel grouping

            Parser=new File_ChannelGrouping;
            Parser->Channel_Pos=1;
            Parser->Common=((File_ChannelGrouping*)Essences[Essence->first-1].Parsers[0])->Common;
            Parser->StreamID=Essence->second.TrackID-1;
        }
        else
        {
            Parser=new File_ChannelGrouping;
            Parser->Channel_Pos=0;
            if (Descriptor!=Descriptors.end() && Descriptor->second.Infos.find("SamplingRate")!=Descriptor->second.Infos.end())
                Parser->SamplingRate=Descriptor->second.Infos["SamplingRate"].To_int16u();
            Essence->second.IsChannelGrouping=true;
        }
        Parser->Channel_Total=2;
        if (Descriptor!=Descriptors.end())
        {
            Parser->BitDepth=(int8u)(Descriptor->second.BlockAlign<=4?(Descriptor->second.BlockAlign*8):(Descriptor->second.BlockAlign*4)); //In one file, BlockAlign is size of the aggregated channelgroup
            if (Descriptor->second.Infos.find("Format_Settings_Endianness")!=Descriptor->second.Infos.end())
            {
                if (Descriptor->second.Infos["Format_Settings_Endianness"]==__T("Big"))
                    Parser->Endianness='B';
                else
                    Parser->Endianness='L';
            }
            else
                Parser->Endianness='L';
        }
        else
            Parser->Endianness='L';

        #if MEDIAINFO_DEMUX
            if (Demux_UnpacketizeContainer)
            {
                Parser->Demux_Level=2; //Container
                Parser->Demux_UnpacketizeContainer=true;
            }
        #endif //MEDIAINFO_DEMUX

        Essence->second.Parsers.push_back(Parser);
    }

    //Adding PCM
    ChooseParser_Pcm(Essence, Descriptor);
}

//---------------------------------------------------------------------------
void File_Mxf::ChooseParser_Mpega(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    Essence->second.StreamKind=Stream_Audio;

    //Filling
    #if defined(MEDIAINFO_MPEGA_YES)
        File_Mpega* Parser=new File_Mpega;
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Audio);
        Parser->Fill(Stream_Audio, 0, Audio_Format, "MPEG Audio");
    #endif
    Essence->second.Parsers.push_back(Parser);
}

//---------------------------------------------------------------------------
void File_Mxf::ChooseParser_Pcm(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    Essence->second.StreamKind=Stream_Audio;

    //Creating the parser
    #if defined(MEDIAINFO_PCM_YES)
        File_Pcm* Parser=new File_Pcm;
        if (Descriptor!=Descriptors.end())
        {
            if (Descriptor->second.Infos.find("Channel(s)")!=Descriptor->second.Infos.end())
                Parser->Channels=Descriptor->second.Infos["Channel(s)"].To_int8u();
            if (Parser->Channels && Descriptor->second.BlockAlign!=(int16u)-1)
                Parser->BitDepth=(int8u)(Descriptor->second.BlockAlign*8/Parser->Channels);
            else if (Descriptor->second.QuantizationBits<256)
                Parser->BitDepth=(int8u)Descriptor->second.QuantizationBits;
            else if (Descriptor->second.Infos.find("BitDepth")!=Descriptor->second.Infos.end())
                Parser->BitDepth=Descriptor->second.Infos["BitDepth"].To_int8u();
            if (Descriptor->second.Infos.find("Format_Settings_Endianness")!=Descriptor->second.Infos.end())
            {
                if (Descriptor->second.Infos["Format_Settings_Endianness"]==__T("Big"))
                    Parser->Endianness='B';
                else
                    Parser->Endianness='L';
            }
            else
                Parser->Endianness='L';
        }
        else
            Parser->Endianness='L';

        #if MEDIAINFO_DEMUX
            if (Demux_UnpacketizeContainer)
            {
                Parser->Demux_Level=2; //Container
                Parser->Demux_UnpacketizeContainer=true;
            }
        #endif //MEDIAINFO_DEMUX

        Essence->second.Parsers.push_back(Parser);
    #endif
}

//---------------------------------------------------------------------------
void File_Mxf::ChooseParser_SmpteSt0331(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    Essence->second.StreamKind=Stream_Audio;

    //Filling
    #if defined(MEDIAINFO_SMPTEST0331_YES)
        File_SmpteSt0331* Parser=new File_SmpteSt0331;
        if (Descriptor!=Descriptors.end() && Descriptor->second.QuantizationBits!=(int32u)-1)
            Parser->QuantizationBits=Descriptor->second.QuantizationBits;

        #if MEDIAINFO_DEMUX
            if (Demux_UnpacketizeContainer)
            {
                Parser->Demux_Level=2; //Container
                Parser->Demux_UnpacketizeContainer=true;
            }
        #endif //MEDIAINFO_DEMUX

        Essence->second.Parsers.push_back(Parser);
    #endif
}

//---------------------------------------------------------------------------
void File_Mxf::ChooseParser_SmpteSt0337(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    Essence->second.StreamKind=Stream_Audio;

    //Filling
    #if defined(MEDIAINFO_SMPTEST0337_YES)
        File_SmpteSt0337* Parser=new File_SmpteSt0337;
        if (Descriptor!=Descriptors.end())
        {
            if (Descriptor->second.BlockAlign<64)
                Parser->Container_Bits=(int8u)(Descriptor->second.BlockAlign*4);
            else if (Descriptor->second.QuantizationBits!=(int32u)-1)
                Parser->Container_Bits=(int8u)Descriptor->second.QuantizationBits;
            if (Descriptor->second.Infos.find("Format_Settings_Endianness")!=Descriptor->second.Infos.end())
            {
                if (Descriptor->second.Infos["Format_Settings_Endianness"]==__T("Big"))
                    Parser->Endianness='B';
                else
                    Parser->Endianness='L';
            }
            else
                Parser->Endianness='L';
        }
        else
            Parser->Endianness='L';
        Parser->Aligned=true;

        #if MEDIAINFO_DEMUX
            if (Demux_UnpacketizeContainer)
            {
                Parser->Demux_Level=2; //Container
                Parser->Demux_UnpacketizeContainer=true;
            }
        #endif //MEDIAINFO_DEMUX

        Essence->second.Parsers.push_back(Parser);
    #endif
}

//---------------------------------------------------------------------------
void File_Mxf::ChooseParser_Jpeg2000(const essences::iterator &Essence, const descriptors::iterator &Descriptor)
{
    Essence->second.StreamKind=Stream_Video;

    //Filling
    #if defined(MEDIAINFO_JPEG_YES)
        File_Jpeg* Parser=new File_Jpeg;
        Parser->StreamKind=Stream_Video;
        if (Descriptor!=Descriptors.end())
        {
            Parser->Interlaced=Descriptor->second.ScanType==__T("Interlaced");
            #if MEDIAINFO_DEMUX
                if (Parser->Interlaced)
                {
                    Parser->Demux_Level=2; //Container
                    Parser->Demux_UnpacketizeContainer=true;
                }
            #endif //MEDIAINFO_DEMUX
        }
    #else
        //Filling
        File__Analyze* Parser=new File_Unknown();
        Open_Buffer_Init(Parser);
        Parser->Stream_Prepare(Stream_Video);
        Parser->Fill(Stream_Video, 0, Video_Format, "JPEG 2000");
    #endif
    Essence->second.Parsers.push_back(Parser);
}

//***************************************************************************
// Helpers
//***************************************************************************

//---------------------------------------------------------------------------
void File_Mxf::Subsampling_Compute(descriptors::iterator Descriptor)
{
    if (Descriptor!=Descriptors.end() && (Descriptor->second.SubSampling_Horizontal==(int32u)-1 || Descriptor->second.SubSampling_Vertical==(int32u)-1))
        return;

    switch (Descriptor->second.SubSampling_Horizontal)
    {
        case 1 :    switch (Descriptor->second.SubSampling_Vertical)
                    {
                        case 1 : Descriptor->second.Infos["ChromaSubsampling"]=__T("4:4:4"); return;
                        default: Descriptor->second.Infos["ChromaSubsampling"].clear(); return;
                    }
        case 2 :    switch (Descriptor->second.SubSampling_Vertical)
                    {
                        case 1 : Descriptor->second.Infos["ChromaSubsampling"]=__T("4:2:2"); return;
                        case 2 : Descriptor->second.Infos["ChromaSubsampling"]=__T("4:2:0"); return;
                        default: Descriptor->second.Infos["ChromaSubsampling"].clear(); return;
                    }
        case 4 :    switch (Descriptor->second.SubSampling_Vertical)
                    {
                        case 1 : Descriptor->second.Infos["ChromaSubsampling"]=__T("4:1:1"); return;
                        default: Descriptor->second.Infos["ChromaSubsampling"].clear(); return;
                    }
        default:    return;
    }
}

//---------------------------------------------------------------------------
void File_Mxf::Locators_CleanUp()
{
    //Testing locators (TODO: check if this is still useful)
    if (Locators.size()==1)
    {
        Locators.clear();
        return;
    }

    locators::iterator Locator=Locators.begin();
    while (Locator!=Locators.end())
    {
        bool IsReferenced=false;
        for (descriptors::iterator Descriptor=Descriptors.begin(); Descriptor!=Descriptors.end(); ++Descriptor)
            for (size_t Pos=0; Pos<Descriptor->second.Locators.size(); Pos++)
                if (Locator->first==Descriptor->second.Locators[Pos])
                    IsReferenced=true;
        if (!IsReferenced)
        {
            //Deleting current locator
            locators::iterator LocatorToDelete=Locator;
            ++Locator;
            Locators.erase(LocatorToDelete);
        }
        else
            ++Locator;
    }

}

//---------------------------------------------------------------------------
void File_Mxf::Locators_Test()
{
    Locators_CleanUp();

    if (!Locators.empty() && ReferenceFiles==NULL)
    {
        ReferenceFiles=new File__ReferenceFilesHelper(this, Config);

        for (locators::iterator Locator=Locators.begin(); Locator!=Locators.end(); ++Locator)
            if (!Locator->second.IsTextLocator && !Locator->second.EssenceLocator.empty())
            {
                File__ReferenceFilesHelper::reference ReferenceFile;
                ReferenceFile.FileNames.push_back(Locator->second.EssenceLocator);
                ReferenceFile.StreamKind=Locator->second.StreamKind;
                ReferenceFile.StreamPos=Locator->second.StreamPos;
                if (Locator->second.LinkedTrackID!=(int32u)-1)
                    ReferenceFile.StreamID=Locator->second.LinkedTrackID;
                else if (!Retrieve(Locator->second.StreamKind, Locator->second.StreamPos, General_ID).empty())
                    ReferenceFile.StreamID=Retrieve(Locator->second.StreamKind, Locator->second.StreamPos, General_ID).To_int64u();
                ReferenceFile.Delay=float64_int64s(DTS_Delay*1000000000);

                //Special cases
                if (Locator->second.StreamKind==Stream_Video)
                {
                    //Searching the corresponding frame rate
                    for (descriptors::iterator Descriptor=Descriptors.begin(); Descriptor!=Descriptors.end(); ++Descriptor)
                        for (size_t LocatorPos=0; LocatorPos<Descriptor->second.Locators.size(); LocatorPos++)
                            if (Descriptor->second.Locators[LocatorPos]==Locator->first)
                                ReferenceFile.FrameRate=Descriptor->second.SampleRate;
                }


                if (ReferenceFile.StreamID!=(int32u)-1)
                {
                    //Descriptive Metadata
                    std::vector<int128u> DMScheme1s_List;

                    for (dmsegments::iterator DMSegment=DMSegments.begin(); DMSegment!=DMSegments.end(); ++DMSegment)
                        for (size_t Pos=0; Pos<DMSegment->second.TrackIDs.size(); Pos++)
                            if (DMSegment->second.TrackIDs[Pos]==ReferenceFile.StreamID)
                                DMScheme1s_List.push_back(DMSegment->second.Framework);

                    for (size_t Pos=0; Pos<DMScheme1s_List.size(); Pos++)
                    {
                        dmscheme1s::iterator DMScheme1=DMScheme1s.find(DMScheme1s_List[Pos]);
                        if (DMScheme1!=DMScheme1s.end())
                        {
                            ReferenceFile.Infos["Language"]=DMScheme1->second.PrimaryExtendedSpokenLanguage;
                        }
                    }
                }

                ReferenceFiles->References.push_back(ReferenceFile);
            }
            else
            {
                Fill(Stream_General, 0, "UnsupportedSources", Locator->second.EssenceLocator);
                (*Stream_More)[Stream_General][0](Ztring().From_Local("UnsupportedSources"), Info_Options)=__T("N NT");
            }

        ReferenceFiles->ParseReferences();
    }
}

//---------------------------------------------------------------------------
void File_Mxf::TryToFinish()
{
    Frame_Count_NotParsedIncluded=(int64u)-1;

    if (!IsSub && IsParsingEnd && File_Size!=(int64u)-1 && Config->ParseSpeed<1 && IsParsingMiddle_MaxOffset==(int64u)-1 && File_Size/2>0x4000000) //TODO: 64 MB by default;
    {
        IsParsingMiddle_MaxOffset=File_Size/2+0x4000000; //TODO: 64 MB by default;
        GoTo(File_Size/2);
        Open_Buffer_Unsynch();
        IsParsingEnd=false;
        Streams_Count=(size_t)-1;
        return;
    }
}

} //NameSpace

#endif //MEDIAINFO_MXF_*
