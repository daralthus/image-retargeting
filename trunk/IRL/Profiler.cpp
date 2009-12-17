#include "Includes.h"
#include "Profiler.h"
#include "Threading.h"

namespace IRL
{
    namespace Tools
    {
        static ThreadLocal<Profiler*> g_LastProfiler;

        class Profiler::Result
        {
        public:
            Result(const std::string& name) 
                : _name(name), _totalTime(0), _timeInside(0)
            {
            }

            ~Result()
            {
                for (unsigned int i = 0; i < _childs.size(); i++)
                    delete _childs[i];
            }

            void SetTotalTime(clock_t totalTime)
            {
                _totalTime = totalTime;
            }

            clock_t GetTotalTime() const
            {
                return _totalTime;
            }

            void AddChild(Result* child)
            {
                _childs.push_back(child);
                _timeInside += child->GetTotalTime();
            }

            void Report(const std::string& prefix)
            {
                std::cout << prefix << _name << " : " 
                    << (_totalTime * 1000 / CLOCKS_PER_SEC) << "("
                    << ((_totalTime - _timeInside) * 1000 / CLOCKS_PER_SEC) << ")\n";
                if (!_childs.empty())
                {
                    std::string newPrefix = prefix + "  ";
                    for (unsigned int i = 0; i < _childs.size(); i++)
                        _childs[i]->Report(newPrefix);
                    std::cout << prefix << "---\n";
                }
            }

        private:
            std::string _name;
            std::vector<Result*> _childs;
            clock_t _totalTime;
            clock_t _timeInside;
        };

        Profiler::Profiler(const std::string& name) : 
            _startTime(clock())
        {
            _result = new Result(name);
            if (g_LastProfiler.IsSet())
                _parent = g_LastProfiler.Get();
            else
                _parent = NULL;
            g_LastProfiler.Set(this);
        }

        Profiler::~Profiler()
        {
            ASSERT(g_LastProfiler.Get() == this);
            _result->SetTotalTime(clock() - _startTime);
            g_LastProfiler.Set(_parent);
            if (_parent != NULL)
                _parent->_result->AddChild(_result);
            else
            {
                _result->Report("");
                delete _result;
            }
        }
    }
}