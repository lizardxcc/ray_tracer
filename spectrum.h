#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <vector>
#include "vec3.h"

constexpr int RANGE = 700 - 400;
constexpr int N_SAMPLE = 30;
constexpr int SAMPLE_SIZE = RANGE/N_SAMPLE;

class Spectrum {
	public:
		explicit Spectrum(void);
		explicit Spectrum(double v);
		explicit Spectrum(const double d[N_SAMPLE]);
		inline Spectrum& operator*=(const double);
		inline Spectrum& operator/=(const double);

		double integrate(double min_wl, double max_wl) const;
		void add(double value, double min_wl, double max_wl);
		double get(double wavelength) const;
		std::vector<double> data;
};

inline Spectrum& Spectrum::operator*=(const double t)
{
	for (auto& d : data) {
		d *= t;
	}
	return *this;
}
inline Spectrum& Spectrum::operator/=(const double t)
{
	for (auto& d : data) {
		d /= t;
	}
	return *this;
}

inline Spectrum operator+(const Spectrum &s1, const Spectrum &s2)
{
	Spectrum s;
	for (size_t i = 0; i < s1.data.size(); i++) {
		s.data[i] = s1.data[i] + s2.data[i];
	}
	return s;
}
inline Spectrum operator-(const Spectrum &s1, const Spectrum &s2)
{
	Spectrum s;
	for (size_t i = 0; i < s1.data.size(); i++) {
		s.data[i] = s1.data[i] - s2.data[i];
	}
	return s;
}
inline Spectrum operator*(const Spectrum &s1, const Spectrum &s2)
{
	Spectrum s;
	for (size_t i = 0; i < s1.data.size(); i++) {
		s.data[i] = s1.data[i] * s2.data[i];
	}
	return s;
}
inline Spectrum operator/(const Spectrum &s1, const Spectrum &s2)
{
	Spectrum s;
	for (size_t i = 0; i < s1.data.size(); i++) {
		s.data[i] = s1.data[i] / s2.data[i];
	}
	return s;
}

inline Spectrum operator*(double t, const Spectrum &s1)
{
	Spectrum s;
	for (size_t i = 0; i < s1.data.size(); i++) {
		s.data[i] = s1.data[i] * t;
	}
	return s;
}
inline Spectrum operator*(const Spectrum &s1, double t)
{
	Spectrum s;
	for (size_t i = 0; i < s1.data.size(); i++) {
		s.data[i] = s1.data[i] * t;
	}
	return s;
}
inline Spectrum operator/(const Spectrum &s1, double t)
{
	Spectrum s;
	for (size_t i = 0; i < s1.data.size(); i++) {
		s.data[i] = s1.data[i] / t;
	}
	return s;
}
vec3 xyz(const Spectrum &s);
vec3 rgb(const Spectrum &s);
vec3 r_xyz(const Spectrum &s);
vec3 r_rgb(const Spectrum &s);
Spectrum RGBtoSpectrum(const vec3& rgb);

#endif
