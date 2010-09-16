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
//
// Information about Mxf files
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
#ifndef MediaInfo_File_MxfH
#define MediaInfo_File_MxfH
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/File__Analyze.h"
#if defined(MEDIAINFO_ANCILLARY_YES)
    #include <MediaInfo/Multiple/File_Ancillary.h>
#endif //defined(MEDIAINFO_ANCILLARY_YES)
#include <vector>
//---------------------------------------------------------------------------

namespace MediaInfoLib
{

//***************************************************************************
// Class File_Mxf
//***************************************************************************

class File_Mxf : public File__Analyze
{
public :
    //Constructor/Destructor
    File_Mxf();
    ~File_Mxf();

    //int256u
    class int256u
    {
        public:
            // Binary correct representation of signed 256bit integer
            int128u lo;
            int128u hi;

            int256u()
            {
                lo.lo=0;
                lo.hi=0;
                hi.lo=0;
                hi.lo=0;
            }
    };


protected :
    //Streams management
    void Streams_Finish ();
    void Streams_Finish_Preface (int128u PrefaceUID);
    void Streams_Finish_ContentStorage (int128u ContentStorageUID);
    void Streams_Finish_Package (int128u PackageUID);
    void Streams_Finish_Track (int128u TrackUID);
    void Streams_Finish_Essence (int32u EssenceUID, int128u TrackUID);
    void Streams_Finish_Descriptor (int128u DescriptorUID, int128u PackageUID);
    void Streams_Finish_Locator (int128u LocatorUID);
    void Streams_Finish_Component (int128u ComponentUID, float64 EditRate);
    void Streams_Finish_Identification (int128u IdentificationUID);

    //Buffer - Global
    void Read_Buffer_Continue ();
    void Read_Buffer_Unsynched();

    //Buffer - File header
    bool FileHeader_Begin();

    //Buffer - Synchro
    bool Synchronize();
    bool Synched_Test();

    //Buffer - Per element
    void Header_Parse();
    void Data_Parse();

    //Elements
    void AES3PCMDescriptor();
    void CDCIEssenceDescriptor();
    void OpenIncompleteHeaderPartition();
    void ClosedIncompleteHeaderPartition();
    void OpenCompleteHeaderPartition();
    void ClosedCompleteHeaderPartition();
    void OpenIncompleteBodyPartition();
    void ClosedIncompleteBodyPartition();
    void OpenCompleteBodyPartition();
    void ClosedCompleteBodyPartition();
    void OpenIncompleteFooterPartition();
    void ClosedIncompleteFooterPartition();
    void OpenCompleteFooterPartition();
    void ClosedCompleteFooterPartition();
    void ContentStorage();
    void DMSegment();
    void EssenceContainerData();
    void EventTrack();
    void FileDescriptor();
    void Identification();
    void IndexTableSegment();
    void InterchangeObject();
    void JPEG2000PictureSubDescriptor();
    void GenerationInterchangeObject();
    void GenericDescriptor();
    void GenericPackage();
    void GenericPictureEssenceDescriptor();
    void GenericSoundEssenceDescriptor();
    void GenericTrack();
    void MaterialPackage();
    void MPEG2VideoDescriptor();
    void MultipleDescriptor();
    void NetworkLocator();
    void PartitionMetadata();
    void Padding();
    void Preface();
    void Primer();
    void RGBAEssenceDescriptor();
    void RandomIndexMetadata();
    void Sequence();
    void SourceClip();
    void SourcePackage();
    void StaticTrack();
    void StructuralComponent();
    void SystemScheme1();
    void TextLocator();
    void TimecodeComponent();
    void Track();
    void WaveAudioDescriptor();
    void AncPacketsDescriptor();
    void Filler();
    void TerminatingFiller();
    void XmlDocumentText();
    void SDTI_SystemMetadataPack();
    void SDTI_PackageMetadataSet();
    void SDTI_PictureMetadataSet();
    void SDTI_SoundMetadataSet();
    void SDTI_DataMetadataSet();
    void SDTI_ControlMetadataSet();
    void Omneon_010201010100();
    void Omneon_010201020100();

