#ifndef MATERIALNODE_H
#define MATERIALNODE_H

#include <boost/filesystem/path.hpp>
#ifndef _CLI
#include "imgui_node_editor.h"
#endif
#include "vec3.h"
#include "hittable.h"
#include "spectrum.h"
#include "onb.h"
#include "json.hpp"
using json = nlohmann::json;

#ifndef _CLI
namespace ed = ax::NodeEditor;
#endif


class MaterialNode;
struct PinInfo;
struct HitRecord;

struct LinkInfo {
#ifndef _CLI
	LinkInfo(int &unique_id, ed::PinId input_id, ed::PinId output_id, PinInfo *input, PinInfo *output);
	const ed::LinkId id;
	const ed::PinId input_id;
	const ed::PinId output_id;
#endif
	LinkInfo(const json& j, PinInfo *input, PinInfo *output);
	void DumpJson(json& j) const;
	const int iid;
	PinInfo * const input;
	PinInfo * const output;
};

enum PinType {
	PinDouble,
	PinVec3,
	PinSpectrum,
	PinUniversal,
	PinBSDF
};

enum PinIOType {
	PinInput,
	PinOutput
};



struct PinInfo {
	PinInfo(int &unique_id, PinIOType io_type, PinType type, const char *name, const MaterialNode *parent_node);
	PinInfo(const json& j, const MaterialNode *parent_node);
#ifndef _CLI
	const ed::PinId id;
#endif
	const int iid;
	const std::string name;
	const PinIOType io_type;
	const PinType type;
	const MaterialNode * const parent_node;
	std::vector<const LinkInfo *> connected_links;
};


struct Argument {
	dvec3 vt;
};


enum MaterialNodeType {
	LambertianType,
	ConductorType,
	ColoredMetalType,
	DiffuseLightType,
	MixBSDFType,
	OutputType,
	UVType,
	SpectrumType,
	RGBColorType,
	RGBtoSpectrumType,
	ImageTextureType,
	CheckerboardType,
	AdditionType,
	MultiplicationType,
	ScalarMultiplicationType,
	RandomSamplingType,
	GGXReflectionType,
	AccessVec3ComponentType,
	CombineVec3ComponentType,
	ValueNoiseType,
	ValueNoise2DType,
	RodriguesRotationType,
	DielectricType,
};

class MaterialNode {
	public:
		MaterialNode(void);
		MaterialNode(int &unique_id, const char *name = "");
		explicit MaterialNode(const json& j);
		void RenderNode(void);
		virtual void RenderEditor(void);
		void RenderPins(void);
		void RenderSpectrum(Spectrum& data, double min, double max);
		const MaterialNode *GetInputParentNode(const PinInfo *pin) const;
		virtual void Compute(const Argument& global_arg, double& data) const;
		virtual void Compute(const Argument& global_arg, Spectrum& data) const;
		virtual void Compute(const Argument& global_arg, dvec3& data) const;
		void AddInput(int &unique_id, PinType type, const char *name);
		void AddOutput(int &unique_id, PinType type, const char *name);


		virtual void DumpJson(json& j) const;
		void DumpIO(json &j) const;
		void DumpSpectrum(json& j, const Spectrum& s, const char *name) const;

#ifndef _CLI
		ed::NodeId id;
		const static ImVec4 pin_colors[];
#endif
		int iid;
		std::string name;
		enum MaterialNodeType type;
		std::vector<PinInfo> inputs;
		std::vector<PinInfo> outputs;
	protected:
		void UpdateNormal(const PinInfo *normal_pin, const HitRecord& rec, dvec3& new_normal) const;
};

// Instead of Radiance, Basic Radiance in Veach's thesis is used in this renderer
// Basic projected solid angle
// Basic solid angle
// Basic throughput measure
// Basic radiance
// Basic spectral radiance
// Basic inner product
// Basic BSDF
//
class BSDFMaterialNode : public virtual MaterialNode {
	public:
		virtual void PreProcess(const Argument& global_arg, HitRecord &rec) const;
		// Fix argument names later
		virtual bool SampleBSDF(const Argument& global_arg, RayType type, const HitRecord& rec, const ONB& uvw, const dvec3& vo, double wlo, dvec3& vi, double& wli, double& BSDF_divided_by_solidangle_pdf, double& BSDF, double& solidangle_pdfval) const;
		// phsycial light passes from vi to vo
		virtual double BSDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const;
		virtual double PDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const;
		virtual double Emitted(const Argument& global_arg, const ray& r, const HitRecord& rec) const;
	protected:
		Spectrum inner_ref_idx = Spectrum(1.0);
		Spectrum outer_ref_idx = Spectrum(1.0);
};
class SpectrumNode : public MaterialNode {
	public:
		SpectrumNode(int &unique_id, const char *name = "Spectrum");
		SpectrumNode(const json& j);
		void DumpJson(json& j) const override;
		void RenderEditor(void) override;
		void Compute(const Argument& global_arg, Spectrum& data) const override;
	private:
		Spectrum data = Spectrum(1.0);
};

