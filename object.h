#ifndef OBJECT_H
#define OBJECT_H

#include "hitable.h"
#include "material.h"
#include "ply.h"
#include "aabb.h"
#include "bvh.h"

class rectangle : public hitable {
	public:
		rectangle() { }
		rectangle(vec3 center, vec3 normal, vec3 width_dir, float width, float height, material *mat_ptr) : center(center), normal(normal), width_dir(width_dir), width(width), height(height), mat_ptr(mat_ptr) {};
		virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
		virtual pdf *generate_pdf_object(const vec3& o);
		virtual bool bounding_box(aabb& box) const;
		vec3 center;
		vec3 normal;
		vec3 width_dir;
		float width;
		float height;
		material *mat_ptr;
};

class xy_rect : public hitable {
	public:
		xy_rect() { }
		xy_rect(float x0, float y0, float x1, float y1, float k, material *mat) :
		x0(x0), x1(x1), y0(y0), y1(y1), k(k), mat_ptr(mat) {};
		virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
		virtual bool bounding_box(aabb& box) const;

		float x0, x1, y0, y1, k;
		material *mat_ptr;
};

class yz_rect : public hitable {
	public:
		yz_rect() { }
		yz_rect(float y0, float z0, float y1, float z1, float k, material *mat) :
		y0(y0), y1(y1), z0(z0), z1(z1), k(k), mat_ptr(mat) {};
		virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
		virtual bool bounding_box(aabb& box) const;

		float y0, y1, z0, z1, k;
		material *mat_ptr;
};

class zx_rect : public hitable {
	public:
		zx_rect() { }
		zx_rect(float z0, float x0, float z1, float x1, float k, material *mat) :
		z0(z0), z1(z1), x0(x0), x1(x1), k(k), mat_ptr(mat) {};
		virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
		virtual bool bounding_box(aabb& box) const;
		//virtual float generate_pdf_dir(const vec3& o, vec3& direction);

		float z0, z1, x0, x1, k;
		material *mat_ptr;
};



class flip_normals : public hitable {
	public:
		flip_normals(hitable *p) : ptr(p) {}
		virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
		virtual bool bounding_box(aabb& box) const;
		hitable *ptr;
};

class box : public hitable {
	public:
		box() { }
		box(const vec3& p0, const vec3& p1, material *mat);
		virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
		virtual bool bounding_box(aabb& box) const;

		vec3 pmin;
		vec3 pmax;
		hitable *list_ptr;
		material *mat_ptr;
};


class translate : public hitable {
	public:
		translate(hitable *p, const vec3& displacement) : ptr(p), offset(displacement) {}
		virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
		virtual bool bounding_box(aabb& box) const;

		hitable *ptr;
		vec3 offset;
};

class plymodel : public hitable {
	public:
		plymodel(const char *filename, material *mat);
		virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
		virtual bool bounding_box(aabb& box) const;
		ply p;
		std::vector<hitable *> polygon;
		bvh_node *pol;
};

class triangle : public hitable {
	public:
		virtual bool hit(const ray& r, float t_min, float t_max, hit_record& rec) const;
		virtual bool bounding_box(aabb& box) const;
		material *mat_ptr;
		vec3 v[3];
		vec3 normal;
};


#endif
