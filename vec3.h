#ifndef VEC3_H
#define VEC3_H

#include <iostream>
#include <algorithm>
#include <math.h>


template <typename T>
class vec3 {
	//static_assert(std::is_same<T, float>::value || std::is_same<T, double>::value, "")
public:

	T e[3];
	vec3()
	{
	}

	vec3(T e0, T e1, T e2)
	{
		e[0] = e0;
		e[1] = e1;
		e[2] = e2;
	}
	inline T x() const { return e[0]; }
	inline T y() const { return e[1]; }
	inline T z() const { return e[2]; }
	inline T r() const { return e[0]; }
	inline T g() const { return e[1]; }
	inline T b() const { return e[2]; }

	inline const vec3& operator+() const { return *this; }
	inline vec3 operator-() const { return vec3(-e[0], -e[1], -e[2]); }
	inline T operator[](int i) const { return e[i]; }
	inline T& operator[](int i) { return e[i]; }

	inline vec3& operator+=(const vec3 &v2);
	inline vec3& operator-=(const vec3 &v2);
	inline vec3& operator*=(const vec3 &v2);
	inline vec3& operator/=(const vec3 &v2);
	inline vec3& operator*=(const T);
	inline vec3& operator/=(const T);

	inline T length() const
	{
		return sqrt(e[0]*e[0] + e[1]*e[1] + e[2]*e[2]);
	}

	inline T squared_length() const
	{
		return e[0]*e[0] + e[1]*e[1] + e[2]*e[2];
	}

	inline void make_unit_vector();

};


template <typename T>
inline std::istream& operator>>(std::istream &is, vec3<T> &t)
{
	is >> t.e[0] >> t.e[1] >> t.e[2];
	return is;
}

template <typename T>
inline std::ostream& operator<<(std::ostream &os, const vec3<T> &t)
{
	os << t.e[0] << " " << t.e[1] << " " << t.e[2];
	return os;
}

template <typename T>
inline void vec3<T>::make_unit_vector()
{
	T k = 1.0 / sqrt(e[0]*e[0] + e[1]*e[1] + e[2]*e[2]);
	e[0] *= k;
	e[1] *= k;
	e[2] *= k;
}

template <typename T>
inline vec3<T> operator+(const vec3<T> &v1, const vec3<T> &v2)
{
	return vec3<T>(v1.e[0] + v2.e[0], v1.e[1] + v2.e[1], v1.e[2] + v2.e[2]);
}
template <typename T>
inline vec3<T> operator-(const vec3<T> &v1, const vec3<T> &v2)
{
	return vec3<T>(v1.e[0] - v2.e[0], v1.e[1] - v2.e[1], v1.e[2] - v2.e[2]);
}
template <typename T>
inline vec3<T> operator*(const vec3<T> &v1, const vec3<T> &v2)
{
	return vec3<T>(v1.e[0] * v2.e[0], v1.e[1] * v2.e[1], v1.e[2] * v2.e[2]);
}
template <typename T>
inline vec3<T> operator/(const vec3<T> &v1, const vec3<T> &v2)
{
	return vec3<T>(v1.e[0] / v2.e[0], v1.e[1] / v2.e[1], v1.e[2] / v2.e[2]);
}

template <typename T>
inline vec3<T> operator*(T t, const vec3<T> &v)
{
	return vec3<T>(t * v.e[0], t * v.e[1], t * v.e[2]);
}
template <typename T>
inline vec3<T> operator*(const vec3<T> &v, T t)
{
	return vec3<T>(t * v.e[0], t * v.e[1], t * v.e[2]);
}
template <typename T>
inline vec3<T> operator/(const vec3<T> &v, T t)
{
	return vec3<T>(v.e[0] / t, v.e[1] / t, v.e[2] / t);
}


template <typename T>
inline T dot(const vec3<T> &v1, const vec3<T> &v2)
{
	return v1.e[0]*v2.e[0] + v1.e[1]*v2.e[1] + v1.e[2]*v2.e[2];
}

template <typename T>
inline vec3<T> cross(const vec3<T> &v1, const vec3<T> &v2)
{
	return vec3<T>( v1.e[1]*v2.e[2] - v1.e[2]*v2.e[1],
	-(v1.e[0]*v2.e[2] - v1.e[2]*v2.e[0]),
	v1.e[0]*v2.e[1] - v1.e[1]*v2.e[0]
	);
}

