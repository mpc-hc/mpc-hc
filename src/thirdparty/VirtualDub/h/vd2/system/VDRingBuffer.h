//	VirtualDub - Video processing and capture application
//	System library component
//	Copyright (C) 1998-2004 Avery Lee, All Rights Reserved.
//
//	Beginning with 1.6.0, the VirtualDub system library is licensed
//	differently than the remainder of VirtualDub.  This particular file is
//	thus licensed as follows (the "zlib" license):
//
//	This software is provided 'as-is', without any express or implied
//	warranty.  In no event will the authors be held liable for any
//	damages arising from the use of this software.
//
//	Permission is granted to anyone to use this software for any purpose,
//	including commercial applications, and to alter it and redistribute it
//	freely, subject to the following restrictions:
//
//	1.	The origin of this software must not be misrepresented; you must
//		not claim that you wrote the original software. If you use this
//		software in a product, an acknowledgment in the product
//		documentation would be appreciated but is not required.
//	2.	Altered source versions must be plainly marked as such, and must
//		not be misrepresented as being the original software.
//	3.	This notice may not be removed or altered from any source
//		distribution.

#ifndef f_SYSTEM_VDRINGBUFFER_H
#define f_SYSTEM_VDRINGBUFFER_H

#include <string.h>
#include <utility>

#include <vd2/system/atomic.h>

class VDRingBufferBase {
public:
	VDRingBufferBase()
		: nSize(0)
		, nReadPoint(0)
		, nWritePoint(0)
	{
	}

	int		 getSize() const { return nSize; }
	int		 getReadOffset() const { return nReadPoint; }
	int		 getWriteOffset() const { return nWritePoint; }

protected:
	int				 nSize;
	int				 nReadPoint;
	int				 nWritePoint;
};

template<class T, class Allocator = std::allocator<T> >
class VDRingBuffer : public VDRingBufferBase, private Allocator {
protected:
	T				*pBuffer;
	VDAtomicInt		 nLevel;

public:
	VDRingBuffer();
	VDRingBuffer(int size);
	~VDRingBuffer();

	void	 Init(int size);
	void	 Shutdown();

	int		 getLevel() const { return nLevel; }
	int		 getSpace() const { return nSize - nLevel; }
	int		 getWriteSpace() const;
	T *		 getWritePtr() const { return pBuffer+nWritePoint; }

	int		 size() const { return nSize; }
	bool	 empty() const { return !nLevel; }
	bool	 full() const { return nLevel == nSize; }

	void	 Flush() { nReadPoint = nWritePoint = nLevel = 0; }

	int		 Read(T *pBuffer, int bytes);
	const T	*LockRead(int requested, int& actual);
	const T	*LockReadAll(int& actual);
	const T *LockReadWrapped(int requested, int& actual, int& nReadPoint);
	const T *LockReadAllWrapped(int& actual, int& nReadPoint);
	int		 UnlockRead(int actual);

	int		 Write(const T *pData, int bytes);
	T		*LockWrite(int requested, int& actual);
	T		*LockWriteAll(int& actual);
	int		 UnlockWrite(int actual);
};

template<class T, class Allocator>
VDRingBuffer<T, Allocator>::VDRingBuffer(int size)
	: pBuffer(NULL)
{
	Init(size);
}

template<class T, class Allocator>
VDRingBuffer<T, Allocator>::VDRingBuffer()
	: pBuffer(NULL)
	, nLevel(0)
{
}

template<class T, class Allocator>
VDRingBuffer<T, Allocator>::~VDRingBuffer() {
	Shutdown();
}

template<class T, class Allocator>
void VDRingBuffer<T, Allocator>::Init(int size) {
	Shutdown();
	pBuffer		= allocate(nSize = size, 0);
	nLevel		= 0;
	nReadPoint	= 0;
	nWritePoint	= 0;
}

template<class T, class Allocator>
void VDRingBuffer<T, Allocator>::Shutdown() {
	if (pBuffer) {
		deallocate(pBuffer, nSize);
		pBuffer = NULL;
	}
}

