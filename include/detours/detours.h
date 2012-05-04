//////////////////////////////////////////////////////////////////////////////
//
//  Core Detours Functionality (detours.h of detours.lib)
//
//  Microsoft Research Detours Package, Version 3.0 Build_316.
//
//  Copyright (c) Microsoft Corporation.  All rights reserved.
//

#pragma once
#ifndef _DETOURS_H_
#define _DETOURS_H_

#define DETOURS_VERSION     30000   // 3.00.00

//////////////////////////////////////////////////////////////////////////////
//

#if (_MSC_VER < 1299)
typedef LONG LONG_PTR;
typedef ULONG ULONG_PTR;
#endif

#ifndef __in_z
#define __in_z
#endif

//////////////////////////////////////////////////////////////////////////////
//
#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct  _GUID
{
    DWORD Data1;
    WORD Data2;
    WORD Data3;
    BYTE Data4[ 8 ];
} GUID;

#ifdef INITGUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        const GUID name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#else
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    const GUID name
#endif // INITGUID
#endif // !GUID_DEFINED

#if defined(__cplusplus)
#ifndef _REFGUID_DEFINED
#define _REFGUID_DEFINED
#define REFGUID             const GUID &
#endif // !_REFGUID_DEFINED
#else // !__cplusplus
#ifndef _REFGUID_DEFINED
#define _REFGUID_DEFINED
#define REFGUID             const GUID * const
#endif // !_REFGUID_DEFINED
#endif // !__cplusplus

//
//////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/////////////////////////////////////////////////// Instruction Target Macros.
//
#define DETOUR_INSTRUCTION_TARGET_NONE          ((PVOID)0)
#define DETOUR_INSTRUCTION_TARGET_DYNAMIC       ((PVOID)(LONG_PTR)-1)
#define DETOUR_SECTION_HEADER_SIGNATURE         0x00727444   // "Dtr\0"

extern const GUID DETOUR_EXE_RESTORE_GUID;
extern const GUID DETOUR_EXE_HELPER_GUID;

#define DETOUR_TRAMPOLINE_SIGNATURE             0x21727444  // Dtr!
typedef struct _DETOUR_TRAMPOLINE DETOUR_TRAMPOLINE, *PDETOUR_TRAMPOLINE;

/////////////////////////////////////////////////////////// Binary Structures.
//
#pragma pack(push, 8)
typedef struct _DETOUR_SECTION_HEADER
{
    DWORD       cbHeaderSize;
    DWORD       nSignature;
    DWORD       nDataOffset;
    DWORD       cbDataSize;

    DWORD       nOriginalImportVirtualAddress;
    DWORD       nOriginalImportSize;
    DWORD       nOriginalBoundImportVirtualAddress;
    DWORD       nOriginalBoundImportSize;

    DWORD       nOriginalIatVirtualAddress;
    DWORD       nOriginalIatSize;
    DWORD       nOriginalSizeOfImage;
    DWORD       cbPrePE;

    DWORD       nOriginalClrFlags;
    DWORD       reserved1;
    DWORD       reserved2;
    DWORD       reserved3;

    // Followed by cbPrePE bytes of data.
} DETOUR_SECTION_HEADER, *PDETOUR_SECTION_HEADER;

typedef struct _DETOUR_SECTION_RECORD
{
    DWORD       cbBytes;
    DWORD       nReserved;
    GUID        guid;
} DETOUR_SECTION_RECORD, *PDETOUR_SECTION_RECORD;

typedef struct _DETOUR_CLR_HEADER
{
    // Header versioning
    ULONG                   cb;
    USHORT                  MajorRuntimeVersion;
    USHORT                  MinorRuntimeVersion;

    // Symbol table and startup information
    IMAGE_DATA_DIRECTORY    MetaData;
    ULONG                   Flags;

    // Followed by the rest of the IMAGE_COR20_HEADER
} DETOUR_CLR_HEADER, *PDETOUR_CLR_HEADER;

typedef struct _DETOUR_EXE_RESTORE
{
    DWORD               cb;
    DWORD               cbidh;
    DWORD               cbinh;
    DWORD               cbclr;

    PBYTE               pidh;
    PBYTE               pinh;
    PBYTE               pclr;

    IMAGE_DOS_HEADER    idh;
    union {
        IMAGE_NT_HEADERS    inh;
        IMAGE_NT_HEADERS32  inh32;
        IMAGE_NT_HEADERS64  inh64;
        BYTE                raw[sizeof(IMAGE_NT_HEADERS64) +
                                sizeof(IMAGE_SECTION_HEADER) * 32];
    };
    DETOUR_CLR_HEADER   clr;

} DETOUR_EXE_RESTORE, *PDETOUR_EXE_RESTORE;

