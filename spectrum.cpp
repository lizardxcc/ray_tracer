#include <cmath>
#include "spectrum.h"


float Spectrum::integrate(float min_wl, float max_wl) const
{
	float sum = 0.0;
	size_t min_i = (size_t)std::ceil((min_wl-400) / SAMPLE_SIZE);
	float mod_min = std::fmod(min_wl-400, RANGE / data.size());
	size_t max_i = (size_t)std::floor((max_wl-400) / SAMPLE_SIZE);
	float mod_max = std::fmod(max_wl-400, RANGE / data.size());
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

void Spectrum::add(float value, float min_wl, float max_wl)
{
	size_t min_i = (size_t)std::ceil((min_wl-400) / SAMPLE_SIZE);
	size_t max_i = (size_t)std::floor((max_wl-400) / SAMPLE_SIZE);
	for (size_t i = min_i; i <= max_i; i++) {
		data[i] += value;
	}
}


float Spectrum::get(float wavelength) const
{
	size_t i = (size_t)std::floor((wavelength - 400) / SAMPLE_SIZE);
	return data[i];
}

vec3 xyz(const Spectrum &s)
{
	Spectrum X, Y, Z;
	X.data[0] = 0.07763;
	X.data[1] = 0.34806;
	X.data[2] = 0.14210;
	X.data[3] = 0.00240;
	X.data[4] = 0.22575;
	X.data[5] = 0.67840;
	X.data[6] = 1.05670;
	X.data[7] = 0.75140;
	X.data[8] = 0.21870;
	X.data[9] = 0.03290;

	Y.data[0] = 0.00218;
	Y.data[1] = 0.02980;
	Y.data[2] = 0.11260;
	Y.data[3] = 0.40730;
	Y.data[4] = 0.91485;
	Y.data[5] = 0.97808;
	Y.data[6] = 0.69490;
	Y.data[7] = 0.32100;
	Y.data[8] = 0.08160;
	Y.data[9] = 0.01192;

	Z.data[0] = 0.37130;
	Z.data[1] = 1.78260;
	Z.data[2] = 1.04190;
	Z.data[3] = 0.21230;
	Z.data[4] = 0.02984;
	Z.data[5] = 0.00275;
	Z.data[6] = 0.00100;
	Z.data[7] = 0.00100;
	Z.data[8] = 0.00000;
	Z.data[9] = 0.00000;

	vec3 xyz;
	xyz.e[0] = (s * X).integrate(400, 699.999);
	xyz.e[1] = (s * Y).integrate(400, 699.999);
	xyz.e[2] = (s * Z).integrate(400, 699.999);
	return xyz;
}



vec3 rgb(const Spectrum &s)
{
	vec3 _xyz = xyz(s);
	vec3 rgb;
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