template<class T, class Allocator>
int VDRingBuffer<T, Allocator>::getWriteSpace() const {
	volatile int tc = nSize - nWritePoint;
	volatile int space = nSize - nLevel;

	if (tc > space)
		tc = space;

	return tc;
}

template<class T, class Allocator>
int VDRingBuffer<T, Allocator>::Read(T *pBuffer, int units) {
	VDASSERT(units >= 0);

	int actual = 0;
	const T *pSrc;

	while(units) {
		int tc;

		pSrc = LockRead(units, tc);

		if (!tc)
			break;

		memcpy(pBuffer, pSrc, tc * sizeof(T));

		UnlockRead(tc);

		actual += tc;
		units -= tc;
		pBuffer += tc;
	}

	return actual;
}

template<class T, class Allocator>
const T *VDRingBuffer<T, Allocator>::LockRead(int requested, int& actual) {
	VDASSERT(requested >= 0);

	int nLevelNow = nLevel;

	if (requested > nLevelNow)
		requested = nLevelNow;

	if (requested + nReadPoint > nSize)
		requested = nSize - nReadPoint;

	actual = requested;

	return pBuffer + nReadPoint;
}

template<class T, class Allocator>
const T *VDRingBuffer<T, Allocator>::LockReadAll(int& actual) {
	int requested = nLevel;

	if (requested + nReadPoint > nSize)
		requested = nSize - nReadPoint;

	actual = requested;

	return pBuffer + nReadPoint;
}

template<class T, class Allocator>
const T *VDRingBuffer<T, Allocator>::LockReadWrapped(int requested, int& actual, int& readpt) {
	int nLevelNow = nLevel;

	if (requested > nLevelNow)
		requested = nLevelNow;

	actual = requested;
	readpt = nReadPoint;

	return pBuffer;
}

template<class T, class Allocator>
const T *VDRingBuffer<T, Allocator>::LockReadAllWrapped(int& actual, int& readpt) {
	int requested = nLevel;

	actual = requested;
	readpt = nReadPoint;

	return pBuffer;
}

template<class T, class Allocator>
int VDRingBuffer<T, Allocator>::UnlockRead(int actual) {
	VDASSERT(actual >= 0);
	VDASSERT(nLevel >= actual);

	int newpt = nReadPoint + actual;

	if (newpt >= nSize)
		newpt -= nSize;

	nReadPoint = newpt;

	return nLevel.add(-actual);
}

template<class T, class Allocator>
int VDRingBuffer<T, Allocator>::Write(const T *src, int elements) {
	VDASSERT(elements >= 0);

	int actual = 0;
	while(elements) {
		int tc;
		void *dst = LockWrite(elements, tc);

		if (!tc)
			break;

		memcpy(dst, src, tc*sizeof(T));

		UnlockWrite(tc);

		actual += tc;
		elements -= tc;
		src += tc;
	}

	return actual;
}

template<class T, class Allocator>
T *VDRingBuffer<T, Allocator>::LockWrite(int requested, int& actual) {
	VDASSERT(requested >= 0);
	int nLevelNow = nSize - nLevel;

	if (requested > nLevelNow)
		requested = nLevelNow;

	if (requested + nWritePoint > nSize)
		requested = nSize - nWritePoint;

	actual = requested;

	return pBuffer + nWritePoint;
}

template<class T, class Allocator>
T *VDRingBuffer<T, Allocator>::LockWriteAll(int& actual) {
	int requested = nSize - nLevel;

	if (requested + nWritePoint > nSize)
		requested = nSize - nWritePoint;

	actual = requested;

	return pBuffer + nWritePoint;
}

template<class T, class Allocator>
int VDRingBuffer<T, Allocator>::UnlockWrite(int actual) {
	VDASSERT(actual >= 0);
	VDASSERT(nLevel + actual <= nSize);

	int newpt = nWritePoint + actual;

	if (newpt >= nSize)
		newpt = 0;

	nWritePoint = newpt;

	return nLevel.add(actual);
}



#endif
