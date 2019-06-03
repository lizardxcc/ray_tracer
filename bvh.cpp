#include <algorithm>
#include "bvh.h"

//bool box_x_compare(const Hittable& a, const Hittable& b);
//bool box_y_compare(const Hittable& a, const Hittable& b);
//bool box_z_compare(const Hittable& a, const Hittable& b);

bool box_x_compare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b)
{
	AABB box_left, box_right;
	if (!a->BoundingBox(box_left) || !b->BoundingBox(box_right)) {
		std::cerr << "no bounding box in box_x_compare()" << std::endl;
	}

	return (box_left.center.x() < box_right.center.x());
}
bool box_y_compare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b)
{
	AABB box_left, box_right;
	if (!a->BoundingBox(box_left) || !b->BoundingBox(box_right)) {
		std::cerr << "no bounding box in box_x_compare()" << std::endl;
	}

	return (box_left.center.y() < box_right.center.y());
}
bool box_z_compare(const std::shared_ptr<Hittable> a, const std::shared_ptr<Hittable> b)
{
	AABB box_left, box_right;
	if (!a->BoundingBox(box_left) || !b->BoundingBox(box_right)) {
		std::cerr << "no bounding box in box_x_compare()" << std::endl;
	}

	return (box_left.center.z() < box_right.center.z());
}

bvh_node::bvh_node(std::vector<std::shared_ptr<Hittable> >& l)
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
		std::vector<std::shared_ptr<Hittable> > left_l(l.begin(), l.begin()+l.size()/2);
		std::vector<std::shared_ptr<Hittable> > right_l(l.begin()+l.size()/2, l.end());
		left = std::shared_ptr<Hittable>(new bvh_node(left_l));
		right = std::shared_ptr<Hittable>(new bvh_node(right_l));
	}

	AABB box_left, box_right;
	if(!left->BoundingBox(box_left) || !right->BoundingBox(box_right)) {
		std::cerr << "no bounding box in bvh_node constructor\n" << std::endl;
	}
	box = surrounding_box(box_left, box_right);
}


bool bvh_node::BoundingBox(AABB& b) const
{
	b = box;
	return true;
}

bool bvh_node::Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const
{
	if (box.Hit(r, t_min, t_max)) {
		HitRecord left_rec, right_rec;
		bool hit_left = left->Hit(r, t_min, t_max, left_rec);
		bool hit_right = right->Hit(r, t_min, t_max, right_rec);

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


void bvh_node::set_Material(std::shared_ptr<Material> mat)
{
	if (left != nullptr)
		left->set_Material(mat);
	if (right != nullptr)
		right->set_Material(mat);
}
