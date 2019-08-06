#ifndef HITABLELIST_H
#define HITABLELIST_H

#include <vector>
#include "hittable.h"
#include "bvh.h"

class HittableList: public Hittable {
	public:
	HittableList() { }
	HittableList(std::vector<std::shared_ptr<Hittable> > l)
	{
		list = l;
	}
	bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const;
	bool BoundingBox(AABB& box) const;

	std::vector<std::shared_ptr<Hittable> > list;
	int list_size;
};





#endif
