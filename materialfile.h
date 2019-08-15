#ifndef MATERIALFILE_H
#define MATERIALFILE_H

#include <fstream>
#include <map>
#include "spectrum.h"
#include "material.h"


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
void MicrofacetMaterialEditor(const std::shared_ptr<Microfacet>& mat_ptr);
void TransparentMaterialEditor(const std::shared_ptr<Transparent>& mat_ptr);
void LightMaterialEditor(const std::shared_ptr<DiffuseLight>& mat_ptr);

#endif
