#include "materialnode.h"
#include "material.h"
#include "pdf.h"
#include "stb_image.h"
#include <nfd.h>

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

LinkInfo::LinkInfo(const json& j, PinInfo *input, PinInfo *output) :
	id(j["id"]),
	iid(j["id"]),
	input_id(j["input_id"]),
	output_id(j["output_id"]),
	input(input),
	output(output)
{
}

void LinkInfo::DumpJson(json& j) const
{
	j["id"] = iid;
	j["input_id"] = input->iid;
	j["output_id"] = output->iid;
}

PinInfo::PinInfo(int &unique_id, PinIOType io_type, PinType type, const char *name, const MaterialNode *parent_node) :
	id(unique_id), iid(unique_id),
	name(name),
	io_type(io_type),
	type(type),
	parent_node(parent_node)
{
	unique_id++;
}
PinInfo::PinInfo(const json& j, const MaterialNode *parent_node) :
	id(j["id"]),
	iid(j["id"]),
	name(j["name"]),
	io_type(static_cast<PinIOType>(j["io_type"])),
	type(static_cast<PinType>(j["type"])),
	parent_node(parent_node)
{
}


struct PinInfo *NodeMaterial::FindPin(int iid)
{
	for (auto& node : material_nodes) {
		for (auto& pin : node->inputs) {
			if (pin.iid == iid)
				return &pin;
		}
		for (auto& pin : node->outputs) {
			if (pin.iid == iid)
				return &pin;
		}
	}
	return nullptr;
}

struct PinInfo *NodeMaterial::FindPin(const ed::PinId& id)
{
	for (auto& node : material_nodes) {
		for (auto& pin : node->inputs) {
			if (pin.id == id)
				return &pin;
		}
		for (auto& pin : node->outputs) {
			if (pin.id == id)
				return &pin;
		}
	}
	return nullptr;
}

struct LinkInfo *NodeMaterial::FindLink(const ed::LinkId& id)
{
	for (auto& link : links) {
		if (link->id == id)
			return link;
	}
	return nullptr;
}

const struct PinInfo *NodeMaterial::FindPinConst(const ed::PinId& id) const
{
	for (auto& node : material_nodes) {
		for (auto& pin : node->inputs) {
			if (pin.id == id)
				return &pin;
		}
		for (auto& pin : node->outputs) {
			if (pin.id == id)
				return &pin;
		}
	}
	return nullptr;
}

const struct LinkInfo *NodeMaterial::FindLinkConst(const ed::LinkId& id) const
{
	for (auto& link : links) {
		if (link->id == id)
			return link;
	}
	return nullptr;
}

MaterialNode::MaterialNode(int &unique_id, const char *name) : id(unique_id), iid(unique_id), name(name)
{
	unique_id++;
}
MaterialNode::MaterialNode(const json& j) : id(j["id"]), iid(j["id"]), name(j["name"].get<std::string>())
{
	for (const auto& input_j : j["inputs"]) {
		PinInfo new_pin(input_j, this);
		inputs.push_back(new_pin);
	}
	for (const auto& output_j : j["outputs"]) {
		PinInfo new_pin(output_j, this);
		outputs.push_back(new_pin);
	}
}
void MaterialNode::Compute(double& data) const
{
}
void MaterialNode::Compute(Spectrum& data) const
{
}
void MaterialNode::Compute(vec3& data) const
{
}

void MaterialNode::DumpJson(json& j) const
{
	DumpIO(j);
};
void MaterialNode::DumpIO(json &j) const
{
	j["name"] = name;
	j["id"] = iid;
	j["type"] = static_cast<int>(type);
	std::cout << "type: " << static_cast<int>(type) << std::endl;
	j["inputs"] = json::array();
	j["outputs"] = json::array();
	for (size_t i = 0; i < inputs.size(); i++) {
		j["inputs"][i]["id"] = inputs[i].iid;
		j["inputs"][i]["name"] = inputs[i].name;
		j["inputs"][i]["io_type"] = inputs[i].io_type;
		j["inputs"][i]["type"] = inputs[i].type;
	}
	for (size_t i = 0; i < outputs.size(); i++) {
		j["outputs"][i]["id"] = outputs[i].iid;
		j["outputs"][i]["name"] = outputs[i].name;
		j["outputs"][i]["io_type"] = outputs[i].io_type;
		j["outputs"][i]["type"] = outputs[i].type;
	}
};
void MaterialNode::DumpSpectrum(json& j, const Spectrum& s, const char *name) const
{
	for (const auto& d : s.data) {
		j[name].push_back(d);
	}
}

void MaterialNode::UpdateNormal(const PinInfo *normal_pin, const HitRecord& rec, vec3& new_normal) const
{
	new_normal = rec.normal;
	if (!normal_pin->connected_links.empty()) {
		if (normal_pin->connected_links.size() != 1) {
			std::cout << "node connection error" << std::endl;
			return;
		}
		const PinInfo *connected_pin = normal_pin->connected_links[0]->input;
		assert(connected_pin != nullptr);
		const MaterialNode *parent = connected_pin->parent_node;
		vec3 normal;
		parent->Compute(normal);
		normal = unit_vector(normal*2-vec3(1.0, 1.0, 1.0));
		new_normal = rec.tbn.LocalToWorld(normal);

		if (dot(rec.normal, rec.tbn.axis[2]) < 0.0) {
			std::cout << "Warning 0: UV mapping may be incorrect" << std::endl;
		}
		//if (dot(face_normal, rec.tbn.axis[2]) < 0.0) {
		//	std::cout << "Warning 1: UV mapping may be incorrect" << std::endl;
		//}
	}
};
void NodeMaterial::AddLink(PinInfo *input, PinInfo *output)
{
	assert(input != nullptr);
	assert(output != nullptr);
	struct LinkInfo *new_link = new LinkInfo(unique_id, input->id, output->id, input, output);
	links.push_back(new_link);
	input->connected_links.push_back(new_link);
	output->connected_links.push_back(new_link);
}

