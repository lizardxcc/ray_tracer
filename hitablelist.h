#ifndef HITABLELIST_H
#define HITABLELIST_H

#include <vector>
#include "hitable.h"
#include "bvh.h"

class hitable_list: public hitable {
	public:
	hitable_list() { }
	hitable_list(std::vector<hitable*> l)
	{
		list = l;
	}
	virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
	virtual bool bounding_box(aabb& box) const;

	std::vector<hitable*> list;
	int list_size;
};





#endif
