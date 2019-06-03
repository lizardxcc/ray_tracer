#ifndef HITABLELIST_H
#define HITABLELIST_H

#include <vector>
#include "Hittable.h"
#include "bvh.h"

class Hittable_list: public Hittable {
	public:
	Hittable_list() { }
	Hittable_list(std::vector<std::shared_ptr<Hittable> > l)
	{
		list = l;
	}
	bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const;
	bool BoundingBox(AABB& box) const;

	std::vector<std::shared_ptr<Hittable> > list;
	int list_size;
};





#endif
