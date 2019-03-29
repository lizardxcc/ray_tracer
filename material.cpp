#include "material.h"
#include "onb.h"
#include "pdf.h"
#include "object.h"

std::vector<hitable *> material::lights;

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

vec3 refract(vec3 v, vec3 normal, float n_in, float n_out)
{
	//vec3 normal = unit_vector(rec.normal);
	vec3 v2 = normal * dot(v, normal);
	vec3 v1 = v - v2;
	vec3 v1_p = (n_in / n_out) * v1.length() * unit_vector(v1);
	vec3 v2_p = -sqrt(1-v1_p.length()*v1_p.length()) * normal;
	return v1_p + v2_p;
}

float lambertian::BxDF(const ray& r_in, const hit_record& rec, const ray& scattered) const
{
	//float rho = 0.5;
	float cosine = dot(rec.normal, unit_vector(scattered.direction()));
	if (cosine < 0)
		return 0;

	//return rho / M_PI;
	return albedo.get(r_in.central_wl)/M_PI;
}

bool lambertian::sample(const ray& r_in, const hit_record& rec, ray& scattered, float& BxDF, float& pdf_val) const
{
	vec3 generated_direction;

	uniform_pdf uni_pdf(rec.normal);
	std::vector<pdf *> pdf_list(lights.size()+1);
	pdf_list[0] = &uni_pdf;
	for (size_t i = 1; i < pdf_list.size(); i++) {
		pdf_list[i] = new hitable_pdf(lights[i-1], rec.p);
	}
	mixture_pdf pdf(pdf_list);

	generated_direction = pdf.generate();
	pdf_val = pdf.pdf_val(generated_direction);

	scattered = ray(rec.p, unit_vector(generated_direction));
	scattered.min_wl = r_in.min_wl;
	scattered.max_wl = r_in.max_wl;
	scattered.central_wl = r_in.central_wl;
	//attenuation = albedo;

	for (size_t i = 1; i < pdf_list.size(); i++) {
		delete pdf_list[i];
	}

	BxDF = this->BxDF(r_in, rec, scattered);
	return true;
}


float metal::BxDF(const ray& r_in, const hit_record& rec, const ray& scattered) const
{
	float cosine = dot(rec.normal, unit_vector(scattered.direction()));
	if (cosine < 0)
		return 0;

	vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);

	return albedo.get(r_in.central_wl) * 3.0*pow(dot(unit_vector(scattered.direction()), unit_vector(reflected)), 100) / M_PI;
	return pow(dot(unit_vector(scattered.direction()), unit_vector(reflected)), 100);
}

