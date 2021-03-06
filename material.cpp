#include "material.h"
#include "onb.h"
#include "pdf.h"
#include "object.h"
#include "stb_image.h"

const dvec3 default_vt(0, 0, 0);

// the approximation of shlick assumes that n is greater than or equal to 1
double shlick(double cos_theta, double n)
{
	return ((n-1)*(n-1)+4.0*n*pow(1.0-cos_theta, 5))/((n+1)*(n+1));
}
double rfresnel(double cos_theta, double n)
{
	const double sin_theta = sqrt(1.0-cos_theta*cos_theta);
	const double sin_t = sin_theta/n;
	if (sin_t > 1.0)
		return 1.0;
	const double sqrt_n_squared_minus_sin_theta_squared = sqrt(n*n-sin_theta*sin_theta);
	const double rte = (cos_theta - sqrt_n_squared_minus_sin_theta_squared)/(cos_theta + sqrt_n_squared_minus_sin_theta_squared);
	const double rtm = (n*n*cos_theta - sqrt_n_squared_minus_sin_theta_squared)/(n*n*cos_theta + sqrt_n_squared_minus_sin_theta_squared);
	return (rte*rte+rtm*rtm)/2.0;
	//return shlick(sqrt(1-sin_beta*sin_beta), 1.0/n);
}
double cfresnel(double cos_theta, std::complex<double> n)
{
	const double sin_theta = sqrt(1.0-cos_theta*cos_theta);
	const double sin_t = sin_theta/std::real(n);
	if (sin_t > 1.0)
		return 1.0;
	const double r = std::abs((cos_theta - sqrt(n*n-sin_theta*sin_theta))/(cos_theta + sqrt(n*n-sin_theta*sin_theta)));
	return r*r;
}

double newshlick(double cos_theta, double n, double k)
{
	return ((n-1.0)*(n-1.0)+4.0*n*pow(1.0-cos_theta, 5)+k*k)/((n+1)*(n+1)+k*k);
}


double Homogenious::Phase(const dvec3& vi, double wli, const dvec3& vo, double wlo) const
{
	return 1.0/(4.0 * M_PI);
}

bool Homogenious::Sample_p(const dvec3& vo, double wlo, dvec3& vi, double& wli, double& phase, double& pdfval) const
{
	double phi = 2.0 * M_PI * drand48();
	double cos_theta = 1.0 - 2.0 * drand48();
	double sin_theta = sqrt(1.0-cos_theta*cos_theta);
	double x = sin(phi) * cos_theta;
	double y = sin(phi) * sin_theta;
	double z = cos(phi);
	vi = dvec3(x, y, z);
	wli = wlo;
	phase = 1.0/(4.0 * M_PI);
	pdfval = 1.0/(4.0 * M_PI);
	return true;
}

double HenyeyGreenstein::Phase(const dvec3& vi, double wli, const dvec3& vo, double wlo) const
{
	const double cos_theta = dot(vi, vo);
	const double denom = 1.0+g*g+2.0*g*cos_theta;
	return (1.0-g*g)/(denom*sqrt(denom)*(4.0 * M_PI));
}

bool HenyeyGreenstein::Sample_p(const dvec3& vo, double wlo, dvec3& vi, double& wli, double& phase, double& pdfval) const
{
	double phi = 2.0 * M_PI * drand48();
	double square = (1.0-g*g)/(1.0-g+2.0*g*drand48());
	double cos_theta = 0.5 / g *(1.0 + g*g - square*square);
	double sin_theta = sqrt(1.0-cos_theta*cos_theta);
	double x = sin(phi) * cos_theta;
	double y = sin(phi) * sin_theta;
	double z = cos(phi);
	vi = dvec3(x, y, z);
	wli = wlo;
	phase = this->Phase(vi, wli, vo, wlo);
	pdfval = phase;
	return true;
}


//std::vector<std::shared_ptr<Hittable> > Material::lights;

dvec3 random_in_unit_Sphere(void)
{
	dvec3 p;
	do {
		p = 2.0*dvec3(drand48(), drand48(), drand48()) - dvec3(1, 1, 1);
	} while (p.squared_length() >= 1.0);

	return p;
}

dvec3 reflect2(dvec3 v, dvec3 normal)
{
	return (-v + 2.0 * normal * dot(v, normal));
}

