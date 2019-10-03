#include <algorithm>
#include <assert.h>
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

BVHNode::BVHNode(std::vector<std::shared_ptr<Hittable> >& l)
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
		left = std::shared_ptr<Hittable>(new BVHNode(left_l));
		right = std::shared_ptr<Hittable>(new BVHNode(right_l));
	}

	AABB box_left, box_right;
	if(!left->BoundingBox(box_left) || !right->BoundingBox(box_right)) {
		std::cerr << "no bounding box in BVHNode constructor\n" << std::endl;
	}
	box = SurroundingBox(box_left, box_right);
}


bool BVHNode::BoundingBox(AABB& b) const
{
	b = box;
	return true;
}

bool BVHNode::Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const
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


bool BVHNode::Occluded(const ray& r, double t_min, double t_max) const
{
	if (box.Hit(r, t_min, t_max)) {
		bool hit_left = left->Occluded(r, t_min, t_max);
		if (hit_left)
			return true;
		bool hit_right = right->Occluded(r, t_min, t_max);
		if (hit_right)
			return true;
	}

	return false;
}

//void BVHNode::SetMaterial(std::shared_ptr<Material> mat)
void BVHNode::SetMaterial(NodeMaterial *mat)
{
	if (left != nullptr)
		left->SetMaterial(mat);
	if (right != nullptr)
		right->SetMaterial(mat);
}


SAHBVHNode::SAHBVHNode(void)
{
}


SAHBVHNode::SAHBVHNode(std::vector<std::shared_ptr<Hittable>>& list)
{
	assert(list.size() > 0);
	if (list.size() == 1) {
		left = list[0];
		right = list[0];
		assert(left->BoundingBox(box));
		return;
	} else if (list.size() == 2) {
		left = list[0];
		right = list[1];
		AABB left_box;
		AABB right_box;
		assert(left->BoundingBox(left_box));
		assert(right->BoundingBox(right_box));
		box = SurroundingBox(left_box, right_box);
		return;
	}
	double best_cost = std::numeric_limits<double>::max();
	int best_axis = -1;
	int best_split_index = -1;
	const double T_aabb = 1.0;
	const double T_triangle = 1.0;
	for (int axis = 0; axis < 3; axis++) {
		std::sort(list.begin(), list.end(),
				[axis](std::shared_ptr<Hittable>& h0, std::shared_ptr<Hittable>& h1){
				AABB aabb0, aabb1;
				h0->BoundingBox(aabb0);
				h1->BoundingBox(aabb1);
				return (aabb0.center[axis] < aabb1.center[axis]);
				});
		double all_surface_area;
		std::vector<double> surfaces_area_0(list.size()-1);
		std::vector<int> triangle_size_0(list.size()-1);
		std::vector<double> surfaces_area_1(list.size()-1);
		std::vector<int> triangle_size_1(list.size()-1);

		list[0]->BoundingBox(box);
		surfaces_area_0[0] = box.SurfaceArea();
		triangle_size_0[0] = 1;
		for (int i = 1; i < list.size()-1; i++) {
			AABB tmp_box;
			list[i]->BoundingBox(tmp_box);
			box = SurroundingBox(box, tmp_box);
			surfaces_area_0[i] = box.SurfaceArea();
			triangle_size_0[i] = triangle_size_0[i-1]+1;
		}

		list[list.size()-1]->BoundingBox(box);
		surfaces_area_1[list.size()-2] = box.SurfaceArea();
		triangle_size_1[list.size()-2] = 1;
		for (int i = list.size()-3; i >= 0; i--) {
			AABB tmp_box;
			list[i]->BoundingBox(tmp_box);
			box = SurroundingBox(box, tmp_box);
			surfaces_area_1[i] = box.SurfaceArea();
			triangle_size_1[i] = triangle_size_1[i+1] + 1;
		}
		{
			AABB tmp_box;
			list[0]->BoundingBox(tmp_box);
			this->box = SurroundingBox(box, tmp_box);
			all_surface_area = box.SurfaceArea();
		}

		for (int i = 0; i < list.size()-1; i++) {
			double cost = T_aabb +
				surfaces_area_0[i]/all_surface_area * triangle_size_0[i] * T_triangle +
				surfaces_area_1[i]/all_surface_area * triangle_size_1[i] * T_triangle;
			if (cost < best_cost) {
				best_cost = cost;
				best_axis = axis;
				best_split_index = i;
			}
		}


	}
	std::vector<std::shared_ptr<Hittable>> polygons0, polygons1;
	if (best_axis == -1) {
		assert(false);
	}
	std::sort(list.begin(), list.end(),
			[best_axis](std::shared_ptr<Hittable>& h0, std::shared_ptr<Hittable>& h1){
			AABB aabb0, aabb1;
			h0->BoundingBox(aabb0);
			h1->BoundingBox(aabb1);
			return (aabb0.center[best_axis] < aabb1.center[best_axis]);
			});
	std::copy(list.begin(), list.begin() + best_split_index+1, std::back_inserter(polygons0));
	std::copy(list.begin()+best_split_index+1, list.end(), std::back_inserter(polygons1));
	left = std::shared_ptr<Hittable>(new SAHBVHNode(polygons0));
	right = std::shared_ptr<Hittable>(new SAHBVHNode(polygons1));

	AABB box_left, box_right;
	assert(left->BoundingBox(box_left));
	assert(right->BoundingBox(box_right));
	box = SurroundingBox(box_left, box_right);
}
