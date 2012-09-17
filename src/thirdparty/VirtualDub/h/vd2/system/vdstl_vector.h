//	VirtualDub - Video processing and capture application
//	System library component
//	Copyright (C) 1998-2009 Avery Lee, All Rights Reserved.
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

#ifndef f_VD2_SYSTEM_VDSTL_VECTOR_H
#define f_VD2_SYSTEM_VDSTL_VECTOR_H

#ifdef _MSC_VER
#pragma once
#endif

template <class T, class A = vdallocator<T> >
class vdvector {
public:
	typedef typename A::reference reference;
	typedef typename A::const_reference const_reference;
	typedef T *iterator;
	typedef const T *const_iterator;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T value_type;
	typedef A allocator_type;
	typedef typename A::pointer pointer;
	typedef typename A::const_pointer const_pointer;
	typedef typename vdreverse_iterator<iterator, T>::type reverse_iterator;
	typedef typename vdreverse_iterator<const_iterator, T>::type const_reverse_iterator;
	
	// 23.2.4.1 construct/copy/destroy:
	explicit vdvector();
	explicit vdvector(const A&);
	explicit vdvector(size_type n);
	explicit vdvector(size_type n, const T& value, const A& = A());
	template <class InputIterator>
	vdvector(InputIterator first, InputIterator last, const A& = A());
	vdvector(const vdvector<T,A>& x);
	~vdvector();
	vdvector<T,A>& operator=(const vdvector<T,A>& x);
	template <class InputIterator>
	void assign(InputIterator first, InputIterator last);
	void			assign(size_type n, const T& u);
	allocator_type	get_allocator() const;

	// iterators:
	iterator				begin();
	const_iterator			begin() const;
	iterator				end();
	const_iterator			end() const;
	reverse_iterator		rbegin();
	const_reverse_iterator	rbegin() const;
	reverse_iterator		rend();
	const_reverse_iterator	rend() const;
	pointer					data();
	const_pointer			data() const;

	// 23.2.4.2 capacity:
	size_type				size() const;
	size_type				max_size() const;
	size_type				capacity() const;
	bool					empty() const;

	// element access:
	reference			operator[](size_type n);
	const_reference		operator[](size_type n) const;
	const_reference		at(size_type n) const;
	reference			at(size_type n);
	reference			front();
	const_reference		front() const;
	reference			back();
	const_reference		back() const;

	void		resize(size_type sz, T c = T());
	void		reserve(size_type n);

	template<class U>
	void		push_back_as(const U& x);

	reference	push_back();
	void		push_back(const T& x);
	void		pop_back();
	iterator	insert(iterator position, const T& x);

	template<class U>
	iterator	insert_as(iterator position, const U& x);

	void		insert(iterator position, size_type n, const T& x);
	template <class InputIterator>
	void insert(iterator position, InputIterator first, InputIterator last);
	iterator	erase(iterator position);
	iterator	erase(iterator first, iterator last);
	void		swap(vdvector<T,A>&);
	void		clear();

private:
	void free_storage();

	struct Data : public A {
		pointer mpBegin;
		pointer mpEnd;
		pointer mpEOS;

		Data() : A(), mpBegin(NULL), mpEnd(NULL), mpEOS(NULL) {}
		Data(const A& alloc) : A(alloc), mpBegin(NULL), mpEnd(NULL), mpEOS(NULL) {}
	} m;
};

template <class T, class A>
vdvector<T,A>::vdvector() {
}

template <class T, class A>
vdvector<T,A>::vdvector(const A& a)
	: m(a)
{
}

template <class T, class A>
vdvector<T,A>::vdvector(size_type n) {
	resize(n);
}

template <class T, class A>
vdvector<T,A>::vdvector(size_type n, const T& value, const A& a)
	: m(a)
{
	resize(n, value);
}

template <class T, class A>
template <class InputIterator>
vdvector<T,A>::vdvector(InputIterator first, InputIterator last, const A& a)
	: m(a)
{
	assign(first, last);
}

template <class T, class A>
vdvector<T,A>::vdvector(const vdvector<T,A>& x)
	: m(static_cast<const A&>(x.m))
{
	assign(x.m.mpBegin, x.m.mpEnd);
}

template <class T, class A>
vdvector<T,A>::~vdvector() {
	clear();

	if (m.mpBegin)
		m.deallocate(m.mpBegin, m.mpEOS - m.mpBegin);
}

template <class T, class A>
vdvector<T,A>& vdvector<T,A>::operator=(const vdvector<T,A>& x) {
	if (&x != this) {
		vdvector tmp(x);

		swap(tmp);
	}

	return *this;
}

