#pragma once

#include "Math.h"

#include "mat4.h"

template<class T>
struct mat<3, T>
{
	using COL_T = vec<3, T>;
	using ROW_T = vec<3, T>;

	constexpr COL_T& operator[](size_t i) { return v[i]; }
	constexpr const COL_T& operator[](size_t i) const { return v[i]; }

	constexpr mat() = default;
	constexpr mat(const COL_T& x, const COL_T& y, const COL_T& z) :
		v{ x, y, z }
	{
	}

	constexpr mat(const mat&) = default;

	template<size_t WIDTH> requires (WIDTH > 3)
	explicit constexpr mat(const mat<WIDTH, T>& b)
	{
		for (size_t y = 0; y < 3; y++)
		for (size_t x = 0; x < 3; x++)
			v[x][y] = b[x][y];
	}

	constexpr bool operator==(const mat& b) const { return v == b.v; }

	constexpr mat operator-() const { return mat_invert(*this); }

	constexpr mat operator*(const mat& b) const
	{
		mat result{};

		for (size_t y = 0; y < 3; y++)
		for (size_t x = 0; x < 3; x++)
		{
			result[x][y] = 0;
			for (size_t i = 0; i < 3; i++)
				result[x][y] += v[x][i] * b[i][y];
		}

		return result;
	}
	constexpr mat& operator*=(const mat& b) { return *this = *this * b; }

	constexpr vec<3, T> operator*(const vec<3, T>& b)
	{
		return mat_multiply(*this, b);
	}

	constexpr mat transpose()
	{
		return mat_transpose(*this);
	}

	constexpr T determinant() const
	{
		// https://dcvp84mxptlac.cloudfront.net/diagrams2/equation-5-shortcut-method-to-obtain-the-determinant-of-a-3x3-matrix.png
		return v[0][0] * v[1][1] * v[2][2] +
			   v[0][1] * v[1][2] * v[2][0] +
			   v[0][2] * v[1][0] * v[2][1] -
			   v[0][0] * v[2][1] * v[2][1] -
			   v[0][1] * v[1][0] * v[2][2] -
			   v[0][2] * v[1][1] * v[2][0];
	}

	constexpr mat inverse() const
	{
		T invDet = (T) 1 / determinant();
		
		mat result{};

		result[0][0] = (v[1][1] * v[2][2] - v[2][1] * v[1][2]) * invDet;
		result[1][0] = -(v[1][0] * v[2][2] - v[2][0] * v[1][2]) * invDet;
		result[2][0] = (v[1][0] * v[2][1] - v[2][0] * v[1][1]) * invDet;
		result[0][1] = -(v[0][1] * v[2][2] - v[2][1] * v[0][2]) * invDet;
		result[1][1] = (v[0][0] * v[2][2] - v[2][0] * v[0][2]) * invDet;
		result[2][1] = -(v[0][0] * v[2][1] - v[2][0] * v[0][1]) * invDet;
		result[0][2] = (v[0][1] * v[1][2] - v[1][1] * v[0][2]) * invDet;
		result[1][2] = -(v[0][0] * v[1][2] - v[1][0] * v[0][2]) * invDet;
		result[2][2] = (v[0][0] * v[1][1] - v[1][0] * v[0][1]) * invDet;

		return result;
	}

private:
	std::array<COL_T, 3> v{};
};

using mat3 = mat<3, float>;