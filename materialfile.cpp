#include <sstream>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "materialfile.h"
#include "material.h"
#include "scene.h"

void LambertianMaterialEditor(const std::shared_ptr<lambertian>& mat_ptr)
{
	ImGui::Text("lambertian");
	const ImVec2 slider_size(18, 160);
	for (int i = 0; i < N_SAMPLE; i++) {
		const double min = 0.0;
		const double max = 1.0;
		if (i > 0)
			ImGui::SameLine();
		ImGui::PushID(i);
		ImGui::VSliderScalar("##v", slider_size, ImGuiDataType_Double, &mat_ptr->albedo.data[i], &min, &max, "");
		if (ImGui::IsItemActive() || ImGui::IsItemHovered())
			ImGui::SetTooltip("%f", mat_ptr->albedo.data[i]);
		ImGui::PopID();
	}
}

void DielectricMaterialEditor(const std::shared_ptr<dielectric>& mat_ptr)
{
	ImGui::Text("dielectric");
	const ImVec2 slider_size(18, 160);
	for (int i = 0; i < N_SAMPLE; i++) {
		const double min = 1.0;
		const double max = 5.0;
		if (i > 0)
			ImGui::SameLine();
		ImGui::PushID(i);
		ImGui::VSliderScalar("##v", slider_size, ImGuiDataType_Double, &mat_ptr->n.data[i], &min, &max, "");
		if (ImGui::IsItemActive() || ImGui::IsItemHovered())
			ImGui::SetTooltip("%f", mat_ptr->n.data[i]);
		ImGui::PopID();
	}
}


void MetalMaterialEditor(const std::shared_ptr<metal>& mat_ptr)
{
	ImGui::Text("metal");
	const ImVec2 slider_size(18, 160);
	for (int i = 0; i < N_SAMPLE; i++) {
		const double min = 0.0;
		const double max = 1.0;
		if (i > 0)
			ImGui::SameLine();
		ImGui::PushID(i);
		ImGui::VSliderScalar("##v", slider_size, ImGuiDataType_Double, &mat_ptr->albedo.data[i], &min, &max, "");
		if (ImGui::IsItemActive() || ImGui::IsItemHovered())
			ImGui::SetTooltip("%f", mat_ptr->albedo.data[i]);
		ImGui::PopID();
	}
}


void MicrofacetMaterialEditor(const std::shared_ptr<torrance_sparrow>& mat_ptr)
{
	ImGui::Text("microfacet");
	const ImVec2 slider_size(18, 160);
	for (int i = 0; i < N_SAMPLE; i++) {
		const double min = 0.0;
		const double max = 1.0;
		if (i > 0)
			ImGui::SameLine();
		ImGui::PushID(i);
		ImGui::VSliderScalar("##v", slider_size, ImGuiDataType_Double, &mat_ptr->albedo.data[i], &min, &max, "");
		if (ImGui::IsItemActive() || ImGui::IsItemHovered())
			ImGui::SetTooltip("%f", mat_ptr->albedo.data[i]);
		ImGui::PopID();
	}
	{
		const double min = 0.0;
		const double max = 3.0;
		ImGui::SliderScalar("alpha", ImGuiDataType_Double, &mat_ptr->alpha, &min, &max, "%f");
	}
}


void TransparentMaterialEditor(const std::shared_ptr<transparent>& mat_ptr)
{
	ImGui::Text("transparent");
	{
		//const double min = 0.0;
		//const double max = 3.0;
		//ImGui::SliderScalar("sigma_t", ImGuiDataType_Double, &mat_ptr->sigma_t, &min, &max, "%f");
		//ImGui::SliderScalar("sigma_s", ImGuiDataType_Double, &mat_ptr->sigma_s, &min, &max, "%f");
	}
}


void LightMaterialEditor(const std::shared_ptr<diffuse_light>& mat_ptr)
{
	ImGui::Text("light");
	const ImVec2 slider_size(18, 160);
	for (int i = 0; i < N_SAMPLE; i++) {
		const double min = 0.0;
		const double max = 1.0;
		if (i > 0)
			ImGui::SameLine();
		ImGui::PushID(i);
		ImGui::VSliderScalar("##v", slider_size, ImGuiDataType_Double, &mat_ptr->light_color.data[i], &min, &max, "");
		if (ImGui::IsItemActive() || ImGui::IsItemHovered())
			ImGui::SetTooltip("%f", mat_ptr->light_color.data[i]);
		ImGui::PopID();
	}
}
void OutputMaterialNode::Render(void)
{
	if (output == nullptr) {
		ImGui::Text("Material is not selected");
	} else {
		output->Render();
	}
}
void LightMaterialNode::Render(void)
{
	ImGui::Text("Light is not implemented yet!");
}
void LambertianMaterialNode::Render(void)
{
	ImGui::Text("Lambertian");
}
void DielectricMaterialNode::Render(void)
{
	ImGui::Text("Dielectric is not implemented yet!");
}
void MetalMaterialNode::Render(void)
{
	ImGui::Text("Mix is not implemented yet!");
}
void MixMaterialNode::Render(void)
{
	ImGui::Text("Mix is not implemented yet!");
}

MaterialEditor::MaterialEditor(void) {
	output_node = new OutputMaterialNode;
	output_node->output = nullptr;
}

