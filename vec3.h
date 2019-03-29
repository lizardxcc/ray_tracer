#ifndef VEC3_H
#define VEC3_H

#include <iostream>
#include <algorithm>
#include <math.h>

class vec3 {
public:

	float e[3];
	vec3()
	{
	}

	vec3(float e0, float e1, float e2)
	{
		e[0] = e0;
		e[1] = e1;
		e[2] = e2;
	}
	inline float x() const { return e[0]; }
	inline float y() const { return e[1]; }
	inline float z() const { return e[2]; }
	inline float r() const { return e[0]; }
	inline float g() const { return e[1]; }
	inline float b() const { return e[2]; }

	inline const vec3& operator+() const { return *this; }
	inline vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
	inline float operator[](int i) const { return e[i]; }
	inline float& operator[](int i) { return e[i]; }

	inline vec3& operator+=(const vec3 &v2);
	inline vec3& operator-=(const vec3 &v2);
	inline vec3& operator*=(const vec3 &v2);
	inline vec3& operator/=(const vec3 &v2);
	inline vec3& operator*=(const float);
	inline vec3& operator/=(const float);

	inline float length() const
	{
		return sqrt(e[0]*e[0] + e[1]*e[1] + e[2]*e[2]);
	}

	inline float squared_length() const
	{
		return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
	}

	inline void make_unit_vector();

};


inline std::istream& operator>>(std::istream &is, vec3 &t)
{
	is >> t.e[0] >> t.e[1] >> t.e[2];
	return is;
}

inline std::ostream& operator<<(std::ostream &os, const vec3 &t)
{
	os << t.e[0] << " " << t.e[1] << " " << t.e[2];
	return os;
}

inline void vec3::make_unit_vector()
{
	float k = 1.0 / sqrt(e[0]*e[0] + e[1]*e[1] + e[2]*e[2]);
	e[0] *= k;
	e[1] *= k;
	e[2] *= k;
}

inline vec3 operator+(const vec3 &v1, const vec3 &v2)
{
	return vec3(v1.e[0] + v2.e[0], v1.e[1] + v2.e[1], v1.e[2] + v2.e[2]);
}
inline vec3 operator-(const vec3 &v1, const vec3 &v2)
{
	return vec3(v1.e[0] - v2.e[0], v1.e[1] - v2.e[1], v1.e[2] - v2.e[2]);
}
inline vec3 operator*(const vec3 &v1, const vec3 &v2)
{
	return vec3(v1.e[0] * v2.e[0], v1.e[1] * v2.e[1], v1.e[2] * v2.e[2]);
}
inline vec3 operator/(const vec3 &v1, const vec3 &v2)
{
	return vec3(v1.e[0] / v2.e[0], v1.e[1] / v2.e[1], v1.e[2] / v2.e[2]);
}

inline vec3 operator*(float t, const vec3 &v)
{
	return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}
inline vec3 operator*(const vec3 &v, float t)
{
	return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}
inline vec3 operator/(const vec3 &v, float t)
{
	return vec3(v.e[0] / t, v.e[1] / t, v.e[2] / t);
}


inline float dot(const vec3 &v1, const vec3 &v2)
{
	return v1.e[0]*v2.e[0] + v1.e[1]*v2.e[1] + v1.e[2]*v2.e[2];
}

inline vec3 cross(const vec3 &v1, const vec3 &v2)
{
	return vec3( v1.e[1]*v2.e[2] - v1.e[2]*v2.e[1],
	-(v1.e[0]*v2.e[2] - v1.e[2]*v2.e[0]),
	v1.e[0]*v2.e[1] - v1.e[1]*v2.e[0]
	);
}

inline vec3& vec3::operator+=(const vec3 &v)
{
	e[0] += v.e[0];
	e[1] += v.e[1];
	e[2] += v.e[2];
	return *this;
}
inline vec3& vec3::operator-=(const vec3 &v)
{
	e[0] -= v.e[0];
	e[1] -= v.e[1];
	e[2] -= v.e[2];
	return *this;
}
inline vec3& vec3::operator*=(const vec3 &v)
{
	e[0] *= v.e[0];
	e[1] *= v.e[1];
	e[2] *= v.e[2];
	return *this;
}
inline vec3& vec3::operator/=(const vec3 &v)
{
	e[0] /= v.e[0];
	e[1] /= v.e[1];
	e[2] /= v.e[2];
	return *this;
}
inline vec3& vec3::operator*=(const float t)
{
	e[0] *= t;
	e[1] *= t;
	e[2] *= t;
	return *this;
}
inline vec3& vec3::operator/=(const float t)
{
	float k = 1.0/t;
	e[0] *= k;
	e[1] *= k;
	e[2] *= k;
	return *this;
}


inline vec3 unit_vector(vec3 v)
{
	return v / v.length();
}


inline vec3 complementary_rgb(vec3 rgb)
{
	float max = std::max({rgb[0], rgb[1], rgb[2]});
	float min = std::min({rgb[0], rgb[1], rgb[2]});
	return vec3(max+min, max+min, max+min) - rgb;
}


inline vec3 RGBtoHSV(const vec3& rgb)
{
	float max = std::max({rgb[0], rgb[1], rgb[2]});
	float min = std::min({rgb[0], rgb[1], rgb[2]});
	float h = 0.0, s, v;
	if (min == max) {
	} else if (min == rgb[2]) {
		h = 60.0 * (rgb[1] - rgb[0]) / (max-min) + 60.0;
	} else if (min == rgb[0]) {
		h = 60.0 * (rgb[2] - rgb[1]) / (max-min) + 180.0;
	} else if (min == rgb[1]) {
		h = 60.0 * (rgb[0] - rgb[2]) / (max-min) + 300.0;
	}
	s = max - min;
	v = max;
	return vec3(h, s, v);
}


inline vec3 HSVtoRGB(const vec3& hsv)
{
	float c = hsv[1];
	float h_p = hsv[0] / 60.0;
	float x = c * (1 - abs(fmod(h_p, 2.0) - 1));
	vec3 rgb = (hsv[2]-c) * vec3(1, 1, 1);
	std::cout << x << " " << c << std::endl;
	if (h_p >= 0 && h_p < 1) {
		rgb += vec3(c, x, 0);
	} else if (h_p >= 1 && h_p < 2) {
		rgb += vec3(x, c, 0);
	} else if (h_p >= 2 && h_p < 3) {
		rgb += vec3(0, c, x);
	} else if (h_p >= 3 && h_p < 4) {
		rgb += vec3(0, x, c);
	} else if (h_p >= 4 && h_p < 5) {
		rgb += vec3(x, 0, c);
	} else if (h_p >= 5 && h_p < 6) {
		rgb += vec3(c, 0, x);
	}

	return rgb;
}

#endif
