#ifndef MATERIAL_H
#define MATERIAL_H

#include <vector>
#include <complex>
#include "vec3.h"
#include "onb.h"
#include "ray.h"
#include "hittable.h"
#include "spectrum.h"

extern const vec3 default_vt;
double cfresnel(double cos_theta, std::complex<double> n);

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
		bool Sample_p(const vec3& vo, double wlo, vec3& vi, double& wli, double& phase, double& pdfval) const override;
		double Phase(const vec3& vi, double wli, const vec3& vo, double wlo) const override;
};

class HenyeyGreenstein : public MediumMaterial {
	public:
		HenyeyGreenstein(const Spectrum& sigma_t, const Spectrum& albedo, double g) : MediumMaterial(sigma_t, albedo), g(g) {}
		bool Sample_p(const vec3& vo, double wlo, vec3& vi, double& wli, double& phase, double& pdfval) const override;
		double Phase(const vec3& vi, double wli, const vec3& vo, double wlo) const override;
		double g;
};

class Material {
	public:
		virtual ~Material(void) {}
		virtual void PreProcess(HitRecord &rec) const
		{
		}
		virtual bool Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const {
			return false;
		}
		virtual double BxDF(const vec3& vi, double wli, const vec3& vo, double wlo, const vec3& vt = default_vt) const {
			return 0.0;
		}
		virtual double Emitted(const ray& r, const HitRecord& rec, const vec3& vt = default_vt) const {
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
		bool Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const override;
		double BxDF(const vec3& vi, double wli, const vec3& vo, double wlo, const vec3& vt = default_vt) const override;


		Spectrum albedo;

};

class Metal : public Material {
	public:
		Metal(const Spectrum& n, const Spectrum& k) : n(n), k(k) {
			specular_flag = true;
		}
		bool Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const override;
		double BxDF(const vec3& vi, double wli, const vec3& vo, double wlo, const vec3& vt = default_vt) const override;

		Spectrum albedo;
		Spectrum n;
		Spectrum k;
		double fuzz;

};

class Dielectric : public Material {
	public:
		//Dielectric(double ref_B, double ref_C) : ref_B(ref_B), ref_C(ref_C) {
		//	albedo = Spectrum(1.0);
		//	//R0 = pow((1.0-ref_idx)/(1.0+ref_idx), 2);
		//}
		//Dielectric(const Spectrum& albedo, double ref_B, double ref_C) : albedo(albedo), ref_B(ref_B), ref_C(ref_C) {}
		Dielectric(const Spectrum& n, const Spectrum& k) : n(n), k(k) {
			// k is deprecated argument
			specular_flag = true;
		}
		//Dielectric(const Spectrum& n, const Spectrum& k) : n(n), k(k) {
		//	specular_flag = true;
		//}
		bool Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const override;

		double ref_B, ref_C;
		Spectrum albedo;
		Spectrum n, k;
		//double R0;
};


class OrenNayar : public Material {
	public:
		OrenNayar(const Spectrum& albedo, double sigma) : albedo(albedo), sigma(sigma) {}
		bool Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const override;
		double BxDF(const vec3& vi, double wli, const vec3& vo, double wlo, const vec3& vt = default_vt) const override;
		Spectrum albedo;
		double sigma;
};


class Microfacet : public Material {
	public:
		Microfacet(const Spectrum& n, const Spectrum& k, double alpha) : n(n), k(k), alpha(alpha) {
		}
		bool Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const override;
		double BxDF(const vec3& vi, double wli, const vec3& vo, double wlo, const vec3& vt = default_vt) const override;
		double lambda(const vec3& v) const;
		Spectrum albedo;
		Spectrum n;
		Spectrum k;
		double alpha;
		bool enable_refraction = true;
};

class GGX : public Material {
	public:
		GGX(const Spectrum& albedo, double alpha) : albedo(albedo), alpha(alpha) {
			specular_flag = true;
		}
		bool Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const override;
		double BxDF(const vec3& vi, double wli, const vec3& vo, double wlo, const vec3& vt = default_vt) const override;
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
		bool Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const override;
};


class DiffuseLight : public Material {
	public:
		DiffuseLight(Spectrum color) : light_color(color) {}
		double Emitted(const ray& r, const HitRecord& rec, const vec3& vt = default_vt) const override;
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
		double Emitted(const ray& r, const HitRecord& rec, const vec3& vt = default_vt) const override;
		bool Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const override;
		double BxDF(const vec3& vi, double wli, const vec3& vo, double wlo, const vec3& vt = default_vt) const override;
		const Material *mat1, *mat2;
		double fac;
};


class TextureMaterial : public Material {
	public:
		TextureMaterial(void)
		{
		}
		bool Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const override;
		double BxDF(const vec3& vi, double wli, const vec3& vo, double wlo, const vec3& vt = default_vt) const override;
		bool LoadImage(const char *path);
		//double Emitted(const ray& r, const HitRecord& rec) const override;
		uint8_t *texture = nullptr;
		int width, height, bpp;
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