void MaterialNode::RenderPins(void)
{
	for (size_t i = 0; i < inputs.size() || i < outputs.size(); i++) {
		if (i < inputs.size()) {
			ax::NodeEditor::BeginPin(inputs[i].id, ax::NodeEditor::PinKind::Input);
			ImGui::TextColored(pin_colors[inputs[i].type], "%s", inputs[i].name.c_str());
			ax::NodeEditor::EndPin();
		}
		if (i < inputs.size() && i < outputs.size())
			ImGui::SameLine();
		if (i < outputs.size()) {
			ax::NodeEditor::BeginPin(outputs[i].id, ax::NodeEditor::PinKind::Output);
			ImGui::TextColored(pin_colors[outputs[i].type], "%s", outputs[i].name.c_str());
			ax::NodeEditor::EndPin();
		}
	}
}

void MaterialNode::RenderSpectrum(Spectrum &data, double min, double max)
{
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
}

void MaterialNode::Render(void)
{
	ax::NodeEditor::BeginNode(id);
	ImGui::Text("%s", name.c_str());

	RenderPins();

	ax::NodeEditor::EndNode();
}


void MaterialNode::AddInput(int &unique_id, PinType type, const char *name)
{
	PinInfo input(unique_id, PinInput, type, name, this);
	inputs.push_back(input);
}


void MaterialNode::AddOutput(int &unique_id, PinType type, const char *name)
{
	PinInfo output(unique_id, PinOutput, type, name, this);
	outputs.push_back(output);
}

LambertianNode::LambertianNode(int &unique_id, const char *name) : MaterialNode(unique_id, name)
{
	type = LambertianType;
	AddInput(unique_id, PinSpectrum, "->Albedo");
	AddInput(unique_id, PinVec3, "->Normal");
	AddOutput(unique_id, PinBSDF, "BSDF->");
	albedo_pin = &inputs[0];
	normal_pin = &inputs[1];
}
LambertianNode::LambertianNode(const json& j) : MaterialNode(j)
{
	type = LambertianType;
	albedo_pin = &inputs[0];
	normal_pin = &inputs[1];
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

void LambertianNode::Render(void)
{
	ImGui::PushID(iid);
	ax::NodeEditor::BeginNode(id);
	ImGui::Text("%s", name.c_str());
	if (albedo_pin->connected_links.empty())
		RenderSpectrum(albedo, 0.0, 1.0);

	RenderPins();
	ax::NodeEditor::EndNode();
	ImGui::PopID();
}


void LambertianNode::PreProcess(HitRecord& rec) const
{
	vec3 new_normal;
	UpdateNormal(normal_pin, rec, new_normal);
	rec.normal = new_normal;
}

bool LambertianNode::Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& bxdf_divided_by_pdf, double& BxDF, double& pdfval) const
{
	CosinePdf Pdf(rec.normal);

	vec3 generated_direction = Pdf.Generate();
	pdfval = Pdf.PdfVal(generated_direction);
	vi = uvw.WorldToLocal(generated_direction);
	wli = wlo;
	BxDF = this->BxDF(vi, wli, vo, wlo);
	bxdf_divided_by_pdf = BxDF / pdfval;
	return true;
}

double LambertianNode::BxDF(const vec3& vi, double wli, const vec3& vo, double wlo, const vec3& vt) const
{
	if (vi.z() < 0.0)
		return 0.0;
	if (albedo_pin->connected_links.empty())
		return albedo.get(wli)/M_PI;
	else {
		if (albedo_pin->connected_links.size() != 1) {
			std::cout << "node connection error" << std::endl;
			return 0.0;
		}

		const PinInfo *connected_pin = albedo_pin->connected_links[0]->input;
		assert(connected_pin != nullptr);
		const MaterialNode *parent = connected_pin->parent_node;
		Spectrum albedo;
		parent->Compute(albedo);
		return albedo.get(wli)/M_PI;
	}

}
double LambertianNode::PDF(const vec3& vi, double wli, const vec3& vo, double wlo, const vec3& vt) const
{
	if (vi.z() < 0.0)
		return 0.0;
	double cos_theta = vi.z();
	return cos_theta/M_PI;
}
ConductorNode::ConductorNode(int &unique_id, const char *name) : MaterialNode(unique_id, name)
{
	type = ConductorType;
	AddInput(unique_id, PinSpectrum, "->n");
	AddInput(unique_id, PinSpectrum, "->k");
	AddInput(unique_id, PinVec3, "->normal");
	AddOutput(unique_id, PinBSDF, "BSDF->");
	normal_pin = &inputs[2];
}
ConductorNode::ConductorNode(const json& j) : MaterialNode(j)
{
	type = ConductorType;
	normal_pin = &inputs[2];

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

void ConductorNode::Render(void)
{
	ImGui::PushID(iid);
	ax::NodeEditor::BeginNode(id);
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
	RenderPins();
	ax::NodeEditor::EndNode();
	ImGui::PopID();
}
void ConductorNode::PreProcess(HitRecord& rec) const
{
	vec3 new_normal;
	UpdateNormal(normal_pin, rec, new_normal);
	rec.normal = new_normal;
}

bool ConductorNode::Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& bxdf_divided_by_pdf, double& BxDF, double& pdfval) const
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
	BxDF = std::numeric_limits<double>::infinity();
	bxdf_divided_by_pdf = fresnel/cos_o;

	return true;
}
double ConductorNode::BxDF(const vec3& vi, double wli, const vec3& vo, double wlo, const vec3& vt) const
{
	if (-vi[0] == vo[0] && -vi[1] == vo[1] && vi[2] == vo[2])
		return std::numeric_limits<double>::infinity();
	return 0.0;
}
double ConductorNode::PDF(const vec3& vi, double wli, const vec3& vo, double wlo, const vec3& vt) const
{
	if (-vi[0] == vo[0] && -vi[1] == vo[1] && vi[2] == vo[2])
		return std::numeric_limits<double>::infinity();
	return 0.0;
}