template <class T, class A>
template <class InputIterator>
void vdvector<T,A>::assign(InputIterator first, InputIterator last) {
	clear();
	insert(m.mpBegin, first, last);
}

template <class T, class A>
void vdvector<T,A>::assign(size_type n, const T& u) {
	clear();
	insert(m.mpBegin, n, u);
}

template <class T, class A> typename vdvector<T,A>::allocator_type			vdvector<T,A>::get_allocator() const { return m; }
template <class T, class A> typename vdvector<T,A>::iterator				vdvector<T,A>::begin()			{ return m.mpBegin; }
template <class T, class A> typename vdvector<T,A>::const_iterator			vdvector<T,A>::begin() const	{ return m.mpBegin; }
template <class T, class A> typename vdvector<T,A>::iterator				vdvector<T,A>::end()			{ return m.mpEnd; }
template <class T, class A> typename vdvector<T,A>::const_iterator			vdvector<T,A>::end() const		{ return m.mpEnd; }
template <class T, class A> typename vdvector<T,A>::reverse_iterator		vdvector<T,A>::rbegin()			{ return reverse_iterator(m.mpEnd); }
template <class T, class A> typename vdvector<T,A>::const_reverse_iterator	vdvector<T,A>::rbegin() const	{ return const_reverse_iterator(m.mpEnd); }
template <class T, class A> typename vdvector<T,A>::reverse_iterator		vdvector<T,A>::rend()			{ return reverse_iterator(m.mpBegin); }
template <class T, class A> typename vdvector<T,A>::const_reverse_iterator	vdvector<T,A>::rend() const		{ return const_reverse_iterator(m.mpBegin); }
template <class T, class A> typename vdvector<T,A>::pointer					vdvector<T,A>::data()			{ return m.mpBegin; }
template <class T, class A> typename vdvector<T,A>::const_pointer			vdvector<T,A>::data() const		{ return m.mpBegin; }

template <class T, class A> typename vdvector<T,A>::size_type	vdvector<T,A>::size() const		{ return m.mpEnd - m.mpBegin; }
template <class T, class A> typename vdvector<T,A>::size_type	vdvector<T,A>::max_size() const	{ return m.max_size(); }
template <class T, class A> typename vdvector<T,A>::size_type	vdvector<T,A>::capacity() const	{ return m.mpEOS - m.mpBegin; }
template <class T, class A>          bool						vdvector<T,A>::empty() const	{ return m.mpBegin == m.mpEnd; }

template <class T, class A> typename vdvector<T,A>::reference			vdvector<T,A>::operator[](size_type n)			{ return m.mpBegin[n]; }
template <class T, class A> typename vdvector<T,A>::const_reference		vdvector<T,A>::operator[](size_type n) const	{ return m.mpBegin[n]; }
template <class T, class A> typename vdvector<T,A>::const_reference		vdvector<T,A>::at(size_type n) const			{ if (n >= (size_type)(m.mpEnd - m.mpBegin)) throw std::out_of_range("The index is out of range."); return m.mpBegin[n]; }
template <class T, class A> typename vdvector<T,A>::reference			vdvector<T,A>::at(size_type n)					{ if (n >= (size_type)(m.mpEnd - m.mpBegin)) throw std::out_of_range("The index is out of range."); return m.mpBegin[n]; }
template <class T, class A> typename vdvector<T,A>::reference			vdvector<T,A>::front()							{ return *m.mpBegin; }
template <class T, class A> typename vdvector<T,A>::const_reference		vdvector<T,A>::front() const					{ return *m.mpBegin; }
template <class T, class A> typename vdvector<T,A>::reference			vdvector<T,A>::back()							{ return m.mpEnd[-1]; }
template <class T, class A> typename vdvector<T,A>::const_reference		vdvector<T,A>::back() const						{ return m.mpEnd[-1]; }

template <class T, class A>
void vdvector<T,A>::resize(size_type sz, T c) {
	const size_type currSize = m.mpEnd - m.mpBegin;

	if (sz < currSize) {
		T *p = m.mpBegin + sz;
		while(m.mpEnd != p) {
			--m.mpEnd;
			m.mpEnd->~T();
		}
	} else if (sz > currSize) {
		const size_type currCapacity = m.mpEOS - m.mpBegin;

		if (sz > currCapacity) {
			pointer p0 = m.allocate(sz);

			try {
				pointer p1 = std::uninitialized_copy(m.mpBegin, m.mpEnd, p0);
				pointer p2 = p0 + sz;

				T *prev0 = m.mpBegin;
				T *prev1 = m.mpEnd;
				T *prev2 = m.mpEOS;
				try {
					std::uninitialized_fill(p1, p2, c);

					// destroy old range
					while(prev1 != prev0) {
						--prev1;
						prev0->~T();
					}

					m.mpBegin = p0;
					m.mpEnd = p2;
					m.mpEOS = p2;
				} catch(...) {
					while(p2 != p1) {
						--p2;
						p2->~T();
					}
					m.deallocate(p0, sz);
					throw;
				}

				m.deallocate(prev0, prev2 - prev0);
			} catch(...) {
				m.deallocate(p0, sz);
				throw;
			}
		} else {
			pointer newEnd = m.mpBegin + sz;
			std::uninitialized_fill(m.mpEnd, newEnd, c);
			m.mpEnd = newEnd;
		}
	}
}

