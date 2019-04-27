#ifndef AABB_H
#define AABB_H

#include "vec3.h"
#include "ray.h"

class aabb {
	public:
		aabb()
		{
		}

		aabb(const vec3& a, const vec3& b)
		{
			minp = a;
			maxp = b;
			center = (a+b)/2.0;
		}

		bool hit(const ray& r, double t_min, double t_max) const;

		vec3 minp;
		vec3 maxp;
		vec3 center;
};


aabb surrounding_box(const aabb& box0, const aabb& box1);
#endif
