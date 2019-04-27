#include <algorithm>
#include "bvh.h"

//bool box_x_compare(const hitable& a, const hitable& b);
//bool box_y_compare(const hitable& a, const hitable& b);
//bool box_z_compare(const hitable& a, const hitable& b);

bool box_x_compare(const hitable * a, const hitable* b)
{
	aabb box_left, box_right;
	if (!a->bounding_box(box_left) || !b->bounding_box(box_right)) {
		std::cerr << "no bounding box in box_x_compare()" << std::endl;
	}

	return (box_left.center.x() < box_right.center.x());
}
bool box_y_compare(const hitable* a, const hitable* b)
{
	aabb box_left, box_right;
	if (!a->bounding_box(box_left) || !b->bounding_box(box_right)) {
		std::cerr << "no bounding box in box_x_compare()" << std::endl;
	}

	return (box_left.center.y() < box_right.center.y());
}
bool box_z_compare(const hitable* a, const hitable* b)
{
	aabb box_left, box_right;
	if (!a->bounding_box(box_left) || !b->bounding_box(box_right)) {
		std::cerr << "no bounding box in box_x_compare()" << std::endl;
	}

	return (box_left.center.z() < box_right.center.z());
}

bvh_node::bvh_node(std::vector<hitable *>& l)
{
	int axis = int(3*drand48());
	if (axis == 0) {
		std::sort(l.begin(), l.end(), box_x_compare);
	} else if (axis == 1) {
		std::sort(l.begin(), l.end(), box_y_compare);
	} else if (axis == 2) {
		std::sort(l.begin(), l.end(), box_z_compare);
	}


	if (l.size() == 1) {
		left = l[0];
		right = l[0];
	} else if (l.size() == 2) {
		left = l[0];
		right = l[1];
	} else {
		std::vector<hitable *> left_l(l.begin(), l.begin()+l.size()/2);
		std::vector<hitable *> right_l(l.begin()+l.size()/2, l.end());
		left = new bvh_node(left_l);
		right = new bvh_node(right_l);
	}

	aabb box_left, box_right;
	if(!left->bounding_box(box_left) || !right->bounding_box(box_right)) {
		std::cerr << "no bounding box in bvh_node constructor\n" << std::endl;
	}
	box = surrounding_box(box_left, box_right);
}


bool bvh_node::bounding_box(aabb& b) const
{
	b = box;
	return true;
}

bool bvh_node::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	if (box.hit(r, t_min, t_max)) {
		hit_record left_rec, right_rec;
		bool hit_left = left->hit(r, t_min, t_max, left_rec);
		bool hit_right = right->hit(r, t_min, t_max, right_rec);

		if (hit_left && hit_right) {
			if (left_rec.t < right_rec.t) {
				rec = left_rec;
			} else {
				rec = right_rec;
			}
			return true;
		} else if (hit_left) {
			rec = left_rec;
			return true;
		} else if (hit_right) {
			rec = right_rec;
			return true;
		} else {
			return false;
		}
	}

	return false;
}