    //Complex types
    void AES3PCMDescriptor_AuxBitsMode();                       //3D08
    void AES3PCMDescriptor_Emphasis();                          //3D0D
    void AES3PCMDescriptor_BlockStartOffset();                  //3D0F
    void AES3PCMDescriptor_ChannelStatusMode();                 //3D10
    void AES3PCMDescriptor_FixedChannelStatusData();            //3D11
    void AES3PCMDescriptor_UserDataMode();                      //3D12
    void AES3PCMDescriptor_FixedUserData();                     //3D13
    void CDCIEssenceDescriptor_ComponentDepth();                //3301
    void CDCIEssenceDescriptor_HorizontalSubsampling();         //3302
    void CDCIEssenceDescriptor_ColorSiting();                   //3303
    void CDCIEssenceDescriptor_BlackRefLevel();                 //3304
    void CDCIEssenceDescriptor_WhiteReflevel();                 //3305
    void CDCIEssenceDescriptor_ColorRange();                    //3306
    void CDCIEssenceDescriptor_PaddingBits();                   //3307
    void CDCIEssenceDescriptor_VerticalSubsampling();           //3308
    void CDCIEssenceDescriptor_AlphaSampleDepth();              //3309
    void CDCIEssenceDescriptor_ReversedByteOrder();             //330B
    void ContentStorage_Packages();                             //1901
    void ContentStorage_EssenceContainerData();                 //1902
    void DMSegment_DMFramework();                               //6101
    void EssenceContainerData_LinkedPackageUID();               //2701
    void EssenceContainerData_IndexSID();                       //3F06
    void EssenceContainerData_BodySID();                        //3F07
    void EventTrack_EventEditRate();                            //4901
    void EventTrack_EventOrigin();                              //4902
    void FileDescriptor_SampleRate();                           //3001
    void FileDescriptor_ContainerDuration();                    //3002
    void FileDescriptor_EssenceContainer();                     //3004
    void FileDescriptor_Codec();                                //3005
    void FileDescriptor_LinkedTrackID();                        //3006
    void InterchangeObject_InstanceUID();                       //3C0A
    void GenerationInterchangeObject_GenerationUID();           //0102
    void GenericDescriptor_Locators();                          //2F01
    void GenericPackage_PackageUID();                           //4401
    void GenericPackage_Name();                                 //4402
    void GenericPackage_Tracks();                               //4403
    void GenericPackage_PackageModifiedDate();                  //4404
    void GenericPackage_PackageCreationDate();                  //4405
    void GenericPictureEssenceDescriptor_PictureEssenceCoding();//3201
    void GenericPictureEssenceDescriptor_StoredHeight();        //3202
    void GenericPictureEssenceDescriptor_StoredWidth();         //3203
    void GenericPictureEssenceDescriptor_SampledHeight();       //3204
    void GenericPictureEssenceDescriptor_SampledWidth();        //3205
    void GenericPictureEssenceDescriptor_SampledXOffset();      //3206
    void GenericPictureEssenceDescriptor_SampledYOffset();      //3207
    void GenericPictureEssenceDescriptor_DisplayHeight();       //3208
    void GenericPictureEssenceDescriptor_DisplayWidth();        //3209
    void GenericPictureEssenceDescriptor_DisplayXOffset();      //320A
    void GenericPictureEssenceDescriptor_DisplayYOffset();      //320B
    void GenericPictureEssenceDescriptor_FrameLayout();         //320C
    void GenericPictureEssenceDescriptor_VideoLineMap();        //320D
    void GenericPictureEssenceDescriptor_AspectRatio();         //320E
    void GenericPictureEssenceDescriptor_AlphaTransparency();   //320F
    void GenericPictureEssenceDescriptor_Gamma();               //3210
    void GenericPictureEssenceDescriptor_ImageAlignmentOffset();//3211
    void GenericPictureEssenceDescriptor_FieldDominance();      //3212
    void GenericPictureEssenceDescriptor_ImageStartOffset();    //3213
    void GenericPictureEssenceDescriptor_ImageEndOffset();      //3214
    void GenericPictureEssenceDescriptor_SignalStandard();      //3215
    void GenericPictureEssenceDescriptor_StoredF2Offset();      //3216
    void GenericPictureEssenceDescriptor_DisplayF2Offset();     //3217
    void GenericPictureEssenceDescriptor_ActiveFormatDescriptor();//3218
    void GenericSoundEssenceDescriptor_QuantizationBits();      //3D01
    void GenericSoundEssenceDescriptor_Locked();                //3D02
    void GenericSoundEssenceDescriptor_AudioSamplingRate();     //3D03
    void GenericSoundEssenceDescriptor_AudioRefLevel();         //3D04
    void GenericSoundEssenceDescriptor_ElectroSpatialFormulation(); //3D05
    void GenericSoundEssenceDescriptor_SoundEssenceCompression(); //3D06
    void GenericSoundEssenceDescriptor_ChannelCount();          //3D07
    void GenericSoundEssenceDescriptor_DialNorm();              //3D0C
    void GenericTrack_TrackID();                                //4801
    void GenericTrack_TrackName();                              //4802
    void GenericTrack_Sequence();                               //4803
    void GenericTrack_TrackNumber();                            //4804
    void Identification_CompanyName();                          //3C01
    void Identification_ProductName();                          //3C02
    void Identification_ProductVersion();                       //3C03
    void Identification_VersionString();                        //3C04
    void Identification_ProductUID();                           //3C05
    void Identification_ModificationDate();                     //3C06
    void Identification_ToolkitVersion();                       //3C07
    void Identification_Platform();                             //3C08
    void Identification_ThisGenerationUID();                    //3C09
    void IndexTableSegment_EditUnitByteCount();                 //3F05
    void IndexTableSegment_IndexSID();                          //3F06
    void IndexTableSegment_BodySID();                           //3F07
    void IndexTableSegment_SliceCount();                        //3F08
    void IndexTableSegment_DeltaEntryArray();                   //3F09
    void IndexTableSegment_IndexEntryArray();                   //3F0A
    void IndexTableSegment_IndexEditRate();                     //3F0B
    void IndexTableSegment_IndexStartPosition();                //3F0C
    void IndexTableSegment_IndexDuration();                     //3F0D
    void IndexTableSegment_PosTableCount();                     //3F0E
    void JPEG2000PictureSubDescriptor_Rsiz();                   //8001
    void JPEG2000PictureSubDescriptor_Xsiz();                   //8002
    void JPEG2000PictureSubDescriptor_Ysiz();                   //8003
    void JPEG2000PictureSubDescriptor_XOsiz();                  //8004
    void JPEG2000PictureSubDescriptor_YOsiz();                  //8005
    void JPEG2000PictureSubDescriptor_XTsiz();                  //8006
    void JPEG2000PictureSubDescriptor_YTsiz();                  //8007
    void JPEG2000PictureSubDescriptor_XTOsiz();                 //8008
    void JPEG2000PictureSubDescriptor_YTOsiz();                 //8009
    void JPEG2000PictureSubDescriptor_Csiz();                   //800A
    void JPEG2000PictureSubDescriptor_PictureComponentSizing(); //800B
    void MultipleDescriptor_SubDescriptorUIDs();                //3F01
    void MPEG2VideoDescriptor_SingleSequence();                 //
    void MPEG2VideoDescriptor_ConstantBFrames();                //
    void MPEG2VideoDescriptor_CodedContentType();               //
    void MPEG2VideoDescriptor_LowDelay();                       //
    void MPEG2VideoDescriptor_ClosedGOP();                      //
    void MPEG2VideoDescriptor_IdenticalGOP();                   //
    void MPEG2VideoDescriptor_MaxGOP();                         //
    void MPEG2VideoDescriptor_BPictureCount();                  //
    void MPEG2VideoDescriptor_ProfileAndLevel();                //
    void MPEG2VideoDescriptor_BitRate();                        //
    void NetworkLocator_URLString();                            //4001
    void Preface_LastModifiedDate();                            //3B02
    void Preface_ContentStorage();                              //3B03
    void Preface_Version();                                     //3B05
    void Preface_Identifications();                             //3B06
    void Preface_ObjectModelVersion();                          //3B07
    void Preface_PrimaryPackage();                              //3B08
    void Preface_OperationalPattern();                          //3B09
    void Preface_EssenceContainers();                           //3B0A
    void Preface_DMSchemes();                                   //3B0B
    void RGBAEssenceDescriptor_PixelLayout();                   //3401
    void RGBAEssenceDescriptor_Palette();                       //3403
    void RGBAEssenceDescriptor_PaletteLayout();                 //3404
    void RGBAEssenceDescriptor_ScanningDirection();             //3405
    void RGBAEssenceDescriptor_ComponentMaxRef();               //3406
    void RGBAEssenceDescriptor_ComponentMinRef();               //3407
    void RGBAEssenceDescriptor_AlphaMaxRef();                   //3408
    void RGBAEssenceDescriptor_AlphaMinRef();                   //3409
    void Sequence_StructuralComponents();                       //1001
    void SourceClip_SourcePackageID();                          //1101
    void SourceClip_SourceTrackID();                            //1102
    void SourceClip_StartPosition();                            //1201
    void SourcePackage_Descriptor();                            //4701
    void StructuralComponent_DataDefinition();                  //0201
    void StructuralComponent_Duration();                        //0202
    void SystemScheme1_TimeCodeArray();                         //0102
    void TextLocator_LocatorName();                             //4101
    void TimecodeComponent_StartTimecode();                     //1501
    void TimecodeComponent_RoundedTimecodeBase();               //1502
    void TimecodeComponent_DropFrame();                         //1503
    void Track_EditRate();                                      //4B01
    void Track_Origin();                                        //4B02
    void WaveAudioDescriptor_AvgBps();                          //3D09
    void WaveAudioDescriptor_BlockAlign();                      //3D0A
    void WaveAudioDescriptor_SequenceOffset();                  //3D0B
    void WaveAudioDescriptor_PeakEnvelopeVersion();             //3D29
    void WaveAudioDescriptor_PeakEnvelopeFormat();              //3D2A
    void WaveAudioDescriptor_PointsPerPeakValue();              //3D2B
    void WaveAudioDescriptor_PeakEnvelopeBlockSize();           //3D2C
    void WaveAudioDescriptor_PeakChannels();                    //3D2D
    void WaveAudioDescriptor_PeakFrames();                      //3D2E
    void WaveAudioDescriptor_PeakOfPeaksPosition();             //3D2F
    void WaveAudioDescriptor_PeakEnvelopeTimestamp();           //3D30
    void WaveAudioDescriptor_PeakEnvelopeData();                //3D31
    void WaveAudioDescriptor_ChannelAssignment();               //3D31
    void Omneon_010201010100_8001();                            //8001
    void Omneon_010201010100_8003();                            //8003
    void Omneon_010201020100_8002();                            //8002
    void Omneon_010201020100_8003();                            //8003
    void Omneon_010201020100_8004();                            //8004
    void Omneon_010201020100_8005();                            //8005
    void Omneon_010201020100_8006();                            //8006

