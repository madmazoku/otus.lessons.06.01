#pragma once

#include <iostream>
#include <memory>
#include <exception>
#include <algorithm>
#include <cstring>

struct page {
    page* next;
    void* data;
    void* usage;
    size_t alloc_size;

    virtual page* create() = 0;
    virtual size_t page_size() = 0;
    virtual size_t obj_size() = 0;

    page() : next(nullptr), data(nullptr), usage(nullptr), alloc_size(0)
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
            alloc_size = n > page_size() ? n : page_size();
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
            for (uint8_t* u = (uint8_t*)usage; u < (uint8_t*)usage + alloc_size; ++u) {
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
        else if ((uint8_t*)data <= (uint8_t*)p && (uint8_t*)p < (uint8_t*)data + alloc_size * obj_size())
            set_usage(((uint8_t*)p - (uint8_t*)data) / obj_size(), n, false);
        else
            next->deallocate(p, n);
    }

    void dump(std::ostream& os) {
        if (data != nullptr) {
            os << "[ " << obj_size() << " / " << page_size() << " ] ";
            for (uint8_t* u = (uint8_t*)usage; u < (uint8_t*)usage + alloc_size; ++u)
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

extern page_info pages;
