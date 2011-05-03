// $Id$
// The script is taken from the ffdshow-tryouts installer
// http://sourceforge.net/projects/ffdshow-tryout/

[Code]
Type
  TSystemInfo = record
    wProcessorArchitecture: Word;
    wReserved: Word;
    dwPageSize: DWORD;
    lpMinimumApplicationAddress: Integer;
    lpMaximumApplicationAddress: Integer;
    dwActiveProcessorMask: DWORD;
    dwNumberOfProcessors: DWORD;
    dwProcessorType: DWORD;
    dwAllocationGranularity: DWORD;
    wProcessorLevel: Word;
    wProcessorRevision: Word;
  end;

  TCPUCoreInfo_T = record
    dwCPUType,
    dwCPUFamily,
    dwCPUModel,
    dwCPUExtFamily,
    dwCPUExtModel,
    dwCPUStepping,
    dwCPUFeatures,                  // result from edx when cpuid called with eax=1
    dwCPUAMDExtSignature,           // AMD extended features
    dwBrandIndex,
    dwCLFlushLineSize,
    dwCPUExtFeatures: DWORD;        // cpuid result when eax = 1
  end;

  THTInfo = record
    htResultCode: DWord;
    nPhysicalProcs,                 // Number of physical processors
    nLogicalProcs,                  // Number of logical processors (including physical processors)
    nLogicalPerPackage: Integer;
    dwPhysicalAffinity,             // Mask for physical CPUs
    dwLogicalAffinity: DWORD;       // Mask for non-physical CPUs
    nMaxPhysGetProcAff: Integer;    // Max number of physical processors to get Phys/Log Affinity table
    pPhysProcAff: Integer;          // Allocate nMaxPhysGetProcAff*sizeof(PHYSPROCAFF_T) before calling WinCPUID_Init
  end;

  TCPUInfo = record
    bIsInitialized,                 // Check this to see if structure was successfully initialized
    bCPUID_Supported,
    bCloneFlag,                     // 0 -> Intel CPU, non-zero -> non-Intel CPU
    bMMX_Supported,
    bSSE_Supported,
    bSSE2_Supported,
    bSSEMMXExt_Supported,           // Some AMD CPUs support SSE MMX (integer) extensions only
    b3DNow_Supported,
    bExt3DNow_Supported,
    bHT_Supported,                  // HT supported and available for use, check htInfo for more details
    bDAZ_Supported,
    bRDTSC_Supported,
    bCMOV_Supported,
    EmulCode: Integer;
    llCPUFreqL,
    llCPUFreqH: DWord;
    eCPUVendor: DWORD;
    coreInfo: TCPUCoreInfo_T;
    htInfo: THTInfo;
    pCacheInfo: Integer;
    pProcString: String;
  end;

var
  cpu_sse: Boolean;
  cpu_sse2: Boolean;


// functions to detect CPU
function WinCPUID_Init(msGetFrequency: Integer; var pInfo: TCPUInfo): Integer; external 'WinCPUID_Init@files:WinCPUID.dll cdecl';
// function to get system information
procedure GetSystemInfo(var lpSystemInfo: TSystemInfo); external 'GetSystemInfo@kernel32.dll stdcall';


procedure CPUCheck();
var
  CPUInfo: TCPUInfo;
begin
  WinCPUID_Init(0, CPUInfo);

  if (CPUInfo.bIsInitialized = 0) then begin
    // something went wrong
  end
  else begin
    if (CPUInfo.bSSE_Supported  = 1) then begin
      cpu_sse  := true;
    end;
    if (CPUInfo.bSSE2_Supported = 1) then begin
      cpu_sse2 := true;
    end;
  end;
end;

function Is_SSE_Supported(): Boolean;
begin
  Result := cpu_sse;
end;

function Is_SSE2_Supported(): Boolean;
begin
  Result := cpu_sse2;
end;
