#ifndef MATERIALFILE_H
#define MATERIALFILE_H

#include <fstream>
#include <map>
#include "spectrum.h"
#include "material.h"

class MaterialNode {
	public:
		virtual void Render(void) = 0;
};

class OutputMaterialNode : public MaterialNode {
	public:
		virtual void Render(void);
		MaterialNode *output;
};

class LightMaterialNode : public MaterialNode {
	public:
		LightMaterialNode(void) {
			light = Spectrum(1.0);
		}
		virtual void Render(void);
		Spectrum light;
};
class LambertianMaterialNode : public MaterialNode {
	public:
		LambertianMaterialNode(void) {
			albedo = RGBtoSpectrum(vec3(0.5, 1.0, 0.5));
		}
		virtual void Render(void);
		Spectrum albedo;
};
class DielectricMaterialNode : public MaterialNode {
	public:
		virtual void Render(void);
		Spectrum albedo;
};
class MetalMaterialNode : public MaterialNode {
	public:
		virtual void Render(void);
		Spectrum albedo;
};

class MixMaterialNode : public MaterialNode {
	public:
		virtual void Render(void);
		float ratio;
		MaterialNode *p1, *p2;
};

class MaterialEditor {
	public:
		MaterialEditor(void);
		void Render(void);
		OutputMaterialNode *output_node;
};


class MaterialLoader {
	public:
		bool Load(const char *filename);
		void Write(const char *filename);
		std::ifstream file;
		std::ofstream ofile;
		std::map<std::string, std::shared_ptr<material>> materials;
	private:
		bool LoadMaterial(void);
		void WriteMaterial(std::shared_ptr<material> mat);
};

#endif
