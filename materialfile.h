#ifndef MaterialFILE_H
#define MaterialFILE_H

#include <fstream>
#include <map>
#include "spectrum.h"
#include "Material.h"

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
		void Clear(void);
		void Write(const char *filename);
		std::ifstream file;
		std::ofstream ofile;
		std::map<std::string, std::shared_ptr<Material>> Materials;
		std::vector<std::string> obj_mat_names;
	private:
		bool LoadMaterial(void);
		void WriteMaterial(std::shared_ptr<Material> mat);
};

void LambertianMaterialEditor(const std::shared_ptr<Lambertian>& mat_ptr);
void DielectricMaterialEditor(const std::shared_ptr<Dielectric>& mat_ptr);
void MetalMaterialEditor(const std::shared_ptr<Metal>& mat_ptr);
void MicrofacetMaterialEditor(const std::shared_ptr<TorranceSparrow>& mat_ptr);
void TransparentMaterialEditor(const std::shared_ptr<Transparent>& mat_ptr);
void LightMaterialEditor(const std::shared_ptr<DiffuseLight>& mat_ptr);

#endif
