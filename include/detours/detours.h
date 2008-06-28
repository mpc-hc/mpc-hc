//////////////////////////////////////////////////////////////////////////////
//
//  File:       detours.h
//  Module:     detours.lib
//
//  Detours for binary functions.  Version 1.5 (Build 46)
//
//  Copyright 1995-2001, Microsoft Corporation
//

#pragma once
#ifndef _DETOURS_H_
#define _DETOURS_H_

#pragma comment(lib, "detours")

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
#define DETOUR_INSTRUCTION_TARGET_NONE          ((PBYTE)0)
#define DETOUR_INSTRUCTION_TARGET_DYNAMIC       ((PBYTE)~0ul)

/////////////////////////////////////////////////////////// Trampoline Macros.
//
// DETOUR_TRAMPOLINE(trampoline_prototype, target_name)
//
// The naked trampoline must be at least DETOUR_TRAMPOLINE_SIZE bytes.
//
#define DETOUR_TRAMPOLINE_SIZE          32
#define DETOUR_SECTION_HEADER_SIGNATURE 0x00727444   // "Dtr\0"

#define DETOUR_TRAMPOLINE(trampoline,target) \
static PVOID __fastcall _Detours_GetVA_##target(VOID) \
{ \
    return &target; \
} \
\
__declspec(naked) trampoline \
{ \
    __asm { nop };\
    __asm { nop };\
    __asm { call _Detours_GetVA_##target };\
    __asm { jmp eax };\
    __asm { ret };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
}

#define DETOUR_TRAMPOLINE_EMPTY(trampoline) \
__declspec(naked) trampoline \
{ \
    __asm { nop };\
    __asm { nop };\
    __asm { xor eax, eax };\
    __asm { mov eax, [eax] };\
    __asm { ret };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
    __asm { nop };\
}

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
    DWORD       nReserve;
} DETOUR_SECTION_HEADER, *PDETOUR_SECTION_HEADER;

typedef struct _DETOUR_SECTION_RECORD
{
    DWORD       cbBytes;
    DWORD       nReserved;
    GUID        guid;
} DETOUR_SECTION_RECORD, *PDETOUR_SECTION_RECORD;
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
                                                          DWORD nOrdinal,
                                                          PCHAR pszOrigSymbol,
                                                          PCHAR pszSymbol,
                                                          PCHAR *ppszOutSymbol);
typedef BOOL (CALLBACK *PF_DETOUR_BINARY_FINAL_CALLBACK)(PVOID pContext);
typedef BOOL (CALLBACK *PF_DETOUR_BINARY_EXPORT_CALLBACK)(PVOID pContext,
                                                          DWORD nOrdinal,
                                                          PCHAR pszName,
                                                          PBYTE pbCode);

typedef VOID * PDETOUR_BINARY;
typedef VOID * PDETOUR_LOADED_BINARY;

//////////////////////////////////////////////////////// Trampoline Functions.
//
PBYTE WINAPI DetourFunction(PBYTE pbTargetFunction,
                            PBYTE pbDetourFunction);

BOOL WINAPI DetourFunctionWithEmptyTrampoline(PBYTE pbTrampoline,
                                              PBYTE pbTarget,
                                              PBYTE pbDetour);

BOOL WINAPI DetourFunctionWithEmptyTrampolineEx(PBYTE pbTrampoline,
                                                PBYTE pbTarget,
                                                PBYTE pbDetour,
                                                PBYTE *ppbRealTrampoline,
                                                PBYTE *ppbRealTarget,
                                                PBYTE *ppbRealDetour);

BOOL  WINAPI DetourFunctionWithTrampoline(PBYTE pbTrampoline,
                                          PBYTE pbDetour);

BOOL  WINAPI DetourFunctionWithTrampolineEx(PBYTE pbTrampoline,
                                            PBYTE pbDetour,
                                            PBYTE *ppbRealTrampoline,
                                            PBYTE *ppbRealTarget);

BOOL  WINAPI DetourRemove(PBYTE pbTrampoline, PBYTE pbDetour);

////////////////////////////////////////////////////////////// Code Functions.
//
PBYTE WINAPI DetourFindFunction(PCHAR pszModule, PCHAR pszFunction);
PBYTE WINAPI DetourGetFinalCode(PBYTE pbCode, BOOL fSkipJmp);

PBYTE WINAPI DetourCopyInstruction(PBYTE pbDst, PBYTE pbSrc, PBYTE *ppbTarget);
PBYTE WINAPI DetourCopyInstructionEx(PBYTE pbDst,
                                     PBYTE pbSrc,
                                     PBYTE *ppbTarget,
                                     LONG *plExtra);

