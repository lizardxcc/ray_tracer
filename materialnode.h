#ifndef MATERIALNODE_H
#define MATERIALNODE_H

#include "imgui_node_editor.h"
#include "material.h"
#include "json.hpp"
using json = nlohmann::json;

namespace ed = ax::NodeEditor;


class MaterialNode;
struct PinInfo;

struct LinkInfo {
	LinkInfo(int &unique_id, ed::PinId input_id, ed::PinId output_id, PinInfo *input, PinInfo *output) :
		id(unique_id),
		iid(unique_id),
		input_id(input_id),
		output_id(output_id),
		input(input),
		output(output)
	{
		unique_id++;
	}
	LinkInfo(const json& j, PinInfo *input, PinInfo *output) :
		id(j["id"]),
		iid(j["id"]),
		input_id(j["input_id"]),
		output_id(j["output_id"]),
		input(input),
		output(output)
	{
	}

	void DumpJson(json& j) const;
	const ed::LinkId id;
	const int iid;
	const ed::PinId input_id;
	const ed::PinId output_id;
	PinInfo * const input;
	PinInfo * const output;
};

enum PinType {
	PinDouble,
	PinVec3,
	PinSpectrum,
	PinUniversal,
	PinBSDF,
};

enum PinIOType {
	PinInput,
	PinOutput
};



struct PinInfo {
	PinInfo(int &unique_id, PinIOType io_type, PinType type, const char *name, const MaterialNode *parent_node) :
		id(unique_id), iid(unique_id),
		io_type(io_type),
		type(type),
		name(name),
		parent_node(parent_node)
	{
		unique_id++;
	}
	PinInfo(const json& j, const MaterialNode *parent_node) :
		id(j["id"]),
		iid(j["id"]),
		name(j["name"]),
		io_type(static_cast<PinIOType>(j["io_type"])),
		type(static_cast<PinType>(j["type"])),
		parent_node(parent_node)
	{
	}
	const ed::PinId id;
	const int iid;
	const PinIOType io_type;
	const PinType type;
	const std::string name;
	const MaterialNode * const parent_node;
	std::vector<const LinkInfo *> connected_links;
};


enum MaterialNodeType {
	LambertianType,
	ConductorType,
	ColoredMetalType,
	MixBSDFType,
	OutputType,
	FixedValueType,
	SpectrumType,
	RGBColorType,
	RGBtoSpectrumType,
	ImageTextureType,
	CheckerboardType,
	AdditionType,
	MultiplicationType,
	ScalarMultiplicationType,
	RandomSamplingType
};

class MaterialNode {
	public:
		MaterialNode(int &unique_id, const char *name = "") : id(unique_id), iid(unique_id), name(name)
		{
			unique_id++;
		}
		explicit MaterialNode(const json& j) : name(j["name"].get<std::string>()), id(j["id"]), iid(j["id"])
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
		virtual void Render(void);
		void RenderPins(void);
		void RenderSpectrum(Spectrum& data, double min, double max);
		virtual void Compute(double& data) const
		{
		}
		virtual void Compute(Spectrum& data) const
		{
		}
		virtual void Compute(vec3& data) const
		{
		}
		void AddInput(int &unique_id, PinType type, const char *name);
		void AddOutput(int &unique_id, PinType type, const char *name);


		virtual void DumpJson(json& j) const
		{
			DumpIO(j);
		};
		void DumpIO(json &j) const
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
		void DumpSpectrum(json& j, const Spectrum& s, const char *name) const
		{
			for (const auto& d : s.data) {
				j[name].push_back(d);
			}
		}

		std::string name;
		ed::NodeId id;
		int iid;
		enum MaterialNodeType type;
		std::vector<PinInfo> inputs;
		std::vector<PinInfo> outputs;
		const static ImVec4 pin_colors[];
	protected:
		void UpdateNormal(const PinInfo *normal_pin, const HitRecord& rec, vec3& new_normal) const
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
};
class SpectrumNode : public MaterialNode {
	public:
		SpectrumNode(int &unique_id, const char *name = "Spectrum") : MaterialNode(unique_id, name)
		{
			type = SpectrumType;
			AddOutput(unique_id, PinSpectrum, "Out->");
		}
		SpectrumNode(const json& j) : MaterialNode(j)
	{
		type = SpectrumType;
		for (size_t i = 0; i < j["data"].size(); i++) {
			data.data[i] = j["data"][i];
		}
	}
		void DumpJson(json& j) const override
		{
			DumpSpectrum(j, data, "data");
			DumpIO(j);
		}
		void Render(void) override;
		void Compute(Spectrum& data) const override;
	private:
		Spectrum data = Spectrum(1.0);
};

