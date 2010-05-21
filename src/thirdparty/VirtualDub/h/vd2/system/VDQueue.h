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

#ifndef f_VD2_SYSTEM_VDQUEUE_H
#define f_VD2_SYSTEM_VDQUEUE_H

#include <vd2/system/List.h>

template<class T>
class VDQueueNode : public ListNode2< VDQueueNode<T> > {
public:
	T t;
	VDQueueNode(const T& t2) : t(t2) {}
};

template<class T>
class VDQueue {
public:
	ListAlloc< VDQueueNode<T> > list;

	VDQueue<T>();
	~VDQueue<T>();
	T Pop();
	T Peek();
	void Push(const T&);
	bool isEmpty() { return list.IsEmpty(); }
};

template<class T>
VDQueue<T>::VDQueue<T>() {
}

template<class T>
VDQueue<T>::~VDQueue<T>() {
	while(!list.IsEmpty())
		delete list.RemoveTail();
}

template<class T>
T VDQueue<T>::Peek() {
	return list.AtHead()->t;
}

template<class T>
T VDQueue<T>::Pop() {
	return list.RemoveHead()->t;
}

template<class T>
void VDQueue<T>::Push(const T& t) {
	list.AddTail(new VDQueueNode<T>(t));
}

/////////////

template<class T>
class VDQueueAlloc : public VDQueue<T> {
public:
	~VDQueueAlloc();
};

template<class T>
VDQueueAlloc<T>::~VDQueueAlloc() {
	for(ListAlloc< VDQueueNode<T> >::fwit it = list.begin(); it; ++it)
		delete &*it;
}

#endif
