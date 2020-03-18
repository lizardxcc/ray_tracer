#ifndef MATERIALNODE_H
#define MATERIALNODE_H

#include <boost/filesystem/path.hpp>
#include <boost/bimap/bimap.hpp>
#include <boost/assign.hpp>
#ifndef _CLI
//#include "imgui_node_editor.h"
#include "imgui.h"
#include "ImNodes.h"
#include "ImNodesEz.h"
#endif
#include "vec3.h"
#include "hittable.h"
#include "spectrum.h"
#include "onb.h"
#include "json.hpp"
using json = nlohmann::json;
#include <boost/preprocessor.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/cat.hpp>


#define ADD_COMMA(r, data, elem) elem,

#define DEFINE_ENUM(enumname, SEQ, use_enumclass) enum BOOST_PP_IIF(use_enumclass, class, ) enumname {\
	BOOST_PP_SEQ_FOR_EACH(ADD_COMMA, _, SEQ)\
};\
std::string BOOST_PP_CAT(enumname, ToString)(enumname type);\
enumname BOOST_PP_CAT(StringTo, enumname)(const std::string& name);

#define TO_STRING_SWITHCASE(r, enumname, elem) case enumname::elem:\
	return std::string(BOOST_PP_STRINGIZE(elem));
#define TO_ENUM_IFELSE(r, enumname, elem) if (name == BOOST_PP_STRINGIZE(elem)) {\
	return enumname::elem;\
}

#define DEFINE_ENUM_FUNCTION(enumname, SEQ)\
std::string BOOST_PP_CAT(enumname, ToString)(enumname type)\
{\
	switch(type) {\
		BOOST_PP_SEQ_FOR_EACH(TO_STRING_SWITHCASE, enumname, SEQ)\
	}\
}\
enumname BOOST_PP_CAT(StringTo, enumname)(const std::string& name)\
{\
	BOOST_PP_SEQ_FOR_EACH(TO_ENUM_IFELSE, enumname, SEQ)\
	assert(false);\
}


class MaterialNode;
struct PinInfo;
struct HitRecord;

struct RandomData {
	std::vector<double> randoms;
};

struct Connection {
	MaterialNode *input_node = nullptr;
	const char *input_slot = nullptr;
	MaterialNode *output_node = nullptr;
	const char *output_slot = nullptr;
	bool operator==(const Connection& other) const
	{
		return (input_node == other.input_node &&
			input_slot == other.input_slot &&
			output_node == other.output_node &&
			output_slot == other.output_slot);
	};

	bool operator!=(const Connection& other) const
	{
		return !operator==(other);
	};
};


#define SLOTTYPE_SEQ (ZERO)(SlotDouble)(SlotVec3)(SlotSpectrum)(SlotUniversal)(SlotBSDF)
DEFINE_ENUM(SlotType, SLOTTYPE_SEQ, 0)
#define MATNODETYPE_SEQ (Lambertian)(Conductor)(ColoredMetal)(DiffuseLight)(Output)(UV)(Spectrum)(RGBColor)(RGBtoSpectrum)(ImageTexture)(Checkerboard)(Addition)(Multiplication)(ScalarMultiplication)(RandomSampling)(GGXReflection)(AccessVec3Component)(CombineVec3Component)(ValueNoise)(ValueNoise2D)(RodriguesRotation)(Dielectric)
DEFINE_ENUM(MaterialNodeType, MATNODETYPE_SEQ, 1)

struct Argument {
	dvec3 vt;
};



class MaterialNode {
	public:
		MaterialNode(const char *name = "");
		explicit MaterialNode(const json& j);
		void RenderNode(void);
		virtual void RenderEditor(void);
		void RenderSpectrum(Spectrum& data, double min, double max);
		const MaterialNode *GetInputParentNode(const char *slot_name) const;
		virtual void Compute(const Argument& global_arg, double& data) const;
		virtual void Compute(const Argument& global_arg, Spectrum& data) const;
		virtual void Compute(const Argument& global_arg, dvec3& data) const;
		void AddInput(SlotType type, const char *name);
		void AddOutput(SlotType type, const char *name);
		void GenerateRandomData(const RandomData& rand_data);
		void DeleteConnection(const Connection& connection);


