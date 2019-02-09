#include "onb.h"

vec3 onb::u() const
{
	return axis[0];
}
vec3 onb::v() const
{
	return axis[1];
}
vec3 onb::w() const
{
	return axis[2];
}

vec3 onb::local(float a, float b, float c) const
{
	return a*u() + b*v() + c*w();
}

vec3 onb::local(const vec3& a) const
{
	return a.x()*u() + a.y()*v() + a.z()*w();
}

void onb::build_from_w(const vec3& n)
{
	axis[2] = unit_vector(n);
	vec3 a;
	// prevent a being pararell to n
	if (fabs(w().x()) > 0.9)
		a = vec3(0, 1, 0);
	else
		a = vec3(1, 0, 0);
	axis[1] = unit_vector(cross(w(), a));
	axis[0] = cross(w(), v());
}
