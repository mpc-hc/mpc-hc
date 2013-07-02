#ifndef f_VD2_SYSTEM_LINEARALLOC_H
#define f_VD2_SYSTEM_LINEARALLOC_H

#include <new>

class VDLinearAllocator {
public:
	explicit VDLinearAllocator(uint32 blockSize = 4096);
	~VDLinearAllocator();
	
	void Clear();

	void *Allocate(size_t bytes) {
		void *p = mpAllocPtr;

		bytes = (bytes + sizeof(void *) - 1) & ((size_t)0 - (size_t)sizeof(void *));

		if (mAllocLeft < bytes)
			p = AllocateSlow(bytes);
		else {
			mAllocLeft -= bytes;
			mpAllocPtr += bytes;
		}

		return p;
	}

	template<class T>
	T *Allocate() {
		return new(Allocate(sizeof(T))) T;
	}

	template<class T, class A1>
	T *Allocate(const A1& a1) {
		return new(Allocate(sizeof(T))) T(a1);
	}

	template<class T, class A1, class A2>
	T *Allocate(const A1& a1, const A2& a2) {
		return new(Allocate(sizeof(T))) T(a1, a2);
	}

	template<class T, class A1, class A2, class A3>
	T *Allocate(const A1& a1, const A2& a2, const A3& a3) {
		return new(Allocate(sizeof(T))) T(a1, a2, a3);
	}

protected:
	void *AllocateSlow(size_t bytes);

	union Block {
		Block *mpNext;
		double mAlign;
	};

	Block *mpBlocks;
	char *mpAllocPtr;
	size_t mAllocLeft;
	size_t mBlockSize;
};

class VDFixedLinearAllocator {
public:
	VDFixedLinearAllocator(void *mem, size_t size)
		: mpAllocPtr((char *)mem)
		, mAllocLeft(size)
	{
	}

	void *Allocate(size_t bytes) {
		void *p = mpAllocPtr;

		if (mAllocLeft < bytes)
			ThrowException();

		mAllocLeft -= bytes;
		mpAllocPtr += bytes;
		return p;
	}

protected:
	void ThrowException();

	char *mpAllocPtr;
	size_t mAllocLeft;
};

#endif
