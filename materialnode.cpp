#include <boost/filesystem/operations.hpp>
#include "materialnode.h"
#include "material.h"
#include "pdf.h"
#include "stb_image.h"
#ifndef _CLI
#include <nfd.h>
#endif

#ifndef _CLI
/*
const ImVec4 MaterialNode::pin_colors[] = {
	ImVec4(1.0f, 0.2f, 0.2f, 1.0f),
	ImVec4(0.2f, 1.0f, 0.2f, 1.0f),
	ImVec4(0.4f, 0.4f, 1.0f, 1.0f),
	ImVec4(1.0f, 1.0f, 1.0f, 1.0f),
	ImVec4(0.7f, 0.8f, 0.5f, 1.0f)
};

LinkInfo::LinkInfo(int &unique_id, ed::PinId input_id, ed::PinId output_id, PinInfo *input, PinInfo *output) :
	id(unique_id),
	iid(unique_id),
	input_id(input_id),
	output_id(output_id),
	input(input),
	output(output)
{
	unique_id++;
}
*/
#endif

DEFINE_ENUM_FUNCTION(SlotType, SLOTTYPE_SEQ)
DEFINE_ENUM_FUNCTION(MaterialNodeType, MATNODETYPE_SEQ)



MaterialNode::MaterialNode(const char *name) : name(name)
{
}
MaterialNode::MaterialNode(const json& j) : name(j["name"].get<std::string>())
{
	for (const auto& input_j : j["input_slots"]) {
		input_slots.push_back({strdup(input_j["name"].get<std::string>().c_str()),
		static_cast<int>(StringToSlotType(input_j["type"]))});
	}
	for (const auto& output_j : j["output_slots"]) {
		output_slots.push_back({strdup(output_j["name"].get<std::string>().c_str()),
		static_cast<int>(StringToSlotType(output_j["type"]))});
	}
	pos.x = j["pos"]["x"];
	pos.y = j["pos"]["y"];
}



void MaterialNode::DeleteConnection(const Connection& connection)
{
	for (auto it = connections.begin(); it != connections.end(); it++) {
		if (*it == connection) {
			connections.erase(it);
			break;
		}
	}
}
const MaterialNode *MaterialNode::GetInputParentNode(const char *slot_name) const
{
	for (const auto& c: connections) {
		if (strncmp(c.input_slot, slot_name, strlen(slot_name)) == 0) {
			return c.output_node;
		}
	}
	return nullptr;
}
void MaterialNode::Compute(const Argument& global_arg, double& data) const
{
}
void MaterialNode::Compute(const Argument& global_arg, Spectrum& data) const
{
}
void MaterialNode::Compute(const Argument& global_arg, dvec3& data) const
{
}

void MaterialNode::DumpJson(json& j) const
{
	DumpIO(j);
}

void MaterialNode::DumpIO(json& j) const
{
	j["name"] = name;
	j["type"] = MaterialNodeTypeToString(type);
	j["input_slots"] = json::array();
	j["output_slots"] = json::array();
	for (size_t i = 0; i < input_slots.size(); i++) {
		j["input_slots"][i]["name"] = input_slots[i].title;
		j["input_slots"][i]["type"] = SlotTypeToString(static_cast<enum SlotType>(input_slots[i].kind));
	}
	for (size_t i = 0; i < output_slots.size(); i++) {
		j["output_slots"][i]["name"] = output_slots[i].title;
		j["output_slots"][i]["type"] = SlotTypeToString(static_cast<enum SlotType>(output_slots[i].kind));
	}
	j["pos"]["x"] = pos.x;
	j["pos"]["y"] = pos.y;
};
void MaterialNode::DumpSpectrum(json& j, const Spectrum& s, const char *name) const
{
	for (const auto& d : s.data) {
		j[name].push_back(d);
	}
}

//void MaterialNode::UpdateNormal(const PinInfo *normal_pin, const HitRecord& rec, dvec3& new_normal) const
//{
//	new_normal = rec.normal;
//	/*
//	if (!normal_pin->connected_links.empty()) {
//		if (normal_pin->connected_links.size() != 1) {
//			std::cout << "node connection error" << std::endl;
//			return;
//		}
//		const PinInfo *connected_pin = normal_pin->connected_links[0]->input;
//		assert(connected_pin != nullptr);
//		const MaterialNode *parent = connected_pin->parent_node;
//		dvec3 normal;
//		Argument global_arg = { rec.vt };
//		parent->Compute(global_arg, normal);
//		normal = unit_vector(normal*2.0-dvec3(1.0, 1.0, 1.0));
//		new_normal = rec.tbn.LocalToWorld(normal);
//
//		if (dot(rec.normal, rec.tbn.axis[2]) < 0.0) {
//			std::cout << "Warning 0: UV mapping may be incorrect" << std::endl;
//		}
//		//if (dot(face_normal, rec.tbn.axis[2]) < 0.0) {
//		//	std::cout << "Warning 1: UV mapping may be incorrect" << std::endl;
//		//}
//	}
//	*/
//};

/*
void NodeMaterial::AddLink(PinInfo *input, PinInfo *output)
{
	assert(input != nullptr);
	assert(output != nullptr);
#ifndef _CLI
	//struct LinkInfo *new_link = new LinkInfo(unique_id, input->id, output->id, input, output);
	//links.push_back(new_link);
	//input->connected_links.push_back(new_link);
	//output->connected_links.push_back(new_link);
#endif
}
*/

void MaterialNode::RenderSpectrum(Spectrum &data, double min, double max)
{
#ifndef _CLI
	const ImVec2 slider_size(14, 100);
	for (int i = 0; i < N_SAMPLE; i++) {
		if (i > 0)
			ImGui::SameLine();
		ImGui::PushID(i);
		ImGui::VSliderScalar("##v", slider_size, ImGuiDataType_Double, &data.data[i], &min, &max, "");
		if (ImGui::IsItemActive() || ImGui::IsItemHovered())
			ImGui::SetTooltip("%f", data.data[i]);
		ImGui::PopID();
	}
#endif
}

void MaterialNode::RenderNode(void)
{
#ifndef _CLI
#endif
}


void MaterialNode::RenderEditor(void)
{
#ifndef _CLI
	/*
	ImGui::Text("%s", name.c_str());
	*/
#endif
}


void MaterialNode::AddInput(SlotType type, const char *name)
{
	input_slots.push_back({name, type});
}


void MaterialNode::AddOutput(SlotType type, const char *name)
{
	output_slots.push_back({name, type});
}

void BSDFMaterialNode::PreProcess(const Argument& global_arg, HitRecord &rec) const
{
}
bool BSDFMaterialNode::SampleBSDF(const Argument& global_arg, RayType type, const HitRecord& rec, const ONB& uvw, const dvec3& vo, double wlo, dvec3& vi, double& wli, double& BSDF_divided_by_pdf, double& BSDF, double& pdfval) const
{
	return false;
}
double BSDFMaterialNode::BSDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const
{
	return 0.0;
}
double BSDFMaterialNode::PDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const
{
	return 0.0;
}
double BSDFMaterialNode::Emitted(const Argument& global_arg, const ray& r, const HitRecord& rec) const
{
	return 0.0;
}
LambertianNode::LambertianNode(const char *name) : MaterialNode(name)
{
	type = MaterialNodeType::Lambertian;
	AddInput(SlotSpectrum, "albedo");
	AddInput(SlotVec3, "normal");
	AddOutput(SlotBSDF, "BSDF");

	//albedo_pin = &inputs[0];
	//normal_pin = &inputs[1];
}
LambertianNode::LambertianNode(const json& j) : MaterialNode(j)
{
	type = MaterialNodeType::Lambertian;
	//albedo_pin = &inputs[0];
	//normal_pin = &inputs[1];
	for (size_t i = 0; i < j["albedo"].size(); i++) {
		albedo.data[i] = j["albedo"][i];
	}
}
void LambertianNode::DumpJson(json& j) const
{
	for (const auto& d : albedo.data) {
		j["albedo"].push_back(d);
	}
	DumpIO(j);
}

