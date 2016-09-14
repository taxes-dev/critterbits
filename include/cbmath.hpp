#pragma once
#ifndef CBMATH_HPP
#define CBMATH_HPP

namespace Critterbits {

template <typename T>
inline bool TestBitMask(const T value, const T mask) {
    return (value & mask) == mask;
}

template <int>
inline bool TestBitMask(const int, const int);

template <unsigned int>
inline bool TestBitMask(const unsigned int, const unsigned int);

inline int Clamp(int value, int min, int max) {
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