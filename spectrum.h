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
		inline Spectrum& operator=(const double);
		inline Spectrum& operator*=(const double);
		inline Spectrum& operator/=(const double);

		double integrate(double min_wl, double max_wl) const;
		void add(double value, double min_wl, double max_wl);
		double get(double wavelength) const;
		std::vector<double> data;
};

inline Spectrum& Spectrum::operator=(const double t)
{
	std::fill(data.begin(), data.end(), t);
	return *this;
}
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
dvec3 xyz(const Spectrum &s);
dvec3 rgb(const Spectrum &s);
dvec3 r_xyz(const Spectrum &s);
dvec3 r_rgb(const Spectrum &s);
Spectrum RGBtoSpectrum(const dvec3& rgb);


constexpr double LIGHT_SPEED = 299792458; // in m/s
//inline double freq_to_wavelength(double freq)
constexpr double freq_to_wavelength(double freq)
{
	return LIGHT_SPEED / freq / 1000.0;
}
//inline double wavelength_to_freq(double wavelength)
constexpr double wavelength_to_freq(double wavelength)
{
	return LIGHT_SPEED / wavelength / 1000.0;
}

constexpr double FSPECTRUM_START = 428;
constexpr double FSPECTRUM_END = 749;
constexpr double FSPECTRUM_RANGE = FSPECTRUM_END - FSPECTRUM_START; // in THz = 10^12 Hz
constexpr int FSPECTRUM_N_SAMPLE = 30;
constexpr double FSPECTRUM_SAMPLE_SIZE = FSPECTRUM_RANGE/FSPECTRUM_N_SAMPLE;

constexpr double WSPECTRUM_START = freq_to_wavelength(FSPECTRUM_END);
constexpr double WSPECTRUM_END = freq_to_wavelength(FSPECTRUM_START);
constexpr double WSPECTRUM_RANGE = WSPECTRUM_END - WSPECTRUM_START;
constexpr int WSPECTRUM_N_SAMPLE = FSPECTRUM_N_SAMPLE;
//constexpr double wspectrum_sample_size[WSPECTRUM_N_SAMPLE];

constexpr double freq_range_start(int i) {
	return FSPECTRUM_START + FSPECTRUM_SAMPLE_SIZE * (i);
}
constexpr double freq_range_end(int i) {
	return FSPECTRUM_START + FSPECTRUM_SAMPLE_SIZE * (i+1);
}
constexpr double freq_range_middle(int i) {
	return FSPECTRUM_START + FSPECTRUM_SAMPLE_SIZE * (static_cast<double>(i) + 0.5);
}
constexpr double wl_range_start(int i) {
	//return freq_to_wavelength(FSPECTRUM_START + FSPECTRUM_SAMPLE_SIZE * (FSPECTRUM_N_SAMPLE - (i)));
	return freq_to_wavelength(freq_range_end(FSPECTRUM_N_SAMPLE-(i+1)));
}
constexpr double wl_range_end(int i) {
	//return freq_to_wavelength(FSPECTRUM_START + FSPECTRUM_SAMPLE_SIZE * (FSPECTRUM_N_SAMPLE - (i+1)));
	return freq_to_wavelength(freq_range_start(FSPECTRUM_N_SAMPLE-(i+1)));
}
constexpr double wl_range_size(int i) {
	return wl_range_end(i)-wl_range_start(i);
}

// Class interval of wavelength based spectrum is not constant,
// so we use the midpoint of frequency based range as the midpoint of wavelegth based range
constexpr double wl_range_middle(int i) {
	return freq_to_wavelength(freq_range_middle(FSPECTRUM_N_SAMPLE-(i+1)));
}

double convert_spec_data_from_wl_to_freq(double data, double wavelength);
double convert_spec_data_from_freq_to_wl(double data, double freq);
using Sequencer = double(*)(int);
template<int N, Sequencer FN> struct Sequence {
	double array[N];
	constexpr Sequence() : array () {
		for (int i = 0; i < N; i++) {
			array[i] = FN(i);
		}
	}
};
constexpr auto freq_range_start_array = Sequence<FSPECTRUM_N_SAMPLE, freq_range_start>();
constexpr auto freq_range_end_array = Sequence<FSPECTRUM_N_SAMPLE, freq_range_end>();
constexpr auto freq_range_middle_array = Sequence<FSPECTRUM_N_SAMPLE, freq_range_middle>();
constexpr auto wl_range_start_array = Sequence<FSPECTRUM_N_SAMPLE, wl_range_start>();
constexpr auto wl_range_end_array = Sequence<FSPECTRUM_N_SAMPLE, wl_range_end>();
constexpr auto wl_range_size_array = Sequence<FSPECTRUM_N_SAMPLE, wl_range_size>();
constexpr auto wl_range_middle_array = Sequence<FSPECTRUM_N_SAMPLE, wl_range_middle>();