class LambertianNode : public MaterialNode, public Material {
	public:
		LambertianNode(int &unique_id, const char *name = "Lambertian") : MaterialNode(unique_id, name)
		{
			type = LambertianType;
			AddInput(unique_id, PinSpectrum, "->Albedo");
			AddInput(unique_id, PinVec3, "->Normal");
			AddOutput(unique_id, PinBSDF, "BSDF->");
			albedo_pin = &inputs[0];
			normal_pin = &inputs[1];
		}
		LambertianNode(const json& j) : MaterialNode(j)
		{
			type = LambertianType;
			albedo_pin = &inputs[0];
			normal_pin = &inputs[1];
			for (size_t i = 0; i < j["albedo"].size(); i++) {
				albedo.data[i] = j["albedo"][i];
			}
		}
		void DumpJson(json& j) const override
		{
			for (const auto& d : albedo.data) {
				j["albedo"].push_back(d);
			}
			DumpIO(j);
		}
		void Render(void) override;
		void PreProcess(HitRecord& rec) const override;
		bool Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const override;
		double BxDF(const vec3& vi, double wli, const vec3& vo, double wlo, const vec3& vt = default_vt) const override;
		//double Emitted(const ray& r, const HitRecord& rec) const override;
		Spectrum albedo = Spectrum(1.0);
		const PinInfo *albedo_pin;
		const PinInfo *normal_pin;
};

class ConductorNode : public MaterialNode, public Material {
	public:
		ConductorNode(int &unique_id, const char *name = "Conductor") : MaterialNode(unique_id, name)
		{
			type = ConductorType;
			AddInput(unique_id, PinSpectrum, "->n");
			AddInput(unique_id, PinSpectrum, "->k");
			AddInput(unique_id, PinVec3, "->normal");
			AddOutput(unique_id, PinBSDF, "BSDF->");
			normal_pin = &inputs[2];
		}
		ConductorNode(const json& j) : MaterialNode(j)
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
		void DumpJson(json& j) const override
		{
			for (const auto& d : n.data) {
				j["n"].push_back(d);
			}
			for (const auto& d : k.data) {
				j["k"].push_back(d);
			}
			DumpIO(j);
		}
		void Render(void) override;
		void PreProcess(HitRecord& rec) const override;
		bool Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const override;
		double BxDF(const vec3& vi, double wli, const vec3& vo, double wlo, const vec3& vt = default_vt) const override;
		Spectrum n = Spectrum(0.2);
		Spectrum k = Spectrum(1.0);
		const PinInfo *normal_pin;
};


class ColoredMetal : public MaterialNode, public Material {
	public:
		ColoredMetal(int &unique_id, const char *name = "Colored Metal") : MaterialNode(unique_id, name)
		{
			type = ColoredMetalType;
			AddInput(unique_id, PinSpectrum, "->albedo");
			AddInput(unique_id, PinVec3, "->normal");
			AddOutput(unique_id, PinBSDF, "BSDF->");
			albedo_pin = &inputs[0];
			normal_pin = &inputs[1];
		}

		explicit ColoredMetal(const json& j) : MaterialNode(j)
	{
		type = ColoredMetalType;
		albedo_pin = &inputs[0];
		normal_pin = &inputs[1];

		for (size_t i = 0; i < j["albedo"].size(); i++) {
			albedo.data[i] = j["albedo"][i];
		}
	}
		void DumpJson(json& j) const override
		{
			for (const auto& d : albedo.data) {
				j["albedo"].push_back(d);
			}
			DumpIO(j);
		}
		void Render(void) override;
		void PreProcess(HitRecord& rec) const override;
		bool Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const override;
		double BxDF(const vec3& vi, double wli, const vec3& vo, double wlo, const vec3& vt = default_vt) const override;
		Spectrum albedo = Spectrum(1.0);
		const PinInfo *albedo_pin;
		const PinInfo *normal_pin;
};

class MixBSDFNode : public MaterialNode, public Material {
	public:
		MixBSDFNode(int &unique_id, const char *name = "Mix BSDF Node") : MaterialNode(unique_id, name)
		{
			type = MixBSDFType;
			AddInput(unique_id, PinBSDF, "->In0");
			AddInput(unique_id, PinBSDF, "->In1");
			AddOutput(unique_id, PinBSDF, "Out->");
		}
		MixBSDFNode(const json& j) : MaterialNode(j)
	{
		type = MixBSDFType;
		ratio = j["ratio"];
	}
		void DumpJson(json& j) const override
		{
			DumpIO(j);
			j["ratio"] = ratio;
		}
		void Render(void) override;
		void PreProcess(HitRecord& rec) const override;
		bool Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const override;
		double BxDF(const vec3& vi, double wli, const vec3& vo, double wlo, const vec3& vt = default_vt) const override;
		double ratio = 0.5;
	private:
		size_t selected_node;
};

