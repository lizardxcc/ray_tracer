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
	LinkInfo(int &unique_id, ed::PinId input_id, ed::PinId output_id, PinInfo *input, PinInfo *output);
	LinkInfo(const json& j, PinInfo *input, PinInfo *output);
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
	PinInfo(int &unique_id, PinIOType io_type, PinType type, const char *name, const MaterialNode *parent_node);
	PinInfo(const json& j, const MaterialNode *parent_node);
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
		MaterialNode(int &unique_id, const char *name = "");
		explicit MaterialNode(const json& j);
		virtual void Render(void);
		void RenderPins(void);
		void RenderSpectrum(Spectrum& data, double min, double max);
		virtual void Compute(double& data) const;
		virtual void Compute(Spectrum& data) const;
		virtual void Compute(vec3& data) const;
		void AddInput(int &unique_id, PinType type, const char *name);
		void AddOutput(int &unique_id, PinType type, const char *name);


		virtual void DumpJson(json& j) const;
		void DumpIO(json &j) const;
		void DumpSpectrum(json& j, const Spectrum& s, const char *name) const;

		std::string name;
		ed::NodeId id;
		int iid;
		enum MaterialNodeType type;
		std::vector<PinInfo> inputs;
		std::vector<PinInfo> outputs;
		const static ImVec4 pin_colors[];
	protected:
		void UpdateNormal(const PinInfo *normal_pin, const HitRecord& rec, vec3& new_normal) const;
};
class SpectrumNode : public MaterialNode {
	public:
		SpectrumNode(int &unique_id, const char *name = "Spectrum");
		SpectrumNode(const json& j);
		void DumpJson(json& j) const override;
		void Render(void) override;
		void Compute(Spectrum& data) const override;
	private:
		Spectrum data = Spectrum(1.0);
};

class LambertianNode : public MaterialNode, public Material {
	public:
		LambertianNode(int &unique_id, const char *name = "Lambertian");
		LambertianNode(const json& j);
		void DumpJson(json& j) const override;
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
		ConductorNode(int &unique_id, const char *name);
		ConductorNode(const json& j);
		void DumpJson(json& j) const override;
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
		ColoredMetal(int &unique_id, const char *name = "Colored Metal");
		explicit ColoredMetal(const json& j);
		void DumpJson(json& j) const override;
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
		MixBSDFNode(int &unique_id, const char *name = "Mix BSDF Node");
		MixBSDFNode(const json& j);
		void DumpJson(json& j) const override;
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
		OutputNode(int &unique_id, const char *name = "Output");
		OutputNode(const json& j);
		void Render(void) override;
};

class FixedValueNode : public MaterialNode {
	public:
		FixedValueNode(int &unique_id, PinType type, const char *name = "");
		FixedValueNode(const json& j);
		double dvalue;
		vec3 vvalue;
		Spectrum svalue;
		void Compute(double& data) const override;
		void Compute(vec3& data) const override;
		void Compute(Spectrum& data) const override;
};


class RGBColorNode : public MaterialNode {
	public:
		RGBColorNode(int &unique_id, const char *name = "RGB Color");
		RGBColorNode(const json& j);
		void DumpJson(json& j) const override;
		void Render(void) override;
		void Compute(vec3& data) const override;
		float col[3] = {0.0f, 0.0f, 0.0f};
};

class RGBtoSpectrumNode : public MaterialNode {
	public:
		RGBtoSpectrumNode(int &unique_id, const char *name = "RGBtoSpectrumNode");
		RGBtoSpectrumNode(const json& j);
		void Compute(Spectrum& data) const override;
};

class ImageTextureNode : public MaterialNode {
	public:
		ImageTextureNode(int &unique_id, const char *path = "", const char *name = "Image Texture");
		ImageTextureNode(const json& j);
		void DumpJson(json& j) const override;
		std::string path;
		uint8_t *texture = nullptr;
		int width, height, bpp;

		void Render(void) override;
		void Compute(vec3& data) const override;
};

class CheckerboardNode : public MaterialNode {
	public:
		CheckerboardNode(int &unique_id, const char *name = "Checkerboard");
		CheckerboardNode(const json& j);
		void DumpJson(json& j) const override;
		void Compute(vec3& data) const override;
		void Render(void) override;
		double size = 0.5;
};

class AdditionNode : public MaterialNode {
	public:
		AdditionNode(int &unique_id, const char *name = "Add");
		AdditionNode(const json& j);
		void Compute(double& data) const override;
		void Compute(vec3& data) const override;
		void Compute(Spectrum& data) const override;
};

class MultiplicationNode : public MaterialNode {
	public:
		MultiplicationNode(int &unique_id, const char *name = "Multiply");
		MultiplicationNode(const json& j);
		void Compute(double& data) const override;
		void Compute(vec3& data) const override;
		void Compute(Spectrum& data) const override;
};

class ScalarMultiplicationNode : public MaterialNode {
	public:
		ScalarMultiplicationNode(int &unique_id, const char *name = "Multiply Scalar");
		ScalarMultiplicationNode(const json& j);
		void DumpJson(json& j) const override;
		void Compute(double& data) const override;
		void Compute(vec3& data) const override;
		void Compute(Spectrum& data) const override;
		void Render(void) override;
		double scale;
};

class RandomSamplingNode : public MaterialNode {
	public:
		RandomSamplingNode(int &unique_id, const char *name = "Multiply Scalar");
		RandomSamplingNode(const json& j);
		void DumpJson(json& j) const override;
		void Compute(double& data) const override;
		void Compute(vec3& data) const override;
		void Compute(Spectrum& data) const override;
		void Render(void) override;
		double ratio;
};


class NodeMaterial : public Material {
	public:
		NodeMaterial(void);
		explicit NodeMaterial(const json& j);
		void Render(void);
		struct PinInfo *FindPin(int iid);
		struct PinInfo *FindPin(const ed::PinId& id);
		struct LinkInfo *FindLink(const ed::LinkId& id);
		const struct PinInfo *FindPinConst(const ed::PinId& id) const;
		const struct LinkInfo *FindLinkConst(const ed::LinkId& id) const;
		void AddLink(PinInfo *input, PinInfo *b);
		void DumpJson(json& j) const;
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
