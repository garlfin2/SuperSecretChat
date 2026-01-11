#pragma once

#include <cstdint>
#include <array>
#include <algorithm>
#include <cmath>

// To explain the difference in naming scheme:
// I based the structure of my math library on glm, so I use a similar naming scheme to it.
// more "integral" math types get c++ standard library like names while my classes get my own scheme.

using uint = uint32_t;

template<size_t COMPONENT_COUNT, class T>
struct vec
{
    std::array<T, COMPONENT_COUNT> v;

    constexpr bool operator==(const vec& b) const { return v == b.v; }

    constexpr T& operator[](size_t i) { return v[i]; }
    constexpr T operator[](size_t i) const { return v[i]; }
};

template<size_t WIDTH, class T>
struct mat
{
    using COL_T = vec<WIDTH, T>;

    constexpr bool operator==(const mat& b) const { return v == b.v; }
     
    constexpr COL_T& operator[](size_t i) { return v[i]; }
    constexpr const COL_T& operator[](size_t i) const { return v[i]; }
    
private:
    std::array<COL_T, WIDTH> v;
};

template<size_t WIDTH, class T>
constexpr inline mat<WIDTH, T> mat_identity = []() consteval
{
    mat<WIDTH, T> result{};

    for (size_t i = 0; i < WIDTH; ++i)
        result[i][i] = 1;

    return result;
}();

template<size_t WIDTH, class T>
constexpr mat<WIDTH, T> mat_multiply(const mat<WIDTH, T>& a, const mat<WIDTH, T>& b)
{
    mat<WIDTH, T> result{};

    for (size_t x = 0; x < WIDTH; x++)
    for (size_t y = 0; y < WIDTH; y++)
    {
        result[x][y] = 0;
        for (size_t k = 0; k < WIDTH; k++)
            result[x][y] += a[k][y] * b[x][k];
    }

    return result;
}

template<size_t WIDTH, class T>
constexpr mat<WIDTH, T> mat_transpose(const mat<WIDTH, T>& a)
{
    mat<WIDTH, T> result{};

    for (size_t y = 0; y < WIDTH; y++)
    for (size_t x = 0; x < WIDTH; x++)
        result[x][y] = a[y][x];

    return result;
}

template<size_t WIDTH, class T>
constexpr vec<WIDTH, T> mat_column(const mat<WIDTH, T>& a, size_t y)
{
    vec<WIDTH, T> result;

    for (size_t x = 0; x < WIDTH; x++)
        result[x] = a[x][y];
    return result;
}

template<size_t WIDTH, class T>
constexpr vec<WIDTH, T> mat_multiply(const mat<WIDTH, T>& a, const vec<WIDTH, T>& b)
{
    vec<WIDTH, T> result{};
    for (size_t i = 0; i < WIDTH; i++)
        result += a[i] * b[i];

    return result;
}

template<size_t WIDTH, class T>
constexpr mat<WIDTH, T> mat_invert(const mat<WIDTH, T>& a)
{
    mat<WIDTH, T> result{};

    for (size_t y = 0; y < WIDTH; y++)
    for (size_t x = 0; x < WIDTH; x++)
        result[x][y] =- a[x][y];

    return result;
}

template<size_t WIDTH, class T>
constexpr T vec_dot(const vec<WIDTH, T>& a, const vec<WIDTH, T>& b)
{
    T result = 0;
    for (size_t i = 0; i < WIDTH; i++)
        result += a[i] * b[i];
    return result;
}

template<size_t WIDTH, class T>
constexpr T vec_length2(const vec<WIDTH, T>& a)
{
    return vec_dot(a, a);
}

template<size_t WIDTH, class T>
constexpr T vec_length(const vec<WIDTH, T>& a)
{
    return std::sqrt(vec_length2(a));
}

template<size_t WIDTH, class T>
constexpr vec<WIDTH, T> vec_normalize(const vec<WIDTH, T>& a)
{
    return a / vec_length(a);
}

template<class T>
constexpr vec<3, T> vec_cross(const vec<3, T>& a, const vec<3, T>& b)
{
    return
    {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

template<class T>
constexpr T saturate(const T& a)
{
    return std::clamp(a, (T)0, (T)1);
}

#define VEC_OP(OP) \
    template<size_t WIDTH, class T> \
    constexpr vec<WIDTH, T> operator OP(T a, const vec<WIDTH, T>& b) \
    { \
        vec<WIDTH, T> result; \
        for (size_t i = 0; i < WIDTH; i++) \
            result[i] = a OP b[i]; \
        return result; \
    } \
    template<size_t WIDTH, class T> \
    constexpr vec<WIDTH, T> operator OP(const vec<WIDTH, T>& a, T b) \
    { \
        vec<WIDTH, T> result; \
        for (size_t i = 0; i < WIDTH; i++) \
            result[i] = a[i] OP b; \
        return result; \
    } \
    template<size_t WIDTH, class T> \
    constexpr vec<WIDTH, T> operator OP(const vec<WIDTH, T>& a, const vec<WIDTH, T>& b) \
    { \
        vec<WIDTH, T> result; \
        for (size_t i = 0; i < WIDTH; i++) \
            result[i] = a[i] OP b[i]; \
        return result; \
    } \
    template<size_t WIDTH, class T> \
    constexpr vec<WIDTH, T>& operator OP##=(vec<WIDTH, T>& a, const vec<WIDTH, T>& b) \
    { \
        for (size_t i = 0; i < WIDTH; i++) \
            a[i] OP##= b[i]; \
        return a; \
    } \
    template<size_t WIDTH, class T> \
    constexpr vec<WIDTH, T>& operator OP##=(vec<WIDTH, T>& a, T b) \
    { \
        for (size_t i = 0; i < WIDTH; i++) \
            a[i] OP##= b; \
        return a; \
    }

VEC_OP(-);
VEC_OP(+);
VEC_OP(*);
VEC_OP(/);

namespace std
{
    template<size_t WIDTH, class T>
    constexpr vec<WIDTH, T> clamp(const vec<WIDTH, T>& val, const vec<WIDTH, T>& min, const vec<WIDTH, T>& max)
    {
        vec<WIDTH, T> result;
        for (size_t i = 0; i < WIDTH; i++)
            result[i] = clamp(val[i], min[i], max[i]);
        return result;
    }

    template<size_t WIDTH, class T>
    constexpr vec<WIDTH, T> clamp(const vec<WIDTH, T>& val, T min, T max)
    {
        vec<WIDTH, T> result;
        for (size_t i = 0; i < WIDTH; i++)
            result[i] = clamp(val[i], min, max);
        return result;
    }
}