typedef struct _DETOUR_EXE_HELPER
{
    DWORD               cb;
    DWORD               pid;
    CHAR                DllName[MAX_PATH];

} DETOUR_EXE_HELPER, *PDETOUR_EXE_HELPER;

#pragma pack(pop)

#define DETOUR_SECTION_HEADER_DECLARE(cbSectionSize) \
{ \
      sizeof(DETOUR_SECTION_HEADER),\
      DETOUR_SECTION_HEADER_SIGNATURE,\
      sizeof(DETOUR_SECTION_HEADER),\
      (cbSectionSize),\
      \
      0,\
      0,\
      0,\
      0,\
      \
      0,\
      0,\
      0,\
      0,\
}

/////////////////////////////////////////////////////////////// Helper Macros.
//
#define DETOURS_STRINGIFY(x)    DETOURS_STRINGIFY_(x)
#define DETOURS_STRINGIFY_(x)    #x

///////////////////////////////////////////////////////////// Binary Typedefs.
//
typedef BOOL (CALLBACK *PF_DETOUR_BINARY_BYWAY_CALLBACK)(PVOID pContext,
                                                         PCHAR pszFile,
                                                         PCHAR *ppszOutFile);

typedef BOOL (CALLBACK *PF_DETOUR_BINARY_FILE_CALLBACK)(PVOID pContext,
                                                        PCHAR pszOrigFile,
                                                        PCHAR pszFile,
                                                        PCHAR *ppszOutFile);

typedef BOOL (CALLBACK *PF_DETOUR_BINARY_SYMBOL_CALLBACK)(PVOID pContext,
                                                          ULONG nOrigOrdinal,
                                                          ULONG nOrdinal,
                                                          ULONG *pnOutOrdinal,
                                                          PCHAR pszOrigSymbol,
                                                          PCHAR pszSymbol,
                                                          PCHAR *ppszOutSymbol);

typedef BOOL (CALLBACK *PF_DETOUR_BINARY_COMMIT_CALLBACK)(PVOID pContext);

typedef BOOL (CALLBACK *PF_DETOUR_ENUMERATE_EXPORT_CALLBACK)(PVOID pContext,
                                                             ULONG nOrdinal,
                                                             PCHAR pszName,
                                                             PVOID pCode);

typedef BOOL (CALLBACK *PF_DETOUR_IMPORT_FILE_CALLBACK)(PVOID pContext,
                                                        HMODULE hModule,
                                                        PCSTR pszFile);

typedef BOOL (CALLBACK *PF_DETOUR_IMPORT_FUNC_CALLBACK)(PVOID pContext,
                                                        DWORD nOrdinal,
                                                        PCSTR pszFunc,
                                                        PVOID pvFunc);

typedef VOID * PDETOUR_BINARY;
typedef VOID * PDETOUR_LOADED_BINARY;

//////////////////////////////////////////////////////////// Transaction APIs.
//
LONG WINAPI DetourTransactionBegin(VOID);
LONG WINAPI DetourTransactionAbort(VOID);
LONG WINAPI DetourTransactionCommit(VOID);
LONG WINAPI DetourTransactionCommitEx(PVOID **pppFailedPointer);

LONG WINAPI DetourUpdateThread(HANDLE hThread);

LONG WINAPI DetourAttach(PVOID *ppPointer,
                         PVOID pDetour);

LONG WINAPI DetourAttachEx(PVOID *ppPointer,
                           PVOID pDetour,
                           PDETOUR_TRAMPOLINE *ppRealTrampoline,
                           PVOID *ppRealTarget,
                           PVOID *ppRealDetour);

LONG WINAPI DetourDetach(PVOID *ppPointer,
                         PVOID pDetour);

BOOL WINAPI DetourSetIgnoreTooSmall(BOOL fIgnore);
BOOL WINAPI DetourSetRetainRegions(BOOL fRetain);

