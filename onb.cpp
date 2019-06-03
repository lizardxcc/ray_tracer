#include "onb.h"

vec3 ONB::u() const
{
	return axis[0];
}
vec3 ONB::v() const
{
	return axis[1];
}
vec3 ONB::w() const
{
	return axis[2];
}

vec3 ONB::LocalToWorld(double a, double b, double c) const
{
	return a*u() + b*v() + c*w();
}

vec3 ONB::LocalToWorld(const vec3& a) const
{
	return a.x()*u() + a.y()*v() + a.z()*w();
}

vec3 ONB::WorldToLocal(const vec3& a) const
{
	return vec3(dot(u(), a), dot(v(), a), dot(w(), a));
}

void ONB::BuildFromW(const vec3& n)
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
