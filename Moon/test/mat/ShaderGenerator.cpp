#include "ShaderGenerator.h"

#include "CodeGenerator.h"
#include "SibGenerator.h"
#include "UibGenerator.h"

#include "MaterialEnums.h"

#include "DescriptorSets.h"
#include "EngineEnums.h"
#include "Variant.h"

#include "MaterialBuilder.h"

#include <test/DriverEnums.h>

#include <test/utils/CString.h>
//#include <utils/debug.h>
#include "test/utils/sstream.h"

#include <algorithm>
#include <iterator>

#include <stddef.h>
#include <stdint.h>

namespace TEST {


	using namespace utils;

	void ShaderGenerator::generateSurfaceMaterialVariantDefines(TEST::sstream& out,
		ShaderStage stage,
		MaterialInfo const& material, Variant variant) noexcept {

		out << '\n';


	}

	void ShaderGenerator::generateSurfaceMaterialVariantProperties(sstream& out,
		MaterialBuilder::PropertyList const properties,
		const MaterialBuilder::PreprocessorDefineList& defines) noexcept {
		// Additional, user-provided defines.
		for (const auto& define : defines) {
			CodeGenerator::generateDefine(out, define.name.c_str(), define.value.c_str());
		}
	}

	void ShaderGenerator::generateVertexDomainDefines(sstream& vs, VertexDomain domain) noexcept {
		switch (domain) {
		case VertexDomain::OBJECT:
			CodeGenerator::generateDefine(vs, "VERTEX_DOMAIN_OBJECT", true);
			break;
		case VertexDomain::WORLD:
			CodeGenerator::generateDefine(vs, "VERTEX_DOMAIN_WORLD", true);
			break;
		case VertexDomain::VIEW:
			CodeGenerator::generateDefine(vs, "VERTEX_DOMAIN_VIEW", true);
			break;
		case VertexDomain::DEVICE:
			CodeGenerator::generateDefine(vs, "VERTEX_DOMAIN_DEVICE", true);
			break;
		}
	}

	void ShaderGenerator::generatePostProcessMaterialVariantDefines(sstream& out,
		PostProcessVariant variant) noexcept {
		switch (variant) {
		case PostProcessVariant::OPAQUE:
			CodeGenerator::generateDefine(out, "POST_PROCESS_OPAQUE", 1u);
			break;
		case PostProcessVariant::TRANSLUCENT:
			CodeGenerator::generateDefine(out, "POST_PROCESS_OPAQUE", 0u);
			break;
		}
	}

	void ShaderGenerator::appendShader(sstream& ss,
		const CString& shader, size_t lineOffset) noexcept {

		auto countLines = [](const char* s) -> size_t {
			size_t lines = 0;
			size_t i = 0;
			while (s[i]) {
				if (s[i++] == '\n') lines++;
			}
			return lines;
			};

		if (!shader.empty()) {
			size_t lines = countLines(ss.c_str());

			ss << shader.c_str();
			if (shader[shader.size() - 1] != '\n') {
				ss << "\n";
				lines++;
			}
		}
	}

	void ShaderGenerator::generateUserSpecConstants(
		const CodeGenerator& cg, sstream& fs, MaterialBuilder::ConstantList const& constants) {
		// Constants 0 to CONFIG_MAX_RESERVED_SPEC_CONSTANTS - 1 are reserved by Filament.
		size_t index = CONFIG_MAX_RESERVED_SPEC_CONSTANTS;
		for (const auto& constant : constants) {
			std::string const fullName = std::string("materialConstants_") + constant.name.c_str();
			switch (constant.type) {
			case ConstantType::INT:
				cg.generateSpecializationConstant(
					fs, fullName.c_str(), index++, constant.defaultValue.i);
				break;
			case ConstantType::FLOAT:
				cg.generateSpecializationConstant(
					fs, fullName.c_str(), index++, constant.defaultValue.f);
				break;
			case ConstantType::BOOL:
				cg.generateSpecializationConstant(
					fs, fullName.c_str(), index++, constant.defaultValue.b);
				break;
			}
		}
	}

	// ------------------------------------------------------------------------------------------------

