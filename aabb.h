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
		}

		bool hit(const ray& r, float t_min, float t_max) const;

		vec3 minp;
		vec3 maxp;
};


aabb surrounding_box(const aabb& box0, const aabb& box1);
#endif