template <class T, class A>
void vdvector<T,A>::reserve(size_type n) {
	const size_type currCapacity = m.mpEOS - m.mpBegin;

	if (n <= currCapacity)
		return;

	pointer p0 = m.allocate(n);

	try {
		pointer p1 = std::uninitialized_copy(m.mpBegin, m.mpEnd, p0);

		free_storage();

		m.mpBegin = p0;
		m.mpEnd = p1;
		m.mpEOS = p0 + n;
	} catch(...) {
		m.deallocate(p0, n);
		throw;
	}
}

template<class T, class A>
template<class U>
void vdvector<T,A>::push_back_as(const U& x) {
	if (m.mpEnd != m.mpEOS) {
		new(m.mpEnd) T(x);
		++m.mpEnd;
	} else {
		insert_as(m.mpEnd, x);
	}
}

template <class T, class A>
typename vdvector<T,A>::reference vdvector<T,A>::push_back() {
	if (m.mpEnd != m.mpEOS) {
		new(m.mpEnd) T();
		return *m.mpEnd++;
	} else {
		return *insert(m.mpEnd, T());
	}
}

template <class T, class A>
void vdvector<T,A>::push_back(const T& x) {
	VDASSERT(m.mpEnd >= m.mpBegin && m.mpEnd <= m.mpEOS);
	if (m.mpEnd != m.mpEOS) {
		new(m.mpEnd) T(x);
		++m.mpEnd;
	} else {
		insert(m.mpEnd, x);
	}
}

template <class T, class A>
void vdvector<T,A>::pop_back() {
	--m.mpEnd;
	m.mpEnd->~T();
}

template <class T, class A>
template<class U>
typename vdvector<T,A>::iterator vdvector<T,A>::insert_as(iterator position, const U& x) {
	if (m.mpEnd == m.mpEOS) {
		const size_type currSize = m.mpEnd - m.mpBegin;
		const size_type newCapacity = currSize + 1;

		const pointer p0 = m.allocate(newCapacity);
		pointer pe = p0;
		try {
			const pointer p1 = std::uninitialized_copy(m.mpBegin, position, p0);
			pe = p1;

			new(pe) T(x);
			++pe;

			const pointer p3 = std::uninitialized_copy(position, m.mpEnd, pe);
			pe = p3;

			free_storage();

			m.mpBegin = p0;
			m.mpEnd = pe;
			m.mpEOS = p0 + newCapacity;

			return p1;
		} catch(...) {
			while(pe != p0) {
				--pe;
				pe->~T();
			}

			m.deallocate(p0, newCapacity);
			throw;
		}
	} else if (position != m.mpEnd) {
		T tmp(*position);

		*position = x;

		new(m.mpEnd) T();

		try {
			vdmove_backward(position + 1, m.mpEnd, m.mpEnd + 1);
			position[1] = tmp;
			++m.mpEnd;
		} catch(...) {
			m.mpEnd->~T();
			throw;
		}

		return position;
	} else {
		new(m.mpEnd) T(x);
		++m.mpEnd;

		return position;
	}
}

template <class T, class A>
typename vdvector<T,A>::iterator vdvector<T,A>::insert(iterator position, const T& x) {
	if (m.mpEnd == m.mpEOS) {
		const size_type currSize = m.mpEnd - m.mpBegin;
		const size_type newCapacity = currSize + 1;

		const pointer p0 = m.allocate(newCapacity);
		pointer pe = p0;
		try {
			const pointer p1 = std::uninitialized_copy(m.mpBegin, position, p0);
			pe = p1;

			new(pe) T(x);
			++pe;

			const pointer p3 = std::uninitialized_copy(position, m.mpEnd, pe);
			pe = p3;

			free_storage();

			m.mpBegin = p0;
			m.mpEnd = pe;
			m.mpEOS = p0 + newCapacity;

			return p1;
		} catch(...) {
			while(pe != p0) {
				--pe;
				pe->~T();
			}

			m.deallocate(p0, newCapacity);
			throw;
		}
	} else if (position != m.mpEnd) {
		T tmp(*position);

		*position = x;

		new(m.mpEnd) T();

		try {
			vdmove_backward(position + 1, m.mpEnd, m.mpEnd + 1);
			position[1] = tmp;
			++m.mpEnd;
		} catch(...) {
			m.mpEnd->~T();
			throw;
		}

		return position;
	} else {
		new(m.mpEnd) T(x);
		++m.mpEnd;

		return position;
	}
}

