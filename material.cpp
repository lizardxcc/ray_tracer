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
	float rho = 0.5;
	float cosine = dot(rec.normal, unit_vector(scattered.direction()));
	if (cosine < 0)
		return 0;

	return rho / M_PI;
}

bool lambertian::scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered, float& pdf_val) const
{
	vec3 generated_direction;

	uniform_pdf uni_pdf(rec.normal);
	std::vector<pdf *> pdf_list(lights.size()+1);
	pdf_list[0] = &uni_pdf;
	for (size_t i = 0; i < lights.size(); i++) {
		pdf_list[i+1] = new hitable_pdf(lights[i], rec.p);
	}
	mixture_pdf pdf(pdf_list);

	generated_direction = pdf.generate();
	pdf_val = pdf.pdf_val(generated_direction);

	scattered = ray(rec.p, unit_vector(generated_direction));
	attenuation = albedo;
	return true;
}


float metal::BxDF(const ray& r_in, const hit_record& rec, const ray& scattered) const
{
	float cosine = dot(rec.normal, unit_vector(scattered.direction()));
	if (cosine < 0)
		return 0;

	vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);

	return 3.0*pow(dot(unit_vector(scattered.direction()), unit_vector(reflected)), 100) / M_PI;
	return pow(dot(unit_vector(scattered.direction()), unit_vector(reflected)), 100);
}

bool metal::scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered, float& pdf) const
{
	onb uvw;
	uvw.build_from_w(rec.normal);
	//vec3 generated_direction = uvw.local(random_on_unit_hemisphere());
	vec3 generated_direction = reflect(unit_vector(r_in.direction()), rec.normal);

	scattered = ray(rec.p, unit_vector(generated_direction));
	attenuation = albedo;
	//pdf = 1/(2*M_PI);
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


bool dielectric::scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered, float& pdf) const
{
	vec3 v = unit_vector(r_in.direction());
	attenuation = vec3(0.8, 0.8, 0.8);
	float n = 1.0;
	float critical_angle = asin(n / ref_idx);
	if (dot(-v, unit_vector(rec.normal)) >= 0.0) {
		vec3 v_p;
		// from outside to inside
		if (drand48() <= shlick(dot(-v, unit_vector(rec.normal)), 1.0, ref_idx)) {
			// reflection
			v_p = reflect(v, unit_vector(rec.normal));
		} else {
			// refraction
			v_p = refract(v, unit_vector(rec.normal), n, ref_idx);
		}
		scattered = ray(rec.p, v_p);

	} else {
		vec3 inverse_normal = -unit_vector(rec.normal);

		if (dot(-v, inverse_normal) < cos(critical_angle)) {
			// total reflection
			vec3 reflected = reflect(v, inverse_normal);
			scattered = ray(rec.p, reflected);
			attenuation = vec3(1.0, 1.0, 1.0);
		} else {
			float costheta_t = sqrt(1-ref_idx*ref_idx / (1.0*1.0) * (1- pow(dot(-v, unit_vector(inverse_normal)), 2.0)));

			vec3 v_p;
			if (drand48() <= shlick(costheta_t, 1.0, ref_idx)) {
				// reflection
				v_p = reflect(v, inverse_normal);
			} else {
				// refraction
				v_p = refract(v, inverse_normal, ref_idx, n);
			}
			scattered = ray(rec.p, v_p);
		}
	}

	return true;
}

bool diffuse_light::scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered, float& pdf) const
{
	return false;
}

vec3 diffuse_light::emitted(float u, float v, const vec3& p) const
{
	//return vec3(1.0, 1.0, 1.0);
	return light_color;
}
