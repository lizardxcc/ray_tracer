#ifndef SPHERE_H
#define SPHERE_H

#include "hitable.h"

class sphere: public hitable {
	public:
		sphere() { }
		sphere(vec3 center, float r) : center(center), radius(r) {};
		virtual bool hit(const ray& r, float tmin, float tmux, hit_record& rec) const;
		vec3 center;
		float radius;
};

#endif
