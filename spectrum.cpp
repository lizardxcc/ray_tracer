#include <cmath>
#include "spectrum.h"

// CVRL
static const double cie_x[N_SAMPLE] = {
	4.742986E-02, // 405nm
	1.446214E-01, // 415nm
	2.488523E-01, // 425nm
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
	1.575417E-02 // 695nm
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

double convert_spec_data_from_wl_to_freq(double data, double wavelength)
{
	return data*(wavelength*wavelength * 1000.0)/LIGHT_SPEED;
}
double convert_spec_data_from_freq_to_wl(double data, double freq)
{
	return data*(freq*freq * 1000.0)/LIGHT_SPEED;
}
FrequencyBasedDividedSpectrum::FrequencyBasedDividedSpectrum(void) : data(FSPECTRUM_N_SAMPLE, 0.0)
{
}

void WSpectrum::Add(double value, double range_start, double range_end)
{
	for (size_t i = 0; i < data.size(); i++) {
		//double i_freq_start = FSPECTRUM_START+FSPECTRUM_SAMPLE_SIZE*(FSPECTRUM_N_SAMPLE-i);
		//double i_freq_end = FSPECTRUM_START+FSPECTRUM_SAMPLE_SIZE*(FSPECTRUM_N_SAMPLE-(i+1));
		//double i_freq_middle = (i_freq_start + i_freq_end) / 2.0; // frequency based middle
		double i_wavelength_start = wl_range_start_array.array[i];
		double i_wavelength_end = wl_range_end_array.array[i];
		//double i_wavelength_middle = freq_to_wavelength(_freq_middle); // frequency based middle
		double i_wavelength_middle = (i_wavelength_start + i_wavelength_end)/2.0;
		double i_wavelength_range_size = wl_range_size_array.array[i];
		if (range_end < i_wavelength_start)
			continue;
		if (i_wavelength_end < range_start)
			continue;
		double start = std::max(i_wavelength_start, range_start);
		double end = std::min(i_wavelength_end, range_end);
		if (end < start) {
			std::cout << "error start: " << start << " end: " << end << std::endl;
			continue;
		}
		//double sample_size = i_wavelength_end - i_wavelength_end;
		data[i] += value * (end-start)/i_wavelength_range_size;
	}
}

void FSpectrum::Add(double value, double range_start, double range_end)
{
	for (size_t i = 0; i < data.size(); i++) {
		double start = FSPECTRUM_START+FSPECTRUM_SAMPLE_SIZE*i;
		if (range_end < start)
			continue;
		double end = FSPECTRUM_START+FSPECTRUM_SAMPLE_SIZE*(i+1);
		if (end < range_start)
			continue;
		start = std::max(start, range_start);
		end = std::min(end, range_end);
		if (end < start) {
			std::cout << "error start: " << start << " end: " << end << std::endl;
			continue;
		}
		data[i] += value * (end-start)/FSPECTRUM_SAMPLE_SIZE;
	}
	//int min_i = (int)std::ceil((range_start-FSPECTRUM_START) / FSPECTRUM_SAMPLE_SIZE);
	//std::cout << "min: " << (range_start-FSPECTRUM_START)/FSPECTRUM_SAMPLE_SIZE << " " << min_i << std::endl;
	//int max_i = (int)std::floor((range_end-FSPECTRUM_START) / FSPECTRUM_SAMPLE_SIZE)-1;
	//std::cout << "max: " << (range_end-FSPECTRUM_START)/FSPECTRUM_SAMPLE_SIZE << " " << max_i << std::endl;
	//std::cout << "min_i :" << min_i << std::endl;
	//std::cout << "max_i :" << max_i << std::endl;
	//min_i = std::max(0, min_i);
	//max_i = std::min(FSPECTRUM_N_SAMPLE-1, max_i);
	////if (min_i < 0)
	////	min_i = 0;
	////if (max_i >= FSPECTRUM_N_SAMPLE)
	////	max_i = FSPECTRUM_N_SAMPLE-1;
	//for (size_t i = min_i; i <= max_i; i++) {
	//	data[i] += value;
	//}
	//if (min_i-1 >= 0) {
	//	double a = value * (FSPECTRUM_START+min_i * FSPECTRUM_SAMPLE_SIZE -range_start) / FSPECTRUM_SAMPLE_SIZE;
	//	std::cout << "min a: " << a << std::endl;
	//	data[min_i-1] +=  a;
	//}
	//if (max_i+1 < FSPECTRUM_N_SAMPLE) {
	//	double a = value * (range_end - ((max_i)*FSPECTRUM_SAMPLE_SIZE+FSPECTRUM_START)) / FSPECTRUM_SAMPLE_SIZE;
	//	std::cout << "max a: "  << a << std::endl;
	//	data[max_i+1] += a;
	//}
}


WSpectrum FSpectrum::ToWSpectrum(void)
{
	WSpectrum result;
	for (size_t i = 0; i < FSPECTRUM_N_SAMPLE; i++) {
		size_t wl_i = FSPECTRUM_N_SAMPLE - (i+1);
		result.data[wl_i] = FSPECTRUM_SAMPLE_SIZE * convert_spec_data_from_freq_to_wl(data[i], freq_range_middle_array.array[i]) / wl_range_size_array.array[wl_i];
	}
	return result;
}

FSpectrum WSpectrum::ToFSpectrum(void)
{
	FSpectrum result;
	for (size_t i = 0; i < data.size(); i++) {
		size_t freq_i = FSPECTRUM_N_SAMPLE - (i+1);
		result.data[freq_i] = wl_range_size_array.array[i] * convert_spec_data_from_wl_to_freq(data[i], wl_range_middle_array.array[i]) / FSPECTRUM_SAMPLE_SIZE;
	}
	return result;
}
void ArbitrarySpectrum::AddLast(double data, double range_start, double range_end)
{
	this->data.push_back(data);
	range.push_back(std::make_pair(range_start, range_end));
}

FSpectrum FrequencyArbitrarySpectrum::ToFSpectrum(void)
{
	FSpectrum result;
	for (size_t i = 0; i < data.size(); i++) {
		result.Add(data[i], range[i].first, range[i].second);
	}
	return result;
}


ArbitrarySpectrum::ArbitrarySpectrum(const double *data, size_t size, double range_start, double range_end) : data(size), range(size)
{
	double range_size = range_end - range_start;
	double interval_size = static_cast<double>(range_size)/static_cast<double>(size);
	for (size_t i = 0; i < size; i++) {
		double start = interval_size*i;
		double end = interval_size*(i+1);
		this->data[i] = data[i];
		this->range[i] = std::make_pair(start, end);
	}
}
FrequencyArbitrarySpectrum::FrequencyArbitrarySpectrum(const double *data, size_t size, double range_start, double range_end) : ArbitrarySpectrum(data, size, range_start, range_end)
{
}
WavelengthArbitrarySpectrum::WavelengthArbitrarySpectrum(const double *data, size_t size, double range_start, double range_end) : ArbitrarySpectrum(data, size, range_start, range_end)
{
}

WSpectrum WavelengthArbitrarySpectrum::ToWSpectrum(void)
{
	WSpectrum result;
	for (size_t i = 0; i < data.size(); i++) {
		result.Add(data[i], range[i].first, range[i].second);
	}
	return result;
}
const WavelengthArbitrarySpectrum cie_x_spec(cie_x, WSPECTRUM_N_SAMPLE, 400.0, 700.0);
const WavelengthArbitrarySpectrum cie_y_spec(cie_y, WSPECTRUM_N_SAMPLE, 400.0, 700.0);
const WavelengthArbitrarySpectrum cie_z_spec(cie_z, WSPECTRUM_N_SAMPLE, 400.0, 700.0);
