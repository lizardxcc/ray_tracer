#ifndef HITABLE_H
#define HITABLE_H

#include <memory>
#include "ray.h"
#include "aabb.h"
//#include "pdf.h"

class material;
class pdf;

struct hit_record {
	double t;
	vec3 p;
	vec3 normal;
	std::shared_ptr<material> mat_ptr;
};


class hitable {
	public:
		virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;
		virtual std::unique_ptr<pdf> generate_pdf_object(const vec3& o);
		virtual bool bounding_box(aabb& box) const = 0;
		virtual void set_material(std::shared_ptr<material> mat);
		std::shared_ptr<material> mat_ptr;
};

#endif
