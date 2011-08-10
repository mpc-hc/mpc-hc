#include "stdafx.h"
#include <vd2/system/error.h>
#include <vd2/system/linearalloc.h>

VDLinearAllocator::VDLinearAllocator(uint32 blockSize)
	: mpBlocks(NULL)
	, mpAllocPtr(NULL)
	, mAllocLeft(0)
	, mBlockSize(blockSize)
{
}

VDLinearAllocator::~VDLinearAllocator() {
	Block *p = mpBlocks;

	while(p) {
		Block *next = p->mpNext;

		free(p);

		p = next;
	}
}

void *VDLinearAllocator::AllocateSlow(size_t bytes) {
	Block *block;
	void *p;

	if ((bytes + bytes) >= mBlockSize) {
		block = (Block *)malloc(sizeof(Block) + bytes);
		if (!block)
			throw MyMemoryError();

		p = block + 1;

	} else {
		block = (Block *)malloc(sizeof(Block) + mBlockSize);
		if (!block)
			throw MyMemoryError();

		p = block + 1;
		mpAllocPtr = (char *)p + bytes;
		mAllocLeft = mBlockSize - bytes;
	}

	block->mpNext = mpBlocks;
	mpBlocks = block;

	return p;
}

void VDFixedLinearAllocator::ThrowException() {
	throw MyMemoryError();
}

