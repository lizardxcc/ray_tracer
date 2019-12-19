#ifndef ONB_H
#define ONB_H

#include "vec3.h"

class ONB
{
	public:
		ONB() {}
		inline dvec3 operator[](int i) const
		{
			return axis[i];
		}


		dvec3 u() const;
		dvec3 v() const;
		dvec3 w() const;

		dvec3 LocalToWorld(double a, double b, double c) const;
		dvec3 LocalToWorld(const dvec3& a) const;
		dvec3 WorldToLocal(const dvec3& a) const;
		void BuildFromW(const dvec3&);

		dvec3 axis[3];

};


#endif
