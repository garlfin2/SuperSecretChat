#pragma once

#include "Math.h"

template<class T>
struct vec<2, T>
{
    constexpr vec() = default;
    constexpr vec(T x_) : x(x_), y(x_) {}
    constexpr vec(T x_, T y_) : x(x_), y(y_) {}
    constexpr vec(const vec&) = default;

    template<size_t WIDTH, class NEW_T> requires (WIDTH <= 4)
    constexpr explicit operator vec<WIDTH, NEW_T>() const
    {
        vec<WIDTH, NEW_T> result;
        for (size_t i = 0; i < WIDTH; i++)
            result[i] = (NEW_T)v[i];
        return result;
    }

    T& operator[](size_t i) { return v[i]; }
    T operator[](size_t i) const { return v[i]; }

    constexpr bool operator==(const vec& b) const { return v == b.v; }

    union
    {
        std::array<T, 2> v{};
        struct
        {
            T x, y;
        };
    };
};

using vec2 = vec<2, float>;
using uvec2 = vec<2, uint32_t>;
using ivec2 = vec<2, int32_t>;
using u16vec2 = vec<2, uint16_t>;
using i16vec2 = vec<2, int16_t>;