#ifndef PDF_H
#define PDF_H

#include <vector>
#include <algorithm>
#include "vec3.h"
#include "object.h"
#include "onb.h"

class hitable;

class pdf {
	public:
		virtual ~pdf(){}
		virtual vec3 generate() const = 0;
		virtual float pdf_val(const vec3& direction) const = 0;
};

class uniform_pdf : public pdf {
	public:
		uniform_pdf(const vec3& w)
		{
			uvw.build_from_w(w);
		}
		virtual vec3 generate() const;
		virtual float pdf_val(const vec3& direction) const;

		onb uvw;
};

class toward_object_pdf : public pdf {
	public:
		toward_object_pdf(const vec3& w, float _theta_max)
		{
			uvw.build_from_w(w);
			this->theta_max = _theta_max;
		}
		virtual vec3 generate() const;
		virtual float pdf_val(const vec3& direction) const;

		onb uvw;
		float theta_max;
};


class hitable_pdf : public pdf {
	public:
		hitable_pdf(hitable *p, const vec3& origin)
		{
			ptr = p;
			o = origin;
			pdf_ptr = p->generate_pdf_object(origin);
		}
		~hitable_pdf()
		{
			delete pdf_ptr;
		}
		virtual vec3 generate() const;
		virtual float pdf_val(const vec3& direction) const;

		vec3 o;
		hitable *ptr;

		pdf *pdf_ptr;
};


class mixture_pdf : public pdf {
	public:
		mixture_pdf(const std::vector<pdf *>& pdf_list)
		{
			this->pdf_list.resize(pdf_list.size());
			std::copy(pdf_list.begin(), pdf_list.end(), this->pdf_list.begin());
		}

		virtual vec3 generate() const;
		virtual float pdf_val(const vec3& direction) const;

		std::vector<pdf *> pdf_list;
};

#endif
