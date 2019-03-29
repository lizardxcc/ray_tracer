#ifndef MATERIAL_H
#define MATERIAL_H

#include <vector>
#include "vec3.h"
#include "ray.h"
#include "hitable.h"
#include "spectrum.h"

class material {
	public:
		virtual bool sample(const ray& r_in, const hit_record& rec, ray& scattered, double& BxDF, double& pdf_val) const = 0;
		virtual double BxDF(const ray& r_in, const hit_record& rec, const ray& scattered) const {
			return 0.0;
		}
		virtual double emitted(double u, double v, const ray& r_in, const hit_record& rec) const {
			return 0.0;
		}
		static std::vector<hitable *> lights;
};

class lambertian : public material {
	public:
		lambertian(const Spectrum& a) : albedo(a) {}
		virtual double BxDF(const ray& r_in, const hit_record& rec, const ray& scatterd) const;
		virtual bool sample(const ray& r_in, const hit_record& rec, ray& scattered, double& BxDF, double& pdf_val) const;


		Spectrum albedo;

};

class metal : public material {
	public:
		metal(const Spectrum& a, double f) : albedo(a)
		{
			if (f < 1.0)
				fuzz = f;
			else
				fuzz = 1.0;
		}
		virtual double BxDF(const ray& r_in, const hit_record& rec, const ray& scatterd) const;
		virtual bool sample(const ray& r_in, const hit_record& rec, ray& scattered, double& BxDF, double& pdf_val) const;


		Spectrum albedo;
		double fuzz;

};

class dielectric : public material {
	public:
		//dielectric(double ref_B, double ref_C) : ref_B(ref_B), ref_C(ref_C) {
		//	albedo = Spectrum(1.0);
		//	//R0 = pow((1.0-ref_idx)/(1.0+ref_idx), 2);
		//}
		//dielectric(const Spectrum& albedo, double ref_B, double ref_C) : albedo(albedo), ref_B(ref_B), ref_C(ref_C) {}
		dielectric(const Spectrum& n, const Spectrum& k) : n(n), k(k), ref_C(ref_C) {}
		virtual bool sample(const ray& r_in, const hit_record& rec, ray& scattered, double& CxDF, double& pdf_val) const;

		double ref_B, ref_C;
		Spectrum albedo;
		Spectrum n, k;
		//double R0;
};


class oren_nayar : public material {
	public:
		oren_nayar(const vec3& albedo, double sigma) : albedo(albedo), sigma(sigma) {}
		virtual bool sample(const ray& r_in, const hit_record& rec, ray& scattered, double& BxDF, double& pdf_val) const;
		vec3 albedo;
		double sigma;
};

class diffuse_light : public material {
	public:
		diffuse_light(Spectrum color) : light_color(color) {}
		virtual bool sample(const ray& r_in, const hit_record& rec, ray& scattered, double& BxDF, double& pdf_val) const;
		virtual double emitted(double u, double v, const ray& r_in, const hit_record& rec) const;
		Spectrum light_color;
};

class straight_light : public material {
	public:
		straight_light(Spectrum color) : light_color(color) {}
		virtual bool sample(const ray& r_in, const hit_record& rec, ray& scattered, double&BxDF, double& pdf_val) const;
		virtual double emitted(double u, double v, const ray& r_in, const hit_record& rec) const;
		Spectrum light_color;
};

#endif
