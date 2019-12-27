#ifndef PDF_H
#define PDF_H

#include <memory>
#include <vector>
#include <algorithm>
#include "vec3.h"
#include "object.h"
#include "onb.h"

class Hittable;

class Pdf {
	public:
		virtual ~Pdf(void);
		virtual dvec3 Generate() const = 0;
		virtual double PdfVal(const dvec3& direction) const = 0;
};

class UniformPdf : public Pdf {
	public:
		UniformPdf(const dvec3& w)
		{
			uvw.BuildFromW(w);
		}
		dvec3 Generate() const override;
		double PdfVal(const dvec3& direction) const override;

		ONB uvw;
};

class CosinePdf : public Pdf {
	public:
		CosinePdf(const dvec3& w)
		{
			uvw.BuildFromW(w);
		}
		dvec3 Generate() const override;
		double PdfVal(const dvec3& direction) const override;

		ONB uvw;
};

class toward_object_Pdf : public Pdf {
	public:
		toward_object_Pdf(const dvec3& w, double _theta_max)
		{
			uvw.BuildFromW(w);
			this->theta_max = _theta_max;
		}
		dvec3 Generate() const override;
		double PdfVal(const dvec3& direction) const override;

		ONB uvw;
		double theta_max;
};


class HittablePdf : public Pdf {
	public:
		HittablePdf(std::shared_ptr<Hittable> p, const dvec3& origin)
		{
			ptr = p;
			o = origin;
			pdf_ptr = p->GeneratePdfObject(origin);
		}
		dvec3 Generate() const override;
		double PdfVal(const dvec3& direction) const override;

		dvec3 o;
		std::shared_ptr<Hittable> ptr;

		std::unique_ptr<Pdf> pdf_ptr;
};


class MixturePdf : public Pdf {
	public:
		MixturePdf(std::vector<std::unique_ptr<Pdf> > pdf_list) : pdf_list(std::move(pdf_list))
		{
		}

		dvec3 Generate() const override;
		double PdfVal(const dvec3& direction) const override;

		std::vector<std::unique_ptr<Pdf> > pdf_list;
};

double PowerHeuristic(double pdf1, double pdf2, double beta);

#endif
