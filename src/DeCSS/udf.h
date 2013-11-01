/*************************************************************************
  vStrip by [maven] (maven@maven.de)
*************************************************************************/

#pragma once

typedef char dstring;   // last BYTE of string indicates encoding/length

#define fio_SECTOR_SIZE                     2048

#define udf_LengthMask                      0x3fffffff

#define udf_TAG_PrimaryVolumeDescriptor     0x0001
#define udf_TAG_AnchorVolumeDescriptor      0x0002
#define udf_TAG_PartitionDescriptor         0x0005
#define udf_TAG_LogicalVolumeDescriptor     0x0006
#define udf_TAG_TerminatingDescriptor       0x0008
#define udf_TAG_FileSetDescriptor           0x0100
#define udf_TAG_FileIdentifierDescriptor    0x0101
#define udf_TAG_IndirectEntry               0x0103
#define udf_TAG_TerminalEntry               0x0104
#define udf_TAG_FileEntry                   0x0105

#define udf_FT_IndirectEntry                0x03
#define udf_FT_Directory                    0x04
#define udf_FT_File                         0x05
#define udf_FT_TerminalEntry                0x0b

#define udf_icbf_Mask                       0x0007
#define udf_icbf_ShortAd                    0x0000
#define udf_icbf_LongAd                     0x0001
#define udf_icbf_ExtAd                      0x0002
#define udf_icbf_Direct                     0x0003
#define udf_icbf_Contiguous                 0x0100

#define udf_FID_Directory                   0x02
#define udf_FID_Parent                      0x08

#pragma pack(push, 1)

typedef struct {
    DWORD   Length;                         // 00, high 2 bits: 0 ^= recorded & used, 1 ^= not recorded & used, 2 ^= not recorded & not used, 3 ^= linked list
    DWORD   Location;                       // 04
} t_udf_short_ad, *tp_udf_short_ad;         // 08

typedef struct {
    DWORD   Length;                         // 00
    DWORD   Location;                       // 04
} t_udf_extent_ad, *tp_udf_extent_ad;       // 08

typedef struct {
    DWORD   Location;                       // 00, relative to volume
    WORD    PartitionNumber;                // 04
} t_udf_lb_addr;                            // 06

typedef struct {
    DWORD           Length;                 // 00, high 2 bits: 0 ^= recorded & used, 1 ^= not recorded & used, 2 ^= not recorded & not used, 3 ^= linked list
    t_udf_lb_addr   Location;               // 04
    BYTE            ImplementationUse[6];   // 10
} t_udf_long_ad, *tp_udf_long_ad;           // 16

typedef struct {
    DWORD           Length;                 // 00, high 2 bits: 0 ^= recorded & used, 1 ^= not recorded & used, 2 ^= not recorded & not used, 3 ^= linked list
    DWORD           RecordedLength;         // 04
    DWORD           InformationLength;      // 08
    t_udf_lb_addr   Location;               // 12
    BYTE            ImplementationUse[2];   // 18
} t_udf_ext_ad, *tp_udf_ext_ad;             // 20

typedef struct {
    BYTE    CharacterSetType;               // 00
    BYTE    CharacterSetInfo[63];           // 01
} t_udf_charspec;                           // 64

typedef struct {
    /* ECMA 167 1/7.3 */
    WORD    TypeAndTimezone;                // 00
    WORD    Year;                           // 02
    BYTE    Month;                          // 04
    BYTE    Day;                            // 05
    BYTE    Hour;                           // 06
    BYTE    Minute;                         // 07
    BYTE    Second;                         // 08
    BYTE    Centiseconds;                   // 09
    BYTE    HundredsofMicroseconds;         // 10
    BYTE    Microseconds;                   // 11
} t_udf_timestamp;                          // 12

typedef struct {
    /* ISO 13346 3/7.2 */
    WORD    TagIdentifier;                  // 00
    WORD    DescriptorVersion;              // 02
    BYTE    TagChecksum;                    // 04
    BYTE    Reserved;                       // 05
    WORD    TagSerialNumber;                // 06
    WORD    DescriptorCRC;                  // 08
    WORD    DescriptorCRCLength;            // 10
    DWORD   TagLocation;                    // 12
} t_udf_tag, *tp_udf_tag;                   // 16

