// otus.lessons.06.01.cpp : Defines the entry point for the console application.
//

#define BOOST_TEST_MODULE TestMain
#include <boost/test/unit_test.hpp>

#include "stdafx.h"

#include <map>
#include "allocator.h"

BOOST_AUTO_TEST_SUITE( test_suite )

BOOST_AUTO_TEST_CASE( test_version )
{
    BOOST_CHECK_GT( 1, 0 );
}

BOOST_AUTO_TEST_SUITE_END()
