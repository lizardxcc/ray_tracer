#include "onb.h"

dvec3 ONB::u() const
{
	return axis[0];
}
dvec3 ONB::v() const
{
	return axis[1];
}
dvec3 ONB::w() const
{
	return axis[2];
}

dvec3 ONB::LocalToWorld(double a, double b, double c) const
{
	return a*u() + b*v() + c*w();
}

dvec3 ONB::LocalToWorld(const dvec3& a) const
{
	return a.x()*u() + a.y()*v() + a.z()*w();
}

dvec3 ONB::WorldToLocal(const dvec3& a) const
{
	return dvec3(dot(u(), a), dot(v(), a), dot(w(), a));
}

void ONB::BuildFromW(const dvec3& n)
{
	axis[2] = unit_vector(n);
	dvec3 a;
	// prevent a being pararell to n
	if (fabs(w().x()) > 0.9)
		a = dvec3(0, 1, 0);
	else
		a = dvec3(1, 0, 0);
	axis[1] = unit_vector(cross(w(), a));
	axis[0] = cross(w(), v());
}