    //Basic types
    void Get_Rational(float64 &Value);
    void Skip_Rational();
    void Info_Rational();
    void Get_Timestamp (Ztring &Value);
    void Skip_Timestamp();
    void Info_Timestamp();
    void Get_UMID       (int256u &Value, const char* Name);
    void Skip_UMID      ();

    void Get_UL (int128u &Value, const char* Name, const char* (*Param) (int128u));
    void Info_UL_01xx01_Essences ();
    void Info_UL_02xx01 ();
    void Info_UL_040101_Labels ();
    void Skip_UL(const char* Name);
    #define Info_UL(_INFO, _NAME, _PARAM) int128u _INFO; Get_UL(_INFO, _NAME, _PARAM)

    struct randomindexmetadata
    {
        int64u ByteOffset;
        int32u BodySID;
    };
    std::vector<randomindexmetadata> RandomIndexMetadatas;
    size_t Streams_Count;
    int128u Code;
    int128u OperationalPattern;
    int128u InstanceUID;
    int64u Buffer_DataSizeToParse;
    int64u Buffer_DataSizeToParse_Complete;
    int16u Code2;
    int16u Length2;
    int64u File_Size_Total; //Used only in Finish()
    bool   Track_Number_IsAvailable;
    bool   IsParsingEnd;

