#ifndef RENDERER_H
#define RENDERER_H

#include <random>
#include <GL/gl3w.h>
#include "obj.h"
#include "object.h"
#include "camera.h"


enum RenderingAlgorithm {
	Naive,
	NEE,
	MIS
};

class NodeMaterial;
class Renderer {
	public:
		Renderer(void);
		void Load(const char *objfilename);
		void LoadMaterials(const std::vector<std::shared_ptr<NodeMaterial>>& materials);
		void Clear(void);
		void RenderImage(int nx, int ny, int ns, int spectral_samples, bool enable_openmp);
		double NaivePathTracing(const ray& r);
		double NEEPathTracingWithoutSpecular(const ray& r);
		double NEEMISPathTracing(const ray& r);
		enum RenderingAlgorithm algorithm_type = NEE;
		std::vector<double> orig_img;
		obj obj_loader;
		std::unique_ptr<ObjModel> world;
		std::vector<const ConvexPolygon *> light_objects;
		//LensCamera cam;
		PinholeCamera cam;
		bool img_updated = false;
		bool rendering_runnnig = false;
		bool stop_rendering = false;
	private:
		std::mt19937 mt;
};

#endif
