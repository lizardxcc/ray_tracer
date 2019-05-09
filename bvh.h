#ifndef BVH_H
#define BVH_H

#include <vector>
#include "vec3.h"
#include "ray.h"
#include "aabb.h"
#include "hitable.h"

//class hitable;




class bvh_node : public hitable {
	public:
		bvh_node() {}
		bvh_node(std::vector<hitable *>& l);
		virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const;
		virtual bool bounding_box(aabb& box) const;
		virtual void set_material(std::shared_ptr<material> mat);
		hitable *left;
		hitable *right;
		aabb box;
};


#endif
