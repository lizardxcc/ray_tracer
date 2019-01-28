#ifndef MATERIAL_H
#define MATERIAL_H

#include "vec3.h"
#include "ray.h"
#include "hitable.h"

class material {
	public:
		virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const = 0;
		virtual vec3 emitted(float u, float v, const vec3& p) const {
			return vec3(0, 0, 0);
		}
};

class lambertian : public material {
	public:
		lambertian(const vec3& a) : albedo(a) {}
		virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const;


		vec3 albedo;

};

class metal : public material {
	public:
		metal(const vec3& a, float f) : albedo(a)
		{
			if (f < 1.0)
				fuzz = f;
			else
				fuzz = 1.0;
		}
		virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const;


		vec3 albedo;
		float fuzz;

};

class dielectric : public material {
	public:
		dielectric(float ref_idx) : ref_idx(ref_idx) {}
		virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const;

		float ref_idx;
};

class diffuse_light : public material {
	public:
		diffuse_light() {}
		virtual bool scatter(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered) const;
		virtual vec3 emitted(float u, float v, const vec3& p) const;
};

#endif