///////////////////////////////////////////////////// Loaded Binary Functions.
//
HMODULE WINAPI DetourEnumerateModules(HMODULE hModuleLast);
PBYTE WINAPI DetourGetEntryPoint(HMODULE hModule);
BOOL WINAPI DetourEnumerateExports(HMODULE hModule,
                                   PVOID pContext,
                                   PF_DETOUR_BINARY_EXPORT_CALLBACK pfExport);

PBYTE WINAPI DetourFindPayload(HMODULE hModule, REFGUID rguid, DWORD *pcbData);
DWORD WINAPI DetourGetSizeOfPayloads(HMODULE hModule);

///////////////////////////////////////////////// Persistent Binary Functions.
//
BOOL WINAPI DetourBinaryBindA(PCHAR pszFile, PCHAR pszDll, PCHAR pszPath);
BOOL WINAPI DetourBinaryBindW(PWCHAR pwzFile, PWCHAR pwzDll, PWCHAR pwzPath);
#ifdef UNICODE
#define DetourBinaryBind  DetourBinaryBindW
#else
#define DetourBinaryBind  DetourBinaryBindA
#endif // !UNICODE

PDETOUR_BINARY WINAPI DetourBinaryOpen(HANDLE hFile);
PBYTE WINAPI DetourBinaryEnumeratePayloads(PDETOUR_BINARY pBinary,
                                           GUID *pGuid,
                                           DWORD *pcbData,
                                           DWORD *pnIterator);
PBYTE WINAPI DetourBinaryFindPayload(PDETOUR_BINARY pBinary,
                                     REFGUID rguid,
                                     DWORD *pcbData);
PBYTE WINAPI DetourBinarySetPayload(PDETOUR_BINARY pBinary,
                                    REFGUID rguid,
                                    PBYTE pbData,
                                    DWORD cbData);
BOOL WINAPI DetourBinaryDeletePayload(PDETOUR_BINARY pBinary, REFGUID rguid);
BOOL WINAPI DetourBinaryPurgePayloads(PDETOUR_BINARY pBinary);
BOOL WINAPI DetourBinaryResetImports(PDETOUR_BINARY pBinary);
BOOL WINAPI DetourBinaryEditImports(PDETOUR_BINARY pBinary,
                                    PVOID pContext,
                                    PF_DETOUR_BINARY_BYWAY_CALLBACK pfByway,
                                    PF_DETOUR_BINARY_FILE_CALLBACK pfFile,
                                    PF_DETOUR_BINARY_SYMBOL_CALLBACK pfSymbol,
                                    PF_DETOUR_BINARY_FINAL_CALLBACK pfFinal);
BOOL WINAPI DetourBinaryWrite(PDETOUR_BINARY pBinary, HANDLE hFile);
BOOL WINAPI DetourBinaryClose(PDETOUR_BINARY pBinary);

/////////////////////////////////////////////// First Chance Exception Filter.
//
LPTOP_LEVEL_EXCEPTION_FILTER WINAPI
DetourFirstChanceExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelFilter);

///////////////////////////////////////////////// Create Process & Inject Dll.
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
                                        LPSTR lpCommandLine,
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
                                        LPWSTR lpCommandLine,
                                        LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                        LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                        BOOL bInheritHandles,
                                        DWORD dwCreationFlags,
                                        LPVOID lpEnvironment,
                                        LPCWSTR lpCurrentDirectory,
                                        LPSTARTUPINFOW lpStartupInfo,
                                        LPPROCESS_INFORMATION lpProcessInformation,
                                        LPCWSTR lpDllName,
                                        PDETOUR_CREATE_PROCESS_ROUTINEW
                                        pfCreateProcessW);
                  
#ifdef UNICODE
#define DetourCreateProcessWithDll  DetourCreateProcessWithDllW
#define PDETOUR_CREATE_PROCESS_ROUTINE     PDETOUR_CREATE_PROCESS_ROUTINEW
#else
#define DetourCreateProcessWithDll  DetourCreateProcessWithDllA
#define PDETOUR_CREATE_PROCESS_ROUTINE     PDETOUR_CREATE_PROCESS_ROUTINEA
#endif // !UNICODE

BOOL WINAPI DetourContinueProcessWithDllA(HANDLE hProcess, LPCSTR lpDllName);
BOOL WINAPI DetourContinueProcessWithDllW(HANDLE hProcess, LPCWSTR lpDllName);

#ifdef UNICODE
#define DetourContinueProcessWithDll    DetourContinueProcessWithDllW
#else
#define DetourContinueProcessWithDll    DetourContinueProcessWithDllA
#endif // !UNICODE
//
//////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif // __cplusplus

