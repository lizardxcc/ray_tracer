#include <sstream>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "materialfile.h"
#include "material.h"
#include "scene.h"


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
	static float a[10];
	const ImVec2 slider_size(18, 160);
	for (int i = 0; i < 10; i++) {
		if (i > 0)
			ImGui::SameLine();
		ImGui::PushID(i);
		ImGui::VSliderFloat("##v", slider_size, &a[i], 0.0f, 1.0f, "");
		ImGui::PopID();
		albedo.data[i] = a[i];
	}
	vec3 col = r_rgb(albedo);
	ImVec4 color = ImVec4(col[0], col[1], col[2], 1.0f);
	ImGui::ColorButton("Albedo", color, ImGuiColorEditFlags_DisplayRGB);
	if (ImGui::Button("Apply")) {
		//if (activeObjectIndex != 0) {
		//	material *mat = new lambertian(albedo);
		//	world->models[activeObjectIndex-1]->set_material(mat);
		//}
	}
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
			std::cout << "Error: " << line << std::endl;
			return false;
		}
		oldpos = file.tellg();
	}

	file.close();

	return true;
}
bool MaterialLoader::LoadMaterial(void)
{
	//std::streampos oldpos = file.tellg();
	std::string line;

	material *mtl;

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
			mtl = new lambertian(albedo);
		} else if (line == "light") {
			Spectrum light;
			for (int i = 0; i < N_SAMPLE; i++) {
				if (getline(file, line)) {
					light.data[i] = stod(line);
				}
			}
			mtl = new diffuse_light(light);
			mtl->light_flag = true;
		} else {
			std::cout << "material " << line << " is not implemented" << std::endl;
			return false;
		}
	}
	materials[material_name] = std::shared_ptr<material>(mtl);
	return true;
}


void MaterialLoader::Write(const char *filename)
{
	ofile.open(filename);
	for (const auto& m : materials) {
		ofile << "newmtl " << m.first << std::endl;
		WriteMaterial(m.second);
	}
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
		} else if (id == typeid(diffuse_light)) {
			std::shared_ptr<diffuse_light> mat_ptr = std::dynamic_pointer_cast<diffuse_light>(mat);
			ofile << "light" << std::endl;
			for (const auto& d : mat_ptr->light_color.data) {
				ofile << d << std::endl;
			}
			ofile << std::endl;
		}
}



