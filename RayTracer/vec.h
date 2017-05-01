#pragma once

struct vec2_t
{
	float x, y;
};

struct vec3_t
{
	float x, y, z;

	vec3_t& operator+=(const vec3_t& rhs) { x += rhs.x;	y += rhs.y;	z += rhs.z;	return *this; }
	vec3_t& operator-=(const vec3_t& rhs) { x -= rhs.x;	y -= rhs.y;	z -= rhs.z;	return *this; }
	vec3_t& operator*=(const vec3_t& rhs) { x *= rhs.x;	y *= rhs.y; z *= rhs.z; return *this; }
	vec3_t& operator*=(float rhs) { x *= rhs; y *= rhs; z *= rhs; return *this; }

	vec3_t operator-() const { return { -x, -y, -z };  }

	vec3_t& operator^=(const vec3_t& rhs)
	{
		float nx = y * rhs.z - z * rhs.y;
		float ny = z * rhs.x - x * rhs.z;
		float nz = x * rhs.y - y * rhs.x;
		x = nx; y = ny; z = nz;
		return *this;
	}

	float sum() const { return x + y + z; }
	float length() const { return sqrtf(x * x + y * y + z * z); }
	vec3_t& normalize() { *this *= 1.0f / length(); return *this; }

	//vec3_t& rotate(const vec3_t& axis, float angle) { *this = ::rotate(*this, axis, angle); return *this; }
};

inline vec3_t operator+(vec3_t lhs, const vec3_t& rhs) { lhs += rhs; return lhs; }
inline vec3_t operator-(vec3_t lhs, const vec3_t& rhs) { lhs -= rhs; return lhs; }
inline vec3_t operator*(vec3_t lhs, const vec3_t& rhs) { lhs *= rhs; return lhs; }
inline vec3_t operator*(vec3_t lhs, float rhs) { lhs *= rhs; return lhs; }
inline vec3_t operator*(float lhs, vec3_t rhs) { rhs *= lhs; return rhs; }
inline vec3_t operator^(vec3_t lhs, const vec3_t& rhs) { lhs ^= rhs; return lhs; }

inline vec3_t normalize(vec3_t v) { v.normalize(); return v; }
inline float dot(const vec3_t& lhs, const vec3_t& rhs) { return (lhs * rhs).sum(); }
inline vec3_t rotate(const vec3_t& v, const vec3_t& axis, float angle)
{
	// https://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula
	float cost = cosf(angle);
	float sint = sinf(angle);
	vec3_t result = cost * v + sint * (v ^ axis) + (1.0f - cost) * axis * dot(axis, v);
	return result;
}


