#include "Profiler.h"

namespace IRL
{
    namespace Tools
    {
        Profiler::Profiler(const char* name) : _name(name), _start(clock())
        { 
        }

        Profiler::~Profiler()
        {
            std::cout << "Execution time of '" << _name << "' is " 
                << ((clock() - _start) * 1000 / CLOCKS_PER_SEC) << " ms.\n";
        }
    }
}