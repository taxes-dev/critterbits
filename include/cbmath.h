#pragma once
#ifndef CBMATH_H
#define CBMATH_H

namespace Critterbits {

template <typename T>
inline bool TestBitMask(T value, T mask) {
    return (value & mask) == mask;
}

template <int>
inline bool TestBitMask(int, int);

template <unsigned int>
inline bool TestBitMask(unsigned int, unsigned int);

inline int Clamp(int value, int min, int max) {
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}
}
#endif