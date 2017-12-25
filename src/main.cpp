// otus.lessons.06.01.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <map>
#include <boost/program_options.hpp>

#include "allocator.h"
#include "container.h"

template<typename T>
constexpr T fact(T n)
{
    return n > 1 ? n * fact(n-1) : 1;
}

void try_std_allocator_std_container()
{
    auto m = std::map<size_t, size_t> {};
    for(size_t i = 0; i < 10; ++i)
        m[i] = fact(i);
    for(auto v : m)
        std::cout << v.first << ' ' << v.second << std::endl;
}

void try_custom_allocator_std_container()
{
    auto m = std::map<size_t, size_t, std::less<size_t>, custom_allocator<std::pair<size_t, size_t>, 10> > {};
    for(size_t i = 0; i < 10; ++i)
        m[i] = fact(i);
    for(auto v : m)
        std::cout << v.first << ' ' << v.second << std::endl;
}

void try_std_allocator_custom_container()
{
    auto ll = linked_list<int, std::allocator<int> > {};
    for(size_t i = 0; i < 10; ++i)
        ll.push_back(i);
    for(auto v : ll)
        std::cout << v << std::endl;
}

void try_custom_allocator_custom_container()
{
    auto ll = linked_list<int, custom_allocator<int, 10> > {};
    for(size_t i = 0; i < 10; ++i)
        ll.push_back(i);
    for(auto v : ll)
        std::cout << v << std::endl;
}

int main()
{
    try_std_allocator_std_container();
    try_custom_allocator_std_container();
    try_std_allocator_custom_container();
    try_custom_allocator_custom_container();

#ifndef __unix__
    getchar();
#endif

    return 0;
}