class OutputNode : public MaterialNode {
	public:
		OutputNode(int &unique_id, const char *name = "Output") : MaterialNode(unique_id, name)
		{
			type = OutputType;
			AddInput(unique_id, PinBSDF, "->BSDF");
			AddInput(unique_id, PinDouble, "->Emission");
		}
		OutputNode(const json& j) : MaterialNode(j)
	{
		type = OutputType;
	}
		void Render(void) override;
};

class FixedValueNode : public MaterialNode {
	public:
		FixedValueNode(int &unique_id, PinType type, const char *name = "") : MaterialNode(unique_id, name)
		{
			this->type = FixedValueType;
			AddOutput(unique_id, type, "Out->");
		}
		FixedValueNode(const json& j) : MaterialNode(j)
		{
			this->type = FixedValueType;
		}
		double dvalue;
		vec3 vvalue;
		Spectrum svalue;
		void Compute(double& data) const override;
		void Compute(vec3& data) const override;
		void Compute(Spectrum& data) const override;
};


class RGBColorNode : public MaterialNode {
	public:
		RGBColorNode(int &unique_id, const char *name = "RGB Color") : MaterialNode(unique_id, name)
		{
			type = RGBColorType;
			AddOutput(unique_id, PinVec3, "RGB->");
		}
		RGBColorNode(const json& j) : MaterialNode(j)
	{
		type = RGBColorType;
		col[0] = j["col"][0];
		col[1] = j["col"][1];
		col[2] = j["col"][2];
	}
		void DumpJson(json& j) const override
		{
			DumpIO(j);
			j["col"][0] = col[0];
			j["col"][1] = col[1];
			j["col"][2] = col[2];
		}
		void Render(void) override;
		void Compute(vec3& data) const override;
		float col[3] = {0.0f, 0.0f, 0.0f};
};

class RGBtoSpectrumNode : public MaterialNode {
	public:
		RGBtoSpectrumNode(int &unique_id, const char *name = "RGBtoSpectrumNode") : MaterialNode(unique_id, name)
		{
			type = RGBtoSpectrumType;
			AddInput(unique_id, PinVec3, "->RGB");
			AddOutput(unique_id, PinSpectrum, "Spectrum->");
		}
		RGBtoSpectrumNode(const json& j) : MaterialNode(j)
	{
		type = RGBtoSpectrumType;
	}
		void Compute(Spectrum& data) const override;
};

class ImageTextureNode : public MaterialNode {
	public:
		ImageTextureNode(int &unique_id, const char *path = "", const char *name = "Image Texture") : MaterialNode(unique_id, name)
		{
			type = ImageTextureType;
			this->path = std::string(path);
			AddInput(unique_id, PinVec3, "->UV");
			AddOutput(unique_id, PinVec3, "Out->");
		}
		ImageTextureNode(const json& j);
		void DumpJson(json& j)
		{
			j["path"] = path;
			DumpIO(j);
		}
		std::string path;
		uint8_t *texture = nullptr;
		int width, height, bpp;

		void Render(void) override;
		void Compute(vec3& data) const override;
};

class CheckerboardNode : public MaterialNode {
	public:
		CheckerboardNode(int &unique_id, const char *name = "Checkerboard") : MaterialNode(unique_id, name)
		{
			type = CheckerboardType;
			AddInput(unique_id, PinVec3, "UV->");
			AddOutput(unique_id, PinVec3, "Out->");
		}
		CheckerboardNode(const json& j) : MaterialNode(j)
	{
		type = CheckerboardType;
		size = j["size"];
	}
		void DumpJson(json& j) const override
		{
			j["size"] = size;
			DumpIO(j);
		}
		void Compute(vec3& data) const override;
		void Render(void) override;
		double size = 0.5;
};

class AdditionNode : public MaterialNode {
	public:
		AdditionNode(int &unique_id, const char *name = "Add") : MaterialNode(unique_id, name)
		{
			type = AdditionType;
			AddInput(unique_id, PinUniversal, "->In0");
			AddInput(unique_id, PinUniversal, "->In1");
			AddOutput(unique_id, PinUniversal, "Out->");
		}
		AdditionNode(const json& j) : MaterialNode(j)
	{
		type = AdditionType;
	}
		void Compute(double& data) const override;
		void Compute(vec3& data) const override;
		void Compute(Spectrum& data) const override;
};

