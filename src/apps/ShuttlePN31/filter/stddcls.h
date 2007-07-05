#ifndef STDDCLS_H
#define STDDCLS_H

// stddcls.h -- Precompiled headers for WDM drivers
// Produced by Walt Oney's driver wizard

#ifdef __cplusplus
extern "C" {
#endif
	
#include <wdm.h>
#include <usbdi.h>
#include <usbioctl.h>

#include <stdio.h>
#include <stdarg.h>
	
#define PAGEDCODE code_seg("page")
#define LOCKEDCODE code_seg()
#define INITCODE code_seg("init")

#define PAGEDDATA data_seg("page")
#define LOCKEDDATA data_seg()
#define INITDATA data_seg("init")

#define arraysize(p) (sizeof(p)/sizeof((p)[0]))

// Override DDK definition of ASSERT so that debugger halts in the
// affected code and halts even in the unchecked OS

#if DBG && defined(_X86_)
#undef ASSERT
#define ASSERT(e) if(!(e)){DbgPrint("Assertion failure in "\
	__FILE__ ", line %d: " #e "\n", __LINE__);\
	_asm int 1\
}
#endif

// Currently, the support routines for managing the device remove lock aren't
// defined in the DDK or implemented by the OS. The following declarations are
// for use with the simplified implementation in RemoveLock.cpp.

#ifdef WIN98

typedef struct _IO_REMOVE_LOCK {
	LONG usage;					// reference count
	BOOLEAN removing;			// true if removal is pending
	KEVENT evRemove;			// event to wait on
} IO_REMOVE_LOCK, *PIO_REMOVE_LOCK;

VOID IoInitializeRemoveLock(PIO_REMOVE_LOCK lock, ULONG tag, ULONG minutes, ULONG maxcount);
NTSTATUS IoAcquireRemoveLock(PIO_REMOVE_LOCK lock, PVOID tag);
VOID IoReleaseRemoveLock(PIO_REMOVE_LOCK lock, PVOID tag);
VOID IoReleaseRemoveLockAndWait(PIO_REMOVE_LOCK lock, PVOID tag);

#endif

#ifdef __cplusplus
}
#endif

#endif
