#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <vector>
#include "vec3.h"

#define RANGE (700 - 400)
#define N_SAMPLE (10)
#define SAMPLE_SIZE (RANGE/N_SAMPLE)

class Spectrum {
	public:
		Spectrum(void)
		{
			data.resize(N_SAMPLE);
		}
		Spectrum(float v)
		{
			data.resize(N_SAMPLE);
			for (auto& d : data) {
				d = v;
			}
		}

		inline Spectrum& operator*=(const float);
		inline Spectrum& operator/=(const float);

		float integrate(float min_wl, float max_wl) const;
		void add(float value, float min_wl, float max_wl);
		float get(float wavelength) const;
		std::vector<float> data;
};

inline Spectrum& Spectrum::operator*=(const float t)
{
	for (auto& d : data) {
		d *= t;
	}
	return *this;
}
inline Spectrum& Spectrum::operator/=(const float t)
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

inline Spectrum operator*(float t, const Spectrum &s1)
{
	Spectrum s;
	for (size_t i = 0; i < s1.data.size(); i++) {
		s.data[i] = s1.data[i] * t;
	}
	return s;
}
inline Spectrum operator*(const Spectrum &s1, float t)
{
	Spectrum s;
	for (size_t i = 0; i < s1.data.size(); i++) {
		s.data[i] = s1.data[i] * t;
	}
	return s;
}
inline Spectrum operator/(const Spectrum &s1, float t)
{
	Spectrum s;
	for (size_t i = 0; i < s1.data.size(); i++) {
		s.data[i] = s1.data[i] / t;
	}
	return s;
}
vec3 xyz(const Spectrum &s);
vec3 rgb(const Spectrum &s);
Spectrum RGBtoSpectrum(const vec3& rgb);

#endif
