#include <vector>
#include <algorithm>
#include "object.h"
#include "ray.h"
#include "hittablelist.h"
#include "pdf.h"

bool IsOccluded(const ray& r, const Hittable *world, const Hittable *p)
{
	HitRecord rec;
	if (world->Hit(r, 0.001, std::numeric_limits<double>::max(), rec)) {
		if (rec.hit_object == p)
			return true;
	}
	return false;
}

//std::unique_ptr<Pdf> Hittable::GeneratePdfObject(const dvec3& o)
//{
//	std::cout << "Warning GeneratePdfObject" << std::endl;
//	return std::make_unique<UniformPdf>(dvec3(0.0, 0.0, 1.0));
//}
std::unique_ptr<Pdf> Hittable::GeneratePdfObject(const dvec3& o)
{
	AABB box;
	if (BoundingBox(box)) {
		dvec3 v = box.center - o;
		double r = (box.center-box.minp).length();
		return std::make_unique<toward_object_Pdf>(unit_vector(v), atan2(r, v.length()));
	} else {
		std::cout << "Warning GeneratePdfObject" << std::endl;
		return nullptr;
	}
}


void Hittable::SetMaterial(NodeMaterial *mat)
{
	mat_ptr = mat;
}


bool Sphere::Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const
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
		rec.hit_object = this;
		rec.hit_object_id = object_id;
		return true;
	}
	double t2 = (-b_prime + sqrt(discriminant_prime)) / a;
	if (t2 >= t_min && t2 <= t_max) {
		rec.t = t2;
		rec.p = r.point_at_parameter(rec.t);
		rec.normal = unit_vector(rec.p - center);
		rec.mat_ptr = mat_ptr;
		rec.hit_object = this;
		rec.hit_object_id = object_id;
		return true;
	}

	return false;
}

bool Sphere::BoundingBox(AABB& box) const
{
	box = AABB(
		center - dvec3(radius, radius, radius),
		center + dvec3(radius, radius, radius)
	);
	return true;
}

std::unique_ptr<Pdf> Sphere::GeneratePdfObject(const dvec3& o)
{
	dvec3 v = center - o;
	double r = radius;
	return std::make_unique<toward_object_Pdf>(unit_vector(v), atan2(r, v.length()));
}


ObjModel::ObjModel(obj& o)
{
	models.resize(o.objects.size());
	for (size_t i = 0; i < o.objects.size(); i++) {
		const auto& object = o.objects[i];
		std::vector<std::shared_ptr<Hittable> > model;
		model.resize(object->faces.size());
		std::cout << "f: " << object->faces.size() << std::endl;

		std::vector<const ConvexPolygon *> polygons;
		for (size_t j = 0; j < o.objects[i]->faces.size(); j++) {
			const auto& face = o.objects[i]->faces[j];
			size_t l = face.size();
			std::shared_ptr<ConvexPolygon> pol = std::make_shared<ConvexPolygon>();
			pol->v.resize(l);
			pol->vt.resize(l);
			pol->normal.resize(l);
			for (size_t k = 0; k < l; k++) {
				pol->v[k] = o.v[*face[k][0]];
				if (face[k][1])
					pol->vt[k] = o.vt[*face[k][1]];
				pol->normal[k] = unit_vector(o.vn[*face[k][2]]);
			}
			pol->face_normal = unit_vector(cross(
						pol->v[1] - pol->v[0],
						pol->v[2] - pol->v[1]
						));
			for (size_t i = 0; i < pol->normal.size(); i++) {
				if (dot(pol->face_normal, pol->normal[i]) < 0.0) {
					std::cout << "Warning: The order of vertices may be incorrect" << std::endl;
					pol->face_normal *= -1.0;
				}
			}
			pol->mat_ptr = nullptr;
			pol->object_id = i;
			pol->CalcTriangleAreas();
			model[j] = pol;
			polygons.push_back(pol.get());
		}
		polygon_models.push_back(polygons);
		std::shared_ptr<Hittable> b = std::make_shared<SAHBVHNode>(model);
		models[i] = b;
	}

	auto v(models);
	bvh = std::make_shared<SAHBVHNode>(v);
}

bool ObjModel::Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const
{
	return bvh->Hit(r, t_min, t_max, rec);
}
bool ObjModel::BoundingBox(AABB& box) const
{
	box = bvh->box;

	return true;
}

