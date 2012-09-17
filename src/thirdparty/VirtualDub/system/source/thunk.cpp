#include "stdafx.h"
#include <windows.h>
#include <map>
#include <vd2/system/atomic.h>
#include <vd2/system/refcount.h>
#include <vd2/system/thunk.h>
#include <vd2/system/binary.h>

class IVDJITAllocator {};

class VDJITAllocator : public vdrefcounted<IVDJITAllocator> {
public:
	VDJITAllocator();
	~VDJITAllocator();

	void *Allocate(size_t len);
	void Free(void *p, size_t len);

	void EndUpdate(void *p, size_t len);

protected:
	typedef std::map<void *, size_t> FreeChunks;
	FreeChunks mFreeChunks;
	FreeChunks::iterator mNextChunk;

	typedef std::map<void *, size_t> Allocations;
	Allocations mAllocations;

	uintptr		mAllocationGranularity;
};

VDJITAllocator::VDJITAllocator()
	: mNextChunk(mFreeChunks.end())
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);

	mAllocationGranularity = si.dwAllocationGranularity;
}

VDJITAllocator::~VDJITAllocator() {
	for(Allocations::iterator it(mAllocations.begin()), itEnd(mAllocations.end()); it!=itEnd; ++it) {
		VirtualFree(it->first, 0, MEM_RELEASE);
	}
}

