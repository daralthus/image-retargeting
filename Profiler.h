#pragma once

#include <iostream>
#include <time.h>

namespace IRL
{
    namespace Tools
    {
        class Profiler
        {
        public:
            Profiler(const char* name);
            ~Profiler();
        private:
            const char* _name;
            clock_t _start;
        };
    }
}