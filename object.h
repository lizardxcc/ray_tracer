#ifndef OBJECT_H
#define OBJECT_H

#include "hitable.h"
#include "Material.h"
#include "obj.h"
#include "mtl.h"
#include "ply.h"
#include "aabb.h"
#include "bvh.h"

class sphere: public hitable {
	public:
		sphere() { }
		sphere(vec3 center, double r, Material *mat_ptr) : center(center), radius(r) {
			this->mat_ptr = std::shared_ptr<Material>(mat_ptr);
		};
		virtual bool hit(const ray& r, double tmin, double tmux, HitRecord& rec) const;
		virtual bool bounding_box(aabb& box) const;
		virtual std::unique_ptr<pdf> generate_pdf_object(const vec3& o);
		vec3 center;
		double radius;
};

class plane: public hitable {
	public:
		plane() { }
		plane(vec3 somewhere, vec3 normal, Material *mat_ptr) : somewhere(somewhere), normal(normal) {
			this->mat_ptr = std::shared_ptr<Material>(mat_ptr);
		};
		virtual bool hit(const ray& r, double tmin, double tmux, HitRecord& rec) const;
		vec3 somewhere;
		vec3 normal;
};

class rectangle : public hitable {
	public:
		rectangle() { }
		rectangle(vec3 center, vec3 normal, vec3 width_dir, double width, double height, Material *mat_ptr) : center(center), normal(normal), width_dir(width_dir), width(width), height(height) {
			this->mat_ptr = std::shared_ptr<Material>(mat_ptr);
		};
		virtual bool hit(const ray& r, double t_min, double t_max, HitRecord& rec) const;
		virtual std::unique_ptr<pdf> generate_pdf_object(const vec3& o);
		virtual bool bounding_box(aabb& box) const;
		vec3 center;
		vec3 normal;
		vec3 width_dir;
		double width;
		double height;
};

class xy_rect : public hitable {
	public:
		xy_rect() { }
		xy_rect(double x0, double y0, double x1, double y1, double k, Material *mat) :
		x0(x0), x1(x1), y0(y0), y1(y1), k(k) {
			mat_ptr = std::shared_ptr<Material>(mat);
		};
		virtual bool hit(const ray& r, double t_min, double t_max, HitRecord& rec) const;
		virtual bool bounding_box(aabb& box) const;

		double x0, x1, y0, y1, k;
};

class yz_rect : public hitable {
	public:
		yz_rect() { }
		yz_rect(double y0, double z0, double y1, double z1, double k, Material *mat) :
		y0(y0), y1(y1), z0(z0), z1(z1), k(k) {
			mat_ptr = std::shared_ptr<Material>(mat);
		};
		virtual bool hit(const ray& r, double t_min, double t_max, HitRecord& rec) const;
		virtual bool bounding_box(aabb& box) const;

		double y0, y1, z0, z1, k;
};

class zx_rect : public hitable {
	public:
		zx_rect() { }
		zx_rect(double z0, double x0, double z1, double x1, double k, Material *mat) :
		z0(z0), z1(z1), x0(x0), x1(x1), k(k) {
			mat_ptr = std::shared_ptr<Material>(mat);
		};
		virtual bool hit(const ray& r, double t_min, double t_max, HitRecord& rec) const;
		virtual bool bounding_box(aabb& box) const;
		//virtual double generate_pdf_dir(const vec3& o, vec3& direction);

		double z0, z1, x0, x1, k;
};



class flip_normals : public hitable {
	public:
		flip_normals(hitable *p) : ptr(p) {}
		virtual bool hit(const ray& r, double t_min, double t_max, HitRecord& rec) const;
		virtual bool bounding_box(aabb& box) const;
		hitable *ptr;
};

class box : public hitable {
	public:
		box() { }
		box(const vec3& p0, const vec3& p1, Material *mat);
		virtual bool hit(const ray& r, double t_min, double t_max, HitRecord& rec) const;
		virtual bool bounding_box(aabb& box) const;

		vec3 pmin;
		vec3 pmax;
		hitable *list_ptr;
};


class translate : public hitable {
	public:
		translate(hitable *p, const vec3& displacement) : ptr(p), offset(displacement) {}
		virtual bool hit(const ray& r, double t_min, double t_max, HitRecord& rec) const;
		virtual bool bounding_box(aabb& box) const;

		hitable *ptr;
		vec3 offset;
};


class objmodel : public hitable {
	public:
		objmodel(obj& o);
		virtual bool hit(const ray& r, double t_min, double t_max, HitRecord& rec) const;
		virtual bool bounding_box(aabb& box) const;
		//std::vector<std::vector<hitable *>> models;
		std::vector<std::shared_ptr<hitable> > models;
		std::shared_ptr<bvh_node> bvh;
};

class plymodel : public hitable {
	public:
		plymodel(const char *filename, Material *mat);
		virtual bool hit(const ray& r, double t_min, double t_max, HitRecord& rec) const;
		virtual bool bounding_box(aabb& box) const;
		ply p;
		std::vector<std::shared_ptr<hitable> > polygon;
		bvh_node *pol;
};

class triangle : public hitable {
	public:
		virtual bool hit(const ray& r, double t_min, double t_max, HitRecord& rec) const;
		virtual bool bounding_box(aabb& box) const;
		virtual std::unique_ptr<pdf> generate_pdf_object(const vec3& o);
		vec3 v[3];
		vec3 normal[3];
		vec3 face_normal;
};

class quadrilateral : public hitable {
	public:
		virtual bool hit(const ray& r, double t_min, double t_max, HitRecord& rec) const;
		virtual bool bounding_box(aabb& box) const;
		virtual std::unique_ptr<pdf> generate_pdf_object(const vec3& o);
		vec3 v[4];
		vec3 normal;
};

#endif
