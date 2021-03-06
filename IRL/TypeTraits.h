#pragma once

namespace IRL
{
    template<class T>
    struct TypeTraits;

    // uint8_t 
    template<>
    struct TypeTraits<uint8_t>
    {
        typedef uint32_t LargerType;
        static uint8_t MaxValue() { return UINT8_MAX; }
        template<class Type>
        static uint8_t Normalize(Type from, Type minValue, Type maxValue)
        {
            int res = (int)((from - minValue) * MaxValue() / (maxValue - minValue));
            if (res < 0) return 0;
            if (res > MaxValue()) return MaxValue();
            return (uint8_t)res;
        }
        template<class To>
        static To Denormalize(uint8_t from, To minValue, To maxValue)
        {
            return (To)(from * (maxValue - minValue) / MaxValue()) + minValue;
        }
        static const bool IsInteger = true;
    };

    // uint16_t
    template<>
    struct TypeTraits<uint16_t>
    {
        typedef uint64_t LargerType;
        static uint16_t MaxValue() { return UINT16_MAX; }
        template<class Type>
        static uint16_t Normalize(Type from, Type minValue, Type maxValue)
        {
            int res = (int)((from - minValue) * MaxValue() / (maxValue - minValue));
            if (res < 0) return 0;
            if (res > MaxValue()) return MaxValue();
            return (uint16_t)res;
        }
        template<class To>
        static To Denormalize(uint16_t from, To minValue, To maxValue)
        {
            return (To)(from * (maxValue - minValue) / UINT16_MAX) + minValue;
        }
        static const bool IsInteger = true;
    };

    // uint32_t
    template<>
    struct TypeTraits<uint32_t>
    {
        typedef uint64_t LargerType;
        static uint32_t MaxValue() { return UINT32_MAX; }
        template<class Type>
        static uint32_t Normalize(Type from, Type minValue, Type maxValue)
        {
            int64_t res = (int)((from - minValue) * MaxValue() / (maxValue - minValue));
            if (res < 0) return 0;
            if (res > MaxValue()) return MaxValue();
            return (uint32_t)res;
        }
        template<class To>
        static To Denormalize(uint32_t from, To minValue, To maxValue)
        {
            return (To)(from * (maxValue - minValue) / UINT32_MAX) + minValue;
        }
        static const bool IsInteger = true;
    };

    // uint64_t
    template<>
    struct TypeTraits<uint64_t>
    {
        typedef uint64_t LargerType;
        static uint64_t MaxValue() { return UINT64_MAX; }
        template<class Type>
        static uint64_t Normalize(Type from, Type minValue, Type maxValue)
        {
            int64_t res = (int64_t)((from - minValue) * MaxValue() / (maxValue - minValue));
            if (res < 0) return 0;
            return (uint64_t)res;
        }
        template<class To>
        static To Denormalize(uint64_t from, To minValue, To maxValue)
        {
            return (To)(from * (maxValue - minValue) / UINT64_MAX) + minValue;
        }
        static bool IsInteger() { return true; }
    };

    // float
    template<>
    struct TypeTraits<float>
    {
        typedef float LargerType;
        static float MaxValue() { return 1.0f; }
        template<class Type>
        static float Normalize(Type from, Type minValue, Type maxValue)
        {
            return (float)((from - minValue) * MaxValue() / (maxValue - minValue));
        }
        template<class To>
        static To Denormalize(float from, To minValue, To maxValue)
        {
            return (To)(from * (maxValue - minValue) / MaxValue()) + minValue;
        }
        static const bool IsInteger = false;
    };

    // double
    template<>
    struct TypeTraits<double>
    {
        typedef double LargerType;
        static double MaxValue() { return 1.0; }
        template<class Type>
        static double Normalize(Type from, Type minValue, Type maxValue)
        {
            return (double)((from - minValue) * MaxValue() / (maxValue - minValue));
        }
        template<class To>
        static To Denormalize(double from, To minValue, To maxValue)
        {
            return (To)(from * (maxValue - minValue) / MaxValue()) + minValue;
        }
        static const bool IsInteger = false;
    };
}