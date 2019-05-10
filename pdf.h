#ifndef PDF_H
#define PDF_H

#include <memory>
#include <vector>
#include <algorithm>
#include "vec3.h"
#include "object.h"
#include "onb.h"

class hitable;

class pdf {
	public:
		virtual vec3 generate() const = 0;
		virtual double pdf_val(const vec3& direction) const = 0;
};

class uniform_pdf : public pdf {
	public:
		uniform_pdf(const vec3& w)
		{
			uvw.build_from_w(w);
		}
		virtual vec3 generate() const;
		virtual double pdf_val(const vec3& direction) const;

		onb uvw;
};

class toward_object_pdf : public pdf {
	public:
		toward_object_pdf(const vec3& w, double _theta_max)
		{
			uvw.build_from_w(w);
			this->theta_max = _theta_max;
		}
		virtual vec3 generate() const;
		virtual double pdf_val(const vec3& direction) const;

		onb uvw;
		double theta_max;
};


class hitable_pdf : public pdf {
	public:
		hitable_pdf(std::shared_ptr<hitable> p, const vec3& origin)
		{
			ptr = p;
			o = origin;
			pdf_ptr = p->generate_pdf_object(origin);
		}
		virtual vec3 generate() const;
		virtual double pdf_val(const vec3& direction) const;

		vec3 o;
		std::shared_ptr<hitable> ptr;

		std::unique_ptr<pdf> pdf_ptr;
};


class mixture_pdf : public pdf {
	public:
		mixture_pdf(std::vector<std::unique_ptr<pdf> > pdf_list)
		{
			this->pdf_list = std::move(pdf_list);
		}

		virtual vec3 generate() const;
		virtual double pdf_val(const vec3& direction) const;

		std::vector<std::unique_ptr<pdf> > pdf_list;
};

#endif