    //Primer
    std::map<int16u, int128u> Primer_Values;

    //Preface
    struct preface
    {
        int128u PrimaryPackage;
        std::vector<int128u> Identifications;
        int128u ContentStorage;

        preface()
        {
            PrimaryPackage.hi=(int64u)-1;
            PrimaryPackage.lo=(int64u)-1;
            ContentStorage.hi=(int64u)-1;
            ContentStorage.lo=(int64u)-1;
        }
    };
    typedef std::map<int128u, preface> prefaces; //Key is InstanceUID of preface
    prefaces Prefaces;
    int128u  Preface_Current;

    //Identification
    struct identification
    {
        Ztring CompanyName;
        Ztring ProductName;
        Ztring ProductVersion;
        Ztring VersionString;
        std::map<std::string, Ztring> Infos;
    };
    typedef std::map<int128u, identification> identifications; //Key is InstanceUID of identification
    identifications Identifications;

    //ContentStorage
    struct contentstorage
    {
        std::vector<int128u> Packages;
    };
    typedef std::map<int128u, contentstorage> contentstorages; //Key is InstanceUID of ContentStorage
    contentstorages ContentStorages;

    //Package
    struct package
    {
        int256u PackageUID;
        int128u Descriptor;
        std::vector<int128u> Tracks;
        bool IsSourcePackage;

