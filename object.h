#ifndef OBJECT_H
#define OBJECT_H

#include "hittable.h"
#include "material.h"
#include "obj.h"
#include "mtl.h"
#include "ply.h"
#include "aabb.h"
#include "bvh.h"


bool IsOccluded(const ray& r, const Hittable *world, const Hittable *p);

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

class ConvexPolygon;

class ObjModel : public Hittable {
	public:
		ObjModel(obj& o);
		bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const override;
		bool BoundingBox(AABB& box) const override;
		//std::vector<std::vector<Hittable *>> models;
		std::vector<std::vector<const ConvexPolygon *> > polygon_models;
		std::vector<std::shared_ptr<Hittable> > models;
		std::shared_ptr<BVHNode> bvh;
};

//class PlyModel : public Hittable {
//	public:
//		PlyModel(const char *filename, Material *mat);
//		bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const override;
//		bool BoundingBox(AABB& box) const override;
//		ply p;
//		std::vector<std::shared_ptr<Hittable> > polygon;
//		BVHNode *pol;
//};

extern bool printed_warning;


class ConvexPolygon : public Hittable {
	public:
		bool Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const override;
		bool BoundingBox(AABB& box) const override;
		std::unique_ptr<Pdf> GeneratePdfObject(const vec3& o) override;
		void GetRandomPointOnPolygon(vec3& p, double& area) const;
		std::vector<vec3> v;
		std::vector<vec3> vt;
		std::vector<vec3> normal;
		vec3 face_normal;
		void CalcTriangleAreas(void);
		double polygon_area;
	private:
		bool HitTriangle(const ray& r, double t_min, double t_max,
				const vec3& v0, const vec3& v1, const vec3& v2,
				const vec3& vt0, const vec3& vt1, const vec3& vt2,
				const vec3& n0, const vec3& n1, const vec3& n2,
				const vec3& face_normal, HitRecord& rec) const;
		std::vector<double> triangle_area_cumulative_sums;
};


#endif
