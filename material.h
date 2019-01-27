#ifndef MATERIAL_H
#define MATERIAL_H

#include "vec3.h"
#include "ray.h"
#include "hitable.h"

class material {
	public:
		virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const = 0;
};

class lambertian : public material {
	public:
		lambertian(const vec3& a) : albedo(a) {}
		virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const;


		vec3 albedo;

};

#endif
