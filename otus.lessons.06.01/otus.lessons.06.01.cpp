// otus.lessons.06.01.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <map>
#include <vector>

template <class T>
struct custom_allocator
{
    using value_type = T;

    struct page {
        T* data;
        bool* usage;
        page* next;

        page(size_t size_, page* next_) : next(next_) 
        {
            data = reinterpret_cast<T*>(std::malloc(size_ * (sizeof(T) + 1)));
            if(!data)
                throw std::bad_alloc();

            usage = new bool[size_];
            for (size_t i = 0; i < size_; ++i)
                usage[i] = false;
        }
        ~page()
        {
            if (next != nullptr)
                delete next;
            std::free(reinterpret_cast<void*>(data));
            delete[] usage;
        }
    }* pages;

    size_t page_size;

    T* ps;
    bool* bs;

    custom_allocator() noexcept : pages(nullptr), page_size(5), ps(nullptr), bs(nullptr) {}
    template<typename U>
    custom_allocator(const custom_allocator<U>& ca) noexcept : pages(nullptr), page_size(5), ps(nullptr), bs(nullptr) {}
    ~custom_allocator() {
        if (pages != nullptr)
            delete pages;
    }

    T* allocate(size_t n)
    {
        std::cout << "allocate " << n;

        if (ps == nullptr) {
            ps = reinterpret_cast<T*>(std::malloc(page_size * sizeof(T)));
            bs = new bool[page_size];
            for (size_t i = 0; i < page_size; ++i)
                bs[i] = false;
        }
        if (!ps || n != 1)
            throw std::bad_alloc();

        T* pp = nullptr;
        bool* pb = bs;
        for (size_t i = 0; i < page_size; ++i) {
            if (!pb[i]) {
                pb[i] = true;
                pp = ps + i;
                break;
            }
        }
        if (pp == nullptr)
            throw std::bad_alloc();

//        T* pp = reinterpret_cast<T*>(std::malloc(sizeof(T)*n));
        std::cout << " " << pp << std::endl;
        return pp;

        bool has_room = false;
        size_t room_start = 0;
        page* current = pages;
        while (current != nullptr) {
            for (size_t i = 0, room_size = 0; i < sizeof(current->usage) && room_size < n; ++i) {
                if (current->usage[i]) {
                    has_room = false;
                }
                else {
                    if (!has_room) {
                        room_start = i;
                        room_size = 0;
                        has_room = true;
                    }
                    ++room_size;
                }
            }
            if (has_room)
                break;
            current = current->next;
        }
        if (!has_room) {
            size_t size = page_size > n ? page_size : n;
            pages = current = new page(size, pages);
            room_start = 0;
        }
        for (size_t i = 0; i < n; ++i)
            current->usage[i + room_start] = true;
        T* p = current->data + room_start;
        std::cout << " " << p << std::endl;
        return p;
    }

    void deallocate(T* p, std::size_t n)
    {
        std::cout << "deallocate " << n << " " << p << std::endl;
        size_t i = p - ps;
        bs[i] = false;
        bool b = bs[0] || bs[1] || bs[2] || bs[3] || bs[4];
        if (!b) {
            std::free(ps);
            delete[] bs;
            ps = nullptr;
            bs = nullptr;
        }
        return;

        page* current = pages;
        while (current != nullptr) {
            if (current->data <= p && p < current->data + sizeof(current->data))
                break;
            current = current->next;
        }
        if (current == nullptr)
            throw std::exception("unknown pointer");
        for (size_t i = p - current->data; i < n; ++i)
            current->usage[i] = false;
    }

    template <typename U, typename ... Args>
    void construct(U* p, Args&& ... args)
    {
        std::cout << "construct " << p << std::endl;
        new ((void *)p) U(std::forward<Args>(args)...);
    }
    void destroy(T* p)
    {
        std::cout << "destruct " << p << std::endl;
        p->~T();
    }
};

int main()
{
    {
//        auto ca = custom_allocator< std::pair<std::string, int> >{};
        auto m = std::map<std::string, int, std::less<std::string>, custom_allocator< std::pair<std::string, int> > >{};

        m["1"] = 1;

        for (const auto &i : m) {
            std::cout << "\"" << i.first.c_str() << "\": " << i.second << std::endl;
        }
    }
    getchar();
    return 0;
}
