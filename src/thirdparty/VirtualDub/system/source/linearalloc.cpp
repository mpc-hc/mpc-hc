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
	Clear();
}

void VDLinearAllocator::Clear() {
	Block *p = mpBlocks;

	while(p) {
		Block *next = p->mpNext;

		free(p);

		p = next;
	}

	mpBlocks = NULL;
	mpAllocPtr = NULL;
	mAllocLeft = 0;
}

void *VDLinearAllocator::AllocateSlow(size_t bytes) {
	Block *block;
	void *p;

	if ((bytes + bytes) >= mBlockSize) {
		block = (Block *)malloc(sizeof(Block) + bytes);
		if (!block)
			throw MyMemoryError();

        mAllocLeft = 0;

	} else {
		block = (Block *)malloc(sizeof(Block) + mBlockSize);

		if (!block)
			throw MyMemoryError();

		mAllocLeft = mBlockSize - bytes;

	}

	p = block + 1;
	mpAllocPtr = (char *)p + bytes;

	block->mpNext = mpBlocks;
	mpBlocks = block;

	return p;
}

void VDFixedLinearAllocator::ThrowException() {
	throw MyMemoryError();
}

