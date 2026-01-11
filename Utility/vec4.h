#pragma once

#include "Math.h"

template<class T>
struct vec<4, T>
{
    constexpr vec() = default;
    constexpr vec(T x_) : x(x_), y(x_), z(x_) {}
    constexpr vec(T x_, T y_, T z_, T w_) : x(x_), y(y_), z(z_), w(w_) {}
    constexpr vec(const vec<3, T>& vec, T w_) : x(vec[0]), y(vec[1]), z(vec[2]), w(w_) {}
    constexpr vec(const vec<2, T>& lower, const vec<2, T>& upper) : x(lower[0]), y(lower[1]), z(upper[0]), w(upper[1]) {}
    constexpr vec(const vec<2, T>& lower, T z_, T w_) : x(lower[0]), y(lower[1]), z(z_), w(w_) {}
    constexpr vec(const vec&) = default;

    constexpr vec operator-() const { return { -x, -y, -z, -w }; }

    constexpr bool operator==(const vec& b) const { return v == b.v; }

    template<size_t WIDTH, class NEW_T> requires (WIDTH <= 4)
    constexpr explicit operator vec<WIDTH, NEW_T>() const 
    {
        vec<WIDTH, NEW_T> result; 
        for (size_t i = 0; i < WIDTH; i++) 
            result[i] = (NEW_T) v[i]; 
        return result; 
    }

    constexpr T& operator[](size_t i) { return v[i]; }
    constexpr T operator[](size_t i) const { return v[i]; }

    union
    {
        std::array<T, 4> v{};
        struct
        {
            T x, y, z, w;
        };
        struct
        {
            T r, g, b, a;
        };
    };
};

using vec4 = vec<4, float>;