void LambertianNode::RenderEditor(void)
{
#ifndef _CLI
	ImGui::PushID(this);
	if (GetInputParentNode("albedo") == nullptr)
		RenderSpectrum(albedo, 0.0, 1.0);

	ImGui::PopID();
#endif
}


void LambertianNode::PreProcess(const Argument& global_arg, HitRecord& rec) const
{
}

bool LambertianNode::SampleBSDF(const Argument& global_arg, RayType type, const HitRecord& rec, const ONB& uvw, const dvec3& vo, double wlo, dvec3& vi, double& wli, double& bxdf_divided_by_pdf, double& BSDF, double& pdfval) const
{
	//HitRecord new_rec = rec;
	//UpdateNormal(normal_pin, rec, new_rec.normal);
	//CosinePdf Pdf(new_rec.normal);
	CosinePdf Pdf(rec.normal);

	dvec3 generated_direction = Pdf.Generate();
	pdfval = Pdf.PdfVal(generated_direction);
	vi = uvw.WorldToLocal(generated_direction);
	wli = wlo;
	BSDF = this->BSDF(global_arg, vi, wli, vo, wlo);
	bxdf_divided_by_pdf = BSDF / pdfval;
	return true;
}

double LambertianNode::BSDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const
{
	if (vi.z() < 0.0)
		return 0.0;
	const MaterialNode *parent = GetInputParentNode("albedo");
	if (parent == nullptr)
		return albedo.get(wli)/M_PI;
	Spectrum albedo;
	parent->Compute(global_arg, albedo);
	return albedo.get(wli)/M_PI;
}
double LambertianNode::PDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const
{
	if (vi.z() < 0.0)
		return 0.0;
	double cos_theta = vi.z();
	return cos_theta/M_PI;
}
DielectricNode::DielectricNode(const char *name) : MaterialNode(name)
{
	type = MaterialNodeType::Dielectric;
	AddInput(SlotSpectrum, "n");
	AddInput(SlotSpectrum, "surface color");
	AddInput(SlotVec3, "normal");
	AddOutput(SlotBSDF, "BSDF");
	//normal_pin = &inputs[2];
}
DielectricNode::DielectricNode(const json& j) : MaterialNode(j)
{
	type = MaterialNodeType::Dielectric;
	//normal_pin = &inputs[2];

	for (size_t i = 0; i < j["n"].size(); i++) {
		n.data[i] = j["n"][i];
	}
	for (size_t i = 0; i < j["surface_color"].size(); i++) {
		surface_color.data[i] = j["surface_color"][i];
	}
}
void DielectricNode::DumpJson(json& j) const
{
	for (const auto& d : n.data) {
		j["n"].push_back(d);
	}
	for (const auto& d : surface_color.data) {
		j["surface_color"].push_back(d);
	}
	DumpIO(j);
}

void DielectricNode::RenderEditor(void)
{
#ifndef _CLI
	ImGui::PushID(this);
	ImGui::Text("%s", name.c_str());
	ImGui::PushID(0);
	ImGui::Text("n");
	RenderSpectrum(n, 0.0, 5.0);
	ImGui::PopID();

	if (GetInputParentNode("surface_color") == nullptr) {
		ImGui::PushID(1);
		ImGui::Text("surface color");
		RenderSpectrum(surface_color, 0.0, 1.0);
		ImGui::PopID();
	}
	ImGui::PopID();
#endif
}
void DielectricNode::PreProcess(const Argument& global_arg, HitRecord& rec) const
{
	dvec3 new_normal;
	//UpdateNormal(normal_pin, rec, new_normal);
	rec.normal = new_normal;
}

bool DielectricNode::SampleBSDF(const Argument& global_arg, RayType type, const HitRecord& rec, const ONB& uvw, const dvec3& vo, double wlo, dvec3& vi, double& wli, double& bxdf_divided_by_pdf, double& BSDF, double& pdfval) const
{
	wli = wlo;
	double ref_idx = n.get(wlo);
	double surface_scale;
	const MaterialNode *node = GetInputParentNode("surface_color");
	if (node == nullptr)
		surface_scale = surface_color.get(wlo);
	else {
		Spectrum surface_color;
		node->Compute(global_arg, surface_color);
		surface_scale = surface_color.get(wlo);
	}

	double cos_o = vo.z();
	double n_vacuum = 1.0;
	dvec3 normal = dvec3(0, 0, 1);
	double n1, n2;
	if (cos_o >= 0.0) {
		n1 = n_vacuum;
		n2 = ref_idx;
	} else {
		normal = dvec3(0, 0, -1);
		n1 = ref_idx;
		n2 = n_vacuum;
		cos_o = abs(cos_o);
	}
	double relative_ref_idx = n2/n1;
	const double sin_o = sqrt(std::max(0.0, 1.0-cos_o*cos_o));
	const double fresnel = rfresnel(cos_o, relative_ref_idx);

	double r = drand48();
	pdfval = std::numeric_limits<double>::infinity();
	BSDF = std::numeric_limits<double>::infinity();
	if (r <= fresnel) {
		vi[0] = -vo[0];
		vi[1] = -vo[1];
		vi[2] = vo[2];
		bxdf_divided_by_pdf = surface_scale/cos_o;
	} else {
		double sin_o = sqrt(1.0-cos_o*cos_o);
		double sin_t = sin_o/relative_ref_idx;
		double cos_t = sqrt(1.0-sin_t*sin_t);
		vi = (-cos_t)*normal - sin_t*unit_vector(dvec3(vo[0], vo[1], 0));
		//bxdf_divided_by_pdf = surface_scale/pow(relative_ref_idx, 3) / cos_t;
		if (type == LightType) {
			bxdf_divided_by_pdf = surface_scale*pow(relative_ref_idx, 3) / cos_t;
		} else if (type == ImportanceType) {
			bxdf_divided_by_pdf = surface_scale/pow(relative_ref_idx, 3) / cos_t;
		}
	}

	return true;
}
double DielectricNode::BSDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const
{
	//if (-vi[0] == vo[0] && -vi[1] == vo[1] && vi[2] == vo[2])
	//	return std::numeric_limits<double>::infinity();
	return 0.0;
}
double DielectricNode::PDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const
{
	//if (-vi[0] == vo[0] && -vi[1] == vo[1] && vi[2] == vo[2])
	//	return std::numeric_limits<double>::infinity();
	return 0.0;
}
ConductorNode::ConductorNode(const char *name) : MaterialNode(name)
{
	type = MaterialNodeType::Conductor;
	AddInput(SlotSpectrum, "n");
	AddInput(SlotSpectrum, "k");
	AddInput(SlotVec3, "normal");
	AddOutput(SlotBSDF, "BSDF");
	//normal_pin = &inputs[2];
}
ConductorNode::ConductorNode(const json& j) : MaterialNode(j)
{
	type = MaterialNodeType::Conductor;
	//normal_pin = &inputs[2];

	for (size_t i = 0; i < j["n"].size(); i++) {
		n.data[i] = j["n"][i];
	}
	for (size_t i = 0; i < j["k"].size(); i++) {
		k.data[i] = j["k"][i];
	}
}
void ConductorNode::DumpJson(json& j) const
{
	for (const auto& d : n.data) {
		j["n"].push_back(d);
	}
	for (const auto& d : k.data) {
		j["k"].push_back(d);
	}
	DumpIO(j);
}