	ShaderGenerator::ShaderGenerator(
		MaterialBuilder::PropertyList const& properties,
		MaterialBuilder::VariableList const& variables,
		MaterialBuilder::OutputList const& outputs,
		MaterialBuilder::PreprocessorDefineList const& defines,
		MaterialBuilder::ConstantList const& constants,
		MaterialBuilder::PushConstantList const& pushConstants,
		CString const& materialCode, size_t lineOffset,
		CString const& materialVertexCode, size_t vertexLineOffset,
		MaterialBuilder::MaterialDomain materialDomain) noexcept {

		if (materialDomain == MaterialBuilder::MaterialDomain::COMPUTE) {
			// we shouldn't have a vertex shader in a compute material
			assert(materialVertexCode.empty());
		}

		std::copy(std::begin(properties), std::end(properties), std::begin(mProperties));
		std::copy(std::begin(variables), std::end(variables), std::begin(mVariables));
		std::copy(std::begin(outputs), std::end(outputs), std::back_inserter(mOutputs));

		mMaterialFragmentCode = materialCode;
		mMaterialVertexCode = materialVertexCode;
		mIsMaterialVertexShaderEmpty = materialVertexCode.empty();
		mMaterialLineOffset = lineOffset;
		mMaterialVertexLineOffset = vertexLineOffset;
		mMaterialDomain = materialDomain;
		mDefines = defines;
		mConstants = constants;
		mPushConstants = pushConstants;

		if (mMaterialFragmentCode.empty()) {
			if (mMaterialDomain == MaterialBuilder::MaterialDomain::SURFACE) {
				mMaterialFragmentCode =
					CString("void material(inout MaterialInputs m) {\n    prepareMaterial(m);\n}\n");
			}
			else if (mMaterialDomain == MaterialBuilder::MaterialDomain::POST_PROCESS) {
				mMaterialFragmentCode =
					CString("void postProcess(inout PostProcessInputs p) {\n}\n");
			}
			else if (mMaterialDomain == MaterialBuilder::MaterialDomain::COMPUTE) {
				mMaterialFragmentCode =
					CString("void compute() {\n}\n");
			}
		}
		if (mMaterialVertexCode.empty()) {
			if (mMaterialDomain == MaterialBuilder::MaterialDomain::SURFACE) {
				mMaterialVertexCode =
					CString("void materialVertex(inout MaterialVertexInputs m) {\n}\n");
			}
			else if (mMaterialDomain == MaterialBuilder::MaterialDomain::POST_PROCESS) {
				mMaterialVertexCode =
					CString("void postProcessVertex(inout PostProcessVertexInputs m) {\n}\n");
			}
		}
	}

	void ShaderGenerator::fixupExternalSamplers(std::string& shader,
		MaterialBuilder::FeatureLevel featureLevel,
		MaterialInfo const& material) noexcept {
		// External samplers are only supported on GL ES at the moment, we must
		// skip the fixup on desktop targets
		if (material.hasExternalSamplers) {
			CodeGenerator::fixupExternalSamplers(shader, material.sib, featureLevel);
		}
	}

	std::string ShaderGenerator::createVertexProgram(
		MaterialInfo const& material, const Variant variant, Interpolation interpolation,
		VertexDomain vertexDomain) const noexcept {

		assert(mMaterialDomain != MaterialBuilder::MaterialDomain::COMPUTE);
		if (mMaterialDomain == MaterialBuilder::MaterialDomain::POST_PROCESS) {
			return createPostProcessVertexProgram(material, variant.key);
		}

		sstream vs;
		const CodeGenerator cg;
		cg.generateProlog(vs, ShaderStage::VERTEX, material, variant);
		generateUserSpecConstants(cg, vs, mConstants);
		generateSurfaceMaterialVariantDefines(vs, ShaderStage::VERTEX, material, variant);
		generateSurfaceMaterialVariantProperties(vs, mProperties, mDefines);

		AttributeBitset attributes = material.requiredAttributes;


		MaterialBuilder::PushConstantList vertexPushConstants;
		std::copy_if(mPushConstants.begin(), mPushConstants.end(),
			std::back_insert_iterator<MaterialBuilder::PushConstantList>(vertexPushConstants),
			[](MaterialBuilder::PushConstant const& constant) {
				return constant.stage == ShaderStage::VERTEX;
			});
		cg.generateShaderInputs(vs, ShaderStage::VERTEX, attributes, interpolation, vertexPushConstants);



		// custom material variables
		size_t variableIndex = 0;
		for (const auto& variable : mVariables) {
			CodeGenerator::generateVariable(vs, ShaderStage::VERTEX, variable, variableIndex++);
		}

		cg.generateUniforms(vs, ShaderStage::VERTEX,
			DescriptorSetBindingPoints::PER_MATERIAL,
			+PerMaterialBindingPoints::MATERIAL_PARAMS,
			material.uib);

		CodeGenerator::generateSeparator(vs);
		cg.generateSamplers(vs, DescriptorSetBindingPoints::PER_MATERIAL, material.sib);
		// main entry point
		appendShader(vs, mMaterialVertexCode, mMaterialVertexLineOffset);
		CodeGenerator::generateEpilog(vs);
		return vs.c_str();
	}