class LambertianNode : public BSDFMaterialNode {
	public:
		LambertianNode(int &unique_id, const char *name = "Lambertian");
		LambertianNode(const json& j);
		void DumpJson(json& j) const override;
		void RenderEditor(void) override;
		void PreProcess(const Argument& global_arg, HitRecord& rec) const override;
		bool SampleBSDF(const Argument& global_arg, RayType type, const HitRecord& rec, const ONB& uvw, const dvec3& vo, double wlo, dvec3& vi, double& wli, double& bxdf_divided_by_pdf, double& BSDF, double& pdfval) const override;
		double BSDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const override;
		double PDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const override;
	private:
		Spectrum albedo = Spectrum(1.0);
		const PinInfo *albedo_pin;
		const PinInfo *normal_pin;
};

class DielectricNode : public BSDFMaterialNode {
	public:
		DielectricNode(int &unique_id, const char *name = "Dielectric");
		DielectricNode(const json& j);
		void DumpJson(json& j) const override;
		void RenderEditor(void) override;
		void PreProcess(const Argument& global_arg, HitRecord& rec) const override;
		bool SampleBSDF(const Argument& global_arg, RayType type, const HitRecord& rec, const ONB& uvw, const dvec3& vo, double wlo, dvec3& vi, double& wli, double& bxdf_divided_by_pdf, double& BSDF, double& pdfval) const override;
		double BSDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const override;
		double PDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const override;
	private:
		Spectrum n = Spectrum(2.4);
		Spectrum surface_color = Spectrum(1.0);
		const PinInfo *normal_pin;
};

class ConductorNode : public BSDFMaterialNode {
	public:
		ConductorNode(int &unique_id, const char *name = "Conductor");
		ConductorNode(const json& j);
		void DumpJson(json& j) const override;
		void RenderEditor(void) override;
		void PreProcess(const Argument& global_arg, HitRecord& rec) const override;
		bool SampleBSDF(const Argument& global_arg, RayType type, const HitRecord& rec, const ONB& uvw, const dvec3& vo, double wlo, dvec3& vi, double& wli, double& bxdf_divided_by_pdf, double& BSDF, double& pdfval) const override;
		double BSDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const override;
		double PDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const override;
	private:
		Spectrum n = Spectrum(0.2);
		Spectrum k = Spectrum(1.0);
		const PinInfo *normal_pin;
};


class ColoredMetal : public BSDFMaterialNode {
	public:
		ColoredMetal(int &unique_id, const char *name = "Colored Metal");
		explicit ColoredMetal(const json& j);
		void DumpJson(json& j) const override;
		void RenderEditor(void) override;
		void PreProcess(const Argument& global_arg, HitRecord& rec) const override;
		bool SampleBSDF(const Argument& global_arg, RayType type, const HitRecord& rec, const ONB& uvw, const dvec3& vo, double wlo, dvec3& vi, double& wli, double& bxdf_divided_by_pdf, double& BSDF, double& pdfval) const override;
		double BSDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const override;
		double PDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const override;
	private:
		Spectrum albedo = Spectrum(1.0);
		const PinInfo *albedo_pin;
		const PinInfo *normal_pin;
};

class GGXReflection : public BSDFMaterialNode {
	public:
		GGXReflection(int &unique_id, const char *name = "GGXReflection");
		explicit GGXReflection(const json& j);
		void DumpJson(json& j) const override;
		void RenderEditor(void) override;
		void PreProcess(const Argument& global_arg, HitRecord& rec) const override;
		bool SampleBSDF(const Argument& global_arg, RayType type, const HitRecord& rec, const ONB& uvw, const dvec3& vo, double wlo, dvec3& vi, double& wli, double& bxdf_divided_by_pdf, double& BSDF, double& pdfval) const override;
		double BSDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const override;
		double PDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const override;
	private:
		Spectrum n = Spectrum(1.0);
		Spectrum k = Spectrum(0.0);
		//const PinInfo *albedo_pin;
		const PinInfo *n_pin;
		const PinInfo *k_pin;
		double alpha;
};


