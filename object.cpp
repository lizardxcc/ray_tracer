#include <vector>
#include <algorithm>
#include "object.h"
#include "ray.h"
#include "hitablelist.h"
#include "pdf.h"


//std::unique_ptr<pdf> hitable::generate_pdf_object(const vec3& o)
//{
//	std::cout << "Warning generate_pdf_object" << std::endl;
//	return std::make_unique<uniform_pdf>(vec3(0.0, 0.0, 1.0));
//}
std::unique_ptr<pdf> hitable::generate_pdf_object(const vec3& o)
{
	aabb box;
	if (bounding_box(box)) {
		vec3 v = box.center - o;
		double r = (box.center-box.minp).length();
		return std::make_unique<toward_object_pdf>(unit_vector(v), atan2(r, v.length()));
	} else {
		std::cout << "Warning generate_pdf_object" << std::endl;
	}
}


void hitable::set_Material(std::shared_ptr<Material> mat)
{
	mat_ptr = mat;
}


bool sphere::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	double a = pow(r.B.length(), 2.0);
	double b_prime = dot(r.A - center, r.B);
	double c = pow((r.A - center).length(), 2.0) - pow(radius, 2.0);
	double discriminant_prime = pow(b_prime, 2.0) - a * c;
	if (discriminant_prime < 0.0) {
		return false;
	}
	double t1 = (-b_prime - sqrt(discriminant_prime)) / a;

	if (t1 >= t_min && t1 <= t_max) {
		rec.t = t1;
		rec.p = r.point_at_parameter(rec.t);
		rec.normal = unit_vector(rec.p - center);
		rec.mat_ptr = mat_ptr;
		rec.hit_object_id = object_id;
		return true;
	}
	double t2 = (-b_prime + sqrt(discriminant_prime)) / a;
	if (t2 >= t_min && t2 <= t_max) {
		rec.t = t2;
		rec.p = r.point_at_parameter(rec.t);
		rec.normal = unit_vector(rec.p - center);
		rec.mat_ptr = mat_ptr;
		rec.hit_object_id = object_id;
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

std::unique_ptr<pdf> sphere::generate_pdf_object(const vec3& o)
{
	vec3 v = center - o;
	double r = radius;
	return std::make_unique<toward_object_pdf>(unit_vector(v), atan2(r, v.length()));
}


bool plane::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	double t = dot(somewhere - r.origin(), normal) / dot(r.direction(), normal);
	if (t >= t_min && t <= t_max) {
		rec.t = t;
		rec.p = r.point_at_parameter(rec.t);
		rec.normal = normal;
		rec.mat_ptr = mat_ptr;
		rec.hit_object_id = object_id;

		return true;
	}
	return false;
}

bool rectangle::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	double t = dot(center - r.origin(), normal) / dot(r.direction(), normal);
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
		rec.hit_object_id = object_id;

		return true;
	}
	return false;
}



std::unique_ptr<pdf> rectangle::generate_pdf_object(const vec3& o)
{
	vec3 v = center - o;
	double r = 0.5 * sqrt(width*width + height*height);
	return std::make_unique<toward_object_pdf>(unit_vector(v), atan2(r, v.length()));
}

bool rectangle::bounding_box(aabb& box) const
{
	return false;
}

bool xy_rect::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	double t = (k - r.origin().z()) / r.direction().z();
	if (t < t_min || t > t_max)
		return false;

	double x = r.origin().x() + t * r.direction().x();
	double y = r.origin().y() + t * r.direction().y();
	if (x < x0 || x > x1 || y < y0 || y > y1)
		return false;

	rec.t = t;
	rec.mat_ptr = mat_ptr;
	rec.p = r.point_at_parameter(t);
	rec.normal = vec3(0, 0, 1);
	rec.hit_object_id = object_id;

	return true;
}
bool yz_rect::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	double t = (k - r.origin().x()) / r.direction().x();
	if (t < t_min || t > t_max)
		return false;

	double y = r.origin().y() + t * r.direction().y();
	double z = r.origin().z() + t * r.direction().z();
	if (y < y0 || y > y1 || z < z0 || z > z1)
		return false;

	rec.t = t;
	rec.mat_ptr = mat_ptr;
	rec.p = r.point_at_parameter(t);
	rec.normal = vec3(1, 0, 0);
	rec.hit_object_id = object_id;

	return true;
}
bool zx_rect::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	double t = (k - r.origin().y()) / r.direction().y();
	if (t < t_min || t > t_max)
		return false;

	double z = r.origin().z() + t * r.direction().z();
	double x = r.origin().x() + t * r.direction().x();
	if (z < z0 || z > z1 || x < x0 || x > x1)
		return false;

	rec.t = t;
	rec.mat_ptr = mat_ptr;
	rec.p = r.point_at_parameter(t);
	rec.normal = vec3(0, 1, 0);
	rec.hit_object_id = object_id;

	return true;
}


