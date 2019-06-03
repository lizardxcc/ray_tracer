#ifndef BVH_H
#define BVH_H

#include <vector>
#include "vec3.h"
#include "ray.h"
#include "aabb.h"
#include "Hittable.h"

//class Hittable;




class BVHNode : public Hittable {
	public:
		BVHNode() {}
		BVHNode(std::vector<std::shared_ptr<Hittable> >& l);
		virtual bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const;
		virtual bool BoundingBox(AABB& box) const;
		virtual void SetMaterial(std::shared_ptr<Material> mat);
		std::shared_ptr<Hittable> left;
		std::shared_ptr<Hittable> right;
		AABB box;
};


#endif