/////////////////////////////////////////////////////////////////// Old Names.
//
#define ContinueProcessWithDll            DetourContinueProcessWithDll 
#define ContinueProcessWithDllA           DetourContinueProcessWithDllA
#define ContinueProcessWithDllW           DetourContinueProcessWithDllW
#define CreateProcessWithDll              DetourCreateProcessWithDll 
#define CreateProcessWithDllA             DetourCreateProcessWithDllA
#define CreateProcessWithDllW             DetourCreateProcessWithDllW
#define DETOUR_TRAMPOLINE_WO_TARGET       DETOUR_TRAMPOLINE_EMPTY
#define DetourBinaryPurgePayload          DetourBinaryPurgePayloads
#define DetourEnumerateExportsForInstance DetourEnumerateExports
#define DetourEnumerateInstances          DetourEnumerateModules
#define DetourFindEntryPointForInstance   DetourGetEntryPoint
#define DetourFindFinalCode               DetourGetFinalCode
#define DetourFindPayloadInBinary         DetourFindPayload
#define DetourGetSizeOfBinary             DetourGetSizeOfPayloads
#define DetourRemoveWithTrampoline        DetourRemove
#define PCREATE_PROCESS_ROUTINE           PDETOUR_CREATE_PROCESS_ROUTINE
#define PCREATE_PROCESS_ROUTINEA          PDETOUR_CREATE_PROCESS_ROUTINEA
#define PCREATE_PROCESS_ROUTINEW          PDETOUR_CREATE_PROCESS_ROUTINEW
//

//////////////////////////////////////////////// Detours Internal Definitions.
//
#ifdef __cplusplus
#ifdef DETOURS_INTERNAL

//////////////////////////////////////////////////////////////////////////////
//
#ifdef IMAGEAPI // defined by IMAGEHLP.H
typedef LPAPI_VERSION (NTAPI *PF_ImagehlpApiVersionEx)(LPAPI_VERSION AppVersion);

typedef BOOL (NTAPI *PF_SymInitialize)(IN HANDLE hProcess,
                                       IN LPSTR UserSearchPath,
                                       IN BOOL fInvadeProcess);
typedef DWORD (NTAPI *PF_SymSetOptions)(IN DWORD SymOptions);
typedef DWORD (NTAPI *PF_SymGetOptions)(VOID);
typedef BOOL (NTAPI *PF_SymLoadModule)(IN HANDLE hProcess,
                                       IN HANDLE hFile,
                                       IN PSTR ImageName,
                                       IN PSTR ModuleName,
                                       IN DWORD BaseOfDll,
                                       IN DWORD SizeOfDll);
typedef BOOL (NTAPI *PF_SymGetModuleInfo)(IN HANDLE hProcess,
                                          IN DWORD dwAddr,
                                          OUT PIMAGEHLP_MODULE ModuleInfo);
typedef BOOL (NTAPI *PF_SymGetSymFromName)(IN HANDLE hProcess,
                                           IN LPSTR Name,
                                           OUT PIMAGEHLP_SYMBOL Symbol);
typedef BOOL (NTAPI *PF_BindImage)(IN LPSTR pszImageName,
                                   IN LPSTR pszDllPath,
                                   IN LPSTR pszSymbolPath);

typedef struct _DETOUR_SYM_INFO
{
    HANDLE                   hProcess;
    HMODULE                  hImageHlp;
    PF_ImagehlpApiVersionEx  pfImagehlpApiVersionEx;
    PF_SymInitialize         pfSymInitialize;
    PF_SymSetOptions         pfSymSetOptions;
    PF_SymGetOptions         pfSymGetOptions;
    PF_SymLoadModule         pfSymLoadModule;
    PF_SymGetModuleInfo      pfSymGetModuleInfo;
    PF_SymGetSymFromName     pfSymGetSymFromName;
    PF_BindImage             pfBindImage;
} DETOUR_SYM_INFO, *PDETOUR_SYM_INFO;

PDETOUR_SYM_INFO DetourLoadImageHlp(VOID);

#endif // IMAGEAPI

//////////////////////////////////////////////////////////////////////////////
//
class CDetourEnableWriteOnCodePage
{
public:
    CDetourEnableWriteOnCodePage(PBYTE pbCode, LONG cbCode = DETOUR_TRAMPOLINE_SIZE)
    {
        m_pbCode = pbCode;
        m_cbCode = cbCode;
        m_dwOldPerm = 0;
        m_hProcess = GetCurrentProcess();

        if (m_pbCode && m_cbCode) {
            if (!FlushInstructionCache(m_hProcess, pbCode, cbCode)) {
                return;
            }
            if (!VirtualProtect(pbCode,
                                cbCode,
                                PAGE_EXECUTE_READWRITE,
                                &m_dwOldPerm)) {
                return;
            }
        }
    }

