// otus.lessons.06.01.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <map>
#include <boost/program_options.hpp>

#ifndef __unix__
#define BOOST_TEST_MODULE test
#define BOOST_TEST_NO_MAIN
#include <boost/test/unit_test.hpp>
#endif

#include "allocator.h"
#include "container.h"

#ifdef __unix__
#include "../bin/version.h"
#else
#include "version.h"
#endif

template<typename T>
T fact(T n)
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

struct std_custom {
  std_custom(std::string const& val): value(val) {}
  std::string value;
};
void validate(boost::any& v,
              const std::vector<std::string>& values,
              std_custom* target_type, int)
{
    boost::program_options::validators::check_first_occurrence(v);
    const std::string& s = boost::program_options::validators::get_single_string(values);

    if(!(s.empty() || s == "std" || s == "custom"))
        throw boost::program_options::validation_error(boost::program_options::validation_error::invalid_option_value);
}

int main(int argc, char** argv)
{
    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
    ("help,h", "print usage message")
#ifndef __unix__
    ("suite,s", "do tests suite")
#endif
    ("version,v", "print version number")
    ("allocator,a", boost::program_options::value<std_custom>(), "[std|custom]")
    ("container,c", boost::program_options::value<std_custom>(), "[std|custom]");

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
    }
    else if (vm.count("version")) {
        std::cout << "Build version: " << version() << std::endl;
        std::cout << "Boost version: " << (BOOST_VERSION / 100000) << '.' << (BOOST_VERSION / 100 % 1000) << '.' << (BOOST_VERSION % 100) << std::endl;
    }
    else if (vm.count("suite")) {
        return boost::unit_test::unit_test_main(&init_unit_test, argc, argv);
    } else {
        std::string allocator = vm.count("allocator") ? vm["allocator"].as<std_custom>().value : "";
        std::string container = vm.count("container") ? vm["container"].as<std_custom>().value : "";
        if((allocator.empty() || allocator == "std") && (container.empty() || container == "std"))
            try_std_allocator_std_container();
        if((allocator.empty() || allocator == "custom") && (container.empty() || container == "std"))
            try_custom_allocator_std_container();
        if((allocator.empty() || allocator == "std") && (container.empty() || container == "custom"))
            try_std_allocator_custom_container();
        if((allocator.empty() || allocator == "custom") && (container.empty() || container == "custom"))
            try_custom_allocator_custom_container();
    }

#ifndef __unix__
    getchar();
#endif

    return 0;
}