////////////////////////////////////////////////////////////// Code Functions.
//
PVOID WINAPI DetourFindFunction(PCSTR pszModule, PCSTR pszFunction);
PVOID WINAPI DetourCodeFromPointer(PVOID pPointer, PVOID *ppGlobals);
PVOID WINAPI DetourCopyInstruction(PVOID pDst,
                                   PVOID *pDstPool,
                                   PVOID pSrc,
                                   PVOID *ppTarget,
                                   LONG *plExtra);

///////////////////////////////////////////////////// Loaded Binary Functions.
//
HMODULE WINAPI DetourGetContainingModule(PVOID pvAddr);
HMODULE WINAPI DetourEnumerateModules(HMODULE hModuleLast);
PVOID WINAPI DetourGetEntryPoint(HMODULE hModule);
ULONG WINAPI DetourGetModuleSize(HMODULE hModule);
BOOL WINAPI DetourEnumerateExports(HMODULE hModule,
                                   PVOID pContext,
                                   PF_DETOUR_ENUMERATE_EXPORT_CALLBACK pfExport);
BOOL WINAPI DetourEnumerateImports(HMODULE hModule,
                                   PVOID pContext,
                                   PF_DETOUR_IMPORT_FILE_CALLBACK pfImportFile,
                                   PF_DETOUR_IMPORT_FUNC_CALLBACK pfImportFunc);

PVOID WINAPI DetourFindPayload(HMODULE hModule, REFGUID rguid, DWORD *pcbData);
PVOID WINAPI DetourFindPayloadEx(REFGUID rguid, DWORD * pcbData);
DWORD WINAPI DetourGetSizeOfPayloads(HMODULE hModule);

///////////////////////////////////////////////// Persistent Binary Functions.
//

PDETOUR_BINARY WINAPI DetourBinaryOpen(HANDLE hFile);
PVOID WINAPI DetourBinaryEnumeratePayloads(PDETOUR_BINARY pBinary,
                                           GUID *pGuid,
                                           DWORD *pcbData,
                                           DWORD *pnIterator);
PVOID WINAPI DetourBinaryFindPayload(PDETOUR_BINARY pBinary,
                                     REFGUID rguid,
                                     DWORD *pcbData);
PVOID WINAPI DetourBinarySetPayload(PDETOUR_BINARY pBinary,
                                    REFGUID rguid,
                                    PVOID pData,
                                    DWORD cbData);
BOOL WINAPI DetourBinaryDeletePayload(PDETOUR_BINARY pBinary, REFGUID rguid);
BOOL WINAPI DetourBinaryPurgePayloads(PDETOUR_BINARY pBinary);
BOOL WINAPI DetourBinaryResetImports(PDETOUR_BINARY pBinary);
BOOL WINAPI DetourBinaryEditImports(PDETOUR_BINARY pBinary,
                                    PVOID pContext,
                                    PF_DETOUR_BINARY_BYWAY_CALLBACK pfByway,
                                    PF_DETOUR_BINARY_FILE_CALLBACK pfFile,
                                    PF_DETOUR_BINARY_SYMBOL_CALLBACK pfSymbol,
                                    PF_DETOUR_BINARY_COMMIT_CALLBACK pfCommit);
BOOL WINAPI DetourBinaryWrite(PDETOUR_BINARY pBinary, HANDLE hFile);
BOOL WINAPI DetourBinaryClose(PDETOUR_BINARY pBinary);

/////////////////////////////////////////////////// Create Process & Load Dll.
//
typedef BOOL (WINAPI *PDETOUR_CREATE_PROCESS_ROUTINEA)
    (LPCSTR lpApplicationName,
     LPSTR lpCommandLine,
     LPSECURITY_ATTRIBUTES lpProcessAttributes,
     LPSECURITY_ATTRIBUTES lpThreadAttributes,
     BOOL bInheritHandles,
     DWORD dwCreationFlags,
     LPVOID lpEnvironment,
     LPCSTR lpCurrentDirectory,
     LPSTARTUPINFOA lpStartupInfo,
     LPPROCESS_INFORMATION lpProcessInformation);

typedef BOOL (WINAPI *PDETOUR_CREATE_PROCESS_ROUTINEW)
    (LPCWSTR lpApplicationName,
     LPWSTR lpCommandLine,
     LPSECURITY_ATTRIBUTES lpProcessAttributes,
     LPSECURITY_ATTRIBUTES lpThreadAttributes,
     BOOL bInheritHandles,
     DWORD dwCreationFlags,
     LPVOID lpEnvironment,
     LPCWSTR lpCurrentDirectory,
     LPSTARTUPINFOW lpStartupInfo,
     LPPROCESS_INFORMATION lpProcessInformation);

