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

#ifndef f_LIST_H
#define f_LIST_H

class ListNode {
public:
	ListNode *next, *prev;

	void Remove() {
		next->prev = prev;
		prev->next = next;
#ifdef _DEBUG
		prev = next = 0;
#endif
	}

	void InsertAfter(ListNode *node) {
		next = node;
		prev = node->prev;
		if (node->prev) node->prev->next = this;
		node->prev = this;
	}

	void InsertBefore(ListNode *node) {
		next = node->next;
		prev = node;
		if (node->next) node->next->prev = this;
		node->next = this;
	}

	ListNode *NextFromHead() const {
		return prev;
	}

	ListNode *NextFromTail() const {
		return next;
	}
};

class List {
private:
public:
	ListNode head, tail;

	// <--- next             prev --->
	//
	// head <-> node <-> node <-> tail

	List();
	List(int) {}

	void Init();

	void AddHead(ListNode *node) {
		node->InsertAfter(&head);
	}

	void AddTail(ListNode *node) {
		node->InsertBefore(&tail);
	}

	ListNode *RemoveHead();
	ListNode *RemoveTail();

	bool IsEmpty() const {
		return !head.prev->prev;
	}

	ListNode *AtHead() const {
		return head.prev;
	}

	ListNode *AtTail() const {
		return tail.next;
	}

	void Take(List& from);
	void Swap(List& with);
};

// Templated classes... templated classes good.

template<class T> class List2;

template<class T>
class ListNode2 : public ListNode {
friend class List2<T>;
public:
	void InsertBefore(ListNode2<T> *node) { ListNode::InsertBefore(node); }
	void InsertAfter(ListNode2<T> *node) { ListNode::InsertAfter(node); }

	void Remove() { ListNode::Remove(); }
	T *NextFromHead() const { return static_cast<T *>(static_cast<ListNode2<T>*>(ListNode::NextFromHead())); }
	T *NextFromTail() const { return static_cast<T *>(static_cast<ListNode2<T>*>(ListNode::NextFromTail())); }
};

template<class T>
class List2 : public List {
public:
	List2<T>() {}

	// This is a really lame, stupid way to postpone initialization of the
	// list.

	List2<T>(int v) : List(v) {}

	void AddHead(ListNode2<T> *node) { List::AddHead(node); }
	void AddTail(ListNode2<T> *node) { List::AddTail(node); }
	T *RemoveHead()   { return static_cast<T *>(static_cast<ListNode2<T>*>(List::RemoveHead())); }
	T *RemoveTail()   { return static_cast<T *>(static_cast<ListNode2<T>*>(List::RemoveTail())); }
	T *AtHead() const { return static_cast<T *>(static_cast<ListNode2<T>*>(List::AtHead())); }
	T *AtTail() const { return static_cast<T *>(static_cast<ListNode2<T>*>(List::AtTail())); }

	// I must admit to being pampered by STL (end is different though!!)

	T *begin() const { return AtHead(); }
	T *end() const { return AtTail(); }

	void take(List2<T>& from) { List::Take(from); }

	class iterator {
	protected:
		ListNode2<T> *node;
		ListNode2<T> *next;

	public:
		iterator() {}
		iterator(const iterator& src) throw() : node(src.node), next(src.next) {}

		bool operator!() const throw() { return 0 == next; }
		T *operator->() const throw() { return (T *)node; }
		operator bool() const throw() { return 0 != next; }
		operator T *() const throw() { return (T *)node; }
		T& operator *() const throw() { return *(T *)node; }
	};

	// fwit: forward iterator (SAFE if node disappears)
	// rvit: reverse iterator (SAFE if node disappears)

	class fwit : public iterator {
	public:
		fwit() throw() {}
		fwit(const fwit& src) throw() : iterator(src) {}
		fwit(ListNode2<T> *start) throw() {
			this->node = start;
			this->next = start->NextFromHead();
		}

		const fwit& operator=(ListNode2<T> *start) throw() {
			this->node = start;
			this->next = start->NextFromHead();

			return *this;
		}

		fwit& operator++() throw() {
			this->node = this->next;
			this->next = this->node->NextFromHead();

			return *this;
		}

		const fwit& operator+=(int v) throw() {
			while(this->next && v--) {
				this->node = this->next;
				this->next = this->node->NextFromHead();
			}

			return *this;
		}

		fwit operator+(int v) const throw() {
			fwit t(*this);

			t += v;

			return t;
		}

		// This one's for my sanity.

		void operator++(int) throw() {
			++*this;
		}
	};

	class rvit : public iterator {
	public:
		rvit() throw() {}

		rvit(ListNode2<T> *start) throw() {
			this->node = start;
			this->next = start->NextFromTail();
		}

		const rvit& operator=(ListNode2<T> *start) throw() {
			this->node = start;
			this->next = start->NextFromTail();

			return *this;
		}

		rvit& operator--() throw() {
			this->node = this->next;
			this->next = this->node->NextFromTail();

			return *this;
		}

		const rvit& operator-=(int v) throw() {
			while(this->next && v--) {
				this->node = this->next;
				this->next = this->node->NextFromTail();
			}

			return *this;
		}

		rvit operator-(int v) const throw() {
			rvit t(*this);

			t -= v;

			return t;
		}

		// This one's for my sanity.

		void operator--(int) throw() {
			--*this;
		}
	};
};

template<class T>
class ListAlloc : public List2<T> {
public:
	ListAlloc<T>() {}
	~ListAlloc<T>() {
		dispose();
	}

	void dispose() {
		T *node;

		while(node = this->RemoveHead())
			delete node;
	}
};

#endif
