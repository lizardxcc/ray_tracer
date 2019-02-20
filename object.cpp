#include <vector>
#include <algorithm>
#include "object.h"
#include "ray.h"
#include "hitablelist.h"
#include "pdf.h"


pdf *hitable::generate_pdf_object(const vec3& o)
{
	pdf *p = new uniform_pdf(vec3(0.0, 0.0, 1.0));
	return p;
}


bool sphere::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
	float a = pow(r.B.length(), 2.0);
	float b_prime = dot(r.A - center, r.B);
	float c = pow((r.A - center).length(), 2.0) - pow(radius, 2.0);
	float discriminant_prime = pow(b_prime, 2.0) - a * c;
	if (discriminant_prime < 0.0) {
		return false;
	}
	float t1 = (-b_prime - sqrt(discriminant_prime)) / a;

	if (t1 >= t_min && t1 <= t_max) {
		rec.t = t1;
		rec.p = r.point_at_parameter(rec.t);
		rec.normal = unit_vector(rec.p - center);
		rec.mat_ptr = mat_ptr;
		return true;
	}
	float t2 = (-b_prime + sqrt(discriminant_prime)) / a;
	if (t2 >= t_min && t2 <= t_max) {
		rec.t = t2;
		rec.p = r.point_at_parameter(rec.t);
		rec.normal = unit_vector(rec.p - center);
		rec.mat_ptr = mat_ptr;
		return true;
	}

	return false;
}

bool sphere::bounding_box(aabb& box) const
{
	box = aabb(
		center - vec3(radius, radius, radius),
		center + vec3(radius, radius, radius)
	);
	return true;
}

pdf *sphere::generate_pdf_object(const vec3& o)
{
	vec3 v = center - o;
	float r = radius;
	pdf *p = new toward_object_pdf(unit_vector(v), atan2(r, v.length()));
	return p;
}


bool plane::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
	float t = dot(somewhere - r.origin(), normal) / dot(r.direction(), normal);
	if (t >= t_min && t <= t_max) {
		rec.t = t;
		rec.p = r.point_at_parameter(rec.t);
		rec.normal = normal;
		rec.mat_ptr = mat_ptr;

		return true;
	}
	return false;
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

