#include <algorithm>
#include "aabb.h"




inline double ffmin(double a, double b)
{
	return a < b ? a : b;
}

inline double ffmax(double a, double b)
{
	return a > b ? a : b;
}

bool AABB::Hit(const ray& r, double t_min, double t_max) const
{
	for (int i = 0; i < 3; i++) {
		double t0 = ffmin(
		(minp[i] - r.origin()[i]) / r.direction()[i],
		(maxp[i] - r.origin()[i]) / r.direction()[i]
		);
		double t1 = ffmax(
		(minp[i] - r.origin()[i]) / r.direction()[i],
		(maxp[i] - r.origin()[i]) / r.direction()[i]
		);
		t_min = ffmax(t0, t_min);
		t_max = ffmin(t1, t_max);
		if (t_max <= t_min)
			return false;
	}
	return true;
}


double AABB::SurfaceArea(void) const
{
	vec3 v = maxp-minp;
	return 2.0*(v[0]*v[1]+v[1]*v[2]+v[2]*v[0]);
}



AABB SurroundingBox(const AABB& box0, const AABB& box1)
{
	vec3 min_p(std::min(box0.minp.x(), box1.minp.x()),
	std::min(box0.minp.y(), box1.minp.y()),
	std::min(box0.minp.z(), box1.minp.z()));
	vec3 max_p(std::max(box0.maxp.x(), box1.maxp.x()),
	std::max(box0.maxp.y(), box1.maxp.y()),
	std::max(box0.maxp.z(), box1.maxp.z()));
	return AABB(min_p, max_p);
}