/*
PlyModel::PlyModel(const char *filename, Material *mat)
{
	p.Load(filename);
	polygon.resize(p.faces.size());
	for (size_t i = 0; i < p.faces.size(); i++) {
		size_t l = p.faces[i].size();
		if (l != 3 && l != 4) {
			std::cerr << "Error: " << std::to_string(l) << " sided polygon is unsupported" << std::endl;
			return;

		}
		std::shared_ptr<Hittable> tmp_polygon;
		if (l == 3) {
			std::shared_ptr<Triangle> tri(new Triangle());
			for (size_t j = 0; j < l; j++) {
				tri->v[j] = p.vertices[p.faces[i][j]][0];
			}
			//tri->normal = p.vertices[p.faces[i][0]][1];
			//tri->mat_ptr = std::shared_ptr<Material>(mat);
			tri->mat_ptr = mat;
			tmp_polygon = tri;
		}
		else if (l == 4) {
			std::shared_ptr<Quadrilateral> quad(new Quadrilateral());
			for (size_t j = 0; j < l; j++) {
				quad->v[j] = p.vertices[p.faces[i][j]][0];
			}
			quad->normal = p.vertices[p.faces[i][0]][1];
			//quad->mat_ptr = std::shared_ptr<Material>(mat);
			quad->mat_ptr = mat;
			tmp_polygon = quad;

		}
		polygon[i] = tmp_polygon;
	}

	pol = new BVHNode(polygon);
}

bool PlyModel::Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const
{
	//bool hit_flag = false;
	//rec.t = t_max;
	//for (size_t i = 0; i < polygon.size(); i++) {
	//	if (polygon[i]->Hit(r, t_min, rec.t, rec)) {
	//		hit_flag = true;
	//	}
	//}

	//return hit_flag;
	//return pol.Hit(r, t_min, t_max, rec);
	return pol->Hit(r, t_min, t_max, rec);
}


bool PlyModel::BoundingBox(AABB& box) const
{
	AABB temp_box;
	if (polygon.size() == 0) {
		return false;
	}

	if (polygon[0]->BoundingBox(temp_box) == false) {
		return false;
	}

	box = temp_box;

	for (size_t i = 0; i < polygon.size(); i++) {
		if (polygon[i]->BoundingBox(temp_box)) {
			box = SurroundingBox(box, temp_box);
		} else {
			return false;
		}
	}

	return true;
}

*/


bool printed_warning = false;

bool ConvexPolygon::HitTriangle(const ray& r, double t_min, double t_max,
		const dvec3& v0, const dvec3& v1, const dvec3& v2,
		const dvec3& vt0, const dvec3& vt1, const dvec3& vt2,
		const dvec3& n0, const dvec3& n1, const dvec3& n2,
		const dvec3& face_normal, HitRecord& rec) const
{
	double t = dot(v0 - r.origin(), face_normal) / dot(r.direction(), face_normal);
	if (!(t >= t_min && t <= t_max)) {
		return false;
	}
	dvec3 p = r.point_at_parameter(t);
	double tri_area = cross(v1-v0, v2-v1).length();
	dvec3 result0 = cross(v1-v0, p-v1);
	dvec3 result1 = cross(v2-v1, p-v2);
	dvec3 result2 = cross(v0-v2, p-v0);
	if (dot(result0, result1) > 0.0 && dot(result1, result2) > 0.0) {
		rec.t = t;
		rec.p = p;
		rec.normal = (result1.length()*n0+result2.length()*n1+result0.length()*n2)/tri_area;

		rec.vt = (result1.length()*vt0+result2.length()*vt1+result0.length()*vt2)/tri_area;

		const dvec3 deltauv1 = vt1 - vt0;
		const dvec3 deltauv2 = vt2 - vt1;
		const dvec3 e1 = v1 - v0;
		const dvec3 e2 = v2 - v1;
		const double denom = deltauv1.x()*deltauv2.y() - deltauv1.y()*deltauv2.x();
		if (!printed_warning) {
			if (denom == 0.0) {
				std::cout << "Warning: an inverse matrix doesn't exit" << std::endl;
				printed_warning = true;
			}
		}
		const double fraction = 1.0/denom;
		const dvec3 t = ((deltauv2.y()*e1) - (deltauv1.y()*e2))*fraction;
		const dvec3 b = ((-deltauv2.x()*e1) + (deltauv1.x()*e2))*fraction;
		rec.tbn.axis[0] = unit_vector(t);
		rec.tbn.axis[1] = unit_vector(b);
		rec.tbn.axis[2] = unit_vector(cross(rec.tbn.axis[0], rec.tbn.axis[1]));
		//rec.tbn.axis[2] = unit_vector(cross(t, b));
		//rec.tbn.axis[2] = rec.normal;
		//rec.tbn.axis[2] = face_normal;

		return true;
	}
	return false;
}

