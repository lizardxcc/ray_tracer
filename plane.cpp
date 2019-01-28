#include "plane.h"
#include "ray.h"


bool plane::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
	float t = dot(somewhere - r.origin(), normal) / dot(r.direction(), normal);
	if (t >= t_min && t <= t_max) {
		rec.t = t;
		rec.p = r.point_at_parameter(rec.t);
		rec.normal = normal;
		rec.mat_ptr = mat_ptr;

		return true;
	}
	return false;
}
