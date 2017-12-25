// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef __unix__

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#endif

#include <sstream>
#include <boost/type_index.hpp>

template<class T>
std::string pretty_type_name()
{
    std::stringstream ss;
    ss << '[' << sizeof(T) << "] " << boost::typeindex::type_id_with_cvr<T>().pretty_name();
    return ss.str();
}