#include "pdf.h"


vec3 from_spherical_to_xyz(double theta, double phi)
{
	return vec3(
	sin(theta)*cos(phi),
	sin(theta)*sin(phi),
	cos(theta)
	);
}

vec3 random_on_unit_hemisphere(void)
{
	double r1 = drand48();
	double r2 = drand48();
	double phi = 2 * M_PI * r1;
	double sin_theta = sqrt(r2*(2-r2));
	double x = sin_theta * cos(phi);
	double y = sin_theta * sin(phi);
	double z = 1-r2;
	return vec3(x, y, z);
}


vec3 random_on_unit_hemisphere(double theta_max)
{
	double r1 = drand48();
	double r2 = drand48();
	double phi = 2*M_PI*r1;
	double theta = acos(1-(1-cos(theta_max))*r2);
	return from_spherical_to_xyz(theta, phi);
}


vec3 uniform_pdf::generate() const
{
	return uvw.localtoworld(random_on_unit_hemisphere());
}


double uniform_pdf::pdf_val(const vec3& direction) const
{
	if (dot(direction, uvw.w()) >= 0.0) {
		return 1.0/(2.0*M_PI);
	} else {
		return 0.0;
	}
}




vec3 toward_object_pdf::generate() const
{
	return uvw.localtoworld(random_on_unit_hemisphere(theta_max));
}

double toward_object_pdf::pdf_val(const vec3& direction) const
{
	double cosine = dot(unit_vector(direction), uvw.w());
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

double hitable_pdf::pdf_val(const vec3& direction) const
{
	return pdf_ptr->pdf_val(direction);
}



vec3 mixture_pdf::generate() const
{
	double r = drand48();
	size_t l = pdf_list.size();
	for (size_t i = 0; i < l; i++) {
		if (i == (l-1) || r < ((double)(i+1)/(double)l)) {
			return pdf_list[i]->generate();
		}
	}
	return pdf_list[l-1]->generate();
}

double mixture_pdf::pdf_val(const vec3& direction) const
{
	double sum = 0.0;
	for (size_t i = 0; i < pdf_list.size(); i++) {
		sum += pdf_list[i]->pdf_val(direction);
	}
	return sum/(double)pdf_list.size();
}
