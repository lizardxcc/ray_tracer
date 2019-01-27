#include "sphere.h"
#include "ray.h"

bool sphere::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
	float a = pow(r.B.length(), 2.0);
	float b_prime = dot(r.A - center, r.B);
	float c = pow((r.A - center).length(), 2.0) - pow(radius, 2.0);
	float discriminant_prime = pow(b_prime, 2.0) - a * c;
	if (discriminant_prime < 0.0) {
		return false;
	}
	float t1 = (-b_prime - sqrt(discriminant_prime)) / a;

	if (t1 >= t_min && t1 <= t_max) {
		rec.t = t1;
		rec.p = r.point_at_parameter(rec.t);
		rec.normal = unit_vector(rec.p - center);
		rec.mat_ptr = mat_ptr;
		return true;
	}
	float t2 = (-b_prime + sqrt(discriminant_prime)) / a;
	if (t2 >= t_min && t2 <= t_max) {
		rec.t = t2;
		rec.p = r.point_at_parameter(rec.t);
		rec.normal = unit_vector(rec.p - center);
		rec.mat_ptr = mat_ptr;
		return true;
	}

	return false;
}