class DiffuseLightNode : public BSDFMaterialNode {
	public:
		DiffuseLightNode(int &unique_id, const char *name = "Diffuse Light");
		explicit DiffuseLightNode(const json& j);
		void DumpJson(json& j) const override;
		void RenderEditor(void) override;
		void PreProcess(const Argument& global_arg, HitRecord& rec) const override;
		double Emitted(const Argument& global_arg, const ray& r, const HitRecord& rec) const override;
	private:
		Spectrum color = Spectrum(1.0);
		const PinInfo *color_pin;
		const PinInfo *normal_pin;
};

/*
class MixBSDFNode : public MaterialNode, public Material {
	public:
		MixBSDFNode(int &unique_id, const char *name = "Mix BSDF Node");
		MixBSDFNode(const json& j);
		void DumpJson(json& j) const override;
		void RenderEditor(void) override;
		void PreProcess(HitRecord& rec) const override;
		bool SampleBSDF(const HitRecord& rec, const ONB& uvw, const dvec3& vo, double wlo, dvec3& vi, double& wli, double& BSDF, double& pdfval) const override;
		double BSDF(const dvec3& vi, double wli, const dvec3& vo, double wlo, const dvec3& vt = default_vt) const override;
		double ratio = 0.5;
	private:
		size_t selected_node;
};
*/

class OutputNode : public MaterialNode {
	public:
		OutputNode(int &unique_id, const char *name = "Output");
		OutputNode(const json& j);
		void RenderEditor(void) override;
};

class UVNode : public MaterialNode {
	public:
		UVNode(int &unique_id, PinType type, const char *name = "");
		UVNode(const json& j);
		void Compute(const Argument& global_arg, dvec3& data) const override;
};


class RGBColorNode : public MaterialNode {
	public:
		RGBColorNode(int &unique_id, const char *name = "RGB Color");
		RGBColorNode(const json& j);
		void DumpJson(json& j) const override;
		void RenderEditor(void) override;
		void Compute(const Argument& global_arg, dvec3& data) const override;
	private:
		float col[3] = {0.0f, 0.0f, 0.0f};
};

class RGBtoSpectrumNode : public MaterialNode {
	public:
		RGBtoSpectrumNode(int &unique_id, const char *name = "RGBtoSpectrumNode");
		RGBtoSpectrumNode(const json& j);
		void Compute(const Argument& global_arg, Spectrum& data) const override;
};

class ImageTextureNode : public MaterialNode {
	public:
		ImageTextureNode(int &unique_id, const char *path = "", const char *name = "Image Texture");
		ImageTextureNode(const json& j);
		void DumpJson(json& j) const override;
		void RenderEditor(void) override;
		void Compute(const Argument& global_arg, dvec3& data) const override;
	private:
		std::string path;
		uint8_t *texture = nullptr;
		int width, height, bpp;

};

class CheckerboardNode : public MaterialNode {
	public:
		CheckerboardNode(int &unique_id, const char *name = "Checkerboard");
		CheckerboardNode(const json& j);
		void DumpJson(json& j) const override;
		void Compute(const Argument& global_arg, dvec3& data) const override;
		void RenderEditor(void) override;
	private:
		double size = 0.5;
};

class AdditionNode : public MaterialNode {
	public:
		AdditionNode(int &unique_id, const char *name = "Add");
		AdditionNode(const json& j);
		void Compute(const Argument& global_arg, double& data) const override;
		void Compute(const Argument& global_arg, dvec3& data) const override;
		void Compute(const Argument& global_arg, Spectrum& data) const override;
};

class MultiplicationNode : public MaterialNode {
	public:
		MultiplicationNode(int &unique_id, const char *name = "Multiply");
		MultiplicationNode(const json& j);
		void Compute(const Argument& global_arg, double& data) const override;
		void Compute(const Argument& global_arg, dvec3& data) const override;
		void Compute(const Argument& global_arg, Spectrum& data) const override;
};

class ScalarMultiplicationNode : public MaterialNode {
	public:
		ScalarMultiplicationNode(int &unique_id, const char *name = "Multiply Scalar");
		ScalarMultiplicationNode(const json& j);
		void DumpJson(json& j) const override;
		void Compute(const Argument& global_arg, double& data) const override;
		void Compute(const Argument& global_arg, dvec3& data) const override;
		void Compute(const Argument& global_arg, Spectrum& data) const override;
		void RenderEditor(void) override;
	private:
		double scale;
};

