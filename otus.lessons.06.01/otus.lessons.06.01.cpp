// otus.lessons.06.01.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <memory>
#include <exception>

#include <boost/type_index.hpp>
/*
template<typename T>
struct logging_allocator {
    using value_type = T;

    T* allocate(size_t n)
    {
        return reinterpret_cast<T*>(std::malloc(n * sizeof(T)));
    }
    void deallocate(T* p, size_t n)
    {
        return std::free(p);
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
*/

template<class T>
struct pretty_type_name {
    std::string operator()() const
    {
        std::stringstream ss;
        ss << '[' << sizeof(T) << "] " << boost::typeindex::type_id_with_cvr<T>().pretty_name();
        return ss.str();
    }
};


template<typename T>
struct logging_allocator  {
    using value_type = T;

    struct pool {
        pool* next;
        T* pdata;
        bool* udata;
        size_t page_size;

        pool(size_t page_size_, pool* next_ = nullptr) : next(next_), pdata(nullptr), udata(nullptr), page_size(page_size_)
        {
            pdata = reinterpret_cast<T*>(std::malloc(page_size * sizeof(T)));
            if(!pdata)
                throw std::bad_alloc();

            udata = new bool[page_size];
            for(size_t i = 0; i < page_size; ++i)
                udata[i] = false;
        }
        ~pool()
        {
            if(next)
                delete next;

            if(pdata)
                std::free(pdata);

            for(size_t i = 0; i < page_size; ++i)
                if(udata[i])
                    throw std::runtime_error("has unfreed objects in the pool");
            if(udata)
                delete[] udata;
        }
    };

    pool* data;
    size_t page_size;

    logging_allocator(size_t page_size_ = 5) : data(nullptr), page_size(page_size_)
    {
    }
    template<typename U>
    logging_allocator(const logging_allocator<U> &ca)  : data(nullptr), page_size(ca.page_size)
    {
    }
    ~logging_allocator()
    {
        if(data != nullptr)
            delete data;
    }

    void reserve(size_t page_size_)
    {
        page_size = page_size_;
    }

    T* allocate(size_t n)
    {
        if(data == nullptr)
            data = new pool(page_size);

        bool has_room = false;
        size_t room_start = 0;
        pool* current = data;
        while(current != nullptr) {
            if(n < current->page_size) {
                size_t room_size = 0;
                for(size_t i = 0; i < current->page_size; ++i) {
                    if(current->udata[i])
                        has_room = false;
                    else {
                        if(!has_room) {
                            has_room = true;
                            room_size = 0;
                            room_start = i;
                        }
                        if(++room_size == n) {
                            break;
                        }
                    }
                }
                if(has_room && room_size == n) {
                    break;
                }
            }
            has_room = false;
            current = current->next;
        }

        if(!has_room) {
            size_t size = n > page_size ? n : page_size;
            data = current = new pool(size, data);
            room_start = 0;
        }

        for(size_t i = 0; i < n; ++i)
            current->udata[i + room_start] = true;

        auto p = current->pdata + room_start;

        return p;
    }
    void deallocate(T* p, size_t n)
    {
        pool* current = data;
        while(current != nullptr) {
            if(current->pdata <= p && p < current->pdata + current->page_size)
                break;
            current = current->next;
        }
        if(current == nullptr)
            throw std::runtime_error("unallocated ponter found");

        size_t room_start = p - current->pdata;
        for(size_t i = 0; i < n; ++i)
            current->udata[i + room_start] = false;
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


int main()
{
    {
        logging_allocator< std::pair< const std::string, int > > la;

        std::cout << "set reserve 4" << std::endl;
        la.reserve(4);

        auto m = std::map<const std::string, int, std::less<const std::string>, decltype(la) >({}, la);

        std::cout << "add 2" << std::endl;
        m["2"] = 1;

        std::cout << "add 3" << std::endl;
        m["3"] = 1;

        std::cout << "add 1" << std::endl;
        m["1"] = 1;

        std::cout << "add 4" << std::endl;
        m["4"] = 1;

        std::cout << "add 5" << std::endl;
        m["5"] = 1;

        std::cout << "add 6" << std::endl;
        m["6"] = 1;

        std::cout << "add 7" << std::endl;
        m["7"] = 1;

        std::cout << "remove 3" << std::endl;
        m.erase("3");

        std::cout << "add 8" << std::endl;
        m["8"] = 1;

        std::cout << "add 9" << std::endl;
        m["9"] = 1;

        std::cout << "add 0" << std::endl;
        m["0"] = 1;

        std::cout << "print pool" << std::endl;
        m.get_allocator().print(std::cout);

        std::cout << "print content" << std::endl;
        for(const auto &p : m)
            std::cout << '"' << p.first << "\": " << p.second << std::endl;

        std::cout << "leave scope" << std::endl;
    }

#ifndef __unix__
    getchar();
#endif

    return 0;
}
