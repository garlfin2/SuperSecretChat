#pragma once

#include "Math.h"

template<class T>
struct vec<3, T>
{
    constexpr vec() = default;
    constexpr vec(T x_) : x(x_), y(x_), z(x_) {}
    constexpr vec(T x_, T y_, T z_) : x(x_), y(y_), z(z_) {}
    constexpr vec(const vec&) = default;

    constexpr vec operator-() const { return { -x, -y, -z }; }

    template<size_t WIDTH, class NEW_T> requires (WIDTH <= 3)
    constexpr explicit operator vec<WIDTH, NEW_T>() const
    {
        vec<WIDTH, NEW_T> result;
        for (size_t i = 0; i < WIDTH; i++)
            result[i] = (NEW_T)v[i];
        return result;
    }

    constexpr bool operator==(const vec& b) const { return v == b.v; }

    constexpr T& operator[](size_t i) { return v[i]; }
    constexpr T operator[](size_t i) const { return v[i]; }

    union
    {
        std::array<T, 3> v{};
        struct
        {
            T x, y, z;
        };
    };
};

using vec3 = vec<3, float>;