#pragma once

// CRT includes
#include <assert.h>
#include <math.h>
#include <time.h>

// Common STL includes 
#include <vector>
#include <list>
#include <set>
#include <string>
#include <sstream>
#include <iostream>

// Portable stdint.h
#include "pstdint.h"

#ifdef _DEBUG
#define ASSERT assert
#else
#ifdef _MSC_VER
#define ASSERT(param) (__assume(param))
#else
#define ASSERT(param)
#endif
#endif

template<class T>
const T& Minimum(const T& l, const T& r) // because MSVC does not know std::min
{
    return (l < r) ? l : r;
}

#include "Config.h"