void *VDJITAllocator::Allocate(size_t len) {
	len = (len + 15) & ~(size_t)15;

	FreeChunks::iterator itMark(mNextChunk), itEnd(mFreeChunks.end()), it(itMark);

	if (it == itEnd)
		it = mFreeChunks.begin();

	for(;;) {
		for(; it!=itEnd; ++it) {
			if (it->second >= len) {
				it->second -= len;

				void *p = (char *)it->first + it->second;

				if (!it->second) {
					if (mNextChunk == it)
						++mNextChunk;

					mFreeChunks.erase(it);
				}

				return p;
			}
		}

		if (itEnd == itMark)
			break;

		it = mFreeChunks.begin();
		itEnd = itMark;
	}

	size_t alloclen = (len + mAllocationGranularity - 1) & ~(mAllocationGranularity - 1);

	void *p = VirtualAlloc(NULL, alloclen, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (p) {
		try {
			Allocations::iterator itA(mAllocations.insert(Allocations::value_type(p, alloclen)).first);

			try {
				if (len < alloclen)
					mFreeChunks.insert(FreeChunks::value_type((char *)p + len, alloclen - len));

			} catch(...) {
				mAllocations.erase(itA);
				throw;
			}
		} catch(...) {
			VirtualFree(p, 0, MEM_RELEASE);
			p = NULL;
		}
	}

	return p;
}

void VDJITAllocator::Free(void *p, size_t len) {
	VDASSERT(p);
	VDASSERT(len < 0x10000);

	FreeChunks::iterator cur(mFreeChunks.lower_bound(p));
	if (cur != mFreeChunks.end() && (char *)p + len == cur->first) {
		len += cur->second;
		if (mNextChunk == cur)
			++mNextChunk;

		mFreeChunks.erase(cur++);
	}

	if (cur != mFreeChunks.begin()) {
		FreeChunks::iterator prev(cur);

		--prev;
		if ((char *)prev->first + prev->second == p) {
			p = prev->first;
			len += prev->second;
			if (mNextChunk == prev)
				++mNextChunk;
			mFreeChunks.erase(prev);
		}
	}

	uintptr start = (size_t)p;
	uintptr end = start + len;

	if (!((start | end) & (mAllocationGranularity - 1))) {
		Allocations::iterator it(mAllocations.find(p));

		if (it != mAllocations.end()) {
			VirtualFree((void *)start, 0, MEM_RELEASE);
			mAllocations.erase(it);
			return;
		}
	}

	mFreeChunks.insert(FreeChunks::value_type((void *)start, end-start));
}

void VDJITAllocator::EndUpdate(void *p, size_t len) {
	FlushInstructionCache(GetCurrentProcess(), p, len);
}

///////////////////////////////////////////////////////////////////////////

VDJITAllocator *g_pVDJITAllocator;
VDAtomicInt g_VDJITAllocatorLock;

bool VDInitThunkAllocator() {
	bool success = true;

	while(g_VDJITAllocatorLock.xchg(1))
		::Sleep(1);

	if (!g_pVDJITAllocator) {
		g_pVDJITAllocator = new_nothrow VDJITAllocator;
		if (!g_pVDJITAllocator)
			success = false;
	}

	if (success)
		g_pVDJITAllocator->AddRef();

	VDVERIFY(1 == g_VDJITAllocatorLock.xchg(0));

	return success;
}

void VDShutdownThunkAllocator() {
	while(g_VDJITAllocatorLock.xchg(1))
		::Sleep(1);

	VDASSERT(g_pVDJITAllocator);

	if (!g_pVDJITAllocator->Release())
		g_pVDJITAllocator = NULL;

	VDVERIFY(1 == g_VDJITAllocatorLock.xchg(0));
}

void *VDAllocateThunkMemory(size_t len) {
	return g_pVDJITAllocator->Allocate(len);
}

void VDFreeThunkMemory(void *p, size_t len) {
	g_pVDJITAllocator->Free(p, len);
}

void VDSetThunkMemory(void *p, const void *src, size_t len) {
	memcpy(p, src, len);
	g_pVDJITAllocator->EndUpdate(p, len);
}

void VDFlushThunkMemory(void *p, size_t len) {
	g_pVDJITAllocator->EndUpdate(p, len);
}

///////////////////////////////////////////////////////////////////////////

#ifdef _M_AMD64
	extern "C" void VDMethodToFunctionThunk64();
#else
	extern "C" void VDMethodToFunctionThunk32();
	extern "C" void VDMethodToFunctionThunk32_4();
	extern "C" void VDMethodToFunctionThunk32_8();
	extern "C" void VDMethodToFunctionThunk32_12();
	extern "C" void VDMethodToFunctionThunk32_16();
#endif

VDFunctionThunk *VDCreateFunctionThunkFromMethod(void *method, void *pThis, size_t argbytes, bool stdcall_thunk) {
#if defined(_M_IX86)
	void *pThunk = VDAllocateThunkMemory(16);

	if (!pThunk)
		return NULL;

	if (stdcall_thunk || !argbytes) {	// thiscall -> stdcall (easy case)
		uint8 thunkbytes[16]={
			0xB9, 0x00, 0x00, 0x00, 0x00,				// mov ecx, this
			0xE9, 0x00, 0x00, 0x00, 0x00				// jmp fn
		};


		VDWriteUnalignedLEU32(thunkbytes+1, (uint32)(uintptr)pThis);
		VDWriteUnalignedLEU32(thunkbytes+6, (uint32)method - ((uint32)pThunk + 10));

		VDSetThunkMemory(pThunk, thunkbytes, 15);
	} else {				// thiscall -> cdecl (hard case)
		uint8 thunkbytes[16]={
			0xE8, 0x00, 0x00, 0x00, 0x00,				// call VDFunctionThunk32
			0xC3,										// ret
			argbytes,									// db argbytes
			0,											// db 0
			0x00, 0x00, 0x00, 0x00,						// dd method
			0x00, 0x00, 0x00, 0x00,						// dd this
		};

		void (*adapter)();

		switch(argbytes) {
		case 4:		adapter = VDMethodToFunctionThunk32_4;	break;
		case 8:		adapter = VDMethodToFunctionThunk32_8;	break;
		case 12:	adapter = VDMethodToFunctionThunk32_12;	break;
		case 16:	adapter = VDMethodToFunctionThunk32_16;	break;
		default:	adapter = VDMethodToFunctionThunk32;	break;
		}

		VDWriteUnalignedLEU32(thunkbytes+1, (uint32)(uintptr)adapter - ((uint32)pThunk + 5));
		VDWriteUnalignedLEU32(thunkbytes+8, (uint32)(uintptr)method);
		VDWriteUnalignedLEU32(thunkbytes+12, (uint32)(uintptr)pThis);

		VDSetThunkMemory(pThunk, thunkbytes, 16);
	}

	return (VDFunctionThunk *)pThunk;
#elif defined(_M_AMD64)
	void *pThunk = VDAllocateThunkMemory(44);
	if (!pThunk)
		return NULL;

	uint8 thunkbytes[44]={
		0x48, 0x8D, 0x05, 0x09, 0x00, 0x00, 0x00,	// lea rax, [rip+9]
		0xFF, 0x25, 0x03, 0x00, 0x00, 0x00,			// jmp qword ptr [rip+3]
		0x90,										// nop
		0x90,										// nop
		0x90,										// nop
		0, 0, 0, 0, 0, 0, 0, 0,						// dq VDFunctionThunk64
		0, 0, 0, 0, 0, 0, 0, 0,						// dq method
		0, 0, 0, 0, 0, 0, 0, 0,						// dq this
		0, 0, 0, 0									// dd argspillbytes
	};

	VDWriteUnalignedLEU64(thunkbytes+16, (uint64)(uintptr)VDMethodToFunctionThunk64);
	VDWriteUnalignedLEU64(thunkbytes+24, (uint64)(uintptr)method);
	VDWriteUnalignedLEU64(thunkbytes+32, (uint64)(uintptr)pThis);

	// The stack must be aligned to a 16 byte boundary when the CALL
	// instruction occurs. On entry to VDFunctionThunk64(), the stack is misaligned
	// to 16n+8. Therefore, the number of argbytes must be 16m+8 and the number of
	// argspillbytes must be 16m+8-24.
	VDWriteUnalignedLEU32(thunkbytes+40, argbytes < 32 ? 0 : ((argbytes - 16 + 15) & ~15));

	VDSetThunkMemory(pThunk, thunkbytes, 44);

	return (VDFunctionThunk *)pThunk;
#else
	return NULL;
#endif
}

void VDDestroyFunctionThunk(VDFunctionThunk *pFnThunk) {
	// validate thunk
#if defined(_M_IX86)
	VDASSERT(((const uint8 *)pFnThunk)[0] == 0xB9 || ((const uint8 *)pFnThunk)[0] == 0xE8);
	VDFreeThunkMemory(pFnThunk, 16);
#elif defined(_M_AMD64)
	VDFreeThunkMemory(pFnThunk, 44);
#else
	VDASSERT(false);
#endif

}