template <typename T>
inline vec3<T>& vec3<T>::operator+=(const vec3<T> &v)
{
	e[0] += v.e[0];
	e[1] += v.e[1];
	e[2] += v.e[2];
	return *this;
}
template <typename T>
inline vec3<T>& vec3<T>::operator-=(const vec3<T> &v)
{
	e[0] -= v.e[0];
	e[1] -= v.e[1];
	e[2] -= v.e[2];
	return *this;
}
template <typename T>
inline vec3<T>& vec3<T>::operator*=(const vec3<T> &v)
{
	e[0] *= v.e[0];
	e[1] *= v.e[1];
	e[2] *= v.e[2];
	return *this;
}
template <typename T>
inline vec3<T>& vec3<T>::operator/=(const vec3<T> &v)
{
	e[0] /= v.e[0];
	e[1] /= v.e[1];
	e[2] /= v.e[2];
	return *this;
}
template <typename T>
inline vec3<T>& vec3<T>::operator*=(const T t)
{
	e[0] *= t;
	e[1] *= t;
	e[2] *= t;
	return *this;
}
template <typename T>
inline vec3<T>& vec3<T>::operator/=(const T t)
{
	T k = 1.0/t;
	e[0] *= k;
	e[1] *= k;
	e[2] *= k;
	return *this;
}


template <typename T>
inline vec3<T> unit_vector(vec3<T> v)
{
	return v / v.length();
}


template <typename T>
inline vec3<T> complementary_rgb(vec3<T> rgb)
{
	T max = std::max({rgb[0], rgb[1], rgb[2]});
	T min = std::min({rgb[0], rgb[1], rgb[2]});
	return vec3<T>(max+min, max+min, max+min) - rgb;
}


template <typename T>
inline vec3<T> RGBtoHSV(const vec3<T>& rgb)
{
	T max = std::max({rgb[0], rgb[1], rgb[2]});
	T min = std::min({rgb[0], rgb[1], rgb[2]});
	T h = 0.0, s, v;
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
	return vec3<T>(h, s, v);
}


template <typename T>
inline vec3<T> HSVtoRGB(const vec3<T>& hsv)
{
	T c = hsv[1];
	T h_p = hsv[0] / 60.0;
	T x = c * (1 - abs(fmod(h_p, 2.0) - 1));
	vec3<T> rgb = (hsv[2]-c) * vec3<T>(1, 1, 1);
	std::cout << x << " " << c << std::endl;
	if (h_p >= 0 && h_p < 1) {
		rgb += vec3<T>(c, x, 0);
	} else if (h_p >= 1 && h_p < 2) {
		rgb += vec3<T>(x, c, 0);
	} else if (h_p >= 2 && h_p < 3) {
		rgb += vec3<T>(0, c, x);
	} else if (h_p >= 3 && h_p < 4) {
		rgb += vec3<T>(0, x, c);
	} else if (h_p >= 4 && h_p < 5) {
		rgb += vec3<T>(x, 0, c);
	} else if (h_p >= 5 && h_p < 6) {
		rgb += vec3<T>(c, 0, x);
	}

	return rgb;
}

template <typename T>
inline vec3<T> rodrigues(const vec3<T>& v, const vec3<T>& axis, T theta)
{
	T c = cos(theta);
	T s = sin(theta);
	vec3<T> r0 = vec3<T>(c+axis.x()*axis.x()*(1-c), axis.x()*axis.y()*(1-c) - axis.z()*s, axis.x()*axis.z()*(1-c)+axis.y()*s);
	vec3<T> r1 = vec3<T>(axis.y()*axis.x()*(1-c)+axis.z()*s, c+axis.y()*axis.y()*(1-c), axis.y()*axis.z()*(1-c)-axis.x()*s);
	vec3<T> r2 = vec3<T>(axis.z()*axis.x()*(1-c)-axis.y()*s, axis.z()*axis.y()*(1-c)+axis.x()*s, c+axis.z()*axis.z()*(1-c));
	return vec3<T>(dot(r0, v), dot(r1, v), dot(r2, v));
}

using dvec3 = vec3<double>;
using fvec3 = vec3<float>;

#endif
