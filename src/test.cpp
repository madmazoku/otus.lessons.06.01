// otus.lessons.06.01.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#ifdef __unix__
#define BOOST_TEST_MODULE TestMain
#endif
#include <boost/test/unit_test.hpp>

#include <map>
#include "allocator.h"
#include "container.h"

#ifdef __unix__
#include "../bin/version.h"
#else
#include "version.h"
#endif

BOOST_AUTO_TEST_SUITE( test_suite )

BOOST_AUTO_TEST_CASE( test_version )
{
    BOOST_CHECK_GT(version(), 0);
}

BOOST_AUTO_TEST_CASE( test_page )
{
    auto p1 = new page_impl<uint8_t, 2>;
    auto p2 = new page_impl<uint16_t, 2>;
    auto p3 = new page_impl<uint32_t, 2>;

    BOOST_CHECK( p1->obj_size() == sizeof(uint8_t) );
    BOOST_CHECK( p1->page_size() == 2 );

    BOOST_CHECK( p2->obj_size() == sizeof(uint16_t) );
    BOOST_CHECK( p2->page_size() == 2 );

    BOOST_CHECK( p3->obj_size() == sizeof(uint32_t) );
    BOOST_CHECK( p3->page_size() == 2 );

    uint8_t* b1 = reinterpret_cast<uint8_t*>(p3->allocate(1));
    uint8_t* b2 = reinterpret_cast<uint8_t*>(p3->allocate(2));
    uint8_t* b3 = reinterpret_cast<uint8_t*>(p3->allocate(3));

    b1[0] = 0x01;
    b2[0] = 0x02;
    b2[1] = 0x03;
    b3[0] = 0x04;
    b3[1] = 0x05;
    b3[2] = 0x06;

    p3->deallocate(reinterpret_cast<void*>(b1), 1);
    p3->deallocate(reinterpret_cast<void*>(b2), 2);
    p3->deallocate(reinterpret_cast<void*>(b3), 3);

    b1 = reinterpret_cast<uint8_t*>(p3->allocate(1));
    b2 = reinterpret_cast<uint8_t*>(p3->allocate(2));
    b3 = reinterpret_cast<uint8_t*>(p3->allocate(3));

    BOOST_CHECK_EQUAL(b1[0], 0x01);
    BOOST_CHECK_EQUAL(b2[0], 0x02);
    BOOST_CHECK_EQUAL(b2[1], 0x03);
    BOOST_CHECK_EQUAL(b3[0], 0x04);
    BOOST_CHECK_EQUAL(b3[1], 0x05);
    BOOST_CHECK_EQUAL(b3[2], 0x06);

    delete p3;
    delete p2;
    delete p1;
}

BOOST_AUTO_TEST_CASE( test_pages )
{
    auto p1 = new page_impl<uint8_t, 51>;
    auto p2 = new page_impl<uint8_t, 52>;
    auto p3 = new page_impl<uint8_t, 53>;

    BOOST_CHECK( pages.get_page(p1->obj_size(), p1->page_size()) == nullptr );
    BOOST_CHECK( pages.get_page(p2->obj_size(), p2->page_size()) == nullptr );
    BOOST_CHECK( pages.get_page(p3->obj_size(), p3->page_size()) == nullptr );

    pages.add_page(p1);
    BOOST_CHECK( pages.get_page(p1->obj_size(), p1->page_size()) == p1 );
    BOOST_CHECK( pages.get_page(p2->obj_size(), p2->page_size()) == nullptr );
    BOOST_CHECK( pages.get_page(p3->obj_size(), p3->page_size()) == nullptr );

    pages.add_page(p2);
    BOOST_CHECK( pages.get_page(p1->obj_size(), p1->page_size()) == p1 );
    BOOST_CHECK( pages.get_page(p2->obj_size(), p2->page_size()) == p2 );
    BOOST_CHECK( pages.get_page(p3->obj_size(), p3->page_size()) == nullptr );

    delete p3;
}

BOOST_AUTO_TEST_CASE( test_allocator )
{
    struct test_data {
        uint32_t data;
        test_data(uint32_t data_ = 0) : data(data_) {}
    };

    auto a = custom_allocator<test_data, 3> {};

    test_data* b = a.allocate(2);
    a.construct(b, 0x01);
    a.construct(b+1, 0x02);

    BOOST_CHECK_EQUAL(b[0].data, 0x01);
    BOOST_CHECK_EQUAL(b[1].data, 0x02);

    a.destroy(b+1);
    a.destroy(b);
    a.deallocate(b, 2);
}

BOOST_AUTO_TEST_CASE( test_allocator_std_container )
{
    auto m = std::map<uint32_t, uint32_t, std::less<uint32_t>, custom_allocator< std::pair< uint32_t, uint32_t >, 4 > > {};

    m[1] = 1;
    m[2] = 2;
    m[3] = 3;

    auto i1 = m.find(1);
    BOOST_CHECK(i1 != m.end());
    BOOST_CHECK((*i1).first == 1);
    BOOST_CHECK((*i1).second == 1);

    auto i2 = m.find(2);
    BOOST_CHECK(i2 != m.end());
    BOOST_CHECK((*i2).first == 2);
    BOOST_CHECK((*i2).second == 2);

    auto i3 = m.find(3);
    BOOST_CHECK(i3 != m.end());
    BOOST_CHECK((*i3).first == 3);
    BOOST_CHECK((*i3).second == 3);

    m.erase(2);
    BOOST_CHECK(m.find(2) == m.end());
}

template<typename Alloc>
void test_container()
{
    auto ll = linked_list<uint32_t, Alloc > {};

    size_t count = 0;
    ll.push_front(++count);
    ll.push_front(++count);
    ll.push_front(++count);

    BOOST_CHECK_EQUAL(ll.size(), 3);

    for(auto v : ll)
        BOOST_CHECK_EQUAL(v, count--);

    auto it = ll.begin();
    BOOST_CHECK_EQUAL(*it, 3);
    ++it;
    BOOST_CHECK_EQUAL(*it, 2);
    ll.erase(it);
    BOOST_CHECK_EQUAL(ll.size(), 2);

    it = ll.begin();
    BOOST_CHECK_EQUAL(*it, 3);
    ++it;
    BOOST_CHECK_EQUAL(*it, 1);
    ll.erase(it);
    BOOST_CHECK_EQUAL(ll.size(), 1);

    it = ll.begin();
    BOOST_CHECK_EQUAL(*it, 3);
    ll.erase(it);
    BOOST_CHECK_EQUAL(ll.size(), 0);
    BOOST_CHECK(ll.begin() == ll.end());

    ll.push_back(++count);
    ll.push_back(++count);
    ll.push_back(++count);
    BOOST_CHECK_EQUAL(ll.size(), 3);
    for(auto v : ll)
        BOOST_CHECK_EQUAL(v, 3-(--count));

    ll.clear();
    BOOST_CHECK_EQUAL(ll.size(), 0);
    BOOST_CHECK(ll.begin() == ll.end());
}

BOOST_AUTO_TEST_CASE( test_allocator_container )
{
    test_container<custom_allocator< uint32_t, 5 > >();
}

BOOST_AUTO_TEST_CASE( test_std_allocator_container )
{
    test_container<std::allocator< uint32_t > >();
}

BOOST_AUTO_TEST_SUITE_END()
