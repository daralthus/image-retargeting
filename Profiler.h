#pragma once

#include <time.h>
#include <string>

namespace IRL
{
    namespace Tools
    {
        class Profiler
        {
        public:
            Profiler(const std::string& name);
            ~Profiler();

        private:
            clock_t _startTime;
            Profiler* _parent;

            class Result;
            Result* _result;
        };
    }
}