ColoredMetal::ColoredMetal(int &unique_id, const char *name) : MaterialNode(unique_id, name)
{
	type = ColoredMetalType;
	AddInput(unique_id, PinSpectrum, "->albedo");
	AddInput(unique_id, PinVec3, "->normal");
	AddOutput(unique_id, PinBSDF, "BSDF->");
	albedo_pin = &inputs[0];
	normal_pin = &inputs[1];
}

ColoredMetal::ColoredMetal(const json& j) : MaterialNode(j)
{
	type = ColoredMetalType;
	albedo_pin = &inputs[0];
	normal_pin = &inputs[1];

	for (size_t i = 0; i < j["albedo"].size(); i++) {
		albedo.data[i] = j["albedo"][i];
	}
}
void ColoredMetal::DumpJson(json& j) const
{
	for (const auto& d : albedo.data) {
		j["albedo"].push_back(d);
	}
	DumpIO(j);
}

void ColoredMetal::Render(void)
{
	ImGui::PushID(iid);
	ax::NodeEditor::BeginNode(id);
	ImGui::Text("%s", name.c_str());
	if (albedo_pin->connected_links.empty()) {
		ImGui::Text("albedo");
		RenderSpectrum(albedo, 0.0, 1.0);
	}
	RenderPins();
	ax::NodeEditor::EndNode();
	ImGui::PopID();
}
void ColoredMetal::PreProcess(HitRecord& rec) const
{
	vec3 new_normal;
	UpdateNormal(normal_pin, rec, new_normal);
	rec.normal = new_normal;
}

bool ColoredMetal::Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& bxdf_divided_by_pdf, double& BxDF, double& pdfval) const
{
	wli = wlo;

	vi[0] = -vo[0];
	vi[1] = -vo[1];
	vi[2] = vo[2];
	pdfval = std::numeric_limits<double>::infinity();
	if (albedo_pin->connected_links.empty()) {
		BxDF = albedo.get(wli)/abs(vo.z());
	} else {

		if (albedo_pin->connected_links.size() != 1) {
			std::cout << "node connection error" << std::endl;
			return 0.0;
		}

		const PinInfo *connected_pin = albedo_pin->connected_links[0]->input;
		assert(connected_pin != nullptr);
		const MaterialNode *parent = connected_pin->parent_node;
		Spectrum albedo;
		parent->Compute(albedo);
		BxDF = std::numeric_limits<double>::infinity();
		bxdf_divided_by_pdf = albedo.get(wli)/M_PI;
	}

	return true;
}
double ColoredMetal::BxDF(const vec3& vi, double wli, const vec3& vo, double wlo, const vec3& vt) const
{
	if (-vi[0] == vo[0] && -vi[1] == vo[1] && vi[2] == vo[2])
		return std::numeric_limits<double>::infinity();
	return 0.0;
}
double ColoredMetal::PDF(const vec3& vi, double wli, const vec3& vo, double wlo, const vec3& vt) const
{
	if (-vi[0] == vo[0] && -vi[1] == vo[1] && vi[2] == vo[2])
		return std::numeric_limits<double>::infinity();
	return 0.0;
}


GGXReflection::GGXReflection(int &unique_id, const char *name) : MaterialNode(unique_id, name)
{
	type = GGXReflectionType;
	AddInput(unique_id, PinSpectrum, "->n");
	AddInput(unique_id, PinSpectrum, "->k");
	AddInput(unique_id, PinDouble, "->roughness");
	AddOutput(unique_id, PinBSDF, "BSDF->");
	n_pin = &inputs[0];
	k_pin = &inputs[1];
}
GGXReflection::GGXReflection(const json& j) : MaterialNode(j)
{
	type = GGXReflectionType;
	for (size_t i = 0; i < j["n"]; i++) {
		n.data[i] = j["n"][i];
	}
	for (size_t i = 0; i < j["k"]; i++) {
		k.data[i] = j["k"][i];
	}
}

void GGXReflection::DumpJson(json& j) const
{
	DumpSpectrum(j, n, "n");
	DumpSpectrum(j, k, "k");
	DumpIO(j);
}

void GGXReflection::Render(void)
{
	ImGui::PushID(iid);
	ax::NodeEditor::BeginNode(id);
	ImGui::Text("%s", name.c_str());
	if (n_pin->connected_links.empty()) {
		ImGui::Text("n");
		RenderSpectrum(n, 0.0, 1.0);
	}
	if (k_pin->connected_links.empty()) {
		ImGui::Text("k");
		RenderSpectrum(n, 0.0, 1.0);
	}
	RenderPins();
	ax::NodeEditor::EndNode();
	ImGui::PopID();
}

void GGXReflection::PreProcess(HitRecord& rec) const
{
}
bool GGXReflection::Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& bxdf_divided_by_pdf, double& BxDF, double& pdfval) const
{
	double phi = 2 * M_PI * drand48();
	double u = drand48();
	double theta = acos(sqrt((1.0-u)/((alpha*alpha-1.0)*u+1)));
	double tan_theta = tan(theta);
	double cos_theta_squared = cos(theta)*cos(theta);
	double sin_theta = sin(theta);

	vec3 micro_normal;
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
	BxDF = fresnel*g*d/(4.0 * abs(vi.z()*vo.z()));
	//double g = 1.0/(1.0+lambda(vi)) * 1.0/(1.0+lambda(vo));
	//BxDF = fresnel * g * abs(dot(vo, micro_normal));
	//bxdf_divided_by_pdf = fresnel * g * abs(dot(vo, micro_normal)) / (abs(vo.z()*vi.z()*micro_normal.z()));
	bxdf_divided_by_pdf = fresnel * g * abs(dot(vo, micro_normal)) / (abs(vo.z()*vi.z()*micro_normal.z()));

	return true;
}
double GGXReflection::BxDF(const vec3& vi, double wli, const vec3& vo, double wlo, const vec3& vt) const
{
}
double GGXReflection::PDF(const vec3& vi, double wli, const vec3& vo, double wlo, const vec3& vt) const
{
}