class RandomSamplingNode : public MaterialNode {
	public:
		RandomSamplingNode(int &unique_id, const char *name = "Multiply Scalar");
		RandomSamplingNode(const json& j);
		void DumpJson(json& j) const override;
		void Compute(const Argument& global_arg, double& data) const override;
		void Compute(const Argument& global_arg, dvec3& data) const override;
		void Compute(const Argument& global_arg, Spectrum& data) const override;
		void RenderEditor(void) override;
	private:
		double ratio;
};


class AccessVec3ComponentNode : public MaterialNode {
	public:
		AccessVec3ComponentNode(int &unique_id, const char *name = "AccessVec3ComponentNode");
		AccessVec3ComponentNode(const json& j);
		void Compute(const Argument& global_arg, double& data) const override;
		void RenderEditor(void) override;
	private:
		uint32_t index = 0;
};


class CombineVec3ComponentNode : public MaterialNode {
	public:
		CombineVec3ComponentNode(int &unique_id, const char *name = "CombineVec3ComponentNode");
		CombineVec3ComponentNode(const json& j);
		void Compute(const Argument& global_arg, dvec3& data) const override;
	private:
};

class ValueNoiseNode : public MaterialNode {
	public:
		ValueNoiseNode(int &unique_id, const char *name = "Value Noise");
		ValueNoiseNode(const json& j);
		void Compute(const Argument& global_arg, double &data) const override;
		void DumpJson(json& j) const override;
		void RenderEditor(void) override;
	private:
		void GenerateRand(unsigned int seed = 323048);
		std::vector<double> r;
		uint32_t size = 256;
};

class ValueNoise2DNode : public MaterialNode {
	public:
		ValueNoise2DNode(int &unique_id, const char *name = "Value Noise 2D");
		ValueNoise2DNode(const json& j);
		void Compute(const Argument& global_arg, double &data) const override;
		void DumpJson(json& j) const override;
		void RenderEditor(void) override;
	private:
		void GenerateRand(unsigned int seed = 323048);
		std::vector<double> r;
		uint32_t u_size = 64;
		uint32_t v_size = 64;
};


class RodriguesRotationNode : public MaterialNode {
	public:
		RodriguesRotationNode(int &unique_id, const char *name = "Rodrigues' Rotation Node");
		RodriguesRotationNode(const json& j);
		void Compute(const Argument& global_arg, dvec3& data) const override;
		void DumpJson(json& j) const override;
		void RenderEditor(void) override;
	private:
		dvec3 n = dvec3(0.0, 0.0, 1.0);
		double theta;
};


class NodeMaterial {
	public:
		NodeMaterial(const char *name, const char *settings_path);
		explicit NodeMaterial(const json& j, const char *settings_path);
		~NodeMaterial(void);
		void RenderNode(void);
		struct PinInfo *FindPin(int iid);
#ifndef _CLI
		struct PinInfo *FindPin(const ed::PinId& id);
		struct LinkInfo *FindLink(const ed::LinkId& id);
		const struct PinInfo *FindPinConst(const ed::PinId& id) const;
		const struct LinkInfo *FindLinkConst(const ed::LinkId& id) const;
#endif
		void AddLink(PinInfo *input, PinInfo *b);
		void DumpJson(json& j) const;
		void PreProcess(HitRecord& rec) const;
		bool SampleBSDF(RayType type, const HitRecord& rec, const ONB& uvw, const dvec3& vo, double wlo, dvec3& vi, double& wli, double& bxdf_divided_by_pdf, double& BSDF, double& pdfval) const;
		double BSDF(const dvec3& vi, double wli, const dvec3& vo, double wlo, const dvec3& vt) const;
		double PDF(const dvec3& vi, double wli, const dvec3& vo, double wlo, const dvec3& vt) const;
		double Emitted(const ray& r, const HitRecord& rec, const dvec3& vt) const;
		std::string name;
		std::string settings_file;
#ifndef _CLI
		ax::NodeEditor::EditorContext *context = nullptr;
#endif
		bool light_flag = false;
		std::vector<MaterialNode *> material_nodes;
	private:
		int unique_id = 1;
		std::vector<LinkInfo *> links;


		size_t uv_i = 0;
		size_t Output_i = 1;
};


#endif