class MultiplicationNode : public MaterialNode {
	public:
		MultiplicationNode(int &unique_id, const char *name = "Multiply") : MaterialNode(unique_id, name)
		{
			type = MultiplicationType;
			AddInput(unique_id, PinUniversal, "->In0");
			AddInput(unique_id, PinUniversal, "->In1");
			AddOutput(unique_id, PinUniversal, "Out->");
		}
		MultiplicationNode(const json& j) : MaterialNode(j)
	{
		type = MultiplicationType;
	}
		void Compute(double& data) const override;
		void Compute(vec3& data) const override;
		void Compute(Spectrum& data) const override;
};

class ScalarMultiplicationNode : public MaterialNode {
	public:
		ScalarMultiplicationNode(int &unique_id, const char *name = "Multiply Scalar") : MaterialNode(unique_id, name)
		{
			type = ScalarMultiplicationType;
			AddInput(unique_id, PinUniversal, "->In");
			AddOutput(unique_id, PinUniversal, "Out->");
		}
		ScalarMultiplicationNode(const json& j) : MaterialNode(j)
	{
		type = ScalarMultiplicationType;
		scale = j["scale"];
	}
		void DumpJson(json& j) const override
		{
			j["scale"] = scale;
			DumpIO(j);
		}
		void Compute(double& data) const override;
		void Compute(vec3& data) const override;
		void Compute(Spectrum& data) const override;
		void Render(void) override;
		double scale;
};

class RandomSamplingNode : public MaterialNode {
	public:
		RandomSamplingNode(int &unique_id, const char *name = "Multiply Scalar") : MaterialNode(unique_id, name)
		{
			type = RandomSamplingType;
			AddInput(unique_id, PinUniversal, "->In0");
			AddInput(unique_id, PinUniversal, "->In1");
			AddOutput(unique_id, PinUniversal, "Out->");
		}
		RandomSamplingNode(const json& j) : MaterialNode(j)
	{
		type = RandomSamplingType;
		ratio = j["ratio"];
	}
		void DumpJson(json& j) const override
		{
			j["ratio"] = ratio;
			DumpIO(j);
		}
		void Compute(double& data) const override;
		void Compute(vec3& data) const override;
		void Compute(Spectrum& data) const override;
		void Render(void) override;
		double ratio;
};


class NodeMaterial : public Material {
	public:
		NodeMaterial(void) : context(ax::NodeEditor::CreateEditor())
		{
			material_nodes.push_back(new FixedValueNode(unique_id, PinVec3, "UV"));
			material_nodes.push_back(new OutputNode(unique_id));
		}
		explicit NodeMaterial(const json& j) : context(ax::NodeEditor::CreateEditor())
		{
			name = j["name"].get<std::string>();
			bool is_there_any_widgets = false;
			for (const auto& node_j : j["nodes"]) {
				is_there_any_widgets = true;
				MaterialNode *node = nullptr;
				std::cout << node_j["name"] << std::endl;
				std::cout << node_j["type"] << std::endl;
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
					case MixBSDFType:
						node = new MixBSDFNode(node_j);
						break;
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
		void Render(void);
		struct PinInfo *FindPin(int iid);
		struct PinInfo *FindPin(const ed::PinId& id);
		struct LinkInfo *FindLink(const ed::LinkId& id);
		const struct PinInfo *FindPinConst(const ed::PinId& id) const;
		const struct LinkInfo *FindLinkConst(const ed::LinkId& id) const;
		void AddLink(PinInfo *input, PinInfo *b);


		void DumpJson(json& j) const
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
		std::string name;
		int unique_id = 1;
		std::vector<LinkInfo *> links;
		std::vector<MaterialNode *> material_nodes;
		ax::NodeEditor::EditorContext *context = nullptr;
		void PreProcess(HitRecord& rec) const override;
		bool Sample(const HitRecord& rec, const ONB& uvw, const vec3& vo, double wlo, vec3& vi, double& wli, double& BxDF, double& pdfval) const override;
		double BxDF(const vec3& vi, double wli, const vec3& vo, double wlo, const vec3& vt = default_vt) const override;
		double Emitted(const ray& r, const HitRecord& rec) const override;

		size_t uv_i = 0;
		size_t Output_i = 1;

	private:
};

#endif
