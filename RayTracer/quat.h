#pragma once

#include <math.h>
#include "vec.h"

struct quat_t
{
	float x, y, z, w;

	quat_t(float w = 0.0f, float x = 0.0f, float y = 1.0f, float z = 0.0f) : w(w), x(x), y(y), z(z)
	{
		normalize();
	}

	quat_t(const vec3_t& axis, float angle)
	{
		float half = angle * 0.5f;
		float sinHalf = sinf(half);
		w = cosf(half);
		x = axis.x * sinHalf;
		y = axis.y * sinHalf;
		z = axis.z * sinHalf;
		normalize();
	}

	//vectors must be normalised
	quat_t(const vec3_t& v1, const vec3_t& v2)
	{
		vec3_t cross;
		float dot = (v1 * v2).sum();
		if (fabs(1.0f - fabsf(dot)) < 0.000001f) // parallel oposite vectors
		{
			// check for (1, 0, 0) and (-1, 0, 0)
			if (fabsf(1.0f - fabsf(v1.x)) < 0.000001f)
				cross = v1 ^ vec3_t{ 0.0f, 1.0f, 0.0f }; // quat_t(vec(0,1,0), 180)
			else
				cross = v1 ^ vec3_t{ 1.0f, 0.0f, 0.0f }; // quat_t(vec(1,0,0), 180)
		}
		else
		{
			cross = v1 ^ v2;
			cross.normalize();
		}

		*this = quat_t(cross, acosf(dot));
	}

	float magnitude()
	{
		float mag = sqrtf(w * w + x * x + y * y + z * z);
		return (mag != 0.0f) ? mag : 1.0f;
	}

	void normalize()
	{
		float invMag = 1.0f / magnitude();
		w *= invMag; x *= invMag; y *= invMag; z *= invMag;
	}

	quat_t& mul(const quat_t& q)
	{
		*this = mul(*this, q);
		return *this;
	}

	quat_t& mul(float val)
	{
		w *= val;
		normalize();
		return *this;
	}

	static quat_t mul(const quat_t& r, const quat_t& q)
	{
		return quat_t
		(
			q.w*r.w - q.x*r.x - q.y*r.y - q.z*r.z,
			q.w*r.x + q.x*r.w + q.y*r.z - q.z*r.y,
			q.w*r.y + q.y*r.w + q.z*r.x - q.x*r.z,
			q.w*r.z + q.z*r.w + q.x*r.y - q.y*r.x
		);
	}

	static quat_t mul(const quat_t& q, float val)
	{
		quat_t ret(q);
		return ret.mul(val);
	}

	static quat_t add(const quat_t& r, const quat_t& q)
	{
		return quat_t(q.w + r.w, q.x + r.x, q.y + r.y, q.z + r.z);
	}

	static quat_t from_euler(const vec3_t& rot)
	{
		static const vec3_t rotX{ 1.0f, 0.0f, 0.0f };
		static const vec3_t rotY{ 0.0f, 1.0f, 0.0f };
		static const vec3_t rotZ{ 0.0f, 0.0f, 1.0f };

		//yaw * pitch * roll
		return quat_t(rotY, rot.y) * quat_t(rotX, rot.x) * quat_t(rotZ, rot.z);
	}

	static vec3_t mul(const vec3_t& v, const quat_t& q)
	{
		return vec3_t
		{
			2.0f*(q.y*q.w*v.z - q.z*q.w*v.y + q.y*q.x*v.y + q.z*q.x*v.z) + q.w*q.w*v.x + q.x*q.x*v.x - q.z*q.z*v.x - q.y*q.y*v.x,
			2.0f*(q.x*q.y*v.x + q.z*q.y*v.z + q.w*q.z*v.x - q.x*q.w*v.z) + q.y*q.y*v.y - q.z*q.z*v.y + q.w*q.w*v.y - q.x*q.x*v.y,
			2.0f*(q.x*q.z*v.x + q.y*q.z*v.y - q.w*q.y*v.x + q.w*q.x*v.y) + q.z*q.z*v.z - q.y*q.y*v.z - q.x*q.x*v.z + q.w*q.w*v.z
		};
	}

	void set_identity()
	{
		w = 1.0f;
		x = y = z = 0;
	}

	quat_t& conjugate()
	{
		x = -x; y = -y; z = -z;
		return *this;
	}

	vec3_t to_axis() const
	{
		return vec3_t{ x, y, z };
	}

	float to_axis_angle(vec3_t &axis) const
	{
		float scale = sqrtf(x * x + y * y + z * z);

		if (scale < 0.000001f)
		{
			// angle is 0 or 360, so set angle to 0 and axis to 0,0,1
			axis.x = axis.y = 0.0f;
			axis.z = 1.0f;
			return 0.0f;
		}

		float invScale = 1.0f / scale;

		axis.x = x * invScale;
		axis.y = y * invScale;
		axis.z = z * invScale;

		return 2.0f * acosf(w);
	}

	static quat_t slerp(const quat_t &q1, const quat_t &q2, const float t)
	{
		// Calculate the cosine of the angle between the two
		float scale1 = 1.0f, scale2 = 0.0f;
		double cosHalfTheta = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;

		if (cosHalfTheta < 0.0f)
		{
			scale1 = -1.0f;
			cosHalfTheta = -cosHalfTheta;
		}

		// If the angle is significant, use the spherical interpolation
		if ((1.0 - fabs(cosHalfTheta)) > 0.001/*DELTA*/)
		{
			double halfTheta = acos(cosHalfTheta);
			double sinHalfTheta = sin(halfTheta);
			scale1 *= (float)(sin((1.0 - t) * halfTheta) / sinHalfTheta);
			scale2 = (float)(sin(t * halfTheta) / sinHalfTheta);
		}
		// Else use the cheaper linear interpolation
		else
		{
			scale1 *= 1.0f - t;
			scale2 = t;
		}

		// Return the interpolated result
		return quat_t
		(
			q1.w  * scale1 + q2.w *  scale2,
			q1.x  * scale1 + q2.x *  scale2,
			q1.y  * scale1 + q2.y *  scale2,
			q1.z  * scale1 + q2.z *  scale2
		);
	}

	quat_t& operator*=(const quat_t& q) { return mul(q); }
	quat_t& operator*=(float val) { return mul(val); }
	quat_t operator*(const quat_t& q) const { return quat_t::mul(*this, q); }
	quat_t operator*(float val) const { return quat_t::mul(*this, val); }
	quat_t operator+(const quat_t& q) const { return quat_t::add(*this, q); }
	quat_t operator~() const { return quat_t(*this).conjugate(); }
};

inline vec3_t operator*(const vec3_t& v, const quat_t& q) { return quat_t::mul(v, q); }
inline void operator*=(vec3_t& v, const quat_t& q) { v = quat_t::mul(v, q); }