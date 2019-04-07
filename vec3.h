#ifndef VEC3_H
#define VEC3_H

#include <iostream>
#include <algorithm>
#include <math.h>

class vec3 {
public:

	double e[3];
	vec3()
	{
	}

	vec3(double e0, double e1, double e2)
	{
		e[0] = e0;
		e[1] = e1;
		e[2] = e2;
	}
	inline double x() const { return e[0]; }
	inline double y() const { return e[1]; }
	inline double z() const { return e[2]; }
	inline double r() const { return e[0]; }
	inline double g() const { return e[1]; }
	inline double b() const { return e[2]; }

	inline const vec3& operator+() const { return *this; }
	inline vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
	inline double operator[](int i) const { return e[i]; }
	inline double& operator[](int i) { return e[i]; }

	inline vec3& operator+=(const vec3 &v2);
	inline vec3& operator-=(const vec3 &v2);
	inline vec3& operator*=(const vec3 &v2);
	inline vec3& operator/=(const vec3 &v2);
	inline vec3& operator*=(const double);
	inline vec3& operator/=(const double);

	inline double length() const
	{
		return sqrt(e[0]*e[0] + e[1]*e[1] + e[2]*e[2]);
	}

	inline double squared_length() const
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
	double k = 1.0 / sqrt(e[0]*e[0] + e[1]*e[1] + e[2]*e[2]);
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

inline vec3 operator*(double t, const vec3 &v)
{
	return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}
inline vec3 operator*(const vec3 &v, double t)
{
	return vec3(t * v.e[0], t * v.e[1], t * v.e[2]);
}
inline vec3 operator/(const vec3 &v, double t)
{
	return vec3(v.e[0] / t, v.e[1] / t, v.e[2] / t);
}


inline double dot(const vec3 &v1, const vec3 &v2)
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
inline vec3& vec3::operator*=(const double t)
{
	e[0] *= t;
	e[1] *= t;
	e[2] *= t;
	return *this;
}
inline vec3& vec3::operator/=(const double t)
{
	double k = 1.0/t;
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
	double max = std::max({rgb[0], rgb[1], rgb[2]});
	double min = std::min({rgb[0], rgb[1], rgb[2]});
	return vec3(max+min, max+min, max+min) - rgb;
}


inline vec3 RGBtoHSV(const vec3& rgb)
{
	double max = std::max({rgb[0], rgb[1], rgb[2]});
	double min = std::min({rgb[0], rgb[1], rgb[2]});
	double h = 0.0, s, v;
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
	double c = hsv[1];
	double h_p = hsv[0] / 60.0;
	double x = c * (1 - abs(fmod(h_p, 2.0) - 1));
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

inline vec3 rodrigues(const vec3& v, const vec3& axis, double theta)
{
	double c = cos(theta);
	double s = sin(theta);
	vec3 r0 = vec3(c+axis.x()*axis.x()*(1-c), axis.x()*axis.y()*(1-c) - axis.z()*s, axis.x()*axis.z()*(1-c)+axis.y()*s);
	vec3 r1 = vec3(axis.y()*axis.x()*(1-c)+axis.z()*s, c+axis.y()*axis.y()*(1-c), axis.y()*axis.z()*(1-c)-axis.x()*s);
	vec3 r2 = vec3(axis.z()*axis.x()*(1-c)-axis.y()*s, axis.z()*axis.y()*(1-c)+axis.x()*s, c+axis.z()*axis.z()*(1-c));
	return vec3(dot(r0, v), dot(r1, v), dot(r2, v));
}

#endif
