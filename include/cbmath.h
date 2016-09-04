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