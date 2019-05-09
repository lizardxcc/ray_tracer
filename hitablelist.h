#ifndef HITABLELIST_H
#define HITABLELIST_H

#include <vector>
#include "hitable.h"
#include "bvh.h"

class hitable_list: public hitable {
	public:
	hitable_list() { }
	hitable_list(std::vector<std::shared_ptr<hitable> > l)
	{
		list = l;
	}
	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const;
	virtual bool bounding_box(aabb& box) const;

	std::vector<std::shared_ptr<hitable> > list;
	int list_size;
};





#endif