bool metal::sample(const ray& r_in, const hit_record& rec, ray& scattered, float& BxDF, float& pdf) const
{
	vec3 normal = unit_vector(rec.normal);
	vec3 v = unit_vector(r_in.direction());
	float abs_cos_o = abs(dot(v, normal));
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

/*
bool metal::scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered, float& pdf) const
{
	vec3 v = unit_vector(r_in.direction());
	vec3 reflected = reflect(v, rec.normal);
	scattered = ray(rec.p, reflected + fuzz*random_in_unit_sphere());
	attenuation = albedo;
	//return (dot(scattered.direction(), rec.normal) > 0);
	return true;
}
*/


float shlick(float theta, float n1, float n2)
{
	float R0 = pow((n1-n2)/(n1+n2), 2.0);
	return R0 + (1-R0)*pow(1-cos(theta), 5.0);
}


bool dielectric::sample(const ray& r_in, const hit_record& rec, ray& scattered, float& BxDF, float& pdf_val) const
{

	//float ref_idx = ref_B + ref_C / pow(r_in.central_wl/1000.0, 2.0);
	float ref_idx = n.get(r_in.central_wl);
	float alpha = 4.0 * M_PI * k.get(r_in.central_wl) / (r_in.central_wl * pow(10, -9));
	//alpha *= 0.000002;
	//alpha = 0.05;
	alpha *= rec.t;

	//vec3 v = unit_vector(r_in.direction());
	vec3 vo = -unit_vector(r_in.direction());
	vec3 normal = unit_vector(rec.normal);
	float cos_o = dot(vo, normal);
	float n_vacuum = 1.0;
	float n_in, n_out;
	float fresnel;
	if (cos_o >= 0.0) {
		n_in = ref_idx;
		n_out = n_vacuum;
		alpha = 0.0;
	} else {
		n_in = n_vacuum;
		n_out = ref_idx;
		cos_o = abs(cos_o);
		normal = -normal;
		//std::cout << rec.t << std::endl;
		//std::cout << alpha << std::endl;
	}
	float sin_o = sqrt(std::max(0.0, 1.0-cos_o*cos_o));
	float sin_t = n_out/n_in * sin_o;
	bool total_internal_reflection = false;
	float cos_t;
	if (sin_t >= 1.0) { // total internal reflection
		fresnel = 1.0;
		total_internal_reflection = true;
		//std::cout << "total internal reflection" << std::endl;
	} else {
		cos_t = sqrt(std::max(0.0, 1.0-sin_t*sin_t));
		float r_p = (n_in*cos_o - n_out*cos_t)/(n_in*cos_o + n_out*cos_t);
		float r_s = (n_out*cos_o - n_in*cos_t)/(n_out*cos_o + n_in*cos_t);
		fresnel = (r_p*r_p + r_s*r_s)/2.0;
		//float R0 = pow((n_out-n_in)/(n_out+n_in), 2);
		//fresnel = R0 + (1-R0) * pow(1-cos_t, 5);
	}

	float rand = drand48();
	// mutiple importance sampling
	if (rand <= fresnel || total_internal_reflection) { // reflection (includes total internal reflection)
		vec3 v_in = unit_vector(r_in.direction());
		vec3 v = v_in+2*cos_o*normal;
		scattered = ray(rec.p, unit_vector(v));
		pdf_val = fresnel;
		BxDF = fresnel/cos_o;
	} else { // refraction
		vec3 v_in = unit_vector(r_in.direction());
		vec3 v_para = unit_vector(v_in + cos_o * normal);
		vec3 v_1 = (-cos_t) * normal;
		vec3 v_2 = sin_t * v_para;
		scattered = ray(rec.p, v_1+v_2);
		pdf_val = 1.0-fresnel;
		BxDF = ((n_out*n_out)/(n_in*n_in)) * (1.0-fresnel) / cos_t;
	}
	BxDF *= exp(-alpha);
	scattered.min_wl = r_in.min_wl;
	scattered.max_wl = r_in.max_wl;
	scattered.central_wl = r_in.central_wl;
	//attenuation = albedo;

	return true;
}


bool oren_nayar::sample(const ray& r_in, const hit_record& rec, ray& scattered, float& BxDF, float& pdf_val) const
{
	vec3 generated_direction;

	uniform_pdf uni_pdf(rec.normal);
	std::vector<pdf *> pdf_list(lights.size()+1);
	pdf_list[0] = &uni_pdf;
	for (size_t i = 1; i < pdf_list.size(); i++) {
		pdf_list[i] = new hitable_pdf(lights[i-1], rec.p);
	}
	mixture_pdf pdf(pdf_list);

	generated_direction = pdf.generate();
	pdf_val = pdf.pdf_val(generated_direction);

	scattered = ray(rec.p, unit_vector(generated_direction));
	scattered.min_wl = r_in.min_wl;
	scattered.max_wl = r_in.max_wl;
	scattered.central_wl = r_in.central_wl;
	//attenuation = albedo;

	for (size_t i = 1; i < pdf_list.size(); i++) {
		delete pdf_list[i];
	}

	vec3 vo = -unit_vector(r_in.direction());
	vec3 normal = unit_vector(rec.normal);
	float cos_theta_i = dot(generated_direction, normal);
	if (cos_theta_i < 0) {
		BxDF = 0;
		return true;
	}
	float cos_theta_o = dot(vo, normal);
	float cos_alpha = std::min(cos_theta_i, cos_theta_o);
	float cos_beta = std::max(cos_theta_i, cos_theta_o);
	float sin_alpha = sqrt(1.0-cos_alpha*cos_alpha);
	float tan_beta = sqrt(1.0/(cos_beta*cos_beta) - 1.0);

	float A = 1 - sigma*sigma/(2*(sigma*sigma + 0.33));
	float B = 0.45 * sigma * sigma / (sigma*sigma+0.09);

	vec3 hori_vo = vo - cos_theta_o * normal;
	vec3 hori_scattered = generated_direction - cos_theta_i * normal;
	float tmp = std::max(0.0f, dot(hori_vo, hori_scattered));

	BxDF = 1.0/M_PI * (A + B * tmp * sin_alpha * tan_beta);

	//BxDF = this->BxDF(r_in, rec, scattered);
	return true;
}

bool diffuse_light::sample(const ray& r_in, const hit_record& rec, ray& scattered, float& BxDF, float& pdf) const
{
	return false;
}

float diffuse_light::emitted(float u, float v, const ray& r_in, const hit_record& rec) const
{
	//return vec3(1.0, 1.0, 1.0);
	return light_color.integrate(r_in.min_wl, r_in.max_wl);
	//return 10.0;
}

bool straight_light::sample(const ray& r_in, const hit_record& rec, ray& scattered, float& BxDF, float& pdf) const
{
	return false;
}

float straight_light::emitted(float u, float v, const ray& r_in, const hit_record& rec) const
{
	//return light_color*pow(dot(unit_vector(rec.normal), -unit_vector(r_in.direction())), 20.0);
	//return light_color.integrate(r_in.min_wl, r_in.max_wl) * pow(dot(unit_vector(rec.normal), -unit_vector(r_in.direction())), 100);
	float t = 0.0;
	float d = dot(unit_vector(rec.normal), -unit_vector(r_in.direction()));
	if (abs(d) >= 0.95)
		t = 1.0;
	return light_color.integrate(r_in.min_wl, r_in.max_wl) * t;
}