typedef struct {
    /* ISO 13346 1/7.4 */
    BYTE    Flags;                          // 00
    char    Identifier[23];                 // 01
    char    IdentifierSuffix[8];            // 24
} t_udf_EntityID;                           // 32

typedef struct {
    /* ISO 13346 3/10.2 */
    t_udf_tag       DescriptorTag;          // 00
    t_udf_extent_ad MainVolumeDescriptorSequenceExtent;     // 16
    t_udf_extent_ad ReserveVolumeDescriptorSequenceExtent;  // 24
    BYTE            Reserved[480];          // 32
} t_udf_AnchorVolumeDescriptorPointer, *tp_udf_AnchorVolumeDescriptorPointer; // 512

typedef struct {
    /* ISO 13346 3/10.6 */
    t_udf_tag       DescriptorTag;                  // 00
    DWORD           VolumeDescriptorSequenceNumber; // 16
    t_udf_charspec  DescriptorCharacterSet;         // 20
    dstring         LogicalVolumeIdentifier[128];   // 84
    DWORD           LogicalBlockSize;               // 212
    t_udf_EntityID  DomainIdentifier;               // 244
    //BYTE            LogicalVolumeContentsUse[16];   // 276
    t_udf_long_ad   FileSetDescriptorSequence;      // 276
    DWORD           MapTableLength;                 // 292
    DWORD           NumberofPartitionMaps;          // 296
    t_udf_EntityID  ImplementationIdentifier;       // 300
    BYTE            ImplementationUse[128];         // 332
    t_udf_extent_ad IntegritySequenceExtent;        // 460
    BYTE            PartitionMaps[1];               // 468
} t_udf_LogicalVolumeDescriptor, *tp_udf_LogicalVolumeDescriptor;

typedef struct {
    t_udf_short_ad  UnallocatedSpaceTable;      // 00
    t_udf_short_ad  UnallocatedSpaceBitmap;     // 08
    t_udf_short_ad  PartitionIntegrityTable;    // 16
    t_udf_short_ad  FreedSpaceTable;            // 24
    t_udf_short_ad  FreedSpaceBitmap;           // 32
    BYTE            Reserved[88];               // 40
} t_udf_PartitionHeaderDescriptor;              // 128

typedef struct {
    /* ECMA 167 3/10.5  */
    t_udf_tag       DescriptorTag;                  // 00
    DWORD           VolumeDescriptorSequenceNumber; // 16
    WORD            PartitionFlags;                 // 20
    WORD            PartitionNumber;                // 22
    t_udf_EntityID  PartitionContents;              // 24
    t_udf_PartitionHeaderDescriptor PartitionHeaderDescriptor; // 56
    DWORD           AccessType;                     // 184, 0 unspecified, 1 read only, 2 write once, 3 rewriteable, 4 overwriteable
    DWORD           PartitionStartingLocation;      // 188
    DWORD           PartitionLength;                // 192
    t_udf_EntityID  ImplementationIdentifier;       // 196
    BYTE            ImplementationUse[128];         // 228
    BYTE            Reserved[156];                  // 356
} t_udf_PartitionDescriptor, *tp_udf_PartitionDescriptor; // 512

typedef struct {
    /* ECMA 167 4/14.1 */
    t_udf_tag       DescriptorTag;                          // 00
    t_udf_timestamp RecordingDateandTime;                   // 16
    WORD            InterchangeLevel;                       // 28
    WORD            MaximumInterchangeLevel;                // 30
    DWORD           CharacterSetList;                       // 32
    DWORD           MaximumCharacterSetList;                // 36
    DWORD           FileSetNumber;                          // 40
    DWORD           FileSetDescriptorNumber;                // 44
    t_udf_charspec  LogicalVolumeIdentifierCharacterSet;    // 48
    dstring         LogicalVolumeIdentifier[128];           // 112
    t_udf_charspec  FileSetCharacterSet;                    // 240
    dstring         FileSetIdentifer[32];                   // 304
    dstring         CopyrightFileIdentifier[32];            // 336
    dstring         AbstractFileIdentifier[32];             // 368
    t_udf_long_ad   RootDirectoryICB;                       // 400
    t_udf_EntityID  DomainIdentifier;                       // 416
    t_udf_long_ad   NextExtent;                             // 448
    t_udf_long_ad   StreamDirectoryICB;                     // 464
    BYTE            Reserved[32];                           // 480
} t_udf_FileSetDescriptor, *tp_udf_FileSetDescriptor;       // 512

