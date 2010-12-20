#ifndef _ARRAY_ALLOCATOR_H_
#define _ARRAY_ALLOCATOR_H_

template <class T,size_t size> class array_allocator
{
private:
    T array[size];
public:
    typedef T value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    typedef T* pointer;
    typedef const T* const_pointer;

    typedef T& reference;
    typedef const T& const_reference;

    pointer address(reference r) const {
        return &r;
    }
    const_pointer address(const_reference r) const {
        return &r;
    }

    array_allocator() throw() {}
    template <class U,size_t sz> array_allocator(const array_allocator<U,sz>& ) throw() {}
    ~array_allocator() throw() {}

    pointer allocate(size_type n, const void* = 0) {
        return array;
    }
    void deallocate(pointer p, size_type) {
    }

    //Use placement new to engage the constructor
    void construct(pointer p, const T& val) {
        new((void*)p) T(val);
    }
    void destroy(pointer p) {
        ((T*)p)->~T();
    }

    size_type max_size() const throw() {
        return size;
    }
    template<class U> struct rebind {
        typedef array_allocator<U,size> other;
    };
};

template<class T,size_t size> struct array_vector : std::vector<T, array_allocator<T,size> > {
};

#if defined(__INTEL_COMPILER) || defined(__GNUC__) || (_MSC_VER>=1300)
template<class T,size_t a> struct allocator_traits<array_allocator<T,a> > {
    enum {is_static=true};
};
#endif

#endif