dvec3 refract(dvec3 v, dvec3 normal, double n) // this n is a relative refraction index
{
	//dvec3 normal = unit_vector(rec.normal);
	dvec3 v2 = normal * dot(v, normal);
	dvec3 v1 = v - v2;
	dvec3 v1_p = -(1.0/n) * v1.length() * unit_vector(v1);
	dvec3 v2_p = -sqrt(1-v1_p.length()*v1_p.length()) * normal;
	return v1_p + v2_p;
}
//dvec3 refract(dvec3 v, dvec3 normal, double n_in, double n_out)
//{
//	//dvec3 normal = unit_vector(rec.normal);
//	dvec3 v2 = normal * dot(v, normal);
//	dvec3 v1 = v - v2;
//	dvec3 v1_p = (n_in / n_out) * v1.length() * unit_vector(v1);
//	dvec3 v2_p = -sqrt(1-v1_p.length()*v1_p.length()) * normal;
//	return v1_p + v2_p;
//}

/*
double Lambertian::BxDF(const dvec3& vi, double wli, const dvec3& vo, double wlo, const dvec3& vt) const
{
	if (vi.z() < 0.0)
		return 0.0;

	return albedo.get(wli)/M_PI;
}

bool Lambertian::Sample(const HitRecord& rec, const ONB& uvw, const dvec3& vo, double wlo, dvec3& vi, double& wli, double& BxDF, double& pdfval) const
{
	CosinePdf Pdf(rec.normal);

	dvec3 generated_direction = Pdf.Generate();
	pdfval = Pdf.PdfVal(generated_direction);
	vi = uvw.WorldToLocal(generated_direction);

	wli = wlo;
	BxDF = this->BxDF(vi, wli, vo, wlo);
	return true;
}


double Metal::BxDF(const dvec3& vi, double wli, const dvec3& vo, double wlo, const dvec3& vt) const
{
	if (vi.z() < 0.0)
		return 0.0;

	return albedo.get(wli) / abs(vi.z());
}
bool Metal::Sample(const HitRecord& rec, const ONB& uvw, const dvec3& vo, double wlo, dvec3& vi, double& wli, double& BxDF, double& pdfval) const
{
	double ref_idx = n.get(wlo);
	double kv = k.get(wlo);

	double cos_o = vo.z();
	double n_vacuum = 1.0;
	std::complex<double> n1, n2;
	if (cos_o >= 0.0) {
		n1 = n_vacuum;
		n2 = std::complex<double>(ref_idx, kv);
	} else {
		n1 = std::complex<double>(ref_idx, kv);
		n2 = n_vacuum;
		cos_o = abs(cos_o);
	}
	std::complex<double> relative_ref_idx = n2/n1;
	const double sin_o = sqrt(std::max(0.0, 1.0-cos_o*cos_o));
	const double fresnel = cfresnel(cos_o, relative_ref_idx);

	vi[0] = -vo[0];
	vi[1] = -vo[1];
	vi[2] = vo[2];
	pdfval = 1.0;
	BxDF = fresnel/cos_o;

	wli = wlo;

	return true;

	//vi[0] = -vo[0];
	//vi[1] = -vo[1];
	//vi[2] = vo[2];
	//wli = wlo;
	//BxDF = this->BxDF(vi, wli, vo, wlo);
	//pdfval = 1;
	//return true;
}
*/
/*
double Metal::BxDF(const ray& r_in, const HitRecord& rec, const ray& scattered) const
{
	double cosine = dot(rec.normal, unit_vector(scattered.direction()));
	if (cosine < 0)
		return 0;

	dvec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);

	return albedo.get(r_in.central_wl) * 3.0*pow(dot(unit_vector(scattered.direction()), unit_vector(reflected)), 100) / M_PI;
	return pow(dot(unit_vector(scattered.direction()), unit_vector(reflected)), 100);
}

bool Metal::Sample(const ray& r_in, const HitRecord& rec, ray& scattered, double& BxDF, double& Pdf) const
{
	dvec3 normal = unit_vector(rec.normal);
	dvec3 v = unit_vector(r_in.direction());
	double abs_cos_o = abs(dot(v, normal));
	dvec3 r_out = v + 2*abs_cos_o*normal;
	scattered = ray(rec.p, unit_vector(r_out));
	scattered.min_wl = r_in.min_wl;
	scattered.max_wl = r_in.max_wl;
	scattered.central_wl = r_in.central_wl;
	//attenuation = albedo;
	BxDF = 1.0/abs_cos_o;
	Pdf = 1;
	return true;
}
*/

