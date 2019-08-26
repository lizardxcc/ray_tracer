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
vec3 reflect2(vec3 v, vec3 normal);
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
		virtual bool Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF_divided_by_pdf, double& BxDF, double& pdfval) const {
			return false;
		}
		virtual double BxDF(const vec3& vi, double wli, const vec3& vo, double wlo) const {
			return 0.0;
		}
		virtual double PDF(const vec3& vi, double wli, const vec3& vo, double wlo) const {
			return 0.0;
		}
		virtual double Emitted(const ray& r, const HitRecord& rec) const {
			return 0.0;
		}
		//std::shared_ptr<MediumMaterial> mi;
		MediumMaterial *mi = nullptr;
		//static std::vector<std::shared_ptr<Hittable> > lights;
		//bool specular_flag = false;
};


#endif
