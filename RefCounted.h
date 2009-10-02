#pragma once

namespace IRL
{
    template<class T>
    class RefCounted
    {
    public:
        RefCounted() : _refs(1)
        { }

        void Acquire() const
        {
            _refs++;
        }
        void Release() const
        {
            --_refs;
            if (_refs == 0)
                T::Delete((T*)this);
        }
        int32_t GetRefs() const 
        { 
            return _refs;
        }

    private:
        mutable int32_t _refs;
    };
}