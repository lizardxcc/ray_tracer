#ifndef OBJECT_H
#define OBJECT_H

#include "hitable.h"
#include "material.h"

class rectangle : public hitable {
	public:
		rectangle() { }
		rectangle(vec3 center, vec3 normal, vec3 width_dir, float width, float height, material *mat_ptr) : center(center), normal(normal), width_dir(width_dir), width(width), height(height), mat_ptr(mat_ptr) {};
		virtual bool hit(const ray& r, float tmin, float tmux, hit_record& rec) const;
		vec3 center;
		vec3 normal;
		vec3 width_dir;
		float width;
		float height;
		material *mat_ptr;
};

#endif
