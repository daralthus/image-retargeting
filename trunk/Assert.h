#pragma once

#include <assert.h>

#ifdef _DEBUG
#define ASSERT assert
#else
#define ASSERT(param)
#endif