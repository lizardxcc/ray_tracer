#ifndef RENDERER_H
#define RENDERER_H

#include <random>
#include <map>
#include <GL/gl3w.h>
#include "obj.h"
#include "object.h"
#include "camera.h"


enum RenderingAlgorithm {
	Naive,
	NEE,
	MIS,
	NaiveBDPTType,
	MISBDPTType
};

class NodeMaterial;
class Renderer {
	public:
		Renderer(void);
		void Load(const char *objfilename);
		bool LoadMaterials(const std::map<std::string, std::shared_ptr<NodeMaterial>>& materials);
		void Clear(void);
		void RenderImage(int nx, int ny, int ns, int spectral_samples, bool enable_openmp, bool print_progress = false);
		double NaivePathTracing(const ray& r);
		double NEEPathTracingWithoutSpecular(const ray& r);
		double NEEMISPathTracing(const ray& r);
		double NaiveBDPT(const ray& r);
		double MISBDPT(const ray& r);
		enum RenderingAlgorithm algorithm_type = NaiveBDPTType;
		std::vector<Spectrum> spectrum_img;
		std::vector<dvec3> preview_img;
		obj obj_loader;
		std::unique_ptr<ObjModel> world;
		std::vector<const ConvexPolygon *> light_objects;
		LensCamera cam;
		//PinholeCamera cam;
		bool preview_img_flag = true;
		bool img_updated = false;
		bool rendering_runnnig = false;
		bool stop_rendering = false;
		float progress = 0.0;
	private:
		double GeometryTerm(double cos_theta0, double cos_theta1, double r);
		double GeometryTerm(const HitRecord& rec0, const HitRecord& rec1);
		std::mt19937 mt;
};

#endif