// c = ^c m/s
// \ = ^\ nm
// f = ^f THz
// c = f*\
// ^c = 10^3 * ^\ * ^f
//
// c = f*h
// f = c/h
// h = c/f
// 0 = df * h + f * dh
// - df * h = f * dh
// df = -f/h * dh
// df = -c/h^2 * dh
// |df| = f/h * |dh|
// |df|/f = |dh|/h
// h*|df| = f*|dh|


// I_f |df| = I_h |dh|
// I_f *f/h * |dh| = I_h |dh|
// I_f * f = I_h * h
// I_f = I_h * h/f
// I_f(f) = I_h(h) * h/f
// I_f(f) = I_h(h) * h^2/c
// I_f(f) = I_h(h) * c/f^2
// ^I_f * A / THz = ^I_h * A / nm * ^c * m/s / (^f * THz)^2
// ^I_f = ^I_h *THz/ nm * ^c * m/s / (^f)^2 / (THz)^2
// ^I_f = ^I_h *THz/ nm * ^c * m/s / (^f)^2 / (THz)^2
// ^I_f = ^I_h / nm * ^c * m/s / (^f)^2 / THz
// ^I_f = ^I_h *(m/ nm) * ^c / (^f)^2 /s / THz
// ^I_f = ^I_h *(10^9) * ^c / (^f)^2 /s / (10^12 Hz)
// ^I_f = ^I_h *(10^9) * ^c / (^f)^2 /s / (10^12 /s)
// ^I_f = ^I_h *(10^9) * ^c / (^f)^2 s/s /(10^12)
// ^I_f = ^I_h *(10^9) * ^c / (^f)^2  /(10^12)
// ^I_f = ^I_h  * ^c / (^f)^2  /(10^3)
// ^I_h = ^I_f / (^c / (^f)^2 / (10^3))
//



class FrequencyBasedDividedSpectrum {
	public:
		//inline Spectrum& operator=(const double);
		//explicit FrequencyBasedDividedSpectrum(const FrequencyBasedDividedSpectrum &a)
		//{
		//	data = a.data;
		//}
		//FrequencyBasedDividedSpectrum &operator=(const FrequencyBasedDividedSpectrum &a)
		//{
		//	if (this != &a) {
		//		data = a.data;
		//	}
		//	return *this;
		//}
		FrequencyBasedDividedSpectrum(void);
		std::vector<double> data;
};
class WSpectrum;

class FSpectrum : public FrequencyBasedDividedSpectrum {
	public:
		void Add(double data, double range_start, double range_end);
		WSpectrum ToWSpectrum(void);
};

class WSpectrum : public FrequencyBasedDividedSpectrum {
	public:
		void Add(double data, double range_start, double range_end);
		FSpectrum ToFSpectrum(void);
};
inline FSpectrum operator+(const FSpectrum &s1, const FSpectrum &s2)
{
	FSpectrum s;
	for (size_t i = 0; i < s1.data.size(); i++) {
		s.data[i] = s1.data[i] + s2.data[i];
	}
	return s;
}
inline FSpectrum operator-(const FSpectrum &s1, const FSpectrum &s2)
{
	FSpectrum s;
	for (size_t i = 0; i < s1.data.size(); i++) {
		s.data[i] = s1.data[i] - s2.data[i];
	}
	return s;
}
inline FSpectrum operator*(double c, const FSpectrum &s)
{
	FSpectrum result;
	for (size_t i = 0; i < s.data.size(); i++) {
		result.data[i] = c * s.data[i];
	}
	return s;
}
inline FSpectrum operator*(const FSpectrum &s, double c)
{
	return c * s;
}

inline WSpectrum operator+(const WSpectrum &s1, const WSpectrum &s2)
{
	WSpectrum s;
	for (size_t i = 0; i < s1.data.size(); i++) {
		s.data[i] = s1.data[i] + s2.data[i];
	}
	return s;
}
inline WSpectrum operator-(const WSpectrum &s1, const WSpectrum &s2)
{
	WSpectrum s;
	for (size_t i = 0; i < s1.data.size(); i++) {
		s.data[i] = s1.data[i] - s2.data[i];
	}
	return s;
}
inline WSpectrum operator*(double c, const WSpectrum &s)
{
	WSpectrum result;
	for (size_t i = 0; i < s.data.size(); i++) {
		result.data[i] = c * s.data[i];
	}
	return s;
}
inline WSpectrum operator*(const WSpectrum &s, double c)
{
	return c * s;
}


class ArbitrarySpectrum {
	public:
		void AddLast(double data, double range_start, double range_end);
		ArbitrarySpectrum(const double *data, size_t size, double range_start, double range_end);
	protected:
		std::vector<double> data;
		std::vector<std::pair<double, double>> range;
};

class FrequencyArbitrarySpectrum : public ArbitrarySpectrum {
	public:
		FrequencyArbitrarySpectrum(const double *data, size_t size, double range_start, double range_end);
		FSpectrum ToFSpectrum(void);
};

class WavelengthArbitrarySpectrum : public ArbitrarySpectrum {
	public:
		WavelengthArbitrarySpectrum(const double *data, size_t size, double range_start, double range_end);
		WSpectrum ToWSpectrum(void);
};


#endif
