#ifndef HITABLE_H
#define HITABLE_H

#include <memory>
#include "ray.h"
#include "aabb.h"
//#include "pdf.h"

class Material;
class pdf;
class Hittable;

struct HitRecord {
	double t;
	vec3 p;
	vec3 normal;
	std::shared_ptr<Material> mat_ptr;
	unsigned int hit_object_id;
};


class Hittable: public std::enable_shared_from_this<Hittable> {
	public:
		virtual bool hit(const ray& r, double t_min, double t_max, HitRecord& rec) const = 0;
		virtual std::unique_ptr<pdf> generate_pdf_object(const vec3& o);
		virtual bool bounding_box(aabb& box) const = 0;
		virtual void set_Material(std::shared_ptr<Material> mat);
		std::shared_ptr<Material> mat_ptr;
		unsigned int object_id;
};

#endif
