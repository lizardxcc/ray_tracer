#include "hittablelist.h"

bool Hittable_list::Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const
{
	HitRecord temp_rec;
	bool hit_anything = false;
	double closest_so_far = t_max;
	for (size_t i = 0; i < list.size(); i++) {
		if (list[i]->Hit(r, t_min, closest_so_far, temp_rec)) {
			hit_anything = true;
			closest_so_far = temp_rec.t;
			rec = temp_rec;
		}
	}

	return hit_anything;
}



bool Hittable_list::BoundingBox(AABB& box) const
{
	AABB temp_box;
	if (list.size() == 0) {
		return false;
	}

	if (list[0]->BoundingBox(temp_box) == false) {
		return false;
	}

	box = temp_box;

	for (size_t i = 0; i < list.size(); i++) {
		if (list[i]->BoundingBox(temp_box)) {
			box = SurroundingBox(box, temp_box);
		} else {
			return false;
		}
	}

	return true;
}
