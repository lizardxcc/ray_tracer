#ifndef OBJECT_H
#define OBJECT_H

#include "Hittable.h"
#include "material.h"
#include "obj.h"
#include "mtl.h"
#include "ply.h"
#include "aabb.h"
#include "bvh.h"

class Sphere: public Hittable {
	public:
		Sphere() { }
		Sphere(vec3 center, double r, Material *mat_ptr) : center(center), radius(r) {
			this->mat_ptr = std::shared_ptr<Material>(mat_ptr);
		};
		virtual bool Hit(const ray& r, double tmin, double tmux, HitRecord& rec) const;
		virtual bool BoundingBox(aabb& box) const;
		virtual std::unique_ptr<Pdf> GeneratePdfObject(const vec3& o);
		vec3 center;
		double radius;
};

class Plane: public Hittable {
	public:
		Plane() { }
		Plane(vec3 somewhere, vec3 normal, Material *mat_ptr) : somewhere(somewhere), normal(normal) {
			this->mat_ptr = std::shared_ptr<Material>(mat_ptr);
		};
		virtual bool Hit(const ray& r, double tmin, double tmux, HitRecord& rec) const;
		vec3 somewhere;
		vec3 normal;
};

class Rectangle : public Hittable {
	public:
		Rectangle() { }
		Rectangle(vec3 center, vec3 normal, vec3 width_dir, double width, double height, Material *mat_ptr) : center(center), normal(normal), width_dir(width_dir), width(width), height(height) {
			this->mat_ptr = std::shared_ptr<Material>(mat_ptr);
		};
		virtual bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const;
		virtual std::unique_ptr<Pdf> GeneratePdfObject(const vec3& o);
		virtual bool BoundingBox(aabb& box) const;
		vec3 center;
		vec3 normal;
		vec3 width_dir;
		double width;
		double height;
};

class xy_rect : public Hittable {
	public:
		xy_rect() { }
		xy_rect(double x0, double y0, double x1, double y1, double k, Material *mat) :
		x0(x0), x1(x1), y0(y0), y1(y1), k(k) {
			mat_ptr = std::shared_ptr<Material>(mat);
		};
		virtual bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const;
		virtual bool BoundingBox(aabb& box) const;

		double x0, x1, y0, y1, k;
};

class yz_rect : public Hittable {
	public:
		yz_rect() { }
		yz_rect(double y0, double z0, double y1, double z1, double k, Material *mat) :
		y0(y0), y1(y1), z0(z0), z1(z1), k(k) {
			mat_ptr = std::shared_ptr<Material>(mat);
		};
		virtual bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const;
		virtual bool BoundingBox(aabb& box) const;

		double y0, y1, z0, z1, k;
};

class zx_rect : public Hittable {
	public:
		zx_rect() { }
		zx_rect(double z0, double x0, double z1, double x1, double k, Material *mat) :
		z0(z0), z1(z1), x0(x0), x1(x1), k(k) {
			mat_ptr = std::shared_ptr<Material>(mat);
		};
		virtual bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const;
		virtual bool BoundingBox(aabb& box) const;
		//virtual double generate_Pdf_dir(const vec3& o, vec3& direction);

		double z0, z1, x0, x1, k;
};



class flip_normals : public Hittable {
	public:
		flip_normals(Hittable *p) : ptr(p) {}
		virtual bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const;
		virtual bool BoundingBox(aabb& box) const;
		Hittable *ptr;
};

class box : public Hittable {
	public:
		box() { }
		box(const vec3& p0, const vec3& p1, Material *mat);
		virtual bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const;
		virtual bool BoundingBox(aabb& box) const;

		vec3 pmin;
		vec3 pmax;
		Hittable *list_ptr;
};


class translate : public Hittable {
	public:
		translate(Hittable *p, const vec3& displacement) : ptr(p), offset(displacement) {}
		virtual bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const;
		virtual bool BoundingBox(aabb& box) const;

		Hittable *ptr;
		vec3 offset;
};


class objmodel : public Hittable {
	public:
		objmodel(obj& o);
		virtual bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const;
		virtual bool BoundingBox(aabb& box) const;
		//std::vector<std::vector<Hittable *>> models;
		std::vector<std::shared_ptr<Hittable> > models;
		std::shared_ptr<bvh_node> bvh;
};

class plymodel : public Hittable {
	public:
		plymodel(const char *filename, Material *mat);
		virtual bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const;
		virtual bool BoundingBox(aabb& box) const;
		ply p;
		std::vector<std::shared_ptr<Hittable> > polygon;
		bvh_node *pol;
};

class Triangle : public Hittable {
	public:
		virtual bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const;
		virtual bool BoundingBox(aabb& box) const;
		virtual std::unique_ptr<Pdf> GeneratePdfObject(const vec3& o);
		vec3 v[3];
		vec3 normal[3];
		vec3 face_normal;
};

class Quadrilateral : public Hittable {
	public:
		virtual bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const;
		virtual bool BoundingBox(aabb& box) const;
		virtual std::unique_ptr<Pdf> GeneratePdfObject(const vec3& o);
		vec3 v[4];
		vec3 normal;
};

#endif
