// otus.lessons.06.01.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <memory>
#include <exception>
#include <algorithm>

#include <boost/type_index.hpp>

template<class T>
struct pretty_type_name {
    std::string operator()() const
    {
        std::stringstream ss;
        ss << '[' << sizeof(T) << "] " << boost::typeindex::type_id_with_cvr<T>().pretty_name();
        return ss.str();
    }
};

struct page {
    page* next;
    void* data;
    void* usage;

    virtual page* create() = 0;
    virtual size_t page_size() = 0;
    virtual size_t obj_size() = 0;

    page() : next(nullptr), data(nullptr), usage(nullptr)
    {
    }
    ~page()
    {
        if (next)
            delete next;
        if (data)
            std::free(data);
        if (usage)
            std::free(usage);
    }

    void set_usage(size_t start, size_t n, uint8_t claim)
    {
        if(n > 0)
            std::memset((void*)((uint8_t*)usage + start), claim, n);
    }

    void* allocate(size_t n)
    {
        if (data == nullptr) {
            size_t alloc_size = n > page_size() ? n : page_size();
            data = std::malloc(alloc_size * obj_size());
            usage = reinterpret_cast<uint8_t*>(std::malloc(alloc_size * obj_size()));
            next = create();
            if (!data || !usage || !next)
                throw std::bad_alloc();
            set_usage(0, n, 0x01);
            set_usage(n, alloc_size - n, 0x00);
            return data;
        }

        if (n < page_size()) {
            size_t start_room = 0;
            size_t size_room = 0;
            bool has_room = false;
            for (uint8_t* u = (uint8_t*)usage; u < (uint8_t*)usage + sizeof(usage); ++u) {
                if (*u == 0x00) {
                    if (!has_room) {
                        has_room = true;
                        size_room = 0;
                        start_room = u - (uint8_t*)usage;
                    }
                    if (++size_room == n) {
                        set_usage(start_room, n, 0x01);
                        return (uint8_t*)data + start_room * obj_size();
                    }
                }
                else
                    has_room = false;
            }
        }

        return next->allocate(n);
    }

    void deallocate(void* p, size_t n)
    {
        if (data == nullptr)
            return;
        else if ((uint8_t*)data <= (uint8_t*)p && (uint8_t*)p < (uint8_t*)data + sizeof(usage) * obj_size())
            set_usage(((uint8_t*)p - (uint8_t*)data) / obj_size(), n, false);
        else
            next->deallocate(p, n);
    }

    void dump(std::ostream& os) {
        if (data != nullptr) {
            os << "[ " << obj_size() << " / " << page_size() << " ] ";
            for (uint8_t* u = (uint8_t*)usage; u < (uint8_t*)usage + page_size(); ++u)
                os << " " << (*u != 0x00 ? "+" : "-");
            os << "\n";
            next->dump(os);
        }
    }
};

template<typename T, size_t size = 11> 
struct page_impl : public page
{
    T* allocate(size_t n) 
    {
        return reinterpret_cast<T*>(allocate(n));
    }
    void deallocate(T* p, size_t n)
    {
        deallocate(reinterpret_cast<void*>(p), n);
    }
    page* create() override { return new page_impl<T, size>; }
    size_t page_size() override { return size; };
    size_t obj_size() override { return  sizeof(T); }
};

struct page_info {
    page* data;
    page_info* next;

    page_info() : data(nullptr), next(nullptr)
    {
    }
    ~page_info()
    {
        if(data)
            delete data;
        if(next)
            delete next;
    }
    page* get_page(size_t obj_size, size_t page_size) {
        if (data == nullptr)
            return nullptr;
        else if(obj_size == data->obj_size() && page_size == data->page_size())
            return data;
        else
            return next->get_page(obj_size, page_size);
    }
    void add_page(page* data_) {
        if (data == nullptr) {
            data = data_;
            next = new page_info;
        } else
            next->add_page(data_);
    }

    void dump(std::ostream &os) {
        if (data != nullptr) {
            data->dump(os);
            next->dump(os);
        }
        else
            os << std::endl;
    }
};

page_info pages;

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

int main()
{
    {
        //        logging_allocator< std::pair< const std::string, int >, 4 > la{};
        auto m = std::map<const std::string, int, std::less<const std::string>, logging_allocator< std::pair< const std::string, int >, 4 > >{};
        pages.dump(std::cout);

        std::cout << "add 2" << std::endl;
        m["2"] = 1;
        pages.dump(std::cout);

        std::cout << "add 3" << std::endl;
        m["3"] = 1;
        pages.dump(std::cout);

        std::cout << "add 1" << std::endl;
        m["1"] = 1;
        pages.dump(std::cout);

        std::cout << "add 4" << std::endl;
        m["4"] = 1;
        pages.dump(std::cout);

        std::cout << "add 5" << std::endl;
        m["5"] = 1;
        pages.dump(std::cout);

        std::cout << "add 6" << std::endl;
        m["6"] = 1;
        pages.dump(std::cout);

        std::cout << "add 7" << std::endl;
        m["7"] = 1;
        pages.dump(std::cout);

        std::cout << "remove 3" << std::endl;
        m.erase("3");
        pages.dump(std::cout);

        std::cout << "add 8" << std::endl;
        m["8"] = 1;
        pages.dump(std::cout);

        std::cout << "add 9" << std::endl;
        m["9"] = 1;
        pages.dump(std::cout);

        std::cout << "add 0" << std::endl;
        m["0"] = 1;
        pages.dump(std::cout);

        std::cout << "print content" << std::endl;
        for (const auto &p : m)
            std::cout << '"' << p.first << "\": " << p.second << std::endl;

        std::cout << "leave scope" << std::endl;
    }

#ifndef __unix__
    getchar();
#endif

    return 0;
}
