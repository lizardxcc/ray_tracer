#include "material.h"


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

bool lambertian::scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
{
	vec3 target = rec.p + rec.normal + random_in_unit_sphere();
	scattered = ray(rec.p, target-rec.p);
	attenuation = albedo;
	return true;
}


bool metal::scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
{
	vec3 v = unit_vector(r_in.direction());
	vec3 reflected = reflect(v, rec.normal);
	scattered = ray(rec.p, reflected + fuzz*random_in_unit_sphere());
	attenuation = albedo;
	//return (dot(scattered.direction(), rec.normal) > 0);
	return true;
}



bool dielectric::scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const
{
	vec3 v = unit_vector(r_in.direction());
	attenuation = vec3(0.8, 0.8, 0.8);
	float n = 1.0;
	float critical_angle = asin(n / ref_idx);
	if (dot(-v, unit_vector(rec.normal)) >= 0.0) {
		// refraction
		vec3 v_p = refract(v, unit_vector(rec.normal), n, ref_idx);
		scattered = ray(rec.p, v_p);
	} else {
		vec3 inverse_normal = -unit_vector(rec.normal);

		if (dot(-v, inverse_normal) < cos(critical_angle)) {
			// total reflection
			vec3 reflected = reflect(v, inverse_normal);
			scattered = ray(rec.p, reflected);
			attenuation = vec3(1.0, 1.0, 1.0);
		} else {
			// refraction
			vec3 v_p = refract(v, inverse_normal, ref_idx, n);
			scattered = ray(rec.p, v_p);
		}
	}

	return true;
}