template <class T, class A>
void vdvector<T,A>::insert(iterator position, size_type n, const T& x) {
	if ((size_type)(m.mpEOS - m.mpEnd) < n) {
		const size_type currSize = m.mpEnd - m.mpBegin;
		const size_type newCapacity = currSize + n;

		const pointer p0 = m.allocate(newCapacity);
		pointer pe = p0;
		try {
			const pointer p1 = std::uninitialized_copy(m.mpBegin, position, p0);
			pe = p1;

			const pointer p2 = p1 + n;
			std::uninitialized_fill(p1, p2, x);
			pe = p2;

			const pointer p3 = std::uninitialized_copy(position, m.mpEnd, p2);
			pe = p3;

			free_storage();

			m.mpBegin = p0;
			m.mpEnd = pe;
			m.mpEOS = p0 + newCapacity;
		} catch(...) {
			while(pe != p0) {
				--pe;
				pe->~T();
			}

			m.deallocate(p0, newCapacity);
			throw;
		}
	} else if (n) {
		pointer newEnd = m.mpEnd + n;
		pointer insEnd = std::copy_backward(position, m.mpEnd, newEnd);

		try {
			std::uninitialized_fill(position, m.mpEnd, x);
			m.mpEnd = newEnd;
		} catch(...) {
			std::copy(insEnd, newEnd, position);
			throw;
		}
	}
}

template <class T, class A>
template <class InputIterator>
void vdvector<T,A>::insert(iterator position, InputIterator first, InputIterator last) {
	const size_type n = last - first;

	if ((size_type)(m.mpEOS - m.mpEnd) < n) {
		const size_type currSize = m.mpEnd - m.mpBegin;
		const size_type newCapacity = currSize + n;

		const pointer p0 = m.allocate(newCapacity);
		pointer pe = p0;
		try {
			const pointer p1 = std::uninitialized_copy(m.mpBegin, position, p0);
			pe = p1;

			const pointer p2 = p1 + n;
			std::uninitialized_copy(first, last, p1);
			pe = p2;

			const pointer p3 = std::uninitialized_copy(position, m.mpEnd, p2);
			pe = p3;

			free_storage();

			m.mpBegin = p0;
			m.mpEnd = pe;
			m.mpEOS = p0 + newCapacity;
		} catch(...) {
			while(pe != p0) {
				--pe;
				pe->~T();
			}

			m.deallocate(p0, newCapacity);
			throw;
		}
	} else if (n) {
		pointer newEnd = m.mpEnd + n;
		pointer insEnd = std::copy_backward(position, m.mpEnd, newEnd);

		try {
			std::uninitialized_copy(first, last, position);
			m.mpEnd = newEnd;
		} catch(...) {
			std::copy(insEnd, newEnd, position);
			throw;
		}
	}
}

template <class T, class A>
typename vdvector<T,A>::iterator vdvector<T,A>::erase(iterator position) {
	m.mpEnd = vdmove_forward(position + 1, m.mpEnd, position);
	m.mpEnd->~T();

	return position;
}

template <class T, class A>
typename vdvector<T,A>::iterator vdvector<T,A>::erase(iterator first, iterator last) {
	if (first != last) {
		pointer p = vdmove_forward(last, m.mpEnd, first);

		for(pointer q = p; q != m.mpEnd; ++q)
			q->~T();

		m.mpEnd = p;
	}

	return first;
}

template <class T, class A>
void vdvector<T,A>::swap(vdvector<T,A>& x) {
	std::swap(m.mpBegin, x.m.mpBegin);
	std::swap(m.mpEnd, x.m.mpEnd);
	std::swap(m.mpEOS, x.m.mpEOS);
}

template <class T, class A>
void vdvector<T,A>::clear() {
	while(m.mpEnd != m.mpBegin) {
		--m.mpEnd;
		m.mpEnd->~T();
	}
}

template <class T, class A>
void vdvector<T,A>::free_storage() {
	pointer b = m.mpBegin;
	pointer p = m.mpEnd;

	while(p != b) {
		--p;
		p->~T();
	}

	m.deallocate(b, m.mpEOS - b);
}

#endif	// f_VD2_SYSTEM_VDSTL_VECTOR_H
