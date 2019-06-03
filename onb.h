#ifndef ONB_H
#define ONB_H

#include "vec3.h"

class ONB
{
	public:
		ONB() {}
		inline vec3 operator[](int i) const
		{
			return axis[i];
		}


		vec3 u() const;
		vec3 v() const;
		vec3 w() const;

		vec3 LocalToWorld(double a, double b, double c) const;
		vec3 LocalToWorld(const vec3& a) const;
		vec3 WorldToLocal(const vec3& a) const;
		void BuildFromW(const vec3&);

		vec3 axis[3];

};


#endif
