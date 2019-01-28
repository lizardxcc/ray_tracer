#ifndef PLANE_H
#define PLANE_H

#include "hitable.h"
#include "material.h"

class plane: public hitable {
	public:
		plane() { }
		plane(vec3 somewhere, vec3 normal, material *mat_ptr) : somewhere(somewhere), normal(normal), mat_ptr(mat_ptr) {};
		virtual bool hit(const ray& r, float tmin, float tmux, hit_record& rec) const;
		vec3 somewhere;
		vec3 normal;
		material *mat_ptr;
};

#endif
