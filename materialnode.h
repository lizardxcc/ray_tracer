#ifndef MATERIALNODE_H
#define MATERIALNODE_H

#include "imgui_node_editor.h"
#include "material.h"

namespace ed = ax::NodeEditor;


class MaterialNode;
struct PinInfo;

struct LinkInfo {
	ed::LinkId id;
	ed::PinId input_id;
	ed::PinId output_id;
	PinInfo *input;
	PinInfo *output;
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
	ed::PinId id;
	PinIOType io_type;
	PinType type;
	std::string name;
	const MaterialNode *parent_node;
	std::vector<const LinkInfo *> connected_links;
};

class MaterialNode {
	public:
		MaterialNode(int &unique_id, const char *name = "") : id(unique_id), iid(unique_id), name(name)
		{
			unique_id++;
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
		std::string name;
		ed::NodeId id;
		int iid;
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
			AddOutput(unique_id, PinSpectrum, "Out->");
		}
		void Render(void);
		void Compute(Spectrum& data) const override;
	private:
		Spectrum data = Spectrum(1.0);
};

class LambertianNode : public MaterialNode, public Material {
	public:
		LambertianNode(int &unique_id, const char *name = "Lambertian") : MaterialNode(unique_id, name)
		{
			AddInput(unique_id, PinSpectrum, "->Albedo");
			AddInput(unique_id, PinVec3, "->Normal");
			AddOutput(unique_id, PinBSDF, "BSDF->");
			albedo_pin = &inputs[0];
			normal_pin = &inputs[1];
		}
		void Render(void);
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
			AddInput(unique_id, PinSpectrum, "->n");
			AddInput(unique_id, PinSpectrum, "->k");
			AddInput(unique_id, PinVec3, "->normal");
			AddOutput(unique_id, PinBSDF, "BSDF->");
			normal_pin = &inputs[2];
		}
		void Render(void);
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
			AddInput(unique_id, PinSpectrum, "->albedo");
			AddInput(unique_id, PinVec3, "->normal");
			AddOutput(unique_id, PinBSDF, "BSDF->");
			albedo_pin = &inputs[0];
			normal_pin = &inputs[1];
		}
		void Render(void);
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
			AddInput(unique_id, PinBSDF, "->In0");
			AddInput(unique_id, PinBSDF, "->In1");
			AddOutput(unique_id, PinBSDF, "Out->");
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
			AddInput(unique_id, PinBSDF, "->BSDF");
			AddInput(unique_id, PinDouble, "->Emission");
		}
		void Render(void);
};

class FixedValueNode : public MaterialNode {
	public:
		FixedValueNode(int &unique_id, PinType type, const char *name = "") : MaterialNode(unique_id, name)
		{
			AddOutput(unique_id, type, "Out->");
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
			AddOutput(unique_id, PinVec3, "RGB->");
		}
		void Render(void) override;
		void Compute(vec3& data) const override;
		float col[3] = {0.0f, 0.0f, 0.0f};
};

class RGBtoSpectrumNode : public MaterialNode {
	public:
		RGBtoSpectrumNode(int &unique_id, const char *name = "RGBtoSpectrumNode") : MaterialNode(unique_id, name)
		{
			AddInput(unique_id, PinVec3, "->RGB");
			AddOutput(unique_id, PinSpectrum, "Spectrum->");
		}
		void Compute(Spectrum& data) const override;
};

class TextureNode : public MaterialNode {
	public:
		TextureNode(int &unique_id, const char *path = "", const char *name = "Texture") : MaterialNode(unique_id, name)
		{
			this->path = std::string(path);
			AddInput(unique_id, PinVec3, "->UV");
			AddOutput(unique_id, PinVec3, "Out->");
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
			AddInput(unique_id, PinVec3, "UV->");
			AddOutput(unique_id, PinVec3, "Out->");
		}
		void Compute(vec3& data) const override;
		void Render(void) override;
		double size = 0.5;
};

class AdditionNode : public MaterialNode {
	public:
		AdditionNode(int &unique_id, const char *name = "Add") : MaterialNode(unique_id, name)
		{
			AddInput(unique_id, PinUniversal, "->In0");
			AddInput(unique_id, PinUniversal, "->In1");
			AddOutput(unique_id, PinUniversal, "Out->");
		}
		void Compute(double& data) const override;
		void Compute(vec3& data) const override;
		void Compute(Spectrum& data) const override;
};

class MultiplicationNode : public MaterialNode {
	public:
		MultiplicationNode(int &unique_id, const char *name = "Multiply") : MaterialNode(unique_id, name)
		{
			AddInput(unique_id, PinUniversal, "->In0");
			AddInput(unique_id, PinUniversal, "->In1");
			AddOutput(unique_id, PinUniversal, "Out->");
		}
		void Compute(double& data) const override;
		void Compute(vec3& data) const override;
		void Compute(Spectrum& data) const override;
};

class ScalarMultiplicationNode : public MaterialNode {
	public:
		ScalarMultiplicationNode(int &unique_id, const char *name = "Multiply Scalar") : MaterialNode(unique_id, name)
		{
			AddInput(unique_id, PinUniversal, "->In");
			AddOutput(unique_id, PinUniversal, "Out->");
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
			AddInput(unique_id, PinUniversal, "->In0");
			AddInput(unique_id, PinUniversal, "->In1");
			AddOutput(unique_id, PinUniversal, "Out->");
		}
		void Compute(double& data) const override;
		void Compute(vec3& data) const override;
		void Compute(Spectrum& data) const override;
		void Render(void) override;
		double ratio;
};


class NodeMaterial : public Material {
	public:
		NodeMaterial(void)
		{
			material_nodes.push_back(new FixedValueNode(unique_id, PinVec3, "UV"));
			material_nodes.push_back(new OutputNode(unique_id));
		}
		void Render(void);
		struct PinInfo *FindPin(const ed::PinId& id);
		struct LinkInfo *FindLink(const ed::LinkId& id);
		const struct PinInfo *FindPinConst(const ed::PinId& id) const;
		const struct LinkInfo *FindLinkConst(const ed::LinkId& id) const;
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