    ~CDetourEnableWriteOnCodePage()
    {
        if (m_dwOldPerm && m_pbCode && m_cbCode) {
            DWORD dwTemp = 0;
            if (!FlushInstructionCache(m_hProcess, m_pbCode, m_cbCode)) {
                return;
            }
            if (!VirtualProtect(m_pbCode, m_cbCode, m_dwOldPerm, &dwTemp)) {
                return;
            }
        }
    }

    BOOL SetPermission(DWORD dwPerms)
    {
        if (m_dwOldPerm && m_pbCode && m_cbCode) {
            m_dwOldPerm = dwPerms;
            return TRUE;
        }
        return FALSE;
    }

    BOOL IsValid(VOID)
    {
        return m_pbCode && m_cbCode && m_dwOldPerm;
    }

private:
    HANDLE  m_hProcess;
    PBYTE   m_pbCode;
    LONG    m_cbCode;
    DWORD   m_dwOldPerm;
};

//////////////////////////////////////////////////////////////////////////////
//
inline PBYTE DetourGenMovEax(PBYTE pbCode, UINT32 nValue)
{
    *pbCode++ = 0xB8;
    *((UINT32*&)pbCode)++ = nValue;
    return pbCode;
}

inline PBYTE DetourGenMovEbx(PBYTE pbCode, UINT32 nValue)
{
    *pbCode++ = 0xBB;
    *((UINT32*&)pbCode)++ = nValue;
    return pbCode;
}

inline PBYTE DetourGenMovEcx(PBYTE pbCode, UINT32 nValue)
{
    *pbCode++ = 0xB9;
    *((UINT32*&)pbCode)++ = nValue;
    return pbCode;
}

inline PBYTE DetourGenMovEdx(PBYTE pbCode, UINT32 nValue)
{
    *pbCode++ = 0xBA;
    *((UINT32*&)pbCode)++ = nValue;
    return pbCode;
}

inline PBYTE DetourGenMovEsi(PBYTE pbCode, UINT32 nValue)
{
    *pbCode++ = 0xBE;
    *((UINT32*&)pbCode)++ = nValue;
    return pbCode;
}

inline PBYTE DetourGenMovEdi(PBYTE pbCode, UINT32 nValue)
{
    *pbCode++ = 0xBF;
    *((UINT32*&)pbCode)++ = nValue;
    return pbCode;
}

inline PBYTE DetourGenMovEbp(PBYTE pbCode, UINT32 nValue)
{
    *pbCode++ = 0xBD;
    *((UINT32*&)pbCode)++ = nValue;
    return pbCode;
}

inline PBYTE DetourGenMovEsp(PBYTE pbCode, UINT32 nValue)
{
    *pbCode++ = 0xBC;
    *((UINT32*&)pbCode)++ = nValue;
    return pbCode;
}

inline PBYTE DetourGenPush(PBYTE pbCode, UINT32 nValue)
{
    *pbCode++ = 0x68;
    *((UINT32*&)pbCode)++ = nValue;
    return pbCode;
}

inline PBYTE DetourGenPushad(PBYTE pbCode)
{
    *pbCode++ = 0x60;
    return pbCode;
}

inline PBYTE DetourGenPopad(PBYTE pbCode)
{
    *pbCode++ = 0x61;
    return pbCode;
}

inline PBYTE DetourGenJmp(PBYTE pbCode, PBYTE pbJmpDst, PBYTE pbJmpSrc = 0)
{
    if (pbJmpSrc == 0) {
        pbJmpSrc = pbCode;
    }
    *pbCode++ = 0xE9;
    *((INT32*&)pbCode)++ = pbJmpDst - (pbJmpSrc + 5);
    return pbCode;
}

inline PBYTE DetourGenCall(PBYTE pbCode, PBYTE pbJmpDst, PBYTE pbJmpSrc = 0)
{
    if (pbJmpSrc == 0) {
        pbJmpSrc = pbCode;
    }
    *pbCode++ = 0xE8;
    *((INT32*&)pbCode)++ = pbJmpDst - (pbJmpSrc + 5);
    return pbCode;
}

inline PBYTE DetourGenBreak(PBYTE pbCode)
{
    *pbCode++ = 0xcc;
    return pbCode;
}

inline PBYTE DetourGenRet(PBYTE pbCode)
{
    *pbCode++ = 0xc3;
    return pbCode;
}

inline PBYTE DetourGenNop(PBYTE pbCode)
{
    *pbCode++ = 0x90;
    return pbCode;
}
#endif DETOURS_INTERAL
#endif // __cplusplus

#endif // _DETOURS_H_
//
////////////////////////////////////////////////////////////////  End of File.