DiffuseLightNode::DiffuseLightNode(int &unique_id, const char *name) : MaterialNode(unique_id, name)
{
	type = DiffuseLightType;
	AddInput(unique_id, PinSpectrum, "->color");
	AddInput(unique_id, PinVec3, "->normal");
	AddOutput(unique_id, PinBSDF, "BSDF->");
	color_pin = &inputs[0];
	normal_pin = &inputs[1];
}

DiffuseLightNode::DiffuseLightNode(const json& j) : MaterialNode(j)
{
	type = DiffuseLightType;
	for (size_t i = 0; i < j["color"].size(); i++) {
		color.data[i] = j["color"][i];
	}
	color_pin = &inputs[0];
	normal_pin = &inputs[1];
}
void DiffuseLightNode::DumpJson(json& j) const
{
	DumpSpectrum(j, color, "color");
	DumpIO(j);
}
void DiffuseLightNode::Render(void)
{
	ImGui::PushID(iid);
	ax::NodeEditor::BeginNode(id);
	ImGui::Text("%s", name.c_str());
	if (color_pin->connected_links.empty()) {
		ImGui::Text("color");
		RenderSpectrum(color, 0.0, 1.0);
	}
	RenderPins();
	ax::NodeEditor::EndNode();
	ImGui::PopID();
}
void DiffuseLightNode::PreProcess(HitRecord &rec) const
{
	vec3 new_normal;
	UpdateNormal(normal_pin, rec, new_normal);
	rec.normal = new_normal;
}
double DiffuseLightNode::Emitted(const ray& r, const HitRecord& rec, const vec3& vt) const
{
	if (color_pin->connected_links.empty())
		return color.integrate(r.min_wl, r.max_wl);
	else {
		const MaterialNode *node = color_pin->connected_links[0]->input->parent_node;
		Spectrum color;
		node->Compute(color);
		return color.integrate(r.min_wl, r.max_wl);
	}
}

