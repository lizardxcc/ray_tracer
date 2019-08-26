#ifndef BVH_H
#define BVH_H

#include <vector>
#include "vec3.h"
#include "ray.h"
#include "aabb.h"
#include "hittable.h"

//class Hittable;




class BVHNode : public Hittable {
	public:
		BVHNode() {}
		BVHNode(std::vector<std::shared_ptr<Hittable> >& l);
		bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const override;
		bool Occluded(const ray& r, double t_min, double t_max) const override;
		bool BoundingBox(AABB& box) const override;
		//void SetMaterial(std::shared_ptr<Material> mat);
		void SetMaterial(NodeMaterial *mat) override;
		std::shared_ptr<Hittable> left;
		std::shared_ptr<Hittable> right;
		AABB box;
};


#endif
