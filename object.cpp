#include <vector>
#include "object.h"
#include "ray.h"
#include "hitablelist.h"
#include "pdf.h"


pdf *hitable::generate_pdf_object(const vec3& o)
{
	pdf *p = new uniform_pdf(vec3(0.0, 0.0, 1.0));
	return p;
}

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

pdf *rectangle::generate_pdf_object(const vec3& o)
{
	vec3 v = center - o;
	float r = 0.5 * sqrt(width*width + height*height);
	pdf *p = new toward_object_pdf(unit_vector(v), atan2(r, v.length()));
	return p;
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


//float zx_rect::generate_pdf_dir(const vec3& o, vec3& direction)
//{
//	vec3 random_point_on_rect = vec3(
//	x0 + drand48()*(x1-x0),
//	k,
//	z0 + drand48()*(z1-z0));
//	vec3 generated_direction = random_point_on_rect - o;
//	direction = unit_vector(generated_direction);
//
//	float area = (x1-x0)*(z1-z0);
//	float distance_squared = generated_direction.squared_length();
//	//float cosine = fabs(dot(direction, rec.normall);
//	float cosine = fabs(dot(direction, vec3(0, -1, 0));
//	return distance_squared / (cosine * area);
//}


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




bool translate::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
	ray moved_r(r.origin() - offset, r.direction());
	if (ptr->hit(moved_r, t_min, t_max, rec)) {
		rec.p += offset;
		return true;
	} else
		return false;
}


bool plymodel::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
	bool hit_flag = false;
	rec.t = t_max;
	for (size_t i = 0; i < p.faces.size(); i++) {
		if (p.faces[i].size() == 3) {
			vec3 normal = unit_vector(p.vertices[p.faces[i][0]][1]);
			vec3 a = p.vertices[p.faces[i][0]][0]*0.8;
			vec3 b = p.vertices[p.faces[i][1]][0]*0.8;
			vec3 c = p.vertices[p.faces[i][2]][0]*0.8;
			float t = dot(a - r.origin(), normal) / dot(r.direction(), normal);
			if (t >= t_min && t <= rec.t) {
				vec3 p = r.point_at_parameter(t);
				vec3 result0 = cross(b-a, p-b);
				vec3 result1 = cross(c-b, p-c);
				vec3 result2 = cross(a-c, p-a);
				if (dot(result0, result1) > 0.0 && dot(result1, result2) > 0.0) {
					rec.t = t;
					rec.p = p;
					rec.normal = normal;
					rec.mat_ptr = mat_ptr;
					hit_flag = true;
				}
			}
		} else {
			return false;
		}
	}

	return hit_flag;
}
