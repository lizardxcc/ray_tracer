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
		virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
		virtual bool bounding_box(aabb& box) const;
		hitable *left;
		hitable *right;
		aabb box;
};


#endif
