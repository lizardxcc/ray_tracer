#include <cmath>
#include "spectrum.h"

static const double cie_x[N_SAMPLE] = {
	4.742986E-02,
	1.446214E-01,
	2.488523E-01,
	3.227087E-01,
	3.418483E-01,
	2.826646E-01,
	2.219781E-01,
	1.29192E-01,
	4.600865E-02,
	7.097731E-03,
	3.649178E-03,
	4.315171E-02,
	1.268468E-01,
	2.405015E-01,
	3.804244E-01,
	5.280233E-01,
	7.016774E-01,
	8.853376E-01,
	1.051011E+00,
	1.14362E+00,
	1.134757E+00,
	1.007344E+00,
	8.135565E-01,
	5.75541E-01,
	3.844986E-01,
	2.277792E-01,
	1.263808E-01,
	6.63996E-02,
	3.292138E-02,
	1.575417E-02
};

static const double cie_y[N_SAMPLE] = {
	4.971717E-03,
	1.429377E-02,
	2.612106E-02,
	4.15794E-02,
	5.743393E-02,
	7.238339E-02,
	1.060145E-01,
	1.535066E-01,
	2.064828E-01,
	2.85068E-01,
	4.277595E-01,
	6.206256E-01,
	7.946448E-01,
	9.071347E-01,
	9.814106E-01,
	9.994608E-01,
	9.902549E-01,
	9.424569E-01,
	8.587203E-01,
	7.544785E-01,
	6.270066E-01,
	4.89595E-01,
	3.609245E-01,
	2.416902E-01,
	1.547397E-01,
	8.979594E-02,
	4.899699E-02,
	2.554223E-02,
	1.261573E-02,
	6.027677E-03
};

static const double cie_z[N_SAMPLE] = {
	2.369246E-01,
	7.378822E-01,
	1.305008E+00,
	1.74828E+00,
	1.918437E+00,
	1.664439E+00,
	1.42844E+00,
	9.991789E-01,
	5.617313E-01,
	3.105939E-01,
	1.720018E-01,
	8.283548E-02,
	3.751912E-02,
	1.566174E-02,
	6.131421E-03,
	2.327186E-03,
	8.822531E-04,
	3.386739E-04,
	1.335031E-04,
	5.460706E-05,
	2.334738E-05,
	1.048387E-05,
	0E+00,
	0E+00,
	0E+00,
	0E+00,
	0E+00,
	0E+00,
	0E+00,
	0E+00
};
static const Spectrum X(cie_x);
static const Spectrum Y(cie_y);
static const Spectrum Z(cie_z);

Spectrum::Spectrum(void) : data(N_SAMPLE, 0.0)
{
}
Spectrum::Spectrum(double v)
{
	data.resize(N_SAMPLE);
	for (auto& d : data) {
		d = v;
	}
}
Spectrum::Spectrum(const double d[N_SAMPLE])
{
	data.resize(N_SAMPLE);
	for (int i = 0; i < N_SAMPLE; i++) {
		data[i] = d[i];
	}
}


double Spectrum::integrate(double min_wl, double max_wl) const
{
	double sum = 0.0;
	size_t min_i = (size_t)std::ceil((min_wl-400) / SAMPLE_SIZE);
	double mod_min = std::fmod(min_wl-400, RANGE / data.size());
	size_t max_i = (size_t)std::floor((max_wl-400) / SAMPLE_SIZE);
	double mod_max = std::fmod(max_wl-400, RANGE / data.size());
	if (min_i >= 1) {
		//sum += data[min_i-1] * (SAMPLE_SIZE * min_i - min_wl);
		sum += data[min_i-1] * mod_min;
	}
	if (max_i <= (data.size()-2)) {
		sum += data[max_i+1] * mod_max;;
	}

	for (size_t i = min_i; i <= max_i; i++) {
		sum += data[i] * SAMPLE_SIZE;
	}

	return sum;
}

void Spectrum::add(double value, double min_wl, double max_wl)
{
	size_t min_i = (size_t)std::ceil((min_wl-400) / SAMPLE_SIZE);
	size_t max_i = (size_t)std::floor((max_wl-400) / SAMPLE_SIZE);
	for (size_t i = min_i; i <= max_i; i++) {
		data[i] += value;
	}
}


double Spectrum::get(double wavelength) const
{
	size_t i = (size_t)std::floor((wavelength - 400) / SAMPLE_SIZE);
	return data[i];
}

dvec3 xyz(const Spectrum &s)
{
	dvec3 xyz;
	xyz.e[0] = (s * X).integrate(400, 699.999);
	xyz.e[1] = (s * Y).integrate(400, 699.999);
	xyz.e[2] = (s * Z).integrate(400, 699.999);
	return xyz;
}

dvec3 r_xyz(const Spectrum &s)
{
	dvec3 _xyz;
	_xyz = xyz(s);
	double k = 1.0 / Y.integrate(400, 699.999);
	_xyz *= k;
	return _xyz;
}



dvec3 rgb(const Spectrum &s)
{
	dvec3 _xyz = xyz(s);
	dvec3 rgb;
	rgb.e[0] = 3.240479 * _xyz.x() +
		(-1.537150) * _xyz.y() +
		(-0.498535) * _xyz.z();
	rgb.e[1] = (-0.969256) * _xyz.x() +
		1.875991 * _xyz.y() +
		0.041556 * _xyz.z();
	rgb.e[2] = 0.055648 * _xyz.x() +
		(-0.204043) * _xyz.y() +
		1.057311 * _xyz.z();

	return rgb;
}

dvec3 r_rgb(const Spectrum &s)
{
	dvec3 _xyz = r_xyz(s);
	dvec3 rgb;
	rgb.e[0] = 3.240479 * _xyz.x() +
		(-1.537150) * _xyz.y() +
		(-0.498535) * _xyz.z();
	rgb.e[1] = (-0.969256) * _xyz.x() +
		1.875991 * _xyz.y() +
		0.041556 * _xyz.z();
	rgb.e[2] = 0.055648 * _xyz.x() +
		(-0.204043) * _xyz.y() +
		1.057311 * _xyz.z();

	return rgb;
}


Spectrum RGBtoSpectrum(const dvec3& rgb)
{
	Spectrum s;
	for (int i = 0; i < N_SAMPLE/3.0; i++) {
		s.data[i] = rgb[2];
	}
	for (int i = N_SAMPLE/3.0; i < N_SAMPLE/3.0*2.0; i++) {
		s.data[i] = rgb[1];
	}
	for (int i = N_SAMPLE/3.0*2.0; i < N_SAMPLE; i++) {
		s.data[i] = rgb[0];
	}
	return s;
}
