#include "material.h"


vec3 random_in_unit_sphere(void)
{
	vec3 p;
	do {
		p = 2.0*vec3(drand48(), drand48(), drand48()) - vec3(1, 1, 1);
	} while (p.squared_length() >= 1.0);

	return p;
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
	vec3 reflected = v - 2*dot(v, rec.normal)*rec.normal;
	scattered = ray(rec.p, reflected + fuzz*random_in_unit_sphere());
	attenuation = albedo;
	//return (dot(scattered.direction(), rec.normal) > 0);
	return true;
}