void ConductorNode::RenderEditor(void)
{
#ifndef _CLI
	ImGui::PushID(this);
	ImGui::Text("Conductor");
	ImGui::Text("%s", name.c_str());
	ImGui::PushID(0);
	ImGui::Text("n");
	RenderSpectrum(n, 0.0, 10.0);
	ImGui::PopID();
	ImGui::PushID(1);
	ImGui::Text("k");
	RenderSpectrum(k, 0.0, 10.0);
	ImGui::PopID();
	ImGui::PopID();
#endif
}
void ConductorNode::PreProcess(const Argument& global_arg, HitRecord& rec) const
{
}

bool ConductorNode::SampleBSDF(const Argument& global_arg, RayType type, const HitRecord& rec, const ONB& uvw, const dvec3& vo, double wlo, dvec3& vi, double& wli, double& bxdf_divided_by_pdf, double& BSDF, double& pdfval) const
{
	wli = wlo;
	double ref_idx = n.get(wlo);
	double kv = k.get(wlo);

	double cos_o = vo.z();
	double n_vacuum = 1.0;
	std::complex<double> n1, n2;
	if (cos_o >= 0.0) {
		n1 = n_vacuum;
		n2 = std::complex<double>(ref_idx, kv);
	} else {
		n1 = std::complex<double>(ref_idx, kv);
		n2 = n_vacuum;
		cos_o = abs(cos_o);
	}
	std::complex<double> relative_ref_idx = n2/n1;
	const double sin_o = sqrt(std::max(0.0, 1.0-cos_o*cos_o));
	const double fresnel = cfresnel(cos_o, relative_ref_idx);

	vi[0] = -vo[0];
	vi[1] = -vo[1];
	vi[2] = vo[2];
	pdfval = std::numeric_limits<double>::infinity();
	BSDF = std::numeric_limits<double>::infinity();
	bxdf_divided_by_pdf = fresnel/cos_o;

	return true;
}
double ConductorNode::BSDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const
{
	if (-vi[0] == vo[0] && -vi[1] == vo[1] && vi[2] == vo[2])
		return std::numeric_limits<double>::infinity();
	return 0.0;
}
double ConductorNode::PDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const
{
	if (-vi[0] == vo[0] && -vi[1] == vo[1] && vi[2] == vo[2])
		return std::numeric_limits<double>::infinity();
	return 0.0;
}

ColoredMetalNode::ColoredMetalNode(const char *name) : MaterialNode(name)
{
	type = MaterialNodeType::ColoredMetal;
	AddInput(SlotSpectrum, "albedo");
	AddInput(SlotVec3, "normal");
	AddOutput(SlotBSDF, "BSDF");
	//albedo_pin = &inputs[0];
	//normal_pin = &inputs[1];
}

ColoredMetalNode::ColoredMetalNode(const json& j) : MaterialNode(j)
{
	type = MaterialNodeType::ColoredMetal;
	//albedo_pin = &inputs[0];
	//normal_pin = &inputs[1];

	for (size_t i = 0; i < j["albedo"].size(); i++) {
		albedo.data[i] = j["albedo"][i];
	}
}
void ColoredMetalNode::DumpJson(json& j) const
{
	for (const auto& d : albedo.data) {
		j["albedo"].push_back(d);
	}
	DumpIO(j);
}

void ColoredMetalNode::RenderEditor(void)
{
#ifndef _CLI
	ImGui::PushID(this);
	ImGui::Text("%s", name.c_str());

	if (GetInputParentNode("albedo") == nullptr) {
		ImGui::Text("albedo");
		RenderSpectrum(albedo, 0.0, 1.0);
	}
	ImGui::PopID();
#endif
}
void ColoredMetalNode::PreProcess(const Argument& global_arg, HitRecord& rec) const
{
	dvec3 new_normal;
	//UpdateNormal(normal_pin, rec, new_normal);
	rec.normal = new_normal;
}

bool ColoredMetalNode::SampleBSDF(const Argument& global_arg, RayType type, const HitRecord& rec, const ONB& uvw, const dvec3& vo, double wlo, dvec3& vi, double& wli, double& bxdf_divided_by_pdf, double& BSDF, double& pdfval) const
{
	wli = wlo;

	vi[0] = -vo[0];
	vi[1] = -vo[1];
	vi[2] = vo[2];

	BSDF = std::numeric_limits<double>::infinity();
	pdfval = std::numeric_limits<double>::infinity();
	const MaterialNode *node = GetInputParentNode("albedo");
	if (node == nullptr) {
		bxdf_divided_by_pdf = albedo.get(wli)/abs(vo.z());
	} else {
		Spectrum albedo;
		node->Compute(global_arg, albedo);
		bxdf_divided_by_pdf = albedo.get(wli)/abs(vo.z());
	}

	return true;
}
double ColoredMetalNode::BSDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const
{
	if (-vi[0] == vo[0] && -vi[1] == vo[1] && vi[2] == vo[2])
		return std::numeric_limits<double>::infinity();
	return 0.0;
}
double ColoredMetalNode::PDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const
{
	if (-vi[0] == vo[0] && -vi[1] == vo[1] && vi[2] == vo[2])
		return std::numeric_limits<double>::infinity();
	return 0.0;
}


GGXReflectionNode::GGXReflectionNode(const char *name) : MaterialNode(name)
{
	type = MaterialNodeType::GGXReflection;
	AddInput(SlotSpectrum, "n");
	AddInput(SlotSpectrum, "k");
	AddInput(SlotDouble, "roughness");
	AddOutput(SlotBSDF, "BSDF");
	//n_pin = &inputs[0];
	//k_pin = &inputs[1];
}
GGXReflectionNode::GGXReflectionNode(const json& j) : MaterialNode(j)
{
	type = MaterialNodeType::GGXReflection;
	for (size_t i = 0; i < j["n"]; i++) {
		n.data[i] = j["n"][i];
	}
	for (size_t i = 0; i < j["k"]; i++) {
		k.data[i] = j["k"][i];
	}
}

void GGXReflectionNode::DumpJson(json& j) const
{
	DumpSpectrum(j, n, "n");
	DumpSpectrum(j, k, "k");
	DumpIO(j);
}

void GGXReflectionNode::RenderEditor(void)
{
#ifndef _CLI
	ImGui::PushID(this);
	ImGui::Text("%s", name.c_str());
	if (GetInputParentNode("n") == nullptr) {
		ImGui::Text("n");
		RenderSpectrum(n, 0.0, 1.0);
	}
	if (GetInputParentNode("k") == nullptr) {
		ImGui::Text("k");
		RenderSpectrum(n, 0.0, 1.0);
	}
	ImGui::PopID();
#endif
}

