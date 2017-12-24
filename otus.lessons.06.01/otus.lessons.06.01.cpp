// otus.lessons.06.01.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <map>
#include "allocator.h"

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
        size_t count = 0;
        for (const auto &p : m) {
            std::cout << "[ " << count << " ]: \"" << p.first << "\": " << p.second << std::endl;
            if(++count == 20)
                break;
        }

        std::cout << "leave scope" << std::endl;
    }

#ifndef __unix__
    getchar();
#endif

    return 0;
}
