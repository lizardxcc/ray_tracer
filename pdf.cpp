#include "pdf.h"


vec3 FromSphericalToXYZ(double theta, double phi)
{
	return vec3(
	sin(theta)*cos(phi),
	sin(theta)*sin(phi),
	cos(theta)
	);
}

vec3 RandomOnUnitHemiSphere(void)
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


vec3 RandomOnUnitHemiSphere(double theta_max)
{
	double r1 = drand48();
	double r2 = drand48();
	double phi = 2*M_PI*r1;
	double theta = acos(1-(1-cos(theta_max))*r2);
	return FromSphericalToXYZ(theta, phi);
}


vec3 CosineWeightedRandomOnUnitHemiSphere(void)
{
	double r1 = drand48();
	double r2 = drand48();
	double phi = 2 * M_PI * r1;
	double sin_theta = sqrt(r2);
	double x = sin_theta * cos(phi);
	double y = sin_theta * sin(phi);
	double z = 1-r2;
	return vec3(x, y, z);
}


vec3 UniformPdf::Generate() const
{
	return uvw.localtoworld(RandomOnUnitHemiSphere());
}


double UniformPdf::PdfVal(const vec3& direction) const
{
	if (dot(direction, uvw.w()) >= 0.0) {
		return 1.0/(2.0*M_PI);
	} else {
		return 0.0;
	}
}


vec3 CosinePdf::Generate() const
{
	return uvw.localtoworld(CosineWeightedRandomOnUnitHemiSphere());
}


double CosinePdf::PdfVal(const vec3& direction) const
{
	double cos_theta = dot(direction, uvw.w());
	if (cos_theta >= 0.0) {
		return cos_theta/M_PI;
	} else {
		return 0.0;
	}
}



vec3 toward_object_Pdf::Generate() const
{
	return uvw.localtoworld(RandomOnUnitHemiSphere(theta_max));
}

double toward_object_Pdf::PdfVal(const vec3& direction) const
{
	double cosine = dot(unit_vector(direction), uvw.w());
	if (cosine >= cos(theta_max)) {
		return 1.0/(2*M_PI*(1-cos(theta_max)));
	} else {
		return 0.0;
	}
}


vec3 HittablePdf::Generate() const
{
	return pdf_ptr->Generate();
}

double HittablePdf::PdfVal(const vec3& direction) const
{
	return pdf_ptr->PdfVal(direction);
}



vec3 MixturePdf::Generate() const
{
	double r = drand48();
	size_t l = pdf_list.size();
	for (size_t i = 0; i < l; i++) {
		if (i == (l-1) || r < ((double)(i+1)/(double)l)) {
			return pdf_list[i]->Generate();
		}
	}
	return pdf_list[l-1]->Generate();
}

double MixturePdf::PdfVal(const vec3& direction) const
{
	double sum = 0.0;
	for (size_t i = 0; i < pdf_list.size(); i++) {
		sum += pdf_list[i]->PdfVal(direction);
	}
	return sum/(double)pdf_list.size();
}
