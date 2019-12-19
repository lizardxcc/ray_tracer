#ifndef RAY_H
#define RAY_H

#include "vec3.h"

class ray {
public:
	ray()
	{
	}

	ray(const dvec3& a, const dvec3& b)
	{
		A = a;
		B = b;
	}

	dvec3 origin() const { return A; }
	dvec3 direction() const { return B; }
	dvec3 point_at_parameter(double t) const {
		return A + t * B;
	}

	dvec3 A;
	dvec3 B;
	double central_wl;
	double min_wl;
	double max_wl;
};


#endif
