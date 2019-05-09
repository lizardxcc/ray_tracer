#include "material.h"
#include "onb.h"
#include "pdf.h"
#include "object.h"

std::vector<std::shared_ptr<hitable> > material::lights;

vec3 random_in_unit_sphere(void)
{
	vec3 p;
	do {
		p = 2.0*vec3(drand48(), drand48(), drand48()) - vec3(1, 1, 1);
	} while (p.squared_length() >= 1.0);

	return p;
}



vec3 reflect(vec3 v, vec3 normal)
{
	return (v - 2 * normal * dot(v, normal));
}

vec3 refract(vec3 v, vec3 normal, double n_in, double n_out)
{
	//vec3 normal = unit_vector(rec.normal);
	vec3 v2 = normal * dot(v, normal);
	vec3 v1 = v - v2;
	vec3 v1_p = (n_in / n_out) * v1.length() * unit_vector(v1);
	vec3 v2_p = -sqrt(1-v1_p.length()*v1_p.length()) * normal;
	return v1_p + v2_p;
}

double lambertian::BxDF(const vec3& vi, double wli, const vec3& vo, double wlo) const
{
	if (vi.z() < 0.0)
		return 0.0;

	return albedo.get(wli)/M_PI;
}

bool lambertian::sample(const hit_record& rec, const onb& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdf_val) const
{
	//std::vector<pdf *> pdf_list(lights.size()+1);
	//pdf_list[0] = new uniform_pdf(rec.normal);
	//for (size_t i = 1; i < pdf_list.size(); i++) {
	//	pdf_list[i] = new hitable_pdf(lights[i-1], rec.p);
	//}
	//mixture_pdf pdf(pdf_list);
	hitable_pdf pdf(lights[0], rec.p);

	vec3 generated_direction = pdf.generate();
	pdf_val = pdf.pdf_val(generated_direction);
	//for (size_t i = 0; i < pdf_list.size(); i++) {
	//	delete pdf_list[i];
	//}
	vi = uvw.worldtolocal(generated_direction);

	wli = wlo;
	BxDF = this->BxDF(vi, wli, vo, wlo);
	return true;
}


double metal::BxDF(const vec3& vi, double wli, const vec3& vo, double wlo) const
{
	if (vi.z() < 0.0)
		return 0.0;

	return albedo.get(wli) / abs(vi.z());
}
bool metal::sample(const hit_record& rec, const onb& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdf_val) const
{
	vi[0] = -vo[0];
	vi[1] = -vo[1];
	vi[2] = vo[2];
	wli = wlo;
	BxDF = this->BxDF(vi, wli, vo, wlo);
	pdf_val = 1;
	return true;
}
/*
double metal::BxDF(const ray& r_in, const hit_record& rec, const ray& scattered) const
{
	double cosine = dot(rec.normal, unit_vector(scattered.direction()));
	if (cosine < 0)
		return 0;

	vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);

	return albedo.get(r_in.central_wl) * 3.0*pow(dot(unit_vector(scattered.direction()), unit_vector(reflected)), 100) / M_PI;
	return pow(dot(unit_vector(scattered.direction()), unit_vector(reflected)), 100);
}

bool metal::sample(const ray& r_in, const hit_record& rec, ray& scattered, double& BxDF, double& pdf) const
{
	vec3 normal = unit_vector(rec.normal);
	vec3 v = unit_vector(r_in.direction());
	double abs_cos_o = abs(dot(v, normal));
	vec3 r_out = v + 2*abs_cos_o*normal;
	scattered = ray(rec.p, unit_vector(r_out));
	scattered.min_wl = r_in.min_wl;
	scattered.max_wl = r_in.max_wl;
	scattered.central_wl = r_in.central_wl;
	//attenuation = albedo;
	BxDF = 1.0/abs_cos_o;
	pdf = 1;
	return true;
}
*/

/*
bool metal::scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered, double& pdf) const
{
	vec3 v = unit_vector(r_in.direction());
	vec3 reflected = reflect(v, rec.normal);
	scattered = ray(rec.p, reflected + fuzz*random_in_unit_sphere());
	attenuation = albedo;
	//return (dot(scattered.direction(), rec.normal) > 0);
	return true;
}
*/


double shlick(double theta, double n1, double n2)
{
	double R0 = pow((n1-n2)/(n1+n2), 2.0);
	return R0 + (1-R0)*pow(1-cos(theta), 5.0);
}


