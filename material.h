#ifndef MATERIAL_H
#define MATERIAL_H

#include <vector>
#include "vec3.h"
#include "onb.h"
#include "ray.h"
#include "hitable.h"
#include "spectrum.h"

class material {
	public:
		virtual bool sample(const hit_record& rec, const onb& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdf_val) const {
			return false;
		}
		virtual double BxDF(const vec3& vi, double wli, const vec3& vo, double wlo) const {
			return 0.0;
		}
		virtual double emitted(const ray& r, const hit_record& rec) const {
			return 0.0;
		}
		static std::vector<hitable *> lights;
		bool light_flag = false;
};

class lambertian : public material {
	public:
		lambertian(const Spectrum& a) : albedo(a) {}
		virtual bool sample(const hit_record& rec, const onb& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdf_val) const;
		virtual double BxDF(const vec3& vi, double wli, const vec3& vo, double wlo) const;


		Spectrum albedo;

};

class metal : public material {
	public:
		metal(const Spectrum& a) : albedo(a) {}
		virtual bool sample(const hit_record& rec, const onb& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdf_val) const;
		virtual double BxDF(const vec3& vi, double wli, const vec3& vo, double wlo) const;

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
		dielectric(const Spectrum& n, const Spectrum& k) : n(n), k(k) {}
		virtual bool sample(const hit_record& rec, const onb& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdf_val) const;

		double ref_B, ref_C;
		Spectrum albedo;
		Spectrum n, k;
		//double R0;
};


class oren_nayar : public material {
	public:
		oren_nayar(const Spectrum& albedo, double sigma) : albedo(albedo), sigma(sigma) {}
		virtual bool sample(const hit_record& rec, const onb& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdf_val) const;
		virtual double BxDF(const vec3& vi, double wli, const vec3& vo, double wlo) const;
		Spectrum albedo;
		double sigma;
};

class diffuse_light : public material {
	public:
		diffuse_light(Spectrum color) : light_color(color) {}
		virtual double emitted(const ray& r, const hit_record& rec) const;
		Spectrum light_color;
};


class mix_material : public material {
	public:
		mix_material(const material *mat1, const material *mat2, double fac)
		{
			this->mat1 = mat1;
			this->mat2 = mat2;
			this->fac = fac;
		}
		virtual double emitted(const ray& r, const hit_record& rec) const;
		virtual bool sample(const hit_record& rec, const onb& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdf_val) const;
		virtual double BxDF(const vec3& vi, double wli, const vec3& vo, double wlo) const;
		const material *mat1, *mat2;
		double fac;
};

/*
class straight_light : public material {
	public:
		straight_light(Spectrum color) : light_color(color) {}
		virtual bool sample(const ray& r_in, const hit_record& rec, ray& scattered, double&BxDF, double& pdf_val) const;
		virtual double emitted(double u, double v, const ray& r_in, const hit_record& rec) const;
		Spectrum light_color;
};
*/

#endif
