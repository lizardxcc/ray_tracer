#include "pdf.h"


vec3 from_spherical_to_xyz(float theta, float phi)
{
	return vec3(
	sin(theta)*cos(phi),
	sin(theta)*sin(phi),
	cos(theta)
	);
}

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


vec3 random_on_unit_hemisphere(float theta_max)
{
	float r1 = drand48();
	float r2 = drand48();
	float phi = 2*M_PI*r1;
	float theta = acos(1-(1-cos(theta_max))*r2);
	return from_spherical_to_xyz(theta, phi);
}


vec3 uniform_pdf::generate() const
{
	return uvw.local(random_on_unit_hemisphere());
}


float uniform_pdf::pdf_val(const vec3& direction) const
{
	if (dot(direction, uvw.w()) >= 0.0) {
		return 1.0/(2.0*M_PI);
	} else {
		return 0.0;
	}
}




vec3 toward_object_pdf::generate() const
{
	return uvw.local(random_on_unit_hemisphere(theta_max));
}

float toward_object_pdf::pdf_val(const vec3& direction) const
{
	float cosine = dot(unit_vector(direction), uvw.w());
	if (cosine >= cos(theta_max)) {
		return 1.0/(2*M_PI*(1-cos(theta_max)));
	} else {
		return 0.0;
	}
}


vec3 hitable_pdf::generate() const
{
	return pdf_ptr->generate();
}

float hitable_pdf::pdf_val(const vec3& direction) const
{
	return pdf_ptr->pdf_val(direction);
}



vec3 mixture_pdf::generate() const
{
	float r = drand48();
	for (size_t i = 0; i < pdf_list.size(); i++) {
		if (r < ((float)(i+1)*1.0/(float)pdf_list.size())) {
			return pdf_list[i]->generate();
		}
	}
}

float mixture_pdf::pdf_val(const vec3& direction) const
{
	float sum = 0.0;
	for (size_t i = 0; i < pdf_list.size(); i++) {
		sum += pdf_list[i]->pdf_val(direction);
	}
	return sum/(float)pdf_list.size();
}
