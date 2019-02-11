#ifndef PDF_H
#define PDF_H

#include "vec3.h"
#include "onb.h"

class pdf {
	public:
		//virtual float value(const vec3& direction) const = 0;
		//virtual vec3 generate() const = 0;
		virtual float generate(vec3& direction) const = 0;
};

class uniform_pdf : public pdf {
	public:
		uniform_pdf(const vec3& w)
		{
			uvw.build_from_w(w);
		}
		//virtual float value(const vec3& direction) const;
		//virtual vec3 generate() const;
		virtual float generate(vec3& direction) const;

		onb uvw;
};

#endif
