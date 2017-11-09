#pragma once
#ifndef __VECTOR_H__
#define __VECTOR_H__
#include <math.h>
#include <string>
/// const
const float PI = 3.1415926535897932384626433832795f;
const float TWOPI = 2.0f * PI;
const float HALFPI = 0.5f * PI;
const float RAD2DEG = 180.0f / PI;
const float DEG2RAD = PI / 180.0f;
const float EPSILON = 0.00005f;
const float NATURALE = 2.71828182845904523536f;
const float F32_MAX = 3.402823466e+38F;
#ifdef _WIN32
char *strsep(char **stringp, const char *delim)
{
	char *s;
	const char *spanp;
	int c, sc;
	char *tok;
	if ((s = *stringp) == NULL)
		return (NULL);
	for (tok = s;;) {
		c = *s++;
		spanp = delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				*stringp = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}
#endif // _WIN32

struct Vector2
{
	float x, y;

	Vector2() {}
	Vector2(float x, float y) : x(x), y(y) {}

	friend bool operator == (const Vector2 & v1, const Vector2 & v2)
	{
		return (v1.x == v2.x) && (v1.y == v2.y);
	}

	friend bool operator != (const Vector2 & v1, const Vector2 & v2)
	{
		return (v1.x != v2.x) || (v1.y != v2.y);
	}
};

inline Vector2 NegativeRotate(const Vector2 v1, float angle)
{
	return Vector2(v1.x * cos(angle) - v1.y * sin(angle), v1.x * sin(angle) + v1.y * cos(angle));
}

struct Vector3
{
	float x, y, z;

	Vector3() {}
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
	Vector3(char* s) {
		x = .0f;
		y = .0f;
		z = .0f;
		char* now = s;
		char* val;

		if (!now)
			return;

		size_t n = strlen(s);
		if (s[n - 1] == ';')
			s[n - 1] = 0;

		val = strsep(&now, ",");
		if (!val)
			return;
		x = strtof(val, NULL);

		val = strsep(&now, ",");
		if (!val)
			return;
		y = strtof(val, NULL);

		val = strsep(&now, ",");
		if (!val)
			return;
		z = strtof(val, NULL);
	}

	void operator = (const Vector3 & v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}

	friend bool operator == (const Vector3 & v1, const Vector3 & v2)
	{
		return (v1.x == v2.x) && (v1.y == v2.y) && (v1.z == v2.z);
	}

	friend bool operator != (const Vector3 & v1, const Vector3 & v2)
	{
		return (v1.x != v2.x) || (v1.y != v2.y) || (v1.z != v2.z);
	}
};

inline Vector3 Cross(const Vector3& a, const Vector3& b)
{
	return Vector3(
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x);
}

inline Vector3 operator+(const Vector3 & a, const Vector3 & b)
{
	return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline Vector3 operator-(const Vector3 & a, const Vector3 & b)
{
	return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline Vector3 operator/(const Vector3 & a, const float & b)
{
	return Vector3(a.x / b, a.y / b, a.z / b);
}

inline Vector3 operator*(const Vector3 & a, const Vector3 & b)
{
	return Vector3(a.x * b.x, a.y * b.y, a.z * b.z);
}

inline float Dot(const Vector3& a, const Vector3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vector3 operator*(const Vector3 & a, const float & b)
{
	return Vector3(a.x * b, a.y * b, a.z * b);
}

inline float Length(const Vector3 & a)
{
	return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}

inline Vector3 Normalize(const Vector3 & a)
{
	float len = Length(a);
	return Vector3(a.x / len, a.y / len, a.z / len);
}

inline float LengthSqr(const Vector3 & a)
{
	return a.x * a.x + a.y * a.y + a.z * a.z;
}

struct Vector4
{
	float x, y, z, w;

	Vector4() {}
	Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

	void operator = (const Vector4 & v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		w = v.w;
	}

	friend bool operator == (const Vector4 & v1, const Vector4 & v2)
	{
		return (v1.x == v2.x) && (v1.y == v2.y) && (v1.z == v2.z) && (v1.w == v2.w);
	}

	friend bool operator != (const Vector4 & v1, const Vector4 & v2)
	{
		return (v1.x != v2.x) || (v1.y != v2.y) || (v1.z != v2.z) || (v1.w != v2.w);
	}
};


struct Quaternion
{
	float x, y, z, w;

	friend bool operator == (const Quaternion & v1, const Quaternion & v2)
	{
		return (v1.x == v2.x) && (v1.y == v2.y) && (v1.z == v2.z);
	}

	friend bool operator != (const Quaternion & v1, const Quaternion & v2)
	{
		return (v1.x != v2.x) || (v1.y != v2.y) || (v1.z != v2.z);
	}

	void operator = (const Quaternion & v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		w = v.w;
	}

	Quaternion()
	{

	}

	Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w)
	{

	}

	Quaternion(const Vector3 & v, float radian)
	{
		w = cosf(radian * 0.5f);
		x = sinf(radian * 0.5f) * v.x;
		y = sinf(radian * 0.5f) * v.y;
		z = sinf(radian * 0.5f) * v.z;
	}

	Vector3 GetZXY() const
	{
		Vector3 v;

		// Extract Sin(pitch)
		float sp = -2.0f * (y*z - w*x);

		// Check for Gimbel lock, giving slight tolerance for numerical imprecision
		if (abs(sp) > 0.9999f)
		{
			v.x = HALFPI * sp;
			v.y = atan2f(-x*z + w*y, 0.5f - y*y - z*z);
			v.z = 0.0f;
		}
		else
		{
			v.x = asinf(sp);
			v.y = atan2f(x*z + w*y, 0.5f - x*x - y*y);
			v.z = atan2f(x*y + w*z, 0.5f - x*x - z*z);
		}

		return v;
	}

	void SetZXY(const Vector3 & xyz)
	{
		float h = xyz.y * 0.5f;
		float p = xyz.x * 0.5f;
		float b = xyz.z * 0.5f;

		float ch = cosf(h);
		float sh = sinf(h);
		float cp = cosf(p);
		float sp = sinf(p);
		float cb = cosf(b);
		float sb = sinf(b);

		w = ch * cp * cb + sh * sp * sb;
		x = ch * sp * cb + sh * cp * sb;
		y = sh * cp * cb - ch * sp * sb;
		z = ch * cp * sb - sh * sp * cb;
	}
};

inline Quaternion Multiply(const Quaternion &q2, const Quaternion& q1)
{
	Quaternion o;
	o.x = q2.y*q1.z - q2.z*q1.y + q2.w*q1.x + q1.w*q2.x;
	o.y = q2.z*q1.x - q2.x*q1.z + q2.w*q1.y + q1.w*q2.y;
	o.z = q2.x*q1.y - q2.y*q1.x + q2.w*q1.z + q1.w*q2.z;
	o.w = q2.w*q1.w - q2.x*q1.x - q2.y*q1.y - q2.z*q1.z;

	return o;
}

inline Vector3 TransformCoord(const Vector3& a, const Quaternion& q)
{
	Vector3 o;
	Quaternion r(a.x, a.y, a.z, 0);
	Quaternion c(-q.x, -q.y, -q.z, q.w);
	r = Multiply(r, c);
	r = Multiply(q, r);
	o = Vector3(r.x, r.y, r.z);
	return o;
}

inline Vector3 operator*(const Vector3& v, const Quaternion& q)
{
	Vector3 t;
	t = TransformCoord(v, q);
	return t;
}
#endif // !__VECTOR_H__

