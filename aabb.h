#ifndef AABB_H
#define AABB_H

#include "vec3.h"
#include "ray.h"

class AABB {
	public:
		AABB()
		{
		}

		AABB(const dvec3& a, const dvec3& b)
		{
			minp = a;
			maxp = b;
			center = (a+b)/2.0;
		}

		bool Hit(const ray& r, double t_min, double t_max) const;
		double SurfaceArea(void) const;

		dvec3 minp;
		dvec3 maxp;
		dvec3 center;
};


AABB SurroundingBox(const AABB& box0, const AABB& box1);
#endif
