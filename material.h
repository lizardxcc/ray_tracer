#ifndef MATERIAL_H
#define MATERIAL_H

#include <vector>
#include "vec3.h"
#include "ray.h"
#include "hitable.h"

class material {
	public:
		virtual bool sample(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered, float& BxDF, float& pdf_val) const = 0;
		virtual float BxDF(const ray& r_in, const hit_record& rec, const ray& scattered) const {
			return 0.0;
		}
		virtual vec3 emitted(float u, float v, const ray& r_in, const hit_record& rec) const {
			return vec3(0, 0, 0);
		}
		static std::vector<hitable *> lights;
};

class lambertian : public material {
	public:
		lambertian(const vec3& a) : albedo(a) {}
		virtual float BxDF(const ray& r_in, const hit_record& rec, const ray& scatterd) const;
		virtual bool sample(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered, float& BxDF, float& pdf_val) const;


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
		virtual float BxDF(const ray& r_in, const hit_record& rec, const ray& scatterd) const;
		virtual bool sample(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered, float& BxDF, float& pdf_val) const;


		vec3 albedo;
		float fuzz;

};

class dielectric : public material {
	public:
		dielectric(float ref_idx) : ref_idx(ref_idx) {
			albedo = vec3(1.0, 1.0, 1.0);
			//R0 = pow((1.0-ref_idx)/(1.0+ref_idx), 2);
		}
		dielectric(const vec3& albedo, float ref_idx) : albedo(albedo), ref_idx(ref_idx) {}
		virtual bool sample(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered, float& BxDF, float& pdf_val) const;

		float ref_idx;
		vec3 albedo;
		//float R0;
};


class oren_nayar : public material {
	public:
		oren_nayar(const vec3& albedo, float sigma) : albedo(albedo), sigma(sigma) {}
		virtual bool sample(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered, float& BxDF, float& pdf_val) const;
		vec3 albedo;
		float sigma;
};

class diffuse_light : public material {
	public:
		diffuse_light(vec3 color) : light_color(color) {}
		virtual bool sample(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered, float& BxDF, float& pdf_val) const;
		virtual vec3 emitted(float u, float v, const ray& r_in, const hit_record& rec) const;
		vec3 light_color;
};

class straight_light : public material {
	public:
		straight_light(vec3 color) : light_color(color) {}
		virtual bool sample(const ray& r_in, const hit_record& rec, vec3& attenuation, ray& scattered, float&BxDF, float& pdf_val) const;
		virtual vec3 emitted(float u, float v, const ray& r_in, const hit_record& rec) const;
		vec3 light_color;
};

#endif