bool rectangle::bounding_box(aabb& box) const
{
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


bool xy_rect::bounding_box(aabb& box) const
{
	box.minp = vec3(x0, y0, k - 0.001);
	box.maxp = vec3(x1, y1, k + 0.001);
	return true;
}
bool yz_rect::bounding_box(aabb& box) const
{
	box.minp = vec3(k - 0.001, y0, z0);
	box.maxp = vec3(k + 0.001, y1, z1);
	return true;
}
bool zx_rect::bounding_box(aabb& box) const
{
	box.minp = vec3(x0, k - 0.001, z0);
	box.maxp = vec3(x1, k + 0.001, z1);
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
bool flip_normals::bounding_box(aabb& box) const
{
	return ptr->bounding_box(box);
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


bool box::bounding_box(aabb& box) const
{
	box.minp = pmin;
	box.maxp = pmax;
	return true;
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
bool translate::bounding_box(aabb& box) const
{
	if (ptr->bounding_box(box)) {
		box.minp += offset;
		box.maxp += offset;
		return true;
	} else {
		return false;
	}
}

plymodel::plymodel(const char *filename, material *mat)
{
	p.Load(filename);
	polygon.resize(p.faces.size());
	for (size_t i = 0; i < p.faces.size(); i++) {
		size_t l = p.faces[i].size();
		if (l != 3 && l != 4) {
			std::cerr << "Error: " << std::to_string(l) << " sided polygon is unsupported" << std::endl;
			return;

		}
		hitable *tmp_polygon;
		if (l == 3) {
			triangle *tri = new triangle();
			for (size_t j = 0; j < l; j++) {
				tri->v[j] = p.vertices[p.faces[i][j]][0];
			}
			tri->normal = p.vertices[p.faces[i][0]][1];
			tri->mat_ptr = mat;
			tmp_polygon = tri;
		}
		else if (l == 4) {
			quadrilateral *quad= new quadrilateral();
			for (size_t j = 0; j < l; j++) {
				quad->v[j] = p.vertices[p.faces[i][j]][0];
			}
			quad->normal = p.vertices[p.faces[i][0]][1];
			quad->mat_ptr = mat;
			tmp_polygon = quad;

		}
		polygon[i] = tmp_polygon;
	}

	pol = new bvh_node(polygon);
}

bool plymodel::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
	//bool hit_flag = false;
	//rec.t = t_max;
	//for (size_t i = 0; i < polygon.size(); i++) {
	//	if (polygon[i]->hit(r, t_min, rec.t, rec)) {
	//		hit_flag = true;
	//	}
	//}

	//return hit_flag;
	//return pol.hit(r, t_min, t_max, rec);
	return pol->hit(r, t_min, t_max, rec);
}


bool plymodel::bounding_box(aabb& box) const
{
	aabb temp_box;
	if (polygon.size() == 0) {
		return false;
	}

	if (polygon[0]->bounding_box(temp_box) == false) {
		return false;
	}

	box = temp_box;

	for (size_t i = 0; i < polygon.size(); i++) {
		if (polygon[i]->bounding_box(temp_box)) {
			box = surrounding_box(box, temp_box);
		} else {
			return false;
		}
	}

	return true;
}


bool triangle::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
	float t = dot(v[0] - r.origin(), normal) / dot(r.direction(), normal);
	if (t >= t_min && t <= t_max) {
		vec3 p = r.point_at_parameter(t);
		vec3 result0 = cross(v[1]-v[0], p-v[1]);
		vec3 result1 = cross(v[2]-v[1], p-v[2]);
		vec3 result2 = cross(v[0]-v[2], p-v[0]);
		if (dot(result0, result1) > 0.0 && dot(result1, result2) > 0.0) {
			rec.t = t;
			rec.p = p;
			rec.normal = normal;
			rec.mat_ptr = mat_ptr;
			return true;
		}
	}
	return false;
}



bool triangle::bounding_box(aabb& box) const
{
	box.minp = vec3(
	std::min(v[0].x(), std::min(v[1].x(), v[2].x())),
	std::min(v[0].y(), std::min(v[1].y(), v[2].y())),
	std::min(v[0].z(), std::min(v[1].z(), v[2].z()))
	);
	box.maxp = vec3(
	std::max(v[0].x(), std::max(v[1].x(), v[2].x())),
	std::max(v[0].y(), std::max(v[1].y(), v[2].y())),
	std::max(v[0].z(), std::max(v[1].z(), v[2].z()))
	);
	return true;
}


bool quadrilateral::hit(const ray& r, float t_min, float t_max, hit_record& rec) const
{
	float t = dot(v[0] - r.origin(), normal) / dot(r.direction(), normal);
	if (t >= t_min && t <= t_max) {
		vec3 p = r.point_at_parameter(t);
		vec3 result0 = cross(v[1]-v[0], p-v[1]);
		vec3 result1 = cross(v[2]-v[1], p-v[2]);
		vec3 result2 = cross(v[0]-v[2], p-v[0]);

		vec3 result3 = cross(v[3]-v[2], p-v[3]);
		vec3 result4 = cross(v[0]-v[3], p-v[0]);
		vec3 result5 = cross(v[2]-v[0], p-v[2]);

		bool dot_result0 = (dot(result0, result1) > 0.0 && dot(result1, result2) > 0.0);
		bool dot_result1 = (dot(result3, result4) > 0.0 && dot(result4, result5) > 0.0);
		if (dot_result0 || dot_result1) {
			rec.t = t;
			rec.p = p;
			rec.normal = normal;
			rec.mat_ptr = mat_ptr;
			return true;
		}
	}
	return false;
}



bool quadrilateral::bounding_box(aabb& box) const
{
	box.minp = vec3(
	std::min({v[0].x(), v[1].x(), v[2].x(), v[3].x()}),
	std::min({v[0].y(), v[1].y(), v[2].y(), v[3].y()}),
	std::min({v[0].z(), v[1].z(), v[2].z(), v[3].z()})
	);
	box.maxp = vec3(
	std::max({v[0].x(), v[1].x(), v[2].x(), v[3].x()}),
	std::max({v[0].y(), v[1].y(), v[2].y(), v[3].y()}),
	std::max({v[0].z(), v[1].z(), v[2].z(), v[3].z()})
	);
	return true;
}


pdf *quadrilateral::generate_pdf_object(const vec3& o)
{
	vec3 c = 0.25 * (v[0]+v[1]+v[2]+v[3]);
	vec3 vec = c - o;
	float r = 0.25*((v[0]-c).length()+(v[1]-c).length()+(v[2]-c).length()+(v[3]-c).length());
	pdf *p = new toward_object_pdf(unit_vector(vec), atan2(r, vec.length()));
	return p;
}
