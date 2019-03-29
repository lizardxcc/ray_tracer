#ifndef MATERIAL_H
#define MATERIAL_H

#include <vector>
#include "vec3.h"
#include "ray.h"
#include "hitable.h"
#include "spectrum.h"

class material {
	public:
		virtual bool sample(const ray& r_in, const hit_record& rec, ray& scattered, float& BxDF, float& pdf_val) const = 0;
		virtual float BxDF(const ray& r_in, const hit_record& rec, const ray& scattered) const {
			return 0.0;
		}
		virtual float emitted(float u, float v, const ray& r_in, const hit_record& rec) const {
			return 0.0;
		}
		static std::vector<hitable *> lights;
};

class lambertian : public material {
	public:
		lambertian(const Spectrum& a) : albedo(a) {}
		virtual float BxDF(const ray& r_in, const hit_record& rec, const ray& scatterd) const;
		virtual bool sample(const ray& r_in, const hit_record& rec, ray& scattered, float& BxDF, float& pdf_val) const;


		Spectrum albedo;

};

class metal : public material {
	public:
		metal(const Spectrum& a, float f) : albedo(a)
		{
			if (f < 1.0)
				fuzz = f;
			else
				fuzz = 1.0;
		}
		virtual float BxDF(const ray& r_in, const hit_record& rec, const ray& scatterd) const;
		virtual bool sample(const ray& r_in, const hit_record& rec, ray& scattered, float& BxDF, float& pdf_val) const;


		Spectrum albedo;
		float fuzz;

};

class dielectric : public material {
	public:
		//dielectric(float ref_B, float ref_C) : ref_B(ref_B), ref_C(ref_C) {
		//	albedo = Spectrum(1.0);
		//	//R0 = pow((1.0-ref_idx)/(1.0+ref_idx), 2);
		//}
		//dielectric(const Spectrum& albedo, float ref_B, float ref_C) : albedo(albedo), ref_B(ref_B), ref_C(ref_C) {}
		dielectric(const Spectrum& n, const Spectrum& k) : n(n), k(k), ref_C(ref_C) {}
		virtual bool sample(const ray& r_in, const hit_record& rec, ray& scattered, float& CxDF, float& pdf_val) const;

		float ref_B, ref_C;
		Spectrum albedo;
		Spectrum n, k;
		//float R0;
};


class oren_nayar : public material {
	public:
		oren_nayar(const vec3& albedo, float sigma) : albedo(albedo), sigma(sigma) {}
		virtual bool sample(const ray& r_in, const hit_record& rec, ray& scattered, float& BxDF, float& pdf_val) const;
		vec3 albedo;
		float sigma;
};

class diffuse_light : public material {
	public:
		diffuse_light(Spectrum color) : light_color(color) {}
		virtual bool sample(const ray& r_in, const hit_record& rec, ray& scattered, float& BxDF, float& pdf_val) const;
		virtual float emitted(float u, float v, const ray& r_in, const hit_record& rec) const;
		Spectrum light_color;
};

class straight_light : public material {
	public:
		straight_light(Spectrum color) : light_color(color) {}
		virtual bool sample(const ray& r_in, const hit_record& rec, ray& scattered, float&BxDF, float& pdf_val) const;
		virtual float emitted(float u, float v, const ray& r_in, const hit_record& rec) const;
		Spectrum light_color;
};

#endif