void GGXReflectionNode::PreProcess(const Argument& global_arg, HitRecord& rec) const
{
}
bool GGXReflectionNode::SampleBSDF(const Argument& global_arg, RayType type, const HitRecord& rec, const ONB& uvw, const dvec3& vo, double wlo, dvec3& vi, double& wli, double& bxdf_divided_by_pdf, double& BSDF, double& pdfval) const
{
	double phi = 2 * M_PI * drand48();
	double u = drand48();
	double theta = acos(sqrt((1.0-u)/((alpha*alpha-1.0)*u+1)));
	double tan_theta = tan(theta);
	double cos_theta_squared = cos(theta)*cos(theta);
	double sin_theta = sin(theta);

	dvec3 micro_normal;
	micro_normal[0] = sin_theta * cos(phi);
	micro_normal[1] = sin_theta * sin(phi);
	micro_normal[2] = sqrt(cos_theta_squared);


	double ref_idx = n.get(wlo);
	double kv = k.get(wlo);

	double fresnel = 1.0;
	double g = 1.0;


	double d;
	{
		double x = dot(micro_normal, rec.normal);
		if (x < 0.0)
			x = 0.0;
		d = alpha*alpha * x / (M_PI * cos_theta_squared*cos_theta_squared * pow(alpha*alpha + tan_theta*tan_theta, 2));
	}
	double pm = d * abs(dot(micro_normal, rec.normal));
	double pi = pm / (4.0 * abs(dot(micro_normal, rec.normal)));

	vi = reflect2(vo, micro_normal);
	wli = wlo;
	pdfval = pi;
	BSDF = fresnel*g*d/(4.0 * abs(vi.z()*vo.z()));
	//double g = 1.0/(1.0+lambda(vi)) * 1.0/(1.0+lambda(vo));
	//BSDF = fresnel * g * abs(dot(vo, micro_normal));
	//bxdf_divided_by_pdf = fresnel * g * abs(dot(vo, micro_normal)) / (abs(vo.z()*vi.z()*micro_normal.z()));
	bxdf_divided_by_pdf = fresnel * g * abs(dot(vo, micro_normal)) / (abs(vo.z()*vi.z()*micro_normal.z()));

	return true;
}
double GGXReflectionNode::BSDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const
{
}
double GGXReflectionNode::PDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const
{
}

DiffuseLightNode::DiffuseLightNode(const char *name) : MaterialNode(name)
{
	type = MaterialNodeType::DiffuseLight;
	AddInput(SlotSpectrum, "color");
	AddInput(SlotVec3, "normal");
	AddOutput(SlotBSDF, "BSDF");
	//color_pin = &inputs[0];
	//normal_pin = &inputs[1];
}

DiffuseLightNode::DiffuseLightNode(const json& j) : MaterialNode(j)
{
	type = MaterialNodeType::DiffuseLight;
	for (size_t i = 0; i < j["color"].size(); i++) {
		color.data[i] = j["color"][i];
	}
	//color_pin = &inputs[0];
	//normal_pin = &inputs[1];
}
void DiffuseLightNode::DumpJson(json& j) const
{
	DumpSpectrum(j, color, "color");
	DumpIO(j);
}
void DiffuseLightNode::RenderEditor(void)
{
#ifndef _CLI
	ImGui::PushID(this);
	ImGui::Text("%s", name.c_str());
	if (GetInputParentNode("color") == nullptr) {
		ImGui::Text("color");
		RenderSpectrum(color, 0.0, 1.0);
	}
	ImGui::PopID();
#endif
}
void DiffuseLightNode::PreProcess(const Argument& global_arg, HitRecord &rec) const
{
	dvec3 new_normal;
	//UpdateNormal(normal_pin, rec, new_normal);
	rec.normal = new_normal;
}
double DiffuseLightNode::Emitted(const Argument& global_arg, const ray& r, const HitRecord& rec) const
{
	const MaterialNode *node = GetInputParentNode("color");
	if (node == nullptr)
		return color.integrate(r.min_wl, r.max_wl);
	else {
		Spectrum color;
		node->Compute(global_arg, color);
		return color.integrate(r.min_wl, r.max_wl);
	}
}

/*
MixBSDFNode::MixBSDFNode(int &unique_id, const char *name) : MaterialNode(unique_id, name)
{
	type = MixBSDFType;
	AddInput(unique_id, SlotBSDF, "->In0");
	AddInput(unique_id, SlotBSDF, "->In1");
	AddOutput(unique_id, SlotBSDF, "Out->");
}
MixBSDFNode::MixBSDFNode(const json& j) : MaterialNode(j)
{
	type = MixBSDFType;
	ratio = j["ratio"];
}
void MixBSDFNode::DumpJson(json& j) const
{
	DumpIO(j);
	j["ratio"] = ratio;
}

void MixBSDFNode::PreProcess(HitRecord& rec) const
{
	size_t i;
	if (drand48() < ratio) {
		i = 0;
	} else {
		i = 1;
	}
	assert(inputs[i].connected_links.size() == 1);
	if (inputs[i].connected_links[0]->input->type != SlotBSDF) {
		std::cout << "Error" << std::endl;
	}
	dynamic_cast<const Material *>(inputs[i].connected_links[0]->input->parent_node)->PreProcess(rec);
}

bool MixBSDFNode::SampleBSDF(const HitRecord& rec, const ONB& uvw, const dvec3& vo, double wlo, dvec3& vi, double& wli, double& BSDF, double& pdfval) const
{
	size_t i;
	if (drand48() < ratio) {
		i = 0;
	} else {
		i = 1;
	}
	assert(inputs[i].connected_links.size() == 1);
	if (inputs[i].connected_links[0]->input->type != SlotBSDF) {
		std::cout << "Error" << std::endl;
	}
	return dynamic_cast<const Material *>(inputs[i].connected_links[0]->input->parent_node)->SampleBSDF(rec, uvw, vo, wlo, vi, wli, BSDF, pdfval);
}

double MixBSDFNode::BSDF(const dvec3& vi, double wli, const dvec3& vo, double wlo, const dvec3& vt) const
{
	size_t i;
	if (drand48() < ratio) {
		i = 0;
	} else {
		i = 1;
	}
	assert(inputs[i].connected_links.size() == 1);
	if (inputs[i].connected_links[0]->input->type != SlotBSDF) {
		std::cout << "Error" << std::endl;
	}
	return dynamic_cast<const Material *>(inputs[i].connected_links[0]->input->parent_node)->BSDF(vi, wli, vo, wlo, vt);
}

void MixBSDFNode::RenderEditor(void)
{
	ImGui::PushID(iid);
	ax::NodeEditor::BeginNode(id);
	ImGui::Text("%s", name.c_str());
	const double min = 0.0;
	const double max = 1.0;
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.25f);
	ImGui::SliderScalar("ratio", ImGuiDataType_Double, &ratio, &min, &max, "%f");
	ImGui::PopItemWidth();
	RenderPins();
	ax::NodeEditor::EndNode();
	ImGui::PopID();
}
*/

OutputNode::OutputNode(const char *name) : MaterialNode(name)
{
	type = MaterialNodeType::Output;
	AddInput(SlotBSDF, "BSDF");
}
OutputNode::OutputNode(const json& j) : MaterialNode(j)
{
	type = MaterialNodeType::Output;
}
void OutputNode::RenderEditor(void)
{
#ifndef _CLI
	ImGui::Text("Final Output");
	//ImGui::Text(name.c_str());
#endif
}

SpectrumNode::SpectrumNode(const char *name) : MaterialNode(name)
{
	type = MaterialNodeType::Spectrum;
	AddOutput(SlotSpectrum, "Out->");
}
SpectrumNode::SpectrumNode(const json& j) : MaterialNode(j)
{
	type = MaterialNodeType::Spectrum;
	for (size_t i = 0; i < j["data"].size(); i++) {
		data.data[i] = j["data"][i];
	}
}
void SpectrumNode::DumpJson(json& j) const
{
	DumpSpectrum(j, data, "data");
	DumpIO(j);
}
void SpectrumNode::RenderEditor(void)
{
#ifndef _CLI
	ImGui::PushID(this);
	ImGui::Text("Spectrum");
	ImGui::Text("%s", name.c_str());
	RenderSpectrum(data, 0.0, 1.0);

	ImGui::PopID();
#endif
}


void SpectrumNode::Compute(const Argument& global_arg, Spectrum& data) const
{
	data = this->data;
}

UVNode::UVNode(const char *name) : MaterialNode(name)
{
	this->type = MaterialNodeType::UV;
	AddOutput(SlotVec3, "Out->");
}
UVNode::UVNode(const json& j) : MaterialNode(j)
{
	this->type = MaterialNodeType::UV;
}

