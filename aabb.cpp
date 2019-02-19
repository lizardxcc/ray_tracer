#include <algorithm>
#include "aabb.h"




inline float ffmin(float a, float b)
{
	return a < b ? a : b;
}

inline float ffmax(float a, float b)
{
	return a > b ? a : b;
}

bool aabb::hit(const ray& r, float t_min, float t_max) const
{
	for (int i = 0; i < 3; i++) {
		float t0 = ffmin(
		(minp[i] - r.origin()[i]) / r.direction()[i],
		(maxp[i] - r.origin()[i]) / r.direction()[i]
		);
		float t1 = ffmax(
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



aabb surrounding_box(const aabb& box0, const aabb& box1)
{
	vec3 min_p(std::min(box0.minp.x(), box1.minp.x()),
	std::min(box0.minp.y(), box1.minp.y()),
	std::min(box0.minp.z(), box1.minp.z()));
	vec3 max_p(std::max(box0.maxp.x(), box1.maxp.x()),
	std::max(box0.maxp.y(), box1.maxp.y()),
	std::max(box0.maxp.z(), box1.maxp.z()));
	return aabb(min_p, max_p);
}


