#ifndef RENDERER_H
#define RENDERER_H

#include <GL/gl3w.h>
#include "obj.h"
#include "object.h"
#include "camera.h"
#include "Materialfile.h"

class Renderer {
	public:
		//Renderer(const char *filename);
		void Load(const char *objfilename, const char *matfilename);
		void Clear(void);
		void RenderImage(int nx, int ny, int ns);
		double NaivePathTracing(const ray& r);
		double NEEPathTracing(const ray& r, bool enableNEE);
		double NEEVolPathTracing(const ray& r, bool enableNEE);
		double GetRadiance(ray& r, int count);
		std::shared_ptr<double[]> orig_img;
		//GLubyte *img = nullptr;
		obj obj_loader;
		MaterialLoader Material_loader;
		std::unique_ptr<ObjModel> world;
		//std::vector<std::shared_ptr<Material> > Materials;
		lens_camera cam;
		bool img_updated = false;
};

#endif