BOOL WINAPI DetourCreateProcessWithDllA(LPCSTR lpApplicationName,
                                        __in_z LPSTR lpCommandLine,
                                        LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                        LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                        BOOL bInheritHandles,
                                        DWORD dwCreationFlags,
                                        LPVOID lpEnvironment,
                                        LPCSTR lpCurrentDirectory,
                                        LPSTARTUPINFOA lpStartupInfo,
                                        LPPROCESS_INFORMATION lpProcessInformation,
                                        LPCSTR lpDllName,
                                        PDETOUR_CREATE_PROCESS_ROUTINEA
                                        pfCreateProcessA);

BOOL WINAPI DetourCreateProcessWithDllW(LPCWSTR lpApplicationName,
                                        __in_z LPWSTR lpCommandLine,
                                        LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                        LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                        BOOL bInheritHandles,
                                        DWORD dwCreationFlags,
                                        LPVOID lpEnvironment,
                                        LPCWSTR lpCurrentDirectory,
                                        LPSTARTUPINFOW lpStartupInfo,
                                        LPPROCESS_INFORMATION lpProcessInformation,
                                        LPCSTR lpDllName,
                                        PDETOUR_CREATE_PROCESS_ROUTINEW
                                        pfCreateProcessW);

#ifdef UNICODE
#define DetourCreateProcessWithDll      DetourCreateProcessWithDllW
#define PDETOUR_CREATE_PROCESS_ROUTINE  PDETOUR_CREATE_PROCESS_ROUTINEW
#else
#define DetourCreateProcessWithDll      DetourCreateProcessWithDllA
#define PDETOUR_CREATE_PROCESS_ROUTINE  PDETOUR_CREATE_PROCESS_ROUTINEA
#endif // !UNICODE

BOOL WINAPI DetourCreateProcessWithDllExA(LPCSTR lpApplicationName,
                                          __in_z LPSTR lpCommandLine,
                                          LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                          LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                          BOOL bInheritHandles,
                                          DWORD dwCreationFlags,
                                          LPVOID lpEnvironment,
                                          LPCSTR lpCurrentDirectory,
                                          LPSTARTUPINFOA lpStartupInfo,
                                          LPPROCESS_INFORMATION lpProcessInformation,
                                          LPCSTR lpDllName,
                                          PDETOUR_CREATE_PROCESS_ROUTINEA
                                          pfCreateProcessA);

BOOL WINAPI DetourCreateProcessWithDllExW(LPCWSTR lpApplicationName,
                                          __in_z LPWSTR lpCommandLine,
                                          LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                          LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                          BOOL bInheritHandles,
                                          DWORD dwCreationFlags,
                                          LPVOID lpEnvironment,
                                          LPCWSTR lpCurrentDirectory,
                                          LPSTARTUPINFOW lpStartupInfo,
                                          LPPROCESS_INFORMATION lpProcessInformation,
                                          LPCSTR lpDllName,
                                          PDETOUR_CREATE_PROCESS_ROUTINEW
                                          pfCreateProcessW);

#ifdef UNICODE
#define DetourCreateProcessWithDllEx    DetourCreateProcessWithDllExW
#define PDETOUR_CREATE_PROCESS_ROUTINE  PDETOUR_CREATE_PROCESS_ROUTINEW
#else
#define DetourCreateProcessWithDllEx    DetourCreateProcessWithDllExA
#define PDETOUR_CREATE_PROCESS_ROUTINE  PDETOUR_CREATE_PROCESS_ROUTINEA
#endif // !UNICODE

BOOL WINAPI DetourProcessViaHelperA(DWORD dwTargetPid,
                                    LPCSTR lpDllName,
                                    PDETOUR_CREATE_PROCESS_ROUTINEA pfCreateProcessA);

BOOL WINAPI DetourProcessViaHelperW(DWORD dwTargetPid,
                                    LPCSTR lpDllName,
                                    PDETOUR_CREATE_PROCESS_ROUTINEW pfCreateProcessW);

#ifdef UNICODE
#define DetourProcessViaHelper          DetourProcessViaHelperW
#else
#define DetourProcessViaHelper          DetourProcessViaHelperA
#endif // !UNICODE

BOOL WINAPI DetourUpdateProcessWithDll(HANDLE hProcess,
                                       LPCSTR *plpDlls,
                                       DWORD nDlls);