	std::string ShaderGenerator::createFragmentProgram(MaterialInfo const& material, const Variant variant,
		Interpolation interpolation, UserVariantFilterMask variantFilter) const noexcept {
		assert(mMaterialDomain != MaterialBuilder::MaterialDomain::COMPUTE);

		if (mMaterialDomain == MaterialBuilder::MaterialDomain::POST_PROCESS) {
			return createPostProcessFragmentProgram(material, variant.key);
		}

		const CodeGenerator cg;

		sstream fs;
		cg.generateProlog(fs, ShaderStage::FRAGMENT, material, variant);

		generateUserSpecConstants(cg, fs, mConstants);

		generateSurfaceMaterialVariantDefines(
			fs, ShaderStage::FRAGMENT, material, variant);



		generateSurfaceMaterialVariantProperties(fs, mProperties, mDefines);

		MaterialBuilder::PushConstantList fragmentPushConstants;
		std::copy_if(mPushConstants.begin(), mPushConstants.end(),
			std::back_insert_iterator<MaterialBuilder::PushConstantList>(fragmentPushConstants),
			[](MaterialBuilder::PushConstant const& constant) {
				return constant.stage == ShaderStage::FRAGMENT;
			});
		cg.generateShaderInputs(fs, ShaderStage::FRAGMENT, material.requiredAttributes, interpolation,
			fragmentPushConstants);

		//CodeGenerator::generateCommonTypes(fs, ShaderStage::FRAGMENT);

		// custom material variables
		size_t variableIndex = 0;
		for (const auto& variable : mVariables) {
			CodeGenerator::generateVariable(fs, ShaderStage::FRAGMENT, variable, variableIndex++);
		}
		cg.generateUniforms(fs, ShaderStage::FRAGMENT,
			DescriptorSetBindingPoints::PER_MATERIAL,
			+PerMaterialBindingPoints::MATERIAL_PARAMS,
			material.uib);
		CodeGenerator::generateSeparator(fs);


		// shading code


		appendShader(fs, mMaterialFragmentCode, mMaterialLineOffset);

		CodeGenerator::generateEpilog(fs);

		return fs.c_str();
	}

	std::string ShaderGenerator::createComputeProgram(
		MaterialInfo const& material) const noexcept {
		assert(mMaterialDomain == MaterialBuilder::MaterialDomain::COMPUTE);
		//assert(featureLevel >= FeatureLevel::FEATURE_LEVEL_2);
		const CodeGenerator cg;
		sstream s;

		cg.generateProlog(s, ShaderStage::COMPUTE, material, {});

		generateUserSpecConstants(cg, s, mConstants);

		CodeGenerator::generateCommonTypes(s, ShaderStage::COMPUTE);

		cg.generateUniforms(s, ShaderStage::COMPUTE,
			DescriptorSetBindingPoints::PER_VIEW,
			+PerViewBindingPoints::FRAME_UNIFORMS,
			UibGenerator::getPerViewUib());

		cg.generateUniforms(s, ShaderStage::COMPUTE,
			DescriptorSetBindingPoints::PER_MATERIAL,
			+PerMaterialBindingPoints::MATERIAL_PARAMS,
			material.uib);

		cg.generateSamplers(s, DescriptorSetBindingPoints::PER_MATERIAL, material.sib);

		// generate SSBO
		cg.generateBuffers(s, material.buffers);

		// TODO: generate images

		CodeGenerator::generateCommon(s, ShaderStage::COMPUTE);

		CodeGenerator::generateGetters(s, ShaderStage::COMPUTE);

		appendShader(s, mMaterialFragmentCode, mMaterialLineOffset);

		CodeGenerator::generateShaderMain(s, ShaderStage::COMPUTE);

		CodeGenerator::generateEpilog(s);
		return s.c_str();
	}

