
#ifndef TNT_FILAMENT_DETAILS_SHADERGENERATOR_H
#define TNT_FILAMENT_DETAILS_SHADERGENERATOR_H


#include "MaterialInfo.h"

#include "UibGenerator.h"

#include "MaterialEnums.h"

#include "MaterialBuilder.h"//<filamat/>

#include "EngineEnums.h"//<private/filament/>
#include "Variant.h"

#include "test/DriverEnums.h"//<backend/DriverEnums.h>

#include <test/utils/CString.h>
#include <test/utils/sstream.h>

#include <string>

#include <stdint.h>
#include <stddef.h>

namespace TEST {

	class CodeGenerator;

	class ShaderGenerator {
	public:
		ShaderGenerator(
			MaterialBuilder::PropertyList const& properties,
			MaterialBuilder::VariableList const& variables,
			MaterialBuilder::OutputList const& outputs,
			MaterialBuilder::PreprocessorDefineList const& defines,
			MaterialBuilder::ConstantList const& constants,
			MaterialBuilder::PushConstantList const& pushConstants,
			utils::CString const& materialCode,
			size_t lineOffset,
			utils::CString const& materialVertexCode,
			size_t vertexLineOffset,
			MaterialBuilder::MaterialDomain materialDomain) noexcept;

		std::string createVertexProgram(
			MaterialInfo const& material, Variant variant,
			TEST::Interpolation interpolation,
			TEST::VertexDomain vertexDomain) const noexcept;

		std::string createFragmentProgram(
			MaterialInfo const& material, Variant variant,
			Interpolation interpolation,
			UserVariantFilterMask variantFilter) const noexcept;

		std::string createComputeProgram(
			MaterialInfo const& material) const noexcept;

		/**
		 * When a GLSL shader is optimized we run it through an intermediate SPIR-V
		 * representation. Unfortunately external samplers cannot be used with SPIR-V
		 * at this time, so we must transform them into regular 2D samplers. This
		 * fixup step can be used to turn the samplers back into external samplers after
		 * the optimizations have been applied.
		 */
		static void fixupExternalSamplers(std::string& shader,
			MaterialBuilder::FeatureLevel featureLevel,
			MaterialInfo const& material) noexcept;

		static DescriptorSetLayout getPerViewDescriptorSetLayoutWithVariant(
			Variant variant,
			UserVariantFilterMask variantFilter,
			bool isLit,
			ReflectionMode reflectionMode,
			RefractionMode refractionMode);

	private:
		static void generateVertexDomainDefines(TEST::sstream& out,
			VertexDomain domain) noexcept;

		static void generateSurfaceMaterialVariantProperties(TEST::sstream& out,
			MaterialBuilder::PropertyList const properties,
			const MaterialBuilder::PreprocessorDefineList& defines) noexcept;

		static void generateSurfaceMaterialVariantDefines(TEST::sstream& out,
			ShaderStage stage,
			MaterialBuilder::FeatureLevel featureLevel,
			MaterialInfo const& material, Variant variant) noexcept;

		static void generatePostProcessMaterialVariantDefines(TEST::sstream& out,
			PostProcessVariant variant) noexcept;

		static void generateUserSpecConstants(
			const CodeGenerator& cg, TEST::sstream& fs,
			MaterialBuilder::ConstantList const& constants);

		std::string createPostProcessVertexProgram(
			MaterialInfo const& material, TEST::Variant::type_t variantKey) const noexcept;

		std::string createPostProcessFragmentProgram(
			MaterialInfo const& material, uint8_t variant) const noexcept;

		static void appendShader(TEST::sstream& ss,
			const utils::CString& shader, size_t lineOffset) noexcept;

		static bool hasSkinningOrMorphing(
			Variant variant,
			MaterialBuilder::FeatureLevel featureLevel) noexcept;

		static bool hasStereo(
			Variant variant,
			MaterialBuilder::FeatureLevel featureLevel) noexcept;

		MaterialBuilder::PropertyList mProperties;
		MaterialBuilder::VariableList mVariables;
		MaterialBuilder::OutputList mOutputs;
		MaterialBuilder::MaterialDomain mMaterialDomain;
		MaterialBuilder::PreprocessorDefineList mDefines;
		MaterialBuilder::ConstantList mConstants;
		MaterialBuilder::PushConstantList mPushConstants;
		utils::CString mMaterialFragmentCode;   // fragment or compute code
		utils::CString mMaterialVertexCode;
		size_t mMaterialLineOffset;
		size_t mMaterialVertexLineOffset;
		bool mIsMaterialVertexShaderEmpty;
	};

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_SHADERGENERATOR_H