typedef struct {
    /* ECMA 167 4/14.6 */
    DWORD           PriorRecordedNumberofDirectEntries;     // 00
    WORD            StrategyType;                           // 04
    BYTE            StrategyParameter[2];                   // 06
    WORD            NumberofEntries;                        // 08
    BYTE            Reserved;                               // 10
    BYTE            FileType;                               // 11
    t_udf_lb_addr   ParentICBLocation;                      // 12
    WORD            Flags;                                  // 18
} t_udf_icbtag;                                             // 20

typedef struct {
    /* ECMA 167 4/14.9 */
    t_udf_tag       DescriptorTag;                          // 00
    t_udf_icbtag    ICBTag;                                 // 16
    DWORD           Uid;                                    // 36
    DWORD           Gid;                                    // 40
    DWORD           Permissions;                            // 44
    WORD            FileLinkCount;                          // 48
    BYTE            RecordFormat;                           // 50
    BYTE            RecordDisplayAttributes;                // 51
    DWORD           RecordLength;                           // 52
    QWORD           InformationLength;                      // 56
    QWORD           LogicalBlocksRecorded;                  // 64
    t_udf_timestamp AccessTime;                             // 72
    t_udf_timestamp ModificationTime;                       // 84
    t_udf_timestamp AttributeTime;                          // 96
    DWORD           Checkpoint;                             // 108
    t_udf_long_ad   ExtendedAttributeICB;                   // 112
    t_udf_EntityID  ImplementationIdentifier;               // 128
    QWORD           UniqueID;                               // 160
    DWORD           LengthofExtendedAttributes;             // 168
    DWORD           LengthofAllocationDescriptors;          // 172
    BYTE            ExtendedAttributes[1];                  // 176
    //BYTE            AllocationDescriptors[];                // 176
} t_udf_FileEntry, *tp_udf_FileEntry;                       // >= 176

typedef struct {
    /* ECMA 167 4/14.4 */
    t_udf_tag       DescriptorTag;                          // 00
    WORD            FileVersionNumber;                      // 16
    BYTE            FileCharacteristics;                    // 18
    BYTE            LengthofFileIdentifier;                 // 19
    t_udf_long_ad   ICB;                                    // 20
    WORD            LengthofImplementationUse;              // 36
    BYTE            ImplementationUse[1];                   // 38
    //char            FileIdentifier[];                       // 38
    //BYTE            Padding[];                              // 38
} t_udf_FileIdentifierDescriptor, *tp_udf_FileIdentifierDescriptor; // >= 38

#define udf_MAX_NAMELEN 256
#define udf_MAX_PATHLEN 2048

typedef struct {
    // public
    char        name[udf_MAX_NAMELEN];
    bool        is_dir, is_parent;
    // internal
    BYTE        *sector;
    tp_udf_FileIdentifierDescriptor fid;
    DWORD       partition_lba;
    DWORD       dir_lba, dir_end_lba;
    DWORD       sec_size;
    int         dir_left;
} t_udf_file, *tp_udf_file;

#pragma pack(pop)

tp_udf_file udf_find_file(const HANDLE hDrive, const WORD partition, const char *name);
tp_udf_file udf_get_root(const HANDLE hDrive, const WORD partition_number);
tp_udf_file udf_get_next(const HANDLE hDrive, tp_udf_file f); // advances f
tp_udf_file udf_get_sub(const HANDLE hDrive, tp_udf_file f);  // creates new f
bool udf_get_lba(const HANDLE hDrive, const tp_udf_file f, DWORD *start_lba, DWORD *end_lba);
void udf_free(tp_udf_file f);
