#ifndef SPHERE_H
#define SPHERE_H

#include "hitable.h"
#include "material.h"

class sphere: public hitable {
	public:
		sphere() { }
		sphere(vec3 center, float r, material *mat_ptr) : center(center), radius(r), mat_ptr(mat_ptr) {};
		virtual bool hit(const ray& r, float tmin, float tmux, hit_record& rec) const;
		//virtual float generate_pdf_dir(const vec3& o, vec3& direction);
		vec3 center;
		float radius;
		material *mat_ptr;
};

#endif
