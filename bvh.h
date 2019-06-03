#ifndef BVH_H
#define BVH_H

#include <vector>
#include "vec3.h"
#include "ray.h"
#include "aabb.h"
#include "Hittable.h"

//class Hittable;




class bvh_node : public Hittable {
	public:
		bvh_node() {}
		bvh_node(std::vector<std::shared_ptr<Hittable> >& l);
		virtual bool hit(const ray& r, double t_min, double t_max, HitRecord& rec) const;
		virtual bool bounding_box(aabb& box) const;
		virtual void set_Material(std::shared_ptr<Material> mat);
		std::shared_ptr<Hittable> left;
		std::shared_ptr<Hittable> right;
		aabb box;
};


#endif
