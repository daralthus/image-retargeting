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
#define ASSERT(param)
#endif

#include "Config.h"