/*
bool Metal::scatter(const ray& r_in, const HitRecord& rec, dvec3& attenuation, ray& scattered, double& Pdf) const
{
	dvec3 v = unit_vector(r_in.direction());
	dvec3 reflected = reflect(v, rec.normal);
	scattered = ray(rec.p, reflected + fuzz*random_in_unit_Sphere());
	attenuation = albedo;
	//return (dot(scattered.direction(), rec.normal) > 0);
	return true;
}
*/


//double shlick(double cos_theta, double n1, double n2)
//{
//	double R0 = pow((n1-n2)/(n1+n2), 2.0);
//	return R0 + (1-R0)*pow(1-cos_theta, 5.0);
//}



/*
bool Dielectric::Sample(const HitRecord& rec, const ONB& uvw, const dvec3& vo, double wlo, dvec3& vi, double& wli, double& BxDF, double& pdfval) const
{

	//double ref_idx = ref_B + ref_C / pow(r_in.central_wl/1000.0, 2.0);
	double ref_idx = n.get(wlo);
	double kv = k.get(wlo);

	double cos_o = vo.z();
	double n_vacuum = 1.0;
	std::complex<double> n1, n2;
	double fresnel;
	dvec3 normal;
	if (cos_o >= 0.0) {
		// from outside to inside of object
		//n_second = ref_idx;
		//n_first = n_vacuum;
		n1 = n_vacuum;
		n2 = std::complex<double>(ref_idx, kv);
		normal = dvec3(0, 0, 1);
	} else {
		// from inside to outside of object
		//n_second = n_vacuum;
		//n_first = ref_idx;
		n1 = std::complex<double>(ref_idx, kv);
		n2 = n_vacuum;
		cos_o = abs(cos_o);
		normal = dvec3(0, 0, -1);
	}
	//double relative_ref_idx = std::real(n2)/std::real(n1);
	std::complex<double> relative_ref_idx = n2/n1;
	fresnel = cfresnel(cos_o, relative_ref_idx);
	//double sin_t = n_first/n_second * sin_o;
	//bool total_internal_reflection = false;
	//if (sin_t >= 1.0) { // total internal reflection
	//	fresnel = 1.0;
	//	total_internal_reflection = true;
	//} else {
	//	cos_t = sqrt(std::max(0.0, 1.0-sin_t*sin_t));
	//	fresnel = shlick
	//	//double r_p = (n_second*cos_o - n_first*cos_t)/(n_second*cos_o + n_first*cos_t);
	//	//double r_s = (n_first*cos_o - n_second*cos_t)/(n_first*cos_o + n_second*cos_t);
	//	//fresnel = (r_p*r_p + r_s*r_s)/2.0;

	//	//if (n_first <= n_second) {
	//	//	//fresnel = shlick(cos_o, n_first, n_second);
	//	//	fresnel = newshlick(cos_o, ref_idx, kv);
	//	//} else {
	//	//	//fresnel = shlick(cos_t, n_first, n_second);
	//	//	fresnel = newshlick(cos_t, ref_idx, kv);
	//	//}
	//	//double R0 = pow((n_first-n_second)/(n_first+n_second), 2);
	//	//fresnel = R0 + (1-R0) * pow(1-cos_t, 5);
	//}

	double rand = drand48();
	// mutiple importance sampling
	//if (rand <= fresnel || total_internal_reflection) { // reflection (includes total internal reflection)
	if (rand <= fresnel) { // reflection (includes total internal reflection)
		vi[0] = -vo[0];
		vi[1] = -vo[1];
		vi[2] = vo[2];
		//pdfval = fresnel;
		//BxDF = fresnel/cos_o;
		pdfval = 1.0;
		BxDF = 1.0/cos_o;
	} else { // refraction
		double sin_o = sqrt(std::max(0.0, 1.0-cos_o*cos_o));
		double sin_t = sin_o / std::real(relative_ref_idx);
		double cos_t = sqrt(1.0-sin_t*sin_t);
		vi = (-cos_t) * normal - sin_t * unit_vector(dvec3(vo[0], vo[1], 0));
		//pdfval = 1.0-fresnel;
		//BxDF = ((n_first*n_first)/(n_second*n_second)) * (1.0-fresnel) / cos_t;
		pdfval = 1.0;
		BxDF = 1.0/(pow(std::real(relative_ref_idx), 2)) / cos_t;
		//BxDF = 1.0/(n*n) * / cos_t;
	}
	wli = wlo;

	return true;
}

double OrenNayar::BxDF(const dvec3& vi, double wli, const dvec3& vo, double wlo, const dvec3& vt) const
{
	double cos_theta_i = vi.z();
	if (cos_theta_i < 0) {
		return 0.0;
	}
	double cos_theta_o = vo.z();
	double cos_alpha = std::min(cos_theta_i, cos_theta_o);
	double cos_beta = std::max(cos_theta_i, cos_theta_o);
	double sin_alpha = sqrt(1.0-cos_alpha*cos_alpha);
	double tan_beta = sqrt(1.0/(cos_beta*cos_beta) - 1.0);

	double A = 1 - sigma*sigma/(2*(sigma*sigma + 0.33));
	double B = 0.45 * sigma * sigma / (sigma*sigma+0.09);

	dvec3 hori_vo = dvec3(vo[0], vo[1], 0);
	dvec3 hori_scattered = dvec3(vi[0], vi[1], 0);
	double tmp = std::max(0.0, dot(hori_vo, hori_scattered));

	return albedo.get(wli)/M_PI * (A + B * tmp * sin_alpha * tan_beta);
}

bool OrenNayar::Sample(const HitRecord& rec, const ONB& uvw, const dvec3& vo, double wlo, dvec3& vi, double& wli, double& BxDF, double& pdfval) const
{
	//std::vector<std::unique_ptr<Pdf> > Pdf_list(lights.size()+1);
	//Pdf_list[0] = std::make_unique<UniformPdf>(rec.normal);
	//for (size_t i = 1; i < Pdf_list.size(); i++) {
	//	Pdf_list[i] = std::make_unique<HittablePdf>(lights[i-1], rec.p);
	//}
	//MixturePdf Pdf(std::move(Pdf_list));

	//dvec3 generated_direction = Pdf.Generate();
	//pdfval = Pdf.PdfVal(generated_direction);
	//vi = uvw.WorldToLocal(generated_direction);

	//wli = wlo;
	//BxDF = this->BxDF(vi, wli, vo, wlo);

	return true;
}


double Microfacet::BxDF(const dvec3& vi, double wli, const dvec3& vo, double wlo, const dvec3& vt) const
{
	double cos_theta_i = vi.z();
	if (cos_theta_i < 0) {
		return 0.0;
	}

	dvec3 vh = (vi+vo)/2.0;

	//double cos_theta_h_squared = vh.z() * vh.z();
	//double tan_theta_h_squared = 1.0/cos_theta_h_squared - 1.0;
	//// beckman spizzichino
	//double d = exp(-tan_theta_h_squared/(alpha*alpha)) / (M_PI * alpha * alpha * cos_theta_h_squared * cos_theta_h_squared);
	
	//GGX
	//double d = alpha * alpha / (M_PI * pow((alpha*alpha-1.0)*vh.z()*vh.z()+1.0, 2.0)
	double d = 1.0;

	//double g = 1 / (1.0 + lambda(vi) + lambda(vo));
	double g = 1.0;

	double fresnel = 1.0;



	//return albedo.get(wli) * d * g * fresnel / (4.0 * vo.z() * vi.z());
	return albedo.get(wli) * d * g * fresnel / (4.0 * vo.z() * vh.z());
}

double Microfacet::lambda(const dvec3& v) const
{
	double cos_theta_squared = v.z()*v.z();
	double tan_theta = sqrt(1.0/cos_theta_squared - 1.0);
	double a = 1.0 / alpha / tan_theta;
	return 0.5 * (std::erf(a) - 1.0 + exp(-a*a) / (a * sqrt(M_PI)));
}


bool Microfacet::Sample(const HitRecord& rec, const ONB& uvw, const dvec3& vo, double wlo, dvec3& vi, double& wli, double& BxDF, double& pdfval) const
{
	//double phi = 2.0 * M_PI * drand48();
	//double tan_theta = sqrt(-alpha*alpha*log(1.0-drand48()));
	
	double phi = 2 * M_PI * drand48();
	double u = drand48();
	double theta = acos(sqrt((1.0-u)/((alpha*alpha-1.0)*u+1)));
	double tan_theta = tan(theta);
	double cos_theta_squared = cos(theta)*cos(theta);
	double sin_theta = sin(theta);
	//double tan_theta = alpha * sqrt(u/(1.0-u));
	//double cos_theta_squared = 1.0/(1.0+tan_theta * tan_theta);
	//double sin_theta = sqrt(1.0 - cos_theta_squared);
	//

	dvec3 micro_normal;
	micro_normal[0] = sin_theta * cos(phi);
	micro_normal[1] = sin_theta * sin(phi);
	micro_normal[2] = sqrt(cos_theta_squared);


	double ref_idx = n.get(wlo);
	double kv = k.get(wlo);

	double n_vacuum = 1.0;
	std::complex<double> n1, n2;
	double cos_o;
	if (vo.z() >= 0.0) {
		cos_o = dot(vo, micro_normal);
		if (cos_o < 0) {
			pdfval = 1.0;
			BxDF = 0.0;
			return true;
		}
		n1 = n_vacuum;
		n2 = std::complex<double>(ref_idx, kv);
	} else {
		micro_normal = -micro_normal;
		cos_o = dot(vo, micro_normal);
		if (cos_o < 0) {
			pdfval = 1.0;
			BxDF = 0.0;
			return true;
		}
		n1 = std::complex<double>(ref_idx, kv);
		n2 = n_vacuum;
	}
	std::complex<double> relative_ref_idx = n2/n1;
	double sin_o = sqrt(std::max(0.0, 1.0-cos_o*cos_o));
	double fresnel = cfresnel(cos_o, relative_ref_idx);

	if (enable_refraction) {
		if (drand48() < fresnel) {
			vi = reflect2(vo, micro_normal);
			wli = wlo;
			pdfval = 1.0;
			double g = 1.0/(1.0+lambda(vi)) * 1.0/(1.0+lambda(vo));
			BxDF = g * abs(dot(vo, micro_normal)) / (abs(vo.z()*vi.z()*micro_normal.z()));
		} else {
			vi = refract(vo, micro_normal, std::real(relative_ref_idx));
			wli = wlo;
			pdfval = 1.0;
			double g = 1.0/(1.0+lambda(vi)) * 1.0/(1.0+lambda(vo));
			BxDF = g * abs(dot(vi, micro_normal)) / (abs(vi.z()*vo.z()*micro_normal.z()));
		}
	} else {
		vi = reflect2(vo, micro_normal);
		wli = wlo;
		pdfval = 1.0;
		double g = 1.0/(1.0+lambda(vi)) * 1.0/(1.0+lambda(vo));
		BxDF = fresnel * g * abs(dot(vo, micro_normal)) / (abs(vo.z()*vi.z()*micro_normal.z()));
	}

	return true;
}

double GGX::BxDF(const dvec3& vi, double wli, const dvec3& vo, double wlo, const dvec3& vt) const
{
	double cos_theta_i = vi.z();
	if (cos_theta_i < 0) {
		return 0.0;
	}

	dvec3 vh = (vi+vo)/2.0;

	double cos_theta_h_squared = vh.z() * vh.z();
	double tan_theta_h_squared = 1.0/cos_theta_h_squared - 1.0;
	// GGX
	double d = alpha*alpha/(M_PI * pow(1.0 - (1.0 - alpha*alpha)*vh.z()*vh.z(), 2.0));

	//double g = 1 / (1.0 + lambda(vi) + lambda(vo));
	double g = 1.0;

	double fresnel = 1.0;

	return albedo.get(wli) * d * g * fresnel / (4.0 * vo.z() * vi.z());
}

double GGX::lambda(const dvec3& v) const
{
	double cos_theta_squared = v.z()*v.z();
	double tan_theta = sqrt(1.0/cos_theta_squared - 1.0);
	double a = 1.0 / alpha / tan_theta;
	return 0.5 * (std::erf(a) - 1.0 + exp(a*a) / (a * sqrt(M_PI)));
}

bool GGX::Sample(const HitRecord& rec, const ONB& uvw, const dvec3& vo, double wlo, dvec3& vi, double& wli, double& BxDF, double& pdfval) const
{
	CosinePdf Pdf(rec.normal);

	dvec3 generated_direction = Pdf.Generate();
	pdfval = Pdf.PdfVal(generated_direction);
	vi = uvw.WorldToLocal(generated_direction);

	wli = wlo;
	BxDF = this->BxDF(vi, wli, vo, wlo);

	return true;
}

*/