void MaterialEditor::Render(void)
{
	ImGui::Begin("Material Editor");
	const char *items[] = {"Lambertian", "Dielectric", "Metal"};
	//static const char *cur_item = items[0];

	static int cur_item = -1;
	ImGui::Combo("select material", &cur_item, items, sizeof(items)/sizeof(const char *));
	switch (cur_item) {
		case 0:
			if (output_node->output == nullptr)
				output_node->output = new LambertianMaterialNode();
			break;
		case 1:
			if (output_node->output == nullptr)
				output_node->output = new DielectricMaterialNode();
			break;
		case 2:
			if (output_node->output == nullptr)
				output_node->output = new MetalMaterialNode();
		default:
			break;
	}
	//if (ImGui::BeginCombo("select material", &cur_item, items, 2)) {
		//for (int i = 0; i < 2; i++) {
		//	if (cur_item == items[i]) {
		//	}
		//}
//		ImGui::EndCombo();
//	}
	output_node->Render();
	ImGui::End();
}


bool MaterialLoader::Load(const char *filename)
{
	file.open(filename);

	std::string line;
	std::streampos oldpos = file.tellg();

	while (getline(file, line)) {
		if (std::all_of(line.begin(), line.end(), isspace)) {
		} else if (line.find("#") == 0) {
		} else if (line.find("newmtl") == 0) {
			file.seekg(oldpos);
			LoadMaterial();
		} else {
			obj_mat_names.push_back(line);
		}
		oldpos = file.tellg();
	}

	file.close();

	return true;
}


void MaterialLoader::Clear(void)
{
	materials.clear();
	obj_mat_names.clear();
}



bool MaterialLoader::LoadMaterial(void)
{
	//std::streampos oldpos = file.tellg();
	std::string line;

	std::shared_ptr<material> mtl;

	getline(file, line);
	std::istringstream iss(line);
	std::string s, material_name;
	iss >> s;
	iss >> material_name;

	if (getline(file, line)) {
		if (line == "lambertian") {
			Spectrum albedo;
			for (int i = 0; i < N_SAMPLE; i++) {
				if (getline(file, line)) {
					albedo.data[i] = stod(line);
				}
			}
			mtl = std::make_shared<lambertian>(albedo);
		} else if (line == "dielectric") {
			Spectrum n;
			for (int i = 0; i < N_SAMPLE; i++) {
				if (getline(file, line)) {
					n.data[i] = stod(line);
				}
			}
			mtl = std::make_shared<dielectric>(n);
			mtl->specular_flag = true;
		} else if (line == "metal") {
			Spectrum albedo;
			for (int i = 0; i < N_SAMPLE; i++) {
				if (getline(file, line)) {
					albedo.data[i] = stod(line);
				}
			}
			mtl = std::make_shared<metal>(albedo);
			mtl->specular_flag = true;
		} else if (line == "microfacet") {
			Spectrum albedo;
			for (int i = 0; i < N_SAMPLE; i++) {
				if (getline(file, line)) {
					albedo.data[i] = stod(line);
				}
			}
			double alpha;
			if (getline(file, line)) {
				alpha = stod(line);
			}
			mtl = std::make_shared<torrance_sparrow>(albedo, alpha);
		} else if (line == "transparent") {
			mtl = std::make_shared<transparent>();
		} else if (line == "light") {
			Spectrum light;
			for (int i = 0; i < N_SAMPLE; i++) {
				if (getline(file, line)) {
					light.data[i] = stod(line);
				}
			}
			mtl = std::make_shared<diffuse_light>(light);
			mtl->light_flag = true;
		} else {
			std::cout << "material " << line << " is not implemented" << std::endl;
			return false;
		}
	}
	materials[material_name] = mtl;
	return true;
}


void MaterialLoader::Write(const char *filename)
{
	ofile.open(filename);
	for (const auto& n : obj_mat_names)
		ofile << n << std::endl;
	for (const auto& m : materials) {
		ofile << "newmtl " << m.first << std::endl;
		WriteMaterial(m.second);
	}
	ofile.close();
}

void MaterialLoader::WriteMaterial(std::shared_ptr<material> mat)
{
		auto& id = typeid(*mat);
		if (id == typeid(lambertian)) {
			std::shared_ptr<lambertian> mat_ptr = std::dynamic_pointer_cast<lambertian>(mat);
			ofile << "lambertian" << std::endl;
			for (const auto& d : mat_ptr->albedo.data) {
				ofile << d << std::endl;
			}
			ofile << std::endl;
		} else if (id == typeid(dielectric)) {
			std::shared_ptr<dielectric> mat_ptr = std::dynamic_pointer_cast<dielectric>(mat);
			ofile << "dielectric" << std::endl;
			for (const auto& d : mat_ptr->n.data) {
				ofile << d << std::endl;
			}
			ofile << std::endl;
		} else if (id == typeid(metal)) {
			std::shared_ptr<metal> mat_ptr = std::dynamic_pointer_cast<metal>(mat);
			ofile << "metal" << std::endl;
			for (const auto& d : mat_ptr->albedo.data) {
				ofile << d << std::endl;
			}
			ofile << std::endl;
		} else if (id == typeid(torrance_sparrow)) {
			std::shared_ptr<torrance_sparrow> mat_ptr = std::dynamic_pointer_cast<torrance_sparrow>(mat);
			ofile << "microfacet" << std::endl;
			for (const auto& d : mat_ptr->albedo.data) {
				ofile << d << std::endl;
			}
			ofile << mat_ptr->alpha << std::endl;
			ofile << std::endl;
		} else if (id == typeid(transparent)) {
			std::shared_ptr<transparent> mat_ptr = std::dynamic_pointer_cast<transparent>(mat);
			ofile << "transparent" << std::endl;
			ofile << std::endl;
		} else if (id == typeid(diffuse_light)) {
			std::shared_ptr<diffuse_light> mat_ptr = std::dynamic_pointer_cast<diffuse_light>(mat);
			ofile << "light" << std::endl;
			for (const auto& d : mat_ptr->light_color.data) {
				ofile << d << std::endl;
			}
			ofile << std::endl;
		}
}