/*
MixBSDFNode::MixBSDFNode(int &unique_id, const char *name) : MaterialNode(unique_id, name)
{
	type = MixBSDFType;
	AddInput(unique_id, PinBSDF, "->In0");
	AddInput(unique_id, PinBSDF, "->In1");
	AddOutput(unique_id, PinBSDF, "Out->");
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
	if (inputs[i].connected_links[0]->input->type != PinBSDF) {
		std::cout << "Error" << std::endl;
	}
	dynamic_cast<const Material *>(inputs[i].connected_links[0]->input->parent_node)->PreProcess(rec);
}

bool MixBSDFNode::Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const
{
	size_t i;
	if (drand48() < ratio) {
		i = 0;
	} else {
		i = 1;
	}
	assert(inputs[i].connected_links.size() == 1);
	if (inputs[i].connected_links[0]->input->type != PinBSDF) {
		std::cout << "Error" << std::endl;
	}
	return dynamic_cast<const Material *>(inputs[i].connected_links[0]->input->parent_node)->Sample(rec, uvw, vo, wlo, vi, wli, BxDF, pdfval);
}

double MixBSDFNode::BxDF(const vec3& vi, double wli, const vec3& vo, double wlo, const vec3& vt) const
{
	size_t i;
	if (drand48() < ratio) {
		i = 0;
	} else {
		i = 1;
	}
	assert(inputs[i].connected_links.size() == 1);
	if (inputs[i].connected_links[0]->input->type != PinBSDF) {
		std::cout << "Error" << std::endl;
	}
	return dynamic_cast<const Material *>(inputs[i].connected_links[0]->input->parent_node)->BxDF(vi, wli, vo, wlo, vt);
}

void MixBSDFNode::Render(void)
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

OutputNode::OutputNode(int &unique_id, const char *name) : MaterialNode(unique_id, name)
{
	type = OutputType;
	AddInput(unique_id, PinBSDF, "->BSDF");
}
OutputNode::OutputNode(const json& j) : MaterialNode(j)
{
	type = OutputType;
}
void OutputNode::Render(void)
{
	ax::NodeEditor::BeginNode(id);
	ImGui::Text("Final Output");
	//ImGui::Text(name.c_str());
	RenderPins();
	ax::NodeEditor::EndNode();
}

SpectrumNode::SpectrumNode(int &unique_id, const char *name) : MaterialNode(unique_id, name)
{
	type = SpectrumType;
	AddOutput(unique_id, PinSpectrum, "Out->");
}
SpectrumNode::SpectrumNode(const json& j) : MaterialNode(j)
{
	type = SpectrumType;
	for (size_t i = 0; i < j["data"].size(); i++) {
		data.data[i] = j["data"][i];
	}
}
void SpectrumNode::DumpJson(json& j) const
{
	DumpSpectrum(j, data, "data");
	DumpIO(j);
}
void SpectrumNode::Render(void)
{
	ImGui::PushID(iid);
	ax::NodeEditor::BeginNode(id);
	ImGui::Text("Spectrum");
	ImGui::Text("%s", name.c_str());
	RenderSpectrum(data, 0.0, 1.0);

	RenderPins();
	ax::NodeEditor::EndNode();
	ImGui::PopID();
}


void SpectrumNode::Compute(Spectrum& data) const
{
	data = this->data;
}

FixedValueNode::FixedValueNode(int &unique_id, PinType type, const char *name) : MaterialNode(unique_id, name)
{
	this->type = FixedValueType;
	AddOutput(unique_id, type, "Out->");
}
FixedValueNode::FixedValueNode(const json& j) : MaterialNode(j)
{
	this->type = FixedValueType;
}

void FixedValueNode::Compute(double& data) const
{
	data = dvalue;
}
void FixedValueNode::Compute(vec3& data) const
{
	data = vvalue;
}
void FixedValueNode::Compute(Spectrum& data) const
{
	data = svalue;
}


RGBColorNode::RGBColorNode(int &unique_id, const char *name) : MaterialNode(unique_id, name)
{
	type = RGBColorType;
	AddOutput(unique_id, PinVec3, "RGB->");
}
RGBColorNode::RGBColorNode(const json& j) : MaterialNode(j)
{
	type = RGBColorType;
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
void RGBColorNode::Render(void)
{
	ImGui::PushID(iid);
	ax::NodeEditor::BeginNode(id);
	ImGui::Text("%s", name.c_str());
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.15f);
	ImGui::ColorPicker3("Color", col);
	ImGui::PopItemWidth();

	RenderPins();

	ax::NodeEditor::EndNode();
	ImGui::PopID();
}

void RGBColorNode::Compute(vec3& data) const
{
	data[0] = col[0];
	data[1] = col[1];
	data[2] = col[2];
}


RGBtoSpectrumNode::RGBtoSpectrumNode(int &unique_id, const char *name) : MaterialNode(unique_id, name)
{
	type = RGBtoSpectrumType;
	AddInput(unique_id, PinVec3, "->RGB");
	AddOutput(unique_id, PinSpectrum, "Spectrum->");
}
RGBtoSpectrumNode::RGBtoSpectrumNode(const json& j) : MaterialNode(j)
{
	type = RGBtoSpectrumType;
}
void RGBtoSpectrumNode::Compute(Spectrum& data) const
{
	vec3 RGB;
	assert(inputs[0].connected_links.size() == 1);
	assert(inputs[0].connected_links[0]->input != nullptr);
	inputs[0].connected_links[0]->input->parent_node->Compute(RGB);
	data = RGBtoSpectrum(RGB);
}

ImageTextureNode::ImageTextureNode(int &unique_id, const char *path, const char *name) : MaterialNode(unique_id, name)
{
	type = ImageTextureType;
	this->path = std::string(path);
	AddInput(unique_id, PinVec3, "->UV");
	AddOutput(unique_id, PinVec3, "Out->");
}
ImageTextureNode::ImageTextureNode(const json& j) : MaterialNode(j)
{
	type = ImageTextureType;
	path = j["path"].get<std::string>();
	if (path != "")
		texture = stbi_load(path.c_str(), &width, &height, &bpp, 0);
}
void ImageTextureNode::DumpJson(json& j) const
{
	j["path"] = path;
	DumpIO(j);
}

void ImageTextureNode::Render(void)
{
	ImGui::PushID(iid);
	ax::NodeEditor::BeginNode(id);
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
	RenderPins();
	ax::NodeEditor::EndNode();
	ImGui::PopID();
}
void ImageTextureNode::Compute(vec3& data) const
{
	if (texture == nullptr)
		return;

	assert(inputs[0].connected_links.size() == 1);
	assert(inputs[0].connected_links[0]->input != nullptr);
	vec3 vt;
	inputs[0].connected_links[0]->input->parent_node->Compute(vt);


	int x = static_cast<double>(width) * vt[0];
	int y = static_cast<double>(height) * (1.0-vt[1]);
	data[0] = static_cast<double>(texture[bpp*(x+y*width)])/255.0;
	data[1] = static_cast<double>(texture[bpp*(x+y*width)+1])/255.0;
	data[2] = static_cast<double>(texture[bpp*(x+y*width)+2])/255.0;
}


CheckerboardNode::CheckerboardNode(int &unique_id, const char *name) : MaterialNode(unique_id, name)
{
	type = CheckerboardType;
	AddInput(unique_id, PinVec3, "UV->");
	AddOutput(unique_id, PinVec3, "Out->");
}
CheckerboardNode::CheckerboardNode(const json& j) : MaterialNode(j)
{
	type = CheckerboardType;
	size = j["size"];
}
void CheckerboardNode::DumpJson(json& j) const
{
	j["size"] = size;
	DumpIO(j);
}
void CheckerboardNode::Compute(vec3& data) const
{
	assert(inputs[0].connected_links.size() == 1);
	assert(inputs[0].connected_links[0]->input != nullptr);
	vec3 vt;
	inputs[0].connected_links[0]->input->parent_node->Compute(vt);
	int x = (vt[0] - fmod(vt[0], size))/size;
	int y = (vt[1] - fmod(vt[1], size))/size;
	if (x%2 == y%2)
		data = vec3(1.0, 1.0, 1.0);
	else
		data = vec3(0.0, 0.0, 0.0);
}


void CheckerboardNode::Render(void)
{
	ImGui::PushID(iid);
	ax::NodeEditor::BeginNode(id);
	ImGui::Text("%s", name.c_str());
	const double min = 0.001;
	const double max = 1.0;
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.25f);
	ImGui::SliderScalar("size", ImGuiDataType_Double, &size, &min, &max, "%f");
	ImGui::PopItemWidth();
	RenderPins();
	ax::NodeEditor::EndNode();
	ImGui::PopID();
}


AdditionNode::AdditionNode(int &unique_id, const char *name) : MaterialNode(unique_id, name)
{
	type = AdditionType;
	AddInput(unique_id, PinUniversal, "->In0");
	AddInput(unique_id, PinUniversal, "->In1");
	AddOutput(unique_id, PinUniversal, "Out->");
}
AdditionNode::AdditionNode(const json& j) : MaterialNode(j)
{
	type = AdditionType;
}

void AdditionNode::Compute(double &data) const
{
	data = 0;
	for (size_t i = 0; i < 2; i++) {
		assert(inputs[i].connected_links.size() == 1);
		if (inputs[i].connected_links[0]->input->type != PinDouble) {
			std::cout << "Error" << std::endl;
		}
		double d;
		inputs[i].connected_links[0]->input->parent_node->Compute(d);
		data += d;
	}
}
void AdditionNode::Compute(vec3& data) const
{
	data = vec3(0.0, 0.0, 0.0);
	for (size_t i = 0; i < 2; i++) {
		assert(inputs[i].connected_links.size() == 1);
		if (inputs[i].connected_links[0]->input->type != PinVec3) {
			std::cout << "Error" << std::endl;
		}
		vec3 d;
		inputs[i].connected_links[0]->input->parent_node->Compute(d);
		data += d;
	}
}
void AdditionNode::Compute(Spectrum& data) const
{
	data = Spectrum(0.0);
	for (size_t i = 0; i < 2; i++) {
		assert(inputs[i].connected_links.size() == 1);
		if (inputs[i].connected_links[0]->input->type != PinSpectrum) {
			std::cout << "Error" << std::endl;
		}
		Spectrum d;
		inputs[i].connected_links[0]->input->parent_node->Compute(d);
		data = data + d;
	}
}


ScalarMultiplicationNode::ScalarMultiplicationNode(int &unique_id, const char *name) : MaterialNode(unique_id, name)
{
	type = ScalarMultiplicationType;
	AddInput(unique_id, PinUniversal, "->In");
	AddOutput(unique_id, PinUniversal, "Out->");
}
ScalarMultiplicationNode::ScalarMultiplicationNode(const json& j) : MaterialNode(j)
{
	type = ScalarMultiplicationType;
	scale = j["scale"];
}
void ScalarMultiplicationNode::DumpJson(json& j) const
{
	j["scale"] = scale;
	DumpIO(j);
}
void ScalarMultiplicationNode::Render(void)
{
	ImGui::PushID(iid);
	ax::NodeEditor::BeginNode(id);
	ImGui::Text("%s", name.c_str());
	const double min = -100.0;
	const double max = 100.0;
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.25f);
	ImGui::SliderScalar("scale", ImGuiDataType_Double, &scale, &min, &max, "%f");
	ImGui::PopItemWidth();
	RenderPins();
	ax::NodeEditor::EndNode();
	ImGui::PopID();
}

void ScalarMultiplicationNode::Compute(double &data) const
{
	assert(inputs[0].connected_links.size() == 1);
	if (inputs[0].connected_links[0]->input->type != PinDouble) {
		std::cout << "Error" << std::endl;
	}
	double d;
	inputs[0].connected_links[0]->input->parent_node->Compute(d);
	data = d * scale;
}
void ScalarMultiplicationNode::Compute(vec3& data) const
{
	assert(inputs[0].connected_links.size() == 1);
	if (inputs[0].connected_links[0]->input->type != PinVec3) {
		std::cout << "Error" << std::endl;
	}
	vec3 d;
	inputs[0].connected_links[0]->input->parent_node->Compute(d);
	data = d * scale;
}
void ScalarMultiplicationNode::Compute(Spectrum& data) const
{
	assert(inputs[0].connected_links.size() == 1);
	if (inputs[0].connected_links[0]->input->type != PinSpectrum) {
		std::cout << "Error" << std::endl;
	}
	Spectrum d;
	inputs[0].connected_links[0]->input->parent_node->Compute(d);
	data = d * scale;
}

MultiplicationNode::MultiplicationNode(int &unique_id, const char *name) : MaterialNode(unique_id, name)
{
	type = MultiplicationType;
	AddInput(unique_id, PinUniversal, "->In0");
	AddInput(unique_id, PinUniversal, "->In1");
	AddOutput(unique_id, PinUniversal, "Out->");
}
MultiplicationNode::MultiplicationNode(const json& j) : MaterialNode(j)
{
	type = MultiplicationType;
}
void MultiplicationNode::Compute(double &data) const
{
	data = 1.0;
	for (size_t i = 0; i < 2; i++) {
		assert(inputs[i].connected_links.size() == 1);
		if (inputs[i].connected_links[0]->input->type != PinDouble) {
			std::cout << "Error" << std::endl;
		}
		double d;
		inputs[i].connected_links[0]->input->parent_node->Compute(d);
		data *= d;
	}
}
void MultiplicationNode::Compute(vec3& data) const
{
	data = vec3(1.0, 1.0, 1.0);
	for (size_t i = 0; i < 2; i++) {
		assert(inputs[i].connected_links.size() == 1);
		if (inputs[i].connected_links[0]->input->type != PinVec3) {
			std::cout << "Error" << std::endl;
		}
		vec3 d;
		inputs[i].connected_links[0]->input->parent_node->Compute(d);
		data *= d;
	}
}
void MultiplicationNode::Compute(Spectrum& data) const
{
	data = Spectrum(1.0);
	for (size_t i = 0; i < 2; i++) {
		assert(inputs[i].connected_links.size() == 1);
		if (inputs[i].connected_links[0]->input->type != PinSpectrum) {
			std::cout << "Error" << std::endl;
		}
		Spectrum d;
		inputs[i].connected_links[0]->input->parent_node->Compute(d);
		for (size_t i = 0; i < data.data.size(); i++) {
			data.data[i] *= d.data[i];
		}
	}
}


RandomSamplingNode::RandomSamplingNode(int &unique_id, const char *name) : MaterialNode(unique_id, name)
{
	type = RandomSamplingType;
	AddInput(unique_id, PinUniversal, "->In0");
	AddInput(unique_id, PinUniversal, "->In1");
	AddOutput(unique_id, PinUniversal, "Out->");
}
RandomSamplingNode::RandomSamplingNode(const json& j) : MaterialNode(j)
{
	type = RandomSamplingType;
	ratio = j["ratio"];
}
void RandomSamplingNode::DumpJson(json& j) const
{
	j["ratio"] = ratio;
	DumpIO(j);
}
void RandomSamplingNode::Compute(double &data) const
{
	size_t i;
	if (drand48() < ratio) {
		i = 0;
	} else {
		i = 1;
	}
	assert(inputs[i].connected_links.size() == 1);
	if (inputs[i].connected_links[0]->input->type != PinDouble) {
		std::cout << "Error" << std::endl;
	}
	inputs[i].connected_links[0]->input->parent_node->Compute(data);
}
void RandomSamplingNode::Compute(vec3& data) const
{
	size_t i;
	if (drand48() < ratio) {
		i = 0;
	} else {
		i = 1;
	}
	assert(inputs[i].connected_links.size() == 1);
	if (inputs[i].connected_links[0]->input->type != PinVec3) {
		std::cout << "Error" << std::endl;
	}
	inputs[i].connected_links[0]->input->parent_node->Compute(data);
}
void RandomSamplingNode::Compute(Spectrum& data) const
{
	size_t i;
	if (drand48() < ratio) {
		i = 0;
	} else {
		i = 1;
	}
	assert(inputs[i].connected_links.size() == 1);
	if (inputs[i].connected_links[0]->input->type != PinSpectrum) {
		std::cout << "Error" << std::endl;
	}
	inputs[i].connected_links[0]->input->parent_node->Compute(data);
}

void RandomSamplingNode::Render(void)
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


NodeMaterial::NodeMaterial(void) : context(ax::NodeEditor::CreateEditor())
{
	material_nodes.push_back(new FixedValueNode(unique_id, PinVec3, "UV"));
	material_nodes.push_back(new OutputNode(unique_id));
}
NodeMaterial::NodeMaterial(const json& j) : context(ax::NodeEditor::CreateEditor())
{
	name = j["name"].get<std::string>();
	bool is_there_any_widgets = false;
	for (const auto& node_j : j["nodes"]) {
		is_there_any_widgets = true;
		MaterialNode *node = nullptr;
		switch (static_cast<MaterialNodeType>(node_j["type"].get<int>())) {
			case LambertianType:
				node = new LambertianNode(node_j);
				break;
			case ConductorType:
				node = new ConductorNode(node_j);
				break;
			case ColoredMetalType:
				node = new ColoredMetal(node_j);
				break;
			case DiffuseLightType:
				node = new DiffuseLightNode(node_j);
				break;
			//case MixBSDFType:
			//	node = new MixBSDFNode(node_j);
			//	break;
			case OutputType:
				node = new OutputNode(node_j);
				break;
			case SpectrumType:
				node = new SpectrumNode(node_j);
				break;
			case RGBColorType:
				node = new RGBColorNode(node_j);
				break;
			case RGBtoSpectrumType:
				node = new RGBtoSpectrumNode(node_j);
				break;
			case ImageTextureType:
				node = new ImageTextureNode(node_j);
				break;
			case CheckerboardType:
				node = new CheckerboardNode(node_j);
				break;
			case AdditionType:
				node = new AdditionNode(node_j);
				break;
			case MultiplicationType:
				node = new MultiplicationNode(node_j);
				break;
			case ScalarMultiplicationType:
				node = new ScalarMultiplicationNode(node_j);
				break;
			case RandomSamplingType:
				node = new RandomSamplingNode(node_j);
				break;
			case FixedValueType:
				node = new FixedValueNode(node_j);
				break;
			default:
				std::cout << "Error: Unimplemented" << std::endl;
				assert(false);
				break;
		}
		unique_id = std::max(unique_id, node->iid);
		for (const auto pin : node->inputs) {
			unique_id = std::max(unique_id, pin.iid);
		}
		for (const auto pin : node->outputs) {
			unique_id = std::max(unique_id, pin.iid);
		}
		material_nodes.push_back(node);
	}
	for (const auto& link_j : j["links"]) {
		is_there_any_widgets = true;
		LinkInfo *new_link = new LinkInfo(link_j, FindPin(link_j["input_id"]), FindPin(link_j["output_id"]));
		links.push_back(new_link);
		auto input = FindPin(link_j["input_id"]);
		auto output = FindPin(link_j["output_id"]);
		input->connected_links.push_back(new_link);
		output->connected_links.push_back(new_link);
		unique_id = std::max(unique_id, new_link->iid);
	}
	if (is_there_any_widgets)
		unique_id++;
}

void NodeMaterial::DumpJson(json& j) const
{
	j["name"] = name;
	for (const auto& node : material_nodes) {
		json node_j;
		node->DumpJson(node_j);
		j["nodes"].push_back(node_j);
	}
	for (const auto& link : links) {
		json link_j;
		link->DumpJson(link_j);
		j["links"].push_back(link_j);
	}
}

void NodeMaterial::Render(void)
{

	for (auto& node : material_nodes) {
		node->Render();
	}

	for (auto& link_info : links) {
		ed::Link(link_info->id, link_info->input_id, link_info->output_id);
	}

	if (ed::BeginCreate()) {
		ed::PinId inputpin_id, outputpin_id;
		ed::PinId pin_id;
		if (ed::QueryNewLink(&inputpin_id, &outputpin_id)) {
			if (inputpin_id && outputpin_id) {
				auto input = FindPin(inputpin_id);
				auto output = FindPin(outputpin_id);
				if (inputpin_id == outputpin_id) {
					ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
				} else if (input->type != PinUniversal &&
						output->type != PinUniversal &&
						input->type != output->type) {
					ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
				} else if (input->io_type == output->io_type) {
					ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
				} else {
					if (input->io_type == PinInput) {
						std::swap(input, output);
						std::swap(inputpin_id, outputpin_id);
					}

					if (!output->connected_links.empty()) {
						ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
					} else if (ed::AcceptNewItem()) {
						AddLink(input, output);
						ed::Link(links.back()->id, links.back()->input_id, links.back()->output_id);
					}
				}
			}
		} else if (ed::QueryNewNode(&pin_id)) {
			if (ed::AcceptNewItem()) {
				ed::Suspend();
				ImGui::OpenPopup("Create New Node");
				ed::Resume();
			}
		}
	}
	ed::EndCreate();

	if (ed::BeginDelete()) {
		ed::LinkId deleted_link_id;
		while (ed::QueryDeletedLink(&deleted_link_id)) {
			if (ed::AcceptDeletedItem()) {
				struct LinkInfo **l = nullptr;
				size_t i;
				for (i = 0; i < links.size(); i++) {
					if (links[i]->id == deleted_link_id) {
						l = &links[i];
						break;
					}
				}
				assert(l != nullptr);
				{ // delete an element of connected_links in PinInfo 
					auto pin = (*l)->input;
					auto it = std::find(pin->connected_links.begin(), pin->connected_links.end(), *l);
					assert(it != pin->connected_links.end());
					pin->connected_links.erase(it);

					pin = (*l)->output;
					it = std::find(pin->connected_links.begin(), pin->connected_links.end(), *l);
					assert(it != pin->connected_links.end());
					pin->connected_links.erase(it);

				}
				assert(i < links.size());
				auto it = links.begin() + i;
				links.erase(it);
			}

		}
		ed::NodeId deleted_node_id;
		while (ed::QueryDeletedNode(&deleted_node_id)) {
			if (ed::AcceptDeletedItem()) {
				size_t i;
				for (i = 0; i < material_nodes.size(); i++) {
					if (material_nodes[i]->id == deleted_node_id) {
						break;
					}
				}
				assert(i < material_nodes.size());
				auto it = material_nodes.begin() + i;
				material_nodes.erase(it);
			}
		}
	}
	ed::EndDelete();


	ed::Suspend();
	if (ImGui::BeginPopup("Create New Node")) {
		ImGui::Text("Select a nodetype");
		MaterialNode *node = nullptr;
		if (ImGui::MenuItem("Lambertian")) {
			node = new LambertianNode(unique_id);
		} else if (ImGui::MenuItem("Conductor")) {
			node = new ConductorNode(unique_id);
		} else if (ImGui::MenuItem("Colored Metal")) {
			node = new ColoredMetal(unique_id);
		} else if (ImGui::MenuItem("Diffuse Light")) {
			node = new DiffuseLightNode(unique_id);
		//} else if (ImGui::MenuItem("Mix BSDF")) {
		//	node = new MixBSDFNode(unique_id);
		} else if (ImGui::MenuItem("Output")) {
			node = new OutputNode(unique_id);
		} else if (ImGui::MenuItem("Spectrum Node")) {
			node = new SpectrumNode(unique_id);
		} else if (ImGui::MenuItem("RGB Color Node")) {
			node = new RGBColorNode(unique_id);
		} else if (ImGui::MenuItem("RGBtoSpectrum Node")) {
			node = new RGBtoSpectrumNode(unique_id);
		} else if (ImGui::MenuItem("ImageTexture Node")) {
			node = new ImageTextureNode(unique_id);
		} else if (ImGui::MenuItem("Checkerboard Node")) {
			node = new CheckerboardNode(unique_id);
		} else if (ImGui::MenuItem("Addition Node")) {
			node = new AdditionNode(unique_id);
		} else if (ImGui::MenuItem("Multiplication Node")) {
			node = new MultiplicationNode(unique_id);
		} else if (ImGui::MenuItem("Scalar Multiplication Node")) {
			node = new ScalarMultiplicationNode(unique_id);
		} else if (ImGui::MenuItem("Random Sampling Node")) {
			node = new RandomSamplingNode(unique_id);
		}
		if (node != nullptr)
			material_nodes.push_back(node);
		ImGui::EndPopup();
	}
	ed::Resume();


}


void NodeMaterial::PreProcess(HitRecord& rec) const
{
	dynamic_cast<FixedValueNode *>(material_nodes[uv_i])->vvalue = rec.vt;

	const PinInfo *bsdf_pin = &material_nodes[Output_i]->inputs[0];
	if (bsdf_pin->connected_links.size() != 1) {
		std::cout << "node connection error" << std::endl;
		return;
	}
	const PinInfo *connected_pin = bsdf_pin->connected_links[0]->input;
	assert(connected_pin != nullptr);
	const MaterialNode *parent = connected_pin->parent_node;

	auto p = dynamic_cast<const Material *>(parent);
	p->PreProcess(rec);
}
bool NodeMaterial::Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& bxdf_divided_by_pdf, double& BxDF, double& pdfval) const
{
	const PinInfo *bsdf_pin = &material_nodes[Output_i]->inputs[0];
	if (bsdf_pin->connected_links.size() != 1) {
		std::cout << "node connection error" << std::endl;
		return 0.0;
	}
	const PinInfo *connected_pin = bsdf_pin->connected_links[0]->input;
	assert(connected_pin != nullptr);
	const MaterialNode *parent = connected_pin->parent_node;

	auto p = dynamic_cast<const Material *>(parent);
	return p->Sample(rec, uvw, vo, wlo, vi, wli, bxdf_divided_by_pdf, BxDF, pdfval);
}
double NodeMaterial::BxDF(const vec3& vi, double wli, const vec3& vo, double wlo, const vec3& vt) const
{

	dynamic_cast<FixedValueNode *>(material_nodes[uv_i])->vvalue = vt;
	const PinInfo *bsdf_pin = &material_nodes[Output_i]->inputs[0];
	if (bsdf_pin->connected_links.size() != 1) {
		std::cout << "node connection error" << std::endl;
		return 0.0;
	}

	const PinInfo *connected_pin = FindPinConst(bsdf_pin->connected_links[0]->input_id);
	assert(connected_pin != nullptr);
	const MaterialNode *parent = connected_pin->parent_node;

	auto p = dynamic_cast<const Material *>(parent);
	return p->BxDF(vi, wli, vo, wlo, vt);
}
double NodeMaterial::PDF(const vec3& vi, double wli, const vec3& vo, double wlo, const vec3& vt) const
{

	dynamic_cast<FixedValueNode *>(material_nodes[uv_i])->vvalue = vt;
	const PinInfo *bsdf_pin = &material_nodes[Output_i]->inputs[0];
	if (bsdf_pin->connected_links.size() != 1) {
		std::cout << "node connection error" << std::endl;
		return 0.0;
	}

	const PinInfo *connected_pin = FindPinConst(bsdf_pin->connected_links[0]->input_id);
	assert(connected_pin != nullptr);
	const MaterialNode *parent = connected_pin->parent_node;

	auto p = dynamic_cast<const Material *>(parent);
	return p->PDF(vi, wli, vo, wlo, vt);
}
double NodeMaterial::Emitted(const ray& r, const HitRecord& rec, const vec3& vt) const
{
	dynamic_cast<FixedValueNode *>(material_nodes[uv_i])->vvalue = vt;
	const PinInfo *bsdf_pin = &material_nodes[Output_i]->inputs[0];
	if (bsdf_pin->connected_links.size() != 1) {
		std::cout << "node connection error" << std::endl;
		return 0.0;
	}

	const PinInfo *connected_pin = FindPinConst(bsdf_pin->connected_links[0]->input_id);
	assert(connected_pin != nullptr);
	const MaterialNode *parent = connected_pin->parent_node;

	auto p = dynamic_cast<const Material *>(parent);
	return p->Emitted(r, rec, vt);
}

