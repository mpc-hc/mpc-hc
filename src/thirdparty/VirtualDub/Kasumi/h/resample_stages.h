#ifndef f_VD2_KASUMI_RESAMPLE_STAGES_H
#define f_VD2_KASUMI_RESAMPLE_STAGES_H

#include <vd2/Kasumi/pixmap.h>

class IVDResamplerFilter;
struct VDResamplerAxis;

class VDSteppedAllocator {
public:
	typedef	size_t		size_type;
	typedef	ptrdiff_t	difference_type;

	VDSteppedAllocator(size_t initialSize = 1024);
	~VDSteppedAllocator();

	void clear();
	void *allocate(size_type n);

protected:
	struct Block {
		Block *next;
	};

	Block *mpHead;
	char *mpAllocNext;
	size_t	mAllocLeft;
	size_t	mAllocNext;
	size_t	mAllocInit;
};

///////////////////////////////////////////////////////////////////////////
//
// resampler stages (common)
//
///////////////////////////////////////////////////////////////////////////

class IVDResamplerStage {
public:
	virtual ~IVDResamplerStage() {}

#if 0
	void *operator new(size_t n, VDSteppedAllocator& a) {
		return a.allocate(n);
	}

	void operator delete(void *p, VDSteppedAllocator& a) {
	}

private:
	// these should NEVER be called
	void operator delete(void *p) {}
#endif
};

class IVDResamplerSeparableRowStage2 {
public:
	virtual void Init(const VDResamplerAxis& axis, uint32 srcw) = 0;
	virtual void Process(void *dst, const void *src, uint32 w) = 0;
};

class IVDResamplerSeparableRowStage : public IVDResamplerStage {
public:
	virtual IVDResamplerSeparableRowStage2 *AsRowStage2() { return NULL; }
	virtual void Process(void *dst, const void *src, uint32 w, uint32 u, uint32 dudx) = 0;
	virtual int GetWindowSize() const = 0;
};

class IVDResamplerSeparableColStage : public IVDResamplerStage {
public:
	virtual int GetWindowSize() const = 0;
	virtual void Process(void *dst, const void *const *src, uint32 w, sint32 phase) = 0;
};

void VDResamplerGenerateTable(sint32 *dst, const IVDResamplerFilter& filter);
void VDResamplerGenerateTableF(float *dst, const IVDResamplerFilter& filter);
void VDResamplerGenerateTable2(sint32 *dst, const IVDResamplerFilter& filter, sint32 count, sint32 u, sint32 dudx);
void VDResamplerSwizzleTable(sint32 *dst, unsigned pairs);

#endif