	std::string ShaderGenerator::createPostProcessVertexProgram(
		MaterialInfo const& material, const Variant::type_t variantKey) const noexcept {
		const CodeGenerator cg;
		sstream vs;
		cg.generateProlog(vs, ShaderStage::VERTEX, material, {});

		generateUserSpecConstants(cg, vs, mConstants);

		CodeGenerator::generateDefine(vs, "LOCATION_POSITION", uint32_t(VertexAttribute::POSITION));

		// custom material variables
		size_t variableIndex = 0;
		for (const auto& variable : mVariables) {
			CodeGenerator::generateVariable(vs, ShaderStage::VERTEX, variable, variableIndex++);
		}

		CodeGenerator::generatePostProcessInputs(vs, ShaderStage::VERTEX);
		generatePostProcessMaterialVariantDefines(vs, PostProcessVariant(variantKey));

		cg.generateUniforms(vs, ShaderStage::VERTEX,
			DescriptorSetBindingPoints::PER_VIEW,
			+PerViewBindingPoints::FRAME_UNIFORMS,
			UibGenerator::getPerViewUib());

		cg.generateUniforms(vs, ShaderStage::VERTEX,
			DescriptorSetBindingPoints::PER_MATERIAL,
			+PerMaterialBindingPoints::MATERIAL_PARAMS,
			material.uib);

		cg.generateSamplers(vs, DescriptorSetBindingPoints::PER_MATERIAL, material.sib);

		CodeGenerator::generatePostProcessCommon(vs, ShaderStage::VERTEX);
		CodeGenerator::generatePostProcessGetters(vs, ShaderStage::VERTEX);

		appendShader(vs, mMaterialVertexCode, mMaterialVertexLineOffset);

		CodeGenerator::generatePostProcessMain(vs, ShaderStage::VERTEX);

		CodeGenerator::generateEpilog(vs);
		return vs.c_str();
	}

	std::string ShaderGenerator::createPostProcessFragmentProgram(
		MaterialInfo const& material, uint8_t variant) const noexcept {
		const CodeGenerator cg;
		sstream fs;
		cg.generateProlog(fs, ShaderStage::FRAGMENT, material, {});

		generateUserSpecConstants(cg, fs, mConstants);

		generatePostProcessMaterialVariantDefines(fs, PostProcessVariant(variant));

		// custom material variables
		size_t variableIndex = 0;
		for (const auto& variable : mVariables) {
			CodeGenerator::generateVariable(fs, ShaderStage::FRAGMENT, variable, variableIndex++);
		}

		cg.generateUniforms(fs, ShaderStage::FRAGMENT,
			DescriptorSetBindingPoints::PER_VIEW,
			+PerViewBindingPoints::FRAME_UNIFORMS,
			UibGenerator::getPerViewUib());

		cg.generateUniforms(fs, ShaderStage::FRAGMENT,
			DescriptorSetBindingPoints::PER_MATERIAL,
			+PerMaterialBindingPoints::MATERIAL_PARAMS,
			material.uib);

		cg.generateSamplers(fs, DescriptorSetBindingPoints::PER_MATERIAL, material.sib);

		// subpass
		//CodeGenerator::generateSubpass(fs, material.subpass);

		CodeGenerator::generatePostProcessCommon(fs, ShaderStage::FRAGMENT);
		CodeGenerator::generatePostProcessGetters(fs, ShaderStage::FRAGMENT);

		// Generate post-process outputs.
		for (const auto& output : mOutputs) {
			if (output.target == MaterialBuilder::OutputTarget::COLOR) {
				cg.generateOutput(fs, ShaderStage::FRAGMENT, output.name, output.location,
					output.qualifier, output.precision, output.type);
			}
			if (output.target == MaterialBuilder::OutputTarget::DEPTH) {
				CodeGenerator::generateDefine(fs, "FRAG_OUTPUT_DEPTH", 1u);
			}
		}

		CodeGenerator::generatePostProcessInputs(fs, ShaderStage::FRAGMENT);

		appendShader(fs, mMaterialFragmentCode, mMaterialLineOffset);

		CodeGenerator::generatePostProcessMain(fs, ShaderStage::FRAGMENT);
		CodeGenerator::generateEpilog(fs);
		return fs.c_str();
	}

	bool ShaderGenerator::hasSkinningOrMorphing(
		Variant variant) noexcept {
		return variant.hasSkinningOrMorphing();
	}

	bool ShaderGenerator::hasStereo(
		Variant variant) noexcept {
		return variant.hasStereo();
	}

	DescriptorSetLayout ShaderGenerator::getPerViewDescriptorSetLayoutWithVariant(
		Variant variant,
		UserVariantFilterMask variantFilter,
		bool isLit,
		ReflectionMode reflectionMode,
		RefractionMode refractionMode) {
		if (Variant::isValidDepthVariant(variant)) {
			return TEST::getDepthVariantLayout();
		}
		if (Variant::isSSRVariant(variant)) {
			return TEST::getSsrVariantLayout();
		}
		// We need to filter out all the descriptors not included in the "resolved" layout below
		return TEST::getPerViewDescriptorSetLayout(
			MaterialDomain::SURFACE, variantFilter,
			isLit, reflectionMode, refractionMode);
	}

} // namespace filament
