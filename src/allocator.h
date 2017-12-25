#pragma once

#include "page.h"

template<typename T, size_t size>
struct custom_allocator {
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    template< class U > struct rebind {
        typedef custom_allocator<U, size> other;
    };

    custom_allocator() {}
    custom_allocator(const custom_allocator<T, size> &a) {}
    template<typename U, size_t ps>
    custom_allocator(const custom_allocator<U, ps> &a) {}
    ~custom_allocator() {}

    T* allocate(size_t n)
    {
        page* data = pages.get_page(sizeof(T), size);
        if (!data)
            pages.add_page(data = new page_impl<T, size>());
        return reinterpret_cast<T*>(data->allocate(n));
    }
    void deallocate(T* p, size_t n)
    {
        page* data = pages.get_page(sizeof(T), size);
        if (data)
            data->deallocate(reinterpret_cast<void*>(p), n);
    }

    template <typename U, typename ... Args>
    void construct(U* p, Args&& ... args)
    {
        new ((void *)p) U(std::forward<Args>(args)...);
    }
    template <typename U>
    void destroy(U* p)
    {
        p->~U();
    }
};
