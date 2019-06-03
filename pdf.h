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
		virtual vec3 Generate() const = 0;
		virtual double PdfVal(const vec3& direction) const = 0;
};

class UniformPdf : public Pdf {
	public:
		UniformPdf(const vec3& w)
		{
			uvw.BuildFromW(w);
		}
		virtual vec3 Generate() const;
		virtual double PdfVal(const vec3& direction) const;

		ONB uvw;
};

class CosinePdf : public Pdf {
	public:
		CosinePdf(const vec3& w)
		{
			uvw.BuildFromW(w);
		}
		virtual vec3 Generate() const;
		virtual double PdfVal(const vec3& direction) const;

		ONB uvw;
};

class toward_object_Pdf : public Pdf {
	public:
		toward_object_Pdf(const vec3& w, double _theta_max)
		{
			uvw.BuildFromW(w);
			this->theta_max = _theta_max;
		}
		virtual vec3 Generate() const;
		virtual double PdfVal(const vec3& direction) const;

		ONB uvw;
		double theta_max;
};


class HittablePdf : public Pdf {
	public:
		HittablePdf(std::shared_ptr<Hittable> p, const vec3& origin)
		{
			ptr = p;
			o = origin;
			pdf_ptr = p->GeneratePdfObject(origin);
		}
		virtual vec3 Generate() const;
		virtual double PdfVal(const vec3& direction) const;

		vec3 o;
		std::shared_ptr<Hittable> ptr;

		std::unique_ptr<Pdf> pdf_ptr;
};


class MixturePdf : public Pdf {
	public:
		MixturePdf(std::vector<std::unique_ptr<Pdf> > pdf_list) : pdf_list(std::move(pdf_list))
		{
		}

		virtual vec3 Generate() const;
		virtual double PdfVal(const vec3& direction) const;

		std::vector<std::unique_ptr<Pdf> > pdf_list;
};

#endif
