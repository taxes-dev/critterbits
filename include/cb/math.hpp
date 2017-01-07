#pragma once
#ifndef CBMATH_HPP
#define CBMATH_HPP

#include <type_traits>

namespace Critterbits {
template <typename T>
inline void SetBitMask(T * value, const T mask) {
    static_assert(std::is_integral<T>() || std::is_enum<T>(), "SetBitMask type must be integral or enum");
    *value = static_cast<T>(*value | mask);
}

template <typename T>
inline void UnsetBitMask(T * value, const T mask) {
    static_assert(std::is_integral<T>() || std::is_enum<T>(), "UnsetBitMask type must be integral or enum");
    *value = static_cast<T>(*value & ~mask);
}

template <typename T>
inline bool TestBitMask(const T value, const T mask) {
    static_assert(std::is_integral<T>() || std::is_enum<T>(), "TestBitMask type must be integral or enum");
    return (value & mask) == mask;
}

template<class T>
inline T operator | (T lhs, T rhs) {
  static_assert(std::is_enum<T>(), "Bit operator | accepts only enum");
  return static_cast<T>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

template<class T>
inline T& operator |= (T& lhs, T rhs) {
    static_assert(std::is_enum<T>(), "Bit operator |= accepts only enum");
    lhs = static_cast<T>(static_cast<int>(lhs) | static_cast<int>(rhs));
    return lhs;
}

template<class T>
inline T operator & (T lhs, T rhs) {
    static_assert(std::is_enum<T>(), "Bit operator & accepts only enum");
    return static_cast<T>(static_cast<int>(lhs) & static_cast<int>(rhs));
}

template<class T>
inline T& operator &= (T& lhs, T rhs) {
    static_assert(std::is_enum<T>(), "Bit operator &= accepts only enum");
    lhs = static_cast<T>(static_cast<int>(lhs) & static_cast<int>(rhs));
    return lhs;
}

template<class T>
inline T operator ~ (T value) {
    static_assert(std::is_enum<T>(), "Bit operator ~ accepts only enum");
    return static_cast<T>(~static_cast<int>(value));
}


inline int Clamp(int value, int min, int max) {
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

inline float Clampf(float value, float min, float max) {
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

// Source: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
//  compute the next highest power of 2 of 32-bit v
inline unsigned int NextPowerOf2(unsigned int v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return ++v;
}
}
#endif