bool xy_rect::bounding_box(aabb& box) const
{
	box = aabb(vec3(x0, y0, k - 0.001), vec3(x1, y1, k + 0.001));
	return true;
}
bool yz_rect::bounding_box(aabb& box) const
{
	box = aabb(vec3(k - 0.001, y0, z0), vec3(k + 0.001, y1, z1));
	return true;
}
bool zx_rect::bounding_box(aabb& box) const
{
	box = aabb(vec3(x0, k - 0.001, z0), vec3(x1, k + 0.001, z1));
	return true;
}
//double zx_rect::generate_pdf_dir(const vec3& o, vec3& direction)
//{
//	vec3 random_point_on_rect = vec3(
//	x0 + drand48()*(x1-x0),
//	k,
//	z0 + drand48()*(z1-z0));
//	vec3 generated_direction = random_point_on_rect - o;
//	direction = unit_vector(generated_direction);
//
//	double area = (x1-x0)*(z1-z0);
//	double distance_squared = generated_direction.squared_length();
//	//double cosine = fabs(dot(direction, rec.normall);
//	double cosine = fabs(dot(direction, vec3(0, -1, 0));
//	return distance_squared / (cosine * area);
//}


bool flip_normals::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
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


box::box(const vec3& p0, const vec3& p1, Material *ptr)
{
	pmin = p0;
	pmax = p1;

	std::vector<std::shared_ptr<hitable> > list;
	list.push_back(std::make_shared<xy_rect>(p0.x(), p0.y(), p1.x(), p1.y(), p1.z(), ptr));
	list.push_back(std::make_shared<flip_normals>(new xy_rect(p0.x(), p0.y(), p1.x(), p1.y(), p0.z(), ptr)));
	list.push_back(std::make_shared<zx_rect>(p0.z(), p0.x(), p1.z(), p1.x(), p1.y(), ptr));
	list.push_back(std::make_shared<flip_normals>(new zx_rect(p0.z(), p0.x(), p1.z(), p1.x(), p0.y(), ptr)));
	list.push_back(std::make_shared<yz_rect>(p0.y(), p0.z(), p1.y(), p1.z(), p1.x(), ptr));
	list.push_back(std::make_shared<flip_normals>(new yz_rect(p0.y(), p0.z(), p1.y(), p1.z(), p0.x(), ptr)));

	list_ptr = new hitable_list(list);
}

bool box::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	return list_ptr->hit(r, t_min, t_max, rec);
}


bool box::bounding_box(aabb& box) const
{
	box = aabb(pmin, pmax);
	return true;
}


bool translate::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
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
		box.center += offset;
		return true;
	} else {
		return false;
	}
}


objmodel::objmodel(obj& o)
{
	models.resize(o.objects.size());
	for (size_t i = 0; i < o.objects.size(); i++) {
		const auto& object = o.objects[i];
		std::vector<std::shared_ptr<hitable> > model;
		model.resize(object->f.size());
		std::cout << "f: " << object->f.size() << std::endl;

		for (size_t j = 0; j < o.objects[i]->f.size(); j++) {
			auto& f = o.objects[i]->f[j];
			size_t l = f.size();
			if (l != 3 && l != 4) {
				std::cerr << "Error: " << l << " sided polygon is unsupported" << std::endl;
				return;
			}
			std::shared_ptr<hitable> tmp_model;
			if (l == 3) {
				std::shared_ptr<triangle> tri = std::make_shared<triangle>();
				for (size_t k = 0; k < l; k++) {
					tri->v[k] = object->v[*f[k][0]];
					tri->normal[k] = object->vn[*f[k][2]];
				}
				tri->face_normal = unit_vector(cross(tri->v[1] - tri->v[0], tri->v[2] - tri->v[1]));
				tri->mat_ptr = nullptr;
				tmp_model = tri;
			} else if (l == 4) {
				std::shared_ptr<quadrilateral> quad = std::make_shared<quadrilateral>();
				quad->normal = vec3(0, 0, 0);
				for (size_t k = 0; k < l; k++) {
					quad->v[k] = object->v[*f[k][0]];
					quad->normal += object->vn[*f[k][2]];
				}
				//quad->normal.make_unit_vector();
				quad->normal = unit_vector(cross(quad->v[1] - quad->v[0], quad->v[2] - quad->v[1]));
				quad->mat_ptr = nullptr;
				tmp_model = quad;
			}
			tmp_model->object_id = i;
			model[j] = tmp_model;
		}
		std::shared_ptr<hitable> b = std::make_shared<bvh_node>(model);
		models[i] = b;
	}

	auto v(models);
	bvh = std::make_shared<bvh_node>(v);
}

bool objmodel::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	return bvh->hit(r, t_min, t_max, rec);
}
bool objmodel::bounding_box(aabb& box) const
{
	box = bvh->box;

	return true;
}

