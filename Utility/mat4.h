#pragma once

#include "Math.h"
#include <cmath>
#include <numbers>

template<class T>
T ToRad(T t)
{
	return t * std::numbers::pi_v<T> / (T) 180;
}

template<class T>
T ToDeg(T t)
{
	return t * (T)180 / std::numbers::pi_v<T>;
}

template<class T>
struct mat<4, T>
{
	using COL_T = vec<4, T>;
	using ROW_T = vec<4, T>;

	constexpr COL_T& operator[](size_t i) { return v[i]; }
	constexpr const COL_T& operator[](size_t i) const { return v[i]; }

	constexpr mat() = default;
	constexpr mat(const COL_T& x, const COL_T& y, const COL_T& z, const COL_T& w) :
		v{ x, y, z, w }
	{}

	constexpr mat(const mat&) = default;

	template<size_t WIDTH>
	explicit constexpr mat(const mat<WIDTH, T>& b) requires (WIDTH < 4)
	{
		for (size_t y = 0; y < WIDTH; y++)
		for (size_t x = 0; x < WIDTH; x++)
			v[x][y] = b[x][y];
	}

	constexpr bool operator==(const mat& b) const { return v == b.v; }

	constexpr mat operator*(const mat& b) const
	{
		return mat_multiply(*this, b);
	}
	constexpr mat& operator*=(const mat& b) { return *this = *this * b; }

	constexpr mat transpose()
	{
		return mat_transpose(*this);
	}

	constexpr vec<4, T> operator*(const vec<4, T>& b)
	{
		return mat_multiply(*this, b);
	}

	constexpr mat operator-() const { return mat_invert(*this); }

	constexpr mat fast_inverse() const
	{
		mat<3, T> inverse = mat_transpose((mat<3, T>) * this);
		vec<3, T> position = inverse * (vec<3, T>) v[3];

		mat result = (mat)inverse;
		result[3] = vec<4, T>(-position, 1);

		return result;
	}

	static mat scale(const vec<3, T>& s)
	{
		return
		{
			{ s[0], 0, 0, 0 },
			{ 0, s[1], 0, 0 },
			{ 0, 0, s[2], 0 },
			{ 0, 0, 0,    1 }
		};
	}

	static mat position(const vec<3, T>& p)
	{
		return
		{
			{ 1,    0,    0,    0 },
			{ 0,    1,    0,    0 },
			{ 0,    0,    1,    0 },
			{ p[0], p[1], p[2], 1 }
		};
	}

	static mat rotate_x(T angle)
	{
		return
		{
			{ 1, 0,               0,                0 },
			{ 0, std::cos(angle), -std::sin(angle), 0 },
			{ 0, std::sin(angle), std::cos(angle),  0 },
			{ 0, 0,               0,                1 }
		};
	}

	static mat rotate_y(T angle)
	{
		return
		{
			{ std::cos(angle),  0, std::sin(angle), 0 },
			{ 0,                1, 0,               0 },
			{ -std::sin(angle), 0, std::cos(angle), 0 },
			{ 0,                0, 0,               1 }
		};
	}

	static mat rotate_z(T angle)
	{
		return
		{
			{ std::cos(angle), -std::sin(angle), 0, 0 },
			{ std::sin(angle), std::cos(angle),  0, 0 },
			{ 0,               0,                1, 0 },
			{ 0,               0,                0, 1 }
		};
	}

	// FOV: Vertical FOV, in radians.
	// for some grand reason near/far are reserved...
	static mat perspective(T fov, T aspect, T near_, T far_)
	{
		if (near_ == 0 || near_ > far_)
			return mat_identity<4, T>;

		fov *= (T) 0.5;
		T yScale = std::cos(fov) / std::sin(fov);
		T xScale = yScale * aspect;

		return
		{
			{ xScale, 0,      0,                                0 },
			{ 0,      yScale, 0,                                0 },
			{ 0,      0,      far_ / (far_ - near_),            1 },
			{ 0,      0,      -(far_ * near_) / (far_ - near_), 0 }
		};
	}

private:
	std::array<COL_T, 4> v{};
};

using mat4 = mat<4, float>;