void UVNode::Compute(const Argument& global_arg, dvec3& data) const
{
	data = global_arg.vt;
}


RGBColorNode::RGBColorNode(const char *name) : MaterialNode(name)
{
	type = MaterialNodeType::RGBColor;
	AddOutput(SlotVec3, "RGB->");
}
RGBColorNode::RGBColorNode(const json& j) : MaterialNode(j)
{
	type = MaterialNodeType::RGBColor;
	col[0] = j["col"][0];
	col[1] = j["col"][1];
	col[2] = j["col"][2];
}
void RGBColorNode::DumpJson(json& j) const
{
	DumpIO(j);
	j["col"][0] = col[0];
	j["col"][1] = col[1];
	j["col"][2] = col[2];
}
void RGBColorNode::RenderEditor(void)
{
#ifndef _CLI
	ImGui::PushID(this);
	ImGui::Text("%s", name.c_str());
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
	ImGui::ColorPicker3("Color", col);
	ImGui::PopItemWidth();

	ImGui::PopID();
#endif
}

void RGBColorNode::Compute(const Argument& global_arg, dvec3& data) const
{
	data[0] = col[0];
	data[1] = col[1];
	data[2] = col[2];
}


RGBtoSpectrumNode::RGBtoSpectrumNode(const char *name) : MaterialNode(name)
{
	type = MaterialNodeType::RGBtoSpectrum;
	AddInput(SlotVec3, "RGB");
	AddOutput(SlotSpectrum, "Spectrum");
}
RGBtoSpectrumNode::RGBtoSpectrumNode(const json& j) : MaterialNode(j)
{
	type = MaterialNodeType::RGBtoSpectrum;
}
void RGBtoSpectrumNode::Compute(const Argument& global_arg, Spectrum& data) const
{
	dvec3 RGB;
	const MaterialNode *parent = GetInputParentNode("RGB");
	if (parent == nullptr)
		return;
	parent->Compute(global_arg, RGB);
	data = RGBtoSpectrum(RGB);
}

ImageTextureNode::ImageTextureNode(const char *path, const char *name) : MaterialNode(name)
{
	type = MaterialNodeType::ImageTexture;
	this->path = std::string(path);
	AddInput(SlotVec3, "UV");
	AddOutput(SlotVec3, "Out");
}
ImageTextureNode::ImageTextureNode(const json& j) : MaterialNode(j)
{
	type = MaterialNodeType::ImageTexture;
	path = j["path"].get<std::string>();
	if (path != "")
		texture = stbi_load(path.c_str(), &width, &height, &bpp, 0);
}
void ImageTextureNode::DumpJson(json& j) const
{
	j["path"] = path;
	DumpIO(j);
}

void ImageTextureNode::RenderEditor(void)
{
#ifndef _CLI
	ImGui::PushID(this);
	ImGui::Text("Image Texture Node");
	if (path == "") {
		ImGui::Text("texture is not loaded yet");
	} else {
		ImGui::Text("%s", path.c_str());
	}
	if (ImGui::Button("Open a texture file")) {
		nfdchar_t *path = nullptr;
		nfdresult_t result = NFD_OpenDialog(nullptr, nullptr, &path);
		if (result == NFD_OKAY) {
			this->path = std::string(path);
			texture = stbi_load(path, &width, &height, &bpp, 0);
			free(path);
		}
	}
	ImGui::PopID();
#endif
}
void ImageTextureNode::Compute(const Argument& global_arg, dvec3& data) const
{
	if (texture == nullptr)
		return;

	dvec3 vt;
	const MaterialNode *node = GetInputParentNode("UV");
	if (node == nullptr)
		return;
	node->Compute(global_arg, vt);


	int x = static_cast<double>(width) * vt[0];
	int y = static_cast<double>(height) * (1.0-vt[1]);
	data[0] = static_cast<double>(texture[bpp*(x+y*width)])/255.0;
	data[1] = static_cast<double>(texture[bpp*(x+y*width)+1])/255.0;
	data[2] = static_cast<double>(texture[bpp*(x+y*width)+2])/255.0;
}


CheckerboardNode::CheckerboardNode(const char *name) : MaterialNode(name)
{
	type = MaterialNodeType::Checkerboard;
	AddInput(SlotVec3, "UV");
	AddOutput(SlotVec3, "Out");
}
CheckerboardNode::CheckerboardNode(const json& j) : MaterialNode(j)
{
	type = MaterialNodeType::Checkerboard;
	size = j["size"];
}
void CheckerboardNode::DumpJson(json& j) const
{
	j["size"] = size;
	DumpIO(j);
}
void CheckerboardNode::Compute(const Argument& global_arg, dvec3& data) const
{
	//assert(inputs[0].connected_links.size() == 1);
	//assert(inputs[0].connected_links[0]->input != nullptr);
	dvec3 vt;
	const MaterialNode *node = GetInputParentNode("UV");
	if (node == nullptr)
		return;
	node->Compute(global_arg, vt);
	//inputs[0].connected_links[0]->input->parent_node->Compute(global_arg, vt);
	int x = (vt[0] - fmod(vt[0], size))/size;
	int y = (vt[1] - fmod(vt[1], size))/size;
	if (x%2 == y%2)
		data = dvec3(1.0, 1.0, 1.0);
	else
		data = dvec3(0.0, 0.0, 0.0);
}


void CheckerboardNode::RenderEditor(void)
{
#ifndef _CLI
	ImGui::PushID(this);
	ImGui::Text("%s", name.c_str());
	const double min = 0.001;
	const double max = 1.0;
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.25f);
	ImGui::SliderScalar("size", ImGuiDataType_Double, &size, &min, &max, "%f");
	ImGui::PopItemWidth();
	ImGui::PopID();
#endif
}


AdditionNode::AdditionNode(const char *name) : MaterialNode(name)
{
	type = MaterialNodeType::Addition;
	AddInput(SlotUniversal, "In0");
	AddInput(SlotUniversal, "In1");
	AddOutput(SlotUniversal, "Out");
}
AdditionNode::AdditionNode(const json& j) : MaterialNode(j)
{
	type = MaterialNodeType::Addition;
}

#define ADDMACRO(type) \
	const MaterialNode *node = GetInputParentNode("In0"); \
	if (node == nullptr)\
		return;\
	node->Compute(global_arg, data);\
	node = GetInputParentNode("In1");\
	if (node == nullptr)\
		return;\
	type tmp;\
	node->Compute(global_arg, tmp);\
	data = data + tmp;

void AdditionNode::Compute(const Argument& global_arg, double &data) const
{
	data = 0;
	ADDMACRO(double)
}
void AdditionNode::Compute(const Argument& global_arg, dvec3& data) const
{
	data = dvec3(0.0, 0.0, 0.0);
	ADDMACRO(dvec3)
}
void AdditionNode::Compute(const Argument& global_arg, Spectrum& data) const
{
	data = Spectrum(0.0);
	ADDMACRO(Spectrum)
}


ScalarMultiplicationNode::ScalarMultiplicationNode(const char *name) : MaterialNode(name)
{
	type = MaterialNodeType::ScalarMultiplication;
	AddInput(SlotUniversal, "In");
	AddOutput(SlotUniversal, "Out");
}
ScalarMultiplicationNode::ScalarMultiplicationNode(const json& j) : MaterialNode(j)
{
	type = MaterialNodeType::ScalarMultiplication;
	scale = j["scale"];
}
void ScalarMultiplicationNode::DumpJson(json& j) const
{
	j["scale"] = scale;
	DumpIO(j);
}
void ScalarMultiplicationNode::RenderEditor(void)
{
#ifndef _CLI
	ImGui::PushID(this);
	ImGui::Text("%s", name.c_str());
	const double min = -100.0;
	const double max = 100.0;
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.25f);
	ImGui::SliderScalar("scale", ImGuiDataType_Double, &scale, &min, &max, "%f");
	ImGui::PopItemWidth();
	ImGui::PopID();