BOOL WINAPI DetourCopyPayloadToProcess(HANDLE hProcess,
                                       REFGUID rguid,
                                       PVOID pvData,
                                       DWORD cbData);
BOOL WINAPI DetourRestoreAfterWith(VOID);
BOOL WINAPI DetourRestoreAfterWithEx(PVOID pvData, DWORD cbData);
BOOL WINAPI DetourIsHelperProcess(VOID);
VOID CALLBACK DetourFinishHelperProcess(HWND, HINSTANCE, LPSTR, INT);

//
//////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif // __cplusplus

//////////////////////////////////////////////// Detours Internal Definitions.
//
#ifdef __cplusplus
#ifdef DETOURS_INTERNAL

#ifndef __deref_out
#define __deref_out
#endif

#ifndef __deref
#define __deref
#endif

//////////////////////////////////////////////////////////////////////////////
//
#if (_MSC_VER < 1299)
#include <imagehlp.h>
typedef IMAGEHLP_MODULE IMAGEHLP_MODULE64;
typedef PIMAGEHLP_MODULE PIMAGEHLP_MODULE64;
typedef IMAGEHLP_SYMBOL SYMBOL_INFO;
typedef PIMAGEHLP_SYMBOL PSYMBOL_INFO;

static inline
LONG InterlockedCompareExchange(LONG *ptr, LONG nval, LONG oval)
{
    return (LONG)::InterlockedCompareExchange((PVOID*)ptr, (PVOID)nval, (PVOID)oval);
}
#else
#include <dbghelp.h>
#endif

#ifdef IMAGEAPI // defined by DBGHELP.H
typedef LPAPI_VERSION (NTAPI *PF_ImagehlpApiVersionEx)(LPAPI_VERSION AppVersion);

typedef BOOL (NTAPI *PF_SymInitialize)(IN HANDLE hProcess,
                                       IN LPCSTR UserSearchPath,
                                       IN BOOL fInvadeProcess);
typedef DWORD (NTAPI *PF_SymSetOptions)(IN DWORD SymOptions);
typedef DWORD (NTAPI *PF_SymGetOptions)(VOID);
typedef DWORD64 (NTAPI *PF_SymLoadModule64)(IN HANDLE hProcess,
                                            IN HANDLE hFile,
                                            IN PSTR ImageName,
                                            IN PSTR ModuleName,
                                            IN DWORD64 BaseOfDll,
                                            IN DWORD SizeOfDll);
typedef BOOL (NTAPI *PF_SymGetModuleInfo64)(IN HANDLE hProcess,
                                            IN DWORD64 qwAddr,
                                            OUT PIMAGEHLP_MODULE64 ModuleInfo);
typedef BOOL (NTAPI *PF_SymFromName)(IN HANDLE hProcess,
                                     IN LPSTR Name,
                                     OUT PSYMBOL_INFO Symbol);

typedef struct _DETOUR_SYM_INFO
{
    HANDLE                  hProcess;
    HMODULE                 hDbgHelp;
    PF_ImagehlpApiVersionEx pfImagehlpApiVersionEx;
    PF_SymInitialize        pfSymInitialize;
    PF_SymSetOptions        pfSymSetOptions;
    PF_SymGetOptions        pfSymGetOptions;
    PF_SymLoadModule64      pfSymLoadModule64;
    PF_SymGetModuleInfo64   pfSymGetModuleInfo64;
    PF_SymFromName          pfSymFromName;
} DETOUR_SYM_INFO, *PDETOUR_SYM_INFO;

PDETOUR_SYM_INFO DetourLoadDbgHelp(VOID);

#endif // IMAGEAPI

#ifndef DETOUR_TRACE
#if DETOUR_DEBUG
#define DETOUR_TRACE(x) printf x
#define DETOUR_BREAK()  __debugbreak()
#include <stdio.h>
#include <limits.h>
#else
#define DETOUR_TRACE(x)
#define DETOUR_BREAK()
#endif
#endif

#ifdef DETOURS_IA64
#error Feature not supported in this release.





























































































#endif // DETOURS_IA64

#ifdef DETOURS_ARM
#error Feature not supported in this release.



#endif // DETOURS_ARM

//////////////////////////////////////////////////////////////////////////////

#endif // DETOURS_INTERNAL
#endif // __cplusplus

#endif // _DETOURS_H_
//
////////////////////////////////////////////////////////////////  End of File.
