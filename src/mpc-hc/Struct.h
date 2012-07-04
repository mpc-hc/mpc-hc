/*********************************************************************
 * Structures and definitions undocumented or included in the NTDDK. *
 *********************************************************************/

#ifndef __STRUCT_H__
#define __STRUCT_H__

///////////////// Windows NT ///////////////
#include <winternl.h>

#ifdef _WIN64
typedef ULONG_PTR KAFFINITY, *PKAFFINITY;
#else
typedef ULONG KAFFINITY, *PKAFFINITY;
#endif

typedef LONG NTSTATUS, *PNTSTATUS;
typedef LONG KPRIORITY;

typedef void (CALLBACK *PKNORMAL_ROUTINE)(PVOID, PVOID, PVOID);

typedef struct _CLIENT_ID
{
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID, *PCLIENT_ID;


typedef struct RTL_DRIVE_LETTER_CURDIR              // Size = 0x10
{
    USHORT          Flags;
    USHORT          Length;
    ULONG           TimeStamp;
    UNICODE_STRING  DosPath;
} RTL_DRIVE_LETTER_CURDIR, *PRTL_DRIVE_LETTER_CURDIR;

typedef struct _RTL_USER_PROCESS_PARAMETERS_MPC     // Size = 0x290
{
    ULONG           AllocationSize;
    ULONG           Size;
    ULONG           Flags;
    ULONG           DebugFlags;
    HANDLE          hConsole;
    ULONG           ProcessGroup;
    HANDLE          hStdInput;
    HANDLE          hStdOutput;
    HANDLE          hStdError;
    UNICODE_STRING  CurrentDirectoryName;
    HANDLE          CurrentDirectoryHandle;
    UNICODE_STRING  DllPath;
    UNICODE_STRING  ImagePathName;
    UNICODE_STRING  CommandLine;
    PWSTR           Environment;
    ULONG           StartingPositionLeft;
    ULONG           StartingPositionTop;
    ULONG           Width;
    ULONG           Height;
    ULONG           CharWidth;
    ULONG           CharHeight;
    ULONG           ConsoleTextAttributes;
    ULONG           WindowFlags;
    ULONG           ShowWindowFlags;
    UNICODE_STRING  WindowTitle;
    UNICODE_STRING  DesktopName;
    UNICODE_STRING  ShellInfo;
    UNICODE_STRING  RuntimeInfo;
    RTL_DRIVE_LETTER_CURDIR DLCurrentDirectory[0x20];
} RTL_USER_PROCESS_PARAMETERS_MPC, *PRTL_USER_PROCESS_PARAMETERS_MPC;

typedef struct _LDR_MODULE
{
    LIST_ENTRY     InLoadOrderModuleList;
    LIST_ENTRY     InMemoryOrderModuleList;
    LIST_ENTRY     InInitializationOrderModuleList;
    PVOID          BaseAddress;
    PVOID          EntryPoint;
    ULONG          SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG          Flags;
    SHORT          LoadCount;
    SHORT          TlsIndex;
    LIST_ENTRY     HashTableEntry;
    ULONG          TimeDateStamp;
} LDR_MODULE, *PLDR_MODULE;

typedef struct _PEB_LDR_DATA_MPC                        // Size = 0x24
{
    ULONG           Length;                             // 00
    BOOLEAN         Initialized;                        // 04
    PVOID           SsHandle;                           // 08
    LIST_ENTRY      InLoadOrderModuleList;              // 0C
    LIST_ENTRY      InMemoryOrderModuleList;            // 14
    LIST_ENTRY      InInitializationOrderModuleList;    // 1C
//  void *          EntryInProgress;                    // 24
} PEB_LDR_DATA_MPC, *PPEB_LDR_DATA_MPC;

typedef struct _PEB_FREE_BLOCK    // Size = 8
{
    struct _PEB_FREE_BLOCK *Next;
    ULONG Size;
} PEB_FREE_BLOCK, *PPEB_FREE_BLOCK;

// Structured Exception Handler
typedef struct _SEH
{
    struct _SEH *pNext;
    FARPROC     pfnHandler;
} SEH, *PSEH;

#pragma pack(1)

// Thread Information Block (FS:[0])
typedef struct _TIB_NT
{
    PSEH        pvExcept;             // 00 Head of exception record list
    PVOID       pvStackBase;          // 04
    PVOID       pvStackLimit;         // 08
    PVOID       SubSystemTib;         // 0C
    union
    {
        PVOID FiberData;             // 10
        DWORD Version;
    };
    PVOID      pvArbitrary;          // 14 Available for application use
    struct _TIB_NT *pTIBSelf;        // 18 Linear address of TIB structure
} TIB_NT, *PTIB_NT;

typedef void (*PPEBLOCKROUTINE)(PVOID);

// PEB (Process Environment Block) data structure (FS:[0x30])
// Located at addr. 0x7FFDF000
typedef struct _PEB_NT                                  // Size = 0x1E8
{
    BOOLEAN             InheritedAddressSpace;          //000
    BOOLEAN             ReadImageFileExecOptions;       //001
    BOOLEAN             BeingDebugged;                  //002
    BOOLEAN             SpareBool;                      //003 Allocation size
    HANDLE              Mutant;                         //004
    HINSTANCE           ImageBaseAddress;               //008 Instance
    PPEB_LDR_DATA_MPC   LdrData;                        //00C
    PRTL_USER_PROCESS_PARAMETERS_MPC ProcessParameters; //010
    ULONG               SubSystemData;                  //014
    HANDLE              ProcessHeap;                    //018
    KSPIN_LOCK          FastPebLock;                    //01C
    PPEBLOCKROUTINE     FastPebLockRoutine;             //020
    PPEBLOCKROUTINE     FastPebUnlockRoutine;           //024
    ULONG               EnvironmentUpdateCount;         //028
    PVOID *             KernelCallbackTable;            //02C
    PVOID               EventLogSection;                //030
    PVOID               EventLog;                       //034
    PPEB_FREE_BLOCK     FreeList;                       //038
    ULONG               TlsExpansionCounter;            //03C
    ULONG               TlsBitmap;                      //040
    LARGE_INTEGER       TlsBitmapBits;                  //044
    PVOID               ReadOnlySharedMemoryBase;       //04C
    PVOID               ReadOnlySharedMemoryHeap;       //050
    PVOID *             ReadOnlyStaticServerData;       //054
    PVOID               AnsiCodePageData;               //058
    PVOID               OemCodePageData;                //05C
    PVOID               UnicodeCaseTableData;           //060
    ULONG               NumberOfProcessors;             //064
    LARGE_INTEGER       NtGlobalFlag;                   //068 Address of a local copy
    LARGE_INTEGER       CriticalSectionTimeout;         //070
    ULONG               HeapSegmentReserve;             //078
    ULONG               HeapSegmentCommit;              //07C
    ULONG               HeapDeCommitTotalFreeThreshold; //080
    ULONG               HeapDeCommitFreeBlockThreshold; //084
    ULONG               NumberOfHeaps;                  //088
    ULONG               MaximumNumberOfHeaps;           //08C
    PVOID **            ProcessHeaps;                   //090
    PVOID               GdiSharedHandleTable;           //094
    PVOID               ProcessStarterHelper;           //098
    PVOID               GdiDCAttributeList;             //09C
    KSPIN_LOCK          LoaderLock;                     //0A0
    ULONG               OSMajorVersion;                 //0A4
    ULONG               OSMinorVersion;                 //0A8
    USHORT              OSBuildNumber;                  //0AC
    USHORT              OSCSDVersion;                   //0AE
    ULONG               OSPlatformId;                   //0B0
    ULONG               ImageSubsystem;                 //0B4
    ULONG               ImageSubsystemMajorVersion;     //0B8
    ULONG               ImageSubsystemMinorVersion;     //0BC
    ULONG               ImageProcessAffinityMask;       //0C0
    ULONG               GdiHandleBuffer[0x22];          //0C4
    ULONG               PostProcessInitRoutine;         //14C
    ULONG               TlsExpansionBitmap;             //150
    UCHAR               TlsExpansionBitmapBits[0x80];   //154
    ULONG               SessionId;                      //1D4
    void *              AppCompatInfo;                  //1D8
    UNICODE_STRING      CSDVersion;                     //1DC
} PEB_NT, *PPEB_NT;

// TEB (Thread Environment Block) data structure (FS:[0x18])
// Located at 0x7FFDE000, 0x7FFDD000, ...
typedef struct _TEB_NT                              // Size = 0xF88
{
    NT_TIB          Tib;                            //000
    PVOID           EnvironmentPointer;             //01C
    CLIENT_ID       ClientId;                       //020
    HANDLE          ActiveRpcHandle;                //028
    PVOID           ThreadLocalStoragePointer;      //02C
    PPEB_NT         ProcessEnvironmentBlock;        //030 PEB
    ULONG           LastErrorValue;                 //034
    ULONG           CountOfOwnedCriticalSections;   //038
    ULONG           CsrClientThread;                //03C
    ULONG           Win32ThreadInfo;                //040
    UCHAR           Win32ClientInfo[0x7C];          //044
    ULONG           WOW32Reserved;                  //0C0
    ULONG           CurrentLocale;                  //0C4
    ULONG           FpSoftwareStatusRegister;       //0C8
    UCHAR           SystemReserved1[0xD8];          //0CC
    ULONG           Spare1;                         //1A4
    ULONG           ExceptionCode;                  //1A8
    UCHAR           SpareBytes1[0x28];              //1AC
    UCHAR           SystemReserved2[0x28];          //1D4
    UCHAR           GdiTebBatch[0x4E0];             //1FC
    ULONG           GdiRgn;                         //6DC
    ULONG           GdiPen;                         //6E0
    ULONG           GdiBrush;                       //6E4
    CLIENT_ID       RealClientId;                   //6E8
    ULONG           GdiCachedProcessHandle;         //6F0
    ULONG           GdiClientPID;                   //6F4
    ULONG           GdiClientTID;                   //6F8
    ULONG           GdiThreadLocalInfo;             //6FC
    UCHAR           UserReserved[0x14];             //700
    UCHAR           glDispatchTable[0x460];         //714
    UCHAR           glReserved1[0x68];              //B74
    ULONG           glReserved2;                    //BDC
    ULONG           glSectionInfo;                  //BE0
    ULONG           glSection;                      //BE4
    ULONG           glTable;                        //BE8
    ULONG           glCurrentRC;                    //BEC
    ULONG           glContext;                      //BF0
    ULONG           LastStatusValue;                //BF4
    LARGE_INTEGER   StaticUnicodeString;            //BF8
    UCHAR           StaticUnicodeBuffer[0x20C];     //C00
    ULONG           DeallocationStack;              //E0C
    UCHAR           TlsSlots[0x100];                //E10
    LARGE_INTEGER   TlsLinks;                       //F10
    ULONG           Vdm;                            //F18
    ULONG           ReservedForNtRpc;               //F1C
    LARGE_INTEGER   DbgSsReserved;                  //F20
    ULONG           HardErrorsAreDisabled;          //F28
    UCHAR           Instrumentation[0x40];          //F2C
    ULONG           WinSockData;                    //F6C
    ULONG           GdiBatchCount;                  //F70
    ULONG           Spare2;                         //F74
    ULONG           Spare3;                         //F78
    ULONG           Spare4;                         //F7C
    ULONG           ReservedForOle;                 //F80
    ULONG           WaitingOnLoaderLock;            //F84
    //PVOID           StackCommit;
    //PVOID           StackCommitMax;
    //PVOID           StackReserved;
    //PVOID           MessageQueue;
} TEB_NT, *PTEB_NT;

#pragma pack()



typedef struct _THREAD_BASIC_INFORMATION
{
    NTSTATUS    ExitStatus;
    PTIB_NT     TebBaseAddress;
    CLIENT_ID   ClientId;
    KAFFINITY   AffinityMask;
    KPRIORITY   Priority;
    KPRIORITY   BasePriority;
} THREAD_BASIC_INFORMATION, *PTHREAD_BASIC_INFORMATION;


#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)
#define SystemProcessesAndThreadsInformation    5


typedef struct _VM_COUNTERS
{
    SIZE_T      PeakVirtualSize;
    SIZE_T      VirtualSize;
    ULONG       PageFaultCount;
    SIZE_T      PeakWorkingSetSize;
    SIZE_T      WorkingSetSize;
    SIZE_T      QuotaPeakPagedPoolUsage;
    SIZE_T      QuotaPagedPoolUsage;
    SIZE_T      QuotaPeakNonPagedPoolUsage;
    SIZE_T      QuotaNonPagedPoolUsage;
    SIZE_T      PagefileUsage;
    SIZE_T      PeakPagefileUsage;
} VM_COUNTERS;

typedef struct _SYSTEM_THREAD_INFORMATION
{
    LARGE_INTEGER   KernelTime;
    LARGE_INTEGER   UserTime;
    LARGE_INTEGER   CreateTime;
    ULONG           WaitTime;
    PVOID           StartAddress;
    CLIENT_ID       ClientId;
    KPRIORITY       Priority;
    KPRIORITY       BasePriority;
    ULONG           ContextSwitchCount;
    LONG            State;
    LONG            WaitReason;
} SYSTEM_THREAD_INFORMATION, *PSYSTEM_THREAD_INFORMATION;

#endif // __STRUCT_H__