#endif
}




#define SCALARMULTIMACRO(type)\
	const MaterialNode *node = GetInputParentNode("scale");\
	if (node == nullptr)\
		return;\
	type scale;\
	node->Compute(global_arg, scale);\
	data = data*scale;



void ScalarMultiplicationNode::Compute(const Argument& global_arg, double &data) const
{
	SCALARMULTIMACRO(double);
}
void ScalarMultiplicationNode::Compute(const Argument& global_arg, dvec3& data) const
{
	SCALARMULTIMACRO(dvec3);
}
void ScalarMultiplicationNode::Compute(const Argument& global_arg, Spectrum& data) const
{
	SCALARMULTIMACRO(Spectrum);
}

MultiplicationNode::MultiplicationNode(const char *name) : MaterialNode(name)
{
	type = MaterialNodeType::Multiplication;
	AddInput(SlotUniversal, "In0");
	AddInput(SlotUniversal, "In1");
	AddOutput(SlotUniversal, "Out");
}
MultiplicationNode::MultiplicationNode(const json& j) : MaterialNode(j)
{
	type = MaterialNodeType::Multiplication;
}
#define MULTIMACRO(type)\
	const MaterialNode *node = GetInputParentNode("In0");\
	if (node == nullptr)\
		return;\
	type d1;\
	node->Compute(global_arg, d1);\
	node = GetInputParentNode("In1");\
	if (node == nullptr)\
		return;\
	type d2;\
	node->Compute(global_arg, d2);\
	data = d1*d2;
	
void MultiplicationNode::Compute(const Argument& global_arg, double &data) const
{
	MULTIMACRO(double);
}
void MultiplicationNode::Compute(const Argument& global_arg, dvec3& data) const
{
	MULTIMACRO(dvec3);
}
void MultiplicationNode::Compute(const Argument& global_arg, Spectrum& data) const
{
	MULTIMACRO(Spectrum);
}


RandomSamplingNode::RandomSamplingNode(const char *name) : MaterialNode(name)
{
	type = MaterialNodeType::RandomSampling;
	AddInput(SlotUniversal, "In0");
	AddInput(SlotUniversal, "In1");
	AddOutput(SlotUniversal, "Out");
}
RandomSamplingNode::RandomSamplingNode(const json& j) : MaterialNode(j)
{
	type = MaterialNodeType::RandomSampling;
	ratio = j["ratio"];
}
void RandomSamplingNode::DumpJson(json& j) const
{
	j["ratio"] = ratio;
	DumpIO(j);
}
void RandomSamplingNode::Compute(const Argument& global_arg, double &data) const
{
	size_t i;
	if (drand48() < ratio) {
		i = 0;
	} else {
		i = 1;
	}
	//assert(inputs[i].connected_links.size() == 1);
	//if (inputs[i].connected_links[0]->input->type != SlotDouble) {
	//	std::cout << "Error" << std::endl;
	//}
	//inputs[i].connected_links[0]->input->parent_node->Compute(global_arg, data);
}
void RandomSamplingNode::Compute(const Argument& global_arg, dvec3& data) const
{
	size_t i;
	if (drand48() < ratio) {
		i = 0;
	} else {
		i = 1;
	}
	//assert(inputs[i].connected_links.size() == 1);
	//if (inputs[i].connected_links[0]->input->type != SlotVec3) {
	//	std::cout << "Error" << std::endl;
	//}
	//inputs[i].connected_links[0]->input->parent_node->Compute(global_arg, data);
}
void RandomSamplingNode::Compute(const Argument& global_arg, Spectrum& data) const
{
	size_t i;
	if (drand48() < ratio) {
		i = 0;
	} else {
		i = 1;
	}
	//assert(inputs[i].connected_links.size() == 1);
	//if (inputs[i].connected_links[0]->input->type != SlotSpectrum) {
	//	std::cout << "Error" << std::endl;
	//}
	//inputs[i].connected_links[0]->input->parent_node->Compute(global_arg, data);
}

void RandomSamplingNode::RenderEditor(void)
{
#ifndef _CLI
	ImGui::PushID(this);
	ImGui::Text("%s", name.c_str());
	const double min = 0.0;
	const double max = 1.0;
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.25f);
	ImGui::SliderScalar("ratio", ImGuiDataType_Double, &ratio, &min, &max, "%f");
	ImGui::PopItemWidth();
	ImGui::PopID();
#endif
}


AccessVec3ComponentNode::AccessVec3ComponentNode(const char *name) : MaterialNode(name)
{
	type = MaterialNodeType::AccessVec3Component;
	AddInput(SlotVec3, "vector");
	AddOutput(SlotDouble, "output");
}

AccessVec3ComponentNode::AccessVec3ComponentNode(const json& j) : MaterialNode(j)
{
	type = MaterialNodeType::AccessVec3Component;
}

void AccessVec3ComponentNode::Compute(const Argument& global_arg, double &data) const
{
	dvec3 v;
	const MaterialNode *node = GetInputParentNode("vector");
	if (node == nullptr)
		return;
	node->Compute(global_arg, v);
	data = v[index];
}


void AccessVec3ComponentNode::RenderEditor(void)
{
#ifndef _CLI
	ImGui::PushID(this);
	ImGui::Text("%s", name.c_str());
	const uint32_t min = 0;
	const uint32_t max = 2;
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.25f);
	ImGui::SliderScalar("index", ImGuiDataType_U32, &index, &min, &max, "%u");
	ImGui::PopItemWidth();
	ImGui::PopID();
#endif
}


CombineVec3ComponentNode::CombineVec3ComponentNode(const char *name) : MaterialNode(name)
{
	type = MaterialNodeType::CombineVec3Component;
	AddInput(SlotDouble, "0");
	AddInput(SlotDouble, "1");
	AddInput(SlotDouble, "2");
	AddOutput(SlotVec3, "vec");
}

CombineVec3ComponentNode::CombineVec3ComponentNode(const json& j) : MaterialNode(j)
{
}


void CombineVec3ComponentNode::Compute(const Argument& global_arg, dvec3& data) const
{
	for (size_t i = 0; i < 3; i++) {
		const MaterialNode *node = nullptr;
		if (i == 0)
			node = GetInputParentNode("0");
		else if (i == 1)
			node = GetInputParentNode("1");
		else
			node = GetInputParentNode("2");
		if (node == nullptr)
			return;
		node->Compute(global_arg, data[i]);
	}
}


ValueNoiseNode::ValueNoiseNode(const char *name) : MaterialNode(name)
{
	type = MaterialNodeType::ValueNoise;
	AddInput(SlotDouble, "input");
	AddOutput(SlotDouble, "output");
	GenerateRand();
}

ValueNoiseNode::ValueNoiseNode(const json& j) : MaterialNode(j)
{
	type = MaterialNodeType::ValueNoise;
	size = j["u_size"];
	GenerateRand();
}


void ValueNoiseNode::Compute(const Argument& global_arg, double &data) const
{
	const MaterialNode *node = GetInputParentNode("input");
	if (node == nullptr)
		return;
	double u;
	node->Compute(global_arg, u);
	u = fmod(u, 1.0);
	if (u < 0.0)
		u += 1.0;
	assert(r.size() > 1);
	double us = 1.0/(r.size()-1);
	double rem = fmod(u, us);
	int x0 = (u-rem)/us;
	int x1 = x0+1;
	double t = rem/us;
	assert(x1<r.size());
	data = (1.0-t)*r[x0] + t*r[x1];
}


