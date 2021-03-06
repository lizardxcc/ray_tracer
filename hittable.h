#ifndef HITABLE_H
#define HITABLE_H

#include <memory>
#include "ray.h"
#include "aabb.h"
#include "onb.h"
//#include "pdf.h"
#include "materialnode.h"

class Material;
class NodeMaterial;

class Pdf;
class Hittable;

struct HitRecord {
	double t;
	dvec3 p;
	dvec3 normal;
	//std::shared_ptr<Material> mat_ptr;
	NodeMaterial *mat_ptr;
	const Hittable *hit_object;
	unsigned int hit_object_id;
	dvec3 vt;
	ONB tbn;
};


class Hittable: public std::enable_shared_from_this<Hittable> {
	public:
		virtual ~Hittable(void) {}
		virtual bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const = 0;
		virtual bool Occluded(const ray& r, double t_min, double t_max) const
		{
			HitRecord rec;
			if (Hit(r, t_min, t_max, rec))
				return true;
			return false;
		}
		virtual std::unique_ptr<Pdf> GeneratePdfObject(const dvec3& o);
		virtual bool BoundingBox(AABB& box) const = 0;
		virtual void SetMaterial(NodeMaterial *mat);
		//std::shared_ptr<Material> mat_ptr;
		NodeMaterial *mat_ptr;
		unsigned int object_id;
};

#endif
