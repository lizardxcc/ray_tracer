#include <vector>
#include "object.h"
#include "ray.h"
#include "hitablelist.h"


bool rectangle::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
	float t = dot(center - r.origin(), normal) / dot(r.direction(), normal);
	if (t >= t_min && t <= t_max) {
		rec.t = t;
		rec.p = r.point_at_parameter(rec.t);

		//vec3 width_dir = unit_vector(width_dir);
		vec3 height_dir = cross(width_dir, normal);
		vec3 v = rec.p - center;
		if (!(abs(dot(v, width_dir)) <= width/2 && abs(dot(v, height_dir)) <= height/2)) {
			return false;
		}

		rec.normal = normal;
		rec.mat_ptr = mat_ptr;

		return true;
	}
	return false;
}


bool xy_rect::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
	float t = (k - r.origin().z()) / r.direction().z();
	if (t < t_min || t > t_max)
		return false;

	float x = r.origin().x() + t * r.direction().x();
	float y = r.origin().y() + t * r.direction().y();
	if (x < x0 || x > x1 || y < y0 || y > y1)
		return false;

	rec.t = t;
	rec.mat_ptr = mat_ptr;
	rec.p = r.point_at_parameter(t);
	rec.normal = vec3(0, 0, 1);

	return true;
}
bool yz_rect::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
	float t = (k - r.origin().x()) / r.direction().x();
	if (t < t_min || t > t_max)
		return false;

	float y = r.origin().y() + t * r.direction().y();
	float z = r.origin().z() + t * r.direction().z();
	if (y < y0 || y > y1 || z < z0 || z > z1)
		return false;

	rec.t = t;
	rec.mat_ptr = mat_ptr;
	rec.p = r.point_at_parameter(t);
	rec.normal = vec3(1, 0, 0);

	return true;
}
bool zx_rect::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
	float t = (k - r.origin().y()) / r.direction().y();
	if (t < t_min || t > t_max)
		return false;

	float z = r.origin().z() + t * r.direction().z();
	float x = r.origin().x() + t * r.direction().x();
	if (z < z0 || z > z1 || x < x0 || x > x1)
		return false;

	rec.t = t;
	rec.mat_ptr = mat_ptr;
	rec.p = r.point_at_parameter(t);
	rec.normal = vec3(0, 1, 0);

	return true;
}



bool flip_normals::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
	if (ptr->hit(r, t_min, t_max, rec)) {
		rec.normal = -rec.normal;
		return true;
	} else
		return false;
}


box::box(const vec3& p0, const vec3& p1, material *ptr)
{
	pmin = p0;
	pmax = p1;

	std::vector<hitable*> list;
	list.push_back(new xy_rect(p0.x(), p0.y(), p1.x(), p1.y(), p1.z(), ptr));
	list.push_back(new flip_normals(new xy_rect(p0.x(), p0.y(), p1.x(), p1.y(), p0.z(), ptr)));
	list.push_back(new zx_rect(p0.z(), p0.x(), p1.z(), p1.x(), p1.y(), ptr));
	list.push_back(new flip_normals(new zx_rect(p0.z(), p0.x(), p1.z(), p1.x(), p0.y(), ptr)));
	list.push_back(new yz_rect(p0.y(), p0.z(), p1.y(), p1.z(), p1.x(), ptr));
	list.push_back(new flip_normals(new yz_rect(p0.y(), p0.z(), p1.y(), p1.z(), p0.x(), ptr)));

	list_ptr = new hitable_list(list);
}

bool box::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
	return list_ptr->hit(r, t_min, t_max, rec);
}