void ValueNoiseNode::DumpJson(json& j) const
{
	DumpIO(j);
	j["u_size"] = size;
}


void ValueNoiseNode::RenderEditor(void)
{
#ifndef _CLI
	ImGui::PushID(this);
	ImGui::Text("%s", name.c_str());
	const uint32_t min = 1;
	const uint32_t max = 1024;
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.25f);
	ImGui::SliderScalar("size", ImGuiDataType_U32, &size, &min, &max, "%u");
	ImGui::PopItemWidth();
	if (ImGui::Button("Generate Random")) {
		GenerateRand();
	}
	ImGui::PopID();
#endif
}


void ValueNoiseNode::GenerateRand(unsigned int seed)
{
	srand48(seed);
	r.resize(size+1);
	for (int i = 0; i < r.size(); i++) {
		r[i] = drand48();
	}
}

ValueNoise2DNode::ValueNoise2DNode(const char *name) : MaterialNode(name)
{
	type = MaterialNodeType::ValueNoise2D;
	AddInput(SlotDouble, "u");
	AddInput(SlotDouble, "v");
	AddOutput(SlotDouble, "output");
	GenerateRand();
}

ValueNoise2DNode::ValueNoise2DNode(const json& j) : MaterialNode(j)
{
	type = MaterialNodeType::ValueNoise2D;
	u_size = j["u_size"];
	v_size = j["v_size"];
	GenerateRand();
}


void ValueNoise2DNode::DumpJson(json& j) const
{
	DumpIO(j);
	j["u_size"] = u_size;
	j["v_size"] = v_size;
}


void ValueNoise2DNode::Compute(const Argument& global_arg, double &data) const
{
	const MaterialNode *u_node = GetInputParentNode("u");
	const MaterialNode *v_node = GetInputParentNode("v");
	if (u_node == nullptr || v_node == nullptr)
		return;
	double u, v;
	u_node->Compute(global_arg, u);
	v_node->Compute(global_arg, v);
	u = fmod(u, 1.0);
	if (u < 0.0)
		u += 1.0;
	v = fmod(v, 1.0);
	if (v < 0.0)
		v += 1.0;
	double us = 1.0/u_size;
	double vs = 1.0/v_size;
	double urem = fmod(u, us);
	double vrem = fmod(v, vs);
	unsigned int u0 = (u-urem)/us;
	unsigned int u1 = u0+1;
	unsigned int v0 = (v-vrem)/vs;
	unsigned int v1 = v0+1;

	double ut = urem/us;
	double vt = vrem/vs;
	ut = ut*ut*(3.0-2.0*ut);
	vt = vt*vt*(3.0-2.0*vt);
	double a = (1.0-ut)*r[u0*(v_size+1)+v0] + ut*r[u1*(v_size+1)+v0];
	double b = (1.0-ut)*r[u0*(v_size+1)+v1] + ut*r[u1*(v_size+1)+v1];
	data = (1.0-vt)*a + vt*b;
}


void ValueNoise2DNode::RenderEditor(void)
{
#ifndef _CLI
	ImGui::PushID(this);
	ImGui::Text("%s", name.c_str());
	const uint32_t min = 1;
	const uint32_t max = 1024;
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.25f);
	ImGui::SliderScalar("u size", ImGuiDataType_U32, &u_size, &min, &max, "%u");
	ImGui::SliderScalar("v size", ImGuiDataType_U32, &v_size, &min, &max, "%u");
	ImGui::PopItemWidth();
	if (ImGui::Button("Generate Random")) {
		GenerateRand();
	}
	ImGui::PopID();
#endif
}


void ValueNoise2DNode::GenerateRand(unsigned int seed)
{
	srand48(seed);
	r.resize((u_size+1)*(v_size+1));
	for (int i = 0; i < u_size+1; i++) {
		for (int j = 0; j < v_size+1; j++)
			r[i*(v_size+1)+j] = drand48();
	}
}



RodriguesRotationNode::RodriguesRotationNode(const char *name) : MaterialNode(name)
{
	AddInput(SlotVec3, "v");
	AddInput(SlotVec3, "n");
	AddInput(SlotDouble, "theta");
	AddOutput(SlotVec3, "output");
	type = MaterialNodeType::RodriguesRotation;
}
RodriguesRotationNode::RodriguesRotationNode(const json& j) : MaterialNode(j)
{
	type = MaterialNodeType::RodriguesRotation;
	for (int i = 0; i < 3; i++)
		n[i] = j["n"][i];
	theta = j["theta"];
}

void RodriguesRotationNode::DumpJson(json& j) const
{
	for (int i = 0; i < 3; i++)
		j["n"][i] = n[i];
	j["theta"] = theta;
}


void RodriguesRotationNode::Compute(const Argument& global_arg, dvec3& data) const
{
	dvec3 nn;
	const MaterialNode *node = GetInputParentNode("n");
	if (node == nullptr) {
		nn = unit_vector(n);
	} else {
		node->Compute(global_arg, nn);
		nn.make_unit_vector();
	}

	double c;
	double s;
	node = GetInputParentNode("theta");
	if (node == nullptr) {
		c = cos(theta);
		s = sin(theta);
	} else {
		double t;
		node->Compute(global_arg, t);
		c = cos(t);
		s = sin(t);
	}
	double nx = nn[0];
	double ny = nn[1];
	double nz = nn[2];
	dvec3 r0 = dvec3(c+nx*nx*(1.0-c), nx*ny*(1.0-c)-nz*s, nx*nz*(1.0-c)+ny*s);
	dvec3 r1 = dvec3(ny*nx*(1.0-c)+nz*s, c+ny*ny*(1.0-c), ny*nz*(1.0-c)-nx*s);
	dvec3 r2 = dvec3(nz*nx*(1.0-c)-ny*s, nz*ny*(1.0-c)+nx*s, c+nz*nz*(1.0-c));
	node = GetInputParentNode("v");
	if (node == nullptr)
		return;
	dvec3 v;
	node->Compute(global_arg, v);
	data[0] = dot(r0, v);
	data[1] = dot(r1, v);
	data[2] = dot(r2, v);
}
void RodriguesRotationNode::RenderEditor(void)
{
#ifndef _CLI
	ImGui::PushID(this);
	ImGui::Text("%s", name.c_str());
	if (GetInputParentNode("n") == nullptr) {
		const double min = 0.0;
		const double max = 1.0;
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.25f);
		ImGui::SliderScalar("nx", ImGuiDataType_Double, &n[0], &min, &max, "%f");
		ImGui::SliderScalar("ny", ImGuiDataType_Double, &n[1], &min, &max, "%f");
		ImGui::SliderScalar("nz", ImGuiDataType_Double, &n[2], &min, &max, "%f");
		ImGui::PopItemWidth();
	}
	if (GetInputParentNode("theta") == nullptr) {
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.25f);
		const double theta_min = -M_PI;
		const double theta_max = M_PI;
		ImGui::SliderScalar("theta", ImGuiDataType_Double, &theta, &theta_min, &theta_max, "%f");
		ImGui::PopItemWidth();
	}
	ImGui::PopID();
#endif
}


NodeMaterial::NodeMaterial(const char *name) : name(name)
{
	material_nodes.push_back(new UVNode());
	material_nodes.push_back(new OutputNode());
}


#define CONSTRUCTOR_SWITCH(r, arg, elem) case MaterialNodeType::elem:\
	node = new BOOST_PP_CAT(elem, Node)(arg);\
	break;