plymodel::plymodel(const char *filename, Material *mat)
{
	p.Load(filename);
	polygon.resize(p.faces.size());
	for (size_t i = 0; i < p.faces.size(); i++) {
		size_t l = p.faces[i].size();
		if (l != 3 && l != 4) {
			std::cerr << "Error: " << std::to_string(l) << " sided polygon is unsupported" << std::endl;
			return;

		}
		std::shared_ptr<hitable> tmp_polygon;
		if (l == 3) {
			std::shared_ptr<triangle> tri(new triangle());
			for (size_t j = 0; j < l; j++) {
				tri->v[j] = p.vertices[p.faces[i][j]][0];
			}
			//tri->normal = p.vertices[p.faces[i][0]][1];
			tri->mat_ptr = std::shared_ptr<Material>(mat);
			tmp_polygon = tri;
		}
		else if (l == 4) {
			std::shared_ptr<quadrilateral> quad(new quadrilateral());
			for (size_t j = 0; j < l; j++) {
				quad->v[j] = p.vertices[p.faces[i][j]][0];
			}
			quad->normal = p.vertices[p.faces[i][0]][1];
			quad->mat_ptr = std::shared_ptr<Material>(mat);
			tmp_polygon = quad;

		}
		polygon[i] = tmp_polygon;
	}

	pol = new bvh_node(polygon);
}

bool plymodel::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
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


bool triangle::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	double t = dot(v[0] - r.origin(), face_normal) / dot(r.direction(), face_normal);
	if (t >= t_min && t <= t_max) {
		vec3 p = r.point_at_parameter(t);
		double tri_area = cross(v[1]-v[0], v[2]-v[1]).length();
		vec3 result0 = cross(v[1]-v[0], p-v[1]);
		vec3 result1 = cross(v[2]-v[1], p-v[2]);
		vec3 result2 = cross(v[0]-v[2], p-v[0]);
		if (dot(result0, result1) > 0.0 && dot(result1, result2) > 0.0) {
			rec.t = t;
			rec.p = p;
			rec.normal = (result1.length()*normal[0]+result2.length()*normal[1]+result0.length()*normal[2])/tri_area;
			rec.mat_ptr = mat_ptr;
			rec.hit_object_id = object_id;
			return true;
		}
	}
	return false;
}



bool triangle::bounding_box(aabb& box) const
{
	vec3 minp = vec3(
	std::min(v[0].x(), std::min(v[1].x(), v[2].x())),
	std::min(v[0].y(), std::min(v[1].y(), v[2].y())),
	std::min(v[0].z(), std::min(v[1].z(), v[2].z()))
	);
	vec3 maxp = vec3(
	std::max(v[0].x(), std::max(v[1].x(), v[2].x())),
	std::max(v[0].y(), std::max(v[1].y(), v[2].y())),
	std::max(v[0].z(), std::max(v[1].z(), v[2].z()))
	);

	for (size_t i = 0; i < 3; i++) {
		if (minp.e[i] == maxp.e[i]) {
			minp.e[i] -= 0.0001;
			maxp.e[i] += 0.0001;
		}
	}
	box = aabb(minp, maxp);
	return true;
}



std::unique_ptr<pdf> triangle::generate_pdf_object(const vec3& o)
{
	vec3 c = (v[0]+v[1]+v[2])/3.0;
	vec3 vec = c - o;
	double r = ((v[0]-c).length()+(v[1]-c).length()+(v[2]-c).length())/3.0;
	return std::make_unique<toward_object_pdf>(unit_vector(vec), atan2(r, vec.length()));
}

bool quadrilateral::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	double t = dot(v[0] - r.origin(), normal) / dot(r.direction(), normal);
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
			rec.hit_object_id = object_id;
			return true;
		}
	}
	return false;
}



bool quadrilateral::bounding_box(aabb& box) const
{
	vec3 minp = vec3(
	std::min({v[0].x(), v[1].x(), v[2].x(), v[3].x()}),
	std::min({v[0].y(), v[1].y(), v[2].y(), v[3].y()}),
	std::min({v[0].z(), v[1].z(), v[2].z(), v[3].z()})
	);
	vec3 maxp = vec3(
	std::max({v[0].x(), v[1].x(), v[2].x(), v[3].x()}),
	std::max({v[0].y(), v[1].y(), v[2].y(), v[3].y()}),
	std::max({v[0].z(), v[1].z(), v[2].z(), v[3].z()})
	);

	for (size_t i = 0; i < 3; i++) {
		if (minp.e[i] == maxp.e[i]) {
			minp.e[i] -= 0.0001;
			maxp.e[i] += 0.0001;
		}
	}

	box = aabb(minp, maxp);
	return true;
}


std::unique_ptr<pdf> quadrilateral::generate_pdf_object(const vec3& o)
{
	vec3 c = 0.25 * (v[0]+v[1]+v[2]+v[3]);
	vec3 vec = c - o;
	double r = 0.25*((v[0]-c).length()+(v[1]-c).length()+(v[2]-c).length()+(v[3]-c).length());
	return std::make_unique<toward_object_pdf>(unit_vector(vec), atan2(r, vec.length()));
}
