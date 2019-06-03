#include "Hittablelist.h"

bool Hittable_list::hit(const ray& r, double t_min, double t_max, HitRecord& rec) const
{
	HitRecord temp_rec;
	bool hit_anything = false;
	double closest_so_far = t_max;
	for (int i = 0; i < list.size(); i++) {
		if (list[i]->hit(r, t_min, closest_so_far, temp_rec)) {
			hit_anything = true;
			closest_so_far = temp_rec.t;
			rec = temp_rec;
		}
	}

	return hit_anything;
}



bool Hittable_list::bounding_box(aabb& box) const
{
	aabb temp_box;
	if (list.size() == 0) {
		return false;
	}

	if (list[0]->bounding_box(temp_box) == false) {
		return false;
	}

	box = temp_box;

	for (size_t i = 0; i < list.size(); i++) {
		if (list[i]->bounding_box(temp_box)) {
			box = surrounding_box(box, temp_box);
		} else {
			return false;
		}
	}

	return true;
}