		virtual void DumpJson(json& j) const;
		void DumpIO(json& j) const;
		void DumpSpectrum(json& j, const Spectrum& s, const char *name) const;

#ifndef _CLI
		//const static ImVec4 pin_colors[];
#endif
		std::string name;
		ImVec2 pos;
		bool selected = false;
		enum MaterialNodeType type;
		std::vector<Connection> connections;
		std::vector<ImNodes::Ez::SlotInfo> input_slots;
		std::vector<ImNodes::Ez::SlotInfo> output_slots;
	protected:
		//void UpdateNormal(const PinInfo *normal_pin, const HitRecord& rec, dvec3& new_normal) const;
};

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
		SpectrumNode(const char *name = "Spectrum");
		SpectrumNode(const json& j);
		void DumpJson(json& j) const override;
		void RenderEditor(void) override;
		void Compute(const Argument& global_arg, Spectrum& data) const override;
	private:
		Spectrum data = Spectrum(1.0);
};

class LambertianNode : public BSDFMaterialNode {
	public:
		LambertianNode(const char *name = "Lambertian");
		LambertianNode(const json& j);
		void DumpJson(json& j) const override;
		void RenderEditor(void) override;
		void PreProcess(const Argument& global_arg, HitRecord& rec) const override;
		bool SampleBSDF(const Argument& global_arg, RayType type, const HitRecord& rec, const ONB& uvw, const dvec3& vo, double wlo, dvec3& vi, double& wli, double& bxdf_divided_by_pdf, double& BSDF, double& pdfval) const override;
		double BSDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const override;
		double PDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const override;
	private:
		Spectrum albedo = Spectrum(1.0);
		//const PinInfo *albedo_pin;
		//const PinInfo *normal_pin;
};

class DielectricNode : public BSDFMaterialNode {
	public:
		DielectricNode(const char *name = "Dielectric");
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
		//const PinInfo *normal_pin;
};

class ConductorNode : public BSDFMaterialNode {
	public:
		ConductorNode(const char *name = "Conductor");
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
		//const PinInfo *normal_pin;
};


class ColoredMetalNode : public BSDFMaterialNode {
	public:
		ColoredMetalNode(const char *name = "Colored Metal");
		explicit ColoredMetalNode(const json& j);
		void DumpJson(json& j) const override;
		void RenderEditor(void) override;
		void PreProcess(const Argument& global_arg, HitRecord& rec) const override;
		bool SampleBSDF(const Argument& global_arg, RayType type, const HitRecord& rec, const ONB& uvw, const dvec3& vo, double wlo, dvec3& vi, double& wli, double& bxdf_divided_by_pdf, double& BSDF, double& pdfval) const override;
		double BSDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const override;
		double PDF(const Argument& global_arg, const dvec3& vi, double wli, const dvec3& vo, double wlo) const override;
	private:
		Spectrum albedo = Spectrum(1.0);
		//const PinInfo *albedo_pin;
		//const PinInfo *normal_pin;
};

class GGXReflectionNode : public BSDFMaterialNode {
	public:
		GGXReflectionNode(const char *name = "GGXReflectionNode");
		explicit GGXReflectionNode(const json& j);
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
		//const PinInfo *n_pin;
		//const PinInfo *k_pin;
		double alpha;
};


class DiffuseLightNode : public BSDFMaterialNode {
	public:
		DiffuseLightNode(const char *name = "Diffuse Light");
		explicit DiffuseLightNode(const json& j);
		void DumpJson(json& j) const override;
		void RenderEditor(void) override;
		void PreProcess(const Argument& global_arg, HitRecord& rec) const override;
		double Emitted(const Argument& global_arg, const ray& r, const HitRecord& rec) const override;
	private:
		Spectrum color = Spectrum(1.0);
		//const PinInfo *color_pin;
		//const PinInfo *normal_pin;
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
		OutputNode(const char *name = "Output");
		OutputNode(const json& j);
		void RenderEditor(void) override;
};