#define CALL_CONSTRUCTOR(SEQ, arg) BOOST_PP_SEQ_FOR_EACH(CONSTRUCTOR_SWITCH, arg, SEQ)
NodeMaterial::NodeMaterial(const json& j) :
	name(j.value("name", "untitled material")),
	light_flag(j.value("light_flag", false))
{
	for (const auto& node_j : j["nodes"]) {
		MaterialNode *node = nullptr;
		switch (StringToMaterialNodeType(node_j["type"].get<std::string>())) {
			CALL_CONSTRUCTOR(MATNODETYPE_SEQ, node_j)
			default:
				std::cout << "Error: Unimplemented" << std::endl;
				assert(false);
				break;
		}

		material_nodes.push_back(node);
	}
	for (const auto& conn_j : j["connections"]) {
		Connection new_conn;
		int output_node_i = conn_j["output_node"].get<int>();
		int input_node_i = conn_j["input_node"].get<int>();
		new_conn.output_node = material_nodes[output_node_i];
		new_conn.output_slot = strdup(conn_j["output_slot"].get<std::string>().c_str());
		new_conn.input_node = material_nodes[input_node_i];
		new_conn.input_slot = strdup(conn_j["input_slot"].get<std::string>().c_str());
		material_nodes[output_node_i]->connections.push_back(new_conn);
		material_nodes[input_node_i]->connections.push_back(new_conn);
	}
}


NodeMaterial::~NodeMaterial(void)
{
#ifndef _CLI
#endif
}

void NodeMaterial::DumpJson(json& j) const
{
	j["name"] = name;
	j["light_flag"] = light_flag;
	for (const auto& node : material_nodes) {
		json node_j;
		node->DumpJson(node_j);
		j["nodes"].push_back(node_j);
	}
	std::map<MaterialNode *, int> nodeid_map;
	for (size_t i = 0; i < material_nodes.size(); i++) {
		nodeid_map.insert(std::make_pair(material_nodes[i], i));
	}
	for (size_t i = 0; i < material_nodes.size(); i++) {
		for (const auto& c : material_nodes[i]->connections) {
			if (static_cast<MaterialNode *>(c.output_node) == material_nodes[i])
				continue;
			json conn;
			conn["input_node"] = nodeid_map.at(c.input_node);
			conn["input_slot"] = std::string(c.input_slot);
			conn["output_node"] = nodeid_map.at(c.output_node);
			conn["output_slot"] = std::string(c.output_slot);
			j["connections"].push_back(conn);
		}
	}
}

void NodeMaterial::RenderNode(void)
{
#ifndef _CLI
	ImNodes::BeginCanvas(&canvas);
	if (ImGui::IsMouseReleased(1) && ImGui::IsWindowHovered() && !ImGui::IsMouseDragging(1))
		ImGui::OpenPopup("Create New Node");


	selected_nodes_count = 0;

	// Render Material Nodes
	for (size_t i = 0; i < material_nodes.size(); i++) {
		MaterialNode *node = material_nodes[i];
		if (ImNodes::Ez::BeginNode((void*)node, node->name.c_str(), &node->pos, &node->selected)) {
			ImNodes::Ez::InputSlots(node->input_slots.data(), node->input_slots.size());
			//node->RenderEditor();
			ImNodes::Ez::OutputSlots(node->output_slots.data(), node->output_slots.size());
			Connection new_conn;
			// Get a new connection if created
			if (ImNodes::GetNewConnection((void **)&new_conn.input_node, &new_conn.input_slot,
						(void **)&new_conn.output_node, &new_conn.output_slot)) {
				new_conn.input_node->connections.push_back(new_conn);
				new_conn.output_node->connections.push_back(new_conn);
			}

			// Render connections
			for (const auto& c : node->connections) {
				if (c.output_node == node)
					continue;
				// Delete a connection if double clicked
				if (!ImNodes::Connection(c.input_node, c.input_slot, c.output_node, c.output_slot)) {
					static_cast<MaterialNode *>(c.input_node)->DeleteConnection(c);
					static_cast<MaterialNode *>(c.output_node)->DeleteConnection(c);
				}
			}
		}
		ImNodes::Ez::EndNode();
		if (node->selected && ImGui::IsKeyPressedMap(ImGuiKey_Delete)) {
			for (const auto& c : node->connections) {
				if (c.output_node == node) {
					static_cast<MaterialNode *>(c.input_node)->DeleteConnection(c);
				} else {
					static_cast<MaterialNode *>(c.output_node)->DeleteConnection(c);
				}
			}
			material_nodes.erase(material_nodes.begin() + i);
		}
		if (node->selected) {
			selected_nodes_count++;
			last_selected_node_index = i;
		}
	}


#define MENUITEM_CONSTRUCTOR_IF(r, data, elem) if (ImGui::MenuItem(BOOST_PP_STRINGIZE(BOOST_PP_CAT(elem, Node)))) {\
	node = new BOOST_PP_CAT(elem, Node)();\
}
#define CALL_MENUITEM_AND_CONSTRUCTOR(SEQ) BOOST_PP_SEQ_FOR_EACH(MENUITEM_CONSTRUCTOR_IF, _, SEQ)

	if (ImGui::BeginPopup("Create New Node")) {
		ImGui::Text("Select a nodetype");
		MaterialNode *node = nullptr;
		CALL_MENUITEM_AND_CONSTRUCTOR(MATNODETYPE_SEQ)
		if (node != nullptr)
			material_nodes.push_back(node);
		ImGui::EndPopup();
	}
	ImNodes::EndCanvas();

#endif
}


void NodeMaterial::PreProcess(HitRecord& rec, RandomData& rand_data) const
{
	const MaterialNode *node = material_nodes[Output_i]->GetInputParentNode("BSDF");
	if (node == nullptr)
		return;

	Argument global_arg = { rec.vt };

	auto p = dynamic_cast<const BSDFMaterialNode *>(node);
	p->PreProcess(global_arg, rec);
}
bool NodeMaterial::SampleBSDF(RayType type, const HitRecord& rec, const ONB& uvw, const dvec3& vo, double wlo, dvec3& vi, double& wli, double& bxdf_divided_by_pdf, double& BSDF, double& pdfval) const
{
	const MaterialNode *node = material_nodes[Output_i]->GetInputParentNode("BSDF");
	if (node == nullptr)
		return false;
	auto p = dynamic_cast<const BSDFMaterialNode *>(node);
	Argument global_arg = { rec.vt };
	return p->SampleBSDF(global_arg, type, rec, uvw, vo, wlo, vi, wli, bxdf_divided_by_pdf, BSDF, pdfval);
}
double NodeMaterial::BSDF(const dvec3& vi, double wli, const dvec3& vo, double wlo, const dvec3& vt) const
{
	const MaterialNode *node = material_nodes[Output_i]->GetInputParentNode("BSDF");
	if (node == nullptr)
		return 0.0;

	auto p = dynamic_cast<const BSDFMaterialNode *>(node);
	Argument global_arg = { vt };
	return p->BSDF(global_arg, vi, wli, vo, wlo);
}
double NodeMaterial::PDF(const dvec3& vi, double wli, const dvec3& vo, double wlo, const dvec3& vt) const
{
	const MaterialNode *node = material_nodes[Output_i]->GetInputParentNode("BSDF");
	if (node == nullptr)
		return 0.0;

	auto p = dynamic_cast<const BSDFMaterialNode *>(node);
	Argument global_arg = { vt };
	return p->PDF(global_arg, vi, wli, vo, wlo);
}
double NodeMaterial::Emitted(const ray& r, const HitRecord& rec, const dvec3& vt) const
{
	const MaterialNode *node = material_nodes[Output_i]->GetInputParentNode("BSDF");
	if (node == nullptr)
		return 0.0;
	auto p = dynamic_cast<const BSDFMaterialNode *>(node);
	Argument global_arg = { vt };
	return p->Emitted(global_arg, r, rec);
}

