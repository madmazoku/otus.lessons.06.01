#pragma once

#include "page.h"

template<typename T, size_t size>
struct logging_allocator {
    using value_type = T;
    template< class U > struct rebind { typedef logging_allocator<U, size> other; };

    logging_allocator()
    {
        std::cout << "allocator() " << this << " " << pretty_type_name<T>()() << std::endl;
    }
    logging_allocator(const logging_allocator<T, size> &a)
    {
        std::cout << "allocator(allocator) " << this << " " << &a << " " << pretty_type_name<T>()() << std::endl;
    }
    template<typename U, size_t ps>
    logging_allocator(const logging_allocator<U, ps> &a)
    {
        std::cout << "allocator(allocator<U>) " << this << " " << &a << " " << pretty_type_name<T>()() << std::endl;
        std::cout << "\tU = " << pretty_type_name<U>()() << std::endl;
    }
    ~logging_allocator()
    {
        std::cout << "~allocator() " << this << " " << pretty_type_name<T>()() << std::endl;
    }

    T* allocate(size_t n)
    {
        page* data = pages.get_page(sizeof(T), size);
        if (!data)
            pages.add_page(data = new page_impl<T, size>());
        auto p = reinterpret_cast<T*>(data->allocate(n));

//        auto p = reinterpret_cast<T*>(std::malloc(n * sizeof(T)));
        std::cout << "allocate() " << this << " " << p << " " << n << " " << pretty_type_name<T>()() << std::endl;
        pages.dump(std::cout);
        return p;
    }
    void deallocate(T* p, size_t n)
    {
        page* data = pages.get_page(sizeof(T), size);
        if (data)
            data->deallocate(reinterpret_cast<void*>(p), n);
        pages.dump(std::cout);
        std::cout << "deallocate() " << this << " " << p << " " << n << " " << pretty_type_name<T>()() << std::endl;
//        std::free(p);
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