bool ConvexPolygon::Hit(const ray& r, double t_min, double t_max, HitRecord& rec) const
{
	for (size_t i = 0; i < normal.size(); i++)
		assert(dot(face_normal, normal[i]) > 0.0);
	for (size_t i = 1; i < v.size()-1; i++) {
		bool hit = HitTriangle(r, t_min, t_max,
				v[0], v[i], v[i+1],
				vt[0], vt[i], vt[i+1],
				normal[0], normal[i], normal[i+1],
				face_normal,
				rec);
		if (hit) {
			rec.mat_ptr = mat_ptr;
			rec.hit_object = this;
			rec.hit_object_id = object_id;
			return true;
		}
	}
	return false;
}



bool ConvexPolygon::BoundingBox(AABB& box) const
{
	dvec3 minp = v[0];
	dvec3 maxp = v[0];
	for (size_t i = 1; i < v.size(); i++) {
		for (size_t j = 0; j < 3; j++) {
			minp.e[j] = std::min(minp.e[j], v[i][j]);
			maxp.e[j] = std::max(maxp.e[j], v[i][j]);
		}
	}

	for (size_t i = 0; i < 3; i++) {
		if (minp.e[i] == maxp.e[i]) {
			minp.e[i] -= 0.0001;
			maxp.e[i] += 0.0001;
		}
	}
	box = AABB(minp, maxp);
	return true;
}



std::unique_ptr<Pdf> ConvexPolygon::GeneratePdfObject(const dvec3& o)
{
	std::cout << "Error: Unimplemented" << std::endl;
	assert(false);
	return nullptr;
}


void ConvexPolygon::GetRandomPointOnPolygon(dvec3& p, double &area) const
{
	double r = drand48();
	for (size_t tri_i = 0; tri_i < triangle_area_cumulative_sums.size(); tri_i++) {
		bool a = false;
		if (tri_i == 0) {
			if (r < triangle_area_cumulative_sums[tri_i])
				a = true;
		} else {
			if (r >= triangle_area_cumulative_sums[tri_i-1] && r < triangle_area_cumulative_sums[tri_i])
				a = true;
		}
		if (a) {
			size_t v_i = tri_i+1;
			assert(v_i+1 < v.size());
			dvec3 va = v[v_i]-v[0];
			dvec3 vb = v[v_i+1]-v[0];

			double x = drand48();
			double y = drand48();
			if (x+y >= 1.0) {
				x = 1.0-x;
				y = 1.0-y;
			}
			p = v[0] + x*va + y*vb;
			area = polygon_area;
			return;
		}
	}
	std::cout << "GetRandomPointOnPolygon error" << std::endl;
	assert(false);
}


void ConvexPolygon::CalcTriangleAreas(void)
{
	polygon_area = 0.0;
	triangle_area_cumulative_sums.resize(v.size()-2);
	for (size_t v_i = 1; v_i < v.size()-1; v_i++) {
		const size_t tri_i = v_i-1;
		dvec3 a = v[v_i]-v[0];
		dvec3 b = v[v_i+1]-v[0];
		double tri_area = cross(a, b).length()/2.0;
		if (tri_i == 0)
			triangle_area_cumulative_sums[tri_i] = tri_area;
		else
			triangle_area_cumulative_sums[tri_i] = triangle_area_cumulative_sums[tri_i-1] + tri_area;
		polygon_area += tri_area;
	}
	for (auto& s : triangle_area_cumulative_sums) {
		s /= polygon_area;
	}
	triangle_area_cumulative_sums.back() = 1.0;
	//std::cout << "polygon area: " << polygon_area << std::endl;
}
