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
		bvh_node(std::vector<std::shared_ptr<hitable> >& l);
		virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const;
		virtual bool bounding_box(aabb& box) const;
		virtual void set_Material(std::shared_ptr<Material> mat);
		std::shared_ptr<hitable> left;
		std::shared_ptr<hitable> right;
		aabb box;
};


#endif