class UVNode : public MaterialNode {
	public:
		UVNode(const char *name = "");
		UVNode(const json& j);
		void Compute(const Argument& global_arg, dvec3& data) const override;
};


class RGBColorNode : public MaterialNode {
	public:
		RGBColorNode(const char *name = "RGB Color");
		RGBColorNode(const json& j);
		void DumpJson(json& j) const override;
		void RenderEditor(void) override;
		void Compute(const Argument& global_arg, dvec3& data) const override;
	private:
		float col[3] = {0.0f, 0.0f, 0.0f};
};

class RGBtoSpectrumNode : public MaterialNode {
	public:
		RGBtoSpectrumNode(const char *name = "RGBtoSpectrumNode");
		RGBtoSpectrumNode(const json& j);
		void Compute(const Argument& global_arg, Spectrum& data) const override;
};

class ImageTextureNode : public MaterialNode {
	public:
		ImageTextureNode(const char *path = "", const char *name = "Image Texture");
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
		CheckerboardNode(const char *name = "Checkerboard");
		CheckerboardNode(const json& j);
		void DumpJson(json& j) const override;
		void Compute(const Argument& global_arg, dvec3& data) const override;
		void RenderEditor(void) override;
	private:
		double size = 0.5;
};

class AdditionNode : public MaterialNode {
	public:
		AdditionNode(const char *name = "Add");
		AdditionNode(const json& j);
		void Compute(const Argument& global_arg, double& data) const override;
		void Compute(const Argument& global_arg, dvec3& data) const override;
		void Compute(const Argument& global_arg, Spectrum& data) const override;
};

class MultiplicationNode : public MaterialNode {
	public:
		MultiplicationNode(const char *name = "Multiply");
		MultiplicationNode(const json& j);
		void Compute(const Argument& global_arg, double& data) const override;
		void Compute(const Argument& global_arg, dvec3& data) const override;
		void Compute(const Argument& global_arg, Spectrum& data) const override;
};

class ScalarMultiplicationNode : public MaterialNode {
	public:
		ScalarMultiplicationNode(const char *name = "Multiply Scalar");
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
		RandomSamplingNode(const char *name = "Multiply Scalar");
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
		AccessVec3ComponentNode(const char *name = "AccessVec3ComponentNode");
		AccessVec3ComponentNode(const json& j);
		void Compute(const Argument& global_arg, double& data) const override;
		void RenderEditor(void) override;
	private:
		uint32_t index = 0;
};


class CombineVec3ComponentNode : public MaterialNode {
	public:
		CombineVec3ComponentNode(const char *name = "CombineVec3ComponentNode");
		CombineVec3ComponentNode(const json& j);
		void Compute(const Argument& global_arg, dvec3& data) const override;
	private:
};

class ValueNoiseNode : public MaterialNode {
	public:
		ValueNoiseNode(const char *name = "Value Noise");
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
		ValueNoise2DNode(const char *name = "Value Noise 2D");
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
		RodriguesRotationNode(const char *name = "Rodrigues' Rotation Node");
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
		NodeMaterial(const char *name);
		explicit NodeMaterial(const json& j);
		~NodeMaterial(void);
		void RenderNode(void);
#ifndef _CLI
		ImNodes::CanvasState canvas;
#endif
		void DumpJson(json& j) const;
		void PreProcess(HitRecord& rec, RandomData& rand_data) const;
		bool SampleBSDF(RayType type, const HitRecord& rec, const ONB& uvw, const dvec3& vo, double wlo, dvec3& vi, double& wli, double& bxdf_divided_by_pdf, double& BSDF, double& pdfval) const;
		double BSDF(const dvec3& vi, double wli, const dvec3& vo, double wlo, const dvec3& vt) const;
		double PDF(const dvec3& vi, double wli, const dvec3& vo, double wlo, const dvec3& vt) const;
		double Emitted(const ray& r, const HitRecord& rec, const dvec3& vt) const;
		std::string name;
		bool light_flag = false;
		std::vector<MaterialNode *> material_nodes;
		size_t selected_nodes_count = 0;
		size_t last_selected_node_index = 0;
	private:
		size_t uv_i = 0;
		size_t Output_i = 1;
};


#endif
