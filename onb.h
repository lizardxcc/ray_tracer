#ifndef ONB_H
#define ONB_H

#include "vec3.h"

class onb
{
	public:
		onb() {}
		inline vec3 operator[](int i) const
		{
			return axis[i];
		}


		vec3 u() const;
		vec3 v() const;
		vec3 w() const;

		vec3 local(double a, double b, double c) const;
		vec3 local(const vec3& a) const;
		void build_from_w(const vec3&);

		vec3 axis[3];

};


#endif