bool dielectric::sample(const hit_record& rec, const onb& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdf_val) const
{

	//double ref_idx = ref_B + ref_C / pow(r_in.central_wl/1000.0, 2.0);
	double ref_idx = n.get(wlo);
	double alpha = 4.0 * M_PI * k.get(wlo) / (wlo * pow(10, -9));
	alpha *= rec.t;

	double cos_o = vo.z();
	double n_vacuum = 1.0;
	double n_in, n_out;
	double fresnel;
	vec3 normal;
	if (cos_o >= 0.0) {
		// from outside to inside of object
		n_in = ref_idx;
		n_out = n_vacuum;
		alpha = 0.0;
		normal = vec3(0, 0, 1);
	} else {
		// from inside to outside of object
		n_in = n_vacuum;
		n_out = ref_idx;
		cos_o = abs(cos_o);
		normal = vec3(0, 0, -1);
	}
	double sin_o = sqrt(std::max(0.0, 1.0-cos_o*cos_o));
	double sin_t = n_out/n_in * sin_o;
	bool total_internal_reflection = false;
	double cos_t;
	if (sin_t >= 1.0) { // total internal reflection
		fresnel = 1.0;
		total_internal_reflection = true;
	} else {
		cos_t = sqrt(std::max(0.0, 1.0-sin_t*sin_t));
		//double r_p = (n_in*cos_o - n_out*cos_t)/(n_in*cos_o + n_out*cos_t);
		//double r_s = (n_out*cos_o - n_in*cos_t)/(n_out*cos_o + n_in*cos_t);
		//fresnel = (r_p*r_p + r_s*r_s)/2.0;
		double R0 = pow((n_out-n_in)/(n_out+n_in), 2);
		fresnel = R0 + (1-R0) * pow(1-cos_t, 5);
	}

	double rand = drand48();
	// mutiple importance sampling
	if (rand <= fresnel || total_internal_reflection) { // reflection (includes total internal reflection)
		vi[0] = -vo[0];
		vi[1] = -vo[1];
		vi[2] = vo[2];
		pdf_val = fresnel;
		BxDF = fresnel/cos_o;
	} else { // refraction
		vi = (-cos_t) * normal - sin_t * unit_vector(vec3(vo[0], vo[1], 0));
		pdf_val = 1.0-fresnel;
		BxDF = ((n_out*n_out)/(n_in*n_in)) * (1.0-fresnel) / cos_t;
	}
	BxDF *= exp(-alpha);
	wli = wlo;

	return true;
}

double oren_nayar::BxDF(const vec3& vi, double wli, const vec3& vo, double wlo) const
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

	vec3 hori_vo = vec3(vo[0], vo[1], 0);
	vec3 hori_scattered = vec3(vi[0], vi[1], 0);
	double tmp = std::max(0.0, dot(hori_vo, hori_scattered));

	return albedo.get(wli)/M_PI * (A + B * tmp * sin_alpha * tan_beta);
}

bool oren_nayar::sample(const hit_record& rec, const onb& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdf_val) const
{
	std::vector<pdf *> pdf_list(lights.size()+1);
	pdf_list[0] = new uniform_pdf(rec.normal);
	for (size_t i = 1; i < pdf_list.size(); i++) {
		pdf_list[i] = new hitable_pdf(lights[i-1], rec.p);
	}
	mixture_pdf pdf(pdf_list);

	vec3 generated_direction = pdf.generate();
	pdf_val = pdf.pdf_val(generated_direction);
	for (size_t i = 0; i < pdf_list.size(); i++) {
		delete pdf_list[i];
	}
	vi = uvw.worldtolocal(generated_direction);

	wli = wlo;
	BxDF = this->BxDF(vi, wli, vo, wlo);

	return true;
}

double diffuse_light::emitted(const ray& r, const hit_record& rec) const
{
	return light_color.integrate(r.min_wl, r.max_wl);
}

double mix_material::emitted(const ray& r, const hit_record& rec) const
{
	if (drand48() < fac) {
		return mat2->emitted(r, rec);
	} else {
		return mat1->emitted(r, rec);
	}
}

bool mix_material::sample(const hit_record& rec, const onb& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdf_val) const
{
	if (drand48() < fac) {
		return mat2->sample(rec, uvw, vo, wlo, vi, wli, BxDF, pdf_val);
	} else {
		return mat1->sample(rec, uvw, vo, wlo, vi, wli, BxDF, pdf_val);
	}
}

double mix_material::BxDF(const vec3& vi, double wli, const vec3& vo, double wlo) const
{
	if (drand48() < fac) {
		return mat2->BxDF(vi, wli, vo, wlo);
	} else {
		return mat1->BxDF(vi, wli, vo, wlo);
	}
}
/*
double straight_light::emitted(double u, double v, const ray& r_in, const hit_record& rec) const
{
	//return light_color*pow(dot(unit_vector(rec.normal), -unit_vector(r_in.direction())), 20.0);
	//return light_color.integrate(r_in.min_wl, r_in.max_wl) * pow(dot(unit_vector(rec.normal), -unit_vector(r_in.direction())), 100);
	double t = 0.0;
	double d = dot(unit_vector(rec.normal), -unit_vector(r_in.direction()));
	if (abs(d) >= 0.95)
		t = 1.0;
	return light_color.integrate(r_in.min_wl, r_in.max_wl) * t;
}
*/
