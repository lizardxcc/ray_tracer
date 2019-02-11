#include "pdf.h"


vec3 random_on_unit_hemisphere(void)
{
	float r1 = drand48();
	float r2 = drand48();
	float phi = 2 * M_PI * r1;
	float sin_theta = sqrt(r2*(2-r2));
	float x = sin_theta * cos(phi);
	float y = sin_theta * sin(phi);
	float z = 1-r2;
	return vec3(x, y, z);
}


//float uniform_pdf::value(const vec3& direction) const
//{
//	return 1.0/(2*M_PI);
//}

float uniform_pdf::generate(vec3& direction) const
{
	direction = uvw.local(random_on_unit_hemisphere());
	return 1.0/(2*M_PI);
}
