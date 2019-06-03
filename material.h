#ifndef Material_H
#define Material_H

#include <vector>
#include "vec3.h"
#include "onb.h"
#include "ray.h"
#include "Hittable.h"
#include "spectrum.h"

class MediumMaterial {
	public:
		MediumMaterial(const Spectrum& sigma_t, const Spectrum& albedo) : sigma_t(sigma_t), albedo(albedo) {
		}
		virtual bool Sample_p(const vec3& vo, double wlo, vec3& vi, double& wli, double& phase, double& pdfval) const {
			return false;
		}
		virtual double Phase(const vec3& vi, double wli, const vec3& vo, double wlo) const = 0;
		Spectrum sigma_t;
		Spectrum albedo;
};

class Homogenious : public MediumMaterial {
	public:
		Homogenious(const Spectrum& sigma_t, const Spectrum& albedo) : MediumMaterial(sigma_t, albedo) {}
		bool Sample_p(const vec3& vo, double wlo, vec3& vi, double& wli, double& phase, double& pdfval) const;
		double Phase(const vec3& vi, double wli, const vec3& vo, double wlo) const;
};

class HenyeyGreenstein : public MediumMaterial {
	public:
		HenyeyGreenstein(const Spectrum& sigma_t, const Spectrum& albedo, double g) : MediumMaterial(sigma_t, albedo), g(g) {}
		bool Sample_p(const vec3& vo, double wlo, vec3& vi, double& wli, double& phase, double& pdfval) const;
		double Phase(const vec3& vi, double wli, const vec3& vo, double wlo) const;
		double g;
};

class Material {
	public:
		virtual bool Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const {
			return false;
		}
		virtual double BxDF(const vec3& vi, double wli, const vec3& vo, double wlo) const {
			return 0.0;
		}
		virtual double Emitted(const ray& r, const HitRecord& rec) const {
			return 0.0;
		}
		//std::shared_ptr<MediumMaterial> mi;
		MediumMaterial *mi = nullptr;
		static std::vector<std::shared_ptr<Hittable> > lights;
		bool light_flag = false;
		bool specular_flag = false;
};

class Lambertian : public Material {
	public:
		Lambertian(const Spectrum& a) : albedo(a) {}
		bool Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const;
		double BxDF(const vec3& vi, double wli, const vec3& vo, double wlo) const;


		Spectrum albedo;

};

class Metal : public Material {
	public:
		Metal(const Spectrum& a) : albedo(a) {
			specular_flag = true;
		}
		bool Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const;
		double BxDF(const vec3& vi, double wli, const vec3& vo, double wlo) const;

		Spectrum albedo;
		double fuzz;

};

class Dielectric : public Material {
	public:
		//Dielectric(double ref_B, double ref_C) : ref_B(ref_B), ref_C(ref_C) {
		//	albedo = Spectrum(1.0);
		//	//R0 = pow((1.0-ref_idx)/(1.0+ref_idx), 2);
		//}
		//Dielectric(const Spectrum& albedo, double ref_B, double ref_C) : albedo(albedo), ref_B(ref_B), ref_C(ref_C) {}
		Dielectric(const Spectrum& n) : n(n) {
			// k is deprecated argument
			k = Spectrum(0.0);
			specular_flag = true;
		}
		Dielectric(const Spectrum& n, const Spectrum& k) : n(n), k(k) {
			specular_flag = true;
		}
		bool Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const;

		double ref_B, ref_C;
		Spectrum albedo;
		Spectrum n, k;
		//double R0;
};


class OrenNayar : public Material {
	public:
		OrenNayar(const Spectrum& albedo, double sigma) : albedo(albedo), sigma(sigma) {}
		bool Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const;
		double BxDF(const vec3& vi, double wli, const vec3& vo, double wlo) const;
		Spectrum albedo;
		double sigma;
};


class TorranceSparrow : public Material {
	public:
		TorranceSparrow(const Spectrum& albedo, double alpha) : albedo(albedo), alpha(alpha) {
			specular_flag = true;
		}
		bool Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const;
		double BxDF(const vec3& vi, double wli, const vec3& vo, double wlo) const;
		double lambda(const vec3& v) const;
		Spectrum albedo;
		double alpha;
};

class Transparent : public Material {
	public:
		Transparent(void)
		{
			specular_flag = true;
		}
		bool Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const;
};


class DiffuseLight : public Material {
	public:
		DiffuseLight(Spectrum color) : light_color(color) {}
		double Emitted(const ray& r, const HitRecord& rec) const;
		Spectrum light_color;
};



class MixMaterial : public Material {
	public:
		MixMaterial(const Material *mat1, const Material *mat2, double fac)
		{
			this->mat1 = mat1;
			this->mat2 = mat2;
			this->fac = fac;
		}
		double Emitted(const ray& r, const HitRecord& rec) const;
		bool Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const;
		double BxDF(const vec3& vi, double wli, const vec3& vo, double wlo) const;
		const Material *mat1, *mat2;
		double fac;
};

/*
class straight_light : public Material {
	public:
		straight_light(Spectrum color) : light_color(color) {}
		virtual bool Sample(const ray& r_in, const HitRecord& rec, ray& scattered, double&BxDF, double& pdfval) const;
		virtual double Emitted(double u, double v, const ray& r_in, const HitRecord& rec) const;
		Spectrum light_color;
};
*/

#endif