        package()
        {
            Descriptor=0;
            IsSourcePackage=false;
        }
    };
    typedef std::map<int128u, package> packages; //Key is InstanceUID of package
    packages Packages;

    //Track
    struct track
    {
        int128u Sequence;
        int32u TrackID;
        int32u TrackNumber;
        float64 EditRate;
        bool   Stream_Finish_Done;

        track()
        {
            Sequence=0;
            TrackID=(int32u)-1;
            TrackNumber=(int32u)-1;
            EditRate=(float64)0;
            Stream_Finish_Done=false;
        }
    };
    typedef std::map<int128u, track> tracks; //Key is InstanceUID of the track
    tracks Tracks;

    //Essence
    struct essence
    {
        stream_t StreamKind;
        size_t   StreamPos;
        File__Analyze* Parser;
        std::map<std::string, Ztring> Infos;
        int64u Stream_Size;
        int32u TrackID;
        bool   TrackID_WasLookedFor;
        bool   Stream_Finish_Done;
        bool   Track_Number_IsMappedToTrack; //if !Track_Number_IsAvailable, is true when it was euristicly mapped
        bool   IsFilled;

        essence()
        {
            StreamKind=Stream_Max;
            StreamPos=(size_t)-1;
            Parser=NULL;
            Stream_Size=(int64u)-1;
            TrackID=(int32u)-1;
            TrackID_WasLookedFor=false;
            Stream_Finish_Done=false;
            Track_Number_IsMappedToTrack=false;
            IsFilled=false;
        }

        ~essence()
        {
            delete Parser; //Parser=NULL;
        }
    };
    typedef std::map<int32u, essence> essences; //Key is TrackNumber
    essences Essences;

    //Descriptor
    struct descriptor
    {
        std::vector<int128u> SubDescriptors;
        std::vector<int128u> Locators;

        stream_t StreamKind;
        float64 SampleRate;
        int128u InstanceUID;
        int32u LinkedTrackID;
        int32u Width;
        int32u Height;
        std::map<std::string, Ztring> Infos;

        descriptor()
        {
            StreamKind=Stream_Max;
            SampleRate=0;
            InstanceUID.hi=(int64u)-1;
            InstanceUID.lo=(int64u)-1;
            LinkedTrackID=(int32u)-1;
            Width=(int32u)-1;
            Height=(int32u)-1;
        }
    };
    typedef std::map<int128u, descriptor> descriptors; //Key is InstanceUID of Descriptor
    descriptors Descriptors;

    //Locator
    struct locator
    {
        Ztring EssenceLocator;
        bool   IsTextLocator;

        locator()
        {
            IsTextLocator=false;
        }
    };
    typedef std::map<int128u, locator> locators; //Key is InstanceUID of the locator
    locators Locators;

    //Component (Sequence, TimeCode, Source Clip)
    struct component
    {
        int64u Duration;
        int256u SourcePackageID; //Sequence from SourcePackage only
        int32u  SourceTrackID;
        std::vector<int128u> StructuralComponents; //Sequence from MaterialPackage only

        component()
        {
            Duration=(int64u)-1;
            SourceTrackID=0;
        }
    };
    typedef std::map<int128u, component> components; //Key is InstanceUID of the component
    components Components;

    int64u TimeCode_StartTimecode;
    int16u TimeCode_RoundedTimecodeBase;
    bool   TimeCode_DropFrame;
    int64u SDTI_TimeCode_StartTimecode;
    int64u SystemScheme1_TimeCodeArray_StartTimecode;
    int64u SystemScheme1_FrameRateFromDescriptor;
    int32u IndexTable_NSL;
    int32u IndexTable_NPE;
    #if defined(MEDIAINFO_ANCILLARY_YES)
        int128u         Ancillary_InstanceUID;
        int32u          Ancillary_LinkedTrackID;
        int32u          Ancillary_TrackNumber;
        File_Ancillary* Ancillary;
    #endif //defined(MEDIAINFO_ANCILLARY_YES)
};

} //NameSpace

#endif
