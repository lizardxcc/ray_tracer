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
};


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

void MaterialNode::RenderPins(void)
{
	for (size_t i = 0; i < inputs.size() || i < outputs.size(); i++) {
		if (i < inputs.size()) {
			ax::NodeEditor::BeginPin(inputs[i].id, ax::NodeEditor::PinKind::Input);
			ImGui::TextColored(pin_colors[inputs[i].type], inputs[i].name.c_str());
			ax::NodeEditor::EndPin();
		}
		if (i < inputs.size() && i < outputs.size())
			ImGui::SameLine();
		if (i < outputs.size()) {
			ax::NodeEditor::BeginPin(outputs[i].id, ax::NodeEditor::PinKind::Output);
			ImGui::TextColored(pin_colors[outputs[i].type], outputs[i].name.c_str());
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
	ImGui::Text(name.c_str());

	RenderPins();

	ax::NodeEditor::EndNode();
}


void MaterialNode::AddInput(int &unique_id, PinType type, const char *name)
{
	PinInfo input;
	input.id = unique_id++;
	input.type = type;
	input.io_type = PinInput;
	input.name = name;
	input.parent_node = this;
	inputs.push_back(input);
}


void MaterialNode::AddOutput(int &unique_id, PinType type, const char *name)
{
	PinInfo output;
	output.id = unique_id++;
	output.type = type;
	output.io_type = PinOutput;
	output.name = name;
	output.parent_node = this;
	outputs.push_back(output);
}

void LambertianNode::Render(void)
{
	ImGui::PushID(iid);
	ax::NodeEditor::BeginNode(id);
	ImGui::Text(name.c_str());
	if (albedo_pin->connected_links.empty())
		RenderSpectrum(albedo, 0.0, 1.0);

	RenderPins();
	ax::NodeEditor::EndNode();
	ImGui::PopID();
}

bool LambertianNode::PreProcess(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const
{
	CosinePdf Pdf(rec.normal);

	vec3 generated_direction = Pdf.Generate();
	pdfval = Pdf.PdfVal(generated_direction);
	vi = uvw.WorldToLocal(generated_direction);
	wli = wlo;
	return true;
}

bool LambertianNode::Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const
{
	BxDF = this->BxDF(vi, wli, vo, wlo);
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

void ConductorNode::Render(void)
{
	ImGui::PushID(iid);
	ax::NodeEditor::BeginNode(id);
	ImGui::Text("Conductor");
	ImGui::Text(name.c_str());
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
bool ConductorNode::PreProcess(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const
{
	wli = wlo;
	return true;
}
bool ConductorNode::Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const
{
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
	pdfval = 1.0;
	BxDF = fresnel/cos_o;


	return true;
}
double ConductorNode::BxDF(const vec3& vi, double wli, const vec3& vo, double wlo, const vec3& vt) const
{
	return 0.0;
}

void OutputNode::Render(void)
{
	ax::NodeEditor::BeginNode(id);
	ImGui::Text("Final Output");
	//ImGui::Text(name.c_str());
	RenderPins();
	ax::NodeEditor::EndNode();
}

void SpectrumNode::Render(void)
{
	ImGui::PushID(iid);
	ax::NodeEditor::BeginNode(id);
	ImGui::Text("Spectrum");
	ImGui::Text(name.c_str());
	RenderSpectrum(data, 0.0, 1.0);
	
	RenderPins();
	ax::NodeEditor::EndNode();
	ImGui::PopID();
}


void SpectrumNode::Compute(Spectrum& data) const
{
	data = this->data;
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


void RGBColorNode::Render(void)
{
	ImGui::PushID(iid);
	ax::NodeEditor::BeginNode(id);
	ImGui::Text(name.c_str());
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

void RGBtoSpectrumNode::Compute(Spectrum& data) const
{
	vec3 RGB;
	assert(inputs[0].connected_links.size() == 1);
	assert(inputs[0].connected_links[0]->input != nullptr);
	inputs[0].connected_links[0]->input->parent_node->Compute(RGB);
	data = RGBtoSpectrum(RGB);
}

void TextureNode::Render(void)
{
	ax::NodeEditor::BeginNode(id);
	ImGui::Text("Color Texture Node");
	if (path == "") {
		ImGui::Text("texture is not loaded yet");
	} else {
		ImGui::Text(path.c_str());
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
}
void TextureNode::Compute(vec3& data) const
{
	if (texture == nullptr)
		return;

	assert(inputs[0].connected_links.size() == 1);
	assert(inputs[0].connected_links[0]->input != nullptr);
	vec3 vt;
	inputs[0].connected_links[0]->input->parent_node->Compute(vt);


	int x = static_cast<double>(width) * vt[0];
	int y = static_cast<double>(height) * (1.0-vt[1]);
	data[0] = texture[bpp*(x+y*width)];
	data[1] = texture[bpp*(x+y*width)+1];
	data[2] = texture[bpp*(x+y*width)+2];
	data *= 0.001;
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
	ax::NodeEditor::BeginNode(id);
	ImGui::Text(name.c_str());
	const double min = 0.001;
	const double max = 1.0;
	ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.25f);
	ImGui::SliderScalar("size", ImGuiDataType_Double, &size, &min, &max, "%f");
	ImGui::PopItemWidth();
	RenderPins();
	ax::NodeEditor::EndNode();
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

						assert(input != nullptr);
						assert(output != nullptr);
						struct LinkInfo *new_link = new LinkInfo;
						//*newlink = {ed::LinkId(unique_id++), inputpin_id, outputpin_id, input_p, output_p};
						new_link->id = ed::LinkId(unique_id++);
						new_link->input_id = inputpin_id;
						new_link->output_id = outputpin_id;
						new_link->input = input;
						new_link->output = output;
						links.push_back(new_link);
						input->connected_links.push_back(new_link);
						output->connected_links.push_back(new_link);
						ed::Link(new_link->id, new_link->input_id, new_link->output_id);
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
		} else if (ImGui::MenuItem("Output")) {
			node = new OutputNode(unique_id);
		} else if (ImGui::MenuItem("Spectrum Node")) {
			node = new SpectrumNode(unique_id);
		} else if (ImGui::MenuItem("RGB Color Node")) {
			node = new RGBColorNode(unique_id);
		} else if (ImGui::MenuItem("RGBtoSpectrum Node")) {
			node = new RGBtoSpectrumNode(unique_id);
		} else if (ImGui::MenuItem("Texture Node")) {
			node = new TextureNode(unique_id);
		} else if (ImGui::MenuItem("Checkerboard Node")) {
			node = new CheckerboardNode(unique_id);
		} else if (ImGui::MenuItem("Addition Double Node")) {
			node = new AdditionNode(unique_id);
		}
		if (node != nullptr)
			material_nodes.push_back(node);
		ImGui::EndPopup();
	}
	ed::Resume();


}


bool NodeMaterial::Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const
{
	const PinInfo *bsdf_pin = &material_nodes[Output_i]->inputs[0];
	if (bsdf_pin->connected_links.size() != 1) {
		std::cout << "node connection error" << std::endl;
		return 0.0;
	}
	const PinInfo *connected_pin = bsdf_pin->connected_links[0]->input;
	assert(connected_pin != nullptr);
	const MaterialNode *parent = connected_pin->parent_node;

	if (typeid(*parent) == typeid(LambertianNode)) {
		auto p = dynamic_cast<const LambertianNode *>(parent);
		p->PreProcess(rec, uvw, vo, wlo, vi, wli, BxDF, pdfval);
		dynamic_cast<FixedValueNode *>(material_nodes[wl_i])->dvalue = wli;
		dynamic_cast<FixedValueNode *>(material_nodes[uv_i])->vvalue = rec.vt;
		return p->Sample(rec, uvw, vo, wlo, vi, wli, BxDF, pdfval);
	} else if (typeid(*parent) == typeid(ConductorNode)) {
		auto p = dynamic_cast<const ConductorNode *>(parent);
		p->PreProcess(rec, uvw, vo, wlo, vi, wli, BxDF, pdfval);
		dynamic_cast<FixedValueNode *>(material_nodes[wl_i])->dvalue = wli;
		dynamic_cast<FixedValueNode *>(material_nodes[uv_i])->vvalue = rec.vt;
		return p->Sample(rec, uvw, vo, wlo, vi, wli, BxDF, pdfval);
	} else {
		return false;
	}
}
double NodeMaterial::BxDF(const vec3& vi, double wli, const vec3& vo, double wlo, const vec3& vt) const
{

	dynamic_cast<FixedValueNode *>(material_nodes[wl_i])->dvalue = wli;
	dynamic_cast<FixedValueNode *>(material_nodes[uv_i])->vvalue = vt;
	const PinInfo *bsdf_pin = &material_nodes[Output_i]->inputs[0];
	if (bsdf_pin->connected_links.size() != 1) {
		std::cout << "node connection error" << std::endl;
		return 0.0;
	}

	const PinInfo *connected_pin = FindPinConst(bsdf_pin->connected_links[0]->input_id);
	assert(connected_pin != nullptr);
	const MaterialNode *parent = connected_pin->parent_node;

	if (typeid(*parent) == typeid(LambertianNode)) {
		return dynamic_cast<const LambertianNode *>(parent)->BxDF(vi, wli, vo, wlo, vt);
	} else {
		return 0.0;
	}
	return 0.0;
}
double NodeMaterial::Emitted(const ray& r, const HitRecord& rec) const
{
	return 0.0;
}

