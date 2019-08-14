#ifndef OBJECT_H
#define OBJECT_H

#include "hittable.h"
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
			//this->mat_ptr = std::shared_ptr<Material>(mat_ptr);
			this->mat_ptr = mat_ptr;
		};
		bool Hit(const ray& r, double tmin, double tmux, HitRecord& rec) const override;
		bool BoundingBox(AABB& box) const override;
		std::unique_ptr<Pdf> GeneratePdfObject(const vec3& o) override;
		vec3 center;
		double radius;
};

class Plane: public Hittable {
	public:
		Plane() { }
		Plane(vec3 somewhere, vec3 normal, Material *mat_ptr) : somewhere(somewhere), normal(normal) {
			//this->mat_ptr = std::shared_ptr<Material>(mat_ptr);
			this->mat_ptr = mat_ptr;
		};
		bool Hit(const ray& r, double tmin, double tmux, HitRecord& rec) const override;
		vec3 somewhere;
		vec3 normal;
};

class Rectangle : public Hittable {
	public:
		Rectangle() { }
		Rectangle(vec3 center, vec3 normal, vec3 width_dir, double width, double height, Material *mat_ptr) : center(center), normal(normal), width_dir(width_dir), width(width), height(height) {
			//this->mat_ptr = std::shared_ptr<Material>(mat_ptr);
			this->mat_ptr = mat_ptr;
		};
		bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const override;
		std::unique_ptr<Pdf> GeneratePdfObject(const vec3& o) override;
		bool BoundingBox(AABB& box) const override;
		vec3 center;
		vec3 normal;
		vec3 width_dir;
		double width;
		double height;
};

class XYRect : public Hittable {
	public:
		XYRect() { }
		XYRect(double x0, double y0, double x1, double y1, double k, Material *mat) :
		x0(x0), x1(x1), y0(y0), y1(y1), k(k) {
			//mat_ptr = std::shared_ptr<Material>(mat);
			mat_ptr = mat;
		};
		bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const override;
		bool BoundingBox(AABB& box) const override;

		double x0, x1, y0, y1, k;
};

class YZRect : public Hittable {
	public:
		YZRect() { }
		YZRect(double y0, double z0, double y1, double z1, double k, Material *mat) :
		y0(y0), y1(y1), z0(z0), z1(z1), k(k) {
			//mat_ptr = std::shared_ptr<Material>(mat);
			mat_ptr = mat;
		};
		bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const override;
		bool BoundingBox(AABB& box) const override;

		double y0, y1, z0, z1, k;
};

class ZXRect : public Hittable {
	public:
		ZXRect() { }
		ZXRect(double z0, double x0, double z1, double x1, double k, Material *mat) :
		z0(z0), z1(z1), x0(x0), x1(x1), k(k) {
			//mat_ptr = std::shared_ptr<Material>(mat);
			mat_ptr = mat;
		};
		bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const override;
		bool BoundingBox(AABB& box) const override;
		//virtual double generate_Pdf_dir(const vec3& o, vec3& direction);

		double z0, z1, x0, x1, k;
};



class FlipNormals : public Hittable {
	public:
		FlipNormals(Hittable *p) : ptr(p) {}
		bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const override;
		bool BoundingBox(AABB& box) const override;
		Hittable *ptr;
};

class box : public Hittable {
	public:
		box() { }
		box(const vec3& p0, const vec3& p1, Material *mat);
		bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const override;
		bool BoundingBox(AABB& box) const override;

		vec3 pmin;
		vec3 pmax;
		Hittable *list_ptr;
};


class Translate : public Hittable {
	public:
		Translate(Hittable *p, const vec3& displacement) : ptr(p), offset(displacement) {}
		bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const override;
		bool BoundingBox(AABB& box) const override;

		Hittable *ptr;
		vec3 offset;
};


class ObjModel : public Hittable {
	public:
		ObjModel(obj& o);
		bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const override;
		bool BoundingBox(AABB& box) const override;
		//std::vector<std::vector<Hittable *>> models;
		std::vector<std::shared_ptr<Hittable> > models;
		std::shared_ptr<BVHNode> bvh;
};

class PlyModel : public Hittable {
	public:
		PlyModel(const char *filename, Material *mat);
		bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const override;
		bool BoundingBox(AABB& box) const override;
		ply p;
		std::vector<std::shared_ptr<Hittable> > polygon;
		BVHNode *pol;
};

class Triangle : public Hittable {
	public:
		bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const override;
		bool BoundingBox(AABB& box) const override;
		std::unique_ptr<Pdf> GeneratePdfObject(const vec3& o) override;
		vec3 v[3];
		vec3 normal[3];
		vec3 face_normal;
		vec3 vt[3];
};

class Quadrilateral : public Hittable {
	public:
		bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const override;
		bool BoundingBox(AABB& box) const override;
		std::unique_ptr<Pdf> GeneratePdfObject(const vec3& o) override;
		vec3 v[4];
		vec3 normal;
		vec3 vt[4];
};

#endif
